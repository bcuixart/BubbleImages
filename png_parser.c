#include "png_parser.h"

int parse_png(FILE* file, struct image_data* image)
{
    unsigned char bit_depth = 0;
    unsigned char color_type = 0;
    unsigned char compression_method = 0;
    unsigned char filter_method = 0;
    unsigned char interlace_method = 0;

    enum png_chunk_type chunk_type;
    do
    {
        chunk_type = read_png_chunk(file, image, &bit_depth, &color_type, &compression_method, &filter_method, &interlace_method);
    } while (chunk_type != IEND && chunk_type != ReadError && chunk_type != IncorrectFormat);
	
    if (chunk_type == IncorrectFormat) printf("Unsupported PNG format.\n");
	return (chunk_type == ReadError || chunk_type == IncorrectFormat) ? -1 : 0;
}

enum png_chunk_type read_png_chunk(FILE* file, struct image_data* image, unsigned char* bit_depth, unsigned char* color_type, unsigned char* compression_method, unsigned char* filter_method, unsigned char* interlace_method)
{
    unsigned int chunk_data_length = read_four_byte_integer(file);
	printf("Chunk data length: %d\n", chunk_data_length);

    enum png_chunk_type chunk_type = read_png_chunk_type(file);
    printf("Chunk type: %d\n", chunk_type);

    switch (chunk_type) 
    {
        case IHDR:
            int result = read_ihdr_chunk(file, image, bit_depth, color_type, compression_method, filter_method, interlace_method);
            if (result == -1) return ReadError;
            if (result == -2) return IncorrectFormat;
            break;
        case IEND:
            if (read_and_ignore_data(file, chunk_data_length) == -1) return ReadError;
            break;
        case IDAT:
            if (read_and_ignore_data(file, chunk_data_length) == -1) return ReadError;
            break;
        case PLTE:
            if (read_and_ignore_data(file, chunk_data_length) == -1) return ReadError;
            break;
        case Ancillary:
            if (read_and_ignore_data(file, chunk_data_length) == -1) return ReadError;
            break;
        case Private:
            if (read_and_ignore_data(file, chunk_data_length) == -1) return ReadError;
            break;
        case ReadError:
            return ReadError;
            break;
        case IncorrectFormat:
            return IncorrectFormat;
            break;
    }
        
    unsigned int chunk_crc = read_four_byte_integer(file);
    printf("Chunk crc: %d\n\n", chunk_crc);

    return chunk_type;
}

unsigned int read_four_byte_integer(FILE* file)
{
    unsigned char bytes[4];
    if (fread(bytes, 1, 4, file) != 4) return 0xFFFFFFFF;

    unsigned int result =
        ((unsigned int)bytes[0] << 24) |
        ((unsigned int)bytes[1] << 16) |
        ((unsigned int)bytes[2] << 8) |
        ((unsigned int)bytes[3]);

    return result;
}

enum png_chunk_type read_png_chunk_type(FILE* file)
{
    unsigned char buff[5];
    if (fread(buff, 1, 4, file) != 4) return ReadError;
    buff[4] = '\0';

    if (strcmp(buff, "IDAT") == 0) return IDAT;
    if (strcmp(buff, "IHDR") == 0) return IHDR;
    if (strcmp(buff, "IEND") == 0) return IEND;
    if (strcmp(buff, "PLTE") == 0) return PLTE;

    if (get_fifth_bit_from_byte(buff[0]) == 1) return Ancillary; // Ancillary chunk (not needed to display image, must ignore)
    if (get_fifth_bit_from_byte(buff[1]) == 1) return Private; // Private chunk (use not defined by standard, should ignore)
    // 3rd byte is reserved, its value does not affect anything
    // 4th byte is only for copying files, not needed

    return ReadError;
}

int read_ihdr_chunk(FILE* file, struct image_data* image, unsigned char* bit_depth, unsigned char* color_type, unsigned char* compression_method, unsigned char* filter_method, unsigned char* interlace_method)
{
    unsigned int width = read_four_byte_integer(file);
    unsigned int height = read_four_byte_integer(file);

    image->width = width;
    image->height = height;

    printf("Width: %d\nHeight: %d\n", width, height);

    image->pixel_rgb_matrix = malloc(width * height * sizeof(struct pixel_rgb));

    *bit_depth = getc(file);
    *color_type = getc(file);
    *compression_method = getc(file);
    *filter_method = getc(file);
    *interlace_method = getc(file);

    printf("Bit depth: %d\nColor type: %d\nCompression method: %d\nFilter method: %d\nInterlace method:%d\n", *bit_depth, *color_type, *compression_method, *filter_method, *interlace_method);

    if (*color_type == 1 || *color_type == 5 || *color_type > 6) return -2; // Non-existent formats

    if (*color_type == 0) {
        if (*bit_depth != 1 && *bit_depth != 2 && *bit_depth != 4 && *bit_depth != 8 && *bit_depth != 16) return -2; // Non-existent formats
    }
    else if (*color_type == 2 || *color_type == 4 || *color_type == 6) {
        if (*bit_depth != 8 && *bit_depth != 16) return -2; // Non-existent formats
    }
    if (*color_type == 3) {
        if (*bit_depth != 1 && *bit_depth != 2 && *bit_depth != 4 && *bit_depth != 8) return -2; // Non-existent formats
    }

    if (*compression_method != 0 || *filter_method != 0) return -2; // Invalid values, must be 0
    if (*interlace_method != 0 && *interlace_method != 1) return -2; // Invalid values, must be 0 or 1

    return 0;
}

int read_and_ignore_data(FILE* file, unsigned int bytes)
{
    for (int i = 0; i < bytes; ++i) if(getc(file) == -1) return -1;
}

unsigned char get_fifth_bit_from_byte(unsigned char byte)
{
    return (byte & 0x20) >> 5;
}