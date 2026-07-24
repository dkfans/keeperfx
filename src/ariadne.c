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
#include "pre_inc.h"
#include "ariadne.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "ariadne_navitree.h"
#include "ariadne_regions.h"
#include "ariadne_tringls.h"
#include "ariadne_points.h"
#include "ariadne_findcache.h"
#include "ariadne_naviheap.h"
#include "thing_stats.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "gui_topmsg.h"
#include "map_columns.h"
#include "map_utils.h"
#include "game_legacy.h"
#include "post_inc.h"

#define EDGEFIT_LEN           64
#define EDGEOR_COUNT           4

typedef int32_t (*NavRules)(NavColour, NavColour);

struct QuadrantOffset {
    int32_t x;
    int32_t y;
};

struct Gate {
  int32_t start_coordinate_x;
  int32_t start_coordinate_y;
  int32_t end_coordinate_x;
  int32_t end_coordinate_y;
  int32_t intersection_coordinate_x;
  int32_t intersection_coordinate_y;
  int32_t pathfinding_direction;
};

struct Pathway {
  int32_t start_coordinate_x;
  int32_t start_coordinate_y;
  int32_t finish_coordinate_x;
  int32_t finish_coordinate_y;
  struct Gate points[256];
  int32_t points_num;
};

struct WayPoints {
  int32_t edge1_start_index;
  int32_t edge2_start_index;
  int32_t edge1_current_index;
  int32_t edge2_current_index;
  int32_t waypoint_index_array[ARID_PATH_WAYPOINTS_COUNT];
};

struct FOV {
    struct PathWayPoint tipA;
    struct PathWayPoint tipB;
    struct PathWayPoint tipC;
};

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

static uint32_t edgelen_initialised;
static uint32_t RadiusEdgeFit[EDGEOR_COUNT][EDGEFIT_LEN];
static NavRules nav_rulesA2B;
static struct WayPoints wayPoints;
static uint32_t *EdgeFit;
static struct Pathway ap_GPathway;
static int32_t tree_routelen;
static int32_t tree_route[TREE_ROUTE_LEN];
static int32_t tree_routecost;
static int32_t tree_triA;
static int32_t tree_triB;
static int32_t tree_altA;
static int32_t tree_altB;
static int32_t tree_Ax8;
static int32_t tree_Ay8;
static int32_t tree_Bx8;
static int32_t tree_By8;
static int32_t route_fwd[ROUTE_LENGTH];
static int32_t route_bak[ROUTE_LENGTH];

NavColour *LastTriangulatedMap;
int32_t ix_Border;
int32_t Border[BORDER_LENGTH];

TbBool nav_map_initialised = 0;

