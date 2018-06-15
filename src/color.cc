#include "color.hh"

#include <cstdlib>

using namespace color;
using namespace err;

Result<RGBColor> RGBColor::from_str(std::string_view raw) {
	using namespace std::literals;

	if (!(raw.length() == 8 || raw.length() == 7)) {
		return "Color string not long enough"sv;
	}

	std::string_view numbers;
	if (raw[1] == 'x') {
		numbers = raw.substr(2, raw.length());
	} else if (raw[0] == '#') {
		numbers = raw.substr(1, raw.length());
	} else {
		return "Color in improper format"sv;
	}

	uint8_t buf[3] = {0};
	*((long*)(buf)) = strtoul((const char*)numbers.data(), NULL, 16);

	return RGBColor(
		buf[2],
		buf[1],
		buf[0]
	);
}
