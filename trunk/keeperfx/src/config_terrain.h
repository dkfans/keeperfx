/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_terrain.h
 *     Header file for config_terrain.c.
 * @par Purpose:
 *     Slabs, rooms, traps and doors configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGTERRAIN_H
#define DK_CFGTERRAIN_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define TERRAIN_ITEMS_MAX 256

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct SlabAttr {
    unsigned short tooltip_idx;
    short field_2;
    short field_4;
    long field_6;
    long field_A;
    unsigned char field_E;
    unsigned char field_F;
    unsigned char field_10;
    unsigned char field_11;
    unsigned char is_safe_land;
    unsigned char field_13;
    unsigned char field_14;
    unsigned char field_15;
};

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/

struct SlabsConfig {
    long slab_types_count;
    struct CommandWord slab_names[TERRAIN_ITEMS_MAX];
    long room_types_count;
    struct CommandWord room_names[TERRAIN_ITEMS_MAX];
    long trap_types_count;
    struct CommandWord trap_names[TERRAIN_ITEMS_MAX];
    long door_types_count;
    struct CommandWord door_names[TERRAIN_ITEMS_MAX];
};
/******************************************************************************/
extern const char keeper_terrain_file[];
extern struct NamedCommand slab_desc[TERRAIN_ITEMS_MAX];
extern struct NamedCommand room_desc[TERRAIN_ITEMS_MAX];
extern struct NamedCommand trap_desc[TERRAIN_ITEMS_MAX];
extern struct NamedCommand door_desc[TERRAIN_ITEMS_MAX];
/******************************************************************************/
struct SlabAttr *get_slab_kind_attrs(long slab_kind);
struct SlabAttr *get_slab_attrs(struct SlabMap *slb);
/******************************************************************************/
TbBool load_terrain_config(const char *conf_fname,unsigned short flags);
TbBool make_all_rooms_free(void);
TbBool set_room_available(long plyr_idx, long room_idx, long resrch, long avail);
TbBool make_available_all_researchable_rooms(long plyr_idx);
TbBool make_all_rooms_researchable(long plyr_idx);
TbBool is_room_available(long plyr_idx, long room_idx);
/******************************************************************************/
const char *door_code_name(long tngmodel);
const char *trap_code_name(long tngmodel);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
