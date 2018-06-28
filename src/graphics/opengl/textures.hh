#pragma once

#include "index.hh"
#include "drawing.hh"

namespace graphics { namespace opengl { namespace textures {

std::vector<float> const blank_tex_data{1.0f, 1.0f, 1.0f};

struct Texture {
	drawing::GLResource texture;

	void activate() const;
	void deactivate() const;

	Texture() : texture(glGenTextures, glDeleteTextures) {}

	template<typename Data>
	Texture(
		std::vector<Data> const& data,
		GLsizei width,
		GLsizei height,
		GLenum internal_format,
		GLenum format,
		GLenum type,
		GLenum wrapping,
		GLenum min_filtering,
		GLenum mag_filtering,
		bool enable_mips
	) : texture(glGenTextures, glDeleteTextures) {
		this->activate();

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapping);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapping);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filtering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filtering);

		glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, data.data());

		if (enable_mips) {
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		this->deactivate();
	}
};

} } };
