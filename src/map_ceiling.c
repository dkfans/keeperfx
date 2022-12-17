/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_ceiling.c
 *     Map ceiling support functions.
 * @par Purpose:
 *     Functions to calculate the ceiling.
 * @par Comment:
 *     None.
 * @author   keeperFx Team
 * @date     12 Nov 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "pre_inc.h"
#include "globals.h"
#include "map_columns.h"
#include "front_simple.h"
#include "globals.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C"
{
#endif

static char ceiling_cache[MAX_SUBTILES_X*MAX_SUBTILES_Y];

static int find_column_height_including_lintels(struct Column *col)
{
    unsigned char i;
    if (!col->solidmask)
        return 0;
    for (i = 7; !col->cubes[i]; --i)
        ;
    return i + 1;
}

static int ceiling_block_is_solid_including_corners_return_height(SubtlCodedCoords stl_num, int cstl_x, int cstl_y)
{
    struct Column *col;
    unsigned char v6;
    int v8;
    unsigned char v10;
    int v11;
    unsigned char v13;
    int v14;
    unsigned char v16;
    int v17;
    unsigned char bitfields;
    if ((game.map[stl_num].flags & SlbAtFlg_Blocking) == 0 && (game.columns.lookup[game.map[stl_num].data & 0x7FF]->bitfields & CLF_CEILING_MASK) == 0)
    {
        if (cstl_x <= 0)
        {
            if (cstl_y > 0)
            {
                v17 = stl_num - 256;
                if ((game.map[v17].flags & SlbAtFlg_Blocking) != 0 || (game.columns.lookup[game.map[v17].data & 0x7FF]->bitfields & CLF_CEILING_MASK) != 0)
                {
                    col = game.columns.lookup[game.map[stl_num - 256].data & 0x7FF];
                    bitfields = col->bitfields;
                    if ((bitfields & CLF_CEILING_MASK) != 0)
                        return find_column_height_including_lintels(col);
                    return bitfields >> 4;
                }
            }
        }
        else
        {
            v8 = stl_num - 1;
            if ((game.map[v8].flags & SlbAtFlg_Blocking) != 0 || (game.columns.lookup[game.map[v8].data & 0x7FF]->bitfields & CLF_CEILING_MASK) != 0)
            {
                col = game.columns.lookup[game.map[stl_num - 1].data & 0x7FF];
                v10 = col->bitfields;
                if ((v10 & CLF_CEILING_MASK) != 0)
                    return find_column_height_including_lintels(col);
                return v10 >> 4;
            }
            if (cstl_y > 0)
            {
                v11 = stl_num - 256;
                if ((game.map[v11].flags & SlbAtFlg_Blocking) != 0 || (game.columns.lookup[game.map[v11].data & 0x7FF]->bitfields & CLF_CEILING_MASK) != 0)
                {
                    col = game.columns.lookup[game.map[stl_num - 256].data & 0x7FF];
                    v13 = col->bitfields;
                    if ((v13 & CLF_CEILING_MASK) != 0)
                        return find_column_height_including_lintels(col);
                    return v13 >> 4;
                }
                v14 = stl_num - 257;
                if ((game.map[v14].flags & SlbAtFlg_Blocking) != 0 || (game.columns.lookup[game.map[v14].data & 0x7FF]->bitfields & CLF_CEILING_MASK) != 0)
                {
                    col = game.columns.lookup[game.map[stl_num - 257].data & 0x7FF];
                    v16 = col->bitfields;
                    if ((v16 & CLF_CEILING_MASK) != 0)
                        return find_column_height_including_lintels(col);
                    return v16 >> 4;
                }
            }
        }
        return -1;
    }
    col = game.columns.lookup[game.map[stl_num].data & 0x7FF];
    v6 = col->bitfields;
    if ((v6 & CLF_CEILING_MASK) != 0)
        return find_column_height_including_lintels(col);
    return v6 >> 4;
}

static int ceiling_calculate_height_from_nearest_walls(int result, int number_of_steps)
{
    int v2;
    v2 = game.ceiling_step * number_of_steps;
    if (result >= game.ceiling_height_max)
    {
        if (result > game.ceiling_height_max)
        {
            result -= v2;
            if (result <= game.ceiling_height_min)
                return game.ceiling_height_min;
        }
    }
    else
    {
        result += v2;
        if (result >= game.ceiling_height_max)
            return game.ceiling_height_max;
    }
    return result;
}


//todo cleanup this function
//also ceiling_cache used to use scratch but that caused a crash somehow so now it just wastes 65kb for no reason
long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey)
{
    int v5;
    int v6;
    int v7;
    int v8;
    int v9;
    int v10;
    int v11;
    int v12;
    int v13;
    int v14;
    int v15;
    int v16;
    int cstl_y;
    int cstl_x;
    int stl_num;
    signed char is_solid;
    unsigned long *p_data;
    int v22;
    int v23;
    struct MapOffset *spir;
    MapSubtlCoord unk2_stl_x;
    MapSubtlCoord unk2_stl_y;
    int v27;
    int v28;
    int v29;
    int v30;
    int v31;
    int v32;
    char v33;
    int v34;
    int v35;
    unsigned int number_of_steps;
    int v38;
    int v39;
    int v40;
    int v41;
    int v42;
    int v43;
    int v44;
    int v45;
    int v46;
    int v47;
    int *v48;
    int v49;
    unsigned int v50;
    int v51;
    int v52;
    MapSubtlCoord unk_stl_y;
    MapSubtlCoord unk_stl_x;
    v5 = game.ceiling_dist;
    if (game.ceiling_dist > 4)
        v5 = 4;
    v6 = sx - v5;
    v7 = v5;
    v8 = v6;
    if (v6 <= 0)
        v8 = 0;
    v45 = v8;
    v9 = sy - v7;
    if (sy - v7 <= 0)
        v9 = 0;
    v40 = v9;
    v10 = ex + v7;
    if (ex + v7 >= 256)
        v10 = 256;
    v11 = ey + v7;
    v47 = v10;
    if (v11 >= 256)
        v11 = 256;
    v39 = v11;
    v12 = v11;
    //ceiling_cache = (signed char*)scratch;
    v13 = v45 - game.ceiling_dist;
    if (v45 - game.ceiling_dist <= 0)
        v13 = 0;
    v44 = v13;
    v14 = v40 - game.ceiling_dist;
    if (v40 - game.ceiling_dist <= 0)
        v14 = 0;
    v15 = game.ceiling_dist + v47;
    if (game.ceiling_dist + v47 >= 256)
        v15 = 256;
    v16 = v12 + game.ceiling_dist;
    if (v12 + game.ceiling_dist >= 256)
        v16 = 256;
    v41 = v16;
    cstl_y = v14;
    
    if (v14 < v16)
    {
        v43 = v14 << 8;
        do
        {
            cstl_x = v44;

            stl_num = v44 + v43;
            while (cstl_x < v15)
            {
                is_solid = ceiling_block_is_solid_including_corners_return_height(stl_num,cstl_x,cstl_y);
                stl_num++;
                cstl_x++;
                ceiling_cache[stl_num - 1] = is_solid;

            }
            ++cstl_y;
            v43 += 256;
        } while (cstl_y < v41);
    }

    if (v40 < v39)
    {
        v46 = v40 << 8;
        v42 = v39 << 8;
        do
        {
            v49 = v45;
            v51 = v46 + v45;
            if (v47 > v45)
            {
                v50 = 5 * v51;
                do
                {
                    p_data = &game.map[v50 / 5].data;
                    v22 = ceiling_cache[v51];
                    v38 = v22;
                    if (v22 <= -1)
                    {
                        v52 = v51;
                        v48 = &v38;
                        unk_stl_x = v51 % 256;
                        unk_stl_y = v51 / 256;
                        v23 = 0;
                        spir = spiral_step;
                        if (game.ceiling_search_dist > 0)
                        {
                            while (1)
                            {
                                unk2_stl_x = unk_stl_x + spir->h;
                                unk2_stl_y = unk_stl_y + spir->v;
                                if (unk2_stl_x >= 0 && unk2_stl_x < map_subtiles_x && unk2_stl_y >= 0 && unk2_stl_y < map_subtiles_y)
                                {
                                    v27 = ceiling_cache[v52 + (*(long *)spir >> 16)];
                                    if (v27 > -1)
                                        break;
                                }
                                ++v23;
                                ++spir;
                                if (v23 >= game.ceiling_search_dist)
                                    goto LABEL_43;
                            }
                            *v48 = v27;
                            v28 = unk_stl_x - unk2_stl_x;
                            v29 = unk_stl_y - unk2_stl_y;
                            if ((int)abs(v28) <= (int)abs(v29))
                                v30 = v29;
                            else
                                v30 = v28;
                            number_of_steps = abs(v30);
                            v31 = 1;
                        }
                        else
                        {
                        LABEL_43:
                            v31 = 0;
                        }
                        if (v31)
                            v22 = ceiling_calculate_height_from_nearest_walls(v38, number_of_steps);
                        else
                            v22 = game.ceiling_height_max;
                    }
                    v32 = v47;
                    v33 = *((char *)p_data + 3) & CLF_FLOOR_MASK;
                    v50 += 5;
                    ++v51;
                    *((char *)p_data + 3) = v33;
                    v35 = ((v22 & 0xF) << 24) | *p_data;
                    v34 = ++v49;
                    *p_data = v35;
                } while (v34 < v32);
            }
            v46 += 256;
        } while (v46 < v42);
    }
    return 1;
}

