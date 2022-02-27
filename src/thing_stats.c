/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_stats.c
 *     thing_stats support functions.
 * @par Purpose:
 *     Functions to thing_stats.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_stats.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "game_merge.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "config_crtrstates.h"
#include "config_objects.h"
#include "config_effects.h"
#include "creature_states.h"
#include "player_data.h"
#include "player_instances.h"
#include "config_magic.h"
#include "vidfade.h"
#include "game_legacy.h"
#include "thing_physics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char *blood_types[] = {
    "ARh+",
    "O-",
    "MoO+",
    "BA",
    "PoE",
    "BO",
    "IkI",
    "C++",
    "AB-",
    "IgG",
    "RhoD",
    "A-",
    "A+",
    "ABO",
    "B+",
};

const char *thing_classes[] = {
    "EMPTY",
    "OBJECT",
    "SHOT",
    "EFFECTELEM",
    "CREATUREBODY",
    "CREATURE",
    "EFFECT",
    "EFFECTGEN",
    "TRAP",
    "DOOR",
    "UNKNOWN10",
    "UNKNOWN11",
    "AMBIENTSND",
    "CAVEIN",
    "UNKNOWN14",
};
/******************************************************************************/
const char *thing_class_code_name(int class_id)
{
    if ((class_id < 0) || (class_id >= sizeof(thing_classes)/sizeof(thing_classes[0])))
        return "INVALID";
    return thing_classes[class_id];
}

/**
 * Gives name of a thing model.
 * @note This function cannot be called more than once in a parameter to something
 *  - it has only one static buffer.
 * @param thing The thing which model is to be described.
 * @return The model name string, static buffer.
 */
const char *thing_class_and_model_name(int class_id, int model)
{
    static char name_buffer[2][32];
    static int bid = 0;
    bid = (bid+1)%2;
    switch (class_id)
    {
    case TCls_Creature:
        snprintf(name_buffer[bid],sizeof(name_buffer[0]),"creature %s",creature_code_name(model));
        break;
    case TCls_DeadCreature:
        snprintf(name_buffer[bid],sizeof(name_buffer[0]),"dead %s",creature_code_name(model));
        break;
    case TCls_Trap:
        snprintf(name_buffer[bid],sizeof(name_buffer[0]),"%s trap",trap_code_name(model));
        break;
    case TCls_Door:
        snprintf(name_buffer[bid],sizeof(name_buffer[0]),"%s door",door_code_name(model));
        break;
    case TCls_Shot:
        snprintf(name_buffer[bid],sizeof(name_buffer[0]),"%s shot",shot_code_name(model));
        break;
    case TCls_Object:
        snprintf(name_buffer[bid],sizeof(name_buffer[0]),"object %s",object_code_name(model));
        break;
    case TCls_Effect:
        snprintf(name_buffer[bid],sizeof(name_buffer[0]),"%s effect",effect_code_name(model));
        break;
    default:
        snprintf(name_buffer[bid],sizeof(name_buffer[0]),"%s model %d",thing_class_code_name(class_id),(int)model);
        break;
    }
    return name_buffer[bid];
}

/**
 * Gives name of a thing model.
 * @note This function cannot be called more than once in a parameter to something
 *  - it has only one static buffer.
 * @param thing The thing which model is to be described.
 * @return The model name string, static buffer.
 */
const char *thing_model_name(const struct Thing *thing)
{
    return thing_class_and_model_name(thing->class_id, thing->model);
}

const char *creatrtng_actstate_name(const struct Thing *thing)
{
    return creature_state_code_name(thing->active_state);
}

const char *creatrtng_realstate_name(const struct Thing *thing)
{
    return creature_state_code_name(get_creature_state_besides_interruptions(thing));
}

