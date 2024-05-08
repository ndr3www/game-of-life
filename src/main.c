#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL2_framerate.h>

const Sint16 CELL_SIZE = 8;
const unsigned int CELL_NUMBER_WIDTH = 128;
const unsigned int CELL_NUMBER_HEIGHT = 100;
const size_t CELL_NUMBER_TOTAL = CELL_NUMBER_WIDTH * CELL_NUMBER_HEIGHT;

const int SCREEN_WIDTH = CELL_SIZE * CELL_NUMBER_WIDTH;
const int SCREEN_HEIGHT = CELL_SIZE * CELL_NUMBER_HEIGHT;
int refresh_rate;

int init_SDL(SDL_Window** window, SDL_Renderer** renderer, FPSmanager* fps_manager) {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());
		return -1;
	}

	// Create window
	*window = SDL_CreateWindow("Game of life", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
	refresh_rate = displayMode.refresh_rate;
	if (SDL_setFramerate(fps_manager, refresh_rate) != 0) {
		fprintf(stderr, "Failed to set target framerate for framerate manager\n");
		return -1;
	}

	// Create renderer
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (*renderer == NULL) {
		fprintf(stderr, "Failed to create renderer: %s\n", SDL_GetError());
		return -1;
	}

	// Initialize RNG
	srand(time(NULL));

	return 0;
}

void close_SDL(SDL_Window** window, SDL_Renderer** renderer) {
	SDL_DestroyRenderer(*renderer);
	SDL_DestroyWindow(*window);

	SDL_Quit();
}

typedef struct CellStruct {
	Sint16 pos_x, pos_y;
	int alive;
	unsigned int alive_neighbours;
} Cell;

