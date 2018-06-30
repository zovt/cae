#pragma once

#include <cstdint>
#include <filesystem>

#include "common_state.hh"

namespace buffer {

struct PointPos {
	size_t line;
	size_t offset;
};

struct PointOffset {
	size_t index;
};

struct Buffer {
	// TODO: Use a more optimized data structure to hold buffer contents
	std::vector<uint8_t> contents;
	std::filesystem::path path;

	// characters are inserted AT this index
	PointOffset point;

	void set_point(PointOffset pos);
	void backspace();
	void insert(uint8_t chr);
};

Buffer slurp_to_buffer(std::filesystem::path path);

}
