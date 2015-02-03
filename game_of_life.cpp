#include "game.h"

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

int32
get_board_value(const int32 *board, int32 rows, int32 cols, int32 x, int32 y)
{
    assert(x >= 0);
    assert(x < cols);
    assert(y >= 0);
    assert(y < rows);
    return board[y * cols + x];
}

void
set_board_value(int32 *board, int32 rows, int32 cols, int32 x, int32 y, int32 value)
{
    assert(x < cols);
    assert(y < rows);
    board[y * cols + x] = value;
}

/*
 Rules:

 1. Any live cell with fewer than two live neighbours dies, as if caused by under-population.
 2. Any live cell with two or three live neighbours lives on to the next generation.
 3. Any live cell with more than three live neighbours dies, as if by overcrowding.
 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 */
int32
next_generation(int32 *next, const int32 *prev, int32 rows, int32 cols)
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

                        if(get_board_value(prev, rows, cols, realX, realY))
                        {
                            count++;
                        }
                    }
                }
            }

            int32 self = get_board_value(prev, rows, cols, x, y);
            set_board_value(next, rows, cols, x, y, self);

            if (self)
            {
                // 1. Any live cell with fewer than two live neighbours dies, as if caused by under-population.
                if(count < 2)
                {
                    set_board_value(next, rows, cols, x, y, 0);
                }
                // 3. Any live cell with more than three live neighbours dies, as if by overcrowding.
                else if(count > 3)
                {
                    set_board_value(next, rows, cols, x, y, 0);
                }
            }
            // 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
            else
            {
                if(count == 3)
                {
                    set_board_value(next, rows, cols, x, y, 1);
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
