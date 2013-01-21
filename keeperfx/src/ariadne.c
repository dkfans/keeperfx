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
#include "ariadne_navitree.h"
#include "ariadne_regions.h"
#include "ariadne_tringls.h"
#include "ariadne_points.h"
#include "ariadne_edge.h"
#include "ariadne_findcache.h"
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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT AriadneReturn _DK_ariadne_initialise_creature_route(struct Thing *thing, struct Coord3d *pos, long ptstart_x, unsigned char ptstart_y);
DLLIMPORT AriadneReturn _DK_creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, long ptstart_y, unsigned char a5);
DLLIMPORT void _DK_path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size);
DLLIMPORT long _DK_route_to_path(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y, long *a5, long a6, struct Path *path, long *a8);
DLLIMPORT void _DK_path_out_a_bit(struct Path *path, long *ptfind_y);
DLLIMPORT void _DK_gate_navigator_init8(struct Pathway *pway, long ptfind_y, long ptstart_x, long ptstart_y, long a5, long a6, unsigned char a7);
DLLIMPORT void _DK_route_through_gates(struct Pathway *pway, struct Path *path, long ptstart_x);
DLLIMPORT long _DK_triangle_findSE8(long, long);
DLLIMPORT long _DK_ma_triangle_route(long ptfind_x, long ptfind_y, long *ptstart_x);
DLLIMPORT void _DK_edgelen_init(void);
DLLIMPORT long _DK_get_navigation_colour(long stl_x, long stl_y);
DLLIMPORT void _DK_triangulate_area(unsigned char *imap, long sx, long sy, long ex, long ey);
DLLIMPORT long _DK_init_navigation(void);
DLLIMPORT long _DK_update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y);
DLLIMPORT void _DK_border_clip_horizontal(const unsigned char *imap, long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y);
DLLIMPORT void _DK_border_clip_vertical(const unsigned char *imap, long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y);
DLLIMPORT void _DK_edge_lock(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y);
DLLIMPORT void _DK_border_internal_points_delete(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y);
DLLIMPORT void _DK_tri_set_rectangle(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y, unsigned char a5);
DLLIMPORT long _DK_fringe_get_rectangle(long *ptfind_x, long *ptfind_y, long *ptstart_x, long *ptstart_y, unsigned char *a5);
DLLIMPORT void _DK_border_unlock(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y);
DLLIMPORT void _DK_triangulation_border_start(long *ptfind_x, long *ptfind_y);
DLLIMPORT long _DK_edge_find(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y, long *a5, long *a6);
DLLIMPORT long _DK_make_3or4point(long *ptfind_x, long *ptfind_y);
DLLIMPORT long _DK_delete_4point(long ptfind_x, long ptfind_y);
DLLIMPORT long _DK_delete_3point(long ptfind_x, long ptfind_y);
DLLIMPORT long _DK_triangle_route_do_fwd(long ptfind_x, long ptfind_y, long *ptstart_x, long *ptstart_y);
DLLIMPORT long _DK_triangle_route_do_bak(long ptfind_x, long ptfind_y, long *ptstart_x, long *ptstart_y);
DLLIMPORT void _DK_ariadne_pull_out_waypoint(struct Thing *thing, struct Ariadne *arid, long ptstart_x, struct Coord3d *pos);
DLLIMPORT long _DK_ariadne_init_movement_to_current_waypoint(struct Thing *thing, struct Ariadne *arid);
DLLIMPORT unsigned char _DK_ariadne_get_next_position_for_route(struct Thing *thing, struct Coord3d *finalpos, long ptstart_y, struct Coord3d *nextpos, unsigned char a5);
DLLIMPORT unsigned char _DK_ariadne_update_state_on_line(struct Thing *thing, struct Ariadne *arid);
DLLIMPORT unsigned char _DK_ariadne_update_state_wallhug(struct Thing *thing, struct Ariadne *arid);
DLLIMPORT long _DK_ariadne_get_wallhug_angle(struct Thing *thing, struct Ariadne *arid);
DLLIMPORT unsigned char _DK_ariadne_init_wallhug(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos);
DLLIMPORT void _DK_insert_point(long pt_x, long pt_y);
DLLIMPORT void _DK_make_edge(long start_x, long end_x, long start_y, long end_y);
DLLIMPORT long _DK_tri_split2(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y, long a5);
DLLIMPORT void _DK_tri_split3(long ptfind_x, long ptfind_y, long ptstart_x);
DLLIMPORT long _DK_pointed_at8(long pos_x, long pos_y, long *retpos_x, long *retpos_y);
DLLIMPORT long _DK_triangle_brute_find8_near(long pos_x, long pos_y);
DLLIMPORT void _DK_waypoint_normal(long ptfind_x, long ptfind_y, long *norm_x, long *norm_y);
DLLIMPORT long _DK_gate_route_to_coords(long trAx, long trAy, long trBx, long trBy, long *a5, long a6, struct Pathway *pway, long a8);
DLLIMPORT long _DK_fill_concave(long a1, long a2, long speed);
/******************************************************************************/
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
#ifdef __cplusplus
}
#endif
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

long thing_nav_block_sizexy(const struct Thing *thing)
{
    long i;
    i = thing->sizexy;
    if (i >= sizeof(actual_sizexy_to_nav_block_sizexy_table)/sizeof(actual_sizexy_to_nav_block_sizexy_table[0]))
        i = sizeof(actual_sizexy_to_nav_block_sizexy_table)/sizeof(actual_sizexy_to_nav_block_sizexy_table[0])-1;
    if (i < 0)
        i = 0;
    return actual_sizexy_to_nav_block_sizexy_table[i];
}

