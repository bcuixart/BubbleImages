#include "game.h"

struct image_data* game_smaller_images_matrix;
int game_smaller_images_level = 0;
int game_current_level = 0;

void init_game_window()
{
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dorga");

	SetTargetFPS(60);

	game_smaller_images_matrix = NULL;
	game_smaller_images_level = 0;

	game_current_level = 0;
}

void close_game_window()
{
	CloseWindow();
}

int should_close_game_window()
{
	return WindowShouldClose();
}

void update_game()
{
	if (!game_smaller_images_matrix) return;

	int screen_width = GetScreenWidth();
	int screen_height = GetScreenHeight();

	if (IsKeyPressed(KEY_P)) game_current_level = (game_current_level + 1) % game_smaller_images_level;

	int image_width = game_smaller_images_matrix[game_current_level].width;
	int image_height = game_smaller_images_matrix[game_current_level].height;

	BeginDrawing();
	ClearBackground(RAYWHITE);

	float image_total_radius = min(screen_width / 3, screen_height / 3);
	float individual_pixel_radius = image_total_radius / image_height;

	Vector2 center_position;
	center_position.x = screen_width / 2;
	center_position.y = screen_height / 2;

	Vector2 min_position;
	min_position.x = center_position.x - image_total_radius;
	min_position.y = center_position.y - image_total_radius;

	float position_pixel_increment = individual_pixel_radius * 2;

	Color color;
	color.a = 255;
	for (int i = 0; i < image_height; ++i)
	{
		for (int j = 0; j < image_width; ++j)
		{
			color.r = game_smaller_images_matrix[game_current_level].pixel_rgb_matrix[i * image_width + j].r;
			color.g = game_smaller_images_matrix[game_current_level].pixel_rgb_matrix[i * image_width + j].g;
			color.b = game_smaller_images_matrix[game_current_level].pixel_rgb_matrix[i * image_width + j].b;

			Vector2 position = min_position;
			position.x += position_pixel_increment * j + individual_pixel_radius;
			position.y += position_pixel_increment * i + individual_pixel_radius;

			DrawCircleV(position, individual_pixel_radius, color);
		}
	}

	//DrawRectangleLines(min_position.x, min_position.y, image_total_radius * 2, image_total_radius * 2, RED);

	EndDrawing();
}

void load_image_matrix(struct image_data* img_mtrx, int level)
{
	game_smaller_images_matrix = img_mtrx;
	game_smaller_images_level = level;

	game_current_level = 0;
}