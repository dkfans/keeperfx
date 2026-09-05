#ifndef RENDERER_OPENGL_GLFUNCTIONS_H
#define RENDERER_OPENGL_GLFUNCTIONS_H

// Post-1.1 GL entry points, loaded at runtime via SDL_GL_GetProcAddress since
// opengl32.dll/libGL.so only statically export GL 1.1. GL 1.1 functions
// (glGenTextures, glTexImage2D, glClear, ...) need no loader and are used
// directly from <SDL3/SDL_opengl.h>.
//
// A real glad build is deferred until this hand-rolled list stops being the
// simplest option (see the P5.0 deviation note) -- this is exactly the small,
// known, finite set P5.4/P5.5 actually call.

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

// glActiveTexture (GL 1.3): SDL_opengl.h declares it as a real prototype, but
// opengl32.dll's mingw import lib doesn't actually export it (Windows only
// ships GL 1.1 statically) -- link fails without this. Renamed + macroed
// rather than declared as `glActiveTexture` directly, since that would
// collide with SDL's function declaration (variable vs. function).
extern PFNGLACTIVETEXTUREPROC glActiveTexture_ptr;
#define glActiveTexture glActiveTexture_ptr

// glTexImage3D / glTexSubImage3D (GL 1.2): same story as glActiveTexture --
// post-1.1, not in opengl32.dll's static export set on Windows. Needed by
// GLTileAtlas's GL_TEXTURE_2D_ARRAY upload path (P5.7.1).
extern PFNGLTEXIMAGE3DPROC    glTexImage3D_ptr;
#define glTexImage3D glTexImage3D_ptr
extern PFNGLTEXSUBIMAGE3DPROC glTexSubImage3D_ptr;
#define glTexSubImage3D glTexSubImage3D_ptr

// glVertexAttribIPointer (GL 3.0), glVertexAttribDivisor / glDrawArraysInstanced
// (GL 3.3): same story again -- needed by GLWorldViewRenderer's instanced
// keeper-sprite path (P5.7.3a).
extern PFNGLVERTEXATTRIBIPOINTERPROC glVertexAttribIPointer_ptr;
#define glVertexAttribIPointer glVertexAttribIPointer_ptr
extern PFNGLVERTEXATTRIBDIVISORPROC  glVertexAttribDivisor_ptr;
#define glVertexAttribDivisor glVertexAttribDivisor_ptr
extern PFNGLDRAWARRAYSINSTANCEDPROC  glDrawArraysInstanced_ptr;
#define glDrawArraysInstanced glDrawArraysInstanced_ptr

// Framebuffer objects (GL 3.0), needed by GLWorldViewRenderer's possession-
// lens scene capture (P5.8a) -- the first FBO use in this branch's GL
// backend, everything before this drew straight to the default framebuffer.
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
// glBlitFramebuffer (GL 3.0), needed by GLMapFadePass::CaptureWorldFrame()
// (P5.8b) to copy the default framebuffer into a texture without a
// redundant re-render.
extern PFNGLBLITFRAMEBUFFERPROC        glBlitFramebuffer_ptr;
#define glBlitFramebuffer glBlitFramebuffer_ptr

// GL_KHR_debug (core since GL 4.3, widely available as an extension on 3.3
// drivers too). Loaded but NOT required for GLFunctions_Load() to succeed --
// unlike everything above, a driver without it should degrade to "no debug
// output," not refuse to run the renderer. Callers must null-check before use.
extern PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback_ptr;
#define glDebugMessageCallback glDebugMessageCallback_ptr
extern PFNGLDEBUGMESSAGECONTROLPROC  glDebugMessageControl_ptr;
#define glDebugMessageControl glDebugMessageControl_ptr

/** Resolves every pointer above via SDL_GL_GetProcAddress. Call once after
 *  the GL context is current. Returns false if any entry point is missing. */
bool GLFunctions_Load();

#endif // RENDERER_OPENGL_GLFUNCTIONS_H
