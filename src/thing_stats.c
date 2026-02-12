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
#include "pre_inc.h"

#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_inputctrl.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_effects.h"
#include "config_magic.h"
#include "config_objects.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "creature_control.h"
#include "creature_states.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "globals.h"
#include "player_data.h"
#include "player_instances.h"
#include "player_utils.h"
#include "thing_effects.h"
#include "thing_list.h"
#include "thing_physics.h"
#include "thing_stats.h"
#include "vidfade.h"
#include "lua_triggers.h"

#include "post_inc.h"

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
    "UNUSEDPARAM10",
    "UNUSEDPARAM11",
    "AMBIENTSND",
    "CAVEIN",
    "UNUSEDPARAM14",
};
/******************************************************************************/
const char *thing_class_code_name(ThingClass class_id)
{
    if ((class_id >= sizeof(thing_classes)/sizeof(thing_classes[0])))
    {
        return "INVALID";
    }
    return thing_classes[class_id];
}

/**
 * Gives name of a thing model.
 * @note This function cannot be called more than once in a parameter to something - it has only one static buffer.
 * @param thing The thing which model is to be described.
 * @return The model name string, static buffer.
 */
const char *thing_class_and_model_name(ThingClass class_id, ThingModel model)
{
    static char name_buffer[4][64];
    static int bid = 0;
    bid = (bid+1)%4;
    switch (class_id)
    {
    case TCls_Creature:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "creature %s", creature_code_name(model));
        break;
    case TCls_DeadCreature:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "dead %s", creature_code_name(model));
        break;
    case TCls_Trap:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "%s trap", trap_code_name(model));
        break;
    case TCls_Door:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "%s door", door_code_name(model));
        break;
    case TCls_Shot:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "%s shot", shot_code_name(model));
        break;
    case TCls_Object:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "object %s", object_code_name(model));
        break;
    case TCls_Effect:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "%s effect", effect_code_name(model));
        break;
    case TCls_EffectElem:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "%s effect element", effect_element_code_name(model));
        break;
    case TCls_EffectGen:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "%s effectgenerator", effectgenerator_code_name(model));
        break;
    default:
        snprintf(name_buffer[bid], sizeof(name_buffer[0]), "%s model %d", thing_class_code_name(class_id), (int)model);
        break;
    }
    return name_buffer[bid];
}

/**
 * Gives name of a thing model.
 * @note This function cannot be called more than once in a parameter to something - it has only one static buffer.
 * @param thing The thing which model is to be described.
 * @return The model name string, static buffer.
 */
const char *thing_model_name(const struct Thing *thing)
{
    return thing_class_and_model_name(thing->class_id, thing->model);
}