long thing_nav_sizexy(const struct Thing *thing)
{
    long i;
    i = thing->sizexy;
    if (i >= sizeof(actual_sizexy_to_nav_sizexy_table)/sizeof(actual_sizexy_to_nav_sizexy_table[0]))
        i = sizeof(actual_sizexy_to_nav_sizexy_table)/sizeof(actual_sizexy_to_nav_sizexy_table[0])-1;
    if (i < 0)
        i = 0;
    return actual_sizexy_to_nav_sizexy_table[i];
}

/*
unsigned char tag_current;
unsigned char Tags[9000];
long tree_val[9001];
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
long route_to_path(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y, long *a5, long a6, struct Path *path, long *a8);
void path_out_a_bit(struct Path *path, long *ptfind_y);
void gate_navigator_init8(struct Pathway *pway, long trAx, long trAy, long trBx, long trBy, long a6, unsigned char a7);
void route_through_gates(struct Pathway *pway, struct Path *path, long ptstart_x);
long triangle_findSE8(long ptfind_x, long ptfind_y);
long ma_triangle_route(long ptfind_x, long ptfind_y, long *ptstart_x);
void edgelen_init(void);
/******************************************************************************/

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
    long stl_x,stl_y;
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

long get_navtree_owner(long treeI)
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
    //return _DK_init_navigation();
    init_navigation_map();
    triangulate_map(IanMap);
    nav_rulesA2B = navigation_rule_normal;
    game.field_14EA4B = 1;
    return 1;
}

long update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y)
{
    long sx,sy,ex,ey;
    long x,y;
    //return _DK_update_navigation_triangulation(start_x, start_y, end_x, end_y);
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

long route_to_path(long ptfind_x, long ptfind_y, long ptstart_x, long ptstart_y, long *a5, long a6, struct Path *path, long *a8)
{
    //Note: uses LbCompareMultiplications()
    NAVIDBG(19,"Starting");
    return _DK_route_to_path(ptfind_x, ptfind_y, ptstart_x, ptstart_y, a5, a6, path, a8);
}

void waypoint_normal(long a1, long a2, long *norm_x, long *norm_y)
{
    _DK_waypoint_normal(a1, a2, norm_x, norm_y);
}

void path_out_a_bit(struct Path *path, long *route)
{
    struct PathWayPoint *ppoint;
    long *wpoint;
    long tip_x,tip_y;
    long norm_x,norm_y;
    long prev_pt,curr_pt;
    long link_fwd,link_bak;
    long i;
    //_DK_path_out_a_bit(path, a2);
    wpoint = &wayPoints.field_10[0];
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

long gate_route_to_coords(long trAx, long trAy, long trBx, long trBy, long *a5, long a6, struct Pathway *pway, long a8)
{
    //Note: uses LbCompareMultiplications()
    return _DK_gate_route_to_coords(trAx, trAy, trBx, trBy, a5, a6, pway, a8);
}

void gate_navigator_init8(struct Pathway *pway, long trAx, long trAy, long trBx, long trBy, long a6, unsigned char a7)
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
            pway->points_num = gate_route_to_coords(trAx, trAy, trBx, trBy, tree_route, tree_routelen, pway, a6);
        }
    }
}

void route_through_gates(struct Pathway *pway, struct Path *path, long mag)
{
    struct PathPoint *ppoint;
    struct PathWayPoint *wpoint;
    long i;
    //_DK_route_through_gates(pway, path, a3);
    if (mag > 16383)
        mag = 16383;
    if (mag < 0)
        mag = 0;
    path->field_0 = pway->field_0;
    path->field_4 = pway->field_4;
    path->field_8 = pway->field_8;
    path->field_C = pway->field_C;
    path->waypoints_num = pway->points_num;
    ppoint = &pway->points[0];
    wpoint = &path->waypoints[0];
    for (i=0; i < pway->points_num-1; i++)
    {
        if (ppoint->field_18)
        {
            wpoint->x = ppoint->field_8 - (mag * (ppoint->field_8 - ppoint->field_0) >> 14);
            wpoint->y = ppoint->field_C - (mag * (ppoint->field_C - ppoint->field_4) >> 14);
        } else
        {
            wpoint->x = ppoint->field_0 + (mag * (ppoint->field_8 - ppoint->field_0) >> 14);
            wpoint->y = ppoint->field_4 + (mag * (ppoint->field_C - ppoint->field_4) >> 14);
        }
        wpoint++;
        ppoint++;
    }
    path->waypoints[i].x = pway->field_8;
    path->waypoints[i].y = pway->field_C;
}

