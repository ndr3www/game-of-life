#include "../include/cells.h"

Cell** cells_new(size_t width, size_t height, unsigned int size) {
	// Create cells in rows
	Cell** cells = malloc(sizeof(Cell) * width);
	if (cells == NULL) {
		fprintf(stderr, "Failed to allocate memory for cells in rows\n");
		return NULL;
	}

	// Create cells in columns
	for (size_t i = 0; i < width; ++i) {
		cells[i] = malloc(sizeof(Cell) * height);
		if (cells[i] == NULL) {
			fprintf(stderr, "Failed to allocate memory for cells in columns\n");
			return NULL;
		}
	}

	// Initialize cells
	for (size_t x = 0; x < width; ++x) {
		for (size_t y = 0; y < height; ++y) {
			cells[x][y].pos_x = x * size;
			cells[x][y].pos_y = y * size;
			cells[x][y].is_alive = rand() % 2;
			cells[x][y].alive_neighbours = 0;
		}
	}

	return cells;
}

void cells_delete(Cell **cells, size_t width) {
	for (size_t i = 0; i < width; ++i) {
		free(cells[i]);
	}
	free(cells);
}