const char *thing_model_only_name(ThingClass class_id, ThingModel model)
{
        switch (class_id)
    {
    case TCls_Creature:
        return creature_code_name(model);
        break;
    case TCls_DeadCreature:
        return creature_code_name(model);
        break;
    case TCls_Trap:
        return trap_code_name(model);
        break;
    case TCls_Door:
        return door_code_name(model);
        break;
    case TCls_Shot:
        return shot_code_name(model);
        break;
    case TCls_Object:
        return object_code_name(model);
        break;
    case TCls_Effect:
        return effect_code_name(model);
        break;
    case TCls_EffectGen:
        return effectgenerator_code_name(model);
        break;
    default:
        return thing_class_code_name(class_id);
        break;
    }
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
    for (i=0; i < THING_CLASSES_COUNT; i++)
    {
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
    for (i=0; i < THING_CLASSES_COUNT; i++)
    {
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
        count[TCls_EffectGen] + count[TCls_AmbientSnd] + count[TCls_CaveIn],
        total
        );
    for (i=1; i < THINGS_COUNT; i++)
    {
        struct Thing* thing = thing_get(i);
        if (thing_exists(thing))
        {
            realcnt[thing->class_id]++;
        }
    }
    int rltotal = 0;
    int rldiffers = 0;
    for (i=0; i < THING_CLASSES_COUNT; i++)
    {
        rltotal += realcnt[i];
        if (realcnt[i] != count[i])
        {
            rldiffers++;
        }
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
            realcnt[TCls_EffectGen] + realcnt[TCls_AmbientSnd] + realcnt[TCls_CaveIn],
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
    return (player_is_roaming(thing->owner));
}

/**
 * Returns a value which decays around some epicenter, like blast damage.
 * @param magnitude Magnitude in nearest whereabouts of the epicenter.
 * @param decay_start Distance after which the magnitude starts decaying.
 * @param decay_length Length of the decaying region.
 * @param distance Distance at which we want to compute the value.
 * @return Value at specified distance from epicenter.
 */
long get_radially_decaying_value(long magnitude, long decay_start, long decay_length, long distance)
{
    if (distance >= decay_start + decay_length)
    {
        return 0;
    }
    else if (distance >= decay_start)
    {
        return magnitude * (decay_length - (distance-decay_start)) / decay_length;
    }
    else
    {
        return magnitude;
    }
}

/**
 * Returns a value which is stronger around some epicenter but can't go beyond, like implosion damage.
 * @param magnitude Magnitude in nearest whereabouts of the epicenter.
 * @param decay_start Distance after which the magnitude starts decaying.
 * @param decay_length Length of the decaying region.
 * @param distance Distance at which we want to compute the value.
 * @param friction is used to calculate the deacceleration and therefore the expected distance travelled.
 * @return Value at how fast it's pulled to epicenter.
 */
long get_radially_growing_value(long magnitude, long decay_start, long decay_length, long distance, long friction)
{
    if (distance >= decay_start + decay_length)
    {
        return 0; // Outside the max range, nothing is pulled inwards.
    }
    if (distance >= decay_start) // Too far away to pull with full power.
    {
        if (decay_length == 0)
        {
            decay_length = 1;
        }
        magnitude = magnitude * (decay_length - (distance - decay_start)) / decay_length;
    }
    if (friction == 0)
    {
        friction = 1;
    }
    long total_distance = abs((COORD_PER_STL / friction * magnitude + magnitude) / 2); // The intended distance to push the thing.
    if (total_distance > distance) // Never return a value that would go past the epicentre.
    {
        short factor = COORD_PER_STL / friction * 3 / 4; // Creatures slide so move further then expected.
        if (factor == 0)
        {
            factor = 1;
        }
        return -(distance / factor);
    }
    return magnitude;
}

long compute_creature_kind_score(ThingModel crkind, CrtrExpLevel exp_level)
{
    struct CreatureModelConfig* crconf = creature_stats_get(crkind);
    return compute_creature_max_health(crconf->health, exp_level)
           + compute_creature_max_defense(crconf->defense, exp_level)
           + compute_creature_max_dexterity(crconf->dexterity, exp_level)
           + compute_creature_max_armour(crconf->armour, exp_level)
           + compute_creature_max_strength(crconf->strength, exp_level);
}

/* Computes max health of a creature on given level. */
HitPoints compute_creature_max_health(HitPoints base_health, CrtrExpLevel exp_level)
{
    if (exp_level >= CREATURE_MAX_LEVEL)
    {
        exp_level = CREATURE_MAX_LEVEL-1;
    }
    // Compute max health using 64-bit arithmetic to ensure precision when multiplied by 'health_increase_on_exp'.
    int64_t compute_max_health = (int64_t)base_health + ((int64_t)game.conf.crtr_conf.exp.health_increase_on_exp * (int64_t)base_health * (int64_t)exp_level) / 100;
    if (compute_max_health >= INT32_MAX)
    {
        compute_max_health = INT32_MAX;
    }
    HitPoints max_health = compute_max_health;
    return max_health;
}

/* Computes strength of a creature on given level. */
long compute_creature_max_strength(long base_param, CrtrExpLevel exp_level)
{
    if (exp_level >= CREATURE_MAX_LEVEL)
    {
        exp_level = CREATURE_MAX_LEVEL-1;
    }
    long max_param = base_param + (game.conf.crtr_conf.exp.strength_increase_on_exp * base_param * (long)exp_level) / 100;
    if (flag_is_set(game.conf.rules[0].game.classic_bugs_flags, ClscBug_Overflow8bitVal))
    {
        return min(max_param, UCHAR_MAX+1); // DK1 limited shot damage to 256, not 255.
    }
    return max_param;
}

/* Computes armour of a creature on given level. */
long compute_creature_max_armour(long base_param, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 60000)
        base_param = 60000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (game.conf.crtr_conf.exp.armour_increase_on_exp * base_param * (long)exp_level) / 100;
    return max_param;
}

/* Computes defense of a creature on given level. */
long compute_creature_max_defense(long base_param, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 10000)
        base_param = 10000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (game.conf.crtr_conf.exp.defense_increase_on_exp * base_param * (long)exp_level) / 100;
    unsigned long long overflow = (1 << (8)) - 1;
    if ((max_param >= overflow) && (!emulate_integer_overflow(8)))
        return overflow; // This is for maps with ClscBug_Overflow8bitVal flag enabled.
    return max_param;
}

