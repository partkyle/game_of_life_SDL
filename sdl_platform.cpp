#include <iostream>

// TODO(partkyle): make this platform independent
#include <sys/mman.h>
#define MAP_ANONYMOUS MAP_ANON
#include <dlfcn.h>

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
                      buffer->game_buffer->memory,
                      buffer->game_buffer->pitch);

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
    if (buffer->game_buffer->memory)
    {
        munmap(buffer->game_buffer->memory,
               buffer->game_buffer->width * buffer->game_buffer->height * bytes_per_pixel);
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
    buffer->game_buffer->width = width;
    buffer->game_buffer->height = height;
    buffer->game_buffer->pitch = width * bytes_per_pixel;
    buffer->game_buffer->memory = mmap(0,
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

internal game_code
SDL_load_game_code(char *filename)
{
  game_code code = {};
  // TODO(partkyle): make this path relative
  code.game_code_dll = dlopen(filename, RTLD_LAZY|RTLD_GLOBAL);
  if(code.game_code_dll)
  {
    // TODO(partkyle): make the reload work
    code.update_and_render = (game_update_and_render *) dlsym(code.game_code_dll, "GameUpdateAndRender");
  }

  return code;
}

internal void
SDL_unload_game_code(game_code *code)
{
  if (code->game_code_dll)
  {
    dlclose(code->game_code_dll);
    code->game_code_dll = 0;
    code->update_and_render = 0;
  }
}

int32
main(int32 argc, char *arg[])
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

      sdl_offscreen_buffer buffer = {};

      game_offscreen_buffer game_buffer = {};
      buffer.game_buffer = &game_buffer;

      char *code_filename = "/Users/partkyle/sdl/build/sdl_platform.app/Contents/MacOS/game.so";
      game_code code = SDL_load_game_code(code_filename);

      if(renderer)
      {
        bool32 Running = true;
        sdl_window_dimension dimension = SDL_get_window_dimension(window);
        SDL_resize_texture(&buffer, renderer, dimension.width, dimension.height);

        while(Running)
        {
          Running = SDL_process_pending_messages();

          SDL_unload_game_code(&code);
          code = SDL_load_game_code(code_filename);

          if(code.update_and_render)
          {
            code.update_and_render(buffer.game_buffer);
          }

          SDL_update_window(window, renderer, &buffer);
        }
      }
    }
    // clean up
    return 0;
}
