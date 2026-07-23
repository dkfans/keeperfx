/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_navitree.h
 *     Header file for ariadne_navitree.c.
 * @par Purpose:
 *     Navigation Tree support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     07 Jun 2010 - 16 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_NAVITREE_H
#define DK_ARIADNE_NAVITREE_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TREEITEMS_COUNT 100000
#define TREEVALS_COUNT 100001
#define DELAUNAY_COUNT 1000

/******************************************************************************/
#pragma pack(1)

extern int32_t tree_val[TREEVALS_COUNT];

#pragma pack()
/******************************************************************************/
void tags_init(void);
int32_t update_border_tags(int32_t tag_id, int32_t *border_pt, int32_t border_len);
int32_t border_tags_to_current(int32_t *border_pt, int32_t border_len);
TbBool is_current_tag(int32_t tag_id);
void store_current_tag(int32_t tag_id);

TbBool navitree_add(int32_t itm_pos, int32_t itm_dat, int32_t mvcost);
int32_t copy_tree_to_route(int32_t tag_start_id, int32_t tag_end_id, int32_t *route_pts, int32_t route_len);

void delaunay_init(void);
TbBool delaunay_add(int32_t itm_pos);
int32_t delaunay_seeded(int32_t start_x, int32_t start_y, int32_t end_x, int32_t end_y, TbBool keep_edge);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
