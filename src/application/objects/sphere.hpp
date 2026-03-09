#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/objects.hpp"
#include "../state.hpp"

class sphere final : public glos::object {
  public:
    sphere() {
        glob_ix(glob_ix_sphere);
        scale = {1.0f, 1.0f, 1.0f};
        bounding_radius = glob().bounding_radius * scale.x;
        is_sphere = true;
        mass(1);
        collision_bits = cb_sphere;
        collision_mask = cb_none;
    }
};
