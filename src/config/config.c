#include "config.h"
#include "config_internal.h"

static Config config = {0};

Config *config_init() {
    if (config_init_load(&config) != 0) {
        config_init_create_default(&config);
        config_init();
    }

    return &config;
}