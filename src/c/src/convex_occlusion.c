#include <math.h>

#include "convex_occlusion.h"
#include "vector.h"
#include "glyphedron.h"

/*
 * index of the largest component of a vector by magnitude
 */
static
int
dominant_axis(struct vector3 *v)
{
	double ax, ay, az;

	ax = fabs(v->x);
	ay = fabs(v->y);
	az = fabs(v->z);

	if (ax >= ay && ax >= az) {
		return 0;
	}

	if (ay >= az) {
		return 1;
	}

	return 2;
}

/*
 * projects a 3D point onto one of the three coordinate planes, dropping the
 * given axis. dropping the axis a face's normal is most aligned with is what
 * keeps the projected face from collapsing to a line
 */
static
void
project_to_plane(point3 *p, int axis, double *u, double *v)
{
	switch (axis) {
	case 0: /* normal mostly along x, so project onto the yz plane */
		*u = p->y;
		*v = p->z;
		break;

	case 1: /* normal mostly along y, so project onto the xz plane */
		*u = p->x;
		*v = p->z;
		break;

	default: /* normal mostly along z, so project onto the xy plane */
		*u = p->x;
		*v = p->y;
		break;
	}
}

/*
 * determines whether a point on a face's plane lies within the face
 *
 * the face is projected onto a coordinate plane and tested with the crossing
 * number rule: cast a ray in +u and count the edges it crosses, odd meaning
 * inside
 *
 * the comparison against v is half open, so each edge owns the span
 * [min(vi, vj), max(vi, vj)). that is what makes a vertex sitting exactly on
 * the ray safe, though not by counting it once in every case: where the
 * boundary genuinely crosses the ray exactly one of the two edges owns v and
 * the vertex counts once, while where the boundary merely touches the ray it
 * counts twice at a local minimum and not at all at a local maximum. both
 * touching cases are even, so the parity stays correct. an edge lying along
 * the ray has vi == vj, fails the guard, and is skipped without dividing
 *
 * the previous implementation cast its ray toward a far point with a hardcoded
 * y of 0 and resolved these cases with explicit collinearity branches instead.
 * for a point whose intersection also lay at y == 0 that ray ran along the
 * polygon's own vertices, and those branches resolved the parity
 * inconsistently: measured against mirror symmetry across every shape file, it
 * contradicted itself on 12 vertices spread over four convex solids, where
 * this rule contradicts itself on none of those. measured end to end through
 * occlude_point_convex, one mirror pair still disagrees: dodecahedron vertices
 * 16 and 17. the shape file stores phi to 5 decimals, so a vertex sits a
 * fraction off its own face plane, its t against an incident face comes out
 * around 1.1e-16, and the intersection lands exactly on that face's boundary.
 * that is the ulp level ambiguity an epsilon was measured to make worse, not
 * better
 *
 * returns 1 if the point is inside, 0 if not
 */
static
int
point_in_polygon(struct shape *s, point3 *inter, struct face *face)
{
	int i, j, axis, inside;
	double u, v, ui, vi, uj, vj;

	axis = dominant_axis(&(face->normal));
	project_to_plane(inter, axis, &u, &v);

	inside = 0;
	j = face->num_v - 1;

	for (i = 0; i < face->num_v; ++i) {
		project_to_plane(&(s->vertices[face->face[i]]), axis, &ui, &vi);
		project_to_plane(&(s->vertices[face->face[j]]), axis, &uj, &vj);

		/*
		 * the half open test guarantees vi != vj here, so the division
		 * is safe
		 */
		if ((vi > v) != (vj > v) &&
		    u < (((uj - ui) * (v - vi)) / (vj - vi)) + ui) {
			inside = !inside;
		}

		j = i;
	}

	return inside;
}

/*
 * occlusion method that works for convex shapes
 *
 * returns 0 if point should be rendered, else 1
 */
int
occlude_point_convex(struct shape *s, point3 *point, struct edge *edge)
{
	int i, k, next_v, flag, check_edge;
	double d, t;
	point3 n, inter;

	/*
	 * iterate through every face in the shape and determine whether the
	 * ray between the center of projection and input point intersects a
	 * face
	 *
	 * note: precomputing the equations for the faces would speed this up
	 */

	/*
	 * a vertex is not on any one edge, so callers testing a vertex pass an
	 * invalid edge. in that case there is no edge to exclude, but every
	 * face must still be tested
	 */
	check_edge = edge->edge[0] >= 0 && edge->edge[1] >= 0;

	flag = 0;
	for (i = 0; i < s->num_f; ++i) {

		/*
		 * if the point is on an edge that constitutes this face, don't
		 * consider this face
		 */
		if (check_edge) {
			k = 0;
			while (1) {
				next_v = (k + 1) % s->faces[i].num_v;

				if ((edge->edge[0] == s->faces[i].face[k] &&
				     edge->edge[1] == s->faces[i].face[next_v]) ||
				    (edge->edge[0] == s->faces[i].face[next_v] &&
				     edge->edge[1] == s->faces[i].face[k])) {
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
		}

		n = s->faces[i].normal;

		/*
		 * with the parameterized equation of the plane given as:
		 * 	ax + by + cz = d
		 * a is the value n.x, b is the value n.y, z is the value n.z
		 * and d is given by solving ax + by + cz = 0, where x, y, and
		 * z are the x, y, and z from any one of the intial points
		 */
		d = vector3_dot(&n, &(s->vertices[s->faces[i].face[0]]));

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

		t = (d - (n.x * point->x + n.y * point->y + n.z * point->z)) /
			 (n.x * (s->cop.x - point->x) +
			  n.y * (s->cop.y - point->y) +
			  n.z * (s->cop.z - point->z));

		/* inter is the intersection point */
		inter.x = (point->x + (t * (s->cop.x - point->x)));
		inter.y = (point->y + (t * (s->cop.y - point->y)));
		inter.z = (point->z + (t * (s->cop.z - point->z)));


		/*
		 * inter = point + t * (cop - point), so the intersection lies
		 * strictly between the point and the centre of projection
		 * exactly when 0 < t < 1. testing t directly avoids the
		 * per-axis comparison, which reported "not between" whenever a
		 * coordinate of the point equalled the same coordinate of the
		 * centre of projection and so stayed constant along the ray
		 *
		 * the condition is negated rather than written as
		 * t <= 0.0 || t >= 1.0, because a ray parallel to the face
		 * plane gives a zero denominator and a NaN t, and every NaN
		 * comparison is false
		 */
		if (!(t > 0.0 && t < 1.0)) {
			continue;
		}

		/*
		 * if the intersection is not on a face, loop again to check
		 * the next face
		 */
		if (!point_in_polygon(s, &inter, &(s->faces[i]))) {
			continue;
		}

		/*
		 * if the point is on a face and the intersection is in front
		 * of the point (determined just by z value), then occlude the
		 * point
		 */
		if (point->z < inter.z) {
			return 1;
		}
	}

	return 0;
}
