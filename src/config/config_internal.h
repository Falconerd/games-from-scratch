#ifndef CONFIG_INTERNAL_H
#define CONFIG_INTERNAL_H

#include "config.h"

int config_init_load(Config_State *config_state);
void config_init_create_default(void);

#endif