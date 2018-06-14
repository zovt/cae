#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "color.hh"

struct Config {
	std::vector<std::string> fonts;
	RGBColor fg;
	RGBColor bg;
	uint8_t tab_size;
	uint8_t font_size;

	Config(
		std::vector<std::string>&& fonts,
		RGBColor fg,
		RGBColor bg,
		uint8_t tab_size,
		uint8_t font_size
	);
};