long triangle_findSE8(long ptfind_x, long ptfind_y)
{
    //Note: uses LbCompareMultiplications()
    return _DK_triangle_findSE8(ptfind_x, ptfind_y);
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

void tri_dispose(long tri_idx)
{
  long pfree_idx;
  pfree_idx = free_Triangles;
  free_Triangles = tri_idx;
  Triangles[tri_idx].field_6 = pfree_idx;
  Triangles[tri_idx].field_C = 255;
  count_Triangles--;
}
*/

TbBool triangulation_border_tag(void)
{
    if (border_tags_to_current(Border, ix_Border) != ix_Border)
    {
        ERRORLOG("Some border Tags were outranged");
        return false;
    }
    return true;
}

long dest_node(long a1, long a2)
{
    long n;
    n = Triangles[a1].tags[a2];
    if (n < 0)
        return -1;
    if (!nav_rulesA2B(get_triangle_tree_alt(a1), get_triangle_tree_alt(n)))
        return -1;
    return n;
}

long cost_to_start(long tri_idx)
{
    long long len_x,len_y;
    long long mincost,newcost;
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

long pointed_at8(long pos_x, long pos_y, long *retpos_x, long *retpos_y)
{
    //Note: uses LbCompareMultiplications()
    return _DK_pointed_at8(pos_x, pos_y, retpos_x, retpos_y);
}

long triangle_brute_find8_near(long pos_x, long pos_y)
{
    return _DK_triangle_brute_find8_near(pos_x, pos_y);
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
    struct Triangle *tri;
    long ttriH1,ttriH2;
    long mvcost,navrule;
    long nskipped;
    long i,k,n;
    NAVIDBG(19,"Starting");
    //return _DK_triangle_route_do_fwd(ttriA, ttriB, route, routecost);
    tags_init();
    if ((ix_Border < 0) || (ix_Border >= BORDER_LENGTH))
    {
        ERRORLOG("Border overflow");
        ix_Border = BORDER_LENGTH-1;
    }
    triangulation_border_tag();

    naviheap_init();
    nskipped = 0;
    if (!navitree_add(ttriB, ttriB, 1))
        nskipped++;
    while (ttriA != naviheap_top())
    {
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
        while ( 1 )
        {
            tri = get_triangle(ttriH1);
            n = 0;
            for (i = 0; i < 3; i++)
            {
              k = tri->tags[i];
              if (!is_current_tag(k))
              {
                if ( fits_thro(ttriH1, n) )
                {
                    navrule = nav_rulesA2B(get_triangle_tree_alt(k), get_triangle_tree_alt(ttriH1));
                    if ( navrule )
                    {
                        mvcost = cost_to_start(k);
                      if (navrule == 2)
                          mvcost *= 16;
                      if (!navitree_add(k,ttriH1,mvcost))
                          nskipped++;
                    }
                }
              }
              n++;
            }
            if (ttriH2 == -1)
              break;
            ttriH1 = ttriH2;
            ttriH2 = -1;
        }
    }
    if (nskipped != 0)
    {
        NAVIDBG(6,"navigate heap full, %ld points ignored",nskipped);
    }

    NAVIDBG(19,"Nearly finished");
    if (naviheap_empty())
        return -1;
    i = copy_tree_to_route(ttriA, ttriB, route, TRIANLGLES_COUNT+1);
    if (i < 0)
    {
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
    struct Triangle *tri;
    long ttriH1,ttriH2;
    long mvcost,navrule;
    long nskipped;
    long i,k,n;
    NAVIDBG(19,"Starting");
    //return _DK_triangle_route_do_bak(ttriA, ttriB, route, routecost);
    tags_init();
    if ((ix_Border < 0) || (ix_Border >= BORDER_LENGTH))
    {
        ERRORLOG("Border overflow");
        ix_Border = BORDER_LENGTH-1;
    }
    triangulation_border_tag();

    naviheap_init();
    nskipped = 0;
    if (!navitree_add(ttriB, ttriB, 1))
        nskipped++;
    while (ttriA != naviheap_top())
    {
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
        while ( 1 )
        {
            tri = get_triangle(ttriH1);
            n = 0;
            for (i = 0; i < 3; i++)
            {
              k = tri->tags[i];
              if (!is_current_tag(k))
              {
                if ( fits_thro(ttriH1, n) )
                {
                    navrule = nav_rulesA2B(get_triangle_tree_alt(ttriH1), get_triangle_tree_alt(k));
                    if ( navrule )
                    {
                        mvcost = cost_to_start(k);
                      if (navrule == 2)
                          mvcost *= 16;
                      if (!navitree_add(k,ttriH1,mvcost))
                          nskipped++;
                    }
                }
              }
              n++;
            }
            if (ttriH2 == -1)
              break;
            ttriH1 = ttriH2;
            ttriH2 = -1;
        }
    }
    if (nskipped != 0)
    {
        NAVIDBG(6,"navigate heap full, %ld points ignored",nskipped);
    }

    NAVIDBG(19,"Nearly finished");
    if (naviheap_empty())
        return -1;
    i = copy_tree_to_route(ttriA, ttriB, route, TRIANLGLES_COUNT+1);
    if (i < 0)
    {
        erstat_inc(ESE_BadRouteTree);
        ERRORDBG(6,"route length overflow");
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
    long len_fwd,len_bak;
    long par_fwd,par_bak;
    long rcost_fwd,rcost_bak;
    long tx,ty;
    long i;
    // We need to make testing system for routing, then fix the rewritten code
    // and compare results with the original code.
    //return _DK_ma_triangle_route(ttriA, ttriB, routecost);

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
    if (par_fwd < par_fwd) // //TODO PATHFINDING maybe condition should be (par_fwd < par_bak) ?
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
    _DK_edgelen_init();
}

TbBool ariadne_creature_reached_position(const struct Thing *thing, const struct Coord3d *pos)
{
    if (thing->mappos.x.val != pos->x.val)
        return false;
    if (thing->mappos.y.val != pos->y.val)
        return false;
    return true;
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
    if (arid->total_waypoints-wpoint_id == 1)
    {
      pos->x.val = arid->waypoints[wpoint_id].x.val;
      pos->y.val = arid->waypoints[wpoint_id].y.val;
      return;
    }
    wp = &arid->waypoints[wpoint_id];
    size_radius = (thing_nav_sizexy(thing) >> 1);
    pos->x.val = wp->x.val + 1;
    pos->x.stl.pos = 0;
    pos->y.val = wp->y.val + 1;
    pos->y.stl.pos = 0;

    if (wp->x.stl.pos == 255)
    {
      if (wp->y.stl.pos == 255)
      {
          pos->x.val -= size_radius+1;
          pos->y.val -= size_radius+1;
      } else
      if (wp->y.stl.pos != 0)
      {
          pos->x.val -= size_radius+1;
          pos->y.val += size_radius;
      } else
      {
          pos->x.val -= size_radius+1;
      }
    } else
    if (wp->x.stl.pos != 0)
    {
        if (wp->y.stl.pos == 255)
        {
            pos->x.val += size_radius;
            pos->y.val -= size_radius+1;
        } else
        if (wp->y.stl.pos != 0)
        {
            pos->x.val += size_radius;
            pos->y.val += size_radius;
        } else
        {
            pos->x.val += size_radius;
        }
    } else
    {
        if (wp->y.stl.pos == 255)
        {
          pos->y.val -= size_radius+1;
        } else
        if (wp->y.stl.pos != 0)
        {
            pos->y.val += size_radius;
        }
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

long ariadne_creature_blocked_by_wall_at(struct Thing *thing, const struct Coord3d *pos)
{
    struct Coord3d mvpos;
    long zmem,ret;
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

long ariadne_get_wallhug_angle(struct Thing *thing, struct Ariadne *arid)
{
    return _DK_ariadne_get_wallhug_angle(thing, arid);
}

AriadneReturn ariadne_init_wallhug(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos)
{
    return _DK_ariadne_init_wallhug(thing, arid, pos);
}

long ariadne_get_blocked_flags(struct Thing *thing, const struct Coord3d *pos)
{
    struct Coord3d lpos;
    unsigned long flags;
    lpos.x.val = pos->x.val;
    lpos.y.val = thing->mappos.y.val;
    lpos.z.val = thing->mappos.z.val;
    flags = 0;
    if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
        flags |= 0x01;
    lpos.x.val = thing->mappos.x.val;
    lpos.y.val = pos->y.val;
    lpos.z.val = thing->mappos.z.val;
    if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
        flags |= 0x02;
    if (flags == 0)
    {
        lpos.x.val = pos->x.val;
        lpos.y.val = pos->y.val;
        lpos.z.val = thing->mappos.z.val;
        if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
          flags |= 0x04;
    }
    return flags;
}

TbBool blocked_by_door_at(struct Thing *thing, struct Coord3d *pos, unsigned long blk_flags)
{
    long radius;
    long start_x,end_x,start_y,end_y;
    long stl_x,stl_y;

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
    long delta_x,delta_y;
    unsigned long blk_flags;
    //return _DK_ariadne_init_movement_to_current_waypoint(thing, arid);
    angle = get_angle_xy_to(&thing->mappos, &arid->current_waypoint_pos);
    delta_x = ((long)arid->move_speed * LbSinL(angle)) >> 16;
    delta_y = ((long)arid->move_speed * LbCosL(angle)) >> 16;
    requested_pos.x.val = (long)thing->mappos.x.val + delta_x;
    requested_pos.y.val = (long)thing->mappos.y.val - delta_y;
    requested_pos.z.val = get_thing_height_at(thing, &requested_pos);
    if (!ariadne_creature_blocked_by_wall_at(thing, &requested_pos))
    {
        arid->field_21 = 1;
        return 1;
    }

    blk_flags = ariadne_get_blocked_flags(thing, &requested_pos);
    if (blocked_by_door_at(thing, &requested_pos, blk_flags))
    {
        arid->field_21 = 1;
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
    arid->field_21 = 3;
    arid->manoeuvre_state = 1;
    return 1;
}

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
    arid->field_1E = 0;
    return AridRet_OK;
}

AriadneReturn ariadne_prepare_creature_route_to_target_f(const struct Thing *thing, struct Ariadne *arid,
    const struct Coord3d *srcpos, const struct Coord3d *dstpos, long speed, unsigned char no_owner, const char *func_name)
{
    struct Path path;
    long nav_sizexy;
    long i,k;
    NAVIDBG(18,"%s: The %s from %3d,%3d to %3d,%3d", func_name, thing_model_name(thing),
        (int)srcpos->x.stl.num, (int)srcpos->y.stl.num, (int)dstpos->x.stl.num, (int)dstpos->y.stl.num);
    LbMemorySet(&path, 0, sizeof(struct Path));
    arid->startpos.x.val = srcpos->x.val;
    arid->startpos.y.val = srcpos->y.val;
    arid->startpos.z.val = srcpos->z.val;
    arid->endpos.x.val = dstpos->x.val;
    arid->endpos.y.val = dstpos->y.val;
    arid->endpos.z.val = dstpos->z.val;
    // Set the required parameters
    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(thing);
    if (no_owner)
        owner_player_navigating = -1;
    else
        owner_player_navigating = thing->owner;
    nav_sizexy = thing_nav_block_sizexy(thing);
    if (nav_sizexy > 0) nav_sizexy--;
    // Find the path
    path_init8_wide_f(&path,
        arid->startpos.x.val, arid->startpos.y.val,
        arid->endpos.x.val, arid->endpos.y.val, -2, nav_sizexy, func_name);
    // Reset globals
    nav_thing_can_travel_over_lava = 0;
    owner_player_navigating = -1;
    if (path.waypoints_num <= 0) {
        NAVIDBG(18,"%s: Cannot find route", func_name);
        return AridRet_Val2;
    }
    // Fill total waypoints number
    if (path.waypoints_num <= 255) {
        arid->total_waypoints = path.waypoints_num;
    } else {
        WARNLOG("%s: The %d waypoints is too many - cutting down", func_name,(int)path.waypoints_num);
        arid->total_waypoints = 255;
    }
    // Fill stored waypoints (up to ARID_WAYPOINTS_COUNT)
    if (arid->total_waypoints < ARID_WAYPOINTS_COUNT) {
        arid->stored_waypoints = arid->total_waypoints;
    } else {
        arid->stored_waypoints = ARID_WAYPOINTS_COUNT;
    }
    k = 0;
    for (i = 0; i < arid->stored_waypoints; i++)
    {
        arid->waypoints[i].x.val = path.waypoints[k].x;
        arid->waypoints[i].y.val = path.waypoints[k].y;
        k++;
    }
    arid->current_waypoint = 0;
    arid->field_1E = no_owner;
    arid->pos_12.x.val = thing->mappos.x.val;
    arid->pos_12.y.val = thing->mappos.y.val;
    arid->pos_12.z.val = thing->mappos.z.val;
    arid->move_speed = speed;
    return AridRet_OK;
}

AriadneReturn ariadne_initialise_creature_route_f(struct Thing *thing, struct Coord3d *pos, long speed, unsigned char storage, const char *func_name)
{
    struct CreatureControl *cctrl;
    struct Ariadne *arid;
    AriadneReturn ret;
    NAVIDBG(18,"%s: Route for %s index %d from %3d,%3d to %3d,%3d", func_name,thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)pos->x.stl.num, (int)pos->y.stl.num);
    //return _DK_ariadne_initialise_creature_route(thing, pos, speed, storage);
    cctrl = creature_control_get_from_thing(thing);
    arid = &(cctrl->arid);
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
        ret = ariadne_prepare_creature_route_to_target_f(thing, arid, &thing->mappos, pos, speed, storage, func_name);
        if (ret != AridRet_OK) {
            NAVIDBG(19,"%s: Failed to prepare route from %5d,%5d to %5d,%5d", func_name,
                (int)thing->mappos.x.val,(int)thing->mappos.y.val, (int)pos->x.val,(int)pos->y.val);
            return ret;
        }
        ariadne_init_current_waypoint(thing, arid);
    }
    ret = ariadne_init_movement_to_current_waypoint(thing, arid);
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
        return AridRet_Val1;
    }
    pos.x.val = arid->endpos.x.val;
    pos.y.val = arid->endpos.y.val;
    pos.z.val = arid->endpos.z.val;
    NAVIDBG(8,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)pos.x.stl.num, (int)pos.y.stl.num);
    return ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->field_1E);
}

TbBool ariadne_creature_reached_waypoint(const struct Thing *thing, const struct Ariadne *arid)
{
    return ariadne_creature_reached_position(thing, &arid->current_waypoint_pos);
}

AriadneReturn ariadne_update_state_manoeuvre_to_position(struct Thing *thing, struct Ariadne *arid)
{
    struct Coord3d pos;
    long dist,angle;
    long i;

    if (ariadne_creature_blocked_by_wall_at(thing, &arid->pos_53))
    {
        pos.x.val = arid->endpos.x.val;
        pos.y.val = arid->endpos.y.val;
        pos.z.val = arid->endpos.z.val;
        NAVIDBG(8,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
            (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)pos.x.stl.num, (int)pos.y.stl.num);
        AriadneReturn aret;
        aret = ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->field_1E);
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
    case 1:
        return ariadne_init_wallhug(thing, arid, &arid->pos_59);
    case 2:
        angle = ariadne_get_wallhug_angle(thing, arid);
        arid->field_60 = angle;
        i = arid->move_speed * LbSinL(angle);
        arid->pos_12.x.val = thing->mappos.x.val + (i >> 16);
        i = arid->move_speed * LbCosL(angle);
        arid->pos_12.y.val = thing->mappos.y.val - (i >> 16);
        arid->field_21 = 2;
        return AridRet_OK;
    default:
        ERRORLOG("Unknown Manoeuvre state %d",(int)arid->manoeuvre_state);
        break;
    }
    return AridRet_OK;
}

AriadneReturn ariadne_update_state_on_line(struct Thing *thing, struct Ariadne *arid)
{
    return _DK_ariadne_update_state_on_line(thing, arid);
}

AriadneReturn ariadne_update_state_wallhug(struct Thing *thing, struct Ariadne *arid)
{
    return _DK_ariadne_update_state_wallhug(thing, arid);
}

AriadneReturn ariadne_get_next_position_for_route(struct Thing *thing, struct Coord3d *finalpos, long speed, struct Coord3d *nextpos, unsigned char a5)
{
    struct CreatureControl *cctrl;
    struct Ariadne *arid;
    AriadneReturn result;
    AriadneReturn aret;
    NAVIDBG(18,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)finalpos->x.stl.num, (int)finalpos->y.stl.num);
    //TODO PATHFINDING rewritten code has been disabled because it has errors (1/2)
    result = _DK_ariadne_get_next_position_for_route(thing, finalpos, speed, nextpos, a5); return result;

    cctrl = creature_control_get_from_thing(thing);
    arid = &cctrl->arid;
    arid->field_22 = 0;
    if ((finalpos->x.val != arid->endpos.x.val)
     || (finalpos->y.val != arid->endpos.y.val)
     || (arid->move_speed != speed))
    {
        aret = ariadne_initialise_creature_route(thing, finalpos, speed, a5);
        if (aret != AridRet_OK)
            return AridRet_Val2;
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
        if (aret == AridRet_Val1)
        {
            arid->endpos.x.val = 0;
            arid->endpos.y.val = 0;
            arid->endpos.z.val = 0;
            nextpos->x.val = thing->mappos.x.val;
            nextpos->y.val = thing->mappos.y.val;
            nextpos->z.val = thing->mappos.z.val;
            return AridRet_Val1;
        } else
        if (aret == AridRet_OK)
        {
            ariadne_init_movement_to_current_waypoint(thing, arid);
        } else
        {
            aret = ariadne_initialise_creature_route(thing, finalpos, speed, a5);
            if (aret != AridRet_OK) {
              return AridRet_PartOK;
            }
        }
    }
    switch (arid->field_21)
    {
    case 1:
        result = ariadne_update_state_on_line(thing, arid);
        nextpos->x.val = arid->pos_12.x.val;
        nextpos->y.val = arid->pos_12.y.val;
        nextpos->z.val = arid->pos_12.z.val;
        break;
    case 2:
        result = ariadne_update_state_wallhug(thing, arid);
        nextpos->x.val = arid->pos_12.x.val;
        nextpos->y.val = arid->pos_12.y.val;
        nextpos->z.val = arid->pos_12.z.val;
        break;
    case 3:
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
        WARNDBG(3,"Update state %d returned %d",(int)arid->field_21,(int)result);
    return result;
}

/** Initializes creature to start moving towards given position.
 *  Hero Gates can be used by the creature to shorten distance.
 * @param thing The creature thing to be moved.
 * @param finalpos Final position coordinates.
 * @param nextpos Next step coordinates.
 * @param a4
 * @param a5
 * @return
 */
AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *finalpos, struct Coord3d *nextpos, long a4, unsigned char a5)
{
    SYNCDBG(18,"Starting");
    //return _DK_creature_follow_route_to_using_gates(thing, finalpos, nextpos, a4, a5);
    if (game.field_14EA4B)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        cctrl->arid.field_23 = 1;
    }
    return ariadne_get_next_position_for_route(thing, finalpos, a4, nextpos, a5);
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
 * @param a6
 * @param nav_size
 */
