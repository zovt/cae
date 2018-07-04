#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>

#include "common_state.hh"

namespace buffer {

struct PointPos {
	size_t line;
	size_t offset;
};

struct PointOffset {
	size_t index;
};

struct Piece {
	std::vector<uint8_t> contents;
	size_t start;
	size_t end;
};

struct Buffer {
	// TODO: Use a more optimized data structure to hold buffer contents
	std::vector<uint8_t> contents;
	std::filesystem::path path;

	// characters are inserted AT this index
	PointOffset point;

	std::vector<Piece> pieces;
	size_t piece_idx;
	size_t active_pieces;
	void _cut_piece();

	void set_point(PointOffset pos);
	void backspace();
	void insert(uint8_t chr);
	void undo();
	void redo();
};

Buffer slurp_to_buffer(std::filesystem::path path);

}
