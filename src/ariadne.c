/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne.c
 *     Dungeon routing and path finding system.
 * @par Purpose:
 *     Defines functions for finding creature routes and navigating
 *     through the dungeon.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Jan 2010 - 20 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "ariadne.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "config_terrain.h"
#include "ariadne_navitree.h"
#include "ariadne_regions.h"
#include "ariadne_tringls.h"
#include "ariadne_points.h"
#include "ariadne_edge.h"
#include "ariadne_findcache.h"
#include "ariadne_naviheap.h"
#include "thing_stats.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "gui_topmsg.h"
#include "map_columns.h"
#include "map_utils.h"
#include "game_legacy.h"

#define EDGEFIT_LEN           64
#define EDGEOR_COUNT           4

typedef long (*NavRules)(long, long);

struct QuadrantOffset {
    long x;
    long y;
};

struct HugStart {
    short field_0;
    unsigned char field_2;
};

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_ariadne_check_forward_for_wallhug_gap(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos, long outfri_y2);
/******************************************************************************/
DLLIMPORT long _DK_tri_initialised;
#define tri_initialised _DK_tri_initialised
DLLIMPORT unsigned long _DK_edgelen_initialised;
#define edgelen_initialised _DK_edgelen_initialised
DLLIMPORT unsigned long *_DK_EdgeFit;
#define EdgeFit _DK_EdgeFit
DLLIMPORT unsigned long _DK_RadiusEdgeFit[EDGEOR_COUNT][EDGEFIT_LEN];
#define RadiusEdgeFit _DK_RadiusEdgeFit
DLLIMPORT NavRules _DK_nav_rulesA2B;
#define nav_rulesA2B _DK_nav_rulesA2B
DLLIMPORT struct WayPoints _DK_wayPoints;
#define wayPoints _DK_wayPoints
/******************************************************************************/
unsigned char const actual_sizexy_to_nav_block_sizexy_table[] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    4,
};

const unsigned long actual_sizexy_to_nav_sizexy_table[] = {
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206,
    206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 206, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462,
    462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 462, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718,
    718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 718, 974,
    974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974,
    974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974,
    974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974, 974,
    974,
};

const struct HugStart blocked_x_hug_start[][2] = {
    {{          0, 1}, {LbFPMath_PI, 2}},
    {{          0, 2}, {LbFPMath_PI, 1}},
};

const struct HugStart blocked_y_hug_start[][2] = {
    {{3*LbFPMath_PI/2, 2}, {LbFPMath_PI/2, 1}},
    {{3*LbFPMath_PI/2, 1}, {LbFPMath_PI/2, 2}},
};

const struct HugStart blocked_xy_hug_start[][2][2] = {
   {{{3*LbFPMath_PI/2, 2}, {          0, 1}},
    {{  LbFPMath_PI/2, 1}, {          0, 2}}},
   {{{3*LbFPMath_PI/2, 1}, {LbFPMath_PI, 2}},
    {{  LbFPMath_PI/2, 2}, {LbFPMath_PI, 1}}},
};

struct Path fwd_path;
struct Path bak_path;
struct Path best_path;
/******************************************************************************/
long thing_nav_block_sizexy(const struct Thing *thing)
{
    long i;
    i = thing->clipbox_size_xy;
    if (i >= (long)(sizeof(actual_sizexy_to_nav_block_sizexy_table)/sizeof(actual_sizexy_to_nav_block_sizexy_table[0])))
        i = (long)(sizeof(actual_sizexy_to_nav_block_sizexy_table)/sizeof(actual_sizexy_to_nav_block_sizexy_table[0]))-1;
    if (i < 0)
        i = 0;
    return actual_sizexy_to_nav_block_sizexy_table[i];
}

long thing_nav_sizexy(const struct Thing *thing)
{
    long i;
    i = thing->clipbox_size_xy;
    if (i >= (long)(sizeof(actual_sizexy_to_nav_sizexy_table)/sizeof(actual_sizexy_to_nav_sizexy_table[0])))
        i = (long)(sizeof(actual_sizexy_to_nav_sizexy_table)/sizeof(actual_sizexy_to_nav_sizexy_table[0]))-1;
    if (i < 0)
        i = 0;
    return actual_sizexy_to_nav_sizexy_table[i];
}

/*
unsigned char tag_current;
unsigned char Tags[9000];
long tree.val[9001];
long tree_dad[9000];
long heap_end;
long Heap[258];
unsigned long edgelen_initialised = 0;
unsigned long *EdgeFit = NULL;
unsigned long RadiusEdgeFit[EDGEOR_COUNT][EDGEFIT_LEN];

long count_Points = 0;
long ix_Points = 0;
long free_Points = -1;
*/
/******************************************************************************/
long route_to_path(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y, const long *route, long wp_lim, struct Path *path, long *total_len);
void path_out_a_bit(struct Path *path, const long *route);
void gate_navigator_init8(struct Pathway *pway, long trAx, long trAy, long trBx, long trBy, long wp_lim, unsigned char a7);
void route_through_gates(const struct Pathway *pway, struct Path *path, long subroute);
long ariadne_push_position_against_wall(struct Thing *thing, const struct Coord3d *pos1, struct Coord3d *pos_out);
long ariadne_check_forward_for_wallhug_gap(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos, long a4);
long ariadne_get_blocked_flags(struct Thing *thing, const struct Coord3d *pos);
static long triangle_findSE8(long ptfind_x, long ptfind_y);
long ma_triangle_route(long ptfind_x, long ptfind_y, long *ptstart_x);
void edgelen_init(void);
/******************************************************************************/

// ariadne_compare_ways is unused by KFX code
// This function was added by mefisto: "Prepared a function for debugging Ariadne structure." - https://github.com/dkfans/keeperfx/commit/e92bb5d7a4ad9a9dc232df160e7fc9909fc822df
void ariadne_compare_ways(const struct Ariadne *arid1, const struct Ariadne *arid2)
{
    const struct Coord3d* p1;
    const struct Coord3d* p2;
    p1 = &arid1->startpos; p2 = &arid2->startpos;
    if (memcmp(p1,p2,sizeof(struct Coord3d)) != 0) {
        ERRORLOG("startpos DIFFERS (%d,%d,%d) (%d,%d,%d)",(int)p1->x.val,(int)p1->y.val,(int)p1->z.val,(int)p2->x.val,(int)p2->y.val,(int)p2->z.val);
    }
    p1 = &arid1->endpos; p2 = &arid2->endpos;
    if (memcmp(p1,p2,sizeof(struct Coord3d)) != 0) {
        ERRORLOG("endpos DIFFERS (%d,%d,%d) (%d,%d,%d)",(int)p1->x.val,(int)p1->y.val,(int)p1->z.val,(int)p2->x.val,(int)p2->y.val,(int)p2->z.val);
    }
    p1 = &arid1->current_waypoint_pos; p2 = &arid2->current_waypoint_pos;
    if (memcmp(p1,p2,sizeof(struct Coord3d)) != 0) {
        ERRORLOG("current_waypoint_pos DIFFERS (%d,%d,%d) (%d,%d,%d)",(int)p1->x.val,(int)p1->y.val,(int)p1->z.val,(int)p2->x.val,(int)p2->y.val,(int)p2->z.val);
    }
    p1 = &arid1->pos_12; p2 = &arid2->pos_12;
    if (memcmp(p1,p2,sizeof(struct Coord3d)) != 0) {
        ERRORLOG("pos_12 DIFFERS (%d,%d,%d) (%d,%d,%d)",(int)p1->x.val,(int)p1->y.val,(int)p1->z.val,(int)p2->x.val,(int)p2->y.val,(int)p2->z.val);
    }
    p1 = &arid1->pos_18; p2 = &arid2->pos_18;
    if (memcmp(p1,p2,sizeof(struct Coord3d)) != 0) {
        ERRORLOG("pos_18 DIFFERS (%d,%d,%d) (%d,%d,%d)",(int)p1->x.val,(int)p1->y.val,(int)p1->z.val,(int)p2->x.val,(int)p2->y.val,(int)p2->z.val);
    }
    if (memcmp(&arid1->route_flags,&arid2->route_flags,14) != 0) {
        ERRORLOG("pos_18..field_24 DIFFERS");
    }
    if (arid1->move_speed != arid2->move_speed) {
        ERRORLOG("move_speed DIFFERS");
    }
    if (arid1->current_waypoint != arid2->current_waypoint) {
        ERRORLOG("current_waypoint DIFFERS");
    }
    int i;
    for (i=0; i < ARID_WAYPOINTS_COUNT; i++) {
        const struct Coord2d* w1;
        const struct Coord2d* w2;
        w1 = &arid1->waypoints[i]; w2 = &arid2->waypoints[i];
        if (memcmp(w1,w2,sizeof(struct Coord3d)) != 0) {
            ERRORLOG("waypoints[%d] DIFFERS (%d,%d) (%d,%d)",i,(int)w1->x.val,(int)w1->y.val,(int)w2->x.val,(int)w2->y.val);
        }
    }
    if (arid1->stored_waypoints != arid2->stored_waypoints) {
        ERRORLOG("stored_waypoints DIFFERS");
    }
    if (arid1->total_waypoints != arid2->total_waypoints) {
        ERRORLOG("total_waypoints DIFFERS");
    }
    p1 = &arid1->pos_53; p2 = &arid2->pos_53;
    if (memcmp(p1,p2,sizeof(struct Coord3d)) != 0) {
        ERRORLOG("pos_53 DIFFERS (%d,%d,%d) (%d,%d,%d)",(int)p1->x.val,(int)p1->y.val,(int)p1->z.val,(int)p2->x.val,(int)p2->y.val,(int)p2->z.val);
    }
    p1 = &arid1->pos_59; p2 = &arid2->pos_59;
    if (memcmp(p1,p2,4) != 0) { // Compare only X and Y here; skip Z
        ERRORLOG("pos_59 DIFFERS (%d,%d,%d) (%d,%d,%d)",(int)p1->x.val,(int)p1->y.val,(int)p1->z.val,(int)p2->x.val,(int)p2->y.val,(int)p2->z.val);
    }
    if (arid1->manoeuvre_state != arid2->manoeuvre_state) {
        ERRORLOG("manoeuvre_state DIFFERS");
    }
    if (arid1->wallhug_angle != arid2->wallhug_angle) {
        ERRORLOG("wallhug_angle DIFFERS");
    }
    if (arid1->field_62 != arid2->field_62) {
        ERRORLOG("field_62 DIFFERS");
    }
}

unsigned long fits_thro(long tri_idx, long ormask_idx)
{
    static unsigned long const edgelen_ORmask[] = {60, 51, 15};
    unsigned long eidx;
    unsigned long emask;

    if (tri_idx >= TRIANLGLES_COUNT)
    {
        ERRORDBG(5,"triangles overflow");
        erstat_inc(ESE_NoFreeTriangls);
        return 0;
    }
    if (ormask_idx >= EDGEOR_COUNT)
    {
        ERRORLOG("ORmask overflow");
        return 0;
    }
    emask = get_triangle_edgelen(tri_idx);
    eidx = edgelen_ORmask[ormask_idx] | emask;
    if (EdgeFit != RadiusEdgeFit[0])
    {
      if (EdgeFit != RadiusEdgeFit[1])
      {
        if (EdgeFit != RadiusEdgeFit[2])
        {
          ERRORLOG("table err");
          return 0;
        }
      }
    }
    if (edgelen_initialised != 1)
    {
        ERRORLOG("uninit");
        return 0;
    }
    if (eidx >= EDGEFIT_LEN)
    {
        ERRORLOG("edgebits overflow");
        return 0;
    }
    return EdgeFit[eidx];
}

void triangulate_map(unsigned char *imap)
{
    triangulate_area(imap, 0, 0, navigation_map_size_x, navigation_map_size_y);
}

void init_navigation_map(void)
{
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    LbMemorySet(game.navigation_map, 0, navigation_map_size_x*navigation_map_size_y);
    for (stl_y=0; stl_y < navigation_map_size_y; stl_y++)
    {
        for (stl_x=0; stl_x < navigation_map_size_x; stl_x++)
        {
            set_navigation_map(stl_x, stl_y, get_navigation_colour(stl_x, stl_y));
        }
    }
    nav_map_initialised = 1;
}

static long get_navtree_owner(long treeI)
{
    long owner;
    owner = ((treeI & 0xE0) >> 5) - 1;
    if (owner == 5)
    {
        owner = game.hero_player_num;
    } else
    if (owner == 6)
    {
        owner = game.neutral_player_num;
    }
    return owner;
}

long Keeper_nav_rulesA2B(long treeA, long treeB)
{
    if ((treeB & 0x0F) - (treeA & 0x0F) > 1)
        return 0;
    if ((treeB & 0x10) == 0)
        return 1;
    return 2;
}

long navigation_rule_normal(long treeA, long treeB)
{
    if ((treeB & 0x0F) - (treeA & 0x0F) > 1)
      return 0;
    if ((treeB & 0xF0) == 0)
      return 1;
    if (owner_player_navigating != -1)
    {
        if (get_navtree_owner(treeB) == owner_player_navigating)
          return 0;
    }
    if ((treeB & 0x10) == 0)
        return 1;
    if ((treeA & 0x10) != 0)
        return 1;
    return nav_thing_can_travel_over_lava;
}

long init_navigation(void)
{
    IanMap = (unsigned char *)&game.navigation_map;
    init_navigation_map();
    triangulate_map(IanMap);
    nav_rulesA2B = navigation_rule_normal;
    game.field_14EA4B = 1;
    return 1;
}

long update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y)
{
    long sx;
    long sy;
    long ex;
    long ey;
    long x;
    long y;
    if (!nav_map_initialised)
        init_navigation_map();
    // Prepare parameter bounds
    if (end_x < start_x)
      start_x = end_x;
    if (end_y < start_y)
      start_y = end_y;
    sx = start_x - 1;
    if (sx <= 2)
      sx = 2;
    sy = start_y - 1;
    if (sy <= 2)
      sy = 2;
    ex = end_x + 1;
    if (ex >= map_subtiles_x-2)
      ex = map_subtiles_x-2;
    ey = end_y + 1;
    if (ey >= map_subtiles_y-2)
      ey = map_subtiles_y-2;
    // Fill a rectangle with nav colors (based on columns and blocks)
    for (y = sy; y <= ey; y++)
    {
        for (x = sx; x <= ex; x++)
        {
            set_navigation_map(x, y, get_navigation_colour(x, y));
        }
    }
    triangulate_area(IanMap, sx, sy, ex, ey);
    return true;
}

static void edge_points8(long ntri_src, long ntri_dst, long *tipA_x, long *tipA_y, long *tipB_x, long *tipB_y)
{
    //_DK_edge_points8(a1, a2, a3, a4, a5, a6);
    struct Point *pt;
    if (Triangles[ntri_src].tags[0] == ntri_dst)
    {
        pt = get_triangle_point(ntri_src,1);
        *tipA_x = pt->x << 8;
        *tipA_y = pt->y << 8;
        pt = get_triangle_point(ntri_src,0);
        *tipB_x = pt->x << 8;
        *tipB_y = pt->y << 8;
    } else
    if (Triangles[ntri_src].tags[1] == ntri_dst)
    {
        pt = get_triangle_point(ntri_src,2);
        *tipA_x = pt->x << 8;
        *tipA_y = pt->y << 8;
        pt = get_triangle_point(ntri_src,1);
        *tipB_x = pt->x << 8;
        *tipB_y = pt->y << 8;
    } else
    if (Triangles[ntri_src].tags[2] == ntri_dst)
    {
        pt = get_triangle_point(ntri_src,0);
        *tipA_x = pt->x << 8;
        *tipA_y = pt->y << 8;
        pt = get_triangle_point(ntri_src,2);
        *tipB_x = pt->x << 8;
        *tipB_y = pt->y << 8;
    }
    else
    {
        ERRORLOG("edge not found %d->%d", ntri_src, ntri_dst);
    }
}
HOOK_DK_FUNC(edge_points8)

long fov_region(long a1, long a2, const struct FOV *fov)
{
    long diff_ax;
    long diff_ay;
    diff_ax = a1 - fov->tipA.x;
    diff_ay = a2 - fov->tipA.y;
    long diff_bx;
    long diff_by;
    diff_bx = fov->tipB.x - fov->tipA.x;
    diff_by = fov->tipB.y - fov->tipA.y;
    if (LbCompareMultiplications(diff_ay, diff_bx, diff_ax, diff_by) < 0) {
        return -1;
    }
    long diff_cx;
    long diff_cy;
    diff_cx = fov->tipC.x - fov->tipA.x;
    diff_cy = fov->tipC.y - fov->tipA.y;
    return (LbCompareMultiplications(diff_ay, diff_cx, diff_ax, diff_cy) > 0);
}

long route_to_path(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y, const long *route, long wp_lim, struct Path *path, long *total_len)
{
    NAVIDBG(19,"Starting");

    struct FOV fov_AC;
    long edge1_x;
    long edge1_y;
    long edge2_x;
    long edge2_y;
    char reg1;
    char reg2;
    long wp1;
    long wp2;
    long wpi;
    int wp_num;

    path->start.x = ptfind_x;
    path->start.y = ptfind_y;
    path->finish.x = ptstart_x;
    path->finish.y = ptstart_y;
    *total_len = 0;
    fov_AC.tipA.x = ptfind_x;
    fov_AC.tipA.y = ptfind_y;
    wp_num = 0;
    if (wp_lim == 0)
    {
      *total_len = LbSqrL((ptstart_x - ptfind_x) * (ptstart_x - ptfind_x) + (ptstart_y - ptfind_y) * (ptstart_y - ptfind_y));
      path->waypoints[0].x = ptstart_x;
      path->waypoints[0].y = ptstart_y;
      wayPoints.wpfield_10[0] = 0;
      path->waypoints_num = 1;
      return 1;
    }
    wp2 = 0;
    wp1 = 0;
    wpi = 0;
    edge_points8(route[wpi+0], route[wpi+1], &fov_AC.tipB.x, &fov_AC.tipB.y, &fov_AC.tipC.x, &fov_AC.tipC.y);
    wayPoints.wpfield_8 = wpi;
    wayPoints.wpfield_C = wpi;
    wpi++;
    while ( 1 )
    {
      if (wpi < wp_lim)
      {
          edge_points8(route[wpi+0], route[wpi+1], &edge1_x, &edge1_y, &edge2_x, &edge2_y);
          wayPoints.wpfield_0 = wpi;
          wayPoints.wpfield_4 = wpi;
          reg1 = fov_region(edge1_x, edge1_y, &fov_AC);
          reg2 = fov_region(edge2_x, edge2_y, &fov_AC);
      } else
      {
          reg2 = fov_region(ptstart_x, ptstart_y, &fov_AC);
          reg1 = reg2;
          if (reg2 == 0)
            break;
      }
      if (reg1 == 0)
      {
          fov_AC.tipB.x = edge1_x;
          fov_AC.tipB.y = edge1_y;
          wayPoints.wpfield_8 = wayPoints.wpfield_0;
          wp1 = wpi;
      }
      if (reg2 == 0)
      {
          fov_AC.tipC.x = edge2_x;
          fov_AC.tipC.y = edge2_y;
          wayPoints.wpfield_C = wayPoints.wpfield_4;
          wp2 = wpi;
      }
      if (reg2 == -1)
      {
        if (wp_num == ARID_PATH_WAYPOINTS_COUNT) {
            ERRORLOG("Exceeded max path length (i:%d,L:%d) (%d,%d)->(%d,%d)",
            wpi, wp_lim, ptfind_x, ptfind_y, ptstart_x, ptstart_y);
        }
        *total_len += LbSqrL((fov_AC.tipB.x - fov_AC.tipA.x) * (fov_AC.tipB.x - fov_AC.tipA.x)
            + (fov_AC.tipB.y - fov_AC.tipA.y) * (fov_AC.tipB.y - fov_AC.tipA.y));
        fov_AC.tipA.x = fov_AC.tipB.x;
        path->waypoints[wp_num].x = fov_AC.tipB.x;
        path->waypoints[wp_num].y = fov_AC.tipB.y;
        wayPoints.wpfield_10[wp_num] = wayPoints.wpfield_8;
        wp_num++;
        wpi = wp1;
        fov_AC.tipA.y = fov_AC.tipB.y;
        edge_points8(route[wpi+0], route[wpi+1], &fov_AC.tipB.x, &fov_AC.tipB.y, &fov_AC.tipC.x, &fov_AC.tipC.y);
        wayPoints.wpfield_8 = wpi;
        wayPoints.wpfield_C = wpi;
      } else
      if (reg1 == 1)
      {
        if (wp_num == ARID_PATH_WAYPOINTS_COUNT) {
            ERRORLOG("Exceeded max path length (i:%d,R:%d) (%d,%d)->(%d,%d)",
            wpi, wp_lim, ptfind_x, ptfind_y, ptstart_x, ptstart_y);
        }
        *total_len += LbSqrL((fov_AC.tipC.x - fov_AC.tipA.x) * (fov_AC.tipC.x - fov_AC.tipA.x)
            + (fov_AC.tipC.y - fov_AC.tipA.y) * (fov_AC.tipC.y - fov_AC.tipA.y));
        fov_AC.tipA.x = fov_AC.tipC.x;
        path->waypoints[wp_num].x = fov_AC.tipC.x;
        path->waypoints[wp_num].y = fov_AC.tipC.y;
        wayPoints.wpfield_10[wp_num] = wayPoints.wpfield_C;
        wp_num++;
        wpi = wp2;
        fov_AC.tipA.y = fov_AC.tipC.y;
        edge_points8(route[wpi+0], route[wpi+1], &fov_AC.tipB.x, &fov_AC.tipB.y, &fov_AC.tipC.x, &fov_AC.tipC.y);
        wayPoints.wpfield_8 = wpi;
        wayPoints.wpfield_C = wpi;
      }
      wpi++;
    }
    if (wp_num == ARID_PATH_WAYPOINTS_COUNT) {
        ERRORLOG("Exceeded max path length - gate_route_to_coords");
    }
    *total_len += LbSqrL((ptstart_x - fov_AC.tipA.x) * (ptstart_x - fov_AC.tipA.x)
        + (ptstart_y - fov_AC.tipA.y) * (ptstart_y - fov_AC.tipA.y));
    path->waypoints[wp_num].x = ptstart_x;
    path->waypoints[wp_num].y = ptstart_y;
    wayPoints.wpfield_10[wp_num] = wp_lim;
    wp_num++;
    path->waypoints_num = wp_num;
    return wp_num;
}

