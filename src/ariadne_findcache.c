/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne_findcache.c
 *     FindCache support functions for Ariadne pathfinding.
 * @par Purpose:
 *     Functions to maintain and use Find Cache.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 05 Aug 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "ariadne_findcache.h"

#include "globals.h"
#include "bflib_basics.h"

#include "ariadne_tringls.h"
#include "ariadne_points.h"
#include "ariadne.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_find_cache[4][4];
#define find_cache _DK_find_cache

/******************************************************************************/
long triangle_brute_find8_near(long pos_x, long pos_y)
{
    long cx = pos_x >> 14;
    if (cx < 0)
        cx = 0;
    if (cx > 3)
        cx = 3;
    long cy = pos_y >> 14;
    if (cy < 0)
        cy = 0;
    if (cy > 3)
        cy = 3;
   // Try sibling
    long tri_id;
    if (cx-1 >= 0)
    {
        tri_id = find_cache[cy][cx-1];
        if (get_triangle_tree_alt(tri_id) != -1)
            return tri_id;
    }
    if (cx+1 < 4)
    {
        tri_id = find_cache[cy][cx+1];
        if (get_triangle_tree_alt(tri_id) != -1)
            return tri_id;
    }
    if (cy-1 >= 0)
    {
        tri_id = find_cache[cy-1][cx];
        if (get_triangle_tree_alt(tri_id) != -1)
            return tri_id;
    }
    if (cy+1 < 4)
    {
        tri_id = find_cache[cy+1][cx];
        if (get_triangle_tree_alt(tri_id) != -1)
            return tri_id;
    }
    // Try any in cache
    for (cy=0; cy < 4; cy++)
    {
        for (cx=0; cx < 4; cx++)
        {
            tri_id = find_cache[cy][cx];
            if (get_triangle_tree_alt(tri_id) != -1)
                return tri_id;
        }
    }
    // Try any
    tri_id = triangle_find_first_used();
    return tri_id;
}

long triangle_find_cache_get(long pos_x, long pos_y)
{
    long cache_x = (pos_x >> 14);
    if (cache_x > 3)
        cache_x = 3;
    if (cache_x < 0)
        cache_x = 0;
    long cache_y = (pos_y >> 14);
    if (cache_y > 3)
        cache_y = 3;
    if (cache_y < 0)
        cache_y = 0;

    long ntri = find_cache[cache_y][cache_x];
    if (get_triangle_tree_alt(ntri) == -1)
    {
        ntri = triangle_brute_find8_near(pos_x, pos_y);
        if ((ntri < 0) || (ntri > ix_Triangles))
        {
            ERRORLOG("triangles count overflow");
            ntri = -1;
        }
        find_cache[cache_y][cache_x] = ntri;
  }
  return ntri;

}

void triangle_find_cache_put(long pos_x, long pos_y, long ntri)
{
    long cache_x = (pos_x >> 14);
    if (cache_x > 3)
        cache_x = 3;
    if (cache_x < 0)
        cache_x = 0;
    long cache_y = (pos_y >> 14);
    if (cache_y > 3)
        cache_y = 3;
    if (cache_y < 0)
        cache_y = 0;
    find_cache[cache_y][cache_x] = ntri;
}

void triangulation_init_cache(long tri_idx)
{
    for (long i = 0; i < 4; i++)
    {
        find_cache[i][0] = tri_idx;
        find_cache[i][1] = tri_idx;
        find_cache[i][2] = tri_idx;
        find_cache[i][3] = tri_idx;
    }
}

long triangle_find8(long pt_x, long pt_y)
{
    NAVIDBG(19,"Starting");
    //TODO PATHFINDING triangulate_area sub-sub-sub-function
    long ntri = triangle_find_cache_get(pt_x, pt_y);
    for (unsigned long k = 0; k < TRIANLGLES_COUNT; k++)
    {
        int eqA = triangle_divide_areas_s8differ(ntri, 0, 1, pt_x, pt_y) > 0;
        int eqB = triangle_divide_areas_s8differ(ntri, 1, 2, pt_x, pt_y) > 0;
        int eqC = triangle_divide_areas_s8differ(ntri, 2, 0, pt_x, pt_y) > 0;

        long ncor = 0;
        long nxcor = 0; // Used only to verify if pointed_at8() didn't failed
        switch ((eqC << 2) + (eqB << 1) + eqA)
        {
        case 1:
            ntri = Triangles[ntri].tags[0];
            nxcor = 0;
            break;
        case 2:
            ntri = Triangles[ntri].tags[1];
            nxcor = 0;
            break;
        case 3:
            ncor = 1;
            nxcor = pointed_at8(pt_x, pt_y, &ntri, &ncor);
            break;
        case 4:
            ntri = Triangles[ntri].tags[2];
            nxcor = 0;
            break;
        case 5:
            ncor = 0;
            nxcor = pointed_at8(pt_x, pt_y, &ntri, &ncor);
            break;
        case 6:
        case 7:
            ncor = 2;
            nxcor = pointed_at8(pt_x, pt_y, &ntri, &ncor);
            break;
        case 0:
            triangle_find_cache_put(pt_x, pt_y, ntri);
            return ntri;
      }
      if (nxcor < 0) {
          ERRORLOG("No position pointed at %d,%d",(int)pt_x, (int)pt_y);
          return -1;
      }
    }
    ERRORLOG("Infinite loop detected");
    return -1;
}

/**
 * Finds given point in list of triangles. Gives triangle index and cor number in triangle.
 * @param pt_x
 * @param pt_y
 * @param out_tri_idx
 * @param out_cor_idx
 * @return
 */
TbBool point_find(long pt_x, long pt_y, long *out_tri_idx, long *out_cor_idx)
{
    long tri_idx = triangle_find8(pt_x << 8, pt_y << 8);
    if (tri_idx < 0)
    {
        return false;
    }
    for (long cor_id = 0; cor_id < 3; cor_id++)
    {
        struct Point* pt = get_triangle_point(tri_idx, cor_id);
        if ((pt->x == pt_x) && (pt->y == pt_y))
        {
          *out_tri_idx = tri_idx;
          *out_cor_idx = cor_id;
          return true;
        }
    }
    return false;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
