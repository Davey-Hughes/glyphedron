all: term_shapes

term_shapes: src/term_shapes.c include/term_shapes.h src/timing.c include/timing.h
	gcc -ggdb3 -std=c11 -Wall -Werror -Wpedantic -O3 -o term_shapes src/timing.c src/term_shapes.c -lm -lncurses -Iinclude

clean:
	rm -rf term_shapes
