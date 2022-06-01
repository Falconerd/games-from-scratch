#include <SDL2/SDL.h>
#include "../time.h"

static Time_State time_state = {0};

Time_State *time_init(f32 framerate) {
	time_state.frame_delay = 1000.f / framerate;
	return &time_state;
}

void time_update() {
	time_state.now = (f32)SDL_GetTicks();
	time_state.delta = (time_state.now - time_state.last_frame) / 1000.f;
	time_state.last_frame = time_state.now;
	++time_state.frame_count;

	if (time_state.now - time_state.previous >= 1000.f) {
		time_state.frame_rate = time_state.frame_count;
		time_state.frame_count = 0;
		time_state.previous = time_state.now;
	}
}

void time_late_update() {
	time_state.frame_time = (f32)SDL_GetTicks() - time_state.now;

	if (time_state.frame_delay > time_state.frame_time) {
		SDL_Delay(time_state.frame_delay - time_state.frame_time);
	}
}

