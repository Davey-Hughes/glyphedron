#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncurses.h>

#define MAX_VERTICES 1024 * 10
#define MAX_EDGES 1024 * 10

#define SCALE 0.4
#define E_DENSITY 50
#define COP {0, 0, 10000}

struct point3 {
	double x;
	double y;
	double z;
};

struct edge {
	int edge[2];
};

struct shape {
	int num_v;     /* number of vertices */
	int num_e;     /* number of edges */
	int e_density; /* number of points to draw along eatch edge */

	struct point3 center; /* center of the shape */

	struct point3 *vertices;
	struct edge *edges;

	char *fname; /* file name of the shape coordinates */

	int print_vertices; /* bool whether or not to print vertices */
	int occlusion;      /* bool to turn occlusion on or off */
	struct point3 cop;  /* center of projection */
};

int
init_from_file(char *fname, struct shape *s)
{
	int err, num_v, num_e, e0, e1, i;
	double x, y, z;

	FILE *file;

	file = fopen(fname, "r");
	if (file == NULL) {
		fprintf(stderr, "could not open file\n");
		return -1;
	}

	err = fscanf(file, "%i, %i", &num_v, &num_e);
	if (err == EOF) {
		fprintf(stderr, "Returned EOF when reading first line of shape file\n");
		goto cleanup_file;
	} else if (err == 0) {
		fprintf(stderr, "Zero bytes read when reading first line of shape file\n");
		goto cleanup_file;
	} else if (num_v > MAX_VERTICES) {
		fprintf(stderr, "Number of vertices exceeds max vertices: %d\n", MAX_VERTICES);
	} else if (num_e > MAX_EDGES) {
		fprintf(stderr, "Number of edges exceeds max edges: %d\n", MAX_VERTICES);
	}

	s->num_v = num_v;
	s->num_e = num_e;

	s->vertices = malloc(sizeof(struct point3) * s->num_v);
	if (s->vertices == NULL) {
		goto cleanup_file;
	}

	s->edges = malloc(sizeof(struct edge) * s->num_e);
	if (s->edges == NULL) {
		goto cleanup_vertices;
	}

	for (i = 0; i < num_v; ++i) {
		err = fscanf(file, "%lf, %lf, %lf", &x, &y, &z);
		if (err == EOF) {
			fprintf(stderr, "Returned EOF when reading vertices in shape file\n");
			goto cleanup_edges;
		} else if (err == 0) {
			fprintf(stderr, "Zero bytes read when reading vertices in shape file\n");
			goto cleanup_edges;
		}

		s->vertices[i].x = x;
		s->vertices[i].y = y;
		s->vertices[i].z = z;
	}

	for (i = 0; i < num_e; ++i) {
		err = fscanf(file, "%i, %i", &e0, &e1);
		if (err == EOF) {
			fprintf(stderr, "Returned EOF when reading edges in shape file\n");
			goto cleanup_edges;
		} else if (err == 0) {
			fprintf(stderr, "Zero bytes read when reading edges in shape file\n");
			goto cleanup_edges;
		}

		if (e0 < 0 || e0 > num_v - 1 || e1 < 0 || e1 > num_v -1) {
			fprintf(stderr, "Edge index out of bounds\n");
			goto cleanup_edges;
		}

		s->edges[i].edge[0] = e0;
		s->edges[i].edge[1] = e1;
	}

	fclose(file);

	s->center = (struct point3) {0.0, 0.0, 0.0};
	s->fname = fname;
	s->print_vertices = 1;

	if (s->num_e) {
		s->print_vertices = 0;
	}

	s->e_density = E_DENSITY;
	s->occlusion = 0;
	s->cop = (struct point3) COP;

	return 0;

cleanup_edges:
	free(s->edges);
cleanup_vertices:
	free(s->vertices);
cleanup_file:
	fclose(file);
	return -1;
}

void
destroy_shape(struct shape *s)
{
	free(s->vertices);
	free(s->edges);
}

int
init_cube(struct shape *c)
{
	c->vertices = malloc(sizeof(struct point3) * 8);
	if (c->vertices == NULL) {
		return -1;
	}

	c->edges = malloc(sizeof(struct edge) * 12);
	if (c->edges == NULL) {
		goto cleanup_verts;
	}

	c->vertices[0] = (struct point3) {1.0, 1.0, 1.0};
	c->vertices[1] = (struct point3) {-1.0, 1.0, 1.0};
	c->vertices[2] = (struct point3) {1.0, -1.0, 1.0};
	c->vertices[3] = (struct point3) {-1.0, -1.0, 1.0};
	c->vertices[4] = (struct point3) {1.0, 1.0, -1.0};
	c->vertices[5] = (struct point3) {-1.0, 1.0, -1.0};
	c->vertices[6] = (struct point3) {1.0, -1.0, -1.0};
	c->vertices[7] = (struct point3) {-1.0, -1.0, -1.0};

	c->edges[0] = (struct edge) {{0, 1}};
	c->edges[1] = (struct edge) {{0, 2}};
	c->edges[2] = (struct edge) {{0, 4}};
	c->edges[3] = (struct edge) {{1, 3}};
	c->edges[4] = (struct edge) {{1, 5}};
	c->edges[5] = (struct edge) {{2, 3}};
	c->edges[6] = (struct edge) {{2, 6}};
	c->edges[7] = (struct edge) {{3, 7}};
	c->edges[8] = (struct edge) {{4, 5}};
	c->edges[9] = (struct edge) {{4, 6}};
	c->edges[10] = (struct edge) {{5, 7}};
	c->edges[11] = (struct edge) {{6, 7}};

	c->num_v = 8;
	c->center = (struct point3) {0.0, 0.0, 0.0};
	c->fname = NULL;
	c->print_vertices = 0;
	c->e_density = E_DENSITY;
	c->occlusion = 0;
	c->cop = (struct point3) COP;

	return 0;

cleanup_verts:
	free(c->vertices);
	return -1;
}

