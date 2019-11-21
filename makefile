all: term_shapes

sources=src/vector.c src/timing.c src/convex_occlusion.c src/occlude_approx.c src/print.c src/init.c src/transform.c src/term_shapes.c

term_shapes: src/term_shapes.c include/term_shapes.h src/timing.c include/timing.h
	gcc -std=c11 -Wall -Werror -Wpedantic -O3 -o term_shapes $(sources) -lm -lncurses -Iinclude

clean:
	rm -rf term_shapes log.txt