void waypoint_normal(long tri1_id, long cor1_id, long *norm_x, long *norm_y)
{
    //_DK_waypoint_normal(a1, a2, norm_x, norm_y);
    int tri2_id;
    int tri3_id;
    int cor2_id;
    int cor3_id;
    tri3_id = tri1_id;
    cor3_id = MOD3[cor1_id+2];
    tri2_id = Triangles[tri1_id].tags[cor1_id];
    cor2_id = link_find(tri2_id, tri1_id);
    cor2_id = MOD3[cor2_id+1];
    unsigned long k;
    k = 0;
    while (1)
    {
        int ntri;
        ntri = Triangles[tri2_id].tags[cor2_id];
        if (!nav_rulesA2B(get_triangle_tree_alt(tri2_id), get_triangle_tree_alt(ntri)))
            break;
        cor2_id = link_find(ntri, tri2_id);
        if (cor2_id < 0)
            break;
        cor2_id = MOD3[cor2_id+1];
        tri2_id = ntri;
        if (tri1_id == tri2_id)
        {
            tri2_id = Triangles[tri1_id].tags[cor1_id];
            cor2_id = link_find(tri2_id, tri1_id);
            cor2_id = MOD3[cor2_id+1];
            break;
        }
        k++;
        if (k > TRIANLGLES_COUNT) {
            ERRORLOG("Infinite loop detected");
            cor2_id = -1;
            break;
        }
    }

    k = 0;
    while ( 1 )
    {
        int ntri;
        ntri = Triangles[tri3_id].tags[cor3_id];
        if (!nav_rulesA2B(get_triangle_tree_alt(tri3_id), get_triangle_tree_alt(ntri)))
            break;
        cor3_id = link_find(ntri, tri3_id);
        if (cor3_id < 0)
            break;
        cor3_id = MOD3[cor3_id+2];
        tri3_id = ntri;
        if (tri1_id == ntri)
        {
            tri3_id = tri1_id;
            cor3_id = MOD3[cor1_id+2];
            break;
        }
        k++;
        if (k > TRIANLGLES_COUNT) {
            ERRORLOG("Infinite loop detected");
            cor3_id = -1;
            break;
        }
    }

    int diff_x;
    int diff_y;
    if ((cor2_id >= 0) && (cor3_id >= 0))
    {
        int pt1;
        int pt2;
        pt1 = Triangles[tri2_id].points[MOD3[cor2_id+1]];
        pt2 = Triangles[tri3_id].points[cor3_id];
        diff_y = ari_Points[pt1].y - ari_Points[pt2].y;
        diff_x = ari_Points[pt2].x - ari_Points[pt1].x;
    } else
    {
        diff_y = 0;
        diff_x = 0;
    }
    long nx;
    long ny;
    nx = -1;
    if (diff_y >= 0)
        nx = diff_y != 0;
    ny = -1;
    if (diff_x >= 0)
        ny = diff_x != 0;
    *norm_x = nx;
    *norm_y = ny;
}

void path_out_a_bit(struct Path *path, const long *route)
{
    struct PathWayPoint *ppoint;
    long *wpoint;
    long tip_x;
    long tip_y;
    long norm_x;
    long norm_y;
    long prev_pt;
    long curr_pt;
    long link_fwd;
    long link_bak;
    long i;
    //_DK_path_out_a_bit(path, route); return;
    wpoint = &wayPoints.wpfield_10[0];
    ppoint = &path->waypoints[0];
    for (i=0; i < path->waypoints_num-1; i++)
    {
        prev_pt = route[*wpoint];
        curr_pt = route[*wpoint+1];
        link_fwd = link_find(prev_pt, curr_pt);
        link_bak = link_find(curr_pt, prev_pt);
        tip_x = (ppoint->x >> 8);
        tip_y = (ppoint->y >> 8);
        if (triangle_tip_equals(prev_pt, link_fwd, tip_x, tip_y))
        {
            waypoint_normal(prev_pt, link_fwd, &norm_x, &norm_y);
        } else
        if (triangle_tip_equals(curr_pt, link_bak, tip_x, tip_y))
        {
            waypoint_normal(curr_pt, link_bak, &norm_x, &norm_y);
        } else
        {
            ERRORLOG("waypoint mismatch");
            continue;
        }
        ppoint->x += norm_x;
        ppoint->y += norm_y;
        wpoint++;
        ppoint++;
    }
}

void cull_gate_to_point(struct Gate *gt, long a2)
{
    //_DK_cull_gate_to_point(gt, a2);
    int diff_a;
    int diff_b;
    diff_a = abs(gt->field_0 - gt->field_8);
    diff_b = abs(gt->field_4 - gt->field_C);
    if (diff_a <= diff_b)
    {
      if (diff_b + (diff_a >> 1) < a2)
          return;
    } else
    {
      if (diff_a + (diff_b >> 1) < a2)
          return;
    }
    if (gt->field_18 == 1)
    {
        diff_a = (gt->field_0 - gt->field_8) << 6;
        diff_b = (gt->field_4 - gt->field_C) << 6;
    } else
    {
        diff_a = (gt->field_8 - gt->field_0) << 6;
        diff_b = (gt->field_C - gt->field_4) << 6;
    }
    long div_a;
    long div_b;
    long cmul;
    long val_x;
    long val_y;
    div_b = LbSqrL(((unsigned long long)(diff_a * diff_a) >> 14) + ((unsigned long long)(diff_b * diff_b) >> 14)) << 13;
    if (div_b < 1)
        div_b = 1;
    div_a = div_b;
    if (diff_a < 0)
    {
        diff_a = -diff_a;
        div_a = -div_b;
    }
    cmul = (a2 >> 8) - 1;
    val_x = cmul * ((unsigned long long)diff_a << 14) / div_a;
    if (diff_b < 0)
    {
        diff_b = -diff_b;
        div_b = -div_b;
    }
    val_y = cmul * ((unsigned long long)diff_b << 14) / div_b;
    if (gt->field_18 == 1)
    {
        gt->field_0 = val_x + gt->field_8;
        gt->field_4 = val_y + gt->field_C;
    } else
    {
        gt->field_8 = val_x + gt->field_0;
        gt->field_C = val_y + gt->field_4;
    }
}

long calc_intersection(struct Gate *gt, long a2, long a3, long a4, long a5)
{
    int diff_a;
    int diff_b;
    int diff_c;
    int diff_d;
    int diff_e;
    int diff_f;
    diff_a = (gt->field_0 - a2) << 6;
    diff_b = (a3 - a5) << 6;
    diff_c = (gt->field_4 - a3) << 6;
    diff_d = (a2 - a4) << 6;
    diff_e = (gt->field_8 - gt->field_0) << 6;
    diff_f = (gt->field_C - gt->field_4) << 6;
    int area_m;
    int area_d;
    int factor;
    area_m = ((unsigned long long)(diff_a * diff_b) >> 14)
       - ((unsigned long long)(diff_c * diff_d) >> 14);
    area_d = ((unsigned long long)(diff_d * diff_f) >> 14)
       - ((unsigned long long)(diff_b * diff_e) >> 14);
    if (area_d == 0)
        return 0;
    if (area_m < 0) {
      area_m = -area_m;
      area_d = -area_d;
    }
    factor = ((unsigned long long)area_m << 14) / area_d;
    if ((factor < -16384) || (factor > 16384))
        return 0;
    gt->field_10 = gt->field_0 + (((unsigned long long)(diff_e * factor) >> 14) >> 6);
    gt->field_14 = gt->field_4 + (((unsigned long long)(diff_f * factor) >> 14) >> 6);

    int vmin;
    vmin = gt->field_8;
    if (vmin >= gt->field_0)
      vmin = gt->field_0;
    if (vmin > gt->field_10)
        return 0;

    vmin = gt->field_8;
    if (vmin <= gt->field_0)
      vmin = gt->field_0;
    if (vmin < gt->field_10)
        return 0;

    vmin = gt->field_4;
    if (vmin >= gt->field_C)
      vmin = gt->field_C;
    if (vmin > gt->field_14)
        return 0;

    vmin = gt->field_4;
    if (vmin <= gt->field_C)
      vmin = gt->field_C;
    if (vmin < gt->field_14)
        return 0;

    return 1;
}

void cull_gate_to_best_point(struct Gate *gt, long a2)
{
    //_DK_cull_gate_to_best_point(gt, a2);
    int diff_min1;
    int diff_min2;
    {
        int diff_x;
        int diff_y;
        diff_x = abs(gt->field_0 - gt->field_10);
        diff_y = abs(gt->field_4 - gt->field_14);
        if (diff_x <= diff_y)
            diff_min1 = (diff_x >> 1) + diff_y;
        else
            diff_min1 = (diff_y >> 1) + diff_x;
        diff_x = abs(gt->field_8 - gt->field_10);
        diff_y = abs(gt->field_C - gt->field_14);
        if (diff_x <= diff_y)
            diff_min2 = diff_y + (diff_x >> 1);
        else
            diff_min2 = (diff_y >> 1) + diff_x;
    }
    if ((diff_min1 >= (a2 >> 1)) || (diff_min2 >= (a2 >> 1)))
    {
        int diff_lim;
        diff_lim = (a2 + 2) >> 1;
        if (diff_min1 < diff_lim)
        {
            gt->field_18 = 0;
            cull_gate_to_point(gt, a2);
        } else
        if (diff_min2 < diff_lim)
        {
            gt->field_18 = 1;
            cull_gate_to_point(gt, a2);
        } else
        {
            int rel_A;
            int rel_B;
            {
                int diff_B;
                int diff_A;
                diff_A = (gt->field_8 - gt->field_0) << 6;
                diff_B = (gt->field_C - gt->field_4) << 6;
                int dlen_A;
                int dlen_B;
                dlen_A = LbSqrL(((unsigned long long)(diff_A * diff_A) >> 14) + ((unsigned long long)(diff_B * diff_B) >> 14)) << 7;
                dlen_B = dlen_A;
                if (dlen_A)
                {
                    if (diff_A < 0)
                    {
                        diff_A = -diff_A;
                        dlen_A = -dlen_A;
                    }
                    rel_A = (((unsigned long long)diff_A << 14) / dlen_A) >> 6;
                } else
                {
                    rel_A = 0;
                }
                if ( dlen_B )
                {
                    if (diff_B < 0)
                    {
                        diff_B = -diff_B;
                        dlen_B = -dlen_B;
                    }
                    rel_B = (((unsigned long long)diff_B << 14) / dlen_B) >> 6;
                } else
                {
                  rel_B = 0;
                }
            }

            long delta_A;
            long delta_B;
            delta_A = (a2 >> 9) * rel_A;
            delta_B = (a2 >> 9) * rel_B;
            int cmin_y;
            int cmin_x;
            int cmax_x;
            int cmax_y;
            cmin_x = gt->field_10 - delta_A;
            cmin_y = gt->field_14 - delta_B;
            cmax_y = gt->field_14 + delta_B;
            cmax_x = gt->field_10 + delta_A;

            int ctst_x1;
            int ctst_x2;
            int ctst_y1;
            int ctst_y2;

            ctst_x1 = gt->field_8;
            if (ctst_x1 >= gt->field_0)
              ctst_x1 = gt->field_0;
            if (ctst_x1 <= cmin_x)
            {
                ctst_x2 = gt->field_8;
                if (ctst_x2 <= gt->field_0)
                    ctst_x2 = gt->field_0;
                if (ctst_x2 >= cmin_x)
                {
                  ctst_y2 = gt->field_4;
                  ctst_y1 = ctst_y2;
                  if (ctst_y1 >= gt->field_C)
                      ctst_y1 = gt->field_C;
                  if (ctst_y1 <= cmin_y)
                  {
                    if (ctst_y2 <= gt->field_C)
                        ctst_y2 = gt->field_C;
                    if (ctst_y2 >= cmin_y)
                    {
                        gt->field_0 = cmin_x;
                        gt->field_4 = cmin_y;
                    }
                  }
                }
            }

            ctst_x1 = gt->field_8;
            if (ctst_x1 >= gt->field_0)
                ctst_x1 = gt->field_0;
            if (ctst_x1 <= cmax_x)
            {
                ctst_x2 = gt->field_8;
                if (ctst_x2 <= gt->field_0)
                    ctst_x2 = gt->field_0;
                if (ctst_x2 >= cmax_x)
                {
                  ctst_y2 = gt->field_4;
                  ctst_y1 = gt->field_4;
                  if (ctst_y2 >= gt->field_C)
                      ctst_y1 = gt->field_C;
                  if (ctst_y1 <= cmax_y)
                  {
                    if (ctst_y2 <= gt->field_C)
                      ctst_y2 = gt->field_C;
                    if (ctst_y2 >= cmax_y)
                    {
                        gt->field_8 = cmax_x;
                        gt->field_C = cmax_y;
                    }
                  }
                }
            }
        }
    }
}

long gate_route_to_coords(long trAx, long trAy, long trBx, long trBy, long *a5, long a6, struct Pathway *pway, long a8)
{
    long total_len;
    best_path.waypoints_num = route_to_path(trAx, trAy, trBx, trBy, a5, a6, &best_path, &total_len);
    pway->field_0 = trAx;
    pway->field_4 = trAy;
    pway->field_8 = trBx;
    pway->field_C = trBy;
    if (a6 < 1)
    {
        pway->points[0].field_C = trBy;
        pway->points[0].field_8 = trBx;
        pway->points[0].field_0 = trBx;
        pway->points[0].field_4 = trBy;
        pway->points[0].field_18 = 0;
        pway->points_num = 1;
        return 1;
    }
    struct FOV fov1;
    struct FOV fov2;
    fov1.tipA.x = trAx;
    fov1.tipA.y = trAy;
    edge_points8(a5[0], a5[1], &fov1.tipB.x, &fov1.tipB.y, &fov1.tipC.x, &fov1.tipC.y);
    memcpy(&fov2, &fov1, sizeof(struct FOV));
    int pt_num;
    int wp_idx;
    wp_idx = 0;
    pt_num = 0;
    int wp_x;
    int wp_y;
    wp_x = pway->field_0;
    wp_y = pway->field_4;

    struct Gate *gt;
    gt = pway->points;
    int wpi;
    for (wpi=1; wpi <= a6; wpi++)
    {
        long edge_x1;
        long edge_y1;
        long edge_x2;
        long edge_y2;
        if (wpi < a6)
        {
            edge_points8(a5[wpi+0], a5[wpi+1], &edge_x1, &edge_y1, &edge_x2, &edge_y2);
        } else
        {
            edge_x2 = trBx;
            edge_x1 = trBx;
            edge_y2 = trBy;
            edge_y1 = trBy;
        }
        char reg1;
        char reg2;
        char reg3;
        char reg4;
        reg1 = fov_region(edge_x1, edge_y1, &fov1);
        reg2 = fov_region(edge_x2, edge_y2, &fov1);
        reg3 = fov_region(edge_x1, edge_y1, &fov2);
        reg4 = fov_region(edge_x2, edge_y2, &fov2);

        if ( reg1 || reg2 || reg3 || reg4 )
        {
            if (pt_num == 256) {
                ERRORLOG("grtc:Exceeded max path length (i:%d,rl:%d)", wpi, a6);
            }
            gt->field_0 = fov1.tipB.x;
            gt->field_4 = fov1.tipB.y;
            gt->field_8 = fov1.tipC.x;
            gt->field_C = fov1.tipC.y;
            gt->field_18 = -1;
            int dist_x;
            int dist_y;
            int bwp_x;
            int bwp_y;

            int dist_A;
            int dist_B;
            bwp_x = best_path.waypoints[wp_idx].x;
            dist_x = abs(gt->field_0 - bwp_x);
            bwp_y = best_path.waypoints[wp_idx].y;
            dist_y = abs(gt->field_4 - bwp_y);
            if (dist_x <= dist_y)
                dist_A = (dist_x >> 1) + dist_y;
            else
                dist_A = (dist_y >> 1) + dist_x;
            dist_B = dist_A;

            int dist_C;
            int dist_D;
            dist_x = abs(gt->field_8 - bwp_x);
            dist_y = abs(gt->field_C - bwp_y);
            if ( dist_x <= dist_y )
                dist_x >>= 1;
            else
                dist_y >>= 1;

            dist_C = dist_x + dist_y;
            dist_D = dist_C;

            if (wp_idx < best_path.waypoints_num-1)
            {
              bwp_x = best_path.waypoints[wp_idx+1].x;
              dist_x = abs(gt->field_0 - bwp_x);
              bwp_y = best_path.waypoints[wp_idx+1].y;
              dist_y = abs(gt->field_4 - bwp_y);
              if (dist_x <= dist_y)
                  dist_B = (dist_x >> 1) + dist_y;
              else
                  dist_B = dist_x + (dist_y >> 1);
              dist_x = abs(gt->field_8 - bwp_x);
              dist_y = abs(gt->field_C - bwp_y);
              if (dist_y >= dist_x)
                  dist_D = (dist_x >> 1) + dist_y;
              else
                  dist_D = dist_x + (dist_y >> 1);
            }
            int dist_min1;
            int dist_min2;
            dist_min1 = dist_C;
            if (dist_min1 >= dist_A)
              dist_min1 = dist_A;
            dist_min2 = dist_D;
            if (dist_min2 >= dist_B)
              dist_min2 = dist_B;
            if (dist_min1 >= dist_min2)
            {
                gt->field_18 = (dist_D <= dist_B);
                if (wp_idx < best_path.waypoints_num-1)
                {
                    wp_x = best_path.waypoints[wp_idx].x;
                    wp_y = best_path.waypoints[wp_idx].y;
                    wp_idx++;
                }
            } else
            {
                gt->field_18 = (dist_C <= dist_A);
                dist_min2 = dist_min1;
            }
            if (dist_min2 < 256)
            {
                cull_gate_to_point(gt, a8);
            } else
            {
                int fld18_mem;
                fld18_mem = gt->field_18;
                gt->field_18 = 2;
                if ( !calc_intersection(gt, wp_x, wp_y, best_path.waypoints[wp_idx].x, best_path.waypoints[wp_idx].y) )
                {
                  if (calc_intersection(gt,
                         best_path.waypoints[wp_idx].x, best_path.waypoints[wp_idx].y,
                         best_path.waypoints[wp_idx+1].x, best_path.waypoints[wp_idx+1].y) )
                  {
                    if ( best_path.waypoints_num - 1 > wp_idx )
                    {
                        wp_x = best_path.waypoints[wp_idx].x;
                        wp_y = best_path.waypoints[wp_idx].y;
                        ++wp_idx;
                    }
                  }
                  else
                  {
                    gt->field_18 = fld18_mem;
                  }
              }
              if (gt->field_18 == 2)
              {
                  cull_gate_to_best_point(gt, a8);
                  gt->field_18 = fld18_mem;
              } else
              {
                  cull_gate_to_point(gt, a8);
                  gt->field_18 = fld18_mem;
              }
            }
            fov1.tipA.x = gt->field_0;
            fov1.tipA.y = gt->field_4;
            fov2.tipA.x = gt->field_8;
            fov2.tipA.y = gt->field_C;
            pt_num++;
            gt++;
        }
        fov2.tipB.x = edge_x1;
        fov1.tipB.x = edge_x1;
        fov2.tipB.y = edge_y1;
        fov1.tipB.y = edge_y1;
        fov2.tipC.x = edge_x2;
        fov1.tipC.x = edge_x2;
        fov2.tipC.y = edge_y2;
        fov1.tipC.y = edge_y2;
    }
    if (pt_num == 256) {
        ERRORLOG("grtc:Exceeded max path length (i:%d,rl:%d)", wpi, a6);
    }
    pt_num++;
    gt->field_8 = trBx;
    gt->field_0 = trBx;
    gt->field_C = trBy;
    gt->field_4 = trBy;
    gt->field_18 = 0;
    pway->points_num = pt_num;
    return pt_num;
}

void gate_navigator_init8(struct Pathway *pway, long trAx, long trAy, long trBx, long trBy, long wp_lim, unsigned char a7)
{
    //_DK_gate_navigator_init8(pway, trAx, trAy, trBx, trBy, a6, a7);
    pway->field_0 = trAx;
    pway->field_4 = trAy;
    pway->field_8 = trBx;
    pway->points_num = 0;
    pway->field_C = trBy;
    tree_routelen = -1;
    tree_triA = triangle_findSE8(trAx, trAy);
    tree_triB = triangle_findSE8(trBx, trBy);
    tree_Ax8 = trAx;
    tree_Ay8 = trAy;
    tree_Bx8 = trBx;
    tree_By8 = trBy;
    tree_altA = get_triangle_tree_alt(tree_triA);
    tree_altB = get_triangle_tree_alt(tree_triB);
    if ((tree_triA != -1) && (tree_triB != -1))
    {
        tree_routelen = ma_triangle_route(tree_triA, tree_triB, &tree_routecost);
        if (tree_routelen != -1) {
            pway->points_num = gate_route_to_coords(trAx, trAy, trBx, trBy, tree_route, tree_routelen, pway, wp_lim);
        }
    }
}

