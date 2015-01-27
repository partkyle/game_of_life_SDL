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

typedef struct game_button_state
{
    int HalfTransitionCount;
    bool32 EndedDown;
} game_button_state;

typedef struct game_controller_input
{
    bool32 IsConnected;
    bool32 IsAnalog;
    real32 StickAverageX;
    real32 StickAverageY;

    union
    {
        game_button_state Buttons[12];
        struct
        {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;

            game_button_state ActionUp;
            game_button_state ActionDown;
            game_button_state ActionLeft;
            game_button_state ActionRight;

            game_button_state LeftShoulder;
            game_button_state RightShoulder;

            game_button_state Back;
            game_button_state Start;

            // NOTE(casey): All buttons must be added above this line

            game_button_state Terminator;
        };
    };
} game_controller_input;

typedef struct game_input
{
    game_button_state MouseButtons[5];
    int32 MouseX, MouseY, MouseZ;

    real32 dtForFrame;

    game_controller_input Controllers[5];
} game_input;

typedef struct game_memory
{
  bool32 is_initialized;

  uint64 permanent_storage_size;
  void *permanent_storage; // NOTE(casey): REQUIRED to be cleared to zero at startup

  uint64 transient_storage_size;
  void *transient_storage; // NOTE(casey): REQUIRED to be cleared to zero at startup
} game_memory;

typedef struct game_state
{
  int32 blueoffset;
  int32 greenoffset;
} game_state;

#define GAME_UPDATE_AND_RENDER(name) int32 name(game_offscreen_buffer *buffer, game_memory *memory, game_input *input)
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
