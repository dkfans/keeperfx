/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file GLContextSDL.cpp
 *     SDL3 OpenGL context for the game window.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/platform/GLContextSDL.h"
#include "bflib_basics.h"
#include "globals.h"
#include <SDL3/SDL.h>
#include "post_inc.h"

void GLContextSDL::RequestWindowAttributes()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    // World geometry depth-tests tile/flat-poly draws for correct
    // near/far ordering -- request a depth buffer explicitly rather than
    // relying on a driver default, which may be none.
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,   8);
}

std::unique_ptr<GLContextSDL> GLContextSDL::Create(SDL_Window* window)
{
    if (window == nullptr || !(SDL_GetWindowFlags(window) & SDL_WINDOW_OPENGL))
    {
        ERRORLOG("GLContextSDL::Create: window was not created with SDL_WINDOW_OPENGL");
        return nullptr;
    }
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (context == nullptr)
    {
        ERRORLOG("GLContextSDL::Create: SDL_GL_CreateContext failed: %s", SDL_GetError());
        return nullptr;
    }

    int floatbuf = 0;
    SDL_GL_GetAttribute(SDL_GL_FLOATBUFFERS, &floatbuf);
    if (floatbuf != 0)
    {
        // TODO : port scrb from dev
    }

    return std::unique_ptr<GLContextSDL>(new GLContextSDL(window, context));
}

GLContextSDL::GLContextSDL(SDL_Window* window, SDL_GLContextState* context)
    : m_window(window), m_context(context)
{
}

GLContextSDL::~GLContextSDL()
{
    if (m_context != nullptr)
        SDL_GL_DestroyContext(m_context);
}

bool GLContextSDL::MakeCurrent()
{
    if (!SDL_GL_MakeCurrent(m_window, m_context))
    {
        ERRORLOG("GLContextSDL::MakeCurrent: %s", SDL_GetError());
        return false;
    }
    return true;
}

bool GLContextSDL::ReleaseCurrent()
{
    if (!SDL_GL_MakeCurrent(m_window, nullptr))
    {
        ERRORLOG("GLContextSDL::ReleaseCurrent: %s", SDL_GetError());
        return false;
    }
    return true;
}

void GLContextSDL::SwapBuffers()
{
    SDL_GL_SwapWindow(m_window);
}

bool GLContextSDL::SetSwapInterval(int interval)
{
    if (!SDL_GL_SetSwapInterval(interval))
    {
        WARNLOG("GLContextSDL::SetSwapInterval(%d): %s", interval, SDL_GetError());
        return false;
    }
    return true;
}

static void* sdl_gl_proc_loader(const char* name)
{
    return (void*)SDL_GL_GetProcAddress(name);
}

IGLContext::ProcLoader GLContextSDL::GetProcLoader() const
{
    return sdl_gl_proc_loader;
}
