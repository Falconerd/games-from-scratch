FLAGS = -g3 -O0 -std=c99 -pedantic -Wall -Wextra
FILES = src/main.c deps/src/glad.c src/render.c src/shared.c src/io.c src/audio.c src/input.c src/entity.c src/physics.c src/sprite.c

ifeq ($(OS), Windows_NT)
	LIBS = -D_REENTRANT -pthread -lm -lSDL2 -lSDL2_mixer -mwindows -lfreetype
#-lmingw32 
	INC = -I./deps/include -L./deps/lib -I/c/msys64/mingw64/include -L/c/msys64/mingw64/bin -L./
else
	LIBS = -D_REENTRANT -pthread -lm -ldl -lSDL2 -lSDL2_mixer -lfreetype
	INC = -I./deps/include -L./deps/lib -I/usr/local/include
endif

build:
	gcc $(FILES) $(FLAGS) $(LIBS) $(INC)
