#include "../include/cells.h"
#include "../include/utils.h"

CellsGrid* CellsGrid_create(size_t width, size_t height, unsigned int cell_size) {
	CellsGrid* cells_grid = malloc(sizeof(CellsGrid));
	if (cells_grid == NULL) {
		fprintf(stderr, "Failed to allocate memory for cells grid\n");
		return NULL;
	}

	cells_grid->width = width;
	cells_grid->height = height;
	cells_grid->cell_size = cell_size;

	// Create cells in rows
	Cell** cell = malloc(sizeof(Cell) * width);
	if (cell == NULL) {
		fprintf(stderr, "Failed to allocate memory for cells in rows\n");
		return NULL;
	}

	// Create cells in columns
	for (size_t i = 0; i < width; ++i) {
		cell[i] = malloc(sizeof(Cell) * height);
		if (cell[i] == NULL) {
			fprintf(stderr, "Failed to allocate memory for cells in columns\n");
			return NULL;
		}
	}

	// Initialize cells
	for (size_t x = 0; x < width; ++x) {
		for (size_t y = 0; y < height; ++y) {
			cell[x][y].pos_x = x * cell_size;
			cell[x][y].pos_y = y * cell_size;
			cell[x][y].is_alive = rand() % 2;
			cell[x][y].alive_neighbours = 0;
		}
	}

	cells_grid->cell = cell;

	return cells_grid;
}

void CellsGrid_delete(CellsGrid* cells_grid) {
	for (size_t i = 0; i < cells_grid->width; ++i) {
		free(cells_grid->cell[i]);
	}
	free(cells_grid->cell);

	free(cells_grid);
}

void CellsGrid_draw(SDL_Renderer* renderer, CellsGrid* cells_grid) {
	for (size_t x = 0; x < cells_grid->width; ++x) {
		for (size_t y = 0; y < cells_grid->height; ++y) {
			int return_code = boxColor(renderer,
									   cells_grid->cell[x][y].pos_x, cells_grid->cell[x][y].pos_y,
									   cells_grid->cell[x][y].pos_x + cells_grid->cell_size, cells_grid->cell[x][y].pos_y + cells_grid->cell_size,
									   cells_grid->cell[x][y].is_alive ? WHITE_HEX : BLACK_HEX);
			if (return_code != 0) {
				fprintf(stderr, "Failed to render cell[%lu][%lu]\n", x, y);
			}
		}
	}
}
