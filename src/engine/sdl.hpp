#pragma once
// reviewed: 2023-12-22
// reviewed: 2023-12-27
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

#include "exception.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_version.h>

namespace glos {

class sdl final {
  public:
    inline auto init() -> void {
        SDL_version compiled{};
        SDL_VERSION(&compiled)

        SDL_version linked{};
        SDL_GetVersion(&linked);

        printf("SDL version compiled %d.%d.%d\n", compiled.major,
               compiled.minor, compiled.patch);
        printf("SDL version linked %d.%d.%d\n\n", linked.major, linked.minor,
               linked.patch);

        if (SDL_Init(SDL_INIT_VIDEO)) {
            throw exception{
                std::format("cannot initiate sdl video: {}", SDL_GetError())};
        }
    }

    inline auto free() -> void { SDL_Quit(); }
} static sdl{};

} // namespace glos
