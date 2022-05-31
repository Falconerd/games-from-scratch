FLAGS = -g3 -O0 -std=c99 -pedantic -Wall -Wextra
#FILES = src/main.c deps/src/glad.c src/render.c src/shared.c src/audio.c src/input.c src/entity.c src/physics.c src/sprite.c

ifeq ($(OS), Windows_NT)
	LIBS = -D_REENTRANT -pthread -lm -lSDL2 -lSDL2_mixer -mwindows -lfreetype
	INC = -I./deps/include -L./deps/lib -I/c/msys64/mingw64/include -L/c/msys64/mingw64/bin -L./
else
	LIBS = -D_REENTRANT -pthread -lm -ldl -lSDL2 -lSDL2_mixer -lfreetype
	INC = -I./deps/include -L./deps/lib -I/usr/local/include
endif

build: src/main.c ./deps/src/glad.c ./io.o ./render.o ./render_init.o ./render_util.o ./input.o ./config.o ./config_init.o ./util.o ./physics.o ./physics_init.o ./time.o ./entity.o
	gcc $(FLAGS) $(LIBS) $(INC) $^

io.o: ./src/engine/io/io.c
	gcc $(FLAGS) -c $^

render.o: ./src/engine/render/render.c ./src/engine/render/render_util.c ./src/engine/render/render_init.c
	gcc $(FLAGS) -c $(INCLUDE) $^

input.o: ./src/engine/input/input.c
	gcc $(FLAGS) -c $(INCLUDE) $^

config.o: ./src/engine/config/config.c ./src/engine/config/config_init.c
	gcc $(FLAGS) -c $^

util.o: ./src/engine/util/util.c
	gcc $(FLAGS) -c $^

physics.o: ./src/engine/physics/physics.c ./src/engine/physics/physics_init.c
	gcc $(FLAGS) -c $(INCLUDE) $^

time.o: ./src/engine/time/time.c
	gcc $(FLAGS) -c $(INCLUDE) $^

entity.o: ./src/engine/entity/entity.c
	gcc $(FLAGS) -c $(INCLUDE) $^

clean:
	@rm -rf ./*.o
	@rm -rf ./a.out ./a.exe