void route_through_gates(const struct Pathway *pway, struct Path *path, long subroute)
{
    const struct Gate *ppoint;
    struct PathWayPoint *wpoint;
    long i;
    //_DK_route_through_gates(pway, path, mag); return;
    if (subroute > 16383)
        subroute = 16383;
    if (subroute < 0)
        subroute = 0;
    path->start.x = pway->field_0;
    path->start.y = pway->field_4;
    path->finish.x = pway->field_8;
    path->finish.y = pway->field_C;
    path->waypoints_num = pway->points_num;
    ppoint = &pway->points[0];
    wpoint = &path->waypoints[0];
    for (i=0; i < pway->points_num-1; i++)
    {
        if (ppoint->field_18)
        {
            wpoint->x = ppoint->field_8 - (subroute * (ppoint->field_8 - ppoint->field_0) >> 14);
            wpoint->y = ppoint->field_C - (subroute * (ppoint->field_C - ppoint->field_4) >> 14);
        } else
        {
            wpoint->x = ppoint->field_0 + (subroute * (ppoint->field_8 - ppoint->field_0) >> 14);
            wpoint->y = ppoint->field_4 + (subroute * (ppoint->field_C - ppoint->field_4) >> 14);
        }
        wpoint++;
        ppoint++;
    }
    path->waypoints[i].x = pway->field_8;
    path->waypoints[i].y = pway->field_C;
}

HOOK_DK_FUNC(triangle_findSE8)
static long triangle_findSE8(long ptfind_x, long ptfind_y)
{
    long ntri;
    long ncor;
    ntri = triangle_find8(ptfind_x, ptfind_y);
    if (ntri < 0) {
        return ntri;
    }
    for (ncor=0; ncor < 3; ncor++)
    {
        struct Point *pt;
        pt = get_triangle_point(ntri,ncor);
        if ((pt->x << 8 == ptfind_x) && (pt->y << 8 == ptfind_y))
        {
            pointed_at8(65792, 65792, &ntri, &ncor);
            return ntri;
        }
    }
    for (ncor=0; ncor < 3; ncor++)
    {
        struct Point *pt;
        pt = get_triangle_point(ntri,ncor);
        int ptA_x;
        int ptA_y;
        ptA_x = pt->x << 8;
        ptA_y = pt->y << 8;
        pt = get_triangle_point(ntri,MOD3[ncor+1]);
        int ptB_x;
        int ptB_y;
        ptB_x = pt->x << 8;
        ptB_y = pt->y << 8;
        if (LbCompareMultiplications(ptfind_y - ptA_y, ptB_x - ptA_x, ptfind_x - ptA_x, ptB_y - ptA_y) == 0)
        {
            if (LbCompareMultiplications(65792 - ptA_y, ptB_x - ptA_x, 65792 - ptA_x, ptB_y - ptA_y) > 0)
            {
                struct Triangle *tri;
                tri = get_triangle(ntri);
                return tri->tags[ncor];
            }
        }
    }
    return ntri;
}

/* TODO PATHFINDING Enable when needed
void tag_open_closed_init(void)
{
    if (tag_current >= 254)
    {
        memset(Tags, 0, sizeof(Tags));
        tag_current = 0;
    }
    tag_current+=2;
    tag_open = tag_current - 1;
}

*/

unsigned long nav_same_component(long ptAx, long ptAy, long ptBx, long ptBy)
{
    NAVIDBG(19,"F=%d Connect %03d,%03d %03d,%03d", game.play_gameturn, ptAx, ptAy, ptBx, ptBy);
    long tri1_id;
    long tri2_id;
    tri1_id = triangle_findSE8(ptAx, ptAy);
    tri2_id = triangle_findSE8(ptBx, ptBy);
    if ((tree_triA == -1) || (tree_triB == -1)) {
        ERRORLOG("triangle not found");
    }
    TbBool reg_con;
    reg_con = regions_connected(tri1_id, tri2_id);
    NAVIDBG(19,"ret %d", reg_con);
    return reg_con;
}

TbBool navigation_points_connected(struct Coord3d *pt1, struct Coord3d *pt2)
{
  return nav_same_component(pt1->x.val, pt1->y.val, pt2->x.val, pt2->y.val);
}

TbBool triangulation_border_tag(void)
{
    if (border_tags_to_current(Border, ix_Border) != ix_Border)
    {
        ERRORLOG("Some border Tags were outranged");
        return false;
    }
    return true;
}

long dest_node(long tri_id, long cor_id)
{
    long n;
    n = Triangles[tri_id].tags[cor_id];
    if (n < 0)
        return -1;
    if (!nav_rulesA2B(get_triangle_tree_alt(tri_id), get_triangle_tree_alt(n)))
        return -1;
    return n;
}

void creature_radius_set(long radius)
{
    edgelen_init();
    if ((radius < 1) || (radius > EDGEOR_COUNT)) {
        ERRORLOG("only radius 1..%d allowed, got %d",EDGEOR_COUNT,(int)radius);
        if (radius < 1) {
            radius = 1;
        } else {
            radius = EDGEOR_COUNT;
        }
    }
    EdgeFit = RadiusEdgeFit[radius];
}

static void set_nearpoint(long tri_id, long cor_id, long dstx, long dsty, long *px, long *py)
{
    static struct QuadrantOffset qdrnt_offs[] = {
       {   0,   0},{ 128, 128},{-128, 128},{   0, 128},
       { 128,-128},{ 128,   0},{ 128, 128},{ 128, 128},
       {-128,-128},{ 128,-128},{-128,   0},{-128, 128},
       {   0,-128},{ 128,-128},{-128,-128},{   0,   0},
    };

    //_DK_set_nearpoint(tri_id, cor_id, dstx, dsty, px, py);
    struct Point *pt1;
    pt1 = get_triangle_point(tri_id,cor_id);
    unsigned int tngflags;
    tngflags = 0;
    if ((LastTriangulatedMap[256 * (pt1->y-1) + (pt1->x-1)] & 0x0F) == 0x0F)
      tngflags = 0x01;
    if ((LastTriangulatedMap[256 * (pt1->y-1) + (pt1->x)]   & 0x0F) == 0x0F)
      tngflags |= 0x02;
    if ((LastTriangulatedMap[256 * (pt1->y)   + (pt1->x-1)] & 0x0F) == 0x0F)
      tngflags |= 0x04;
    if ((LastTriangulatedMap[256 * (pt1->y)   + (pt1->x)]   & 0x0F) == 0x0F)
      tngflags |= 0x08;
    struct Point *pt2;
    switch (tngflags)
    {
    case 6:
        pt2 = get_triangle_point(tri_id,MOD3[cor_id+1]);
        if ((pt2->x < pt1->x) || (pt2->y > pt1->y))
            tngflags |= 0x08;
        else
            tngflags |= 0x01;
        break;
    case 9:
        pt2 = get_triangle_point(tri_id,MOD3[cor_id+1]);
        if ((pt2->x < pt1->x) || (pt2->y > pt1->y))
            tngflags |= 0x02;
        else
            tngflags |= 0x04;
        break;
    case 15:
        ERRORLOG("solid");
        break;
    }
    *px = (pt1->x << 8) + qdrnt_offs[tngflags].x;
    *py = (pt1->y << 8) + qdrnt_offs[tngflags].y;
}
HOOK_DK_FUNC(set_nearpoint)

void nearest_search_f(long sizexy, long srcx, long srcy, long dstx, long dsty, long *px, long *py, const char *func_name)
{
    //_DK_nearest_search(sizexy, srcx, srcy, dstx, dsty, px, py); return;
    creature_radius_set(sizexy+1);
    tags_init();
    long tri1_id;
    long tri2_id;
    tri1_id = triangle_findSE8(srcx, srcy);
    tri2_id = triangle_findSE8(dstx, dsty);
    region_store_init();
    store_current_tag(tri1_id);
    region_put(tri1_id);
    if (tri2_id == tri1_id)
    {
        *px = dstx;
        *py = dsty;
        return;
    }
    long seltri_id;
    int selcor_id;
    long min_dist;
    signed int cor_id;
    seltri_id = 0;
    selcor_id = 0;
    min_dist = LONG_MAX;
    for (cor_id = 0; cor_id < 3; cor_id++)
    {
        int pt_id;
        pt_id = Triangles[tri1_id].points[cor_id];
        long diff_x;
        long diff_y;
        diff_x = ((ari_Points[pt_id].x << 8) - dstx) >> 5;
        diff_y = ((ari_Points[pt_id].y << 8) - dsty) >> 5;
        long dist;
        dist = diff_x * diff_x + diff_y * diff_y;
        if (min_dist > dist)
        {
          min_dist = dist;
          seltri_id = tri1_id;
          selcor_id = cor_id;
        }
    }
    while (1)
    {
        int regn;
        regn = region_get();
        if (regn == -1)
        {
            if ((Triangles[seltri_id].tree_alt & 0xF) == 15) {
                ERRORLOG("%s: non navigable found",func_name);
            }
            break;
        }
        struct Triangle *tri;
        tri = &Triangles[regn];
        unsigned int ncor1;
        for (ncor1=0; ncor1 < 3; ncor1++)
        {
            long ntri;
            ntri = tri->tags[ncor1];
            if ((ntri != -1) && !is_current_tag(ntri))
            {
                if ((Triangles[ntri].tree_alt & 0xF) != 15)
                {
                    if (fits_thro(regn, ncor1))
                    {
                        store_current_tag(ntri);
                        region_put(ntri);
                        if (tri2_id == ntri)
                        {
                            *px = dstx;
                            *py = dsty;
                            return;
                        }
                        unsigned int ncor2;
                        for (ncor2=0; ncor2 < 3; ncor2++)
                        {
                            int pt_id;
                            long diff_x;
                            long diff_y;
                            pt_id = Triangles[ntri].points[ncor2];
                            diff_x = ((ari_Points[pt_id].x << 8) - dstx) >> 5;
                            diff_y = ((ari_Points[pt_id].y << 8) - dsty) >> 5;
                            int dist;
                            dist = diff_x * diff_x + diff_y * diff_y;
                            if (min_dist > dist)
                            {
                              min_dist = dist;
                              seltri_id = ntri;
                              selcor_id = ncor2;
                            }
                        }
                    }
                }
            }
        }
    }
    set_nearpoint(seltri_id, selcor_id, dstx, dsty, px, py);
}

long cost_to_start(long tri_idx)
{
    long long len_x;
    long long len_y;
    long long mincost;
    long long newcost;
    struct Point *pt;
    struct Triangle *tri;
    long i;
    mincost = 16777215;
    tri = get_triangle(tri_idx);
    for (i=0; i < 3; i++)
    {
        pt = point_get(tri->points[i]);
        len_x = ((tree_Ax8 >> 8) - (long)(pt->x));
        len_y = ((tree_Ay8 >> 8) - (long)(pt->y));
        newcost = len_x*len_x+len_y*len_y;
        if (newcost < mincost)
            mincost = newcost;
    }
    return mincost;
}

/**
 *
 * @param pos_x
 * @param pos_y
 * @param retpos_x
 * @param retpos_y
 */
HOOK_DK_FUNC(pointed_at8)
long pointed_at8(long pos_x, long pos_y, long *ret_tri, long *ret_pt)
{
    //TODO PATHFINDING triangulate_area sub-sub-sub-function, verify
    long npt;
    long ntri;
    int pt_id;
    int ptBx;
    int ptBy;
    int ptAx;
    int ptAy;

    ntri = *ret_tri;
    npt = *ret_pt;
    pt_id = Triangles[ntri].points[npt];
    ptAx = (ari_Points[pt_id].x << 8) - pos_x;
    ptAy = (ari_Points[pt_id].y << 8) - pos_y;
    pt_id =  Triangles[ntri].points[MOD3[npt+2]];
    ptBx = (ari_Points[pt_id].x << 8) - pos_x;
    ptBy = (ari_Points[pt_id].y << 8) - pos_y;
    char pt_rel;
    pt_rel = LbCompareMultiplications(ptBy, ptAx, ptBx, ptAy) > 0;
    char prev_rel;
    unsigned long k;
    k = 0;
    while ( 1 )
    {
        prev_rel = pt_rel;
        pt_id = Triangles[ntri].points[MOD3[npt+1]];
        ptBy = (ari_Points[pt_id].y << 8) - pos_y;
        ptBx = (ari_Points[pt_id].x << 8) - pos_x;
        pt_rel = LbCompareMultiplications(ptBy, ptAx, ptBx, ptAy) > 0;

        if ( prev_rel && !pt_rel )
        {
            *ret_tri = ntri;
            *ret_pt = npt;
            return MOD3[npt+1];
        }
        long tri_id;
        int tri_link;
        tri_id = Triangles[ntri].tags[npt];
        if (tri_id < 0) {
            break;
        }
        tri_link = link_find(tri_id, ntri);
        if (tri_link < 0) {
            ERRORLOG("no tri link");
            break;
        }
        npt = MOD3[tri_link+1];
        ntri = tri_id;
        k++;
        if (k > TRIANLGLES_COUNT) {
            ERRORLOG("Infinite loop detected");
            break;
        }
    }
    return -1;
}

TbBool triangle_check_and_add_navitree_fwd(long ttri)
{
    struct Triangle *tri;
    tri = get_triangle(ttri);
    if (triangle_is_invalid(tri)) {
        ERRORLOG("invalid triangle received, no %d",(int)ttri);
        return false;
    }
    long n;
    long nskipped;
    n = 0;
    nskipped = 0;
    long i;
    long k;
    for (i = 0; i < 3; i++)
    {
        k = tri->tags[i];
        if (!is_current_tag(k))
        {
            if ( fits_thro(ttri, n) )
            {
                long ttri_alt;
                long k_alt;
                ttri_alt = get_triangle_tree_alt(ttri);
                k_alt = get_triangle_tree_alt(k);
                if ((ttri_alt != -1) && (k_alt != -1))
                {
                    long mvcost;
                    long navrule;
                    navrule = nav_rulesA2B(k_alt, ttri_alt);
                    if (navrule)
                    {
                        mvcost = cost_to_start(k);
                        if (navrule == 2)
                            mvcost *= 16;
                        if (!navitree_add(k,ttri,mvcost))
                            nskipped++;
                    }
                }
            }
        }
        n++;
    }
    if (nskipped != 0) {
        NAVIDBG(6,"navigate heap full, %ld points ignored",nskipped);
        return false;
    }
    return true;
}

TbBool triangle_check_and_add_navitree_bak(long ttri)
{
    struct Triangle *tri;
    tri = get_triangle(ttri);
    if (triangle_is_invalid(tri)) {
        ERRORLOG("invalid triangle received");
        return false;
    }
    long n;
    long nskipped;
    n = 0;
    nskipped = 0;
    long i;
    long k;
    for (i = 0; i < 3; i++)
    {
        k = tri->tags[i];
        if (!is_current_tag(k))
        {
            long ttri_alt;
            long k_alt;
            ttri_alt = get_triangle_tree_alt(ttri);
            k_alt = get_triangle_tree_alt(k);
            if ((ttri_alt != -1) && (k_alt != -1))
            {
                long mvcost;
                long navrule;
                navrule = nav_rulesA2B(ttri_alt, k_alt);
                if (navrule)
                {
                    mvcost = cost_to_start(k);
                    if (navrule == 2)
                        mvcost *= 16;
                    if (!navitree_add(k,ttri,mvcost))
                        nskipped++;
                }
            }
        }
        n++;
    }
    if (nskipped != 0) {
        NAVIDBG(6,"navigate heap full, %ld points ignored",nskipped);
        return false;
    }
    return true;
}

/**
 * Triangulates a forward route to ttriB from ttriA.
 * @param ttriA Beginning region triangle.
 * @param ttriB Final region triangle.
 * @param route Output array of size TRIANLGLES_COUNT where route is copied.
 * @param routecost Output integer where the tree route cost is returned.
 * @return Amount of points copied into the route array, or -1 on routing failure.
 */
long triangle_route_do_fwd(long ttriA, long ttriB, long *route, long *routecost)
{
    NAVIDBG(19,"Starting");
    tags_init();
    if ((ix_Border < 0) || (ix_Border >= BORDER_LENGTH))
    {
        ERRORLOG("Border overflow");
        ix_Border = BORDER_LENGTH-1;
    }
    triangulation_border_tag();

    naviheap_init();
    // Add final region to navigation tree
    if (!navitree_add(ttriB, ttriB, 1)) {
        ERRORLOG("Navigate heap full after cleaning");
        return -1;
    }
    // Keep adding sibling regions until we are in beginning region
    // Do two of them at a time
    while (ttriA != naviheap_top())
    {
        long ttriH1;
        long ttriH2;
        if (naviheap_empty())
            break;
        ttriH1 = naviheap_remove();
        if (naviheap_empty())
        {
            ttriH2 = -1;
        } else
        {
            ttriH2 = naviheap_top();
            if (ttriH2 == ttriA)
                break;
            naviheap_remove();
        }
        if (ttriH1 != -1)
        {
            triangle_check_and_add_navitree_fwd(ttriH1);
        }
        if (ttriH2 != -1)
        {
            triangle_check_and_add_navitree_fwd(ttriH2);
        }
    }
    NAVIDBG(19,"Almost finished");
    if (naviheap_empty()) {
        // The beginning region was never reached
        return -1;
    }
    long i;
    i = copy_tree_to_route(ttriA, ttriB, route, TRIANLGLES_COUNT+1);
    if (i < 0) {
        erstat_inc(ESE_BadRouteTree);
        ERRORLOG("route length overflow");
    }
    return i;
}

/**
 * Triangulates a backward route to ttriB from ttriA.
 * @param ttriA Beginning region triangle.
 * @param ttriB Final region triangle.
 * @param route Output array of size TRIANLGLES_COUNT where route is copied.
 * @param routecost Output integer where the tree route cost is returned.
 * @return Amount of points copied into the route array, or -1 on routing failure.
 * @note This function should differ from triangle_route_do_bak() in only one line
 */
long triangle_route_do_bak(long ttriA, long ttriB, long *route, long *routecost)
{
    NAVIDBG(19,"Starting");
    tags_init();
    if ((ix_Border < 0) || (ix_Border >= BORDER_LENGTH))
    {
        ERRORLOG("Border overflow");
        ix_Border = BORDER_LENGTH-1;
    }
    triangulation_border_tag();

    naviheap_init();
    // Add final region to navigation tree
    if (!navitree_add(ttriB, ttriB, 1)) {
        ERRORLOG("Navigate heap full after cleaning");
        return -1;
    }
    // Keep adding sibling regions until we are in beginning region
    // Do two of them at a time
    while (ttriA != naviheap_top())
    {
        long ttriH1;
        long ttriH2;
        if (naviheap_empty())
            break;
        ttriH1 = naviheap_remove();
        if (naviheap_empty())
        {
            ttriH2 = -1;
        } else
        {
            ttriH2 = naviheap_top();
            if (ttriH2 == ttriA)
                break;
            naviheap_remove();
        }
        if (ttriH1 != -1)
        {
            triangle_check_and_add_navitree_bak(ttriH1);
        }
        if (ttriH2 != -1)
        {
            triangle_check_and_add_navitree_bak(ttriH2);
        }
    }
    NAVIDBG(19,"Almost finished");
    if (naviheap_empty()) {
        // The beginning region was never reached
        return -1;
    }
    long i;
    i = copy_tree_to_route(ttriA, ttriB, route, TRIANLGLES_COUNT+1);
    if (i < 0) {
        erstat_inc(ESE_BadRouteTree);
        ERRORLOG("route length overflow");
    }
    return i;
}

/**
 * Prepares a tree route for reaching ttriB from ttriA.
 * @param ttriA Beginning region triangle.
 * @param ttriB Final region triangle.
 * @param routecost Pointer where the tree route cost is returned.
 * @return
 */
long ma_triangle_route(long ttriA, long ttriB, long *routecost)
{
    long len_fwd;
    long len_bak;
    long par_fwd;
    long par_bak;
    long rcost_fwd;
    long rcost_bak;
    long tx;
    long ty;
    long i;
    // We need to make testing system for routing, then fix the rewritten code
    // and compare results with the original code.
    // Forward route
    NAVIDBG(19,"Making forward route");
    rcost_fwd = 0;
    len_fwd = triangle_route_do_fwd(ttriA, ttriB, route_fwd, &rcost_fwd);
    if (len_fwd == -1)
    {
        NAVIDBG(19,"No forward route");
        return -1;
    }
    route_to_path(tree_Ax8, tree_Ay8, tree_Bx8, tree_By8, route_fwd, len_fwd, &fwd_path, &par_fwd);
    tx = tree_Ax8;
    ty = tree_Ay8;
    tree_Ax8 = tree_Bx8;
    tree_Ay8 = tree_By8;
    tree_Bx8 = tx;
    tree_By8 = ty;
    // Backward route
    NAVIDBG(19,"Making backward route");
    rcost_bak = 0;
    len_bak = triangle_route_do_bak(ttriB, ttriA, route_bak, &rcost_bak);
    if (len_bak == -1)
    {
        NAVIDBG(19,"No backward route");
        return -1;
    }
    route_to_path(tree_Ax8, tree_Ay8, tree_Bx8, tree_By8, route_bak, len_bak, &bak_path, &par_bak);
    tx = tree_Ax8;
    ty = tree_Ay8;
    tree_Ax8 = tree_Bx8;
    tree_Ay8 = tree_By8;
    tree_Bx8 = tx;
    tree_By8 = ty;
    // Select a route
    NAVIDBG(19,"Selecting route");
    if (par_fwd < par_bak)
    {
        for (i=0; i <= sizeof(tree_route)/sizeof(tree_route[0]); i++)
        {
             tree_route[i] = route_fwd[i];
        }
        *routecost = rcost_fwd;
        return len_fwd;
    } else
    {
        for (i=0; i <= len_bak; i++)
        {
             tree_route[i] = route_bak[len_bak-i];
        }
        *routecost = rcost_bak;
        return len_bak;
    }
}

