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

// https://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value
float fclampf(float x, float min, float max) {
	const float t = x < min ? min : x;
	return t > max ? max : t;
}

