#pragma once

#include <stdlib.h>

#include "slice.hh"
#include "buf.hh"

using Str = Slice<char>;
using StrMut = SliceMut<char>;
using StrBuf = Buf<char>;

static constexpr size_t compile_time_strlen(const char* str) {
	return *str ? 1 + compile_time_strlen(str + 1) : 0;
}

static constexpr const Str str(const char* chars) {
	Str ret = {};
	ret.data = chars;
	size_t sz = compile_time_strlen(chars);
	ret.len = sz;
	return ret;
}

Str str_from_c_str(const char* c_str);
StrMut str_mut_from_c_str(char* c_str);
const char* str_to_c_str(Str str);
char* str_mut_to_c_str(StrMut str);
