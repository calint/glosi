#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

// make common used namespace available in game code
using namespace glos;
using namespace glm;

// forward declarations
static auto application_init_shaders() -> void;
static auto create_asteroids(uint32_t num) -> void;
static auto create_ufo() -> void;
static auto create_cubes(uint32_t num) -> void;
static auto create_spheres(uint32_t num) -> void;

// game state
enum class state { init, asteroids, ufo };

static state state = state::init;
static uint32_t level = 1;
static std::atomic_int32_t score{0};
static int32_t score_prv = 0;
static std::atomic_uint32_t asteroids_alive{0};
static std::atomic_uint32_t ufos_alive{0};
static std::atomic_uint32_t object_id{0};
// note: used when 'debug_multiplayer' is true to give objects unique numbers

static object *hero = nullptr;

// glob indexes
// note: set by 'application_init()' when loading models and used by objects
static uint32_t glob_ix_ship = 0;
static uint32_t glob_ix_ship_engine_on = 0;
static uint32_t glob_ix_bullet = 0;
static uint32_t glob_ix_asteroid_large = 0;
static uint32_t glob_ix_asteroid_medium = 0;
static uint32_t glob_ix_asteroid_small = 0;
static uint32_t glob_ix_fragment = 0;
static uint32_t glob_ix_power_up = 0;
static uint32_t glob_ix_cube = 0;
static uint32_t glob_ix_tetra = 0;
static uint32_t glob_ix_sphere = 0;
static uint32_t glob_ix_skydome = 0;
static uint32_t glob_ix_ufo = 0;
static uint32_t glob_ix_ufo_bullet = 0;

// objects
#include "objects/asteroid_large.hpp"
#include "objects/cube.hpp"
#include "objects/fragment.hpp"
#include "objects/ship.hpp"
#include "objects/sphere.hpp"
#include "objects/tetra.hpp"
#include "objects/ufo.hpp"

// engine interface
static inline auto application_init() -> void {
  application_init_shaders();

  printf("\ntime is %lu ms\n\n", frame_context.ms);

  printf("class sizes:\n");
  printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
  printf(": %15s : %-9s :\n", "class", "bytes");
  printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
  printf(": %15s : %-9zu :\n", "asteroid_large", sizeof(asteroid_large));
  printf(": %15s : %-9zu :\n", "asteroid_medium", sizeof(asteroid_medium));
  printf(": %15s : %-9zu :\n", "asteroid_small", sizeof(asteroid_small));
  printf(": %15s : %-9zu :\n", "ship_bullet", sizeof(ship_bullet));
  printf(": %15s : %-9zu :\n", "fragment", sizeof(fragment));
  printf(": %15s : %-9zu :\n", "power_up", sizeof(power_up));
  printf(": %15s : %-9zu :\n", "ship", sizeof(ship));
  printf(": %15s : %-9zu :\n", "cube", sizeof(cube));
  printf(": %15s : %-9zu :\n", "sphere", sizeof(sphere));
  printf(": %15s : %-9zu :\n", "tetra", sizeof(tetra));
  printf(": %15s : %-9zu :\n", "ufo", sizeof(ufo));
  printf(": %15s : %-9zu :\n", "ufo_bullet", sizeof(ufo_bullet));
  printf(":-%15s-:-%-9s-:\n", "---------------", "---------");
  puts("");

  // assert that classes used fit in objects store slot
  static_assert(sizeof(asteroid_large) <= objects_instance_size_B);
  static_assert(sizeof(asteroid_medium) <= objects_instance_size_B);
  static_assert(sizeof(asteroid_small) <= objects_instance_size_B);
  static_assert(sizeof(ship_bullet) <= objects_instance_size_B);
  static_assert(sizeof(fragment) <= objects_instance_size_B);
  static_assert(sizeof(power_up) <= objects_instance_size_B);
  static_assert(sizeof(ship) <= objects_instance_size_B);
  static_assert(sizeof(cube) <= objects_instance_size_B);
  static_assert(sizeof(sphere) <= objects_instance_size_B);
  static_assert(sizeof(tetra) <= objects_instance_size_B);
  static_assert(sizeof(ufo) <= objects_instance_size_B);
  static_assert(sizeof(ufo_bullet) <= objects_instance_size_B);

  // load the objects and assign the glob indexes

  // stock objects
  glob_ix_cube = globs.load("assets/obj/cube.obj", "assets/obj/cube_bp.obj");
  glob_ix_tetra = globs.load("assets/obj/tetra.obj", "assets/obj/tetra.obj");
  glob_ix_sphere = globs.load("assets/obj/sphere.obj", nullptr);

  // game objects
  glob_ix_ship = globs.load("assets/obj/asteroids/ship.obj",
                            "assets/obj/asteroids/ship.obj");
  glob_ix_ship_engine_on =
      globs.load("assets/obj/asteroids/ship_engine_on.obj",
                 "assets/obj/asteroids/ship_engine_on.obj");
  glob_ix_bullet = globs.load("assets/obj/asteroids/ship_bullet.obj",
                              "assets/obj/asteroids/ship_bullet_bp.obj");
  glob_ix_asteroid_large =
      globs.load("assets/obj/asteroids/asteroid_large.obj",
                 "assets/obj/asteroids/asteroid_large_bp.obj");
  glob_ix_asteroid_medium =
      globs.load("assets/obj/asteroids/asteroid_medium.obj",
                 "assets/obj/asteroids/asteroid_medium.obj");
  glob_ix_asteroid_small =
      globs.load("assets/obj/asteroids/asteroid_small.obj",
                 "assets/obj/asteroids/asteroid_small.obj");
  glob_ix_fragment = globs.load("assets/obj/asteroids/fragment.obj", nullptr);
  glob_ix_power_up = globs.load("assets/obj/asteroids/power_up.obj", nullptr);
  glob_ix_ufo = globs.load("assets/obj/asteroids/ufo.obj", nullptr);
  glob_ix_ufo_bullet =
      globs.load("assets/obj/asteroids/ufo_bullet.obj", nullptr);

  glob_ix_skydome = globs.load("assets/obj/skydome.obj", nullptr);

  if (!is_performance_test) {
    // setup scene

    // the dome
    object *skydome = new (objects.alloc()) object{};
    skydome->glob_ix(glob_ix_skydome);
    float const skydome_scale = length(vec2{grid_size / 2, grid_size / 2});
    skydome->bounding_radius = skydome_scale;
    skydome->scale = {skydome_scale, skydome_scale, skydome_scale};

    if (net.enabled) {
      // multiplayer mode
      ship *p1 = new (objects.alloc()) ship{};
      p1->position.x = -5;
      p1->net_state = &net.states[1];
      hero = p1;

      ship *p2 = new (objects.alloc()) ship{};
      p2->position.x = 5;
      p2->net_state = &net.states[2];
    } else {
      // single player mode
      ship *p = new (objects.alloc()) ship{};
      p->net_state = &net.states[1];
      hero = p;
      // create_ufo();
    }
  } else {
    switch (performance_test_type) {
    case 1:
      create_cubes(objects_count);
      break;
    case 2:
      create_spheres(objects_count);
      break;
    default:
      throw exception{std::format("unknown 'performance_test_type' '{}'",
                                  performance_test_type)};
    }
  }

  background_color = {0, 0, 0};

  // setup light and camera
  ambient_light = normalize(vec3{1, 1, 1});

  camera.type = camera::type::ORTHOGONAL;
  camera.position = {0, 50, 0};
  camera.look_at = {0, 0, -.000001f};
  // note: -.000001f because of the math of 'look at'
  camera.ortho_min_x = -game_area_half_x;
  camera.ortho_min_y = -game_area_half_z;
  camera.ortho_max_x = game_area_half_x;
  camera.ortho_max_y = game_area_half_z;

  hud.load_font("assets/fonts/digital-7 (mono).ttf", 20);
}

