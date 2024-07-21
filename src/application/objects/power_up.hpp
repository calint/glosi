#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../utils.hpp"

class power_up final : public object {
  uint64_t death_time_ms = 0;
  uint64_t scale_time_ms = 0;
  bool scale_up = false;

public:
  inline power_up() {
    if (debug_multiplayer) {
      uint32_t const oid = ++object_id;
      // note: 'object_id' increment and assignment to 'oid' is atomic
      name.append("power_up_").append(std::to_string(oid));
      printf("%lu: %lu: create %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }
    glob_ix(glob_ix_power_up);
    scale = {0.5f, 0.5f, 0.5f};
    bounding_radius = glob().bounding_radius * scale.x;
    mass = 10;
    is_sphere = true;
    collision_bits = cb_power_up;
    collision_mask = cb_hero;
    death_time_ms = frame_context.ms + power_up_lifetime_ms;
  }

  inline ~power_up() override {
    if (debug_multiplayer) {
      printf("%lu: %lu: free %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }
  }

  inline auto update() -> bool override {
    if (!object::update()) {
      return false;
    }

    if (death_time_ms < frame_context.ms) {
      return false;
    }

    if (scale_time_ms < frame_context.ms) {
      if (scale_up) {
        scale_up = false;
        scale = {0.75f, 0.75f, 0.75f};
        bounding_radius = glob().bounding_radius * scale.x;
      } else {
        scale_up = true;
        scale = {0.5f, 0.5f, 0.5f};
        bounding_radius = glob().bounding_radius * scale.x;
      }
      scale_time_ms = frame_context.ms + 1'000;
    }

    game_area_roll(position);

    return true;
  }

  inline auto on_collision(object *o) -> bool override {
    if (debug_multiplayer) {
      printf("%lu: %lu: %s collision with %s\n", frame_context.frame_num,
             frame_context.ms, name.c_str(), o->name.c_str());
    }

    return false;
  }
};
