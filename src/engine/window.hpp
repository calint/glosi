#pragma once
// reviewed: 2023-12-22
// reviewed: 2023-12-27
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

#include "../application/configuration.hpp"
#include "exception.hpp"
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <utility>

namespace glos {

class window final {
  public:
    inline auto init() -> void {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_ES);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

        sdl_window = SDL_CreateWindow("glos", SDL_WINDOWPOS_CENTERED,
                                      SDL_WINDOWPOS_CENTERED, window_width,
                                      window_height, SDL_WINDOW_OPENGL);
        if (!sdl_window) {
            throw exception{
                std::format("cannot create window: {}", SDL_GetError())};
        }

        sdl_gl_context = SDL_GL_CreateContext(sdl_window);
        if (!sdl_gl_context) {
            throw exception{
                std::format("cannot create gl context: {}", SDL_GetError())};
        }

        sdl_renderer = SDL_CreateRenderer(
            sdl_window, -1,
            window_vsync
                ? (SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC)
                : (SDL_RENDERER_ACCELERATED));
        if (!sdl_renderer) {
            throw exception{
                std::format("cannot create renderer: {}", SDL_GetError())};
        }

        gl_print_context_profile_and_version();
    }

    inline auto free() -> void {
        SDL_DestroyRenderer(sdl_renderer);
        SDL_GL_DeleteContext(sdl_gl_context);
        SDL_DestroyWindow(sdl_window);
    }

    inline auto swap_buffers() const -> void { SDL_GL_SwapWindow(sdl_window); }

    inline auto get_width_and_height() const -> std::pair<int, int> {
        int w = 0;
        int h = 0;
        SDL_GetWindowSize(sdl_window, &w, &h);
        return {w, h};
    }

  private:
    SDL_Window* sdl_window = nullptr;
    SDL_Renderer* sdl_renderer = nullptr;
    SDL_GLContext sdl_gl_context = nullptr;

    static inline auto gl_print_context_profile_and_version() -> void {
        int value = 0;
        if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &value)) {
            throw exception{
                std::format("cannot get opengl attribute: {}", SDL_GetError())};
        }
        printf("SDL_GL_CONTEXT_PROFILE_MASK = ");
        switch (value) {
        case SDL_GL_CONTEXT_PROFILE_CORE:
            printf("SDL_GL_CONTEXT_PROFILE_CORE");
            break;
        case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
            printf("SDL_GL_CONTEXT_PROFILE_COMPATIBILITY");
            break;
        case SDL_GL_CONTEXT_PROFILE_ES:
            printf("SDL_GL_CONTEXT_PROFILE_ES");
            break;
        case 0:
            printf("any");
            break;
        default:
            perror("unknown option");
            printf("%s:%d: %d\n", __FILE__, __LINE__, value);
        }
        printf(" (%d)\n", value);

        if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value)) {
            throw exception{
                std::format("cannot get opengl attribute: {}", SDL_GetError())};
        }
        printf("SDL_GL_CONTEXT_MAJOR_VERSION = %d\n", value);

        if (SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value)) {
            throw exception{
                std::format("cannot get opengl attribute: {}", SDL_GetError())};
        }
        printf("SDL_GL_CONTEXT_MINOR_VERSION = %d\n", value);
    }
} static window{};

} // namespace glos
