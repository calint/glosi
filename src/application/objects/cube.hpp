#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/camera.hpp"
#include "../../engine/decouple.hpp"
#include "../../engine/objects.hpp"
#include "../state.hpp"
#include "../utils.hpp"

class cube final : public glos::object {
  public:
    inline cube() {
        if (debug_multiplayer) {
            uint32_t const oid = ++object_id;
            // note: 'object_id' increment and assignment to 'oid' is atomic
            name.append("cube_").append(std::to_string(oid));
            printf("%lu: %lu: create %s\n", glos::frame_context.frame_num,
                   glos::frame_context.ms, name.c_str());
        }
        glob_ix(glob_ix_cube);
        scale = {1.0f, 1.0f, 1.0f};
        bounding_radius = glob().bounding_radius * scale.x;
        mass = 10;
        collision_bits = cb_power_up;
        collision_mask = cb_power_up;
    }

    inline ~cube() override {
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
        angular_velocity = {};

        uint64_t const keys = net_state->keys;

        // handle controls
        float const v = 1;
        float const a = glm::radians(10.0f);
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
        if (keys & glos::key_q) {
            angular_velocity.y = a;
        }
        if (keys & glos::key_e) {
            angular_velocity.y = -a;
        }
        if (keys & glos::key_i) {
            glos::camera.position = {0, 50, 0};
        }
        if (keys & glos::key_k) {
            glos::camera.position = {0, 0, 50};
        }

        return true;
    }

    inline auto on_collision(object* o) -> bool override {
        if (debug_multiplayer) {
            printf("%lu: %lu: %s collision with %s\n",
                   glos::frame_context.frame_num, glos::frame_context.ms,
                   name.c_str(), o->name.c_str());
        }

        score += 1;

        return true;
    }
};
