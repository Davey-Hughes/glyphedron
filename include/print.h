#ifndef PRINT_H
#define PRINT_H

#include "vector.h"
#include "term_shapes.h"

/*
 * states for determining whether an edge is not occluded, fully occluded, or
 * partially occluded. If the state is NEITHER or BOTH, all points along the
 * edge can be determined as not occluded or fully occluded respectfully, and
 * convex occlusion calculation of each point along the edge can be skipped
 */
enum edge_occlusion {
	NEITHER = 0,
	BOTH = 1,
	PARTIAL = 2
};

/* prototypes */
void print_shape(struct shape *s);

#endif /* PRINT_H */
