/******************************************************************************/
/** @file SaveUnionArms.h
 *     Which member of the Thing union is in use. The save tables list a table for each member and pick one with
 *     the selector function registered under the name "thing_arm".
 */
/******************************************************************************/
#ifndef KFX_SAVEUNIONARMS_H
#define KFX_SAVEUNIONARMS_H

#include "globals.h"
#include "thing_data.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "thing_shots.h"
#include "thing_effects.h"
#include "config_objects.h"
#include "kfx/save/core/save_schema.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** The members of the Thing union, as the values the selector returns. Members with the same layout share one. */
enum SaveThingArm {
    /** Not a member: the class isn't known to the tables, so the bytes are stored as they are. */
    STA_Unknown = 0,
    /** The thing's class keeps nothing in the union. */
    STA_None,
    /** An object that keeps at most a byte or two: custom_box, call_to_arms_flag, heart, hero_gate or lightning. */
    STA_ObjectBytes,
    /** An object that holds gold. */
    STA_Valuable,
    /** An object that is food. */
    STA_Food,
    /** A lair or a torture device: lair and torturer have the same layout. */
    STA_Lair,
    /** A spell object: armor and disease have the same layout. */
    STA_Armor,
    /** The status flag of a room. */
    STA_RoomFlag,
    STA_Shot,
    STA_ShotLizard,
    STA_Corpse,
    STA_Creature,
    STA_ShotEffect,
    /** The number shown by a price tag. */
    STA_PriceEffect,
    STA_EffectGenerator,
    STA_Trap,
    STA_Door,
    STA_CaveIn
};

/** The member of an object's union that its model needs. genre is the object's config genre (OCtg_*), or -1 when
 *  there is no config to ask, as in the tools and tests: then only the models the game itself names are known. */
static inline int save_object_arm(ThingModel model, int genre)
{
    switch (model)
    {
    case ObjMdl_GoldChest: case ObjMdl_GoldPot: case ObjMdl_Goldl: case ObjMdl_GoldBag:
    case ObjMdl_GoldHoard1: case ObjMdl_GoldHoard2: case ObjMdl_GoldHoard3: case ObjMdl_GoldHoard4: case ObjMdl_GoldHoard5:
        return STA_Valuable;
    case ObjMdl_ChickenGrowing: case ObjMdl_ChickenMature: case ObjMdl_ChickenStb: case ObjMdl_ChickenWob:
    case ObjMdl_ChickenCrk:
        return STA_Food;
    case ObjMdl_Torturer:
        return STA_Lair;
    case ObjMdl_LightBall: case ObjMdl_Disease:
        return STA_Armor;
    case ObjMdl_RoomFlag:
        return STA_RoomFlag;
    default:
        break;
    }
    if ((genre == OCtg_Valuable) || (genre == OCtg_GoldHoard))
        return STA_Valuable;
    if (genre == OCtg_Food)
        return STA_Food;
    if (genre == OCtg_LairTotem)
        return STA_Lair;
    return STA_ObjectBytes;
}

/** The member of a thing's union that is in use. object_genre is as for save_object_arm. */
static inline int save_thing_arm(const struct Thing *thing, int object_genre)
{
    switch (thing->class_id)
    {
    case TCls_Empty: case TCls_unusedparam10: case TCls_unusedparam11: case TCls_AmbientSnd:
        return STA_None;
    case TCls_EffectElem:
        return (thing->model == TngEffElm_Price) ? STA_PriceEffect : STA_None;
    case TCls_Object:
        return save_object_arm(thing->model, object_genre);
    case TCls_Shot:
        return (thing->model == ShM_Lizard) ? STA_ShotLizard : STA_Shot;
    case TCls_DeadCreature:
        return STA_Corpse;
    case TCls_Creature:
        return STA_Creature;
    case TCls_Effect:
        return STA_ShotEffect;
    case TCls_EffectGen:
        return STA_EffectGenerator;
    case TCls_Trap:
        return STA_Trap;
    case TCls_Door:
        return STA_Door;
    case TCls_CaveIn:
        return STA_CaveIn;
    default:
        return STA_Unknown;
    }
}

/** The selector for programs that have no game config (the tools and the tests). */
static inline int64_t save_thing_arm_without_config(const void *owner)
{
    /* cppcheck-suppress cstyleCast */
    return save_thing_arm((const struct Thing *)owner, -1);
}

static inline void save_register_standalone_selectors(void)
{
    save_register_union_selector("thing_arm", save_thing_arm_without_config);
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
