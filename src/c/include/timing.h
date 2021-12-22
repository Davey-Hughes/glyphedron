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

#ifndef TIMING_H
#define TIMING_H

#include <time.h>

void timespec_add(struct timespec *, struct timespec *, struct timespec *);
void timespec_diff(struct timespec *, struct timespec *, struct timespec *);
void timespec_avg(struct timespec *, struct timespec *, struct timespec *);

#endif /* TIMING_H */
