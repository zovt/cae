#pragma once

#include <utility>

#include "drawing.hh"
#include "shaders.hh"

namespace graphics { namespace opengl { namespace primitives {

static std::vector<GLuint> const pixel_indices = {
	0, 1, 2,
	0, 2, 3,
};

void pixel_attrib_setup();
void mc_pixel_attrib_setup();

} } }
