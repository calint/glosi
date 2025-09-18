#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-01-16
// reviewed: 2024-07-08
// reviewed: 2024-07-19

#include "decouple.hpp"
#include "globs.hpp"
#include "net.hpp"
#include "o1store.hpp"
#include "planes.hpp"

namespace glos {

class object {
    friend class grid;
    friend class cell;

  public:
    // members in order they are accessed by 'grid::add', 'cell::add',
    //  'cell::update', 'cell:resolve_collisions', 'cell::render' with cache
    //   coherence in mind

    // -- grid::add, cell::add
    glm::vec3 position{};        // in meters
    float bounding_radius = 0;   // in meters
    uint32_t collision_bits = 0; // mask & bits for collision subscription
    uint32_t collision_mask = 0; // ...
  private:
    bool overlaps_cells = false; // used by grid to flag cell overlap
  public:
    bool is_static = false; // immovable object
                            // -- cell::update
  private:
    std::atomic_flag lock = ATOMIC_FLAG_INIT;
    uint32_t updated_at_tick = 0; // used by cell to avoid updating twice
  public:
    glm::vec3 acceleration{};     // in meters/second^2
    glm::vec3 linear_velocity{};  // in meters/second
    glm::vec3 angular_velocity{}; // in radians/second
    glm::vec3 angle{};            // in radians
  private:
    std::vector<object const*> handled_collisions{};
    bool is_dead = false; // used by 'cell' to avoid events to dead objects
  public:
    // -- cell::resolve_collisions: spheres
    bool is_sphere = false; // true if object can be considered a sphere
    float mass = 0;         // in kg
  private:
    // -- cell::resolve_collisions: planes
    planes planes{}; // bounding planes (if any)
    std::atomic_flag lock_Mmw = ATOMIC_FLAG_INIT;
    glm::vec3 Mmw_pos{}; // position of current Mmw matrix
    glm::vec3 Mmw_agl{}; // angle of current Mmw matrix
    glm::vec3 Mmw_scl{}; // scale of current Mmw matrix
  public:
    glm::vec3 scale{}; // in meters
  private:
    glm::mat4 Mmw{}; // model -> world matrix
    // -- cell::render
    uint32_t rendered_at_tick = 0; // used by 'cell' to avoid rendering twice
    uint32_t glob_ix_ = 0;         // index in globs store
  public:
    // -- other
    // rest of object public state
    net_state* net_state = nullptr; // pointer to signals used by this object
    std::string name{};             // instance name
    object** alloc_ptr;             // initiated at allocate by 'o1store'

    // note: 32 bit resolution of 'updated_at_tick' and 'rendered_at_tick' vs 64
    //       bit comparison source ok since only checking for equality

  public:
    inline virtual ~object() = default;
    // note: 'delete obj;' may not be used because memory is managed by
    // 'o1store'.
    //       destructor is invoked at 'objects.apply_free(...)'

    // called from 'cell'
    inline virtual auto render() -> void {
        glob().render(updated_Mmw());

        if (is_debug_object_planes_normals) {
            planes.debug_render_normals();
        }

        if (is_debug_object_bounding_sphere) {
            debug_render_bounding_sphere(debug_get_Mmw_for_bounding_sphere());
        }
    }

    // called from 'cell'
    // @return false if object has died, true otherwise
    // note: only one thread at a time is active in this section
    inline virtual auto update() -> bool {
        float const dt = frame_context.dt;
        linear_velocity += acceleration * dt;
        position += linear_velocity * dt;
        angle += angular_velocity * dt;

        if (is_debug_object_planes_normals) {
            // note: update planes for the normals to be rendered at 'render()'
            glm::mat4 const& M = updated_Mmw();
            class glob const& g = glob();
            planes.update_model_to_world(g.planes_points, g.planes_normals, M,
                                         position, angle, scale);
        }

        return true;
    }

    // called from 'cell'
    // note: only on thread at a time is active in this section
    // @return false if object has died, true otherwise
    inline virtual auto on_collision([[maybe_unused]] object* obj) -> bool {
        return true;
    }

