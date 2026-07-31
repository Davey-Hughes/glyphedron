#include <math.h>

#include "test.h"
#include "vector.h"

#define EPS 1e-12

static void
test_add_sub_mult(void)
{
	struct vector3 a = {1.0, 2.0, 3.0};
	struct vector3 b = {0.5, -1.0, 4.0};
	struct vector3 r;

	vector3_add(&a, &b, &r);
	CHECK_NEAR(r.x, 1.5, EPS);
	CHECK_NEAR(r.y, 1.0, EPS);
	CHECK_NEAR(r.z, 7.0, EPS);

	vector3_sub(&a, &b, &r);
	CHECK_NEAR(r.x, 0.5, EPS);
	CHECK_NEAR(r.y, 3.0, EPS);
	CHECK_NEAR(r.z, -1.0, EPS);

	vector3_mult(&a, 2.0, &r);
	CHECK_NEAR(r.x, 2.0, EPS);
	CHECK_NEAR(r.y, 4.0, EPS);
	CHECK_NEAR(r.z, 6.0, EPS);
}

static void
test_dot_cross(void)
{
	struct vector3 x = {1.0, 0.0, 0.0};
	struct vector3 y = {0.0, 1.0, 0.0};
	struct vector3 r;

	CHECK_NEAR(vector3_dot(&x, &y), 0.0, EPS);
	CHECK_NEAR(vector3_dot(&x, &x), 1.0, EPS);

	/* right handed: x cross y is z */
	vector3_cross(&x, &y, &r);
	CHECK_NEAR(r.x, 0.0, EPS);
	CHECK_NEAR(r.y, 0.0, EPS);
	CHECK_NEAR(r.z, 1.0, EPS);
}

static void
test_mag_unit(void)
{
	struct vector3 v = {3.0, 4.0, 0.0};
	struct vector3 u;

	CHECK_NEAR(vector3_mag(&v), 5.0, EPS);

	vector3_unit(&v, &u);
	CHECK_NEAR(vector3_mag(&u), 1.0, EPS);
	CHECK_NEAR(u.x, 0.6, EPS);
	CHECK_NEAR(u.y, 0.8, EPS);
}

static void
test_normal(void)
{
	/* three points in the z = 1 plane, so the normal lies along z */
	struct vector3 p0 = {0.0, 0.0, 1.0};
	struct vector3 p1 = {1.0, 0.0, 1.0};
	struct vector3 p2 = {0.0, 1.0, 1.0};
	struct vector3 n;

	vector3_normal(&p0, &p1, &p2, &n);

	CHECK_NEAR(n.x, 0.0, EPS);
	CHECK_NEAR(n.y, 0.0, EPS);
	CHECK_NEAR(n.z, 1.0, EPS);
}

void
suite_vector(void)
{
	test_add_sub_mult();
	test_dot_cross();
	test_mag_unit();
	test_normal();
}
