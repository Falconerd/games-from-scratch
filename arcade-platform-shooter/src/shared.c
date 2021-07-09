#include "shared.h"

void error_and_exit(i32 code, const char *message) {
	fprintf(stderr, "Error: %s\n", message);
	exit(code);
}

f32 fsign(f32 a) {
	if (a > 0.0f) return 1.0f;
	if (a < 0.0f) return -1.0f;
	return 0.0f;
}