void path_init8_wide_f(struct Path *path, long start_x, long start_y, long end_x, long end_y,
    long a6, unsigned char nav_size, const char *func_name)
{
    long route_dist;
    //_DK_path_init8_wide(path, start_x, start_y, end_x, end_y, a6, nav_size);
    NAVIDBG(9,"%s: Path from %5ld,%5ld to %5ld,%5ld on turn %lu", func_name, start_x, start_y, end_x, end_y, game.play_gameturn);
    if (a6 == -1)
      WARNLOG("%s: implement random externally", func_name);
    path->field_0 = start_x;
    path->field_4 = start_y;
    path->field_8 = end_x;
    path->field_C = end_y;
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
    if (a6 == -2)
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
        route_through_gates(&ap_GPathway, path, a6);
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
    long l0,l1,n;
    //return _DK_make_3or4point(pt_tri, pt_cor);
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

long delete_4point(long a1, long a2)
{
    //Note: uses LbCompareMultiplications()
    return _DK_delete_4point(a1, a2);
}

long delete_3point(long a1, long a2)
{
    return _DK_delete_3point(a1, a2);
}

TbBool delete_point(long pt_tri, long pt_cor)
{
    long n;
    long ntri,ncor;
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

void tri_split3(long a1, long a2, long a3)
{
    NAVIDBG(19,"Starting");
    _DK_tri_split3(a1, a2, a3);
}

long tri_split2(long a1, long a2, long a3, long a4, long a5)
{
    return _DK_tri_split2(a1, a2, a3, a4, a5);
}

TbBool edge_split(long ntri, long ncor, long pt_x, long pt_y)
{
    long pt_idx;
    long ntr2,ncr2;
    long tri_sp1,tri_sp2;
    NAVIDBG(19,"Starting");
    // Create and fill new point
    pt_idx = point_new();
    if (pt_idx < 0)
        return false;
    point_set(pt_idx, pt_x, pt_y);
    // Find second ntri and ncor
    ntr2 = Triangles[ntri].tags[ncor];
    ncr2 = link_find(ntr2, ntri);
    if (ncr2 < 0)
        return false;
    // Do the splitting
    tri_sp1 = tri_split2(ntri, ncor, pt_x, pt_y, pt_idx);
    tri_sp2 = tri_split2(ntr2, ncr2, pt_x, pt_y, pt_idx);
    Triangles[ntr2].tags[ncr2] = tri_sp1;
    Triangles[ntri].tags[ncor] = tri_sp2;
    return true;
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
    long tipA_x,tipA_y;
    long tipB_x,tipB_y;
    struct Point *pt;

    pt = get_triangle_point(ntri,ncorA);
    tipA_x = pt->x;
    tipA_y = pt->y;
    pt = get_triangle_point(ntri,ncorB);
    tipB_x = pt->x;
    tipB_y = pt->y;
    return LbCompareMultiplications(pt_y-tipA_y, tipB_x-tipA_x, pt_x-tipA_x, tipB_y-tipA_y);
}

TbBool insert_point(long pt_x, long pt_y)
{
    long ntri;
    NAVIDBG(19,"Starting");
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
        return edge_split(ntri, 0, pt_x, pt_y);
    }
    if (triangle_divide_areas_differ(ntri, 1, 2, pt_x, pt_y) == 0)
    {
        return edge_split(ntri, 1, pt_x, pt_y);
    }
    if (triangle_divide_areas_differ(ntri, 2, 0, pt_x, pt_y) == 0)
    {
        return edge_split(ntri, 2, pt_x, pt_y);
    }
    tri_split3(ntri, pt_x, pt_y);
    return true;
}

long fill_concave(long a1, long a2, long a3)
{
    return _DK_fill_concave(a1, a2, a3);
}

void make_edge_sub(long start_tri_id1, long start_cor_id1, long start_tri_id4, long start_cor_id4, long sx, long sy, long ex, long ey)
{
    struct Triangle *tri;
    struct Point *pt;
    struct Point *pt1;
    struct Point *pt2;
    struct Point *pt3;
    long tri_id1, cor_id1;
    long tri_id2, cor_id2;
    long tri_id3, cor_id3;
    long tri_id4, cor_id4;
    long i;
    long cx, cy;
    cor_id1 = start_cor_id1;
    tri_id1 = start_tri_id1;
    cor_id4 = start_cor_id4;
    tri_id4 = start_tri_id4;
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
        if (LbCompareMultiplications(pt->y - ey, sx - ex, pt->x - ex, sy - ey) < 0)
            break;

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
    } while ((cx != sx) || (cy != sy));
}

