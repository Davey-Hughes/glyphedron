#include <stdlib.h>
#include <math.h>
#include <ncurses.h>

#include "vector.h"
#include "transform.h"
#include "init.h"
#include "print.h"
#include "term_shapes.h"

/*
 * rotates each point in one of 6 directions, given the angle (can be positive
 * or negative) and the axis around which to rotate
 */
void
rotate_shape(double theta, char axis, struct shape *s)
{
	int i;
	double *fst, *snd;
	double presin, precos, cfst, csnd, tempfst, tempsnd;

	/* get center values and test whether axis in 'xyz' */
	switch (axis) {
	case 'x':
		cfst = s->center.y;
		csnd = s->center.z;
		break;
	case 'y':
		cfst = s->center.z;
		csnd = s->center.x;
		break;
	case 'z':
		cfst = s->center.x;
		csnd = s->center.y;
		break;
	default:
		return;
	}

	/* precompute sin theta and cos theta */
	presin = sin(theta);
	precos = cos(theta);

	/* change every point */
	for (i = 0; i < s->num_v; ++i) {
		switch (axis) {
		case 'x':
			fst = &(s->vertices[i].y);
			snd = &(s->vertices[i].z);
			break;
		case 'y':
			fst = &(s->vertices[i].z);
			snd = &(s->vertices[i].x);
			break;
		case 'z':
			fst = &(s->vertices[i].x);
			snd = &(s->vertices[i].y);
			break;
		default: /* shouldn't get here */
			exit(1);
		}

		tempfst = *fst - cfst;
		tempsnd = *snd - csnd;

		*fst = tempfst * precos - tempsnd * presin;
		*snd = tempsnd * precos + tempfst * presin;

		*fst += cfst;
		*snd += csnd;
	}
}

/*
 * scale the object by a given magnitude
 */
void
scale_shape(double mag, struct shape *s)
{
	int i;

	for (i = 0; i < s->num_v; ++i) {
		vector3_sub(&(s->vertices[i]), &(s->center), &(s->vertices[i]));
		vector3_mult(&(s->vertices[i]), mag, &(s->vertices[i]));
		vector3_add(&(s->vertices[i]), &(s->center), &(s->vertices[i]));
	}
}

/*
 * translates each vertex given a distance and the axis which to translate
 * along
 */
void
translate_shape(double dist, char axis, struct shape *s)
{
	int i;

	switch (axis) {
	case 'x':
		s->center.x += dist;
		break;
	case 'y':
		s->center.y += dist;
		break;
	case 'z':
		s->center.z += dist;
		break;
	default:
		return;
	}

	for (i = 0; i < s->num_v; ++i) {
		switch (axis) {
		case 'x':
			s->vertices[i].x += dist;
			printw("%f\n", s->vertices[i].y);
			break;
		case 'y':
			s->vertices[i].y += dist;
			break;
		case 'z':
			s->vertices[i].z += dist;
			break;
		default: /* shouldn't get here */
			return;
		}
	}
}

/* resize the points_to_print array. */
/* defaults to resetting the whole shape if malloc fails */
int
resize_points_to_print(struct shape *s)
{
	free(s->fronts);
	s->fronts = malloc(sizeof(struct point_to_print) * (s->num_e + 1) * (s->e_density + 1));
	if (s->fronts == NULL) {
		reset_shape(s);
		return 1;
	}

	free(s->behinds);
	s->behinds = malloc(sizeof(struct point_to_print) * (s->num_e + 1) * (s->e_density + 1));
	if (s->behinds == NULL) {
		reset_shape(s);
		return 1;
	}

	return 0;
}

#if USE_NCURSES
void
autorotate(struct shape *s)
{
	char c;

	mvprintw(1, 1, "Please enter 'a' for automatic rotate or 'm' for manual angle.");
	c = getch();

	if (c == 'm') {
		curs_set(1);
		echo();
		wclear(stdscr);

		mvprintw(1, 1, "Please enter x angle: ");
		scanw("%lf", &(s->dir.x));
		mvprintw(2, 1, "Please enter y angle: ");
		scanw("%lf", &(s->dir.y));
		mvprintw(3, 1, "Please enter z angle: ");
		scanw("%lf", &(s->dir.z));

		noecho();
		curs_set(0);
	} else {
		s->dir.x = M_PI / 80;
		s->dir.y = M_PI / 120;
		s->dir.z = M_PI / 60;

	}

	s->interval.tv_sec = 0;
	s->interval.tv_nsec = 60000000;

	nodelay(stdscr, TRUE);

	while (1) {
		wclear(stdscr);
		print_shape(s);
		mvprintw(1, 1, "%f %f %f", s->dir.x, s->dir.y, s->dir.z);

		c = getch();
		if (c == 'a') {
			break;
		}

		rotate_shape(s->dir.x, 'x', s);
		rotate_shape(s->dir.y, 'y', s);
		rotate_shape(s->dir.z, 'z', s);

		nanosleep(&s->interval, NULL);
	}

	nodelay(stdscr, FALSE);
}
#endif
