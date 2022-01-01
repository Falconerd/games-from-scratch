FLAGS = -g3 -O0 -std=c99
INC = -I./deps/include -L./deps/lib -I/usr/local/include
FILES = src/main.c deps/src/glad.c src/render.c src/shared.c src/io.c src/audio.c src/input.c src/entity.c src/physics.c src/sprite.c

ifeq ($(OS), Windows_NT)
	LIBS = -D_REENTRANT -pthread -lm -lmingw32 -lSDL2 -lSDL2_mixer -mwindows -lfreetype
else
	LIBS = -D_REENTRANT -pthread -lm -ldl -lSDL2 -lSDL2_mixer -lfreetype
endif

build:
	gcc $(FILES) $(FLAGS) $(LIBS) $(INC)