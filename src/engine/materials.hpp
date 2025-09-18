#pragma once
// reviewed: 2023-12-22
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-01-16
// reviewed: 2024-07-08

#include "exception.hpp"
#include "textures.hpp"
#include <GLES3/gl3.h>
#include <fstream>
#include <glm/glm.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace glos {

class material final {
  public:
    std::string path{};
    std::string name{};
    float Ns = 0;
    glm::vec3 Ka{};
    glm::vec3 Kd{}; // color RGB
    glm::vec3 Ks{};
    glm::vec3 Ke{};
    float Ni = 0;
    float d = 0; // opacity
    uint32_t illum = 0;
    std::string map_Kd{};
    GLuint texture_id = 0;
};

class materials final {
    std::vector<material> store{};

  public:
    inline auto init() -> void {}

    inline auto free() -> void { store.clear(); }

    // sample of text to parse:

    // newmtl texture
    // Ns 96.078431
    // Ka 1.000000 1.000000 1.000000
    // Kd 0.640000 0.640000 0.640000
    // Ks 0.500000 0.500000 0.500000
    // Ke 0.000000 0.000000 0.000000
    // Ni 1.000000
    // d 1.000000
    // illum 2
    // map_Kd /home/c/w/glos/logo.jpeg

    inline auto load(char const* path) -> void {
        printf("   * loading materials from '%s'\n", path);

        std::ifstream file{path};
        if (!file) {
            throw exception{std::format("cannot open file '{}'", path)};
        }

        std::string line{};
        while (std::getline(file, line)) {
            std::istringstream line_stream{line};
            std::string token{};
            line_stream >> token;

            if (token == "newmtl") {
                material mtl{};
                mtl.path = std::string{path};
                line_stream >> mtl.name;
                store.push_back(mtl);
                printf("     %s\n", mtl.name.c_str());

            } else if (token == "Ns") {
                line_stream >> store.back().Ns;

            } else if (token == "Ka") {
                glm::vec3 v{};
                line_stream >> v.x >> v.y >> v.z;
                store.back().Ka = v;

            } else if (token == "Kd") {
                glm::vec3 v{};
                line_stream >> v.x >> v.y >> v.z;
                store.back().Kd = v;

            } else if (token == "Ks") {
                glm::vec3 v{};
                line_stream >> v.x >> v.y >> v.z;
                store.back().Ks = v;

            } else if (token == "Ke") {
                glm::vec3 v{};
                line_stream >> v.x >> v.y >> v.z;
                store.back().Ke = v;

            } else if (token == "Ni") {
                line_stream >> store.back().Ni;

            } else if (token == "d") {
                line_stream >> store.back().d;

            } else if (token == "illum") {
                line_stream >> store.back().illum;

            } else if (token == "map_Kd") {
                // texture path
                line_stream >> store.back().map_Kd;
                store.back().texture_id =
                    textures.find_id_or_load(store.back().map_Kd);
            }

            // unknown token
        }
    }

    inline auto find_material_ix_or_throw(std::string const& path,
                                          std::string const& name) const
        -> uint32_t {

        auto it =
            std::ranges::find_if(store, [&path, &name](material const& mtl) {
                return mtl.path == path && mtl.name == name;
            });

        if (it == store.cend()) {
            throw exception{std::format(
                "cannot find material: path '{}' name '{}'", path, name)};
        }

        return uint32_t(std::distance(store.cbegin(), it));
    }

    inline auto at(uint32_t ix) const -> material const& { return store[ix]; }
} static materials{};

} // namespace glos
