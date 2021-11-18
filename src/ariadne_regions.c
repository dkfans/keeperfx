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
/******************************************************************************/
/** Array of regions.
 * Note that region[0] is used for storing unused triangles and shouldn't be
 * used for actual calculations.
 */
DLLIMPORT struct RegionT _DK_Regions[REGIONS_COUNT];
#define Regions _DK_Regions
DLLIMPORT long _DK_max_RegionStore;
#define max_RegionStore _DK_max_RegionStore
DLLIMPORT long _DK_ix_RegionQput;
#define ix_RegionQput _DK_ix_RegionQput
DLLIMPORT long _DK_ix_RegionQget;
#define ix_RegionQget _DK_ix_RegionQget
DLLIMPORT long _DK_count_RegionQ;
#define count_RegionQ _DK_count_RegionQ
DLLIMPORT long _DK_RegionQueue[REGION_QUEUE_LEN];
#define RegionQueue _DK_RegionQueue
/******************************************************************************/
struct RegionT bad_region;
/******************************************************************************/
/**
 * Allocates region ID.
 * @return The region ID, or 0 on failure.
 */
static unsigned long region_alloc(void)
{
    long i;

    NAVIDBG(19,"Starting");
    long reg_id = -1;
    int min_f0 = 2147483647;
    for (i=1; i < REGIONS_COUNT; i++)
    {
        struct RegionT* rgn = &Regions[i];
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
        ERRORLOG("regions overflow");
        return 0;
    }
    NAVIDBG(19,"removing triangles from region %ld",reg_id);
    for (i=0; i < ix_Triangles; i++)
    {
        long sreg_id = get_triangle_region_id(i);
        if (sreg_id >= REGIONS_COUNT)
        {
            ERRORLOG("triangle %ld in outranged region", (long)i);
            continue;
        }
        if (sreg_id == reg_id)
        {
            if (sreg_id > 0)
            {
                Regions[sreg_id].num_triangles--;
                set_triangle_region_id(i, 0);
                Regions[sreg_id].field_2 = 0;
                Regions[0].num_triangles++;
            }
            if (Regions[reg_id].num_triangles == 0)
                break;
        }      
    }
    return reg_id;
}

void region_lnk(int nreg)
{
    for (int ncor = 0; ncor < 3; ncor++)
    {
        int ctri_id = nreg;
        int ccor_id = ncor;
        unsigned long k = 0;
        TbBool notfound;
        while (1)
        {
            if ((Triangles[ctri_id].tree_alt & 0xF) == 15)
            {
                notfound = 1;
                break;
            }
            int ntri_id = Triangles[ctri_id].tags[ccor_id];
            if (ntri_id == -1)
            {
                notfound = 1;
                break;
            }
          ccor_id = link_find(ntri_id, ctri_id);
          if (ccor_id < 0) {
              ERRORLOG("no tri link");
              notfound = 1;
              break;
          }
          ccor_id = MOD3[ccor_id+1];
          ctri_id = ntri_id;
          if (nreg == ntri_id) {
              notfound = 0;
              break;
          }
          k++;
          if (k > TRIANLGLES_COUNT) {
              ERRORLOG("Infinite loop detected");
              notfound = 1;
              break;
          }
      }
      if (notfound) {
          continue;
      }
      ctri_id = nreg;
      ccor_id = ncor;
      while ( 1 )
      {
          set_triangle_edgelen(ctri_id, get_triangle_edgelen(ctri_id) | (3 << 2 * ccor_id));
          int ntri_id = Triangles[ctri_id].tags[ccor_id];
          if (ntri_id == -1)
              break;
          ccor_id = link_find(ntri_id, ctri_id);
          ctri_id = ntri_id;
          set_triangle_edgelen(ctri_id, get_triangle_edgelen(ctri_id) | (3 << 2 * ccor_id));
          ccor_id = MOD3[ccor_id+1];
          if (nreg == ntri_id) {
              break;
          }
      }
  }
}

static void region_connect(unsigned long tree_reg)
{
    //_DK_region_connect(tree_reg); return;
    long nreg_id = region_alloc();
    Regions[nreg_id].field_2 = 1;
    region_store_init();
    region_set(tree_reg, nreg_id);
    region_put(tree_reg);
    region_lnk(tree_reg);
    while ( 1 )
    {
        long creg_id = region_get();
        if (creg_id == -1)
          break;
        for (unsigned int ncor1 = 0; ncor1 < 3; ncor1++)
        {
            int ntri_id = Triangles[creg_id].tags[ncor1];
            if (ntri_id != -1)
            {
              if ((Triangles[ntri_id].tree_alt & 0xF) != 15)
              {
                  long preg_id = get_triangle_region_id(ntri_id);
                  if (preg_id != nreg_id)
                  {
                      region_lnk(creg_id);
                      region_set(ntri_id, nreg_id);
                      region_put(ntri_id);
                  }
              }
            }
        }
    }
}
HOOK_DK_FUNC(region_connect)

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
    if ((tree_reg1 < 0) || (tree_reg1 >= TRIANLGLES_COUNT))
        return false;
    if ((tree_reg2 < 0) || (tree_reg2 >= TRIANLGLES_COUNT))
        return false;
    if (((get_triangle_tree_alt(tree_reg1) & 0x0F) == 0x0F)
    ||  ((get_triangle_tree_alt(tree_reg2) & 0x0F) == 0x0F))
        return false;
    long reg_id1 = get_triangle_region_id(tree_reg1);
    long reg_id2 = get_triangle_region_id(tree_reg2);
    if (Regions[reg_id1].field_2 == 1)
        return (reg_id2 == reg_id1);
    if (Regions[reg_id2].field_2 == 1)
        return (reg_id2 == reg_id1);
    region_connect(tree_reg1);
    // Fast version of comparing region id values
    unsigned long intersect = (Triangles[tree_reg2].field_E ^ Triangles[tree_reg1].field_E);
    return ((intersect & 0xFFC0) == 0);
}

void region_store_init(void)
{
    ix_RegionQput = 0;
    ix_RegionQget = 0;
    count_RegionQ = 0;
}

long region_get(void)
{
    long qget = ix_RegionQget;
    count_RegionQ--;
    long regn;
    if (ix_RegionQget != ix_RegionQput)
    {
        qget = ix_RegionQget + 1;
        if (qget >= REGION_QUEUE_LEN)
            qget = 0;
        regn = RegionQueue[ix_RegionQget];
    } else
    {
        regn = -1;
    }
    ix_RegionQget = qget;
    return regn;
}

void region_put(long nreg)
{
    long qpos = ix_RegionQput;
    ix_RegionQput++;
    if (ix_RegionQput >= REGION_QUEUE_LEN) {
        ix_RegionQput = 0;
    }
    if (ix_RegionQput == ix_RegionQget) {
        ERRORLOG("Q overflow");
    }
    RegionQueue[qpos] = nreg;
    count_RegionQ++;
    if (max_RegionStore < count_RegionQ) {
        max_RegionStore = count_RegionQ;
    }
}

void region_set_f(long ntri, unsigned long nreg, const char *func_name)
{
    if ((ntri < 0) || (ntri >= TRIANLGLES_COUNT) || (nreg >= REGIONS_COUNT))
    {
        ERRORMSG("%s: can't set triangle %ld region %lu",func_name,ntri,nreg);
        return;
    }
    // Get old region
    unsigned long oreg = get_triangle_region_id(ntri);
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
    unsigned long oreg = get_triangle_region_id(ntri);
    if (oreg < REGIONS_COUNT)
    {
        Regions[oreg].field_2 = 0;
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
