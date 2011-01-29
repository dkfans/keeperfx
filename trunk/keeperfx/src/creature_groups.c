/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_groups.c
 *     Creature grouping and groups support functions.
 * @par Purpose:
 *     Functions to creature_groups.
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
#include "creature_groups.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_math.h"
#include "thing_list.h"
#include "thing_creature.h"
#include "creature_control.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_add_creature_to_group(struct Thing *crthing, struct Thing *grthing);
DLLIMPORT long _DK_get_highest_experience_level_in_group(struct Thing *thing);

/******************************************************************************/
long get_highest_experience_level_in_group(struct Thing *thing)
{
  return _DK_get_highest_experience_level_in_group(thing);
}

long add_creature_to_group(struct Thing *crthing, struct Thing *grthing)
{
  return _DK_add_creature_to_group(crthing, grthing);
}

struct Party *get_party_of_name(const char *prtname)
{
  struct Party *party;
  int i;
  for (i=0; i < game.script.creature_partys_num; i++)
  {
    party = &game.script.creature_partys[i];
    if (stricmp(party->prtname, prtname) == 0)
      return party;
  }
  return NULL;
}

int get_party_index_of_name(const char *prtname)
{
  struct Party *party;
  int i;
  for (i=0; i < game.script.creature_partys_num; i++)
  {
    party = &game.script.creature_partys[i];
    if (stricmp(party->prtname, prtname) == 0)
      return i;
  }
  return -1;
}

TbBool create_party(char *prtname)
{
    struct Party *party;
    if (game.script.creature_partys_num >= CREATURE_PARTYS_COUNT)
    {
        SCRPTERRLOG("Too many partys in script");
        return false;
    }
    party = (&game.script.creature_partys[game.script.creature_partys_num]);
    strncpy(party->prtname, prtname, sizeof(party->prtname));
    party->members_num = 0;
    game.script.creature_partys_num++;
    return true;
}

TbBool add_member_to_party_name(const char *prtname, long crtr_model, long crtr_level, long carried_gold, long objctv_id, long countdown)
{
    struct Party *party;
    struct PartyMember *member;
    party = get_party_of_name(prtname);
    if (party == NULL)
    {
      SCRPTERRLOG("Party of requested name, '%s', is not defined", prtname);
      return false;
    }
    if (party->members_num >= PARTY_MEMBERS_COUNT)
    {
      SCRPTERRLOG("Too many creatures in party '%s' (limit is %d members)",
          prtname, PARTY_MEMBERS_COUNT);
      return false;
    }
    member = &(party->members[party->members_num]);
    set_flag_byte(&(member->flags), TrgF_DISABLED, false);
    member->crtr_kind = crtr_model;
    member->carried_gold = carried_gold;
    member->crtr_level = crtr_level-1;
    member->field_6F = 1;
    member->objectv = objctv_id;
    member->countdown = countdown;
    party->members_num++;
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
