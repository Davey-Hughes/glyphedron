#include <math.h>

#include "occlude_approx.h"
#include "vector.h"
#include "term_shapes.h"

/*
 * first approximation of occluding points
 *
 * operates by finding the angle between the vectors given by the point to the
 * center of the object and the point to the center of projection. This is only
 * an approximation as the angle given by this calculation doesn't determine
 * entirely whether a point should be occluded. The threshold is given by pi/3,
 * but as this is approximate it can be adjusted.
 *
 * returns 0 if point should be rendered, else 1
 */
int
occlude_point_approx(struct shape *s, point3 point)
{
	double dprod, mag0, mag1, theta;
	point3 v0, v1;

	/* vector from point to the center of solid */
	v0 = vector3_sub(s->center, point);

	/* vector from point to the center of projection */
	v1 = vector3_sub(s->cop, point);

	/*
	 * angle between two vectors is given by the arc cosine of the dot
	 * product of the two vectors divided by the product of magnitudes of
	 * the vectors
	 */
	dprod = vector3_dot(v0, v1);
	mag0 = vector3_mag(v0);
	mag1 = vector3_mag(v1);

	theta = acos(dprod / (mag0 * mag1));

	if (theta < M_PI / 3) {
		return 1;
	}

	return 0;
}
