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
#include "room_data.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define TERRAIN_ITEMS_MAX    256
#define SLABSETS_PER_SLAB   (9*3+1)


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
    SlbAtFlg_Unexplored = 0x04,
    SlbAtFlg_Digable = 0x08,
    SlbAtFlg_Blocking = 0x10,
    SlbAtFlg_Filled = 0x20,
    SlbAtFlg_IsDoor = 0x40,
    SlbAtFlg_TaggedValuable = 0x80,
};

enum SlabFillStyle {
    SlbFillStl_Normal = 0,
    SlbFillStl_Lava = 1,
    SlbFillStl_Water = 2,
};

enum RoomCfgFlags {
    RoCFlg_None            = 0x00,
    RoCFlg_NoEnsign        = 0x01,
    RoCFlg_CantVandalize   = 0x02,
    RoCFlg_BuildTillBroke  = 0x04,
    RoCFlg_CannotBeSold    = 0x08,
    RoCFlg_CannotBeClaimed = 0x10,
    RoCFlg_ListEnd         = 0x20,
};

/**
 * Enumeration used to assign roles to rooms.
 */
enum RoomRoleFlags {
    RoRoF_None           = 0x00000000,
    RoRoF_KeeperStorage  = 0x00000001, /**< The room is a storage for keeper soul (dungeon heart). */
    RoRoF_LairStorage    = 0x00000002, /**< The room is a storage for creature lair totems. */
    RoRoF_GoldStorage    = 0x00000004, /**< The room is a storage for gold. */
    RoRoF_FoodStorage    = 0x00000008, /**< The room is a storage for food for creatures. */
    RoRoF_CratesStorage  = 0x00000010, /**< The room is a storage for crates (trap and door boxes). */
    RoRoF_PowersStorage  = 0x00000020, /**< The room is a storage for keeper powers (spellbooks and specials). */
    RoRoF_Prison         = 0x00000040, /**< The room is a prison, forcing friends and foes to stay within. */
    RoRoF_DeadStorage    = 0x00000080, /**< The room is a storage for dead bodies. */
    RoRoF_CrPoolSpawn    = 0x00000100, /**< The room is a spawn point for creatures coming into dungeon from creature pool. */
    RoRoF_CrConditSpawn  = 0x00000200, /**< The room is a spawn point for creatures with special spawn conditions programmed. */
    RoRoF_CrSacrifice    = 0x00000400, /**< The room can be used to sacrifice creatures and gain rewards. */
    RoRoF_CrPurifySpell  = 0x00000800, /**< The room can be used to cancel negative spells affecting creatures. */
    RoRoF_FoodSpawn      = 0x00001000, /**< The room is a spawn place for food. */
    RoRoF_CratesManufctr = 0x00002000, /**< The room is a manufacture place for trap crates. */
    RoRoF_Research       = 0x00004000, /**< The room is a research place for spellbooks, traps and rooms. */
    RoRoF_Torture        = 0x00008000, /**< The room is a torture chamber, allowing torture of friends and foes. */
    RoRoF_CrHappyPray    = 0x00010000, /**< The room makes its workers increase their happiness by praying. */
    RoRoF_CrHealSleep    = 0x00020000, /**< The room makes its workers to heal by sleeping. */
    RoRoF_CrScavenge     = 0x00040000, /**< The room makes its workers scavenge enemy creatures. */
    RoRoF_CrTrainExp     = 0x00080000, /**< The room makes its workers increase their experience by training. */
    RoRoF_CrMakeGroup    = 0x00100000, /**< The room makes its workers form a group of creatures. */
    RoRoF_CrGuard        = 0x00200000, /**< The room makes its workers guard the room area for enemies. */
    RoRoF_CrPoolLeave    = 0x00400000, /**< The room is a gate which allows a creature to leave the players dungeon back to pool. */
    RoRoF_PassWater      = 0x00800000, /**< The room is a bridge for use over water. */
    RoRoF_PassLava       = 0x01000000, /**< The room is a bridge for use over lava. */
};

struct SlabMap;

#pragma pack()
/******************************************************************************/
struct SlabConfigStats {
    char code_name[COMMAND_WORD_LEN];
    TextStringId tooltip_stridx;
    RoomKind assigned_room;
    short block_flags_height;
    short block_health_index;
    uint32_t block_flags;
    uint32_t noblck_flags;
    unsigned char fill_style;
    unsigned char category;
    unsigned char slb_id;
    unsigned char wibble;
    unsigned char is_safe_land;
    unsigned char animated;
    unsigned char is_diggable;
    unsigned char wlb_type;
    unsigned char is_ownable;
    unsigned char indestructible;
    GoldAmount gold_held;
};

