#include <string.h>

#include "str.hh"

Str str_from_c_str(const char* c_str) {
	Str ret = {};
	ret.data = c_str;
	ret.len = strlen(c_str);
	return ret;
}

StrMut str_from_c_str_mut(char* c_str) {
	StrMut ret = {};
	ret.data = c_str;
	ret.len = strlen(c_str);
	return ret;
}

const char* str_to_c_str(Str str) {
	return str.data;
}

char* str_mut_to_c_str(StrMut str) {
	return str.data;
}
