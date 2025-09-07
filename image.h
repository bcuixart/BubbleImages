#ifndef IMAGE_HH
#define IMAGE_HH

struct pixel_rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

struct image_data {
	unsigned int width;
	unsigned int height;
	struct pixel_rgb* pixel_rgb_matrix;
};

#endif