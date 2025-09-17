#pragma once
#include <atomic>
#include <cstdint>

enum class state { init, asteroids, ufo };

static state state = state::init;
static uint32_t level = 1;
static std::atomic_int32_t score{0};
static std::atomic_uint32_t asteroids_alive{0};
static std::atomic_uint32_t ufos_alive{0};
static std::atomic_uint32_t object_id{0};
// note: used when 'debug_multiplayer' is true to give objects unique numbers

class ship;
static ship *hero = nullptr;

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
static uint32_t glob_ix_landing_pad = 0;
