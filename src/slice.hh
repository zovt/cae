#pragma once

#include <stdlib.h>

template <class T>
struct Slice {
	const T* data;
	size_t len;
};

template <class T>
Slice<T> slice_from_ptr(const T* ptr, size_t len) {
	Slice<T> ret = {0};
	ret.ptr = ptr;
	ret.len = len;

	return ret;
}

template <class T>
struct SliceMut {
	T* data;
	size_t len;
};

template <class T>
SliceMut<T> slice_mut_from_ptr(T* ptr, size_t len) {
	SliceMut<T> ret = {0};
	ret.ptr = ptr;
	ret.len = len;

	return ret;
}