/******************************************************************************/
static unsigned char const actual_sizexy_to_nav_block_sizexy_table[] = {
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

static const uint32_t actual_sizexy_to_nav_sizexy_table[] = {
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
    {{ANGLE_NORTH, 1}, {ANGLE_SOUTH, 2}},
    {{ANGLE_NORTH, 2}, {ANGLE_SOUTH, 1}},
};

const struct HugStart blocked_y_hug_start[][2] = {
    {{ANGLE_WEST, 2}, {ANGLE_EAST, 1}},
    {{ANGLE_WEST, 1}, {ANGLE_EAST, 2}},
};

const struct HugStart blocked_xy_hug_start[][2][2] = {
   {{{ANGLE_WEST, 2}, {ANGLE_NORTH, 1}},
    {{ANGLE_EAST, 1}, {ANGLE_NORTH, 2}}},
   {{{ANGLE_WEST, 1}, {ANGLE_SOUTH, 2}},
    {{ANGLE_EAST, 2}, {ANGLE_SOUTH, 1}}},
};

static struct Path fwd_path;
static struct Path bak_path;
static struct Path best_path;
/******************************************************************************/


int32_t thing_nav_block_sizexy(const struct Thing *thing)
{
    int32_t i;
    i = thing->clipbox_size_xy;
    if (i >= (int32_t)(sizeof(actual_sizexy_to_nav_block_sizexy_table)/sizeof(actual_sizexy_to_nav_block_sizexy_table[0])))
        i = (int32_t)(sizeof(actual_sizexy_to_nav_block_sizexy_table)/sizeof(actual_sizexy_to_nav_block_sizexy_table[0]))-1;
    if (i < 0)
        i = 0;
    return actual_sizexy_to_nav_block_sizexy_table[i];
}

int32_t thing_nav_sizexy(const struct Thing *thing)
{
    int32_t i;
    i = thing->clipbox_size_xy;
    if (i >= (int32_t)(sizeof(actual_sizexy_to_nav_sizexy_table)/sizeof(actual_sizexy_to_nav_sizexy_table[0])))
        i = (int32_t)(sizeof(actual_sizexy_to_nav_sizexy_table)/sizeof(actual_sizexy_to_nav_sizexy_table[0]))-1;
    if (i < 0)
        i = 0;
    return actual_sizexy_to_nav_sizexy_table[i];
}


/******************************************************************************/
static int32_t route_to_path(int32_t ptfind_x, int32_t ptfind_y, int32_t ptstart_x, int32_t ptstart_y, const int32_t *route, int32_t wp_lim, struct Path *path, int32_t *total_len);
static void path_out_a_bit(struct Path *path, const int32_t *route);
static void gate_navigator_init8(struct Pathway *pway, int32_t trAx, int32_t trAy, int32_t trBx, int32_t trBy, int32_t wp_lim, unsigned char unusedparam);
static void route_through_gates(const struct Pathway *pway, struct Path *path, int32_t subroute);
static int32_t ariadne_push_position_against_wall(struct Thing *thing, const struct Coord3d *pos1, struct Coord3d *pos_out);
static TbBool ariadne_check_forward_for_wallhug_gap(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos, int32_t hug_angle);
static int32_t ariadne_get_blocked_flags(struct Thing *thing, const struct Coord3d *pos);
static int32_t triangle_findSE8(int32_t ptfind_x, int32_t ptfind_y);
static int32_t ma_triangle_route(int32_t ptfind_x, int32_t ptfind_y, int32_t *ptstart_x);
static void edgelen_init(void);
/******************************************************************************/


static uint32_t fits_thro(int32_t tri_idx, int32_t ormask_idx)
{
    static uint32_t const edgelen_ORmask[] = {60, 51, 15, 0};
    uint32_t eidx;
    uint32_t emask;

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


static PlayerBitFlags get_navtree_owner_flags(NavColour treeI)
{
    return treeI >> NAVMAP_OWNERSELECT_BIT;
}

static int32_t navigation_rule_normal(NavColour treeA, NavColour treeB)
{
    if ((treeB & NAVMAP_FLOORHEIGHT_MASK) - (treeA & NAVMAP_FLOORHEIGHT_MASK) > 1)
      return NavigationRule_Blocked;
    if ((treeB & (NAVMAP_OWNERSELECT_MASK | NAVMAP_UNSAFE_SURFACE)) == 0)
      return NavigationRule_Normal;
    if (owner_player_navigating != -1)
    {
        if (get_navtree_owner_flags(treeB) & (1 << owner_player_navigating))
          return NavigationRule_Blocked;
    }
    if ((treeB & NAVMAP_UNSAFE_SURFACE) == 0)
        return NavigationRule_Normal;
    if ((treeA & NAVMAP_UNSAFE_SURFACE) != 0)
        return NavigationRule_Normal;
    return nav_thing_can_travel_over_lava;
}

void set_nav_rule_default(void)
{
    nav_rulesA2B = navigation_rule_normal;
}


static void edge_points8(int32_t ntri_src, int32_t ntri_dst, int32_t *tipA_x, int32_t *tipA_y, int32_t *tipB_x, int32_t *tipB_y)
{
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

static int32_t fov_region(int32_t point_x, int32_t point_y, const struct FOV *fov)
{
    int32_t diff_ax;
    int32_t diff_ay;
    diff_ax = point_x - fov->tipA.x;
    diff_ay = point_y - fov->tipA.y;
    int32_t diff_bx;
    int32_t diff_by;
    diff_bx = fov->tipB.x - fov->tipA.x;
    diff_by = fov->tipB.y - fov->tipA.y;
    if (LbCompareMultiplications(diff_ay, diff_bx, diff_ax, diff_by) < 0) {
        return FieldOfViewRegion_OutsideLeft;
    }
    int32_t diff_cx;
    int32_t diff_cy;
    diff_cx = fov->tipC.x - fov->tipA.x;
    diff_cy = fov->tipC.y - fov->tipA.y;
    if (LbCompareMultiplications(diff_ay, diff_cx, diff_ax, diff_cy) > 0) {
        return FieldOfViewRegion_OutsideRight;
    }
    return FieldOfViewRegion_WithinBounds;
}

static int32_t route_to_path(int32_t ptfind_x, int32_t ptfind_y, int32_t ptstart_x, int32_t ptstart_y, const int32_t *route, int32_t wp_lim, struct Path *path, int32_t *total_len)
{
    NAVIDBG(19,"Starting");

    struct FOV fov_AC;
    int32_t edge1_x;
    int32_t edge1_y;
    int32_t edge2_x;
    int32_t edge2_y;
    char edge1_region;
    char edge2_region;
    int32_t waypoint_edge1_index;
    int32_t waypoint_edge2_index;
    int32_t wpi;
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
      wayPoints.waypoint_index_array[0] = 0;
      path->waypoints_num = 1;
      return 1;
    }
    waypoint_edge2_index = 0;
    waypoint_edge1_index = 0;
    wpi = 0;
    edge_points8(route[wpi+0], route[wpi+1], &fov_AC.tipB.x, &fov_AC.tipB.y, &fov_AC.tipC.x, &fov_AC.tipC.y);
    wayPoints.edge1_current_index = wpi;
    wayPoints.edge2_current_index = wpi;
    wpi++;
    while ( 1 )
    {
      if (wpi < wp_lim)
      {
          edge_points8(route[wpi+0], route[wpi+1], &edge1_x, &edge1_y, &edge2_x, &edge2_y);
          wayPoints.edge1_start_index = wpi;
          wayPoints.edge2_start_index = wpi;
          edge1_region = fov_region(edge1_x, edge1_y, &fov_AC);
          edge2_region = fov_region(edge2_x, edge2_y, &fov_AC);
      } else
      {
          edge2_region = fov_region(ptstart_x, ptstart_y, &fov_AC);
          edge1_region = edge2_region;
          if (edge2_region == FieldOfViewRegion_WithinBounds)
            break;
      }
      if (edge1_region == FieldOfViewRegion_WithinBounds)
      {
          fov_AC.tipB.x = edge1_x;
          fov_AC.tipB.y = edge1_y;
          wayPoints.edge1_current_index = wayPoints.edge1_start_index;
          waypoint_edge1_index = wpi;
      }
      if (edge2_region == FieldOfViewRegion_WithinBounds)
      {
          fov_AC.tipC.x = edge2_x;
          fov_AC.tipC.y = edge2_y;
          wayPoints.edge2_current_index = wayPoints.edge2_start_index;
          waypoint_edge2_index = wpi;
      }
      if (edge2_region == FieldOfViewRegion_OutsideLeft)
      {
        *total_len += LbSqrL((fov_AC.tipB.x - fov_AC.tipA.x) * (fov_AC.tipB.x - fov_AC.tipA.x)
            + (fov_AC.tipB.y - fov_AC.tipA.y) * (fov_AC.tipB.y - fov_AC.tipA.y));
        fov_AC.tipA.x = fov_AC.tipB.x;
        path->waypoints[wp_num].x = fov_AC.tipB.x;
        path->waypoints[wp_num].y = fov_AC.tipB.y;
        wayPoints.waypoint_index_array[wp_num] = wayPoints.edge1_current_index;
        wp_num++;
        wpi = waypoint_edge1_index;
        fov_AC.tipA.y = fov_AC.tipB.y;
        edge_points8(route[wpi+0], route[wpi+1], &fov_AC.tipB.x, &fov_AC.tipB.y, &fov_AC.tipC.x, &fov_AC.tipC.y);
        wayPoints.edge1_current_index = wpi;
        wayPoints.edge2_current_index = wpi;
        if (wp_num >= ARID_PATH_WAYPOINTS_COUNT) {
            ERRORLOG("Exceeded max path length (i:%d,L:%d) (%d,%d)->(%d,%d)",
            wpi, wp_lim, ptfind_x, ptfind_y, ptstart_x, ptstart_y);
            break;
        }
      } else
      if (edge1_region == FieldOfViewRegion_OutsideRight)
      {
        *total_len += LbSqrL((fov_AC.tipC.x - fov_AC.tipA.x) * (fov_AC.tipC.x - fov_AC.tipA.x)
            + (fov_AC.tipC.y - fov_AC.tipA.y) * (fov_AC.tipC.y - fov_AC.tipA.y));
        fov_AC.tipA.x = fov_AC.tipC.x;
        path->waypoints[wp_num].x = fov_AC.tipC.x;
        path->waypoints[wp_num].y = fov_AC.tipC.y;
        wayPoints.waypoint_index_array[wp_num] = wayPoints.edge2_current_index;
        wp_num++;
        wpi = waypoint_edge2_index;
        fov_AC.tipA.y = fov_AC.tipC.y;
        edge_points8(route[wpi+0], route[wpi+1], &fov_AC.tipB.x, &fov_AC.tipB.y, &fov_AC.tipC.x, &fov_AC.tipC.y);
        wayPoints.edge1_current_index = wpi;
        wayPoints.edge2_current_index = wpi;
        if (wp_num >= ARID_PATH_WAYPOINTS_COUNT) {
            ERRORLOG("Exceeded max path length (i:%d,R:%d) (%d,%d)->(%d,%d)",
            wpi, wp_lim, ptfind_x, ptfind_y, ptstart_x, ptstart_y);
            break;
        }
      }
      wpi++;
    }
    if (wp_num >= ARID_PATH_WAYPOINTS_COUNT) {
        ERRORLOG("Exceeded max path length - gate_route_to_coords");
        wp_num = ARID_PATH_WAYPOINTS_COUNT - 1;
    }
    *total_len += LbSqrL((ptstart_x - fov_AC.tipA.x) * (ptstart_x - fov_AC.tipA.x)
        + (ptstart_y - fov_AC.tipA.y) * (ptstart_y - fov_AC.tipA.y));
    path->waypoints[wp_num].x = ptstart_x;
    path->waypoints[wp_num].y = ptstart_y;
    wayPoints.waypoint_index_array[wp_num] = wp_lim;
    wp_num++;
    path->waypoints_num = wp_num;
    return wp_num;
}

static void waypoint_normal(int32_t tri1_id, int32_t cor1_id, int32_t *norm_x, int32_t *norm_y)
{
    int tri2_id;
    int tri3_id;
    int cor2_id;
    int cor3_id;
    tri3_id = tri1_id;
    cor3_id = MOD3[cor1_id+2];
    tri2_id = Triangles[tri1_id].tags[cor1_id];
    cor2_id = link_find(tri2_id, tri1_id);
    cor2_id = MOD3[cor2_id+1];
    uint32_t k;
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
        int triangle_point1_index;
        int triangle_point2_index;
        triangle_point1_index = Triangles[tri2_id].points[MOD3[cor2_id+1]];
        triangle_point2_index = Triangles[tri3_id].points[cor3_id];
        diff_y = ari_Points[triangle_point1_index].y - ari_Points[triangle_point2_index].y;
        diff_x = ari_Points[triangle_point2_index].x - ari_Points[triangle_point1_index].x;
    } else
    {
        diff_y = 0;
        diff_x = 0;
    }
    int32_t nx;
    int32_t ny;
    nx = -1;
    if (diff_y >= 0)
        nx = diff_y != 0;
    ny = -1;
    if (diff_x >= 0)
        ny = diff_x != 0;
    *norm_x = nx;
    *norm_y = ny;
}

static void path_out_a_bit(struct Path *path, const int32_t *route)
{
    struct PathWayPoint *ppoint;
    int32_t *wpoint;
    int32_t tip_x;
    int32_t tip_y;
    int32_t norm_x;
    int32_t norm_y;
    int32_t prev_pt;
    int32_t curr_pt;
    int32_t link_fwd;
    int32_t link_bak;
    int32_t i;
    wpoint = &wayPoints.waypoint_index_array[0];
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

static void cull_gate_to_point(struct Gate *gt, int32_t distance_threshold)
{
    int diff_a;
    int diff_b;
    diff_a = abs(gt->start_coordinate_x - gt->end_coordinate_x);
    diff_b = abs(gt->start_coordinate_y - gt->end_coordinate_y);
    if (diff_a <= diff_b)
    {
      if (diff_b + (diff_a >> 1) < distance_threshold)
          return;
    } else
    {
      if (diff_a + (diff_b >> 1) < distance_threshold)
          return;
    }
    if (gt->pathfinding_direction == PathDir_EndToStart)
    {
        diff_a = (gt->start_coordinate_x - gt->end_coordinate_x) << 6;
        diff_b = (gt->start_coordinate_y - gt->end_coordinate_y) << 6;
    } else
    {
        diff_a = (gt->end_coordinate_x - gt->start_coordinate_x) << 6;
        diff_b = (gt->end_coordinate_y - gt->start_coordinate_y) << 6;
    }
    int32_t div_a;
    int32_t div_b;
    int32_t cmul;
    int32_t val_x;
    int32_t val_y;
    div_b = LbSqrL(((uint64_t)(diff_a * diff_a) >> 14) + ((uint64_t)(diff_b * diff_b) >> 14)) << 13;
    if (div_b < 1)
        div_b = 1;
    div_a = div_b;
    if (diff_a < 0)
    {
        diff_a = -diff_a;
        div_a = -div_b;
    }
    cmul = (distance_threshold >> 8) - 1;
    val_x = cmul * ((uint64_t)diff_a << 14) / div_a;
    if (diff_b < 0)
    {
        diff_b = -diff_b;
        div_b = -div_b;
    }
    val_y = cmul * ((uint64_t)diff_b << 14) / div_b;
    if (gt->pathfinding_direction == PathDir_EndToStart)
    {
        gt->start_coordinate_x = val_x + gt->end_coordinate_x;
        gt->start_coordinate_y = val_y + gt->end_coordinate_y;
    } else
    {
        gt->end_coordinate_x = val_x + gt->start_coordinate_x;
        gt->end_coordinate_y = val_y + gt->start_coordinate_y;
    }
}

static TbBool calc_intersection(struct Gate *gt, int32_t line_start_x, int32_t line_start_y, int32_t line_end_x, int32_t line_end_y)
{
    int gate_start_x_delta;
    int line_y_delta;
    int gate_start_y_delta;
    int line_x_delta;
    int gate_x_span;
    int gate_y_span;
    gate_start_x_delta = (gt->start_coordinate_x - line_start_x) << 6;
    line_y_delta = (line_start_y - line_end_y) << 6;
    gate_start_y_delta = (gt->start_coordinate_y - line_start_y) << 6;
    line_x_delta = (line_start_x - line_end_x) << 6;
    gate_x_span = (gt->end_coordinate_x - gt->start_coordinate_x) << 6;
    gate_y_span = (gt->end_coordinate_y - gt->start_coordinate_y) << 6;
    int intersection_numerator;
    int intersection_denominator;
    int intersection_factor;
    intersection_numerator = ((uint64_t)(gate_start_x_delta * line_y_delta) >> 14)
       - ((uint64_t)(gate_start_y_delta * line_x_delta) >> 14);
    intersection_denominator = ((uint64_t)(line_x_delta * gate_y_span) >> 14)
       - ((uint64_t)(line_y_delta * gate_x_span) >> 14);
    if (intersection_denominator == 0)
        return false;
    if (intersection_numerator < 0) {
      intersection_numerator = -intersection_numerator;
      intersection_denominator = -intersection_denominator;
    }
    intersection_factor = ((uint64_t)intersection_numerator << 14) / intersection_denominator;
    if ((intersection_factor < -16384) || (intersection_factor > 16384))
        return false;
    gt->intersection_coordinate_x = gt->start_coordinate_x + (((uint64_t)(gate_x_span * intersection_factor) >> 14) >> 6);
    gt->intersection_coordinate_y = gt->start_coordinate_y + (((uint64_t)(gate_y_span * intersection_factor) >> 14) >> 6);

    int vmin;
    vmin = gt->end_coordinate_x;
    if (vmin >= gt->start_coordinate_x)
      vmin = gt->start_coordinate_x;
    if (vmin > gt->intersection_coordinate_x)
        return false;

    vmin = gt->end_coordinate_x;
    if (vmin <= gt->start_coordinate_x)
      vmin = gt->start_coordinate_x;
    if (vmin < gt->intersection_coordinate_x)
        return false;

    vmin = gt->start_coordinate_y;
    if (vmin >= gt->end_coordinate_y)
      vmin = gt->end_coordinate_y;
    if (vmin > gt->intersection_coordinate_y)
        return false;

    vmin = gt->start_coordinate_y;
    if (vmin <= gt->end_coordinate_y)
      vmin = gt->end_coordinate_y;
    if (vmin < gt->intersection_coordinate_y)
        return false;

    return true;
}

static void cull_gate_to_best_point(struct Gate *gt, int32_t distance_threshold)
{
    int start_to_intersection_distance;
    int end_to_intersection_distance;
    {
        int diff_x;
        int diff_y;
        diff_x = abs(gt->start_coordinate_x - gt->intersection_coordinate_x);
        diff_y = abs(gt->start_coordinate_y - gt->intersection_coordinate_y);
        if (diff_x <= diff_y)
            start_to_intersection_distance = (diff_x >> 1) + diff_y;
        else
            start_to_intersection_distance = (diff_y >> 1) + diff_x;
        diff_x = abs(gt->end_coordinate_x - gt->intersection_coordinate_x);
        diff_y = abs(gt->end_coordinate_y - gt->intersection_coordinate_y);
        if (diff_x <= diff_y)
            end_to_intersection_distance = diff_y + (diff_x >> 1);
        else
            end_to_intersection_distance = (diff_y >> 1) + diff_x;
    }
    if ((start_to_intersection_distance >= (distance_threshold >> 1)) || (end_to_intersection_distance >= (distance_threshold >> 1)))
    {
        int diff_lim;
        diff_lim = (distance_threshold + 2) >> 1;
        if (start_to_intersection_distance < diff_lim)
        {
            gt->pathfinding_direction = PathDir_StartToEnd;
            cull_gate_to_point(gt, distance_threshold);
        } else
        if (end_to_intersection_distance < diff_lim)
        {
            gt->pathfinding_direction = PathDir_EndToStart;
            cull_gate_to_point(gt, distance_threshold);
        } else
        {
            int rel_A;
            int rel_B;
            {
                int diff_B;
                int diff_A;
                diff_A = (gt->end_coordinate_x - gt->start_coordinate_x) << 6;
                diff_B = (gt->end_coordinate_y - gt->start_coordinate_y) << 6;
                int dlen_A;
                int dlen_B;
                dlen_A = LbSqrL(((uint64_t)(diff_A * diff_A) >> 14) + ((uint64_t)(diff_B * diff_B) >> 14)) << 7;
                dlen_B = dlen_A;
                if (dlen_A)
                {
                    if (diff_A < 0)
                    {
                        diff_A = -diff_A;
                        dlen_A = -dlen_A;
                    }
                    rel_A = (((uint64_t)diff_A << 14) / dlen_A) >> 6;
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
                    rel_B = (((uint64_t)diff_B << 14) / dlen_B) >> 6;
                } else
                {
                  rel_B = 0;
                }
            }

            int32_t delta_A;
            int32_t delta_B;
            delta_A = (distance_threshold >> 9) * rel_A;
            delta_B = (distance_threshold >> 9) * rel_B;
            int cmin_y;
            int cmin_x;
            int cmax_x;
            int cmax_y;
            cmin_x = gt->intersection_coordinate_x - delta_A;
            cmin_y = gt->intersection_coordinate_y - delta_B;
            cmax_y = gt->intersection_coordinate_y + delta_B;
            cmax_x = gt->intersection_coordinate_x + delta_A;

            int clamp_min_x;
            int clamp_max_x;
            int clamp_min_y;
            int clamp_max_y;

            clamp_min_x = gt->end_coordinate_x;
            if (clamp_min_x >= gt->start_coordinate_x)
              clamp_min_x = gt->start_coordinate_x;
            if (clamp_min_x <= cmin_x)
            {
                clamp_max_x = gt->end_coordinate_x;
                if (clamp_max_x <= gt->start_coordinate_x)
                    clamp_max_x = gt->start_coordinate_x;
                if (clamp_max_x >= cmin_x)
                {
                  clamp_max_y = gt->start_coordinate_y;
                  clamp_min_y = clamp_max_y;
                  if (clamp_min_y >= gt->end_coordinate_y)
                      clamp_min_y = gt->end_coordinate_y;
                  if (clamp_min_y <= cmin_y)
                  {
                    if (clamp_max_y <= gt->end_coordinate_y)
                        clamp_max_y = gt->end_coordinate_y;
                    if (clamp_max_y >= cmin_y)
                    {
                        gt->start_coordinate_x = cmin_x;
                        gt->start_coordinate_y = cmin_y;
                    }
                  }
                }
            }

            clamp_min_x = gt->end_coordinate_x;
            if (clamp_min_x >= gt->start_coordinate_x)
                clamp_min_x = gt->start_coordinate_x;
            if (clamp_min_x <= cmax_x)
            {
                clamp_max_x = gt->end_coordinate_x;
                if (clamp_max_x <= gt->start_coordinate_x)
                    clamp_max_x = gt->start_coordinate_x;
                if (clamp_max_x >= cmax_x)
                {
                  clamp_max_y = gt->start_coordinate_y;
                  clamp_min_y = gt->start_coordinate_y;
                  if (clamp_max_y >= gt->end_coordinate_y)
                      clamp_min_y = gt->end_coordinate_y;
                  if (clamp_min_y <= cmax_y)
                  {
                    if (clamp_max_y <= gt->end_coordinate_y)
                      clamp_max_y = gt->end_coordinate_y;
                    if (clamp_max_y >= cmax_y)
                    {
                        gt->end_coordinate_x = cmax_x;
                        gt->end_coordinate_y = cmax_y;
                    }
                  }
                }
            }
        }
    }
}

static int32_t gate_route_to_coords(int32_t trAx, int32_t trAy, int32_t trBx, int32_t trBy, int32_t *route_array, int32_t route_length, struct Pathway *pway, int32_t distance_threshold)
{
    int32_t total_len;
    best_path.waypoints_num = route_to_path(trAx, trAy, trBx, trBy, route_array, route_length, &best_path, &total_len);
    pway->start_coordinate_x = trAx;
    pway->start_coordinate_y = trAy;
    pway->finish_coordinate_x = trBx;
    pway->finish_coordinate_y = trBy;
    if (route_length < 1)
    {
        pway->points[0].end_coordinate_y = trBy;
        pway->points[0].end_coordinate_x = trBx;
        pway->points[0].start_coordinate_x = trBx;
        pway->points[0].start_coordinate_y = trBy;
        pway->points[0].pathfinding_direction = PathDir_StartToEnd;
        pway->points_num = 1;
        return 1;
    }
    struct FOV fov1;
    struct FOV fov2;
    fov1.tipA.x = trAx;
    fov1.tipA.y = trAy;
    edge_points8(route_array[0], route_array[1], &fov1.tipB.x, &fov1.tipB.y, &fov1.tipC.x, &fov1.tipC.y);
    memcpy(&fov2, &fov1, sizeof(struct FOV));
    int pt_num;
    int wp_idx;
    wp_idx = 0;
    pt_num = 0;
    int wp_x;
    int wp_y;
    wp_x = pway->start_coordinate_x;
    wp_y = pway->start_coordinate_y;

    struct Gate *gt;
    gt = pway->points;
    int wpi;
    for (wpi=1; wpi <= route_length; wpi++)
    {
        int32_t edge_x1;
        int32_t edge_y1;
        int32_t edge_x2;
        int32_t edge_y2;
        if (wpi < route_length)
        {
            edge_points8(route_array[wpi+0], route_array[wpi+1], &edge_x1, &edge_y1, &edge_x2, &edge_y2);
        } else
        {
            edge_x2 = trBx;
            edge_x1 = trBx;
            edge_y2 = trBy;
            edge_y1 = trBy;
        }
        char edge1_region;
        char edge2_region;
        char edge1_region_secondary_fov;
        char edge2_region_secondary_fov;
        edge1_region = fov_region(edge_x1, edge_y1, &fov1);
        edge2_region = fov_region(edge_x2, edge_y2, &fov1);
        edge1_region_secondary_fov = fov_region(edge_x1, edge_y1, &fov2);
        edge2_region_secondary_fov = fov_region(edge_x2, edge_y2, &fov2);

        if ( edge1_region || edge2_region || edge1_region_secondary_fov || edge2_region_secondary_fov )
        {
            if (pt_num == 256) {
                ERRORLOG("grtc:Exceeded max path length (i:%d,rl:%d)", wpi, route_length);
            }
            gt->start_coordinate_x = fov1.tipB.x;
            gt->start_coordinate_y = fov1.tipB.y;
            gt->end_coordinate_x = fov1.tipC.x;
            gt->end_coordinate_y = fov1.tipC.y;
            gt->pathfinding_direction = PathDir_Reverse;
            int dist_x;
            int dist_y;
            int bwp_x;
            int bwp_y;

            int dist_A;
            int dist_B;
            bwp_x = best_path.waypoints[wp_idx].x;
            dist_x = abs(gt->start_coordinate_x - bwp_x);
            bwp_y = best_path.waypoints[wp_idx].y;
            dist_y = abs(gt->start_coordinate_y - bwp_y);
            if (dist_x <= dist_y)
                dist_A = (dist_x >> 1) + dist_y;
            else
                dist_A = (dist_y >> 1) + dist_x;
            dist_B = dist_A;

            int dist_C;
            int dist_D;
            dist_x = abs(gt->end_coordinate_x - bwp_x);
            dist_y = abs(gt->end_coordinate_y - bwp_y);
            if ( dist_x <= dist_y )
                dist_x >>= 1;
            else
                dist_y >>= 1;

            dist_C = dist_x + dist_y;
            dist_D = dist_C;

            if (wp_idx < best_path.waypoints_num-1)
            {
              bwp_x = best_path.waypoints[wp_idx+1].x;
              dist_x = abs(gt->start_coordinate_x - bwp_x);
              bwp_y = best_path.waypoints[wp_idx+1].y;
              dist_y = abs(gt->start_coordinate_y - bwp_y);
              if (dist_x <= dist_y)
                  dist_B = (dist_x >> 1) + dist_y;
              else
                  dist_B = dist_x + (dist_y >> 1);
              dist_x = abs(gt->end_coordinate_x - bwp_x);
              dist_y = abs(gt->end_coordinate_y - bwp_y);
              if (dist_y >= dist_x)
                  dist_D = (dist_x >> 1) + dist_y;
              else
                  dist_D = dist_x + (dist_y >> 1);
            }
            int minimum_distance_first;
            int minimum_distance_second;
            minimum_distance_first = dist_C;
            if (minimum_distance_first >= dist_A)
              minimum_distance_first = dist_A;
            minimum_distance_second = dist_D;
            if (minimum_distance_second >= dist_B)
              minimum_distance_second = dist_B;
            if (minimum_distance_first >= minimum_distance_second)
            {
                gt->pathfinding_direction = (dist_D <= dist_B);
                if (wp_idx < best_path.waypoints_num-1)
                {
                    wp_x = best_path.waypoints[wp_idx].x;
                    wp_y = best_path.waypoints[wp_idx].y;
                    wp_idx++;
                }
            } else
            {
                gt->pathfinding_direction = (dist_C <= dist_A);
                minimum_distance_second = minimum_distance_first;
            }
            if (minimum_distance_second < 256)
            {
                cull_gate_to_point(gt, distance_threshold);
            } else
            {
                int fld18_mem;
                fld18_mem = gt->pathfinding_direction;
                gt->pathfinding_direction = PathDir_BestPoint;
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
                    gt->pathfinding_direction = fld18_mem;
                  }
              }
              if (gt->pathfinding_direction == PathDir_BestPoint)
              {
                  cull_gate_to_best_point(gt, distance_threshold);
                  gt->pathfinding_direction = fld18_mem;
              } else
              {
                  cull_gate_to_point(gt, distance_threshold);
                  gt->pathfinding_direction = fld18_mem;
              }
            }
            fov1.tipA.x = gt->start_coordinate_x;
            fov1.tipA.y = gt->start_coordinate_y;
            fov2.tipA.x = gt->end_coordinate_x;
            fov2.tipA.y = gt->end_coordinate_y;
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
        ERRORLOG("grtc:Exceeded max path length (i:%d,rl:%d)", wpi, route_length);
    }
    pt_num++;
    gt->end_coordinate_x = trBx;
    gt->start_coordinate_x = trBx;
    gt->end_coordinate_y = trBy;
    gt->start_coordinate_y = trBy;
    gt->pathfinding_direction = PathDir_StartToEnd;
    pway->points_num = pt_num;
    return pt_num;
}

static void gate_navigator_init8(struct Pathway *pway, int32_t trAx, int32_t trAy, int32_t trBx, int32_t trBy, int32_t wp_lim, unsigned char unusedparam)
{
    pway->start_coordinate_x = trAx;
    pway->start_coordinate_y = trAy;
    pway->finish_coordinate_x = trBx;
    pway->points_num = 0;
    pway->finish_coordinate_y = trBy;
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

static void route_through_gates(const struct Pathway *pway, struct Path *path, int32_t subroute)
{
    const struct Gate *ppoint;
    struct PathWayPoint *wpoint;
    int32_t i;
    if (subroute > 16383)
        subroute = 16383;
    if (subroute < 0)
        subroute = 0;
    path->start.x = pway->start_coordinate_x;
    path->start.y = pway->start_coordinate_y;
    path->finish.x = pway->finish_coordinate_x;
    path->finish.y = pway->finish_coordinate_y;
    path->waypoints_num = pway->points_num;
    ppoint = &pway->points[0];
    wpoint = &path->waypoints[0];
    for (i=0; i < pway->points_num-1; i++)
    {
        if (ppoint->pathfinding_direction)
        {
            wpoint->x = ppoint->end_coordinate_x - (subroute * (ppoint->end_coordinate_x - ppoint->start_coordinate_x) >> 14);
            wpoint->y = ppoint->end_coordinate_y - (subroute * (ppoint->end_coordinate_y - ppoint->start_coordinate_y) >> 14);
        } else
        {
            wpoint->x = ppoint->start_coordinate_x + (subroute * (ppoint->end_coordinate_x - ppoint->start_coordinate_x) >> 14);
            wpoint->y = ppoint->start_coordinate_y + (subroute * (ppoint->end_coordinate_y - ppoint->start_coordinate_y) >> 14);
        }
        wpoint++;
        ppoint++;
    }
    path->waypoints[i].x = pway->finish_coordinate_x;
    path->waypoints[i].y = pway->finish_coordinate_y;
}

static int32_t triangle_findSE8(int32_t ptfind_x, int32_t ptfind_y)
{
    int32_t ntri;
    int32_t ncor;
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

static uint32_t nav_same_component(int32_t ptAx, int32_t ptAy, int32_t ptBx, int32_t ptBy)
{
    NAVIDBG(19,"F=%u Connect %03d,%03d %03d,%03d", get_gameturn(), ptAx, ptAy, ptBx, ptBy);
    int32_t tri1_id;
    int32_t tri2_id;
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

static TbBool triangulation_border_tag(void)
{
    if (border_tags_to_current(Border, ix_Border) != ix_Border)
    {
        ERRORLOG("Some border Tags were outranged");
        return false;
    }
    return true;
}

static void creature_radius_set(int32_t radius)
{
    edgelen_init();
    if ((radius < CreatureRadius_Small) || (radius >= EDGEOR_COUNT)) {
        ERRORLOG("only radius 1..%d allowed, got %d",EDGEOR_COUNT,(int)radius);
        if (radius < CreatureRadius_Small) {
            radius = CreatureRadius_Small;
        } else {
            radius = EDGEOR_COUNT - 1;
        }
    }
    EdgeFit = RadiusEdgeFit[radius];
}

static void set_nearpoint(int32_t tri_id, int32_t cor_id, int32_t dstx, int32_t dsty, int32_t *px, int32_t *py)
{
    static struct QuadrantOffset qdrnt_offs[] = {
       {   0,   0},{ 128, 128},{-128, 128},{   0, 128},
       { 128,-128},{ 128,   0},{ 128, 128},{ 128, 128},
       {-128,-128},{ 128,-128},{-128,   0},{-128, 128},
       {   0,-128},{ 128,-128},{-128,-128},{   0,   0},
    };

    struct Point *pt1;
    pt1 = get_triangle_point(tri_id,cor_id);
    unsigned int tngflags;
    tngflags = 0;
    if ((LastTriangulatedMap[256 * (pt1->y-1) + (pt1->x-1)] & TriangleFlag_All) == TriangleFlag_All)
      tngflags = TriangleFlag_TopLeft;
    if ((LastTriangulatedMap[256 * (pt1->y-1) + (pt1->x)]   & TriangleFlag_All) == TriangleFlag_All)
      tngflags |= TriangleFlag_TopRight;
    if ((LastTriangulatedMap[256 * (pt1->y)   + (pt1->x-1)] & TriangleFlag_All) == TriangleFlag_All)
      tngflags |= TriangleFlag_BottomLeft;
    if ((LastTriangulatedMap[256 * (pt1->y)   + (pt1->x)]   & TriangleFlag_All) == TriangleFlag_All)
      tngflags |= TriangleFlag_BottomRight;
    struct Point *pt2;
    switch (tngflags)
    {
    case TriangleFlag_TopRight + TriangleFlag_BottomLeft:
        pt2 = get_triangle_point(tri_id,MOD3[cor_id+1]);
        if ((pt2->x < pt1->x) || (pt2->y > pt1->y))
            tngflags |= TriangleFlag_BottomRight;
        else
            tngflags |= TriangleFlag_TopLeft;
        break;
    case TriangleFlag_TopLeft + TriangleFlag_BottomRight:
        pt2 = get_triangle_point(tri_id,MOD3[cor_id+1]);
        if ((pt2->x < pt1->x) || (pt2->y > pt1->y))
            tngflags |= TriangleFlag_TopRight;
        else
            tngflags |= TriangleFlag_BottomLeft;
        break;
    case TriangleFlag_All:
        ERRORLOG("solid");
        break;
    }
    *px = (pt1->x << 8) + qdrnt_offs[tngflags].x;
    *py = (pt1->y << 8) + qdrnt_offs[tngflags].y;
}

void nearest_search_f(int32_t sizexy, int32_t srcx, int32_t srcy, int32_t dstx, int32_t dsty, int32_t *px, int32_t *py, const char *func_name)
{
    creature_radius_set(sizexy+1);
    tags_init();
    int32_t tri1_id;
    int32_t tri2_id;
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
    int32_t seltri_id;
    int selcor_id;
    int32_t min_dist;
    signed int cor_id;
    seltri_id = 0;
    selcor_id = 0;
    min_dist = INT32_MAX;
    for (cor_id = 0; cor_id < 3; cor_id++)
    {
        int pt_id;
        pt_id = Triangles[tri1_id].points[cor_id];
        int32_t diff_x;
        int32_t diff_y;
        diff_x = ((ari_Points[pt_id].x << 8) - dstx) >> 5;
        diff_y = ((ari_Points[pt_id].y << 8) - dsty) >> 5;
        int32_t dist;
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
            int32_t ntri;
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
                            int32_t diff_x;
                            int32_t diff_y;
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

static int32_t cost_to_start(int32_t tri_idx)
{
    int64_t len_x;
    int64_t len_y;
    int64_t mincost;
    int64_t newcost;
    struct Point *pt;
    struct Triangle *tri;
    int32_t i;
    mincost = 16777215;
    tri = get_triangle(tri_idx);
    for (i=0; i < 3; i++)
    {
        pt = point_get(tri->points[i]);
        len_x = ((tree_Ax8 >> 8) - (int32_t)(pt->x));
        len_y = ((tree_Ay8 >> 8) - (int32_t)(pt->y));
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
int32_t pointed_at8(int32_t pos_x, int32_t pos_y, int32_t *ret_tri, int32_t *ret_pt)
{
    //TODO PATHFINDING triangulate_area sub-sub-sub-function, verify
    int32_t npt;
    int32_t ntri;
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
    uint32_t k;
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
        int32_t tri_id;
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

static TbBool triangle_check_and_add_navitree_fwd(int32_t ttri)
{
    struct Triangle *tri;
    tri = get_triangle(ttri);
    if (triangle_is_invalid(tri)) {
        ERRORLOG("invalid triangle received, no %d",(int)ttri);
        return false;
    }
    int32_t n;
    int32_t nskipped;
    n = 0;
    nskipped = 0;
    int32_t i;
    int32_t k;
    for (i = 0; i < 3; i++)
    {
        k = tri->tags[i];
        if (!is_current_tag(k))
        {
            if ( fits_thro(ttri, n) )
            {
                NavColour ttri_alt;
                NavColour k_alt;
                ttri_alt = get_triangle_tree_alt(ttri);
                k_alt = get_triangle_tree_alt(k);
                if ((ttri_alt != NAV_COL_UNSET) && (k_alt != NAV_COL_UNSET))
                {
                    int32_t mvcost;
                    int32_t navrule;
                    navrule = nav_rulesA2B(k_alt, ttri_alt);
                    if (navrule)
                    {
                        mvcost = cost_to_start(k);
                        if (navrule == NavigationRule_Special)
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
        NAVIDBG(6,"navigate heap full, %d points ignored",nskipped);
        return false;
    }
    return true;
}

static TbBool triangle_check_and_add_navitree_bak(int32_t ttri)
{
    struct Triangle *tri;
    tri = get_triangle(ttri);
    if (triangle_is_invalid(tri)) {
        ERRORLOG("invalid triangle received");
        return false;
    }
    int32_t nskipped;
    nskipped = 0;
    int32_t i;
    int32_t k;
    for (i = 0; i < 3; i++)
    {
        k = tri->tags[i];
        if (!is_current_tag(k))
        {
            NavColour ttri_alt;
            NavColour k_alt;
            ttri_alt = get_triangle_tree_alt(ttri);
            k_alt = get_triangle_tree_alt(k);
            if ((ttri_alt != NAV_COL_UNSET) && (k_alt != NAV_COL_UNSET))
            {
                int32_t mvcost;
                int32_t navrule;
                navrule = nav_rulesA2B(ttri_alt, k_alt);
                if (navrule)
                {
                    mvcost = cost_to_start(k);
                    if (navrule == NavigationRule_Special)
                        mvcost *= 16;
                    if (!navitree_add(k,ttri,mvcost))
                        nskipped++;
                }
            }
        }
    }
    if (nskipped != 0) {
        NAVIDBG(6,"navigate heap full, %d points ignored",nskipped);
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
static int32_t triangle_route_do_fwd(int32_t ttriA, int32_t ttriB, int32_t *route, int32_t *routecost)
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
        int32_t triangle_heap_first;
        int32_t triangle_heap_second;
        if (naviheap_empty())
            break;
        triangle_heap_first = naviheap_remove();
        if (naviheap_empty())
        {
            triangle_heap_second = -1;
        } else
        {
            triangle_heap_second = naviheap_top();
            if (triangle_heap_second == ttriA)
                break;
            naviheap_remove();
        }
        if (triangle_heap_first != -1)
        {
            triangle_check_and_add_navitree_fwd(triangle_heap_first);
        }
        if (triangle_heap_second != -1)
        {
            triangle_check_and_add_navitree_fwd(triangle_heap_second);
        }
    }
    NAVIDBG(19,"Almost finished");
    if (naviheap_empty()) {
        // The beginning region was never reached
        return -1;
    }
    int32_t i;
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
static int32_t triangle_route_do_bak(int32_t ttriA, int32_t ttriB, int32_t *route, int32_t *routecost)
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
        int32_t triangle_heap_first;
        int32_t triangle_heap_second;
        if (naviheap_empty())
            break;
        triangle_heap_first = naviheap_remove();
        if (naviheap_empty())
        {
            triangle_heap_second = -1;
        } else
        {
            triangle_heap_second = naviheap_top();
            if (triangle_heap_second == ttriA)
                break;
            naviheap_remove();
        }
        if (triangle_heap_first != -1)
        {
            triangle_check_and_add_navitree_bak(triangle_heap_first);
        }
        if (triangle_heap_second != -1)
        {
            triangle_check_and_add_navitree_bak(triangle_heap_second);
        }
    }
    NAVIDBG(19,"Almost finished");
    if (naviheap_empty()) {
        // The beginning region was never reached
        return -1;
    }
    int32_t i;
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
static int32_t ma_triangle_route(int32_t ttriA, int32_t ttriB, int32_t *routecost)
{
    int32_t forward_route_length;
    int32_t backward_route_length;
    int32_t par_fwd;
    int32_t par_bak;
    int32_t rcost_fwd;
    int32_t rcost_bak;
    int32_t tx;
    int32_t ty;
    // We need to make testing system for routing, then fix the rewritten code
    // and compare results with the original code.
    // Forward route
    NAVIDBG(19,"Making forward route");
    rcost_fwd = 0;
    forward_route_length = triangle_route_do_fwd(ttriA, ttriB, route_fwd, &rcost_fwd);
    if (forward_route_length == -1)
    {
        NAVIDBG(19,"No forward route");
        return -1;
    }
    route_to_path(tree_Ax8, tree_Ay8, tree_Bx8, tree_By8, route_fwd, forward_route_length, &fwd_path, &par_fwd);
    tx = tree_Ax8;
    ty = tree_Ay8;
    tree_Ax8 = tree_Bx8;
    tree_Ay8 = tree_By8;
    tree_Bx8 = tx;
    tree_By8 = ty;
    // Backward route
    NAVIDBG(19,"Making backward route");
    rcost_bak = 0;
    backward_route_length = triangle_route_do_bak(ttriB, ttriA, route_bak, &rcost_bak);
    if (backward_route_length == -1)
    {
        NAVIDBG(19,"No backward route");
        return -1;
    }
    route_to_path(tree_Ax8, tree_Ay8, tree_Bx8, tree_By8, route_bak, backward_route_length, &bak_path, &par_bak);
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
        for (size_t i = 0; i < sizeof(tree_route)/sizeof(tree_route[0]); i++)
        {
             tree_route[i] = route_fwd[i];
        }
        *routecost = rcost_fwd;
        return forward_route_length;
    } else
    {
        for (size_t i = 0; i <= (size_t) backward_route_length; i++)
        {
             tree_route[i] = route_bak[backward_route_length-i];
        }
        *routecost = rcost_bak;
        return backward_route_length;
    }
}

static void edgelen_init(void)
{
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

static TbBool ariadne_creature_reached_position(const struct Thing *thing, const struct Coord3d *pos)
{
    if (thing->mappos.x.val != pos->x.val)
    return false;
    if (thing->mappos.y.val != pos->y.val)
        return false;
    return true;
}

static int32_t ariadne_creature_blocked_by_wall_at(struct Thing *thing, const struct Coord3d *pos)
{
    struct Coord3d mvpos;
    int32_t zmem;
    int32_t ret;
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

static void ariadne_pull_out_waypoint(const struct Thing *thing, const struct Ariadne *arid, int32_t wpoint_id, struct Coord3d *pos)
{
    const struct Coord2d *wp;
    int32_t size_radius;
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
    if (wp->x.stl.pos == (COORD_PER_STL-1))
    {
        pos->x.val -= size_radius+1;
    } else
    if (wp->x.stl.pos != 0)
    {
        pos->x.val += size_radius;
    }

    if (wp->y.stl.pos == (COORD_PER_STL-1))
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
static void ariadne_init_current_waypoint(const struct Thing *thing, struct Ariadne *arid)
{
    ariadne_pull_out_waypoint(thing, arid, arid->current_waypoint, &arid->current_waypoint_pos);
    arid->current_waypoint_pos.z.val = get_thing_height_at(thing, &arid->current_waypoint_pos);
    arid->straight_dist_to_next_waypoint = get_2d_distance(&thing->mappos, &arid->current_waypoint_pos);
}

int32_t angle_to_quadrant(int32_t angle)
{
    return ((angle + DEGREES_45) / DEGREES_90) & 3;
}

static TbBool ariadne_wallhug_angle_valid(struct Thing *thing, struct Ariadne *arid, int32_t angle)
{
    struct Coord3d pos;
    pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, angle);
    pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, angle);
    pos.z.val = subtile_coord(1,0);
    return (ariadne_creature_blocked_by_wall_at(thing, &pos) == 0);
}

static int32_t ariadne_get_wallhug_angle(struct Thing *thing, struct Ariadne *arid)
{
    int32_t whangle;
    if (arid->hug_side == WallhugPreference_Right)
    {
        whangle = DEGREES_90 * ((angle_to_quadrant(thing->move_angle_xy) - 1) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = DEGREES_90 * (angle_to_quadrant(thing->move_angle_xy) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = DEGREES_90 * ((angle_to_quadrant(thing->move_angle_xy) + 1) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = DEGREES_90 * ((angle_to_quadrant(thing->move_angle_xy) + 2) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
    } else
    if (arid->hug_side == WallhugPreference_Left)
    {
        whangle = DEGREES_90 * ((angle_to_quadrant(thing->move_angle_xy) + 1) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = DEGREES_90 * (angle_to_quadrant(thing->move_angle_xy) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = DEGREES_90 * ((angle_to_quadrant(thing->move_angle_xy) - 1) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
        whangle = DEGREES_90 * ((angle_to_quadrant(thing->move_angle_xy) + 2) & 3);
        if (ariadne_wallhug_angle_valid(thing, arid, whangle))
            return whangle;
    }
    return -1;
}

static void ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(struct Thing *thing, struct Ariadne *arid, int32_t inangle, short *rangle, unsigned char *rflag)
{
    struct Coord3d bkp_mappos;
    bkp_mappos = thing->mappos;
    int inangle_oneaxis;
    int bkp_angle_xy;
    int bkp_hug_side;
    int bkp_speed;
    bkp_angle_xy = thing->move_angle_xy;
    bkp_speed = arid->move_speed;
    bkp_hug_side = arid->hug_side;
    int angle_beg;
    int hug_side;
    int angle_end;
    if (inangle == ANGLE_NORTH)
    {
        angle_beg = ANGLE_WEST;
        hug_side = WallhugPreference_Left;
        angle_end = ANGLE_EAST;
        inangle_oneaxis = 1;
    } else
    if (inangle == ANGLE_EAST)
    {
        angle_beg = ANGLE_NORTH;
        hug_side = WallhugPreference_Left;
        angle_end = ANGLE_SOUTH;
        inangle_oneaxis = 1;
    } else
    if (inangle == ANGLE_SOUTH)
    {
        angle_beg = ANGLE_EAST;
        hug_side = WallhugPreference_Left;
        angle_end = ANGLE_WEST;
        inangle_oneaxis = 1;
    } else
    if (inangle == ANGLE_WEST)
    {
        angle_beg = ANGLE_SOUTH;
        hug_side = WallhugPreference_Left;
        angle_end = ANGLE_NORTH;
        inangle_oneaxis = 1;
    } else
    {
        NAVIDBG(9,"Unsupported inangle %d",(int)inangle);
        angle_beg = 0;
        hug_side = WallhugPreference_None;
        angle_end = 0;
        inangle_oneaxis = 0;
    }
    arid->move_speed = 256;
    int whsteps;
    int wallhug_distance_left;
    int wallhug_distance_right;
    int size_steps;
    size_steps = thing_nav_sizexy(thing) >> 9;
    thing->move_angle_xy = angle_beg;
    size_steps += 2;
    whsteps = size_steps;
    wallhug_distance_right = size_steps;
    wallhug_distance_left = size_steps;
    arid->hug_side = hug_side;
    struct Coord3d pos;
    int i;
    int32_t hug_angle;
    for (i = 0; i < whsteps; i++)
    {
        hug_angle = ariadne_get_wallhug_angle(thing, arid);
        if (hug_angle != -1)
        {
            if (hug_angle == inangle) {
                wallhug_distance_right = i;
                break;
            }
            pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, hug_angle);
            pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, hug_angle);
            pos.z.val = get_thing_height_at(thing, &pos);
            if (ariadne_check_forward_for_wallhug_gap(thing, arid, &pos, hug_angle)) {
                wallhug_distance_right = i;
                break;
            }
            thing->mappos = pos;
        }
    }
    thing->move_angle_xy = angle_end;
    arid->hug_side = inangle_oneaxis;
    thing->mappos = bkp_mappos;
    for (i = 0; i < whsteps; i++)
    {
        hug_angle = ariadne_get_wallhug_angle(thing, arid);
        if (hug_angle != -1)
        {
            if (hug_angle == inangle) {
                wallhug_distance_left = i;
                break;
            }
            pos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, hug_angle);
            pos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, hug_angle);
            pos.z.val = get_thing_height_at(thing, &pos);
            if (ariadne_check_forward_for_wallhug_gap(thing, arid, &pos, hug_angle)) {
                wallhug_distance_left = i;
                break;
            }
            thing->mappos = pos;
        }
    }
    thing->move_angle_xy = bkp_angle_xy;
    thing->mappos = bkp_mappos;
    arid->move_speed = bkp_speed;
    arid->hug_side = bkp_hug_side;
    if (wallhug_distance_left > wallhug_distance_right)
    {
        *rangle = angle_beg;
        *rflag = hug_side;
    } else
    if (wallhug_distance_left >= wallhug_distance_right)
    {
        *rangle = angle_beg;
        *rflag = hug_side;
    } else
    {
        *rangle = angle_end;
        *rflag = inangle_oneaxis;
    }
}

static TbBool ariadne_get_starting_angle_and_side_of_wallhug(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos, short *rangle, unsigned char *rflag)
{
    TbBool nxdelta_x_neg;
    TbBool nxdelta_y_neg;
    TbBool crdelta_x_neg;
    TbBool crdelta_y_neg;
    crdelta_x_neg = (thing->mappos.x.val - (int32_t)pos->x.val) <= 0;
    crdelta_y_neg = (thing->mappos.y.val - (int32_t)pos->y.val) <= 0;
    nxdelta_x_neg = (thing->mappos.x.val - (int32_t)arid->current_waypoint_pos.x.val) <= 0;
    nxdelta_y_neg = (thing->mappos.y.val - (int32_t)arid->current_waypoint_pos.y.val) <= 0;
    int axis_closer;
    int nav_radius;
    axis_closer = abs(thing->mappos.x.val - arid->current_waypoint_pos.x.val) < abs(thing->mappos.y.val - arid->current_waypoint_pos.y.val);
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
    uint32_t blk_flags;
    blk_flags = ariadne_get_blocked_flags(thing, pos);
    if ((blk_flags & SlbBloF_WalledX) != 0)
    {
        if ((wp_y >= cur_pos_y_beg) && (wp_y <= cur_pos_y_end))
        {
            if (nxdelta_x_neg)
                ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, ANGLE_EAST, rangle, rflag);
            else
                ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, ANGLE_WEST, rangle, rflag);
        } else
        {
            *rangle = blocked_x_hug_start[crdelta_x_neg][nxdelta_y_neg].wh_angle;
            *rflag = blocked_x_hug_start[crdelta_x_neg][nxdelta_y_neg].wh_side;
        }
        return true;
    }
    if ((blk_flags & SlbBloF_WalledY) != 0)
    {
        if ((wp_x >= cur_pos_x_beg) && (wp_x <= cur_pos_x_end))
        {
            if (nxdelta_y_neg)
                ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, ANGLE_SOUTH, rangle, rflag);
            else
                ariadne_get_starting_angle_and_side_of_wallhug_for_desireable_move(thing, arid, ANGLE_NORTH, rangle, rflag);
        } else
        {
            *rangle = blocked_y_hug_start[crdelta_y_neg][nxdelta_x_neg].wh_angle;
            *rflag = blocked_y_hug_start[crdelta_y_neg][nxdelta_x_neg].wh_side;
        }
        return true;
    }
    if ((blk_flags & SlbBloF_WalledZ) != 0)
    {
        *rangle = blocked_xy_hug_start[crdelta_y_neg][crdelta_x_neg][axis_closer].wh_angle;
        *rflag = blocked_xy_hug_start[crdelta_y_neg][crdelta_x_neg][axis_closer].wh_side;
        return true;
    }
    return false;
}

static AriadneReturn ariadne_init_wallhug(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos)
{
    if (arid->move_speed <= 0) {
        ERRORLOG("Ariadne Speed not positive");
    }
    if (!ariadne_get_starting_angle_and_side_of_wallhug(thing, arid, pos, &arid->wallhug_angle, &arid->hug_side))
    {
        arid->next_position.x.val = thing->mappos.x.val;
        arid->next_position.y.val = thing->mappos.y.val;
        arid->next_position.z.val = thing->mappos.z.val;
        arid->update_state = AridUpSt_OnLine;
        return AridRet_OK;
    }
    arid->update_state = AridUpSt_Wallhug;
    arid->next_position.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, arid->wallhug_angle);
    arid->next_position.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, arid->wallhug_angle);
    arid->next_position.z.val = get_thing_height_at(thing, &arid->next_position);
    arid->previous_position = thing->mappos;
    arid->wallhug_stored_angle = arid->wallhug_angle;
    if (ariadne_check_forward_for_wallhug_gap(thing, arid, &arid->next_position, arid->wallhug_angle))
    {
        arid->manoeuvre_fixed_position.x.val = arid->next_position.x.val;
        arid->manoeuvre_fixed_position.y.val = arid->next_position.y.val;
        arid->manoeuvre_fixed_position.z.val = arid->next_position.z.val;
        arid->update_state = AridUpSt_Manoeuvre;
        arid->manoeuvre_state = AridUpSStM_ContinueWallhug;
        return AridRet_OK;
    }
    int32_t cannot_move;
    {
        MapCoord tng_z_mem;
        tng_z_mem = thing->mappos.z.val;
        struct Coord3d mvpos;
        mvpos.x.val = arid->next_position.x.val;
        mvpos.y.val = arid->next_position.y.val;
        mvpos.z.val = get_floor_height_under_thing_at(thing, &thing->mappos);
        thing->mappos.z.val = mvpos.z.val;
        cannot_move = creature_cannot_move_directly_to(thing, &mvpos);
        thing->mappos.z.val = tng_z_mem;
    }
    if ( cannot_move )
    {
        struct Coord3d pos2;
        ariadne_push_position_against_wall(thing, &arid->next_position, &pos2);
        arid->manoeuvre_fixed_position.x.val = pos2.x.val;
        arid->manoeuvre_fixed_position.y.val = pos2.y.val;
        arid->manoeuvre_fixed_position.z.val = pos2.z.val;
        arid->manoeuvre_requested_position.x.val = arid->next_position.x.val;
        arid->manoeuvre_requested_position.y.val = arid->next_position.y.val;
        arid->manoeuvre_requested_position.z.val = arid->next_position.z.val;
        arid->update_state = AridUpSt_Manoeuvre;
        arid->manoeuvre_state = AridUpSStM_StartWallhug;
        return AridRet_OK;
    }
    return AridRet_OK;
}

static int32_t ariadne_get_blocked_flags(struct Thing *thing, const struct Coord3d *pos)
{
    struct Coord3d lpos;
    uint32_t blkflags;
    lpos.x.val = pos->x.val;
    lpos.y.val = thing->mappos.y.val;
    lpos.z.val = thing->mappos.z.val;
    blkflags = SlbBloF_None;
    if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
        blkflags |= SlbBloF_WalledX;
    lpos.x.val = thing->mappos.x.val;
    lpos.y.val = pos->y.val;
    lpos.z.val = thing->mappos.z.val;
    if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
        blkflags |= SlbBloF_WalledY;
    if (blkflags == SlbBloF_None)
    {
        lpos.x.val = pos->x.val;
        lpos.y.val = pos->y.val;
        lpos.z.val = thing->mappos.z.val;
        if (ariadne_creature_blocked_by_wall_at(thing, &lpos))
          blkflags |= SlbBloF_WalledZ;
    }
    return blkflags;
}

static TbBool blocked_by_door_at(struct Thing *thing, struct Coord3d *pos, uint32_t blk_flags)
{
    int32_t radius;
    int32_t start_x;
    int32_t end_x;
    int32_t start_y;
    int32_t end_y;
    int32_t stl_x;
    int32_t stl_y;

    radius = thing_nav_sizexy(thing) >> 1;
    start_x = ((int32_t)pos->x.val - radius) / 256;
    end_x = ((int32_t)pos->x.val + radius) / 256;
    start_y = ((int32_t)pos->y.val - radius) / 256;
    end_y = ((int32_t)pos->y.val + radius) / 256;
    if ((blk_flags & SlbBloF_WalledX) != 0)
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
    if ((blk_flags & SlbBloF_WalledY) != 0)
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

static int32_t ariadne_push_position_against_wall(struct Thing *thing, const struct Coord3d *pos1, struct Coord3d *pos_out)
{
    struct Coord3d lpos;
    int32_t radius;
    uint32_t blk_flags;
    blk_flags = ariadne_get_blocked_flags(thing, pos1);
    radius = thing_nav_sizexy(thing) >> 1;
    lpos.x.val = pos1->x.val;
    lpos.y.val = pos1->y.val;
    lpos.z.val = 0;

    if ((blk_flags & SlbBloF_WalledX) != 0)
    {
      if (pos1->x.val >= thing->mappos.x.val)
      {
          lpos.x.val = thing->mappos.x.val + radius;
          lpos.x.stl.pos = COORD_PER_STL-1;
          lpos.x.val -= radius;
      } else
      {
          lpos.x.val = thing->mappos.x.val - radius;
          lpos.x.stl.pos = 0;
          lpos.x.val += radius;
      }
      lpos.z.val = get_thing_height_at(thing, &lpos);
    }
    if ((blk_flags & SlbBloF_WalledY) != 0)
    {
      if (pos1->y.val >= thing->mappos.y.val)
      {
        lpos.y.val = thing->mappos.y.val + radius;
        lpos.y.stl.pos = COORD_PER_STL-1;
        lpos.y.val -= radius;
      } else
      {
        lpos.y.val = thing->mappos.y.val - radius;
        lpos.y.stl.pos = 0;
        lpos.y.val += radius;
      }
      lpos.z.val = get_thing_height_at(thing, &lpos);
    }
    if ((blk_flags & SlbBloF_WalledZ) != 0)
    {
      if (pos1->x.val >= thing->mappos.x.val)
      {
          lpos.x.val = thing->mappos.x.val + radius;
          lpos.x.stl.pos = COORD_PER_STL-1;
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
          lpos.y.stl.pos = COORD_PER_STL-1;
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

static int32_t ariadne_init_movement_to_current_waypoint(struct Thing *thing, struct Ariadne *arid)
{
    struct Coord3d requested_pos;
    struct Coord3d fixed_pos;
    int32_t angle;
    int32_t delta_x;
    int32_t delta_y;
    uint32_t blk_flags;
    TRACE_THING(thing);
    angle = get_angle_xy_to(&thing->mappos, &arid->current_waypoint_pos);
    delta_x = distance_with_angle_to_coord_x(arid->move_speed, angle);
    delta_y = distance_with_angle_to_coord_y(arid->move_speed, angle);
    requested_pos.x.val = (int32_t)thing->mappos.x.val + delta_x;
    requested_pos.y.val = (int32_t)thing->mappos.y.val + delta_y;
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
    if ( (((blk_flags & SlbBloF_WalledX) != 0) && (thing->mappos.x.val == fixed_pos.x.val))
      || (((blk_flags & SlbBloF_WalledY) != 0) && (thing->mappos.y.val == fixed_pos.y.val)) )
    {
        ariadne_init_wallhug(thing, arid, &requested_pos);
        arid->wallhug_active = WallhugActive_On;
        return 1;
    }
    arid->manoeuvre_fixed_position.x.val = fixed_pos.x.val;
    arid->manoeuvre_fixed_position.y.val = fixed_pos.y.val;
    arid->manoeuvre_fixed_position.z.val = fixed_pos.z.val;
    arid->manoeuvre_requested_position.x.val = requested_pos.x.val;
    arid->manoeuvre_requested_position.y.val = requested_pos.y.val;
    arid->manoeuvre_requested_position.z.val = requested_pos.z.val;
    arid->next_position.x.val = fixed_pos.x.val;
    arid->next_position.y.val = fixed_pos.y.val;
    arid->next_position.z.val = fixed_pos.z.val;
    arid->update_state = AridUpSt_Manoeuvre;
    arid->manoeuvre_state = AridUpSStM_StartWallhug;
    return 1;
}

static int32_t ariadne_creature_can_continue_direct_line_to_waypoint(struct Thing *thing, struct Ariadne *arid, int32_t speed)
{
    int32_t angle;
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
static AriadneReturn ariadne_prepare_creature_route_target_reached(const struct Thing *thing, struct Ariadne *arid, const struct Coord3d *srcpos, const struct Coord3d *dstpos)
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
    arid->next_position.x.val = thing->mappos.x.val;
    arid->next_position.y.val = thing->mappos.y.val;
    arid->next_position.z.val = thing->mappos.z.val;
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
static AriadneReturn ariadne_prepare_creature_route_to_target_f(const struct Thing *thing, struct Ariadne *arid,
    const struct Coord3d *srcpos, const struct Coord3d *dstpos, int32_t speed, AriadneRouteFlags flags, const char *func_name)
{
    struct Path path;
    int32_t nav_sizexy;
    NAVIDBG(18,"%s: The %s index %d from %3d,%3d to %3d,%3d", func_name, thing_model_name(thing), (int)thing->index,
        (int)srcpos->x.stl.num, (int)srcpos->y.stl.num, (int)dstpos->x.stl.num, (int)dstpos->y.stl.num);
    memset(&path, 0, sizeof(struct Path));
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
        return AridRet_Failed;
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
    int32_t i;
    int32_t k;
    k = 0;
    for (i = 0; i < arid->stored_waypoints; i++)
    {
        arid->waypoints[i].x.val = path.waypoints[k].x;
        arid->waypoints[i].y.val = path.waypoints[k].y;
        k++;
    }
    arid->current_waypoint = 0;
    arid->route_flags = flags;
    arid->next_position.x.val = thing->mappos.x.val;
    arid->next_position.y.val = thing->mappos.y.val;
    arid->next_position.z.val = thing->mappos.z.val;
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
int32_t ariadne_count_waypoints_on_creature_route_to_target_f(const struct Thing *thing,
    const struct Coord3d *srcpos, const struct Coord3d *dstpos, AriadneRouteFlags flags, const char *func_name)
{
    struct Path path;
    int32_t nav_sizexy;
    NAVIDBG(18,"%s: The %s index %d from %3d,%3d to %3d,%3d", func_name, thing_model_name(thing), (int)thing->index,
        (int)srcpos->x.stl.num, (int)srcpos->y.stl.num, (int)dstpos->x.stl.num, (int)dstpos->y.stl.num);
    memset(&path, 0, sizeof(struct Path));
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
    memset(arid, 0, sizeof(struct Ariadne));
    return AridRet_OK;
}

AriadneReturn ariadne_initialise_creature_route_f(struct Thing *thing, const struct Coord3d *pos, int32_t speed, AriadneRouteFlags flags, const char *func_name)
{
    struct CreatureControl *cctrl;
    struct Ariadne *arid;
    AriadneReturn ret;
    NAVIDBG(18,"%s: Route for %s index %d from %3d,%3d to %3d,%3d", func_name,thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)pos->x.stl.num, (int)pos->y.stl.num);
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    arid = &cctrl->arid;
    memset(arid, 0, sizeof(struct Ariadne));
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

static AriadneReturn ariadne_creature_get_next_waypoint(struct Thing *thing, struct Ariadne *arid)
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

static TbBool ariadne_creature_reached_waypoint(const struct Thing *thing, const struct Ariadne *arid)
{
    return ariadne_creature_reached_position(thing, &arid->current_waypoint_pos);
}

static AriadneReturn ariadne_update_state_manoeuvre_to_position(struct Thing *thing, struct Ariadne *arid)
{
    struct Coord3d pos;
    MapCoord dist;
    int32_t hug_angle;

    if (ariadne_creature_blocked_by_wall_at(thing, &arid->manoeuvre_fixed_position))
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
    if (arid->straight_dist_to_next_waypoint > dist)
        arid->straight_dist_to_next_waypoint = dist;
    if ((thing->mappos.x.val != arid->manoeuvre_fixed_position.x.val)
     || (thing->mappos.y.val != arid->manoeuvre_fixed_position.y.val))
    {
        arid->next_position.x.val = arid->manoeuvre_fixed_position.x.val;
        arid->next_position.y.val = arid->manoeuvre_fixed_position.y.val;
        arid->next_position.z.val = arid->manoeuvre_fixed_position.z.val;
        return AridRet_OK;
    }
    switch (arid->manoeuvre_state)
    {
    case AridUpSStM_StartWallhug:
        return ariadne_init_wallhug(thing, arid, &arid->manoeuvre_requested_position);
    case AridUpSStM_ContinueWallhug:
        hug_angle = ariadne_get_wallhug_angle(thing, arid);
        arid->wallhug_angle = hug_angle;
        arid->next_position.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, hug_angle);
        arid->next_position.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, hug_angle);
        arid->update_state = AridUpSt_Wallhug;
        return AridRet_OK;
    default:
        ERRORLOG("Unknown Manoeuvre state %d",(int)arid->manoeuvre_state);
        break;
    }
    return AridRet_OK;
}

static AriadneReturn ariadne_update_state_on_line(struct Thing *thing, struct Ariadne *arid)
{
    int32_t angle;
    int32_t distance;
    NAVIDBG(19,"Starting");
    angle = get_angle_xy_to(&thing->mappos, &arid->current_waypoint_pos);
    distance = get_2d_distance(&thing->mappos, &arid->current_waypoint_pos);
    if ((distance - arid->straight_dist_to_next_waypoint) > 4 * COORD_PER_STL)
    {
        struct Coord3d pos;
        arid->next_position.x.val = thing->mappos.x.val;
        arid->next_position.y.val = thing->mappos.y.val;
        arid->next_position.z.val = thing->mappos.z.val;
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
            arid->next_position.x.val = arid->current_waypoint_pos.x.val;
            arid->next_position.y.val = arid->current_waypoint_pos.y.val;
            arid->next_position.z.val = arid->current_waypoint_pos.z.val;
        }
        else
        {
            arid->next_position.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, angle);
            arid->next_position.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, angle);
            arid->next_position.z.val = get_thing_height_at(thing, &arid->next_position);
        }
    }
    if (arid->straight_dist_to_next_waypoint > distance) {
        arid->straight_dist_to_next_waypoint = distance;
    }
    if (ariadne_creature_blocked_by_wall_at(thing, &arid->next_position))
    {
        if ( arid->may_need_reroute )
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
            uint32_t blk_flags;
            blk_flags = ariadne_get_blocked_flags(thing, &arid->next_position);
            if (!blocked_by_door_at(thing, &arid->next_position, blk_flags))
            {
                struct Coord3d pos;
                ariadne_push_position_against_wall(thing, &arid->next_position, &pos);
                arid->update_state = AridUpSt_Manoeuvre;
                arid->manoeuvre_state = AridUpSStM_StartWallhug;
                arid->manoeuvre_fixed_position.x.val = pos.x.val;
                arid->manoeuvre_fixed_position.y.val = pos.y.val;
                arid->manoeuvre_fixed_position.z.val = pos.z.val;
                arid->manoeuvre_requested_position.x.val = arid->next_position.x.val;
                arid->manoeuvre_requested_position.y.val = arid->next_position.y.val;
                arid->manoeuvre_requested_position.z.val = arid->next_position.z.val;
                arid->next_position.x.val = pos.x.val;
                arid->next_position.y.val = pos.y.val;
                arid->next_position.z.val = pos.z.val;
            }
        }
    }
    return AridRet_OK;
}

static TbBool ariadne_check_forward_for_wallhug_gap(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos, int32_t hug_angle)
{
    struct Coord3d nav_boundry_pos;
    struct Coord3d potentional_next_pos_3d;
    struct Coord3d original_mappos;

    int32_t nav_radius = thing_nav_sizexy(thing) / 2;

    TbBool isOk = false;
    switch (hug_angle)
    {
    case ANGLE_NORTH:
        if ((int)((pos->y.val - nav_radius) & 0xFFFFFF00) < (int)((thing->mappos.y.val - nav_radius) & 0xFFFFFF00))
        {
            nav_boundry_pos.x.val = pos->x.val;
            nav_boundry_pos.y.val = subtile_coord((thing->mappos.y.val - nav_radius) >> 8, 0) + nav_radius;
            isOk = true;
        }
        break;
    case ANGLE_SOUTH:
        if ((int)((nav_radius + pos->y.val) & 0xFFFFFF00) > (int)((nav_radius + thing->mappos.y.val) & 0xFFFFFF00))
        {
            nav_boundry_pos.x.val = pos->x.val;
            nav_boundry_pos.y.val = subtile_coord((nav_radius + thing->mappos.y.val) >> 8, COORD_PER_STL - 1) - nav_radius;
            isOk = true;
        }
        break;
    case ANGLE_WEST:
        if ((int)((pos->x.val - nav_radius) & 0xFFFFFF00) < (int)((thing->mappos.x.val - nav_radius) & 0xFFFFFF00))
        {
            nav_boundry_pos.y.val = pos->y.val;
            nav_boundry_pos.x.val = subtile_coord((thing->mappos.x.val - nav_radius) >> 8, 0) + nav_radius;
            isOk = true;
        }
        break;
    case ANGLE_EAST:
        if ((int)((nav_radius + pos->x.val) & 0xFFFFFF00) > (int)((nav_radius + thing->mappos.x.val) & 0xFFFFFF00))
        {
            nav_boundry_pos.y.val = pos->y.val;
            nav_boundry_pos.x.val = subtile_coord((nav_radius + thing->mappos.x.val) >> 8, COORD_PER_STL - 1) - nav_radius;
            isOk = true;
        }
        break;
    default:
        return 0;
    }
    if (!isOk)
        return 0;

    char angle_offset;

    if (arid->hug_side == WallhugPreference_Right)
        angle_offset = -1;
    else if (arid->hug_side == WallhugPreference_Left)
        angle_offset = 1;
    else
        return 0;

    int32_t quadrant = DEGREES_90 * ((angle_to_quadrant(hug_angle) + angle_offset) & 3);

    potentional_next_pos_3d.x.val = move_coord_with_angle_x(thing->mappos.x.val, arid->move_speed, quadrant);
    potentional_next_pos_3d.y.val = move_coord_with_angle_y(thing->mappos.y.val, arid->move_speed, quadrant);
    potentional_next_pos_3d.z.val = get_floor_height_under_thing_at(thing, &thing->mappos);

    thing->mappos.z.val = potentional_next_pos_3d.z.val;
    TbBool cant_move_to_pos_directly = creature_cannot_move_directly_to(thing, &potentional_next_pos_3d);

    if (cant_move_to_pos_directly)
    {
        original_mappos = thing->mappos;
        thing->mappos = nav_boundry_pos;

        thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);

        potentional_next_pos_3d.x.val = move_coord_with_angle_x(thing->mappos.x.val, arid->move_speed, quadrant);
        potentional_next_pos_3d.y.val = move_coord_with_angle_y(thing->mappos.y.val, arid->move_speed, quadrant);
        potentional_next_pos_3d.z.val = get_floor_height_under_thing_at(thing, &thing->mappos);
        thing->mappos.z.val = potentional_next_pos_3d.z.val;
        cant_move_to_pos_directly = creature_cannot_move_directly_to(thing, &potentional_next_pos_3d);

        thing->mappos = original_mappos;

        if (cant_move_to_pos_directly)
        {
            return false;
        }
        *pos = nav_boundry_pos;
        return true;
    }
    return false;
}

static TbBool ariadne_creature_on_circular_hug(const struct Thing *thing, const struct Ariadne *arid)
{
    int32_t src_x;
    int32_t src_y;
    int32_t dst_x;
    int32_t dst_y;
    dst_x = arid->previous_position.x.val;
    src_x = thing->mappos.x.val;
    dst_y = arid->previous_position.y.val;
    src_y = thing->mappos.y.val;
    if (dst_x == src_x)
    {
      if ((dst_y >= src_y) && (arid->next_position.y.val >= dst_y)) {
          return true;
      }
      if ((dst_y <= src_y) && (arid->next_position.y.val <= dst_y)) {
          return true;
      }
      return false;
    }
    if (dst_y == src_y)
    {
        if ((dst_x >= src_x) && (arid->next_position.x.val >= dst_x)) {
              return true;
        }
        if ((dst_x <= src_x) && (arid->next_position.x.val <= dst_x)) {
            return true;
        }
    }
    return false;
}

static AriadneReturn ariadne_update_state_wallhug(struct Thing *thing, struct Ariadne *arid)
{
    MapCoordDelta distance;
    NAVIDBG(18,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.val, (int)thing->mappos.y.val, (int)arid->current_waypoint_pos.x.val, (int)arid->current_waypoint_pos.y.val);
    distance = get_2d_distance(&thing->mappos, &arid->current_waypoint_pos);
    if ((distance - arid->straight_dist_to_next_waypoint) > 4 * COORD_PER_STL)
    {
        struct Coord3d pos;
        arid->next_position.x.val = thing->mappos.x.val;
        arid->next_position.y.val = thing->mappos.y.val;
        arid->next_position.z.val = thing->mappos.z.val;
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
        arid->next_position.x.val = arid->current_waypoint_pos.x.val;
        arid->next_position.y.val = arid->current_waypoint_pos.y.val;
        arid->next_position.z.val = arid->current_waypoint_pos.z.val;
        if (ariadne_creature_blocked_by_wall_at(thing, &arid->next_position))
        {
            if ( arid->may_need_reroute )
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
                ariadne_push_position_against_wall(thing, &arid->next_position, &pos);
                arid->update_state = AridUpSt_Manoeuvre;
                arid->manoeuvre_state = AridUpSStM_StartWallhug;
                arid->manoeuvre_fixed_position.x.val = pos.x.val;
                arid->manoeuvre_fixed_position.y.val = pos.y.val;
                arid->manoeuvre_fixed_position.z.val = pos.z.val;
                arid->manoeuvre_requested_position.x.val = arid->next_position.x.val;
                arid->manoeuvre_requested_position.y.val = arid->next_position.y.val;
                arid->manoeuvre_requested_position.z.val = arid->next_position.z.val;
                arid->next_position.x.val = pos.x.val;
                arid->next_position.y.val = pos.y.val;
                arid->next_position.z.val = pos.z.val;
            }
        }
    } else
    if (thing->move_angle_xy == arid->wallhug_angle)
    {
        if ((thing->mappos.x.val != arid->next_position.x.val) || (arid->next_position.y.val != thing->mappos.y.val))
        {
            ariadne_init_movement_to_current_waypoint(thing, arid);
            return 0;
        }
        if (distance < arid->straight_dist_to_next_waypoint)
        {
            if (ariadne_creature_can_continue_direct_line_to_waypoint(thing, arid, arid->move_speed)) {
                if (ariadne_init_movement_to_current_waypoint(thing, arid) < 1) {
                    return AridRet_PartOK;
                }
                return AridRet_OK;
            }
            arid->straight_dist_to_next_waypoint = distance;
        }
        int32_t hug_angle;
        hug_angle = ariadne_get_wallhug_angle(thing, arid);
        if (hug_angle == -1)
        {
            ariadne_init_movement_to_current_waypoint(thing, arid);
            return AridRet_OK;
        }
        if (hug_angle != thing->move_angle_xy)
        {
            struct Coord3d pos;
            pos.x.val = arid->endpos.x.val;
            pos.y.val = arid->endpos.y.val;
            pos.z.val = arid->endpos.z.val;
            if (ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags) == AridRet_OK) {
                return AridRet_OK;
            }
        }
        arid->next_position.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(arid->move_speed, hug_angle);
        arid->next_position.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(arid->move_speed, hug_angle);
        arid->next_position.z.val = get_thing_height_at(thing, &arid->next_position);
        if ((thing->move_angle_xy == hug_angle) && ariadne_check_forward_for_wallhug_gap(thing, arid, &arid->next_position, hug_angle))
        {
            arid->update_state = AridUpSt_Manoeuvre;
            arid->manoeuvre_state = AridUpSStM_ContinueWallhug;
            arid->manoeuvre_fixed_position.x.val = arid->next_position.x.val;
            arid->manoeuvre_fixed_position.y.val = arid->next_position.y.val;
            arid->manoeuvre_fixed_position.z.val = arid->next_position.z.val;
            return AridRet_OK;
        }
        arid->wallhug_angle = hug_angle;
        if (ariadne_creature_on_circular_hug(thing, arid))
        {
            struct Coord3d pos;
            arid->next_position.x.val = thing->mappos.x.val;
            arid->next_position.y.val = thing->mappos.y.val;
            arid->next_position.z.val = thing->mappos.z.val;
            pos.x.val = arid->endpos.x.val;
            pos.y.val = arid->endpos.y.val;
            pos.z.val = arid->endpos.z.val;
            if (ariadne_initialise_creature_route(thing, &pos, arid->move_speed, arid->route_flags) >= 1) {
                return 3;
            }
            return AridRet_OK;
        }
        if (ariadne_creature_blocked_by_wall_at(thing, &arid->next_position))
        {
            if (!arid->may_need_reroute)
            {
                struct Coord3d pos;
                ariadne_push_position_against_wall(thing, &arid->next_position, &pos);
                arid->update_state = AridUpSt_Manoeuvre;
                arid->manoeuvre_state = AridUpSStM_ContinueWallhug;
                arid->manoeuvre_fixed_position.x.val = pos.x.val;
                arid->manoeuvre_fixed_position.y.val = pos.y.val;
                arid->manoeuvre_fixed_position.z.val = pos.z.val;
                arid->manoeuvre_requested_position.x.val = arid->next_position.x.val;
                arid->manoeuvre_requested_position.y.val = arid->next_position.y.val;
                arid->manoeuvre_requested_position.z.val = arid->next_position.z.val;
                arid->next_position.x.val = pos.x.val;
                arid->next_position.y.val = pos.y.val;
                arid->next_position.z.val = pos.z.val;
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
static AriadneReturn ariadne_get_next_position_for_route(struct Thing *thing, struct Coord3d *finalpos, int32_t speed, struct Coord3d *nextpos, AriadneRouteFlags flags)
{
    struct CreatureControl *cctrl;
    struct Ariadne *arid;
    AriadneReturn result;
    AriadneReturn aret;
    NAVIDBG(18,"Route for %s index %d from %3d,%3d to %3d,%3d", thing_model_name(thing),(int)thing->index,
        (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)finalpos->x.stl.num, (int)finalpos->y.stl.num);
    cctrl = creature_control_get_from_thing(thing);
    arid = &cctrl->arid;
    arid->wallhug_active = WallhugActive_Off;
    if ((finalpos->x.val != arid->endpos.x.val)
     || (finalpos->y.val != arid->endpos.y.val)
     || (arid->move_speed != speed))
    {
        aret = ariadne_initialise_creature_route(thing, finalpos, speed, flags);
        if (aret != AridRet_OK) {
            return AridRet_Failed;
        }
        arid->move_speed = speed;
        if (arid->wallhug_active)
        {
            nextpos->x.val = arid->next_position.x.val;
            nextpos->y.val = arid->next_position.y.val;
            nextpos->z.val = arid->next_position.z.val;
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
        nextpos->x.val = arid->next_position.x.val;
        nextpos->y.val = arid->next_position.y.val;
        nextpos->z.val = arid->next_position.z.val;
        break;
    case AridUpSt_Wallhug:
        result = ariadne_update_state_wallhug(thing, arid);
        nextpos->x.val = arid->next_position.x.val;
        nextpos->y.val = arid->next_position.y.val;
        nextpos->z.val = arid->next_position.z.val;
        break;
    case AridUpSt_Manoeuvre:
        result = ariadne_update_state_manoeuvre_to_position(thing, arid);
        nextpos->x.val = arid->next_position.x.val;
        nextpos->y.val = arid->next_position.y.val;
        nextpos->z.val = arid->next_position.z.val;
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
AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *finalpos, struct Coord3d *nextpos, int32_t speed, AriadneRouteFlags flags)
{
    SYNCDBG(18,"Starting");
    if (game.map_changed_for_navigation)
    {
        struct CreatureControl *cctrl;
        cctrl = creature_control_get_from_thing(thing);
        cctrl->arid.may_need_reroute = 1;
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
void path_init8_wide_f(struct Path *path, int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y,
    int32_t subroute, unsigned char nav_size, const char *func_name)
{
    int32_t route_dist;
    NAVIDBG(9,"%s: Path from %5d,%5d to %5d,%5d on turn %u", func_name, start_x, start_y, end_x, end_y, get_gameturn());
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
        ERRORLOG("%s: Boundary triangle not found: %d -> %d.", func_name,tree_triA,tree_triB);
        return;
    }
    NAVIDBG(19,"%s: prepared triangles %d -> %d", func_name,tree_triA,tree_triB);
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
        if ((creature_radius < CreatureRadius_Small) || (creature_radius > CreatureRadius_Large))
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
        NAVIDBG(9,"%s: Finished with %3d waypoints, start: (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d)",
            func_name,(int32_t)path->waypoints_num,
            (int)path->waypoints[0].x,(int)path->waypoints[0].y,
            (int)path->waypoints[1].x,(int)path->waypoints[1].y,
            (int)path->waypoints[2].x,(int)path->waypoints[2].y,
            (int)path->waypoints[3].x,(int)path->waypoints[3].y,
            (int)path->waypoints[4].x,(int)path->waypoints[4].y,
            (int)path->waypoints[5].x,(int)path->waypoints[5].y,
            (int)path->waypoints[6].x,(int)path->waypoints[6].y,
            (int)path->waypoints[7].x,(int)path->waypoints[7].y,
            (int)path->waypoints[8].x,(int)path->waypoints[8].y);
    } else {
        NAVIDBG(9,"%s: Finished with %3d waypoints", func_name,(int32_t)path->waypoints_num);
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
