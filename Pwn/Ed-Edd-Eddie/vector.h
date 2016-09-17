#ifndef VECTOR_H
#define VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ed.h"

struct vector {
    size_t size;
    size_t count;
    struct ed_str** data;
};

void vector_init(struct vector *);
size_t vector_count(struct vector *);
void vector_add(struct vector *, struct ed_str *);
void vector_set(struct vector *, size_t, struct ed_str *);
void vector_insert(struct vector *, size_t, struct ed_str *);
struct ed_str *vector_get(struct vector *, size_t);
void vector_delete(struct vector *, size_t);
void vector_free(struct vector *);
void vector_print(struct vector *);

#endif
