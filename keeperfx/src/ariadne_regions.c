/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_regions.c
 *     Regions array for Ariande system support functions.
 * @par Purpose:
 *     Functions to manage list or Regions.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 22 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "ariadne_regions.h"

#include "globals.h"
#include "bflib_basics.h"
#include "ariadne_tringls.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned long _DK_regions_connected(long tree_reg1, long tree_reg1);
DLLIMPORT void _DK_region_alloc(unsigned long tree_reg);
/******************************************************************************/
DLLIMPORT struct RegionT _DK_Regions[REGIONS_COUNT];
#define Regions _DK_Regions
/******************************************************************************/
struct RegionT bad_region;
/******************************************************************************/
void region_alloc(unsigned long tree_reg)
{
    _DK_region_alloc(tree_reg); return;
}

struct RegionT *get_region(long reg_id)
{
    if ((reg_id < 0) || (reg_id >= REGIONS_COUNT))
        return INVALID_REGION;
    return &Regions[reg_id];
}

TbBool regions_connected(long tree_reg1, long tree_reg2)
{
    long reg_id1,reg_id2;
    //return _DK_regions_connected(tree_reg1, tree_reg2);
    if ((tree_reg1 < 0) || (tree_reg1 >= TRIANLGLES_COUNT))
        return false;
    if ((tree_reg2 < 0) || (tree_reg2 >= TRIANLGLES_COUNT))
        return false;
    if (((Triangles[tree_reg1].field_C & 0x0F) == 0x0F)
    ||  ((Triangles[tree_reg2].field_C & 0x0F) == 0x0F))
        return false;
    reg_id1 = get_triangle_region_id(tree_reg1);
    reg_id2 = get_triangle_region_id(tree_reg2);
    if (Regions[reg_id1].field_2 == 1)
        return (reg_id2 == reg_id1);
    if (Regions[reg_id2].field_2 == 1)
        return (reg_id2 == reg_id1);
    region_alloc(tree_reg1);
    return ((Triangles[tree_reg2].field_E ^ Triangles[tree_reg1].field_E) & 0xFFC0) == 0;
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
    oreg = get_triangle_region_id(ntri);
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
        set_triangle_region_id(ntri, nreg);
        Regions[nreg].field_0++;
    }
}

void region_unlock(long ntri)
{
    unsigned long oreg;
    oreg = get_triangle_region_id(ntri);
    if (oreg < REGIONS_COUNT)
    {
        Regions[oreg].field_2 = 0;
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
