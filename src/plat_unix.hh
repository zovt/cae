#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fontconfig/fontconfig.h>

#include "errs.hh"
#include "plat_com.hh"
#include "str.hh"
#include "slice.hh"

static Err get_config_path(StrBuf* out) {
	const char* xdg_config = getenv("XDG_CONFIG_HOME");

	if (xdg_config) {
		check_err(out->write(str_from_c_str(xdg_config)));
		return out->write(str("/cae/cae.toml"));
	}

	const char* home = getenv("HOME");
	if (home) {
		check_err(out->write(str_from_c_str(home)));
		return out->write(str("/.config/cae/cae.toml"));
	}

	return Err::PATH_NOT_FOUND;
}

static Err get_closest_font_match(Slice<std::string> font_names, StrBuf* path_out) {
	auto fontconfig = FcInitLoadConfigAndFonts();

	for (size_t i = 0; i < font_names.len; i++) {
		auto fc_result = FcResultNoMatch;
		auto pattern = FcNameParse((FcChar8*)((font_names.data + i)->c_str()));
		FcConfigSubstitute(fontconfig, pattern, FcMatchPattern);
		FcDefaultSubstitute(pattern);

		auto res = FcFontMatch(fontconfig, pattern, &fc_result);
		if (res) {
			FcChar8* file = nullptr;
			if (FcPatternGetString(res, "file", 0, &file) == FcResultMatch) {
				Str file_str = str_from_c_str((const char*)file);

				FcPatternDestroy(pattern);
				FcConfigDestroy(fontconfig);
				return path_out->write(file_str);
			}
		}

		FcPatternDestroy(pattern);
	}

	FcConfigDestroy(fontconfig);
	return Err::FONT_LOOKUP_ERR;
}
