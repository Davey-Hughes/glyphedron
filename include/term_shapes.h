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

#ifndef TIMING
#define TIMING 1
#endif

#if TIMING
#include "timing.h"
#endif

#ifndef TERM_SHAPES_H
#define TERM_SHAPES_H

#define M_PI 3.14159265358979323846264338327950288

#define MAX_VERTICES 1024 * 10
#define MAX_EDGES 1024 * 10

#define SCALE 0.4
#define E_DENSITY 50
#define COP {0, 0, 10000}

/* point in 3 dimensions */
struct point3 {
	double x;
	double y;
	double z;
};

/* edge as the index of two points */
struct edge {
	int edge[2];
};

/* choose which occlusion method to use */
enum occ_method {
	NONE,
	APPROX,
	CONVEX, /* not implemented */
	EXACT   /* not implemented */
};

/* a shape/solid object */
struct shape {
	int num_v;     /* number of vertices */
	int num_e;     /* number of edges */
	int e_density; /* number of points to draw along eatch edge */

	struct point3 center; /* center of the shape */

	struct point3 *vertices; /* list of vertices */
	struct edge *edges;      /* list of edges */

	char *fname; /* file name of the shape coordinates */

	int print_vertices;        /* bool whether or not to print vertices */
	enum occ_method occlusion; /* choose which occlusion method to use */
	struct point3 cop;         /* center of projection */
};

int init_from_file(char *, struct shape *);
void destroy_shape(struct shape *);
int init_cube(struct shape *);
void movexy(double *, double *);
int occlude_point_approx(struct shape, struct point3);
int occlude_point_convex(struct shape, struct point3);
int occlude_point(struct shape, struct point3);
void print_edges(struct shape);
void print_vertices(struct shape);
void print_shape(struct shape);
void rotate_shape(double, char, struct shape *);
void scale_shape(double, struct shape *);
void translate_shape(double, char, struct shape *);
int reset_shape(struct shape *);

#endif /* TERM_SHAPES_H */
