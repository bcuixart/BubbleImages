#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "image.h"

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

int main(int argc, char** argv) 
{
	if (argc != 2) usage();

	struct image_data read_image_data;
	if (read_image(argv[1], &read_image_data) == -1) error_and_exit("Loading image", 7);

	struct image_data smaller_image_data;
	if (get_smaller_image_data(&read_image_data, &smaller_image_data, 10, 10) == -1) error_and_exit("Making smaller image", 8);
	if (save_image_as_ppm(&smaller_image_data, "output/Output.ppm") == -1) error_and_exit("Saving smaller image", 9);

	free(smaller_image_data.pixel_rgb_matrix);
	free(read_image_data.pixel_rgb_matrix);
}