/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_regions.h
 *     Header file for ariadne_regions.c.
 * @par Purpose:
 *     Regions array for Ariadne system support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 22 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ARIADNE_REGIONS_H
#define DK_ARIADNE_REGIONS_H

#include "bflib_basics.h"
#include "globals.h"

#define REGIONS_COUNT        300
#define REGION_QUEUE_LEN     200

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct RegionT { // sizeof = 3
  unsigned short num_triangles;
  unsigned char field_2;
};

#pragma pack()
/******************************************************************************/
extern struct RegionT bad_region;
#define INVALID_REGION &bad_region;
/******************************************************************************/
TbBool regions_connected(long tree_reg1, long tree_reg2);
void region_store_init(void);
long region_get(void);
void region_put(long nreg);
#define region_set(ntri, nreg) region_set_f(ntri, nreg, __func__)
void region_set_f(long ntri, unsigned long nreg, const char *func_name);
#define region_unset(ntri, nreg) region_unset_f(ntri, nreg, __func__)
void region_unset_f(long ntri, unsigned long nreg, const char *func_name);
void region_unlock(long ntri);
void triangulation_init_regions(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
