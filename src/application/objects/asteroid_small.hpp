#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

// include order relevant
#include "../configuration.hpp"
//
#include "power_up.hpp"
//
#include "../utils.hpp"

class asteroid_small final : public object {
  static inline uint64_t power_up_soonest_next_spawn_ms = 0;

public:
  inline asteroid_small() {
    if (debug_multiplayer) {
      uint32_t const oid = ++object_id;
      // note: 'object_id' increment and assignment to 'oid' is atomic
      name.append("asteroid_small_").append(std::to_string(oid));
      printf("%lu: %lu: create %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }
    glob_ix(glob_ix_asteroid_small);
    scale = vec3{asteroid_small_scale};
    bounding_radius = glob().bounding_radius * scale.x;
    mass = 500;
    collision_bits = cb_asteroid;
    collision_mask = cb_hero_bullet | cb_hero;
    ++asteroids_alive;
  }

  inline ~asteroid_small() override {
    if (debug_multiplayer) {
      printf("%lu: %lu: free %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }

    --asteroids_alive;
  }

  inline auto update() -> bool override {
    if (!object::update()) {
      return false;
    }

    game_area_roll(position);

    return true;
  }

  inline auto on_collision(object *o) -> bool override {
    if (debug_multiplayer) {
      printf("%lu: %lu: %s collision with %s\n", frame_context.frame_num,
             frame_context.ms, name.c_str(), o->name.c_str());
    }

    score += 40;

    if (frame_context.ms > power_up_soonest_next_spawn_ms &&
        rnd3(power_up_chance_rem)) {
      power_up_soonest_next_spawn_ms =
          frame_context.ms + power_up_min_span_interval_ms;
      power_up *pu = new (objects.alloc()) power_up{};
      pu->position = position;
      pu->angular_velocity.y = radians(90.0f);
    }

    return false;
  }
};
