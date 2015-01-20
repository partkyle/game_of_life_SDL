#include <iostream>

#include <SDL2/SDL.h>

// TODO(partkyle): make this platform independent
#include <sys/mman.h>
#define MAP_ANONYMOUS MAP_ANON


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
    SDL_UpdateTexture(buffer->texture,
                      0,
                      buffer->memory,
                      buffer->pitch);

    SDL_RenderCopy(renderer,
                   buffer->texture,
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


internal void
SDL_resize_texture(sdl_offscreen_buffer *buffer, SDL_Renderer *renderer, int32 width, int32 height)
{
    int bytes_per_pixel = 4;
    if (buffer->memory)
    {
        munmap(buffer->memory,
               buffer->width * buffer->height * bytes_per_pixel);
    }
    if (buffer->texture)
    {
        SDL_DestroyTexture(buffer->texture);
    }
    buffer->texture = SDL_CreateTexture(renderer,
                                        SDL_PIXELFORMAT_ARGB8888,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        width,
                                        height);
    buffer->width = width;
    buffer->height = height;
    buffer->pitch = width * bytes_per_pixel;
    buffer->memory = mmap(0,
                          width * height * bytes_per_pixel,
                          PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS,
                          -1,
                          0);
}

sdl_window_dimension
SDL_get_window_dimension(SDL_Window *Window)
{
    sdl_window_dimension result;

    SDL_GetWindowSize(Window, &result.width, &result.height);

    return(result);
}

internal void
RenderWeirdGradient(sdl_offscreen_buffer *buffer, int blueoffset, int greenoffset)
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

int
game_update_and_render(sdl_offscreen_buffer *buffer)
{
  RenderWeirdGradient(buffer, 100, 0);

  return(0);
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

    if(window)
    {
      SDL_Renderer *renderer = SDL_CreateRenderer(window,
                                                  -1,
                                                  SDL_RENDERER_PRESENTVSYNC);

      sdl_offscreen_buffer buffer;

      if(renderer)
      {
        bool32 Running = true;
        sdl_window_dimension dimension = SDL_get_window_dimension(window);
        SDL_resize_texture(&buffer, renderer, dimension.width, dimension.height);

        while(Running)
        {
          Running = SDL_process_pending_messages();

          game_update_and_render(&buffer);

          SDL_update_window(window, renderer, &buffer);
        }
      }
    }
    // clean up
    return 0;
}
