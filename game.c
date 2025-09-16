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
	int screen_width = GetScreenWidth();
	int screen_height = GetScreenHeight();

	BeginDrawing();
	ClearBackground(RAYWHITE);

	BeginMode2D(camera);

	camera.target = (Vector2){ 128, 128 };
	camera.offset = (Vector2){ (float)screen_width / 2, (float)screen_height / 2 };
	camera.rotation = 0.0f;
	camera.zoom = 0.6666f * min(screen_width, screen_height) / 256.f;

	if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
	{
		Vector2 mouse_pos = GetMousePosition();
		Vector2 mouse_world = GetScreenToWorld2D(mouse_pos, camera);

		struct game_image_node* list_iterator = nodes_list_first;
		while (list_iterator != NULL)
		{
			float dx = mouse_world.x - list_iterator->position.x;
			float dy = mouse_world.y - list_iterator->position.y;
			if (sqrtf(dx * dx + dy * dy) <= list_iterator->radius)
			{
				click_node(list_iterator);
				break;
			}

			list_iterator = list_iterator->next;
		}
	}

	struct game_image_node* list_iterator = nodes_list_first;
	while (list_iterator != NULL)
	{
		DrawCircleV(list_iterator->position, list_iterator->radius, list_iterator->color);

		list_iterator = list_iterator->next;
	}

	EndMode2D();
	EndDrawing();
}

void click_node(struct game_image_node* node)
{
	if (node->level >= game_smaller_images_level - 1) return;

	int new_level = node->level + 1;
	int new_level_color_matrix_width = (1 << new_level);
	int new_radius = node->radius / 2;

	struct game_image_node* n1 = malloc(sizeof(struct game_image_node));
	struct game_image_node* n2 = malloc(sizeof(struct game_image_node));
	struct game_image_node* n3 = malloc(sizeof(struct game_image_node));
	//if (!n1 || !n2 || !n3) return -1;

	Vector2 pos0 = (Vector2){ node->position.x + new_radius, node->position.y + new_radius };
	Vector2 pos1 = (Vector2){ node->position.x - new_radius, node->position.y + new_radius };
	Vector2 pos2 = (Vector2){ node->position.x + new_radius, node->position.y - new_radius };
	Vector2 pos3 = (Vector2){ node->position.x - new_radius, node->position.y - new_radius };

	int i0 = (int)((pos0.x / 256) * new_level_color_matrix_width);
	int j0 = (int)((pos0.y / 256) * new_level_color_matrix_width);
	int i1 = (int)((pos1.x / 256) * new_level_color_matrix_width);
	int j1 = (int)((pos1.y / 256) * new_level_color_matrix_width);
	int i2 = (int)((pos2.x / 256) * new_level_color_matrix_width);
	int j2 = (int)((pos2.y / 256) * new_level_color_matrix_width);
	int i3 = (int)((pos3.x / 256) * new_level_color_matrix_width);
	int j3 = (int)((pos3.y / 256) * new_level_color_matrix_width);

	int idx0 = j0 * new_level_color_matrix_width + i0;
	int idx1 = j1 * new_level_color_matrix_width + i1;
	int idx2 = j2 * new_level_color_matrix_width + i2;
	int idx3 = j3 * new_level_color_matrix_width + i3;

	Color color0 = (Color){ game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx0].r,
		game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx0].g,
		game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx0].b, 255
	};
	Color color1 = (Color) { game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx1].r,
		game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx1].g,
		game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx1].b, 255
	};
	Color color2 = (Color) { game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx2].r,
		game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx2].g,
		game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx2].b, 255
	};
	Color color3 = (Color) { game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx3].r,
		game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx3].g,
		game_smaller_images_matrix[new_level].pixel_rgb_matrix[idx3].b, 255
	};

	set_node_values(n1, color0, pos0, new_level, new_radius);
	set_node_values(n2, color1, pos1, new_level, new_radius);
	set_node_values(n3, color2, pos2, new_level, new_radius);
	set_node_values(node, color3, pos3, new_level, new_radius);

	n3->next = node->next;
	node->next = n1;
	n1->next = n2;
	n2->next = n3;
}

void set_node_values(struct game_image_node* node, Color color, Vector2 position, int level, int radius)
{
	node->position = position;
	node->level = level;
	node->radius = radius;
	node->color = color;
}

/*
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
*/

void load_image_matrix(struct image_data* img_mtrx, int level)
{
	game_smaller_images_matrix = img_mtrx;
	game_smaller_images_level = level;

	game_current_level = 0;

	nodes_list_first = malloc(sizeof(struct game_image_node));
	//if (!nodes_list_first) { perror("malloc"); exit(1); }

	nodes_list_first->level = 0;
	nodes_list_first->radius = 128;
	nodes_list_first->color = (Color){ img_mtrx[0].pixel_rgb_matrix[0].r,img_mtrx[0].pixel_rgb_matrix[0].g, img_mtrx[0].pixel_rgb_matrix[0].b, 255 };
	nodes_list_first->position = (Vector2){ 128, 128 };
	nodes_list_first->next = NULL;
}