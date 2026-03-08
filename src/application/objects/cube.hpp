#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/objects.hpp"
#include "../state.hpp"

class cube final : public glos::object {
  public:
    cube() {
        glob_ix(glob_ix_cube);
        scale = {1.f, 1.f, 1.f};
        bounding_radius = glob().bounding_radius * scale.x;
        mass = 1;
        collision_bits = cb_cube;
        collision_mask = cb_cube | cb_ship;
    }

    bool on_collision(object* obj) override {
        std::printf("collision between %p and %p\n",
                    static_cast<void const*>(this),
                    static_cast<void const*>(obj));
        return false;
    }
};
