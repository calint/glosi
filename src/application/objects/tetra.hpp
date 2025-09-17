#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/objects.hpp"
#include "../state.hpp"

class tetra final : public glos::object {
  public:
    inline tetra() {
        if (debug_multiplayer) {
            uint32_t const oid = ++object_id;
            // note: 'object_id' increment and assignment to 'oid' is atomic
            name.append("tetra_").append(std::to_string(oid));
            printf("%lu: %lu: create %s\n", glos::frame_context.frame_num,
                   glos::frame_context.ms, name.c_str());
        }
        glob_ix(glob_ix_tetra);
        scale = {2.0f, 1.0f, 7.0f};
        bounding_radius = glob().bounding_radius * scale.x;
        mass = 10;
        collision_bits = cb_power_up;
        collision_mask = cb_hero;
    }

    inline ~tetra() override {
        if (debug_multiplayer) {
            printf("%lu: %lu: free %s\n", glos::frame_context.frame_num,
                   glos::frame_context.ms, name.c_str());
        }
    }

    inline auto update() -> bool override {
        if (!object::update()) {
            return false;
        }

        return true;
    }

    inline auto on_collision(object* o) -> bool override {
        if (debug_multiplayer) {
            printf("%lu: %lu: %s collision with %s\n",
                   glos::frame_context.frame_num, glos::frame_context.ms,
                   name.c_str(), o->name.c_str());
        }

        ++score;

        return true;
    }
};
