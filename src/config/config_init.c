#include <string.h>
#include "../io/io.h"
#include "config.h"
#include "config_internal.h"

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

int config_load(Config *config) {
    const char *config_buffer = io_file_read("./config.ini");

    if (!config_buffer) {
        return 1;
    }

    char *left = strstr(config_buffer, "left");
    char *right = strstr(config_buffer, "right");
    char *jump = strstr(config_buffer, "jump");
    char *shoot = strstr(config_buffer, "shoot");

    char buffer[10] = {0};
    config_get_value(buffer, left);
    printf("left: %s\n", buffer);
    config_get_value(buffer, right);
    printf("right: %s\n", buffer);
    config_get_value(buffer, jump);
    printf("jump: %s\n", buffer);
    config_get_value(buffer, shoot);
    printf("shoot: %s\n", buffer);
}

void config_create_default(Config *config) {}