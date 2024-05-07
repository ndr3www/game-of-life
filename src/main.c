#include <SDL2/SDL_stdinc.h>
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
		fprintf(stderr, "Failed to set target framerate for framerate manager: %s\n", SDL_GetError());
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
	Sint16 x, y;
	int alive;
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

	// Create cells
	Cell* cells = malloc(sizeof(Cell) * CELL_NUMBER_TOTAL);
	if (cells == NULL) {
		fprintf(stderr, "Failed to allocate cells memory\n");
		return 2;
	}

	// Initialize cells
	size_t index = 0;
	for (int y = 0; y < SCREEN_HEIGHT; y += CELL_SIZE) {
		for (int x = 0; x < SCREEN_WIDTH; x += CELL_SIZE) {
			cells[index].x = x;
			cells[index].y = y;
			cells[index++].alive = rand() % 2;
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
				}
			}

			// Change hovered cell state to alive when left mouse button has been clicked
			int mouse_x, mouse_y;	
			if (SDL_GetMouseState(&mouse_x, &mouse_y) == 1) {
				for (size_t i = 0; i < CELL_NUMBER_TOTAL; ++i) {
					if ((mouse_x >= cells[i].x && mouse_x <= cells[i].x + CELL_SIZE) && (mouse_y >= cells[i].y && mouse_y <= cells[i].y + CELL_SIZE)) {
						cells[i].alive = 1;
					}
				}
			}
		}

		// Clear screen with specified color
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Drawing 
		for (size_t i = 0; i < CELL_NUMBER_TOTAL; ++i) {
			boxColor(renderer, cells[i].x, cells[i].y, cells[i].x + CELL_SIZE, cells[i].y + CELL_SIZE, cells[i].alive ? 0xffffffff : 0x000000ff);
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
	free(cells);
	close_SDL(&window, &renderer);

	return 0;
}
