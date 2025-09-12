#include "png_parser.h"

int parse_png(FILE* file, struct image_data* image)
{
    struct png_info image_info;
    image_info.has_palette = 0;
    image_info.read_first_idat_chunk = 0;

    image_info.data_stream = NULL;
    image_info.data_total_size = 0;

    enum png_chunk_type chunk_type;
    do
    {
        chunk_type = read_png_chunk(file, image, &image_info);
    } while (chunk_type != IEND && chunk_type != ReadError && chunk_type != IncorrectFormat);
	
    if (chunk_type == IncorrectFormat) printf("Unsupported PNG format.\n");
	if (chunk_type == ReadError || chunk_type == IncorrectFormat) return -1;

    printf("Read PNG. Decompressing...\n");

    char* decompressed_data = NULL;
    uLongf dest_len = 0;
    if (uncompress_zlib_data_stream(&image_info, image, &decompressed_data, &dest_len) == -1) return -1;

    printf("Decompressed PNG. Unfiltering...\n");
    if (unfilter_data_stream(&image_info, image, decompressed_data, dest_len) == -1) return -1;

    printf("Filtered PNG. Reading pixels...\n");
    if (fill_rgb_matrix(&image_info, image, decompressed_data, dest_len) == -1) return -1;

    return 0;
}

enum png_chunk_type read_png_chunk(FILE* file, struct image_data* image, struct png_info* image_info)
{
    unsigned int chunk_data_length = read_four_byte_integer(file);
	printf("Chunk data length: %d\n", chunk_data_length);

    enum png_chunk_type chunk_type = read_png_chunk_type(file);
    printf("Chunk type: %d\n", chunk_type);

