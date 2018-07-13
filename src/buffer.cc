#include "buffer.hh"

#include <cstring>
#include <fstream>
#include <iostream>

#define DEBUG_NAMESPACE "buffer"
#include "debug.hh"
#include "util.hh"
#include "paired_chars.hh"

using namespace buffer;
using namespace util;
using namespace cae;

Diff Diff::inverse() const {
	if (std::holds_alternative<Addition>(element)) {
		auto addition = std::get<Addition>(element);

		std::vector<uint8_t> contents = {addition.contents.rbegin(), addition.contents.rend()};
		return Diff {
			Deletion {
				contents,
				addition.start,
				addition.end
			}
		};
	} else if (std::holds_alternative<Deletion>(element)) {
		auto deletion = std::get<Deletion>(element);

		std::vector<uint8_t> contents = {deletion.contents.rbegin(), deletion.contents.rend()};
		return Diff {
			Addition {
				contents,
				deletion.start,
				deletion.end
			}
		};
	} else {
		return Diff { unit };
	}
}

void Diff::apply(std::vector<uint8_t>& contents) const {
	if (std::holds_alternative<Addition>(element)) {
		auto addition = std::get<Addition>(element);
		dbg_printval(addition.start);
		dbg_printval(addition.end);
		dbg_printval((char*)addition.contents.data());
		contents.insert(
			contents.begin() + addition.start,
			addition.contents.begin(),
			addition.contents.end()
		);
	} else if (std::holds_alternative<Deletion>(element)) {
		auto deletion = std::get<Deletion>(element);
		dbg_printval(deletion.start);
		dbg_printval(deletion.end);
		dbg_printval((char*)deletion.contents.data());
		contents.erase(
			contents.begin() + deletion.start,
			contents.begin() + deletion.end
		);
	}
}

Buffer buffer::slurp_to_buffer(std::filesystem::path path) {
	Buffer buf{};
	std::vector<uint8_t> contents(std::filesystem::file_size(path));
	std::ifstream in(path.native(), std::ios::in | std::ios::binary);
	in.read((char*)contents.data(), contents.size());

	buf.contents = std::move(contents);
	buf.path = path;
	buf.current_change = Diff { unit };
	return buf;
}

Buffer::Buffer()
: contents{}, path{}, point{}, current_change{unit} {}

void Buffer::set_point(Point pos) {
	point = pos;

	if (!std::holds_alternative<Unit>(current_change.element)) {
		undo_chain.push_back(current_change.inverse());
	}
	current_change.element = unit;
}

size_t find_pair_chr(
	Buffer::Contents const& contents,
	size_t index,
	uint8_t at,
	uint8_t wanted,
	int dir
) {
	size_t depth = 0;
	for (int i = index; i < contents.size() && i >= 0; i += dir) {
		auto chr = contents[i];
		if (chr == at) {
			++depth;
		} else if (chr == wanted) {
			if (depth == 0) {
				return i;
			}
			--depth;
		}
	}

	return index;
}

void Buffer::handle_set_point(Point pos) {
	dbg_println("handle_set_point");
	if (pos.point == point.point && pos.mark == point.mark) {
		if (!contents.empty()) {
			auto chr = contents[point.point];
			
			if (auto pair_chr_o = get_paired_char(chr)) {
				auto pair_chr = *pair_chr_o;
				if (pair_chr.side == PairSide::Right) {
					set_point({point.point, find_pair_chr(contents, std::min(point.point + 1, contents.size()), chr, pair_chr.chr, 1) + 1});
				} else {
					set_point({point.point + 1, find_pair_chr(contents, std::max((int)point.point - 1, 0), chr, pair_chr.chr, -1)});
				}
				return;
			}
		}
		
	} else {
		set_point(pos);
	}
}

void Buffer::backspace() {
	if (point.point == 0) {
		return;
	}

	redo_chain.clear();

	auto selection_min = std::min(point.point, point.mark);
	auto selection_max = std::max(point.point, point.mark);
	if (point.point == point.mark) {
		selection_min = point.point - 1;
		selection_max = point.point;
	}
	std::vector<uint8_t> chrs{contents.begin() + selection_min, contents.begin() + selection_max};
	chrs = {chrs.rbegin(), chrs.rend()};
	contents.erase(contents.begin() + selection_min, contents.begin() + selection_max);

	point = {selection_min, selection_min};
	if (auto deletion_ref = opt_get<Deletion>(current_change.element)) {
		auto& deletion = deletion_ref->get();
		deletion.contents.insert(deletion.contents.end(), chrs.begin(), chrs.end());
		deletion.start = selection_min;
	} else {
		if (!std::holds_alternative<Unit>(current_change.element)) {
			undo_chain.push_back(current_change.inverse());
		}
		current_change.element = Deletion{chrs, selection_min, selection_max};
	}
}