void edgelen_init(void)
{
    //_DK_edgelen_init();
    if (edgelen_initialised)
        return;
    edgelen_initialised = true;
    int i;
    // Fill edge values
    EdgeFit = RadiusEdgeFit[0];
    for (i=0; i < EDGEFIT_LEN; i++)
    {
        EdgeFit[i] = 0;
    }
    EdgeFit = RadiusEdgeFit[1];
    for (i=0; i < EDGEFIT_LEN; i++)
    {
        EdgeFit[i] = 1;
    }
    EdgeFit = RadiusEdgeFit[2];
    for (i=0; i < EDGEFIT_LEN; i++)
    {
        EdgeFit[i] = ((i & 0x2A) == 0x2A);
    }
    EdgeFit = RadiusEdgeFit[3];
    for (i=0; i < EDGEFIT_LEN; i++)
    {
        EdgeFit[i] = ((i & 0x3F) == 0x3F);
    }
    // Reset pointer
    EdgeFit = RadiusEdgeFit[1];
}

TbBool ariadne_creature_reached_position(const struct Thing *thing, const struct Coord3d *pos)
{
    if (thing->mappos.x.val != pos->x.val)
    return false;
    if (thing->mappos.y.val != pos->y.val)
        return false;
    return true;
}

long ariadne_creature_blocked_by_wall_at(struct Thing *thing, const struct Coord3d *pos)
{
    struct Coord3d mvpos;
    long zmem;
    long ret;
    zmem = thing->mappos.z.val;
    mvpos.x.val = pos->x.val;
    mvpos.y.val = pos->y.val;
    mvpos.z.val = pos->z.val;
    mvpos.z.val = get_floor_height_under_thing_at(thing, &thing->mappos);
    thing->mappos.z.val = mvpos.z.val;
    ret = creature_cannot_move_directly_to(thing, &mvpos);
    thing->mappos.z.val = zmem;
    return ret;
}

void ariadne_pull_out_waypoint(const struct Thing *thing, const struct Ariadne *arid, long wpoint_id, struct Coord3d *pos)
{
    const struct Coord2d *wp;
    long size_radius;
    //_DK_ariadne_pull_out_waypoint(thing, arid, wpoint_id, pos); return;
    if ((wpoint_id < 0) || (wpoint_id >= ARID_WAYPOINTS_COUNT))
    {
        pos->x.val = 0;
        pos->y.val = 0;
        return;
    }
    wp = &arid->waypoints[wpoint_id];
    if (arid->total_waypoints-wpoint_id == 1)
    {
        pos->x.val = wp->x.val;
        pos->y.val = wp->y.val;
        return;
    }
    size_radius = (thing_nav_sizexy(thing) >> 1);
    pos->x.val = wp->x.val + 1;
    pos->x.stl.pos = 0;
    pos->y.val = wp->y.val + 1;
    pos->y.stl.pos = 0;

    // looks like 0 and 255 are special values - check where they are set
    if (wp->x.stl.pos == 255)
    {
        pos->x.val -= size_radius+1;
    } else
    if (wp->x.stl.pos != 0)
    {
        pos->x.val += size_radius;
    }

    if (wp->y.stl.pos == 255)
    {
        pos->y.val -= size_radius+1;
    } else
    if (wp->y.stl.pos != 0)
    {
        pos->y.val += size_radius;
    }
}

/** Initializes current waypoint position in Ariadne based on the current waypoint index.
 *
 * @param thing The thing which is moving.
 * @param arid The Ariadne which is being changed.
 */
void ariadne_init_current_waypoint(const struct Thing *thing, struct Ariadne *arid)
{
    ariadne_pull_out_waypoint(thing, arid, arid->current_waypoint, &arid->current_waypoint_pos);
    arid->current_waypoint_pos.z.val = get_thing_height_at(thing, &arid->current_waypoint_pos);
    arid->field_62 = get_2d_distance(&thing->mappos, &arid->current_waypoint_pos);
}

long angle_to_quadrant(long angle)
{
    return ((angle + LbFPMath_PI/4) & 0x600u) >> 9;
}

long ariadne_wallhug_angle_valid(struct Thing *thing, struct Ariadne *arid, long angle)
{
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, angle);
    pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, angle);
    pos.z.val = subtile_coord(1,0);
    return (ariadne_creature_blocked_by_wall_at(thing, &pos) == 0);
}

long ariadne_get_wallhug_angle(struct Thing *thing, struct Ariadne *arid)
{
    long whangle;
    if (arid->field_20 == 1)
    {
        whangle = (LbFPMath_PI/2) * ((angle_to_quadrant(thing->move_angle_xy) - 1) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = (LbFPMath_PI/2) * (angle_to_quadrant(thing->move_angle_xy) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = (LbFPMath_PI/2) * ((angle_to_quadrant(thing->move_angle_xy) + 1) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = (LbFPMath_PI/2) * ((angle_to_quadrant(thing->move_angle_xy) + 2) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
    } else
    if (arid->field_20 == 2)
    {
        whangle = (LbFPMath_PI/2) * ((angle_to_quadrant(thing->move_angle_xy) + 1) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = (LbFPMath_PI/2) * (angle_to_quadrant(thing->move_angle_xy) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = (LbFPMath_PI/2) * ((angle_to_quadrant(thing->move_angle_xy) - 1) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = (LbFPMath_PI/2) * ((angle_to_quadrant(thing->move_angle_xy) + 2) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
    }
    return -1;
}

void ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(struct Thing *thing, struct Ariadne *arid, long inangle, short *rangle, unsigned char *rflag)
{
    //_DK_ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, inangle, rangle, rflag); return;
    struct Coord3d bkp_mappos;
    bkp_mappos = thing->mappos;
    int inangle_oneaxis;
    int bkp_angle_xy;
    int bkp_fld20;
    int bkp_speed;
    bkp_angle_xy = thing->move_angle_xy;
    bkp_speed = arid->move_speed;
    bkp_fld20 = arid->field_20;
    int angle_beg;
    int nfld20;
    int angle_end;
    if (inangle == 0)
    {
        angle_beg = 3*LbFPMath_PI/2;
        nfld20 = 2;
        angle_end = LbFPMath_PI/2;
        inangle_oneaxis = 1;
    } else
    if (inangle == LbFPMath_PI/2)
    {
        angle_beg = 0;
        nfld20 = 2;
        angle_end = LbFPMath_PI;
        inangle_oneaxis = 1;
    } else
    if (inangle == LbFPMath_PI)
    {
        angle_beg = LbFPMath_PI/2;
        nfld20 = 2;
        angle_end = 3*LbFPMath_PI/2;
        inangle_oneaxis = 1;
    } else
    if (inangle == 3*LbFPMath_PI/2)
    {
        angle_beg = LbFPMath_PI;
        nfld20 = 2;
        angle_end = 0;
        inangle_oneaxis = 1;
    } else
    {
        NAVIDBG(9,"Unsupported inangle %d",(int)inangle);
        angle_beg = 0;
        nfld20 = 0;
        angle_end = 0;
        inangle_oneaxis = 0;
    }
    arid->move_speed = 256;
    int whsteps;
    int whgot2;
    int whgot1;
    int size_steps;
    size_steps = thing_nav_sizexy(thing) >> 9;
    thing->move_angle_xy = angle_beg;
    size_steps += 2;
    whsteps = size_steps;
    whgot1 = size_steps;
    whgot2 = size_steps;
    arid->field_20 = nfld20;
    struct Coord3d pos;
    int i;
    long hug_angle;
    for (i = 0; i < whsteps; i++)
    {
        hug_angle = ariadne_get_wallhug_angle(thing, arid);
        if (hug_angle != -1)
        {
            if (hug_angle == inangle) {
                whgot1 = i;
                break;
            }
            pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, hug_angle);
            pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, hug_angle);
            pos.z.val = get_thing_height_at(thing, &pos);
            if (ariadne_check_forward_for_wallhug_gap(thing, arid, &pos, hug_angle)) {
                whgot1 = i;
                break;
            }
            thing->mappos = pos;
        }
    }
    thing->move_angle_xy = angle_end;
    arid->field_20 = inangle_oneaxis;
    thing->mappos = bkp_mappos;
    for (i = 0; i < whsteps; i++)
    {
        hug_angle = ariadne_get_wallhug_angle(thing, arid);
        if (hug_angle != -1)
        {
            if (hug_angle == inangle) {
                whgot2 = i;
                break;
            }
            pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, hug_angle);
            pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, hug_angle);
            pos.z.val = get_thing_height_at(thing, &pos);
            if (ariadne_check_forward_for_wallhug_gap(thing, arid, &pos, hug_angle)) {
                whgot2 = i;
                break;
            }
            thing->mappos = pos;
        }
    }
    thing->move_angle_xy = bkp_angle_xy;
    thing->mappos = bkp_mappos;
    arid->move_speed = bkp_speed;
    arid->field_20 = bkp_fld20;
    if (whgot2 > whgot1)
    {
        *rangle = angle_beg;
        *rflag = nfld20;
    } else
    if (whgot2 >= whgot1)
    {
        *rangle = angle_beg;
        *rflag = nfld20;
    } else
    {
        *rangle = angle_end;
        *rflag = inangle_oneaxis;
    }
}

long ariadne_get_starting_angle_and_side_of_wallhug(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos, short *rangle, unsigned char *rflag)
{
    TbBool nxdelta_x_neg;
    TbBool nxdelta_y_neg;
    TbBool crdelta_x_neg;
    TbBool crdelta_y_neg;
    crdelta_x_neg = (thing->mappos.x.val - (long)pos->x.val) <= 0;
    crdelta_y_neg = (thing->mappos.y.val - (long)pos->y.val) <= 0;
    nxdelta_x_neg = (thing->mappos.x.val - (long)arid->current_waypoint_pos.x.val) <= 0;
    nxdelta_y_neg = (thing->mappos.y.val - (long)arid->current_waypoint_pos.y.val) <= 0;
    int axis_closer;
    int nav_radius;
    axis_closer = abs(thing->mappos.x.val - (long)arid->current_waypoint_pos.x.val) < abs(thing->mappos.y.val - (long)arid->current_waypoint_pos.y.val);
    nav_radius = thing_nav_sizexy(thing) / 2;
    MapCoord cur_pos_y_beg;
    MapCoord cur_pos_y_end;
    MapCoord cur_pos_x_beg;
    MapCoord cur_pos_x_end;
    cur_pos_x_beg = thing->mappos.x.val - nav_radius;
    cur_pos_x_end = thing->mappos.x.val + nav_radius;
    cur_pos_y_beg = thing->mappos.y.val - nav_radius;
    cur_pos_y_end = thing->mappos.y.val + nav_radius;
    int wp_num;
    MapCoord wp_x;
    MapCoord wp_y;
    wp_num = arid->current_waypoint;
    wp_x = arid->waypoints[wp_num].x.val;
    wp_y = arid->waypoints[wp_num].y.val;
    unsigned long blk_flags;
    blk_flags = ariadne_get_blocked_flags(thing, pos);
    if ((blk_flags & 0x01) != 0)
    {
        if ((wp_y >= cur_pos_y_beg) && (wp_y <= cur_pos_y_end))
        {
            if (nxdelta_x_neg)
                ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, LbFPMath_PI/2, rangle, rflag);
            else
                ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, 3*LbFPMath_PI/2, rangle, rflag);
        } else
        {
            *rangle = blocked_x_hug_start[crdelta_x_neg][nxdelta_y_neg].field_0;
            *rflag = blocked_x_hug_start[crdelta_x_neg][nxdelta_y_neg].field_2;
        }
        return 1;
    }
    if ((blk_flags & 0x02) != 0)
    {
        if ((wp_x >= cur_pos_x_beg) && (wp_x <= cur_pos_x_end))
        {
            if (nxdelta_y_neg)
                ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, LbFPMath_PI, rangle, rflag);
            else
                ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, 0, rangle, rflag);
        } else
        {
            *rangle = blocked_y_hug_start[crdelta_y_neg][nxdelta_x_neg].field_0;
            *rflag = blocked_y_hug_start[crdelta_y_neg][nxdelta_x_neg].field_2;
        }
        return 1;
    }
    if ((blk_flags & 0x04) != 0)
    {
        *rangle = blocked_xy_hug_start[crdelta_y_neg][crdelta_x_neg][axis_closer].field_0;
        *rflag = blocked_xy_hug_start[crdelta_y_neg][crdelta_x_neg][axis_closer].field_2;
        return 1;
    }
    return 0;
}

AriadneReturn ariadne_init_wallhug(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos)
{
    if (arid->move_speed <= 0) {
        ERRORLOG("Ariadne Speed not positive");
    }
    if (!ariadne_get_starting_angle_and_side_of_wallhug(thing, arid, pos, &arid->wallhug_angle, &arid->field_20))
    {
        arid->pos_12.x.val = thing->mappos.x.val;
        arid->pos_12.y.val = thing->mappos.y.val;
        arid->pos_12.z.val = thing->mappos.z.val;
        arid->update_state = AridUpSt_OnLine;
        return AridRet_OK;
    }
    arid->update_state = AridUpSt_Wallhug;
    arid->pos_12.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, arid->wallhug_angle);
    arid->pos_12.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, arid->wallhug_angle);
    arid->pos_12.z.val = get_thing_height_at(thing, &arid->pos_12);
    arid->pos_18 = thing->mappos;
    arid->field_24 = arid->wallhug_angle;
    if (ariadne_check_forward_for_wallhug_gap(thing, arid, &arid->pos_12, arid->wallhug_angle))
    {
        arid->pos_53.x.val = arid->pos_12.x.val;
        arid->pos_53.y.val = arid->pos_12.y.val;
        arid->pos_53.z.val = arid->pos_12.z.val;
        arid->update_state = AridUpSt_Manoeuvre;
        arid->manoeuvre_state = AridUpSStM_Unkn2;
        return AridRet_OK;
    }
    long cannot_move;
    {
        MapCoord tng_z_mem;
        tng_z_mem = thing->mappos.z.val;
        struct Coord3d mvpos;
        mvpos.x.val = arid->pos_12.x.val;
        mvpos.y.val = arid->pos_12.y.val;
        mvpos.z.val = get_floor_height_under_thing_at(thing, &thing->mappos);
        thing->mappos.z.val = mvpos.z.val;
        cannot_move = creature_cannot_move_directly_to(thing, &mvpos);
        thing->mappos.z.val = tng_z_mem;
    }
    if ( cannot_move )
    {
        struct Coord3d pos2;
        ariadne_push_position_against_wall(thing, &arid->pos_12, &pos2);
        arid->pos_53.x.val = pos2.x.val;
        arid->pos_53.y.val = pos2.y.val;
        arid->pos_53.z.val = pos2.z.val;
        arid->pos_59.x.val = arid->pos_12.x.val;
        arid->pos_59.y.val = arid->pos_12.y.val;
        arid->pos_59.z.val = arid->pos_12.z.val;
        arid->update_state = AridUpSt_Manoeuvre;
        arid->manoeuvre_state = AridUpSStM_Unkn1;
        return AridRet_OK;
    }
    return AridRet_OK;
}

void clear_wallhugging_path(struct Navigation *navi)
{
    navi->navstate = 1;
    navi->pos_final.x.val = subtile_coord_center(map_subtiles_x/2);
    navi->pos_final.y.val = subtile_coord_center(map_subtiles_y/2);
    navi->pos_final.z.val = subtile_coord(1,0);
    navi->field_1[2] = 0;
    navi->field_1[1] = 0;
    navi->field_4 = 0;
}

void initialise_wallhugging_path_from_to(struct Navigation *navi, struct Coord3d *mvstart, struct Coord3d *mvend)
{
    navi->navstate = 1;
    navi->pos_final.x.val = mvend->x.val;
    navi->pos_final.y.val = mvend->y.val;
    navi->pos_final.z.val = mvend->z.val;
    navi->field_1[2] = 0;
    navi->field_1[1] = 0;
    navi->field_4 = 0;
}

long ariadne_get_blocked_flags(struct Thing *thing, const struct Coord3d *pos)
{
    struct Coord3d lpos;
    unsigned long blkflags;
    lpos.x.val = pos->x.val;
    lpos.y.val = thing->mappos.y.val;
    lpos.z.val = thing->mappos.z.val;
    blkflags = 0;
    if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
        blkflags |= 0x01;
    lpos.x.val = thing->mappos.x.val;
    lpos.y.val = pos->y.val;
    lpos.z.val = thing->mappos.z.val;
    if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
        blkflags |= 0x02;
    if (blkflags == 0)
    {
        lpos.x.val = pos->x.val;
        lpos.y.val = pos->y.val;
        lpos.z.val = thing->mappos.z.val;
        if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
          blkflags |= 0x04;
    }
    return blkflags;
}

TbBool blocked_by_door_at(struct Thing *thing, struct Coord3d *pos, unsigned long blk_flags)
{
    long radius;
    long start_x;
    long end_x;
    long start_y;
    long end_y;
    long stl_x;
    long stl_y;

    radius = thing_nav_sizexy(thing) >> 1;
    start_x = ((long)pos->x.val - radius) / 256;
    end_x = ((long)pos->x.val + radius) / 256;
    start_y = ((long)pos->y.val - radius) / 256;
    end_y = ((long)pos->y.val + radius) / 256;
    if ((blk_flags & 0x01) != 0)
    {
        stl_x = end_x;
        if (thing->mappos.x.val >= pos->x.val)
            stl_x = start_x;
        if (end_y >= start_y)
        {
            for (stl_y = start_y; stl_y <= end_y; stl_y++)
            {
                if (subtile_is_door(stl_x, stl_y))
                    return true;
            }
        }
    }
    if ((blk_flags & 0x02) != 0)
    {
        stl_y = end_y;
        if (thing->mappos.y.val >= pos->y.val)
              stl_y = start_y;
        if (end_x >= start_x)
        {
            for (stl_x = start_x; stl_x <= end_x; stl_x++)
            {
                if (subtile_is_door(stl_x, stl_y))
                    return true;
            }
        }
    }
    return false;
}

long ariadne_push_position_against_wall(struct Thing *thing, const struct Coord3d *pos1, struct Coord3d *pos_out)
{
    struct Coord3d lpos;
    long radius;
    unsigned long blk_flags;
    blk_flags = ariadne_get_blocked_flags(thing, pos1);
    radius = thing_nav_sizexy(thing) >> 1;
    lpos.x.val = pos1->x.val;
    lpos.y.val = pos1->y.val;
    lpos.z.val = 0;

    if ((blk_flags & 0x01) != 0)
    {
      if (pos1->x.val >= thing->mappos.x.val)
      {
          lpos.x.val = thing->mappos.x.val + radius;
          lpos.x.stl.pos = 255;
          lpos.x.val -= radius;
      } else
      {
          lpos.x.val = thing->mappos.x.val - radius;
          lpos.x.stl.pos = 0;
          lpos.x.val += radius;
      }
      lpos.z.val = get_thing_height_at(thing, &lpos);
    }
    if ((blk_flags & 0x02) != 0)
    {
      if (pos1->y.val >= thing->mappos.y.val)
      {
        lpos.y.val = thing->mappos.y.val + radius;
        lpos.y.stl.pos = 255;
        lpos.y.val -= radius;
      } else
      {
        lpos.y.val = thing->mappos.y.val - radius;
        lpos.y.stl.pos = 0;
        lpos.y.val += radius;
      }
      lpos.z.val = get_thing_height_at(thing, &lpos);
    }
    if ((blk_flags & 0x04) != 0)
    {
      if (pos1->x.val >= thing->mappos.x.val)
      {
          lpos.x.val = thing->mappos.x.val + radius;
          lpos.x.stl.pos = 255;
          lpos.x.val -= radius;
      } else
      {
          lpos.x.val = thing->mappos.x.val - radius;
          lpos.x.stl.pos = 0;
          lpos.x.val += radius;
      }
      if (pos1->y.val >= thing->mappos.y.val)
      {
          lpos.y.val = thing->mappos.y.val + radius;
          lpos.y.stl.pos = 255;
          lpos.y.val -= radius;
      }
      else
      {
          lpos.y.val = thing->mappos.y.val - radius;
          lpos.y.stl.pos = 0;
          lpos.y.val += radius;
      }
      lpos.z.val = get_thing_height_at(thing, &lpos);
    }
    pos_out->x.val = lpos.x.val;
    pos_out->y.val = lpos.y.val;
    pos_out->z.val = lpos.z.val;
    return blk_flags;
}

