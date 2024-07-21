#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

class sphere final : public object {
public:
  inline sphere() {
    if (debug_multiplayer) {
      uint32_t const oid = ++object_id;
      // note: 'object_id' increment and assignment to 'oid' is atomic
      name.append("sphere_").append(std::to_string(oid));
      printf("%lu: %lu: create %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
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
      printf("%lu: %lu: free %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
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

    velocity = {};

    uint64_t const keys = net_state->keys;

    // handle ship controls
    float const v = 1;
    if (keys & key_w) {
      velocity.z = -v;
    }
    if (keys & key_s) {
      velocity.z = v;
    }
    if (keys & key_a) {
      velocity.x = -v;
    }
    if (keys & key_d) {
      velocity.x = v;
    }
    if (keys & key_i) {
      camera.position = {0, 50, 0};
    }
    if (keys & key_k) {
      camera.position = {0, 0, 50};
    }

    return true;
  }

  inline auto on_collision(object *o) -> bool override {
    if (debug_multiplayer) {
      printf("%lu: %lu: %s collision with %s\n", frame_context.frame_num,
             frame_context.ms, name.c_str(), o->name.c_str());
    }

    ++score;

    return true;
  }
};
