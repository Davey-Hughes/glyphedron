#ifndef RASTER_H
#define RASTER_H

#include <sys/types.h>

#include "glyphedron.h"

/*
 * hands every point in arr to emit, in order, converted to the integer screen
 * coordinates ncurses wants. ctx is passed through untouched
 */
void ptp_emit(const struct point_to_print *arr, ssize_t len,
	      void (*emit)(int y, int x, char t, void *ctx), void *ctx);

/*
 * decides whether a newly rasterised point should be appended to the list it
 * belongs to: the front list when it is visible, the rear list when occluded.
 *
 * returns 1 when the point should be appended, writing the glyph to store into
 * glyph, or 0 when that cell already carries it
 */
int ptp_admit(const struct point_to_print *fronts, ssize_t nfronts,
	      const struct point_to_print *behinds, ssize_t nbehinds,
	      int occluded, const struct point_to_print *p,
	      enum t_pixel_print *glyph);

#endif /* RASTER_H */
