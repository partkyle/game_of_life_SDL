#ifndef SDL_EXAMPLE_H
#define SDL_EXAMPLE_H

#include "types.h"

struct sdl_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    SDL_Texture *texture;

    void *memory;

    int32 width;
    int32 height;
    int32 pitch;
};

struct sdl_window_dimension
{
  int32 width;
  int32 height;
};

#endif
