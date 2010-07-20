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

#define PATH_HEAP_LEN 258
#define TREEITEMS_COUNT 9000
#define TREEVALS_COUNT 9001

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif


#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
TbBool naviheap_empty(void);
void naviheap_init(void);
TbBool naviheap_add(long heapid);
long naviheap_remove(void);
long naviheap_top(void);

void tags_init(void);
long update_border_tags(long tag_id, long *border_pt, long border_len);
long border_tags_to_current(long *border_pt, long border_len);
TbBool is_current_tag(long tag_id);

TbBool navitree_add(long itm_pos, long itm_dat, long mvcost);
long copy_tree_to_route(long tag_start_id, long tag_end_id, long *route_pts, long route_len);
long tree_to_route(long tag_start_id, long tag_end_id, long *route_pts);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
