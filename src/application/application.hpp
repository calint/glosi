#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../engine/engine.hpp"
#include "../engine/exception.hpp"
#include "objects/asteroid_large.hpp"
#include "objects/cube.hpp"
#include "objects/fragment.hpp"
#include "objects/ship.hpp"
#include "objects/sphere.hpp"
#include "objects/static_object.hpp"
#include "objects/tetra.hpp"
#include "objects/ufo.hpp"

// forward declarations
static auto application_init_shaders() -> void;
static auto create_asteroids(uint32_t num) -> void;
static auto create_ufo() -> void;
static auto create_cubes(uint32_t const num) -> void;
static auto load_map(std::filesystem::path path) -> void;

// hello
static auto application_print_hello() { printf("\nprogram glosi\n\n"); }

// engine interface
static inline auto application_init() -> void {
  application_init_shaders();

  printf("\ntime is %lu ms\n\n", glos::frame_context.ms);

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
  glob_ix_cube =
      glos::globs.load("assets/obj/cube.obj", "assets/obj/cube_bp.obj");
  glob_ix_tetra =
      glos::globs.load("assets/obj/tetra.obj", "assets/obj/tetra.obj");
  glob_ix_sphere = glos::globs.load("assets/obj/sphere.obj", nullptr);

  // game objects
  glob_ix_ship = glos::globs.load("assets/obj/asteroids/ship.obj",
                                  "assets/obj/asteroids/ship.obj");
  glob_ix_ship_engine_on =
      glos::globs.load("assets/obj/asteroids/ship_engine_on.obj",
                       "assets/obj/asteroids/ship_engine_on.obj");
  glob_ix_bullet = glos::globs.load("assets/obj/asteroids/ship_bullet.obj",
                                    "assets/obj/asteroids/ship_bullet_bp.obj");
  glob_ix_asteroid_large =
      glos::globs.load("assets/obj/asteroids/asteroid_large.obj",
                       "assets/obj/asteroids/asteroid_large_bp.obj");
  glob_ix_asteroid_medium =
      glos::globs.load("assets/obj/asteroids/asteroid_medium.obj",
                       "assets/obj/asteroids/asteroid_medium.obj");
  glob_ix_asteroid_small =
      glos::globs.load("assets/obj/asteroids/asteroid_small.obj",
                       "assets/obj/asteroids/asteroid_small.obj");
  glob_ix_fragment =
      glos::globs.load("assets/obj/asteroids/fragment.obj", nullptr);
  glob_ix_power_up =
      glos::globs.load("assets/obj/asteroids/power_up.obj", nullptr);
  glob_ix_ufo = glos::globs.load("assets/obj/asteroids/ufo.obj", nullptr);
  glob_ix_ufo_bullet =
      glos::globs.load("assets/obj/asteroids/ufo_bullet.obj", nullptr);
  glob_ix_skydome = glos::globs.load("assets/obj/skydome.obj", nullptr);
  glob_ix_landing_pad = glos::globs.load("assets/obj/landing_pad.obj",
                                         "assets/obj/landing_pad_bp.obj");

  // the dome
  // object *skydome = new (objects.alloc()) object{};
  // skydome->glob_ix(glob_ix_skydome);
  // float const skydome_scale = length(vec2{grid_size / 2, grid_size / 2});
  // skydome->bounding_radius = skydome_scale;
  // skydome->scale = {skydome_scale, skydome_scale, skydome_scale};

  glos::background_color = {0, 0, 0};

  // setup light and camera
  glos::ambient_light = normalize(glm::vec3{2, 10, 1});

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

  if (performance_test_type == 1) {
    create_cubes(objects_count);
    return;
  }

  load_map("assets/maps/level_1.map");

  if (glos::net.enabled) {
    // multiplayer mode
    ship *p1 = new (glos::objects.alloc()) ship{};
    p1->position.x = -5;
    p1->position.z = 0;
    p1->net_state = &glos::net.states[1];
    if (glos::net.player_ix == 1) {
      hero = p1;
    }

    ship *p2 = new (glos::objects.alloc()) ship{};
    p2->position.x = 5;
    p2->position.z = 0;
    p2->net_state = &glos::net.states[2];
    if (glos::net.player_ix == 2) {
      hero = p2;
    }
  } else {
    // single player mode
    ship *p = new (glos::objects.alloc()) ship{};
    p->position.z = 0;
    p->net_state = &glos::net.states[1];
    hero = p;

    glos::object *o = new (glos::objects.alloc()) cube{};
    o->position = {0.0f, 0.0f, -5.0f};
    o->scale = {2.0f, 1.0f, 0.5f};
    o->angular_velocity = {0, glm::radians(5.0f), 0};
    o->bounding_radius = o->glob().bounding_radius * o->scale.x;

    o = new (glos::objects.alloc()) cube{};
    o->position = {0.0f, 0.0f, -10.0f};
    o->scale = {2.0f, 2.0f, 2.0f};
    o->angular_velocity = {0, glm::radians(5.0f), 0};
    o->bounding_radius = o->glob().bounding_radius * o->scale.x;
  }
}

// engine interface
static inline auto application_on_update_done() -> void {
  if (is_performance_test) {
    return;
  }

  // camera.position = {hero->position.x, 50, hero->position.z};
  // camera.look_at = {hero->position.x, 0, hero->position.z - 0.00001f};
  // note: -0.00001f because of look_at does not work properly when x and z are
  //       equal

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
  std::array<char, 128> buf;
  int const s = score;
  if (performance_test_type == 0) {
    sprintf(buf.data(), "%03d %02u %06d", int32_t(hero->fuel),
            uint32_t(length(hero->linear_velocity)), s);
  } else {
    sprintf(buf.data(), "%06d", s);
  }
  glos::hud.print(buf.data(), SDL_Color{255, 0, 0, 255}, 60, 10);
}