long ariadne_init_movement_to_current_waypoint(struct Thing *thing, struct Ariadne *arid)
{
    struct Coord3d requested_pos;
    struct Coord3d fixed_pos;
    long angle;
    long delta_x;
    long delta_y;
    unsigned long blk_flags;
    TRACE_THING(thing);
    angle = get_angle_xy_to(&thing->mappos, &arid->current_waypoint_pos);
    delta_x = distance_with_angle_to_coord_x(arid->move_speed, angle);
    delta_y = distance_with_angle_to_coord_y(arid->move_speed, angle);
    requested_pos.x.val = (long)thing->mappos.x.val + delta_x;
    requested_pos.y.val = (long)thing->mappos.y.val + delta_y;
    requested_pos.z.val = get_thing_height_at(thing, &requested_pos);
    if (!ariadne_creature_blocked_by_wall_at(thing, &requested_pos))
    {
        arid->update_state = AridUpSt_OnLine;
        return 1;
    }
    blk_flags = ariadne_get_blocked_flags(thing, &requested_pos);
    if (blocked_by_door_at(thing, &requested_pos, blk_flags))
    {
        arid->update_state = AridUpSt_OnLine;
        return 1;
    }
    ariadne_push_position_against_wall(thing, &requested_pos, &fixed_pos);
    if ( (((blk_flags & 0x01) != 0) && (thing->mappos.x.val == fixed_pos.x.val))
      || (((blk_flags & 0x02) != 0) && (thing->mappos.y.val == fixed_pos.y.val)) )
    {
        ariadne_init_wallhug(thing, arid, &requested_pos);
        arid->field_22 = 1;
        return 1;
    }
    arid->pos_53.x.val = fixed_pos.x.val;
    arid->pos_53.y.val = fixed_pos.y.val;
    arid->pos_53.z.val = fixed_pos.z.val;
    arid->pos_59.x.val = requested_pos.x.val;
    arid->pos_59.y.val = requested_pos.y.val;
    arid->pos_59.z.val = requested_pos.z.val;
    arid->pos_12.x.val = fixed_pos.x.val;
    arid->pos_12.y.val = fixed_pos.y.val;
    arid->pos_12.z.val = fixed_pos.z.val;
    arid->update_state = AridUpSt_Manoeuvre;
    arid->manoeuvre_state = AridUpSStM_Unkn1;
    return 1;
}

long ariadne_creature_can_continue_direct_line_to_waypoint(struct Thing *thing, struct Ariadne *arid, long speed)
{
    long angle;
    angle = get_angle_xy_to(&thing->mappos, &arid->current_waypoint_pos);
    struct Coord3d pos_dlim;
    pos_dlim.x.val = thing->mappos.x.val;
    pos_dlim.y.val = thing->mappos.y.val;
    pos_dlim.z.val = thing->mappos.z.val;
    pos_dlim.x.val += distance_with_angle_to_coord_x(speed, angle);
    pos_dlim.y.val += distance_with_angle_to_coord_y(speed, angle);
    pos_dlim.z.val = get_thing_height_at(thing, &pos_dlim);
    struct Coord3d pos_xlim;
    struct Coord3d pos_ylim;
    {
        MapCoord coord_fix;
        coord_fix = thing->mappos.x.val;
        if (pos_dlim.x.val > thing->mappos.x.val) {
            coord_fix += speed;
        } else
        if (pos_dlim.x.val < thing->mappos.x.val) {
            coord_fix -= speed;
        }
        pos_xlim.x.val = coord_fix;
        pos_xlim.y.val = thing->mappos.y.val;
        pos_xlim.z.val = get_thing_height_at(thing, &pos_xlim);
        coord_fix = thing->mappos.y.val;
        if (pos_dlim.y.val > thing->mappos.y.val) {
            coord_fix += speed;
        } else
        if (pos_dlim.y.val < thing->mappos.y.val) {
            coord_fix -= speed;
        }
        pos_ylim.x.val = thing->mappos.x.val;
        pos_ylim.y.val = coord_fix;
        pos_ylim.z.val = get_thing_height_at(thing, &pos_ylim);
    }
    if (!ariadne_creature_blocked_by_wall_at(thing, &pos_xlim))
    {
        if (!ariadne_creature_blocked_by_wall_at(thing, &pos_ylim))
        {
            if (!ariadne_creature_blocked_by_wall_at(thing, &pos_dlim)) {
                return 1;
            }
        }
    }
    return 0;
}

/**
 * Prepares a creature route in which targed is already reached.
 *
 * @param thing
 * @param arid
 * @param srcpos
 * @param dstpos
 * @return
 */
AriadneReturn ariadne_prepare_creature_route_target_reached(const struct Thing *thing, struct Ariadne *arid, const struct Coord3d *srcpos, const struct Coord3d *dstpos)
{
    arid->startpos.x.val = srcpos->x.val;
    arid->startpos.y.val = srcpos->y.val;
    arid->startpos.z.val = srcpos->z.val;
    arid->endpos.x.val = dstpos->x.val;
    arid->endpos.y.val = dstpos->y.val;
    arid->endpos.z.val = dstpos->z.val;
    arid->waypoints[0].x.val = srcpos->x.val;
    arid->waypoints[0].y.val = srcpos->y.val;
    arid->current_waypoint_pos.x.val = srcpos->x.val;
    arid->current_waypoint_pos.y.val = srcpos->y.val;
    arid->current_waypoint_pos.z.val = srcpos->z.val;
    arid->current_waypoint = 0;
    arid->pos_12.x.val = thing->mappos.x.val;
    arid->pos_12.y.val = thing->mappos.y.val;
    arid->pos_12.z.val = thing->mappos.z.val;
    arid->stored_waypoints = 1;
    arid->total_waypoints = 1;
    arid->route_flags = 0;
    return AridRet_OK;
}

/**
 * Prepares creature route from source to destination position.
 * @param thing
 * @param arid
 * @param srcpos
 * @param dstpos
 * @param speed
 * @param flags
 * @param func_name
 * @return
 */
AriadneReturn ariadne_prepare_creature_route_to_target_f(const struct Thing *thing, struct Ariadne *arid,
    const struct Coord3d *srcpos, const struct Coord3d *dstpos, long speed, AriadneRouteFlags flags, const char *func_name)
{
    struct Path path;
    long nav_sizexy;
    NAVIDBG(18,"%s: The %s index %d from %3d,%3d to %3d,%3d", func_name, thing_model_name(thing), (int)thing->index,
        (int)srcpos->x.stl.num, (int)srcpos->y.stl.num, (int)dstpos->x.stl.num, (int)dstpos->y.stl.num);
    LbMemorySet(&path, 0, sizeof(struct Path));
    // Set the required parameters
    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(thing);
    if ((flags & AridRtF_NoOwner) != 0)
        owner_player_navigating = -1;
    else
        owner_player_navigating = thing->owner;
    nav_sizexy = thing_nav_block_sizexy(thing);
    if (nav_sizexy > 0) nav_sizexy--;
    // Find the path
    path_init8_wide_f(&path, srcpos->x.val, srcpos->y.val,
        dstpos->x.val, dstpos->y.val, -2, nav_sizexy, func_name);
    // Reset globals
    nav_thing_can_travel_over_lava = 0;
    owner_player_navigating = -1;
    // Fill the Ariadne struct
    arid->startpos.x.val = srcpos->x.val;
    arid->startpos.y.val = srcpos->y.val;
    arid->startpos.z.val = srcpos->z.val;
    arid->endpos.x.val = dstpos->x.val;
    arid->endpos.y.val = dstpos->y.val;
    arid->endpos.z.val = dstpos->z.val;
    if (path.waypoints_num <= 0) {
        NAVIDBG(18,"%s: Cannot find route", func_name);
        arid->total_waypoints = 0;
        arid->stored_waypoints = arid->total_waypoints;
        return AridRet_Val2;
    }
    // Fill total waypoints number
    if (path.waypoints_num < ARID_PATH_WAYPOINTS_COUNT) {
        arid->total_waypoints = path.waypoints_num;
    } else {
        WARNLOG("%s: The %d waypoints is too many - cutting down", func_name,(int)path.waypoints_num);
        arid->total_waypoints = ARID_PATH_WAYPOINTS_COUNT-1;
    }
    // Fill stored waypoints (up to ARID_WAYPOINTS_COUNT)
    if (arid->total_waypoints < ARID_WAYPOINTS_COUNT) {
        arid->stored_waypoints = arid->total_waypoints;
    } else {
        arid->stored_waypoints = ARID_WAYPOINTS_COUNT;
    }
    long i;
    long k;
    k = 0;
    for (i = 0; i < arid->stored_waypoints; i++)
    {
        arid->waypoints[i].x.val = path.waypoints[k].x;
        arid->waypoints[i].y.val = path.waypoints[k].y;
        k++;
    }
    arid->current_waypoint = 0;
    arid->route_flags = flags;
    arid->pos_12.x.val = thing->mappos.x.val;
    arid->pos_12.y.val = thing->mappos.y.val;
    arid->pos_12.z.val = thing->mappos.z.val;
    arid->move_speed = speed;
    return AridRet_OK;
}

/**
 * Prepares creature route from source to destination position, and gives amount of waypoints.
 * @param thing
 * @param srcpos
 * @param dstpos
 * @param flags
 * @param func_name
 * @return
 * @see ariadne_prepare_creature_route_to_target() similar function which stores resulting route in Ariadne struct.
 */
long ariadne_count_waypoints_on_creature_route_to_target_f(const struct Thing *thing,
    const struct Coord3d *srcpos, const struct Coord3d *dstpos, AriadneRouteFlags flags, const char *func_name)
{
    struct Path path;
    long nav_sizexy;
    NAVIDBG(18,"%s: The %s index %d from %3d,%3d to %3d,%3d", func_name, thing_model_name(thing), (int)thing->index,
        (int)srcpos->x.stl.num, (int)srcpos->y.stl.num, (int)dstpos->x.stl.num, (int)dstpos->y.stl.num);
    LbMemorySet(&path, 0, sizeof(struct Path));
    // Set the required parameters
    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(thing);
    if ((flags & AridRtF_NoOwner) != 0)
        owner_player_navigating = -1;
    else
        owner_player_navigating = thing->owner;
    nav_sizexy = thing_nav_block_sizexy(thing);
    if (nav_sizexy > 0) nav_sizexy--;
    // Find the path
    path_init8_wide_f(&path, srcpos->x.val, srcpos->y.val,
        dstpos->x.val, dstpos->y.val, -2, nav_sizexy, func_name);
    // Reset globals
    nav_thing_can_travel_over_lava = 0;
    owner_player_navigating = -1;
    // Note: since this point, the function body should be identical to ariadne_prepare_creature_route_to_target().
    NAVIDBG(19,"%s: Finished, %d waypoints",func_name,(int)path.waypoints_num);
    return path.waypoints_num;
}

AriadneReturn ariadne_invalidate_creature_route(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct Ariadne *arid;
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    arid = &cctrl->arid;
    LbMemorySet(arid, 0, sizeof(struct Ariadne));
    return AridRet_OK;
}

AriadneReturn ariadne_initialise_creature_route_f(struct Thing *thing, const struct Coord3d *pos, long speed, AriadneRouteFlags flags, const char *func_name)
{
    struct CreatureControl *cctrl;
    struct Ariadne *arid;
    AriadneReturn ret;
    NAVIDBG(18,"%s: Route for %s index %d from %3d,%3d to %3d,%3d", func_name,thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)pos->x.stl.num, (int)pos->y.stl.num);
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    arid = &cctrl->arid;
    LbMemorySet(arid, 0, sizeof(struct Ariadne));
    if (ariadne_creature_reached_position(thing, pos))
    {
        ret = ariadne_prepare_creature_route_target_reached(thing, arid, &thing->mappos, pos);
        if (ret != AridRet_OK) {
            NAVIDBG(19,"%s: Failed to reach route from %5d,%5d to %5d,%5d", func_name,
                (int)thing->mappos.x.val,(int)thing->mappos.y.val, (int)pos->x.val,(int)pos->y.val);
            return ret;
        }
    } else
    {
        ret = ariadne_prepare_creature_route_to_target_f(thing, arid, &thing->mappos, pos, speed, flags, func_name);
        if (ret != AridRet_OK) {
            NAVIDBG(19,"%s: Failed to prepare route from %5d,%5d to %5d,%5d", func_name,
                (int)thing->mappos.x.val,(int)thing->mappos.y.val, (int)pos->x.val,(int)pos->y.val);
            return ret;
        }
        ariadne_init_current_waypoint(thing, arid);
    }
    ret = ariadne_init_movement_to_current_waypoint(thing, arid);
    NAVIDBG(19,"%s: Route prepared", func_name);
    return AridRet_OK;
}

AriadneReturn ariadne_creature_get_next_waypoint(struct Thing *thing, struct Ariadne *arid)
{
    struct Coord3d pos;
    // Make sure we didn't exceeded the stored waypoints count
    if (arid->current_waypoint >= arid->stored_waypoints)
    {
        return AridRet_PartOK;
    }
    // Note that the last condition assured us that we
    // won't make overflow by this increase
    arid->current_waypoint++;
    if (arid->current_waypoint != arid->stored_waypoints)
    {
        // Init route to the new current waypoint
        ariadne_init_current_waypoint(thing, arid);
        ariadne_init_movement_to_current_waypoint(thing, arid);
        return AridRet_OK;
    }
    // We've reached the last waypoint
    if (arid->stored_waypoints >= arid->total_waypoints)
    {
        return AridRet_FinalOK;
    }
    pos.x.val = arid->endpos.x.val;
    pos.y.val = arid->endpos.y.val;
    pos.z.val = arid->endpos.z.val;
    NAVIDBG(8,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)pos.x.stl.num, (int)pos.y.stl.num);
    return ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags);
}

TbBool ariadne_creature_reached_waypoint(const struct Thing *thing, const struct Ariadne *arid)
{
    return ariadne_creature_reached_position(thing, &arid->current_waypoint_pos);
}

AriadneReturn ariadne_update_state_manoeuvre_to_position(struct Thing *thing, struct Ariadne *arid)
{
    struct Coord3d pos;
    MapCoord dist;
    long hug_angle;

    if (ariadne_creature_blocked_by_wall_at(thing, &arid->pos_53))
    {
        pos.x.val = arid->endpos.x.val;
        pos.y.val = arid->endpos.y.val;
        pos.z.val = arid->endpos.z.val;
        NAVIDBG(8,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
            (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)pos.x.stl.num, (int)pos.y.stl.num);
        AriadneReturn aret;
        aret = ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags);
        if (aret != AridRet_OK) {
            return AridRet_PartOK;
        }
    }
    dist = get_2d_distance(&thing->mappos, &arid->current_waypoint_pos);
    if (arid->field_62 > dist)
        arid->field_62 = dist;
    if ((thing->mappos.x.val != arid->pos_53.x.val)
     || (thing->mappos.y.val != arid->pos_53.y.val))
    {
        arid->pos_12.x.val = arid->pos_53.x.val;
        arid->pos_12.y.val = arid->pos_53.y.val;
        arid->pos_12.z.val = arid->pos_53.z.val;
        return AridRet_OK;
    }
    switch (arid->manoeuvre_state)
    {
    case AridUpSStM_Unkn1:
        return ariadne_init_wallhug(thing, arid, &arid->pos_59);
    case AridUpSStM_Unkn2:
        hug_angle = ariadne_get_wallhug_angle(thing, arid);
        arid->wallhug_angle = hug_angle;
        arid->pos_12.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, hug_angle);
        arid->pos_12.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, hug_angle);
        arid->update_state = AridUpSt_Wallhug;
        return AridRet_OK;
    default:
        ERRORLOG("Unknown Manoeuvre state %d",(int)arid->manoeuvre_state);
        break;
    }
    return AridRet_OK;
}

AriadneReturn ariadne_update_state_on_line(struct Thing *thing, struct Ariadne *arid)
{
    long angle;
    long distance;
    NAVIDBG(19,"Starting");
    angle = get_angle_xy_to(&thing->mappos, &arid->current_waypoint_pos);
    distance = get_2d_distance(&thing->mappos, &arid->current_waypoint_pos);
    if ((distance - arid->field_62) > 1024)
    {
        struct Coord3d pos;
        arid->pos_12.x.val = thing->mappos.x.val;
        arid->pos_12.y.val = thing->mappos.y.val;
        arid->pos_12.z.val = thing->mappos.z.val;
        pos.x.val = arid->endpos.x.val;
        pos.y.val = arid->endpos.y.val;
        pos.z.val = arid->endpos.z.val;
        if (ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags)) {
            return AridRet_PartOK;
        }
    } else
    {
        if (distance <= arid->move_speed)
        {
            arid->pos_12.x.val = arid->current_waypoint_pos.x.val;
            arid->pos_12.y.val = arid->current_waypoint_pos.y.val;
            arid->pos_12.z.val = arid->current_waypoint_pos.z.val;
        }
        else
        {
            arid->pos_12.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, angle);
            arid->pos_12.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, angle);
            arid->pos_12.z.val = get_thing_height_at(thing, &arid->pos_12);
        }
    }
    if (arid->field_62 > distance) {
        arid->field_62 = distance;
    }
    if (ariadne_creature_blocked_by_wall_at(thing, &arid->pos_12))
    {
        if ( arid->field_23 )
        {
            struct Coord3d pos;
            pos.x.val = arid->endpos.x.val;
            pos.y.val = arid->endpos.y.val;
            pos.z.val = arid->endpos.z.val;
            if (ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags)) {
                return AridRet_PartOK;
            }
        }
        else
        {
            unsigned long blk_flags;
            blk_flags = ariadne_get_blocked_flags(thing, &arid->pos_12);
            if (!blocked_by_door_at(thing, &arid->pos_12, blk_flags))
            {
                struct Coord3d pos;
                ariadne_push_position_against_wall(thing, &arid->pos_12, &pos);
                arid->update_state = AridUpSt_Manoeuvre;
                arid->manoeuvre_state = AridUpSStM_Unkn1;
                arid->pos_53.x.val = pos.x.val;
                arid->pos_53.y.val = pos.y.val;
                arid->pos_53.z.val = pos.z.val;
                arid->pos_59.x.val = arid->pos_12.x.val;
                arid->pos_59.y.val = arid->pos_12.y.val;
                arid->pos_59.z.val = arid->pos_12.z.val;
                arid->pos_12.x.val = pos.x.val;
                arid->pos_12.y.val = pos.y.val;
                arid->pos_12.z.val = pos.z.val;
            }
        }
    }
    return AridRet_OK;
}

long ariadne_check_forward_for_wallhug_gap(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos, long a4)
{
    return _DK_ariadne_check_forward_for_wallhug_gap(thing, arid, pos, a4);
}

TbBool ariadne_creature_on_circular_hug(const struct Thing *thing, const struct Ariadne *arid)
{
    long src_x;
    long src_y;
    long dst_x;
    long dst_y;
    dst_x = arid->pos_18.x.val;
    src_x = thing->mappos.x.val;
    dst_y = arid->pos_18.y.val;
    src_y = thing->mappos.y.val;
    if (dst_x == src_x)
    {
      if ((dst_y >= src_y) && (arid->pos_12.y.val >= dst_y)) {
          return true;
      }
      if ((dst_y <= src_y) && (arid->pos_12.y.val <= dst_y)) {
          return true;
      }
      return false;
    }
    if (dst_y == src_y)
    {
        if ((dst_x >= src_x) && (arid->pos_12.x.val >= dst_x)) {
              return true;
        }
        if ((dst_x <= src_x) && (arid->pos_12.x.val <= dst_x)) {
            return true;
        }
    }
    return false;
}

AriadneReturn ariadne_update_state_wallhug(struct Thing *thing, struct Ariadne *arid)
{
    MapCoordDelta distance;
    NAVIDBG(18,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.val, (int)thing->mappos.y.val, (int)arid->current_waypoint_pos.x.val, (int)arid->current_waypoint_pos.y.val);
    distance = get_2d_distance(&thing->mappos, &arid->current_waypoint_pos);
    if ((distance - arid->field_62) > 1024)
    {
        struct Coord3d pos;
        arid->pos_12.x.val = thing->mappos.x.val;
        arid->pos_12.y.val = thing->mappos.y.val;
        arid->pos_12.z.val = thing->mappos.z.val;
        pos.x.val = arid->endpos.x.val;
        pos.y.val = arid->endpos.y.val;
        pos.z.val = arid->endpos.z.val;
        if (ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags)) {
            return AridRet_PartOK;
        }
        return AridRet_OK;
    }
    if (distance <= arid->move_speed)
    {
        arid->pos_12.x.val = arid->current_waypoint_pos.x.val;
        arid->pos_12.y.val = arid->current_waypoint_pos.y.val;
        arid->pos_12.z.val = arid->current_waypoint_pos.z.val;
        if (ariadne_creature_blocked_by_wall_at(thing, &arid->pos_12))
        {
            if ( arid->field_23 )
            {
                struct Coord3d pos;
                pos.x.val = arid->endpos.x.val;
                pos.y.val = arid->endpos.y.val;
                pos.z.val = arid->endpos.z.val;
                if (ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags)) {
                    return AridRet_PartOK;
                }
            }
            else
            {
                struct Coord3d pos;
                ariadne_push_position_against_wall(thing, &arid->pos_12, &pos);
                arid->update_state = AridUpSt_Manoeuvre;
                arid->manoeuvre_state = AridUpSStM_Unkn1;
                arid->pos_53.x.val = pos.x.val;
                arid->pos_53.y.val = pos.y.val;
                arid->pos_53.z.val = pos.z.val;
                arid->pos_59.x.val = arid->pos_12.x.val;
                arid->pos_59.y.val = arid->pos_12.y.val;
                arid->pos_59.z.val = arid->pos_12.z.val;
                arid->pos_12.x.val = pos.x.val;
                arid->pos_12.y.val = pos.y.val;
                arid->pos_12.z.val = pos.z.val;
            }
        }
    } else
    if (thing->move_angle_xy == arid->wallhug_angle)
    {
        if ((thing->mappos.x.val != arid->pos_12.x.val) || (arid->pos_12.y.val != thing->mappos.y.val))
        {
            ariadne_init_movement_to_current_waypoint(thing, arid);
            return 0;
        }
        if (distance < arid->field_62)
        {
            if (ariadne_creature_can_continue_direct_line_to_waypoint(thing, arid, arid->move_speed)) {
                if (ariadne_init_movement_to_current_waypoint(thing, arid) < 1) {
                    return AridRet_PartOK;
                }
                return AridRet_OK;
            }
            arid->field_62 = distance;
        }
        long hug_angle;
        hug_angle = ariadne_get_wallhug_angle(thing, arid);
        if (hug_angle == -1)
        {
            ariadne_init_movement_to_current_waypoint(thing, arid);
            return AridRet_OK;
        }
        arid->pos_12.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, hug_angle);
        arid->pos_12.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, hug_angle);
        arid->pos_12.z.val = get_thing_height_at(thing, &arid->pos_12);
        if ((thing->move_angle_xy == hug_angle) && ariadne_check_forward_for_wallhug_gap(thing, arid, &arid->pos_12, hug_angle))
        {
            arid->update_state = AridUpSt_Manoeuvre;
            arid->manoeuvre_state = AridUpSStM_Unkn2;
            arid->pos_53.x.val = arid->pos_12.x.val;
            arid->pos_53.y.val = arid->pos_12.y.val;
            arid->pos_53.z.val = arid->pos_12.z.val;
            return AridRet_OK;
        }
        arid->wallhug_angle = hug_angle;
        if (ariadne_creature_on_circular_hug(thing, arid))
        {
            struct Coord3d pos;
            arid->pos_12.x.val = thing->mappos.x.val;
            arid->pos_12.y.val = thing->mappos.y.val;
            arid->pos_12.z.val = thing->mappos.z.val;
            pos.x.val = arid->endpos.x.val;
            pos.y.val = arid->endpos.y.val;
            pos.z.val = arid->endpos.z.val;
            if (ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags) >= 1) {
                return 3;
            }
            return AridRet_OK;
        }
        if (ariadne_creature_blocked_by_wall_at(thing, &arid->pos_12))
        {
            if (!arid->field_23)
            {
                struct Coord3d pos;
                ariadne_push_position_against_wall(thing, &arid->pos_12, &pos);
                arid->update_state = AridUpSt_Manoeuvre;
                arid->manoeuvre_state = AridUpSStM_Unkn2;
                arid->pos_53.x.val = pos.x.val;
                arid->pos_53.y.val = pos.y.val;
                arid->pos_53.z.val = pos.z.val;
                arid->pos_59.x.val = arid->pos_12.x.val;
                arid->pos_59.y.val = arid->pos_12.y.val;
                arid->pos_59.z.val = arid->pos_12.z.val;
                arid->pos_12.x.val = pos.x.val;
                arid->pos_12.y.val = pos.y.val;
                arid->pos_12.z.val = pos.z.val;
                return AridRet_OK;
            }
            struct Coord3d pos;
            pos.x.val = arid->endpos.x.val;
            pos.y.val = arid->endpos.y.val;
            pos.z.val = arid->endpos.z.val;
            if (ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags)) {
                return 3;
            }
            return AridRet_OK;
          }
    }
    return AridRet_OK;
}

