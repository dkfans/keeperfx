/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_columns.c
 *     Column and columns array data management functions.
 * @par Purpose:
 *     Functions to support the column data array.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 Oct 2009 - 09 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_columns.h"
#include "globals.h"

#include "bflib_memory.h"
#include "slab_data.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
DLLIMPORT void _DK_init_columns(void);
DLLIMPORT long _DK_get_top_cube_at_pos(long mpos);
DLLIMPORT void _DK_clear_columns(void);
DLLIMPORT long _DK_find_column(struct Column *col);
DLLIMPORT long _DK_create_column(struct Column *col);
DLLIMPORT void _DK_init_top_texture_to_cube_table(void);
DLLIMPORT void _DK_init_whole_blocks(void);
/******************************************************************************/
struct Column *get_column(long idx)
{
  if ((idx < 1) || (idx >= COLUMNS_COUNT))
    return INVALID_COLUMN;
  return game.columns.lookup[idx];
}

struct Column *get_column_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  struct Map *map;
  map = get_map_block_at(stl_x, stl_y);
  if (map_block_invalid(map))
    return INVALID_COLUMN;
  return game.columns.lookup[map->data & 0x7FF];
}

struct Column *get_map_column(const struct Map *map)
{
  if (map_block_invalid(map))
    return INVALID_COLUMN;
  return game.columns.lookup[map->data & 0x7FF];
}

TbBool column_invalid(const struct Column *colmn)
{
  if (colmn == NULL)
    return true;
  if (colmn == INVALID_COLUMN)
    return true;
  return (colmn <= game.columns.lookup[0]) || (colmn > game.columns.lookup[COLUMNS_COUNT-1]) || (colmn == NULL);
}

long get_top_cube_at_pos(long stl_num)
{
    struct Column *col;
    struct Map *map;
    unsigned long top_pos;
    long tcube;
    //return _DK_get_top_cube_at_pos(mpos);
    map = get_map_block_at_pos(stl_num);
    col = get_map_column(map);
    top_pos = (col->bitfileds >> 4) & 0x0F;
    if (top_pos > 0)
        tcube = col->cubes[top_pos-1];
    else
        tcube = game.field_14BB65[col->baseblock];
    return tcube;
}

long get_top_cube_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  struct Column *col;
  unsigned long top_pos;
  long tcube;
  col = get_column_at(stl_x, stl_y);
  top_pos = (col->bitfileds >> 4) & 0x0F;
  if (top_pos > 0)
    tcube = col->cubes[top_pos-1];
  else
    tcube = game.field_14BB65[col->baseblock];
  return tcube;
}

void make_solidmask(struct Column *col)
{
  int i;
  col->solidmask = 0;
  for (i=0; i<COLUMN_STACK_HEIGHT; i++)
  {
    if (col->cubes[i] != 0)
      col->solidmask |= (1 << i);
  }
}

unsigned short find_column_height(struct Column *col)
{
  unsigned short h;
  h = 0;
  if (col->solidmask == 0)
    return h;
  while (col->cubes[h] > 0)
  {
    h++;
    if (h >= COLUMN_STACK_HEIGHT)
      return COLUMN_STACK_HEIGHT;
  }
  return h;
}

long get_floor_height_at(struct Coord3d *pos)
{
    const struct Map *mapblk;
    const struct Column *colmn;
    long i,cubes_height;
    mapblk = get_map_block_at(pos->x.val >> 8, pos->y.val >> 8);
    colmn = get_map_column(mapblk);
    cubes_height = 0;
    i = colmn->bitfileds;
    if ((i & 0xF0) > 0)
        cubes_height = i >> 4;
    return cubes_height << 8;
}

long find_column(struct Column *colmn)
{
  return _DK_find_column(colmn);
}

long create_column(struct Column *colmn)
{
  return _DK_create_column(colmn);
}

void clear_columns(void)
{
  //  _DK_clear_columns();
  struct Column *colmn;
  int i;
  for (i=0; i < COLUMNS_COUNT; i++)
  {
    colmn = &game.columns_data[i];
    LbMemorySet(colmn, 0, sizeof(struct Column));
    colmn->baseblock = 1;
    make_solidmask(colmn);
  }
  game.field_149E6E = -1;
  game.field_149E7C = 24;
  game.unrevealed_column_idx = 0;
  for (i=0; i < 18; i++)
  {
    game.field_14A818[i] = 0;
  }
}

void init_columns(void)
{
  _DK_init_columns();
}

void init_whole_blocks(void)
{
    struct Column *colmn;
    struct Column lcolmn;
    long i;
    //_DK_init_whole_blocks(); return;
    game.field_149E6E = -1;
    LbMemorySet(&lcolmn, 0, sizeof(struct Column));
    // Prepare the local column
    lcolmn.baseblock = 22;
    lcolmn.cubes[0] = 10;
    lcolmn.cubes[1] = 1;
    lcolmn.cubes[2] = 1;
    lcolmn.cubes[3] = 1;
    lcolmn.cubes[4] = 141;
    make_solidmask(&lcolmn);
    // Find it or add to column list
    i = find_column(&lcolmn);
    if (i == 0)
      i = create_column(&lcolmn);
    colmn = get_column(i);
    // Update its parameters
    colmn->bitfileds |= 0x01;
    game.field_149E7C = 24;
    game.unrevealed_column_idx = i;
}

void init_top_texture_to_cube_table(void)
{
  _DK_init_top_texture_to_cube_table();
}

TbBool cube_is_water(long cube_id)
{
    return (cube_id == 39);
}

TbBool cube_is_lava(long cube_id)
{
    return (cube_id == 40) || (cube_id == 41);
}

TbBool cube_is_sacrificial(long cube_id)
{
    return (cube_id >= 294) && (cube_id <= 302);
}

/**
 * Returns if given subtile has water cube on its top.
 * @param stl_x Subtile X coordinate.
 * @param stl_y Subtile Y coordinate.
 * @return True if the top cube is water; false otherwise.
 */
TbBool subtile_has_water_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    long i;
    i = get_top_cube_at(stl_x, stl_y);
    return cube_is_water(i);
}

/**
 * Returns if given subtile has lava cube on its top.
 * @param stl_x Subtile X coordinate.
 * @param stl_y Subtile Y coordinate.
 * @return True if the top cube is lava; false otherwise.
 */
TbBool subtile_has_lava_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    long i;
    i = get_top_cube_at(stl_x, stl_y);
    return cube_is_lava(i);
}

/**
 * Returns if given subtile has sacrificial (temple) cube on its top.
 * @param stl_x Subtile X coordinate.
 * @param stl_y Subtile Y coordinate.
 * @return True if the top cube is sacrificial; false otherwise.
 */
TbBool subtile_has_sacrificial_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    long i;
    i = get_top_cube_at(stl_x, stl_y);
    return cube_is_sacrificial(i);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