/* Computes dexterity of a creature on given level. */
long compute_creature_max_dexterity(long base_param, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 10000)
        base_param = 10000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (game.conf.crtr_conf.exp.dexterity_increase_on_exp * base_param * (long)exp_level) / 100;
    return saturate_set_unsigned(max_param, 8);
}

/* Computes loyalty of a creature on given level. */
long compute_creature_max_loyalty(long base_param, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 60000)
        base_param = 60000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (game.conf.crtr_conf.exp.loyalty_increase_on_exp * base_param * (long)exp_level) / 100;
    return saturate_set_unsigned(max_param, 24);
}

/* Computes salary of a creature on given level. */
GoldAmount compute_creature_max_pay(GoldAmount base_param, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 100000)
        base_param = 100000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    GoldAmount max_param = base_param + (game.conf.crtr_conf.exp.pay_increase_on_exp * base_param * (long)exp_level) / 100;
    return saturate_set_signed(max_param, 16);
}

/* Computes training cost of a creature on given level. */
GoldAmount compute_creature_max_training_cost(GoldAmount base_param, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 100000)
        base_param = 100000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    GoldAmount max_param = base_param + (game.conf.crtr_conf.exp.training_cost_increase_on_exp * base_param * (long)exp_level) / 100;
    return saturate_set_signed(max_param, 16);
}

/* Computes scavenging cost of a creature on given level. */
GoldAmount compute_creature_max_scavenging_cost(GoldAmount base_param, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 100000)
        base_param = 100000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    GoldAmount max_param = base_param + (game.conf.crtr_conf.exp.scavenging_cost_increase_on_exp * base_param * (long)exp_level) / 100;
    return saturate_set_signed(max_param, 16);
}

/**
 * Projects expected damage of a melee attack, taking luck and creature level into account.
 * Uses no random factors - instead, projects a best estimate.
 * This function allows evaluating damage creature can make. It shouldn't be used to actually inflict the damage.
 * @param base_param Base damage.
 * @param luck Creature luck, scaled 0..100.
 * @param exp_level Creature level, 0..9.
 */
long project_creature_attack_melee_damage(long base_param, short damage_percent, long luck, CrtrExpLevel exp_level, const struct Thing* thing)
{
    long max_param = base_param;
    if (damage_percent != 0)
    {
        max_param = (max_param * damage_percent) / 100;
    }
    if (luck > 0)
    {
        if (luck > 100) luck = 100;
            max_param += luck*max_param/100;
    }
    return max_param;
}

/**
 * Computes damage of a melee attack, taking luck and creature level into account.
 * @param base_param Base damage.
 * @param luck Creature luck, scaled 0..100.
 * @param exp_level Creature level, 0..9.
 */
long compute_creature_attack_melee_damage(long base_param, long luck, CrtrExpLevel exp_level, struct Thing* thing)
{
    long max_param = base_param;
    if (luck > 0)
    {
        if (THING_RANDOM(thing, 100) < luck)
            max_param *= 2;
    }
    return max_param;
}

/**
 * Computes damage of an attack shot, taking luck and creature level into account.
 * @param base_param Base damage.
 * @param luck Creature luck, scaled 0..100.
 * @param exp_level Creature level, 0..9.
 */
long compute_creature_attack_spell_damage(long base_param, long luck, CrtrExpLevel exp_level, PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (game.conf.crtr_conf.exp.spell_damage_increase_on_exp * base_param * (long)exp_level) / 100;
    // Apply modifier.
    if (!dungeon_invalid(dungeon))
    {
        unsigned short modifier = dungeon->modifier.spell_damage;
        max_param = (max_param * modifier) / 100;
    }
    if (luck > 0)
    {
        if (PLAYER_RANDOM(plyr_idx,100) < luck)
            max_param *= 2;
    }
    return max_param;
}

