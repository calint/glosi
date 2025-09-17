#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/objects.hpp"
#include "../configuration.hpp"
#include "../state.hpp"

class static_object final : public glos::object {
  public:
    inline static_object() {
        if (debug_multiplayer) {
            uint32_t const oid = ++object_id;
            // note: 'object_id' increment and assignment to 'oid' is atomic
            name.append("static_object_").append(std::to_string(oid));
            printf("%lu: %lu: create %s\n", glos::frame_context.frame_num,
                   glos::frame_context.ms, name.c_str());
        }
        collision_bits = cb_static_object;
        collision_mask = cb_none;
    }

    inline ~static_object() override {
        if (debug_multiplayer) {
            printf("%lu: %lu: free %s\n", glos::frame_context.frame_num,
                   glos::frame_context.ms, name.c_str());
        }
    }

    inline auto update() -> bool override { return true; }

    inline auto on_collision(object* o) -> bool override {
        if (debug_multiplayer) {
            printf("%lu: %lu: %s collision with %s\n",
                   glos::frame_context.frame_num, glos::frame_context.ms,
                   name.c_str(), o->name.c_str());
        }
        return false;
    }
};
