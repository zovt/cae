#pragma once

#include <stdlib.h>

#include "errs.hh"

template <class T>
struct Slice {
	const T* data;
	size_t len;

	Err sub(const size_t start, Slice<T>* out) const {
		return this->sub(start, this->len, out);
	}

	Err sub(const size_t start, const size_t end, Slice<T>* out) const {
		if (start > len) {
			return Err::SLICE_INDEX_OUT_OF_RANGE;
		}

		if (end < start) {
			return Err::SLICE_INDEX_OUT_OF_RANGE;
		}

		out->data = this->data + start;
		out->len = this->len - start - end;
		return Err::OK;
	}
};

template <class T>
Slice<T> slice_from_ptr(const T* ptr, size_t len) {
	Slice<T> ret = {};
	ret.data = ptr;
	ret.len = len;

	return ret;
}

template <class T>
struct SliceMut {
	T* data;
	size_t len;

	Slice<T> to_const() const {
		Slice<T> ret = {};
		ret.data = this->data;
		ret.len = this->len;
		return ret;
	}
};

template <class T>
SliceMut<T> slice_mut_from_ptr(T* ptr, size_t len) {
	SliceMut<T> ret = {};
	ret.data = ptr;
	ret.len = len;

	return ret;
}
