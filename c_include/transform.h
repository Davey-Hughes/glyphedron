#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "term_shapes.h"

/* prototypes */
void rotate_shape(double, char, struct shape *);
void scale_shape(double, struct shape *);
void translate_shape(double, char, struct shape *);
int resize_points_to_print(struct shape *s);
void autorotate(struct shape *);

#endif /* TRANSFORM_H */
