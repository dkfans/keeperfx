/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_objects.c
 *     Things of class 'object' handling functions.
 * @par Purpose:
 *     Functions to maintain object things in the game.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     05 Nov 2009 - 01 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "thing_objects.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_sound.h"
#include "bflib_planar.h"

#include "config_strings.h"
#include "config_objects.h"
#include "config_terrain.h"
#include "config_creature.h"
#include "config_effects.h"
#include "config_magic.h"
#include "thing_stats.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "power_hand.h"
#include "player_instances.h"
#include "map_data.h"
#include "map_columns.h"
#include "map_utils.h"
#include "magic_powers.h"
#include "room_entrance.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "engine_arrays.h"
#include "sounds.h"
#include "creature_states_pray.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "game_loop.h"
#include "config_spritecolors.h"
#include "lua_cfg_funcs.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static long food_moves(struct Thing *objtng);
static long food_grows(struct Thing *objtng);
static long object_being_dropped(struct Thing *objtng);
static TngUpdateRet object_update_dungeon_heart(struct Thing *heartng);
static TngUpdateRet object_update_call_to_arms(struct Thing *objtng);
static TngUpdateRet object_update_armour(struct Thing *objtng);
static TngUpdateRet object_update_object_scale(struct Thing *objtng);
static TngUpdateRet object_update_power_sight(struct Thing *objtng);
static TngUpdateRet object_update_power_lightning(struct Thing *objtng);

static Thing_State_Func object_state_functions[] = {
    NULL,
    food_moves,
    food_grows,
    NULL,
    object_being_dropped,
    NULL,
};

const struct NamedCommand object_update_functions_desc[] = {
  {"UPDATE_DUNGEON_HEART",   1},
  {"UPDATE_CALL_TO_ARMS",    2},
  {"UPDATE_ARMOUR",          3},
  {"UPDATE_OBJECT_SCALE",    4},
  {"UPDATE_POWER_SIGHT",     5},
  {"UPDATE_POWER_LIGHTNING", 6},
  {"NULL",                   0},
  {NULL,                  0},
  };

static Thing_Class_Func object_update_functions[] = {
    NULL,
    object_update_dungeon_heart,
    object_update_call_to_arms,
    object_update_armour,
    object_update_object_scale,
    object_update_power_sight,
    object_update_power_lightning,
};

unsigned short lightning_spangles[] =   {TngEffElm_RedTwinkle3, TngEffElm_BlueTwinke2, TngEffElm_GreenTwinkle2, TngEffElm_YellowTwinkle2, TngEffElm_WhiteTwinkle2, TngEffElm_None,TngEffElm_PurpleTwinkle2,TngEffElm_BlackTwinkle2,TngEffElm_OrangeTwinkle2,};
unsigned short twinkle_eff_elements[] = {TngEffElm_RedTwinkle,  TngEffElm_BlueTwinkle, TngEffElm_GreenTwinkle,  TngEffElm_YellowTwinkle,  TngEffElm_WhiteTwinkle,  TngEffElm_None,TngEffElm_PurpleTwinkle, TngEffElm_BlackTwinkle, TngEffElm_OrangeTwinkle, };

unsigned short gold_hoard_objects[] = {ObjMdl_GoldHoard1, ObjMdl_GoldHoard2, ObjMdl_GoldHoard3, ObjMdl_GoldHoard4, ObjMdl_GoldHoard5};
unsigned short food_grow_objects[] = {ObjMdl_ChickenStb, ObjMdl_ChickenWob, ObjMdl_ChickenCrk};

struct CallToArmsGraphics call_to_arms_graphics[10];

/******************************************************************************/
struct Thing *create_object(const struct Coord3d *pos, ThingModel model, unsigned short owner, long parent_idx)
{
    long i;
    long start_frame;

    if (!i_can_allocate_free_thing_structure(TCls_Object))
    {
        ERRORDBG(3,"Cannot create object model %d (%s) for player %d. There are too many things allocated.",(int)model,object_code_name(model),(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    struct Thing* thing = allocate_free_thing_structure(TCls_Object);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate object %d (%s) for player %d, but failed.",(int)model,object_code_name(model),(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = TCls_Object;
    thing->model = model;
    if (parent_idx == -1)
      thing->parent_idx = -1;
    else
      thing->parent_idx = parent_idx;
    memcpy(&thing->mappos, pos, sizeof(struct Coord3d));
    struct ObjectConfigStats* objst = get_object_model_stats(model);
    thing->clipbox_size_xy = objst->size_xy;
    thing->clipbox_size_z = objst->size_z;
    thing->solid_size_xy = objst->size_xy;
    thing->solid_size_z = objst->size_z;
    thing->anim_speed = objst->anim_speed;
    thing->anim_sprite = objst->sprite_anim_idx;
    thing->health = saturate_set_signed(objst->health,32);
    thing->fall_acceleration = objst->fall_acceleration;
    thing->inertia_floor = 204;
    thing->inertia_air = 51;
    thing->bounce_angle = 0;
    thing->movement_flags |= TMvF_ZeroVerticalVelocity;

    set_flag_value(thing->movement_flags, TMvF_Immobile, objst->immobile);
    thing->owner = owner;
    thing->creation_turn = game.play_gameturn;

    if (!objst->random_start_frame)
    {
      i = convert_td_iso(objst->sprite_anim_idx);
      start_frame = 0;
    } else
    {
      i = convert_td_iso(objst->sprite_anim_idx);
      start_frame = -1;
    }
    set_thing_draw(thing, i, objst->anim_speed, objst->sprite_size_max, 0, start_frame, objst->draw_class);
    set_flag_value(thing->rendering_flags, TRF_Unshaded, objst->light_unaffected);

    set_flag(thing->rendering_flags, objst->transparency_flags);

    thing->active_state = objst->initial_state;
    if (objst->ilght.radius != 0)
    {
        struct InitLight ilight;
        memset(&ilight, 0, sizeof(struct InitLight));
        memcpy(&ilight.mappos, &thing->mappos, sizeof(struct Coord3d));
        ilight.radius = objst->ilght.radius;
        ilight.intensity = objst->ilght.intensity;
        ilight.flags = objst->ilght.flags;
        ilight.is_dynamic = objst->ilght.is_dynamic;
        thing->light_id = light_create_light(&ilight);
        if (thing->light_id == 0) {
            SYNCDBG(8,"Cannot allocate light to %s",thing_model_name(thing));
        }
    } else {
        thing->light_id = 0;
    }
    if (thing_is_beating_dungeon_heart(thing))
    {
        thing->heart.beat_direction = 1;
        light_set_light_minimum_size_to_cache(thing->light_id, 0, 56);
    }
    switch (thing->model)
    {
      case ObjMdl_TempleSpangle: // Why it is hardcoded? And what is TempleS
        thing->rendering_flags &= TRF_Transpar_Flags;
        thing->rendering_flags |= TRF_Transpar_4;
        break;
      case ObjMdl_GoldChest:
      case ObjMdl_GoldPot:
      case ObjMdl_Goldl:
      case ObjMdl_GoldBag:
        thing->valuable.gold_stored = gold_object_typical_value(thing);
        break;
      case ObjMdl_SpinningKey:
        if ((thing->mappos.z.stl.num == 4) && (subtile_is_door(thing->mappos.x.stl.num, thing->mappos.y.stl.num)))
        {
            thing->mappos.z.stl.num = 5; // Move keys from old maps from inside to on top of the doors.
            thing->mappos.z.stl.pos = 0;
        }
        break;
      default:
        break;
    }
    if (objst->genre == OCtg_HeroGate)
    {
        i = get_free_hero_gate_number();
        if (i > 0)
        {
            thing->hero_gate.number = i;
        }
        else
        {
            thing->hero_gate.number = 0;
            ERRORLOG("Could not allocate number for hero gate");
        }
    }

    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);

    thing->flags = 0;
    thing->flags |= objst->rotation_flag << TAF_ROTATED_SHIFT;

    return thing;
}

void destroy_food(struct Thing *foodtng)
{
    SYNCDBG(8,"Starting");
    PlayerNumber plyr_idx = foodtng->owner;
    if (game.neutral_player_num != plyr_idx) {
        struct Dungeon* dungeon = get_dungeon(plyr_idx);
        dungeon->lvstats.chickens_wasted++;
    }
    struct Coord3d pos;
    pos.x.val = foodtng->mappos.x.val;
    pos.y.val = foodtng->mappos.y.val;
    pos.z.val = foodtng->mappos.z.val + 256;
    if (object_is_mature_food(foodtng))
    {
        struct Thing* efftng = create_effect(&foodtng->mappos, TngEff_FeatherPuff, plyr_idx);
        if (!thing_is_invalid(efftng))
        {
            thing_play_sample(efftng, 112 + SOUND_RANDOM(3), NORMAL_PITCH, 0, 3, 0, 2, FULL_LOUDNESS);
        }
    }
    create_effect(&pos, TngEff_ChickenBlood, plyr_idx);
    struct Room* room = get_room_thing_is_on(foodtng);
    if (!room_is_invalid(room))
    {
        if (room_role_matches(room->kind, RoRoF_FoodSpawn) && (room->owner == foodtng->owner))
        {
            int required_cap = get_required_room_capacity_for_object(RoRoF_FoodStorage, foodtng->model, 0);
            if (room->used_capacity >= required_cap)
            {
                room->used_capacity -= required_cap;
            }
            foodtng->food.life_remaining = game.conf.rules[plyr_idx].game.food_life_out_of_hatchery;
        }
    }
    delete_thing_structure(foodtng, 0);
}

void destroy_object(struct Thing *thing)
{
    if (object_is_mature_food(thing) || object_is_growing_food(thing))
    {
        destroy_food(thing);
    } else
    {
        delete_thing_structure(thing, 0);
    }
}

TbBool object_can_be_damaged (const struct Thing* thing)
{
    //todo make this an object property. Then include the possibility to kill the other object types.
    if (thing->class_id != TCls_Object)
        return false;
    if (thing_is_dungeon_heart(thing) || object_is_mature_food(thing) || object_is_growing_food(thing))
        return true;
    return false;
}

TbBool thing_is_object_with_tooltip(const struct Thing* thing, TbBool is_optional)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return ((objst->tooltip_stridx != GUIStr_Empty) && (objst->tooltip_optional == is_optional));
}
TbBool thing_is_object_with_mandatory_tooltip(const struct Thing* thing)
{
    return thing_is_object_with_tooltip(thing, 0);
}
TbBool thing_is_object_with_optional_tooltip(const struct Thing* thing)
{
    return thing_is_object_with_tooltip(thing, 1);
}

TbBool thing_is_object(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
      return false;
    if (thing->class_id != TCls_Object)
      return false;
    return true;
}

void change_object_owner(struct Thing *objtng, PlayerNumber nowner)
{
    //TODO make this function more advanced - switch object types and update dungeon and rooms for spellbook/workshop box/lair
    SYNCDBG(6,"Starting for %s, owner %d to %d",thing_model_name(objtng),(int)objtng->owner,(int)nowner);
    objtng->owner = nowner;
}

/**
 * Gives power kind associated with given spellbook thing.
 * @param thing The spellbook object thing.
 * @return Power kind, or 0 if the thing is not a spellbook object.
 */
PowerKind book_thing_to_power_kind(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return 0;
    if ( (thing->class_id != TCls_Object) || (thing->model >= game.conf.object_conf.object_types_count) )
        return 0;
    return game.conf.object_conf.object_to_power_artifact[thing->model];
}

TbBool thing_is_special_box(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_SpecialBox);
}

