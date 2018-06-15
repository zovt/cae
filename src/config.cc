#include "config.hh"

using namespace config;
using namespace color;

Config::Config(
		std::vector<std::string>&& fonts,
		RGBColor fg,
		RGBColor bg,
		uint8_t tab_size,
		uint8_t font_size
	) : fonts(fonts), fg(fg), bg(bg), tab_size(tab_size), font_size(font_size) {}
