#include "shaders.hh"

#include <iostream>
#include "../primitives.hh"

using namespace graphics::opengl::shaders;
using namespace graphics::primitives;

ShaderCommon::ShaderCommon(std::string const& source, GLenum shader_type) {
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

ShaderCommon::ShaderCommon(ShaderCommon&& other) {
	this->shader = other.shader;
	other.shader = 0;
}

ShaderCommon& ShaderCommon::operator=(ShaderCommon&& other) {
	this->shader = other.shader;
	other.shader = 0;

	return *this;
}

ShaderCommon::~ShaderCommon() {
	glDeleteShader(this->shader);
}

Program::Program(VertShader const& vert_shdr, FragShader const& frag_shdr)
: program(glCreateProgram()) {
	glAttachShader(this->program, vert_shdr.shader);
	glAttachShader(this->program, frag_shdr.shader);
	glBindFragDataLocation(program, 0, "out_color");
	glLinkProgram(this->program);
}

void Program::activate() const {
	glUseProgram(this->program);
}

