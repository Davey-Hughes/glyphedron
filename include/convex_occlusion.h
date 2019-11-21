#ifndef CONVEX_OCCLUSION_H
#define CONVEX_OCCLUSION_H

#include "vector.h"
#include "term_shapes.h"

/* prototypes */
int occlude_point_convex(struct shape *s, point3 point, struct edge edge);

#endif /* CONVEX_OCCLUSION_H */
