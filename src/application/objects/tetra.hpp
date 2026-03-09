#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/objects.hpp"
#include "../state.hpp"

class tetra final : public glos::object {
  public:
    tetra() {
        glob_ix(glob_ix_tetra);
        scale = {1.f, 1.f, 1.f};
        bounding_radius = glob().bounding_radius * scale.x;
        mass(1);
        collision_bits = cb_tetra;
        collision_mask = cb_cube;
    }
};
