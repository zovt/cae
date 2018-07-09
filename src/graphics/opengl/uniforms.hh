#pragma once

#include <tuple>
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <string>
#include <functional>

#include "index.hh"
#include "drawing.hh"
#include "textures.hh"

namespace graphics { namespace opengl { namespace uniforms {

struct GlobalDrawingUniforms {
	glm::mat4 proj;
	glm::mat4 world;
	std::string proj_name;
	std::string world_name;

	GlobalDrawingUniforms(float window_w, float window_h, std::string proj_name, std::string world_name);

	void activate(GLuint program) const;
	void regen_proj(float window_w, float window_h);
};

struct TransformUniform {
	glm::mat4 transform;
	std::string name;

	void activate(GLuint program) const;
};

struct BufferTextureUniform {
	drawing::VBO vbo;
	textures::Texture texture;
	std::string name;
	GLenum texture_unit;

	template<typename Data>
	BufferTextureUniform(const std::vector<Data>& data, GLenum internal_format, std::string name, GLenum texture_unit)
	: vbo(data), texture(), name(name), texture_unit(texture_unit) {
		glBindTexture(GL_TEXTURE_BUFFER, this->texture.texture.id);
		glTexBuffer(GL_TEXTURE_BUFFER, internal_format, this->vbo.vbo.id);
	}

	void activate(GLuint program) const;
};

struct BufferTextureUniformGroup {
	std::vector<BufferTextureUniform> buffer_textures;
	GLenum texture_unit_offset;

	void activate(GLuint program) const;
};


struct TextureUniform {
	textures::Texture& texture;
	std::string name;
	GLenum texture_unit;

	void activate(GLuint program) const;
};

struct TextureUniformGroup {
	std::vector<TextureUniform> textures;
	GLenum texture_unit_offset;

	void activate(GLuint program) const;
};

template <typename Item, typename GLType>
struct SimpleUniform {
	Item item;
	std::string name;
	typedef void(*uniform_fn_ptr)(GLint, GLType);
	uniform_fn_ptr fn;

	void activate(GLuint program) const {
		(*this->fn)(glGetUniformLocation(program, this->name.c_str()), this->item);
	}
};

template <typename GlType, size_t Size>
struct VecUniform {
	std::array<GlType, Size> data;
	std::string name;

	void activate(GLuint program) const;
};

template <typename HUniform, typename... RUniforms>
struct UniformGroup {
	HUniform uni;
	UniformGroup<RUniforms...> rest;

	UniformGroup(HUniform uni, RUniforms... rest) : uni(uni), rest(rest...) {}

	void activate(GLuint program) const {
		this->uni.activate(program);
		this->rest.activate(program);
	}
};

template <typename HUniform, typename... RUniforms>
struct UniformGroup<std::reference_wrapper<HUniform>, RUniforms...> {
	std::reference_wrapper<HUniform> uni_ref;
	UniformGroup<RUniforms...> rest;

	UniformGroup(std::reference_wrapper<HUniform> uni_ref, RUniforms... rest)
	: uni_ref(uni_ref), rest(rest...) {}

	void activate(GLuint program) const {
		this->uni_ref.get().activate(program);
		this->rest.activate(program);
	}
};

template <typename HUniform>
struct UniformGroup<HUniform> {
	HUniform uni;

	UniformGroup(HUniform uni) : uni(uni) {}

	void activate(GLuint program) const {
		this->uni.activate(program);
	}
};

template <typename HUniform>
struct UniformGroup<std::reference_wrapper<HUniform>> {
	std::reference_wrapper<HUniform> uni_ref;

	UniformGroup(std::reference_wrapper<HUniform> uni_ref) : uni_ref(uni_ref) {}

	void activate(GLuint program) const {
		this->uni_ref.get().activate(program);
	}
};


} } }
