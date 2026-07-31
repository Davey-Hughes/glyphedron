#include <sys/types.h>

#include "raster.h"
#include "term_shapes.h"

void
ptp_emit(const struct point_to_print *arr, ssize_t len,
	 void (*emit)(int y, int x, char t, void *ctx), void *ctx)
{
	ssize_t i;

	for (i = 0; i < len; ++i) {
		emit((int) arr[i].y, (int) arr[i].x, (char) arr[i].t, ctx);
	}
}
