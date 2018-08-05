#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>

struct point3 {
	double x;
	double y;
	double z;
};

struct shape {
	int num_v; /* number of vertices */

	struct point3 center; /* center of the shape */
	struct point3 *vertices;

	char *fname; /* file name of the shape coordinates */
};

int
init_from_file(char *fname, struct shape *c)
{
	int cap;
	double x, y, z;

	FILE *file;

	file = fopen(fname, "r");
	if (file == NULL) {
		printf("could not open file\n");
		return -1;
	}

	c->num_v = 0;
	cap = 1;
	c->vertices = malloc(sizeof(struct point3) * cap);
	if (c->vertices == NULL) {
		return -1;
	}

	while(fscanf(file, "%lf, %lf, %lf", &x, &y, &z) != EOF) {
		if (c->num_v == cap) {
			cap *= 2;
			c->vertices = realloc(c->vertices, sizeof(struct point3) * cap);
			if (c->vertices == NULL) {
				goto cleanup;
			}
		}

		c->vertices[c->num_v].x = x;
		c->vertices[c->num_v].y = y;
		c->vertices[c->num_v].z = z;

		c->num_v++;
	}

	c->num_v = c->num_v;
	c->center = (struct point3) {0.0, 0.0, 0.0};
	c->fname = fname;

	return 0;

cleanup:
	free(c->vertices);
	return -1;
}

int
init_cube(struct shape *c)
{
	c->vertices = malloc(sizeof(struct point3) * 8);
	if (c->vertices == NULL) {
		return -1;
	}

	c->vertices[0] = (struct point3) {1.0, 1.0, 1.0};
	c->vertices[1] = (struct point3) {-1.0, 1.0, 1.0};
	c->vertices[2] = (struct point3) {1.0, -1.0, 1.0};
	c->vertices[3] = (struct point3) {-1.0, -1.0, 1.0};
	c->vertices[4] = (struct point3) {1.0, 1.0, -1.0};
	c->vertices[5] = (struct point3) {-1.0, 1.0, -1.0};
	c->vertices[6] = (struct point3) {1.0, -1.0, -1.0};
	c->vertices[7] = (struct point3) {-1.0, -1.0, -1.0};

	c->num_v = 8;
	c->center = (struct point3) {0.0, 0.0, 0.0};
	c->fname = NULL;

	return 0;
}

void
print_shape(struct shape s)
{
	int winx, winy, i;
	double x, y, scale;

	getmaxyx(stdscr, winy, winx);
	scale = 0.4;

	for (i = 0; i < s.num_v; ++i) {
		x = s.vertices[i].x;
		y = s.vertices[i].y;

		mvprintw((int) ((y * scale * .5 * winy) + (0.5 * winy)),
			 (int) ((x * scale * winy) + (0.5 * winx)), "%c", 'o');
	}

}

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

void
scale_shape(double mag, struct shape *s)
{
	int i;

	for (i = 0; i < s->num_v; ++i) {
		s->vertices[i].x -= s->center.x;
		s->vertices[i].y -= s->center.y;
		s->vertices[i].z -= s->center.z;

		s->vertices[i].x *= mag;
		s->vertices[i].y *= mag;
		s->vertices[i].z *= mag;

		s->vertices[i].x += s->center.x;
		s->vertices[i].y += s->center.y;
		s->vertices[i].z += s->center.z;
	}
}

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

int
reset_shape(struct shape *s)
{
	free(s->vertices);
	if (s->fname != NULL) {
		return init_from_file(s->fname, s);
	}

	return init_cube(s);
}

void
loop(struct shape *s)
{
	int c;
	double theta, dist, scale;

	theta = M_PI / 200;
	dist = 0.1;
	scale = 1.1;

	while(1) {
		wclear(stdscr);
		print_shape(*s);

		c = getch();
		switch(c) {
		case 'q':
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
			rotate_shape(theta, 'y', s);
			break;
		case 'p':
			rotate_shape(-theta, 'y', s);
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
			translate_shape(dist, 'y', s);
			break;

		case 'k':
			translate_shape(-dist, 'y', s);
			break;

		/* translate along z axis */
		case 'f':
			translate_shape(-dist, 'z', s);
			break;
		case 'g':
			translate_shape(dist, 'z', s);
			break;


		/* RESET */
		case 'r':
			reset_shape(s);
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
	struct shape cube;

	if (argc > 1) {
		err = init_from_file(argv[1], &cube);
	} else {
		err = init_cube(&cube);
	}
	if (err == -1) {
		printf("error allocating cube\n");
		exit(1);
	}

	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);

	loop(&cube);

	endwin();

	free(cube.vertices);

	return 0;
}
