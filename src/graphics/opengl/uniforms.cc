#include "uniforms.hh"

#include <glm/gtc/matrix_transform.hpp>

#include "index.hh"

using namespace graphics::opengl::uniforms;

GlobalDrawingUniforms::GlobalDrawingUniforms(float window_w, float window_h, std::string proj_name, std::string world_name)
:
	proj(glm::ortho(0.0f, window_w, window_h, 0.0f, -1.0f, 1.0f)),
	world(1.0f),
	proj_name(proj_name),
	world_name(world_name)
{}

void GlobalDrawingUniforms::regen_proj(float window_w, float window_h) {
	this->proj = glm::ortho(0.0f, window_w, window_h, 0.0f, 1.0f, -1.0f);
}

void GlobalDrawingUniforms::activate(GLuint program) const {
	glUniformMatrix4fv(glGetUniformLocation(program, this->proj_name.c_str()), 1, GL_FALSE, &this->proj[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(program, this->world_name.c_str()), 1, GL_FALSE, &this->world[0][0]);
}

void TransformUniform::activate(GLuint program) const {
	glUniformMatrix4fv(glGetUniformLocation(program, this->name.c_str()), 1, GL_FALSE, &this->transform[0][0]);
}

void TextureUniform::activate(GLuint program) const {
	glUniform1i(glGetUniformLocation(program, this->name.c_str()), this->texture_unit - GL_TEXTURE0);
	glActiveTexture(this->texture_unit);
	this->texture.activate();
}

void TextureUniformGroup::activate(GLuint program) const {
	for (size_t i = 0; i < this->textures.size(); i++) {
		auto const& texture = this->textures[i];
		glUniform1i(glGetUniformLocation(program, texture.name.c_str()), (this->texture_unit_offset + i) - GL_TEXTURE0);
		glActiveTexture(this->texture_unit_offset + i);
		glBindTexture(GL_TEXTURE_2D, texture.texture.texture.id);
	}
}

void BufferTextureUniform::activate(GLuint program) const {
	glUniform1i(glGetUniformLocation(program, this->name.c_str()), this->texture_unit - GL_TEXTURE0);
	glActiveTexture(this->texture_unit);
	glBindTexture(GL_TEXTURE_BUFFER, this->texture.texture.id);
}

void BufferTextureUniformGroup::activate(GLuint program) const {
	for (size_t i = 0; i < this->buffer_textures.size(); i++) {
		auto const& texture = this->buffer_textures[i];
		glUniform1i(glGetUniformLocation(program, texture.name.c_str()), (this->texture_unit_offset + i) - GL_TEXTURE0);
		glActiveTexture(this->texture_unit_offset + i);
		glBindTexture(GL_TEXTURE_BUFFER, texture.texture.texture.id);
	}
}

#define VEC_UNIFORM_IMPL(GlType, Size, glUniformv)\
template<>\
void VecUniform<GlType, Size>::activate(GLuint program) const {\
	glUniformv(\
		glGetUniformLocation(program, name.c_str()),\
		1,\
		data.data()\
	);\
}

VEC_UNIFORM_IMPL(GLuint, 3, glUniform3uiv)


