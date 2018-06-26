#pragma once

#include <string>

#include "index.hh"

#include "../../unit.hh"
#include "../primitives.hh"

namespace graphics { namespace opengl { namespace shaders {

struct ShaderCommon {
	GLuint shader;

	// FIXME: Make this return a Result
	ShaderCommon(std::string const& source, GLenum shader_type);

	ShaderCommon(ShaderCommon const& other) = delete;
	ShaderCommon& operator=(ShaderCommon const& other) = delete;

	ShaderCommon(ShaderCommon&& other);
	ShaderCommon& operator=(ShaderCommon&& other);

	~ShaderCommon();
};

struct VertShader : public ShaderCommon {
	VertShader(std::string const& source) : ShaderCommon(source, GL_VERTEX_SHADER) {}
};

struct FragShader : public ShaderCommon {
	FragShader(std::string const& source) : ShaderCommon(source, GL_FRAGMENT_SHADER) {}
};

struct Program {
	GLuint program;

	Program(VertShader const& vert_shdr, FragShader const& frag_shdr);

	void activate() const;
};

} } }
