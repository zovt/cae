#pragma once

#include <cstdint>
#include <filesystem>

#include "common_state.hh"

namespace buffer {

struct Buffer {
	// TODO: Use a more optimized data structure to hold buffer contents
	std::vector<uint8_t> contents;
	std::filesystem::path path;
};

Buffer slurp_to_buffer(std::filesystem::path path);

}