TbBool thing_is_hardcoded_special_box(const struct Thing* thing)
{
    if (thing->class_id != TCls_Object)
        return false;
    switch (thing->model)
    {
    case ObjMdl_SpecboxRevealMap:
    case ObjMdl_SpecboxResurect:
    case ObjMdl_SpecboxTransfer:
    case ObjMdl_SpecboxStealHero:
    case ObjMdl_SpecboxMultiply:
    case ObjMdl_SpecboxIncreaseLevel:
    case ObjMdl_SpecboxMakeSafe:
    case ObjMdl_SpecboxMakeUnsafe:
    case ObjMdl_SpecboxHiddenWorld:
    case ObjMdl_SpecboxHealAll:
    case ObjMdl_SpecboxGetGold:
    case ObjMdl_SpecboxMakeAngry:
        return true;
    default:
        return false;
    }
}

TbBool thing_is_custom_special_box(const struct Thing* thing)
{
    return (thing_is_special_box(thing) && !thing_is_hardcoded_special_box(thing));
}

TbBool thing_is_workshop_crate(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_WrkshpBox);
}

TbBool thing_is_trap_crate(const struct Thing *thing)
{
    return (crate_thing_to_workshop_item_class(thing) == TCls_Trap);
}

TbBool thing_is_door_crate(const struct Thing *thing)
{
    return (crate_thing_to_workshop_item_class(thing) == TCls_Door);
}

TbBool thing_is_dungeon_heart(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return (objst->model_flags & OMF_Heart);
}

TbBool thing_is_beating_dungeon_heart(const struct Thing* thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return ((objst->model_flags & (OMF_Beating | OMF_Heart)) == (OMF_Beating | OMF_Heart));
}

TbBool thing_is_mature_food(const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    return (thing->class_id == TCls_Object) && (thing->model == ObjMdl_ChickenMature);
}

TbBool object_is_buoyant(const struct Thing* thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return (objst->model_flags & OMF_Buoyant);
}

TbBool thing_is_spellbook(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_Spellbook);
}

TbBool thing_is_lair_totem(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_LairTotem);
}

TbBool object_is_hero_gate(const struct Thing *thing)
{
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_HeroGate);
}

TbBool object_is_infant_food(const struct Thing *thing)
{
  return (thing->model == food_grow_objects[0]) || (thing->model == food_grow_objects[1]) || (thing->model == food_grow_objects[2]);
}

TbBool object_is_growing_food(const struct Thing *thing)
{
  return (thing->model == ObjMdl_ChickenGrowing);
}

TbBool object_is_mature_food(const struct Thing *thing)
{
  return (thing->model == ObjMdl_ChickenMature);
}

TbBool object_is_gold(const struct Thing *thing)
{
    return object_is_gold_pile(thing) || object_is_gold_hoard(thing);
}

/**
 * Returns if given thing is a gold hoard.
 * Gold hoards may only exist in treasure rooms.
 * @param thing
 * @return
 */
TbBool object_is_gold_hoard(const struct Thing *thing)
{
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return (objst->genre == OCtg_GoldHoard);
}

TbBool object_is_gold_pile(const struct Thing *thing)
{
    if (thing->class_id != TCls_Object)
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return ((objst->genre == OCtg_Valuable) || (thing->model == ObjMdl_SpinningCoin));
}

TbBool object_is_gold_laying_on_ground(const struct Thing *thing)
{
    return (thing->model == ObjMdl_Goldl);
}

/**
 * Returns if given thing is a guardpost flag.
 * @param thing
 * @return
 */
TbBool object_is_guard_flag(const struct Thing *thing)
{
    switch (thing->model)
    {
      case ObjMdl_GuardFlagRed:
      case ObjMdl_GuardFlagBlue:
      case ObjMdl_GuardFlagGreen:
      case ObjMdl_GuardFlagYellow:
      case ObjMdl_GuardFlagWhite:
      case ObjMdl_GuardFlagPurple:
      case ObjMdl_GuardFlagBlack:
      case ObjMdl_GuardFlagOrange:
      case ObjMdl_GuardFlagPole:
          return true;
      default:
          return false;
    }
}

/**
 * Returns if given object thing is a room equipment.
 * Equipment are the objects which room always has, put there for the lifetime of a room.
 * @param thing
 * @return
 */
TbBool object_is_room_equipment(const struct Thing *thing, RoomKind rkind)
{
    switch (rkind)
    {
    case RoK_ENTRANCE:
        // No objects
        return false;
    case RoK_TREASURE:
        return (thing->model == ObjMdl_Candlestick);
    case RoK_LIBRARY:
        // No objects
        return false;
    case RoK_PRISON:
        return (thing->model == ObjMdl_PrisonBar);
    case RoK_TORTURE:
        return (thing->model == ObjMdl_TortureSpike) || (thing->model == ObjMdl_Torturer);
    case RoK_TRAINING:
        return (thing->model == ObjMdl_TrainingPost) || (thing->model == ObjMdl_Torch);
    case RoK_DUNGHEART:
        return (thing->model == ObjMdl_HeartFlameRed) || (thing->model == ObjMdl_HeartFlameBlue) || (thing->model == ObjMdl_HeartFlameGreen) || (thing->model == ObjMdl_HeartFlameYellow);
    case RoK_WORKSHOP:
        return (thing->model == ObjMdl_WorkshopMachine) || (thing->model == ObjMdl_Anvil);
    case RoK_SCAVENGER:
        // Scavenge eye and torch
        return (thing->model == ObjMdl_ScavangeEye) || (thing->model == ObjMdl_Torch);
    case RoK_TEMPLE:
        // Temple statue
        return (thing->model == ObjMdl_StatueLit);
    case RoK_GRAVEYARD:
        // Gravestone and torch
        return (thing->model == ObjMdl_Gravestone) || (thing->model == ObjMdl_Torch);
    case RoK_BARRACKS:
        // No break
    case RoK_GARDEN:
        // Torch
        return (thing->model == ObjMdl_Torch);
    case RoK_LAIR:
        // No objects
        return false;
    case RoK_BRIDGE:
        // No objects
        return false;
    case RoK_GUARDPOST:
        // Guard flags
        return object_is_guard_flag(thing);
    default:
        return false;
    }
}

/**
 * Returns if given object thing is a room inventory.
 * Inventory are the objects which can be stored in a room, but are movable and optional.
 * @param thing
 * @return
 */
TbBool object_is_room_inventory(const struct Thing *thing, RoomRole rrole)
{

    if((rrole & RoRoF_GoldStorage) && object_is_gold_hoard(thing))
        return true;
    if((rrole & RoRoF_PowersStorage) && (thing_is_spellbook(thing) || thing_is_special_box(thing)))
        return true;
    if((rrole & RoRoF_KeeperStorage) && thing_is_dungeon_heart(thing))
        return true;
    if((rrole & RoRoF_CratesStorage) && thing_is_workshop_crate(thing))
        return true;
    if((rrole & RoRoF_FoodStorage) && (object_is_infant_food(thing) || object_is_growing_food(thing) || object_is_mature_food(thing)))
        return true;
    if((rrole & RoRoF_LairStorage) && thing_is_lair_totem(thing))
        return true;

    return false;
}

/**
 * Checks if thing is an object with the OMF_IgnoredByImps flag.
 */
TbBool object_is_ignored_by_imps(const struct Thing* thing)
{
    if (!thing_is_object(thing))
        return false;
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return flag_is_set(objst->model_flags, OMF_IgnoredByImps);
}

