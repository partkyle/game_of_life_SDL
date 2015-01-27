#include "game.h"

#ifdef __APPLE__
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

  internal void
  osx_relative_path(char *buffer, uint32 size, char *endfile)
  {
    // TODO(partkyle) is this the only way to do this?
    _NSGetExecutablePath(buffer, &size);

    // find one index past the last slash
    char *last_slash = strrchr(buffer, '/');
    ++last_slash;

    // add the filename from the slash
    strcpy(last_slash, endfile);
  }

  #define MAX_PATH 1024

  internal game_code
  platform_load_game_code(char *filename)
  {
    game_code code = {};

    // TODO(partkyle): make this less of a hack
    // relative path lookup
    char dylib_filename[MAX_PATH];
    uint32 size = MAX_PATH;

    char fileend[10];
    strcpy(fileend, filename);
    strcat(fileend, ".so");

    osx_relative_path(dylib_filename, MAX_PATH, fileend);

    char lockfile[MAX_PATH];
    strcpy(lockfile, dylib_filename);
    strcat(lockfile, ".lock");


    // don't load if the lockfile exists
    struct stat ignored;
    if(stat(lockfile, &ignored) < 0)
    {
      struct stat dllstat;
      if(stat(dylib_filename, &dllstat) >= 0)
      {
        if(code.DLL_last_write_time < dllstat.st_mtime)
        {
          code.DLL_last_write_time = dllstat.st_mtime;
          code.game_code_dll = dlopen(dylib_filename, RTLD_LAZY|RTLD_GLOBAL);
          if(code.game_code_dll)
          {
            code.update_and_render = (game_update_and_render *) dlsym(code.game_code_dll, "GameUpdateAndRender");
          }
        }
      }
    }

    return code;
  }
#endif

#ifdef _WIN32
  #include "windows.h"
  #include <string.h>

  internal platform_dynamic_game
  platform_dynamic_game_load(char *dll_filename)
  {
    platform_dynamic_game game = {};

    char module_filename[MAX_PATH_LENGTH] = {};
    GetModuleFileNameA(0, module_filename, array_count(module_filename));

    strcpy(game.dll_filename, module_filename);
    // find one index past the last slash
    char *last_slash = strrchr(game.dll_filename, '\\');
    ++last_slash;

    // add the filename from the slash
    strcpy(last_slash, dll_filename);

    strcpy(game.tmp_dll_filename, game.dll_filename);
    strcat(game.tmp_dll_filename, "_tmp.dll");

    strcat(game.dll_filename, ".dll");

    strcpy(game.lock_filename, game.dll_filename);
    strcat(game.lock_filename, ".lock");

    return game;
  }

  inline uint64
  win32_get_last_write_time(char *filename)
  {
    uint64 result = 0;

    WIN32_FILE_ATTRIBUTE_DATA data;
    if(GetFileAttributesExA(filename, GetFileExInfoStandard, &data))
    {
      result = ((uint64)data.ftLastWriteTime.dwLowDateTime << 32) | ((uint64)data.ftLastWriteTime.dwLowDateTime);
    }

    return result;
  }

  internal void
  platform_unload_game_code(game_code *code)
  {
    if (code->game_code_dll)
    {
      FreeLibrary((HMODULE)code->game_code_dll);
      code->game_code_dll = 0;
      code->update_and_render = 0;
    }
  }

  internal void
  platform_load_game_code(platform_dynamic_game *game, game_code *code)
  {
    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx(game->lock_filename, GetFileExInfoStandard, &Ignored))
    {
      uint64 time_updated = win32_get_last_write_time(game->dll_filename);
      if(code->DLL_last_write_time < time_updated)
      {
        platform_unload_game_code(code);

        code->DLL_last_write_time = time_updated;
        CopyFile(game->dll_filename, game->tmp_dll_filename, false);
        code->game_code_dll = LoadLibraryA(game->tmp_dll_filename);
        if(code->game_code_dll)
        {
          code->update_and_render = (game_update_and_render *)
              GetProcAddress((HMODULE)code->game_code_dll, "GameUpdateAndRender");
        }
      }
    }
  }

#endif
