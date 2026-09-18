/******************************************************************************/
/** @file KfxProfilingC.h
 *   Tracy zones, plots and named memory pools for C files.
 */
/******************************************************************************/
#ifndef KFX_PROFILING_C_H
#define KFX_PROFILING_C_H

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
#  define KFX_C_ZONE_VALUE(ctx, value)                        TracyCZoneValue(ctx, value)
#  define KFX_C_ZONE_NAME(ctx, txt, size)                     TracyCZoneName(ctx, txt, size)
#  define KFX_C_PLOT(name, val)                               TracyCPlot(name, (double)(val))
#  define KFX_C_PLOT_CONFIG(name,type,step,fill,color)        TracyCPlotConfig(name, type, step, fill, color)
#  define KFX_C_ALLOC_NAMED(ptr, size, name)                  TracyCAllocN(ptr, size, name)
#  define KFX_C_FREE_NAMED(ptr, name)                         TracyCFreeN(ptr, name)
#else
#  define KFX_C_ZONE_BEGIN(ctx, name)
#  define KFX_C_ZONE_BEGIN_COLOR(ctx, name, color)
#  define KFX_C_ZONE_END(ctx)                                 ((void)0)
#  define KFX_C_ZONE_VALUE(ctx, value)                        ((void)0)
#  define KFX_C_ZONE_NAME(ctx, txt, size)                     ((void)0)
#  define KFX_C_PLOT(name, val)                               ((void)0)
#  define KFX_C_PLOT_CONFIG(name,type,step,fill,color)        ((void)0)
#  define KFX_C_ALLOC_NAMED(ptr, size, name)                  ((void)0)
#  define KFX_C_FREE_NAMED(ptr, name)                         ((void)0)
#endif

#endif /* KFX_PROFILING_C_H */