void make_edge(long start_x, long start_y, long end_x, long end_y)
{
    struct Triangle *tri;
    struct Point *pt;
    long sx,ex,sy,ey;
    long tri_id1,cor_id1;
    long tri_id2,cor_id2;
    long tri_id3,cor_id3;
    long tmpX,tmpY,pt_cor;
    unsigned long k;
    //Note: uses LbCompareMultiplications()
    NAVIDBG(19,"Starting");
    //_DK_make_edge(start_x, start_y, end_x, end_y);
    if ((end_x == start_x) && (end_y == start_y))
        return;
    k = 0;
    sx = start_x;
    sy = start_y;
    ex = end_x;
    ey = end_y;
    while (1)
    {
        if (!point_find(ex, ey, &tri_id1, &cor_id1))
            break;
        if (!point_find(sx, sy, &tri_id2, &cor_id2))
            break;
        pt_cor = pointed_at8(sx << 8, sy << 8, &tri_id1, &cor_id1);
        if (pt_cor == -1)
        {
            ERRORLOG("border point");
            break;
        }
        pt = get_triangle_point(tri_id1, pt_cor);
        if ((pt->x == sx) && (pt->y == sy))
            break;
        tri = get_triangle(tri_id1);
        tri_id3 = tri->tags[cor_id1];
        cor_id3 = link_find(tri_id3,tri_id1);
        pt = get_triangle_point(tri_id3, cor_id3);
        tmpX = pt->x;
        tmpY = pt->y;
        if (LbCompareMultiplications(tmpY-ey, sx-ex, tmpX-ex, sy-ey) == 0)
        {
          make_edge(ex, ey, tmpX, tmpY);
          make_edge(sx, sy, tmpX, tmpY);
          break;
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
            ERRORLOG("Infinite loop detected");
            break;
        }
    }

}

