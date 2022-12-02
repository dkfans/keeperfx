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

DLLIMPORT long _DK_ceiling_init(unsigned long a1, unsigned long a2);
static char ceiling_cache[256*256];

static int find_column_height_including_lintels(struct Column *col)
{
    unsigned char i;
    if (!col->solidmask)
        return 0;
    for (i = 7; !col->cubes[i]; --i)
        ;
    return i + 1;
}

static int ceiling_block_is_solid_including_corners_return_height(SubtlCodedCoords stl_num, MapSubtlCoord cstl_x, MapSubtlCoord cstl_y)
{
    MapSubtlCoord stl_x = stl_num_decode_x(stl_num);
    MapSubtlCoord stl_y = stl_num_decode_y(stl_num);
    struct Map *mapblk = get_map_block_at(stl_x,stl_y);
    struct Column *col = get_map_column(mapblk);

    if ((mapblk->flags & SlbAtFlg_Blocking) == 0 && (col->bitfields & CLF_CEILING_MASK) == 0)
    {
        if (cstl_x <= 0)
        {
            if (cstl_y > 0)
            {
                mapblk = get_map_block_at(stl_x,stl_y - 1);
                col = get_map_column(mapblk);
                if ((mapblk->flags & SlbAtFlg_Blocking) != 0 || (col->bitfields & CLF_CEILING_MASK) != 0)
                {
                    if ((col->bitfields & CLF_CEILING_MASK) != 0)
                        return find_column_height_including_lintels(col);
                    return col->bitfields >> 4;
                }
            }
        }
        else
        {
            mapblk = get_map_block_at(stl_x - 1,stl_y);
            col = get_map_column(mapblk);
            if ((mapblk->flags & SlbAtFlg_Blocking) != 0 || (col->bitfields & CLF_CEILING_MASK) != 0)
            {
                if ((col->bitfields & CLF_CEILING_MASK) != 0)
                    return find_column_height_including_lintels(col);
                return col->bitfields >> 4;
            }
            if (cstl_y > 0)
            {
                mapblk = get_map_block_at(stl_x,stl_y - 1);
                col = get_map_column(mapblk);
                if ((mapblk->flags & SlbAtFlg_Blocking) != 0 || (col->bitfields & CLF_CEILING_MASK) != 0)
                {
                    if ((col->bitfields & CLF_CEILING_MASK) != 0)
                        return find_column_height_including_lintels(col);
                    return col->bitfields >> 4;
                }
                mapblk = get_map_block_at(stl_x - 1,stl_y - 1);
                col = get_map_column(mapblk);
                if ((mapblk->flags & SlbAtFlg_Blocking) != 0 || (col->bitfields & CLF_CEILING_MASK) != 0)
                {
                    if ((col->bitfields & CLF_CEILING_MASK) != 0)
                        return find_column_height_including_lintels(col);
                    return col->bitfields >> 4;
                }
            }
        }
        return -1;
    }

    mapblk = get_map_block_at(stl_x,stl_y);
    col = get_map_column(mapblk);
    if ((col->bitfields & CLF_CEILING_MASK) != 0)
        return find_column_height_including_lintels(col);
    return col->bitfields >> 4;
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

long ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey)
{
    int v11;
    int v15;
    int v16;
    int cstl_y;
    int cstl_x;
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
    int v42;
    int v44;
    int v47;
    int *v48;
    int v49;
    MapSubtlCoord unk_stl_y;
    MapSubtlCoord unk_stl_x;
    int ceil_dist = game.ceiling_dist;
    if (game.ceiling_dist > 4)
        ceil_dist = 4;
    MapSubtlCoord unk3stl_x = sx - ceil_dist;
    if (unk3stl_x <= 0)
        unk3stl_x = 0;
    MapSubtlCoord unk3stl_y = sy - ceil_dist;
    if (sy - ceil_dist <= 0)
        unk3stl_y = 0;
    v47 = ex + ceil_dist;
    if (ex + ceil_dist >= (map_subtiles_x + 1))
        v47 = (map_subtiles_x + 1);
    v11 = ey + ceil_dist;
    if (v11 >= (map_subtiles_y + 1))
        v11 = (map_subtiles_y + 1);
    //ceiling_cache = (signed char*)scratch;
    v44 = unk3stl_x - game.ceiling_dist;
    if (unk3stl_x - game.ceiling_dist <= 0)
        v44 = 0;
    cstl_y = unk3stl_y - game.ceiling_dist;
    if (unk3stl_y - game.ceiling_dist <= 0)
        cstl_y = 0;
    v15 = game.ceiling_dist + v47;
    if (game.ceiling_dist + v47 >= (map_subtiles_x + 1))
        v15 = (map_subtiles_x + 1);
    v16 = v11 + game.ceiling_dist;
    if (v11 + game.ceiling_dist >= (map_subtiles_y + 1))
        v16 = (map_subtiles_y + 1);
    
    if (cstl_y < v16)
    {
        do
        {
            cstl_x = v44;
            while (cstl_x < v15)
            {
                SubtlCodedCoords stl_num = get_subtile_number(cstl_x,cstl_y);
                ceiling_cache[stl_num] = ceiling_block_is_solid_including_corners_return_height(stl_num,cstl_x,cstl_y);
                cstl_x++;
            }
            cstl_y++;
        } while (cstl_y < v16);
    }

    if (unk3stl_y < v11)
    {
        v42 = v11;
        do
        {
            v49 = unk3stl_x;
            SubtlCodedCoords stl_num2 = get_subtile_number(unk3stl_x,unk3stl_y);
            if (v47 > unk3stl_x)
            {
                do
                {
                    p_data = &game.map[stl_num2].data;
                    v22 = ceiling_cache[stl_num2];
                    v38 = v22;
                    if (v22 <= -1)
                    {
                        v48 = &v38;
                        unk_stl_x = stl_num_decode_x(stl_num2);
                        unk_stl_y = stl_num_decode_y(stl_num2);
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
                                    v27 = ceiling_cache[get_subtile_number(unk2_stl_x ,unk2_stl_y)];
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
                    ++stl_num2;
                    *((char *)p_data + 3) = v33;
                    v35 = ((v22 & 0xF) << 24) | *p_data;
                    v34 = ++v49;
                    *p_data = v35;
                } while (v34 < v32);
            }
            unk3stl_y ++;
        } while (unk3stl_y < v42);
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

static int get_ceiling_or_floor_filled_subtiles(int stl_num)
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
    //return _DK_ceiling_init(a1, a2);
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
                            filled_h = ceiling_block_is_solid_including_corners_return_height(get_subtile_number(stl_x + sstep->v ,stl_y + sstep->h), cstl_x, cstl_y);
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
