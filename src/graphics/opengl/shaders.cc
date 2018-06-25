#include "shaders.hh"
#include "../primitives.hh"

using namespace graphics::opengl::shaders;
using namespace graphics::primitives;

void DefaultFragOutputs::bind_frag_data_locations(GLuint program) {
	glBindFragDataLocation(program, 0, "out_color");
}

XYZVertInputs::XYZVertInputs() : attr_pos(-1) {}

XYZVertInputs::XYZVertInputs(GLuint program) {
	this->attr_pos = glGetAttribLocation(program, "position");
}

void XYZVertInputs::activate() const {
	glEnableVertexAttribArray(this->attr_pos);
	glVertexAttribPointer(this->attr_pos, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

XYZRGBVertInputs::XYZRGBVertInputs() : attr_pos(-1), attr_color(-1) {}

XYZRGBVertInputs::XYZRGBVertInputs(GLuint program) {
	this->attr_pos = glGetAttribLocation(program, "position");
	this->attr_color = glGetAttribLocation(program, "color");
}

void XYZRGBVertInputs::activate() const {
	glEnableVertexAttribArray(this->attr_pos);
	glEnableVertexAttribArray(this->attr_color);
	glVertexAttribPointer(
		this->attr_pos,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZRGBVert),
		0
	);
	glVertexAttribPointer(
		this->attr_color,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZRGBVert),
		(const void*)(3 * sizeof(float))
	);
}

