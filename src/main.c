#include <time.h>
#include "../lib/SDL_FontCache.h"

#include "../include/utils.h"
#include "../include/cells.h"

static const Uint32 FONT_SIZE = 24;
static const Uint32 GUI_GAP = FONT_SIZE * 3;

static const Sint16 CELL_SIZE = 8;
static const unsigned int CELL_NUMBER_WIDTH = 128;
static const unsigned int CELL_NUMBER_HEIGHT = 100;

static const int SCREEN_WIDTH = CELL_SIZE * CELL_NUMBER_WIDTH;
static const int SCREEN_HEIGHT = CELL_SIZE * CELL_NUMBER_HEIGHT + GUI_GAP;

int main() {
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	FPSmanager fpsManager;

	if (init_SDL(&window, &renderer, &fpsManager, SCREEN_WIDTH, SCREEN_HEIGHT) != 0) {
		return 1;
	}

	// Font setup
	FC_Font* font = FC_CreateFont();
	FC_LoadFont(font, renderer, "../../fonts/Minecraft-Regular.otf", FONT_SIZE, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL);
	
	// Initialize RNG
	srand(time(NULL));

	// Main loop flags
	int quit = 0, pause = 1;

	// Events handler
	SDL_Event e;

	// Stuff for calculating FPS at set interval
	Uint64 fps_prev_time = 0, fps_current_time;
	Uint32 fps = 0;
	Uint32 fps_avg = 0;
	unsigned int frame_count = 1;

	// Stuff for controlling logic calculations speed
	Uint64 logic_prev_time = 0, logic_current_time;
	Uint64 logic_delay = 20;  // in miliseconds

	size_t tick = 0;

	// Cells creation
	CellsGrid* cells_grid = CellsGrid_create(CELL_NUMBER_WIDTH, CELL_NUMBER_HEIGHT, CELL_SIZE);

	// Define directions for counting alive neighbours
	Pair_Sint16 directions[] = {
		{TOP, LEFT},
		{TOP, NA},
		{TOP, RIGHT},
		{NA, RIGHT},
		{BOTTOM, RIGHT},
		{BOTTOM, NA},
		{BOTTOM, LEFT},
		{NA, LEFT}
	};
	const int DIRECTIONS_SIZE = sizeof(directions) / sizeof(directions[0]);

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

			// Change hovered cell state (left button - alive, others - dead)
			int mouse_x, mouse_y;
			Uint32 mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);
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

		// Logic
		logic_current_time = SDL_GetTicks64();
		if (!pause && logic_current_time > logic_prev_time + logic_delay) {
			// Count alive neighbours
			for (size_t x = 0; x < cells_grid->width; ++x) {
				for (size_t y = 0; y < cells_grid->height; ++y) {
					cells_grid->cell[x][y].alive_neighbours = 0;

					for (int i = 0; i < DIRECTIONS_SIZE; ++i) {
						Sint16 x_new = x + directions[i].x;
						Sint16 y_new = y + directions[i].y;

						if ((x_new >= 0 && x_new < (Sint16)cells_grid->width) && (y_new >= 0 && y_new < (Sint16)cells_grid->height)) {
							cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x_new][y_new].is_alive ? 1 : 0;
						}
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
							 			   cells_grid->cell[x][y].pos_x + cells_grid->cell_size, cells_grid->cell[x][y].pos_y + cells_grid->cell_size,
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
	CellsGrid_delete(cells_grid);
	FC_FreeFont(font);
	close_SDL(window, renderer);

	return 0;
}
