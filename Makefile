# Variables
CXX = gcc
CXXFLAGS = -Wall
LDFLAGS = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lz

SRC = main.c image.c png_parser.c ppm_parser.c game.c tinyfiledialogs.c
HDR = image.h png_parser.h ppm_parser.h game.h tinyfiledialogs.h
OBJ = $(SRC:.c=.o)

TARGET = main

# Regla principal
all: $(TARGET)

# Linkar
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compilar cada .cc a .o, depenent també dels .hh
%.o: %.cc $(HDR)
	$(CXX) $(CXXFLAGS) -c $<

# Netejar
clean:
	rm -f $(OBJ) $(TARGET)
