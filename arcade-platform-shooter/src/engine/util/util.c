#include "../util.h"

float fsignf(float x) {
	if (x > 0.f) return 1.f;
	if (x < 0.f) return -1.f;
	return 0.f;
}

float fminf(float a, float b) {
	if (a < b) return a;
	return b;
}

float fmaxf(float a, float b) {
	if (a > b) return a;
	return b;
}

float fclampf(float x, float min, float max) {
	return fminf(fmaxf(x, min), max);
}

