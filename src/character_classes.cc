#include "character_classes.hh"
#include "debug.hh"

using namespace cae;

std::optional<PairChar> cae::get_paired_char(uint8_t chr) {
	for (size_t i = 0; i < paired_chars.size(); ++i) {
		if (chr == paired_chars[i]) {
			if (i % 2 == 0) {
				return {{ PairSide::Right, paired_chars[i + 1] }};
			} else {
				return {{ PairSide::Left, paired_chars[i - 1] }};
			}
		}
	}

	return std::nullopt;
}