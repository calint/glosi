#pragma once
// reviewed: 2024-01-04
// reviewed: 2024-01-06
// reviewed: 2024-01-10
// reviewed: 2024-01-16
// reviewed: 2024-07-08

namespace glos {

class texture final {
public:
  GLuint id = 0;
  size_t size_B = 0;

  inline auto load(std::string const &path) -> void {
    // load texture
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    printf(" * loading texture %u from '%s'\n", id, path.c_str());
    SDL_Surface *surface = IMG_Load(path.c_str());
    if (!surface) {
      throw exception{std::format("cannot load image from '{}'", path)};
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, GLsizei(surface->w),
                 GLsizei(surface->h), 0, GL_RGB, GL_UNSIGNED_BYTE,
                 surface->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    size_B = size_t(surface->w * surface->h) * sizeof(uint32_t);
    SDL_FreeSurface(surface);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
};

class textures final {
public:
  inline auto init() -> void {}

  inline auto free() -> void {
    for (auto const &pair : store) {
      glDeleteTextures(1, &pair.second.id);
      metrics.buffered_texture_data -= pair.second.size_B;
    }
    store.clear();
  };

  inline auto find_id_or_load(std::string const &path) -> GLuint {
    auto it = store.find(path);
    if (it != store.end()) {
      return it->second.id;
    }

    texture tex{};
    tex.load(path);
    metrics.buffered_texture_data += tex.size_B;
    // get id before moving tex
    GLuint const id = tex.id;
    store.insert({path, std::move(tex)});
    return id;
  }

private:
  std::unordered_map<std::string, texture> store{};
};

static textures textures{};
} // namespace glos