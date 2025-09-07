#include "ppm_parser.h"

struct image_data parse_ppm(FILE* file, char ppm_header_type)
{
	struct image_data result;

	result.width = get_ascii_number_from_ppm(file);
	result.height = get_ascii_number_from_ppm(file);
	printf("Width: %d\nHeight: %d\n", result.width, result.height);

	int maxcolorval = get_ascii_number_from_ppm(file);
	if (maxcolorval < 0 || maxcolorval >= 65536)
	{
		printf("Maximum color value: %d (Invalid! Must be between 0 and 65535)\n", maxcolorval);
		return result;
	}
	printf("Maximum color value: %d\n", maxcolorval);

	char bytesperpixel = (maxcolorval < 256) ? 1 : 2;
	if (ppm_header_type == 6) printf("Bytes per pixel: %d\n", bytesperpixel);

	result.pixel_rgb_matrix = malloc(result.width * result.height * sizeof(struct pixel_rgb));

	int was_error = 0;
	if (ppm_header_type == 6) was_error = parse_ppm_p6_pixel_data(file, &result, maxcolorval, bytesperpixel);
	else if (ppm_header_type == 3) was_error = parse_ppm_p3_pixel_data(file, &result, maxcolorval, bytesperpixel);

	if (was_error) result.width = -1;

	return result;
}

int save_ppm(char* filename, struct image_data* image)
{
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1) return -1;

	char buff[128];
	sprintf(buff, "P6\n%d %d\n255\n", image->width, image->height);
	if (write(fd, buff, strlen(buff)) == -1) return -1;

	for (int i = 0; i < image->height; ++i) {
		for (int j = 0; j < image->width; ++j) {
			unsigned char r = image->pixel_rgb_matrix[i * image->width + j].r;
			unsigned char g = image->pixel_rgb_matrix[i * image->width + j].g;
			unsigned char b = image->pixel_rgb_matrix[i * image->width + j].b;

			unsigned char rgb[3] = { r, g, b };
			if (write(fd, rgb, 3) == -1) return -1;
		}
	}

	if (close(fd) == -1) return -1;

	return 0;
}

int parse_ppm_p3_pixel_data(FILE* file, struct image_data* image, int maxcolorval, char bytesperpixel)
{
	for (int i = 0; i < image->height; ++i)
	{
		for (int j = 0; j < image->width; ++j)
		{
			unsigned char r = get_rgb_ascii_number_from_ppm(file, maxcolorval);
			unsigned char g = get_rgb_ascii_number_from_ppm(file, maxcolorval);
			unsigned char b = get_rgb_ascii_number_from_ppm(file, maxcolorval);

			image->pixel_rgb_matrix[i * image->width + j].r = r;
			image->pixel_rgb_matrix[i * image->width + j].g = g;
			image->pixel_rgb_matrix[i * image->width + j].b = b;

			//printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", i, j, r, g, b);
		}
	}

	return 0;
}

int parse_ppm_p6_pixel_data(FILE* file, struct image_data* image, int maxcolorval, char bytesperpixel)
{
	for (int i = 0; i < image->height; ++i)
	{
		for (int j = 0; j < image->width; ++j)
		{
			unsigned char r = get_rgb_binary_number_from_ppm(file, maxcolorval, bytesperpixel);
			unsigned char g = get_rgb_binary_number_from_ppm(file, maxcolorval, bytesperpixel);
			unsigned char b = get_rgb_binary_number_from_ppm(file, maxcolorval, bytesperpixel);

			image->pixel_rgb_matrix[i * image->width + j].r = r;
			image->pixel_rgb_matrix[i * image->width + j].g = g;
			image->pixel_rgb_matrix[i * image->width + j].b = b;

			//printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", i, j, r, g, b);
		}
	}

	return 0;
}

unsigned char get_rgb_ascii_number_from_ppm(FILE* file, int maxcolorval)
{
	char c = getc(file);
	while (!isdigit(c))
	{
		c = getc(file);
	}

	float number = 0;
	while (isdigit(c)) {
		number = number * 10 + (c - '0');
		c = getc(file);
	}

	number = number / (float)maxcolorval;
	number = min(number, 1.0f);

	return (unsigned char)(number * 255.0f);
}

unsigned char get_rgb_binary_number_from_ppm(FILE* file, int maxcolorval, char bytes)
{
	float b;
	if (bytes == 1)
	{
		b = (float)getc(file) / (float)maxcolorval;
	}
	else if (bytes == 2)
	{
		unsigned char b1 = getc(file);
		unsigned char b2 = getc(file);

		int val = (b1 << 8) | b2;
		b = (float)val / (float)maxcolorval;
	}

	b = min(b, 1.0f);

	return (unsigned char)(b * 255.0f);
}

void ignore_ppm_comment_line(FILE* file)
{
	char c;
	do {
		c = getc(file);
	} while (c != '\n');
}

int get_ascii_number_from_ppm(FILE* file)
{
	char c = getc(file);
	while (!isdigit(c))
	{
		if (c == '#') ignore_ppm_comment_line(file);
		c = getc(file);
	}

	int number = 0;
	while (isdigit(c)) {
		number = number * 10 + (c - '0');
		c = getc(file);
	}

	return number;
}

float min(float a, float b)
{
	if (a < b) return a;
	return b;
}