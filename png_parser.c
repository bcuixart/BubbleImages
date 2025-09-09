#include "png_parser.h"

int parse_png(FILE* file, struct image_data* image)
{
    enum png_chunk_type chunk_type;
    do
    {
        chunk_type = read_png_chunk(file, image);
    } while (chunk_type != IEND && chunk_type != ReadError);
	
	return (chunk_type == ReadError) ? -1 : 0;
}

enum png_chunk_type read_png_chunk(FILE* file, struct image_data* image)
{
    unsigned int chunk_data_length = read_four_byte_integer(file);
	printf("Chunk data length: %d\n", chunk_data_length);

    enum png_chunk_type chunk_type = read_png_chunk_type(file);
    printf("Chunk type: %d\n", chunk_type);

    for (int i = 0; i < chunk_data_length; ++i)
    {
        unsigned char c = getc(file);
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

unsigned char get_fifth_bit_from_byte(unsigned char byte)
{
    return (byte & 0x20) >> 5;
}