TbBool creature_remove_lair_totem_from_room(struct Thing *creatng, struct Room *room)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (cctrl->lair_room_id != room->index)
    {
        ERRORLOG("Attempt to remove a lair which belongs to %s index %d from room index %d he didn't think he was in",thing_model_name(creatng),(int)creatng->index,(int)room->index);
        return false;
    }
    TbBool result = true;
    int required_cap = get_required_room_capacity_for_object(RoRoF_LairStorage, 0, creatng->model);
    // Remove lair from room capacity
    if (room->content_per_model[creatng->model] <= 0)
    {
        ERRORLOG("Attempt to remove a lair which belongs to %s index %d from room index %d not containing this creature model",thing_model_name(creatng),(int)creatng->index,(int)room->index);
        result = false;
    } else
    if ( room->used_capacity < required_cap)
    {
        ERRORLOG("Attempt to remove creature lair from room with too little used space");
        result = false;
    } else
    {
        room->used_capacity -= required_cap;
        room->content_per_model[creatng->model]--;
    }
    cctrl->lair_room_id = 0;
    //Remove the totem thing
    if (cctrl->lairtng_idx > 0)
    {
        struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
        TRACE_THING(lairtng);
        create_effect(&lairtng->mappos, imp_spangle_effects[get_player_color_idx(creatng->owner)], creatng->owner);
        delete_lair_totem(lairtng);
    }
    return result;
}

TbBool delete_lair_totem(struct Thing *lairtng)
{
    struct Thing* creatng = thing_get(lairtng->lair.belongs_to);
    if (thing_is_creature(creatng)) {
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        cctrl->lair_room_id = 0;
        cctrl->lairtng_idx = 0;
    } else {
        ERRORLOG("No totem owner");
    }
    delete_thing_structure(lairtng, 0);
    return true;
}

static long food_moves(struct Thing *objtng)
{
    struct Coord3d pos;
    pos.x.val = objtng->mappos.x.val;
    pos.y.val = objtng->mappos.y.val;
    pos.z.val = objtng->mappos.z.val;
    unsigned int snd_smplidx = 0;
    if (objtng->food.some_chicken_was_sacrificed)
    {
        destroy_food(objtng);
        return -1;
    }
    TbBool dirct_ctrl = is_thing_directly_controlled(objtng);
    if (dirct_ctrl)
    {
        if (objtng->food.possession_startup_timer > 0)
        {
            objtng->food.possession_startup_timer--;
            return 1;
        }
    }
    struct Room* room = get_room_thing_is_on(objtng);
    if (!dirct_ctrl)
    {
      if (objtng->food.life_remaining >= 0)
      {
        if (!room_is_invalid(room) && !is_neutral_thing(objtng) && (objtng->food.life_remaining != -1))
        {
            if (room_role_matches(room->kind, RoRoF_FoodSpawn) && (room->owner == objtng->owner) && (room->total_capacity > room->used_capacity))
            {
                room->used_capacity++;
                objtng->food.life_remaining = -1;
                objtng->parent_idx = room->index;
            }
        }
      }
      else
      {
            if ( (room_is_invalid(room)) || (!room_role_matches(room->kind, RoRoF_FoodStorage)) || (room->owner != objtng->owner) || (room->used_capacity > room->total_capacity) )
            {
                objtng->food.life_remaining = game.conf.rules[objtng->owner].game.food_life_out_of_hatchery;
                struct Room* hatchroom = room_get(objtng->parent_idx);
                if (!room_is_invalid(hatchroom))
                {
                    if (room_role_matches(hatchroom->kind, RoRoF_FoodStorage))
                    {
                        update_room_contents(hatchroom);
                    }
                }
                objtng->parent_idx = -1;
            }
      }
      if (objtng->food.life_remaining >= 0)
      {
        objtng->food.life_remaining--;
        if (objtng->food.life_remaining <= 0)
        {
            if (objtng->owner != game.neutral_player_num)
            {
                struct Dungeon* dungeon = get_dungeon(objtng->owner);
                dungeon->lvstats.chickens_wasted++;
            }
            create_effect(&objtng->mappos, TngEff_FeatherPuff, objtng->owner);
            create_effect(&objtng->mappos, TngEff_ChickenBlood, objtng->owner);
            delete_thing_structure(objtng, 0);
            return -1;
        }
      }
    }
    TbBool has_near_creature = false;
    if (!room_is_invalid(room) && (room_role_matches(room->kind, RoRoF_FoodStorage)) && (objtng->food.life_remaining < 0))
    {
        objtng->parent_idx = room->index;
        struct Thing* near_creatng;
        if (room->hatch_gameturn == game.play_gameturn)
        {
            near_creatng = thing_get(room->cached_nearby_creature_index);
        } else
        {
            room->hatch_gameturn = game.play_gameturn;
            near_creatng = get_nearest_thing_of_class_and_model_owned_by(pos.x.val, pos.y.val, -1, TCls_Creature, -1);
            if (!thing_is_invalid(near_creatng))
                room->cached_nearby_creature_index = near_creatng->index;
        }
        has_near_creature = (thing_exists(near_creatng) && (get_chessboard_distance(&objtng->mappos, &near_creatng->mappos) < 768));
        if (has_near_creature)
        {
            objtng->food.angle = get_angle_xy_to(&near_creatng->mappos, &pos);
            if (objtng->snd_emitter_id == 0)
            {
                if (SOUND_RANDOM(16) == 0) {
                  snd_smplidx = 109 + SOUND_RANDOM(3);
                }
            }
        }
    }
    if (objtng->food.freshness_state <= 0)
    {
        if (objtng->food.freshness_state == 0)
        {
            objtng->food.freshness_state = -1;
            set_thing_draw(objtng, 820, -1, -1, -1, 0, ODC_Default);
            if (dirct_ctrl) {
                objtng->food.possession_startup_timer = 6;
            } else {
                objtng->food.possession_startup_timer = THING_RANDOM(objtng ,4) + 1;
            }
        }
        if ((has_near_creature && (objtng->food.possession_startup_timer < 5)) || (objtng->food.possession_startup_timer == 0))
        {
            set_thing_draw(objtng, 819, -1, -1, -1, 0, ODC_Default);
            objtng->food.freshness_state = THING_RANDOM(objtng, 0x39);
            objtng->food.angle = THING_RANDOM(objtng, ANGLE_MASK);
            objtng->food.possession_startup_timer = 0;
        } else
        if ((objtng->anim_speed * objtng->max_frames <= objtng->anim_speed + objtng->anim_time) && (objtng->food.possession_startup_timer < 5))
        {
            objtng->food.possession_startup_timer--;
        }
    }
    else
    {
        int vel_x = 32 * LbSinL(objtng->food.angle) >> 16;
        pos.x.val += vel_x;
        int vel_y = -(32 * LbCosL(objtng->food.angle) >> 8) >> 8;
        pos.y.val += vel_y;
        if (thing_in_wall_at(objtng, &pos))
        {
            objtng->food.angle = THING_RANDOM(objtng, ANGLE_MASK);
        }
        long dangle = get_angle_difference(objtng->move_angle_xy, objtng->food.angle);
        int sangle = get_angle_sign(objtng->move_angle_xy, objtng->food.angle);
        if (dangle > 62)
            dangle = 62;
        objtng->move_angle_xy = (objtng->move_angle_xy + dangle * sangle) & ANGLE_MASK;
        if (get_angle_difference(objtng->move_angle_xy, objtng->food.angle) < DEGREES_50)
        {
            struct ComponentVector cvec;
            cvec.x = vel_x;
            cvec.y = vel_y;
            cvec.z = 0;
            objtng->food.freshness_state--;
            apply_transitive_velocity_to_thing(objtng, &cvec);
        }
        if (objtng->snd_emitter_id == 0)
        {
            if (snd_smplidx > 0) {
              thing_play_sample(objtng, snd_smplidx, 100, 0, 3u, 0, 1, 256);
              return 1;
            }
            if (SOUND_RANDOM(0x50) == 0)
            {
              snd_smplidx = 100 + SOUND_RANDOM(9);
            }
        }
    }
    if (snd_smplidx > 0) {
        thing_play_sample(objtng, snd_smplidx, 100, 0, 3u, 0, 1, 256);
        return 1;
    }
    return 1;
}

static long food_grows(struct Thing *objtng)
{
    if (objtng->food.life_remaining > 0)
    {
        objtng->food.life_remaining--;
        return 1;
    }
    struct Coord3d pos;
    pos.x.val = objtng->mappos.x.val;
    pos.y.val = objtng->mappos.y.val;
    pos.z.val = objtng->mappos.z.val;
    long ret = 0;
    PlayerNumber tngowner = objtng->owner;
    struct Thing* nobjtng;
    struct Room* room = subtile_room_get(pos.x.stl.num, pos.y.stl.num);
    short room_idx = (!room_is_invalid(room)) ? room->index : -1;
    switch (objtng->anim_sprite)
    {
      case 893:
      case 897:
        delete_thing_structure(objtng, 0);
        nobjtng = create_object(&pos, food_grow_objects[0], tngowner, room_idx);
        if (!thing_is_invalid(nobjtng)) {
            nobjtng->food.life_remaining = (nobjtng->max_frames << 8) / nobjtng->anim_speed - 1;
        }
        ret = -1;
        break;
      case 894:
      case 898:
        delete_thing_structure(objtng, 0);
        nobjtng = create_object(&pos, food_grow_objects[1], tngowner, room_idx);
        if (!thing_is_invalid(nobjtng)) {
            nobjtng->food.life_remaining = 3 * ((nobjtng->max_frames << 8) / nobjtng->anim_speed - 1);
        }
        ret = -1;
        break;
      case 895:
      case 899:
        delete_thing_structure(objtng, 0);
        nobjtng = create_object(&pos, food_grow_objects[2], tngowner, room_idx);
        if (!thing_is_invalid(nobjtng)) {
            nobjtng->food.life_remaining = (nobjtng->max_frames << 8) / nobjtng->anim_speed - 1;
        }
        ret = -1;
        break;
      case 896:
      case 900:
        delete_thing_structure(objtng, 0);
        nobjtng = create_object(&pos, ObjMdl_ChickenMature, tngowner, room_idx);
        if (!thing_is_invalid(nobjtng)) {
            nobjtng->move_angle_xy = THING_RANDOM(objtng, DEGREES_360);
            nobjtng->food.freshness_state = THING_RANDOM(objtng, 0x6FF);
            nobjtng->food.possession_startup_timer = 0;
          thing_play_sample(nobjtng, 80 + SOUND_RANDOM(3), 100, 0, 3u, 0, 1, 64);
          if (!is_neutral_thing(nobjtng)) {
              struct Dungeon *dungeon;
              dungeon = get_dungeon(nobjtng->owner);
              dungeon->lvstats.chickens_hatched++;
          }
          nobjtng->food.life_remaining = -1;
        }
        ret = -1;
        break;
      default:
        break;
    }
    return ret;
}

