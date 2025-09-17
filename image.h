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
	PNG,
	PPM_P3,
	PPM_P6,
	UNKNOWN
};

// Si, ha d'anar qui.
#include "png_parser.h"
#include "ppm_parser.h"

int read_image(char* filename, struct image_data** result);
int save_image_as_png(struct image_data* data, char* filename);
int save_image_as_ppm(struct image_data* data, char* filename);

enum ImageType get_image_type_from_file(FILE* file);

int get_smaller_image_data(struct image_data* ori_data, struct image_data* new_data, int new_width, int new_height);
struct pixel_rgb get_average_color_from_image_pixels(struct image_data* image, int from_x, int to_x, int from_y, int to_y);

#endif