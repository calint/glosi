#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-01-16
// reviewed: 2024-07-08

namespace glos {

class planes final {
  // cached world coordinate system points and planes
  // note: there is a point for each plane followed by additional points
  //       without planes for use in collision detection
  std::vector<glm::vec4> world_points{}; // x, y, z, 1
  std::vector<glm::vec4> world_planes{}; // A*X + B*Y + C*Z + D = 0
  // the components used in the cached points and planes
  glm::vec3 Mmw_pos{};
  glm::vec3 Mmw_agl{};
  glm::vec3 Mmw_scl{};
  //
  std::atomic_flag lock = ATOMIC_FLAG_INIT;

public:
  bool invalidated = true;

  // points and normals are in model coordinates
  // Mmw matrix was constructed using pos, agl, scl
  inline auto update_model_to_world(std::vector<glm::vec4> const &points,
                                    std::vector<glm::vec3> const &normals,
                                    glm::mat4 const &Mmw, glm::vec3 const &pos,
                                    glm::vec3 const &agl,
                                    glm::vec3 const &scl) -> void {

    bool const angle_scale_changed = Mmw_agl != agl || Mmw_scl != scl;

    if (invalidated || pos != Mmw_pos || angle_scale_changed) {
      // world points and normals are not in sync with object Mmw
      world_points.clear();
      world_points.reserve(points.size());
      for (glm::vec4 const &point : points) {
        glm::vec4 const world_point = Mmw * point;
        world_points.emplace_back(world_point);
      }

      if (invalidated || angle_scale_changed) {
        // the generalized solution is:
        //  glm::mat3 const N = glm::transpose(glm::inverse(glm::mat3(Mmw)));
        //   but since it is known how Mmw is composed a less expensive
        //    operations is done

        bool const is_uniform_scale = scl.x == scl.y && scl.y == scl.z;

        glm::mat3 const N =
            is_uniform_scale
                ? glm::eulerAngleXYZ(agl.x, agl.y, agl.z)
                : glm::scale(glm::eulerAngleXYZ(agl.x, agl.y, agl.z),
                             1.0f / scl);

        world_planes.clear();
        world_planes.reserve(normals.size());
        for (glm::vec3 const &normal : normals) {
          glm::vec3 const world_normal = N * normal;
          // note: world_normal length may not be 1 due to scaling
          world_planes.emplace_back(glm::vec4{world_normal, 0});
          // note: D component (distance to plane from origin along the normal)
          //       in plane equation is set to 0 and will be updated when
          //       'world_points' change
        }
        // save the state of the cache
        Mmw_agl = agl;
        Mmw_scl = scl;
      }

      // update planes D component
      size_t const n = world_planes.size();
      for (size_t i = 0; i < n; ++i) {
        // point on plane
        glm::vec3 const &point = world_points[i];
        glm::vec4 &plane = world_planes[i];
        // D in A*x+B*y+C*z+D=0 stored in w
        plane.w = -glm::dot(glm::vec3{plane}, point);
      }
      // save the state of the cache
      Mmw_pos = pos;
      invalidated = false;
    }
  }

  inline auto debug_render_normals() -> void {
    acquire_lock();
    size_t const n = world_planes.size();
    for (uint32_t i = 0; i < n; ++i) {
      glm::vec4 const &point = world_points.at(i);
      glm::vec4 const &plane = world_planes.at(i);
      debug_render_wcs_line(point, point + plane, {1, 0, 0, 0.5f}, false);
    }
    size_t const m = world_points.size();
    std::vector<glm::vec3> points{};
    for (size_t i = n; i < m; ++i) {
      points.emplace_back(world_points.at(i));
    }
    debug_render_wcs_points(points, {1, 1, 1, 0.5f});
    release_lock();
  }

  // assumes 'this' points and 'pns' planes are updated to world coordinate
  // system
  // @return true if any point in this is behind all planes in 'pns'
  inline auto is_any_point_in_volume(planes const &pns) const -> bool {
    return std::ranges::any_of(world_points, [&](glm::vec4 const &point) {
      return pns.is_point_in_volume(point);
    });
  }

  // assumes update planes to world coordinate system
  inline auto is_point_in_volume(glm::vec4 const &point) const -> bool {
    return std::ranges::all_of(world_planes, [&](glm::vec4 const &plane) {
      return glm::dot(plane, point) <= 0;
    });
  }

