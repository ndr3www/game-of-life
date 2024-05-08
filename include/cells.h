#pragma once

#include <SDL2/SDL2_gfxPrimitives.h>

typedef struct CellStruct {
	Sint16 pos_x, pos_y;
	int alive;
	unsigned int alive_neighbours;
} Cell;

// Constructor
Cell** cells_new(size_t width, size_t height, unsigned int size);

// Destructor
void cells_delete(Cell** cells, size_t width);
