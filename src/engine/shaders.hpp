#pragma once
// reviewed: 2023-12-24
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-07-08

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
  static inline char const *vertex_shader_source = R"(
#version 330 core
uniform mat4 umtx_mw;  // model-to-world-matrix
uniform mat4 umtx_wvp; // world-to-view-to-projection
layout(location = 0) in vec4 apos;
layout(location = 1) in vec4 argba;
layout(location = 2) in vec3 anorm;
layout(location = 3) in vec2 atex;
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

  static inline char const *fragment_shader_source = R"(
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
  // vertex attributes layout in shaders
  static GLint constexpr apos = 0;
  static GLint constexpr argba = 1;
  static GLint constexpr anorm = 2;
  static GLint constexpr atex = 3;
  // uniform matrixes
  static GLint constexpr umtx_mw = 0;  // model -> world matrix
  static GLint constexpr umtx_wvp = 1; // world -> view -> projection matrix
  // uniform textures
  static GLint constexpr utex = 2; // texture mapper
  // uniform ambient light
  static GLint constexpr ulht = 3; // light vector

  inline auto init() -> void {
    puts("");
    gl_print_string("GL_VENDOR", GL_VENDOR);
    gl_print_string("GL_RENDERER", GL_RENDERER);
    gl_print_string("GL_VERSION", GL_VERSION);
    gl_print_string("GL_SHADING_LANGUAGE_VERSION", GL_SHADING_LANGUAGE_VERSION);
    puts("");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    uint32_t const default_program_ix =
        load_program_from_source(vertex_shader_source, fragment_shader_source);

    programs.at(default_program_ix).use();

    GLuint const def_prog_id = programs.at(default_program_ix).id;

    printf("shader uniforms locations:\n");
    printf(":-%10s-:-%7s-:\n", "----------", "-------");
    printf(": %10s : %-7d :\n", "umtx_mw",
           glGetUniformLocation(def_prog_id, "umtx_mw"));
    printf(": %10s : %-7d :\n", "umtx_wvp",
           glGetUniformLocation(def_prog_id, "umtx_wvp"));
    printf(": %10s : %-7d :\n", "utex",
           glGetUniformLocation(def_prog_id, "utex"));
    printf(": %10s : %-7d :\n", "ulht",
           glGetUniformLocation(def_prog_id, "ulht"));
    printf(":-%10s-:-%7s-:\n", "----------", "-------");

    puts("");
    printf("shader attributes locations:\n");
    printf(":-%10s-:-%7s-:\n", "----------", "-------");
    printf(": %10s : %-7d :\n", "apos",
           glGetAttribLocation(def_prog_id, "apos"));
    printf(": %10s : %-7d :\n", "argba",
           glGetAttribLocation(def_prog_id, "argba"));
    printf(": %10s : %-7d :\n", "anorm",
           glGetAttribLocation(def_prog_id, "anorm"));
    printf(": %10s : %-7d :\n", "atex",
           glGetAttribLocation(def_prog_id, "atex"));
    printf(":-%10s-:-%7s-:\n", "----------", "-------");
    puts("");
  }

  inline auto free() -> void {
    for (program const &p : programs) {
      glDeleteProgram(p.id);
    }
    programs.clear();
  }

  inline auto load_program_from_source(char const *vert_src,
                                       char const *frag_src) -> uint32_t {
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

  inline auto use_program(uint32_t const ix) const -> void {
    programs.at(ix).use();
  }

private:
  static inline auto compile(GLenum const shader_type,
                             char const *src) -> GLuint {
    GLuint const shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, &src, nullptr);
    glCompileShader(shader_id);
    GLint ok = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
      GLchar msg[1024];
      glGetShaderInfoLog(shader_id, sizeof(msg), nullptr, msg);
      throw exception{std::format("compile error in {} shader:\n{}",
                                  shader_name_for_type(shader_type), msg)};
    }
    return shader_id;
  }

public:
  //
  // static functions
  //
  static inline auto gl_print_string(char const *name,
                                     GLenum const gl_str) -> void {
    char const *str = (char const *)glGetString(gl_str);
    printf("%s = %s\n", name, str);
  }

  static inline auto
  shader_name_for_type(GLenum const shader_type) -> char const * {
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