TbBool things_stats_debug_dump(void)
{
    int count[THING_CLASSES_COUNT];
    int realcnt[THING_CLASSES_COUNT];
    int i;
    for (i=0; i < THING_CLASSES_COUNT; i++) {
        count[i] = 0;
        realcnt[i] = 0;
    }
    count[TCls_Object] = game.thing_lists[TngList_Objects].count;
    count[TCls_Shot] = game.thing_lists[TngList_Shots].count;
    count[TCls_EffectElem] = game.thing_lists[TngList_EffectElems].count;
    count[TCls_DeadCreature] = game.thing_lists[TngList_DeadCreatrs].count;
    count[TCls_Creature] = game.thing_lists[TngList_Creatures].count;
    count[TCls_Effect] = game.thing_lists[TngList_Effects].count;
    count[TCls_EffectGen] = game.thing_lists[TngList_EffectGens].count;
    count[TCls_Trap] = game.thing_lists[TngList_Traps].count;
    count[TCls_Door] = game.thing_lists[TngList_Doors].count;
    count[TCls_AmbientSnd] = game.thing_lists[TngList_AmbientSnds].count;
    count[TCls_CaveIn] = game.thing_lists[TngList_CaveIns].count;
    int total = 0;
    for (i=0; i < THING_CLASSES_COUNT; i++) {
        total += count[i];
    }
    JUSTMSG("Check things: Creats%d, Objs%d, Bods%d, Trps%d, Drs%d, Shts%d, Effs%d, EffEls%d Othrs%d Total%d",
        count[TCls_Creature],
        count[TCls_Object],
        count[TCls_DeadCreature],
        count[TCls_Trap],
        count[TCls_Door],
        count[TCls_Shot],
        count[TCls_Effect],
        count[TCls_EffectElem],
        count[TCls_EffectGen] +  count[TCls_AmbientSnd] + count[TCls_CaveIn],
        total
        );
    for (i=1; i < THINGS_COUNT; i++) {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing)) {
            realcnt[thing->class_id]++;
        }
    }
    int rltotal = 0;
    int rldiffers = 0;
    for (i=0; i < THING_CLASSES_COUNT; i++) {
        rltotal += realcnt[i];
        if (realcnt[i] != count[i])
            rldiffers++;
    }
    if (rldiffers) {
        WARNMSG("Real: Creats%d, Objs%d, Bods%d, Trps%d, Drs%d, Shts%d, Effs%d, EffEls%d Othrs%d Total%d",
            realcnt[TCls_Creature],
            realcnt[TCls_Object],
            realcnt[TCls_DeadCreature],
            realcnt[TCls_Trap],
            realcnt[TCls_Door],
            realcnt[TCls_Shot],
            realcnt[TCls_Effect],
            realcnt[TCls_EffectElem],
            realcnt[TCls_EffectGen] +  realcnt[TCls_AmbientSnd] + realcnt[TCls_CaveIn],
            rltotal
            );
        return true;
    }
    return false;
}

TbBool is_neutral_thing(const struct Thing *thing)
{
    return (thing->owner == game.neutral_player_num);
}

TbBool is_hero_thing(const struct Thing *thing)
{
    return (thing->owner == game.hero_player_num);
}

/**
 * Returns a value which decays around some epicenter, like blast damage.
 *
 * @param magnitude Magnitude in nearest whereabouts of the epicenter.
 * @param decay_start Distance after which the magnitude starts decaying.
 * @param decay_length Length of the decaying region.
 * @param distance Distance at which we want to compute the value.
 * @return Value at specified distane from epicenter.
 */
long get_radially_decaying_value(long magnitude,long decay_start,long decay_length,long distance)
{
  if (distance >= decay_start+decay_length)
    return 0;
  else
  if (distance >= decay_start)
    return magnitude * (decay_length - (distance-decay_start)) / decay_length;
  else
    return magnitude;
}

long compute_creature_kind_score(ThingModel crkind,unsigned short crlevel)
{
    struct CreatureStats* crstat = creature_stats_get(crkind);
    return compute_creature_max_health(crstat->health,crlevel)
        + compute_creature_max_defense(crstat->defense,crlevel)
        + compute_creature_max_dexterity(crstat->dexterity,crlevel)
        + compute_creature_max_armour(crstat->armour,crlevel,false)
        + compute_creature_max_strength(crstat->strength,crlevel);
}

/**
 * Computes max health of a creature on given level.
 */
long compute_creature_max_health(long base_health,unsigned short crlevel)
{
  if (base_health < -100000)
    base_health = -100000;
  if (base_health > 100000)
    base_health = 100000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  long max_health = base_health + (gameadd.crtr_conf.exp.health_increase_on_exp * base_health * (long)crlevel) / 100;
  return saturate_set_signed(max_health, 16);
}

/**
 * Computes gold pay of a creature on given level.
 */
long compute_creature_max_pay(long base_param,unsigned short crlevel)
{
  if (base_param <= 0)
    return 0;
  if (base_param > 100000)
    base_param = 100000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  long max_param = base_param + (gameadd.crtr_conf.exp.pay_increase_on_exp * base_param * (long)crlevel) / 100;
  return saturate_set_signed(max_param, 16);
}

