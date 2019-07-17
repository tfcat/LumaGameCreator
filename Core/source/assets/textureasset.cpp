#include "textureasset.h"
#include "filesystem.h"


TextureAsset::TextureAsset(std::string path): path(path) {
  std::string str = FileSystem::read_file(path, true);

  SDL_RWops* rw = SDL_RWFromMem(&str[0], static_cast<int>(str.size()*sizeof(char)));
  SDL_Surface* surface = IMG_Load_RW(rw, false);

  size.x = static_cast<unsigned int>(surface->w);
  size.y = static_cast<unsigned int>(surface->h);

  glGenTextures(1, &texture_id);

  glBindTexture(GL_TEXTURE_2D, texture_id);

  GLuint mode = GL_RGB;
  if(surface->format->BytesPerPixel == 4) mode = GL_RGBA;
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, mode, GL_UNSIGNED_BYTE, surface->pixels);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  SDL_FreeSurface(surface);
}

TextureAsset::~TextureAsset() {
  glDeleteTextures(1, &texture_id);
}

GLuint TextureAsset::get_texture_id() {
  return texture_id;
}

Vector2<unsigned int> TextureAsset::get_size() {
  return size;
}
