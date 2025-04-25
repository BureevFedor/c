#include "../include/image.h"

struct image create_image(uint64_t width, uint64_t height) {
    struct image img = {0};
    img.data = malloc(width * height * sizeof(struct pixel));
    if (!img.data) return img;
    img.width = width;
    img.height = height;
    return img;
}

void free_image(struct image* image) {
    free(image->data);
}

void free_heap(struct image* input_image, struct image* output_image) {
    free_image(input_image);
    if (output_image->data != input_image->data) {
        free_image(output_image);
    }
}