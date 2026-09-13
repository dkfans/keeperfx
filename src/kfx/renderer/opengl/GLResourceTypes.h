#ifndef RENDERER_OPENGL_GLRESOURCETYPES_H
#define RENDERER_OPENGL_GLRESOURCETYPES_H

#include "kfx/renderer/opengl/GLFunctions.h"
#include <vector>

/// A realized 2D texture or texture array.
struct GLTexture
{
    GLuint id            = 0;
    GLenum target        = GL_TEXTURE_2D;
    int32_t width        = 0;
    int32_t height       = 0;
    int32_t array_layers = 1;
};

/// A realized framebuffer with N attachments.
struct GLRenderTarget
{
    GLuint fbo = 0;
    std::vector<GLuint> color_attachments;
    GLuint depth_stencil_attachment = 0; ///< Zero means none.
    int32_t width  = 0;
    int32_t height = 0;
};

/// A realized linked program.
struct GLProgram
{
    GLuint id = 0;
};

/// A realized VAO plus VBO, and optionally an EBO.
struct GLGeometryBuffer
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0; ///< Zero means no index buffer.
    size_t vertex_capacity = 0;
    size_t index_capacity  = 0;
};

#endif // RENDERER_OPENGL_GLRESOURCETYPES_H
