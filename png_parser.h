#ifndef PNG_PARSER_HH
#define PNG_PARSER_HH

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <zlib.h>

#include "image.h"

enum png_chunk_type {
	IHDR,
	IEND,
	IDAT,
	PLTE,
	Ancillary,
	Private,
	ReadError,
	IncorrectFormat
};

struct png_palette_color {
	unsigned int r;
	unsigned int g;
	unsigned int b;
};

struct png_palette {
	unsigned int number_of_colors;

	struct png_palette_color* colors;
};

struct png_info {
	unsigned char bit_depth;
	unsigned char color_type;
	unsigned char compression_method;
	unsigned char filter_method;
	unsigned char interlace_method;

	unsigned int bits_per_pixel;
	unsigned int bytes_per_pixel;

	unsigned char read_first_idat_chunk;
	unsigned char has_palette;
	struct png_palette palette;

	unsigned char* data_stream;
	unsigned int data_total_size;
};

int parse_png(FILE* file, struct image_data* image);

enum png_chunk_type read_png_chunk(FILE* file, struct image_data* image, struct png_info* image_info);

unsigned int read_four_byte_integer(FILE* file);
enum png_chunk_type read_png_chunk_type(FILE* file);

int read_ihdr_chunk(FILE* file, struct image_data* image, struct png_info* image_info);
int read_plte_chunk(FILE* file, struct png_info* image_info, int chunk_length);
int read_idat_chunk(FILE* file, struct png_info* image_info, unsigned int chunk_length);
int read_and_ignore_data(FILE* file, unsigned int bytes);

int uncompress_zlib_data_stream(struct png_info* image_info, struct image_data* image, char** decompressed_data, uLongf* dest_len);
int unfilter_data_stream(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);

int unfilter_type_sub(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int line_index);
int unfilter_type_up(char* decompressed_data, int bytes_per_scanline, int line_index);
int unfilter_type_average(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int line_index);
int unfilter_type_paeth(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int line_index);

int fill_rgb_matrix(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);

int fill_rgb_matrix_rgb(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);
int fill_rgb_matrix_rgb_alpha(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);
int fill_rgb_matrix_grayscale_8_16(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);
int fill_rgb_matrix_grayscale_1_2_4(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);
int fill_rgb_matrix_grayscale_alpha(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);
int fill_rgb_matrix_palette_8(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);
int fill_rgb_matrix_palette_1_2_4(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length);

unsigned char get_fifth_bit_from_byte(unsigned char byte);

#endif