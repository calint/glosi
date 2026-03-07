#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-08
// reviewed: 2024-07-08

#include <cstddef>
#include <cstdint>
#include <numbers>

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

// seed for random number generator
static int constexpr random_seed = 2;

// ----------------------------------------------------------------------
// application configuration
// ----------------------------------------------------------------------

// helper function
static auto constexpr deg_to_rad(float const deg) -> float {
    return std::numbers::pi_v<float> * deg / 180.0f;
}

// game or performance test
// 0: none (game)  1: cubes  2: spheres
static uint32_t constexpr performance_test_type = 0;

static bool constexpr is_performance_test = performance_test_type != 0;
static float constexpr cube_speed = 10;
static float constexpr cube_angular_velocity = deg_to_rad(90.0f);
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
static uint32_t constexpr cb_ship = 1u << 0;
static uint32_t constexpr cb_cube = 1u << 1;
static uint32_t constexpr cb_sphere = 1u << 2;
static uint32_t constexpr cb_static_object = 1u << 3;
static uint32_t constexpr cb_tetra = 1u << 4;

// game area based on grid and biggest object
static float constexpr game_area_half_x = grid_size / 2;
static float constexpr game_area_half_y = 10; // screen depth
static float constexpr game_area_half_z = grid_size / 2;
