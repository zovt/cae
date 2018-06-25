#pragma once

#include <iostream>
#include <string>

#include "index.hh"

#include "../../unit.hh"
#include "../primitives.hh"

namespace graphics { namespace opengl { namespace shaders {

template <typename Inputs, typename Outputs>
struct ShaderCommon {
	using inputs = Inputs;
	using outputs = Outputs;

	GLuint shader;

	// FIXME: Make this return a Result
	ShaderCommon(std::string const& source, GLenum shader_type) {
		this->shader = glCreateShader(shader_type);
		auto data = source.c_str();
		glShaderSource(this->shader, 1, &data, nullptr);
		glCompileShader(this->shader);
		GLint status;
		glGetShaderiv(this->shader, GL_COMPILE_STATUS, &status);
		if (status != GL_TRUE) {
			GLsizei log_len;
			glGetShaderiv(this->shader, GL_INFO_LOG_LENGTH, &log_len);
			std::cerr << log_len << std::endl;
			std::string output{};
			output.reserve(log_len + 1);
			glGetShaderInfoLog(this->shader, log_len, nullptr, output.data());
			std::cerr << "SHADER ERROR" << std::endl;
			std::cerr << output << std::endl;
		}
	}

	ShaderCommon(ShaderCommon const& other) = delete;
	ShaderCommon& operator=(ShaderCommon const& other) = delete;

	ShaderCommon(ShaderCommon&& other) {
		this->shader = other.shader;
		other.shader = 0;
	}

	ShaderCommon& operator=(ShaderCommon&& other) {
		this->shader = other.shader;
		other.shader = 0;
	}

	~ShaderCommon() {
		glDeleteShader(this->shader);
	}
};

template <typename Inputs>
struct VertShader : public ShaderCommon<Inputs, Unit> {
	VertShader(std::string const& source) : ShaderCommon<Inputs, Unit>(source, GL_VERTEX_SHADER) {}
};

struct DefaultFragOutputs {
	static void bind_frag_data_locations(GLuint program);
};

template <typename Outputs = DefaultFragOutputs>
struct FragShader : public ShaderCommon<Unit, Outputs> {
	FragShader(std::string const& source) : ShaderCommon<Unit, Outputs>(source, GL_FRAGMENT_SHADER) {}
};

struct XYZVertInputs {
	using vertex_data = graphics::primitives::XYZVert;
	GLint attr_pos;

	XYZVertInputs();
	XYZVertInputs(GLuint program);
	void activate() const;
};

struct XYZRGBVertInputs {
	using vertex_data = graphics::primitives::XYZRGBVert;
	GLint attr_pos;
	GLint attr_color;

	XYZRGBVertInputs();
	XYZRGBVertInputs(GLuint program);
	void activate() const;
};

template <typename VertInputs, typename FragOutputs = DefaultFragOutputs>
struct Program {
	GLuint program;
	VertInputs vert_inputs;

	Program(VertShader<VertInputs> const& vert_shdr, FragShader<FragOutputs> const& frag_shdr)
	: program(glCreateProgram())
	{
		glAttachShader(this->program, vert_shdr.shader);
		glAttachShader(this->program, frag_shdr.shader);
		FragOutputs::bind_frag_data_locations(this->program);

		glLinkProgram(this->program);

		this->vert_inputs = VertInputs(this->program);
	}

	void activate() const {
		glUseProgram(this->program);
	}

	void activate_vert_attr_arrays() const {
		this->vert_inputs.activate();
	}
};

} } }