/**
 * Computes defense of a creature on given level.
 */
long compute_creature_max_defense(long base_param,unsigned short crlevel)
{
    if (base_param <= 0)
      return 0;
    if (base_param > 10000)
      base_param = 10000;
    if (crlevel >= CREATURE_MAX_LEVEL)
      crlevel = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (gameadd.crtr_conf.exp.defense_increase_on_exp * base_param * (long)crlevel) / 100;
    return saturate_set_unsigned(max_param, 8);
}

/**
 * Computes dexterity of a creature on given level.
 */
long compute_creature_max_dexterity(long base_param,unsigned short crlevel)
{
  if (base_param <= 0)
    return 0;
  if (base_param > 10000)
    base_param = 10000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  long max_param = base_param + (gameadd.crtr_conf.exp.dexterity_increase_on_exp * base_param * (long)crlevel) / 100;
  return saturate_set_unsigned(max_param, 8);
}

/**
 * Computes strength of a creature on given level.
 */
long compute_creature_max_strength(long base_param,unsigned short crlevel)
{
  if (base_param <= 0)
      return 0;
  if (base_param > 60000)
        base_param = 60000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  long max_param = base_param + (gameadd.crtr_conf.exp.strength_increase_on_exp * base_param * (long)crlevel) / 100;
  return saturate_set_unsigned(max_param, 15);
}

/**
 * Computes loyalty of a creature on given level.
 */
long compute_creature_max_loyalty(long base_param,unsigned short crlevel)
{
  if (base_param <= 0)
      return 0;
  if (base_param > 60000)
      base_param = 60000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  long max_param = base_param + (gameadd.crtr_conf.exp.loyalty_increase_on_exp * base_param * (long)crlevel) / 100;
  return saturate_set_unsigned(max_param, 24);
}

/**
 * Computes armour of a creature on given level.
 */
long compute_creature_max_armour(long base_param, unsigned short crlevel, TbBool armour_spell)
{
  if (base_param <= 0)
     return 0;
  if (base_param > 60000)
     base_param = 60000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  long max_param = base_param + (gameadd.crtr_conf.exp.armour_increase_on_exp * base_param * (long)crlevel) / 100;
  if (armour_spell)
      max_param = (320 * max_param) / 256;
  // This limit makes armor absorb up to 80% of damage, never more
  if (max_param > 204)
      max_param = 204;
  if (max_param < 0)
      max_param = 0;
  return max_param;
}

/**
 * Projects expected damage of a melee attack, taking luck and creature level into account.
 * Uses no random factors - instead, projects a best estimate.
 * This function allows evaluating damage creature can make. It shouldn't be used
 * to actually inflict the damage.
 * @param base_param Base damage.
 * @param luck Creature luck, scaled 0..100.
 * @param crlevel Creature level, 0..9.
 */
long project_creature_attack_melee_damage(long base_param,long luck,unsigned short crlevel)
{
    if (base_param < -60000)
        base_param = -60000;
    if (base_param > 60000)
        base_param = 60000;
    long max_param = base_param;
    if (luck > 0)
    {
        if (luck > 100) luck = 100;
          max_param += luck*max_param/100;
    }
    return saturate_set_signed(max_param, 16);
}

/**
 * Projects expected damage of an attack shot, taking luck and creature level into account.
 * Uses no random factors - instead, projects a best estimate.
 * This function allows evaluating damage creature can make. It shouldn't be used
 * to actually inflict the damage.
 * @param base_param Base damage.
 * @param luck Creature luck, scaled 0..100.
 * @param crlevel Creature level, 0..9.
 */
long project_creature_attack_spell_damage(long base_param,long luck,unsigned short crlevel)
{
    if (base_param < -60000)
        base_param = -60000;
    if (base_param > 60000)
        base_param = 60000;
    if (crlevel >= CREATURE_MAX_LEVEL)
        crlevel = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (gameadd.crtr_conf.exp.spell_damage_increase_on_exp * base_param * (long)crlevel) / 100;
    if (luck > 0)
    {
        if (luck > 100) luck = 100;
          max_param += luck*max_param/100;
    }
    return saturate_set_signed(max_param, 16);
}

/**
 * Computes damage of a melee attack, taking luck and creature level into account.
 * @param base_param Base damage.
 * @param luck Creature luck, scaled 0..100.
 * @param crlevel Creature level, 0..9.
 */
