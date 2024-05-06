#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_framerate.h>

const int SCREEN_WIDTH = 1024;
const int SCREEN_HEIGHT = 800;
int refresh_rate;

signed char init_SDL(SDL_Window** window, SDL_Renderer** renderer, FPSmanager* fps_manager) {
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

	return 0;
}

void close_SDL(SDL_Window** window, SDL_Renderer** renderer) {
	SDL_DestroyRenderer(*renderer);
	SDL_DestroyWindow(*window);

	SDL_Quit();
}

int main() {
	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
	FPSmanager fpsManager;

	if (init_SDL(&window, &renderer, &fpsManager) != 0) { return 1; }

	// Main loop flags
	signed char quit = 0;

	// Events handler
	SDL_Event e;

	// Stuff for calculating FPS at set interval
	Uint64 prevTime = 0, currentTime;
	Uint32 fps = 0;
	Uint32 avgFPS = refresh_rate;
	unsigned int frameCount = 0;

	// Main loop
	while (!quit) {
		Uint32 frameTime = SDL_framerateDelay(&fpsManager);

		while(SDL_PollEvent(&e) != 0) {
			if (e.type == SDL_QUIT) { quit = 1; }
			else if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
				switch (e.key.keysym.sym) {
					case SDLK_ESCAPE: quit = 1; break;
				}
			}
		}

		// Clear screen with specified color
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Calculate FPS every second
		currentTime = SDL_GetTicks64();
		if (currentTime > prevTime + 1000) {
			avgFPS = fps / frameCount;
			frameCount = 0;
			fps = 0;

			prevTime = currentTime;
		}
		else {
			fps += 1000.0f / frameTime;
			++frameCount;
		}

		SDL_RenderPresent(renderer);
	}

	// Clean up
	close_SDL(&window, &renderer);

	return 0;
}
