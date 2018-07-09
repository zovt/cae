#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <variant>
#include <gsl/span>

#include "common_state.hh"
#include "unit.hh"

namespace buffer {

struct PointOffset {
	size_t index;
};

struct Addition {
	std::vector<uint8_t> contents;
	size_t start;
	size_t end;
};

struct Deletion {
	std::vector<uint8_t> contents;
	size_t start;
	size_t end;
};

struct Diff {
	std::variant<Addition, Deletion, Unit> element;

	Diff inverse() const;
	void apply(std::vector<uint8_t>& contents) const;
};

struct Buffer {
	Buffer();

	std::vector<uint8_t> contents;
	std::filesystem::path path;

	// characters are inserted AT this index
	PointOffset point;

	Diff current_change;
	std::vector<Diff> undo_chain;
	std::vector<Diff> redo_chain;

	void set_point(PointOffset pos);
	void backspace();
	void insert(uint8_t chr);
	void insert_all(gsl::span<uint8_t const> chrs);
	void undo();
	void redo();
	void save();
};

Buffer slurp_to_buffer(std::filesystem::path path);

}
