INCLUDE = -I./deps/include
LIBS = -lSDL2

build: ./src/main.c ./deps/src/glad.c ./io.o ./render.o ./render_init.o ./render_util.o
	gcc $(INCLUDE) $(LIBS) $^

io.o: ./src/io/io.c
	gcc -c $^

render.o: ./src/render/render.c ./src/render/render_util.c ./src/render/render_init.c
	gcc -c $(INCLUDE) $^

clean:
	@rm -rf *.o
	@rm -rf a.out

run:
	./a.out