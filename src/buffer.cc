#include "buffer.hh"

#include <cstring>
#include <fstream>
#include <iostream>

using namespace buffer;

Buffer buffer::slurp_to_buffer(std::filesystem::path path) {
	std::vector<uint8_t> contents(std::filesystem::file_size(path));
	std::ifstream in(path.native(), std::ios::in | std::ios::binary);
	in.read((char*)contents.data(), contents.size());
	return { contents, path, {}, {{{}, 0, 0}}, {}, 1 };
}

void Buffer::_cut_piece() {
	std::cout << this->piece_idx << std::endl;
	auto& current_piece = this->pieces[this->piece_idx];

	if (current_piece.start == current_piece.end) {
		return;
	}

	current_piece.end = this->point.index;

	current_piece.contents.clear();
	auto n_chars = this->point.index - current_piece.start;
	current_piece.contents.reserve(n_chars);
	std::memcpy(
		current_piece.contents.data(),
		this->contents.data() + current_piece.start,
		n_chars
	);

	Piece new_piece{{}, this->point.index, this->point.index};
	++this->piece_idx;
	++this->active_pieces;
	if (this->piece_idx == this->pieces.size()) {
		this->pieces.push_back(new_piece);
	} else {
		this->pieces[this->piece_idx] = new_piece;
	}
}

void Buffer::set_point(PointOffset pos) {
	this->_cut_piece();
	this->point = pos;
}

void Buffer::backspace() {
	this->active_pieces = this->piece_idx;
	this->contents.erase(this->contents.begin() + this->point.index - 1);
	--this->point.index;
}

void Buffer::insert(uint8_t chr) {
	this->active_pieces = this->piece_idx;
	this->contents.insert(this->contents.begin() + this->point.index, chr);
	++this->point.index;
}

void Buffer::undo() {
	std::cout << this->piece_idx << std::endl;
	std::cout << this->active_pieces << std::endl;

	auto const& current_piece = this->pieces[this->piece_idx];
	std::cout << (char*)current_piece.contents.data() << std::endl;
	this->contents.erase(
		this->contents.begin() + current_piece.start,
		this->contents.begin() + this->point.index
	);

	if (this->piece_idx == 0) {
		this->pieces.insert(this->pieces.begin() + this->piece_idx + 1, current_piece);
		this->pieces[this->piece_idx] = {{}, this->point.index, this->point.index};
	} else {
		--this->piece_idx;
	}
}

void Buffer::redo() {
	std::cout << this->piece_idx << std::endl;
	std::cout << this->active_pieces << std::endl;
	if (this->piece_idx == this->active_pieces - 1) {
		return;
	}

	++this->piece_idx;
	auto const& current_piece = this->pieces[this->piece_idx];
	std::cout << (char*)current_piece.contents.data() << std::endl;
	this->contents.insert(
		this->contents.begin() + current_piece.start,
		current_piece.contents.begin(),
		current_piece.contents.end()
	);
}
