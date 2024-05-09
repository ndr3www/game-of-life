#include <time.h>
#include <SDL2/SDL2_framerate.h>
#include "../lib/SDL_FontCache.h"

#include "../include/cells.h"

const Uint32 FONT_SIZE = 24;
const Uint32 GUI_GAP = FONT_SIZE * 3;

const Sint16 CELL_SIZE = 8;
const unsigned int CELL_NUMBER_WIDTH = 128;
const unsigned int CELL_NUMBER_HEIGHT = 100;

const int SCREEN_WIDTH = CELL_SIZE * CELL_NUMBER_WIDTH;
const int SCREEN_HEIGHT = CELL_SIZE * CELL_NUMBER_HEIGHT + GUI_GAP;
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

void close_SDL(SDL_Window* window, SDL_Renderer* renderer) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}

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
	Uint64 fps_prev_time = 0, fps_current_time;
	Uint32 fps = 0;
	Uint32 fps_avg = refresh_rate;
	unsigned int frame_count = 1;

	// Stuff for controlling logic calculations speed
	Uint64 logic_prev_time = 0, logic_current_time;
	Uint64 logic_delay = 20;  // in miliseconds
	
	// Cells creation
	CellsGrid* cells_grid = cells_grid_new(CELL_NUMBER_WIDTH, CELL_NUMBER_HEIGHT, CELL_SIZE);

	// Font setup
	FC_Font* font = FC_CreateFont();
	FC_LoadFont(font, renderer, "../../fonts/Minecraft-Regular.otf", FONT_SIZE, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL);

	size_t tick = 0;

	// Main loop
	while (!quit) {
		Uint32 frame_time = SDL_framerateDelay(&fpsManager);

		while(SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) {
				quit = 1;
			}
			else if (e.type == SDL_KEYDOWN) {
				if (e.key.repeat == 0) {
					switch (e.key.keysym.sym) {
						case SDLK_ESCAPE:
							quit = 1;
							break;
						case SDLK_p:
							pause = !pause;
							break;
						case SDLK_r:
							for (size_t x = 0; x < cells_grid->width; ++x) {
								for (size_t y = 0; y < cells_grid->height; ++y) {
									cells_grid->cell[x][y].is_alive = rand() % 2;
								}
							}
							tick = 0;
							break;
						case SDLK_RIGHT:
							logic_delay -= logic_delay > 0u ? 10u : 0u;
							break;
						case SDLK_LEFT:
							logic_delay += logic_delay < 1000u ? 10u : 0;
							break;
					}
				}
				else {
					switch (e.key.keysym.sym) {
						case SDLK_RIGHT:
							logic_delay -= logic_delay > 0u ? 10u : 0u;
							break;
						case SDLK_LEFT:
							logic_delay += logic_delay < 1000u ? 10u : 0;
							break;
					}
				}
			}

			// Change hovered cell state to alive when left mouse button has been clicked
			int mouse_x, mouse_y;	
			if (SDL_GetMouseState(&mouse_x, &mouse_y) == 1) {
				for (size_t x = 0; x < cells_grid->width; ++x) {
					for (size_t y = 0; y < cells_grid->height; ++y) {
						if ((mouse_x >= cells_grid->cell[x][y].pos_x && mouse_x <= cells_grid->cell[x][y].pos_x + CELL_SIZE) &&
							(mouse_y >= cells_grid->cell[x][y].pos_y && mouse_y <= cells_grid->cell[x][y].pos_y + CELL_SIZE)) {
							cells_grid->cell[x][y].is_alive = 1;
						}
					}
				}
			}
		}

		// Logic
		logic_current_time = SDL_GetTicks64();
		if (!pause && logic_current_time > logic_prev_time + logic_delay) {
			// Count alive neighbours
			for (size_t x = 0; x < cells_grid->width; ++x) {
				for (size_t y = 0; y < cells_grid->height; ++y) {
					cells_grid->cell[x][y].alive_neighbours = 0;

					// right
					if (x < cells_grid->width - 1) {
						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x + 1][y].is_alive ? 1 : 0;
					}
					// left
					if (x > 0) {
						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x - 1][y].is_alive ? 1 : 0;
					}
					// top
					if (y > 0) {
						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x][y - 1].is_alive ? 1 : 0;
					}
					// bottom
					if (y < cells_grid->height - 1) {
						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x][y + 1].is_alive ? 1 : 0;
					}
					// top right
					if (x < cells_grid->width - 1 && y > 0) {
						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x + 1][y - 1].is_alive ? 1 : 0;
					}
					// top left
					if (x > 0 && y > 0) {
						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x - 1][y - 1].is_alive ? 1 : 0;
					}
					// bottom right
					if (x < cells_grid->width - 1 && y < cells_grid->height - 1) {
						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x + 1][y + 1].is_alive ? 1 : 0;
					}
					// bottom left
					if (x > 0 && y < cells_grid->height - 1) {
						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x - 1][y + 1].is_alive ? 1 : 0;
					}
				}
			}

			// Check rules
			for (size_t x = 0; x < cells_grid->width; ++x) {
				for (size_t y = 0; y < cells_grid->height; ++y) {
					if (cells_grid->cell[x][y].is_alive) {
						cells_grid->cell[x][y].is_alive = !(cells_grid->cell[x][y].alive_neighbours < 2 || cells_grid->cell[x][y].alive_neighbours > 3);
					}
					else {
						cells_grid->cell[x][y].is_alive = (cells_grid->cell[x][y].alive_neighbours == 3);
					}
				}
			}

			++tick;

			logic_prev_time = logic_current_time;
		}

		// Clear the screen with specified color
		if (SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0) {
			fprintf(stderr, "Failed to set the color for SDL_RenderClear: %s\n", SDL_GetError());
		}
		if (SDL_RenderClear(renderer) < 0) {
			fprintf(stderr, "Failed to clear the screen: %s\n", SDL_GetError());
		}

		// Drawing 
		for (size_t x = 0; x < cells_grid->width; ++x) {
			for (size_t y = 0; y < cells_grid->height; ++y) {
				int return_code = boxColor(renderer,
							 			   cells_grid->cell[x][y].pos_x, cells_grid->cell[x][y].pos_y,
							 			   cells_grid->cell[x][y].pos_x + CELL_SIZE, cells_grid->cell[x][y].pos_y + CELL_SIZE,
							 			   cells_grid->cell[x][y].is_alive ? 0xffffffff : 0x000000ff);
				if (return_code == -1) {
					fprintf(stderr, "Failed to render cell[%lu][%lu]\n", x, y);
				}
			}
		}

		// Calculate FPS every second
		fps_current_time = SDL_GetTicks64();
		if (fps_current_time > fps_prev_time + 1000) {
			fps_avg = fps / frame_count + 1;
			frame_count = 1;
			fps = 0;

			fps_prev_time = fps_current_time;
		}
		else {
			fps += 1000.0f / (frame_time == 0u ? 1u : frame_time);
			++frame_count;
		}

		FC_Draw(font, renderer, 0, SCREEN_HEIGHT - GUI_GAP, "FPS: %d\nTick: %lu\nDelay: %d\n", fps_avg, tick, logic_delay);

		SDL_RenderPresent(renderer);
	}

	// Clean up
	cells_grid_delete(cells_grid);
	FC_FreeFont(font);
	close_SDL(window, renderer);

	return 0;
}
