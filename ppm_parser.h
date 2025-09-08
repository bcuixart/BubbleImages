#ifndef PPM_PARSER_HH
#define PPM_PARSER_HH

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "image.h"

int parse_ppm(FILE* file, struct image_data* image, char ppm_type);
int save_ppm(char* filename, struct image_data* image);

int parse_ppm_p3_pixel_data(FILE* file, struct image_data* image, int maxcolorval, char bytesperpixel);
int parse_ppm_p6_pixel_data(FILE* file, struct image_data* image, int maxcolorval, char bytesperpixel);

unsigned char get_rgb_ascii_number_from_ppm(FILE* file, int maxcolorval);
unsigned char get_rgb_binary_number_from_ppm(FILE* file, int maxcolorval, char bytes);

void ignore_ppm_comment_line(FILE* file);
int get_ascii_number_from_ppm(FILE* file);

float min(float a, float b);

#endif