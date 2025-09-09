#ifndef PNG_PARSER_HH
#define PNG_PARSER_HH

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

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

struct png_header_info {
	unsigned char bit_depth;
	unsigned char color_type;
	unsigned char compression_method;
	unsigned char filter_method;
	unsigned char interlace_method;
};

int parse_png(FILE* file, struct image_data* image);

enum png_chunk_type read_png_chunk(FILE* file, struct image_data* image, struct png_header_info* header_info);

unsigned int read_four_byte_integer(FILE* file);
enum png_chunk_type read_png_chunk_type(FILE* file);

int read_ihdr_chunk(FILE* file, struct image_data* image, struct png_header_info* header_info);
int read_and_ignore_data(FILE* file, unsigned int bytes);

unsigned char get_fifth_bit_from_byte(unsigned char byte);

#endif