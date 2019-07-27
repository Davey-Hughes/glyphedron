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

#ifndef TERM_SHAPES_H
#define TERM_SHAPES_H

#ifndef TIMING
#define TIMING 1
#endif

#if TIMING
#include "timing.h"
#endif

#include "vector.h"

/* used for valgrind testing, since ncurses shows a lot of errors in valgrind */
#ifndef USE_NCURSES
#define USE_NCURSES 1
#endif

#define M_PI 3.14159265358979323846264338327950288

#define MAX_VERTICES 1024 * 10
#define MAX_EDGES 1024 * 10
#define MAX_FACES 1024 * 10
#define MAX_FACE_VERTICES 1024 * 10
#define FACE_VERTS_BUFSIZE 1024 * 2

#define SCALE 0.4
#define E_DENSITY 50
#define COP {0, 0, 10000}

typedef struct vector3 point3;

/* edge as the index of two points */
struct edge {
	int edge[2];
};

/* edge as the index of three points */
struct face {
	int num_v;     /* number of vertices on this face */
	point3 normal; /* normal vector to this face */
	int *face;     /* array of indices corresponding to vertices on this face */
};

/* choose which occlusion method to use */
enum occ_method {
	NONE,
	APPROX,
	CONVEX,
	CONVEX_CLEAR,
	EXACT   /* not implemented */
};


/* point to be printed in a later step */
struct point_to_print {
	double x;
	double y;
};

/* a shape/solid object */
struct shape {
	int num_v;     /* number of vertices */
	int num_e;     /* number of edges */
	int num_f;     /* number of faces */
	int e_density; /* number of points to draw along eatch edge */

	point3 center; /* center of the shape */

	point3 *vertices; /* list of vertices */
	struct edge *edges;      /* list of edges */
	struct face *faces;      /* list of faces */

	char *fname; /* file name of the shape coordinates */

	int print_vertices;        /* bool whether or not to print vertices */
	int print_edges;           /* bool whether or not to print edges */
	enum occ_method occlusion; /* choose which occlusion method to use */
	point3 cop;         /* center of projection */

	char front_symbol;
	char rear_symbol;

	struct point_to_print *fronts; /* points detected as not occluded */
	struct point_to_print *behinds; /* points detected as occluded */
};

/* prototypes */

int init_from_file(char *, struct shape *);
void destroy_shape(struct shape *);
int init_cube(struct shape *);
void movexy(double *, double *);
int orientation(point3, point3, point3, struct face);
int intersects(point3, point3, point3, point3, struct face);
int is_inside(struct shape *, point3, point3, struct face);
int point_in_polygon(struct shape *, point3, struct face, point3, double);
int occlude_point_approx(struct shape *, point3);
int occlude_point_convex(struct shape *, point3, struct edge);
int occlude_point(struct shape *, point3, struct edge);
void print_edges(struct shape *);
void print_vertices(struct shape *);
void print_shape(struct shape *);
void rotate_shape(double, char, struct shape *);
void scale_shape(double, struct shape *);
void translate_shape(double, char, struct shape *);
int reset_shape(struct shape *);

#endif /* TERM_SHAPES_H */
