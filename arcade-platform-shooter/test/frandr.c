#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

float frandr(float min, float max) {
	float r = (rand() % 100) / 100.0f;
	return r * (max - min) + min;
}

int main(void) {
	srand(0);
	for (int i = 0; i < 1000; ++i) {
		float min = -(float)(rand() % 100);
		float max = (float)(rand() % 100);
		float r = frandr(min, max);
		printf("%f - %f, %f\n", min, max, r);
		assert(r >= min && r <= max);
	}
}
