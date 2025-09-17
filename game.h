#ifndef GAME_HH
#define GAME_HH

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <raylib.h>
#include <raymath.h>

#include "image.h"
#include "lib/nfd/nfd.h"

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

#define GAME_LEVELS 7
#define ANIMATION_ACCELERATOR 5

struct game_image_node 
{
	Color color;
	Vector2 position;
	int level;
	float radius;

	struct game_image_node* next;

	float animation_lerp_value;
	Vector2 animation_start_position;
	Color animation_start_color;
	float animation_start_radius;
	Vector2 animation_end_position;
	Color animation_end_color;
	float animation_end_radius;

	unsigned char can_be_pressed;
};

enum game_state {
	No_Image,
	Loaded_Image
};

void init_game_window();
void close_game_window();
int should_close_game_window();

int update_game();
int update_game_no_image(int screen_width, int screen_height, float delta_time);
int update_game_loaded_image(int screen_width, int screen_height, float delta_time);

void animate_node(struct game_image_node* node, float delta_time);

int click_node(struct game_image_node* node);
void set_node_values(struct game_image_node* node, Color color_start, Color color_end, Vector2 position_start, Vector2 position_end, int level, float radius_start, float radius_end);

int game_load_image(char* filename);
int game_load_image_matrix(struct image_data* img_mtrx, int level);

#endif