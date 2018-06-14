#pragma once

#include <string>
#include <vector>

#include <fontconfig/fontconfig.h>

#include "plat.hh"
#include "err.hh"


#ifdef UNIX
static Result<std::string> get_closest_font_match(
	const std::vector<std::string>& font_names
) {
	using namespace std::literals;

	auto fontconfig = FcInitLoadConfigAndFonts();

	for (size_t i = 0; i < font_names.size(); i++) {
		auto fc_result = FcResultNoMatch;
		auto pattern = FcNameParse((FcChar8*)font_names[i].c_str());
		FcConfigSubstitute(fontconfig, pattern, FcMatchPattern);
		FcDefaultSubstitute(pattern);

		auto res = FcFontMatch(fontconfig, pattern, &fc_result);
		if (res) {
			FcChar8* file = nullptr;
			if (FcPatternGetString(res, "file", 0, &file) == FcResultMatch) {
				std::string file_str((const char*)file);

				FcPatternDestroy(pattern);
				FcConfigDestroy(fontconfig);
				return file_str;
			}
		}

		FcPatternDestroy(pattern);
	}

	FcConfigDestroy(fontconfig);
	return "Couldn't find font"sv;
}
#endif
