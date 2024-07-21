#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-01-16
// reviewed: 2024-07-08
// reviewed: 2024-07-19

namespace glos {

class cell final {
  // object entry in a cell. object members used in the hot code path copied for
  //  better cache utilization
  struct entry {
    glm::vec3 position{};
    float radius = 0;
    uint32_t collision_bits = 0;
    uint32_t collision_mask = 0;
    object *object = nullptr;
  };

  // entry in list of objects whose bounding spheres are in collision. if not
  //  both spheres further processed to check for collision using bounding
  //   planes
  struct collision {
    object *o1 = nullptr;
    object *o2 = nullptr;
    bool notify1 = false;
    bool notify2 = false;
    bool is_collision = false;
  };

  std::vector<entry> entry_list{};
  std::vector<collision> check_collisions_list{};

public:
  // called from grid
  inline auto update() const -> void {
    uint32_t const frame_num = uint32_t(frame_context.frame_num);
    // note: ok to truncate because only equality is checked
    for (entry const &ce : entry_list) {
      if (threaded_grid) {
        // multithreaded mode
        if (ce.object->overlaps_cells) [[unlikely]] {
          // object is in several cells and may be called from multiple threads
          ce.object->acquire_lock();
          if (ce.object->updated_at_tick == frame_num) {
            // object already updated in a different cell by a different thread
            ce.object->release_lock();
            continue;
          }
          ce.object->updated_at_tick = frame_num;
          ce.object->release_lock();
        }
      } else {
        // single threaded mode
        if (ce.object->overlaps_cells) [[unlikely]] {
          // object is in several cells and may be called from multiple cells
          if (ce.object->updated_at_tick == frame_num) {
            // already called from a different cell
            continue;
          }
          ce.object->updated_at_tick = frame_num;
        }
      }

      // only one thread at a time is here for 'ce.object'

      if (!ce.object->update()) {
        ce.object->is_dead = true;
        objects.free(ce.object);
        continue;
      }

      // note: opportunity to clear the list prior to 'resolve_collisions'
      ce.object->clear_handled_collisions();
    }
  }

  inline auto resolve_collisions() -> void {
    make_check_collisions_list();
    process_check_collisions_list();
    handle_check_collisions_list();
  }

  // called from grid (from only one thread)
  inline auto render() const -> void {
    uint32_t const frame_num = uint32_t(frame_context.frame_num);
    // note: ok to truncate because only equality is checked
    for (entry const &ce : entry_list) {
      if (ce.object->overlaps_cells) [[unlikely]] {
        // check if object has been rendered by another cell
        if (ce.object->rendered_at_tick == frame_num) {
          continue;
        }
        ce.object->rendered_at_tick = frame_num;
      }
      ce.object->render();
      ++metrics.rendered_objects;
    }
  }

  // called from grid (from only one thread)
  inline auto clear() -> void { entry_list.clear(); }

  // called from grid (from only one thread)
  inline auto add(object *o) -> void {
    entry_list.emplace_back(o->position, o->bounding_radius, o->collision_bits,
                            o->collision_mask, o);
  }

  inline auto print() const -> void {
    uint32_t i = 0;
    for (entry const &ce : entry_list) {
      if (i++) {
        printf(", ");
      }
      printf("%s", ce.object->name.c_str());
    }
    printf("\n");
  }

  inline auto objects_count() const -> uint32_t {
    return uint32_t(entry_list.size());
  }

private:
  // called from one thread
  inline auto make_check_collisions_list() -> void {
    check_collisions_list.clear();

    uint32_t const len = uint32_t(entry_list.size());
    if (len < 2) {
      return;
    }

    for (uint32_t i = 0; i < len - 1; ++i) {
      for (uint32_t j = i + 1; j < len; ++j) {

        entry const &e1 = entry_list[i];
        entry const &e2 = entry_list[j];

        bool const notify1 = e1.collision_mask & e2.collision_bits;
        bool const notify2 = e2.collision_mask & e1.collision_bits;
        if ((notify1 || notify2) && bounding_spheres_are_in_collision(e1, e2)) {
          check_collisions_list.emplace_back(e1.object, e2.object, notify1,
                                             notify2);
        }
      }
    }
  }

  // called from one thread
  inline auto process_check_collisions_list() -> void {
    for (collision &cc : check_collisions_list) {
      // bounding spheres are in collision
      object *o1 = cc.o1;
      object *o2 = cc.o2;

      bool const o1_is_sphere = o1->is_sphere;
      bool const o2_is_sphere = o2->is_sphere;

      if (o1_is_sphere && o2_is_sphere) {
        cc.is_collision = true;
        continue;
      }

      // check if sphere vs planes

      if (o1_is_sphere) {
        // o2 is not a sphere
        o2->update_planes_world_coordinates();
        if (o2->planes.are_in_collision_with_sphere(o1->position,
                                                    o1->bounding_radius)) {
          cc.is_collision = true;
        }
        continue;
      }

      if (o2_is_sphere) {
        // o1 is not a sphere
        o1->update_planes_world_coordinates();
        if (o1->planes.are_in_collision_with_sphere(o2->position,
                                                    o2->bounding_radius)) {
          cc.is_collision = true;
        }
        continue;
      }

      // both objects are convex volumes bounded by planes

      if (bounding_planes_are_in_collision(o1, o2)) {
        cc.is_collision = true;
      }
    }
  }

