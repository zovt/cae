#include <string>
#include <vector>
#include <variant>
#include <iostream>

#include "unit.hh"
#include "config.hh"
#include "color.hh"
#include "fonts.hh"

using namespace config;
using namespace color;
using namespace fonts;

static const Config conf(
	{"Iosevka Term"},
	RGBColor(255, 255, 255),
	RGBColor(30, 30, 30),
	2,
	16
);

Result<Unit> run() {
	try(auto font_path, get_closest_font_match(conf.fonts));
	std::cout << font_path << std::endl;

	return unit;
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	auto out = run();
	if (std::holds_alternative<Err>(out)) {
		std::cout << std::get<Err>(out) << std::endl;
	}
	return 0;
}
