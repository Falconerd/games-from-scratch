FLAGS = -std=c99 -g3 -pedantic -Wall -Wextra -Werror
INCLUDE = -I./deps/include
LIBS = -lSDL2

build: ./src/main.c ./deps/src/glad.c ./io.o ./render.o ./render_init.o ./render_util.o ./input.o ./config.o ./config_init.o
	gcc $(FLAGS) $(INCLUDE) $(LIBS) $^

io.o: ./src/io/io.c
	gcc $(FLAGS) -c $^

render.o: ./src/render/render.c ./src/render/render_util.c ./src/render/render_init.c
	gcc $(FLAGS) -c $(INCLUDE) $^

input.o: ./src/input/input.c
	gcc $(FLAGS) -c $(INCLUDE) $^

config.o: ./src/config/config.c ./src/config/config_init.c
	gcc $(FLAGS) -c $^

clean:
	@rm -rf *.o
	@rm -rf a.out

run:
	./a.out
