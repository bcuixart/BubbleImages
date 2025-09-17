#include "image.h"

int read_image(char* filename, struct image_data** result)
{
	*result = malloc(sizeof(struct image_data));

	FILE* file = fopen(filename, "r");
	if (file == NULL) return -1;

	enum ImageType image_type = get_image_type_from_file(file);

	if (image_type == UNKNOWN) printf("Could not identify file type. You idiot.\nCurrently only PPM is supported.\n");
	else if (image_type == PNG) {
		printf("Type: PNG\n");
		if (parse_png(file, *result) == -1) return -1;
	}
	else {
		printf("Type: P%d\n", (image_type == PPM_P3) ? 3 : 6);
		if (parse_ppm(file, *result, (image_type == PPM_P3) ? 3 : 6) == -1) return -1;
	}

	if (fclose(file) == -1) return -1;

	return 0;
}

int save_image_as_png(struct image_data* data, char* filename)
{
	return save_png(data, filename);
}

int save_image_as_ppm(struct image_data* data, char* filename)
{
	return save_ppm(data, filename);
}

enum ImageType get_image_type_from_file(FILE* file)
{
	unsigned char c = getc(file);

	if (c == 'P')
	{
		char c2 = getc(file);
		if (c2 == '3') return PPM_P3;
		if (c2 == '6') return PPM_P6;
	}
	else if (c == 137)
	{
		char buff[8];
		fread(buff, 1, 7, file);

		char expected_png_header[8] = { 80, 78, 71, 13, 10, 26, 10, '\0' };
		if (strcmp(buff, expected_png_header) == 0) return PNG;
	}

	return UNKNOWN;
}

int get_smaller_image_data(struct image_data* ori_data, struct image_data* new_data, int new_width, int new_height)
{
	new_data->width = new_width;
	new_data->height = new_height;

	new_data->pixel_rgb_matrix = malloc(new_width * new_height * sizeof(struct pixel_rgb));

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

			new_data->pixel_rgb_matrix[i * new_width + j] = get_average_color_from_image_pixels(
				ori_data,
				from_x,
				to_x,
				from_y,
				to_y);
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
	result.r = result.g = result.b = 0;

	if (total_pixels == 0) return result;

	result.r = total_r / total_pixels;
	result.g = total_g / total_pixels;
	result.b = total_b / total_pixels;

	return result;
}