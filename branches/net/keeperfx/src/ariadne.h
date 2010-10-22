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
#define PATH_HEAP_LEN 258
#define TREE_ROUTE_LEN 3000
#define TRIANLGLES_COUNT 9000
#define BORDER_LENGTH 100
#define REGIONS_COUNT 300
#define POINTS_COUNT 4500
#define ROUTE_LENGTH 12000

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

typedef unsigned char AriadneReturn;

enum AriadneReturnValues {
    AridRet_OK    = 0,
    AridRet_Val1,
    AridRet_Val2,
    AridRet_Val3,
};

struct Ariadne { // sizeof = 102
    struct Coord3d startpos;
    struct Coord3d endpos;
  struct Coord3d pos_C;
  struct Coord3d pos_12;
  unsigned char field_18[6];
  unsigned char field_1E;
  unsigned char field_1F[2];
  unsigned char field_21;
  unsigned char field_22;
  unsigned char field_23;
  unsigned char field_24[2];
  unsigned short field_26;
    unsigned char current_waypoint;
    struct Coord2d waypoints[10];
    unsigned char stored_waypoints; // offs = 0x51
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

struct Triangle { // sizeof = 16
  short field_0[3];
  short field_6[3];
  unsigned char field_C;
  unsigned char field_D;
  unsigned short field_E;
};

struct Point { // sizeof = 18
  short x;
  short y;
};

struct Pathway { // sizeof = 7192
  unsigned char field_0[7188];
  unsigned long field_1C14;
};

struct RegionT { // sizeof = 3
  unsigned short field_0;
  unsigned char field_2;
};

#ifdef __cplusplus
#pragma pack()
#endif
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
DLLIMPORT long _DK_tri_initialised;
#define tri_initialised _DK_tri_initialised
DLLIMPORT struct Triangle _DK_Triangles[TRIANLGLES_COUNT];
#define Triangles _DK_Triangles
DLLIMPORT long _DK_count_Triangles;
#define count_Triangles _DK_count_Triangles
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
//DLLIMPORT long _DK_ix_EdgePoints;
//#define ix_EdgePoints _DK_ix_EdgePoints
DLLIMPORT long _DK_fringe_y[256];
#define fringe_y _DK_fringe_y
DLLIMPORT long _DK_ix_Border;
#define ix_Border _DK_ix_Border
//DLLIMPORT long _DK_count_RegionQ;
//#define count_RegionQ _DK_count_RegionQ
DLLIMPORT long _DK_Border[BORDER_LENGTH];
#define Border _DK_Border
DLLIMPORT struct Point _DK_Points[POINTS_COUNT];
#define Points _DK_Points
DLLIMPORT struct RegionT _DK_Regions[REGIONS_COUNT];
#define Regions _DK_Regions
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
void path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size);
long get_navigation_colour(long stl_x, long stl_y);
void border_clip_horizontal(unsigned char *imap, long a1, long a2, long a3, long a4);
void border_clip_vertical(unsigned char *imap, long a1, long a2, long a3, long a4);
long link_find(long ntri, long val);
#define edge_lock(fin_x, fin_y, bgn_x, bgn_y) edge_lock_f(fin_x, fin_y, bgn_x, bgn_y, __func__)
TbBool edge_lock_f(long fin_x, long fin_y, long bgn_x, long bgn_y, const char *func_name);
#define region_set(ntri, nreg) region_set_f(ntri, nreg, __func__)
void region_set_f(long ntri, unsigned long nreg, const char *func_name);
void border_internal_points_delete(long a1, long a2, long a3, long a4);
void tri_set_rectangle(long a1, long a2, long a3, long a4, unsigned char a5);
long fringe_get_rectangle(long *a1, long *a2, long *a3, long *a4, unsigned char *a5);
long delaunay_seeded(long a1, long a2, long a3, long a4);
void border_unlock(long a1, long a2, long a3, long a4);
void triangulation_border_start(long *a1, long *a2);
void triangulate_area(unsigned char *imap, long sx, long sy, long ex, long ey);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