GoldAmount add_gold_to_treasure_room_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, GoldAmount gold_store)
{
    struct Room* room = subtile_room_get(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    struct Thing* gldtng = find_gold_hoard_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (thing_is_invalid(gldtng))
    {
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
        pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
        pos.z.val = get_floor_height_at(&pos);
        gldtng = create_gold_hoarde(room, &pos, gold_store);
        if (!thing_is_invalid(gldtng)) {
            gold_store -= gldtng->valuable.gold_stored;
        }
    } else
    {
        gold_store -= add_gold_to_hoarde(gldtng, room, gold_store);
    }
    return gold_store;
}

long gold_being_dropped_at_treasury(struct Thing *thing, struct Room *room)
{
    GoldAmount gold_store = thing->valuable.gold_stored;
    {
        MapSlabCoord slb_x = coord_slab(thing->mappos.x.val);
        MapSlabCoord slb_y = coord_slab(thing->mappos.y.val);
        gold_store = add_gold_to_treasure_room_slab(slb_x, slb_y, gold_store);
    }
    unsigned long k;
    long n = THING_RANDOM(thing, room->slabs_count);
    SlabCodedCoords slbnum = room->slabs_list;
    for (k = n; k > 0; k--)
    {
        if (slbnum == 0)
            break;
        slbnum = get_next_slab_number_in_room(slbnum);
    }
    if (slbnum == 0) {
        ERRORLOG("Taking random slab (%d/%d) in %s index %d failed - internal inconsistency.",(int)n,(int)room->slabs_count,room_code_name(room->kind),(int)room->index);
        slbnum = room->slabs_list;
    }
    k = 0;
    while (1)
    {
        MapSlabCoord slb_x = slb_num_decode_x(slbnum);
        MapSlabCoord slb_y = slb_num_decode_y(slbnum);
        // Per slab code
        if (gold_store <= 0)
            break;
        gold_store = add_gold_to_treasure_room_slab(slb_x, slb_y, gold_store);
        // Per slab code ends
        slbnum = get_next_slab_number_in_room(slbnum);
        if (slbnum == 0) {
            slbnum = room->slabs_list;
        }
        k++;
        if (k >= room->slabs_count) {
            break;
        }
    }
    thing->valuable.gold_stored = gold_store;
    if (thing->valuable.gold_stored <= 0)
    {
        delete_thing_structure(thing, 0);
        return -1;
    }
    return 0;
}

TbBool temple_check_for_arachnid_join_dungeon(struct Dungeon *dungeon)
{
    if ((dungeon->chickens_sacrificed % 16) == 0)
    {
        ThingModel crmodel = get_creature_model_with_model_flags(CMF_IsArachnid);
        ThingModel spdigmodel = get_players_special_digger_model(dungeon->owner);
        if ((dungeon->gold_piles_sacrificed == 4) &&
            (dungeon->creature_sacrifice[spdigmodel] == 4) &&
            (dungeon->owned_creatures_of_model[crmodel] < 4))
        {
            SYNCLOG("Conditions to trigger arachnid met");
            struct Room* room = pick_random_room_of_role(dungeon->owner, RoRoF_CrPoolSpawn);
            if (room_is_invalid(room))
            {
                ERRORLOG("Could not get a random entrance for player %d",(int)dungeon->owner);
                return false;
            }
            struct Thing* ncreatng = create_creature_at_entrance(room, crmodel);
            set_creature_level(ncreatng, THING_RANDOM(ncreatng, CREATURE_MAX_LEVEL));
            return true;
        }
    }
    return false;
}

long process_temple_special(struct Thing *thing, long sacowner)
{
    struct Dungeon* dungeon = get_dungeon(sacowner);
    if (object_is_mature_food(thing))
    {
        dungeon->chickens_sacrificed++;
        if (temple_check_for_arachnid_join_dungeon(dungeon) && (game.easter_eggs_enabled == true))
            return true;
    } else
    {
        dungeon->gold_piles_sacrificed++;
    }
    return false;
}

void process_object_sacrifice(struct Thing *thing, long sacowner)
{
    PlayerNumber slbowner;
    {
        struct SlabMap* slb = get_slabmap_thing_is_on(thing);
        slbowner = slabmap_owner(slb);
    }
    if (object_is_mature_food(thing))
    {
        process_temple_special(thing, sacowner);
        kill_all_players_chickens(thing->owner);
        if (is_my_player_number(sacowner))
            output_message(SMsg_SacrificePunish, 0);
    } else
    if (object_is_gold_pile(thing))
    {
        if (thing->valuable.gold_stored > 0)
        {
            process_temple_special(thing, sacowner);
            int num_allies = 0;
            PlayerNumber plyr_idx;
            for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
            {
                if ((slbowner != plyr_idx) && players_are_mutual_allies(slbowner, plyr_idx))
                {
                    num_allies++;
                }
            }
            if (num_allies > 0)
            {
                GoldAmount value = thing->valuable.gold_stored / num_allies;
                for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
                {
                    if ((slbowner != plyr_idx) && players_are_mutual_allies(slbowner, plyr_idx))
                    {
                        player_add_offmap_gold(plyr_idx, value);
                    }
                }
            } else
            {
                if (is_my_player_number(sacowner))
                    output_message(SMsg_SacrificeWishing, 0);
            }
        }
    }
}


/**
 * Finds a thing with the same location, class and model as the provided thing
 * @param thing The thing you want to find something similar to.
 * @return other thing that matches location, class and model
 */
struct Thing *find_base_thing_on_mapwho_excluding_self(struct Thing *thing)
{
    struct Map* mapblk = get_map_block_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* result = thing_get(i);
        TRACE_THING(result);
        if (thing_is_invalid(result))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = result->next_on_mapblk;
        // Per thing code start
        if (result->class_id == thing->class_id && thing->model == result->model && thing != result)
        {
            return result;
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return INVALID_THING;
}

static long object_being_dropped(struct Thing *thing)
{
    if (!thing_touching_floor(thing)) {
        return 1;
    }
    if (subtile_has_sacrificial_on_top(thing->mappos.x.stl.num, thing->mappos.y.stl.num))
    {
        struct Room* room = get_room_thing_is_on(thing);
        process_object_sacrifice(thing, room->owner);
        delete_thing_structure(thing, 0);
        return -1;
    }
    if (object_is_gold_pile(thing))
    {
        if (thing->valuable.gold_stored <= 0)
        {
            delete_thing_structure(thing, 0);
            return -1;
        }
        struct Room* room = get_room_thing_is_on(thing);
        if (!room_is_invalid(room) && room_role_matches(room->kind, RoRoF_GoldStorage))
        {
            if ((thing->owner == room->owner) || is_neutral_thing(thing))
            {
                if (gold_being_dropped_at_treasury(thing, room) == -1) {
                    return -1;
                }
            }
        }
        if (thing->model == ObjMdl_SpinningCoin)
        {
            drop_gold_pile(thing->valuable.gold_stored, &thing->mappos);
            delete_thing_structure(thing, 0);
            return -1;
        }
        struct Thing* gldtng = find_base_thing_on_mapwho_excluding_self(thing);
        if (!thing_is_invalid(gldtng))
        {
            add_gold_to_pile(gldtng, thing->valuable.gold_stored);
            delete_thing_structure(thing, 0);
            return -1;
        }
    }
    thing->active_state = thing->continue_state;
    return 1;
}

void update_dungeon_heart_beat(struct Thing *heartng)
{
    if (!thing_exists(heartng))
    {
        ERRORLOG("Trying to beat non-existing heart");
        return;
    }
    const long base_heart_beat_rate = 2304;
    static long bounce = 0;
    if (heartng->active_state != ObSt_BeingDestroyed)
    {
        long i = (char)heartng->heart.beat_direction;
        heartng->anim_speed = 0;

        struct ObjectConfigStats* objst = get_object_model_stats(heartng->model);
        long long k = 1;
        if (objst->health != 0)
        {
            k = 384 * (long)(objst->health - heartng->health) / objst->health;
        }
        if ((k + 128) > 0)
        {
            k = base_heart_beat_rate / (k + 128);
        }
        if (k > 0)
        {
            int intensity = light_get_light_intensity(heartng->light_id) + (i * 36 / k);
            // intensity capped to 63 to fix the first beat flickering black which is visible when SKIP_HEART_ZOOM is on
            light_set_light_intensity(heartng->light_id, min(intensity, 63));
            heartng->anim_time += (i * base_heart_beat_rate / k);
            if (heartng->anim_time < 0)
            {
                heartng->anim_time = 0;
                light_set_light_intensity(heartng->light_id, 20);
                heartng->heart.beat_direction = 1;
            }
            if (heartng->anim_time > base_heart_beat_rate - 1)
            {
                heartng->anim_time = base_heart_beat_rate - 1;
                light_set_light_intensity(heartng->light_id, 56);
                heartng->heart.beat_direction = (unsigned char)-1;
                if (bounce)
                {
                    thing_play_sample(heartng, 151, NORMAL_PITCH, 0, 3, 1, 6, FULL_LOUDNESS);
                }
                else
                {
                    thing_play_sample(heartng, 150, NORMAL_PITCH, 0, 3, 1, 6, FULL_LOUDNESS);
                }
                bounce = !bounce;
            }
        }
        k = (((unsigned long long)heartng->anim_time >> 32) & 0xFF) + heartng->anim_time;
        heartng->current_frame = (k >> 8) & 0xFF;
        if (LbIsFrozenOrPaused())
        {
            stop_thing_playing_sample(heartng, 93);
        }
        else if ( !S3DEmitterIsPlayingSample(heartng->snd_emitter_id, 93, 0) )
        {
            thing_play_sample(heartng, 93, NORMAL_PITCH, -1, 3, 1, 6, FULL_LOUDNESS);
        }
    }
}

static TngUpdateRet object_update_dungeon_heart(struct Thing *heartng)
{
    SYNCDBG(18,"Starting");
    struct Dungeon* dungeon = INVALID_DUNGEON;
    struct ObjectConfigStats* objst = get_object_model_stats(heartng->model);

    if (heartng->owner != game.neutral_player_num)
    {
        dungeon = get_players_num_dungeon(heartng->owner);
    }

    if ((heartng->health > 0) && (game.conf.rules[heartng->owner].game.dungeon_heart_heal_time != 0))
    {
        if ((game.play_gameturn % game.conf.rules[heartng->owner].game.dungeon_heart_heal_time) == 0)
        {
            heartng->health += game.conf.rules[heartng->owner].game.dungeon_heart_heal_health;
            if (heartng->health < 0)
            {
              heartng->health = 0;
            } else
            if (heartng->health > objst->health)
            {
              heartng->health = objst->health;
            }
        }
        if (objst->health > 0) //prevent divide by 0 crash
        {
            long long k = ((heartng->health << 8) / objst->health) << 7;
            long i = (saturate_set_signed(k, 32) >> 8) + 128;
            heartng->sprite_size = i * (long)objst->sprite_size_max >> 8;
            heartng->solid_size_xy = i * (long)objst->size_xy >> 8;
            heartng->solid_size_z = i * (long)objst->size_z >> 8;
            heartng->clipbox_size_z = heartng->solid_size_z;
        }
    }
    else if (!dungeon_invalid(dungeon) && (heartng->index == dungeon->dnheart_idx))
    {
        if (dungeon->heart_destroy_state == 0)
        {
            dungeon->heart_destroy_turn = 0;
            dungeon->heart_destroy_state = 1;
            dungeon->essential_pos.x.val = heartng->mappos.x.val;
            dungeon->essential_pos.y.val = heartng->mappos.y.val;
            dungeon->essential_pos.z.val = heartng->mappos.z.val;
        }
    }
    if (dungeon_invalid(dungeon) || (heartng->index != dungeon->dnheart_idx))
    {
        SYNCDBG(18, "Inactive Heart");
        if (heartng->health <= 0)
        {
            struct Thing* efftng;
            efftng = create_used_effect_or_element(&heartng->mappos, objst->effect.explosion1, heartng->owner, heartng->index);
            if (!thing_is_invalid(efftng))
                efftng->shot_effect.hit_type = THit_HeartOnlyNotOwn;
            efftng = create_used_effect_or_element(&heartng->mappos, objst->effect.explosion2, heartng->owner, heartng->index);
            if (!thing_is_invalid(efftng))
                efftng->shot_effect.hit_type = THit_HeartOnlyNotOwn;
            if (!dungeon_invalid(dungeon) && heartng->index == dungeon->backup_heart_idx)
            {
                dungeon->backup_heart_idx = 0;
            }
            destroy_dungeon_heart_room(heartng->owner, heartng);
            if (!thing_is_invalid(heartng))
            {
                delete_thing_structure(heartng, 0);
            }
        }
        return TUFRet_Unchanged;
    }
    else
    {
        if (!thing_is_dungeon_heart(heartng))
        {
            if (dungeon->backup_heart_idx > 0)
            {
                dungeon->dnheart_idx = dungeon->backup_heart_idx;
                dungeon->backup_heart_idx = 0;
                struct Thing* scndthing = find_players_backup_dungeon_heart(heartng->owner);
                {
                    if (thing_exists(scndthing))
                    {
                        dungeon->backup_heart_idx = scndthing->index;
                    }
                }
            }

        }
    }
    process_dungeon_destroy(heartng);

    SYNCDBG(18,"Beat update");
    if ((heartng->alloc_flags & TAlF_Exists) == 0)
      return TUFRet_Modified;
    if (objst->model_flags & OMF_Beating)
    {
        update_dungeon_heart_beat(heartng);
    }
    return TUFRet_Modified;
}

void set_call_to_arms_as_birthing(struct Thing *objtng)
{
    int frame;
    switch (objtng->call_to_arms_flag.state)
    {
    case CTAOL_Birthing:
        frame = objtng->current_frame;
        break;
    case CTAOL_Alive:
        frame = 0;
        break;
    case CTAOL_Dying:
    case CTAOL_Rebirthing:
        frame = objtng->max_frames - (int)objtng->current_frame;
        break;
    default:
        ERRORLOG("Invalid CTA object life state %d",(int)objtng->call_to_arms_flag.state);
        frame = 0;
        break;
    }
    struct CallToArmsGraphics* ctagfx = &call_to_arms_graphics[get_player_color_idx(objtng->owner)];
    struct ObjectConfigStats* objst = get_object_model_stats(objtng->model);
    set_thing_draw(objtng, ctagfx->birth_anim_idx, 256, objst->sprite_size_max, 0, frame, ODC_Default);
    objtng->call_to_arms_flag.state = CTAOL_Birthing;
    struct PowerConfigStats* powerst = get_power_model_stats(PwrK_CALL2ARMS);
    stop_thing_playing_sample(objtng, powerst->select_sound_idx);
    thing_play_sample(objtng, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 6, FULL_LOUDNESS);
}

void set_call_to_arms_as_dying(struct Thing *objtng)
{
    int frame;
    switch (objtng->call_to_arms_flag.state)
    {
    case CTAOL_Birthing:
        frame = objtng->max_frames - (int)objtng->current_frame;
        break;
    case CTAOL_Alive:
        frame = 0;
        break;
    case CTAOL_Dying:
    case CTAOL_Rebirthing:
        frame = objtng->current_frame;
        break;
    default:
        ERRORLOG("Invalid CTA object life state %d",(int)objtng->call_to_arms_flag.state);
        frame = 0;
        break;
    }
    struct CallToArmsGraphics* ctagfx = &call_to_arms_graphics[get_player_color_idx(objtng->owner)];
    struct ObjectConfigStats* objst = get_object_model_stats(objtng->model);
    set_thing_draw(objtng, ctagfx->leave_anim_idx, 256, objst->sprite_size_max, 0, frame, ODC_Default);
    objtng->call_to_arms_flag.state = CTAOL_Dying;
}

void set_call_to_arms_as_rebirthing(struct Thing *objtng)
{
    int frame;
    switch (objtng->call_to_arms_flag.state)
    {
    case CTAOL_Birthing:
        frame = objtng->max_frames - (int)objtng->current_frame;
        break;
    case CTAOL_Alive:
        frame = 0;
        break;
    case CTAOL_Dying:
    case CTAOL_Rebirthing:
        frame = objtng->current_frame;
        break;
    default:
        ERRORLOG("Invalid CTA object life state %d",(int)objtng->call_to_arms_flag.state);
        frame = 0;
        break;
    }
    struct CallToArmsGraphics* ctagfx = &call_to_arms_graphics[get_player_color_idx(objtng->owner)];
    struct ObjectConfigStats* objst = get_object_model_stats(objtng->model);
    set_thing_draw(objtng, ctagfx->leave_anim_idx, 256, objst->sprite_size_max, 0, frame, ODC_Default);
    objtng->call_to_arms_flag.state = CTAOL_Rebirthing;
}

static TngUpdateRet object_update_call_to_arms(struct Thing *thing)
{
    struct PlayerInfo* player = get_player(thing->owner);
    if (thing->index != player->cta_flag_idx)
    {
        delete_thing_structure(thing, 0);
        return -1;
    }
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct CallToArmsGraphics* ctagfx = &call_to_arms_graphics[dungeon->color_idx];
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);

    switch (thing->call_to_arms_flag.state)
    {
    case CTAOL_Birthing:
        if (thing->max_frames - 1 <= thing->current_frame)
        {
            thing->call_to_arms_flag.state = CTAOL_Alive;
            set_thing_draw(thing, ctagfx->alive_anim_idx, 256, objst->sprite_size_max, 0, 0, ODC_Default);
            return 1;
        }
        break;
    case CTAOL_Alive:
        break;
    case CTAOL_Dying:
        if (thing->max_frames - 1 == thing->current_frame)
        {
            player->cta_flag_idx = 0;
            delete_thing_structure(thing, 0);
            return -1;
        }
        break;
    case CTAOL_Rebirthing:
    {
        if (thing->max_frames - 1 == thing->current_frame)
        {
            struct PowerConfigStats* powerst = get_power_model_stats(PwrK_CALL2ARMS);
            struct Coord3d pos;
            pos.x.val = subtile_coord_center(dungeon->cta_stl_x);
            pos.y.val = subtile_coord_center(dungeon->cta_stl_y);
            pos.z.val = get_thing_height_at(thing, &pos);
            move_thing_in_map(thing, &pos);
            reset_interpolation_of_thing(thing);
            set_thing_draw(thing, ctagfx->birth_anim_idx, 256, objst->sprite_size_max, 0, 0, ODC_Default);
            thing->call_to_arms_flag.state = CTAOL_Birthing;
            stop_thing_playing_sample(thing, powerst->select_sound_idx);
            thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 6, FULL_LOUDNESS);
        }
        break;
    }
    default:
        break;
    }
    return 1;
}

