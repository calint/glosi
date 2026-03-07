#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-08
// reviewed: 2024-07-08
// reviewed: 2024-07-15

#include "../../engine/objects.hpp"
#include "../configuration.hpp"

class static_object final : public glos::object {
  public:
    static_object() {
        collision_bits = cb_static_object;
        collision_mask = cb_none;
    }
};
