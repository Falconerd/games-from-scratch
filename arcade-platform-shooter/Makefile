flags = -Wall -pedantic -std=c99
libs = -lX11 -lglfw -ldl -lm
inc = -I./deps/include

build: src/main.c glad.o render.o shared.o io.o entity.o physics.o
	gcc -g3 -O0 $(flags) $(libs) $(inc) $^

glad.o: deps/src/glad.c
	gcc -c -g3 -O0 $(inc) $^

render.o: src/render.c
	gcc -c -g3 -O0 $(inc) $^

shared.o: src/shared.c
	gcc -c -g3 -O0 $(inc) $^

io.o: src/io.c
	gcc -c -g3 -O0 $(inc) $^

entity.o: src/entity.c
	gcc -c -g3 -O0 $(inc) $^

physics.o: src/physics.c
	gcc -c -g3 -O0 $(inc) $^

run:
	./a.out

clean:
	@rm -f ./a.out
	@rm -f ./*.o
	@rm -f ./*.obj
