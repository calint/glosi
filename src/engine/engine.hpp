#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-01-16
// reviewed: 2024-07-08
// reviewed: 2024-07-10

#include "camera.hpp"
#include "decouple.hpp"
#include "globs.hpp"
#include "grid.hpp"
#include "hud.hpp"
#include "materials.hpp"
#include "metrics.hpp"
#include "net.hpp"
#include "objects.hpp"
#include "sdl.hpp"
#include "shaders.hpp"
#include "textures.hpp"
#include "window.hpp"
#include <GLES3/gl3.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_ttf.h>
#include <arpa/inet.h>
#include <condition_variable>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/string_cast.hpp>
#include <mutex>
#include <netinet/tcp.h>
#include <numbers>

//
// application interface
//

// called at initiation
static auto application_init() -> void;

// called by update thread when an update is done
static auto application_on_update_done() -> void;

// called by render thread when a frame has been rendered
static auto application_on_render_done() -> void;

// called at shutdown
static auto application_free() -> void;

namespace glos {

// color to clear screen with
static glm::vec3 background_color{0, 0, 0};

// ambient light used by shader
static glm::vec3 ambient_light = glm::normalize(glm::vec3{0, 1, 1});

// object the camera should follow
static object* camera_follow_object = nullptr;

// mouse settings
static float constexpr mouse_rad_over_pixels =
    std::numbers::pi_v<float> * 0.02f / 180.0f;
static float mouse_sensitivity = 1.5f;

// a sphere used when debugging object bounding sphere (set at 'init()')
static uint32_t glob_ix_bounding_sphere = 0;

class engine final {
  public:
    inline auto init() -> void {
        // set random number generator seed for deterministic behaviour
        srand(random_seed);

        // initiate subsystems, order matters
        metrics.init();
        net.init();
        sdl.init();
        window.init();
        shaders.init();
        hud.init();
        textures.init();
        materials.init();
        globs.init();
        objects.init();
        grid.init();

        // line rendering shader
        {
            char constexpr const* vtx = R"(
#version 330 core
uniform mat4 umtx_wvp; // world-to-view-to-projection
layout(location = 0) in vec4 apos; // world coordinates
void main() {
  gl_Position = umtx_wvp * apos;
}
      )";

            char constexpr const* frag = R"(
#version 330 core
uniform vec4 ucolor;
out vec4 rgba;
void main() {
  rgba = ucolor;
}
      )";
            shader_program_ix_render_line =
                shaders.load_program_from_source(vtx, frag);
        }

        // points rendering shader
        {
            char constexpr const* vtx = R"(
#version 330 core
uniform mat4 umtx_wvp; // world-to-view-to-projection
layout(location = 0) in vec4 apos; // world coordinates
void main() {
  gl_Position = umtx_wvp * apos;
  gl_PointSize = 5.0;
}
      )";

