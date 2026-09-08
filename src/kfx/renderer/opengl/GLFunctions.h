#ifndef RENDERER_OPENGL_GLFUNCTIONS_H
#define RENDERER_OPENGL_GLFUNCTIONS_H

// Theoretically can remove this with glad i believe.
#include <SDL3/SDL_opengl.h>
#include <SDL3/SDL_opengl_glext.h>

extern PFNGLCREATESHADERPROC       glCreateShader;
extern PFNGLSHADERSOURCEPROC       glShaderSource;
extern PFNGLCOMPILESHADERPROC      glCompileShader;
extern PFNGLGETSHADERIVPROC        glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC   glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC       glDeleteShader;
extern PFNGLCREATEPROGRAMPROC      glCreateProgram;
extern PFNGLATTACHSHADERPROC       glAttachShader;
extern PFNGLLINKPROGRAMPROC        glLinkProgram;
extern PFNGLGETPROGRAMIVPROC       glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC  glGetProgramInfoLog;
extern PFNGLDELETEPROGRAMPROC      glDeleteProgram;
extern PFNGLUSEPROGRAMPROC         glUseProgram;

extern PFNGLGENVERTEXARRAYSPROC    glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC    glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
extern PFNGLGENBUFFERSPROC         glGenBuffers;
extern PFNGLBINDBUFFERPROC         glBindBuffer;
extern PFNGLBUFFERDATAPROC         glBufferData;
extern PFNGLBUFFERSUBDATAPROC      glBufferSubData;
extern PFNGLDELETEBUFFERSPROC      glDeleteBuffers;
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;

extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORM1IPROC          glUniform1i;
extern PFNGLUNIFORM1FPROC          glUniform1f;
extern PFNGLUNIFORM2FPROC          glUniform2f;
extern PFNGLUNIFORM4FPROC          glUniform4f;

extern PFNGLACTIVETEXTUREPROC glActiveTexture_ptr;
#define glActiveTexture glActiveTexture_ptr

extern PFNGLTEXIMAGE3DPROC    glTexImage3D_ptr;
#define glTexImage3D glTexImage3D_ptr
extern PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D_ptr;
#define glTexSubImage3D glTexSubImage3D_ptr

extern PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer_ptr;
#define glVertexAttribIPointer glVertexAttribIPointer_ptr
extern PFNGLVERTEXATTRIBDIVISORPROC  glVertexAttribDivisor_ptr;
#define glVertexAttribDivisor glVertexAttribDivisor_ptr
extern PFNGLDRAWARRAYSINSTANCEDPROC  glDrawArraysInstanced_ptr;
#define glDrawArraysInstanced glDrawArraysInstanced_ptr

extern PFNGLGENFRAMEBUFFERSPROC        glGenFramebuffers_ptr;
#define glGenFramebuffers glGenFramebuffers_ptr
extern PFNGLBINDFRAMEBUFFERPROC        glBindFramebuffer_ptr;
#define glBindFramebuffer glBindFramebuffer_ptr
extern PFNGLFRAMEBUFFERTEXTURE2DPROC   glFramebufferTexture2D_ptr;
#define glFramebufferTexture2D glFramebufferTexture2D_ptr
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus_ptr;
#define glCheckFramebufferStatus glCheckFramebufferStatus_ptr
extern PFNGLDELETEFRAMEBUFFERSPROC     glDeleteFramebuffers_ptr;
#define glDeleteFramebuffers glDeleteFramebuffers_ptr
extern PFNGLGENRENDERBUFFERSPROC       glGenRenderbuffers_ptr;
#define glGenRenderbuffers glGenRenderbuffers_ptr
extern PFNGLBINDRENDERBUFFERPROC       glBindRenderbuffer_ptr;
#define glBindRenderbuffer glBindRenderbuffer_ptr
extern PFNGLRENDERBUFFERSTORAGEPROC    glRenderbufferStorage_ptr;
#define glRenderbufferStorage glRenderbufferStorage_ptr
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer_ptr;
#define glFramebufferRenderbuffer glFramebufferRenderbuffer_ptr
extern PFNGLDELETERENDERBUFFERSPROC    glDeleteRenderbuffers_ptr;
#define glDeleteRenderbuffers glDeleteRenderbuffers_ptr
extern PFNGLBLITFRAMEBUFFERPROC        glBlitFramebuffer_ptr;
#define glBlitFramebuffer glBlitFramebuffer_ptr
extern PFNGLDRAWBUFFERSPROC            glDrawBuffers_ptr;
#define glDrawBuffers glDrawBuffers_ptr


extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_ptr;
#define glDebugMessageCallback glDebugMessageCallback_ptr
extern PFNGLDEBUGMESSAGECONTROLPROC  glDebugMessageControl_ptr;
#define glDebugMessageControl glDebugMessageControl_ptr

/** Resolves every pointer above via SDL_GL_GetProcAddress. Call once after
 *  the GL context is current. Returns false if any entry point is missing. */
bool GLFunctions_Load();

#endif // RENDERER_OPENGL_GLFUNCTIONS_H
