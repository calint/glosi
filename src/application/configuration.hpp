#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-08
// reviewed: 2024-07-08

// ----------------------------------------------------------------------
// engine configuration
// ----------------------------------------------------------------------

// multithreaded grid
// note: in some cases multithreaded mode is a degradation of performance
// note: multiplayer mode cannot use 'threaded_grid' because of the
//       non-deterministic behavior
static bool constexpr threaded_grid = false;
static bool constexpr threaded_update = false;

// o1store debugging (assertions should be on in development)
static bool constexpr o1store_check_double_free = false;
static bool constexpr o1store_check_free_limits = false;

// metrics print to console
static bool constexpr metrics_print = true;

// initially render the hud
static bool constexpr hud_enabled = true;

// platform cache line size
static size_t constexpr cache_line_size_B = 64;

// ----------------------------------------------------------------------
// application configuration
// ----------------------------------------------------------------------

// game or performance test
// 0: none (game)  1: cubes  2: spheres
static uint32_t constexpr performance_test_type = 0;

static bool constexpr is_performance_test = performance_test_type != 0;
static float constexpr cube_speed = 10;
static float constexpr sphere_speed = 10;

// grid dimensions
static float constexpr grid_size = is_performance_test ? 3200 : 40;
// square side in e.g. meters

static uint32_t constexpr grid_rows = is_performance_test ? 16 : 4;
static uint32_t constexpr grid_columns = grid_rows;
static float constexpr grid_cell_size = grid_size / grid_rows;

// window dimensions
static uint32_t constexpr window_width = 1024;
static uint32_t constexpr window_height = 1024;
static bool constexpr window_vsync = is_performance_test ? false : true;
// note: vsync should be on when not doing performance tests

// number of players in networked mode
static uint32_t constexpr net_players = 2;

// multiplayer debugging output
static bool constexpr debug_multiplayer = false;

// maximum size of any object instance in bytes
// note: must be multiple of 'cache_line_size_B'
static size_t constexpr objects_instance_size_B = 512;

// number of preallocated objects
static uint32_t constexpr objects_count =
    is_performance_test ? 64 * 1024 : 1 * 1024;

// collision bits
static uint32_t constexpr cb_none = 0;
static uint32_t constexpr cb_hero = 1U << 0U;
static uint32_t constexpr cb_hero_bullet = 1U << 1U;
static uint32_t constexpr cb_asteroid = 1U << 2U;
static uint32_t constexpr cb_power_up = 1U << 3U;
static uint32_t constexpr cb_ufo = 1U << 4U;
static uint32_t constexpr cb_ufo_bullet = 1U << 5U;

// settings
static uint32_t constexpr asteroids_per_level = 2;

static float constexpr asteroid_large_agl_vel_rnd = glm::radians(75.0f);
static float constexpr asteroid_large_speed = 10;
static float constexpr asteroid_large_scale = 2;
static uint32_t constexpr asteroid_large_split = 4;
static float constexpr asteroid_large_split_speed = 6;
static float constexpr asteroid_large_split_agl_vel_rnd = glm::radians(120.0f);

static float constexpr asteroid_medium_scale = 1.2f;
static uint32_t constexpr asteroid_medium_split = 4;
static float constexpr asteroid_medium_split_speed = 6;
static float constexpr asteroid_medium_split_agl_vel_rnd = glm::radians(200.0f);

static float constexpr asteroid_small_scale = 0.75f;

static float constexpr ship_turn_rate = glm::radians(120.0f);
static float constexpr ship_speed = 6;
static float constexpr ship_bullet_speed = 17;

static float constexpr bullet_fragment_agl_vel_rnd = glm::radians(360.0f);

static int32_t constexpr power_up_chance_rem = 5;
static uint32_t constexpr power_up_lifetime_ms = 30'000;
static uint32_t constexpr power_up_min_span_interval_ms = 5'000;

static float constexpr ufo_velocity = 15;
static float constexpr ufo_angle_x_rate = 40.0f;
static float constexpr ufo_bullet_velocity = 12;
static uint32_t constexpr ufo_fire_rate_interval_ms = 2'500;
static uint32_t constexpr ufo_power_ups_at_death = 3;
static float constexpr ufo_power_up_velocity = 5;

// game area based on grid and biggest object
static float constexpr game_area_half_x = grid_size / 2;
static float constexpr game_area_half_y = 10; // screen depth
static float constexpr game_area_half_z = grid_size / 2;

// set game area so the largest object, asteroid_large, just fits outside the
// screen before rollover
static float constexpr game_area_min_x =
    -game_area_half_x - asteroid_large_scale;
static float constexpr game_area_max_x =
    game_area_half_x + asteroid_large_scale;
static float constexpr game_area_min_y =
    -game_area_half_y - asteroid_large_scale;
static float constexpr game_area_max_y =
    game_area_half_y + asteroid_large_scale;
static float constexpr game_area_min_z =
    -game_area_half_z - asteroid_large_scale;
static float constexpr game_area_max_z =
    game_area_half_z + asteroid_large_scale;
