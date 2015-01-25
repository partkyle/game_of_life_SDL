#ifndef SDL_EXAMPLE_H
#define SDL_EXAMPLE_H

#include "SDL.h"
#include "types.h"
#include "game.h"

struct sdl_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    SDL_Texture *texture;

    game_offscreen_buffer *game_buffer;
};

struct sdl_window_dimension
{
  int32 width;
  int32 height;
};

#endif
