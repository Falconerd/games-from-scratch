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

f32 fclamp(f32 a, f32 min, f32 max) {
	if (a < min) return min;
	if (a > max) return max;
	return a;
}

f32 frandr(f32 min, f32 max) {
	f32 r = (rand() % 100) / 100.0f;
	return r * (max - min) + min;
}

f32 vec2_dist(vec2 a, vec2 b) {
	f32 dx = b[0] - a[0];
	f32 dy = b[1] - a[1];
	return sqrtf(dx * dx + dy * dy);
}

f32 vec2_sqr_dist(vec2 a, vec2 b) {
	f32 dx = b[0] - a[0];
	f32 dy = b[1] - a[1];
	return dx * dx + dy * dy;
}
