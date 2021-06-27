#include "shared.h"

void error_and_exit(i32 code, const char *message) {
	fprintf(stderr, "Error: %s\n", message);
	exit(code);
}
