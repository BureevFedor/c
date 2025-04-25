#ifndef ROTATION_H
#define ROTATION_H

#include "image.h"
#include <string.h>

typedef struct image (*transform_func)(const struct image*);

struct transform_entry {
    const char* name;
    transform_func func;
};

extern struct transform_entry transforms[];

transform_func find_transform(const char* name);

struct image none(const struct image* input_image);
struct image rotate_left(const struct image* input_image);
struct image rotate_right(const struct image* input_image);
struct image flip_horizontal(const struct image* input_image);
struct image flip_vertical(const struct image* input_image);

#endif