#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "ship_bullet.hpp"

struct bullet_level final {
  uint32_t fire_interval_ms;
  uint32_t bullets_per_fire;
  float bullet_spread;
};

static bullet_level constexpr bullet_levels[]{
    {1000, 1, 0}, {500, 1, 0}, {250, 1, 0}, {125, 1, 0}, {500, 3, 2},
    {250, 3, 2},  {125, 3, 2}, {500, 5, 4}, {250, 5, 4}, {125, 5, 4}};

static uint32_t constexpr bullet_levels_length =
    sizeof(bullet_levels) / sizeof(bullet_level);

static uint32_t bullet_levels_index = 0;

class ship final : public object {
  uint64_t ready_to_fire_at_ms = 0;

public:
  inline ship() {
    if (debug_multiplayer) {
      uint32_t const oid = ++object_id;
      // note: 'object_id' increment and assignment to 'oid' is atomic
      name.append("ship_").append(std::to_string(oid));
      printf("%lu: %lu: create %s\n", frame_context.frame_num, frame_context.ms,
             name.c_str());
    }
    glob_ix(glob_ix_ship);
    scale = {1.0f, 1.0f, 1.0f};
    bounding_radius = glob().bounding_radius * scale.x;
    mass = 150;
    collision_bits = cb_hero;
    collision_mask = cb_asteroid | cb_power_up | cb_ufo | cb_ufo_bullet;
  }

  inline ~ship() override {
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

    angular_velocity = {};
    glob_ix(glob_ix_ship);

    if (net_state == nullptr) {
      return true;
    }

    uint64_t const keys = net_state->keys;

    // handle ship controls
    if (keys & key_w) {
      velocity +=
          ship_speed * vec3{-sin(angle.y), 0, -cos(angle.y)} * frame_context.dt;
      glob_ix(glob_ix_ship_engine_on);
    }
    if (keys & key_a) {
      angular_velocity.y = ship_turn_rate;
    }
    if (keys & key_d) {
      angular_velocity.y = -ship_turn_rate;
    }
    if (keys & key_j) {
      fire();
    }
    if (keys & key_k) {
      camera.type = camera::type::LOOK_AT;
      camera.position = {0, 30, 30};
      camera.look_at = {0, 0, -.000001f};
      // note: -.000001f because of the math of 'look at'
    }
    if (keys & key_l) {
      camera.type = camera::type::ORTHOGONAL;
      camera.position = {0, 50, 0};
      camera.look_at = {0, 0, -.000001f};
      // note: -.000001f because of the math of 'look at'
    }

    return true;
  }

  inline auto on_collision(object *o) -> bool override {
    if (debug_multiplayer) {
      printf("%lu: %lu: %s collision with %s\n", frame_context.frame_num,
             frame_context.ms, name.c_str(), o->name.c_str());
    }

    if (typeid(*o) == typeid(power_up)) {
      score += 100;
      if (bullet_levels_index < bullet_levels_length - 1) {
        ++bullet_levels_index;
      }
      return true;
    }

    angle.y += radians(rnd1(45));

    fragment *frg = new (objects.alloc()) fragment{};
    frg->position = o->position;
    frg->angular_velocity = vec3{rnd1(bullet_fragment_agl_vel_rnd)};
    frg->death_time_ms = frame_context.ms + 500;

    score -= 100;

    if (bullet_levels_index > 0) {
      --bullet_levels_index;
    }

    return true;
  }

private:
  inline auto fire() -> void {
    if (ready_to_fire_at_ms > frame_context.ms) {
      return;
    }

    mat4 const &M = updated_Mmw();
    vec3 const forward_vec = -normalize(vec3{M[2]});
    // note: M[2] is third column: z-axis
    // note: forward for object model space is negative z

    bullet_level const &bl = bullet_levels[bullet_levels_index];
    for (uint32_t i = 0; i < bl.bullets_per_fire; ++i) {
      ship_bullet *blt = new (objects.alloc()) ship_bullet{};
      blt->position = position + forward_vec;
      blt->angle = angle;
      blt->velocity = ship_bullet_speed * forward_vec;
      blt->velocity.x += rnd1(bl.bullet_spread);
      blt->velocity.z += rnd1(bl.bullet_spread);
    }
    ready_to_fire_at_ms = frame_context.ms + bl.fire_interval_ms;
  }
};
