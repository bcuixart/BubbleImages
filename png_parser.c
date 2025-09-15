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

    //printf("Read PNG. Decompressing...\n");

    char* decompressed_data = NULL;
    uLongf dest_len = 0;
    if (uncompress_zlib_data_stream(&image_info, image, &decompressed_data, &dest_len) == -1) return -1;

    //printf("Decompressed PNG. Unfiltering...\n");
    if (unfilter_data_stream(&image_info, image, decompressed_data, dest_len) == -1) return -1;

    //printf("Filtered PNG. Reading pixels...\n");
    if (fill_rgb_matrix(&image_info, image, decompressed_data, dest_len) == -1) {
        if (image_info.palette.colors) free(image_info.palette.colors);
        free(decompressed_data);
        return -1;
    }

    //printf("Correctly read.\n");

    if (image_info.palette.colors) free(image_info.palette.colors);
    if (decompressed_data) free(decompressed_data);

    return 0;
}

enum png_chunk_type read_png_chunk(FILE* file, struct image_data* image, struct png_info* image_info)
{
    unsigned int chunk_data_length = read_four_byte_integer(file);
	//printf("Chunk data length: %d\n", chunk_data_length);

    enum png_chunk_type chunk_type = read_png_chunk_type(file);
    //printf("Chunk type: %d\n", chunk_type);

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
    //printf("Chunk crc: %d\n\n", chunk_crc);

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
    if (!image->pixel_rgb_matrix) return -1;

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
    if (!image_info->palette.colors) return -1;

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
        //printf("Scanline filter type: %d\n", unfilter_index);

        switch (unfilter_index)
        {
        case 1:
            if (unfilter_type_sub(decompressed_data, bytes_per_scanline, image_info->bytes_per_pixel, i) == -1) return -1;
            break;
        case 2:
            if (unfilter_type_up(decompressed_data, bytes_per_scanline, i) == -1) return -1;
            break;
        case 3:
            if (unfilter_type_average(decompressed_data, bytes_per_scanline, image_info->bytes_per_pixel, i) == -1) return -1;
            break;
        case 4:
            if (unfilter_type_paeth(decompressed_data, bytes_per_scanline, image_info->bytes_per_pixel, i) == -1) return -1;
            break;
        }
    }

    return 0;
}

int unfilter_type_sub(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int line_index)
{
    int offset = line_index * bytes_per_scanline;

    for (int j = bytes_per_pixel + 1; j < bytes_per_scanline; ++j)
    {
        decompressed_data[offset + j] += decompressed_data[offset + j - bytes_per_pixel];
    }

    return 0;
}

int unfilter_type_up(char* decompressed_data, int bytes_per_scanline, int line_index)
{
    if (line_index <= 0) return 0; // Does not apply to first line

    int offset = line_index * bytes_per_scanline;
    int offset_prev_line = (line_index - 1) * bytes_per_scanline;

    for (int j = 1; j < bytes_per_scanline; ++j)
    {
        decompressed_data[offset + j] += decompressed_data[offset_prev_line + j];
    }

    return 0;
}

int unfilter_type_average(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int line_index)
{
    int offset = line_index * bytes_per_scanline;
    int offset_prev_line = (line_index - 1) * bytes_per_scanline;

    for (int j = 1; j < bytes_per_scanline; ++j)
    {
        unsigned char left = (j >= (bytes_per_pixel + 1)) ? decompressed_data[offset + j - bytes_per_pixel] : 0;
        unsigned char up = (line_index > 0) ? decompressed_data[offset_prev_line + j] : 0;

        decompressed_data[offset + j] += (left + up) / 2;
    }

    return 0;
}

int unfilter_type_paeth(char* decompressed_data, int bytes_per_scanline, int bytes_per_pixel, int line_index)
{
    int offset = line_index * bytes_per_scanline;
    int offset_prev_line = (line_index - 1) * bytes_per_scanline;

    for (int j = 1; j < bytes_per_scanline; ++j)
    {
        unsigned char left = (j >= (bytes_per_pixel + 1)) ? decompressed_data[offset + j - bytes_per_pixel] : 0;
        unsigned char up = (line_index > 0) ? decompressed_data[offset_prev_line + j] : 0;
        unsigned char up_left = (j >= (bytes_per_pixel + 1) && line_index > 0) ? decompressed_data[offset_prev_line + j - bytes_per_pixel] : 0;

        int p = left + up - up_left;
        int p_left = abs(p - left);
        int p_up = abs(p - up);
        int p_up_left = abs(p - up_left);

        if (p_left <= p_up && p_left <= p_up_left) p = left;
        else if (p_up <= p_up_left) p = up;
        else p = up_left;

        decompressed_data[offset + j] += p;
    }

    return 0;
}

