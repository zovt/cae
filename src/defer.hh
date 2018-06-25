#pragma once

#include "macros.hh"

namespace defer {

template<typename T>
struct Defer {
	T defer_cb;

	constexpr Defer(const T defer_cb) : defer_cb(defer_cb) {}

	~Defer() {
		defer_cb();
	}
};

}

#define defer(callable)\
	const defer::Defer CAT(__defer__, __LINE__)(callable)
