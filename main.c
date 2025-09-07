#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "image.h"
#include "ppm_parser.h"

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

void make_smaller_image(char* filename, struct image_data* ori_data, int new_width, int new_height)
{
	struct image_data smaller_image_data = get_smaller_image_data(ori_data, new_width, new_height);

	printf("Writing...\n");
	if (save_ppm(filename, &smaller_image_data) == -1) error_and_exit("Write output", 7);
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
		if (read_image_data.width == -1) error_and_exit("Read data", 6);

		make_smaller_image("output/Output.ppm", &read_image_data, 1, 1);
		make_smaller_image("output/Output1.ppm", &read_image_data, 2, 2);
		make_smaller_image("output/Output2.ppm", &read_image_data, 4, 4);
		make_smaller_image("output/Output3.ppm", &read_image_data, 16, 16);
		make_smaller_image("output/Output4.ppm", &read_image_data, 64, 64);

		free(read_image_data.pixel_rgb_matrix);
	}

	if (fclose(file) == -1) error_and_exit("FClose", 3);
}