#include <ncurses.h>
#include <math.h>

#include "print.h"
#include "convex_occlusion.h"
#include "vector.h"
#include "term_shapes.h"

/*
 * translate an x and y value based on the window dimensions and some magic
 * numbers so an object described with a "radius" approximately 1 will be
 * centered in the center of the screen and entirely fit on the screen
 */
static
enum t_pixel_print
movexy(double *x, double *y)
{
	int winx, winy;
	double integralx, integraly, fractionalx, fractionaly;

	getmaxyx(stdscr, winy, winx);

	fractionalx = ((*x * SCALE * winy) + (0.5 * winx));
	fractionaly = (-(*y * SCALE * .5 * winy) + (0.5 * winy));

	fractionalx = modf(fractionalx, &integralx);
	fractionaly = modf(fractionaly, &integraly);

	*x = integralx;
	*y = integraly;

	/* opposite direction than what's intuitive because y is reversed */
	if (fractionaly >= 0.5) {
		return LOWER;
	}

	return UPPER;
}

/*
 * searches a point_to_print array to determine if the point on the ncurses
 * screen has already been processed including the t_pixel_print.
 *
 * if there's no match on the {x y} coordinates, return -2.
 *
 * if there's a match on the {x y} coordinates and the t_pixel_print is the
 * same, return -1.
 *
 * if there's a match but the t_pixel_print is different, return the index of
 * the match.
 */
static
ssize_t
search_ptp(struct point_to_print *arr, ssize_t len, struct point_to_print *p)
{
	ssize_t i;

	for (i = 0; i < len; ++i) {
		if (p->x == arr[i].x && p->y == arr[i].y) {
			if (p->t == arr[i].t || arr[i].t == FULL) {
				return -1;
			}

			return i;
		}
	}

	return -2;
}

/*
 * midpoint between two points
 * TODO: move somewhere else
 */
static
void
midpoint(point3 *p0, point3 *p1, point3 *mp)
{
	mp->x = (p0->x + p1->x) / 2;
	mp->y = (p0->y + p1->y) / 2;
	mp->z = (p0->z + p1->z) / 2;
}

/*
 * prints the edges by calculating the normal vector from two given points, and
 * then prints points along the edge by multiplying the normal
 */
