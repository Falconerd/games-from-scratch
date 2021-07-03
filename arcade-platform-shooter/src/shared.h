#ifndef src_shared_h_INCLUDED
#define src_shared_h_INCLUDED
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>


#include "../deps/lib/linmath.h"

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define f32 float
#define f64 double
#define i32 int32_t
#define m4 mat4x4

typedef enum direction {
	LEFT   = 1,
	RIGHT  = 2,
	TOP    = 4,
	BOTTOM = 8
} DIRECTION;

void error_and_exit(i32 code, const char *message);

#endif

