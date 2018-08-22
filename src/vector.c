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
struct vector3
vector3_add(struct vector3 v0, struct vector3 v1)
{
	v0.x += v1.x;
	v0.y += v1.y;
	v0.z += v1.z;

	return v0;
}

/*
 * subtracts v1 from v0
 */
struct vector3
vector3_sub(struct vector3 v0, struct vector3 v1)
{
	v0.x -= v1.x;
	v0.y -= v1.y;
	v0.z -= v1.z;

	return v0;
}

/*
 * multiplies a vector by a magnitude
 */
struct vector3
vector3_mult(struct vector3 v0, double mag)
{
	v0.x *= mag;
	v0.y *= mag;
	v0.z *= mag;

	return v0;
}

/*
 * dot product of vectors v0 and v1
 */
double
vector3_dot(struct vector3 v0, struct vector3 v1)
{
	return (v0.x * v1.x) + (v0.y * v1.y) + (v0.z * v1.z);
}

/*
 * cross product of v0 × v1
 */
struct vector3
vector3_cross(struct vector3 v0, struct vector3 v1)
{
	struct vector3 ret;

	ret.x = (v0.y * v1.z) - (v0.z * v1.y);
	ret.y = (v0.z * v1.x) - (v0.x * v1.z);
	ret.z = (v0.x * v1.y) - (v0.y * v1.x);

	return ret;
}

/*
 * magnitude of vector v
 */
double
vector3_mag(struct vector3 v)
{
	return sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

/*
 * calculates the unit vector of v
 */
struct vector3
vector3_unit(struct vector3 v)
{
	double v_len;

	v_len = vector3_mag(v);

	v.x = v.x / v_len;
	v.y = v.y / v_len;
	v.z = v.z / v_len;

	return v;
}
