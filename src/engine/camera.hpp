#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

#define GLM_ENABLE_EXPERIMENTAL

#include "exception.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace glos {

class camera final {
  public:
    enum class type { LOOK_AT, ANGLE, ORTHOGONAL };

    glm::vec3 position{0, 50, 0};
    glm::vec3 look_at{0, 0, -.000001f};
    // note: -.000001f because of the math in 'look at' mode
    glm::vec3 angle{};

    float near_plane = 0.1f;
    float far_plane = 300;
    float field_of_view = 60.0f; // in degrees

    // window dimensions
    float width = 1024;
    float height = 1024;

    // orthogonal projection settings
    float ortho_min_x = -50;
    float ortho_max_x = 50;
    float ortho_min_y = -50;
    float ortho_max_y = 50;

    glm::mat4 Mwvp{}; // world to view to projection matrix

    type type = type::ORTHOGONAL;

    inline auto init() -> void {}

    inline auto free() -> void {}

    inline auto update_matrix_wvp() -> void {
        switch (type) {
        case type::LOOK_AT: {
            float const aspect_ratio = height == 0 ? 1 : (width / height);
            glm::mat4 const Mv = glm::lookAt(position, look_at, {0, 1, 0});
            glm::mat4 const Mp =
                glm::perspective(glm::radians(field_of_view), aspect_ratio,
                                 near_plane, far_plane);
            Mwvp = Mp * Mv;
            break;
        }
        case type::ORTHOGONAL: {
            glm::mat4 const Mv = glm::lookAt(position, look_at, {0, 1, 0});
            glm::mat4 const Mp =
                glm::ortho(ortho_min_x, ortho_max_x, ortho_min_y, ortho_max_y,
                           near_plane, far_plane);
            Mwvp = Mp * Mv;
            break;
        }
        case type::ANGLE: {
            float const aspect_ratio = height == 0 ? 1 : (width / height);
            glm::mat4 const Mv = glm::eulerAngleXYZ(angle.x, angle.y, angle.z);
            glm::mat4 const Mp =
                glm::perspective(glm::radians(field_of_view), aspect_ratio,
                                 near_plane, far_plane);
            Mwvp = Mp * Mv;
            break;
        }
        default:
            throw exception{"unknown case"};
        }
    }
} static camera{};

} // namespace glos
