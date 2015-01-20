#ifndef SDL_EXAMPLE_H
#define SDL_EXAMPLE_H

#include "types.h"

struct sdl_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    SDL_Texture *Texture;

    void *Memory;

    int32 Width;
    int32 Height;
    int32 Pitch;
};

#endif
