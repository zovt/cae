#pragma once

#include <cstdint>
#include <string_view>

#include "err.hh"

namespace color {

struct RGBColor {
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	constexpr RGBColor(uint8_t red, uint8_t green, uint8_t blue)
		: red(red), green(green), blue(blue) {}

	static err::Result<RGBColor> from_str(std::string_view raw);
};

}