static long get_ceiling_filled_subtiles_from_cubes(const struct Column *col)
{
    if (col->solidmask == 0) {
        return 0;
    }
    int i;
    for (i = COLUMN_STACK_HEIGHT-1; i >= 0; i--)
    {
        if (col->cubes[i] != 0)
            break;
    }
    return i + 1;
}

static int get_ceiling_or_floor_filled_subtiles(SubtlCodedCoords stl_num)
{
    const struct Map *mapblk;
    mapblk = get_map_block_at_pos(stl_num);
    const struct Column *col;
    col = get_map_column(mapblk);
    if (get_map_ceiling_filled_subtiles(mapblk) > 0) {
        return get_ceiling_filled_subtiles_from_cubes(col);
    } else {
        return get_map_floor_filled_subtiles(mapblk);
    }
}

long ceiling_init(unsigned long a1, unsigned long a2)
{
    return _DK_ceiling_init(a1, a2);
    //TODO Fix, then enable rewritten version
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    for (stl_y=0; stl_y < map_subtiles_y; stl_y++)
    {
        for (stl_x=0; stl_x < map_subtiles_x; stl_x++)
        {
            int filled_h;
            if (map_pos_solid_at_ceiling(stl_x, stl_y))
            {
                filled_h = get_ceiling_or_floor_filled_subtiles(get_subtile_number(stl_x,stl_y));
            } else
            if (stl_x > 0 && map_pos_solid_at_ceiling(stl_x-1, stl_y))
            {
                filled_h = get_ceiling_or_floor_filled_subtiles(get_subtile_number(stl_x-1,stl_y));
            } else
            if (stl_y > 0 && map_pos_solid_at_ceiling(stl_x, stl_y-1))
            {
                filled_h = get_ceiling_or_floor_filled_subtiles(get_subtile_number(stl_x,stl_y-1));
            } else
            if (stl_x > 0 && stl_y > 0 && map_pos_solid_at_ceiling(stl_x-1, stl_y-1)) {
                filled_h = get_ceiling_or_floor_filled_subtiles(get_subtile_number(stl_x-1,stl_y-1));
            } else {
                filled_h = -1;
            }

            if (filled_h <= -1)
            {
              if (game.ceiling_search_dist <= 0)
              {
                  filled_h = game.ceiling_height_max;
              }
              else
              {
                int i;
                i = 0;
                while ( 1 )
                {
                    struct MapOffset *sstep;
                    sstep = &spiral_step[i];
                    MapSubtlCoord cstl_x;
                    MapSubtlCoord cstl_y;
                    cstl_x = stl_x + sstep->h;
                    cstl_y = stl_y + sstep->v;
                    if ((cstl_x >= 0) && (cstl_x <= map_subtiles_x))
                    {
                        if ((cstl_y >= 0) && (cstl_y <= map_subtiles_y))
                        {
                            filled_h = ceiling_block_is_solid_including_corners_return_height(sstep->both + get_subtile_number(stl_x,stl_y), cstl_x, cstl_y);
                            if (filled_h > -1)
                            {
                                int delta_tmp;
                                int delta_max;
                                delta_tmp = abs(stl_x - cstl_x);
                                delta_max = abs(stl_y - cstl_y);
                                if (delta_max <= delta_tmp)
                                    delta_max = delta_tmp;
                                if (filled_h < game.ceiling_height_max)
                                {
                                    filled_h += game.ceiling_step * delta_max;
                                    if (filled_h >= game.ceiling_height_max)
                                        filled_h = game.ceiling_height_max;
                                } else
                                if ( filled_h > game.ceiling_height_max )
                                {
                                    filled_h -= game.ceiling_step * delta_max;
                                    if (filled_h <= game.ceiling_height_min)
                                        filled_h = game.ceiling_height_min;
                                }
                                break;
                            }
                        }
                    }
                    ++i;
                    if (i >= game.ceiling_search_dist) {
                        filled_h = game.ceiling_height_max;
                        break;
                    }
                }
              }
            }
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x,stl_y);
            set_mapblk_filled_subtiles(mapblk, filled_h);
        }
    }
    return 1;
}

short ceiling_set_info(long height_max, long height_min, long step)
{
    SYNCDBG(6,"Starting");
    long dist;
    if (step <= 0)
    {
      ERRORLOG("Illegal ceiling step value");
      return 0;
    }
    if (height_max > 15)
    {
      ERRORLOG("Max height is too high");
      return 0;
    }
    if (height_min > height_max)
    {
      ERRORLOG("Ceiling max height is smaller than min height");
      return 0;
    }
    dist = (height_max - height_min) / step;
    if ( dist >= 2500 )
      dist = 2500;
    game.ceiling_dist = dist;
    if (dist > 20)
    {
      ERRORLOG("Ceiling search distance too big");
      return 0;
    }
    game.ceiling_height_max = height_max;
    game.ceiling_height_min = height_min;
    game.ceiling_step = step;
    game.ceiling_search_dist = (2*game.ceiling_dist+1) * (2*game.ceiling_dist+1);
    return 1;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
