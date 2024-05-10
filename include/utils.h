#pragma once

#include <SDL2/SDL2_framerate.h>

typedef struct Pair_Sint16_Struct {
	Sint16 y, x;
} Pair_Sint16;

enum Directions {
	TOP = -1,
	RIGHT = 1,
	BOTTOM = 1,
	LEFT = -1,
	NA = 0
};

int init_SDL(SDL_Window** window, SDL_Renderer** renderer, FPSmanager* fps_manager, int screen_width, int screen_height);
void close_SDL(SDL_Window* window, SDL_Renderer* renderer);
