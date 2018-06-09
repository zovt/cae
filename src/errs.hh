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
};
