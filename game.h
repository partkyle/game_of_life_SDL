#ifndef GAME_H
#define GAME_H

#include "types.h"

typedef struct game_offscreen_buffer
{
    // NOTE(casey): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
} game_offscreen_buffer;

#define GAME_UPDATE_AND_RENDER(name) int32 name(game_offscreen_buffer *buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

typedef struct game_code
{
  void* game_code_dll;
  int64 DLL_last_write_time;

    // IMPORTANT(casey): Either of the callbacks can be 0!  You must
    // check before calling.
  game_update_and_render* update_and_render;

  bool32 valid;
} game_code;

#endif
