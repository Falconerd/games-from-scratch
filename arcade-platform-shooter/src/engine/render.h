#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include "../../deps/lib/linmath.h"
#include "../engine/types.h"

typedef struct texture {
	u32 id;
	i32 width;
	i32 height;
	i32 channel_count;
} Texture;

typedef struct sprite_sheet {
	Texture texture;
	u8 rows;
	u8 columns;
	u8 frame_width;
	u8 frame_height;
} Sprite_Sheet;

SDL_Window *render_init(float width, float height);
void render_quad(vec2 pos, vec2 size, vec4 color);
void render_aabb(void *aabb, vec4 color);
void render_line_segment(vec2 start, vec2 end, vec4 color);
void render_screen_shake_add(f32 duration, f32 magnitude);
void render_screen_shake(f32 delta_time);
void render_text(const char *text, vec2 pos, vec4 color, u8 is_centered);
void render_circle(vec2 pos, f32 radius, vec4 color);
void render_point(vec2 pos, vec4 color);
void render_ray(vec2 start, vec2 direction, f32 length, vec4 color, u8 arrow);
void render_sprite(Texture texture, vec2 size, vec3 pos, f32 tex_coords[8], f32 rotation, vec4 color, u8 is_flipped);
void render_sprite_sheet_frame(Sprite_Sheet sprite_sheet, u8 row, u8 column, vec3 pos, f32 rotation, vec4 color, u8 is_flipped);
Texture render_texture_create(const char *path);

#endif