long compute_creature_attack_melee_damage(long base_param, long luck, unsigned short crlevel, struct Thing* thing)
{
    if (base_param < -60000)
        base_param = -60000;
    if (base_param > 60000)
        base_param = 60000;
    long max_param = base_param;
    if (luck > 0)
    {
        if (CREATURE_RANDOM(thing, 100) < luck)
          max_param *= 2;
    }
    return saturate_set_signed(max_param, 16);
}

/**
 * Computes damage of an attack shot, taking luck and creature level into account.
 * @param base_param Base damage.
 * @param luck Creature luck, scaled 0..100.
 * @param crlevel Creature level, 0..9.
 */
long compute_creature_attack_spell_damage(long base_param, long luck, unsigned short crlevel, struct Thing* thing)
{
    if (base_param < -60000)
        base_param = -60000;
    if (base_param > 60000)
        base_param = 60000;
    if (crlevel >= CREATURE_MAX_LEVEL)
        crlevel = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (gameadd.crtr_conf.exp.spell_damage_increase_on_exp * base_param * (long)crlevel) / 100;
    if (luck > 0)
    {
        if (CREATURE_RANDOM(thing, 100) < luck)
          max_param *= 2;
    }
    return saturate_set_signed(max_param, 16);
}

/**
 * Computes spell range/area of effect for a creature on given level.
 */
long compute_creature_attack_range(long base_param, long luck, unsigned short crlevel)
{
  if (base_param <= 0)
    return 0;
  if (base_param > 100000)
    base_param = 100000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  long max_param = base_param + (gameadd.crtr_conf.exp.range_increase_on_exp * base_param * (long)crlevel) / 100;
  return saturate_set_signed(max_param, 16);
}

/**
 * Computes work value, taking creature level into account.
 * The job value is an efficiency of doing a job by a creature.
 * @param base_param Base value of the parameter.
 * @param efficiency Room efficiency, scaled 0..ROOM_EFFICIENCY_MAX.
 * @param crlevel Creature level.
 */
long compute_creature_work_value(long base_param,long efficiency,unsigned short crlevel)
{
  if (base_param < -100000)
      base_param = -100000;
  if (base_param > 100000)
      base_param = 100000;
  if (crlevel >= CREATURE_MAX_LEVEL)
      crlevel = CREATURE_MAX_LEVEL-1;
  if (efficiency > 1024)
      efficiency = 1024;
  long max_param = base_param + (gameadd.crtr_conf.exp.job_value_increase_on_exp * base_param * (long)crlevel) / 100;
  return (max_param * efficiency) / ROOM_EFFICIENCY_MAX;
}

long compute_creature_work_value_for_room_role(const struct Thing *creatng, RoomRole rrole, long efficiency)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    long i = 256;
    if ((rrole & RoRoF_Research) != 0)
    {
        i = compute_creature_work_value(crstat->research_value*256, efficiency, cctrl->explevel);
    }
    if ((rrole & RoRoF_CratesManufctr) != 0)
    {
        i = compute_creature_work_value(crstat->manufacture_value*256, efficiency, cctrl->explevel);
    }
    if ((rrole & RoRoF_CrTrainExp) != 0)
    {
        // Training speed does not grow with experience - otherwise it would be too fast
        i = compute_creature_work_value(crstat->training_value*256, efficiency, 0);
    }
    if ((rrole & RoRoF_CrScavenge) != 0)
    {
        i = compute_creature_work_value(crstat->scavenge_value*256, efficiency, cctrl->explevel);
    }
    return process_work_speed_on_work_value(creatng, i);
}

long compute_controlled_speed_increase(long prev_speed, long speed_limit)
{
    long speed;
    if (speed_limit < 4)
        speed = prev_speed + 1;
    else
        speed = prev_speed + speed_limit/4;
    if (speed < -speed_limit)
        return -speed_limit;
    else
    if (speed > speed_limit)
        return speed_limit;
    return speed;
}

long compute_controlled_speed_decrease(long prev_speed, long speed_limit)
{
    long speed;
    if (speed_limit < 4)
        speed = prev_speed-1;
    else
    speed = prev_speed - speed_limit/4;
    if (speed < -speed_limit)
        return -speed_limit;
    else
    if (speed > speed_limit)
        return speed_limit;
    return speed;
}