            char constexpr const* frag = R"(
#version 330 core
uniform vec4 ucolor;
out vec4 rgba;
void main() {
  rgba = ucolor;
}
      )";
            shader_program_ix_render_points =
                shaders.load_program_from_source(vtx, frag);
        }

        // info
        printf("class sizes:\n");
        printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
        printf(": %15s : %-9s :\n", "class", "bytes");
        printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
        printf(": %15s : %-9zu :\n", "object", sizeof(object));
        printf(": %15s : %-9zu :\n", "glob", sizeof(glob));
        printf(": %15s : %-9zu :\n", "planes", sizeof(planes));
        printf(": %15s : %-9zu :\n", "cell", sizeof(cell));
        printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
        puts("");

        if (threaded_grid) {
            printf("threaded grid on %u cores\n\n",
                   std::thread::hardware_concurrency());
        }

        // the bounding sphere used for debugging
        glob_ix_bounding_sphere =
            globs.load("assets/obj/bounding_sphere.obj", nullptr);

        // initiate 'frame_context' with current time from server or local timer
        //  in case 'application_init()' needs current time
        frame_context = {0, net.enabled ? net.ms : SDL_GetTicks64(), 0};

        // set defaults for metrics
        metrics.fps.calculation_interval_ms = 1000;
        metrics.enable_print = metrics_print;

        application_init();

        // add static objects created at 'application_init()' to grid
        objects.apply_allocated_instances([](object* o) {
            if (o->is_static) {
                grid.add_static(o);
            }
        });

        printf("\nglobs: %u   vertex data: %zu B  texture data: %zu B\n\n",
               metrics.allocated_globs, metrics.buffered_vertex_data,
               metrics.buffered_texture_data);
    }

    inline auto free() -> void {
        application_free();
        grid.free();
        objects.free();
        globs.free();
        materials.free();
        textures.free();
        hud.free();
        shaders.free();
        window.free();
        sdl.free();
        net.free();
        metrics.free();
    }

    inline auto run() -> void {
        if (net.enabled) {
            // initiate networking by sending initial signals
            net.begin();
        }

        if (threaded_update) {
            // update runs as separate thread
            start_update_thread();
        }

        // start metrics
        metrics.print_headers(stderr);
        metrics.begin();

        SDL_SetRelativeMouseMode(is_mouse_mode ? SDL_TRUE : SDL_FALSE);

        // enter game loop
        while (true) {
            metrics.at_frame_begin();

            // read keyboard, mouse and handle window events
            handle_events();

            // check if quit was received
            if (!is_running) {
                break;
            }

            if (threaded_update) {
                // update runs in separate thread
                render_thread_loop_body();
            } else {
                // update runs on main thread
                render();

                metrics.update_begin();
                update_pass_1();
                update_pass_2();
                metrics.update_end();

                // swap buffers after update to allow debugging rendering
                window.swap_buffers();
            }

            metrics.allocated_objects = uint32_t(objects.allocated_list_len());
            metrics.at_frame_end(stderr);
        }

        // exit
        if (threaded_update) {
            // trigger update thread incase it is waiting
            std::unique_lock<std::mutex> lock{is_rendering_mutex};
            is_rendering = false;
            lock.unlock();
            is_rendering_cv.notify_one();
            update_thread.join();
        }
    }

  private:
    friend auto debug_render_wcs_line(glm::vec3 const& from_wcs,
                                      glm::vec3 const& to_wcs,
                                      glm::vec4 const& color,
                                      bool const depth_test) -> void;

    friend auto debug_render_wcs_points(std::vector<glm::vec3> const& points,
                                        glm::vec4 const& color) -> void;

    uint64_t frame_num = 0;
    bool is_running = true;
    bool is_render = true;
    bool is_render_hud = hud_enabled;
    bool is_resolve_collisions = true;
    bool is_render_grid = false;
    bool is_print_grid = false;
    bool is_mouse_mode = false;

    // index of shader that renders world coordinate system line
    uint32_t shader_program_ix_render_line = 0;
    // index of shader that renders world coordinate system points
    uint32_t shader_program_ix_render_points = 0;
    // index of current shader
    uint32_t shader_program_ix = 0;
    // index of previous shader
    uint32_t shader_program_ix_prv = shader_program_ix;

    // synchronization of update and render thread
    std::thread update_thread{};
    bool is_rendering = true;
    std::mutex is_rendering_mutex{};
    std::condition_variable is_rendering_cv{};

    inline auto render() -> void {
        metrics.render_begin();
        if (!is_render) {
            metrics.render_end();
            return;
        }

        // check if shader program has changed
        if (shader_program_ix_prv != shader_program_ix) {
            printf(" * switching to program at index %u\n", shader_program_ix);
            shaders.use_program(shader_program_ix);
            shader_program_ix_prv = shader_program_ix;
        }

        if (camera_follow_object) {
            camera.look_at = camera_follow_object->position;
        }

        camera.update_matrix_wvp();

        glUniformMatrix4fv(shaders.umtx_wvp, 1, GL_FALSE,
                           glm::value_ptr(camera.Mwvp));

        glUniform3fv(shaders.ulht, 1, glm::value_ptr(ambient_light));

        glClearColor(background_color.r, background_color.g, background_color.b,
                     1.0);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (is_render_hud) {
            shaders.use_program(hud.program_ix);
            hud.render();
            shaders.use_program(shader_program_ix);
        }

        grid.render();

        if (is_render_grid) {
            grid.debug_render_grid();
        }

        application_on_render_done();

        metrics.render_end();
    }

    // in 'threaded_update' runs before render and update is done parallel in
    //  'update_pass_2()'
    inline auto update_pass_1() -> void {

        grid.clear_non_static_entries();

        // add all allocated non static objects to the grid
        objects.for_each([](object* o) {
            if (!o->is_static) {
                grid.add(o);
            }
        });

        // update frame context used throughout the frame
        //  in multiplayer mode use 'dt' and 'ms' from server
        //   in single player mode use 'dt' from previous frame and current 'ms'
        ++frame_num;
        if (net.enabled) {
            frame_context = {frame_num, net.ms, net.dt};
        } else {
            frame_context = {frame_num, SDL_GetTicks64(), metrics.dt};
        }
    }

    // in 'threaded_update' runs in parallel with rendering
    inline auto update_pass_2() const -> void {
        if (is_print_grid) {
            grid.print();
        }

        // note: data racing between render and update thread on objects
        //       position, angle, scale glob index is ok (?)

        grid.update();

        if (is_resolve_collisions) {
            grid.resolve_collisions();
        }

        // remove freed static objects from grid
        // note: at 'update()' and 'resolve_collisions()' objects might be freed
        // and created
        objects.apply_freed_instances([](object* o) {
            if (o->is_static) {
                grid.remove_static(o);
            }
        });

        // add newly allocated static objects to grid
        objects.apply_allocated_instances([](object* o) {
            if (o->is_static) {
                grid.add_static(o);
            }
        });

        // callback application
        application_on_update_done();

        // apply changes done by application

        // removed freed static objects from grid
        objects.apply_freed_instances([](object* o) {
            if (o->is_static) {
                grid.remove_static(o);
            }
        });

        // add newly allocated static objects to grid
        objects.apply_allocated_instances([](object* o) {
            if (o->is_static) {
                grid.add_static(o);
            }
        });

        // update signals from network or local
        if (net.enabled) {
            // receive signals from previous frame and send signals of current
            // frame
            net.receive_and_send();
        } else {
            // copy signals to active player
            net.states[net.player_ix] = net.next_state;
        }
    }

    inline auto start_update_thread() -> void {
        update_thread = std::thread([this]() {
            while (true) {
                {
                    // wait until render thread is done before removing and
                    // adding objects
                    //  to grid
                    std::unique_lock<std::mutex> lock{is_rendering_mutex};
                    is_rendering_cv.wait(lock,
                                         [this] { return !is_rendering; });
                    // note: wait until 'is_rendering' is false

                    if (!is_running) {
                        return;
                    }

                    metrics.update_begin();

                    update_pass_1();

                    // notify render thread to start rendering
                    is_rendering = true;
                    lock.unlock();
                    is_rendering_cv.notify_one();
                }

                // running in parallel with render thread
                update_pass_2();

                metrics.update_end();
            }
        });
    }

    inline auto render_thread_loop_body() -> void {
        // wait for update thread to remove and add objects to grid
        std::unique_lock<std::mutex> lock{is_rendering_mutex};
        is_rendering_cv.wait(lock, [this] { return is_rendering; });
        // note: wait until 'is_rendering' is true

        // note: render and update have acceptable (?) data races on objects
        //       position, angle, scale, glob index etc

        render();

        window.swap_buffers();

        // notify update thread that rendering is done
        is_rendering = false;
        lock.unlock();
        is_rendering_cv.notify_one();
    }

    inline auto handle_events() -> void {
        // poll events
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            default: // unknown
                break;
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                case SDL_WINDOWEVENT_SIZE_CHANGED: {
                    auto [w, h] = window.get_width_and_height();
                    camera.width = float(w);
                    camera.height = float(h);
                    glViewport(0, 0, w, h);
                    printf(" * window resize to  %d x %d\n", w, h);
                    break;
                }
                default: // ignored
                    break;
                }
                break;
            }

            case SDL_QUIT:
                printf(" * exit\n");
                is_running = false;
                break;

            case SDL_MOUSEMOTION: {
                if (event.motion.xrel != 0) {
                    net.next_state.look_angle_y += (float)event.motion.xrel *
                                                   mouse_rad_over_pixels *
                                                   mouse_sensitivity;
                }
                if (event.motion.yrel != 0) {
                    net.next_state.look_angle_x += (float)event.motion.yrel *
                                                   mouse_rad_over_pixels *
                                                   mouse_sensitivity;
                }
                break;
            }

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                case SDLK_w:
                    net.next_state.keys |= key_w;
                    break;
                case SDLK_a:
                    net.next_state.keys |= key_a;
                    break;
                case SDLK_s:
                    net.next_state.keys |= key_s;
                    break;
                case SDLK_d:
                    net.next_state.keys |= key_d;
                    break;
                case SDLK_q:
                    net.next_state.keys |= key_q;
                    break;
                case SDLK_e:
                    net.next_state.keys |= key_e;
                    break;
                case SDLK_i:
                    net.next_state.keys |= key_i;
                    break;
                case SDLK_j:
                    net.next_state.keys |= key_j;
                    break;
                case SDLK_k:
                    net.next_state.keys |= key_k;
                    break;
                case SDLK_l:
                    net.next_state.keys |= key_l;
                    break;
                case SDLK_u:
                    net.next_state.keys |= key_u;
                    break;
                case SDLK_o:
                    net.next_state.keys |= key_o;
                    break;
                case SDLK_SPACE:
                    net.next_state.keys |= key_space;
                    break;
                default: // ignored
                    break;
                }
                break;

            case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                case SDLK_w:
                    net.next_state.keys &= ~key_w;
                    break;
                case SDLK_a:
                    net.next_state.keys &= ~key_a;
                    break;
                case SDLK_s:
                    net.next_state.keys &= ~key_s;
                    break;
                case SDLK_d:
                    net.next_state.keys &= ~key_d;
                    break;
                case SDLK_q:
                    net.next_state.keys &= ~key_q;
                    break;
                case SDLK_e:
                    net.next_state.keys &= ~key_e;
                    break;
                case SDLK_i:
                    net.next_state.keys &= ~key_i;
                    break;
                case SDLK_j:
                    net.next_state.keys &= ~key_j;
                    break;
                case SDLK_k:
                    net.next_state.keys &= ~key_k;
                    break;
                case SDLK_l:
                    net.next_state.keys &= ~key_l;
                    break;
                case SDLK_u:
                    net.next_state.keys &= ~key_u;
                    break;
                case SDLK_o:
                    net.next_state.keys &= ~key_o;
                    break;
                case SDLK_SPACE:
                    net.next_state.keys &= ~key_space;
                    break;
                case SDLK_ESCAPE:
                    is_mouse_mode = is_mouse_mode ? SDL_FALSE : SDL_TRUE;
                    SDL_SetRelativeMouseMode(is_mouse_mode ? SDL_TRUE
                                                           : SDL_FALSE);
                    break;
                case SDLK_F1:
                    is_print_grid = !is_print_grid;
                    break;
                case SDLK_F2:
                    is_resolve_collisions = !is_resolve_collisions;
                    break;
                case SDLK_F3:
                    is_render = !is_render;
                    break;
                case SDLK_F4:
                    ++shader_program_ix;
                    if (shader_program_ix >= shaders.programs_count()) {
                        shader_program_ix = 0;
                    }
                    break;
                case SDLK_F5:
                    is_debug_object_planes_normals =
                        !is_debug_object_planes_normals;
                    break;
                case SDLK_F6:
                    is_debug_object_bounding_sphere =
                        !is_debug_object_bounding_sphere;
                    break;
                case SDLK_F7:
                    is_render_hud = !is_render_hud;
                    break;
                case SDLK_F8:
                    is_render_grid = !is_render_grid;
                    break;
                case SDLK_F9:
                    shaders.print_current_shader_info();
                    break;
                default: // ignored
                    break;
                }
                break;
            }
        }
    }
};

