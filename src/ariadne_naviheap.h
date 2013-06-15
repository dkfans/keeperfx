/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_naviheap.h
 *     Header file for ariadne_naviheap.c.
 * @par Purpose:
 *     Navigation heap for Ariadne system support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2011 - 14 Jun 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_NAVIHEAP_H
#define DK_ARIADNE_NAVIHEAP_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PATH_HEAP_LEN 258
/******************************************************************************/
TbBool naviheap_empty(void);
void naviheap_init(void);

long naviheap_top(void);
long naviheap_get(long heapid);
long naviheap_remove(void);
TbBool naviheap_add(long heapid);

long naviheap_item_tree_val(long heapid);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
