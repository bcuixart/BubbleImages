#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include "image.h"
#include "game.h"

void usage()
{
	printf("Usage:\n./main -s filename (Save images)\n./main -g [filename] (Play game)\n");
	exit(1);
}

void error_and_exit(char* errorMsg, int exitCode) 
{
	perror(errorMsg);
	exit(exitCode);
}

int main(int argc, char** argv) 
{
	if (argc == 1) usage();
	if (argc > 3) usage();
	if (strcmp(argv[1], "-s") != 0 && strcmp(argv[1], "-g") != 0) usage();

	char execution_type;
	if (strcmp(argv[1], "-s") == 0) execution_type = 'S';
	if (strcmp(argv[1], "-g") == 0) execution_type = 'G';

	if (execution_type != 'S' && execution_type != 'G') usage();
	if (execution_type == 'S' && argc == 2) usage();

	struct image_data* read_image_data = NULL;

	if (execution_type == 'S' || argc == 3) if (read_image(argv[2], &read_image_data) == -1) error_and_exit("Loading image", 7);

	if (execution_type == 'S')
	{
		struct image_data smaller_image_data;
		char buff[128];
		for (int i = 0; i < GAME_LEVELS; ++i)
		{
			int power_of_two = (1 << i);
			sprintf(buff, "output/Output%d.png", i);

			if (get_smaller_image_data(read_image_data, &smaller_image_data, power_of_two, power_of_two) == -1) error_and_exit("Making smaller image", 8);
			//if (save_image_as_ppm(&smaller_image_data, buff) == -1) error_and_exit("Saving smaller image", 9);
			if (save_image_as_png(&smaller_image_data, buff) == -1) error_and_exit("Saving smaller image", 9);
			free(smaller_image_data.pixel_rgb_matrix);

		}
	}
	else if (execution_type == 'G')
	{
		struct image_data* smaller_images_matrix = NULL;

		init_game_window();

		if (read_image_data)
		{
			smaller_images_matrix = malloc(sizeof(struct image_data) * GAME_LEVELS);
			if (!smaller_images_matrix) error_and_exit("Alloc smaller_images_matrix", 11);

			for (int i = 0; i < GAME_LEVELS; ++i)
			{
				int power_of_two = (1 << i);

				if (get_smaller_image_data(read_image_data, &smaller_images_matrix[i], power_of_two, power_of_two) == -1)
					error_and_exit("Making smaller image", 8);
			}

			if (game_load_image_matrix(smaller_images_matrix, GAME_LEVELS) == -1)
				error_and_exit("Loading game matrix", 10);
		}
		else
		{
		}

		while (!should_close_game_window()) {
			if (update_game() == -1) error_and_exit("Game", 6);
		}

		close_game_window();
	}

	if (read_image_data) {
		if (read_image_data->pixel_rgb_matrix) free(read_image_data->pixel_rgb_matrix);
		free(read_image_data);
	}
}