#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

#include <SDL2/SDL_timer.h>
#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace glos {

class metrics final {
  public:
    struct fps final {
        uint32_t calculation_interval_ms = 1000;
        uint64_t time_at_start_of_interval_ms = 0;
        uint32_t frame_count = 0;
        uint32_t average_during_last_interval = 0;
    } fps{};

    uint64_t timer_tick_at_start_of_frame = 0;
    uint64_t ms = 0;
    float dt = 0;
    size_t buffered_vertex_data = 0;
    size_t buffered_texture_data = 0;
    uint32_t allocated_objects = 0;
    uint32_t allocated_globs = 0;
    uint32_t rendered_objects = 0;
    uint32_t rendered_globs = 0;
    uint32_t rendered_triangles = 0;
    uint64_t update_begin_tick = 0;
    float update_pass_ms = 0;
    uint64_t render_begin_tick = 0;
    float render_pass_ms = 0;
    float net_ms = 0;
    bool enable_print = true;

    inline auto init() -> void {}

    inline auto free() -> void {}

    inline auto begin() -> void {
        timer_tick_at_start_of_frame = SDL_GetPerformanceCounter();
        fps.time_at_start_of_interval_ms = SDL_GetTicks64();
    }

    inline auto print_headers(FILE* f) const -> void {
        if (!enable_print) {
            return;
        }

        fprintf(f, " %7s  %7s  %5s  %7s  %7s  %7s  %6s  %6s  %6s  %9s\n", "ms",
                "dt_ms", "fps", "drw_ms", "upd_ms", "net_ms", "nobj", "drw_o",
                "drw_g", "drw_t");
    }

    inline auto print(FILE* f) const -> void {
        if (!enable_print) {
            return;
        }

        fprintf(f,
                " %07lu  %7.4f  %05u  %7.4f  %7.4f  %7.4f  %06u  %06u  %06u  "
                "%09u\n",
                ms, double(dt) * 1000, fps.average_during_last_interval,
                double(render_pass_ms), double(update_pass_ms), double(net_ms),
                allocated_objects, rendered_objects, rendered_globs,
                rendered_triangles);
    }

    inline auto update_begin() -> void {
        update_begin_tick = SDL_GetPerformanceCounter();
    }

    inline auto update_end() -> void {
        uint64_t const tick = SDL_GetPerformanceCounter();
        update_pass_ms = float(tick - update_begin_tick) * 1000 /
                         float(SDL_GetPerformanceFrequency());
    }

    inline auto render_begin() -> void {
        render_begin_tick = SDL_GetPerformanceCounter();
    }

    inline auto render_end() -> void {
        uint64_t const tick = SDL_GetPerformanceCounter();
        render_pass_ms = float(tick - render_begin_tick) * 1000 /
                         float(SDL_GetPerformanceFrequency());
    }

    inline auto at_frame_begin() -> void {
        ++fps.frame_count;
        rendered_objects = 0;
        rendered_globs = 0;
        rendered_triangles = 0;
        ms = SDL_GetTicks64();
        net_ms = 0;
    }

    inline auto at_frame_end(FILE* f) -> void {
        {
            uint64_t const t1 = SDL_GetPerformanceCounter();
            uint64_t const dt_ticks = t1 - timer_tick_at_start_of_frame;
            dt = float(dt_ticks) / float(SDL_GetPerformanceFrequency());
            timer_tick_at_start_of_frame = t1;

            if (dt > 0.1f) {
                dt = 0.1f; // minimum 10 fps
            } else if (dt == 0) {
                dt = 0.000001f; // maximum 100'000 fps
            }
        }

        uint64_t const t1 = SDL_GetTicks64();
        uint32_t const dt_interval =
            uint32_t(t1 - fps.time_at_start_of_interval_ms);

        if (dt_interval < fps.calculation_interval_ms) {
            return;
        }

        fps.average_during_last_interval = fps.frame_count * 1000 / dt_interval;
        fps.time_at_start_of_interval_ms = t1;
        fps.frame_count = 0;

        print(f);
    }
} static metrics{};

} // namespace glos