void Buffer::insert(uint8_t chr) {
	redo_chain.clear();
	if (point.point != point.mark) {
		backspace();
	}
	contents.insert(contents.begin() + point.point, chr);
	++point.point;
	++point.mark;

	if (auto addition_ref = opt_get<Addition>(current_change.element)) {
		dbg_println("Insert has addition");
		auto& addition = addition_ref->get();
		addition.contents.push_back(chr);
		++addition.end;
		dbg_printval(addition.end);
	} else {
		if (!std::holds_alternative<Unit>(current_change.element)) {
			dbg_println("Insert has unit");
			undo_chain.push_back(current_change.inverse());
		}
		dbg_println("New addition");
		current_change.element = Addition{{chr}, point.point - 1, point.point};
	}
}

void Buffer::insert_all(gsl::span<uint8_t const> chrs) {
	redo_chain.clear();
	if (point.point != point.mark) {
		backspace();
	}
	contents.insert(contents.begin() + point.point, chrs.begin(), chrs.end());
	point.point += chrs.size();
	point.mark += chrs.size();

	if (auto addition_ref = opt_get<Addition>(current_change.element)) {
		auto& addition = addition_ref->get();
		addition.contents.insert(addition.contents.end(), chrs.begin(), chrs.end());
		addition.end += chrs.size();
	} else {
		if (!std::holds_alternative<Unit>(current_change.element)) {
			undo_chain.push_back(current_change.inverse());
		}
		current_change.element = Addition{{chrs.begin(), chrs.end()}, point.point - chrs.size(), point.point};
	}
}

gsl::span<uint8_t> Buffer::get_selection() {
	auto selection_min = std::min(point.point, point.mark);
	auto selection_max = std::max(point.point, point.mark);
	return {
		contents.data() + selection_min,
		contents.data() + selection_max
	};
}

void Buffer::undo() {
	dbg_println("Undo");
	if (!std::holds_alternative<Unit>(current_change.element)) {
		dbg_println("Committing current change");
		undo_chain.push_back(current_change.inverse());
		current_change.element = unit;
	}

	if (undo_chain.size() == 0) {
		dbg_println("Nothing to undo");
		return;
	}

	auto change = undo_chain.back();
	undo_chain.pop_back();
	redo_chain.push_back(change.inverse());
	change.apply(contents);

	point.point = std::min(point.point, contents.size());
	point.mark = std::min(point.point, contents.size());
}

void Buffer::redo() {
	dbg_println("Redo");
	if (redo_chain.size() == 0) {
		dbg_println("Nothing to redo");
		return;
	}

	auto change = redo_chain.back();
	redo_chain.pop_back();
	undo_chain.push_back(change.inverse());
	change.apply(contents);

	point.point = std::min(point.point, contents.size());
	point.mark = std::min(point.point, contents.size());
}

void Buffer::save() {
	dbg_println("Save");
	std::ofstream out(path.native(), std::ios::out | std::ios::binary);
	out.write((const char*)contents.data(), contents.size());
}

int find_prev_newline(Buffer::Contents const& contents, int index) {
	auto start = index;
	for (; start >= 0; --start) {
		if (contents[start] == '\n') {
			return start;
		}
	}
	return -1;
}

int find_next_newline(Buffer::Contents const& contents, int index) {
	auto start = index;
	for (; start < contents.size(); ++start) {
		if (contents[start] == '\n') {
			return start;
		}
	}
	return contents.size() - 1;
}
	
void Buffer::point_up() {
	auto new_point = find_prev_newline(contents, point.point - 2) + 1;
	set_point({new_point, new_point});
}

void Buffer::point_down() {
	auto new_point = find_next_newline(contents, point.point) + 1;
	set_point({new_point, new_point});
}

void Buffer::point_left() {
	auto point_idx = std::max((size_t)1, point.point) - 1;
	set_point({point_idx, point_idx});
}

void Buffer::point_right() {
	auto point_idx = std::min(contents.size(), point.point + 1);
	set_point({point_idx, point_idx});
}