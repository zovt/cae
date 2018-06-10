#pragma once

#include <stdlib.h>
#include <string.h>

#include "macros.hh"
#include "errs.hh"
#include "slice.hh"

template <class T>
struct Buf {
	SliceMut<T> data;
	size_t used;

	void clear() {
		this->used = 0;
	}

	Err write(Slice<T> in) {
		if (this->data.len - this->used < in.len) {
			return Err::BUF_NOT_BIG_ENOUGH;
		}

		memcpy(this->data.data + this->used, in.data, in.len);
		this->used += in.len;
		return Err::OK;
	}

	size_t write_cap(Slice<T> in) {
		const size_t buf_free = this->data.len - this->used;
		const size_t write_sz = buf_free < in.len ? buf_free : in.len;

		memcpy(this->data.data + this->used, in.data, in.len);
		this->used += write_sz;
		return write_sz;
	}
};

template <typename T>
Buf<T> make_buf(T* buf, size_t sz) {
	Buf<T> ret = {};
	ret.data.data = buf;
	ret.data.len = sz;
	ret.used = 0;
	return ret;
}

#define stack_buf(name, T, sz)\
	T CAT(__ ## buf ## __, __LINE__) [sz] = {};\
	Buf<T> name = make_buf(CAT(__ ## buf ## __, __LINE__), sz)
