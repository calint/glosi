#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "fragment.hpp"

class ship_bullet final : public object {
public:
  inline ship_bullet() {
    if (debug_multiplayer) {
      uint32_t const oid = ++object_id;
      // note: 'object_id' increment and assignment to 'oid' is atomic
      name.append("ship_bullet_").append(std::to_string(oid));
      printf("%lu: %lu: create %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }
    glob_ix(glob_ix_bullet);
    scale = {1.0f, 1.0f, 1.0f};
    bounding_radius = glob().bounding_radius * scale.x;
    mass = 5;
    collision_bits = cb_hero_bullet;
    collision_mask = cb_asteroid | cb_ufo;
  }

  inline ~ship_bullet() override {
    if (debug_multiplayer) {
      printf("%lu: %lu: free %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }
  }

  inline auto update() -> bool override {
    if (!object::update()) {
      return false;
    }

    if (is_outside_game_area(position)) {
      return false;
    }

    return true;
  }

  inline auto on_collision(object *o) -> bool override {
    if (debug_multiplayer) {
      printf("%lu: %lu: %s collision with %s\n", frame_context.frame_num,
             frame_context.ms, name.c_str(), o->name.c_str());
    }

    fragment *frg = new (objects.alloc()) fragment{};
    frg->position = position;
    frg->angular_velocity = vec3{rnd1(bullet_fragment_agl_vel_rnd)};
    frg->death_time_ms = frame_context.ms + 500;

    return false;
  }
};
