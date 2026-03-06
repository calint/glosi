#pragma once
// reviewed: 2023-12-22
// reviewed: 2023-12-27
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

#include "exception.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_version.h>

namespace glos {

class sdl final {
  public:
    auto init() -> void {
        int compiled = SDL_VERSION;
        int linked = SDL_GetVersion();

        auto printv = [](int v) {
            printf("%d.%d.%d", SDL_VERSIONNUM_MAJOR(v), SDL_VERSIONNUM_MINOR(v),
                   SDL_VERSIONNUM_MICRO(v));
        };

        printf("SDL version compiled ");
        printv(compiled);
        printf("\n");
        printf("SDL version linked   ");
        printv(linked);
        printf("\n\n");

        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw exception{
                std::format("cannot initiate sdl video: {}", SDL_GetError())};
        }
    }

    auto free() -> void { SDL_Quit(); }
} static sdl{};

} // namespace glos
