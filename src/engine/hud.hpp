#pragma once
// reviewed: 2024-01-06
// reviewed: 2024-01-10

//
//! colors do not get converted correctly. color 'red' works though.
//

#include "exception.hpp"
#include "shaders.hpp"
#include <GLES3/gl3.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_ttf.h>
#include <cstdint>

namespace glos {

class hud final {
  public:
    uint32_t program_ix = 0;

    inline auto init() -> void {
        if (TTF_Init()) {
            throw exception{
                std::format("cannot initiate ttf: {}", TTF_GetError())};
        }

        GLfloat constexpr quad_vertices[] = {
            // position, texture coord
            -1.0f, 1.0f,  0.0f, 0.0f, // top left
            1.0f,  1.0f,  1.0f, 0.0f, // top right
            1.0f,  -1.0f, 1.0f, 1.0f, // bottom right
            -1.0f, -1.0f, 0.0f, 1.0f  // bottom left
        };

        GLuint constexpr quad_indices[] = {
            2, 1, 0, // first triangle
            3, 2, 0  // second triangle
        };

        // define the rectangle
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices,
                     GL_STATIC_DRAW);

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_indices),
                     quad_indices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              nullptr);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                              (GLvoid*)(2 * sizeof(float)));

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // define texture
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture_width, texture_height,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, 0);

        program_ix = shaders.load_program_from_source(vertex_shader_source,
                                                      fragment_shader_source);
    }

    inline auto free() -> void {
        if (font) {
            TTF_CloseFont(font);
        }
        TTF_Quit();
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
        glDeleteTextures(1, &texture);
    }

    inline auto load_font(char const* ttf_path, int const size) -> void {
        font = TTF_OpenFont(ttf_path, size);
        if (!font) {
            throw exception{std::format("cannot load font '{}': {}", ttf_path,
                                        TTF_GetError())};
        }
    }

    inline auto render() const -> void {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        glBindVertexArray(vao);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(0, 0); // sets uniform "utex" to texture unit 0
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glDepthMask(GL_TRUE);
        glEnable(GL_DEPTH_TEST);

        glDisable(GL_BLEND);
    }

    inline auto print(char const* text, SDL_Color const color, int const x,
                      const int y) const -> void {

        SDL_Surface* text_surface = TTF_RenderUTF8_Blended(font, text, color);

        if (!text_surface) {
            throw exception{
                std::format("cannot render text: {}", SDL_GetError())};
        }

        // printf("Surface Format: %s\n",
        //        SDL_GetPixelFormatName(text_surface->format->format));
        // ARGB8888

        SDL_Surface* converted_surface =
            SDL_ConvertSurfaceFormat(text_surface, SDL_PIXELFORMAT_RGBA8888, 0);

        if (!converted_surface) {
            throw exception{
                std::format("cannot convert surface: {}", SDL_GetError())};
        }

        // printf("Surface Format: %s\n",
        //        SDL_GetPixelFormatName(converted_surface->format->format));
        // RGBA8888

        glBindTexture(GL_TEXTURE_2D, texture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, GLint(x), GLint(y),
                        GLsizei(converted_surface->w),
                        GLsizei(converted_surface->h), GL_RGBA,
                        GL_UNSIGNED_BYTE, converted_surface->pixels);

        // uint32_t constexpr pixels[] = {0x00ff0000, 0x00ff0000, 0x00ff0000,
        //                                0x00ff0000};
        // glTexSubImage2D(GL_TEXTURE_2D, 0, GLint(x), GLint(y), 2, 2, GL_RGBA,
        //                 GL_UNSIGNED_BYTE, pixels);

        SDL_FreeSurface(text_surface);
        SDL_FreeSurface(converted_surface);
    }

  private:
    GLuint vbo = 0;
    GLuint vao = 0;
    GLuint ebo = 0;
    GLuint texture = 0;
    TTF_Font* font = nullptr;

    static GLsizei constexpr texture_width = 256;
    static GLsizei constexpr texture_height = 256;

    static inline char const* vertex_shader_source = R"(
#version 330 core
layout(location = 0) in vec2 apos;
layout(location = 1) in vec2 atex;
out vec2 vtex;
void main() {
    gl_Position = vec4(apos, 0, 1.0);
    vtex = atex;
}
)";

    static inline char const* fragment_shader_source = R"(
#version 330 core
uniform sampler2D utex;
in vec2 vtex;
out vec4 rgba;
void main() {
    rgba = texture(utex, vtex);
}
)";
} static hud{};

} // namespace glos
