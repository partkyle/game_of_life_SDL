#include "game.h"
#include <stdio.h>

internal void
RenderWeirdGradient(game_offscreen_buffer *buffer, int blueoffset, int greenoffset)
{
    // todo(casey): let's see what the optimizer does

    uint8 *row = (uint8 *)buffer->memory;
    for(int y = 0;
        y < buffer->height;
        ++y)
    {
        uint32 *pixel = (uint32 *)row;
        for(int x = 0;
            x < buffer->width;
            ++x)
        {
            uint8 blue = (uint8)(x + blueoffset);
            uint8 green = (uint8)(y + greenoffset);

            *pixel++ = ((green << 8) | blue);
        }

        row += buffer->pitch;
    }
}

internal uint32
round_real32_to_int32(real32 value)
{
    return((int32)value + 0.5f);
}

internal uint32
round_real32_to_uint32(real32 value)
{
    return((uint32)value + 0.5f);
}

internal void
draw_rectangle(game_offscreen_buffer *buffer,
               real32 x, real32 y, real32 width, real32 height,
               real32 r, real32 g, real32 b)
{
    int32 min_x = round_real32_to_int32(x);
    int32 min_y = round_real32_to_int32(y);
    int32 max_x = round_real32_to_int32(x + width);
    int32 max_y = round_real32_to_int32(y + height);

    if(min_x < 0)
    {
        min_x = 0;
    }

    if(min_y < 0)
    {
        min_y = 0;
    }

    if(max_x > buffer->width)
    {
        max_x = buffer->width;
    }

    if(max_y > buffer->height)
    {
        max_y = buffer->height;
    }

    uint32 color = ((round_real32_to_uint32(r * 255.0f) << 16) |
                    (round_real32_to_uint32(g * 255.0f) << 8) |
                    (round_real32_to_uint32(b * 255.0f) << 0));

    uint8 *row = ((uint8 *)buffer->memory +
                 min_x*buffer->bytes_per_pixel +
                 min_y*buffer->pitch);

    for(int iterY = min_y;
        iterY < max_y;
       ++iterY)
    {
      uint32 *pixel = (uint32 *)row;
      for(int iterX = min_x;
          iterX < max_x;
          ++iterX)
      {
          *pixel++ = color;
      }

      row += buffer->pitch;
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
  game_state *state = (game_state *)memory->permanent_storage;

  if(!memory->is_initialized)
  {
    memory->is_initialized = true;
  }

  for(int32 i = 0;
      i < 5;
      ++i)
  {
    game_controller_input *controller = &input->controllers[i];
    if(controller->move_up.ended_down)
    {
        --state->greenoffset;
    }
    if(controller->move_down.ended_down)
    {
        ++state->greenoffset;
    }
    if(controller->move_left.ended_down)
    {
        --state->blueoffset;
    }
    if(controller->move_right.ended_down)
    {
        ++state->blueoffset;
    }
  }

  state->blueoffset -= input->rel_mouse_x;
  state->greenoffset -= input->rel_mouse_y;

  RenderWeirdGradient(buffer, state->blueoffset, state->greenoffset);

  int x = input->mouse_x;
  int y = input->mouse_y;
  draw_rectangle(buffer, x, y, 20, 20, 0.5f, 0, 0.5f);

  return(0);
}
