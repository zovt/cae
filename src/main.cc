#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <cpptoml.h>
#include <string>

#include "plat.hh"
#include "str.hh"
#include "config.hh"

Err run() {
	stack_buf(path, char, 500);
	check_err(get_config_path(&path));

	auto conf_toml = cpptoml::parse_file(str_mut_to_c_str(path.data));
	Config conf = {};

	auto tab_size = conf_toml->get_as<uint8_t>("tab_size");
	if (tab_size) {
		conf.tab_size = *tab_size;
	}

	auto font_size = conf_toml->get_as<uint8_t>("font_size");
	if (font_size) {
		conf.font_size = *font_size;
	}

	auto fg = conf_toml->get_as<std::string>("fg");
	if (fg) {
		const char* fg_str = fg->c_str();
		check_err(rgb_color_from_str(str_from_c_str(fg_str), &conf.fg));
	}

	auto bg = conf_toml->get_as<std::string>("bg");
	if (bg) {
		const char* bg_str = bg->c_str();
		check_err(rgb_color_from_str(str_from_c_str(bg_str), &conf.bg));
	}

	return Err::OK;
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	Err e = run();
	if (e != Err::OK) {
		fprintf(stdout, "Error encountered while running: %d. See errs.hh for details\n", (int)e);
	}
	return 0;
}
