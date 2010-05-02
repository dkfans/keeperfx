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
/******************************************************************************/
#ifdef __cplusplus
}
#endif
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

long route_to_path(long a1, long a2, long a3, long a4, long *a5, long a6, struct Path *path, long *a8)
{
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

long ma_triangle_route(long a1, long a2, long *a3)
{
    return _DK_ma_triangle_route(a1, a2, a3);
}

void edgelen_init(void)
{
    _DK_edgelen_init();
}

unsigned long regions_connected(long tree_reg1, long tree_reg2)
{
    return _DK_regions_connected(tree_reg1, tree_reg2);
}

AriadneReturn ariadne_initialise_creature_route(struct Thing *thing, struct Coord3d *pos, long a3, unsigned char a4)
{
    return _DK_ariadne_initialise_creature_route(thing, pos, a3, a4);
}

AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, long a4, unsigned char a5)
{
    SYNCDBG(18,"Starting");
    return _DK_creature_follow_route_to_using_gates(thing, pos1, pos2, a4, a5);
}

void path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size)
{
    int creature_radius;
    long route_dist;
    //TODO: hangs; probably if out of things/rooms
    //_DK_path_init8_wide(path, start_x, start_y, end_x, end_y, a6, nav_size);
    NAVIDBG(19,"F=%ld Path    %03ld,%03ld %03ld,%03ld", game.play_gameturn, start_x, start_y, end_x, end_y);
    if (a6 == -1)
      WARNLOG("implement random externally");
    path->field_0 = start_x;
    path->field_4 = start_y;
    path->field_8 = end_x;
    path->field_C = end_y;
    path->field_10 = 0;
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
          path->field_10 = route_to_path(start_x, start_y, end_x, end_y, tree_route, tree_routelen, path, &route_dist);
          path_out_a_bit(path, tree_route);
          NAVIDBG(19,"way=%ld", path->field_10);
        }
    } else
    {
      NAVIDBG(19,"gate");
      gate_navigator_init8(&ap_GPathway, start_x, start_y, end_x, end_y, 4096, nav_size);
      route_through_gates(&ap_GPathway, path, a6);
    }
    NAVIDBG(19,"Finished");
}

/******************************************************************************/