    inline auto updated_Mmw() -> glm::mat4 const& {
        // * synchronize if render and update run on different threads; both
        // racing
        //   for this function from 'render()' and 'update()'
        // * synchronize if 'threaded_grid' because objects in different cells
        //   running on different threads might race when calling this function

        bool constexpr synchronize = threaded_update || threaded_grid;

        if (synchronize) {
            while (lock_Mmw.test_and_set(std::memory_order_acquire)) {
            }
        }

        if (position == Mmw_pos && angle == Mmw_agl && scale == Mmw_scl) {
            if (synchronize) {
                lock_Mmw.clear(std::memory_order_release);
            }
            return Mmw;
        }

        // save the state of the matrix
        Mmw_pos = position;
        Mmw_agl = angle;
        Mmw_scl = scale;
        // make a new matrix
        glm::mat4 const Ms = glm::scale(glm::mat4(1), Mmw_scl);
        glm::mat4 const Mr =
            glm::eulerAngleXYZ(Mmw_agl.x, Mmw_agl.y, Mmw_agl.z);
        glm::mat4 const Mt = glm::translate(glm::mat4(1), Mmw_pos);
        Mmw = Mt * Mr * Ms;

        if (synchronize) {
            lock_Mmw.clear(std::memory_order_release);
        }

        return Mmw;
    }

    inline auto glob_ix(uint32_t const i) -> void {
        if (glob_ix_ != i) {
            planes.invalidated = true;
        }
        glob_ix_ = i;
    }

    inline auto glob_ix() const -> uint32_t { return glob_ix_; }

    inline auto glob() const -> glob const& { return globs.at(glob_ix_); }

  private:
    // called from 'cell' in thread safe way
    inline auto clear_handled_collisions() -> void {
        handled_collisions.clear();
    }

    // called from 'cell' in thread safe way
    inline auto is_collision_handled_and_if_not_add(object const* obj) -> bool {
        bool const found = std::ranges::find(handled_collisions, obj) !=
                           handled_collisions.cend();

        if (!found) {
            handled_collisions.push_back(obj);
        }

        return found;
    }

    // called from 'cell'
    inline auto update_planes_world_coordinates() -> void {
        bool const synchronize = threaded_grid && overlaps_cells;

        class glob const& g = glob();

        if (synchronize) {
            planes.acquire_lock();
        }

        glm::mat4 const& M = updated_Mmw();
        planes.update_model_to_world(g.planes_points, g.planes_normals, M,
                                     Mmw_pos, Mmw_agl, Mmw_scl);

        if (synchronize) {
            planes.release_lock();
        }
    }

    inline auto debug_get_Mmw_for_bounding_sphere() const -> glm::mat4 {
        return glm::scale(glm::translate(glm::mat4(1), Mmw_pos),
                          glm::vec3{bounding_radius});
    }

    inline auto acquire_lock() -> void {
        while (lock.test_and_set(std::memory_order_acquire)) {
        }
    }

    inline auto release_lock() -> void {
        lock.clear(std::memory_order_release);
    }
};

class objects final {
  public:
    inline auto init() -> void {}

    inline auto free() -> void {
        for_each([](object* o) { o->~object(); });
        //? what if destructor created objects
    }

    inline auto alloc() -> object* { return store_.allocate_instance(); }

    inline auto free(object* o) -> void { store_.free_instance(o); }

    inline auto allocated_list_len() const -> size_t {
        return allocated_list_len_;
    }

    inline auto for_each(auto&& func) -> void {
        for (object** it = store_.allocated_list(); it < allocated_list_end_;
             ++it) {
            object* obj = *it;
            func(obj);
        }
    }

    inline auto apply_allocated_instances(auto&& callback) -> void {
        // retrieve the end of list because during objects' 'update' and
        //  'on_collision' new objects might be created which would change the
        //   end-of-list pointer
        object** alloc_iter = store_.allocated_list() + allocated_list_len_;
        object** const alloc_end = store_.allocated_list_end();
        while (alloc_iter < alloc_end) {
            object* o = *alloc_iter;
            callback(o);
            ++alloc_iter;
        }
        allocated_list_len_ = store_.allocated_list_len();
        allocated_list_end_ = alloc_end;
    }

    inline auto apply_freed_instances(auto&& callback) -> void {
        store_.apply_free(callback);
    }

  private:
    o1store<object, objects_count, 0, false, threaded_grid,
            objects_instance_size_B, cache_line_size_B>
        store_{};
    object** allocated_list_end_ = nullptr;
    uint32_t allocated_list_len_ = 0;
} static objects{};

} // namespace glos