int main() {
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	FPSmanager fpsManager;

	if (init_SDL(&window, &renderer, &fpsManager) != 0) {
		return 1;
	}

	// Main loop flags
	int quit = 0, pause = 1;

	// Events handler
	SDL_Event e;

	// Stuff for calculating FPS at set interval
	Uint64 prevTime = 0, currentTime;
	Uint32 fps = 0;
	Uint32 avgFPS = refresh_rate;
	unsigned int frameCount = 1;

	// Create cells in rows
	Cell** cells = malloc(sizeof(Cell) * CELL_NUMBER_WIDTH);
	if (cells == NULL) {
		fprintf(stderr, "Failed to allocate memory for cells in rows\n");
		return 2;
	}

	// Create cells in columns
	for (size_t i = 0; i < CELL_NUMBER_WIDTH; ++i) {
		cells[i] = malloc(sizeof(Cell) * CELL_NUMBER_HEIGHT);
		if (cells[i] == NULL) {
			fprintf(stderr, "Failed to allocate memory for cells in columns\n");
			return 2;
		}
	}

	// Initialize cells
	for (size_t x = 0; x < CELL_NUMBER_WIDTH; ++x) {
		for (size_t y = 0; y < CELL_NUMBER_HEIGHT; ++y) {
			cells[x][y].pos_x = x * CELL_SIZE;
			cells[x][y].pos_y = y * CELL_SIZE;
			cells[x][y].alive = rand() % 2;
			cells[x][y].alive_neighbours = 0;
		}
	}

	// Main loop
	while (!quit) {
		Uint32 frameTime = SDL_framerateDelay(&fpsManager);

		while(SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
			else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE:
						quit = 1;
						break;
					case SDLK_p:
						pause = !pause;
						break;
					case SDLK_r:
						for (size_t x = 0; x < CELL_NUMBER_WIDTH; ++x) {
							for (size_t y = 0; y < CELL_NUMBER_HEIGHT; ++y) {
								cells[x][y].alive = rand() % 2;
							}
						}
						break;
				}
			}

			// Change hovered cell state to alive when left mouse button has been clicked
			int mouse_x, mouse_y;	
			if (SDL_GetMouseState(&mouse_x, &mouse_y) == 1) {
				for (size_t x = 0; x < CELL_NUMBER_WIDTH; ++x) {
					for (size_t y = 0; y < CELL_NUMBER_HEIGHT; ++y) {
						if ((mouse_x >= cells[x][y].pos_x && mouse_x <= cells[x][y].pos_x + CELL_SIZE) &&
							(mouse_y >= cells[x][y].pos_y && mouse_y <= cells[x][y].pos_y + CELL_SIZE)) {
							cells[x][y].alive = 1;
						}
					}
				}
			}
		}

		// Logic
		if (!pause) {
			// Count alive neighbours
			for (size_t x = 0; x < CELL_NUMBER_WIDTH; ++x) {
				for (size_t y = 0; y < CELL_NUMBER_HEIGHT; ++y) {
					cells[x][y].alive_neighbours = 0;

					// right
					if (x < CELL_NUMBER_WIDTH - 1) {
						cells[x][y].alive_neighbours += cells[x + 1][y].alive ? 1 : 0;
					}
					// left
					if (x > 0) {
						cells[x][y].alive_neighbours += cells[x - 1][y].alive ? 1 : 0;
					}
					// top
					if (y > 0) {
						cells[x][y].alive_neighbours += cells[x][y - 1].alive ? 1 : 0;
					}
					// bottom
					if (y < CELL_NUMBER_HEIGHT - 1) {
						cells[x][y].alive_neighbours += cells[x][y + 1].alive ? 1 : 0;
					}
					// top right
					if (x < CELL_NUMBER_WIDTH - 1 && y > 0) {
						cells[x][y].alive_neighbours += cells[x + 1][y - 1].alive ? 1 : 0;
					}
					// top left
					if (x > 0 && y > 0) {
						cells[x][y].alive_neighbours += cells[x - 1][y - 1].alive ? 1 : 0;
					}
					// bottom right
					if (x < CELL_NUMBER_WIDTH - 1 && y < CELL_NUMBER_HEIGHT - 1) {
						cells[x][y].alive_neighbours += cells[x + 1][y + 1].alive ? 1 : 0;
					}
					// bottom left
					if (x > 0 && y < CELL_NUMBER_HEIGHT - 1) {
						cells[x][y].alive_neighbours += cells[x - 1][y + 1].alive ? 1 : 0;
					}
				}
			}

			// Check rules
			for (size_t x = 0; x < CELL_NUMBER_WIDTH; ++x) {
				for (size_t y = 0; y < CELL_NUMBER_HEIGHT; ++y) {
					if (cells[x][y].alive) {
						cells[x][y].alive = !(cells[x][y].alive_neighbours < 2 || cells[x][y].alive_neighbours > 3);
					}
					else {
						cells[x][y].alive = (cells[x][y].alive_neighbours == 3);
					}
				}
			}
		}

		// Clear the screen with specified color
		if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0) {
			fprintf(stderr, "Failed to set the color for SDL_RenderClear: %s\n", SDL_GetError());
		}
		if (SDL_RenderClear(renderer) < 0) {
			fprintf(stderr, "Failed to clear the screen: %s\n", SDL_GetError());
		}

		// Drawing 
		for (size_t x = 0; x < CELL_NUMBER_WIDTH; ++x) {
			for (size_t y = 0; y < CELL_NUMBER_HEIGHT; ++y) {
				int return_code = boxColor(renderer,
							 			   cells[x][y].pos_x, cells[x][y].pos_y,
							 			   cells[x][y].pos_x + CELL_SIZE, cells[x][y].pos_y + CELL_SIZE,
							 			   cells[x][y].alive ? 0xffffffff : 0x000000ff);
				if (return_code == -1) {
					fprintf(stderr, "Failed to render cell[%lu][%lu]\n", x, y);
				}
			}
		}

		// Calculate FPS every second
		currentTime = SDL_GetTicks64();
		if (currentTime > prevTime + 1000) {
			avgFPS = fps / frameCount;
			frameCount = 1;
			fps = 0;

			prevTime = currentTime;
		}
		else {
			fps += 1000.0f / (frameTime == (Uint32)0 ? (Uint32)1 : frameTime);
			++frameCount;
		}

		SDL_RenderPresent(renderer);
	}

	// Clean up
	for (size_t i = 0; i < CELL_NUMBER_WIDTH; ++i) {
		free(cells[i]);
	}
	free(cells);

	close_SDL(&window, &renderer);

	return 0;
}
