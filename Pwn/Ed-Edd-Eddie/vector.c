#include "vector.h"

void vector_init(struct vector *v)
{
    v->data = NULL;
    v->size = 0;
    v->count = 0;
}

size_t vector_count(struct vector *v)
{
    return v->count;
}

void vector_add(struct vector *v, struct ed_str *e)
{
    if (v->size == 0) {
        v->size = 10;
        v->data = malloc(sizeof(struct ed_str *) * v->size);

        if (NULL == v->data) {
            exit(0);
        }

        memset(v->data, '\0', sizeof(struct ed_str *) * v->size);
    }

    if (v->size == v->count) {
        v->size *= 2;
        v->data = realloc(v->data, sizeof(struct ed_str *) * v->size);

        if (NULL == v->data) {
            exit(0);
        }
    }

    v->data[v->count] = e;
    v->count++;
}

void vector_insert(struct vector *v, size_t index, struct ed_str *e)
{
    size_t i;
    if (index > v->count) {
        return;
    }

    if (index == v->count) {
        vector_add(v, e);
        return;
    }

    if (v->size == v->count + 1) {
        v->size += 1;
        v->data = realloc(v->data, sizeof(struct ed_str *) * v->size);

        if (NULL == v->data) {
            exit(0);
        }
    }

    for (i = v->count - 1; i > index; i--) {
        v->data[i + 1] = v->data[i];
    }
    v->data[index] = e;
    v->count++;
}

void vector_set(struct vector *v, size_t index, struct ed_str *e)
{
    if (index >= v->count) {
        return;
    }

    free(v->data[index]->str);
    free(v->data[index]);
    v->data[index] = e;
}

struct ed_str *vector_get(struct vector *v, size_t index)
{
    if (index >= v->count) {
        return NULL;
    }

    return v->data[index];
}

void vector_delete(struct vector *v, size_t index)
{
    size_t i;

    if (index >= v->count) {
        return;
    }

    free(v->data[index]->str);
    free(v->data[index]);
    for (i = index; i < v->count - 1; i++) {
        v->data[i] = v->data[i + 1];
    }
    // This should be here, but it makes exploitation easier
    // v->data[i] = NULL;

    v->count--;
}

void vector_free(struct vector *v)
{
    // This should be here
    /*
    size_t i;

    for (i = 0; i < v->count; i++) {
        free(v->data[i]->str);
        free(v->data[i]);
    }
    */
    free(v->data);

    v->count = 0;
    v->size = 0;
    v->data = NULL;
}

void vector_print(struct vector *v) {
    size_t i;
    for (i = 0; i < vector_count(v); i++) {
        printf("[%zu] = %s\n", i, vector_get(v, i)->str);
    }
}
