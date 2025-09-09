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
	ReadError
};

int parse_png(FILE* file, struct image_data* image);

enum png_chunk_type read_png_chunk(FILE* file, struct image_data* image);

unsigned int read_four_byte_integer(FILE* file);
enum png_chunk_type read_png_chunk_type(FILE* file);

unsigned char get_fifth_bit_from_byte(unsigned char byte);

#endif