#ifndef RENDERER_OPENGL_GLFUNCTIONS_H
#define RENDERER_OPENGL_GLFUNCTIONS_H

#include <glad/glad.h>

/** Resolves every GL entry point through proc_loader. Call once after the GL
 *  context is current. Returns false if any entry point is missing. */
bool GLFunctions_Load(void* (*proc_loader)(const char* name));

#endif // RENDERER_OPENGL_GLFUNCTIONS_H
