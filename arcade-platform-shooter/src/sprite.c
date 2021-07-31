#include "shared.h"

Animation_State animation_state = {0};
static Animation_State *state = &animation_state;

// What exact data is required for playing a sprite animation??
// - texture id
// - texture coordinates of frame
// - how long the frame lasts
// - the next frame or
//   - an array of frames
//   - the current index
//   
// In this game, sprites in an animation are all the same size.
// With that in mind, the texture coordinates could be calculated.
// - texture width
// - texture height
// - rows
// - columns
// - frame width
// - frame height
// - index or x, y coordinates (in the array)

// render function could look like this:
// void render_sprite_frame(Sprite_Sheet sprite_sheet, u8 row, u8 column)
// storage could be something like this:

// For the animation part, timings and a sequence are required.
// Should animations handle themselves or be explicitly managed in
// game code?
// Perhaps a simple current animation state could suffice?
// The animation will play and stop or continue to repeat if looping.
// Since sprite sheets exist, sequences can be defined as such:
// A sprite sheet id or pointer, an array of frame coordinates, an
// array of frame times.

u32 sprite_sheet_create(Texture texture, f32 frame_width, f32 frame_height) {
	u32 index = state->sprite_sheet_array_count++;
	if (index == MAX_SPRITE_SHEETS)
		error_and_exit(-1, "No more space for sprite sheets.");

	Sprite_Sheet *ss = &state->sprite_sheet_array[index];
	ss->texture = texture;
	ss->frame_width = frame_width;
	ss->frame_height = frame_height;
	ss->rows = (u8)(texture.width / frame_width);
	ss->columns = (u8)(texture.height / frame_height);

	return index;
}

u32 sprite_animation_create(u32 sprite_sheet_id, u8 length, u8 *row_coordinate_array, u8 *column_coordinate_array, f32 *frame_time_array) {
	u32 index = state->sprite_animation_array_count++;
	if (index == MAX_SPRITE_ANIMATIONS)
		error_and_exit(-1, "No more space for sprite animations.");

	Sprite_Animation *sa = &state->sprite_animation_array[index];
	sa->sprite_sheet_id = sprite_sheet_id;
	sa->length = length;
	sa->current_frame = 0;
	memcpy(sa->row_coordinate_array, row_coordinate_array, length * sizeof(u8));
	memcpy(sa->column_coordinate_array, column_coordinate_array, length * sizeof(u8));
	memcpy(sa->frame_time_array, frame_time_array, length * sizeof(f32));
	return index;
}

void sprite_animation_tick(f32 delta_time) {
	for (u32 i = 0; i < state->sprite_animation_array_count; ++i) {
		Sprite_Animation *sa = &state->sprite_animation_array[i];

		sa->current_frame_time -= delta_time;

		// Switch to next frame.
		if (sa->current_frame_time <= 0) {
			// Reset to first frame.
			if (sa->current_frame + 1 == sa->length && sa->does_loop) {
				sa->current_frame = 0;
				sa->current_frame_time = sa->frame_time_array[0];
				continue;
			}

			++sa->current_frame;
			sa->current_frame_time = sa->frame_time_array[sa->current_frame];
		}
	}
}
