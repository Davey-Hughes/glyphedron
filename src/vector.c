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

#include "vector.h"
#include "math.h"

/*
 * adds two vectors together
 */
void
vector3_add(struct vector3 *v0, struct vector3 *v1, struct vector3 *ret)
{
	ret->x = v0->x + v1->x;
	ret->y = v0->y + v1->y;
	ret->z = v0->z + v1->z;
}

/*
 * subtracts v1 from v0
 */
void
vector3_sub(struct vector3 *v0, struct vector3 *v1, struct vector3 *ret)
{
	ret->x = v0->x - v1->x;
	ret->y = v0->y - v1->y;
	ret->z = v0->z - v1->z;
}

/*
 * multiplies a vector by a magnitude
 */
void
vector3_mult(struct vector3 *v0, double mag, struct vector3 *ret)
{
	ret->x = v0->x * mag;
	ret->y = v0->y * mag;
	ret->z = v0->z * mag;
}

/*
 * dot product of vectors v0 and v1
 */
double
vector3_dot(struct vector3 *v0, struct vector3 *v1)
{
	return (v0->x * v1->x) + (v0->y * v1->y) + (v0->z * v1->z);
}

/*
 * cross product of v0 × v1
 */
void
vector3_cross(struct vector3 *v0, struct vector3 *v1, struct vector3 *ret)
{
	ret->x = (v0->y * v1->z) - (v0->z * v1->y);
	ret->y = (v0->z * v1->x) - (v0->x * v1->z);
	ret->z = (v0->x * v1->y) - (v0->y * v1->x);
}

/*
 * magnitude of vector v
 */
double
vector3_mag(struct vector3 *v)
{
	return sqrt((v->x * v->x) + (v->y * v->y) + (v->z * v->z));
}

/*
 * calculates the unit vector of v
 */
void
vector3_unit(struct vector3 *v, struct vector3 *ret)
{
	double v_len;

	v_len = vector3_mag(v);

	ret->x = v->x / v_len;
	ret->y = v->y / v_len;
	ret->z = v->z / v_len;
}

/*
 * calculates the normal from three vectors
 */
void
vector3_normal(struct vector3 *v0, struct vector3 *v1,
	       struct vector3 *v2, struct vector3 *ret)
{
	struct vector3 tmp0, tmp1;

	vector3_sub(v0, v1, &tmp0);
	vector3_sub(v0, v2, &tmp1);
	vector3_cross(&tmp0, &tmp1, ret);
}
