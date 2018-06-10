#pragma once

#include "macros.hh"

#define check_err(err)\
	Err CAT(__e_, __LINE__) = err;\
	if ((int)CAT(__e_, __LINE__)) {\
		return CAT(__e_, __LINE__); \
	}

enum class Err {
	OK = 0,
	// Paths
	PATH_NOT_FOUND = 100,
	// Buffers
	BUF_NOT_BIG_ENOUGH = 200,
	// Colors
	COLOR_PARSE_STR_NOT_LONG_ENOUGH = 300,
	COLOR_FAILED_TO_PARSE,
	// Slices
	SLICE_INDEX_OUT_OF_RANGE = 400,
	// Fonts
	FONT_LOOKUP_ERR = 500,
};
