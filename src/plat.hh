#pragma once

#if defined(unix) || defined(__unix__) || defined(__unix) || defined(__APPLE__)
#define UNIX
#endif

#include <string>
#include <cstdint>

bool plat_file_exists(std::string const& name);
void plat_mkdir(std::string const& name);
int64_t plat_get_file_size(std::string const& name);
