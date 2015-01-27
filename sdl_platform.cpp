#include "platform.cpp"

#include "types.h"
#include "sdl_platform.h"

internal void
SDL_process_keyboard_control(game_button_state *new_state, bool32 is_down)
{
  if(new_state->EndedDown != is_down)
  {
      new_state->EndedDown = is_down;
      ++new_state->HalfTransitionCount;
  }
}

internal void
SDL_process_keyboard_message(SDL_Keysym keysym, game_controller_input *controller, bool32 is_down)
{
    switch(keysym.sym)
    {
        case SDLK_w:
        {
            SDL_process_keyboard_control(&controller->MoveUp, is_down);
        } break;

        case SDLK_a:
        {
            SDL_process_keyboard_control(&controller->MoveLeft, is_down);
        } break;

        case SDLK_s:
        {
            SDL_process_keyboard_control(&controller->MoveDown, is_down);
        } break;

        case SDLK_d:
        {
            SDL_process_keyboard_control(&controller->MoveRight, is_down);
        } break;
    }
}

internal bool32
handle_event(SDL_Event *event, game_input *input)
{
    bool32 should_quit;
    switch(event->type)
    {
        case SDL_QUIT:
        {
            should_quit = true;
        } break;

        case SDL_KEYDOWN:
        {
            SDL_process_keyboard_message(event->key.keysym, &input->Controllers[0], true);
        } break;

        case SDL_KEYUP:
        {
            SDL_process_keyboard_message(event->key.keysym, &input->Controllers[0], false);
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
SDL_process_pending_messages(game_input *input)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        if (handle_event(&event, input))
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
        free(buffer->game_buffer->memory);
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
    buffer->game_buffer->memory = malloc(
                          width * height * bytes_per_pixel
                          // PROT_READ | PROT_WRITE,
                          // MAP_PRIVATE | MAP_ANONYMOUS,
                          // -1,
                          // 0);
                          );
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
  return platform_load_game_code(filename);
}

internal void
SDL_unload_game_code(game_code *code)
{
  platform_unload_game_code(code);
}

int
main(int argc, char *arg[])
{
    // init SDL
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
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

      char *code_filename = "/Users/partkyle/sdl/build/game.so";
      game_code code = SDL_load_game_code(code_filename);

      if(renderer)
      {
        bool32 Running = true;
        sdl_window_dimension dimension = SDL_get_window_dimension(window);
        SDL_resize_texture(&buffer, renderer, dimension.width, dimension.height);

        game_input input = {};

        game_memory memory = {};
        memory.permanent_storage_size = 64 * 1024 * 1024;
        memory.transient_storage_size = 1 * 1024 * 1024 * 1024;
        void *memory_block = malloc(memory.permanent_storage_size + memory.transient_storage_size);

        memory.permanent_storage = memory_block;
        memory.transient_storage = ((uint8 *)memory.permanent_storage +
                                           memory.permanent_storage_size);

        while(Running)
        {
          Running = SDL_process_pending_messages(&input);

          SDL_unload_game_code(&code);
          code = SDL_load_game_code(code_filename);

          if(code.update_and_render)
          {
            code.update_and_render(buffer.game_buffer, &memory, &input);
          }

          SDL_update_window(window, renderer, &buffer);
        }
      }
    }

    return 0;
}
