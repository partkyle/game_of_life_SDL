#include "game.h"

typedef struct cell
{
    bool32 alive;
    uint32 age;
} cell;


int
constrain(int x, int bound)
{
    int result = x;

    if(result < 0)
    {
        result = bound + x;
    }

    if(result >= bound)
    {
        result = x % bound;
    }

    return result;
}

const cell *
get_board_value(const cell *board, int32 rows, int32 cols, int32 x, int32 y)
{
    assert(x >= 0);
    assert(x < cols);
    assert(y >= 0);
    assert(y < rows);
    return &board[y * cols + x];
}

void
make_cell_alive(cell *board, int32 rows, int32 cols, int32 x, int32 y)
{
    cell *c = &board[y * cols + x];
    if(c->alive)
    {
        c->age = (c->age+1) % 60;
    }
    else
    {
        c->alive = 1;
        c->age = 0;
    }
}

void
kill_cell(cell *board, int32 rows, int32 cols, int32 x, int32 y)
{
    cell *c = &board[y * cols + x];
    c->alive = 0;
    c->age = 0;
}

/*
 Rules:

 1. Any live cell with fewer than two live neighbours dies, as if caused by under-population.
 2. Any live cell with two or three live neighbours lives on to the next generation.
 3. Any live cell with more than three live neighbours dies, as if by overcrowding.
 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 */
int32
next_generation(cell *next, const cell *prev, int32 rows, int32 cols)
{
    int32 population = 0;
    for(int32 y = 0;
        y < rows;
        ++y)
    {
        for(int32 x = 0; x < cols; ++x)
        {
            int count = 0;
            // count neighbors
            for(int32 yy = -1; yy <= 1; ++yy)
            {
                for(int32 xx = -1; xx <= 1; ++xx)
                {
                    // skip the middle
                    if(xx != 0 || yy != 0)
                    {
                        int32 realX = constrain(xx + x, cols);
                        int32 realY = constrain(yy + y, rows);

                        const cell *c = get_board_value(prev, rows, cols, realX, realY);
                        if(c->alive)
                        {
                            count++;
                        }
                    }
                }
            }

            const cell *self = get_board_value(prev, rows, cols, x, y);
            next[rows*y + cols] = *self;

            if (self->alive)
            {
                // 1. Any live cell with fewer than two live neighbours dies, as if caused by under-population.
                if(count < 2)
                {
                    kill_cell(next, rows, cols, x, y);
                }
                // 3. Any live cell with more than three live neighbours dies, as if by overcrowding.
                else if(count > 3)
                {
                    kill_cell(next, rows, cols, x, y);
                }
                else
                {
                    make_cell_alive(next, rows, cols, x, y);
                }
            }
            // 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
            else
            {
                if(count == 3)
                {
                    make_cell_alive(next, rows, cols, x, y);
                }
                else
                {
                    kill_cell(next, rows, cols, x, y);
                }
            }

            // increase pop
            if(get_board_value(next, rows, cols, x, y))
            {
                population++;
            }
        }
    }

    return population;
}
