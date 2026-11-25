#ifndef RENDERER_OPENGL_GLSHADERS_H
#define RENDERER_OPENGL_GLSHADERS_H

/**
 * @brief Shader source code for the OpenGL renderer.
 * 
 * I really don't like this.
 * 
 * The eventual plan is the define shaders like this;;
 * 
 * inline constexpr std::string_view UI_VERTEX_SHADER = R"( 
 * foo...) and have a shader manager that can compile and cache them. 
 * This will allow us to have a single source of truth for shaders,
 *  and will also allow us to have a single place to manage shader compilation and caching.
 * 
 * It'll also allow us to implement a VFS and open up the possibility of having shaders be loaded from mod files
 */

#include "kfx/renderer/opengl/shaders/UIShaders.h"
#include "kfx/renderer/opengl/shaders/WorldShaders.h"
#include "kfx/renderer/opengl/shaders/SpriteShaders.h"
#include "kfx/renderer/opengl/shaders/LensShaders.h"
#include "kfx/renderer/opengl/shaders/PassShaders.h"

#endif // RENDERER_OPENGL_GLSHADERS_H
