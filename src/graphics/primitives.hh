#pragma once

#include <vector>
#include <array>
#include <utility>

namespace graphics { namespace primitives {

// All primitives are specified as flat arrays of (XYZ) clockwise

struct XYZVert {
	std::array<float, 3> xy;
};

static std::vector<XYZVert> const pixel_data = {
	{ -1.0f, 1.0f },
	{ 0.0f, 1.0f },
	{ 0.0f, 0.0f },
	{ -1.0f, 0.0f },
};

struct XYZRGBVert {
	std::array<float, 3> xyz;
	std::array<float, 3> rgb;
};

static std::vector<XYZRGBVert> const mc_pixel_data {
	{
		{ -1.0f, 1.0f },
		{ 1.0, 0.0f, 0.0f, }
	},
	{
		{ 0.0f, 1.0f },
		{ 0.0f, 1.0f, 0.0f, }
	},
	{
		{ 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, }
	},
	{
		{ -1.0f, 0.0f },
		{ 1.0f, 0.0f, 1.0f, }
	},
};

struct XYZRGBUVVert {
	std::array<float, 3> xyz;
	std::array<float, 3> rgb;
	std::array<float, 2> uv;
};

std::vector<XYZRGBUVVert> const tex_pixel_data {
	{
		{ -1.f, 1.f },
		{ 1.f, 0.f, 0.f },
		{ 0.f, 1.f }
	},
	{
		{ 0.f, 1.f },
		{ 0.f, 1.f, 0.f },
		{ 1.f, 1.f }
	},
	{
		{ 0.f, 0.f },
		{ 0.f, 0.f, 1.f },
		{ 1.f, 0.f }
	},
	{
		{ -1.f, 0.f },
		{ 1.f, 0.f, 1.f},
		{ 0.f, 0.f }
	}
};

struct XYZUVVert {
	std::array<float, 3> xyz;
	std::array<float, 2> uv;
};

} }