TbBool border_clip_horizontal(const unsigned char *imap, long start_x, long end_x, long start_y, long end_y)
{
    unsigned char map_center,map_up;
    const unsigned char *mapp_center,*mapp_up;
    TbBool r;
    long i;
    r = true;
    NAVIDBG(19,"Starting from (%ld,%ld) to (%ld,%ld)",start_x, start_y, end_x, end_y);
    //TODO PATHFINDING triangulate_area sub-function
    _DK_border_clip_horizontal(imap, start_x, end_x, start_y, end_y); return true;

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
    make_edge(start_x, start_y, end_x, start_y);
    //TODO PATHFINDING on a failure, we could release all allocated points...
    return r;
}

TbBool border_clip_vertical(const unsigned char *imap, long start_x, long end_x, long start_y, long end_y)
{
    unsigned char map_center,map_left;
    const unsigned char *mapp_center,*mapp_left;
    TbBool r;
    long i;
    r = true;
    NAVIDBG(19,"Starting");
    //TODO PATHFINDING triangulate_area sub-function
    _DK_border_clip_vertical(imap, start_x, end_x, start_y, end_y); return true;
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
    make_edge(start_x, start_y, start_x, end_y);
    //TODO PATHFINDING on a failure, we could release all allocated points...
    return r;
}

