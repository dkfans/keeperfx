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
#include "thing_navigate.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT AriadneReturn _DK_ariadne_initialise_creature_route(struct Thing *thing, struct Coord3d *pos, long a3, unsigned char a4);
DLLIMPORT AriadneReturn _DK_creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, long a4, unsigned char a5);
DLLIMPORT void _DK_path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size);
DLLIMPORT long _DK_route_to_path(long a1, long a2, long a3, long a4, long *a5, long a6, struct Path *path, long *a8);
DLLIMPORT void _DK_path_out_a_bit(struct Path *path, long *a2);
DLLIMPORT void _DK_gate_navigator_init8(struct Pathway *pway, long a2, long a3, long a4, long a5, long a6, unsigned char a7);
DLLIMPORT void _DK_route_through_gates(struct Pathway *pway, struct Path *path, long a3);
DLLIMPORT long _DK_triangle_findSE8(long, long);
DLLIMPORT long _DK_ma_triangle_route(long a1, long a2, long *a3);
DLLIMPORT void _DK_edgelen_init(void);
DLLIMPORT unsigned long _DK_regions_connected(long tree_reg1, long tree_reg1);
DLLIMPORT long _DK_get_navigation_colour(long stl_x, long stl_y);
DLLIMPORT void _DK_triangulate_area(unsigned char *imap, long sx, long sy, long ex, long ey);
DLLIMPORT long _DK_init_navigation(void);
DLLIMPORT long _DK_update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y);
DLLIMPORT void _DK_border_clip_horizontal(unsigned char *imap, long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_border_clip_vertical(unsigned char *imap, long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_edge_lock(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_border_internal_points_delete(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_tri_set_rectangle(long a1, long a2, long a3, long a4, unsigned char a5);
DLLIMPORT long _DK_fringe_get_rectangle(long *a1, long *a2, long *a3, long *a4, unsigned char *a5);
DLLIMPORT long _DK_delaunay_seeded(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_border_unlock(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_triangulation_border_start(long *a1, long *a2);
DLLIMPORT void _DK_triangulation_initxy(long a1, long a2, long a3, long a4);
DLLIMPORT long _DK_edge_find(long a1, long a2, long a3, long a4, long *a5, long *a6);
DLLIMPORT long _DK_make_3or4point(long *a1, long *a2);
DLLIMPORT long _DK_delete_4point(long a1, long a2);
DLLIMPORT long _DK_delete_3point(long a1, long a2);
DLLIMPORT long _DK_edge_rotateAC(long a1, long a2);
DLLIMPORT long _DK_triangle_route_do_fwd(long a1, long a2, long *a3, long *a4);
DLLIMPORT long _DK_triangle_route_do_bak(long a1, long a2, long *a3, long *a4);
DLLIMPORT void _DK_ariadne_pull_out_waypoint(struct Thing *thing, struct Ariadne *arid, long a3, struct Coord3d *pos);
DLLIMPORT long _DK_ariadne_init_movement_to_current_waypoint(struct Thing *thing, struct Ariadne *arid);

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

const long MOD3[] = {0, 1, 2, 0, 1, 2};
/******************************************************************************/
long route_to_path(long a1, long a2, long a3, long a4, long *a5, long a6, struct Path *path, long *a8);
void path_out_a_bit(struct Path *path, long *a2);
void gate_navigator_init8(struct Pathway *pway, long a2, long a3, long a4, long a5, long a6, unsigned char a7);
void route_through_gates(struct Pathway *pway, struct Path *path, long a3);
long triangle_findSE8(long a1, long a2);
long ma_triangle_route(long a1, long a2, long *a3);
void edgelen_init(void);
unsigned long regions_connected(long tree_reg1, long tree_reg1);
/******************************************************************************/

long init_navigation(void)
{
  return _DK_init_navigation();
}

long update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y)
{
    long sx,sy,ex,ey;
    long x,y;
    //return _DK_update_navigation_triangulation(start_x, start_y, end_x, end_y);
    if (!nav_map_initialised)
    {
      for (y = 0; y < 255; y ++)
      {
        for (x = 0; x < 255; x++)
        {
            set_navigation_map(x, y, get_navigation_colour(x, y));
        }
      }
      nav_map_initialised = 1;
    }
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
    if (ex >= 252)
      ex = 252;
    ey = end_y + 1;
    if (ey >= 252)
      ey = 252;
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

long route_to_path(long a1, long a2, long a3, long a4, long *a5, long a6, struct Path *path, long *a8)
{
    NAVIDBG(19,"Starting");
    return _DK_route_to_path(a1, a2, a3, a4, a5, a6, path, a8);
}

void path_out_a_bit(struct Path *path, long *a2)
{
    _DK_path_out_a_bit(path, a2);
}

void gate_navigator_init8(struct Pathway *pway, long a2, long a3, long a4, long a5, long a6, unsigned char a7)
{
    _DK_gate_navigator_init8(pway, a2, a3, a4, a5, a6, a7);
}

void route_through_gates(struct Pathway *pway, struct Path *path, long a3)
{
    _DK_route_through_gates(pway, path, a3);
}

long triangle_findSE8(long a1, long a2)
{
    return _DK_triangle_findSE8(a1, a2);
}

long triangle_route_do_fwd(long a1, long a2, long *a3, long *a4)
{
    //TODO: PATHFUNDING may hang if out of triangles; rework
/*    if (game.play_gameturn > 3200)
        NAVIDBG(9,"Starting");*/
    return _DK_triangle_route_do_fwd(a1, a2, a3, a4);
}

long triangle_route_do_bak(long a1, long a2, long *a3, long *a4)
{
    return _DK_triangle_route_do_bak(a1, a2, a3, a4);
}

long ma_triangle_route(long a1, long a2, long *a3)
{
    long len_fwd,len_bak;
    long par_fwd,par_bak;
    long tx,ty;
    long i;
    //return _DK_ma_triangle_route(a1, a2, a3);
    // Forward route
    NAVIDBG(19,"Making forward route");
    len_fwd = triangle_route_do_fwd(a1, a2, route_fwd, a3);
    if (len_fwd == -1)
    {
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
    len_bak = triangle_route_do_bak(a2, a1, route_bak, a3);
    if (len_bak == -1)
    {
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
    if (par_fwd < par_bak) //TODO PATHFINDING originally the condition was different - verify
    {
        for (i=0; i <= sizeof(tree_route)/sizeof(tree_route[0]); i++)
        {
             tree_route[i] = route_fwd[i];
        }
        return len_fwd;
    } else
    {
        for (i=0; i <= len_bak; i++)
        {
             tree_route[i] = route_bak[len_bak-i];
        }
        return len_bak;
    }
}

void edgelen_init(void)
{
    _DK_edgelen_init();
}

unsigned long regions_connected(long tree_reg1, long tree_reg2)
{
    return _DK_regions_connected(tree_reg1, tree_reg2);
}

TbBool ariadne_creature_reached_position(struct Thing *thing, struct Coord3d *pos)
{
    if (thing->mappos.x.val != pos->x.val)
        return false;
    if (thing->mappos.y.val != pos->y.val)
        return false;
    return true;
}

void ariadne_pull_out_waypoint(struct Thing *thing, struct Ariadne *arid, long a3, struct Coord3d *pos)
{
  _DK_ariadne_pull_out_waypoint(thing, arid, a3, pos);
}

void ariadne_init_current_waypoint(struct Thing *thing, struct Ariadne *arid)
{
    ariadne_pull_out_waypoint(thing, arid, arid->field_28, &arid->field_C);
    arid->field_C.z.val = get_thing_height_at(thing, &arid->field_C);
    arid->field_62 = get_2d_distance(&thing->mappos, &arid->field_C);
}

long ariadne_init_movement_to_current_waypoint(struct Thing *thing, struct Ariadne *arid)
{
    return _DK_ariadne_init_movement_to_current_waypoint(thing, arid);
}

AriadneReturn ariadne_prepare_creature_route_target_reached(struct Thing *thing, struct Ariadne *arid, struct Coord3d *srcpos, struct Coord3d *dstpos)
{
    arid->startpos.x.val = srcpos->x.val;
    arid->startpos.y.val = srcpos->y.val;
    arid->startpos.z.val = srcpos->z.val;
    arid->endpos.x.val = dstpos->x.val;
    arid->endpos.y.val = dstpos->y.val;
    arid->endpos.z.val = dstpos->z.val;
    arid->waypoints[0].x.val = srcpos->x.val;
    arid->waypoints[0].y.val = srcpos->y.val;
    arid->field_C.x.val = srcpos->x.val;
    arid->field_C.y.val = srcpos->y.val;
    arid->field_C.z.val = srcpos->z.val;
    arid->field_28 = 0;
    arid->field_12.x.val = thing->mappos.x.val;
    arid->field_12.y.val = thing->mappos.y.val;
    arid->field_12.z.val = thing->mappos.z.val;
    arid->stored_waypoints = 1;
    arid->total_waypoints = 1;
    arid->field_1E = 0;
    return 0;
}

AriadneReturn ariadne_prepare_creature_route_to_target(struct Thing *thing, struct Ariadne *arid, struct Coord3d *srcpos, struct Coord3d *dstpos, long a3, unsigned char a4)
{
    struct Path path;
    long nav_sizexy;
    long i,k;
    memset(&path, 0, sizeof(struct Path));
    arid->startpos.x.val = srcpos->x.val;
    arid->startpos.y.val = srcpos->y.val;
    arid->startpos.z.val = srcpos->z.val;
    arid->endpos.x.val = dstpos->x.val;
    arid->endpos.y.val = dstpos->y.val;
    arid->endpos.z.val = dstpos->z.val;
    nav_thing_can_travel_over_lava = creature_can_travel_over_lava(thing);
    if ( a4 )
        owner_player_navigating = -1;
    else
        owner_player_navigating = thing->owner;
    nav_sizexy = actual_sizexy_to_nav_block_sizexy_table[thing->field_56%(MAX_SIZEXY+1)]-1;
    path_init8_wide(&path,
        arid->startpos.x.val, arid->startpos.y.val,
        arid->endpos.x.val, arid->endpos.y.val, -2, nav_sizexy);
    nav_thing_can_travel_over_lava = 0;
    if (path.waypoints_num <= 0)
        return 2;
    if (path.waypoints_num <= 255)
        arid->total_waypoints = path.waypoints_num;
    else
        arid->total_waypoints = 255;
    if (arid->total_waypoints < 10)
        arid->stored_waypoints = arid->total_waypoints;
    else
        arid->stored_waypoints = 10;
    k = 0;
    for (i = 0; i < arid->stored_waypoints; i++)
    {
        arid->waypoints[i].x.val = path.waypoints[k].x;
        arid->waypoints[i].y.val = path.waypoints[k].y;
        k++;
    }
    arid->field_28 = 0;
    arid->field_1E = a4;
    arid->field_12.x.val = thing->mappos.x.val;
    arid->field_12.y.val = thing->mappos.y.val;
    arid->field_12.z.val = thing->mappos.z.val;
    arid->field_26 = a3;
    return 0;
}

AriadneReturn ariadne_initialise_creature_route(struct Thing *thing, struct Coord3d *pos, long a3, unsigned char a4)
{
    struct CreatureControl *cctrl;
    struct Ariadne *arid;
    AriadneReturn ret;
    SYNCDBG(18,"Starting");
    //return _DK_ariadne_initialise_creature_route(thing, pos, a3, a4);
    cctrl = creature_control_get_from_thing(thing);
    arid = &(cctrl->arid);
    memset(arid, 0, sizeof(struct Ariadne));
    if (ariadne_creature_reached_position(thing, pos))
    {
        ret = ariadne_prepare_creature_route_target_reached(thing, arid, &thing->mappos, pos);
        if (ret != AridRet_OK)
            return ret;
    } else
    {
        ret = ariadne_prepare_creature_route_to_target(thing, arid, &thing->mappos, pos, a3, a4);
        if (ret != AridRet_OK)
            return ret;
        ariadne_init_current_waypoint(thing, arid);
    }
    ariadne_init_movement_to_current_waypoint(thing, arid);
    return 0;
}

AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, long a4, unsigned char a5)
{
    SYNCDBG(18,"Starting");
    return _DK_creature_follow_route_to_using_gates(thing, pos1, pos2, a4, a5);
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
void path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size)
{
    int creature_radius;
    long route_dist;
    //TODO PATHFINDING hangs; probably if out of triangles
    //_DK_path_init8_wide(path, start_x, start_y, end_x, end_y, a6, nav_size);
    NAVIDBG(19,"F=%ld Path    %03ld,%03ld %03ld,%03ld", game.play_gameturn, start_x, start_y, end_x, end_y);
    if (a6 == -1)
      WARNLOG("implement random externally");
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
        ERRORLOG("triangle not found");
        return;
    }
    NAVIDBG(19,"prepared triangles %ld %ld",tree_triA,tree_triB);
    if (!regions_connected(tree_triA, tree_triB))
    {
        NAVIDBG(19,"regions not connected");
        return;
    }
    NAVIDBG(19,"regions connected");
    edgelen_init();
    creature_radius = nav_size + 1;
    if ((creature_radius < 1) || (creature_radius > 3))
    {
        ERRORLOG("nav.creature_radius_set : only radius 1..3 allowed, got %d",creature_radius);
        return;
    }
    EdgeFit = RadiusEdgeFit[creature_radius];
    tree_altA = Triangles[tree_triA].field_C;
    tree_altB = Triangles[tree_triB].field_C;
    if (a6 == -2)
    {
        tree_routelen = ma_triangle_route(tree_triA, tree_triB, &tree_routecost);
        NAVIDBG(19,"route=%d", tree_routelen);
        if (tree_routelen != -1)
        {
          path->waypoints_num = route_to_path(start_x, start_y, end_x, end_y, tree_route, tree_routelen, path, &route_dist);
          path_out_a_bit(path, tree_route);
          NAVIDBG(19,"way=%ld", path->waypoints_num);
        }
    } else
    {
      NAVIDBG(19,"gate");
      gate_navigator_init8(&ap_GPathway, start_x, start_y, end_x, end_y, 4096, nav_size);
      route_through_gates(&ap_GPathway, path, a6);
    }
    NAVIDBG(19,"Finished");
}

void triangulation_initxy(long a1, long a2, long a3, long a4)
{
    _DK_triangulation_initxy(a1, a2, a3, a4);
}

long edge_rotateAC(long a1, long a2)
{
    return _DK_edge_rotateAC(a1, a2);
}

long point_loop(long pt_tri, long pt_cor)
{
    long ntri,ncor;
    long i,k,n;
    ntri = pt_tri;
    ncor = pt_cor;
    if (ntri < 0)
        return -1;
    k = 0;
    do
    {
      n = Triangles[ntri].field_6[ncor];
      i = link_find(n, ntri);
      if (i < 0)
          return -1;
      ncor = MOD3[i+1];
      ntri = n;
      k++;
    } while (n != pt_tri);
    return k;
}

long reduce_point(long *pt_tri, long *pt_cor)
{
    long first_tri,ntri,ncor;
    long k,i,ctri;
    k = 0;
    ntri = *pt_tri;
    ncor = *pt_cor;
    first_tri = *pt_tri;
    do
    {
        ctri = Triangles[ntri].field_6[ncor];
        if (ctri < 0)
            return -1;
        if (!edge_rotateAC(ntri, ncor))
        {
            i = link_find(ctri, ntri);
            if (i < 0)
                return -1;
            ncor = MOD3[i+1];
            ntri = ctri;
            k++;
        }
    }
    while (ctri != first_tri);
    *pt_tri = ntri;
    *pt_cor = ncor;
    return k;
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

void border_clip_horizontal(unsigned char *imap, long a1, long a2, long a3, long a4)
{
    _DK_border_clip_horizontal(imap, a1, a2, a3, a4);
}

void border_clip_vertical(unsigned char *imap, long a1, long a2, long a3, long a4)
{
    _DK_border_clip_vertical(imap, a1, a2, a3, a4);
}

long edge_find(long a1, long a2, long a3, long a4, long *a5, long *a6)
{
    return _DK_edge_find(a1, a2, a3, a4, a5, a6);
}

long link_find(long ntri, long val)
{
    if (ntri < 0)
    {
        return -1;
    }
    if (Triangles[ntri].field_6[0] == val)
    {
        return 0;
    } else
    if (Triangles[ntri].field_6[1] == val)
    {
        return 1;
    } else
    if (Triangles[ntri].field_6[2] == val)
    {
        return 2;
    } else
    {
        return -1;
    }
}
TbBool edge_lock_f(long fin_x, long fin_y, long bgn_x, long bgn_y, const char *func_name)
{
    long tri_n,tri_k;
    long x,y;
    long i,k;
    //_DK_edge_lock(fin_x, fin_y, bgn_x, bgn_y); return true;
    x = bgn_x;
    y = bgn_y;
    while ((x != fin_x) || (y != fin_y))
    {
      if (!edge_find(x, y, fin_x, fin_y, &tri_n, &tri_k))
      {
        ERRORMSG("%s: edge not found",func_name);
        return false;
      }
      Triangles[tri_n].field_D |= 1 << (tri_k+3);
      k = MOD3[tri_k+1];
      i = Triangles[tri_n].field_0[k];
      x = Points[i].x;
      y = Points[i].y;
    }
    return true;
}

TbBool border_lock(long start_x, long start_y, long end_x, long end_y)
{
    TbBool r = true;
    r &= edge_lock(start_x, start_y, start_x, end_y);
    r &= edge_lock(start_x, end_y, end_x, end_y);
    r &= edge_lock(end_x, end_y, end_x, start_y);
    r &= edge_lock(end_x, start_y, start_x, start_y);
    return r;
}

TbBool outer_locked(long ntri, long ncor)
{
    long shft,n;
    n = Triangles[ntri].field_6[ncor];
    shft = link_find(n, ntri);
    if (shft < 0)
    {
        ERRORLOG("border edge");
        return true;
    }
    return ( (Triangles[n].field_D & (1 << (shft + 3)) ) != 0);
}

void region_set_f(long ntri, unsigned long nreg, const char *func_name)
{
    unsigned long oreg;
    if ((ntri < 0) || (ntri >= TRIANLGLES_COUNT) || (nreg >= REGIONS_COUNT))
    {
        ERRORMSG("%s: can't set triangle %ld region %lu",func_name,ntri,nreg);
        return;
    }
    // Get old region
    oreg = Triangles[ntri].field_E >> 6;
    // If the region changed
    if (oreg != nreg)
    {
        // Remove from old region
        if (oreg < REGIONS_COUNT)
        {
            Regions[oreg].field_0--;
            Regions[oreg].field_2 = 0;
        }
        // And add to new one
        Triangles[ntri].field_E &= 0x3F;
        Triangles[ntri].field_E |= (nreg << 6);
        Regions[nreg].field_0++;
    }
}

void border_internal_points_delete(long start_x, long start_y, long end_x, long end_y)
{
    long edge_tri,edge_cor;
    long ntri,ncor;
    unsigned long k;
    long i,n;
    //_DK_border_internal_points_delete(start_x, start_y, end_x, end_y);

    if (!edge_find(start_x, start_y, end_x, start_y, &edge_tri, &edge_cor))
    {
        ERRORLOG("Top not found");
        return;
    }

    ntri = Triangles[edge_tri].field_6[edge_cor];
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
        i = Triangles[ntri].field_0[MOD3[ncor+2]];
        if ((Points[i].x != start_x) && (Points[i].x != end_x)
         && (Points[i].y != start_y) && (Points[i].y != end_y))
        {
            if (!delete_point(ntri, MOD3[ncor+2]))
                break;
            ntri = Triangles[edge_tri].field_6[edge_cor];
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
            n = Triangles[ntri].field_6[ncor];
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
        if (Triangles[ntri].field_6[ncor] == edge_tri)
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

long delaunay_seeded(long a1, long a2, long a3, long a4)
{
    return _DK_delaunay_seeded(a1, a2, a3, a4);
}

void border_unlock(long a1, long a2, long a3, long a4)
{
    _DK_border_unlock(a1, a2, a3, a4);
}

void triangulation_border_start(long *a1, long *a2)
{
    _DK_triangulation_border_start(a1, a2);
}

long get_navigation_colour(long stl_x, long stl_y)
{
    return _DK_get_navigation_colour(stl_x, stl_y);
}

long uniform_area_colour(unsigned char *imap, long start_x, long start_y, long end_x, long end_y)
{
    long uniform;
    long x,y;
    uniform = imap[get_subtile_number(start_x,start_y)];
    for (y = start_y; y < end_y; y++)
    {
        for (x = start_x; x < end_x; x++)
        {
            if (imap[get_subtile_number(x,y)] != uniform)
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
          i = Triangles[tri_a].field_6[tri_b];
          n = link_find(i, tri_a);
          if (n < 0)
              break;
          tri_b = MOD3[n+1];
          tri_a = i;
      }
    }
    while ((tri_a != border_a) || (tri_b != border_b));
}

void triangulate_area(unsigned char *imap, long start_x, long start_y, long end_x, long end_y)
{
    TbBool one_tile,not_whole_map;
    long colour;
    unsigned char ccolour;
    long rect_sx,rect_sy,rect_ex,rect_ey;
    long i;
    LastTriangulatedMap = imap;
    NAVIDBG(9,"F=%ld Area %03ld,%03ld %03ld,%03ld T=%04ld",game.play_gameturn,start_x,start_y,end_x,end_y,count_Triangles);
    //_DK_triangulate_area(imap, sx, sy, ex, ey); return;
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
         return;
    }
    // Prepare some basic logic information
    one_tile = ((end_x - start_x == 1) && (end_y - start_y == 1));
    not_whole_map = (start_x != 0) || (start_y != 0) || (end_x != 256) || (end_y != 256);
    // If coordinates are out of range, update the whole map area
    if ((start_x < 1) || (start_y < 1) || (end_x >= 255) || (end_y >= 255))
    {
        one_tile = 0;
        not_whole_map = 0;
        end_x = 256;
        start_x = 0;
        start_y = 0;
        end_y = 256;
    }
    if (!tri_initialised)
    {
        tri_initialised = 1;
        triangulation_initxy(-256, -256, 512, 512);
    }
    if ( not_whole_map )
    {
      border_clip_horizontal(imap, start_x, end_x, start_y, 0);
      border_clip_horizontal(imap, start_x, end_x, end_y, -1);
      border_clip_vertical(imap, start_x, -1, start_y, end_y);
      border_clip_vertical(imap, end_x, 0, start_y, end_y);
      border_lock(start_x, start_y, end_x, end_y);
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
        colour = imap[get_subtile_number(start_x,start_y)];
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
}
/******************************************************************************/
