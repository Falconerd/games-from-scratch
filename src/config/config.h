#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

typedef struct config {
    uint32_t key_left;
    uint32_t key_right;
    uint32_t key_jump;
    uint32_t key_shoot;
} Config;

Config *config_init();

#endif