static TngUpdateRet object_update_armour(struct Thing *objtng)
{
    struct Thing* thing = thing_get(objtng->armor.belongs_to);
    if (thing_is_picked_up(thing))
    {
        objtng->rendering_flags |= TRF_Invisible;
        return 1;
    }
    struct Coord3d pos;
    struct ComponentVector cvect;
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    if ((abs(objtng->mappos.x.val - pos.x.val) > 512)
     || (abs(objtng->mappos.y.val - pos.y.val) > 512)
     || (abs(objtng->mappos.z.val - pos.z.val) > 512))
    {
        short shspeed = objtng->armor.shspeed;
        pos.x.val += 32 * LbSinL(682 * shspeed) >> 16;
        pos.y.val += -(32 * LbCosL(682 * shspeed) >> 8) >> 8;
        pos.z.val += shspeed * (thing->clipbox_size_z >> 1);
        move_thing_in_map(objtng, &pos);
        objtng->move_angle_xy = thing->move_angle_xy;
        objtng->move_angle_z = thing->move_angle_z;
        angles_to_vector(thing->move_angle_xy, thing->move_angle_z, 32, &cvect);
    }
    else
    {
        pos.z.val += (thing->clipbox_size_z >> 1);
        objtng->move_angle_xy = get_angle_xy_to(&objtng->mappos, &pos);
        objtng->move_angle_z = get_angle_yz_to(&objtng->mappos, &pos);
        angles_to_vector(objtng->move_angle_xy, objtng->move_angle_z, 32, &cvect);
        long cvect_len = LbSqrL(cvect.x * cvect.x + cvect.z * cvect.z + cvect.y * cvect.y);
        if (cvect_len > 128)
        {
          pos.x.val = (cvect.x << 7) / cvect_len;
          pos.y.val = (cvect.y << 7) / cvect_len;
          pos.z.val = (cvect.z << 7) / cvect_len;
          cvect.x = pos.x.val;
          cvect.y = pos.y.val;
          cvect.z = pos.z.val;
        }
    }
    objtng->state_flags |= TF1_PushAdd;
    objtng->veloc_push_add.x.val += cvect.x;
    objtng->veloc_push_add.y.val += cvect.y;
    objtng->veloc_push_add.z.val += cvect.z;
    objtng->rendering_flags &= ~TRF_Invisible;
    return 1;
}