long calculate_correct_creature_maxspeed(const struct Thing *thing)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    long speed = crstat->base_speed;
    if (creature_affected_by_slap(thing))
        speed *= 2;
    if (creature_affected_by_spell(thing, SplK_Speed))
        speed *= 2;
    if (creature_affected_by_spell(thing, SplK_Slow))
        speed /= 2;
    if (!is_neutral_thing(thing))
    {
        struct Dungeon* dungeon = get_dungeon(thing->owner);
        if (dungeon->tortured_creatures[thing->model] > 0)
            speed = 5 * speed / 4;
        if (player_uses_power_obey(thing->owner))
            speed = 5 * speed / 4;
    }
    return speed;
}

long calculate_correct_creature_pay(const struct Thing *thing)
{
    struct Dungeon* dungeon = get_dungeon(thing->owner);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    long pay = compute_creature_max_pay(crstat->pay, cctrl->explevel);
    // If torturing creature of that model, halve the salary
    if (dungeon->tortured_creatures[thing->model] > 0)
        pay /= 2;
    return pay;
}

long calculate_correct_creature_scavenge_required(const struct Thing *thing, PlayerNumber callplyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(callplyr_idx);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    long scavngpts = (dungeon->creatures_scavenged[thing->model] + 1) *
        compute_creature_max_loyalty(crstat->scavenge_require, cctrl->explevel);
    return scavngpts;
}

/**
 * Computes parameter (luck,armour) of a creature on given level.
 * Applies for situations where the level doesn't really matters.
 */
long compute_creature_max_unaffected(long base_param,unsigned short crlevel)
{
  if (base_param <= 0)
    return 0;
  if (base_param > 10000)
    base_param = 10000;
  return saturate_set_unsigned(base_param, 8);
}

/** Computes percentage of given value.
 *
 * @param base_val Value to compute percentage of.
 * @param npercent Percentage; 0..100%, but may be higher too.
 * @return Gives npercent of base_val, with proper rounding.
 */
long compute_value_percentage(long base_val, short npercent)
{
    if (base_val > 0)
    {
        if ( base_val > LONG_MAX/(abs(npercent)+1) )
            base_val = LONG_MAX/(abs(npercent)+1);
    } else
    if (base_val < 0)
    {
        if ( base_val < LONG_MIN/(abs(npercent)+1) )
            base_val = LONG_MIN/(abs(npercent)+1);
    }
    return (base_val*(long)npercent+49)/100;
}

/** Computes 8-bit percentage of given value.
 *
 * @param base_val Value to compute percentage of.
 * @param npercent Percentage; 0..256, but may be higher too.
 * @return Gives npercent of base_val, with proper rounding.
 */
long compute_value_8bpercentage(long base_val, short npercent)
{
    if (base_val > 0)
    {
        if ( base_val > LONG_MAX/(abs(npercent)+1) )
            base_val = LONG_MAX/(abs(npercent)+1);
    } else
    if (base_val < 0)
    {
        if ( base_val < LONG_MIN/(abs(npercent)+1) )
            base_val = LONG_MIN/(abs(npercent)+1);
    }
    return (base_val*(long)npercent+127)/256;
}

/**
 * Re-computes max health of a creature and changes it current health to max.
 * @param thing
 * @return
 */
TbBool update_creature_health_to_max(struct Thing *thing)
{
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    cctrl->max_health = compute_creature_max_health(crstat->health,cctrl->explevel);
    thing->health = cctrl->max_health;
    return true;
}

TbBool apply_health_to_thing(struct Thing *thing, long amount)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    HitPoints new_health = thing->health;
    if ((cctrl->max_health != new_health) && (amount > 0))
    {
        new_health += amount;
        if (new_health >= cctrl->max_health)
            new_health = cctrl->max_health;
        thing->health = new_health;
        return true;
    }
    return false;
}

void apply_health_to_thing_and_display_health(struct Thing *thing, long amount)
{
    if (apply_health_to_thing(thing, amount)) {
        thing->creature.health_bar_turns = 8;
    }
}

/**
 * Applies given amount of damage to a creature, with armour based modifier.
 * @param thing The thing which is going to be modified.
 * @param dmg Amount of damage.
 * @return Amount of damage really inflicted.
 * @see apply_damage_to_thing() should be called instead of this function.
 */
