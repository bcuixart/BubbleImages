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

#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

#define GAME_LEVELS 8

void init_game_window();
void close_game_window();
int should_close_game_window();

void update_game();

void load_image_matrix(struct image_data* img_mtrx, int level);

extern struct image_data* game_smaller_images_matrix;
extern int game_smaller_images_level;
extern int game_current_level;

#endif