static TngUpdateRet object_update_object_scale(struct Thing *objtng)
{
    struct Thing* creatng = thing_get(objtng->lair.belongs_to);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct ObjectConfigStats* objst = get_object_model_stats(objtng->model);
    int spr_size;
    int start_frame = objtng->current_frame;
    if (objtng->lair.belongs_to) {
        spr_size = game.conf.crtr_conf.sprite_size + (game.conf.crtr_conf.sprite_size * cctrl->exp_level * game.conf.crtr_conf.exp.size_increase_on_exp) / 100;
    } else {
        spr_size = objst->sprite_size_max;
    }
    int cssize = objtng->lair.cssize;
    objtng->lair.spr_size = spr_size;
    long i;
    if (cssize+32 < spr_size)
    {
        objtng->lair.cssize = cssize+32;
        i = convert_td_iso(objst->sprite_anim_idx);
    } else
    if (cssize-32 > spr_size)
    {
        objtng->lair.cssize = cssize-32;
        i = convert_td_iso(objst->sprite_anim_idx);
    } else
    {
        objtng->lair.cssize = spr_size;
        i = convert_td_iso(objst->sprite_anim_idx);
    }
    if ((i & 0x8000u) != 0) {
        i = objst->sprite_anim_idx;
    }
    set_thing_draw(objtng, i, objst->anim_speed, objtng->lair.cssize, 0, start_frame, objst->draw_class);
    return 1;
}

static TngUpdateRet object_update_power_sight(struct Thing *objtng)
{
    int result; // eax
    objtng->health = 2;
    if (is_neutral_thing(objtng))
    {
        ERRORLOG("Neutral %s index %d cannot be power sight.", thing_model_name(objtng), (int)objtng->index);
        delete_thing_structure(objtng, 0);
        return 0;
    }
    struct Dungeon * dungeon = get_dungeon(objtng->owner);
    struct PowerConfigStats* powerst = get_power_model_stats(PwrK_SIGHT);

    if ( !S3DEmitterIsPlayingSample(objtng->snd_emitter_id, powerst->select_sound_idx, 0) ) {
        thing_play_sample(objtng, powerst->select_sound_idx, NORMAL_PITCH, -1, 3, 1, 3, FULL_LOUDNESS);
    }

    KeepPwrLevel sight_casted_power_level = dungeon->sight_casted_power_level;
    int max_time_active = powerst->strength[sight_casted_power_level];
    int strength = min(powerst->strength[sight_casted_power_level], (MAX_SOE_RADIUS * COORD_PER_STL / 4));

    if ( game.play_gameturn - objtng->creation_turn >= max_time_active
        && game.play_gameturn - dungeon->sight_casted_gameturn < max_time_active )
    {
        int time_active = game.play_gameturn - dungeon->sight_casted_gameturn;
        if ( game.play_gameturn >= dungeon->sight_casted_gameturn)
        {
            if ( max_time_active / 16 < time_active )
                time_active = max_time_active / 16;
        }
        else
        {
            time_active = 0;
        }
        const int time_interval_divisor = (max_time_active / 16) / power_sight_close_instance_time[sight_casted_power_level];
        dungeon->sight_casted_gameturn = game.play_gameturn - max_time_active + time_active / time_interval_divisor - power_sight_close_instance_time[sight_casted_power_level];
    }
    if ( max_time_active <= game.play_gameturn - dungeon->sight_casted_gameturn )
    {
        if ( power_sight_close_instance_time[dungeon->sight_casted_power_level] <= (game.play_gameturn - dungeon->sight_casted_gameturn) - max_time_active )
        {
            if ( (dungeon->computer_enabled & 4) != 0 )
            {
                dungeon->sight_casted_gameturn = game.play_gameturn;
                struct Coord3d pos;
                pos.x.val = (dungeon->sight_casted_stl_x << 8) + 128;
                pos.z.val = 1408;
                pos.y.val = (dungeon->sight_casted_stl_y << 8) + 128;
                memset(dungeon->soe_explored_flags, 0, sizeof(dungeon->soe_explored_flags));
                move_thing_in_map(objtng, &pos);
                result = 1;
                dungeon->computer_enabled &= ~4u;
            }
            else
            {
                dungeon->sight_casted_thing_idx = 0;
                memset(dungeon->soe_explored_flags, 0, sizeof(dungeon->soe_explored_flags));
                delete_thing_structure(objtng, 0);
                return 0;
            }
        }
        else
        {
            // draw 32 particles in a collapsing starburst pattern
            const int anim_time = (game.play_gameturn - dungeon->sight_casted_gameturn);
            const int anim_radius = 4 * anim_time;
            const int close_radius = 32 * (power_sight_close_instance_time[dungeon->sight_casted_power_level] - (anim_time - max_time_active));
            const int max_duration_radius = max_time_active / 4;
            const int strength_radius = strength/4;
            const int radius = max(0, min(min(min(close_radius, max_duration_radius), anim_radius), strength_radius));
            for (int i = 0; i < 32; ++i) {
                const int step = ((DEGREES_360) / 32);
                const int angle = step * i;
                struct Coord3d pos;
                pos.x.val = objtng->mappos.x.val + ((radius * LbSinL(angle)) / 8192);
                pos.y.val = objtng->mappos.y.val + ((radius * LbCosL(angle)) / 8192);
                pos.z.val = 1408;
                create_effect_element(&pos, twinkle_eff_elements[get_player_color_idx(objtng->owner)], objtng->owner);
            }
            return 1;
        }
    }
    else
    {
        // draw 32 particles in an expanding radial pattern, 4 at a time, exploring terrain as we go
        const int anim_time = (game.play_gameturn - dungeon->sight_casted_gameturn);
        const int anim_radius = 4 * anim_time;
        const int max_duration_radius = max_time_active / 4;
        const int strength_radius = strength/4;
        const int radius = max(0, min(min(max_duration_radius, anim_radius), strength_radius));
        for (int i = 0; i < 4; ++i) {
            const int step = ((DEGREES_360) / 32);
            const int angle = step * ((4 * anim_time) + i);
            const int pos_x = objtng->mappos.x.val + ((radius * LbSinL(angle)) / 8192);
            const int pos_y = objtng->mappos.y.val + ((radius * LbCosL(angle)) / 8192);
            struct Coord3d pos;
            pos.x.val = pos_x;
            pos.y.val = pos_y;
            pos.z.val = 1408;
            create_effect_element(&pos, twinkle_eff_elements[get_player_color_idx(objtng->owner)], objtng->owner);
            if ( pos_x >= 0 && pos_x < game.map_subtiles_x * COORD_PER_STL && pos_y >= 0 && pos_y < game.map_subtiles_y * COORD_PER_STL ) {
                const int shift_x = pos.x.stl.num - objtng->mappos.x.stl.num + MAX_SOE_RADIUS;
                const int shift_y = pos.y.stl.num - objtng->mappos.y.stl.num + MAX_SOE_RADIUS;
                dungeon->soe_explored_flags[shift_y][shift_x] = pos.x.val < game.map_subtiles_x * COORD_PER_STL && pos.y.val < game.map_subtiles_y * COORD_PER_STL;
            }
        }
        return 1;
    }
    return result;
}

#define NUM_ANGLES 16
static TngUpdateRet object_update_power_lightning(struct Thing *objtng)
{
    objtng->health = 2;
    unsigned long exist_turns = game.play_gameturn - objtng->creation_turn;
    long variation = NUM_ANGLES * exist_turns;
    for (long i = 0; i < NUM_ANGLES; i++)
    {
        int angle = (variation % NUM_ANGLES) * DEGREES_360 / NUM_ANGLES;
        struct Coord3d pos;
        if (set_coords_to_cylindric_shift(&pos, &objtng->mappos, 8 * variation, angle, 0))
        {
            struct Map* mapblk = get_map_block_at(pos.x.stl.num, pos.y.stl.num);
            if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
            {
                pos.z.val = get_floor_height_at(&pos) + 128;
                create_effect_element(&pos, lightning_spangles[get_player_color_idx(objtng->owner)], objtng->owner);
            }
        }
        variation++;
    }
    const struct PowerConfigStats *powerst = get_power_model_stats(PwrK_LIGHTNING);
    if (exist_turns > abs(powerst->strength[objtng->lightning.power_level]))
    {
        delete_thing_structure(objtng, 0);
        return TUFRet_Deleted;
    }
    return TUFRet_Modified;
}
#undef NUM_ANGLES

