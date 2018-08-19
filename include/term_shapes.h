#ifndef TERM_SHAPES_H
#define TERM_SHAPES_H

#define M_PI 3.14159265358979323846264338327950288

#define MAX_VERTICES 1024 * 10
#define MAX_EDGES 1024 * 10

#define SCALE 0.4
#define E_DENSITY 50
#define COP {0, 0, 10000}

#define TIMING 1

#if TIMING
struct avg_timing {
	int a;
};
#endif

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

/* a shape/solid object */
struct shape {
	int num_v;     /* number of vertices */
	int num_e;     /* number of edges */
	int e_density; /* number of points to draw along eatch edge */

	struct point3 center; /* center of the shape */

	struct point3 *vertices; /* list of vertices */
	struct edge *edges;      /* list of edges */

	char *fname; /* file name of the shape coordinates */

	int print_vertices; /* bool whether or not to print vertices */
	int occlusion;      /* bool to turn occlusion on or off */
	struct point3 cop;  /* center of projection */
};

int init_from_file(char *, struct shape *);
void destroy_shape(struct shape *);
int init_cube(struct shape *);
void movexy(double *, double *);
int occlude_point(struct shape, struct point3);
void print_edges(struct shape);
void print_vertices(struct shape);
void print_shape(struct shape);
void rotate_shape(double, char, struct shape *);
void scale_shape(double, struct shape *);
void translate_shape(double, char, struct shape *);
int reset_shape(struct shape *);

#endif /* TERM_SHAPES_H */
