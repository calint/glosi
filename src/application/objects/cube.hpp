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
        InvIm = calculate_InvIm(mass, scale);
        collision_bits = cb_cube;
        collision_mask = cb_cube | cb_ship | cb_tetra;
    }

    bool on_collision(object* obj) override {
        std::printf("collision between %p and %p\n",
                    static_cast<void const*>(this),
                    static_cast<void const*>(obj));
        return true;
    }

    [[nodiscard]]
    static auto calculate_InvIm(float const m, glm::vec3 const& s)
        -> glm::mat3 {

        // a mass of 0 or infinity indicates a static object
        if (m <= 0.0f) [[unlikely]] {
            return glm::mat3{0.0f};
        }

        auto const s2 = s * s;
        auto const fraction = m / 12.0f;

        // principle moments of inertia for a solid cuboid
        auto const ix = fraction * (s2.y + s2.z);
        auto const iy = fraction * (s2.x + s2.z);
        auto const iz = fraction * (s2.x + s2.y);

        // return the inverse diagonal matrix
        // 1.0f / i to get the inverse required for angular velocity updates
        return glm::mat3{1.0f / ix, 0.0f, 0.0f, 0.0f,     1.0f / iy,
                         0.0f,      0.0f, 0.0f, 1.0f / iz};
    }
};
