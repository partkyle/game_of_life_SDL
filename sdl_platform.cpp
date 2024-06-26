#include "platform.cpp"

#include "common.h"
#include "sdl_platform.h"

// TODO(partkyle): remove these global variables
//     they are only used for fullscreen
global_variable SDL_Window *window;
global_variable bool32 is_fullscreen = false;
global_variable sdl_offscreen_buffer *global_buffer;

#ifndef ICON_LOCATION
#define ICON_LOCATION "data/icon32x32.bmp"
#endif

internal void
SDL_process_keyboard_control(game_button_state *new_state, bool32 is_down)
{
    if(new_state->ended_down != is_down)
    {
        new_state->ended_down = is_down;
        ++new_state->half_transition_count;
    }
}

internal void
SDL_toggle_fullscreen()
{
    is_fullscreen = !is_fullscreen;
    if(is_fullscreen)
    {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else
    {
        SDL_SetWindowFullscreen(window, 0);
    }
}

sdl_window_dimension
SDL_get_window_dimension(SDL_Window *Window)
{
    sdl_window_dimension result;

    SDL_GetWindowSize(Window, &result.width, &result.height);

    return(result);
}

internal void
SDL_process_keyboard_message(SDL_Event *event, game_controller_input *controller, bool32 is_down)
{
    switch(event->key.keysym.sym)
    {
        case SDLK_w:
        {
            SDL_process_keyboard_control(&controller->move_up, is_down);
        } break;

        case SDLK_a:
        {
            SDL_process_keyboard_control(&controller->move_left, is_down);
        } break;

        case SDLK_s:
        {
            SDL_process_keyboard_control(&controller->move_down, is_down);
        } break;

        case SDLK_d:
        {
            SDL_process_keyboard_control(&controller->move_right, is_down);
        } break;

        case SDLK_UP:
        {
            SDL_process_keyboard_control(&controller->action_up, is_down);
        } break;

        case SDLK_DOWN:
        {
            SDL_process_keyboard_control(&controller->action_down, is_down);
        } break;

        case SDLK_LEFT:
        {
            SDL_process_keyboard_control(&controller->action_left, is_down);
        } break;

        case SDLK_RIGHT:
        {
            SDL_process_keyboard_control(&controller->action_right, is_down);
        } break;

        case SDLK_SPACE:
        {
            SDL_process_keyboard_control(&controller->start, is_down);
        } break;

        case SDLK_ESCAPE:
        {
            SDL_process_keyboard_control(&controller->back, is_down);
        } break;

        // TODO(partkyle): find a better way to handle platform vs game input
        //     should there even be a difference?
        case SDLK_f:
        {
            if(is_down && !event->key.repeat)
            {
                SDL_toggle_fullscreen();
            }
        } break;
    }
}

internal bool32
handle_event(SDL_Event *event, game_input *input)
{
    bool32 should_quit = false;
    switch(event->type)
    {
        case SDL_QUIT:
        {
            should_quit = true;
        } break;

        case SDL_KEYDOWN:
        {
            SDL_process_keyboard_message(event, &input->controllers[0], true);
        } break;

        case SDL_KEYUP:
        {
            SDL_process_keyboard_message(event, &input->controllers[0], false);
        } break;

        // TODO(partkyle): this supports double and triple clicks
        //     that could be useful.
        // TODO(partkyle): there is a mouse button issue when clicking the titlebar:
        //     https://bugzilla.libsdl.org/show_bug.cgi?id=2842
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        {
            // NOTE(partkyle): the mouse bug may still exist in OSX, but it seems fixed on windows
            //     there are issues with the mouse getting held down since the mouseUP only worked
            //     within the window as well
            switch(event->button.button)
            {
                case SDL_BUTTON_LEFT:
                {
                    SDL_process_keyboard_control(&input->MouseLeft, event->button.type == SDL_MOUSEBUTTONDOWN);
                } break;

                case SDL_BUTTON_MIDDLE:
                {
                    SDL_process_keyboard_control(&input->MouseMiddle, event->button.type == SDL_MOUSEBUTTONDOWN);
                } break;
            }
        } break;

        case SDL_MOUSEWHEEL:
        {
            input->mouse_z += event->wheel.y;
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
    buffer->game_buffer->bytes_per_pixel = 4;
    buffer->game_buffer->width = width;
    buffer->game_buffer->height = height;
    buffer->game_buffer->pitch = width * buffer->game_buffer->bytes_per_pixel;
    buffer->game_buffer->memory = malloc(
                          width * height * buffer->game_buffer->bytes_per_pixel
                          // PROT_READ | PROT_WRITE,
                          // MAP_PRIVATE | MAP_ANONYMOUS,
                          // -1,
                          // 0);
                          );
}

internal platform_dynamic_game
SDL_dynamic_platform_game(char *dll_filename)
{
      return platform_dynamic_game_load(dll_filename);
}

internal void
SDL_load_game_code(platform_dynamic_game *game, game_code *code)
{
      platform_load_game_code(game, code);
}

internal void
SDL_set_icon()
{
    SDL_Surface *image = SDL_LoadBMP(ICON_LOCATION);

    SDL_SetWindowIcon(window, image);
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
    window = SDL_CreateWindow("@partkyle SDL Platform",   // window title
                                          SDL_WINDOWPOS_CENTERED,     // x position, centered
                                          SDL_WINDOWPOS_CENTERED,     // y position, centered
                                          1920 / 2,                   // width, in pixels
                                          1080 / 2,                   // height, in pixels
                                          SDL_WINDOW_OPENGL           // flags
                                          );

    SDL_set_icon();
    if(window)
    {
        SDL_Renderer *renderer = SDL_CreateRenderer(window,
                                                    -1,
                                                    SDL_RENDERER_PRESENTVSYNC);

        sdl_offscreen_buffer buffer = {};
        global_buffer = &buffer;

        game_offscreen_buffer game_buffer = {};
        buffer.game_buffer = &game_buffer;

        char *code_filename = "game";
        platform_dynamic_game dynamic_game = SDL_dynamic_platform_game(code_filename);
        game_code code = {};
        SDL_load_game_code(&dynamic_game, &code);

        if(renderer)
        {
            bool32 Running = true;
            sdl_window_dimension dimension = SDL_get_window_dimension(window);
            SDL_resize_texture(&buffer, renderer, dimension.width, dimension.height);

            game_input input[2] = {};

            game_input *current_input = &input[0];
            game_input *last_input = &input[1];

            game_memory memory = {};
            memory.permanent_storage_size = MB(256);
            memory.transient_storage_size = GB(1);
            void *memory_block = malloc(memory.permanent_storage_size + memory.transient_storage_size);

            memory.permanent_storage = memory_block;
            memory.transient_storage = ((uint8 *)memory.permanent_storage +
                                        memory.permanent_storage_size);

            while(Running)
            {
                memset(current_input, 0, sizeof(game_input));

                // NOTE(partkyle): copy old input over to handle the appropriate ended_down state
                for(int controller_index = 0;
                    controller_index < array_count(current_input->controllers);
                    ++controller_index)
                {
                    game_controller_input *controller = &current_input->controllers[controller_index];
                    game_controller_input *last_controller = &last_input->controllers[controller_index];
                    for(int button_index = 0;
                        button_index < array_count(controller->buttons);
                        ++button_index)
                    {
                        controller->buttons[button_index].ended_down =
                            last_controller->buttons[button_index].ended_down;
                    }
                }

                // NOTE(partkyle): copy old input over to handle the appropriate ended_down state
                for(int i = 0;
                    i < array_count(current_input->mouse_buttons);
                    ++i)
                {
                    current_input->mouse_buttons[i].ended_down = last_input->mouse_buttons[i].ended_down;
                }

                Running = SDL_process_pending_messages(current_input);

                current_input->t = time(0);
                current_input->dtForFrame = 1000.0f*(current_input->t - last_input->t);

                // NOTE(partkyle): get mouse_rel from previous frame
                current_input->rel_mouse_x = last_input->rel_mouse_x;
                current_input->rel_mouse_y = last_input->rel_mouse_y;
                SDL_GetRelativeMouseState(&current_input->rel_mouse_x, &current_input->rel_mouse_y);

                // NOTE(partkyle): we want the immediate mouse position
                //     It might be possible to copy the last frames position instead
                // NOTE(partkyle): we don't care about the buttons result as that is
                //     handled in the event loop
                SDL_GetMouseState(&current_input->mouse_x, &current_input->mouse_y);

                if(is_fullscreen)
                {
                    sdl_window_dimension dim = SDL_get_window_dimension(window);
                    current_input->mouse_x = current_input->mouse_x * ((real32)global_buffer->game_buffer->width / (real32)dim.width);
                    current_input->mouse_y = current_input->mouse_y * ((real32)global_buffer->game_buffer->height / (real32)dim.height);
                }
                else
                {
                    current_input->mouse_x = current_input->mouse_x;
                    current_input->mouse_y = current_input->mouse_y;
                }

                SDL_load_game_code(&dynamic_game, &code);

                if(code.update_and_render)
                {
                    code.update_and_render(buffer.game_buffer, &memory, current_input);
                }

                SDL_update_window(window, renderer, &buffer);

                std::swap(current_input, last_input);
            }
        }
    }

    return 0;
}
