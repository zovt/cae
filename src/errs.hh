#pragma once

#define check_err(err)\
	Err e = err;\
	if ((int)e) {\
		return e; \
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
};
