#include "buffer.hh"

#include <fstream>

using namespace buffer;

Buffer buffer::slurp_to_buffer(std::filesystem::path path) {
	std::vector<uint8_t> contents(std::filesystem::file_size(path));
	std::ifstream in(path.native(), std::ios::in | std::ios::binary);
	in.read((char*)contents.data(), contents.size());
	return { contents, path, {} };
}

void Buffer::set_point(PointOffset pos) {
	this->point = pos;
}

void Buffer::backspace() {
	this->contents.erase(this->contents.begin() + this->point.index - 1);
	--this->point.index;
}

void Buffer::insert(uint8_t chr) {
	this->contents.insert(this->contents.begin() + this->point.index, chr);
	++this->point.index;
}
