#pragma once

#include <vector>
#include <array>
#include <utility>

namespace graphics { namespace primitives {

// All primitives are specified as flat arrays of (XYZ) clockwise

struct XYZVert {
	std::array<float, 3> pos;
};

static std::vector<XYZVert> const pixel_data = {
	{ -1.0f, 1.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ -1.0f, 0.0f, 0.0f },
};

struct XYZRGBVert {
	std::array<float, 3> pos;
	std::array<float, 3> color;
};

static std::vector<XYZRGBVert> const mc_pixel_data {
	{
		{ -1.0f, 1.0f, 0.0f, },
		{ 1.0, 0.0f, 0.0f, }
	},
	{
		{ 0.0f, 1.0f, 0.0f, },
		{ 0.0f, 1.0f, 0.0f, }
	},
	{
		{ 0.0f, 0.0f, 0.0f, },
		{ 0.0f, 0.0f, 1.0f, }
	},
	{
		{ -1.0f, 0.0f, 0.0f, },
		{ 1.0f, 0.0f, 1.0f, }
	},
};

} }
