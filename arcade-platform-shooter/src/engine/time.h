#ifndef TIME_H
#define TIME_H

#include "./types.h"

typedef struct time_state {
	f32 now;
	f32 previous;
	f32 delta;
	f32 last_frame;
	f32 frame_time;
	f32 frame_delay;
	u16 frame_count;
	u16 frame_rate;
} Time_State;

Time_State *time_init(f32 framerate);
void time_update(void);
void time_late_update(void);

#endif

