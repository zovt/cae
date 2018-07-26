#include "plat.hh"

#ifdef UNIX
#include <sys/stat.h>
#include <cstdlib>

bool plat_file_exists(std::string const& name) {
	struct stat buf;
	return stat(name.c_str(), &buf) == 0;
}

void plat_mkdir(std::string const& name) {
	// FIXME: dirty hack
	std::string const command = std::string("mkdir -p '") + name + "'; exit";
	system(command.c_str());
}

int64_t plat_get_file_size(std::string const& name) {
	struct stat buf;
	int rc = stat(name.c_str(), &buf);
	return rc == 0 ? buf.st_size : -1;
}
#endif

