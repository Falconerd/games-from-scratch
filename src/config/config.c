#include <SDL2/SDL.h>
#include "config.h"
#include "config_internal.h"
#include "../input/input.h"

static Config_State config_state;

Config_State *config_init(void) {
    if (config_init_load() != 0) {
        config_init_create_default();
        config_init();
    }

    return &config_state;
}

void config_key_bind(Input_Key key, const char *key_name) {
    SDL_Scancode scan_code = SDL_GetScancodeFromName(key_name);
    config_state.keybinds[key] = scan_code;
}
