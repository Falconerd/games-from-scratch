#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include "input.h"

typedef struct config {
	uint8_t keybinds[4];
	float display_width;
	float display_height;
	float framerate;
} Config;

Config *config_init(void);
void config_key_bind(Input_Key key, const char *key_name);

#endif

