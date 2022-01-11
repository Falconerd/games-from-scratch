FLAGS = -std=c99 -g3 -pedantic -Wall -Wextra -Werror
INCLUDE = -I./deps/include
LIBS = -lSDL2

build: ./src/main.c ./deps/src/glad.c ./io.o ./render.o ./render_init.o ./render_util.o
	gcc $(FLAGS) $(INCLUDE) $(LIBS) $^

io.o: ./src/io/io.c
	gcc $(FLAGS) -c $^

render.o: ./src/render/render.c ./src/render/render_util.c ./src/render/render_init.c
	gcc $(FLAGS) -c $(INCLUDE) $^

clean:
	@rm -rf *.o
	@rm -rf a.out

run:
	./a.out
