/******************************************************************************/
/** @file TargetSearchProbe.h
 *     Target search funnel counters and sampled per-stage zones. Game thread only.
 */
/******************************************************************************/
#ifndef KFX_AI_TARGETSEARCHPROBE_H
#define KFX_AI_TARGETSEARCHPROBE_H

#include "bflib_basics.h"
#include "globals.h"
#include "kfx/profiling/KfxProfilingC.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define TSP_SAMPLE_EVERY 256

enum TargetSearchProbeCounter {
    TSP_ScanCalls_Creature = 0,
    TSP_ScanCalls_Object,
    TSP_ScanCalls_Trap,
    TSP_ScanCalls_Other,
    TSP_Visited_Creature,
    TSP_Visited_Object,
    TSP_Visited_Trap,
    TSP_Visited_Other,
    TSP_ScanCalls_Sampled,
    // Entered == sum of the exits below
    TSP_Funnel_Entered,
    TSP_Rej_ClassModelOwner,
    TSP_Rej_Distance,
    TSP_Rej_WillAttack_State,
    TSP_Rej_WillAttack_AllyIdle,
    TSP_Rej_WillAttack_AllyOtherTarget,
    TSP_Rej_WillAttack_Self,
    TSP_Rej_WillAttack_Custody,
    TSP_Rej_WillAttack_Dropping,
    TSP_Rej_WillAttack_NoControlOrUnseen,
    TSP_Rej_InCombat,
    TSP_Rej_Reachability,
    TSP_Accepted,
    TSP_HostileTowards_Calls,
    TSP_HostileTowards_Iterations,
    TSP_NUM_COUNTERS
};

#ifdef TRACY_ENABLE

extern uint32_t g_tsp_counters[TSP_NUM_COUNTERS];
extern int32_t g_tsp_scan_depth;
extern int32_t g_tsp_sampling;

void TargetSearchProbe_BeginScan(ThingClass class_id);
void TargetSearchProbe_EndScan(ThingClass class_id, unsigned long visited);
void TargetSearchProbe_EmitTurn(void);

#  define TSP_INC(c)            do { if (g_tsp_scan_depth > 0) g_tsp_counters[c]++; } while (0)
#  define TSP_ADD(c, n)         do { if (g_tsp_scan_depth > 0) g_tsp_counters[c] += (uint32_t)(n); } while (0)
#  define TSP_SAMPLING()        (g_tsp_sampling != 0)
#  define TSP_SCAN_BEGIN(cls)   TargetSearchProbe_BeginScan(cls)
#  define TSP_SCAN_END(cls, k)  TargetSearchProbe_EndScan(cls, k)
#  define TSP_EMIT_TURN()       TargetSearchProbe_EmitTurn()

// Branch before the Tracy call: unsampled scans stay free of zone overhead
#  define TSP_ZONE_BEGIN(ctx, name) \
    static const struct ___tracy_source_location_data TracyConcat(__tsp_srcloc_,TracyLine) = { name, __func__, TracyFile, (uint32_t)TracyLine, KFX_COLOR_AI }; \
    TracyCZoneCtx ctx = {0}; \
    if (g_tsp_sampling) ctx = ___tracy_emit_zone_begin(&TracyConcat(__tsp_srcloc_,TracyLine), 1)
#  define TSP_ZONE_END(ctx) do { if (ctx.active) ___tracy_emit_zone_end(ctx); } while (0)

#  define TSP_SCAN_ZONE_BEGIN(ctx) \
    static const struct ___tracy_source_location_data __tsp_scan_srcloc = { "get_nth_thing_of_class_with_filter", __func__, TracyFile, (uint32_t)TracyLine, KFX_COLOR_AI }; \
    static const struct ___tracy_source_location_data __tsp_scan_srcloc_sampled = { "get_nth_thing_of_class_with_filter [sampled]", __func__, TracyFile, (uint32_t)TracyLine, KFX_COLOR_AI }; \
    TracyCZoneCtx ctx = ___tracy_emit_zone_begin(g_tsp_sampling ? &__tsp_scan_srcloc_sampled : &__tsp_scan_srcloc, 1)
#  define TSP_SCAN_ZONE_END(ctx, k) do { TracyCZoneValue(ctx, k) ___tracy_emit_zone_end(ctx); } while (0)

#else

#  define TSP_INC(c)            ((void)0)
#  define TSP_ADD(c, n)         ((void)0)
#  define TSP_SAMPLING()        (0)
#  define TSP_SCAN_BEGIN(cls)   ((void)0)
#  define TSP_SCAN_END(cls, k)  ((void)0)
#  define TSP_EMIT_TURN()       ((void)0)
#  define TSP_ZONE_BEGIN(ctx, name)
#  define TSP_ZONE_END(ctx)     ((void)0)
#  define TSP_SCAN_ZONE_BEGIN(ctx)
#  define TSP_SCAN_ZONE_END(ctx, k) ((void)0)

#endif

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
