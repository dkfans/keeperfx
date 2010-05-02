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
#define TRIANLGLES_COUNT 9000

typedef unsigned char AriadneReturn;

struct Path { // sizeof = 2068
    long field_0;
    long field_4;
    long field_8;
    long field_C;
    long field_10;
    unsigned char field_14[12];
    unsigned char field_20[1968];
    unsigned char field_7D0[48];
    unsigned char field_800[16];
    unsigned char field_810[4];
};

struct Triangle { // sizeof = 16
  unsigned short field_0;
  unsigned short field_2;
  unsigned short field_4;
  unsigned short field_6;
  unsigned char field_8;
  unsigned char field_9;
  unsigned short field_A;
  unsigned char field_C;
  unsigned char field_D;
  unsigned short field_E;
};

struct Point { // sizeof = 18
  long field_0;
  unsigned short field_4;
  unsigned short field_6;
  unsigned short field_8;
  unsigned short field_A;
  unsigned short field_C;
  unsigned short field_E;
  unsigned short field_10;
};

struct Pathway { // sizeof = 7192
  unsigned char field_0[7188];
  unsigned long field_1C14;
};

/******************************************************************************/
DLLIMPORT unsigned long *_DK_EdgeFit;
#define EdgeFit _DK_EdgeFit
DLLIMPORT unsigned long _DK_RadiusEdgeFit[4][64];
#define RadiusEdgeFit _DK_RadiusEdgeFit
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
DLLIMPORT struct Triangle _DK_Triangles[TRIANLGLES_COUNT];
#define Triangles _DK_Triangles
/******************************************************************************/
AriadneReturn ariadne_initialise_creature_route(struct Thing *thing, struct Coord3d *pos, long a3, unsigned char a4);
AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, long a4, unsigned char a5);
void path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
