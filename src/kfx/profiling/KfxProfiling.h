/******************************************************************************/
/** @file KfxProfiling.h
 *   Tracy CPU/GPU profiling and GL debug label helpers — C++ renderer files only.
 *
 *   Include this after pre_inc.h in any renderer .cpp file that wants zones.
 *   Compiles to zero overhead when TRACY_ENABLE is not defined, so it is safe
 *   to include unconditionally.
 */
/******************************************************************************/
#pragma once

// ── Subsystem colour palette (0xRRGGBB, Tracy uses uint32_t) ────────────────
#define KFX_COLOR_SIMULATION   0x2266FFu  // Blue
#define KFX_COLOR_RENDER_CPU   0xFF8C00u  // Orange
#define KFX_COLOR_RENDER_GPU   0xFF3344u  // Red
#define KFX_COLOR_CREATURE     0x44BB44u  // Green
#define KFX_COLOR_PATHFINDING  0xAA44CCu  // Purple
#define KFX_COLOR_AI           0xDDCC00u  // Yellow
#define KFX_COLOR_SCRIPT       0x00BBCCu  // Cyan
#define KFX_COLOR_LIGHTING     0xFFEE66u  // Light Yellow

// ── Tracy CPU zones ──────────────────────────────────────────────────────────
#ifdef TRACY_ENABLE
#  include <tracy/Tracy.hpp>
#  define KFX_ZONE(name)                        ZoneScopedN(name)
#  define KFX_ZONE_COLOR(name, color)           ZoneScopedNC(name, color)
// Attach a numeric value to the enclosing KFX_ZONE*/KFX_ZONE_COLOR in this scope.
#  define KFX_ZONE_VALUE(value)                 ZoneValue(value)
#  define KFX_PLOT(name, val)                   TracyPlot(name, static_cast<int64_t>(val))
#  define KFX_PLOT_CONFIG(name,type,step,fill,color) TracyPlotConfig(name, type, step, fill, color)
#  define KFX_FRAMEMARK()                       FrameMark
// Named memory pool tracking for custom (non-malloc) allocators -- see the C
// KFX_C_ALLOC_NAMED/KFX_C_FREE_NAMED equivalent for the rationale.
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

// The GL renderer backend is always compiled in this tree (no feature macro
// gate), so the GPU-zone / debug-label helpers below only need to check
// TRACY_ENABLE.

// glad must be included before TracyOpenGL.hpp since Tracy's GL header uses
// GL types and entry-points directly without pulling them in itself.
#ifdef TRACY_ENABLE
#  include <glad/glad.h>
#  include <cstdio>
#endif

// ── Tracy GPU zones (OpenGL backend) ─────────────────────────────────────────
// Caller is responsible for TracyGpuContext (init) and TracyGpuCollect (per-frame).
// KFX_GPU_COLLECT and KFX_GPU_ZONE guard against a context that was never
// created, so they stay safe even if the Tracy profiler connects later at runtime.
#ifdef TRACY_ENABLE
#  include <tracy/TracyOpenGL.hpp>
#  define KFX_GPU_CTX_CREATE()  TracyGpuContext
// Only collect when the GPU context was actually created (ptr != nullptr).
#  define KFX_GPU_COLLECT() \
    do { if (tracy::GetGpuCtx().ptr) { TracyGpuCollect; } } while(0)
// Pass ptr-liveness as the 'active' flag so GpuCtxScope::m_active is false
// (and no glQueryCounter is issued) when the GPU context was never created.
#  define KFX_GPU_ZONE(name) \
    TracyGpuNamedZone(___tracy_gpu_zone, name, tracy::GetGpuCtx().ptr != nullptr)
#else
#  define KFX_GPU_ZONE(name)
#  define KFX_GPU_CTX_CREATE()
#  define KFX_GPU_COLLECT()
#endif

// ── GL debug labels & groups (GL_KHR_debug) ──────────────────────────────────
// glObjectLabel / glPushDebugGroup are GL_KHR_debug (core in 4.3, extension in
// 3.3).  All calls are guarded at runtime by GLAD_GL_KHR_debug.  Only compiled
// in when Tracy is enabled -- these are a profiling/RenderDoc aid, not needed
// for normal builds.
#ifdef TRACY_ENABLE
// glad and cstdio already included above

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

/// RAII debug group scope — handles early-return paths correctly.
struct KfxGLDebugScope {
    explicit KfxGLDebugScope(const char* n) { kfx_gl_debug::push_group(n); }
    ~KfxGLDebugScope()                      { kfx_gl_debug::pop_group(); }
    KfxGLDebugScope(const KfxGLDebugScope&)            = delete;
    KfxGLDebugScope& operator=(const KfxGLDebugScope&) = delete;
};
/// Declare a scoped debug group — prefer over explicit push/pop when possible.
#  define KFX_GL_SCOPE(var, name) KfxGLDebugScope var(name)

#else  // !TRACY_ENABLE

#  define KFX_GL_PUSH(name)
#  define KFX_GL_POP()
#  define KFX_GL_LABEL(type, id, name)
#  define KFX_GL_LABEL_FMT(type, id, f, n)
#  define KFX_GL_SCOPE(var, name)

#endif // TRACY_ENABLE