/* Computes spell range/area of effect for a creature on given level. */
long compute_creature_attack_range(long base_param, long luck, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 100000)
        base_param = 100000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    long max_param = base_param + (game.conf.crtr_conf.exp.range_increase_on_exp * base_param * (long)exp_level) / 100;
    return saturate_set_signed(max_param, 16);
}

/**
 * Computes damage of a spell with damage over time.
 * @param spell_damage Base Damage.
 * @param caster_level Caster Level.
 * @param caster_owner Caster Owner.
 */
HitPoints compute_creature_spell_damage_over_time(HitPoints spell_damage, CrtrExpLevel caster_level, PlayerNumber caster_owner)
{
    struct Dungeon* dungeon;
    if (caster_level >= CREATURE_MAX_LEVEL)
    {
        caster_level = CREATURE_MAX_LEVEL-1;
    }
    HitPoints max_damage = spell_damage + (game.conf.crtr_conf.exp.spell_damage_increase_on_exp * spell_damage * caster_level) / 100;
    // Apply modifier.
    if (!player_is_neutral(caster_owner))
    {
        dungeon = get_dungeon(caster_owner);
        unsigned short modifier = dungeon->modifier.spell_damage;
        max_damage = (max_damage * modifier) / 100;
    }
    return max_damage;
}

/**
 * Computes work value, taking creature level into account.
 * The job value is an efficiency of doing a job by a creature.
 * @param base_param Base value of the parameter.
 * @param efficiency Room efficiency, scaled 0..ROOM_EFFICIENCY_MAX.
 * @param exp_level Creature level.
 */
long compute_creature_work_value(long base_param, long efficiency, CrtrExpLevel exp_level)
{
    if (base_param < -100000)
        base_param = -100000;
    if (base_param > 100000)
        base_param = 100000;
    if (exp_level >= CREATURE_MAX_LEVEL)
        exp_level = CREATURE_MAX_LEVEL-1;
    if (efficiency > 1024)
        efficiency = 1024;
    long max_param = base_param + (game.conf.crtr_conf.exp.job_value_increase_on_exp * base_param * (long)exp_level) / 100;
    return (max_param * efficiency) / ROOM_EFFICIENCY_MAX;
}

