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

#include <stdio.h>
#include <time.h>

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
	int num_v;             /* number of vertices on this face */
	struct vector3 normal; /* normal vector to this face */
	int *face;             /* array of indices corresponding to vertices on this face */
};

/* choose which occlusion method to use */
enum occ_method {
	NONE,
	APPROX,
	CONVEX,
	CONVEX_CLEAR,
	EXACT   /* not implemented */
};

/*
 * considering a "terminal pixel" as approximately 2x as high as it is wide,
 * this enum is to determine whether the upper, lower, or both sections of the
 * "terminal pixel" should be printed
 */
enum t_pixel_print {
	UPPER = '\'',
	LOWER = ',',
	FULL = ';'
};


/* point to be printed in a later step */
struct point_to_print {
	double x;
	double y;
	enum t_pixel_print t;
};

struct autorotate_dir {
	double x;
	double y;
	double z;
};

/* a shape/solid object */
struct shape {
	int num_v;     /* number of vertices */
	int num_e;     /* number of edges */
	int num_f;     /* number of faces */
	int e_density; /* number of points to draw along eatch edge */

	point3 center; /* center of the shape */

	point3 *vertices;        /* list of vertices */
	struct edge *edges;      /* list of edges */
	struct face *faces;      /* list of faces */

	char *fname; /* file name of the shape coordinates */

	int print_vertices;        /* bool whether or not to print vertices */
	int print_edges;           /* bool whether or not to print edges */
	enum occ_method occlusion; /* choose which occlusion method to use */
	point3 cop;                /* center of projection */

	struct point_to_print *fronts;  /* points detected as not occluded */
	struct point_to_print *behinds; /* points detected as occluded */

	int autorotate;            /* whether auto-rotate is on or off */
	struct autorotate_dir dir; /* direction to rotate the shape in radians */
	struct timespec interval;  /* interval to redraw shape */

	FILE *log; /* log file */
};

/* prototypes */
int occlude_point(struct shape *s, point3 *point, struct edge *edge);

#endif /* TERM_SHAPES_H */