    int result;
    switch (chunk_type) 
    {
        case IHDR:
            result = read_ihdr_chunk(file, image, image_info);
            if (result == -1) return ReadError;
            if (result == -2) return IncorrectFormat;
            break;
        case IEND:
            if (read_and_ignore_data(file, chunk_data_length) == -1) return ReadError;
            break;
        case IDAT:
            result = read_idat_chunk(file, image_info, chunk_data_length);
            if (result == -1) return ReadError;
            if (result == -2) return IncorrectFormat;
            break;
        case PLTE:
            result = read_plte_chunk(file, image_info, chunk_data_length);
            if (result == -1) return ReadError;
            if (result == -2) return IncorrectFormat;            
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

int read_ihdr_chunk(FILE* file, struct image_data* image, struct png_info* image_info)
{
    unsigned int width = read_four_byte_integer(file);
    unsigned int height = read_four_byte_integer(file);

    image->width = width;
    image->height = height;

    printf("Width: %d\nHeight: %d\n", width, height);

    image->pixel_rgb_matrix = malloc(width * height * sizeof(struct pixel_rgb));

    unsigned char bit_depth = getc(file);
    unsigned char color_type = getc(file);
    unsigned char compression_method = getc(file);
    unsigned char filter_method = getc(file);
    unsigned char interlace_method = getc(file);

    image_info->bit_depth = bit_depth;
    image_info->color_type = color_type;
    image_info->compression_method = compression_method;
    image_info->filter_method = filter_method;
    image_info->interlace_method = interlace_method;


    if (color_type == 1 || color_type == 5 || color_type > 6) return -2; // Non-existent formats

    if (color_type == 0) {
        if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 && bit_depth != 8 && bit_depth != 16) return -2; // Non-existent formats
        image_info->bits_per_pixel = bit_depth; // Single grayscale color
    }
    else if (color_type == 2) {
        if (bit_depth != 8 && bit_depth != 16) return -2; // Non-existent formats
        image_info->bits_per_pixel = bit_depth * 3; // RGB values
    }
    else if (color_type == 3) {
        if (bit_depth != 1 && bit_depth != 2 && bit_depth != 4 && bit_depth != 8) return -2; // Non-existent formats
        image_info->bits_per_pixel = bit_depth; // Palette index
    }
    else if (color_type == 4) {
        if (bit_depth != 8 && bit_depth != 16) return -2; // Non-existent formats
        image_info->bits_per_pixel = bit_depth * 2; // Grayscale + alpha
    }
    else if (color_type == 6) {
        if (bit_depth != 8 && bit_depth != 16) return -2; // Non-existent formats
        image_info->bits_per_pixel = bit_depth * 4; // RGBA values
    }

    image_info->bytes_per_pixel = (image_info->bits_per_pixel + 7) / 8; // + 7 to round up

    printf("Bit depth: %d\nColor type: %d\nCompression method: %d\nFilter method: %d\nInterlace method:%d\nBits per pixel:%d\nBytes per pixel:%d\n", bit_depth, color_type, compression_method, filter_method, interlace_method, image_info->bits_per_pixel, image_info->bytes_per_pixel);
    
    if (compression_method != 0 || filter_method != 0) return -2; // Invalid values, must be 0
    if (interlace_method != 0 && interlace_method != 1) return -2; // Invalid values, must be 0 or 1

    return 0;
}

int read_plte_chunk(FILE* file, struct png_info* image_info, int chunk_length)
{
    if (image_info->has_palette) return -2; // There must only be a palette
    if (image_info->read_first_idat_chunk) return -2; // Palettes, if present, must appear before IDAT chunks
    if (chunk_length % 3 != 0) return -2; // Must be a multiple of 3

    unsigned int number_of_colors = chunk_length / 3;
    if (number_of_colors > (1 << image_info->bit_depth)) return -2; // There cannot be more colors than can be represented with bit depth bits

    image_info->has_palette = 1;
    image_info->palette.number_of_colors = number_of_colors;
    image_info->palette.colors = malloc(number_of_colors * sizeof(struct png_palette_color));

    for (int i = 0; i < number_of_colors; ++i)
    {
        image_info->palette.colors[i].r = getc(file);
        image_info->palette.colors[i].g = getc(file);
        image_info->palette.colors[i].b = getc(file);
    }

    return 0;
}

int read_idat_chunk(FILE* file, struct png_info* image_info, unsigned int chunk_length)
{
    if (image_info->color_type == 3 && !image_info->has_palette) return -2; // If color type is 3, a palette must have been defined
    image_info->read_first_idat_chunk = 1;

    unsigned int new_total_size = image_info->data_total_size + chunk_length;
    unsigned int offset = image_info->data_total_size;

    unsigned char* new_data_stream = realloc(image_info->data_stream, new_total_size);
    if (!new_data_stream) return -1;
    image_info->data_stream = new_data_stream;

    if (fread(image_info->data_stream + offset, 1, chunk_length, file) != chunk_length) return -1;

    image_info->data_total_size = new_total_size;

    return 0;
}

int read_and_ignore_data(FILE* file, unsigned int bytes)
{
    char buff[bytes];
    if (fread(buff, 1, bytes, file) != bytes) return -1;

    return 0;
}

// TO DO: Make own uncompressor
int uncompress_zlib_data_stream(struct png_info* image_info, struct image_data* image, char** decompressed_data, uLongf* dest_len)
{
    int bytes_per_scanline = image->width * image_info->bytes_per_pixel + 1;
    unsigned int expected_size = image->height * bytes_per_scanline;

    *decompressed_data = malloc(expected_size);
    if (!*decompressed_data) return -1;

    *dest_len = expected_size;
    int result = uncompress(*decompressed_data, dest_len, image_info->data_stream, image_info->data_total_size); // Cheating!

    if (result != Z_OK) {
        free(*decompressed_data);
        return -1;
    }

    return 0;
}

int unfilter_data_stream(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int bytes_per_scanline = image->width * image_info->bytes_per_pixel + 1;

    for (int i = 0; i < image->height; ++i)
    {
        unsigned char unfilter_index = decompressed_data[i * bytes_per_scanline];
        printf("Scanline filter type: %d\n", unfilter_index);

        switch (unfilter_index)
        {
        case 1:
            if (unfilter_type_sub(decompressed_data, bytes_per_scanline, image_info->bytes_per_pixel, image->height, i) == -1) return -1;
            break;
        case 2:
            if (unfilter_type_up(decompressed_data, bytes_per_scanline, image_info->bytes_per_pixel, image->height, i) == -1) return -1;
            break;
        case 3:
            if (unfilter_type_average(decompressed_data, bytes_per_scanline, image_info->bytes_per_pixel, image->height, i) == -1) return -1;
            break;
        case 4:
            if (unfilter_type_paeth(decompressed_data, bytes_per_scanline, image_info->bytes_per_pixel, image->height, i) == -1) return -1;
            break;
        }
    }

    return 0;
}

int unfilter_type_sub(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int height, int line_index)
{
    int offset = line_index * bytes_per_scanline;

    for (int j = bytes_per_pixel + 1; j < bytes_per_scanline; ++j)
    {
        decompressed_data[offset + j] += decompressed_data[offset + j - bytes_per_pixel];
    }

    return 0;
}

int unfilter_type_up(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int height, int line_index)
{
    return 0;
}

int unfilter_type_average(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int height, int line_index)
{
    return 0;
}

int unfilter_type_paeth(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int height, int line_index)
{
    return 0;
}


int fill_rgb_matrix(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int decompressed_data_byte_index = 0;
    for (int i = 0; i < image->height; ++i)
    {
        // To account for filter type byte
        decompressed_data_byte_index++;

        for (int j = 0; j < image->width; ++j)
        {
            unsigned char r = decompressed_data[decompressed_data_byte_index++];
            unsigned char g = decompressed_data[decompressed_data_byte_index++];
            unsigned char b = decompressed_data[decompressed_data_byte_index++];

            image->pixel_rgb_matrix[i * image->width + j].r = r;
            image->pixel_rgb_matrix[i * image->width + j].g = g;
            image->pixel_rgb_matrix[i * image->width + j].b = b;

            printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", j, i, r, g, b);
        }

        // TO REMOVE, TO TEST FIRST LINE!
        return 0;
    }

    return 0;
}

unsigned char get_fifth_bit_from_byte(unsigned char byte)
{
    return (byte & 0x20) >> 5;
}