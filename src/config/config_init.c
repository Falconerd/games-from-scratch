#include <string.h>
#include "../io/io.h"
#include "config.h"
#include "config_internal.h"
#include "../input/input.h"

static void config_get_value(char *result, char *string) {
    char *start = NULL;
    size_t nbytes = 0;
    char *curr = string;

    while (*curr != '\n' && *curr != '\r') {
        if (*curr == '=') {
            curr += 2;
            start = curr;
            continue;
        }

        if (start) {
            ++nbytes;
        }

        ++curr;
    }

    memcpy(result, start, nbytes);
    result[nbytes] = 0;
}

int config_init_load(Config *config) {
    char *config_buffer = io_file_read("./config.ini");

    if (!config_buffer) {
        return 1;
    }

    char *left = strstr(config_buffer, "left");
    char *right = strstr(config_buffer, "right");
    char *jump = strstr(config_buffer, "jump");
    char *shoot = strstr(config_buffer, "shoot");

    char buffer[10] = {0};

    config_get_value(buffer, left);
    input_key_bind(INPUT_KEY_LEFT, buffer);

    config_get_value(buffer, right);
    input_key_bind(INPUT_KEY_RIGHT, buffer);

    config_get_value(buffer, jump);
    input_key_bind(INPUT_KEY_JUMP, buffer);

    config_get_value(buffer, shoot);
    input_key_bind(INPUT_KEY_SHOOT, buffer);

    return 0;
}

void config_init_create_default(Config *config) {
    char *default_config_file = "[controls]\n"
        "left = A\n"
        "right = D\n"
        "jump = Space\n"
        "shoot = H\n";
    
    io_file_write(default_config_file, strlen(default_config_file), "./config.ini");
}