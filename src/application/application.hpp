#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../engine/engine.hpp"
#include "objects/cube.hpp"
#include "objects/ship.hpp"
#include "objects/sphere.hpp"
#include "objects/static_object.hpp"
#include "objects/tetra.hpp"
#include <glm/gtx/quaternion.hpp>

// forward declarations
static auto application_init_shaders() -> void;
static auto setup1() -> void;
static auto setup2() -> void;
static auto setup3() -> void;

// engine interface
static auto application_print_hello() -> void { printf("\nprogram glosi\n\n"); }

// engine interface
static auto application_init() -> void {
    application_init_shaders();

    printf("\ntime is %lu ms\n\n", glos::frame_context.ms);

    printf("class sizes:\n");
    printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
    printf(": %15s : %-9s :\n", "class", "bytes");
    printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
    printf(": %15s : %-9zu :\n", "cube", sizeof(cube));
    printf(": %15s : %-9zu :\n", "ship", sizeof(ship));
    printf(": %15s : %-9zu :\n", "sphere", sizeof(sphere));
    printf(": %15s : %-9zu :\n", "static_object", sizeof(static_object));
    printf(": %15s : %-9zu :\n", "tetra", sizeof(tetra));
    printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
    puts("");

    // assert that classes used fit in objects store slot
    static_assert(sizeof(cube) <= objects_instance_size_B);
    static_assert(sizeof(ship) <= objects_instance_size_B);
    static_assert(sizeof(sphere) <= objects_instance_size_B);
    static_assert(sizeof(static_object) <= objects_instance_size_B);
    static_assert(sizeof(tetra) <= objects_instance_size_B);

    // load the objects and assign the glob indexes

    // stock objects
    glob_ix_cube =
        glos::globs.load("assets/obj/cube.obj", "assets/obj/cube_bp.obj");
    glob_ix_sphere = glos::globs.load("assets/obj/sphere.obj", nullptr);
    glob_ix_tetra =
        glos::globs.load("assets/obj/tetra.obj", "assets/obj/tetra.obj");

    // game objects

    // the dome
    glob_ix_skydome = glos::globs.load("assets/obj/skydome.obj", nullptr);
    glos::object* skydome = new (glos::objects.alloc()) glos::object{};
    skydome->is_static = true;
    skydome->glob_ix(glob_ix_skydome);
    float const skydome_scale = length(glm::vec2{grid_size / 2, grid_size / 2});
    skydome->bounding_radius = skydome_scale;
    skydome->scale = {skydome_scale, skydome_scale, skydome_scale};

    glos::background_color = {0, 0, 0};

    // setup light and camera

    glos::ambient_light = normalize(glm::vec3{1, 1, 1});

    glos::camera.type = glos::camera::type::ORTHOGONAL;
    glos::camera.position = {0, 50, 0};
    glos::camera.look_at = {0, 0, -0.000001f};
    // note: -0.000001f because of the math of 'look at' does not handle x and z
    //       being equal camera_follow_object = hero;
    glos::camera.ortho_min_x = -game_area_half_x;
    glos::camera.ortho_min_y = -game_area_half_z;
    glos::camera.ortho_max_x = game_area_half_x;
    glos::camera.ortho_max_y = game_area_half_z;

    glos::hud.load_font("assets/fonts/digital-7 (mono).ttf", 20);

    if (glos::net.enabled) {
        // multiplayer mode
    } else {
        // single player mode
        setup3();
    }
}

static auto setup1() -> void {
    // single player mode
    auto* o0 = new (glos::objects.alloc()) ship{};
    o0->position.z = 4;
    o0->net_state = &glos::net.states[1];

    auto* o1 = new (glos::objects.alloc()) cube{};
    o1->position.x = 5;
    o1->linear_velocity.x = -1;

    auto* o2 = new (glos::objects.alloc()) cube{};
    o2->position.x = -5;
    o2->position.y = 0.1f;
    o2->linear_velocity.x = 1;

    // the vector from the center of the cube to its corner
    glm::vec3 const corner_dir = glm::normalize(glm::vec3{1.0f, 1.0f, 1.0f});

    // the world x-axis
    glm::vec3 const x_axis = glm::vec3{1.0f, 0.0f, 0.0f};

    // calculate the rotation that aligns the corner to the x-axis
    o2->orientation = glm::rotation(corner_dir, x_axis);
}

static auto setup2() -> void {
    auto* o1 = new (glos::objects.alloc()) cube{};

    auto* o2 = new (glos::objects.alloc()) cube{};
    o2->position.z = 3.0f;
    o2->linear_acceleration.z = -1;

    auto* o3 = new (glos::objects.alloc()) cube{};
    o3->position.z = 6.0f;
    o3->linear_acceleration.z = -1;
}

static auto setup3() -> void {
    // single player mode
    auto* o0 = new (glos::objects.alloc()) ship{};
    o0->position.z = 4;
    o0->net_state = &glos::net.states[1];

    auto* o1 = new (glos::objects.alloc()) cube{};
    o1->position.x = 5;
    o1->linear_velocity.x = -1;

    auto* o2 = new (glos::objects.alloc()) cube{};
    o2->position.x = -5;
    o2->linear_velocity.x = 1;
}

// engine interface
static auto application_on_update_done() -> void {}

// engine interface
static auto application_on_render_done() -> void {}

// engine interface
static auto application_free() -> void {}

// some additional shaders
static auto application_init_shaders() -> void {
    {
        char constexpr const* vtx = R"(
#version 320 es
precision highp float;

uniform mat4 umtx_mw; // model-to-world-matrix
uniform mat4 umtx_wvp;// world-to-view-to-projection
layout(location = 0) in vec4 apos;
layout(location = 1) in vec4 argba;
layout(location = 2) in vec3 anorm;
layout(location = 3) in vec2 atex;

void main() {
  gl_Position = umtx_wvp * umtx_mw * apos;
}
    )";
        char constexpr const* frag = R"(
#version 320 es
precision highp float;

out vec4 rgba;
void main() {
  rgba = vec4(vec3(gl_FragCoord.z), 1.0);
}
    )";
        glos::shaders.load_program_from_source(vtx, frag);
    }
    {
        char constexpr const* vtx = R"(
#version 320 es
precision highp float;

uniform mat4 umtx_mw; // model-to-world-matrix
uniform mat4 umtx_wvp;// world-to-view-to-projection
in vec4 apos;
in vec4 argba;
out vec4 vrgba;
void main() {
  gl_Position = umtx_wvp * umtx_mw * apos;
  vrgba = argba;
}
    )";
        char constexpr const* frag = R"(
#version 320 es
precision highp float;

in vec4 vrgba;
out vec4 rgba;
void main(){
  rgba = vrgba;
}
    )";
        glos::shaders.load_program_from_source(vtx, frag);
    }
}
