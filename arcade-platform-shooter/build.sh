#!/bin/sh
set -e

CC="${CC:-gcc}"
CFLAGS="-g3 -O0 -std=c99 -Wall -Wextra -pedantic"
INCLUDES="-Ideps/include"
LIBS="-lm -ldl -lpthread -lSDL2 -lSDL2_mixer -lfreetype"
OUT="game"

$CC $CFLAGS $INCLUDES \
    src/main.c \
    src/shared.c \
    src/render.c \
    src/input.c \
    src/entity.c \
    src/physics.c \
    src/sprite.c \
    src/audio.c \
    src/util.c \
    src/engine/io/io.c \
    src/engine/config/config.c \
    src/engine/config/config_init.c \
    deps/src/glad.c \
    -o $OUT \
    $LIBS

echo "Built $OUT"
