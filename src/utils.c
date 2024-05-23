#include "../include/utils.h"

float g_scale;

static const unsigned int ORIGIN_RES_WIDTH = 1920;
static const unsigned int ORIGIN_RES_HEIGHT = 1080;

int init_SDL(SDL_Window** window, SDL_Renderer** renderer, FPSmanager* fps_manager, int window_width, int window_height) {
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "SDL initialization error", SDL_GetError(), NULL);

		return -1;
	}

	// Create window
	*window = SDL_CreateWindow("Game of life", 0, 0, 0, 0, SDL_WINDOW_SHOWN);
	if (*window == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create window: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to create window", SDL_GetError(), NULL);

		SDL_Quit();
		return -1;
	}

	// Initialize framerate manager
	SDL_initFramerate(fps_manager);

	// Get index of a display associated with SDL window
	int displayIndex = SDL_GetWindowDisplayIndex(*window);
	if (displayIndex != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to retrieve index of a display associated with SDL window: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to retrieve index of a display associated with SDL window", SDL_GetError(), *window);

		SDL_DestroyWindow(*window);
		SDL_Quit();
		return -1;
	}

	// Get current display mode
	SDL_DisplayMode displayMode;
	if (SDL_GetCurrentDisplayMode(displayIndex, &displayMode) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to retrieve display information: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to retrieve display information", SDL_GetError(), *window);

		SDL_DestroyWindow(*window);
		SDL_Quit();
		return -1;
	}

	// Set target framerate for framerate manager
	if (SDL_setFramerate(fps_manager, displayMode.refresh_rate) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to set target framerate for framerate manager\n");
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Framerate manager initialization error", "Failed to set target framerate for framerate manager", *window);

		SDL_DestroyWindow(*window);
		SDL_Quit();
		return -1;
	}

	// Create renderer
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (*renderer == NULL) {
		SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "Failed to create renderer: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to create renderer", SDL_GetError(), *window);

		SDL_DestroyWindow(*window);
		SDL_Quit();
		return -1;
	}

	// Set global scaling factor
	g_scale = displayMode.w == (signed)ORIGIN_RES_WIDTH ? displayMode.w / (float)ORIGIN_RES_WIDTH : displayMode.h / (float)ORIGIN_RES_HEIGHT;

	// Set appropriate window size and position
	SDL_SetWindowSize(*window, window_width * g_scale, window_height * g_scale);
	SDL_SetWindowPosition(*window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

	// Set device independent resolution for rendering
	if (SDL_RenderSetLogicalSize(*renderer, window_width, window_height) != 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to set device independent resolution for rendering: %s\n", SDL_GetError());
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Failed to set device independent resolution for rendering", SDL_GetError(), *window);

		SDL_DestroyRenderer(*renderer);
		SDL_DestroyWindow(*window);
		SDL_Quit();
		return -1;
	}

	return 0;
}

void close_SDL(SDL_Window* window, SDL_Renderer* renderer) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}

void clear_screen(SDL_Renderer* renderer, Uint32 color) {
	Uint8* c = (Uint8*)&color;
	if (SDL_SetRenderDrawColor(renderer, c[0], c[1], c[2], c[3]) != 0) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to set the color for SDL_RenderClear(): %s\n", SDL_GetError());
	}
	if (SDL_RenderClear(renderer) != 0) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Failed to clear the screen: %s\n", SDL_GetError());
	}
}
