all: term-shapes

term-shapes: src/term_shapes.c
	gcc -std=c11 -Wall -Werror -Wpedantic -O3 -o term_shapes src/term_shapes.c -lm -lncurses -Iinclude

clean:
	rm -rf term-shapes
