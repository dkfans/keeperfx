/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_regions.c
 *     Regions array for Ariadne system support functions.
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
DLLIMPORT unsigned long _DK_regions_connected(long tree_reg1, long tree_reg2);
DLLIMPORT void _DK_region_alloc(unsigned long tree_reg);
/******************************************************************************/
/** Array of regions.
 * Note that region[0] is used for storing unused triangles and shouldn't be
 * used for actual calculations.
 */
DLLIMPORT struct RegionT _DK_Regions[REGIONS_COUNT];
#define Regions _DK_Regions
/******************************************************************************/
struct RegionT bad_region;
/******************************************************************************/
/**
 * Allocates region ID.
 * @return The region ID, or 0 on failure.
 */
unsigned long region_alloc_id(void)
{
    struct RegionT * rgn;
    struct Triangle * tri;
    long reg_id,sreg_id;
    long i;
    int min_f0;

    NAVIDBG(19,"Starting");
    reg_id = -1;
    min_f0 = 2147483647;
    for (i=1; i < REGIONS_COUNT; i++)
    {
        rgn = &Regions[i];
        if (min_f0 > rgn->num_triangles)
        {
            if (rgn->num_triangles == 0) {
                return i;
            }
            if (rgn->field_2 == 0) {
                reg_id = i;
                min_f0 = rgn->num_triangles;
            }
        }
    }
    if (reg_id == -1) {
        ERRORLOG("region_alloc:overflow");
        return 0;
    }
    NAVIDBG(19,"removing triangles from region %ld",reg_id);
    for (i=0; i < ix_Triangles; i++)
    {
        tri = &Triangles[i];
        if (tri->field_D != -1)
        {
          sreg_id = (tri->field_E >> 6);
          if (sreg_id >= REGIONS_COUNT) {
              ERRORLOG("triangle %ld in outranged region",(long)i);
              continue;
          }
          if (sreg_id == reg_id)
          {
              if (sreg_id > 0) {
                  Regions[sreg_id].num_triangles--;
                  tri->field_E &= 0x3F;
                  Regions[sreg_id].field_2 = 0;
                  Regions[0].num_triangles++;
              }
              if (Regions[reg_id].num_triangles == 0)
                  break;
          }
        }
    }
    return reg_id;
}

void region_alloc(unsigned long tree_reg)
{
    /* Note that the beginning of this function is:
       reg_id = region_alloc_id();
    */
    _DK_region_alloc(tree_reg); return;
}

void triangulation_init_regions(void)
{
    memset(Regions, 0, REGIONS_COUNT*sizeof(struct RegionT));
}

struct RegionT *get_region(long reg_id)
{
    if ((reg_id < 0) || (reg_id >= REGIONS_COUNT))
        return INVALID_REGION;
    return &Regions[reg_id];
}

/**
 * Returns whether two regions represented by tree triangles are connected.
 * @param tree_reg1
 * @param tree_reg2
 * @return
 */
TbBool regions_connected(long tree_reg1, long tree_reg2)
{
    long reg_id1,reg_id2;
    unsigned long intersect;
    //return _DK_regions_connected(tree_reg1, tree_reg2);
    if ((tree_reg1 < 0) || (tree_reg1 >= TRIANLGLES_COUNT))
        return false;
    if ((tree_reg2 < 0) || (tree_reg2 >= TRIANLGLES_COUNT))
        return false;
    if (((get_triangle_tree_alt(tree_reg1) & 0x0F) == 0x0F)
    ||  ((get_triangle_tree_alt(tree_reg2) & 0x0F) == 0x0F))
        return false;
    reg_id1 = get_triangle_region_id(tree_reg1);
    reg_id2 = get_triangle_region_id(tree_reg2);
    if (Regions[reg_id1].field_2 == 1)
        return (reg_id2 == reg_id1);
    if (Regions[reg_id2].field_2 == 1)
        return (reg_id2 == reg_id1);
    region_alloc(tree_reg1);
    intersect = (Triangles[tree_reg2].field_E ^ Triangles[tree_reg1].field_E);
    return ((intersect & 0xFFC0) == 0);
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
            Regions[oreg].num_triangles--;
            Regions[oreg].field_2 = 0;
        }
        // And add to new one
        set_triangle_region_id(ntri, nreg);
        Regions[nreg].num_triangles++;
    }
}

void region_unset_f(long ntri, unsigned long nreg, const char *func_name)
{
    Regions[nreg].num_triangles--;
    Regions[nreg].field_2 = 0;
    set_triangle_region_id(ntri, 0);
    Regions[0].num_triangles++;
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
