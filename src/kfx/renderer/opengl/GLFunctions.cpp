#include "pre_inc.h"
#include "kfx/renderer/opengl/GLFunctions.h"
#include "bflib_basics.h"
#include "globals.h" // ERRORLOG
#include <SDL3/SDL.h>
#include "post_inc.h"

PFNGLCREATESHADERPROC       glCreateShader       = nullptr;
PFNGLSHADERSOURCEPROC       glShaderSource       = nullptr;
PFNGLCOMPILESHADERPROC      glCompileShader      = nullptr;
PFNGLGETSHADERIVPROC        glGetShaderiv        = nullptr;
PFNGLGETSHADERINFOLOGPROC   glGetShaderInfoLog   = nullptr;
PFNGLDELETESHADERPROC       glDeleteShader       = nullptr;
PFNGLCREATEPROGRAMPROC      glCreateProgram      = nullptr;
PFNGLATTACHSHADERPROC       glAttachShader       = nullptr;
PFNGLLINKPROGRAMPROC        glLinkProgram        = nullptr;
PFNGLGETPROGRAMIVPROC       glGetProgramiv       = nullptr;
PFNGLGETPROGRAMINFOLOGPROC  glGetProgramInfoLog  = nullptr;
PFNGLDELETEPROGRAMPROC      glDeleteProgram      = nullptr;
PFNGLUSEPROGRAMPROC         glUseProgram         = nullptr;

PFNGLGENVERTEXARRAYSPROC    glGenVertexArrays    = nullptr;
PFNGLBINDVERTEXARRAYPROC    glBindVertexArray    = nullptr;
PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = nullptr;
PFNGLGENBUFFERSPROC         glGenBuffers         = nullptr;
PFNGLBINDBUFFERPROC         glBindBuffer         = nullptr;
PFNGLBUFFERDATAPROC         glBufferData         = nullptr;
PFNGLBUFFERSUBDATAPROC      glBufferSubData      = nullptr;
PFNGLDELETEBUFFERSPROC      glDeleteBuffers      = nullptr;
PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer     = nullptr;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = nullptr;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = nullptr;

PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = nullptr;
PFNGLUNIFORM1IPROC          glUniform1i          = nullptr;
PFNGLUNIFORM1FPROC          glUniform1f          = nullptr;
PFNGLUNIFORM2FPROC          glUniform2f          = nullptr;
PFNGLUNIFORM4FPROC          glUniform4f          = nullptr;
PFNGLACTIVETEXTUREPROC      glActiveTexture_ptr  = nullptr;
PFNGLTEXIMAGE3DPROC         glTexImage3D_ptr     = nullptr;
PFNGLTEXSUBIMAGE3DPROC      glTexSubImage3D_ptr  = nullptr;
PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer_ptr = nullptr;
PFNGLVERTEXATTRIBDIVISORPROC  glVertexAttribDivisor_ptr  = nullptr;
PFNGLDRAWARRAYSINSTANCEDPROC  glDrawArraysInstanced_ptr  = nullptr;

PFNGLGENFRAMEBUFFERSPROC        glGenFramebuffers_ptr        = nullptr;
PFNGLBINDFRAMEBUFFERPROC        glBindFramebuffer_ptr        = nullptr;
PFNGLFRAMEBUFFERTEXTURE2DPROC   glFramebufferTexture2D_ptr   = nullptr;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_ptr = nullptr;
PFNGLDELETEFRAMEBUFFERSPROC     glDeleteFramebuffers_ptr     = nullptr;
PFNGLGENRENDERBUFFERSPROC       glGenRenderbuffers_ptr       = nullptr;
PFNGLBINDRENDERBUFFERPROC       glBindRenderbuffer_ptr       = nullptr;
PFNGLRENDERBUFFERSTORAGEPROC    glRenderbufferStorage_ptr    = nullptr;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer_ptr = nullptr;
PFNGLDELETERENDERBUFFERSPROC    glDeleteRenderbuffers_ptr    = nullptr;
PFNGLBLITFRAMEBUFFERPROC        glBlitFramebuffer_ptr        = nullptr;
PFNGLDRAWBUFFERSPROC            glDrawBuffers_ptr            = nullptr;

PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_ptr = nullptr;
PFNGLDEBUGMESSAGECONTROLPROC  glDebugMessageControl_ptr  = nullptr;

