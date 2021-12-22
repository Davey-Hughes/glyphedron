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

#include <time.h>

#include "timing.h"

/*
 * adds two struct timespec
 */
void
timespec_add(struct timespec *t1, struct timespec *t2,
	     struct timespec *result)
{
	result->tv_sec = t1->tv_sec + t2->tv_sec;
	result->tv_nsec = t1->tv_nsec + t2->tv_nsec;

	if (result->tv_nsec >= 1000000000) {
		result->tv_nsec -= 1000000000;
		result->tv_sec++;
	}
}

/*
 * subtracts two struct timespec
 */
void
timespec_diff(struct timespec *start, struct timespec *stop,
	      struct timespec *result)
{
	if ((stop->tv_nsec - start->tv_nsec) < 0) {
		result->tv_sec = stop->tv_sec - start->tv_sec - 1;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	} else {
		result->tv_sec = stop->tv_sec - start->tv_sec;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}

	return;
}

/*
 * averages two struct timespec
 */
void
timespec_avg(struct timespec *t1, struct timespec *t2,
	     struct timespec *result)
{
	struct timespec res;

	/* if either of t1 or t2 are negative, then set res to the other
 	 * of t1 or t2. This accounts for the initialization case where the
 	 * average is uninitialized because a negative time in this context is
 	 * impossible
 	 */
	if (t1->tv_sec < 0 && t1->tv_nsec < 0) {
		res = *t2;
	} else if (t2->tv_sec < 0 && t2->tv_nsec < 0) {
		res = *t1;
	} else {
		timespec_add(t1, t2, &res);
		res.tv_sec /= 2;
		res.tv_nsec /= 2;
	}

	*result = res;
}
