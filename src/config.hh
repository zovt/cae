#pragma once

#include <stdint.h>
#include <string>

#include "color.hh"

struct Config {
	Slice<std::string> fonts;
	RGBColor fg;
	RGBColor bg;
	uint8_t tab_size;
	uint8_t font_size;
};
