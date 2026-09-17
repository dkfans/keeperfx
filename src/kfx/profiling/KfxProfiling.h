/******************************************************************************/
/** @file KfxProfiling.h
 *   Tracy CPU/GPU zones and GL debug labels for C++ files.
 */
/******************************************************************************/
#pragma once

#define KFX_COLOR_SIMULATION   0x2266FFu
#define KFX_COLOR_RENDER_CPU   0xFF8C00u
#define KFX_COLOR_RENDER_GPU   0xFF3344u
#define KFX_COLOR_CREATURE     0x44BB44u
#define KFX_COLOR_PATHFINDING  0xAA44CCu
#define KFX_COLOR_AI           0xDDCC00u
#define KFX_COLOR_SCRIPT       0x00BBCCu
#define KFX_COLOR_LIGHTING     0xFFEE66u

#ifdef TRACY_ENABLE
#  include <tracy/Tracy.hpp>
#  define KFX_ZONE(name)                        ZoneScopedN(name)
#  define KFX_ZONE_COLOR(name, color)           ZoneScopedNC(name, color)
#  define KFX_ZONE_VALUE(value)                 ZoneValue(value)
#  define KFX_PLOT(name, val)                   TracyPlot(name, static_cast<int64_t>(val))
#  define KFX_PLOT_CONFIG(name,type,step,fill,color) TracyPlotConfig(name, type, step, fill, color)
#  define KFX_FRAMEMARK()                       FrameMark
#  define KFX_ALLOC_NAMED(ptr, size, name)      TracyAllocN(ptr, size, name)
#  define KFX_FREE_NAMED(ptr, name)             TracyFreeN(ptr, name)
#else
#  define KFX_ZONE(name)
#  define KFX_ZONE_COLOR(name, color)
#  define KFX_ZONE_VALUE(value)
#  define KFX_PLOT(name, val)
#  define KFX_PLOT_CONFIG(name,type,step,fill,color)
#  define KFX_FRAMEMARK()
#  define KFX_ALLOC_NAMED(ptr, size, name)
#  define KFX_FREE_NAMED(ptr, name)
#endif

// glad must precede TracyOpenGL.hpp
#ifdef TRACY_ENABLE
#  include <glad/glad.h>
#  include <cstdio>
#endif

// GPU zones and collection are no-ops until the GPU context exists
#ifdef TRACY_ENABLE
#  include <tracy/TracyOpenGL.hpp>
#  define KFX_GPU_CTX_CREATE()  TracyGpuContext
#  define KFX_GPU_COLLECT() \
    do { if (tracy::GetGpuCtx().ptr) { TracyGpuCollect; } } while(0)
#  define KFX_GPU_ZONE(name) \
    TracyGpuNamedZone(___tracy_gpu_zone, name, tracy::GetGpuCtx().ptr != nullptr)
#else
#  define KFX_GPU_ZONE(name)
#  define KFX_GPU_CTX_CREATE()
#  define KFX_GPU_COLLECT()
#endif

#ifdef TRACY_ENABLE

namespace kfx_gl_debug {
    inline void push_group(const char* name) {
#ifdef GL_KHR_debug
        if (GLAD_GL_KHR_debug) glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name);
#endif
    }
    inline void pop_group() {
#ifdef GL_KHR_debug
        if (GLAD_GL_KHR_debug) glPopDebugGroup();
#endif
    }
    inline void label_obj(GLenum type, GLuint id, const char* name) {
#ifdef GL_KHR_debug
        if (GLAD_GL_KHR_debug) glObjectLabel(type, id, -1, name);
#endif
    }
    inline void label_obj_fmt(GLenum type, GLuint id, const char* fmt, int idx) {
#ifdef GL_KHR_debug
        if (GLAD_GL_KHR_debug) {
            char buf[64];
            snprintf(buf, sizeof(buf), fmt, idx);
            glObjectLabel(type, id, -1, buf);
        }
#endif
    }
} // namespace kfx_gl_debug

#  define KFX_GL_PUSH(name)                kfx_gl_debug::push_group(name)
#  define KFX_GL_POP()                     kfx_gl_debug::pop_group()
#  define KFX_GL_LABEL(type, id, name)     kfx_gl_debug::label_obj(type, id, name)
#  define KFX_GL_LABEL_FMT(type, id, f, n) kfx_gl_debug::label_obj_fmt(type, id, f, n)

struct KfxGLDebugScope {
    explicit KfxGLDebugScope(const char* n) { kfx_gl_debug::push_group(n); }
    ~KfxGLDebugScope()                      { kfx_gl_debug::pop_group(); }
    KfxGLDebugScope(const KfxGLDebugScope&)            = delete;
    KfxGLDebugScope& operator=(const KfxGLDebugScope&) = delete;
};
#  define KFX_GL_SCOPE(var, name) KfxGLDebugScope var(name)

#else

#  define KFX_GL_PUSH(name)
#  define KFX_GL_POP()
#  define KFX_GL_LABEL(type, id, name)
#  define KFX_GL_LABEL_FMT(type, id, f, n)
#  define KFX_GL_SCOPE(var, name)

#endif
