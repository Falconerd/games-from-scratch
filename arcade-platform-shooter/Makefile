flags = -Wall -pedantic -std=c99
libs = -D_REENTRANT -pthread -lm -ldl -lSDL2 -lfreetype
inc = -I./deps/include -I/usr/include/freetype2

build: main.o glad.o render.o shared.o io.o input.o entity.o physics.o
	clang -g3 -O0 $(flags) $(libs) $(inc) $^

main.o: src/main.c
	clang -c -g3 -O0 $(inc) $^

glad.o: deps/src/glad.c
	clang -c -g3 -O0 $(inc) $^

render.o: src/render.c
	clang -c -g3 -O0 $(inc) $^

shared.o: src/shared.c
	clang -c -g3 -O0 $(inc) $^

io.o: src/io.c
	clang -c -g3 -O0 $(inc) $^

input.o: src/input.c
	clang -c -g3 -O0 $(inc) $^

entity.o: src/entity.c
	clang -c -g3 -O0 $(inc) $^

physics.o: src/physics.c
	clang -c -g3 -O0 $(inc) $^

run:
	./a.out

clean:
	@rm -f ./a.out
	@rm -f ./*.o
	@rm -f ./*.obj
