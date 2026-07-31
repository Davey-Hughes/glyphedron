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

/*
 * a rasterised point is deduplicated only against the list it will be stored
 * in. comparing it against both lists discarded a visible point whenever an
 * occluded one already held the same cell, which broke foreground lines
 * wherever a background line crossed them
 */
static
void
test_front_survives_a_matching_behind(void)
{
	struct point_to_print behinds[1];
	struct point_to_print p;
	enum t_pixel_print glyph;

	behinds[0].x = 10.0;
	behinds[0].y = 4.0;
	behinds[0].t = UPPER;

	p.x = 10.0;
	p.y = 4.0;
	p.t = UPPER;

	glyph = LOWER;

	/* the front list is empty, so this point is new and must be admitted */
	CHECK_EQ(ptp_admit(NULL, 0, behinds, 1, 0, &p, &glyph), 1);
	CHECK_EQ(glyph, (int) UPPER);
}

static
void
test_behind_survives_a_matching_front(void)
{
	struct point_to_print fronts[1];
	struct point_to_print p;
	enum t_pixel_print glyph;

	fronts[0].x = 10.0;
	fronts[0].y = 4.0;
	fronts[0].t = UPPER;

	p.x = 10.0;
	p.y = 4.0;
	p.t = UPPER;

	glyph = LOWER;

	CHECK_EQ(ptp_admit(fronts, 1, NULL, 0, 1, &p, &glyph), 1);
	CHECK_EQ(glyph, (int) UPPER);
}

static
void
test_duplicate_in_its_own_list_is_dropped(void)
{
	struct point_to_print fronts[1];
	struct point_to_print p;
	enum t_pixel_print glyph;

	fronts[0].x = 10.0;
	fronts[0].y = 4.0;
	fronts[0].t = UPPER;

	p.x = 10.0;
	p.y = 4.0;
	p.t = UPPER;

	CHECK_EQ(ptp_admit(fronts, 1, NULL, 0, 0, &p, &glyph), 0);
}

/*
 * a terminal cell is twice as high as it is wide, so two points can share it
 * while occupying different halves. the second promotes the cell to FULL
 */
static
void
test_other_half_of_the_cell_promotes_to_full(void)
{
	struct point_to_print fronts[1];
	struct point_to_print p;
	enum t_pixel_print glyph;

	fronts[0].x = 10.0;
	fronts[0].y = 4.0;
	fronts[0].t = UPPER;

	p.x = 10.0;
	p.y = 4.0;
	p.t = LOWER;

	CHECK_EQ(ptp_admit(fronts, 1, NULL, 0, 0, &p, &glyph), 1);
	CHECK_EQ(glyph, (int) FULL);
}

static
void
test_a_full_cell_absorbs_further_points(void)
{
	struct point_to_print fronts[1];
	struct point_to_print p;
	enum t_pixel_print glyph;

	fronts[0].x = 10.0;
	fronts[0].y = 4.0;
	fronts[0].t = FULL;

	p.x = 10.0;
	p.y = 4.0;
	p.t = UPPER;

	CHECK_EQ(ptp_admit(fronts, 1, NULL, 0, 0, &p, &glyph), 0);
}

void
suite_raster(void)
{
	test_emits_every_point();
	test_emits_a_single_point();
	test_emits_nothing_when_empty();
	test_front_survives_a_matching_behind();
	test_behind_survives_a_matching_front();
	test_duplicate_in_its_own_list_is_dropped();
	test_other_half_of_the_cell_promotes_to_full();
	test_a_full_cell_absorbs_further_points();
}