int fill_rgb_matrix(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    switch (image_info->color_type)
    {
    case 0:
        if (image_info->bit_depth == 8 || image_info->bit_depth == 16) return fill_rgb_matrix_grayscale_8_16(image_info, image, decompressed_data, decompressed_data_length);
        else return fill_rgb_matrix_grayscale_1_2_4(image_info, image, decompressed_data, decompressed_data_length);
        break;
    case 2:
        return fill_rgb_matrix_rgb(image_info, image, decompressed_data, decompressed_data_length);
        break;    
    case 3:
        if (image_info->bit_depth == 8) return fill_rgb_matrix_palette_8(image_info, image, decompressed_data, decompressed_data_length);
        else return fill_rgb_matrix_palette_1_2_4(image_info, image, decompressed_data, decompressed_data_length);
        break;
    case 4:
        return fill_rgb_matrix_grayscale_alpha(image_info, image, decompressed_data, decompressed_data_length);
        break;
    case 6:
        return fill_rgb_matrix_rgb_alpha(image_info, image, decompressed_data, decompressed_data_length);
        break;
    }
}

int fill_rgb_matrix_rgb(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int decompressed_data_byte_index = 0;
    for (int i = 0; i < image->height; ++i)
    {
        // To account for filter type byte
        decompressed_data_byte_index++;

        for (int j = 0; j < image->width; ++j)
        {
            unsigned int r;
            unsigned int g;
            unsigned int b;

            if (image_info->bit_depth == 8)
            {
                r = decompressed_data[decompressed_data_byte_index++];
                g = decompressed_data[decompressed_data_byte_index++];
                b = decompressed_data[decompressed_data_byte_index++];
            }
            else if (image_info->bit_depth == 16)
            {
                unsigned char r2;
                unsigned char g2;
                unsigned char b2;

                r = decompressed_data[decompressed_data_byte_index++];
                r2 = decompressed_data[decompressed_data_byte_index++];
                g = decompressed_data[decompressed_data_byte_index++];
                g2 = decompressed_data[decompressed_data_byte_index++];
                b = decompressed_data[decompressed_data_byte_index++];
                b2 = decompressed_data[decompressed_data_byte_index++];

                r = (r2 * 256 + r) / 255;
                g = (g2 * 256 + g) / 255;
                b = (b2 * 256 + b) / 255;
            }

            image->pixel_rgb_matrix[i * image->width + j].r = (unsigned char) r;
            image->pixel_rgb_matrix[i * image->width + j].g = (unsigned char) g;
            image->pixel_rgb_matrix[i * image->width + j].b = (unsigned char) b;

            //printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", j, i, r, g, b);
        }
    }

    return 0;
}

int fill_rgb_matrix_rgb_alpha(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int decompressed_data_byte_index = 0;
    for (int i = 0; i < image->height; ++i)
    {
        // To account for filter type byte
        decompressed_data_byte_index++;

        for (int j = 0; j < image->width; ++j)
        {
            unsigned int r;
            unsigned int g;
            unsigned int b;
            unsigned int a;

            if (image_info->bit_depth == 8)
            {
                r = decompressed_data[decompressed_data_byte_index++];
                g = decompressed_data[decompressed_data_byte_index++];
                b = decompressed_data[decompressed_data_byte_index++];
                a = decompressed_data[decompressed_data_byte_index++];
            }
            else if (image_info->bit_depth == 16)
            {
                unsigned char r2;
                unsigned char g2;
                unsigned char b2;
                unsigned char a2;

                r = decompressed_data[decompressed_data_byte_index++];
                r2 = decompressed_data[decompressed_data_byte_index++];
                g = decompressed_data[decompressed_data_byte_index++];
                g2 = decompressed_data[decompressed_data_byte_index++];
                b = decompressed_data[decompressed_data_byte_index++];
                b2 = decompressed_data[decompressed_data_byte_index++];                
                a = decompressed_data[decompressed_data_byte_index++];
                a2 = decompressed_data[decompressed_data_byte_index++];

                r = (r2 * 256 + r) / 255;
                g = (g2 * 256 + g) / 255;
                b = (b2 * 256 + b) / 255;
            }

            image->pixel_rgb_matrix[i * image->width + j].r = (unsigned char)r;
            image->pixel_rgb_matrix[i * image->width + j].g = (unsigned char)g;
            image->pixel_rgb_matrix[i * image->width + j].b = (unsigned char)b;

            //printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", j, i, r, g, b);
        }
    }

    return 0;
}

int fill_rgb_matrix_grayscale_8_16(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int decompressed_data_byte_index = 0;
    for (int i = 0; i < image->height; ++i)
    {
        // To account for filter type byte
        decompressed_data_byte_index++;

        for (int j = 0; j < image->width; ++j)
        {
            unsigned int gs;

            if (image_info->bit_depth == 8)
            {
                gs = decompressed_data[decompressed_data_byte_index++];
            }
            else if (image_info->bit_depth == 16)
            {
                unsigned char gs2;

                gs = decompressed_data[decompressed_data_byte_index++];
                gs2 = decompressed_data[decompressed_data_byte_index++];

                gs = (gs2 * 256 + gs) / 255;
            }

            image->pixel_rgb_matrix[i * image->width + j].r = (unsigned char)gs;
            image->pixel_rgb_matrix[i * image->width + j].g = (unsigned char)gs;
            image->pixel_rgb_matrix[i * image->width + j].b = (unsigned char)gs;

            //printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", j, i, r, g, b);
        }
    }

    return 0;
}

