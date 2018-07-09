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

void Buffer::set_point(PointOffset pos) {
	point = pos;

	if (!std::holds_alternative<Unit>(current_change.element)) {
		undo_chain.push_back(current_change);
	}
	current_change.element = unit;
}

void Buffer::backspace() {
	dbg_printval(point.index);
	if (point.index == 0) {
		return;
	}

	redo_chain.clear();
	auto chr = contents[point.index - 1];
	contents.erase(contents.begin() + point.index - 1);
	--point.index;

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
		current_change.element = Deletion{{chr}, point.index, point.index + 1};
	}
}

void Buffer::insert(uint8_t chr) {
	redo_chain.clear();
	contents.insert(contents.begin() + point.index, chr);
	++point.index;

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
		current_change.element = Addition{{chr}, point.index - 1, point.index};
	}
}

void Buffer::insert_all(gsl::span<uint8_t const> chrs) {
	redo_chain.clear();
	contents.insert(contents.begin() + point.index, chrs.begin(), chrs.end());
	point.index += chrs.size();

	if (std::holds_alternative<Addition>(current_change.element)) {
		auto& addition = std::get<Addition>(current_change.element);
		addition.contents.insert(addition.contents.end(), chrs.begin(), chrs.end());
		addition.end += chrs.size();
	} else {
		if (!std::holds_alternative<Unit>(current_change.element)) {
			undo_chain.push_back(current_change.inverse());
		}
		current_change.element = Addition{{chrs.begin(), chrs.end()}, point.index - chrs.size(), point.index};
	}
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

	point.index = std::min(point.index, contents.size());
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

	point.index = std::min(point.index, contents.size());
}

void Buffer::save() {
	dbg_println("Save");
	std::ofstream out(path.native(), std::ios::out | std::ios::binary);
	out.write((const char*)contents.data(), contents.size());
}
