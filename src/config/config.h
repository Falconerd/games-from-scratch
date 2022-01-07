#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include "../input/input.h"

typedef struct config_state {
    uint8_t keybinds[4];
} Config_State;

Config_State *config_init(void);
void config_key_bind(Input_Key key, const char *key_name);

#endif