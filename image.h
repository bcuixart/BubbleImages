#ifndef IMAGE_HH
#define IMAGE_HH

#include <stdio.h>
#include <stdlib.h>

struct pixel_rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct image_data {
	unsigned int width;
	unsigned int height;
	struct pixel_rgb* pixel_rgb_matrix;
};

enum ImageType {
	PPM_P3,
	PPM_P6,
	UNKNOWN
};

#include "ppm_parser.h" // Si, ha d'anar aqui.

int read_image(char* filename, struct image_data* result);

enum ImageType get_image_type_from_file(FILE* file);

struct image_data get_smaller_image_data(struct image_data* ori_data, int new_width, int new_height);
int make_smaller_image(char* filename, struct image_data* ori_data, int new_width, int new_height);

struct pixel_rgb get_average_color_from_image_pixels(struct image_data* image, int from_x, int to_x, int from_y, int to_y);

#endif