static HitPoints apply_damage_to_creature(struct Thing *thing, HitPoints dmg)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    if ((cctrl->flgfield_1 & CCFlg_PreventDamage) != 0) {
        return 0;
    }
    // Compute armor value
    long carmor = compute_creature_max_armour(crstat->armour, cctrl->explevel, creature_affected_by_spell(thing, SplK_Armour));
    // Now compute damage
    HitPoints cdamage = (dmg * (256 - carmor)) / 256;
    if (cdamage <= 0)
      cdamage = 1;
    // Apply damage to the thing
    thing->health -= cdamage;
    thing->field_4F |= TF4F_Unknown80;
    // Red palette if the possessed creature is hit very strong
    if (is_thing_some_way_controlled(thing))
    {
        struct PlayerInfo* player = get_player(thing->owner);
        HitPoints max_health = cctrl->max_health;
        if (max_health < 1)
            max_health = 1;
        long i = (10 * cdamage) / max_health;
        if (i > 10) {
            i = 10;
        } else
        if (i <= 0) {
            i = 1;
        }
        PaletteApplyPainToPlayer(player, i);
    }
    return cdamage;
}

static HitPoints apply_damage_to_object(struct Thing *thing, HitPoints dmg)
{
    HitPoints cdamage = dmg;
    thing->health -= cdamage;
    thing->field_4F |= TF4F_Unknown80;
    return cdamage;
}

static HitPoints apply_damage_to_door(struct Thing *thing, HitPoints dmg)
{
    HitPoints cdamage = dmg;
    thing->health -= cdamage;
    return cdamage;
}

HitPoints calculate_shot_real_damage_to_door(const struct Thing *doortng, const struct Thing *shotng)
{
    HitPoints dmg;
    const struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    const struct ObjectConfig* objconf = get_object_model_stats2(door_crate_object_model(doortng->model));
    //TODO CONFIG replace deals_physical_damage with check for shotst->damage_type (magic in this sense is DmgT_Electric, DmgT_Combustion and DmgT_Heatburn)
    if ( !objconf->resistant_to_nonmagic || (shotst->damage_type == DmgT_Magical))
    {
        dmg = shotng->shot.damage;
    } else
    {
        dmg = shotng->shot.damage / 10;
        if (dmg < 1)
            dmg = 1;
    }
    return dmg;
}

/**
 * Applies given damage points to a thing.
 * In case of targeting creature, uses its defense values to compute the actual damage.
 * Can be used only to make damage - never to heal creature.
 *
 * @param thing
 * @param dmg
 * @param damage_type
 * @param inflicting_plyr_idx
 * @return Amount of damage really inflicted.
 */
HitPoints apply_damage_to_thing(struct Thing *thing, HitPoints dmg, DamageType damage_type, PlayerNumber dealing_plyr_idx)
{
    // We're here to damage, not to heal
    SYNCDBG(19,"Dealing %d damage to %s by player %d",(int)dmg,thing_model_name(thing),(int)dealing_plyr_idx);
    if (dmg <= 0)
        return 0;
    // If it's already dead, then don't interfere
    if (thing->health < 0)
        return 0;
    HitPoints cdamage;
    switch (thing->class_id)
    {
    case TCls_Creature:
        cdamage = apply_damage_to_creature(thing, dmg);
        break;
    case TCls_Object:
        cdamage = apply_damage_to_object(thing, dmg);
        break;
    case TCls_Door:
        cdamage = apply_damage_to_door(thing, dmg);
        break;
    default:
        cdamage = 0;
        break;
    }
    if ((thing->class_id == TCls_Creature) && (thing->health < 0))
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if ((cctrl->fighting_player_idx == -1) && (dealing_plyr_idx != -1))
        {
            cctrl->fighting_player_idx = dealing_plyr_idx;
        }
    }
    return cdamage;
}

long calculate_damage_did_to_slab_with_single_hit(const struct Thing *diggertng, const struct SlabMap *slb)
{
    long dig_damage;
    if (slabmap_owner(slb) == diggertng->owner)
        dig_damage = game.default_imp_dig_own_damage;
    else
        dig_damage = game.default_imp_dig_damage;
    return dig_damage;
}

long calculate_gold_digged_out_of_slab_with_single_hit(long damage_did_to_slab, PlayerNumber plyr_idx, unsigned short crlevel, const struct SlabMap *slb)
{
    long gold = (damage_did_to_slab * (long)game.gold_per_gold_block) / game.block_health[1];
    if (slb->kind == SlbT_GEMS)
      gold = gold * gameadd.gem_effectiveness / 100;
    if (gold <= 1)
      return 1;
    return gold;
}

