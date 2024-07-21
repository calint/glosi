#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-01-16
// reviewed: 2024-07-08

namespace glos {

class glob final {
  // a range of triangles rendered with a specified material
  class range final {
  public:
    // index in vertex buffer where triangle data starts
    uint32_t vertex_begin = 0;
    // number of triangle vertices in the range
    uint32_t vertex_count = 0;
    // index in 'materials'
    uint32_t material_ix = 0;
  };

  GLuint vertex_array_id = 0;
  GLuint vertex_buffer_id = 0;
  std::vector<range> ranges{};
  size_t size_B = 0;

public:
  std::string name{};
  float bounding_radius = 0;
  // planes
  std::vector<glm::vec4> planes_points{}; // x, y, z, 1
  std::vector<glm::vec3> planes_normals{};
  // note: each normal has a point at the same index; however there might be
  //       additional points in 'planes_points' to better define a volume and
  //       points it contains

  inline glob(char const *obj_path, char const *bounding_planes_path) {
    load_object(obj_path);
    if (bounding_planes_path) {
      load_planes(bounding_planes_path);
    }
  }

  inline auto render(glm::mat4 const &Mmw) const -> void {
    glUniformMatrix4fv(shaders::umtx_mw, 1, GL_FALSE, glm::value_ptr(Mmw));
    glBindVertexArray(vertex_array_id);
    for (range const &rng : ranges) {
      material const &mtl = materials.at(rng.material_ix);
      glActiveTexture(GL_TEXTURE0);
      if (mtl.texture_id) {
        glUniform1i(shaders::utex, 0);
        glBindTexture(GL_TEXTURE_2D, mtl.texture_id);
      } else {
        glBindTexture(GL_TEXTURE_2D, 0);
      }
      glDrawArrays(GL_TRIANGLES, GLint(rng.vertex_begin),
                   GLint(rng.vertex_count));
      metrics.rendered_triangles += rng.vertex_count / 3;
      if (mtl.texture_id) {
        glBindTexture(GL_TEXTURE_2D, 0);
      }
    }
    glBindVertexArray(0); //? can be removed?
    ++metrics.rendered_globs;
  }

  inline auto free() const -> void {
    glDeleteBuffers(1, &vertex_buffer_id);
    glDeleteVertexArrays(1, &vertex_array_id);
    metrics.buffered_vertex_data -= size_B;
    --metrics.allocated_globs;
  }

private:
  // loads definition of object from and 'obj' file
  inline auto load_object(std::filesystem::path path) -> void {
    printf(" * loading glob from '%s'\n", path.string().c_str());

    std::ifstream file{path};
    if (!file) {
      throw exception{
          std::format("cannot open file '{}'", path.string().c_str())};
    }

    std::vector<float> vertices{};
    std::vector<glm::vec3> positions{};
    std::vector<glm::vec3> normals{};
    std::vector<glm::vec2> texture_uv{};

    std::string mtl_path{};
    uint32_t current_material_ix = 0;
    uint32_t vertex_ix = 0;
    uint32_t vertex_ix_prv = 0;

    std::string line{};
    while (std::getline(file, line)) {
      std::istringstream line_stream{line};
      std::string token{};
      line_stream >> token;

      if (token == "mtllib") {
        line_stream >> token;
        mtl_path = (path.parent_path() / token).string();
        materials.load(mtl_path.c_str());
        continue;

      } else if (token == "o") {
        line_stream >> name;
        std::cout << "   " << name << "\n";
        continue;

      } else if (token == "usemtl") {
        line_stream >> token;
        current_material_ix =
            materials.find_material_ix_or_throw(mtl_path, token);
        if (vertex_ix_prv != vertex_ix) {
          // is not the first 'usemtl' directive
          ranges.emplace_back(vertex_ix_prv, vertex_ix - vertex_ix_prv,
                              current_material_ix);
          vertex_ix_prv = vertex_ix;
        }
        continue;

      } else if (token == "v") {
        glm::vec3 position{};
        line_stream >> position.x >> position.y >> position.z;
        positions.push_back(position);
        float const r = glm::length(position);
        if (r > bounding_radius) {
          bounding_radius = r;
        }
        continue;

      } else if (token == "vt") {
        glm::vec2 tex_uv{};
        line_stream >> tex_uv.x >> tex_uv.y;
        texture_uv.push_back(tex_uv);
        continue;

      } else if (token == "vn") {
        glm::vec3 normal{};
        line_stream >> normal.x >> normal.y >> normal.z;
        normals.push_back(normal);

      } else if (token == "f") {
        material const &current_material = materials.at(current_material_ix);
        for (uint32_t i = 0; i < 3; ++i) {
          uint32_t ix1 = 0;
          uint32_t ix2 = 0;
          uint32_t ix3 = 0;
          char slash = '\0';

          line_stream >> ix1 >> slash >> ix2 >> slash >> ix3;

          glm::vec3 const &position = positions.at(ix1 - 1);
          glm::vec2 const &texture = texture_uv.at(ix2 - 1);
          glm::vec3 const &normal = normals.at(ix3 - 1);

          // add to buffer

          // position
          vertices.push_back(position.x);
          vertices.push_back(position.y);
          vertices.push_back(position.z);
          // color
          vertices.push_back(current_material.Kd.r); // color red
          vertices.push_back(current_material.Kd.g); // green
          vertices.push_back(current_material.Kd.b); // blue
          vertices.push_back(current_material.d);    // opacity
          // normal
          vertices.push_back(normal.x);
          vertices.push_back(normal.y);
          vertices.push_back(normal.z);
          // texture
          vertices.push_back(texture.x);
          vertices.push_back(texture.y);

          ++vertex_ix;
        }
        continue;
      }
    }
    // add the last material range
    ranges.emplace_back(vertex_ix_prv, vertex_ix - vertex_ix_prv,
                        current_material_ix);

    uint32_t triangles_count = 0;
    for (range const &rng : ranges) {
      triangles_count += rng.vertex_count / 3;
    }

    printf("   %zu range%c   %zu vertices   %u triangles   %zu B   radius: "
           "%0.2f\n",
           ranges.size(), ranges.size() == 1 ? ' ' : 's',
           vertices.size() * sizeof(float) / sizeof(vertex), triangles_count,
           vertices.size() * sizeof(float), bounding_radius);

    ++metrics.allocated_globs;

    //
    // upload to opengl
    //
    glGenVertexArrays(1, &vertex_array_id);
    glBindVertexArray(vertex_array_id);

    glGenBuffers(1, &vertex_buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_id);
    glBufferData(GL_ARRAY_BUFFER, GLsizeiptr(vertices.size() * sizeof(float)),
                 vertices.data(), GL_STATIC_DRAW);

    // describe the data format
    glEnableVertexAttribArray(shaders::apos);
    glVertexAttribPointer(shaders::apos, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (GLvoid *)offsetof(vertex, position));
    // note: 'vertex' defined in 'shaders.hpp'

    glEnableVertexAttribArray(shaders::argba);
    glVertexAttribPointer(shaders::argba, 4, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (GLvoid *)offsetof(vertex, color));

    glEnableVertexAttribArray(shaders::anorm);
    glVertexAttribPointer(shaders::anorm, 3, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (GLvoid *)offsetof(vertex, normal));

    glEnableVertexAttribArray(shaders::atex);
    glVertexAttribPointer(shaders::atex, 2, GL_FLOAT, GL_FALSE, sizeof(vertex),
                          (GLvoid *)offsetof(vertex, texture));

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    size_B = vertices.size() * sizeof(float);

    metrics.buffered_vertex_data += size_B;
  }