namespace {
template<typename T>
bool load(T& fn, const char* name)
{
    // ToDo: Ideally, this should not need SDL, but SDL_GL_GetProcAddress is the only cross-platform way to get OpenGL function pointers that I know of and we're using SDL for OpenGL context management right now anyway..
    fn = (T)SDL_GL_GetProcAddress(name);
    if (!fn) ERRORLOG("GLFunctions_Load: missing entry point %s", name);
    return fn != nullptr;
}
} // namespace

bool GLFunctions_Load()
{
    bool ok = true;
    ok &= load(glCreateShader, "glCreateShader");
    ok &= load(glShaderSource, "glShaderSource");
    ok &= load(glCompileShader, "glCompileShader");
    ok &= load(glGetShaderiv, "glGetShaderiv");
    ok &= load(glGetShaderInfoLog, "glGetShaderInfoLog");
    ok &= load(glDeleteShader, "glDeleteShader");
    ok &= load(glCreateProgram, "glCreateProgram");
    ok &= load(glAttachShader, "glAttachShader");
    ok &= load(glLinkProgram, "glLinkProgram");
    ok &= load(glGetProgramiv, "glGetProgramiv");
    ok &= load(glGetProgramInfoLog, "glGetProgramInfoLog");
    ok &= load(glDeleteProgram, "glDeleteProgram");
    ok &= load(glUseProgram, "glUseProgram");

    ok &= load(glGenVertexArrays, "glGenVertexArrays");
    ok &= load(glBindVertexArray, "glBindVertexArray");
    ok &= load(glDeleteVertexArrays, "glDeleteVertexArrays");
    ok &= load(glGenBuffers, "glGenBuffers");
    ok &= load(glBindBuffer, "glBindBuffer");
    ok &= load(glBufferData, "glBufferData");
    ok &= load(glBufferSubData, "glBufferSubData");
    ok &= load(glDeleteBuffers, "glDeleteBuffers");
    ok &= load(glVertexAttribPointer, "glVertexAttribPointer");
    ok &= load(glEnableVertexAttribArray, "glEnableVertexAttribArray");
    ok &= load(glDisableVertexAttribArray, "glDisableVertexAttribArray");

    ok &= load(glGetUniformLocation, "glGetUniformLocation");
    ok &= load(glUniform1i, "glUniform1i");
    ok &= load(glUniform1f, "glUniform1f");
    ok &= load(glUniform2f, "glUniform2f");
    ok &= load(glUniform4f, "glUniform4f");
    ok &= load(glActiveTexture_ptr, "glActiveTexture");
    ok &= load(glTexImage3D_ptr, "glTexImage3D");
    ok &= load(glTexSubImage3D_ptr, "glTexSubImage3D");
    ok &= load(glVertexAttribIPointer_ptr, "glVertexAttribIPointer");
    ok &= load(glVertexAttribDivisor_ptr, "glVertexAttribDivisor");
    ok &= load(glDrawArraysInstanced_ptr, "glDrawArraysInstanced");

    ok &= load(glGenFramebuffers_ptr, "glGenFramebuffers");
    ok &= load(glBindFramebuffer_ptr, "glBindFramebuffer");
    ok &= load(glFramebufferTexture2D_ptr, "glFramebufferTexture2D");
    ok &= load(glCheckFramebufferStatus_ptr, "glCheckFramebufferStatus");
    ok &= load(glDeleteFramebuffers_ptr, "glDeleteFramebuffers");
    ok &= load(glGenRenderbuffers_ptr, "glGenRenderbuffers");
    ok &= load(glBindRenderbuffer_ptr, "glBindRenderbuffer");
    ok &= load(glRenderbufferStorage_ptr, "glRenderbufferStorage");
    ok &= load(glFramebufferRenderbuffer_ptr, "glFramebufferRenderbuffer");
    ok &= load(glDeleteRenderbuffers_ptr, "glDeleteRenderbuffers");
    ok &= load(glBlitFramebuffer_ptr, "glBlitFramebuffer");
    ok &= load(glDrawBuffers_ptr, "glDrawBuffers");

    glDebugMessageCallback_ptr = (PFNGLDEBUGMESSAGECALLBACKPROC)SDL_GL_GetProcAddress("glDebugMessageCallback");
    glDebugMessageControl_ptr  = (PFNGLDEBUGMESSAGECONTROLPROC)SDL_GL_GetProcAddress("glDebugMessageControl");
    return ok;
}
