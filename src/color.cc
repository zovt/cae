#include <stdio.h>
#include "color.hh"

Err rgb_color_from_str(Str raw, RGBColor* out) {
	if (!(raw.len == 8 || raw.len == 7)) {
		return Err::COLOR_PARSE_STR_NOT_LONG_ENOUGH;
	}

	Str numbers;
	if (raw.data[1] == 'x') {
		check_err(raw.sub(2, &numbers));
	} else if (raw.data[0] == '#') {
		check_err(raw.sub(1, &numbers));
	} else {
		return Err::COLOR_FAILED_TO_PARSE;
	}

	uint8_t buf[sizeof(long) / sizeof(uint8_t)] = {0};
	*((long*)(buf)) = strtoul(numbers.data, NULL, 16);

	out->red = buf[sizeof(buf) - 3];
	out->green = buf[sizeof(buf) - 2];
	out->blue = buf[sizeof(buf) - 1];

	return Err::OK;
}
