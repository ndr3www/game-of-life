#pragma once

#include "SDL2_framerate.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	#define BLACK_HEX 0x000000ff
	#define GRAY_HEX  0x777777ff
#else
	#define BLACK_HEX 0xff000000
	#define GRAY_HEX  0xff777777
#endif

extern float g_scale;

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

int init_SDL(SDL_Window** window, SDL_Renderer** renderer, FPSmanager* fps_manager, int window_width, int window_height);
void close_SDL(SDL_Window* window, SDL_Renderer* renderer);

void clear_screen(SDL_Renderer* renderer, Uint32 color);
