#include "primitives.hh"
#include "../primitives.hh"

using namespace graphics::opengl::primitives;
using namespace graphics::opengl::drawing;
using namespace graphics::opengl::shaders;
using namespace graphics::primitives;

GLVertexData graphics::opengl::primitives::make_pixel(Program<XYZVertInputs> const& shdr) {
	PrimitiveData<XYZVert> const pixel_data = {
		pixel_verts,
		pixel_indices
	};
	GLVertexData pixel(pixel_data, shdr);

	return pixel;
}

GLVertexData graphics::opengl::primitives::make_mc_pixel(Program<XYZRGBVertInputs> const& shdr) {
	PrimitiveData<XYZRGBVert> const mc_pixel_data = {
		multicolor_pixel_verts,
		pixel_indices
	};
	GLVertexData mc_pixel(mc_pixel_data, shdr);

	return mc_pixel;
}
