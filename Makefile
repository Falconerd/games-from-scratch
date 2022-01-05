INCLUDE = -I./deps/include
LIBS = -lSDL2

build: ./src/main.c ./deps/src/glad.c ./io.o ./render.o ./render_init.o ./render_util.o ./input.o ./config.o ./config_init.o
	gcc $(INCLUDE) $(LIBS) $^

io.o: ./src/io/io.c
	gcc -c $^

render.o: ./src/render/render.c ./src/render/render_util.c ./src/render/render_init.c
	gcc -c $(INCLUDE) $^

input.o: ./src/input/input.c
	gcc -c $(INCLUDE) $^

config.o: ./src/config/config.c ./src/config/config_init.c
	gcc -c $^

clean:
	@rm -rf *.o
	@rm -rf a.out

run:
	./a.out