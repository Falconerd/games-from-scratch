@echo off
setlocal

set CC=gcc
set CFLAGS=-g3 -O0 -std=c99 -Wall -Wextra -pedantic
set INCLUDES=-Ideps/include
set LIBS=-lm -lSDL2 -lSDL2_mixer -lfreetype -mwindows
set OUT=game.exe

%CC% %CFLAGS% %INCLUDES% ^
    src/main.c ^
    src/shared.c ^
    src/render.c ^
    src/input.c ^
    src/entity.c ^
    src/physics.c ^
    src/sprite.c ^
    src/audio.c ^
    src/util.c ^
    src/engine/io/io.c ^
    src/engine/config/config.c ^
    src/engine/config/config_init.c ^
    deps/src/glad.c ^
    -o %OUT% ^
    %LIBS%

if %ERRORLEVEL% EQU 0 (
    echo Built %OUT%
) else (
    echo Build failed
    exit /b 1
)
