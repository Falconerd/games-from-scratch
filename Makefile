FLAGS = -std=c99 -g3 -pedantic -Wall -Wextra -Werror
INCLUDE = -I./deps/include
LIBS = -lSDL2

build: src/main.c deps/src/glad.c
	gcc $(FLAGS) $(INCLUDE) $(LIBS) $^
