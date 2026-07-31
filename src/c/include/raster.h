#ifndef RASTER_H
#define RASTER_H

#include <sys/types.h>

#include "term_shapes.h"

/*
 * hands every point in arr to emit, in order, converted to the integer screen
 * coordinates ncurses wants. ctx is passed through untouched
 */
void ptp_emit(const struct point_to_print *arr, ssize_t len,
	      void (*emit)(int y, int x, char t, void *ctx), void *ctx);

#endif /* RASTER_H */
