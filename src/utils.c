#include "../include/utils.h"

int init_SDL(SDL_Window** window, SDL_Renderer** renderer, FPSmanager* fps_manager, int screen_width, int screen_height) {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	// Create window
	*window = SDL_CreateWindow("Game of life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, SDL_WINDOW_SHOWN);
	if (*window == NULL) {
		fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
		return -1;
	}

	// Initialize framerate manager
	SDL_initFramerate(fps_manager);

	// Get index of a display associated with SDL window
	int displayIndex = SDL_GetWindowDisplayIndex(*window);
	if (displayIndex < 0) {
		fprintf(stderr, "Failed to retrieve index of a display associated with SDL window: %s\n", SDL_GetError());
		return -1;
	}

	// Get current display mode
	SDL_DisplayMode displayMode;
	if (SDL_GetCurrentDisplayMode(displayIndex, &displayMode) != 0) {
		fprintf(stderr, "Failed to retrieve display information: %s\n", SDL_GetError());
		return -1;
	}

	// Set target framerate for framerate manager
	if (SDL_setFramerate(fps_manager, displayMode.refresh_rate) != 0) {
		fprintf(stderr, "Failed to set target framerate for framerate manager\n");
		return -1;
	}

	// Create renderer
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (*renderer == NULL) {
		fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
		return -1;
	}

	return 0;
}

void close_SDL(SDL_Window* window, SDL_Renderer* renderer) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}

void mouse_controls(CellsGrid* cells_grid) {
	int mouse_x, mouse_y;
	Uint32 mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);

	// Change hovered cell state (left button - alive, others - dead)
	if (mouse_button > 0) {
		for (size_t x = 0; x < cells_grid->width; ++x) {
			for (size_t y = 0; y < cells_grid->height; ++y) {
				if (mouse_x >= cells_grid->cell[x][y].pos_x && (unsigned)mouse_x <= cells_grid->cell[x][y].pos_x + cells_grid->cell_size &&
					mouse_y >= cells_grid->cell[x][y].pos_y && (unsigned)mouse_y <= cells_grid->cell[x][y].pos_y + cells_grid->cell_size) {
					cells_grid->cell[x][y].is_alive = mouse_button == 1 ? 1 : 0;
				}
			}
		}
	}
}

void clear_screen(SDL_Renderer* renderer, Uint32 color) {
	Uint8* c = (Uint8*)&color;
	if (SDL_SetRenderDrawColor(renderer, c[0], c[1], c[2], c[3]) != 0) {
		fprintf(stderr, "Failed to set the color for SDL_RenderClear: %s\n", SDL_GetError());
	}
	if (SDL_RenderClear(renderer) != 0) {
		fprintf(stderr, "Failed to clear the screen: %s\n", SDL_GetError());
	}
}
