#pragma once

#include "SDL2_gfxPrimitives.h"

typedef struct CellStruct {
	Sint16 pos_x, pos_y;
	int is_alive;
	unsigned int alive_neighbours;
	Uint8 r, g, b;
} Cell;

typedef struct CellsGridStruct {
	size_t width, height;
	unsigned int cell_size;
	Cell** cell;
} CellsGrid;

// Constructor
CellsGrid* CellsGrid_create(size_t width, size_t height, unsigned int cell_size);

// Destructor
void CellsGrid_delete(CellsGrid* cells_grid);

// Drawing
void CellsGrid_draw(CellsGrid* cells_grid, SDL_Renderer* renderer, SDL_Rect* viewport, SDL_Texture* mesh_texture, int draw_mesh);
