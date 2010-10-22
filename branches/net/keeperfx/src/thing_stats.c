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
#include "game_merge.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "player_data.h"
#include "config_magic.hpp"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_apply_damage_to_thing(struct Thing *thing, long a2, char a3);

/******************************************************************************/
/*
 * Computes max health of a creature on given level.
 */
long compute_creature_max_health(long base_health,unsigned short crlevel)
{
  long max_health;
  if (base_health <= 0)
    return 0;
  if (base_health > 100000)
    base_health = 100000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  max_health = base_health + (CREATURE_HEALTH_INCREASE_ON_EXP*base_health*(long)crlevel)/100;
  return saturate_set_signed(max_health, 16);
}

/**
 * Computes gold pay of a creature on given level.
 */
long compute_creature_max_pay(long base_param,unsigned short crlevel)
{
  long max_param;
  if (base_param <= 0)
    return 0;
  if (base_param > 100000)
    base_param = 100000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  max_param = base_param + (CREATURE_PAY_INCREASE_ON_EXP*base_param*(long)crlevel)/100;
  return saturate_set_signed(max_param, 16);
}

/**
 * Computes 16-bit parameter of a creature on given level.
 */
long compute_creature_max_sparameter(long base_param,unsigned short crlevel)
{
  long max_param;
  if (base_param <= 0)
    return 0;
  if (base_param > 100000)
    base_param = 100000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  max_param = base_param + (CREATURE_PROPERTY_INCREASE_ON_EXP*base_param*(long)crlevel)/100;
  return saturate_set_signed(max_param, 16);
}

/**
 * Computes defence of a creature on given level.
 */
long compute_creature_max_defence(long base_param,unsigned short crlevel)
{
  long max_param;
  if (base_param <= 0)
    return 0;
  if (base_param > 10000)
    base_param = 10000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  max_param = base_param + (CREATURE_DEFENSE_INCREASE_ON_EXP*base_param*(long)crlevel)/100;
  return saturate_set_unsigned(max_param, 8);
}

/**
 * Computes dexterity of a creature on given level.
 */
long compute_creature_max_dexterity(long base_param,unsigned short crlevel)
{
  long max_param;
  if (base_param <= 0)
    return 0;
  if (base_param > 10000)
    base_param = 10000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  max_param = base_param + (CREATURE_DEXTERITY_INCREASE_ON_EXP*base_param*(long)crlevel)/100;
  return saturate_set_unsigned(max_param, 8);
}

/**
 * Computes strength of a creature on given level.
 */
long compute_creature_max_strength(long base_param,unsigned short crlevel)
{
  long max_param;
  if ((base_param <= 0) || (base_param > 60000))
    base_param = 60000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  max_param = base_param + (CREATURE_STRENGTH_INCREASE_ON_EXP*base_param*(long)crlevel)/100;
  return saturate_set_unsigned(max_param, 15);
}

/**
 * Computes damage of an attack, taking luck and creature level into account.
 */
long compute_creature_attack_damage(long base_param,long luck,unsigned short crlevel)
{
  long max_param;
  if (base_param < -60000)
    base_param = -60000;
  if (base_param > 60000)
    base_param = 60000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  max_param = base_param + (CREATURE_DAMAGE_INCREASE_ON_EXP*base_param*(long)crlevel)/100;
  if (luck > 0)
  {
    if (ACTION_RANDOM(101) < luck)
      max_param *= 2;
  }
  return saturate_set_signed(max_param, 16);
}

/**
 * Computes spell range/area of effect for a creature on given level.
 */
long compute_creature_attack_range(long base_param,long luck,unsigned short crlevel)
{
  long max_param;
  if (base_param <= 0)
    return 0;
  if (base_param > 100000)
    base_param = 100000;
  if (crlevel >= CREATURE_MAX_LEVEL)
    crlevel = CREATURE_MAX_LEVEL-1;
  max_param = base_param + (CREATURE_RANGE_INCREASE_ON_EXP*base_param*(long)crlevel)/100;
  return saturate_set_signed(max_param, 16);
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

TbBool update_creature_health_to_max(struct Thing *thing)
{
  struct CreatureStats *crstat;
  struct CreatureControl *cctrl;
  crstat = creature_stats_get_from_thing(thing);
  cctrl = creature_control_get_from_thing(thing);
  thing->health = compute_creature_max_health(crstat->health,cctrl->explevel);
  return true;
}

void apply_damage_to_thing(struct Thing *thing, long dmg, char a3)
{
    struct PlayerInfo *player;
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    long carmor, cdamage;
    long i;
    //_DK_apply_damage_to_thing(thing, dmg, a3);
    // We're here to damage, not to heal
    if (dmg <= 0)
        return;
    // If it's already dead, then don't interfere
    if (thing->health < 0)
        return;

    switch (thing->class_id)
    {
    case TCls_Creature:
        cctrl = creature_control_get_from_thing(thing);
        crstat = creature_stats_get_from_thing(thing);
        if ((cctrl->flgfield_1 & 0x04) == 0)
        {
            // Compute armor value
            carmor = compute_creature_max_armour(crstat->armour,cctrl->explevel);
            if ((cctrl->spell_flags & CSF_Armour) != 0)
                carmor = (320 * carmor) / 256;
            // This limit makes armor absorb up to 80% of damage, never more
            if (carmor > 204)
                carmor = 204;
            if (carmor < 0)
                carmor = 0;
            // Now compute damage
            cdamage = (dmg * (256 - carmor)) / 256;
            if (cdamage <= 0)
              cdamage = 1;
            // Apply damage to the thing
            thing->health -= cdamage;
            thing->word_17 = 8;
            thing->field_4F |= 0x80;
            // Red palette if the possessed creature is hit very strong
            if (thing->owner != game.neutral_player_num)
            {
                player = get_player(thing->owner);
                if (thing_get(player->field_2F) == thing)
                {
                  i = (10 * cdamage) / compute_creature_max_health(crstat->health,cctrl->explevel);
                  if (i > 10)
                  {
                      i = 10;
                  } else
                  if (i <= 0)
                  {
                      i = 1;
                  }
                  PaletteApplyPainToPlayer(player, i);
                }
            }
        }
        break;
    case TCls_Object:
        cdamage = dmg;
        thing->health -= cdamage;
        thing->field_4F |= 0x80;
        break;
    case TCls_Door:
        cdamage = dmg;
        thing->health -= cdamage;
        break;
    default:
        break;
    }
    if ((thing->class_id == TCls_Creature) && (thing->health < 0))
    {
        cctrl = creature_control_get_from_thing(thing);
        if ((cctrl->field_1D2 == -1) && (a3 != -1))
        {
            cctrl->field_1D2 = a3;
        }
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
