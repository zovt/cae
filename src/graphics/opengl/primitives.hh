#pragma once

#include <vector>

#include "index.hh"

namespace graphics { namespace opengl { namespace primitives {

static std::vector<GLuint> const pixel_indices = {
	0, 1, 2,
	0, 2, 3,
};

template <typename VertexType>
void attrib_setup();

} } }
