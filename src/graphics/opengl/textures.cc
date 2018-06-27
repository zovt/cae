#include "textures.hh"

using namespace graphics::opengl::textures;

void Texture::activate() const {
	glBindTexture(GL_TEXTURE_2D, this->texture.id);
}

void Texture::deactivate() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}
