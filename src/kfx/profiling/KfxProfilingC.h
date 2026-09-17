/******************************************************************************/
/** @file KfxProfilingC.h
 *   Tracy instrumentation helpers — C-compatible (.c files).
 *
 *   Use KFX_C_ZONE_BEGIN / KFX_C_ZONE_END around function bodies.
 *   TracyCZoneN declares a local variable named by ctx, so ctx must be unique
 *   within its scope.  No-ops compile to nothing when TRACY_ENABLE is off.
 */
/******************************************************************************/
#ifndef KFX_PROFILING_C_H
#define KFX_PROFILING_C_H

// ── Subsystem colour palette (match KfxProfiling.h — duplicated for C) ──────
#define KFX_COLOR_SIMULATION   0x2266FFu
#define KFX_COLOR_RENDER_CPU   0xFF8C00u
#define KFX_COLOR_RENDER_GPU   0xFF3344u
#define KFX_COLOR_CREATURE     0x44BB44u
#define KFX_COLOR_PATHFINDING  0xAA44CCu
#define KFX_COLOR_AI           0xDDCC00u
#define KFX_COLOR_SCRIPT       0x00BBCCu
#define KFX_COLOR_LIGHTING     0xFFEE66u

#ifdef TRACY_ENABLE
#  include <tracy/TracyC.h>
#  define KFX_C_ZONE_BEGIN(ctx, name)                         TracyCZoneN(ctx, name, 1)
#  define KFX_C_ZONE_BEGIN_COLOR(ctx, name, color)            TracyCZoneNC(ctx, name, color, 1)
#  define KFX_C_ZONE_END(ctx)                                 TracyCZoneEnd(ctx)
// Attach a numeric value (e.g. loop-iteration count, node-expansion count) to
// the current zone instance -- shown per-call in Tracy's zone info, not just
// aggregated. Call before KFX_C_ZONE_END, same ctx.
#  define KFX_C_ZONE_VALUE(ctx, value)                        TracyCZoneValue(ctx, value)
#  define KFX_C_PLOT(name, val)                               TracyCPlot(name, (double)(val))
#  define KFX_C_PLOT_CONFIG(name,type,step,fill,color)        TracyCPlotConfig(name, type, step, fill, color)
// Named memory pool tracking for custom (non-malloc) allocators -- e.g. Ariadne's
// fixed-size Triangle/Point arrays with free-list reuse. Using the array slot's
// real address as the "pointer" gives Tracy's Memory view genuine alloc/free
// lifetime + active-count tracking for a pool that never calls malloc/free.
#  define KFX_C_ALLOC_NAMED(ptr, size, name)                  TracyCAllocN(ptr, size, name)
#  define KFX_C_FREE_NAMED(ptr, name)                         TracyCFreeN(ptr, name)
#else
#  define KFX_C_ZONE_BEGIN(ctx, name)                         /* nothing */
#  define KFX_C_ZONE_BEGIN_COLOR(ctx, name, color)            /* nothing */
#  define KFX_C_ZONE_END(ctx)                                 ((void)0)
#  define KFX_C_ZONE_VALUE(ctx, value)                        ((void)0)
#  define KFX_C_PLOT(name, val)                               ((void)0)
#  define KFX_C_PLOT_CONFIG(name,type,step,fill,color)        ((void)0)
#  define KFX_C_ALLOC_NAMED(ptr, size, name)                  ((void)0)
#  define KFX_C_FREE_NAMED(ptr, name)                         ((void)0)
#endif

#endif /* KFX_PROFILING_C_H */
