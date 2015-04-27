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
    SlbAtCtg_Unclaimed = 0,
    SlbAtCtg_FriableDirt,
    SlbAtCtg_FortifiedGround,
    SlbAtCtg_FortifiedWall,
    SlbAtCtg_RoomInterior,
    SlbAtCtg_Obstacle,
};

enum SlabAttrFlags {
    SlbAtFlg_None = 0x00,
    SlbAtFlg_Valuable = 0x01, /*< Set for valuable terrain to dig (gold and gems). */
    SlbAtFlg_IsRoom = 0x02,
    SlbAtFlg_Unk04 = 0x04,
    SlbAtFlg_Digable = 0x08,
    SlbAtFlg_Blocking = 0x10,
    SlbAtFlg_Filled = 0x20,
    SlbAtFlg_IsDoor = 0x40,
    SlbAtFlg_Unk80 = 0x80,
};

enum RoomCfgFlags {
    RoCFlg_None          = 0x00,
    RoCFlg_NoEnsign      = 0x01,
    RoCFlg_CantVandalize = 0x02,
    RoCFlg_BuildToBroke  = 0x04,
};

struct SlabMap;

struct SlabAttr {
    unsigned short tooltip_str_idx;
    short field_2;
    short field_4;
    unsigned long block_flags;
    unsigned long noblck_flags;
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
    TextStringId tooltip_str_idx;
    RoomKind assigned_room;
};

struct RoomConfigStats {
    char code_name[COMMAND_WORD_LEN];
    TextStringId name_stridx;
    TextStringId tooltip_str_idx;
    long creature_creation_model;
    SlabKind assigned_slab;
    unsigned long flags;
    long panel_tab_idx;
    long bigsym_sprite_idx;
    long medsym_sprite_idx;
    long pointer_sprite_idx;
    unsigned int ambient_snd_smp_id;
    long msg_needed;
    long msg_too_small;
    long msg_no_route;
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
extern struct SlabsConfig slab_conf;
/******************************************************************************/
TbBool load_terrain_config(const char *conf_fname,unsigned short flags);
/******************************************************************************/
struct SlabAttr *get_slab_kind_attrs(SlabKind slab_kind);
struct SlabAttr *get_slab_attrs(const struct SlabMap *slb);
struct SlabConfigStats *get_slab_kind_stats(SlabKind slab_kind);
struct SlabConfigStats *get_slab_stats(struct SlabMap *slb);
const char *room_code_name(RoomKind rkind);
const char *slab_code_name(SlabKind slbkind);
/******************************************************************************/
TbBool slab_kind_is_indestructible(RoomKind slbkind);
TbBool slab_kind_is_fortified_wall(RoomKind slbkind);
TbBool slab_kind_is_friable_dirt(RoomKind slbkind);
TbBool slab_kind_is_door(SlabKind slbkind);
TbBool slab_kind_is_nonmagic_door(SlabKind slbkind);
TbBool slab_kind_is_liquid(SlabKind slbkind);
TbBool slab_kind_is_room(SlabKind slbkind);
/******************************************************************************/
struct RoomConfigStats *get_room_kind_stats(RoomKind room_kind);
TbBool make_all_rooms_free(void);
TbBool set_room_available(PlayerNumber plyr_idx, RoomKind room_idx, long resrch, long avail);
TbBool make_available_all_researchable_rooms(PlayerNumber plyr_idx);
TbBool make_all_rooms_researchable(PlayerNumber plyr_idx);
TbBool is_room_available(PlayerNumber plyr_idx, RoomKind room_idx);
ThingModel get_room_create_creature_model(RoomKind room_kind);
TbBool enemies_may_work_in_room(RoomKind rkind);
TbBool room_grows_food(RoomKind rkind);
TbBool room_has_surrounding_flames(RoomKind rkind);
TbBool room_cannot_vandalise(RoomKind rkind);
TbBool room_never_buildable(RoomKind rkind);
TbBool room_can_have_ensign(RoomKind rkind);
SlabKind room_corresponding_slab(RoomKind rkind);
RoomKind slab_corresponding_room(SlabKind slbkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