//TODO investigate when we get same coords
AriadneReturn ariadne_get_next_position_for_route(struct Thing *thing, struct Coord3d *finalpos, long speed, struct Coord3d *nextpos, AriadneRouteFlags flags)
{
    struct CreatureControl *cctrl;
    struct Ariadne *arid;
    AriadneReturn result;
    AriadneReturn aret;
    NAVIDBG(18,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)finalpos->x.stl.num, (int)finalpos->y.stl.num);
    cctrl = creature_control_get_from_thing(thing);
    arid = &cctrl->arid;
    //result = _DK_ariadne_get_next_position_for_route(thing, finalpos, speed, nextpos, a5); return result;
    arid->field_22 = 0;
    if ((finalpos->x.val != arid->endpos.x.val)
     || (finalpos->y.val != arid->endpos.y.val)
     || (arid->move_speed != speed))
    {
        aret = ariadne_initialise_creature_route(thing, finalpos, speed, flags);
        if (aret != AridRet_OK) {
            return AridRet_Val2;
        }
        arid->move_speed = speed;
        if (arid->field_22)
        {
            nextpos->x.val = arid->pos_12.x.val;
            nextpos->y.val = arid->pos_12.y.val;
            nextpos->z.val = arid->pos_12.z.val;
            return AridRet_OK;
        }
    }
    if (ariadne_creature_reached_waypoint(thing,arid))
    {
        aret = ariadne_creature_get_next_waypoint(thing,arid);
        if (aret == AridRet_FinalOK)
        {
            arid->endpos.x.val = 0;
            arid->endpos.y.val = 0;
            arid->endpos.z.val = 0;
            nextpos->x.val = thing->mappos.x.val;
            nextpos->y.val = thing->mappos.y.val;
            nextpos->z.val = thing->mappos.z.val;
            return AridRet_FinalOK;
        } else
        if (aret == AridRet_OK)
        {
            ariadne_init_movement_to_current_waypoint(thing, arid);
        } else
        {
            aret = ariadne_initialise_creature_route(thing, finalpos, speed, flags);
            if (aret != AridRet_OK) {
                return AridRet_PartOK;
            }
        }
    }
    switch (arid->update_state)
    {
    case AridUpSt_OnLine:
        result = ariadne_update_state_on_line(thing, arid);
        nextpos->x.val = arid->pos_12.x.val;
        nextpos->y.val = arid->pos_12.y.val;
        nextpos->z.val = arid->pos_12.z.val;
        break;
    case AridUpSt_Wallhug:
        result = ariadne_update_state_wallhug(thing, arid);
        nextpos->x.val = arid->pos_12.x.val;
        nextpos->y.val = arid->pos_12.y.val;
        nextpos->z.val = arid->pos_12.z.val;
        break;
    case AridUpSt_Manoeuvre:
        result = ariadne_update_state_manoeuvre_to_position(thing, arid);
        nextpos->x.val = arid->pos_12.x.val;
        nextpos->y.val = arid->pos_12.y.val;
        nextpos->z.val = arid->pos_12.z.val;
        break;
    default:
        result = AridRet_PartOK;
        break;
    }
    if (result != AridRet_OK)
    {
        WARNDBG(3, "Update state %d returned %d", (int)arid->update_state, (int)result);
    }
    return result;
}

/** Initializes creature to start moving towards given position.
 *  Hero Gates can be used by the creature to shorten distance.
 * @param thing The creature thing to be moved.
 * @param finalpos Final position coordinates.
 * @param nextpos Next step coordinates.
 * @param speed
 * @param flags
 * @return
 */
AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *finalpos, struct Coord3d *nextpos, long speed, AriadneRouteFlags flags)
{
    SYNCDBG(18,"Starting");
    if (game.field_14EA4B)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        cctrl->arid.field_23 = 1;
    }
    return ariadne_get_next_position_for_route(thing, finalpos, speed, nextpos, flags);
}

/**
 * Initializes Path structure with path data to travel between given coordinates.
 * Note that it works a bit different than in original DK - makes more error checks.
 *
 * @param path Target Path structure.
 * @param start_x Starting point coordinate.
 * @param start_y Starting point coordinate.
 * @param end_x Destination point coordinate.
 * @param end_y Destination point coordinate.
 * @param subroute Random factor for determining position within route, or negative special value.
 * @param nav_size
 */
static void path_init8_wide(struct Path* path, long start_x, long start_y, long end_x, long end_y, long subroute, unsigned char nav_size)
{
    path_init8_wide_f(path, start_x, start_y, end_x, end_y, subroute, nav_size, "path_init8_wide called from keeperfx.dll");
}
HOOK_DK_FUNC(path_init8_wide)

void path_init8_wide_f(struct Path *path, long start_x, long start_y, long end_x, long end_y,
    long subroute, unsigned char nav_size, const char *func_name)
{
    long route_dist;
    //_DK_path_init8_wide(path, start_x, start_y, end_x, end_y, a6, nav_size); return;
    NAVIDBG(9,"%s: Path from %5ld,%5ld to %5ld,%5ld on turn %lu", func_name, start_x, start_y, end_x, end_y, game.play_gameturn);
    if (subroute == -1)
      WARNLOG("%s: implement random externally", func_name);
    path->start.x = start_x;
    path->start.y = start_y;
    path->finish.x = end_x;
    path->finish.y = end_y;
    path->waypoints_num = 0;
    tree_Ax8 = start_x;
    tree_Ay8 = start_y;
    tree_Bx8 = end_x;
    tree_By8 = end_y;
    tree_routelen = -1;
    tree_triA = triangle_findSE8(start_x, start_y);
    tree_triB = triangle_findSE8(end_x, end_y);
    if ((tree_triA == -1) || (tree_triB == -1))
    {
        ERRORLOG("%s: Boundary triangle not found: %ld -> %ld.", func_name,tree_triA,tree_triB);
        return;
    }
    NAVIDBG(19,"%s: prepared triangles %ld -> %ld", func_name,tree_triA,tree_triB);
    if (!regions_connected(tree_triA, tree_triB))
    {
        NAVIDBG(9,"%s: Regions not connected, cannot trace a path.", func_name);
        return;
    }
    NAVIDBG(19,"%s: regions connected", func_name);
    edgelen_init();
    {
        int creature_radius;
        creature_radius = nav_size + 1;
        if ((creature_radius < 1) || (creature_radius > 3))
        {
            ERRORLOG("%s: only radius 1..3 allowed, got %d", func_name,creature_radius);
            return;
        }
        EdgeFit = RadiusEdgeFit[creature_radius];
    }
    tree_altA = get_triangle_tree_alt(tree_triA);
    tree_altB = get_triangle_tree_alt(tree_triB);
    if (subroute == -2)
    {
        tree_routelen = ma_triangle_route(tree_triA, tree_triB, &tree_routecost);
        NAVIDBG(19,"%s: route=%d", func_name, tree_routelen);
        if (tree_routelen != -1)
        {
            path->waypoints_num = route_to_path(start_x, start_y, end_x, end_y, tree_route, tree_routelen, path, &route_dist);
            path_out_a_bit(path, tree_route);
        }
    } else
    {
        gate_navigator_init8(&ap_GPathway, start_x, start_y, end_x, end_y, 4096, nav_size);
        route_through_gates(&ap_GPathway, path, subroute);
    }
    if (path->waypoints_num > 0) {
        NAVIDBG(9,"%s: Finished with %3ld waypoints, start: (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d)", func_name,(long)path->waypoints_num,
            (int)path->waypoints[0].x,(int)path->waypoints[0].y,(int)path->waypoints[1].x,(int)path->waypoints[1].y,(int)path->waypoints[2].x,(int)path->waypoints[2].y,
            (int)path->waypoints[3].x,(int)path->waypoints[3].y,(int)path->waypoints[4].x,(int)path->waypoints[4].y,(int)path->waypoints[5].x,(int)path->waypoints[5].y,
            (int)path->waypoints[6].x,(int)path->waypoints[6].y,(int)path->waypoints[7].x,(int)path->waypoints[7].y,(int)path->waypoints[8].x,(int)path->waypoints[8].y);
    } else {
        NAVIDBG(9,"%s: Finished with %3ld waypoints", func_name,(long)path->waypoints_num);
    }
}

/**
 * Changes path level to 3 or 4.
 *
 * @param pt_tri
 * @param pt_cor
 * @return Returns resulted level (3 or 4), or non-positive value on error.
 */
long make_3or4point(long *pt_tri, long *pt_cor)
{
    long l0;
    long l1;
    long n;
    do
    {
        l0 = point_loop(*pt_tri, *pt_cor);
        if (l0 == 3)
          return 3;
        n = reduce_point(pt_tri, pt_cor);
        if (n == 3)
          return 3;
        if (n < 0)
          return -1;
        l1 = point_loop(*pt_tri, *pt_cor);
        if (l1 == n)
        {
            if ((n == 3) || (n == 4))
              break;
        }
        if ((l1 != n) || (l1 >= l0))
        {
            ERRORLOG("bad state, l0:%02ld n:%02ld l1:%02ld", l0, n, l1);
            return -1;
        }
    } while (n > 4);
    return n;
}

static long delete_4point(long tri1_id, long cor1_id)
{
    struct Triangle *tri1;
    tri1 = &Triangles[tri1_id];
    long del_pt_id;
    del_pt_id = tri1->points[cor1_id];
    long cor2_id;
    long cor3_id;
    long cor4_id;
    long tri2_id;
    long tri3_id;
    long tri4_id;

    tri2_id = tri1->tags[cor1_id];
    cor2_id = link_find(tri2_id, tri1_id);
    cor2_id = MOD3[cor2_id+1];
    struct Triangle *tri2;
    tri2 = &Triangles[tri2_id];

    tri3_id = tri2->tags[cor2_id];
    cor3_id = link_find(tri3_id, tri2_id);
    cor3_id = MOD3[cor3_id+1];
    struct Triangle *tri3;
    tri3 = &Triangles[tri3_id];

    tri4_id = tri3->tags[cor3_id];
    cor4_id = link_find(tri4_id, tri3_id);
    cor4_id = MOD3[cor4_id+1];
    struct Triangle *tri4;
    tri4 = &Triangles[tri4_id];

    if (tri4->tags[cor4_id] != tri1_id) {
        return false;
    }

    int nreg;
    long tri5_id;
    long tri6_id;
    long cor5_id;
    long cor6_id;

    int ptA_cor;
    int ptB_cor;
    int ptC_cor;
    int ptD_cor;
    ptA_cor = tri1->points[MOD3[cor1_id+1]];
    ptB_cor = tri3->points[MOD3[cor3_id+1]];

    int diff_ax;
    int diff_ay;
    diff_ax = ari_Points[ptA_cor].x - ari_Points[ptB_cor].x;
    diff_ay = ari_Points[ptA_cor].y - ari_Points[ptB_cor].y;
    ptC_cor = tri3->points[MOD3[cor3_id+2]];
    int diff_bx;
    int diff_by;
    diff_bx = ari_Points[ptC_cor].x - ari_Points[ptB_cor].x;
    diff_by = ari_Points[ptC_cor].y - ari_Points[ptB_cor].y;

    ptD_cor = tri1->points[MOD3[cor1_id + 2]];
    int diff_cx;
    int diff_cy;
    diff_cx = ari_Points[ptD_cor].x - ari_Points[ptB_cor].x;
    diff_cy = ari_Points[ptD_cor].y - ari_Points[ptB_cor].y;

    if ((LbCompareMultiplications(diff_ay, diff_bx, diff_ax, diff_by) >= 0) ||
        (LbCompareMultiplications(diff_ay, diff_cx, diff_ax, diff_cy) <= 0))
    {
        tri5_id = tri2->tags[MOD3[cor2_id+1]];
        cor5_id = link_find(tri5_id, tri2_id);
        struct Triangle *tri5;
        tri5 = &Triangles[tri5_id];
        tri1->points[cor1_id] = tri2->points[MOD3[cor2_id+1]];
        tri1->tags[cor1_id] = tri5_id;
        tri5->tags[cor5_id] = tri1_id;

        tri6_id = tri4->tags[MOD3[cor4_id+1]];
        cor6_id = link_find(tri6_id, tri4_id);
        struct Triangle *tri6;
        tri6 = &Triangles[tri6_id];
        tri3->points[cor3_id] = tri4->points[MOD3[cor4_id+1]];
        tri3->tags[cor3_id] = tri6_id;
        tri6->tags[cor6_id] = tri3_id;

        tri1->tags[MOD3[cor1_id+2]] = tri3_id;
        tri3->tags[MOD3[cor3_id+2]] = tri1_id;

        nreg = get_triangle_region_id(tri2_id);
        if (nreg) {
            region_unset(tri2_id, nreg);
        }
        nreg = get_triangle_region_id(tri4_id);
        if (nreg) {
            region_unset(tri4_id, nreg);
        }
        tri_dispose(tri2_id);
        tri_dispose(tri4_id);
        edgelen_set(tri1_id);
        edgelen_set(tri3_id);
    }
    else
    {
        tri5_id = tri3->tags[MOD3[cor3_id+1]];
        cor5_id = link_find(tri5_id, tri3_id);
        struct Triangle *tri5;
        tri5 = &Triangles[tri5_id];
        tri2->points[cor2_id] = ptB_cor;
        tri2->tags[cor2_id] = tri5_id;
        tri5->tags[cor5_id] = tri2_id;

        tri6_id = tri1->tags[MOD3[cor1_id+1]];
        cor6_id = link_find(tri6_id, tri1_id);
        struct Triangle *tri6;
        tri6 = &Triangles[tri6_id];
        tri4->points[cor4_id] = tri1->points[MOD3[cor1_id+1]];
        tri4->tags[cor4_id] = tri6_id;
        tri6->tags[cor6_id] = tri4_id;

        tri2->tags[MOD3[cor2_id+2]] = tri4_id;
        tri4->tags[MOD3[cor4_id+2]] = tri2_id;

        nreg = get_triangle_region_id(tri3_id);
        if (nreg) {
            region_unset(tri3_id, nreg);
        }
        nreg = get_triangle_region_id(tri1_id);
        if (nreg) {
            region_unset(tri1_id, nreg);
        }
        tri_dispose(tri3_id);
        tri_dispose(tri1_id);
        edgelen_set(tri2_id);
        edgelen_set(tri4_id);
    }
    point_dispose(del_pt_id);
    return 1;
}
HOOK_DK_FUNC(delete_4point)

static long delete_3point(long tri1_id, long cor1_id)
{
    struct Triangle *tri1;
    tri1 = &Triangles[tri1_id];
    long del_pt_id;
    del_pt_id = tri1->points[cor1_id];

    int tri2_id;
    tri2_id = tri1->tags[cor1_id];
    if (tri2_id == -1) {
        return false;
    }
    int cor2_id;
    cor2_id = link_find(tri2_id, tri1_id);
    cor2_id = MOD3[cor2_id + 1];
    struct Triangle *tri2;
    tri2 = &Triangles[tri2_id];

    long tri3_id;
    tri3_id = tri2->tags[cor2_id];
    if (tri3_id == -1) {
      return false;
    }
    int cor3_id;
    cor3_id = link_find(tri3_id, tri2_id);
    cor3_id = MOD3[cor3_id+1];
    struct Triangle *tri3;
    tri3 = &Triangles[tri3_id];

    if (tri3->tags[cor3_id] != tri1_id) {
      return false;
    }
    int cor4_id;
    int cor5_id;
    cor4_id = tri2->tags[MOD3[cor2_id+1]];
    cor5_id = tri3->tags[MOD3[cor3_id+1]];
    int tri4_id;
    int tri5_id;
    tri4_id = link_find(cor4_id, tri2_id);
    tri5_id = link_find(cor5_id, tri3_id);
    tri1->points[cor1_id] = tri2->points[MOD3[cor2_id+1]];
    tri1->tags[cor1_id] = cor4_id;
    tri1->tags[MOD3[cor1_id+2]] = cor5_id;
    Triangles[cor4_id].tags[tri4_id] = tri1_id;
    Triangles[cor5_id].tags[tri5_id] = tri1_id;
    int nreg;
    nreg = get_triangle_region_id(tri2_id);
    if (nreg)
    {
        region_unset(tri2_id, nreg);
    }
    nreg = get_triangle_region_id(tri3_id);
    if (nreg)
    {
        region_unset(tri3_id, nreg);
    }
    tri_dispose(tri2_id);
    tri_dispose(tri3_id);
    edgelen_set(tri1_id);
    point_dispose(del_pt_id);
    return true;
}
HOOK_DK_FUNC(delete_3point)

TbBool delete_point(long pt_tri, long pt_cor)
{
    long n;
    long ntri;
    long ncor;
    ntri = pt_tri;
    ncor = pt_cor;
    n = make_3or4point(&ntri, &ncor);
    if (n <= 0)
    {
        ERRORLOG("make_3or4point failure");
        return false;
    }
    if (n == 4)
    {
        if (!delete_4point(ntri, ncor))
        {
          ERRORLOG("variant 4 fails");
          return false;
        }
    } else
    {
        if (!delete_3point(ntri, ncor))
        {
          ERRORLOG("variant 3 fails");
          return false;
        }
    }
    return true;
}

long tri_split3(long btri_id, long pt_x, long pt_y)
{
    NAVIDBG(19,"Starting");
    //_DK_tri_split3(a1, a2, a3);
    struct Triangle *btri;
    struct Triangle *tri1;
    struct Triangle *tri2;
    long tri_id1;
    long tri_id2;
    tri_id1 = tri_new();
    if (tri_id1 < 0) {
        return -1;
    }
    tri_id2 = tri_new();
    if (tri_id2 < 0) {
        tri_dispose(tri_id1);
        return -1;
    }
    btri = &Triangles[btri_id];
    tri1 = &Triangles[tri_id1];
    tri2 = &Triangles[tri_id2];
    memcpy(tri1,btri,sizeof(struct Triangle));
    memcpy(tri2,btri,sizeof(struct Triangle));
    long pt_id;
    pt_id = point_set_new_or_reuse(pt_x, pt_y);
    //pt_id = point_new(); // from before point_set_new_or_reuse()
    if (pt_id < 0) {
        tri_dispose(tri_id1);
        tri_dispose(tri_id2);
        return -1;
    }
    //point_set(pt_id, pt_x, pt_y); // from before point_set_new_or_reuse()
    btri->points[2] = pt_id;
    tri1->points[0] = pt_id;
    tri2->points[1] = pt_id;
    btri->tags[1] = tri_id1;
    btri->tags[2] = tri_id2;
    btri->field_D |= 0x06;
    btri->field_D &= 0x0F;
    tri1->tags[0] = btri_id;
    tri1->tags[2] = tri_id2;
    tri1->field_D |= 0x05;
    tri1->field_D &= 0x17;
    tri2->tags[0] = btri_id;
    tri2->tags[1] = tri_id1;
    tri2->field_D |= 0x03;
    tri2->field_D &= 0x27;

    long ttri_id;
    long ltri_id;
    ttri_id = tri1->tags[1];
    if (ttri_id != -1)
    {
        ltri_id = link_find(ttri_id, btri_id);
        if (ltri_id >= 0) {
            Triangles[ttri_id].tags[ltri_id] = tri_id1;
        } else {
            ERRORLOG("A not found");
        }
    }
    ttri_id = tri2->tags[2];
    if (ttri_id != -1)
    {
        ltri_id = link_find(ttri_id, btri_id);
        if (ltri_id >= 0) {
            Triangles[ttri_id].tags[ltri_id] = tri_id2;
        } else {
            ERRORLOG("B not found");
        }
    }
    long reg_id;
    reg_id = get_triangle_region_id(btri_id);
    if (reg_id > 0) {
        region_unset(btri_id, reg_id);
    }
    tri1->field_E = 0;
    tri2->field_E = 0;
    edgelen_set(btri_id);
    edgelen_set(tri_id1);
    edgelen_set(tri_id2);
    return pt_id;
}