  inline auto load_planes(std::filesystem::path path) -> void {
    // load from blender exported 'obj' file
    printf("   * loading planes from '%s'\n", path.string().c_str());
    std::ifstream file{path};
    if (!file) {
      throw exception{
          std::format("cannot open file '{}'", path.string().c_str())};
    }

    std::vector<glm::vec4> points{};
    std::vector<glm::vec3> normals{};

    std::string line{};
    while (std::getline(file, line)) {
      std::istringstream line_stream{line};
      std::string token{};
      line_stream >> token;

      if (token == "v") {
        glm::vec4 point{};
        line_stream >> point.x >> point.y >> point.z;
        point.w = 1.0f;
        points.push_back(point);

      } else if (token == "vn") {
        glm::vec3 normal{};
        line_stream >> normal.x >> normal.y >> normal.z;
        normals.push_back(normal);

      } else if (token == "f") {
        uint32_t ix1 = 0;
        uint32_t ix2 = 0;
        uint32_t ix3 = 0;
        char slash = '\0';
        line_stream >> ix1 >> slash;
        if (line_stream.peek() == '/') {
          // handle the case where there is no texture
          ix2 = 0;
          line_stream.ignore();
        } else {
          line_stream >> ix2 >> slash;
        }
        line_stream >> ix3;

        glm::vec4 const &point = points.at(ix1 - 1);
        glm::vec3 const &normal = normals.at(ix3 - 1);

        planes_points.emplace_back(point);
        planes_normals.emplace_back(normal);

        continue;
      }
      // unknown token
    }
    // add points not connected to normals to 'planes_points'
    for (glm::vec4 const &pt : points) {
      if (std::ranges::find(planes_points, pt) == planes_points.cend()) {
        planes_points.emplace_back(pt);
      }
    }
    printf("     %zu planes  %zu points\n", planes_normals.size(),
           planes_points.size());
  }
};

class globs final {
  std::vector<glob> store{};

public:
  inline auto init() -> void {}

  inline auto free() -> void {
    for (glob const &g : store) {
      g.free();
    }
  }

  inline auto load(char const *obj_path,
                   char const *bounding_planes_path) -> uint32_t {

    store.emplace_back(obj_path, bounding_planes_path);
    return uint32_t(store.size() - 1);
  }

  inline auto at(uint32_t const ix) const -> glob const & {
    return store.at(ix);
  }
};

static globs globs{};
} // namespace glos