  // works in cases where the sphere is much smaller than the convex volume
  //  e.g. bullets vs walls. gives false positives at corners because there are
  //   positions where the sphere is within the collision planes although
  //    outside the volume
  // workaround: add planes to the volume at the corners
  inline auto are_in_collision_with_sphere(glm::vec3 const &position,
                                           float const radius) const -> bool {

    // return are_in_collision_with_sphere_sat(position, radius);

    glm::vec4 const point{position, 1.0f};
    return std::ranges::all_of(world_planes, [&](glm::vec4 const &plane) {
      return glm::dot(point, plane) <= radius * glm::length(glm::vec3{plane});
      // note: division by length of plane normal is necessary because normal
      //       may not be unit vector due to scaling. division moved to the
      //       right-hand side as multiplication for slightly faster operation
    });
  }

  // note: gives false positives. works in 2D. (not used)
  inline auto
  are_in_collision_with_sphere_sat(glm::vec3 const &position,
                                   float const radius) const -> bool {

    // check for separation along each normal
    for (glm::vec4 const &plane : world_planes) {
      glm::vec3 const plane_normal = glm::normalize(glm::vec3{plane});

      float min_projection = glm::dot(glm::vec3{world_points[0]}, plane_normal);
      float max_projection = min_projection;

      // project both the sphere and the convex volume onto the plane normal
      size_t const n = world_planes.size();
      for (uint32_t i = 1; i < n; ++i) {
        glm::vec3 const &point = world_points[i];
        float const projection = glm::dot(point, plane_normal);
        min_projection = std::min(min_projection, projection);
        max_projection = std::max(max_projection, projection);
      }

      float const sphere_projection = glm::dot(position, plane_normal);

      // check for separation
      if (sphere_projection + radius < min_projection ||
          sphere_projection - radius > max_projection) {
        return false; // separating axis found, no collision
      }
    }

    // find the closest world point in planes to sphere
    glm::vec3 closest_point{};
    float min_distance_sq = std::numeric_limits<float>::max();
    for (glm::vec4 const &point : world_points) {
      glm::vec3 const v = glm::vec3{point} - position;
      float const distance_sq = glm::dot(v, v);
      if (distance_sq < min_distance_sq) {
        min_distance_sq = distance_sq;
        closest_point = point;
      }
    }

    debug_render_wcs_points({closest_point}, {1.0f, 1.0f, 1.0f, 1.0f});

    // make that into an axis and check for separation
    glm::vec3 const axis = glm::normalize(closest_point - position);

    // debug_render_wcs_line(glm::vec3{0, 0, 0}, axis * 4.0f,
    //                       {0.0f, 1.0f, 0.0f, 1.0f});

    // project both the sphere and the convex volume onto the axis
    float volume_min_projection = glm::dot(glm::vec3{world_points[0]}, axis);
    float volume_max_projection = volume_min_projection;
    size_t const n = world_points.size();
    for (uint32_t i = 1; i < n; ++i) {
      float const projection = glm::dot(glm::vec3{world_points[i]}, axis);
      volume_min_projection = std::min(volume_min_projection, projection);
      volume_max_projection = std::max(volume_max_projection, projection);
    }

    float const sphere_projection = glm::dot(position, axis);

    debug_render_wcs_line(axis * (sphere_projection - radius),
                          axis * (sphere_projection + radius),
                          {1.0f, 0.0f, 0.0f, 1.0f}, false);

    debug_render_wcs_line(axis * volume_min_projection,
                          axis * volume_max_projection,
                          {1.0f, 1.0f, 1.0f, 0.5f}, false);

    // check for separation along the axis
    if (sphere_projection + radius < volume_min_projection ||
        sphere_projection - radius > volume_max_projection) {
      return false; // separating axis found, no collision
    }

    // debug_render_wcs_line(pos, pos + axis, {0.0f, 1.0f, 0.0f, 1.0f});

    // no separating axis found, collision detected
    return true;
  }

  inline auto acquire_lock() -> void {
    while (lock.test_and_set(std::memory_order_acquire)) {
    }
  }

  inline auto release_lock() -> void { lock.clear(std::memory_order_release); }

  // tests whether any point in 'pns1' is within the volume defined by 'pns2'
  //  and vice versa
  inline static auto are_in_collision(planes const &pns1,
                                      planes const &pns2) -> bool {
    return pns1.is_any_point_in_volume(pns2) ||
           pns2.is_any_point_in_volume(pns1);
  }
};
} // namespace glos