#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "image.h"

void usage()
{
	printf("Usage: ./main -s/-g filename\n-s: Save image\n-g: Play game\n");
	exit(1);
}

void error_and_exit(char* errorMsg, int exitCode) 
{
	perror(errorMsg);
	exit(exitCode);
}

int main(int argc, char** argv) 
{
	if (argc != 3) usage();
	if (strcmp(argv[1], "-s") != 0 && strcmp(argv[1], "-g") != 0) usage();

	struct image_data read_image_data;
	if (read_image(argv[2], &read_image_data) == -1) error_and_exit("Loading image", 7);

	if (strcmp(argv[1], "-s") == 0)
	{
		struct image_data smaller_image_data;
		char buff[128];
		for (int i = 0; i < 10; ++i)
		{
			int power_of_two = (1 << i);
			sprintf(buff, "output/Output%d.png", i);

			if (get_smaller_image_data(&read_image_data, &smaller_image_data, power_of_two, power_of_two) == -1) error_and_exit("Making smaller image", 8);
			//if (save_image_as_ppm(&smaller_image_data, buff) == -1) error_and_exit("Saving smaller image", 9);
			if (save_image_as_png(&smaller_image_data, buff) == -1) error_and_exit("Saving smaller image", 9);
			free(smaller_image_data.pixel_rgb_matrix);
		}
	}
	else if (strcmp(argv[1], "-g") == 0)
	{
		struct image_data smaller_images_matrix[10];
		for (int i = 0; i < 10; ++i)
		{
			int power_of_two = (1 << i);

			if (get_smaller_image_data(&read_image_data, &smaller_images_matrix[i], power_of_two, power_of_two) == -1) error_and_exit("Making smaller image", 8);
		}

		printf("The game will eventually load here...\nPlease imagine it did.\n");

		for (int i = 0; i < 10; ++i) free(smaller_images_matrix[i].pixel_rgb_matrix);
	}

	if (read_image_data.pixel_rgb_matrix) free(read_image_data.pixel_rgb_matrix);
}