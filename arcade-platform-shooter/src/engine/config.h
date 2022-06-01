#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include "./input.h"
#include "./types.h"

typedef struct config {
	u8 keybinds[5];
	f32 display_width;
	f32 display_height;
	f32 framerate;
} Config_State;

Config_State *config_init(void);
void config_key_bind(Input_Key key, const char *key_name);

#endif

