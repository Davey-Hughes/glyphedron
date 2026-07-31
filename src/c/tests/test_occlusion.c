#include "test.h"
#include "glyphedron.h"
#include "occlusion.h"
#include "convex_occlusion.h"
#include "init.h"

#define CUBE "./shapes/platonic_solids/cube.txt"
#define ICOSAHEDRON "./shapes/platonic_solids/icosahedron.txt"

/*
 * with occlusion disabled every point renders, so occlude_point() returns 0
 * regardless of where the point sits. returning 1 here routes the whole shape
 * into the "behind" list, which print_edges() draws dim
 */
static void
test_none_renders_every_point(void)
{
	struct shape s;
	struct edge e;
	int i;

	CHECK_EQ(test_shape_load(CUBE, &s), 0);
	s.occlusion = NONE;

	e = s.edges[0];

	for (i = 0; i < s.num_v; ++i) {
		CHECK_EQ(occlude_point(&s, &s.vertices[i], &e), 0);
	}

	test_shape_unload(&s);
}

/*
 * print_vertices() passes a deliberately invalid edge, because a vertex is not
 * on any particular edge. viewed from the default centre of projection out at
 * +z, a cube hides exactly its four rear vertices
 */
static void
test_vertex_occlusion_with_invalid_edge(void)
{
	struct shape s;
	struct edge invalid;
	int i, occluded, rear;

	CHECK_EQ(test_shape_load(CUBE, &s), 0);
	s.occlusion = CONVEX;

	invalid.edge[0] = -1;
	invalid.edge[1] = -1;

	occluded = 0;
	rear = 0;

	for (i = 0; i < s.num_v; ++i) {
		if (occlude_point_convex(&s, &s.vertices[i], &invalid)) {
			occluded++;
		}

		if (s.vertices[i].z < 0.0) {
			rear++;
		}
	}

	CHECK_EQ(rear, 4);
	CHECK_EQ(occluded, 4);

	test_shape_unload(&s);
}

/*
 * every sample along a rear cube edge sits behind the front face, so every one
 * is occluded. the midpoint of each rear edge has one coordinate exactly zero,
 * which is where the centre of projection at {0, 0, 10000} ties with it
 */
static void
test_rear_edges_fully_occluded(void)
{
	struct shape s;
	int i, k, rear_edges;
	point3 a, b, p;

	CHECK_EQ(test_shape_load(CUBE, &s), 0);
	s.occlusion = CONVEX;

	rear_edges = 0;

	for (i = 0; i < s.num_e; ++i) {
		a = s.vertices[s.edges[i].edge[0]];
		b = s.vertices[s.edges[i].edge[1]];

		if (!(a.z < 0.0 && b.z < 0.0)) {
			continue;
		}

		rear_edges++;

		for (k = 0; k <= 10; ++k) {
			p.x = a.x + (k / 10.0) * (b.x - a.x);
			p.y = a.y + (k / 10.0) * (b.y - a.y);
			p.z = a.z + (k / 10.0) * (b.z - a.z);

			CHECK_EQ(occlude_point_convex(&s, &p, &s.edges[i]), 1);
		}
	}

	CHECK_EQ(rear_edges, 4);

	test_shape_unload(&s);
}

/*
 * the counterpart: nothing on a front edge may be occluded. without this the
 * fix could pass by occluding everything
 */
static void
test_front_edges_never_occluded(void)
{
	struct shape s;
	int i, k, front_edges;
	point3 a, b, p;

	CHECK_EQ(test_shape_load(CUBE, &s), 0);
	s.occlusion = CONVEX;

	front_edges = 0;

	for (i = 0; i < s.num_e; ++i) {
		a = s.vertices[s.edges[i].edge[0]];
		b = s.vertices[s.edges[i].edge[1]];

		if (!(a.z > 0.0 && b.z > 0.0)) {
			continue;
		}

		front_edges++;

		for (k = 0; k <= 10; ++k) {
			p.x = a.x + (k / 10.0) * (b.x - a.x);
			p.y = a.y + (k / 10.0) * (b.y - a.y);
			p.z = a.z + (k / 10.0) * (b.z - a.z);

			CHECK_EQ(occlude_point_convex(&s, &p, &s.edges[i]), 0);
		}
	}

	CHECK_EQ(front_edges, 4);

	test_shape_unload(&s);
}

/*
 * all twelve icosahedron vertices carry an exact zero coordinate, and four of
 * them sit at exactly y == 0 — the same coordinate the containment ray used to
 * hardcode. asserting on freshly loaded, unperturbed data is what lets the
 * cancelling rotations in loop() be deleted
 */
static void
test_icosahedron_rear_vertices_occluded(void)
{
	struct shape s;
	struct edge invalid;
	int i, rear, rear_occluded, front, front_occluded;

	CHECK_EQ(test_shape_load(ICOSAHEDRON, &s), 0);
	s.occlusion = CONVEX;

	invalid.edge[0] = -1;
	invalid.edge[1] = -1;

	rear = 0;
	rear_occluded = 0;
	front = 0;
	front_occluded = 0;

	for (i = 0; i < s.num_v; ++i) {
		/* skip the equator, where occlusion is genuinely ambiguous */
		if (s.vertices[i].z < -0.5) {
			rear++;
			if (occlude_point_convex(&s, &s.vertices[i], &invalid)) {
				rear_occluded++;
			}
		} else if (s.vertices[i].z > 0.5) {
			front++;
			if (occlude_point_convex(&s, &s.vertices[i], &invalid)) {
				front_occluded++;
			}
		}
	}

	CHECK_EQ(rear, 4);
	CHECK_EQ(front, 4);
	CHECK_EQ(rear_occluded, 4);
	CHECK_EQ(front_occluded, 0);

	test_shape_unload(&s);
}

void
suite_occlusion(void)
{
	test_none_renders_every_point();
	test_vertex_occlusion_with_invalid_edge();
	test_rear_edges_fully_occluded();
	test_front_edges_never_occluded();
	test_icosahedron_rear_vertices_occluded();
}
