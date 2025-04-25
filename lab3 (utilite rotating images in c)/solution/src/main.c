#include "../include/bmp.h"
//#include "../include/file_utils.h"
#include "../include/image.h"
#include "../include/rotation.h"
#include <errno.h>

int main(int argc, char* argv[]) {

    if (argc != 4) {
        perror("Not valid argc");
        return EXIT_FAILURE;
    }

    const char* input_file_path = argv[1];
    const char* output_file_path = argv[2];
    const char* transform = argv[3];

    transform_func transform_func = find_transform(transform);
    if (!transform_func) {
        perror("Not valid transform");
        return ENOENT;
    }

    //FILE* input_file = read_file(input_file_path);
    FILE *input_file = fopen(input_file_path, "rb");
    if (!input_file) {
        fprintf(stderr, "Invalid file %s", input_file_path);
    }

    if (!input_file) {
        return ENOENT;
    }

    struct image input_image;
    enum read_status read_status = from_bmp(input_file, &input_image);

    //close_file(input_file);
    if (!input_file) {
        perror("Closing file is NULL");
    }
    if (!fclose(input_file)) {
        perror("Can't close file");
    }

    if (read_status != READ_OK) {
        perror("Invalid BMP");
        return read_status;
    }

    struct image output_image = transform_func(&input_image);
    if (!output_image.data) {
        return ENOMEM;
    }

    //FILE* output_file = write_file(output_file_path);
    FILE *output_file = fopen(output_file_path, "wb");
    if (!output_file) {
        fprintf(stderr, "Invalid file %s", output_file_path);
    }
    
    if (!output_file) {
        return ENOENT;
    }

    enum write_status write_status = to_bmp(output_file, &output_image);
    //close_file(output_file);
    if (!output_file) {
        perror("Closing file is NULL");
    }
    if (!fclose(output_file)) {
        perror("Can't close file");
    }
    
    if (write_status != WRITE_OK) {
        free_heap(&input_image, &output_image);
        perror("Failed to write BMP file");
        return write_status;
    }

    free_heap(&input_image, &output_image);

    return 0;
}