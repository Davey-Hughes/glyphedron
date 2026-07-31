#include "occlusion.h"
#include "convex_occlusion.h"
#include "occlude_approx.h"
#include "term_shapes.h"

/*
 * chooses which occlusion method to use based on the s.occlusion enum
 *
 * returns 0 if point should be rendered, else 1
 */
int
occlude_point(struct shape *s, point3 *point, struct edge *edge)
{
	switch (s->occlusion) {
	case NONE:
		return 0;

	case APPROX:
		return occlude_point_approx(s, point);

	case CONVEX:
		return occlude_point_convex(s, point, edge);

	case CONVEX_CLEAR:
		return occlude_point_convex(s, point, edge);

	case EXACT: /* not implemented */
		return 0;
	}

	return 0;
}
