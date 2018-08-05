all: term-shapes

term-shapes: term-shapes.c
	gcc -Wall -Werror -Wpedantic -O3 -o term-shapes term-shapes.c -lm -lncurses

clean:
	rm -rf term-shapes
