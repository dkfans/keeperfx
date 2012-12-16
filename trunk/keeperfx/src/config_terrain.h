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

#define TERRAIN_ITEMS_MAX    256
// Amount of possible types of slabs
#define SLAB_TYPES_COUNT      58

/******************************************************************************/
#pragma pack(1)

enum SlabAttrCategory {
    SlbAtCtg_Unset = 0,
    SlbAtCtg_Unknown1,
    SlbAtCtg_Unknown2,
    SlbAtCtg_Unknown3,
    SlbAtCtg_Unknown4,
    SlbAtCtg_Unknown5,
};

enum SlabAttrFlags {
    SlbAtFlg_None =  0x00,
    SlbAtFlg_Unk01 = 0x01,
    SlbAtFlg_Unk02 = 0x02,
    SlbAtFlg_Unk04 = 0x04,
    SlbAtFlg_Unk08 = 0x08,
    SlbAtFlg_Unk10 = 0x10,
    SlbAtFlg_Unk20 = 0x20,
    SlbAtFlg_Unk40 = 0x40,
    SlbAtFlg_Unk80 = 0x80,
};

struct SlabMap;

struct SlabAttr {
    unsigned short tooltip_stridx;
    short field_2;
    short field_4;
    unsigned long flags;
    long field_A;
    unsigned char field_E;
    unsigned char category;
    unsigned char field_10;
    unsigned char is_unknflg11;
    unsigned char is_safe_land;
    unsigned char is_unknflg13;
    unsigned char is_unknflg14;
    unsigned char is_unknflg15;
};

#pragma pack()
/******************************************************************************/
struct SlabConfigStats {
    char code_name[COMMAND_WORD_LEN];
    long tooltip_stridx;
};

struct RoomConfigStats {
    char code_name[COMMAND_WORD_LEN];
    long tooltip_stridx;
    long creature_creation_model;
};

struct SlabsConfig {
    long slab_types_count;
    struct SlabConfigStats slab_cfgstats[TERRAIN_ITEMS_MAX];
    long room_types_count;
    struct RoomConfigStats room_cfgstats[TERRAIN_ITEMS_MAX];
};
/******************************************************************************/
extern const char keeper_terrain_file[];
extern struct NamedCommand slab_desc[TERRAIN_ITEMS_MAX];
extern struct NamedCommand room_desc[TERRAIN_ITEMS_MAX];
/******************************************************************************/
TbBool load_terrain_config(const char *conf_fname,unsigned short flags);
/******************************************************************************/
struct SlabAttr *get_slab_kind_attrs(SlabKind slab_kind);
struct SlabAttr *get_slab_attrs(struct SlabMap *slb);
struct SlabConfigStats *get_slab_kind_stats(SlabKind slab_kind);
struct SlabConfigStats *get_slab_stats(struct SlabMap *slb);
const char *room_code_name(RoomKind rkind);
const char *slab_code_name(SlabKind slbkind);
/******************************************************************************/
TbBool slab_indestructible(RoomKind slbkind);
/******************************************************************************/
struct RoomConfigStats *get_room_kind_stats(RoomKind room_kind);
TbBool make_all_rooms_free(void);
TbBool set_room_available(PlayerNumber plyr_idx, RoomKind room_idx, long resrch, long avail);
TbBool make_available_all_researchable_rooms(PlayerNumber plyr_idx);
TbBool make_all_rooms_researchable(PlayerNumber plyr_idx);
TbBool is_room_available(PlayerNumber plyr_idx, RoomKind room_idx);
ThingModel get_room_create_creature_model(RoomKind room_kind);
TbBool enemies_may_work_in_room(RoomKind rkind);
TbBool room_cannot_vandalize(RoomKind rkind);
TbBool room_never_buildable(RoomKind rkind);
TbBool room_can_have_flag(RoomKind rkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
