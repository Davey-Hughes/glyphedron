#ifndef INIT_H
#define INIT_H

#include "term_shapes.h"

/* prototypes */
int init_from_file(char *fname, struct shape *s);
int init_cube(struct shape *c);
void destroy_shape(struct shape *s);
int reset_shape(struct shape *s);

#endif /* INIT_H */
