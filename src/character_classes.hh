#pragma once

#include <cstdint>
#include <vector>
#include <optional>

namespace cae {

enum struct PairSide {
	Left,
	Right
};

struct PairChar {
	PairSide side;
	uint8_t chr;
};

static std::vector<uint8_t> const paired_chars = {
	'{', '}',
	'(', ')',
	'[', ']',
	'<', '>',
	'\'', '\'',
	'"', '"',
	'`', '`'
};

std::optional<PairChar> get_paired_char(uint8_t chr);

}