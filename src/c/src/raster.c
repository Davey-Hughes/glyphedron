#include <sys/types.h>

#include "raster.h"
#include "glyphedron.h"

/*
 * searches a point_to_print array for a point already occupying the same
 * screen cell.
 *
 * returns -2 when the cell is untouched, -1 when it already carries this exact
 * glyph, and otherwise the index of the entry holding the cell's other half
 */
static
ssize_t
search_ptp(const struct point_to_print *arr, ssize_t len,
	   const struct point_to_print *p)
{
	ssize_t i;

	for (i = 0; i < len; ++i) {
		if (p->x == arr[i].x && p->y == arr[i].y) {
			if (p->t == arr[i].t || arr[i].t == FULL) {
				return -1;
			}

			return i;
		}
	}

	return -2;
}

void
ptp_emit(const struct point_to_print *arr, ssize_t len,
	 void (*emit)(int y, int x, char t, void *ctx), void *ctx)
{
	ssize_t i;

	for (i = 0; i < len; ++i) {
		emit((int) arr[i].y, (int) arr[i].x, (char) arr[i].t, ctx);
	}
}

int
ptp_admit(const struct point_to_print *fronts, ssize_t nfronts,
	  const struct point_to_print *behinds, ssize_t nbehinds,
	  int occluded, const struct point_to_print *p,
	  enum t_pixel_print *glyph)
{
	const struct point_to_print *arr;
	ssize_t len, found;

	/*
	 * only the list this point is headed for may veto it. consulting both
	 * dropped a visible point whenever an occluded one already held the
	 * cell, so a background line crossing a foreground one punched a gap
	 * in the foreground
	 */
	if (occluded) {
		arr = behinds;
		len = nbehinds;
	} else {
		arr = fronts;
		len = nfronts;
	}

	found = search_ptp(arr, len, p);
	if (found == -1) {
		return 0;
	}

	*glyph = found >= 0 ? FULL : p->t;

	return 1;
}
