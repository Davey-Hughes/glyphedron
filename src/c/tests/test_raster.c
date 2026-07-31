#include <string.h>

#include "test.h"
#include "raster.h"
#include "term_shapes.h"

#define MAX_RECORDED 8

struct recorder {
	int count;
	int y[MAX_RECORDED];
	int x[MAX_RECORDED];
	char t[MAX_RECORDED];
};

static void
record(int y, int x, char t, void *ctx)
{
	struct recorder *r = ctx;

	if (r->count < MAX_RECORDED) {
		r->y[r->count] = y;
		r->x[r->count] = x;
		r->t[r->count] = t;
	}

	r->count++;
}

/*
 * every collected point must reach the emitter. the draw loops in print.c
 * stopped one short, so the last point of each frame was silently dropped
 */
static void
test_emits_every_point(void)
{
	struct point_to_print pts[3];
	struct recorder r;

	pts[0].x = 10.0; pts[0].y = 4.0; pts[0].t = UPPER;
	pts[1].x = 11.0; pts[1].y = 4.0; pts[1].t = LOWER;
	pts[2].x = 12.0; pts[2].y = 5.0; pts[2].t = FULL;

	memset(&r, 0, sizeof(r));
	ptp_emit(pts, 3, record, &r);

	CHECK_EQ(r.count, 3);

	/* the last point is the one that used to go missing */
	CHECK_EQ(r.x[2], 12);
	CHECK_EQ(r.y[2], 5);
	CHECK_EQ(r.t[2], (char) FULL);
}

static void
test_emits_a_single_point(void)
{
	struct point_to_print p;
	struct recorder r;

	p.x = 1.0;
	p.y = 2.0;
	p.t = UPPER;

	memset(&r, 0, sizeof(r));
	ptp_emit(&p, 1, record, &r);

	CHECK_EQ(r.count, 1);
	CHECK_EQ(r.x[0], 1);
	CHECK_EQ(r.y[0], 2);
}

static void
test_emits_nothing_when_empty(void)
{
	struct recorder r;

	memset(&r, 0, sizeof(r));
	ptp_emit(NULL, 0, record, &r);

	CHECK_EQ(r.count, 0);
}

void
suite_raster(void)
{
	test_emits_every_point();
	test_emits_a_single_point();
	test_emits_nothing_when_empty();
}
