#include <iostream>

#include <SDL2/SDL.h>

#include "types.h"
#include "sdl_platform.h"

internal bool32
handle_event(SDL_Event *event)
{
    bool32 should_quit;
    switch(event->type)
    {
        case SDL_QUIT:
        {
            printf("SDL_QUIT\n");
            should_quit = true;
        } break;

    }
    return(should_quit);
}

internal void
SDL_update_window(SDL_Window *_, SDL_Renderer *renderer, sdl_offscreen_buffer *buffer)
{
    SDL_UpdateTexture(buffer->Texture,
                      0,
                      buffer->Memory,
                      buffer->Pitch);

    SDL_RenderCopy(renderer,
                   buffer->Texture,
                   0,
                   0);

    SDL_RenderPresent(renderer);
}


internal bool32
SDL_process_pending_messages()
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if (handle_event(&event))
        {
            return(false);
        }
    }

    return(true);
}

int32
main(int32 argc, char * arg[])
{
    // init SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        std::cout << "ERROR SDL_Init" << std::endl;

        return -1;
    }

    // create a window
    SDL_Window *window = SDL_CreateWindow("@partkyle SDL Platform",             // window title
                                          SDL_WINDOWPOS_CENTERED,     // x position, centered
                                          SDL_WINDOWPOS_CENTERED,     // y position, centered
                                          1920 / 2,                        // width, in pixels
                                          1080 / 2,                        // height, in pixels
                                          SDL_WINDOW_OPENGL           // flags
                                          );

    bool32 Running = true;

    while(Running)
    {
        Running = SDL_process_pending_messages();
    }
    // clean up
    return 0;
}
