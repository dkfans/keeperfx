#ifndef RENDERER_OPENGL_GLFUNCTIONS_H
#define RENDERER_OPENGL_GLFUNCTIONS_H

#include <glad/glad.h>

/** Resolves every GL entry point via SDL_GL_GetProcAddress. Call once after
 *  the GL context is current. Returns false if any entry point is missing. */
bool GLFunctions_Load();

#endif // RENDERER_OPENGL_GLFUNCTIONS_H
