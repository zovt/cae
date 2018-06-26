#include "uniforms.hh"

#include <glm/gtc/matrix_transform.hpp>

#include "index.hh"

using namespace graphics::opengl::uniforms;

GLint const uni_proj = 0;
GLint const uni_world = 1;
GLint const uni_transform = 2;

GlobalDrawingUniforms::GlobalDrawingUniforms(float window_w, float window_h)
: proj(glm::ortho(0.0f, window_w, window_h, 0.0f, -1.0f, 1.0f)), world(1.0f) {}

void GlobalDrawingUniforms::regen_proj(int window_w, int window_h) {
	this->proj = glm::ortho(0, window_w, window_h, 0);
}

void GlobalDrawingUniforms::activate() const {
	glUniformMatrix4fv(uni_proj, 1, GL_FALSE, &this->proj[0][0]);
	glUniformMatrix4fv(uni_world, 1, GL_FALSE, &this->world[0][0]);
}

void TransformUniform::activate() const {
	glUniformMatrix4fv(uni_transform, 1, GL_FALSE, &this->transform[0][0]);
}

