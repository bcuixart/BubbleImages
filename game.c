#include "game.h"

// Private variables
enum game_state state;

struct image_data* game_smaller_images_matrix;
struct game_image_node* nodes_list_first;

int game_smaller_images_level = 0;
int game_current_level = 0;

int total_nodes_to_click;
int clicked_nodes;

Camera2D camera = (Camera2D){ 0 };

void init_game_window()
{
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dorga");

	SetTargetFPS(60);

	state = No_Image;

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

int update_game()
{
	int screen_width = GetScreenWidth();
	int screen_height = GetScreenHeight();

	float delta_time = GetFrameTime();

	BeginDrawing();
	ClearBackground(RAYWHITE);

	BeginMode2D(camera);

	camera.target = (Vector2){ 128, 128 };
	camera.offset = (Vector2){ (float)screen_width / 2, (float)screen_height / 2 };
	camera.rotation = 0.0f;
	camera.zoom = 0.6666f * min(screen_width, screen_height) / 256.f;

	switch (state) {
	case No_Image:
		if (update_game_no_image(screen_width, screen_height, delta_time) == -1) return -1;
		break;
	case Loaded_Image:
		if (update_game_loaded_image(screen_width, screen_height, delta_time) == -1) return -1;
		break;
	}

	EndMode2D();
	EndDrawing();

	return 0;
}

int update_game_no_image(int screen_width, int screen_height, float delta_time)
{
	Vector2 load_button_position = (Vector2){ 0, 128 };
	Vector2 load_button_size = (Vector2){ 256, 64 };

	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	{
		Vector2 mouse_pos = GetMousePosition();
		Vector2 mouse_world = GetScreenToWorld2D(mouse_pos, camera);

		if (mouse_world.x >= load_button_position.x && mouse_world.x <= load_button_position.x + load_button_size.x &&
			mouse_world.y >= load_button_position.y && mouse_world.y <= load_button_position.y + load_button_size.y)
		{

		}
	}

	DrawRectangleV(load_button_position, load_button_size, RED);

	return 0;
}

int update_game_loaded_image(int screen_width, int screen_height, float delta_time)
{
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
				if (click_node(list_iterator) == -1) return -1;
				break;
			}

			list_iterator = list_iterator->next;
		}
	}

	struct game_image_node* list_iterator = nodes_list_first;
	while (list_iterator != NULL)
	{
		animate_node(list_iterator, delta_time);
		DrawCircleV(list_iterator->position, list_iterator->radius, list_iterator->color);

		list_iterator = list_iterator->next;
	}

	Vector2 bar_position = GetScreenToWorld2D((Vector2) { 0, 10 }, camera);
	DrawRectangle(0, bar_position.y, 256, 10, RED);
	int bar_length = (int)(((float)clicked_nodes / (float)total_nodes_to_click) * 256.f);
	DrawRectangle(0, bar_position.y, bar_length, 10, GREEN);

	return 0;
}

void animate_node(struct game_image_node* node, float delta_time)
{
	if (node->can_be_pressed) return;

	node->animation_lerp_value = min(node->animation_lerp_value + delta_time * ANIMATION_ACCELERATOR, 1.f);
	node->radius = Lerp(node->animation_start_radius, node->animation_end_radius, node->animation_lerp_value);
	node->position.x = Lerp(node->animation_start_position.x, node->animation_end_position.x, node->animation_lerp_value);
	node->position.y = Lerp(node->animation_start_position.y, node->animation_end_position.y, node->animation_lerp_value);
	node->color.r = Lerp(node->animation_start_color.r, node->animation_end_color.r, node->animation_lerp_value);
	node->color.g = Lerp(node->animation_start_color.g, node->animation_end_color.g, node->animation_lerp_value);
	node->color.b = Lerp(node->animation_start_color.b, node->animation_end_color.b, node->animation_lerp_value);

	if (node->animation_lerp_value >= 1) node->can_be_pressed = 1;
}

int click_node(struct game_image_node* node)
{
	if (!node->can_be_pressed) return 0;
	if (node->level >= game_smaller_images_level - 1) return 0;

	int new_level = node->level + 1;
	int new_level_color_matrix_width = (1 << new_level);
	int new_radius = node->radius / 2;
	int radius_start = node->radius;

	struct game_image_node* n1 = malloc(sizeof(struct game_image_node));
	struct game_image_node* n2 = malloc(sizeof(struct game_image_node));
	struct game_image_node* n3 = malloc(sizeof(struct game_image_node));
	if (!n1 || !n2 || !n3) return -1;

	Vector2 pos_start = node->position;
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

	Color color_start = node->color;
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

	set_node_values(n1, color_start, color0, pos_start, pos0, new_level, radius_start, new_radius);
	set_node_values(n2, color_start, color1, pos_start, pos1, new_level, radius_start, new_radius);
	set_node_values(n3, color_start, color2, pos_start, pos2, new_level, radius_start, new_radius);
	set_node_values(node, color_start, color3, pos_start, pos3, new_level, radius_start, new_radius);

	n3->next = node->next;
	node->next = n1;
	n1->next = n2;
	n2->next = n3;

	++clicked_nodes;
	//printf("%d/%d\n", clicked_nodes, total_nodes_to_click);

	return 0;
}

void set_node_values(struct game_image_node* node, Color color_start, Color color_end, Vector2 position_start, Vector2 position_end, int level, float radius_start, float radius_end)
{
	node->position = position_start;
	node->level = level;
	node->radius = radius_start;
	node->color = color_start;

	node->can_be_pressed = 0;
	node->animation_lerp_value = 0;
	node->animation_start_radius = radius_start;
	node->animation_start_position = position_start;
	node->animation_start_color = color_start;
	node->animation_end_radius = radius_end;
	node->animation_end_position = position_end;
	node->animation_end_color = color_end;
}

int game_load_image_matrix(struct image_data* img_mtrx, int level)
{
	game_smaller_images_matrix = img_mtrx;
	game_smaller_images_level = level;

	game_current_level = 0;

	nodes_list_first = malloc(sizeof(struct game_image_node));
	if (!nodes_list_first) return -1;

	nodes_list_first->level = 0;
	nodes_list_first->radius = 128;
	nodes_list_first->color = (Color){ img_mtrx[0].pixel_rgb_matrix[0].r, img_mtrx[0].pixel_rgb_matrix[0].g, img_mtrx[0].pixel_rgb_matrix[0].b, 255 };
	nodes_list_first->position = (Vector2){ 128, 128 };
	nodes_list_first->next = NULL;

	nodes_list_first->can_be_pressed = 0;
	nodes_list_first->animation_lerp_value = 0;
	nodes_list_first->animation_start_radius = 0;
	nodes_list_first->animation_start_position = (Vector2){ 128, 128 };
	nodes_list_first->animation_start_color = (Color){ 255, 255, 255, 255 };
	nodes_list_first->animation_end_radius = 128;
	nodes_list_first->animation_end_position = (Vector2){ 128, 128 };
	nodes_list_first->animation_end_color = (Color){ img_mtrx[0].pixel_rgb_matrix[0].r, img_mtrx[0].pixel_rgb_matrix[0].g, img_mtrx[0].pixel_rgb_matrix[0].b, 255 };

	clicked_nodes = 0;
	total_nodes_to_click = 0;
	total_nodes_to_click = (pow(4, level - 1) - 1) / 3;

	return 0;
}