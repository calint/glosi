#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

// include order relevant
#include "../configuration.hpp"
//
#include "asteroid_small.hpp"
//
#include "../utils.hpp"

class asteroid_medium final : public object {
public:
  inline asteroid_medium() {
    if (debug_multiplayer) {
      uint32_t const oid = ++object_id;
      // note: 'object_id' increment and assignment to 'oid' is atomic
      name.append("asteroid_medium_").append(std::to_string(oid));
      printf("%lu: %lu: create %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }
    glob_ix(glob_ix_asteroid_medium);
    scale = vec3{asteroid_medium_scale};
    bounding_radius = glob().bounding_radius * scale.x;
    mass = 1000;
    collision_bits = cb_asteroid;
    collision_mask = cb_hero_bullet | cb_hero;
    ++asteroids_alive;
  }

  inline ~asteroid_medium() override {
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

    score += 20;

    for (uint32_t i = 0; i < asteroid_medium_split; ++i) {
      asteroid_small *ast = new (objects.alloc()) asteroid_small{};
      float const br = bounding_radius / 2;
      vec3 const rel_pos = {rnd1(br), 0, rnd1(br)};
      ast->position = position + rel_pos;
      ast->velocity =
          velocity + rnd2(asteroid_medium_split_speed) * normalize(rel_pos);
      ast->angular_velocity = vec3{rnd1(asteroid_medium_split_agl_vel_rnd)};
    }

    return false;
  }
};
