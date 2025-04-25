#include "../include/bmp_utils.h"

enum read_status validate_bmp_header(FILE* input_file, struct bmp_header* header) {
    if (!fread(header, sizeof(struct bmp_header), 1, input_file)) {
        return READ_INVALID_HEADER;
    }
    if (header->bfType != BMP_FTYPE) {
        return READ_INVALID_SIGNATURE;
    }
    if (header->biBitCount != BMP_BIT_COUNT) {
        return READ_INVALID_BITS;
    }
    return READ_OK;
}

struct bmp_header create_bmp_header(struct image image) {
    struct bmp_header header = {0};
    header.bfType = BMP_FTYPE;
    header.biSize = BMP_SIZE;
    header.bfileSize = sizeof(struct bmp_header) + image.width * image.height * sizeof(struct pixel);
    header.bOffBits = sizeof(struct bmp_header);
    header.biWidth = image.width;
    header.biHeight = image.height;
    header.biPlanes = BMP_PLANES;
    header.biBitCount = BMP_BIT_COUNT;
    header.biSizeImage = image.width * image.height * sizeof(struct pixel);
    return header;
}

uint32_t count_padding(uint32_t width) {
    return (BMP_ALIGNMENT - (width * BYTES_PER_PIXEL) % BMP_ALIGNMENT) % BMP_ALIGNMENT;
}