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

struct pixel_rgb* read_image_rbg_matrix;

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
	while (c != '\n') {
		c = getc(file);
	}
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

int parse_ppm_p3_pixel_data(FILE* file, int width, int height, int maxcolorval, char bytesperpixel)
{
	for (int i = 0; i < height; ++i)
	{
		for (int j = 0; j < width; ++j)
		{
			unsigned char r = get_rgb_ascii_number_from_ppm(file, maxcolorval);
			unsigned char g = get_rgb_ascii_number_from_ppm(file, maxcolorval);
			unsigned char b = get_rgb_ascii_number_from_ppm(file, maxcolorval);

			read_image_rbg_matrix[i * width + j].r = r;
			read_image_rbg_matrix[i * width + j].g = g;
			read_image_rbg_matrix[i * width + j].b = b;

			//printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", i, j, r, g, b);
		}
	}

	return 0;
}

int parse_ppm_p6_pixel_data(FILE* file, int width, int height, int maxcolorval, char bytesperpixel)
{
	for (int i = 0; i < height; ++i) 
	{
		for (int j = 0; j < width; ++j) 
		{
			unsigned char r = get_rgb_binary_number_from_ppm(file, maxcolorval, bytesperpixel);
			unsigned char g = get_rgb_binary_number_from_ppm(file, maxcolorval, bytesperpixel);
			unsigned char b = get_rgb_binary_number_from_ppm(file, maxcolorval, bytesperpixel);

			read_image_rbg_matrix[i * width + j].r = r;
			read_image_rbg_matrix[i * width + j].g = g;
			read_image_rbg_matrix[i * width + j].b = b;

			//printf("Pixel (%d, %d): RGB = (%d, %d, %d)\n", i, j, r, g, b);
		}
	}

	return 0;
}

int save_ppm(char* filename, struct pixel_rgb* data, int width, int height)
{
	// El fitxer no canvia si ja existeix... Pero no dona error
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) error_and_exit("Open output", 7);

	char buff[128];
	sprintf(buff, "P6\n%d %d\n255\n", width, height);
	write(fd, buff, strlen(buff));

	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			unsigned char r = data[i * width + j].r;
			unsigned char g = data[i * width + j].g;
			unsigned char b = data[i * width + j].b;

			unsigned char rgb[3] = { r, g, b };
			write(fd, rgb, 3);
		}
	}

	close(fd);

	return 0;
}

int parse_ppm(FILE* file, char ppm_header_type)
{
	int width = get_ascii_number_from_ppm(file);
	int height = get_ascii_number_from_ppm(file);
	printf("Width: %d\nHeight: %d\n", width, height);

	int maxcolorval = get_ascii_number_from_ppm(file);
	if (maxcolorval < 0 || maxcolorval >= 65536)
	{
		printf("Maximum color value: %d (Invalid! Must be between 0 and 65535)\n", maxcolorval);
		return -1;
	}
	printf("Maximum color value: %d\n", maxcolorval);

	char bytesperpixel = (maxcolorval < 256) ? 1 : 2;
	if (ppm_header_type == 6) printf("Bytes per pixel: %d\n", bytesperpixel);

	read_image_rbg_matrix = malloc(width * height * sizeof(struct pixel_rgb));

	if (ppm_header_type == 6) parse_ppm_p6_pixel_data(file, width, height, maxcolorval, bytesperpixel);
	else if (ppm_header_type == 3) parse_ppm_p3_pixel_data(file, width, height, maxcolorval, bytesperpixel);

	printf("Writing...\n");
	save_ppm("Output.ppm", read_image_rbg_matrix, width, height);
	printf("Done!\n");

	free(read_image_rbg_matrix);

	return 0;
}

int main(int argc, char** argv) 
{
	if (argc != 2) usage();

	int fd = open(argv[1], O_RDONLY);
	if (fd == -1) error_and_exit("Open", 2);

	FILE* file = fdopen(fd, "r");
	if (file == NULL) {
		close(fd);
		error_and_exit("FDOpen", 5);
	}

	char ppm_header_type = get_ppm_header_type(file);
	if (ppm_header_type != 3 && ppm_header_type != 6) printf("Could not identify type. Idiot.\nAre you sure this is a PPM file?\n");
	else {
		printf("Type: P%d\n", ppm_header_type);
		parse_ppm(file, ppm_header_type);
	}

	if (close(fd) == -1) error_and_exit("Close", 3);
}