/**
 * Finds an empty safe adjacent position on slab.
 * @param thing The thing which is to be moved.
  * @param pos The target position pointer.
 */
static TbBool find_free_position_on_slab(struct Thing* thing, struct Coord3d* pos)
{
    MapSubtlCoord start_stl = THING_RANDOM(thing, AROUND_TILES_COUNT);
    int nav_sizexy = subtile_coord(thing_nav_block_sizexy(thing), 0);

    for (long nround = 0; nround < AROUND_TILES_COUNT; nround++)
    {
        MapSubtlCoord x = start_stl % 3 + thing->mappos.x.stl.num;
        MapSubtlCoord y = start_stl / 3 + thing->mappos.y.stl.num;
        if (get_floor_filled_subtiles_at(x, y) == 1)
        {
            struct Thing* objtng = find_base_thing_on_mapwho(TCls_Object, 0, x, y);
            if (thing_is_invalid(objtng))
            {
                pos->x.val = subtile_coord_center(x);
                pos->y.val = subtile_coord_center(y);
                pos->z.val = get_thing_height_at_with_radius(thing, pos, nav_sizexy);
                if (!thing_in_wall_at_with_radius(thing, pos, nav_sizexy)) {
                    return true;
                }
            }
        }
        start_stl = (start_stl + 1) % 9;
    }
    return false;
}

TngUpdateRet move_object(struct Thing *thing)
{
    SYNCDBG(18,"Starting");
    if (!thing_exists(thing))
    {
        ERRORLOG("Attempt to move non-existing object.");
        return TUFRet_Deleted;
    }
    TRACE_THING(thing);
    struct Coord3d pos;
    TbBool move_allowed = get_thing_next_position(&pos, thing);
    if ( !positions_equivalent(&thing->mappos, &pos) )
    {
        if ((!move_allowed) || thing_in_wall_at(thing, &pos))
        {
            long blocked_flags = get_thing_blocked_flags_at(thing, &pos);
            if (blocked_flags & SlbBloF_WalledZ)
            {
                TbBool is_sight_of_evil = false;
                if (thing->owner != PLAYER_NEUTRAL)
                {
                    struct Dungeon* dungeon = get_dungeon(thing->owner);
                    if (dungeon->sight_casted_thing_idx == thing->index)
                    {
                        is_sight_of_evil = true;
                    }
                }
                if (!is_sight_of_evil)
                {
                    if (!find_free_position_on_slab(thing, &pos))
                    {
                        SYNCDBG(7, "Found no free position next to (%ld,%ld) due to blocked flag %ld. Move to valid position.",
                            pos.x.val, pos.y.val, blocked_flags);
                        move_creature_to_nearest_valid_position(thing);
                    }
                }
            }
            else
            {
                slide_thing_against_wall_at(thing, &pos, blocked_flags);
                remove_relevant_forces_from_thing_after_slide(thing, &pos, blocked_flags);
            }
            // GOLD_POT to make a sound when hitting the floor
            if (thing->model == ObjMdl_GoldPot)
            {
                thing_play_sample(thing, 79, NORMAL_PITCH, 0, 3, 0, 1, FULL_LOUDNESS);
            }
            if (thing_in_wall_at(thing, &pos) == 0) //TODO: Improve 'slide_thing_against_wall_at' so it does not return a pos inside a wall
            {
                move_thing_in_map(thing, &pos);
            }
        }
        else
        {
            move_thing_in_map(thing, &pos);
        }
    }
    thing->floor_height = get_thing_height_at(thing, &thing->mappos);
    return TUFRet_Modified;
}

TngUpdateRet update_object(struct Thing *thing)
{
    SYNCDBG(18,"Starting for %s",thing_model_name(thing));
    TRACE_THING(thing);


    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);

    if (objst->updatefn_idx > 0)
    {
        Thing_Class_Func upcallback = NULL;
        upcallback = object_update_functions[objst->updatefn_idx];
        if (upcallback != NULL)
        {
            if (upcallback(thing) <= 0) {
                return TUFRet_Deleted;
            }
        }
    }
    else if (objst->updatefn_idx < 0)
    {
        if (luafunc_thing_update_func(objst->updatefn_idx, thing) <= 0) {
            return TUFRet_Deleted;
        }
    }

    Thing_State_Func stcallback = NULL;
    if (thing->active_state < sizeof(object_state_functions)/sizeof(object_state_functions[0])) {
        stcallback = object_state_functions[thing->active_state];
    } else {
        ERRORLOG("The %s state %d exceeds state_functions dimensions",thing_model_name(thing),(int)thing->active_state);
    }
    if (stcallback != NULL)
    {
        SYNCDBG(18,"Updating state");
        if (stcallback(thing) <= 0) {
            return TUFRet_Deleted;
        }
    }
    SYNCDBG(18,"Updating position");
    thing->movement_flags &= ~TMvF_IsOnWater;
    thing->movement_flags &= ~TMvF_IsOnLava;
    if ( ((thing->movement_flags & TMvF_Immobile) == 0) && thing_touching_floor(thing) )
    {
        if (subtile_has_lava_on_top(thing->mappos.x.stl.num, thing->mappos.y.stl.num))
        {
            thing->movement_flags |= TMvF_IsOnLava;
            if ( (objst->destroy_on_lava) && !thing_is_dragged_or_pulled(thing) )
            {
                destroy_object(thing);
                return TUFRet_Deleted;
            }
        } else
        if (subtile_has_water_on_top(thing->mappos.x.stl.num, thing->mappos.y.stl.num))
        {
            thing->movement_flags |= TMvF_IsOnWater;
        }
    }
    if ((thing->movement_flags & TMvF_Immobile) != 0)
        return TUFRet_Modified;
    return move_object(thing);
}

/**
 * Creates a coloured object.
 * @param pos Position where the object is to be created.
 * @param plyr_idx Player who will own the flag.
 * @param parent_idx Slab number associated with the flag.
 * @return object thing.
 */
struct Thing *create_coloured_object(const struct Coord3d *pos, PlayerNumber plyr_idx, long parent_idx, ThingModel base_model)
{
    ThingModel model = get_player_colored_object_model(base_model,plyr_idx);
    if (model <= 0)
        return INVALID_THING;
    // Guard posts have slab number set as parent
    struct Thing* thing = create_object(pos, model, plyr_idx, parent_idx);
    if (thing_is_invalid(thing))
        return INVALID_THING;
    return thing;
}

struct Thing *create_gold_pot_at(long pos_x, long pos_y, PlayerNumber plyr_idx)
{
    struct Coord3d pos;
    pos.x.val = pos_x;
    pos.y.val = pos_y;
    pos.z.val = subtile_coord(3,0);
    struct Thing* gldtng = create_object(&pos, ObjMdl_GoldPot, plyr_idx, -1);
    if (thing_is_invalid(gldtng))
        return INVALID_THING;
    gldtng->valuable.gold_stored = gold_object_typical_value(gldtng);
    // Update size of the gold object
    add_gold_to_pile(gldtng, 0);
    return gldtng;
}

/**
 * For given gold hoard thing model, returns the wealth size, scaled 0..max_size.
 */
int get_wealth_size_of_gold_hoard_model(ThingModel objmodel)
{
    // Check gold_hoard_objects array to determine wealth_size of the hoard model
    const int count = get_wealth_size_types_count();
    for (int i = 0; i < count; ++i)
    {
        if (gold_hoard_objects[i] == objmodel) {
            int wealth_size = i+1;
            return wealth_size;
        }
    }
    return 0;
}

/**
 * For given gold hoard thing, returns the wealth size, scaled 0..max_size.
 */
int get_wealth_size_of_gold_hoard_object(const struct Thing *objtng)
{
    return get_wealth_size_of_gold_hoard_model(objtng->model);
}

/**
 * For gold amount, returns the weath size, which is the size of the hoard.
 For example:
 400 gold = 1 wealth size
 800 gold = 2 wealth size
 1200 gold = 3 wealth size
 1600 gold = 4 wealth size
 2000 gold = 5 wealth size
 */
int get_wealth_size_of_gold_amount(GoldAmount value)
{
    long wealth_size_holds = game.conf.rules[0].game.gold_per_hoard / get_wealth_size_types_count();
    int wealth_size = (value + wealth_size_holds - 1) / wealth_size_holds;
    if (wealth_size > get_wealth_size_types_count()) {
        WARNLOG("Gold hoard with %d gold would be oversized",(int)value);
        wealth_size = get_wealth_size_types_count();
    }
    return wealth_size;
}

/**
 * Gives amount of possible wealth sizes of gold hoard.
 */
int get_wealth_size_types_count(void)
{
    // This will return a value of 5 because there's 5 items in gold_hoard_objects array
    return sizeof(gold_hoard_objects)/sizeof(gold_hoard_objects[0]);
}

/**
 * Creates a gold hoard object.
 * Note that this function does not create a fully operable object - gold hoard requires room
 * association to be fully functional. This is just a utility sub-function.
 * @param pos Position where the hoard is to be created.
 * @param plyr_idx Player who will own the hoard.
 * @param value The max amount of gold to be stored inside the hoard.
 * @return Hoard object thing, which still require to be associated to room.
 */
