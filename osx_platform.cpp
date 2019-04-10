#include <sys/mman.h>
#define MAP_ANONYMOUS MAP_ANON

// dynamic loading
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <strings.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

internal void
platform_unload_game_code(game_code *code)
{
    if (code->game_code_dll)
    {
        dlclose(code->game_code_dll);
        code->game_code_dll = 0;
        code->update_and_render = 0;
    }
}

internal platform_dynamic_game
platform_dynamic_game_load(const char *dll_filename)
{
    platform_dynamic_game game = {};

    char module_filename[MAX_PATH_LENGTH];
    uint32 size = MAX_PATH_LENGTH;
    _NSGetExecutablePath(module_filename, &size);

    strcpy(game.dll_filename, module_filename);
    // find one index past the last slash
    char *last_slash = strrchr(game.dll_filename, '/');
    ++last_slash;

    // add the filename from the slash
    strcpy(last_slash, dll_filename);

    strcat(game.dll_filename, ".so");

    strcpy(game.lock_filename, game.dll_filename);
    strcat(game.lock_filename, ".lock");

    return(game);
}

internal void
platform_load_game_code(platform_dynamic_game *game, game_code *code)
{
    // don't load if the lockfile exists
    struct stat ignored;
    if(stat(game->lock_filename, &ignored) < 0)
    {
        struct stat dllstat;
        if(stat(game->dll_filename, &dllstat) >= 0)
        {
            if(code->DLL_last_write_time < dllstat.st_mtime)
            {
                platform_unload_game_code(code);
                code->DLL_last_write_time = dllstat.st_mtime;
                code->game_code_dll = dlopen(game->dll_filename, RTLD_LAZY|RTLD_GLOBAL);
                if(code->game_code_dll)
                {
                    code->update_and_render = (game_update_and_render *) dlsym(code->game_code_dll, "GameUpdateAndRender");
                }
            }
        }
    }
}