int fill_rgb_matrix_grayscale_1_2_4(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int decompressed_data_byte_index = 0;
    for (int i = 0; i < image->height; ++i)
    {
        // To account for filter type byte
        decompressed_data_byte_index++;

        int bits_read = 0;
        for (int j = 0; j < image->width; ++j)
        {
            int byte_offset = decompressed_data_byte_index + bits_read / 8;
            int bit_offset = bits_read % 8;

            unsigned char byte = decompressed_data[byte_offset];
            unsigned char gs;

            switch (image_info->bit_depth)
            {
            case 1:
                gs = (byte >> (7 - bit_offset)) & 0x01;
                gs = gs * 255;
                bits_read += 1;
                break;
            case 2:
                gs = (gs >> (6 - bit_offset)) & 0x03;
                gs = (gs * 255) / 3;
                bits_read += 2;
                break;
            case 4:
                gs = (byte >> (4 - bit_offset)) & 0x0F;
                gs = (gs * 255) / 15;
                bits_read += 4;
                break;
            }

            image->pixel_rgb_matrix[i * image->width + j].r = gs;
            image->pixel_rgb_matrix[i * image->width + j].g = gs;
            image->pixel_rgb_matrix[i * image->width + j].b = gs;
        }

        decompressed_data_byte_index += (image->width * image_info->bit_depth + 7) / 8;
    }

    return 0;
}

int fill_rgb_matrix_grayscale_alpha(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int decompressed_data_byte_index = 0;
    for (int i = 0; i < image->height; ++i)
    {
        // To account for filter type byte
        decompressed_data_byte_index++;

        for (int j = 0; j < image->width; ++j)
        {
            unsigned int gs;
            unsigned int a;

            if (image_info->bit_depth == 8)
            {
                gs = decompressed_data[decompressed_data_byte_index++];
                a = decompressed_data[decompressed_data_byte_index++];
            }
            else if (image_info->bit_depth == 16)
            {
                unsigned char gs2;
                unsigned char a2;

                gs = decompressed_data[decompressed_data_byte_index++];
                gs2 = decompressed_data[decompressed_data_byte_index++];
                a = decompressed_data[decompressed_data_byte_index++];
                a2 = decompressed_data[decompressed_data_byte_index++];

                gs = (gs2 * 256 + gs) / 255;
            }

            image->pixel_rgb_matrix[i * image->width + j].r = (unsigned char)gs;
            image->pixel_rgb_matrix[i * image->width + j].g = (unsigned char)gs;
            image->pixel_rgb_matrix[i * image->width + j].b = (unsigned char)gs;

            //printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", j, i, r, g, b);
        }
    }

    return 0;
}

int fill_rgb_matrix_palette_8(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int decompressed_data_byte_index = 0;
    for (int i = 0; i < image->height; ++i)
    {
        // To account for filter type byte
        decompressed_data_byte_index++;

        for (int j = 0; j < image->width; ++j)
        {
            unsigned char palette_index = decompressed_data[decompressed_data_byte_index++];

            image->pixel_rgb_matrix[i * image->width + j].r = image_info->palette.colors[palette_index].r;
            image->pixel_rgb_matrix[i * image->width + j].g = image_info->palette.colors[palette_index].g;
            image->pixel_rgb_matrix[i * image->width + j].b = image_info->palette.colors[palette_index].b;

            //printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", j, i, r, g, b);
        }
    }

    return 0;
}

int fill_rgb_matrix_palette_1_2_4(struct png_info* image_info, struct image_data* image, char* decompressed_data, uLongf decompressed_data_length)
{
    int decompressed_data_byte_index = 0;
    for (int i = 0; i < image->height; ++i)
    {
        // To account for filter type byte
        decompressed_data_byte_index++;

        int bits_read = 0;
        for (int j = 0; j < image->width; ++j)
        {
            int byte_offset = decompressed_data_byte_index + bits_read / 8;
            int bit_offset = bits_read % 8;

            unsigned char byte = decompressed_data[byte_offset];
            unsigned char palette_index;

            switch (image_info->bit_depth)
            {
            case 1:
                palette_index = (byte >> (7 - bit_offset)) & 0x01;
                bits_read += 1;
                break;
            case 2:
                palette_index = (byte >> (6 - bit_offset)) & 0x03;
                bits_read += 2;
                break;
            case 4:
                palette_index = (byte >> (4 - bit_offset)) & 0x0F;
                bits_read += 4;
                break;
            default:
                return -1;
            }

            if (palette_index >= image_info->palette.number_of_colors) return -1;

            image->pixel_rgb_matrix[i * image->width + j].r = image_info->palette.colors[palette_index].r;
            image->pixel_rgb_matrix[i * image->width + j].g = image_info->palette.colors[palette_index].g;
            image->pixel_rgb_matrix[i * image->width + j].b = image_info->palette.colors[palette_index].b;
        }

        decompressed_data_byte_index += (image->width * image_info->bit_depth + 7) / 8;
    }

    return 0;
}


unsigned char get_fifth_bit_from_byte(unsigned char byte)
{
    return (byte & 0x20) >> 5;
}