#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "errs.hh"
#include "plat_com.hh"
#include "str.hh"

static Err get_config_path(StrBuf* out) {
	const char* xdg_config = getenv("XDG_CONFIG_HOME");

	if (xdg_config) {
		check_err(out->write(str_from_c_str(xdg_config)));
		return out->write(str("/cae/config.json"));
	}

	const char* home = getenv("HOME");
	if (home) {
		check_err(out->write(str_from_c_str(home)));
		return out->write(str("/.config/cae/config.json"));
	}

	return Err::PATH_NOT_FOUND;
}
