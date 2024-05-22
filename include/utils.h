#pragma once

#include "SDL2_framerate.h"

#define WHITE_HEX 0xffffffff

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	#define BLACK_HEX 0x000000ff
#else
	#define BLACK_HEX 0xff000000
#endif

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

void clear_screen(SDL_Renderer* renderer, Uint32 color);