long compute_creature_weight(const struct Thing* creatng, struct CreatureStats* crstat, struct CreatureControl* cctrl)
{
    long eye_height = (crstat->eye_height + (crstat->eye_height * gameadd.crtr_conf.exp.size_increase_on_exp * cctrl->explevel) / 100);
    long weight = eye_height >> 2;
    weight += (crstat->hunger_fill + crstat->lair_size + 1) * cctrl->explevel;
    
    if (!crstat->affected_by_wind)
    {
        weight = weight * 3 / 2;
    }
    
    if ((get_creature_model_flags(creatng) & CMF_TremblingFat) != 0)
    {
        weight = weight * 3 / 2;
    }

    if ((get_creature_model_flags(creatng) & CMF_IsDiptera) != 0)
    {
        weight = weight / 2;
    }

    if (crstat->can_go_locked_doors == true)
    {
        weight = weight / 10;
    }

    return weight;
}

const char *creature_statistic_text(const struct Thing *creatng, CreatureLiveStatId clstat_id)
{
    const char *text;
    struct CreatureStats* crstat = creature_stats_get_from_thing(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    long i;
    static char loc_text[16];
    switch (clstat_id)
    {
    case CrLStat_FirstName:
        text = creature_own_name(creatng);
        break;
    case CrLStat_BloodType:
        i = cctrl->blood_type;
        text = blood_types[i%BLOOD_TYPES_COUNT];
        break;
    case CrLStat_ExpLevel:
        i = cctrl->explevel + 1;
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Health:
        i = creatng->health;
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_MaxHealth:
        i = compute_creature_max_health(crstat->health,cctrl->explevel);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Strength:
        i = compute_creature_max_strength(crstat->strength,cctrl->explevel);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Armour:
        i = compute_creature_max_armour(crstat->armour,cctrl->explevel,creature_affected_by_spell(creatng, SplK_Armour));
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Defence:
        i = compute_creature_max_defense(crstat->defense,cctrl->explevel);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Dexterity:
        i = compute_creature_max_dexterity(crstat->dexterity,cctrl->explevel);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Luck:
        i = compute_creature_max_luck(crstat->luck,cctrl->explevel);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Speed:
        i = calculate_correct_creature_maxspeed(creatng);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Loyalty:
        i = compute_creature_max_loyalty(crstat->scavenge_require,cctrl->explevel);
        snprintf(loc_text,sizeof(loc_text),"%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_AgeTime:
        i = (game.play_gameturn-creatng->creation_turn) / 1200; // + cctrl->joining_age;
        if (i >= 999)
          i = 999;
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Kills:
        i = cctrl->kills_num;
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_GoldHeld:
        i = creatng->creature.gold_carried;
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_GoldWage:
        i = calculate_correct_creature_pay(creatng);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Score:
        i = compute_creature_kind_score(creatng->model,cctrl->explevel);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_ResearchSkill:
        i = compute_creature_work_value_for_room_role(creatng, RoRoF_Research, ROOM_EFFICIENCY_MAX);
        snprintf(loc_text,sizeof(loc_text),"%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_ManufactureSkill:
        i = compute_creature_work_value_for_room_role(creatng, RoRoF_CratesManufctr, ROOM_EFFICIENCY_MAX);
        snprintf(loc_text,sizeof(loc_text),"%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_TrainingSkill:
        i = compute_creature_work_value_for_room_role(creatng, RoRoF_CrTrainExp, ROOM_EFFICIENCY_MAX);
        snprintf(loc_text,sizeof(loc_text),"%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_ScavengeSkill:
        i = compute_creature_work_value_for_room_role(creatng, RoRoF_CrScavenge, ROOM_EFFICIENCY_MAX);
        snprintf(loc_text,sizeof(loc_text),"%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_TrainingCost:
        i = crstat->training_cost;
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_ScavengeCost:
        i = crstat->scavenger_cost;
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_Weight:
        i = compute_creature_weight(creatng, crstat, cctrl);
        snprintf(loc_text,sizeof(loc_text),"%ld", i);
        text = loc_text;
        break;
    case CrLStat_BestDamage:
        //TODO compute damage of best attack
        text = lbEmptyString;
        break;
    default:
        ERRORLOG("Invalid statistic %d",(int)clstat_id);
        text = lbEmptyString;
        break;
    }
    return text;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