long compute_creature_work_value_for_room_role(const struct Thing *creatng, RoomRole rrole, long efficiency)
{
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    long i = 256;
    if ((rrole & RoRoF_Research) != 0)
    {
        i = compute_creature_work_value(crconf->research_value* game.conf.rules[creatng->owner].rooms.research_efficiency, efficiency, cctrl->exp_level);
    }
    if ((rrole & RoRoF_CratesManufctr) != 0)
    {
        i = compute_creature_work_value(crconf->manufacture_value* game.conf.rules[creatng->owner].rooms.work_efficiency, efficiency, cctrl->exp_level);
    }
    if ((rrole & RoRoF_CrTrainExp) != 0)
    {
        // Training speed does not grow with experience - otherwise it would be too fast.
        i = compute_creature_work_value(crconf->training_value* game.conf.rules[creatng->owner].rooms.train_efficiency, efficiency, 0);
    }
    if ((rrole & RoRoF_CrScavenge) != 0)
    {
        i = compute_creature_work_value(crconf->scavenge_value* game.conf.rules[creatng->owner].rooms.scavenge_efficiency, efficiency, cctrl->exp_level);
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

HitPoints calculate_correct_creature_max_health(const struct Thing *thing)
{
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    HitPoints max_health = compute_creature_max_health(crconf->health, cctrl->exp_level);
    // Apply modifier.
    if (!is_neutral_thing(thing))
    {
        dungeon = get_dungeon(thing->owner);
        unsigned short modifier = dungeon->modifier.health;
        // Compute max health using 64-bit arithmetic to ensure precision when multiplied by 'modifier'.
        int64_t compute_max_health = ((int64_t)max_health * (int64_t)modifier) / 100;
        if (compute_max_health >= INT32_MAX)
        {
            compute_max_health = INT32_MAX;
        }
        max_health = compute_max_health;
    }
    return max_health;
}

long calculate_correct_creature_strength(const struct Thing *thing)
{
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    long max_param = compute_creature_max_strength(crconf->strength, cctrl->exp_level);
    // Apply modifier.
    if (!is_neutral_thing(thing))
    {
        dungeon = get_dungeon(thing->owner);
        unsigned short modifier = dungeon->modifier.strength;
        max_param = (max_param * modifier) / 100;
    }
    return max_param;
}

long calculate_correct_creature_armour(const struct Thing *thing)
{
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    long max_param = compute_creature_max_armour(crconf->armour, cctrl->exp_level);
    if (creature_under_spell_effect(thing, CSAfF_Armour))
        max_param = (320 * max_param) / 256;
    // This limit makes armour absorb up to 80% of damage even with the buff.
    if (max_param > 204)
        max_param = 204;
    if (max_param < 0)
        max_param = 0;
    // Apply modifier after the buff.
    if (!is_neutral_thing(thing))
    {
        dungeon = get_dungeon(thing->owner);
        unsigned short modifier = dungeon->modifier.armour;
        max_param = (max_param * modifier) / 100;
    }
    // Value cannot exceed 255 with modifier.
    if (max_param >= 255)
        max_param = 255;
    return max_param;
}

long calculate_correct_creature_defense(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    long max_param = compute_creature_max_defense(crconf->defense, cctrl->exp_level);
    // TODO: Add a dungeon modifier.
    return max_param;
}

long calculate_correct_creature_dexterity(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    long max_param = compute_creature_max_dexterity(crconf->dexterity, cctrl->exp_level);
    // TODO: Add a dungeon modifier.
    return max_param;
}

long calculate_correct_creature_maxspeed(const struct Thing *thing)
{
    struct Dungeon* dungeon;
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    long speed = crconf->base_speed;
    if ((creature_affected_by_slap(thing)) || (creature_under_spell_effect(thing, CSAfF_Timebomb)))
        speed *= 2;
    if (creature_under_spell_effect(thing, CSAfF_Speed))
        speed *= 2;
    if (creature_under_spell_effect(thing, CSAfF_Slow))
        speed /= 2;
    // Apply modifier.
    if (!is_neutral_thing(thing))
    {
        dungeon = get_dungeon(thing->owner);
        unsigned short modifier = dungeon->modifier.speed;
        speed = (speed * modifier) / 100;
        if (dungeon->tortured_creatures[thing->model] > 0)
            speed = 5 * speed / 4;
        if (player_uses_power_obey(thing->owner))
            speed = 5 * speed / 4;
    }
    return speed;
}

long calculate_correct_creature_loyalty(const struct Thing *thing)
{
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    long max_param = compute_creature_max_loyalty(crconf->scavenge_require, cctrl->exp_level);
    // Apply modifier.
    if (!is_neutral_thing(thing))
    {
        dungeon = get_dungeon(thing->owner);
        unsigned short modifier = dungeon->modifier.loyalty;
        max_param = (max_param * modifier) / 100;
    }
    return max_param;
}

GoldAmount calculate_correct_creature_pay(const struct Thing *thing)
{
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    GoldAmount pay = compute_creature_max_pay(crconf->pay, cctrl->exp_level);
    // Apply modifier.
    if (!is_neutral_thing(thing))
    {
        dungeon = get_dungeon(thing->owner);
        unsigned short modifier = dungeon->modifier.pay;
        pay = (pay * modifier) / 100;
        // If torturing creature of that model, change the salary with a percentage set in rules.cfg.
        if (dungeon->tortured_creatures[thing->model] > 0)
            pay = (pay * game.conf.rules[dungeon->owner].game.torture_payday) / 100;
    }
    return pay;
}

GoldAmount calculate_correct_creature_training_cost(const struct Thing *thing)
{
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    GoldAmount training_cost = compute_creature_max_training_cost(crconf->training_cost, cctrl->exp_level);
    // Apply modifier.
    if (!is_neutral_thing(thing))
    {
        dungeon = get_dungeon(thing->owner);
        unsigned short modifier = dungeon->modifier.training_cost;
        training_cost = (training_cost * modifier) / 100;
        // If torturing creature of that model, change the training cost with a percentage set in rules.cfg.
        if (dungeon->tortured_creatures[thing->model] > 0)
            training_cost = (training_cost * game.conf.rules[dungeon->owner].game.torture_training_cost) / 100;
    }
    return training_cost;
}

GoldAmount calculate_correct_creature_scavenging_cost(const struct Thing *thing)
{
    struct Dungeon* dungeon;
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    GoldAmount scavenger_cost = compute_creature_max_scavenging_cost(crconf->scavenger_cost, cctrl->exp_level);
    // Apply modifier.
    if (!is_neutral_thing(thing))
    {
        dungeon = get_dungeon(thing->owner);
        unsigned short modifier = dungeon->modifier.scavenging_cost;
        scavenger_cost = (scavenger_cost * modifier) / 100;
        // If torturing creature of that model, change the scavenging cost with a percentage set in rules.cfg.
        if (dungeon->tortured_creatures[thing->model] > 0)
            scavenger_cost = (scavenger_cost * game.conf.rules[dungeon->owner].game.torture_scavenging_cost) / 100;
    }
    return scavenger_cost;
}

long calculate_correct_creature_scavenge_required(const struct Thing *thing, PlayerNumber callplyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(callplyr_idx);
    long scavngpts = (dungeon->creatures_scavenged[thing->model] + 1) * calculate_correct_creature_loyalty(thing);
    return scavngpts;
}

/* Computes parameter (luck, armour) of a creature on given level. Applies for situations where the level doesn't really matters. */
long compute_creature_max_unaffected(long base_param, CrtrExpLevel exp_level)
{
    if (base_param <= 0)
        return 0;
    if (base_param > 10000)
        base_param = 10000;
    // TODO: Need to remove this and make new function 'compute_creature_max_luck' along with 'calculate_correct_creature_luck' for a luck dungeon modifier.
    return saturate_set_unsigned(base_param, 8);
}

/** Computes percentage of given value.
 * @param base_val Value to compute percentage of.
 * @param npercent Percentage; 0..100%, but may be higher too.
 * @return Gives npercent of base_val, with proper rounding.
 */
long compute_value_percentage(long base_val, short npercent)
{
    if (base_val > 0)
    {
        if (base_val > INT32_MAX/(abs(npercent)+1))
            base_val = INT32_MAX/(abs(npercent)+1);
    } else
    if (base_val < 0)
    {
        if (base_val < INT32_MIN/(abs(npercent)+1))
            base_val = INT32_MIN/(abs(npercent)+1);
    }
    return (base_val*(long)npercent+49)/100;
}

/**
 * Re-computes max health of a creature and changes it current health to max.
 * @param thing
 * @return
 */
TbBool update_creature_health_to_max(struct Thing * creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->max_health = calculate_correct_creature_max_health(creatng);
    creatng->health = cctrl->max_health;
    return true;
}

HitPoints get_thing_max_health(const struct Thing *thing)
{
    switch (thing->class_id)
    {
    case TCls_Creature:
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        return cctrl->max_health;
    case TCls_Object:
        struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
        return objst->health;
    case TCls_Door:
        struct DoorConfigStats* doorst = get_door_model_stats(thing->model);
        return doorst->health;
    case TCls_Shot:
        struct ShotConfigStats* shotst = get_shot_model_stats(thing->model);
        return shotst->health;
    case TCls_Trap:
        struct TrapConfigStats* trapst = get_trap_model_stats(thing->model);
        return trapst->health;
    case TCls_EffectElem:
    case TCls_EffectGen:
    default:
        ERRORLOG("class %s not supported in get_thing_max_health()", thing_class_code_name(thing->class_id) );
        return 0;
    }
}

/**
 * Re-computes new max health of a creature and updates the health value to stay relative to the old max.
 * @param thing
 * @return
 */
TbBool update_relative_creature_health(struct Thing* creatng)
{
    HitPoints health_permil = get_creature_health_permil(creatng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    cctrl->max_health = calculate_correct_creature_max_health(creatng);
    // Compute max health using 64-bit arithmetic to ensure precision when multiplied by 'health_permil'.
    int64_t health_scaled = (int64_t)cctrl->max_health * (int64_t)health_permil / 1000;
    creatng->health = health_scaled;
    return true;
}

TbBool set_creature_health_to_max_with_heal_effect(struct Thing* thing)
{ // Hardcoded function for 'SpcKind_HealAll'. TODO: Refactor when specials are made more configurable.
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (cctrl->max_health > thing->health) // 'SpcKind_HealAll' bypasses immunity.
    {
        cctrl->spell_aura = -TngEffElm_Heal;
        cctrl->spell_aura_duration = 50;
        thing->health = cctrl->max_health;
    }
    return true;
}

TbBool apply_health_to_thing(struct Thing *thing, HitPoints amount)
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

void apply_health_to_thing_and_display_health(struct Thing *thing, HitPoints amount)
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
    if ((cctrl->creature_control_flags & CCFlg_PreventDamage) != 0) {
        return 0;
    }
    // Get correct armour value.
    long carmor = calculate_correct_creature_armour(thing);
    // Now compute damage.
    HitPoints cdamage = (dmg * (256 - carmor)) / 256;
    if (cdamage <= 0)
      cdamage = 1;
    // Apply damage to the thing.
    thing->health -= cdamage;
    thing->rendering_flags |= TRF_BeingHit;
    // Red palette if the possessed creature is hit very strong.
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

        if (is_my_player(player))
        {
            controller_rumble(100);
        }
    }
    return cdamage;
}

static HitPoints apply_damage_to_object(struct Thing *thing, HitPoints dmg)
{
    HitPoints cdamage = dmg;
    thing->health -= cdamage;
    thing->rendering_flags |= TRF_BeingHit;
    return cdamage;
}

static HitPoints apply_damage_to_door(struct Thing *thing, HitPoints dmg)
{
    HitPoints cdamage = dmg;
    thing->health -= cdamage;
    return cdamage;
}

HitPoints reduce_damage_for_midas(PlayerNumber owner, HitPoints damage, short multiplier)
{
    if (multiplier == 0)
        return 0;
    HitPoints cost = (damage + multiplier - 1) / multiplier; // This ensures we round up the division.
    GoldAmount received = take_money_from_dungeon(owner, cost, 0); // Take gold from the player.
    return (received * multiplier);
}

HitPoints calculate_shot_real_damage_to_door(const struct Thing *doortng, const struct Thing *shotng)
{
    HitPoints dmg;
    const struct ShotConfigStats* shotst = get_shot_model_stats(shotng->model);
    const struct DoorConfigStats* doorst = get_door_model_stats(doortng->model);
    if (flag_is_set(doorst->model_flags, DoMF_ResistNonMagic) && (!shotst->is_magical))
    {
        dmg = shotng->shot.damage / 8;
        if (dmg < 1)
        {
            dmg = 1;
        }
    }
    else
    {
        dmg = shotng->shot.damage;
    }
    if (flag_is_set(doorst->model_flags, DoMF_Midas))
    {
        HitPoints absorbed = reduce_damage_for_midas(doortng->owner, dmg, doorst->health);
        dmg -= absorbed;
        // Generate effects for the gold taken.
        for (int i = absorbed; i > 0; i -= 32)
        {
            create_effect(&shotng->mappos, TngEff_CoinFountain, doortng->owner);
        }
    }
    return dmg;
}

/**
 * Applies given damage points to a thing.
 * In case of targeting creature, uses its defense values to compute the actual damage.
 * Can be used only to make damage - never to heal creature.
 * @param thing
 * @param dmg
 * @param inflicting_plyr_idx
 * @return Amount of damage really inflicted.
 */
HitPoints apply_damage_to_thing(struct Thing *thing, HitPoints dmg, PlayerNumber dealing_plyr_idx)
{
    // We're here to damage, not to heal.
    SYNCDBG(19, "Dealing %d damage to %s by player %d", (int)dmg, thing_model_name(thing), (int)dealing_plyr_idx);
    if (dmg <= 0)
        return 0;
    // If it's already dead, then don't interfere.
    if (thing->health < 0)
        return 0;
    lua_on_apply_damage_to_thing(thing, dmg, dealing_plyr_idx);

    HitPoints cdamage;
    switch (thing->class_id)
    {
    case TCls_Creature:
        cdamage = apply_damage_to_creature(thing, dmg);
        if (thing->health < 0)
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
            if ((cctrl->fighting_player_idx == -1) && (dealing_plyr_idx != -1))
            {
                cctrl->fighting_player_idx = dealing_plyr_idx;
            }
        }
        break;
    case TCls_Trap:
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
    return cdamage;
}

long calculate_damage_did_to_slab_with_single_hit(const struct Thing *diggertng, const struct SlabMap *slb)
{
    long dig_damage;
    if (slabmap_owner(slb) == diggertng->owner)
        dig_damage = game.conf.rules[diggertng->owner].workers.default_imp_dig_own_damage;
    else
        dig_damage = game.conf.rules[diggertng->owner].workers.default_imp_dig_damage;
    return dig_damage;
}

GoldAmount calculate_gold_digged_out_of_slab_with_single_hit(long damage_did_to_slab, const struct SlabMap *slb)
{
    struct SlabConfigStats *slabst = get_slab_stats(slb);
    GoldAmount gold_per_block = slabst->gold_held;
    GoldAmount gold = (damage_did_to_slab * gold_per_block) / game.block_health[slabst->block_health_index];
    if (slb->health == 0)
    // If the last hit deals the damage exactly, just drop a pile and the remainder.
    {
        gold += (gold_per_block % gold);
    }
    else if (slb->health < 0)
    // If the damage dealt is more than the remaining health, then health is not divisible by damage,
    // so this should return whatever is left, as this is less than the gold given for a full hit.
    {
        gold = gold_per_block - (game.block_health[slabst->block_health_index] / damage_did_to_slab) * gold;
    // Subtract all of the "full hits" and return what's left.
    }
    if (gold < 1)
    {
        return 1;
    }
    return gold;
}

long compute_creature_weight(const struct Thing* creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    if (!creature_control_invalid(cctrl))
    {
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
        long eye_height = get_creature_eye_height(creatng);
        long weight = eye_height >> 2;
        weight += (crconf->hunger_fill + crconf->lair_size + 1) * cctrl->exp_level;
        if (creature_is_immune_to_spell_effect(creatng, CSAfF_Wind))
        {
            weight = weight * 3 / 2;
        }
        if ((get_creature_model_flags(creatng) & CMF_Trembling) != 0)
        {
            weight = weight * 3 / 2;
        }
        if ((get_creature_model_flags(creatng) & CMF_IsDiptera) != 0)
        {
            weight = weight / 2;
        }
        if (crconf->can_go_locked_doors == true)
        {
            weight = weight / 10;
        }
        return weight;
    }
    return 0;
}

const char *creature_statistic_text(const struct Thing *creatng, CreatureLiveStatId clstat_id)
{
    const char *text;
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
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
        i = cctrl->exp_level + 1;
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Health:
        i = creatng->health;
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_MaxHealth:
        i = calculate_correct_creature_max_health(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Strength:
        i = calculate_correct_creature_strength(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Armour:
        i = calculate_correct_creature_armour(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Defence:
        i = calculate_correct_creature_defense(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Dexterity:
        i = calculate_correct_creature_dexterity(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Luck:
        i = compute_creature_max_luck(crconf->luck, cctrl->exp_level);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Speed:
        i = calculate_correct_creature_maxspeed(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Loyalty:
        i = calculate_correct_creature_loyalty(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_AgeTime:
        i = (game.play_gameturn-creatng->creation_turn) / 1200; // + cctrl->joining_age;
        if (i >= 999)
          i = 999;
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Kills:
        i = cctrl->kills_num;
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_GoldHeld:
        i = creatng->creature.gold_carried;
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_GoldWage:
        i = calculate_correct_creature_pay(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Score:
        i = compute_creature_kind_score(creatng->model, cctrl->exp_level);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_ResearchSkill:
        i = compute_creature_work_value_for_room_role(creatng, RoRoF_Research, ROOM_EFFICIENCY_MAX);
        snprintf(loc_text, sizeof(loc_text), "%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_ManufactureSkill:
        i = compute_creature_work_value_for_room_role(creatng, RoRoF_CratesManufctr, ROOM_EFFICIENCY_MAX);
        snprintf(loc_text, sizeof(loc_text), "%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_TrainingSkill:
        i = compute_creature_work_value_for_room_role(creatng, RoRoF_CrTrainExp, ROOM_EFFICIENCY_MAX);
        snprintf(loc_text, sizeof(loc_text), "%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_ScavengeSkill:
        i = compute_creature_work_value_for_room_role(creatng, RoRoF_CrScavenge, ROOM_EFFICIENCY_MAX);
        snprintf(loc_text, sizeof(loc_text), "%ld", i/256);
        text = loc_text;
        break;
    case CrLStat_TrainingCost:
        i = calculate_correct_creature_training_cost(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_ScavengeCost:
        i = calculate_correct_creature_scavenging_cost(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_Weight:
        i = compute_creature_weight(creatng);
        snprintf(loc_text, sizeof(loc_text), "%ld", i);
        text = loc_text;
        break;
    case CrLStat_BestDamage:
        // TODO: (???) compute damage of best attack.
        text = "";
        break;
    default:
        ERRORLOG("Invalid statistic %d", (int)clstat_id);
        text = "";
        break;
    }
    return text;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
