#include "primitives.hh"
#include "../primitives.hh"

using namespace graphics::opengl::primitives;
using namespace graphics::primitives;

GLint const attr_pos = 0;
GLint const attr_color = 1;
GLint const attr_uv = 2;

template<>
void graphics::opengl::primitives::attrib_setup<XYZVert>() {
	glEnableVertexAttribArray(attr_pos);
	glVertexAttribPointer(
		attr_pos,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZVert),
		0
	);
}

template<>
void graphics::opengl::primitives::attrib_setup<XYZRGBVert>() {
	glEnableVertexAttribArray(attr_pos);
	glEnableVertexAttribArray(attr_color);
	glVertexAttribPointer(
		attr_pos,
		3,
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
		(const void*)(3 * sizeof(float))
	);
}

template<>
void graphics::opengl::primitives::attrib_setup<XYZRGBUVVert>() {
	glEnableVertexAttribArray(attr_pos);
	glEnableVertexAttribArray(attr_color);
	glEnableVertexAttribArray(attr_uv);
	glVertexAttribPointer(
		attr_pos,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZRGBUVVert),
		0
	);
	glVertexAttribPointer(
		attr_color,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZRGBUVVert),
		(const void*)(3 * sizeof(float))
	);
	glVertexAttribPointer(
		attr_uv,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZRGBUVVert),
		(const void*)(6 * sizeof(float))
	);
}

template<>
void graphics::opengl::primitives::attrib_setup<XYZUVVert>() {
	glEnableVertexAttribArray(attr_pos);
	glEnableVertexAttribArray(attr_uv);
	glVertexAttribPointer(
		attr_pos,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZUVVert),
		0
	);
	glVertexAttribPointer(
		attr_uv,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(XYZUVVert),
		(const void*)(3 * sizeof(float))
	);
}