TbBool point_redundant(long tri_idx, long cor_idx)
{
    long tri_first,cor_first;
    long tri_secnd,cor_secnd;
    tri_first = tri_idx;
    cor_first = cor_idx;
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
    }
    return false;
}

long edge_find(long stlstart_x, long stlstart_y, long stlend_x, long stlend_y, long *edge_tri, long *edge_cor)
{
    //Note: uses LbCompareMultiplications()
    struct Triangle *tri;
    struct Point *pt;
    long dst_tri_idx,dst_cor_idx;
    long tri_idx,cor_idx,tri_id2;
    long delta_x,delta_y;
    long len_x,len_y;
    long i;
    NAVIDBG(19,"Starting");
    //return _DK_edge_find(stlstart_x, stlstart_y, stlend_x, stlend_y, edge_tri, edge_cor);
    if (!point_find(stlstart_x, stlstart_y, &dst_tri_idx, &dst_cor_idx))
    {
        return 0;
    }
    tri_idx = dst_tri_idx;
    cor_idx = dst_cor_idx;
    len_y = stlend_y - stlstart_y;
    len_x = stlend_x - stlstart_x;
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
        tri_idx = tri_id2;
        cor_idx = MOD3[i+1];
    }
    while (tri_idx != dst_tri_idx);
    return 0;
}

