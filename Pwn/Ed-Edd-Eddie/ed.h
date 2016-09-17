#ifndef ED_H
#define ED_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <dlfcn.h>

#include "vector.h"

#define DEBUG 1

#ifdef DEBUG
#define debug(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define debug(M, ...)
#endif

#define print_err(x) fprintf(stderr, x); exit(0);
#define MAX_MARKS 255
#define STDIN 0

enum cmd_type {APPEND, PRINT, PRINT_NUM, INSERT, DELETE, JOIN, CHANGE, MOVE, MARK, COPY, LIST, FIND, READ, WRITE, LOAD, RUN, SET_LINE, QUIT, ERROR};

struct ed_command {
    enum cmd_type cmd;
    size_t src_start;
    size_t src_end;
    size_t dst_start;
    size_t dst_end;
    char mark_char;
    char *path;
};

struct ed_mark {
    size_t line;
    char mark_char;
};

struct ed_properties {
    char *filename;
    bool can_save;
    // File buffer is an array of lines
    struct vector *line_buffer;
    size_t current_line;
    struct ed_mark marks[MAX_MARKS];
    int mark_count;
};

struct ed_str {
    char *str;
    size_t len;
};

extern struct ed_properties ed_props;

void *scalloc(size_t);
void *srealloc(void *, size_t);

struct ed_str *ed_read_line();

void ed_prompt();

#endif