// engine interface
static inline auto application_free() -> void {}

static inline auto create_asteroids(uint32_t const num) -> void {
  float constexpr v = asteroid_large_speed;
  float constexpr d = game_area_max_x - game_area_min_x;
  for (uint32_t i = 0; i < num; ++i) {
    asteroid_large *o = new (glos::objects.alloc()) asteroid_large{};
    o->position = {rnd1(d), 0, rnd1(d)};
    o->linear_velocity = {rnd1(v), 0, rnd1(v)};
  }
}

static inline auto create_ufo() -> void {
  ufo *u = new (glos::objects.alloc()) ufo{};
  u->position = {-grid_size / 2, 0, -grid_size / 2};
  u->angle = {glm::radians(-65.0f), 0, 0};
  u->angular_velocity = {0, glm::radians(90.0f), 0};
  u->linear_velocity = {rnd1(ufo_velocity), 0, rnd1(ufo_velocity)};
}

static inline auto create_cubes(uint32_t const num) -> void {
  float constexpr a = cube_angular_velocity;
  float constexpr v = cube_speed;
  float constexpr gx = game_area_max_x - game_area_min_x;
  float constexpr gz = game_area_max_z - game_area_min_z;
  for (uint32_t i = 0; i < num; ++i) {
    cube *o = new (glos::objects.alloc()) cube{};
    o->position = {rnd1(gx), 0, rnd1(gz)};
    o->linear_velocity = {rnd1(v), 0, rnd1(v)};
    o->angular_velocity = {rnd1(a), rnd1(a), rnd1(a)};
  }
}

static inline auto load_static_object_list(std::filesystem::path path) -> void {
  printf(" * loading map from '%s'\n", path.string().c_str());

  std::ifstream file{path};
  if (!file) {
    throw glos::exception{
        std::format("cannot open file '{}'", path.string().c_str())};
  }
  uint32_t const glob_ix[] = {glob_ix_asteroid_large, glob_ix_asteroid_medium,
                              glob_ix_asteroid_small, glob_ix_landing_pad};
  std::string line{};
  while (std::getline(file, line)) {
    std::istringstream line_stream{line};
    std::string token{};
    line_stream >> token;
    if (token == "solid") {
      static_object *o = new (glos::objects.alloc()) static_object{};
      o->is_static = true;
      line_stream >> o->position.x;
      line_stream >> o->position.y;
      line_stream >> o->position.z;

      float agl = 0;
      line_stream >> agl;
      o->angle.x = glm::radians(agl);
      line_stream >> agl;
      o->angle.y = glm::radians(agl);
      line_stream >> agl;
      o->angle.z = glm::radians(agl);

      line_stream >> o->scale.x;
      line_stream >> o->scale.y;
      line_stream >> o->scale.z;

      uint32_t ix = 0;
      line_stream >> ix;
      o->glob_ix(glob_ix[ix]);
      o->bounding_radius = o->glob().bounding_radius * o->scale.x;
      o->mass = 1000;
    }
  }
}

static inline auto load_map(std::filesystem::path path) -> void {
  printf(" * loading map from '%s'\n", path.string().c_str());

  std::ifstream file{path};
  if (!file) {
    throw glos::exception{
        std::format("cannot open file '{}'", path.string().c_str())};
  }
  uint32_t const glob_ix[] = {glob_ix_cube, glob_ix_asteroid_large,
                              glob_ix_asteroid_medium, glob_ix_asteroid_small,
                              glob_ix_landing_pad};
  uint32_t map_width = 0;
  uint32_t map_height = 0;
  {
    std::string line{};
    std::getline(file, line);
    std::istringstream line_stream{line};
    line_stream >> map_width;
    line_stream >> map_height;
  }
  printf("     %u x %u\n", map_width, map_height);

  float const col_incr = grid_columns * grid_cell_size / float(map_width);
  float const row_incr = grid_rows * grid_cell_size / float(map_height);

  std::string line{};
  float row_z = -float(grid_rows) * grid_cell_size / 2 + row_incr / 2;
  for (uint32_t row = 0; row < map_height; ++row) {
    std::getline(file, line);
    std::istringstream line_stream{line};
    float row_x = -float(grid_columns) * grid_cell_size / 2 + col_incr / 2;
    for (uint32_t col = 0; col < map_width; ++col) {
      char ch = '\0';
      line_stream.get(ch);
      if (ch != ' ' && ch != '\n' && ch != '\0') {
        static_object *o = new (glos::objects.alloc()) static_object{};
        o->is_static = true;
        o->position.x = row_x;
        o->position.z = row_z;
        o->scale = {1.0f, 1.0f, 1.0f};
        uint32_t const ix = uint32_t(ch - '0');
        assert(ix < sizeof(glob_ix) / sizeof(uint32_t));
        if (ix > 0 && ix < 4) {
          o->angle = glm::vec3{rnd1(glm::radians(360.0f))};
        }
        o->glob_ix(glob_ix[ix]);
        o->bounding_radius = o->glob().bounding_radius * o->scale.x;
        o->mass = 1000 * o->bounding_radius;
      }
      row_x += col_incr;
    }
    row_z += row_incr;
  }
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