static
void
print_edges(struct shape *s)
{
	char occlude_val;
	ssize_t fronts_index, behinds_index, found_front, found_behind;
	int i, k, winx, winy;
	double x0, y0, z0, v_len, x, y, z, movex, movey;
	point3 v, u;
	enum edge_occlusion edge_occlude_state;
	enum t_pixel_print tpp;
	struct point_to_print test_point;

	getmaxyx(stdscr, winy, winx);

	fronts_index = 0;
	behinds_index = 0;

	/* iterates over the edges */
	for (i = s->num_e - 1; i >= 0; --i) {
		v = s->vertices[s->edges[i].edge[0]];
		x0 = v.x;
		y0 = v.y;
		z0 = v.z;

		/* v is the vector given by two points */
		vector3_sub(&(s->vertices[s->edges[i].edge[1]]),
			    &(s->vertices[s->edges[i].edge[0]]),
			    &v);
		v_len = vector3_mag(&v);

		/* u is the unit vector from v */
		vector3_unit(&v, &u);

		/*
		 * if both vertices are occluded, then all the points in
		 * between are occluded.
		 *
		 * if neither vertices are occluded, then none of the points in
		 * betwen are occluded
		 *
		 * if one vertex is occluded and the other is not occluded,
		 * then do the occlusion calculation on every point on the edge
		 */
		edge_occlude_state = PARTIAL;

		if (s->occlusion == CONVEX ||
		    s->occlusion == CONVEX_CLEAR) {
			int occ0, occ1, occ_mp;
			point3 *p0, *p1;
			point3 mp;

			p0 = &(s->vertices[s->edges[i].edge[0]]);
			p1 = &(s->vertices[s->edges[i].edge[1]]);

			occ0 = occlude_point_convex(s, p0, &(s->edges[i]));
			occ1 = occlude_point_convex(s, p1, &(s->edges[i]));

			if (occ0 == 0 && occ1 == 0) {
				midpoint(p0, p1, &mp);
				occ_mp = occlude_point_convex(s, &mp, &(s->edges[i]));
				if (occ_mp == 0) {
					edge_occlude_state = NEITHER;
				}
			} else if (occ0 == 1 && occ1 == 1) {
				edge_occlude_state = BOTH;
			}
		}


		/*
		 * prints points along the edge
		 *
		 * e_density is a natural number directly corresponding to the
		 * number of points printed along the edge
		 */
		for (k = 0; k <= s->e_density; ++k) {
			x = x0 + ((k / (double) s->e_density * v_len) * u.x);
			y = y0 + ((k / (double) s->e_density * v_len) * u.y);
			z = z0 + ((k / (double) s->e_density * v_len) * u.z);

			movex = x;
			movey = y;
			tpp = movexy(&movex, &movey);

			/*
 			 * only worry about points that are on screen and that
 			 * don't overlap with previous points
			 */
			if ((int) movex < 0 || (int) movex > winx ||
			    (int) movey < 0 || (int) movey > winy) {
				continue;
			}

			test_point.x = movex;
			test_point.y = movey;
			test_point.t = tpp;

			found_front = search_ptp(s->fronts, fronts_index, &test_point);
			if (found_front == -1) {
				continue;
			}

			found_behind = search_ptp(s->behinds, behinds_index, &test_point);
			if (found_behind == -1) {
				continue;
			}

			/*
			 * if the occlusion flag is set and a point shouldn't
			 * be occluded, the rest of the loop prints the point
			 */
			if (edge_occlude_state == PARTIAL) {
				occlude_val = occlude_point(s,
						&((point3) {x, y, z}),
						&(s->edges[i]));
			} else {
				occlude_val = edge_occlude_state;
			}

			if (s->occlusion == CONVEX && occlude_val) {
				continue;
			}

			/*
			 * renders the rear and front symbols based on whether
			 * the point is detected to be "behind" or "in front"
			 */
#if USE_NCURSES
			if (occlude_val == 1) {
				s->behinds[behinds_index].x = movex;
				s->behinds[behinds_index].y = movey;
				if (found_behind >= 0) {
					s->behinds[behinds_index].t = FULL;
				} else {
					s->behinds[behinds_index].t = tpp;
				}
				behinds_index++;
			} else {
				s->fronts[fronts_index].x = movex;
				s->fronts[fronts_index].y = movey;
				if (found_front >= 0) {
					s->fronts[fronts_index].t = FULL;
				} else {
					s->fronts[fronts_index].t = tpp;
				}
				fronts_index++;
			}
#endif
		}
	}

#if USE_NCURSES
	/* print all the points behind */
	attron(A_DIM);
	if (s->occlusion != CONVEX) {
		for (ssize_t j = 0; j < behinds_index - 1; ++j) {
			mvprintw(s->behinds[j].y,
				 s->behinds[j].x,
				 "%c", s->behinds[j].t);
		}
	}
	attroff(A_DIM);

	/* print all the points in front */
	attron(A_BOLD);
	for (ssize_t j = 0; j < fronts_index - 1; ++j) {
		mvprintw(s->fronts[j].y,
			 s->fronts[j].x,
			 "%c", s->fronts[j].t);
	}
	attroff(A_BOLD);
#endif
}

/*
 * prints the vertices with their index (to make connecting edges easier)
 *
 * vertices are printed backwards so that lower numbered indices are printed in
 * front of the higher number indices if they overlap
 */
static
void
print_vertices(struct shape *s)
{
	int i;
	double x, y, z;

	struct edge edge;

	/* specifically invalid edge */
	edge.edge[0] = -1;
	edge.edge[1] = -1;

	for (i = s->num_v - 1; i >= 0; --i) {
		x = s->vertices[i].x;
		y = s->vertices[i].y;
		z = s->vertices[i].z;

		if (s->occlusion &&
			occlude_point(s, &((point3) {x, y, z}), &edge)) {
			continue;
		}

		movexy(&x, &y);
#if USE_NCURSES
		mvprintw(y, x, "%i", i);
#endif
	}
}

/*
 * only prints edges if edges are stored in the shape struct
 */
void
print_shape(struct shape *s)
{
	if (s->print_edges && s->num_e) {
		print_edges(s);
	}

	if (s->print_vertices) {
		print_vertices(s);
	}
}
