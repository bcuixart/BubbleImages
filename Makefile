all:
	gcc -o main main.c image.h image.c png_parser.c png_parser.h ppm_parser.c ppm_parser.h -lz