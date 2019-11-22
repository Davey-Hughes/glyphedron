#include <math.h>

#include "convex_occlusion.h"
#include "vector.h"
#include "term_shapes.h"

/*
 * determines whether the point p2 lies on the line segment given by p0 and p1
 *
 * returns 1 if true and 0 if false
 */

static
int
on_segment(point3 p0, point3 p1, point3 p2)
{
	if (p2.x <= fmax(p0.x, p1.x) && p2.x >= fmin(p0.x, p1.x) &&
	    p2.y <= fmax(p0.y, p1.y) && p2.y >= fmin(p0.y, p1.y) &&
	    p2.z <= fmax(p0.z, p1.z) && p2.z >= fmin(p0.z, p1.z)) {
		return 1;
	}

	return 0;
}

/*
 * returns 0 if p0, p1, p2 are colinear
 * returns 1 if points are clockwise
 * returns 2 if points are counterclockwise
 *
 * the points are colinear if n = <0, 0, 0>
 */
static
int
orientation(point3 p0, point3 p1, point3 p2, struct face face)
{
	double dot;
	point3 t0, t1, t2;

	/*
	 * if n ⋅ ((p1 - p0) × (p2 - p0)) < 0 then the points are clockwise
	 * if n ⋅ ((p1 - p0) × (p2 - p0)) > 0 then the points are counterclockwise
	 * if n ⋅ ((p1 - p0) × (p2 - p0)) = 0 then the points are colinear
	 */

	t0 = vector3_sub(p1, p0);
	t1 = vector3_sub(p2, p0);

	/* t0 × t1 */
	t2 = vector3_cross(t0, t1);

	/* normal ⋅ t2 */
	dot = vector3_dot(face.normal, t2);

	if (dot < 0) {        /* clockwise */
		return 1;
	} else if (dot > 0) { /* counterclockwise */
		return 2;
	} else {              /* colinear */
		return 0;
	}
}

/*
 * determine whether these two line segments intersect
 *
 * returns 1 if they intersect, 0 if they don't
 */
static
int
intersects(point3 f0, point3 f1, point3 inter,
	   point3 far, struct face face)
{
	int o0, o1, o2, o3;

	/* find orientations for the general and special cases */
	o0 = orientation(f0, f1, inter, face);
	o1 = orientation(f0, f1, far, face);
	o2 = orientation(inter, far, f0, face);
	o3 = orientation(inter, far, f1, face);

	/* general case */
	if (o0 != o1 && o2 != o3) {
		return 1;
	}

	/* special cases for colinear */
	/* f0, f1, and inter are colinear and inter lies on the segment f0, f1 */
	if (o0 == 0 && on_segment(f0, f1, inter)) {
		return 1;
	}
	/* f0, f1, and far are colinear and far lies on the segment f0, f1 */
	else if (o1 == 0 && on_segment(f0, f1, far)) {
		return 1;
	}
	/* inter, far, and f0 are colinear and f0 lies on the segment inter, far */
	else if (o2 == 0 && on_segment(inter, far, f0)) {
		return 1;
	}
	/* inter, far, and f1 are colinear and f1 lies on the segment inter, far */
	else if (o3 == 0 && on_segment(inter, far, f1)) {
		return 1;
	}

	return 0;
}

/*
 * determines whether a point is inside the polygon named "face"
 *
 * returns 1 if the point is inside, and 0 if not
 */
static
int
is_inside(struct shape *s, point3 inter, point3 far, struct face face)
{
	int count, i, next_v;

	/*
	 * count number of intersections from the line segment with the polygon
	 */
	count = 0;
	i = 0;
	while (1) {
		/* next vertex */
		next_v = (i + 1) % face.num_v;

		/*
		 * first check if the line segment from inter to far intersects
		 * the edge from the face vertices with indices i and next_v
		 */
		if (intersects(s->vertices[face.face[i]],
			       s->vertices[face.face[next_v]], inter, far, face)) {
			/*
			 * if the point inter is colinear with the line segment
			 * given from i and next_v, check if it lies on the
			 * segment
			 */
			if (orientation(s->vertices[face.face[i]], inter,
				        s->vertices[face.face[next_v]], face) == 0) {
				return on_segment(s->vertices[face.face[i]],
						  s->vertices[face.face[next_v]], inter);
			}

			count++;
		}

		i = next_v;

		/* exit condition */
		if (i == 0) {
			break;
		}
	}

	return count % 2 == 1;
}

/*
 * determine whether a point is contained within a given polygon
 *
 * for this program, the point comes from the intersection given in
 * occlude_point_convex(), and we determine whether the point intersects the
 * plane within the boundaries of the vertices that define the face of the
 * polyhedron being rendered
 */
