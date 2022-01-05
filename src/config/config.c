#include "config.h"
#include "config_internal.h"

static Config config = {0};

Config *config_init() {
    if (config_load(&config) != 0) {
        config_create_default(&config);
    }

    return &config;
}