static engine engine{};

// debugging (highly inefficient) function for rendering world coordinate system
//  lines
static inline auto debug_render_wcs_line(glm::vec3 const& from_wcs,
                                         glm::vec3 const& to_wcs,
                                         glm::vec4 const& color,
                                         bool const depth_test) -> void {
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glm::vec3 const line_vertices[]{from_wcs, to_wcs};

    glBufferData(GL_ARRAY_BUFFER, sizeof(line_vertices), line_vertices,
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    camera.update_matrix_wvp();

    shaders.use_program(engine.shader_program_ix_render_line);

    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera.Mwvp));
    glUniform4fv(1, 1, glm::value_ptr(color));

    if (!depth_test) {
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_LINES, 0, 2);
    glDisable(GL_BLEND);
    if (!depth_test) {
        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    shaders.use_program(engine.shader_program_ix);
}

static inline auto debug_render_bounding_sphere(glm::mat4 const& Mmw) -> void {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    globs.at(glob_ix_bounding_sphere).render(Mmw);
    glDisable(GL_BLEND);
}

// debugging (highly inefficient) function for rendering world coordinate system
//  points
static inline auto debug_render_wcs_points(std::vector<glm::vec3> const& points,
                                           glm::vec4 const& color) -> void {
    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(points.size() * sizeof(glm::vec3)),
                 points.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    camera.update_matrix_wvp();

    shaders.use_program(engine.shader_program_ix_render_points);

    glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(camera.Mwvp));
    glUniform4fv(1, 1, glm::value_ptr(color));

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_POINTS, 0, GLsizei(points.size()));
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);

    shaders.use_program(engine.shader_program_ix);
}

} // namespace glos
