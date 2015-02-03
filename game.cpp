#include "game.h"
#include <stdio.h>
#include "game_of_life.cpp"
#include <math.h>

#define MIN_FRAMERATE 0
#define MAX_FRAMERATE 60

#define BOARD_COLS 1920/2/2
#define BOARD_ROWS 1080/2/2

#define NUM_SHAPES 5

typedef struct v2
{
    real32 x, y;
} v2;

typedef struct shape
{
    v2 shapes[20];
    uint32 count;
} shape;

typedef struct game_state
{
    memory_arena arena;

    int32 current_generation[BOARD_ROWS*BOARD_COLS];
    int32 prev_generation[BOARD_ROWS*BOARD_COLS];

    int32 rows;
    int32 cols;

    real32 cell_height;
    real32 cell_width;

    real32 camera_x;
    real32 camera_y;

    int32 framecount;
    int32 framerate;

    bool32 paused;

    shape saved_shapes[NUM_SHAPES];
    uint32 current_saved_shape;
    uint32 total_shapes;

    uint64 total_time;
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
draw_rectangle_alpha(game_offscreen_buffer *buffer,
               real32 x, real32 y, real32 width, real32 height,
               real32 r, real32 g, real32 b, real32 a)
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

    uint8 *row = ((uint8 *)buffer->memory +
                 min_x*buffer->bytes_per_pixel +
                 min_y*buffer->pitch);

    real32 a_inverse = (1.0f - a);

    for(int iterY = min_y;
        iterY < max_y;
       ++iterY)
    {
        uint32 *pixel = (uint32 *)row;
        for(int iterX = min_x;
            iterX < max_x;
            ++iterX)
        {
            real32 dest_r = (real32)((*pixel >> 16) & 0xFF);
            real32 dest_g = (real32)((*pixel >>  8) & 0xFF);
            real32 dest_b = (real32)((*pixel >>  0) & 0xFF);

            real32 new_r = a_inverse*dest_r + a*r;
            real32 new_g = a_inverse*dest_g + a*g;
            real32 new_b = a_inverse*dest_b + a*b;

            uint32 color = ((round_real32_to_uint32(new_r * 255.0f) << 16) |
                            (round_real32_to_uint32(new_g * 255.0f) << 8) |
                            (round_real32_to_uint32(new_b * 255.0f) << 0));

            *pixel++ = color;
        }

        row += buffer->pitch;
    }
}

internal void
draw_rectangle(game_offscreen_buffer *buffer,
               real32 x, real32 y, real32 width, real32 height,
               real32 r, real32 g, real32 b)
{
    draw_rectangle_alpha(buffer,
                         x, y,
                         width, height,
                         r, g, b, 1.0f);
}

