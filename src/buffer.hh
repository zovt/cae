#pragma once

#include <cstdint>
#include <functional>
#include <optional>
#include <variant>
#include <string_view>
#include <gsl/span>

#include "common_state.hh"
#include "unit.hh"
#include "plat.hh"

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

enum struct SelectionMode {
	Char,
	Word,
	Line
};

struct Buffer {
	using Contents = std::vector<uint8_t>;

	Buffer();

	Contents contents;
	std::string path;

	// characters are inserted AT this index start
	Point point;

	Diff current_change;
	std::vector<Diff> undo_chain;
	std::vector<Diff> redo_chain;

	SelectionMode mode;

	void set_point(Point pos);
	void handle_set_point(Point pos);
	void backspace();
	void insert(uint8_t chr);
	void handle_insert(uint8_t chr);
	void insert_all(gsl::span<uint8_t const> chrs);
	gsl::span<uint8_t> get_selection();
	void undo();
	void redo();
	void save();

	void point_up();
	void point_down();
	void point_left();
	void point_right();
};

Buffer slurp_to_buffer(std::string const& path);

}
