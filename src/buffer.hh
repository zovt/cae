#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <optional>
#include <variant>
#include <gsl/span>

#include "common_state.hh"
#include "unit.hh"

namespace buffer {

struct Point {
	size_t point;
	size_t mark;
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

	// characters are inserted AT this index start
	Point point;

	Diff current_change;
	std::vector<Diff> undo_chain;
	std::vector<Diff> redo_chain;

	void set_point(Point pos);
	void backspace();
	void insert(uint8_t chr);
	void insert_all(gsl::span<uint8_t const> chrs);
	void undo();
	void redo();
	void save();
};

Buffer slurp_to_buffer(std::filesystem::path path);

}
