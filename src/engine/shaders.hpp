#pragma once
// reviewed: 2023-12-24
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

#include "exception.hpp"
#include <GLES3/gl3.h>
#include <cstdio>
#include <glm/glm.hpp>
#include <vector>

namespace glos {

struct vertex final {
    glm::vec3 position{};
    glm::vec4 color{};
    glm::vec3 normal{};
    glm::vec2 texture{};
};

class shaders final {

    class program final {
      public:
        GLuint id = 0;

        inline auto use() const -> void { glUseProgram(id); }
    };

    // default shader source
    static inline char const* vertex_shader_source = R"(
#version 330 core
uniform mat4 umtx_mw;  // model-to-world-matrix
uniform mat4 umtx_wvp; // world-to-view-to-projection
in vec4 apos;
in vec4 argba;
in vec3 anorm;
in vec2 atex;
out vec4 vrgba;
out vec3 vnorm;
out vec2 vtex;
void main() {
  gl_Position = umtx_wvp * umtx_mw * apos;
  vrgba = argba;
  
  // normal transform for general case (e.g. non-uniform scaling)
  // note: https://paroj.github.io/gltut/Illumination/Tut09%20Normal%20Transformation.html
  // note: Polar Decomposition theorem
  vnorm = normalize(transpose(inverse(mat3(umtx_mw))) * anorm);
  
  // normal transform for uniform scaling case
  //vnorm = normalize(mat3(umtx_mw) * anorm);

  // normal transform for identity scale
  //vnorm = mat3(umtx_mw) * anorm;

  vtex = atex;
}
  )";

    static inline char const* fragment_shader_source = R"(
#version 330 core
uniform sampler2D utex;
uniform vec3 ulht; // ambient light vector
in vec4 vrgba;
in vec3 vnorm;
in vec2 vtex;
out vec4 rgba;
void main() {
  float diff = max(dot(vnorm, ulht), 0.5);
  rgba = vec4(vec3(texture2D(utex, vtex)) + diff * vec3(vrgba), vrgba.a);
}
)";

    std::vector<program> programs{};

  public:
    // vertex attributes layout in shader

    // vertex coordinate
    GLint apos = -1;

    // color
    GLint argba = -1;

    // normal
    GLint anorm = -1;

    // texture coordinate
    GLint atex = -1;

    // uniform attributes layout in shader

    // model -> world matrix
    GLint umtx_mw = -1;

    // world -> view -> projection matrix
    GLint umtx_wvp = -1;

    // texture mapper uniform
    GLint utex = -1;

    // ambient light vector
    GLint ulht = -1;

    inline auto init() -> void {
        puts("");
        gl_print_string("GL_VENDOR", GL_VENDOR);
        gl_print_string("GL_RENDERER", GL_RENDERER);
        gl_print_string("GL_VERSION", GL_VERSION);
        gl_print_string("GL_SHADING_LANGUAGE_VERSION",
                        GL_SHADING_LANGUAGE_VERSION);
        puts("");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        uint32_t const default_program_ix = load_program_from_source(
            vertex_shader_source, fragment_shader_source);

        use_program(default_program_ix);
    }

    inline auto print_current_shader_info() -> void {
        printf("shader uniforms locations:\n");
        printf(":-%10s-:-%4s-:\n", "----------", "----");
        printf(": %10s : %-4d :\n", "umtx_mw", umtx_mw);
        printf(": %10s : %-4d :\n", "umtx_wvp", umtx_wvp);
        printf(": %10s : %-4d :\n", "utex", utex);
        printf(": %10s : %-4d :\n", "ulht", ulht);
        printf(":-%10s-:-%4s-:\n", "----------", "----");

        printf("shader attributes locations:\n");
        printf(":-%10s-:-%4s-:\n", "----------", "----");
        printf(": %10s : %-4d :\n", "apos", apos);
        printf(": %10s : %-4d :\n", "argba", argba);
        printf(": %10s : %-4d :\n", "anorm", anorm);
        printf(": %10s : %-4d :\n", "atex", atex);
        printf(":-%10s-:-%4s-:\n", "----------", "----");
    }

    inline auto free() -> void {
        for (program const& p : programs) {
            glDeleteProgram(p.id);
        }
        programs.clear();
    }

    inline auto load_program_from_source(char const* vert_src,
                                         char const* frag_src) -> uint32_t {
        GLuint const program_id = glCreateProgram();
        GLuint const vertex_shader_id = compile(GL_VERTEX_SHADER, vert_src);
        GLuint const fragment_shader_id = compile(GL_FRAGMENT_SHADER, frag_src);
        glAttachShader(program_id, vertex_shader_id);
        glAttachShader(program_id, fragment_shader_id);
        glLinkProgram(program_id);
        // glValidateProgram(program_id);
        GLint ok = 0;
        glGetProgramiv(program_id, GL_LINK_STATUS, &ok);
        if (!ok) {
            GLchar msg[1024];
            glGetProgramInfoLog(program_id, sizeof(msg), nullptr, msg);
            throw exception{std::format("program linking error:\n{}", msg)};
        }

        glDeleteShader(vertex_shader_id);
        glDeleteShader(fragment_shader_id);

        programs.push_back({program_id});

        return uint32_t(programs.size() - 1);
    }

    inline auto programs_count() const -> size_t { return programs.size(); }

    inline auto use_program(uint32_t const ix) -> void {
        program const& prog = programs.at(ix);
        prog.use();
        umtx_mw = glGetUniformLocation(prog.id, "umtx_mw");
        umtx_wvp = glGetUniformLocation(prog.id, "umtx_wvp");
        utex = glGetUniformLocation(prog.id, "utex");
        ulht = glGetUniformLocation(prog.id, "ulht");
        apos = glGetAttribLocation(prog.id, "apos");
        argba = glGetAttribLocation(prog.id, "argba");
        anorm = glGetAttribLocation(prog.id, "anorm");
        atex = glGetAttribLocation(prog.id, "atex");
    }

  private:
    static inline auto compile(GLenum const shader_type, char const* src)
        -> GLuint {
        GLuint const shader_id = glCreateShader(shader_type);
        glShaderSource(shader_id, 1, &src, nullptr);
        glCompileShader(shader_id);
        GLint ok = 0;
        glGetShaderiv(shader_id, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            GLchar msg[1024];
            glGetShaderInfoLog(shader_id, sizeof(msg), nullptr, msg);
            throw exception{std::format("compile error in {} shader:\n{}",
                                        shader_name_for_type(shader_type),
                                        msg)};
        }
        return shader_id;
    }

  public:
    //
    // static functions
    //
    static inline auto gl_print_string(char const* name, GLenum const gl_str)
        -> void {
        GLubyte const* str = glGetString(gl_str);
        printf("%s = %s\n", name, str);
    }

    static inline auto shader_name_for_type(GLenum const shader_type)
        -> char const* {
        switch (shader_type) {
        case GL_VERTEX_SHADER:
            return "vertex";
        case GL_FRAGMENT_SHADER:
            return "fragment";
        default:
            return "unknown";
        }
    }
};

static shaders shaders{};
} // namespace glos
