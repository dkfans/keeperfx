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
#include "config.h"

#include "thing_list.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define OBJECT_TYPES_COUNT_ORIGINAL  136
#define OBJECT_TYPES_COUNT  255

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

//avoid using this enum for new stuff, all properties should come from config, not hardcoded to 1 type
//only the ones still used by code are mentioned here
enum ObjectModels
{
    ObjMdl_Torch = 2,
    ObjMdl_GoldChest = 3,
    ObjMdl_StatueLit = 4,
    ObjMdl_SoulCountainer = 5,
    ObjMdl_GoldPot = 6,
    ObjMdl_ChickenGrowing = 9,
    ObjMdl_ChickenMature = 10,
    ObjMdl_CTAEnsign = 24,
    ObjMdl_RoomFlag = 25,
    ObjMdl_Anvil = 26,
    ObjMdl_PrisonBar = 27,
    ObjMdl_Candlestick = 28,
    ObjMdl_Gravestone = 29,
    ObjMdl_TrainingPost = 31,
    ObjMdl_TortureSpike = 32,
    ObjMdl_TempleSpangle = 33,
    ObjMdl_PowerHand = 37,
    ObjMdl_PowerHandGrab = 38,
    ObjMdl_PowerHandWhip = 39,
    ObjMdl_Goldl = 43,
    ObjMdl_SpinningKey = 44,
    ObjMdl_HeroGate = 49,
    ObjMdl_LightBall =  51,
    ObjMdl_GoldPile = 52,
    ObjMdl_GoldHorde1 = 53,
    ObjMdl_GoldHorde2 = 54,
    ObjMdl_GoldHorde3 = 55,
    ObjMdl_GoldHorde4 = 56,
    ObjMdl_SpecboxRevealMap = 86,
    ObjMdl_SpecboxResurect = 87,
    ObjMdl_SpecboxTransfer = 88,
    ObjMdl_SpecboxStealHero = 89,
    ObjMdl_SpecboxMultiply = 90,
    ObjMdl_SpecboxIncreaseLevel = 91,
    ObjMdl_SpecboxMakeSafe = 92,
    ObjMdl_SpecboxHiddenWorld = 93,
    ObjMdl_HeartFlameRed = 111,
    ObjMdl_Disease = 112,
    ObjMdl_ScavangeEye = 113,
    ObjMdl_WorkshopMachine = 114,
    ObjMdl_GuardFlagRed = 115,
    ObjMdl_GuardFlagBlue = 116,
    ObjMdl_GuardFlagGreen = 117,
    ObjMdl_GuardFlagYellow = 118,
    ObjMdl_GuardFlagPole = 119,
    ObjMdl_HeartFlameBlue = 120,
    ObjMdl_HeartFlameGreen = 121,
    ObjMdl_HeartFlameYellow = 122,
    ObjMdl_PowerSight = 123,
    ObjMdl_PowerLightning = 124,
    ObjMdl_Torturer = 125,
    ObjMdl_PowerHandWithGold = 127,
    ObjMdl_SpinningCoin = 128,
    ObjMdl_SpecboxCustom = 133,
    ObjMdl_GoldBag = 136
};
/******************************************************************************/
#pragma pack(1)

struct Objects {
    unsigned char initial_state;
    unsigned char start_frame_to_minus1;
    unsigned char not_drawn;
    short sprite_anim_idx;
    short anim_speed;
    short size_xy;
    short size_yz;
    short sprite_size_max;
    unsigned char field_F;      // Lower 2 bits are transparency flags
    unsigned short fp_smpl_idx;
    unsigned char draw_class;
    unsigned char destroy_on_lava;
    /** Creature model related to the object, ie for lairs - which creature lair it is. */
    unsigned char related_creatr_model;
    unsigned char own_category;
    unsigned char destroy_on_liquid;
    unsigned char rotation_flag;
    unsigned char updatefn_idx;
};

struct CallToArmsGraphics {
    int birth_anim_idx;
    int alive_anim_idx;
    int leave_anim_idx;
};

#pragma pack()
/******************************************************************************/
extern unsigned short player_guardflag_objects[];
extern unsigned short dungeon_flame_objects[];
extern const struct NamedCommand object_update_functions_desc[];

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
TbBool object_is_room_inventory(const struct Thing *thing, RoomRole rrole);
TbBool object_is_unaffected_by_terrain_changes(const struct Thing *thing);
TbBool object_can_be_damaged(const struct Thing* thing);
TbBool object_is_buoyant(const struct Thing* thing);
TbBool thing_is_hardcoded_special_box(const struct Thing* thing);
TbBool thing_is_custom_special_box(const struct Thing* thing);

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
struct Thing* create_gold_pile(struct Coord3d* pos, PlayerNumber plyr_idx, long value);
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
