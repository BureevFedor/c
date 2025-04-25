#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#include "stdlib.h"

struct pixel {
    uint8_t b, g, r;
};

struct image {
    uint64_t width, height;
    struct pixel* data;
};

struct image create_image(uint64_t width, uint64_t height);
void free_image(struct image* image);

void free_heap(struct image* input_image, struct image* output_image);

#endif