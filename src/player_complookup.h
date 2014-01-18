/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_complookup.h
 *     Header file for player_complookup.c.
 * @par Purpose:
 *     Computer player lookups definitions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PLYR_COMPLOOKUP_H
#define DK_PLYR_COMPLOOKUP_H

#include "bflib_basics.h"
#include "globals.h"

#define GOLD_LOOKUP_COUNT      40

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
#pragma pack(1)

struct GoldLookup { // sizeof = 28
    unsigned char flags;
    /* Informs whether players are interested in that gold vein. */
    unsigned char player_interested[5];
unsigned short x_stl_num;
unsigned short y_stl_num;
unsigned short field_A;
unsigned short field_C;
unsigned short num_gold_slabs;
unsigned long num_gem_slabs;
unsigned char field_14[6];
unsigned short field_1A;
};

#pragma pack()
/******************************************************************************/
void check_map_for_gold(void);
struct GoldLookup *get_gold_lookup(long idx);
long gold_lookup_index(const struct GoldLookup *gldlook);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