struct Thing *create_gold_hoard_object(const struct Coord3d *pos, PlayerNumber plyr_idx, GoldAmount value)
{
    if (value >= game.conf.rules[plyr_idx].game.gold_per_hoard)
        value = game.conf.rules[plyr_idx].game.gold_per_hoard;
    int wealth_size = get_wealth_size_of_gold_amount(value);
    struct Thing* gldtng = create_object(pos, gold_hoard_objects[wealth_size-1], plyr_idx, -1);
    if (thing_is_invalid(gldtng))
        return INVALID_THING;
    gldtng->valuable.gold_stored = value;
    return gldtng;
}

struct Thing *create_gold_hoarde(struct Room *room, const struct Coord3d *pos, GoldAmount value)
{
    struct Thing* thing = INVALID_THING;
    GoldAmount wealth_size_holds = game.conf.rules[room->owner].game.gold_per_hoard / get_wealth_size_types_count();
    if ((value <= 0) || (room->slabs_count < 1)) {
        ERRORLOG("Attempt to create a gold hoard with %ld gold", (long)value);
        return thing;
    }
    GoldAmount max_hoard_size_in_room = wealth_size_holds * room->total_capacity / room->slabs_count;
    if (value > max_hoard_size_in_room)
        value = max_hoard_size_in_room;
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    const struct Map* mapblk = get_map_block_at(pos->x.stl.num, pos->y.stl.num);
    if ((roomst->storage_height < 0) || (get_map_floor_filled_subtiles(mapblk) == roomst->storage_height))
    {
        thing = create_gold_hoard_object(pos, room->owner, value);
    }
    if (!thing_is_invalid(thing))
    {
        room->capacity_used_for_storage += thing->valuable.gold_stored;
        struct Dungeon* dungeon = get_dungeon(room->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->total_money_owned += thing->valuable.gold_stored;
        }
        int wealth_size = get_wealth_size_of_gold_amount(thing->valuable.gold_stored);
        room->used_capacity += wealth_size;
    }
    return thing;
}

/**
 * Adds gold to hoard stored in room.
 *
 * @param gldtng The gold hoard thing.
 * @param room The room which stores this hoard.
 * @param amount Amount of gold to be added.
 * @return Gives amount really added to the hoard.
 */
long add_gold_to_hoarde(struct Thing *gldtng, struct Room *room, GoldAmount amount)
{
    GoldAmount wealth_size_holds = game.conf.rules[room->owner].game.gold_per_hoard / get_wealth_size_types_count();
    GoldAmount max_hoard_size_in_room = wealth_size_holds * room->total_capacity / room->slabs_count;
    // Fix amount
    if (gldtng->valuable.gold_stored + amount > max_hoard_size_in_room)
        amount = max_hoard_size_in_room - gldtng->valuable.gold_stored;
    if (amount <= 0) {
        return 0;
    }
    // Remove prev wealth size
    int wealth_size = get_wealth_size_of_gold_amount(gldtng->valuable.gold_stored);
    if (wealth_size > room->used_capacity) {
        ERRORLOG("Room %s index %d has used capacity %d but stores gold hoard index %d of wealth size %d (%ld gold)",
            room_code_name(room->kind),(int)room->index,(int)room->used_capacity,(int)gldtng->index,(int)wealth_size,(long)gldtng->valuable.gold_stored);
        wealth_size = room->used_capacity;
    }
    room->used_capacity -= wealth_size;
    // Add amount of gold
    gldtng->valuable.gold_stored += amount;
    room->capacity_used_for_storage += amount;
    if (room->owner != game.neutral_player_num)
    {
        struct Dungeon* dungeon = get_dungeon(room->owner);
        if (!dungeon_invalid(dungeon)) {
            dungeon->total_money_owned += amount;
        }
    }
    // Add new wealth size
    wealth_size = get_wealth_size_of_gold_amount(gldtng->valuable.gold_stored);
    room->used_capacity += wealth_size;
    // switch hoard object model
    gldtng->model = gold_hoard_objects[wealth_size-1];
    // Set visual appearance
    struct ObjectConfigStats* objst = get_object_model_stats(gldtng->model);
    unsigned short i = objst->sprite_anim_idx;
    unsigned short n = convert_td_iso(i);
    if ((n & 0x8000u) == 0) {
      i = n;
    }
    set_thing_draw(gldtng, i, objst->anim_speed, objst->sprite_size_max, 0, 0, objst->draw_class);
    return amount;
}

/**
 * Removes gold from hoard stored in room.
 *
 * @param gldtng The gold hoard thing.
 * @param room The room which stores this hoard.
 * @param amount Amount of gold to be taken.
 * @return Gives amount really taken from the hoard.
 */
long remove_gold_from_hoarde(struct Thing *gldtng, struct Room *room, GoldAmount amount)
{
    if (amount <= 0) {
        return 0;
    }
    if (amount > gldtng->valuable.gold_stored)
        amount = gldtng->valuable.gold_stored;
    // Remove prev wealth size
    int wealth_size = get_wealth_size_of_gold_amount(gldtng->valuable.gold_stored);
    if (wealth_size > room->used_capacity) {
        ERRORLOG("Room %s index %d has used capacity %d but stores gold hoard index %d of wealth size %d (%ld gold)",
            room_code_name(room->kind),(int)room->index,(int)room->used_capacity,(int)gldtng->index,(int)wealth_size,(long)gldtng->valuable.gold_stored);
        wealth_size = room->used_capacity;
    }
    room->used_capacity -= wealth_size;
    // Add amount of gold
    gldtng->valuable.gold_stored -= amount;
    room->capacity_used_for_storage -= amount;
    struct Dungeon* dungeon = get_dungeon(gldtng->owner);
    if (!dungeon_invalid(dungeon)) {
        dungeon->total_money_owned -= amount;
    }

    if (gldtng->valuable.gold_stored <= 0)
    {
        delete_thing_structure(gldtng, 0);
        return amount;
    }
    // Add new wealth size
    wealth_size = get_wealth_size_of_gold_amount(gldtng->valuable.gold_stored);
    room->used_capacity += wealth_size;
    // switch hoard object model
    gldtng->model = gold_hoard_objects[wealth_size-1];
    // Set visual appearance
    struct ObjectConfigStats* objst = get_object_model_stats(gldtng->model);
    unsigned short i = objst->sprite_anim_idx;
    unsigned short n = convert_td_iso(i);
    if ((n & 0x8000u) == 0) {
      i = n;
    }
    set_thing_draw(gldtng, i, objst->anim_speed, objst->sprite_size_max, 0, 0, objst->draw_class);
    return amount;
}

/**
 * Returns if given thing is a hoard of gold.
 * @note originally was thing_is_gold_hoarde().
 * @param thing
 * @return
 */
TbBool thing_is_gold_hoard(const struct Thing *thing)
{
    if (!thing_is_object(thing))
        return false;
    return object_is_gold_hoard(thing);
}

struct Thing *find_gold_hoard_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    unsigned long k = 0;
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            WARNLOG("Jump out of things array");
            break;
      }
      i = thing->next_on_mapblk;
      // Per-thing block
      if (thing_is_gold_hoard(thing))
          return thing;
      // Per-thing block ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break_mapwho_infinite_chain(mapblk);
        break;
      }
    }
    return INVALID_THING;
}

GoldAmount gold_object_typical_value(struct Thing *thing)
{
    switch (thing->model)
    {
      case ObjMdl_GoldChest:
          return game.conf.rules[thing->owner].game.chest_gold_hold;
      case ObjMdl_GoldPot:
          return game.conf.rules[thing->owner].game.pot_of_gold_holds;
      case ObjMdl_Goldl:
          return game.conf.rules[thing->owner].game.gold_pile_value;
      case ObjMdl_GoldBag:
          return game.conf.rules[thing->owner].game.bag_gold_hold;
      case ObjMdl_SpinningCoin:
          return game.conf.rules[thing->owner].game.gold_pile_maximum;
      default:
        break;
    }
    return 0;
}

/**
 * Adds given amount of gold to gold pile, gold pot or gold chest.
 * Scales size of the pile or pot accordingly.
 *
 * @param thing
 * @param value
 * @return
 */
TbBool add_gold_to_pile(struct Thing *thing, long value)
{
    long scaled_val;
    if (thing_is_invalid(thing)) {
        return false;
    }
    GoldAmount typical_value = gold_object_typical_value(thing);
    if (typical_value <= 0) {
        return false;
    }

    thing->valuable.gold_stored += value;
    if (thing->valuable.gold_stored == 0) {
        return false;
    }
    if (thing->valuable.gold_stored < 0)
        thing->valuable.gold_stored = INT32_MAX;
    if (thing->valuable.gold_stored < typical_value)
        scaled_val = 196 * thing->valuable.gold_stored / typical_value + 128;
    else
        scaled_val = 196 + (24 * (thing->valuable.gold_stored-typical_value) / typical_value) + 128;
    if (scaled_val > 640)
      scaled_val = 640;
    thing->sprite_size = scaled_val;
    return true;
}

struct Thing *create_gold_pile(struct Coord3d *pos, PlayerNumber plyr_idx, long value)
{
    struct Thing* gldtng = create_object(pos, ObjMdl_Goldl, plyr_idx, -1);
    if (thing_is_invalid(gldtng)) {
        return INVALID_THING;
    }
    gldtng->valuable.gold_stored = 0;
    add_gold_to_pile(gldtng, value);
    return gldtng;
}

struct Thing *drop_gold_pile(long value, struct Coord3d *pos)
{
    struct Thing* thing = smallest_gold_pile_at_xy(pos->x.stl.num, pos->y.stl.num);
    if (thing_is_invalid(thing)) {
        thing = create_gold_pile(pos, game.neutral_player_num, value);
    } else {
        add_gold_to_pile(thing, value);
    }
    return thing;
}

struct PickedUpOffset* get_object_picked_up_offset(struct Thing* thing)
{
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    return &objst->object_picked_up_offset;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
