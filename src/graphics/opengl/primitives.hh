#pragma once

#include "drawing.hh"
#include "shaders.hh"

namespace graphics { namespace opengl { namespace primitives {

static std::vector<GLuint> const pixel_indices = {
	0, 1, 2,
	0, 2, 3,
};

drawing::GLVertexData make_pixel(
	shaders::Program<shaders::XYZVertInputs> const& shdr
);

drawing::GLVertexData make_mc_pixel(
	shaders::Program<shaders::XYZRGBVertInputs> const& shdr
);

} } }
