#include "pre_inc.h"
#include "kfx/renderer/opengl/GLFunctions.h"
#include "bflib_basics.h"
#include "globals.h" // ERRORLOG
#include <SDL3/SDL.h>
#include "post_inc.h"

bool GLFunctions_Load()
{
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        ERRORLOG("GLFunctions_Load: gladLoadGLLoader failed");
        return false;
    }
    return true;
}