void
movexy(double *x, double *y)
{
	int winx, winy;
	double movex, movey;

	getmaxyx(stdscr, winy, winx);

	movex = (int) ((*x * SCALE * winy) + (0.5 * winx));
	movey = (int) (-(*y * SCALE * .5 * winy) + (0.5 * winy));

	*x = movex;
	*y = movey;
}

/*
 * first approximation
 * returns 0 if point should be rendered, else 1
 */
int
occlude_point(struct shape s, struct point3 point)
{
	(void) s; (void) point;

	double x0, y0, z0, x1, y1, z1, dprod, mag0, mag1, theta;

	/* vector from point to the center of solid */
	x0 = s.center.x - point.x;
	y0 = s.center.y - point.y;
	z0 = s.center.z - point.z;

	/* vector from point to the center of projection */
	x1 = s.cop.x - point.x;
	y1 = s.cop.y - point.y;
	z1 = s.cop.z - point.z;

	/*
	 * angle between two vectors is given by the arc cosine of the dot
	 * product of the two vectors divided by the product of magnitudes of
	 * the vectors
	 */
	dprod = (x0 * x1) + (y0 * y1) + (z0 * z1);
	mag0 = sqrt((x0 * x0) + (y0 * y0) + (z0 * z0));
	mag1 = sqrt((x1 * x1) + (y1 * y1) + (z1 * z1));

	theta = acos(dprod / (mag0 * mag1));

	if (theta < M_PI / 3) {
		return 1;
	}

	return 0;
}

void
print_edges(struct shape s)
{
	int i, k;
	double x0, y0, z0, x1, y1, z1, v_len, x, y, z, movex, movey;
	struct point3 v, u;

	for (i = s.num_e - 1; i >= 0; --i) {
		v = s.vertices[s.edges[i].edge[0]];
		x0 = v.x;
		y0 = v.y;
		z0 = v.z;

		v = s.vertices[s.edges[i].edge[1]];
		x1 = v.x;
		y1 = v.y;
		z1 = v.z;

		v = (struct point3) {x1 - x0, y1 - y0, z1 - z0};
		v_len = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));

		u = (struct point3) {v.x / v_len, v.y / v_len, v.z / v_len};

		for (k = 0; k <= s.e_density; ++k) {
			x = x0 + ((k / (float) s.e_density * v_len) * u.x);
			y = y0 + ((k / (float) s.e_density * v_len) * u.y);
			z = z0 + ((k / (float) s.e_density * v_len) * u.z);

			if (s.occlusion && occlude_point(s, (struct point3) {x, y, z})) {
				continue;
			}

			movex = x;
			movey = y;
			movexy(&movex, &movey);

			if (z < 0) {
				mvprintw(movey, movex, "%s", ".");
			} else {
				mvprintw(movey, movex, "%s", "o");
			}
		}
	}
}

void
print_vertices(struct shape s)
{
	int i;
	double x, y, z;

	for (i = s.num_v - 1; i >= 0; --i) {
		x = s.vertices[i].x;
		y = s.vertices[i].y;
		z = s.vertices[i].z;

		if (s.occlusion && occlude_point(s, (struct point3) {x, y, z})) {
			continue;
		}

		movexy(&x, &y);
		mvprintw(y, x, "%i", i);
	}
}

void
print_shape(struct shape s)
{
	if (s.num_e) {
		print_edges(s);
	}

	if (s.print_vertices) {
		print_vertices(s);
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
	destroy_shape(s);
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


		/* RESET */
		case 'r':
			reset_shape(s);
			break;

		/* flip printing of vertices */
		case '1':
			s->print_vertices = !(s->print_vertices);
			break;

		/* turn occlusion on or off */
		case '2':
			s->occlusion = !(s->occlusion);
			break;

		/* **CHANGE EDGE DENSITY** */
		/* increase edge density */
		case '0':
			s->e_density++;
			break;

		/* decrease edge density */
		case '9':
			if (s->e_density > 0) {
				s->e_density--;
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

	destroy_shape(&cube);

	return 0;
}
