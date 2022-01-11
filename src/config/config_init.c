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

int config_init_load(void) {
    char *config_buffer = io_file_read("./config.ini");
    if (!config_buffer) {
        return 1;
    }

    char *left = strstr(config_buffer, "left");
    char *right = strstr(config_buffer, "right");
    char *jump = strstr(config_buffer, "jump");
    char *shoot = strstr(config_buffer, "shoot");

    config_key_bind(INPUT_KEY_LEFT, config_get_value(left));
    config_key_bind(INPUT_KEY_RIGHT, config_get_value(right));
    config_key_bind(INPUT_KEY_JUMP, config_get_value(jump));
    config_key_bind(INPUT_KEY_SHOOT, config_get_value(shoot));

    return 0;
}

void config_init_create_default(void) {
    char *default_config_file = "[controls]\n"
        "left = A\n"
        "right = D\n"
        "jump = Space\n"
        "shoot = H\n";
    
    io_file_write(default_config_file, strlen(default_config_file), "./config.ini");
}