long tri_split2(long tri_id1, long cor_id1, long pt_x, long pt_y, long pt_id1)
{
    long tri_id2;
    //tri_id2 = _DK_tri_split2(tri_id1, cor_id1, pt_x, pt_y, pt_id1); return tri_id2;
    tri_id2 = tri_new();
    if (tri_id2 < 0) {
        return -1;
    }
    struct Triangle *tri1;
    struct Triangle *tri2;
    tri1 = &Triangles[tri_id1];
    tri2 = &Triangles[tri_id2];
    memcpy(tri2, tri1, sizeof(struct Triangle));
    long cor_id2;
    long reg_id1;
    cor_id2 = MOD3[cor_id1 + 1];
    tri1->points[cor_id2] = pt_id1;
    tri2->points[cor_id1] = pt_id1;
    tri1->tags[cor_id2] = tri_id2;
    tri1->field_D |= (1 << cor_id2);
    tri1->field_D &= ~(1 << (cor_id2 + 3));
    long tri_id3;
    tri_id3 = MOD3[cor_id1 + 2];
    tri2->tags[tri_id3] = tri_id1;
    tri2->field_D |= (1 << tri_id3);
    tri2->field_D &= ~(1 << (tri_id3 + 3));
    long tri_id4;
    tri_id3 = tri2->tags[cor_id2];
    if (tri_id3 != -1)
    {
        tri_id4 = link_find(tri_id3, tri_id1);
        if (tri_id4 == -1) {
            ERRORLOG("A not found");
        }
        Triangles[tri_id3].tags[tri_id4] = tri_id2;
    }
    reg_id1 = get_triangle_region_id(tri_id1);
    if (reg_id1 > 0) {
        region_unset(tri_id1, reg_id1);
    }
    tri2->field_E = 0;
    edgelen_set(tri_id1);
    edgelen_set(tri_id2);
    return tri_id2;
}

long edge_split(long ntri, long ncor, long pt_x, long pt_y)
{
    long pt_idx;
    long ntr2;
    long ncr2;
    long tri_sp1;
    long tri_sp2;
    NAVIDBG(19,"Starting");
    // Create and fill new point
    pt_idx = point_set_new_or_reuse(pt_x, pt_y);
    //pt_idx = point_new(); // from before point_set_new_or_reuse()
    if (pt_idx < 0) {
        return -1;
    }
    //point_set(pt_idx, pt_x, pt_y); // from before point_set_new_or_reuse()
    // Find second ntri and ncor
    ntr2 = Triangles[ntri].tags[ncor];
    ncr2 = link_find(ntr2, ntri);
    if (ncr2 < 0) {
        point_dispose(pt_idx);
        return -1;
    }
    // Do the splitting
    tri_sp1 = tri_split2(ntri, ncor, pt_x, pt_y, pt_idx);
    tri_sp2 = tri_split2(ntr2, ncr2, pt_x, pt_y, pt_idx);
    Triangles[ntr2].tags[ncr2] = tri_sp1;
    Triangles[ntri].tags[ncor] = tri_sp2;
    return pt_idx;
}

/** Returns if given coords can divide triangle into same areas.
 *
 * @param ntri Triangle index.
 * @param ncorA First tip of the edge to be divided.
 * @param ncorB Second tip of the edge to be divided.
 * @param pt_x Coord X of the dividing point.
 * @param pt_y Coord Y of the dividing point.
 * @return Zero if areas do not differ; -1 or 1 otherwise.
 */
char triangle_divide_areas_differ(long ntri, long ncorA, long ncorB, long pt_x, long pt_y)
{
    long tipA_x;
    long tipA_y;
    long tipB_x;
    long tipB_y;
    struct Point *pt;

    pt = get_triangle_point(ntri,ncorA);
    tipA_x = pt->x;
    tipA_y = pt->y;
    pt = get_triangle_point(ntri,ncorB);
    tipB_x = pt->x;
    tipB_y = pt->y;
    return LbCompareMultiplications(pt_y-tipA_y, tipB_x-tipA_x, pt_x-tipA_x, tipB_y-tipA_y);
}

/*
 * There are mesh of all triangles on a map
 * This function inserts another point into mesh by splitting triangles into parts
 */
static TbBool insert_point(long pt_x, long pt_y)
{
    long ntri;
    NAVIDBG(19,"Starting for (%d,%d)", (int)pt_x, (int)pt_y);
    //TODO PATHFINDING triangulate_area sub-sub-function, verify
    //_DK_insert_point(pt_x, pt_y); return true;
    ntri = triangle_find8(pt_x << 8, pt_y << 8);
    if ((ntri < 0) || (ntri >= TRIANLGLES_COUNT))
    {
        ERRORLOG("triangle not found");
        return false;
    }
    if (triangle_tip_equals(ntri, 0, pt_x, pt_y))
        return true;
    if (triangle_tip_equals(ntri, 1, pt_x, pt_y))
        return true;
    if (triangle_tip_equals(ntri, 2, pt_x, pt_y))
        return true;

    if (triangle_divide_areas_differ(ntri, 0, 1, pt_x, pt_y) == 0)
    {
        return edge_split(ntri, 0, pt_x, pt_y) >= 0;
    }
    if (triangle_divide_areas_differ(ntri, 1, 2, pt_x, pt_y) == 0)
    {
        return edge_split(ntri, 1, pt_x, pt_y) >= 0;
    }
    if (triangle_divide_areas_differ(ntri, 2, 0, pt_x, pt_y) == 0)
    {
        return edge_split(ntri, 2, pt_x, pt_y) >= 0;
    }
    return tri_split3(ntri, pt_x, pt_y) >= 0;
}
HOOK_DK_FUNC(insert_point)

long fill_concave(long tri_beg_id, long tag_id, long tri_end_id)
{
    long tri_id;
    long cor_id;
    while ( 1 )
    {
      tri_id = Triangles[tri_beg_id].tags[tag_id];
      if (tri_id == -1) {
        return 0;
      }
      cor_id = link_find(tri_id, tri_beg_id);
      cor_id = MOD3[cor_id+1];
      int rotate_n;
      int rotate_y;
      rotate_y = 0;
      rotate_n = 0;
      while (Triangles[tri_id].tags[cor_id] != tri_end_id)
      {
          if (edge_rotateAC(tri_id, cor_id))
          {
              rotate_y++;
          } else
          {
              long n;
              n = Triangles[tri_id].tags[cor_id];
              if ((n == -1) || (n == tri_beg_id)) {
                  return 0;
              }
              cor_id = link_find(n, tri_id);
              cor_id = MOD3[cor_id+1];
              tri_id = n;
              rotate_n++;
          }
      }
      if ( !rotate_n || !rotate_y )
        break;
    }
    return 1;
}

void make_edge_sub(long start_tri_id1, long start_cor_id1, long start_tri_id4, long start_cor_id4, long sx, long sy, long ex, long ey)
{
    struct Triangle *tri;
    struct Point *pt;
    struct Point *pt1;
    struct Point *pt2;
    struct Point *pt3;
    long tri_id1;
    long cor_id1;
    long tri_id2;
    long cor_id2;
    long tri_id3;
    long cor_id3;
    long tri_id4;
    long cor_id4;
    long i;
    long cx;
    long cy;
    unsigned long k;
    cor_id1 = start_cor_id1;
    tri_id1 = start_tri_id1;
    cor_id4 = start_cor_id4;
    tri_id4 = start_tri_id4;
    k = 0;
    do
    {
        tri = get_triangle(tri_id1);
        tri_id2 = tri->tags[cor_id1];
        i = link_find(tri_id2,tri_id1);
        cor_id2 = MOD3[i+2];
        tri_id1 = tri->tags[cor_id1];
        cor_id1 = MOD3[i+1];
        pt = get_triangle_point(tri_id2, cor_id2);
        cx = pt->x;
        cy = pt->y;
        if (LbCompareMultiplications(cy - ey, sx - ex, cx - ex, sy - ey) < 0) {
            continue;
        }
        tri = get_triangle(tri_id2);
        tri_id3 = tri->tags[cor_id1];
        cor_id3 = link_find(tri_id3,tri_id2);
        pt1 = get_triangle_point(tri_id3, cor_id3);
        pt2 = get_triangle_point(tri_id4, MOD3[cor_id4+1]);
        pt3 = get_triangle_point(tri_id4, cor_id4);
        //if (triangle_divide_areas_differ(tri_id4, MOD3[cor_id4+1], cor_id4, pt1->x, pt1->y) < 0)
        if (LbCompareMultiplications(pt1->y - pt2->y, pt3->x - pt2->x,
                                     pt1->x - pt2->x, pt3->y - pt2->y) < 0)
        {
            fill_concave(tri_id4, cor_id4, tri_id3);
            break;
        }
        tri_id4 = tri_id3;
        cor_id4 = cor_id3;
        cor_id1 = cor_id2;
        k++;
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected");
            break;
        }
    } while ((cx != sx) || (cy != sy));
}

static TbBool make_edge(long start_x, long start_y, long end_x, long end_y)
{
    struct Triangle *tri;
    struct Point *pt;
    long sx;
    long ex;
    long sy;
    long ey;
    long tri_id1;
    long cor_id1;
    long tri_id2;
    long cor_id2;
    long tri_id3;
    long cor_id3;
    long tmpX;
    long tmpY;
    long pt_cor;
    unsigned long k;
    NAVIDBG(19,"Starting");
    //TODO PATHFINDING triangulate_area sub-sub-function, verify
    //_DK_make_edge(start_x, start_y, end_x, end_y); return;
    k = 0;
    sx = start_x;
    sy = start_y;
    ex = end_x;
    ey = end_y;
    while ((ex != sx) || (ey != sy))
    {
        // Find triangles to which start point and end point belongs
        if (!point_find(ex, ey, &tri_id1, &cor_id1))
            break;
        if (!point_find(sx, sy, &tri_id2, &cor_id2))
            break;
        pt_cor = pointed_at8(sx << 8, sy << 8, &tri_id1, &cor_id1);
        if (pt_cor == -1)
        {
            ERRORLOG("border point not found, pointed at %ld,%ld",sx,sy);
            return false;
        }
        pt = get_triangle_point(tri_id1, pt_cor);
        if ((pt->x == sx) && (pt->y == sy))
            break;
        tri = get_triangle(tri_id1);
        tri_id3 = tri->tags[cor_id1];
        cor_id3 = link_find(tri_id3,tri_id1);
        pt = get_triangle_point(tri_id3, cor_id3);
        tri = get_triangle(tri_id1);
        tri_id3 = tri->tags[cor_id1];
        cor_id3 = link_find(tri_id3,tri_id1);
        pt = get_triangle_point(tri_id3, cor_id3);
        tmpX = pt->x;
        tmpY = pt->y;
        SYNCDBG(18,"Triangle %d point %d is (%d,%d)",(int)tri_id3,(int)tri_id1,(int)tmpX,(int)tmpY);
        if (LbCompareMultiplications(tmpY-ey, sx-ex, tmpX-ex, sy-ey) == 0)
        {
            if (!make_edge(ex, ey, tmpX, tmpY)) {
                return false;
            }
            ex = tmpX;
            ey = tmpY;
            continue;
        }
        make_edge_sub(tri_id1, pt_cor, tri_id3, cor_id3, sx, sy, ex, ey);
        tmpX = ex;
        tmpY = ey;
        ex = sx;
        ey = sy;
        sx = tmpX;
        sy = tmpY;
        k++;
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected at area (%d,%d) to (%d,%d)",(int)sx,(int)sy,(int)ex,(int)ey);
            return false;
        }
    }
    return true;
}
HOOK_DK_FUNC(make_edge)

TbBool border_clip_horizontal(const unsigned char *imap, long start_x, long end_x, long start_y, long end_y)
{
    unsigned char map_center;
    unsigned char map_up;
    const unsigned char* mapp_center;
    const unsigned char* mapp_up;
    TbBool r;
    long i;
    r = true;
    NAVIDBG(19,"Starting from (%ld,%ld) to (%ld,%ld)",start_x, start_y, end_x, end_y);
    //TODO PATHFINDING triangulate_area sub-function, verify
    //_DK_border_clip_horizontal(imap, start_x, end_x, start_y, end_y); return true;

    i = start_x;
    {
        mapp_center = &imap[navmap_tile_number(i,start_y)];
        mapp_up = &imap[navmap_tile_number(i,start_y-1)];
        {
            r &= insert_point(i, start_y);
            map_up = *mapp_up;
            map_center = *mapp_center;
        }
    }
    for (i++; i < end_x; i++)
    {
        mapp_center = &imap[navmap_tile_number(i,start_y)];
        mapp_up = &imap[navmap_tile_number(i,start_y-1)];
        if ((*mapp_center != map_center) || (*mapp_up != map_up))
        {
            r &= insert_point(i, start_y);
            map_up = *mapp_up;
            map_center = *mapp_center;
        }
    }
    r &= insert_point(end_x, start_y);
    if (!r) {
        ERRORLOG("Couldn't insert points to make border");
        //TODO PATHFINDING on a failure, we could release all allocated points...
        return r;
    }
    r &= make_edge(start_x, start_y, end_x, start_y);
    if (!r) {
        ERRORLOG("Couldn't make edge for border");
    }
    return r;
}

TbBool border_clip_vertical(const unsigned char *imap, long start_x, long end_x, long start_y, long end_y)
{
    unsigned char map_center;
    unsigned char map_left;
    const unsigned char* mapp_center;
    const unsigned char* mapp_left;
    TbBool r;
    long i;
    r = true;
    NAVIDBG(19,"Starting from (%ld,%ld) to (%ld,%ld)",start_x, start_y, end_x, end_y);
    //TODO PATHFINDING triangulate_area sub-function, verify
    //_DK_border_clip_vertical(imap, start_x, end_x, start_y, end_y); return true;
    i = start_y;
    {
        mapp_center = &imap[navmap_tile_number(start_x,i)];
        mapp_left = &imap[navmap_tile_number(start_x-1,i)];
        {
            r &= insert_point(start_x, i);
            map_left = *mapp_left;
            map_center = *mapp_center;
        }
    }
    for (i++; i < end_y; i++)
    {
        mapp_center = &imap[navmap_tile_number(start_x,i)];
        mapp_left = &imap[navmap_tile_number(start_x-1,i)];
        if ((*mapp_center != map_center) || (*mapp_left != map_left))
        {
            r &= insert_point(start_x, i);
            map_left = *mapp_left;
            map_center = *mapp_center;
        }
    }
    r &= insert_point(start_x, end_y);
    if (!r) {
        ERRORLOG("Couldn't insert points to make border");
        //TODO PATHFINDING on a failure, we could release all allocated points...
        return r;
    }
    r &= make_edge(start_x, start_y, start_x, end_y);
    if (!r) {
        ERRORLOG("Couldn't make edge for border");
    }
    return r;
}

TbBool point_redundant(long tri_idx, long cor_idx)
{
    long tri_first;
    long cor_first;
    long tri_secnd;
    long cor_secnd;
    tri_first = tri_idx;
    cor_first = cor_idx;
    unsigned long k;
    k = 0;
    while ( 1 )
    {
        tri_secnd = Triangles[tri_first].tags[cor_first];
        if ((tri_secnd < 0) || (tri_secnd >= TRIANLGLES_COUNT))
            break;
        if (get_triangle_tree_alt(tri_secnd) != get_triangle_tree_alt(tri_idx))
          break;
        cor_secnd = link_find(tri_secnd, tri_first);
        if (cor_secnd < 0)
            break;
        cor_first = MOD3[cor_secnd+1];
        tri_first = tri_secnd;
        if (tri_secnd == tri_idx)
        {
            return true;
        }
        k++;
        if (k >= TRIANLGLES_COUNT) {
            ERRORDBG(9,"Infinite loop detected");
            break;
        }
    }
    return false;
}

static long edge_find(long stlstart_x, long stlstart_y, long stlend_x, long stlend_y, long *edge_tri, long *edge_cor)
{
    //Note: uses LbCompareMultiplications()
    struct Triangle *tri;
    struct Point *pt;
    long dst_tri_idx;
    long dst_cor_idx;
    long tri_idx;
    long cor_idx;
    long tri_id2;
    long delta_x;
    long delta_y;
    long len_x;
    long len_y;
    long i;
    NAVIDBG(19,"Starting");
    if (!point_find(stlstart_x, stlstart_y, &dst_tri_idx, &dst_cor_idx))
    {
        return 0;
    }
    tri_idx = dst_tri_idx;
    cor_idx = dst_cor_idx;
    len_y = stlend_y - stlstart_y;
    len_x = stlend_x - stlstart_x;
    unsigned long k;
    k = 0;
    do
    {
        pt = get_triangle_point(tri_idx, MOD3[cor_idx+1]);
        delta_y = (pt->y - stlstart_y);
        delta_x = (pt->x - stlstart_x);
        if (LbCompareMultiplications(len_y, delta_x, len_x, delta_y) == 0)
        {
            if (LbNumberSignsSame(delta_x, len_x) && LbNumberSignsSame(delta_y, len_y))
            {
                *edge_tri = tri_idx;
                *edge_cor = cor_idx;
                return 1;
            }
        }
        tri = get_triangle(tri_idx);
        tri_id2 = tri->tags[cor_idx];
        if (tri_id2 == -1)
          break;
        i = link_find(tri_id2, tri_idx);
        if (i < 0) {
            ERRORLOG("no tri link");
            break;
        }
        tri_idx = tri_id2;
        cor_idx = MOD3[i+1];
        k++;
        if (k > TRIANLGLES_COUNT) {
            ERRORLOG("Infinite loop detected");
            break;
        }
    }
    while (tri_idx != dst_tri_idx);
    return 0;
}
HOOK_DK_FUNC(edge_find)

TbBool edge_lock_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name)
{
    long pt_x;
    long pt_y;
    unsigned long k;
    //_DK_edge_lock(ptend_x, ptend_y, ptstart_x, ptstart_y); return true;
    pt_x = ptstart_x;
    pt_y = ptstart_y;
    k = 0;
    while ((pt_x != ptend_x) || (pt_y != ptend_y))
    {
        long tri_id;
        long cor_id;
        if (!edge_find(pt_x, pt_y, ptend_x, ptend_y, &tri_id, &cor_id))
        {
            ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)pt_x, (int)pt_y, (int)ptend_x, (int)ptend_y);
            return false;
        }
        struct Triangle *tri;
        tri = get_triangle(tri_id);
        tri->field_D |= 1 << (cor_id+3);
        struct Point *pt;
        pt = get_triangle_point(tri_id,MOD3[cor_id+1]);
        if (point_is_invalid(pt)) {
            ERRORLOG("Invalid point on edge");
            return false;
        }
        pt_x = pt->x;
        pt_y = pt->y;
        k++;
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected");
            return false;
        }
    }
    return true;
}

TbBool edge_unlock_record_and_regions_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name)
{
    long pt_x;
    long pt_y;
    unsigned long k;
    long nerr;
    pt_x = ptstart_x;
    pt_y = ptstart_y;
    k = 0;
    nerr = 0;
    while (pt_x != ptend_x || pt_y != ptend_y)
    {
        long tri_id;
        long cor_id;
        if (!edge_find(pt_x, pt_y, ptend_x, ptend_y, &tri_id, &cor_id))
        {
            ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)pt_x, (int)pt_y, (int)ptend_x, (int)ptend_y);
            return false;
        }
        if (edge_point_add(pt_x, pt_y) < 0) {
            nerr++;
        }
        region_unlock(tri_id);
        struct Triangle *tri;
        tri = get_triangle(tri_id);
        tri->field_D &= ~(1 << (cor_id+3));
        struct Point *pt;
        pt = get_triangle_point(tri_id,MOD3[cor_id+1]);
        if (point_is_invalid(pt)) {
            ERRORLOG("Invalid point on edge");
            return false;
        }
        pt_x = pt->x;
        pt_y = pt->y;
        k++;
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected");
            return false;
        }
    }
    if (nerr != 0)
    {
        ERRORMSG("%s: overflow for %ld edge points",func_name,nerr);
        //ERRORLOG("edge_setlock_record:overflow");
        return true; //TODO PATHFINDING make sure we should return true
    }
    return true;
}

TbBool border_lock(long start_x, long start_y, long end_x, long end_y)
{
    TbBool r;
    NAVIDBG(19,"Starting");
    r = true;
    r &= edge_lock(start_x, start_y, start_x, end_y);
    r &= edge_lock(start_x, end_y, end_x, end_y);
    r &= edge_lock(end_x, end_y, end_x, start_y);
    r &= edge_lock(end_x, start_y, start_x, start_y);
    return r;
}

