#include "pre_inc.h"
#include "kfx/platform/PlatformWindows.h"
#include <SDL3/SDL.h>
#include <cstdlib>
#include "post_inc.h"

bool PlatformWindows::VideoInit()
{
    // SDL disables the screensaver by default, which can disrupt the HDR
    // compositor; re-allow it before initialising video.
    SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");
    if (!SDL_Init(SDL_INIT_VIDEO))
        return false;
    atexit(SDL_Quit);
    return true;
}
