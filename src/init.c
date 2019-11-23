#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>

#include "init.h"
#include "vector.h"
#include "term_shapes.h"

/*
 * initialize an object from a file
 *
 * the first line of the file is two comma separated ints describing the number
 * of vertices n, number of edges m, and number of faces k
 *
 * the next n lines are each 3 comma separated floating point values describing
 * the (x, y, z) locations of the vertex in 3D coordinates
 *
 * the next m lines are 2 comma separated positive integer values
 * correspoinding to the indices of two vertices that describe an edge. Each
 * edge only needs to be described once
 *
 * the next k lines are comma separated positive integer values corresponding
 * to the indices of the points that make up a face. It's imperative that the
 * points go around the face in order, along the edges, so that points across
 * the face aren't being connected
 */
int
init_from_file(char *fname, struct shape *s)
{
	int err, num_v, num_e, num_f, e0, e1, vi, i, k, cap;
	void *face_err;
	double x, y, z;
	char buf[FACE_VERTS_BUFSIZE];
	char delimit[] = ", ";
	char *str;

	s->log = fopen("log.txt", "w+");
	if (s->log == NULL) {
		fprintf(stderr, "could not open log file\n");
		return -1;
	}

	FILE *file;
	file = fopen(fname, "r");
	if (file == NULL) {
		fprintf(stderr, "could not open file\n");
		goto cleanup_log_file;
	}

	/* read number of vertices and edges */
	err = fscanf(file, "%i, %i, %i", &num_v, &num_e, &num_f);
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
	} else if (num_f > MAX_FACES) {
		fprintf(stderr, "Number of faces exceeds max faces: %d\n", MAX_FACES);
	}

	s->num_v = num_v;
	s->num_e = num_e;
	s->num_f = num_f;

	s->e_density = E_DENSITY;

	s->vertices = malloc(sizeof(point3) * s->num_v);
	if (s->vertices == NULL) {
		goto cleanup_file;
	}

	s->edges = malloc(sizeof(struct edge) * s->num_e);
	if (s->edges == NULL) {
		goto cleanup_vertices;
	}

	s->faces = malloc(sizeof(struct face) * s->num_f);
	if (s->faces == NULL) {
		goto cleanup_edges;
	}

	/* allocate space to hold coordinates for printing step */
	s->fronts = malloc(sizeof(struct point_to_print) * (s->num_e + 1) * (s->e_density + 1));
	if (s->fronts == NULL) {
		goto cleanup_faces;
	}

	s->behinds = malloc(sizeof(struct point_to_print) * (s->num_e + 1) * (s->e_density + 1));
	if (s->behinds == NULL) {
		goto cleanup_fronts;
	}

	/* read 3D coordinates describing every vertex */
	for (i = 0; i < num_v; ++i) {
		err = fscanf(file, "%lf, %lf, %lf", &x, &y, &z);
		if (err == EOF) {
			fprintf(stderr, "Returned EOF when reading vertices in shape file\n");
			goto cleanup_behinds;
		} else if (err == 0) {
			fprintf(stderr, "Zero bytes read when reading vertices in shape file\n");
			goto cleanup_behinds;
		}

		s->vertices[i].x = x;
		s->vertices[i].y = y;
		s->vertices[i].z = z;
	}

	/* read indices of edges described by two vertices */
	for (i = 0; i < num_e; ++i) {
		err = fscanf(file, "%i, %i", &e0, &e1);
		if (err == EOF) {
			fprintf(stderr, "Returned EOF when reading edges in shape file\n");
			goto cleanup_behinds;
		} else if (err == 0) {
			fprintf(stderr, "Zero bytes read when reading edges in shape file\n");
			goto cleanup_behinds;
		}

		if (e0 < 0 || e0 > num_v - 1 || e1 < 0 || e1 > num_v -1) {
			fprintf(stderr, "Edge index out of bounds\n");
			goto cleanup_behinds;
		}

		s->edges[i].edge[0] = e0;
		s->edges[i].edge[1] = e1;
	}

	for (i = 0; i < num_f; ++i) {
		str = fgets(buf, FACE_VERTS_BUFSIZE, file);
		if (str == NULL) {
			goto cleanup_behinds;
		} else if (*buf == '\n') { /* skip the single newline */
			i--;
			continue;
		}

		k = 0;
		cap = 8;
		s->faces[i].face = malloc(sizeof(int) * cap);

		str = strtok(buf, delimit);
		while (1) {
			if (k > MAX_FACE_VERTICES) {
				fprintf(stderr,
					"number of vertices exceeds max vertices per face\n");
				goto cleanup_face_vertices;
			}

			if (k == cap) {
				cap *= 2;
				face_err = realloc(s->faces[i].face, sizeof(int) * cap);
				if (face_err == NULL) {
					goto cleanup_face_vertices;
				}

				s->faces[i].face = face_err;
			}

			vi = atoi(str);

			if (vi < 0 || vi > num_v) {
				fprintf(stderr, "Face vertex out of bounds\n");
				goto cleanup_face_vertices;
			}

			s->faces[i].face[k++] = vi;

			str = strtok(NULL, delimit);
			if (str == NULL) {
				break;
			}
		}

		s->faces[i].num_v = k;
	}

	fclose(file);

	/* calculate the normal for each face */
	for (i = 0; i < num_f; ++i) {
		vector3_normal(&(s->vertices[s->faces[i].face[0]]),
			       &(s->vertices[s->faces[i].face[1]]),
			       &(s->vertices[s->faces[i].face[2]]),
			       &(s->faces[i].normal));
	}


	s->center = (point3) {0.0, 0.0, 0.0};
	s->fname = fname;

	/*
	 * by default, vertices are not drawn directly, but if no edges are
	 * described by the input file, the vertices will be drawn by the
	 * function print_vertices()
	 */
	s->print_vertices = s->num_e == 0;

	s->print_edges = 1;

	s->occlusion = NONE;
	s->cop = (point3) COP;

	return 0;

cleanup_face_vertices:
	for (i = 0; i < num_f; ++i) {
		free(s->faces[i].face);
	}
cleanup_behinds:
	free(s->behinds);
cleanup_fronts:
	free(s->fronts);
cleanup_faces:
	free(s->faces);
cleanup_edges:
	free(s->edges);
cleanup_vertices:
	free(s->vertices);
cleanup_file:
	fclose(file);
cleanup_log_file:
	fclose(s->log);
	return -1;
}

/*
 * default shape if no file is given
 */
int
init_cube(struct shape *c)
{
	/* TODO should do absolute path better */
	return init_from_file("./shapes/platonic_solids/cube.txt", c);
}

/*
 * free memory allocated for shape
 */
void
destroy_shape(struct shape *s)
{
	int i;

	free(s->vertices);
	free(s->edges);
	free(s->behinds);
	free(s->fronts);

	for (i = 0; i < s->num_f; ++i) {
		free(s->faces[i].face);
	}

	free(s->faces);
}

/*
 * completely resets the shape, either by the filename originally given or the
 * default, hardcoded cube
 */
int
reset_shape(struct shape *s)
{
	destroy_shape(s);
	if (s->fname != NULL) {
		return init_from_file(s->fname, s);
	}

	return init_cube(s);
}
