#pragma once
// declarations here are used both by engine and client code that leads to
// circular references if declared in `engine.hpp`

#include <glm/glm.hpp>
#include <vector>

namespace glos {

// forward declaration of debugging functions
static inline auto debug_render_wcs_line(glm::vec3 const& from_wcs,
                                         glm::vec3 const& to_wcs,
                                         glm::vec4 const& color,
                                         bool const depth_test) -> void;

static inline auto debug_render_wcs_points(std::vector<glm::vec3> const& points,
                                           glm::vec4 const& color) -> void;

static inline auto debug_render_bounding_sphere(glm::mat4 const& Mmw) -> void;

// information about the current frame
class frame_context final {
  public:
    uint64_t frame_num = 0; // frame number (will rollover)
    uint64_t ms = 0; // current time since start in milliseconds (will rollover)
    float dt = 0;    // frame delta time in seconds (time step)
};

// globally accessible current frame info
static frame_context frame_context{};

// set and unset by engine
static bool is_debug_object_planes_normals = false;
static bool is_debug_object_bounding_sphere = false;

// signal bit corresponding to keyboard key (max 64)
static uint32_t constexpr key_w = 1u << 0u;
static uint32_t constexpr key_a = 1u << 1u;
static uint32_t constexpr key_s = 1u << 2u;
static uint32_t constexpr key_d = 1u << 3u;
static uint32_t constexpr key_q = 1u << 4u;
static uint32_t constexpr key_e = 1u << 5u;
static uint32_t constexpr key_i = 1u << 6u;
static uint32_t constexpr key_j = 1u << 7u;
static uint32_t constexpr key_k = 1u << 8u;
static uint32_t constexpr key_l = 1u << 9u;
static uint32_t constexpr key_u = 1u << 10u;
static uint32_t constexpr key_o = 1u << 11u;
static uint32_t constexpr key_space = 1u << 12u;

} // namespace glos
