#include "game.h"
#include <stdio.h>
#include "game_of_life.cpp"

#define MIN_FRAMERATE 0
#define MAX_FRAMERATE 30

typedef struct game_state
{
    memory_arena arena;

    int32 *current_generation;
    int32 *prev_generation;

    int32 rows;
    int32 cols;

    int32 framecount;
    int32 framerate;

    bool32 paused;
} game_state;

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

        initialize_arena(&state->arena, memory->permanent_storage_size - sizeof(game_state),
        (uint8 *)memory->permanent_storage + sizeof(game_state));

        state->rows = 9*5;
        state->cols = 16*5;

        state->current_generation = push_array(&state->arena, state->rows * state->cols, int32);
        state->prev_generation = push_array(&state->arena, state->rows * state->cols, int32);

        // glider
        state->current_generation[0*state->cols + 1] = 1;
        state->current_generation[1*state->cols + 2] = 1;
        state->current_generation[2*state->cols + 0] = 1;
        state->current_generation[2*state->cols + 1] = 1;
        state->current_generation[2*state->cols + 2] = 1;

        state->current_generation[5*state->cols + 11] = 1;
        state->current_generation[6*state->cols + 12] = 1;
        state->current_generation[7*state->cols + 10] = 1;
        state->current_generation[7*state->cols + 11] = 1;
        state->current_generation[7*state->cols + 12] = 1;

        state->framecount = 0;
        state->framerate = 5;
    }

    for(int i = 0;
        i < array_count(input->controllers);
        ++i)
    {
        game_controller_input *controller = &input->controllers[i];
        if(controller->back.ended_down)
        {
            state->paused = true;
        }

        if(controller->start.ended_down)
        {
            state->paused = false;
        }
    }

    real32 cell_width = (real32)buffer->width / (real32)state->cols;
    real32 cell_height = (real32)buffer->height / (real32)state->rows;

    // debug rectangle
    draw_rectangle(buffer, 0, 0, buffer->width, buffer->height, 1.0f, 0.0f, 1.0f);

    int32 mouse_x = (int32)(input->mouse_x / cell_width);
    int32 mouse_y = (int32)(input->mouse_y / cell_height);

    if(input->mouse_buttons[0].ended_down)
    {
        set_board_value(state->current_generation, state->rows, state->cols,
        mouse_x, mouse_y, 1);
    }

    // update framerate from the mouse wheel
    state->framerate -= input->mouse_z;
    if(state->framerate < MIN_FRAMERATE)
    {
        state->framerate = MIN_FRAMERATE;
    }
    if(state->framerate > MAX_FRAMERATE)
    {
        state->framerate = MAX_FRAMERATE;
    }

    if(!state->paused)
    {
        if(++state->framecount > state->framerate)
        {
            state->framecount = 0;

            swap(state->current_generation, state->prev_generation);

            next_generation(state->current_generation, state->prev_generation, state->rows, state->cols);
        }
    }

    for (int y = 0; y < state->rows; ++y)
    {
        for (int x = 0; x < state->cols; ++x)
        {
            int32 cell = get_board_value(state->current_generation, state->rows, state->cols, x, y);

            real32 r = 0.0f;
            real32 g = 0.0f;
            real32 b = 0.0f;

            if(cell)
            {
                r = 1.0f - (real32)state->framerate / (real32)MAX_FRAMERATE;
                g = 0.4f;
                b = (real32)state->framerate / (real32)MAX_FRAMERATE;
            }

            if(mouse_x == x && mouse_y == y)
            {
                r = 0.5f;
                g = 0.5f;
                b = 0.1f;
            }

            draw_rectangle(buffer,
                           x*cell_width, y*cell_height,
                           cell_width, cell_height,
                           r, g, b);

            int32 prev_cell = get_board_value(state->prev_generation,
                                              state->rows, state->cols,
                                              x, y);
            if(prev_cell)
            {
                r = 1.0f;
                g = 1.0f;
                b = 1.0f;
            }

            draw_rectangle(buffer,
                           (x*cell_width)+(cell_width*0.45f), (y*cell_height)+(cell_height*0.5f),
                           1, 1,
                           r, g, b);
        }
    }

    return(0);
}
