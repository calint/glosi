#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/objects.hpp"
#include "../state.hpp"

class fragment final : public glos::object {
  public:
    uint64_t death_time_ms = 0;

    inline fragment() {
        if (debug_multiplayer) {
            uint32_t const oid = ++object_id;
            // note: 'object_id' increment and assignment to 'oid' is atomic
            name.append("fragment_").append(std::to_string(oid));
            printf("%lu: %lu: create %s\n", glos::frame_context.frame_num,
                   glos::frame_context.ms, name.c_str());
        }
        glob_ix(glob_ix_fragment);
        scale = {0.5f, 0.5f, 0.5f};
        bounding_radius = glob().bounding_radius * scale.x;
        mass = 10;
        collision_bits = cb_none;
        collision_mask = cb_none;
    }

    inline ~fragment() override {
        if (debug_multiplayer) {
            printf("%lu: %lu: free %s\n", glos::frame_context.frame_num,
                   glos::frame_context.ms, name.c_str());
        }
    }

    inline auto update() -> bool override {
        if (!object::update()) {
            return false;
        }

        if (death_time_ms < glos::frame_context.ms) {
            return false;
        }

        return true;
    }
};
