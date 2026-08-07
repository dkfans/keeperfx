#include "pre_inc.h"
#include "kfx/platform/PlatformLinux.h"
#include <SDL3/SDL.h>
#include <cstdlib>
#include "post_inc.h"

bool PlatformLinux::VideoInit()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;
    atexit(SDL_Quit);
    return true;
}
