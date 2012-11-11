/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne.h
 *     Header file for ariadne.c.
 * @par Purpose:
 *     Dungeon routing and path finding system.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Jan 2010 - 20 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_H
#define DK_ARIADNE_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define TREE_ROUTE_LEN 3000
#define BORDER_LENGTH 100
#define ROUTE_LENGTH 12000
#define ARID_WAYPOINTS_COUNT 10

/******************************************************************************/
#pragma pack(1)

struct Thing;

typedef unsigned char AriadneReturn;

enum AriadneReturnValues {
    AridRet_OK    = 0,
    AridRet_Val1,
    AridRet_Val2,
    AridRet_PartOK,
};

struct Ariadne { // sizeof = 102
    /** Position where the journey stated. */
    struct Coord3d startpos;
    /** Final position where we're heading. */
    struct Coord3d endpos;
    /** Position of the last reached waypoint. */
    struct Coord3d current_waypoint_pos;
  struct Coord3d pos_12;
  unsigned char field_18[6];
  unsigned char field_1E;
  unsigned char field_1F[2];
  unsigned char field_21;
  unsigned char field_22;
  unsigned char field_23;
  unsigned char field_24[2];
  unsigned short move_speed;
    /** Index of the current waypoint in list of nearest waypoints stored. */
    unsigned char current_waypoint;
    /** List of nearest waypoints in the way towards destination, stored in an array. */
    struct Coord2d waypoints[ARID_WAYPOINTS_COUNT];
    /** Amount of nearest waypoints stored in the array. */
    unsigned char stored_waypoints; // offs = 0x51
    /** Total amount of waypoints planned on the way towards endpos. */
    unsigned char total_waypoints;
  struct Coord3d pos_53;
  struct Coord3d pos_59;
  unsigned char manoeuvre_state;
  unsigned short field_60;
  unsigned long field_62;
};

struct PathWayPoint { // sizeof = 8
    long x;
    long y;
};

struct Path { // sizeof = 2068
    long field_0;
    long field_4;
    long field_8;
    long field_C;
    long waypoints_num;
    struct PathWayPoint waypoints[256];
};

struct PathPoint { // sizeof = 28
  long field_0;
  long field_4;
  long field_8;
  long field_C;
  long field_10;
  long field_14;
  long field_18;
};

struct Pathway { // sizeof = 7192
  long field_0;
  long field_4;
  long field_8;
  long field_C;
  struct PathPoint points[256];
  long points_num;
  long field_1C14;
};

struct WayPoints { // sizeof = 1040
  long field_0;
  long field_4;
  long field_8;
  long field_C;
  long field_10[256];
};

#pragma pack()
/******************************************************************************/
DLLIMPORT unsigned long *_DK_EdgeFit;
#define EdgeFit _DK_EdgeFit
DLLIMPORT struct Pathway _DK_ap_GPathway;
#define ap_GPathway _DK_ap_GPathway
DLLIMPORT long _DK_tree_routelen;
#define tree_routelen _DK_tree_routelen
DLLIMPORT long _DK_tree_route[TREE_ROUTE_LEN];
#define tree_route _DK_tree_route
DLLIMPORT long _DK_tree_routecost;
#define tree_routecost _DK_tree_routecost
DLLIMPORT long _DK_tree_triA;
#define tree_triA _DK_tree_triA
DLLIMPORT long _DK_tree_triB;
#define tree_triB _DK_tree_triB
DLLIMPORT long _DK_tree_altA;
#define tree_altA _DK_tree_altA
DLLIMPORT long _DK_tree_altB;
#define tree_altB _DK_tree_altB
DLLIMPORT long _DK_tree_Ax8;
#define tree_Ax8 _DK_tree_Ax8
DLLIMPORT long _DK_tree_Ay8;
#define tree_Ay8 _DK_tree_Ay8
DLLIMPORT long _DK_tree_Bx8;
#define tree_Bx8 _DK_tree_Bx8
DLLIMPORT long _DK_tree_By8;
#define tree_By8 _DK_tree_By8
DLLIMPORT unsigned char *_DK_LastTriangulatedMap;
#define LastTriangulatedMap _DK_LastTriangulatedMap
DLLIMPORT unsigned char *_DK_fringe_map;
#define fringe_map _DK_fringe_map
DLLIMPORT long _DK_fringe_x2;
#define fringe_x2 _DK_fringe_x2
DLLIMPORT long _DK_fringe_y1;
#define fringe_y1 _DK_fringe_y1
DLLIMPORT long _DK_fringe_x1;
#define fringe_x1 _DK_fringe_x1
DLLIMPORT long _DK_fringe_y2;
#define fringe_y2 _DK_fringe_y2
DLLIMPORT long _DK_fringe_y[256];
#define fringe_y _DK_fringe_y
DLLIMPORT long _DK_ix_Border;
#define ix_Border _DK_ix_Border
//DLLIMPORT long _DK_count_RegionQ;
//#define count_RegionQ _DK_count_RegionQ
DLLIMPORT long _DK_Border[BORDER_LENGTH];
#define Border _DK_Border
DLLIMPORT long _DK_route_fwd[ROUTE_LENGTH];
#define route_fwd _DK_route_fwd
DLLIMPORT long _DK_route_bak[ROUTE_LENGTH];
#define route_bak _DK_route_bak
DLLIMPORT struct Path _DK_fwd_path;
#define fwd_path _DK_fwd_path
DLLIMPORT struct Path _DK_bak_path;
#define bak_path _DK_bak_path

/******************************************************************************/
extern unsigned char const actual_sizexy_to_nav_block_sizexy_table[];
/******************************************************************************/
long init_navigation(void);
long update_navigation_triangulation(long start_x, long start_y, long end_x, long end_y);
AriadneReturn ariadne_initialise_creature_route(struct Thing *thing, struct Coord3d *pos, long a3, unsigned char a4);
AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, long a4, unsigned char a5);
long thing_nav_block_sizexy(const struct Thing *thing);
void path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size);
long get_navigation_colour(long stl_x, long stl_y);
TbBool border_clip_horizontal(const unsigned char *imap, long a1, long a2, long a3, long a4);
TbBool border_clip_vertical(const unsigned char *imap, long a1, long a2, long a3, long a4);
#define edge_lock(fin_x, fin_y, bgn_x, bgn_y) edge_lock_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
TbBool edge_lock_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name);
#define edge_unlock_record_and_regions(fin_x, fin_y, bgn_x, bgn_y) edge_unlock_record_and_regions_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
TbBool edge_unlock_record_and_regions_f(long ptend_x, long ptend_y, long ptstart_x, long ptstart_y, const char *func_name);
void border_internal_points_delete(long a1, long a2, long a3, long a4);
void tri_set_rectangle(long a1, long a2, long a3, long a4, unsigned char a5);
long fringe_get_rectangle(long *a1, long *a2, long *a3, long *a4, unsigned char *a5);
long delaunay_seeded(long a1, long a2, long a3, long a4);
void border_unlock(long a1, long a2, long a3, long a4);
TbBool triangulation_border_start(long *a1, long *a2);
TbBool triangulate_area(unsigned char *imap, long sx, long sy, long ex, long ey);
long pointed_at8(long pos_x, long pos_y, long *retpos_x, long *retpos_y);
long triangle_brute_find8_near(long pos_x, long pos_y);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