struct RoomConfigStats {
    char code_name[COMMAND_WORD_LEN];
    TextStringId name_stridx;
    TextStringId tooltip_stridx;
    int32_t creature_creation_model;
    SlabKind assigned_slab;
    short synergy_slab;
    char storage_height;
    uint32_t flags;
    RoomRole roles;
    int32_t panel_tab_idx;
    /** Sprite index of big symbol icon representing the room. */
    int32_t bigsym_sprite_idx;
    /** Sprite index of medium symbol icon representing the room. */
    int32_t medsym_sprite_idx;
    int32_t pointer_sprite_idx;
    uint32_t ambient_snd_smp_id;
    int32_t msg_needed;
    int32_t msg_too_small;
    int32_t msg_no_route;
    short cost;
    HitPoints health;
    FuncIdx update_total_capacity_idx;
    FuncIdx update_storage_in_room_idx;
    FuncIdx update_workers_in_room_idx;
};

struct SlabsConfig {
    int32_t slab_types_count;
    struct SlabConfigStats slab_cfgstats[TERRAIN_ITEMS_MAX];
    int32_t room_types_count;
    struct RoomConfigStats room_cfgstats[TERRAIN_ITEMS_MAX];
};
/******************************************************************************/
extern const struct ConfigFileData keeper_terrain_file_data;
extern struct NamedCommand slab_desc[TERRAIN_ITEMS_MAX];
extern struct NamedCommand room_desc[TERRAIN_ITEMS_MAX];
extern const struct NamedCommand room_roles_desc[];
extern Room_Update_Func terrain_room_total_capacity_func_list[13];
extern Room_Update_Func terrain_room_used_capacity_func_list[10];

extern const struct NamedFieldSet terrain_room_named_fields_set;
/******************************************************************************/
struct SlabConfigStats *get_slab_kind_stats(SlabKind slab_kind);
struct SlabConfigStats *get_slab_stats(const struct SlabMap *slb);
const char *room_role_code_name(RoomRole rrole);
const char *room_code_name(RoomKind rkind);
const char *slab_code_name(SlabKind slbkind);
/******************************************************************************/
TbBool slab_kind_is_indestructible(RoomKind slbkind);
TbBool slab_kind_is_fortified_wall(RoomKind slbkind);
TbBool slab_kind_is_room_wall(RoomKind slbkind);
TbBool slab_kind_is_friable_dirt(RoomKind slbkind);
TbBool slab_kind_is_door(SlabKind slbkind);
TbBool slab_kind_is_liquid(SlabKind slbkind);
TbBool slab_kind_is_room(SlabKind slbkind);
TbBool slab_kind_has_torches(SlabKind slbkind);
/******************************************************************************/
struct RoomConfigStats *get_room_kind_stats(RoomKind room_kind);
TbBool make_all_rooms_free(void);
TbBool set_room_available(PlayerNumber plyr_idx, RoomKind roomkind, long resrch, long avail);
TbBool make_available_all_researchable_rooms(PlayerNumber plyr_idx);
TbBool make_all_rooms_researchable(PlayerNumber plyr_idx);
TbBool is_room_available(PlayerNumber plyr_idx, RoomKind roomkind);
TbBool is_room_obtainable(PlayerNumber plyr_idx, RoomKind rkind);
TbBool is_room_of_role_available(PlayerNumber plyr_idx, RoomRole rrole);
RoomKind find_first_available_roomkind_with_role(PlayerNumber plyr_idx, RoomRole rrole);
ThingModel get_room_create_creature_model(RoomKind room_kind);
TbBool enemies_may_work_in_room(RoomKind rkind);
RoomRole get_room_roles(RoomKind rkind);
TbBool room_role_matches(RoomKind rkind, RoomRole rrole);
TbBool room_has_surrounding_flames(RoomKind rkind);
TbBool room_cannot_vandalise(RoomKind rkind);
TbBool room_never_buildable(RoomKind rkind);
TbBool room_can_have_ensign(RoomKind rkind);
SlabKind room_corresponding_slab(RoomKind rkind);
RoomKind slab_corresponding_room(SlabKind slbkind);
RoomKind find_first_roomkind_with_role(RoomRole rrole);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
