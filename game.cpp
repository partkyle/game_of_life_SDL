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
    game_controller_input *controller = &input->Controllers[i];
    if(controller->MoveUp.EndedDown)
    {
        --state->greenoffset;
    }
    if(controller->MoveDown.EndedDown)
    {
        ++state->greenoffset;
    }
    if(controller->MoveLeft.EndedDown)
    {
        --state->blueoffset;
    }
    if(controller->MoveRight.EndedDown)
    {
        ++state->blueoffset;
    }
  }

  RenderWeirdGradient(buffer, state->blueoffset, state->greenoffset);

  return(0);
}
