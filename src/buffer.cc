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
