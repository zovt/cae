#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "str.hh"
#include "buf.hh"

struct RGBColor {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};

Err rgb_color_from_str(Str raw, RGBColor* out);
