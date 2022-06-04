#ifndef UTIL_H
#define UTIL_H

#include <stdlib.h>

float fsignf(float x);
float fminf(float a, float b);
float fmaxf(float a, float b);
float fclampf(float x, float min, float max);

#define WHITE  (vec4){1, 1, 1, 1}
#define RED    (vec4){1, 0, 0, 1}
#define GREEN  (vec4){0, 1, 0, 1}
#define BLUE   (vec4){0, 0, 1, 1}
#define YELLOW (vec4){1, 1, 0, 1}
#define PINK   (vec4){1, 0, 1, 1}

#endif

