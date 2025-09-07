#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

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

void usage()
{
	printf("You imbecile! You must write 1 file name!\n");
	exit(1);
}

void error_and_exit(char* errorMsg, int exitCode) 
{
	perror(errorMsg);
	exit(exitCode);
}

void ignore_ppm_comment_line(FILE* file)
{
	char c;
	do {
		c = getc(file);
	} while (c != '\n');
}

char get_ppm_header_type(FILE* file) 
{
	char c = getc(file);
	while (c == '#') 
	{
		ignore_ppm_comment_line(file);
		c = getc(file);
	}

	char c2;
	c2 = getc(file);

	if (c == 'P' && c2 == '3') return 3;
	if (c == 'P' && c2 == '6') return 6;

	return 0;
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

struct pixel_rgb get_average_color_from_image_pixels(struct image_data* image, int from_x, int to_x, int from_y, int to_y)
{
	from_x = (from_x < 0) ? 0 : from_x;
	from_y = (from_y < 0) ? 0 : from_y;
	to_x = (to_x > image->width) ? image->width : to_x;
	to_y = (to_y > image->height) ? image->height : to_y;

	int total_r = 0;
	int total_g = 0;
	int total_b = 0;

	int total_pixels = 0;

	//printf("From: (%d, %d), To: (%d, %d)\n", from_x, from_y, to_x, to_y);

	for (int i = from_x; i < to_x; ++i)
	{
		for (int j = from_y; j < to_y; ++j)
		{
			total_r += image->pixel_rgb_matrix[(j * image->width) + i].r;
			total_g += image->pixel_rgb_matrix[(j * image->width) + i].g;
			total_b += image->pixel_rgb_matrix[(j * image->width) + i].b;

			++total_pixels;
		}
	}

	struct pixel_rgb result;

	if (total_pixels == 0) {
		result.r = result.g = result.b = 0;
		return result;
	}

	result.r = total_r / total_pixels;
	result.g = total_g / total_pixels;
	result.b = total_b / total_pixels;

	return result;
}

struct image_data get_smaller_image_data(struct image_data* ori_data, int new_width, int new_height)
{
	struct image_data result;
	result.width = new_width;
	result.height = new_height;

	result.pixel_rgb_matrix = malloc(new_width * new_height * sizeof(struct pixel_rgb));

	for (int i = 0; i < new_height; ++i) 
	{
		for (int j = 0; j < new_width; ++j) 
		{
			int from_x = (float)(j) / (float)(new_width)*ori_data->width;
			int to_x = (float)(j + 1) / (float)(new_width)*ori_data->width;
			int from_y = (float)(i) / (float)(new_height)*ori_data->height;
			int to_y = (float)(i + 1) / (float)(new_height)*ori_data->height;

			if (to_x <= from_x) to_x = from_x + 1;
			if (to_y <= from_y) to_y = from_y + 1;

			result.pixel_rgb_matrix[i * new_width + j] = get_average_color_from_image_pixels(
				ori_data,
				from_x,
				to_x,
				from_y,
				to_y);
		}
	}

	return result;
}

int save_ppm(char* filename, struct image_data* image)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) error_and_exit("Open output", 7);

	char buff[128];
	sprintf(buff, "P6\n%d %d\n255\n", image->width, image->height);
	write(fd, buff, strlen(buff));

	for (int i = 0; i < image->height; ++i) {
		for (int j = 0; j < image->width; ++j) {
			unsigned char r = image->pixel_rgb_matrix[i * image->width + j].r;
			unsigned char g = image->pixel_rgb_matrix[i * image->width + j].g;
			unsigned char b = image->pixel_rgb_matrix[i * image->width + j].b;

			unsigned char rgb[3] = { r, g, b };
			write(fd, rgb, 3);
		}
	}

	close(fd);

	return 0;
}

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

	if (ppm_header_type == 6) parse_ppm_p6_pixel_data(file, &result, maxcolorval, bytesperpixel);
	else if (ppm_header_type == 3) parse_ppm_p3_pixel_data(file, &result, maxcolorval, bytesperpixel);

	return result;
}

void make_smaller_image(char* filename, struct image_data* ori_data, int new_width, int new_height)
{
	struct image_data smaller_image_data = get_smaller_image_data(ori_data, new_width, new_height);

	printf("Writing...\n");
	save_ppm(filename, &smaller_image_data);
	printf("Done!\n");

	free(smaller_image_data.pixel_rgb_matrix);
}

int main(int argc, char** argv) 
{
	if (argc != 2) usage();

	FILE* file = fopen(argv[1], "r");
	if (file == NULL) error_and_exit("FDOpen", 5);

	char ppm_header_type = get_ppm_header_type(file);
	if (ppm_header_type != 3 && ppm_header_type != 6) printf("Could not identify type. Idiot.\nAre you sure this is a PPM file?\n");
	else {
		printf("Type: P%d\n", ppm_header_type);
		struct image_data read_image_data = parse_ppm(file, ppm_header_type);

		make_smaller_image("Output.ppm", &read_image_data, 10, 10);

		free(read_image_data.pixel_rgb_matrix);
	}

	if (fclose(file) == -1) error_and_exit("FClose", 3);
}