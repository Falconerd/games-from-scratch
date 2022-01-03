INCLUDE = -I./deps/include
LIBS = -lSDL2

build: src/main.c deps/src/glad.c
	gcc $(INCLUDE) $(LIBS) $^