static
int
point_in_polygon(struct shape *s, point3 inter, struct face face,
		 point3 coeffs, double d)
{
	double z;

	/*
	 * we need a point on the same plane as the face, but far off one side
	 * to determine how many edges are intersected by the line segment from
	 * the intersection point and the far off distance point.
	 *
	 * using a large x value, and y value of 0, we can determine the z
	 * value that corresponds to a point on the plane with the equation:
	 * 	z = (d - ax - by) / c
	 * We simply use x = 10000 and y = 0, and plug in the coefficients
	 * passed into this function
	 */

	z = (d - (coeffs.x * 10000) - (coeffs.y * 0)) / coeffs.z;

	/*
	 * now the line segment is defined by the points inter and
	 * {10000, 0, z}
	 */

	return is_inside(s, inter, (point3) {10000, 0, z}, face);
}

/*
 * returns 1 if p0 is between p1 and p2, 0 otherwise
 */
int
is_between(point3 p0, point3 p1, point3 p2)
{
	return ((p1.x < p0.x && p0.x < p2.x) ||
	        (p2.x < p0.x && p0.x < p1.x)) &&
	       ((p1.y < p0.y && p0.y < p2.y) ||
		(p2.y < p0.y && p0.y < p1.y)) &&
	       ((p1.z < p0.z && p0.z < p2.z) ||
		(p2.z < p0.z && p0.z < p1.z));
}

/*
 * occlusion method that works for convex shapes
 *
 * returns 0 if point should be rendered, else 1
 */
int
occlude_point_convex(struct shape *s, point3 point, struct edge edge)
{
	int i, k, next_v, flag;
	double d, t;
	point3 v0, v1, n, inter;

	/*
	 * iterate through every face in the shape and determine whether the
	 * ray between the center of projection and input point intersects a
	 * face
	 *
	 * note: precomputing the equations for the faces would speed this up
	 */

	flag = 0;
	for (i = 0; i < s->num_f; ++i) {

		/*
		 * if the point is on an edge that constitutes this face, don't
		 * consider this face
		 *
		 * also check specifically if the edge passed in is invalid (in
		 * the case where the point being tested for occlusion is a
		 * vertex)
		 */

		if (edge.edge[0] < 0 || edge.edge[1] < 0) {
			continue;
		}

		k = 0;
		while (1) {
			next_v = (k + 1) % s->faces[i].num_v;

			if ((edge.edge[0] == s->faces[i].face[k] &&
			     edge.edge[1] == s->faces[i].face[next_v]) ||
			    (edge.edge[0] == s->faces[i].face[next_v] &&
			     edge.edge[1] == s->faces[i].face[k])) {
				flag = 1;
				break;
			}

			k = next_v;

			if (k == 0) {
				break;
			}
		}

		if (flag) {
			flag = 0;
			continue;
		}

		/*
		 * two vectors from the points that define the plane-face of
		 * the object
		 */
		v0 = vector3_sub(s->vertices[s->faces[i].face[0]],
				 s->vertices[s->faces[i].face[1]]);
		v1 = vector3_sub(s->vertices[s->faces[i].face[0]],
				 s->vertices[s->faces[i].face[2]]);

		/* n is the cross product of the two vectors */
		n = vector3_cross(v0, v1);

		s->faces[i].normal.x = n.x;
		s->faces[i].normal.y = n.y;
		s->faces[i].normal.z = n.z;

		/*
		 * with the parameterized equation of the plane given as:
		 * 	ax + by + cz = d
		 * a is the value n.x, b is the value n.y, z is the value n.z
		 * and d is given by solving ax + by + cz = 0, where x, y, and
		 * z are the x, y, and z from any one of the intial points
		 */
		d = vector3_dot(n, s->vertices[s->faces[i].face[0]]);

		/*
		 * the intersection of the line between the point we're
		 * evaluating and the center of projection with the face-plane
		 * is given by finding the parametric form of the line (where p
		 * is the point we're evaluating and cop is the center of
		 * projection point:
		 * 	r(t) = <x_p, y_p, z_p> + t<x_cop - x_p, y_cop - y_p, z_cop - z_p>
		 * After substituting these points in, we can then solve for z
		 * by plugging in the parametric form of the line to our
		 * equation of the plane.
		 */

		t = (d - (n.x * point.x + n.y * point.y + n.z * point.z)) /
			 (n.x * (s->cop.x - point.x) +
			  n.y * (s->cop.y - point.y) +
			  n.z * (s->cop.z - point.z));

		/* inter is the intersection point */
		inter.x = (point.x + (t * (s->cop.x - point.x)));
		inter.y = (point.y + (t * (s->cop.y - point.y)));
		inter.z = (point.z + (t * (s->cop.z - point.z)));


		/*
		 * if the intersection point isn't between the center of
		 * projection and the input point, skip the point in polygon
		 * calculation
		 */
		if (!is_between(inter, s->cop, point)) {
			continue;
		}

		/*
		 * if the intersection is not on a face, loop again to check
		 * the next face
		 */
		if (!point_in_polygon(s, inter, s->faces[i], n, d)) {
			continue;
		}

		/*
		 * if the point is on a face and the intersection is in front
		 * of the point (determined just by z value), then occlude the
		 * point
		 */
		if (point.z < inter.z) {
			return 1;
		}
	}

	return 0;
}
