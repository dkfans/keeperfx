/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_objects.h
 *     Header file for thing_objects.c.
 * @par Purpose:
 *     Things of class 'object' handling functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     05 Nov 2009 - 01 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THINGOBJCT_H
#define DK_THINGOBJCT_H

#include "globals.h"

#include "thing_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define OBJECT_TYPES_COUNT_ORIGINAL  136
#define OBJECT_TYPES_COUNT  255

#define OBJECT_TYPE_SPECBOX_CUSTOM    133

enum ObjectStates {
    ObSt_Unused = 0,
    ObSt_FoodMoves,
    ObSt_FoodGrows,
    ObSt_BeingDestroyed,
    ObSt_BeingDropped,
    ObSt_State5,
};

enum ObjectOwningCategory {
    ObOC_Unknown0 = 0,
    ObOC_Unknown1,
    ObOC_Unknown2,
    ObOC_Unknown3,
};

enum CallToArmsObjectLife {
    CTAOL_Unset = 0,
    CTAOL_Birthing,
    CTAOL_Alive,
    CTAOL_Dying,
    CTAOL_Rebirthing,
};
/******************************************************************************/
#pragma pack(1)

struct Objects {
    unsigned char initial_state;
    unsigned char field_1;
    unsigned char field_2;
    unsigned char field_3;
    unsigned char field_4;
    short sprite_anim_idx;
    short anim_speed;
    short size_xy;
    short size_yz;
    short sprite_size_max;
    unsigned char field_F;      // Lower 2 bits are transparency flags
    unsigned char field_10;
    unsigned char draw_class;
    unsigned char destroy_on_lava;
    /** Creature model related to the object, ie for lairs - which creature lair it is. */
    unsigned char related_creatr_model;
    unsigned char own_category;
    unsigned char destroy_on_liquid;
    unsigned char rotation_flag;
};

struct CallToArmsGraphics {
    int birth_anim_idx;
    int alive_anim_idx;
    int leave_anim_idx;
};

/******************************************************************************/
/*
TODO: Test and remove these
DLLIMPORT unsigned char _DK_object_to_special[OBJECT_TYPES_COUNT];
DLLIMPORT unsigned char _DK_object_to_magic[OBJECT_TYPES_COUNT];
DLLIMPORT unsigned char _DK_workshop_object_class[OBJECT_TYPES_COUNT];
DLLIMPORT unsigned char _DK_object_to_door_or_trap[OBJECT_TYPES_COUNT];
DLLIMPORT extern unsigned char _DK_magic_to_object[24];
*/

#pragma pack()
/******************************************************************************/
extern Thing_State_Func object_state_functions[];
extern Thing_Class_Func object_update_functions[];
extern unsigned short player_guardflag_objects[];
extern unsigned short dungeon_flame_objects[];

/******************************************************************************/
struct Thing *create_object(const struct Coord3d *pos, unsigned short model, unsigned short owner, long parent_idx);
void destroy_object(struct Thing *thing);
TngUpdateRet update_object(struct Thing *thing);
TbBool thing_is_object(const struct Thing *thing);
void change_object_owner(struct Thing *objtng, PlayerNumber nowner);
void destroy_food(struct Thing *foodtng);

struct Objects *get_objects_data_for_thing(struct Thing *thing);
struct Objects *get_objects_data(unsigned int tmodel);
struct Thing *get_spellbook_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *get_special_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *get_crate_at_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);

SpecialKind box_thing_to_special(const struct Thing *thing);
PowerKind book_thing_to_power_kind(const struct Thing *thing);

TbBool thing_is_special_box(const struct Thing *thing);
TbBool thing_is_workshop_crate(const struct Thing *thing);
TbBool thing_is_trap_crate(const struct Thing *thing);
TbBool thing_is_door_crate(const struct Thing *thing);
TbBool thing_is_dungeon_heart(const struct Thing *thing);
TbBool thing_is_mature_food(const struct Thing *thing);
TbBool object_is_hero_gate(const struct Thing *thing);
TbBool object_is_infant_food(const struct Thing *thing);
TbBool object_is_growing_food(const struct Thing *thing);
TbBool object_is_mature_food(const struct Thing *thing);
TbBool object_is_gold(const struct Thing *thing);
TbBool object_is_gold_pile(const struct Thing *thing);
TbBool object_is_gold_hoard(const struct Thing *thing);
TbBool object_is_gold_laying_on_ground(const struct Thing *thing);
TbBool object_is_guard_flag(const struct Thing *thing);
TbBool thing_is_gold_hoard(const struct Thing *thing);
TbBool thing_is_spellbook(const struct Thing *thing);
TbBool thing_is_lair_totem(const struct Thing *thing);
TbBool object_is_room_equipment(const struct Thing *thing, RoomKind rkind);
TbBool object_is_room_inventory(const struct Thing *thing, RoomKind rkind);
TbBool object_is_unaffected_by_terrain_changes(const struct Thing *thing);
TbBool object_can_be_damaged(const struct Thing* thing);

TbBool creature_remove_lair_totem_from_room(struct Thing *creatng, struct Room *room);
TbBool delete_lair_totem(struct Thing *lairtng);

struct Thing *create_guard_flag_object(const struct Coord3d *pos, PlayerNumber plyr_idx, long parent_idx);

int get_wealth_size_of_gold_hoard_object(const struct Thing *objtng);
int get_wealth_size_of_gold_hoard_model(ThingModel objmodel);
int get_wealth_size_of_gold_amount(GoldAmount value);
int get_wealth_size_types_count(void);
struct Thing *create_gold_hoard_object(const struct Coord3d *pos, PlayerNumber plyr_idx, GoldAmount value);
struct Thing *find_gold_hoard_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *create_gold_hoarde(struct Room *room, const struct Coord3d *pos, GoldAmount value);
long add_gold_to_hoarde(struct Thing *thing, struct Room *room, GoldAmount amount);
long remove_gold_from_hoarde(struct Thing *thing, struct Room *room, GoldAmount amount);
long gold_being_dropped_at_treasury(struct Thing* thing, struct Room* room);

struct Thing *drop_gold_pile(long value, struct Coord3d *pos);
struct Thing *create_gold_pot_at(long pos_x, long pos_y, PlayerNumber plyr_idx);
TbBool add_gold_to_pile(struct Thing *thing, long value);
GoldAmount gold_object_typical_value(ThingModel tngmodel);

void set_call_to_arms_as_birthing(struct Thing *objtng);
void set_call_to_arms_as_dying(struct Thing *objtng);
void set_call_to_arms_as_rebirthing(struct Thing *objtng);

void define_custom_object(int obj_id, short anim_idx);
void init_thing_objects();
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