HOOK_DK_FUNC(border_internal_points_delete)
void border_internal_points_delete(long start_x, long start_y, long end_x, long end_y)
{
    long edge_tri;
    long edge_cor;
    long ntri;
    long ncor;
    unsigned long k;
    struct Point *pt;
    long i;
    long n;
    NAVIDBG(19,"Starting");
    //TODO PATHFINDING triangulate_area sub-function, verify
    //_DK_border_internal_points_delete(start_x, start_y, end_x, end_y); return;

    if (!edge_find(start_x, start_y, end_x, start_y, &edge_tri, &edge_cor))
    {
        ERRORLOG("Top not found");
        return;
    }

    ntri = Triangles[edge_tri].tags[edge_cor];
    ncor = link_find(ntri, edge_tri);
    if (ncor < 0)
    {
        ERRORLOG("Cannot find edge link");
        return;
    }
    k = 0;
    while ( 1 )
    {
        if (k >= TRIANLGLES_COUNT)
        {
            ERRORLOG("Infinite loop detected");
            break;
        }
        region_set(ntri, 0);
        pt = get_triangle_point(ntri,MOD3[ncor+2]);
        if ((pt->x != start_x) && (pt->x != end_x)
         && (pt->y != start_y) && (pt->y != end_y))
        {
            if (!delete_point(ntri, MOD3[ncor+2]))
                break;
            ntri = Triangles[edge_tri].tags[edge_cor];
            ncor = link_find(ntri, edge_tri);
            if (ncor < 0)
            {
                ERRORLOG("Cannot re-find edge link");
                break;
            }
            k++;
            continue;
        }
        ncor = MOD3[ncor+1];
        while (!outer_locked(ntri, ncor))
        {
            if (k >= TRIANLGLES_COUNT)
            {
                // Message will be displayed in big loop, so not here
                break;
            }
            n = Triangles[ntri].tags[ncor];
            i = link_find(n, ntri);
            if (i < 0)
            {
                ERRORLOG("Cannot find triangle link");
                return;
            }
            ntri = n;
            ncor = MOD3[i+1];
            region_set(ntri, 0);
            k++;
        }
        k++;
        if (Triangles[ntri].tags[ncor] == edge_tri)
            break;
    }
}

long triangle_area1(long tri_idx)
{
    int ptidx0;
    int ptidx1;
    int ptidx2;
    ptidx0 = Triangles[tri_idx].points[0];
    ptidx1 = Triangles[tri_idx].points[1];
    ptidx2 = Triangles[tri_idx].points[2];
    long long area1;
    long long area2;
    area1 = (ari_Points[ptidx2].x - (int)ari_Points[ptidx0].x) * (ari_Points[ptidx0].y - (int)ari_Points[ptidx1].y);
    area2 = (ari_Points[ptidx1].x - (int)ari_Points[ptidx0].x) * (ari_Points[ptidx2].y - (int)ari_Points[ptidx0].y);
    return llabs(area1+area2);
}

static void brute_fill_rectangle(long start_x, long start_y, long end_x, long end_y, unsigned char ntree_alt)
{
    //_DK_brute_fill_rectangle(start_x, start_y, end_x, end_y, a5);
    // Replace start and end if they are switched
    if (end_x < start_x)
    {
        long i;
        i = end_x;
        end_x = start_x;
        start_x = i;
    }
    if (end_y < start_y)
    {
        long i;
        i = end_y;
        end_y = start_y;
        start_y = i;
    }
    long long area;
    area = 0;
    long tri_idx;
    for (tri_idx = ix_Triangles - 1; tri_idx >= 0; tri_idx--)
    {
        struct Triangle *tri;
        tri = &Triangles[tri_idx];
        int ptidx;
        long x;
        long y;
        ptidx = tri->points[0];
        x = ari_Points[ptidx].x;
        y = ari_Points[ptidx].y;
        if ((x >= start_x) && (x <= end_x) && (y >= start_y) && (y <= end_y))
        {
            ptidx = tri->points[1];
            x = ari_Points[ptidx].x;
            y = ari_Points[ptidx].y;
            if ((x >= start_x) && (x <= end_x) && (y >= start_y) && (y <= end_y))
            {
                ptidx = tri->points[2];
                x = ari_Points[ptidx].x;
                y = ari_Points[ptidx].y;
                if ((x >= start_x) && (x <= end_x) && (y >= start_y) && (y <= end_y))
                {
                    tri->tree_alt = ntree_alt;
                    area += triangle_area1(tri_idx);
                    if (2 * (end_x - start_x) * (end_y - start_y) == area) {
                        break;
                    }
                }
            }
        }
    }
}
HOOK_DK_FUNC(brute_fill_rectangle)

#define fill_rectangle(start_x, start_y, end_x, end_y, a5) fill_rectangle_f(start_x, start_y, end_x, end_y, a5, __func__)
void fill_rectangle_f(long start_x, long start_y, long end_x, long end_y, unsigned char nav_colour, const char *func_name)
{
    //_DK_fill_rectangle(start_x, start_y, end_x, end_y, a5); return;
    long tri_n0;
    long tri_k0;
    long tri_n1;
    long tri_k1;
    long tri_n2;
    long tri_k2;
    long tri_n3;
    long tri_k3;
    long tri_area;
    long req_area;
    req_area = 2 * (end_x - start_x) * (end_y - start_y);
    if (!edge_find(start_x, start_y, start_x, end_y, &tri_n0, &tri_k0))
    {
        ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)start_x, (int)start_y, (int)start_x, (int)end_y);
        return;
    }
    Triangles[tri_n0].tree_alt = nav_colour;
    tri_area = triangle_area1(tri_n0);
    if (tri_area == req_area) {
        return;
    }
    if (!edge_find(end_x, end_y, end_x, start_y, &tri_n1, &tri_k1))
    {
        ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)end_x, (int)end_y, (int)end_x, (int)start_y);
        return;
    }
    if (tri_n1 != tri_n0)
    {
        Triangles[tri_n1].tree_alt = nav_colour;
        tri_area += triangle_area1(tri_n1);
    }
    if (tri_area == req_area) {
        return;
    }
    if (!edge_find(end_x, start_y, start_x, start_y, &tri_n2, &tri_k2))
    {
        ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)end_x, (int)start_y, (int)start_x, (int)start_y);
        return;
    }
    if ((tri_n2 != tri_n0) && (tri_n2 != tri_n1))
    {
        Triangles[tri_n2].tree_alt = nav_colour;
        tri_area += triangle_area1(tri_n2);
    }
    if (tri_area == req_area) {
        return;
    }
    if (!edge_find(start_x, end_y, end_x, end_y, &tri_n3, &tri_k3))
    {
        ERRORMSG("%s: edge from (%d,%d) to (%d,%d) not found",func_name,(int)start_x, (int)end_y, (int)end_x, (int)end_y);
        return;
    }
    if ((tri_n3 != tri_n0) && (tri_n1 != tri_n3) && (tri_n2 != tri_n3))
    {
        Triangles[tri_n3].tree_alt = nav_colour;
        tri_area += triangle_area1(tri_n3);
    }
    if (tri_area == req_area) {
        return;
    }
    brute_fill_rectangle(start_x, start_y, end_x, end_y, nav_colour);
}

HOOK_DK_FUNC(tri_set_rectangle)
TbBool tri_set_rectangle(long start_x, long start_y, long end_x, long end_y, unsigned char nav_colour)
{
    long sx;
    long sy;
    long ex;
    long ey;
    NAVIDBG(19,"Starting");
    //_DK_tri_set_rectangle(start_x, start_y, end_x, end_y, a5); return true;
    sx = start_x;
    ex = end_x;
    if (sx > ex) {
      sx = end_x;
      ex = start_x;
    }
    sy = start_y;
    ey = end_y;
    if (sy > ey) {
      sy = end_y;
      ey = start_y;
    }
    TbBool r;
    r = true;
    r &= insert_point(sx, sy);
    r &= insert_point(sx, ey);
    r &= insert_point(ex, ey);
    r &= insert_point(ex, sy);
    if (!r) {
        ERRORLOG("Couldn't insert points to make rectangle; start (%d,%d), end (%d,%d)",(int)start_x,(int)start_y,(int)end_x,(int)end_y);
        return r;
    }
    r &= make_edge(sx, sy, sx, ey);
    r &= make_edge(sx, ey, ex, ey);
    r &= make_edge(ex, ey, ex, sy);
    r &= make_edge(ex, sy, sx, sy);
    if (!r) {
        ERRORLOG("Couldn't make edge for rectangle; start (%d,%d), end (%d,%d)",(int)start_x,(int)start_y,(int)end_x,(int)end_y);
        return r;
    }
    fill_rectangle(sx, sy, ex, ey, nav_colour);
    return r;
}

long fringe_scan(long *outfri_x, long *outfri_y, long *outlen_x, long *outlen_y)
{
    long loc_x;
    long sub_y;
    long sub_x;
    int dist_x;
    sub_y = fringe_y2;
    sub_x = 0;
    dist_x = 0;
    for (loc_x = fringe_x1; loc_x < fringe_x2; )
    {
        if (fringe_y[loc_x] < sub_y)
        {
          sub_y = fringe_y[loc_x];
          sub_x = loc_x;
          for (loc_x++; loc_x < fringe_x2; loc_x++)
          {
              if (sub_y != fringe_y[loc_x])
                break;
          }
          dist_x = loc_x - sub_x;
        } else
        {
          loc_x++;
        }
    }
    if (sub_y == fringe_y2) {
        return 0;
    }
    *outfri_x = sub_x;
    *outfri_y = sub_y;
    *outlen_x = dist_x;
    *outlen_y = fringe_y2 - sub_y;
    return 1;
}

long fringe_get_rectangle(long *outfri_x1, long *outfri_y1, long *outfri_x2, long *outfri_y2, unsigned char *oval)
{
    NAVIDBG(19,"Starting");
    long fri_x;
    long fri_y;
    long len_x;
    long len_y;
    len_x = 0;
    len_y = 0;
    if (!fringe_scan(&fri_x,&fri_y,&len_x,&len_y)) {
        return 0;
    }
    unsigned char *fri_map;
    fri_map = &fringe_map[256 * fri_y + fri_x];
    // Find dx and dy
    long dx;
    long dy;
    for (dx = 1; dx < len_x; dx++)
    {
        if (fri_map[dx] != fri_map[0]) {
            break;
        }
    }
    for (dy = 1; dy < len_y; dy++)
    {
        // Our data is 0-terminated, so we can use string functions to compare
        if (memcmp(&fri_map[256*dy], &fri_map[0], dx) != 0) {
            break;
        }
    }
    long i;
    for (i = 0; i < dx; i++) {
        fringe_y[fri_x+i] = fri_y+dy;
    }
    *oval = fri_map[0];
    *outfri_x1 = fri_x;
    *outfri_y1 = fri_y;
    *outfri_x2 = fri_x + dx;
    *outfri_y2 = fri_y + dy;
    return 1;
}

HOOK_DK_FUNC(border_unlock)
void border_unlock(long a1, long a2, long a3, long a4)
{
    struct EdgePoint *ept;
    long ept_id;
    long tri_idx;
    long cor_idx;
    long nerr;
    //TODO PATHFINDING triangulate_area sub-function, verify
    //_DK_border_unlock(a1, a2, a3, a4); return;
    edge_points_clean();
    edge_unlock_record_and_regions(a1, a2, a1, a4);
    edge_unlock_record_and_regions(a1, a4, a3, a4);
    edge_unlock_record_and_regions(a3, a4, a3, a2);
    edge_unlock_record_and_regions(a3, a2, a1, a2);
    if (ix_EdgePoints < 1)
        return;
    nerr = 0;
    for (ept_id = ix_EdgePoints; ept_id > 0; ept_id--)
    {
        ept = edge_point_get(ept_id-1);
        if (!point_find(ept->field_0, ept->field_4, &tri_idx, &cor_idx))
        {
            nerr++;
            continue;
        }
        if (point_redundant(tri_idx, cor_idx))
        {
            delete_point(tri_idx, cor_idx);
        }
    }
    if (nerr != 0)
    {
        ERRORLOG("Out of %ld edge points, %ld were not found",(long)ix_EdgePoints,nerr);
    }
}

TbBool triangulation_border_start(long *border_a, long *border_b)
{
    struct Triangle *tri;
    long tri_idx;
    long brd_idx;
    long i;
    long k;
    //_DK_triangulation_border_start(border_a, border_b);
    // First try - border
    for (brd_idx=0; brd_idx < ix_Border; brd_idx++)
    {
        tri_idx = Border[brd_idx];
        if (get_triangle_tree_alt(tri_idx) != -1)
        {
            tri = get_triangle(tri_idx);
            for (i=0; i < 3; i++)
            {
                k = tri->tags[i];
                if (k == -1)
                {
                    *border_a = tri_idx;
                    *border_b = i;
                    return true;
                }
            }
        }
    }
    // Second try - triangles
    for (tri_idx=0; tri_idx < ix_Triangles; tri_idx++)
    {
        if (get_triangle_tree_alt(tri_idx) != -1)
        {
            tri = get_triangle(tri_idx);
            for (i=0; i < 3; i++)
            {
                k = tri->tags[i];
                if (k == -1)
                {
                    *border_a = tri_idx;
                    *border_b = i;
                    return true;
                }
            }
        }
    }
    // Failure
    ERRORLOG("not found");
    *border_a = 0;
    *border_b = 0;
    return false;
}

long get_navigation_colour_for_door(long stl_x, long stl_y)
{
    struct Thing *doortng;
    long owner;
    doortng = get_door_for_position(stl_x, stl_y);
    if (thing_is_invalid(doortng))
    {
        ERRORLOG("Cannot find door for flagged position (%d,%d)",(int)stl_x,(int)stl_y);
        return (1 << NAVMAP_FLOORHEIGHT_BIT);
    }
    if (doortng->door.is_locked == 0)
    {
        return (1 << NAVMAP_FLOORHEIGHT_BIT);
    }
    if (is_hero_thing(doortng))
        owner = NAVMAP_OWNER_HERO;
    else
    if (doortng->owner == game.neutral_player_num)
        owner = NAVMAP_OWNER_NEUTRAL;
    else
        owner = doortng->owner;
    if (owner > NAVMAP_OWNERSELECT_MAX)
    {
        ERRORLOG("Doors at (%d,%d) have outranged player %ld",(int)stl_x,(int)stl_y,owner);
        return (1 << NAVMAP_FLOORHEIGHT_BIT);
    }
    return ((owner+1) << NAVMAP_OWNERSELECT_BIT) | (1 << NAVMAP_FLOORHEIGHT_BIT);
}

long get_navigation_colour_for_cube(long stl_x, long stl_y)
{
    long tcube;
    long cube_pos;
    long i;
    i = get_floor_filled_subtiles_at(stl_x, stl_y);
    if (i > NAVMAP_FLOORHEIGHT_MAX)
      i = NAVMAP_FLOORHEIGHT_MAX;
    tcube = get_top_cube_at(stl_x, stl_y, &cube_pos);
    if (cube_is_lava(tcube) || (cube_pos<4 && cube_is_sacrificial(tcube)))
      i |= NAVMAP_UNSAFE_SURFACE;
    return i;
}

long get_navigation_colour(long stl_x, long stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    if ((mapblk->flags & SlbAtFlg_IsDoor) != 0)
    {
        return get_navigation_colour_for_door(stl_x, stl_y);
    }
    if ((mapblk->flags & SlbAtFlg_Blocking) != 0)
    {
        return (NAVMAP_FLOORHEIGHT_MAX << NAVMAP_FLOORHEIGHT_BIT);
    }
    return get_navigation_colour_for_cube(stl_x, stl_y);
}

long uniform_area_colour(const unsigned char *imap, long start_x, long start_y, long end_x, long end_y)
{
    long uniform;
    long x;
    long y;
    uniform = imap[navmap_tile_number(start_x,start_y)];
    for (y = start_y; y < end_y; y++)
    {
        for (x = start_x; x < end_x; x++)
        {
            if (imap[navmap_tile_number(x,y)] != uniform)
            {
                return -1;
            }
        }
    }
    return uniform;
}

void triangulation_border_init(void)
{
    long border_a;
    long border_b;
    long tri_a;
    long tri_b;
    long i;
    long n;
    NAVIDBG(9,"Starting");
    triangulation_border_start(&border_a, &border_b);
    tri_a = border_a;
    tri_b = border_b;
    ix_Border = 0;
    do
    {
      Border[ix_Border] = tri_a;
      ix_Border++;
      if (ix_Border >= BORDER_LENGTH)
      {
        ERRORLOG("Border overflow");
        break;
      }
      tri_b = MOD3[tri_b+1];
      while ( 1 )
      {
          i = Triangles[tri_a].tags[tri_b];
          n = link_find(i, tri_a);
          if (n < 0) {
              break;
          }
          tri_b = MOD3[n+1];
          tri_a = i;
      }
    }
    while ((tri_a != border_a) || (tri_b != border_b));
    NAVIDBG(19,"Finished");
}

HOOK_DK_FUNC(triangulation_initxy)
void triangulation_initxy(long startx, long starty, long endx, long endy)
{
    long i;
    //_DK_triangulation_initxy(startx, starty, endx, endy);return;
    for (i=0; i < TRIANLGLES_COUNT; i++)
    {
        struct Triangle *tri;
        tri = &Triangles[i];
        tri->tree_alt = 255;
    }
    tri_initialised = 1;
    triangulation_initxy_points(startx, starty, endx, endy);
    triangulation_init_triangles(0, 1, 2, 3);
    edgelen_set(0);
    edgelen_set(1);
    triangulation_init_cache(triangle_find_first_used());
    triangulation_init_regions();
}

void triangulation_init(void)
{
    if (!tri_initialised)
    {
        tri_initialised = 1;
        triangulation_initxy(-256, -256, 512, 512);
    }
}

HOOK_DK_FUNC(triangulate_area)
TbBool triangulate_area(unsigned char *imap, long start_x, long start_y, long end_x, long end_y)
{
    TbBool one_tile;
    TbBool not_whole_map;
    long colour;
    unsigned char ccolour;
    long rect_sx;
    long rect_sy;
    long rect_ex;
    long rect_ey;
    TbBool r;
    long i;
    r = true;
    LastTriangulatedMap = imap;
    NAVIDBG(9,"Area from (%03ld,%03ld) to (%03ld,%03ld) with %04ld triangles",start_x,start_y,end_x,end_y,count_Triangles);
    //_DK_triangulate_area(imap, start_x, start_y, end_x, end_y); return true;
    // Switch coords to make end_x larger than start_x
    if (end_x < start_x)
    {
        i = start_x;
        start_x = end_x;
        end_x = i;
    }
    if (end_y < start_y)
    {
        i = start_y;
        start_y = end_y;
        end_y = i;
    }
    if ((start_x == -1) || (start_y == -1) || (end_x == start_x) || (end_y == start_y))
    {
        NAVIDBG(9,"Invalid area bounds");
        return false;
    }
    // Prepare some basic logic information
    one_tile = (((end_x - start_x) == 1) && ((end_y - start_y) == 1));
    not_whole_map = (start_x != 0) || (start_y != 0) || (end_x != 256) || (end_y != 256);
    // If coordinates are out of range, update the whole map area
    if ((start_x < 1) || (start_y < 1) || (end_x >= 255) || (end_y >= 255))
    {
        one_tile = 0;
        not_whole_map = 0;
        start_x = 0;
        end_x = 256;
        start_y = 0;
        end_y = 256;
    }
    triangulation_init();
    if ( not_whole_map )
    {
        r &= border_clip_horizontal(imap, start_x, end_x, start_y, 0);
        r &= border_clip_horizontal(imap, start_x, end_x, end_y, -1);
        r &= border_clip_vertical(imap, start_x, -1, start_y, end_y);
        r &= border_clip_vertical(imap, end_x, 0, start_y, end_y);
        r &= border_lock(start_x, start_y, end_x, end_y);
        if ( !one_tile ) {
            border_internal_points_delete(start_x, start_y, end_x, end_y);
        }
    } else
    {
        triangulation_initxy(-256, -256, 512, 512);
        tri_set_rectangle(start_x, start_y, end_x, end_y, 0);
    }
    colour = -1;
    if ( one_tile )
    {
        colour = imap[navmap_tile_number(start_x,start_y)];
    } else
    if ((not_whole_map) && (end_x - start_x <= 3) && (end_y - start_y <= 3))
    {
        colour = uniform_area_colour(imap, start_x, start_y, end_x, end_y);
    }

    if (colour == -1)
    {
        fringe_map = imap;
        fringe_x1 = start_x;
        fringe_y1 = start_y;
        fringe_x2 = end_x;
        fringe_y2 = end_y;
        for (i = start_x; i < end_x; i++)
        {
            fringe_y[i] = start_y;
        }
        while ( fringe_get_rectangle(&rect_sx, &rect_sy, &rect_ex, &rect_ey, &ccolour) )
        {
            if ((ccolour) || (not_whole_map))
            {
                if (!tri_set_rectangle(rect_sx, rect_sy, rect_ex, rect_ey, ccolour))
                    break; // Run out of triangle space
                //JUSTLOG("fringe_rect (%d, %d)-(%d,%d) triangles ix:%d, free:%d\n", rect_sx, rect_sy, rect_ex, rect_ey,
                //          get_ix_points(), get_free_points());
                delaunay_seeded(rect_sx, rect_sy, rect_ex, rect_ey);
            }
        }
    } else
    {
        tri_set_rectangle(start_x, start_y, end_x, end_y, colour);
    }
    delaunay_seeded(start_x, start_y, end_x, end_y);
    if ( not_whole_map )
        border_unlock(start_x, start_y, end_x, end_y);
    triangulation_border_init();
    NAVIDBG(9,"Done");
    return r;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
