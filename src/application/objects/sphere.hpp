#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/camera.hpp"
#include "../../engine/objects.hpp"
#include "../state.hpp"
#include "../utils.hpp"

class sphere final : public glos::object {
public:
  inline sphere() {
    if (debug_multiplayer) {
      uint32_t const oid = ++object_id;
      // note: 'object_id' increment and assignment to 'oid' is atomic
      name.append("sphere_").append(std::to_string(oid));
      printf("%lu: %lu: create %s\n", glos::frame_context.frame_num,
             glos::frame_context.ms, name.c_str());
    }
    glob_ix(glob_ix_sphere);
    scale = {1.0f, 1.0f, 1.0f};
    bounding_radius = glob().bounding_radius * scale.x;
    is_sphere = true;
    mass = 1;
    collision_bits = cb_power_up;
    collision_mask = cb_power_up;
  }

  inline ~sphere() override {
    if (debug_multiplayer) {
      printf("%lu: %lu: free %s\n", glos::frame_context.frame_num,
             glos::frame_context.ms, name.c_str());
    }
  }

  inline auto update() -> bool override {
    if (!object::update()) {
      return false;
    }

    game_area_roll(position);

    if (net_state == nullptr) {
      return true;
    }

    linear_velocity = {};

    uint64_t const keys = net_state->keys;

    // handle ship controls
    float const v = 1;
    if (keys & glos::key_w) {
      linear_velocity.z = -v;
    }
    if (keys & glos::key_s) {
      linear_velocity.z = v;
    }
    if (keys & glos::key_a) {
      linear_velocity.x = -v;
    }
    if (keys & glos::key_d) {
      linear_velocity.x = v;
    }
    if (keys & glos::key_i) {
      glos::camera.position = {0, 50, 0};
    }
    if (keys & glos::key_k) {
      glos::camera.position = {0, 0, 50};
    }

    return true;
  }

  inline auto on_collision(object *o) -> bool override {
    if (debug_multiplayer) {
      printf("%lu: %lu: %s collision with %s\n", glos::frame_context.frame_num,
             glos::frame_context.ms, name.c_str(), o->name.c_str());
    }

    ++score;

    return true;
  }
};
