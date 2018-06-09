#pragma once

#include <stdint.h>

#include "color.hh"

struct Config {
	RGBColor fg;
	RGBColor bg;
	uint8_t tab_size;
	uint8_t font_size;
};
