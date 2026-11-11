#include "pre_inc.h"
#include "kfx/renderer/opengl/GLFunctions.h"
#include "bflib_basics.h"
#include "globals.h" // ERRORLOG
#include "post_inc.h"

bool GLFunctions_Load(void* (*proc_loader)(const char* name))
{
    if (proc_loader == nullptr || !gladLoadGLLoader((GLADloadproc)proc_loader))
    {
        ERRORLOG("GLFunctions_Load: gladLoadGLLoader failed");
        return false;
    }
    return true;
}
