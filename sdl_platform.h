#ifndef SDL_EXAMPLE_H
#define SDL_EXAMPLE_H

#include "SDL.h"
#include "common.h"
#include "game.h"

// TODO(partkyle): handle game.h import here
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

typedef struct platform_dynamic_game
{
    char dll_filename[MAX_PATH_LENGTH];
    char tmp_dll_filename[MAX_PATH_LENGTH];
    char lock_filename[MAX_PATH_LENGTH];
} platform_dynamic_game;

#endif
