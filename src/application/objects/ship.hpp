#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/camera.hpp"
#include "../../engine/objects.hpp"
#include "../state.hpp"

class ship final : public glos::object {
  public:
    ship() {
        glob_ix(glob_ix_cube);
        scale = {1.0f, 1.0f, 1.0f};
        bounding_radius = glob().bounding_radius * scale.x;
        mass = 1;
        collision_bits = cb_ship;
        collision_mask = cb_cube | cb_static_object;
    }

    auto update() -> bool override {
        if (!object::update()) {
            return false;
        }

        float const dt = glos::frame_context.dt;

        angular_velocity = {};

        if (net_state == nullptr) {
            return true;
        }

        uint64_t const keys = net_state->keys;

        // handle ship controls
        if (keys & glos::key_w) {
            linear_velocity +=
                10.f * glm::vec3{-sinf(angle.y), 0, -cosf(angle.y)} * dt;
        }
        if (keys & glos::key_a) {
            angular_velocity.y = deg_to_rad(120.f);
        }
        if (keys & glos::key_d) {
            angular_velocity.y = -deg_to_rad(120.f);
        }
        if (keys & glos::key_k) {
            glos::camera.type = glos::camera::type::LOOK_AT;
            glos::camera.position = {0, 30, 30};
            glos::camera.look_at = {0, 0, -.000001f};
            // note: -.000001f because of the math of 'look at'
        }
        if (keys & glos::key_l) {
            glos::camera.type = glos::camera::type::ORTHOGONAL;
            glos::camera.position = {0, 50, 0};
            glos::camera.look_at = {0, 0, -.000001f};
            // note: -.000001f because of the math of 'look at'
        }

        return true;
    }
};
