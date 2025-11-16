#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-07
// reviewed: 2024-01-10
// reviewed: 2024-01-16
// reviewed: 2024-07-08

#include "cell.hpp"
#include <execution>

namespace glos {

class grid final {
    std::array<std::array<cell, grid_columns>, grid_rows> cells{};

  public:
    inline auto init() -> void {}

    inline auto free() -> void {}

    // called from engine
    inline auto update() -> void {
        if (threaded_grid) {
            std::for_each(std::execution::par_unseq, std::cbegin(cells),
                          std::cend(cells), [](auto const& row) {
                              for (cell const& c : row) {
                                  c.update();
                              }
                          });
            return;
        }

        for (auto const& row : cells) {
            for (cell const& c : row) {
                c.update();
            }
        }
    }

    // called from engine
    inline auto resolve_collisions() -> void {
        if (threaded_grid) {
            std::for_each(std::execution::par_unseq, std::begin(cells),
                          std::end(cells), [](auto& row) {
                              for (cell& c : row) {
                                  c.resolve_collisions();
                              }
                          });
            return;
        }

        for (auto& row : cells) {
            for (cell& c : row) {
                c.resolve_collisions();
            }
        }
    }

    // called from engine
    inline auto render() const -> void {
        for (auto const& row : cells) {
            for (cell const& c : row) {
                c.render();
            }
        }
    }

    // called from engine
    inline auto clear_non_static_entries() -> void {
        for (auto& row : cells) {
            for (cell& c : row) {
                c.clear_non_static_entries();
            }
        }
    }

    // called from engine
    inline auto add(object* o) -> void {
        o->overlaps_cells =
            for_each_cell_object_is_in(o, [o](cell& c) { c.add(o); });
    }

    inline auto add_static(object* o) -> void {
        o->overlaps_cells =
            for_each_cell_object_is_in(o, [o](cell& c) { c.add_static(o); });
    }

    inline auto remove_static(object* o) -> void {
        for_each_cell_object_is_in(o, [o](cell& c) { c.remove_static(o); });
    }

    inline auto print() const -> void {
        for (auto const& row : cells) {
            for (cell const& c : row) {
                printf(" %04u/%04u ", c.objects_count(),
                       c.static_objects_count());
            }
            printf("\n");
        }
        printf("------------------------\n");
    }

    inline auto debug_render_grid() const -> void {
        float constexpr gw = grid_cell_size * grid_columns;
        float constexpr gh = grid_cell_size * grid_rows;
        for (float z = -gh / 2; z <= gh / 2; z += grid_cell_size) {
            debug_render_wcs_line({-gw / 2, 0, z}, {gw / 2, 0, z}, {0, 1, 0, 1},
                                  false);
        }
        for (float x = -gw / 2; x <= gw / 2; x += grid_cell_size) {
            debug_render_wcs_line({x, 0, -gh / 2}, {x, 0, gh / 2}, {0, 1, 0, 1},
                                  false);
        }
    }

  private:
    static inline auto clamp(int32_t const i, uint32_t const max) -> uint32_t {
        if (i < 0) {
            return 0;
        }
        if (uint32_t(i) > max) {
            return max;
        }
        return uint32_t(i);
    }

    // @return true if object overlaps cells
    inline auto for_each_cell_object_is_in(object* o, auto&& func) -> bool {
        float constexpr gw = grid_cell_size * grid_columns;
        float constexpr gh = grid_cell_size * grid_rows;

        float const r = o->bounding_radius;

        // calculate min max x and z in cell array
        float const xl = gw / 2 + o->position.x - r;
        float const xr = gw / 2 + o->position.x + r;
        float const zt = gh / 2 + o->position.z - r;
        float const zb = gh / 2 + o->position.z + r;

        uint32_t const xil =
            clamp(int32_t(xl / grid_cell_size), grid_columns - 1);
        uint32_t const xir =
            clamp(int32_t(xr / grid_cell_size), grid_columns - 1);
        uint32_t const zit = clamp(int32_t(zt / grid_cell_size), grid_rows - 1);
        uint32_t const zib = clamp(int32_t(zb / grid_cell_size), grid_rows - 1);

        // add to cells
        for (uint32_t z = zit; z <= zib; ++z) {
            for (uint32_t x = xil; x <= xir; ++x) {
                cell& c = cells[z][x];
                func(c);
            }
        }

        return xil != xir || zit != zib;
    }
} static grid{};

} // namespace glos
