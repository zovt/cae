#pragma once

#include "index.hh"
#include "drawing.hh"

namespace graphics { namespace opengl { namespace textures {

std::vector<float> const blank_tex_data{1.0f, 1.0f, 1.0f};

struct Texture {
	drawing::GLResource texture;

	void activate() const;
	void deactivate() const;

	template<typename Data>
	Texture(
		std::vector<Data> const& data,
		GLsizei width,
		GLsizei height,
		GLenum format,
		GLenum type,
		GLenum wrapping,
		GLenum min_filtering,
		GLenum mag_filtering
	) : texture(glGenTextures, glDeleteTextures) {
		this->activate();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filtering);

		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, data.data());

		glGenerateMipmap(GL_TEXTURE_2D);
		this->deactivate();
	}
};

} } };
