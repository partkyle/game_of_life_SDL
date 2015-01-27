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
    int half_transition_count;
    bool32 ended_down;
} game_button_state;

typedef struct game_controller_input
{
    bool32 is_connected;
    bool32 is_analog;
    real32 stick_average_x;
    real32 stick_average_y;

    union
    {
        game_button_state Buttons[12];
        struct
        {
            game_button_state move_up;
            game_button_state move_down;
            game_button_state move_left;
            game_button_state move_right;

            game_button_state action_up;
            game_button_state action_down;
            game_button_state action_left;
            game_button_state action_right;

            game_button_state left_shoulder;
            game_button_state right_shoulder;

            game_button_state back;
            game_button_state start;

            // NOTE(casey): All buttons must be added above this line

            game_button_state terminator;
        };
    };
} game_controller_input;

typedef struct game_input
{
    game_button_state mouse_buttons[5];
    int32 mouse_x, mouse_y, mouse_z;

    real32 dtForFrame;

    game_controller_input controllers[5];
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
  uint64 DLL_last_write_time;

    // IMPORTANT(casey): Either of the callbacks can be 0!  You must
    // check before calling.
  game_update_and_render* update_and_render;

  bool32 valid;
} game_code;

#endif
