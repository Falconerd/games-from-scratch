#include <string.h>
#include "../io/io.h"
#include "config.h"
#include "config_internal.h"
#include "../input/input.h"

static char *config_get_value(char *string) {
    char *line = strdup(string);
    char *curr = line;

    while (*curr != '\n' && *curr != 0) {
        ++curr;
    }
    *curr = 0;

    char *delimeter = strstr(line, "= ") ? "= " : "=";
    strtok(line, delimeter);
    char *value = strtok(NULL, delimeter);

    return value;
}

static void load_controls(char *config_buffer) {
    char *left = strstr(config_buffer, "left");
    char *right = strstr(config_buffer, "right");
    char *jump = strstr(config_buffer, "jump");
    char *shoot = strstr(config_buffer, "shoot");

    config_key_bind(INPUT_KEY_LEFT, config_get_value(left));
    config_key_bind(INPUT_KEY_RIGHT, config_get_value(right));
    config_key_bind(INPUT_KEY_JUMP, config_get_value(jump));
    config_key_bind(INPUT_KEY_SHOOT, config_get_value(shoot));
}

static void load_display(Config_State *config_state, char *config_buffer) {
    char *width = strstr(config_buffer, "width");
    char *height = strstr(config_buffer, "height");

    config_state->display_width = (float)atof(config_get_value(width));
    config_state->display_height = (float)atof(config_get_value(height));
}

int config_init_load(Config_State *config_state) {
    char *config_buffer = io_file_read("./config.ini");
    if (!config_buffer) {
        return 1;
    }

    load_controls(config_buffer);
    load_display(config_state, config_buffer);

    return 0;
}

void config_init_create_default(void) {
    char *default_config_file = "[controls]\n"
        "left = A\n"
        "right = D\n"
        "jump = Space\n"
        "shoot = H\n"
        "\n"
        "[display]\n"
        "width = 800\n"
        "height = 600\n";

    io_file_write(default_config_file, strlen(default_config_file), "./config.ini");
}