  // called from one thread
  inline auto handle_check_collisions_list() const -> void {
    for (collision const &cc : check_collisions_list) {
      if (!cc.is_collision) {
        continue;
      }

      // objects are in collision
      object *o1 = cc.o1;
      object *o2 = cc.o2;

      if (o1->is_sphere && o2->is_sphere) {
        bool const o1_handled_o2 =
            cc.notify1 ? dispatch_collision(o1, o2) : false;
        bool const o2_handled_o1 =
            cc.notify2 ? dispatch_collision(o2, o1) : false;

        // check if collision has already been handled, possibly on a different
        //  thread in a different cell
        if (!o1_handled_o2 && !o2_handled_o1) [[likely]] {
          // collision has not been handled during this frame by any cell
          handle_sphere_collision(o1, o2);
        }
        continue;
      }

      // both objects are not spheres

      if (cc.notify1) {
        dispatch_collision(o1, o2);
      }

      if (cc.notify2) {
        dispatch_collision(o2, o1);
      }
    }
  }

  static inline auto handle_sphere_collision(object *o1, object *o2) -> void {
    // synchronize objects that overlap cells

    bool const o1_overlaps_cells = o1->overlaps_cells;
    bool const o2_overlaps_cells = o2->overlaps_cells;

    if (threaded_grid && o1_overlaps_cells) {
      o1->acquire_lock();
    }
    if (threaded_grid && o2_overlaps_cells) {
      o2->acquire_lock();
    }

    glm::vec3 const collision_normal =
        glm::normalize(o2->position - o1->position);

    float const relative_velocity_along_collision_normal =
        glm::dot(o2->velocity - o1->velocity, collision_normal);

    if (relative_velocity_along_collision_normal >= 0 ||
        std::isnan(relative_velocity_along_collision_normal)) {
      // spheres are not moving towards each other
      if (threaded_grid && o2_overlaps_cells) {
        o2->release_lock();
      }
      if (threaded_grid && o1_overlaps_cells) {
        o1->release_lock();
      }
      return;
    }

    // resolve collision between the spheres

    float constexpr restitution = 1;
    float const impulse = (1.0f + restitution) *
                          relative_velocity_along_collision_normal /
                          (o1->mass + o2->mass);

    o1->velocity += impulse * o2->mass * collision_normal;
    o2->velocity -= impulse * o1->mass * collision_normal;

    if (threaded_grid && o2_overlaps_cells) {
      o2->release_lock();
    }
    if (threaded_grid && o1_overlaps_cells) {
      o1->release_lock();
    }
  }

  // @return true if collision with 'obj' has already been handled by 'receiver'
  static inline auto dispatch_collision(object *receiver, object *obj) -> bool {
    // if object overlaps cells then this code might be called by several
    //  threads at the same time from different cells

    bool const receiver_overlaps_cells = receiver->overlaps_cells;

    bool const synchronize = threaded_grid && receiver_overlaps_cells;

    if (synchronize) {
      receiver->acquire_lock();
    }

    // if both objects overlap cells then the same collision is detected and
    //  dispatched in multiple cells
    if (receiver_overlaps_cells && obj->overlaps_cells &&
        receiver->is_collision_handled_and_if_not_add(obj)) {
      // collision already dispatched for this 'receiver' and 'obj'
      if (synchronize) {
        receiver->release_lock();
      }
      return true;
    }

    // only one thread at a time can be here for 'receiver'

    if (!receiver->is_dead && !receiver->on_collision(obj)) {
      receiver->is_dead = true;
      objects.free(receiver);
    }

    if (synchronize) {
      receiver->release_lock();
    }

    return false;
  }

  static inline auto
  bounding_spheres_are_in_collision(entry const &ce1,
                                    entry const &ce2) -> bool {

    glm::vec3 const v = ce2.position - ce1.position;
    float const d = ce1.radius + ce2.radius;
    float const dsq = d * d;
    float const vsq = glm::dot(v, v); // distance squared
    float const diff = vsq - dsq;
    return diff < 0;
  }

  static inline auto bounding_planes_are_in_collision(object *o1,
                                                      object *o2) -> bool {

    o1->update_planes_world_coordinates();
    o2->update_planes_world_coordinates();

    // planes can be update only once per 'resolve_collisions' pass because
    //  bounding plane objects state do not change because there is no handle
    //   collision implementation such as spheres do

    return planes::are_in_collision(o1->planes, o2->planes);
  }
};
} // namespace glos