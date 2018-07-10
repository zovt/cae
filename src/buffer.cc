#include "buffer.hh"

#include <cstring>
#include <fstream>
#include <iostream>

#define DEBUG_NAMESPACE "buffer"
#include "debug.hh"

using namespace buffer;

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
		undo_chain.push_back(current_change);
	}
	current_change.element = unit;
}

void Buffer::backspace() {
	if (point.point == 0) {
		return;
	}

	redo_chain.clear();
	auto selection_min = std::min(point.point - 1, point.mark);
	auto selection_max = std::max(point.point - 1, point.mark);
	if (point.point == point.mark) {
		selection_max = point.point - 1;
	}
	contents.erase(contents.begin() + selection_min, contents.begin() + selection_max + 1);

	point = {selection_min, selection_min};
	/*
	if (std::holds_alternative<Deletion>(current_change.element)) {
		dbg_println("Backspace has deletion");
		auto& deletion = std::get<Deletion>(current_change.element);
		deletion.contents.push_back(chr);
		--deletion.start;
	} else {
		if (!std::holds_alternative<Unit>(current_change.element)) {
			dbg_println("Backspace has unit");
			undo_chain.push_back(current_change.inverse());
		}
		dbg_println("New Deletion");
		current_change.element = Deletion{{chr}, point.point, point.point + 1};
	}*/
}

void Buffer::insert(uint8_t chr) {
	redo_chain.clear();
	if (point.point != point.mark) {
		backspace();
	}
	contents.insert(contents.begin() + point.point, chr);
	++point.point;
	++point.mark;

	/*
	if (std::holds_alternative<Addition>(current_change.element)) {
		dbg_println("Insert has addition");
		auto& addition = std::get<Addition>(current_change.element);
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
	*/
}

void Buffer::insert_all(gsl::span<uint8_t const> chrs) {
	redo_chain.clear();
	if (point.point != point.mark) {
		backspace();
	}
	contents.insert(contents.begin() + point.point, chrs.begin(), chrs.end());
	point.point += chrs.size();
	point.mark += chrs.size();

	/*
	if (std::holds_alternative<Addition>(current_change.element)) {
		auto& addition = std::get<Addition>(current_change.element);
		addition.contents.insert(addition.contents.end(), chrs.begin(), chrs.end());
		addition.end += chrs.size();
	} else {
		if (!std::holds_alternative<Unit>(current_change.element)) {
			undo_chain.push_back(current_change.inverse());
		}
		current_change.element = Addition{{chrs.begin(), chrs.end()}, point.point - chrs.size(), point.point};
	}
	*/
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
}

void Buffer::save() {
	dbg_println("Save");
	std::ofstream out(path.native(), std::ios::out | std::ios::binary);
	out.write((const char*)contents.data(), contents.size());
}
