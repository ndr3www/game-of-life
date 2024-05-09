#pragma once

#include <SDL2/SDL2_framerate.h>

int init_SDL(SDL_Window** window, SDL_Renderer** renderer, FPSmanager* fps_manager, int screen_width, int screen_height);

void close_SDL(SDL_Window* window, SDL_Renderer* renderer);
