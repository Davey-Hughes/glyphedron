all: term_shapes

term_shapes: src/term_shapes.c
	gcc -std=c11 -Wall -Werror -Wpedantic -O3 -o term_shapes src/timing.c src/term_shapes.c -lm -lncurses -Iinclude

clean:
	rm -rf term_shapes
