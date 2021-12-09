/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_objects.h
 *     Header file for config_objects.c.
 * @par Purpose:
 *     Object things configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Jun 2012 - 16 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGOBJECTS_H
#define DK_CFGOBJECTS_H

#include "globals.h"
#include "bflib_basics.h"

#include "config.h"
#include "light_data.h"
#include "thing_objects.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define OBJECT_TYPES_MAX  256

enum ObjectCategoryIndex {
    OCtg_Unknown = 0,
    OCtg_Decoration, //< Object has no strong function
    OCtg_Furniture,  //< Object is crucial part of a room
    OCtg_Valuable,   //< Object is gold in some form
    OCtg_Spellbook,  //< Object is a spellbook
    OCtg_SpecialBox, //< Object is a dungeon special box
    OCtg_WrkshpBox,  //< Object is a manufactured box for workshop
    OCtg_GoldHoard,  //< Object is a hoard of treasure stored in room
    OCtg_Food,       //< Object is food for creatures
    OCtg_Power,      //< Object is a keeper power effect, ie. hand of evil or keeper spell
    OCtg_LairTotem,  //< Object is a creature lair
    OCtg_Effect,     //< Object is some kind of effect which has influence on things or on terrain
};

enum ObjectModelFlags {
    OMF_ExistsOnlyInRoom     = 0x0001, // Some objects, ie. gold hoards, are strictly bound to room
    OMF_DestroyedOnRoomClaim = 0x0002, // Some objects should be destroyed if they're in a room which is changing owner
    OMF_ChOwnedOnRoomClaim   = 0x0004, // Most objects should change their owner with the room
    OMF_DestroyedOnRoomPlace = 0x0008, // Some objects should be destroyed when a new room/trap/door is placed on a slab
};

/******************************************************************************/
#pragma pack(1)

struct ObjectConfig { // sizeof=0x1D
    long health;
char fall_acceleration;
char light_unaffected;
char is_heart;
char resistant_to_nonmagic;
char movement_flag;
    struct InitLight ilght;
};

#pragma pack()
/******************************************************************************/
struct ObjectConfigStats {
    char code_name[COMMAND_WORD_LEN];
    unsigned short model_flags;
    long genre;
    long name_stridx;
    long map_icon;
};

struct ObjectsConfig {
    long object_types_count;
    struct ObjectConfigStats object_cfgstats[OBJECT_TYPES_MAX];
    ThingModel object_to_door_or_trap[OBJECT_TYPES_MAX];
    ThingModel object_to_power_artifact[OBJECT_TYPES_MAX];
    ThingModel object_to_special_artifact[OBJECT_TYPES_MAX];
    ThingClass workshop_object_class[OBJECT_TYPES_MAX];
    struct ObjectConfig base_config[OBJECT_TYPES_MAX];
};
/******************************************************************************/
extern const char keeper_objects_file[];
extern struct NamedCommand object_desc[OBJECT_TYPES_MAX];
extern const struct NamedCommand objects_genres_desc[];
extern const struct NamedCommand objects_object_commands[];
/******************************************************************************/
TbBool load_objects_config(const char *conf_fname,unsigned short flags);
struct ObjectConfigStats *get_object_model_stats(ThingModel tngmodel);
struct ObjectConfig *get_object_model_stats2(ThingModel tngmodel);
const char *object_code_name(ThingModel tngmodel);
ThingModel object_model_id(const char * code_name);
ThingClass crate_to_workshop_item_class(ThingModel tngmodel);
ThingModel crate_to_workshop_item_model(ThingModel tngmodel);
ThingClass crate_thing_to_workshop_item_class(const struct Thing *thing);
ThingModel crate_thing_to_workshop_item_model(const struct Thing *thing);
void init_objects(void);
int get_required_room_capacity_for_object(RoomRole room_role, ThingModel objmodel, ThingModel relmodel);
void update_all_object_stats();
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
