#include <time.h>
#include "../lib/SDL_FontCache.h"

#include "../include/utils.h"
#include "../include/cells.h"

#define SDL_MAIN_HANDLED

static const Uint32 FONT_SIZE = 24;
static const Uint32 GUI_GAP = FONT_SIZE * 3;

static const Sint16 CELL_SIZE = 8;
static const unsigned int CELL_NUMBER_WIDTH = 128;
static const unsigned int CELL_NUMBER_HEIGHT = 100;

static const int WINDOW_WIDTH = CELL_NUMBER_WIDTH * CELL_SIZE;
static const int WINDOW_HEIGHT = CELL_NUMBER_HEIGHT * CELL_SIZE;

int main(int argc, char* argv[]) {
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	FPSmanager fpsManager;

	if (init_SDL(&window, &renderer, &fpsManager, WINDOW_WIDTH, WINDOW_HEIGHT) != 0) {
		return 1;
	}

	// Font setup
	FC_Font* font = FC_CreateFont();
	const char* font_path = "fonts/Minecraft-Regular.otf";
	if (!FC_LoadFont(font, renderer, font_path, FONT_SIZE, FC_MakeColor(255, 255, 255, 255), TTF_STYLE_NORMAL)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to load font '%s'\n", font_path);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to load font", font_path, window);

		FC_FreeFont(font);
		close_SDL(window, renderer);
		return 2;
	}
	
	// Initialize RNG
	time_t unix_time = time(NULL);
	if (unix_time == (time_t)(-1)) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to retrieve current Unix timestamp\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "RNG initialization error", "Failed to retrieve current Unix timestamp", window);

		FC_FreeFont(font);
		close_SDL(window, renderer);
		return 3;
	}
	srand(unix_time);

	// Main loop flags
	int quit = 0, pause = 1, draw_mesh = 0;

	// Events handler
	SDL_Event e;

	// Stuff for calculating FPS at set interval
	Uint64 fps_prev_time = 0, fps_current_time;
	Uint32 fps = 0;
	Uint32 fps_avg = 0;
	unsigned int frame_count = 1;

	// Stuff for controlling logic calculations speed
	Uint64 logic_prev_time = 0, logic_current_time;
	Uint64 logic_delay = 0;  // in miliseconds

	size_t tick = 0;

	SDL_Rect viewport = {0, GUI_GAP, WINDOW_WIDTH, WINDOW_HEIGHT - GUI_GAP};

	// Cells creation
	CellsGrid* cells_grid = CellsGrid_create(CELL_NUMBER_WIDTH, CELL_NUMBER_HEIGHT, CELL_SIZE);
	if (cells_grid == NULL) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Cells initialization error", "Failed to create cells", window);

		FC_FreeFont(font);
		close_SDL(window, renderer);
		return 4;
	}

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
			switch (e.type) {
				case SDL_QUIT:
					quit = 1;
					break;
				case SDL_KEYDOWN:
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
							case SDLK_c:
								for (size_t x = 0; x < cells_grid->width; ++x) {
									for (size_t y = 0; y < cells_grid->height; ++y) {
										cells_grid->cell[x][y].is_alive = 0;
									}
								}
								tick = 0;
								break;
							case SDLK_e:  // turn the mesh on/off
								draw_mesh = !draw_mesh;
								break;
						}
					}

					switch (e.key.keysym.sym) {
						case SDLK_RIGHT:
							logic_delay -= logic_delay > 0u ? 10u : 0u;
							break;
						case SDLK_LEFT:
							logic_delay += logic_delay < 990u ? 10u : 0;
							break;
					}

					break;
			}

			// Change hovered cell state (left button - alive, others - dead)
			int mouse_x, mouse_y;
			Uint32 mouse_button = SDL_GetMouseState(&mouse_x, &mouse_y);
			mouse_x /= g_scale;
			mouse_y /= g_scale;
			mouse_x -= viewport.x;
			mouse_y -= viewport.y;

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
						if (x_new < 0) {
							x_new = cells_grid->width - 1;
						}
						else if ((Uint16)x_new > cells_grid->width - 1) {
							x_new = 0;
						}

						Sint16 y_new = y + directions[i].y;
						if (y_new < 0) {
							y_new = cells_grid->height - 1;
						}
						else if ((Uint16)y_new > cells_grid->height - 1) {
							y_new = 0;
						}

						cells_grid->cell[x][y].alive_neighbours += cells_grid->cell[x_new][y_new].is_alive ? 1 : 0;
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

		clear_screen(renderer, BLACK_HEX);

		CellsGrid_draw(renderer, &viewport, cells_grid, draw_mesh);

		// Calculate FPS every second
		fps_current_time = SDL_GetTicks64();
		if (fps_current_time > fps_prev_time + 1000) {
			fps_avg = fps / frame_count + 2;
			frame_count = 1;
			fps = 0;

			fps_prev_time = fps_current_time;
		}
		else {
			fps += 1000.0f / (frame_time == 0u ? 1u : frame_time);
			++frame_count;
		}

		// Draw GUI
		if (SDL_RenderSetViewport(renderer, NULL) != 0) {
			SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to set viewport for GUI: %s\n", SDL_GetError());
		}
		FC_Draw(font, renderer, 0, 0, "FPS: %d\nTick: %lu\nSpeed: x%.2f\n", fps_avg, tick, (-(logic_delay / 100.0f) + 10.0f) / 10.0f);

		SDL_RenderPresent(renderer);
	}

	// Clean up
	CellsGrid_delete(cells_grid);
	FC_FreeFont(font);
	close_SDL(window, renderer);

	return 0;
}
