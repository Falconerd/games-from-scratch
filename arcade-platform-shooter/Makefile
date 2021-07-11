flags = -Wall -pedantic -std=c99
libs = -lX11 -lglfw -ldl -lm
inc = -I./deps/include

build: src/main.c glad.o render.o shared.o io.o entity.o physics.o
	gcc -g -ggdb $(flags) $(libs) $(inc) $^

glad.o: deps/src/glad.c
	gcc -c -g -ggdb $(inc) $^

render.o: src/render.c
	gcc -c -g -ggdb $(inc) $^

shared.o: src/shared.c
	gcc -c -g -ggdb $(inc) $^

io.o: src/io.c
	gcc -c -g -ggdb $(inc) $^

entity.o: src/entity.c
	gcc -c -g -ggdb $(inc) $^

physics.o: src/physics.c
	gcc -c -g -ggdb $(inc) $^

run:
	./a.out

clean:
	@rm -f ./a.out
	@rm -f ./*.o
	@rm -f ./*.obj
