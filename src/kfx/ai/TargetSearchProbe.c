/******************************************************************************/
/** @file TargetSearchProbe.c
 *     Target search funnel counters and per-turn plot emission.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/ai/TargetSearchProbe.h"

#include "thing_list.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#ifdef TRACY_ENABLE

uint32_t g_tsp_counters[TSP_NUM_COUNTERS];
int32_t g_tsp_scan_depth = 0;
int32_t g_tsp_sampling = 0;

static uint32_t tsp_top_level_scans = 0;

void TargetSearchProbe_BeginScan(ThingClass class_id)
{
    if (g_tsp_scan_depth == 0)
    {
        tsp_top_level_scans++;
        g_tsp_sampling = ((tsp_top_level_scans % TSP_SAMPLE_EVERY) == 0);
        if (g_tsp_sampling)
            g_tsp_counters[TSP_ScanCalls_Sampled]++;
    }
    g_tsp_scan_depth++;
    switch (class_id)
    {
    case TCls_Creature: g_tsp_counters[TSP_ScanCalls_Creature]++; break;
    case TCls_Object:   g_tsp_counters[TSP_ScanCalls_Object]++;   break;
    case TCls_Trap:     g_tsp_counters[TSP_ScanCalls_Trap]++;     break;
    default:            g_tsp_counters[TSP_ScanCalls_Other]++;    break;
    }
}

void TargetSearchProbe_EndScan(ThingClass class_id, unsigned long visited)
{
    switch (class_id)
    {
    case TCls_Creature: g_tsp_counters[TSP_Visited_Creature] += visited; break;
    case TCls_Object:   g_tsp_counters[TSP_Visited_Object]   += visited; break;
    case TCls_Trap:     g_tsp_counters[TSP_Visited_Trap]     += visited; break;
    default:            g_tsp_counters[TSP_Visited_Other]    += visited; break;
    }
    g_tsp_scan_depth--;
    if (g_tsp_scan_depth == 0)
        g_tsp_sampling = 0;
}

// Tracy keeps the pointer, so these must be string literals
static const char *const tsp_plot_names[TSP_NUM_COUNTERS] = {
    "TS/Scan/Calls creature list",
    "TS/Scan/Calls object list",
    "TS/Scan/Calls trap list",
    "TS/Scan/Calls other list",
    "TS/Scan/Visited creature list",
    "TS/Scan/Visited object list",
    "TS/Scan/Visited trap list",
    "TS/Scan/Visited other list",
    "TS/Scan/Sampled scans",
    "TS/Funnel/00 Entered",
    "TS/Funnel/01 Rej ClassModelOwner",
    "TS/Funnel/02 Rej Distance",
    "TS/Funnel/03 Rej WillAttack State",
    "TS/Funnel/04 Rej WillAttack AllyIdle",
    "TS/Funnel/05 Rej WillAttack AllyOtherTarget",
    "TS/Funnel/06 Rej WillAttack Self",
    "TS/Funnel/07 Rej WillAttack Custody",
    "TS/Funnel/08 Rej WillAttack Dropping",
    "TS/Funnel/09 Rej WillAttack NoControlOrUnseen",
    "TS/Funnel/10 Rej InCombat",
    "TS/Funnel/11 Rej Reachability",
    "TS/Funnel/12 Accepted",
    "TS/Hostile/HostileTowards calls",
    "TS/Hostile/HostileTowards loop iterations",
};

void TargetSearchProbe_EmitTurn(void)
{
    for (int i = 0; i < TSP_NUM_COUNTERS; i++)
    {
        TracyCPlot(tsp_plot_names[i], (double)g_tsp_counters[i]);
        g_tsp_counters[i] = 0;
    }
}

#endif
/******************************************************************************/
#ifdef __cplusplus
}
#endif
