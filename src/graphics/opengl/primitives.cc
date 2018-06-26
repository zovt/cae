#include "primitives.hh"
#include "../primitives.hh"

using namespace graphics::opengl::primitives;
using namespace graphics::opengl::drawing;
using namespace graphics::opengl::shaders;
using namespace graphics::primitives;

GLint const attr_pos = 0;
GLint const attr_color = 1;
GLint const attr_uv = 2;

void graphics::opengl::primitives::pixel_attrib_setup() {
	glEnableVertexAttribArray(attr_pos);
	glVertexAttribPointer(
		attr_pos,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZVert),
		0
	);
}

void graphics::opengl::primitives::mc_pixel_attrib_setup() {
	glEnableVertexAttribArray(attr_pos);
	glEnableVertexAttribArray(attr_color);
	glVertexAttribPointer(
		attr_pos,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZRGBVert),
		0
	);
	glVertexAttribPointer(
		attr_color,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZRGBVert),
		(const void*)(2 * sizeof(float))
	);
}