TbBool edge_lock_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name)
{
    long tri_n,tri_k;
    long x,y;
    struct Point *pt;
    long k;
    //_DK_edge_lock(ptend_x, ptend_y, ptstart_x, ptstart_y); return true;
    x = ptstart_x;
    y = ptstart_y;
    while ((x != ptend_x) || (y != ptend_y))
    {
      if (!edge_find(x, y, ptend_x, ptend_y, &tri_n, &tri_k))
      {
        ERRORMSG("%s: edge not found",func_name);
        return false;
      }
      Triangles[tri_n].field_D |= 1 << (tri_k+3);
      k = MOD3[tri_k+1];
      pt = get_triangle_point(tri_n,k);
      x = pt->x;
      y = pt->y;
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

void border_internal_points_delete(long start_x, long start_y, long end_x, long end_y)
{
    long edge_tri,edge_cor;
    long ntri,ncor;
    unsigned long k;
    struct Point *pt;
    long i,n;
    NAVIDBG(19,"Starting");
    //TODO PATHFINDING triangulate_area sub-function
    _DK_border_internal_points_delete(start_x, start_y, end_x, end_y); return;

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

void tri_set_rectangle(long a1, long a2, long a3, long a4, unsigned char a5)
{
    _DK_tri_set_rectangle(a1, a2, a3, a4, a5);
}

long fringe_get_rectangle(long *a1, long *a2, long *a3, long *a4, unsigned char *a5)
{
    return _DK_fringe_get_rectangle(a1, a2, a3, a4, a5);
}

TbBool edge_unlock_record_and_regions_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name)
{
    struct Triangle *tri;
    struct Point *pt;
    long pt_x,pt_y;
    long tri_id,cor_id;
    long nerr;
    pt_x = ptstart_x;
    pt_y = ptstart_y;
    nerr = 0;
    while (pt_x != ptend_x || pt_y != ptend_y)
    {
      if (!edge_find(pt_x, pt_y, ptend_x, ptend_y, &tri_id, &cor_id))
      {
          ERRORMSG("%s: edge not found at (%ld,%ld)",func_name,pt_x,pt_y);
          return false;
      }
      if (edge_point_add(pt_x, pt_y) < 0)
      {
          nerr++;
      }
      region_unlock(tri_id);
      tri = get_triangle(tri_id);
      tri->field_D &= ~(1 << (cor_id + 3));
      pt = get_triangle_point(tri_id,MOD3[cor_id+1]);
      pt_x = pt->x;
      pt_y = pt->y;
    }
    if (nerr != 0)
    {
        ERRORMSG("%s: overflow for %ld edge points",func_name,nerr);
        //ERRORLOG("edge_setlock_record:overflow");
        return true; //TODO PATHFINDING make sure we should return true
    }
    return true;
}

void border_unlock(long a1, long a2, long a3, long a4)
{
    struct EdgePoint *ept;
    long ept_id, tri_idx, cor_idx;
    long nerr;
    //TODO PATHFINDING triangulate_area sub-function
    _DK_border_unlock(a1, a2, a3, a4); return;
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
    long tri_idx,brd_idx;
    long i,k;
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
        return 0x01;
    }
    if (doortng->byte_18 == 0)
    {
        return 0x01;
    }
    if (is_hero_thing(doortng))
        owner = 5;
    else
    if (doortng->owner == game.neutral_player_num)
        owner = 6;
    else
        owner = doortng->owner;
    if (owner > 6)
    {
        ERRORLOG("Doors at (%d,%d) have outranged player %ld",(int)stl_x,(int)stl_y,owner);
        return 0x01;
    }
    return (0x20 * (owner+1)) | 0x01;
}

long get_navigation_colour_for_cube(long stl_x, long stl_y)
{
    long tcube;
    long i;
    i = get_column_height_at(stl_x, stl_y);
    if (i > 15)
      i = 15;
    tcube = get_top_cube_at(stl_x, stl_y);
    if (cube_is_lava(tcube) || cube_is_sacrificial(tcube))
      i |= 0x10;
    return i;
}

long get_navigation_colour(long stl_x, long stl_y)
{
    struct Map *mapblk;
    //return _DK_get_navigation_colour(stl_x, stl_y);
    mapblk = get_map_block_at(stl_x, stl_y);
    if ((mapblk->flags & MapFlg_IsDoor) != 0)
    {
        return get_navigation_colour_for_door(stl_x, stl_y);
    }
    if ((mapblk->flags & MapFlg_IsTall) != 0)
    {
        return 0x0F;
    }
    return get_navigation_colour_for_cube(stl_x, stl_y);
}

long uniform_area_colour(const unsigned char *imap, long start_x, long start_y, long end_x, long end_y)
{
    long uniform;
    long x,y;
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
    long border_a,border_b;
    long tri_a,tri_b;
    long i,n;
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
          if (n < 0)
              break;
          tri_b = MOD3[n+1];
          tri_a = i;
      }
    }
    while ((tri_a != border_a) || (tri_b != border_b));
    NAVIDBG(19,"Finished");
}

TbBool triangulate_area(unsigned char *imap, long start_x, long start_y, long end_x, long end_y)
{
    TbBool one_tile,not_whole_map;
    long colour;
    unsigned char ccolour;
    long rect_sx,rect_sy,rect_ex,rect_ey;
    TbBool r;
    long i;
    r = true;
    LastTriangulatedMap = imap;
    NAVIDBG(9,"F=%ld Area %03ld,%03ld %03ld,%03ld T=%04ld",game.play_gameturn,start_x,start_y,end_x,end_y,count_Triangles);
    //TODO PATHFINDING rewritten code has been disabled because it has errors (2/2)
    _DK_triangulate_area(imap, start_x, start_y, end_x, end_y); return true;
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
        if ( !one_tile )
            border_internal_points_delete(start_x, start_y, end_x, end_y);
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
        fringe_x2 = start_y;
        fringe_y2 = end_x;
        fringe_y1 = end_y;
        for (i = start_x; i < end_x; i++)
        {
            fringe_y[i-1] = start_y;
        }
        while ( fringe_get_rectangle(&rect_sx, &rect_sy, &rect_ex, &rect_ey, &ccolour) )
        {
            if ((ccolour) || (not_whole_map))
            {
                tri_set_rectangle(rect_sx, rect_sy, rect_ex, rect_ey, ccolour);
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
    return r;
}
/******************************************************************************/
