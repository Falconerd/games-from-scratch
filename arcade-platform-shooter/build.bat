@echo off
setlocal

set CC=cl
set CFLAGS=/nologo /Zi /Od /TC /W4
set INCLUDES=/Ideps/include
set LIBS=SDL2.lib SDL2main.lib SDL2_mixer.lib opengl32.lib shell32.lib user32.lib gdi32.lib winmm.lib imm32.lib ole32.lib oleaut32.lib version.lib setupapi.lib
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
    /Fe:%OUT% ^
    /link /LIBPATH:deps/lib %LIBS% /SUBSYSTEM:CONSOLE

if %ERRORLEVEL% EQU 0 (
    echo Built %OUT%
) else (
    echo Build failed
    exit /b 1
)
