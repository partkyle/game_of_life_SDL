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

  internal void
  platform_unload_game_code(game_code *code)
  {
    if (code->game_code_dll)
    {
      FreeLibrary((HMODULE)code->game_code_dll);
      code->game_code_dll = 0;
      code->game_code_dll = 0;
      code->update_and_render = 0;
    }
  }

  internal game_code
  platform_load_game_code(char *filename)
  {
    game_code code = {};

    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if(!GetFileAttributesEx("w:\\sdl_platform\\build\\gamedll.lock", GetFileExInfoStandard, &Ignored))
    {
      CopyFile("w:\\sdl_platform\\build\\game.dll", "w:\\sdl_platform\\build\\game_running.dll", FALSE);
      code.game_code_dll = LoadLibraryA("w:\\sdl_platform\\build\\game_running.dll");
      if(code.game_code_dll)
      {
        code.update_and_render = (game_update_and_render *)
            GetProcAddress((HMODULE)code.game_code_dll, "GameUpdateAndRender");
      }
    }

    return code;
  }
#endif
