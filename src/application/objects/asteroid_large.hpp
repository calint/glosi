#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

// include order relevant
#include "../configuration.hpp"
//
#include "asteroid_medium.hpp"
//
#include "../utils.hpp"

class asteroid_large final : public object {
public:
  inline asteroid_large() {
    if (debug_multiplayer) {
      uint32_t const oid = ++object_id;
      // note: 'object_id' increment and assignment to 'oid' is atomic
      name.append("asteroid_large_").append(std::to_string(oid));
      printf("%lu: %lu: create %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }
    glob_ix(glob_ix_asteroid_large);
    scale = vec3{asteroid_large_scale};
    bounding_radius = glob().bounding_radius * scale.x;
    mass = 1500;
    collision_bits = cb_asteroid;
    collision_mask = cb_hero_bullet | cb_hero;
    angular_velocity.y = rnd1(asteroid_large_agl_vel_rnd);
    ++asteroids_alive;
  }

  inline ~asteroid_large() override {
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

    score += 10;

    for (uint32_t i = 0; i < asteroid_large_split; ++i) {
      asteroid_medium *ast = new (objects.alloc()) asteroid_medium{};
      float const br = bounding_radius / 2;
      vec3 const rel_pos = {rnd1(br), 0, rnd1(br)};
      ast->position = position + rel_pos;
      ast->velocity =
          velocity + rnd2(asteroid_large_split_speed) * normalize(rel_pos);
      ast->angular_velocity = vec3{rnd1(asteroid_large_split_agl_vel_rnd)};
    }

    return false;
  }
};
