# Compilador i flags
CC = gcc
CFLAGS = -Wall
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lz

# Fitxers font i headers
SRC = main.c image.c png_parser.c ppm_parser.c game.c \
      lib/nfd/nfd_common.c lib/nfd/nfd_zenity.c
HDR = image.h png_parser.h ppm_parser.h game.h \
      lib/nfd/nfd.h lib/nfd/common.h lib/nfd/nfd_common.h lib/nfd/simple_exec.h
OBJ = $(SRC:.c=.o)

TARGET = main

# Regla principal
all: $(TARGET)

# Enllaçar
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compilar .c a .o
%.o: %.c $(HDR)
	$(CC) $(CFLAGS) -c $< -o $@

# Neteja
clean:
	rm -f $(OBJ) $(TARGET)
