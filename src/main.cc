#include <stdio.h>
#include <stdlib.h>

#include "plat.hh"
#include "str.hh"

Err run() {
	stack_buf(path, char, 500);
	check_err(get_config_path(&path));
	fprintf(stdout, "%s\n", str_mut_to_c_str(path.data));

	return Err::OK;
}

int main(int argc, char* argv[]) {
	(void)argc;
	(void)argv;

	Err e = run();
	if (e != Err::OK) {
		fprintf(stdout, "Error encountered while running: %d. See errs.hh for details\n", (int)e);
	}
	return 0;
}
