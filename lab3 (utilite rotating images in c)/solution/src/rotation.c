#include "../include/rotation.h"

struct transform_entry transforms[] = {
    {"none", none},
    {"cw90", rotate_right},
    {"ccw90", rotate_left},
    {"fliph", flip_horizontal},
    {"flipv", flip_vertical},
};

static size_t offset_by_coords(const struct image* image, size_t x, size_t y) {
    return x + y * image->width;
}

struct image none(const struct image* input_image) {
    return *input_image;
}

struct image rotate_left(const struct image* input_image) {
    struct image transformed = create_image(input_image->height, input_image->width);
    if (!transformed.data) return transformed;
    for (size_t y = 0; y < input_image->height; y++) {
        for (size_t x = 0; x < input_image->width; x++) {
            transformed.data[offset_by_coords(&transformed, input_image->height - y - 1, x)] = input_image->data[offset_by_coords(input_image, x, y)];
        }
    }
    return transformed;
}

struct image rotate_right(const struct image* input_image) {
    struct image transformed = create_image(input_image->height, input_image->width);
    if (!transformed.data) return transformed;
    for (size_t y = 0; y < input_image->height; y++) {
        for (size_t x = 0; x < input_image->width; x++) {
            transformed.data[offset_by_coords(&transformed, y, input_image->width - x - 1)] = input_image->data[offset_by_coords(input_image, x, y)];
        }
    }
    return transformed;
}

struct image flip_horizontal(const struct image* input_image) {
    struct image transformed = create_image(input_image->width, input_image->height);
    if (!transformed.data) return transformed;
    for (size_t y = 0; y < input_image->height; y++) {
        for (size_t x = 0; x < input_image->width; x++) {
            transformed.data[offset_by_coords(&transformed, input_image->width - x - 1, y)] = input_image->data[offset_by_coords(input_image, x, y)];
        }
    }
    return transformed;
}

struct image flip_vertical(const struct image* input_image) {
    struct image transformed = create_image(input_image->width, input_image->height);
    if (!transformed.data) return transformed;
    for (size_t y = 0; y < input_image->height; y++) {
        for (size_t x = 0; x < input_image->width; x++) {
            transformed.data[offset_by_coords(&transformed, x, input_image->height - y - 1)] = input_image->data[offset_by_coords(input_image, x, y)];
        }
    }
    return transformed;
}

transform_func find_transform(const char* name) {
    if (name == NULL) return NULL;
    for (size_t i = 0; i < sizeof(transforms) / sizeof(transforms[0]); i++) {
        if (strcmp(transforms[i].name, name) == 0) {
            return transforms[i].func;
        }
    }
    return NULL;
}