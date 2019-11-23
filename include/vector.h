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

#ifndef VECTOR_H
#define VECTOR_H

/* vector with three elements */
struct vector3 {
	double x;
	double y;
	double z;
};

void vector3_add(struct vector3 *, struct vector3 *, struct vector3 *);
void vector3_sub(struct vector3 *, struct vector3 *, struct vector3 *);
void vector3_mult(struct vector3 *, double, struct vector3 *);
double vector3_dot(struct vector3 *, struct vector3 *);
void vector3_cross(struct vector3 *, struct vector3 *, struct vector3 *);
double vector3_mag(struct vector3 *);
void vector3_unit(struct vector3 *, struct vector3 *);

#endif /* VECTOR_H */