// engine interface
static inline auto application_on_update_done() -> void {
  if (is_performance_test) {
    return;
  }

  switch (state) {
  case state::init:
    create_asteroids(level * asteroids_per_level);
    state = state::asteroids;
    break;
  case state::asteroids:
    if (asteroids_alive == 0) {
      create_ufo();
      state = state::ufo;
    }
    break;
  case state::ufo:
    if (ufos_alive == 0) {
      ++level;
      create_asteroids(level * asteroids_per_level);
      state = state::asteroids;
    }
    break;
  }
}

// engine interface
static inline auto application_on_render_done() -> void {
  if (score != score_prv) {
    std::array<char, 32> buf;
    score_prv = score;
    sprintf(buf.data(), "score: %06d", score_prv);
    hud.print(buf.data(), SDL_Color{255, 0, 0, 255}, 60, 10);
  }
}

// engine interface
static inline auto application_free() -> void {}

static inline auto create_asteroids(uint32_t const num) -> void {
  float constexpr v = asteroid_large_speed;
  float constexpr d = game_area_max_x - game_area_min_x;
  for (uint32_t i = 0; i < num; ++i) {
    asteroid_large *o = new (objects.alloc()) asteroid_large{};
    o->position = {rnd1(d), 0, rnd1(d)};
    o->velocity = {rnd1(v), 0, rnd1(v)};
  }
}

static inline auto create_cubes(uint32_t const num) -> void {
  float constexpr v = cube_speed;
  float constexpr d = game_area_max_x - game_area_min_x;
  for (uint32_t i = 0; i < num; ++i) {
    cube *o = new (objects.alloc()) cube{};
    o->position = {rnd1(d), 0, rnd1(d)};
    o->velocity = {rnd1(v), 0, rnd1(v)};
    o->angular_velocity.y = radians(20.0f);
  }
}

static inline auto create_spheres(uint32_t const num) -> void {
  float constexpr v = sphere_speed;
  float constexpr d = game_area_max_x - game_area_min_x;
  for (uint32_t i = 0; i < num; ++i) {
    sphere *o = new (objects.alloc()) sphere{};
    o->position = {rnd1(d), 0, rnd1(d)};
    o->velocity = {rnd1(v), 0, rnd1(v)};
  }
}

static inline auto create_ufo() -> void {
  ufo *u = new (objects.alloc()) ufo{};
  u->position = {-grid_size / 2, 0, -grid_size / 2};
  u->angle = {radians(-65.0f), 0, 0};
  u->angular_velocity = {0, radians(90.0f), 0};
  u->velocity = {rnd1(ufo_velocity), 0, rnd1(ufo_velocity)};
}

// some additional shaders
static inline auto application_init_shaders() -> void {
  {
    char constexpr const *vtx = R"(
#version 330 core
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
    char constexpr const *frag = R"(
#version 330 core
out vec4 rgba;
void main() {
  rgba = vec4(vec3(gl_FragCoord.z), 1.0);
}
    )";
    glos::shaders.load_program_from_source(vtx, frag);
  }
  {
    char constexpr const *vtx = R"(
#version 330 core
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
    char constexpr const *frag = R"(
#version 330 core
in vec4 vrgba;
out vec4 rgba;
void main(){
  rgba = vrgba;
}
    )";
    glos::shaders.load_program_from_source(vtx, frag);
  }
}