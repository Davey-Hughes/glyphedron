/*
 *     Copyright (C) 2018  Davey Hughes
 *
 *     This program is free software: you can redistribute it and/or modify it
 *     under the terms of the GNU General Public License as published by the
 *     Free Software Foundation, either version 3 of the License, or (at your
 *     option) any later version.
 *
 *     This program is distributed in the hope that it will be useful, but
 *     WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *     General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License along
 *     with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#define _POSIX_C_SOURCE 199309L

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

#if TIMING
#include "timing.h"
#endif

#include "term_shapes.h"
#include "vector.h"
#include "convex_occlusion.h"
#include "occlude_approx.h"
#include "print.h"
#include "transform.h"
#include "init.h"


/*
 * chooses which occlusion method to use based on the s.occlusion enum
 *
 * returns 0 if point should be rendered, else 1
 */
int
occlude_point(struct shape *s, point3 point, struct edge edge)
{
	switch (s->occlusion) {
	case NONE:
		return 1;

	case APPROX:
		return occlude_point_approx(s, point);

	case CONVEX:
		return occlude_point_convex(s, point, edge);

	case CONVEX_CLEAR:
		return occlude_point_convex(s, point, edge);

	case EXACT: /* not implemented */
		return 0;
	}

	return 0;
}


/*
 * loop which re-prints the shape with every keypress, and checks for certain
 * keyboard input to determine functions to run on the shape
 */
static
void
loop(struct shape *s)
{
	int c;
	double theta, dist, scale;

#if USE_NCURSES
	char *occlusion_type = "";
#endif

#if TIMING
	struct timespec start, end, diff, avg_op, avg_print;

	avg_op.tv_sec = -1;
	avg_op.tv_nsec = -1;

	avg_print.tv_sec = -1;
	avg_print.tv_nsec = -1;
#endif

#if USE_NCURSES
	/* start ncurses mode */
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);
#endif

	theta = M_PI / 200;
	dist = 0.1;
	scale = 1.1;

#if TIMING
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
#endif

	while (1) {

#if USE_NCURSES
		wclear(stdscr);

		switch (s->occlusion) {
		case NONE:
			occlusion_type = "none";
			break;

		case APPROX:
			occlusion_type = "approximate";
			break;

		case CONVEX:
			occlusion_type = "convex";
			s->front_symbol = '.';
			s->rear_symbol = '\0';
			break;
		case CONVEX_CLEAR:
			occlusion_type = "convex_clear";
			s->front_symbol = 'o';
			s->rear_symbol = '.';
			break;
		case EXACT:
			occlusion_type = "exact not implemented";
			s->front_symbol = 'o';
			break;
		}
		mvprintw(1, 1, "Occlusion type: %s", occlusion_type);
#endif


#if TIMING
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
		timespec_diff(&start, &end, &diff);

# if USE_NCURSES
		mvprintw(3, 1, "Operation time: %ld.%06ld seconds",
			diff.tv_sec, diff.tv_nsec / 1000);
# endif

		timespec_avg(&avg_op, &diff, &avg_op);

		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
#endif

		print_shape(s);

#if TIMING
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);

		timespec_diff(&start, &end, &diff);

# if USE_NCURSES
		mvprintw(2, 1, "Print time: %ld.%06ld seconds",
			diff.tv_sec, diff.tv_nsec / 1000);
# endif

		timespec_avg(&avg_print, &diff, &avg_print);
#endif


#if USE_NCURSES
		c = getch();
#else
		c = getchar();
#endif

#if TIMING
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
#endif

		switch(c) {
		case 'q':

#if USE_NCURSES
			/* end ncurses mode */
			endwin();
#endif

#if TIMING
			printf("Average operation time: %ld.%06ld seconds\n",
				avg_op.tv_sec, avg_op.tv_nsec / 1000);

			printf("Average print time: %ld.%06ld seconds\n",
				avg_print.tv_sec, avg_print.tv_nsec / 1000);
#endif

			return;

		/* ** ROTATIONS ** */
		/* rotate around z axis */
		case 't':
			rotate_shape(theta, 'z', s);
			break;
		case 'y':
			rotate_shape(-theta, 'z', s);
			break;

		/* rotate around x axis */
		case 'u':
			rotate_shape(theta, 'x', s);
			break;
		case 'i':
			rotate_shape(-theta, 'x', s);
			break;

		/* rotate around y axis */
		case 'o':
			rotate_shape(-theta, 'y', s);
			break;
		case 'p':
			rotate_shape(theta, 'y', s);
			break;

		/* ** SCALE ** */
		case '=':
			scale_shape(scale, s);
			break;

		case '-':
			scale_shape(1.0 / scale, s);
			break;


		/* ** TRANSLATIONS ** */
		/* translate along x axis */
		case 'h':
			translate_shape(-dist, 'x', s);
			break;
		case 'l':
			translate_shape(dist, 'x', s);
			break;


		/* translate along y axis */
		case 'j':
			translate_shape(-dist, 'y', s);
			break;

		case 'k':
			translate_shape(dist, 'y', s);
			break;

		/* translate along z axis */
		case 'f':
			translate_shape(-dist, 'z', s);
			break;
		case 'g':
			translate_shape(dist, 'z', s);
			break;

#if USE_NCURSES
		/* AUTOROTATE */
		case 'a':
			autorotate(s);
			break;
#endif

		/* RESET */
		case 'r':
			reset_shape(s);
			break;

		/* flip printing of vertices */
		case '1':
			s->print_vertices = !(s->print_vertices);
			break;

		/* flip printing of edges */
		case '2':
			s->print_edges = !(s->print_edges);
			break;

		/* turn occlusion on or off */
		case '3':
			s->occlusion++;
			if (s->occlusion > EXACT) {
				s->occlusion = NONE;
			}

			/*
			 * because the icosahedron and dodecahedron are put in
			 * using an approximation of phi, do a quick set of
			 * rotations to fix some apparent floating point
			 * precision weirdness with occlusion
			 */

			if (s->occlusion == CONVEX) {
				rotate_shape(theta, 'x', s);
				rotate_shape(theta, 'y', s);
				rotate_shape(theta, 'z', s);

				rotate_shape(-theta, 'x', s);
				rotate_shape(-theta, 'y', s);
				rotate_shape(-theta, 'z', s);
			}

			break;

		/* **CHANGE EDGE DENSITY** */
		/* increase edge density */
		case '0':
			s->e_density++;
			resize_points_to_print(s);
			break;

		/* decrease edge density */
		case '9':
			if (s->e_density > 0) {
				s->e_density--;
				resize_points_to_print(s);
			}
			break;

		default:
			continue;
		}
	}
}

int
main(int argc, char **argv)
{
	int err;
	struct shape s;

	if (argc > 1) {
		err = init_from_file(argv[1], &s);
	} else {
		err = init_cube(&s);
	}
	if (err != 0) {
		printf("error allocating shape\n");
		exit(1);
	}

	loop(&s);

	destroy_shape(&s);

	return 0;
}