internal void
constrain_camera(game_state *state, game_offscreen_buffer *buffer)
{
    real32 min_camera_x = 0;
    real32 max_camera_x = state->cols*state->cell_width - buffer->width;
    real32 min_camera_y = 0;
    real32 max_camera_y = state->rows*state->cell_height - buffer->height;

    state->camera_x = MIN(MAX(state->camera_x, min_camera_x), max_camera_x);
    state->camera_y = MIN(MAX(state->camera_y, min_camera_y), max_camera_y);
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    assert(sizeof(game_state) <= memory->permanent_storage_size);

    game_state *state = (game_state *)memory->permanent_storage;

    if(!memory->is_initialized)
    {
        memory->is_initialized = true;

        initialize_arena(&state->arena, memory->permanent_storage_size - sizeof(game_state),
        (uint8 *)memory->permanent_storage + sizeof(game_state));

        state->rows = BOARD_ROWS;
        state->cols = BOARD_COLS;

        state->cell_width = 30.0f;
        state->cell_height = 30.0f;

        // glider
        // state->current_generation[0*state->cols + 1] = 1;
        // state->current_generation[1*state->cols + 2] = 1;
        // state->current_generation[2*state->cols + 0] = 1;
        // state->current_generation[2*state->cols + 1] = 1;
        // state->current_generation[2*state->cols + 2] = 1;

        // state->current_generation[5*state->cols + 11] = 1;
        // state->current_generation[6*state->cols + 12] = 1;
        // state->current_generation[7*state->cols + 10] = 1;
        // state->current_generation[7*state->cols + 11] = 1;
        // state->current_generation[7*state->cols + 12] = 1;

        // setup shapes

        // 1. single point
        {
            uint32 *count = &state->saved_shapes[state->total_shapes].count;

            state->saved_shapes[state->total_shapes].count = 1;
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {0, 0};

            ++state->total_shapes;
        }

        // 2. Line
        {
            uint32 *count = &state->saved_shapes[state->total_shapes].count;

            state->saved_shapes[state->total_shapes].count = 1;
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {0, -1};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {0, 0};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {0, 1};

            ++state->total_shapes;
        }

        // 3. flower
        {
            uint32 *count = &state->saved_shapes[state->total_shapes].count;

            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {0, -1};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {-1, 0};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {-1, 1};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {0, 2};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {1, 1};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {1, 0};

            ++state->total_shapes;
        }

        // 4. glider
        {
            uint32 *count = &state->saved_shapes[state->total_shapes].count;

            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {0, 0};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {1, 1};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {1, 2};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {0, 2};
            state->saved_shapes[state->total_shapes].shapes[(*count)++] = {-1, 2};

            ++state->total_shapes;
        }

        state->framecount = 0;
        state->framerate = 5;
    }

    for(int i = 0;
        i < array_count(input->controllers);
        ++i)
    {
        game_controller_input *controller = &input->controllers[i];
        if(controller->back.ended_down && controller->back.half_transition_count == 1)
        {
            state->paused = !state->paused;
        }

        if(controller->start.ended_down && controller->start.half_transition_count == 1)
        {
            ++state->current_saved_shape;
        }

        if(controller->action_up.ended_down)
        {
            state->framerate = MAX(state->framerate-1, MIN_FRAMERATE);
        }

        if(controller->action_down.ended_down)
        {
            state->framerate = MIN(state->framerate+1, MAX_FRAMERATE);
        }

        if(controller->action_left.ended_down)
        {
            state->framerate = MAX_FRAMERATE;
        }

        if(controller->action_right.ended_down)
        {
            state->framerate = MIN_FRAMERATE;
        }

        if(controller->move_up.ended_down)
        {
            state->camera_y -= 15.0f;
        }

        if(controller->move_down.ended_down)
        {
            state->camera_y += 15.0f;
        }

        if(controller->move_left.ended_down)
        {
            state->camera_x -= 15.0f;
        }

        if(controller->move_right.ended_down)
        {
            state->camera_x += 15.0f;
        }
    }

    if(input->MouseMiddle.ended_down)
    {
        state->camera_x += input->rel_mouse_x;
        state->camera_y += input->rel_mouse_y;
    }

    // NOTE(partkyle): update size based on mouse wheel
    state->cell_height = MAX(state->cell_height + input->mouse_z, 2);
    state->cell_width = MAX(state->cell_width + input->mouse_z, 2);

    constrain_camera(state, buffer);

    int32 mouse_x = (int32)((input->mouse_x + state->camera_x) / state->cell_width);
    int32 mouse_y = (int32)((input->mouse_y + state->camera_y) / state->cell_height);

    shape current_shape = state->saved_shapes[state->current_saved_shape%state->total_shapes];

    if(input->MouseLeft.ended_down)
    {
        for(int i=0;
            i < current_shape.count;
            ++i)
        {
            int32 cell_x = constrain(mouse_x + current_shape.shapes[i].x, BOARD_COLS);
            int32 cell_y = constrain(mouse_y + current_shape.shapes[i].y, BOARD_ROWS);

            set_board_value(state->current_generation, state->rows, state->cols,
                            cell_x, cell_y, 1);

        }
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

    // DEBUG layer to show edges
    draw_rectangle(buffer, 0, 0, buffer->width, buffer->height, 1.0f, 0.0f, 1.0f);


    real32 alpha_value = 1.0f;
    if(state->paused)
    {
        state->total_time += 1;
        alpha_value = MIN(0.33f*((real32)sin((real32)state->total_time / 25.0f) + 1.0f) + 0.25f, 1.0f);
    }

    for (int y = 0; y < state->rows; ++y)
    {
        for (int x = 0; x < state->cols; ++x)
        {
            int32 cell = get_board_value(state->current_generation, state->rows, state->cols, x, y);

            real32 cell_pos_x = x*state->cell_width - state->camera_x;
            real32 cell_pos_y = y*state->cell_height - state->camera_y;

#if 0
            // grid lines
            draw_rectangle(buffer,
                           cell_pos_x, cell_pos_y,
                           state->cell_width, state->cell_height,
                           0.5f, 0.5f, 0.5f);

            // dead cells by default (used for alpha)
            draw_rectangle(buffer,
                           1+cell_pos_x, 1+cell_pos_y,
                           -1+state->cell_width, -1+state->cell_height,
                           0.0f, 0.0f, 0.0f);
#else
            // dead cells by default (used for alpha)
            draw_rectangle(buffer,
                           cell_pos_x, cell_pos_y,
                           state->cell_width, state->cell_height,
                           0.0f, 0.0f, 0.0f);
#endif


            if(cell)
            {
                real32 r = 1.0f - (real32)state->framerate / (real32)MAX_FRAMERATE;
                real32 g = 0.4f;
                real32 b = (real32)state->framerate / (real32)MAX_FRAMERATE;

                // live cells
                draw_rectangle_alpha(buffer,
                                     1+cell_pos_x, 1+cell_pos_y,
                                     state->cell_width-1, state->cell_height-1,
                                     r, g, b, alpha_value);
            }
#if 0
            // prev gen dots
            int32 prev_cell = get_board_value(state->prev_generation,
                                              state->rows, state->cols,
                                              x, y);
            if(prev_cell)
            {
                real32 r = 1.0f;
                real32 g = 1.0f;
                real32 b = 1.0f;

                draw_rectangle(buffer,
                               cell_pos_x+(state->cell_width*0.45f), cell_pos_y+(state->cell_height*0.5f),
                               1, 1,
                               r, g, b);
            }
#endif
        }
#if 0
        real32 offset_x = 0;
        real32 offset_y = 0;
        draw_rectangle(buffer,
                       offset_x + mouse_x*state->cell_width - state->camera_x, offset_y + mouse_y*state->cell_height - state->camera_y,
                       state->cell_width, state->cell_height,
                       1.0f, 1.0f, 0.0f);

#else
        for(int i=0;
            i < current_shape.count;
            ++i)
        {
            real32 offset_x = current_shape.shapes[i].x * state->cell_width;
            real32 offset_y = current_shape.shapes[i].y * state->cell_height;

            // NOTE(partkyle): draw mouse position
            draw_rectangle(buffer,
                           offset_x + mouse_x*state->cell_width - state->camera_x, offset_y + mouse_y*state->cell_height - state->camera_y,
                           state->cell_width, state->cell_height,
                           1.0f, 1.0f, 0.0f);
        }
    }
#endif

    return(0);
}
