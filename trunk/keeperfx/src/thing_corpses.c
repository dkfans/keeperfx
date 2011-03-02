/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_corpses.c
 *     Dead creature things support functions.
 * @par Purpose:
 *     Functions to create and operate on dead creature corpses.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 02 Mar 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "thing_corpses.h"

#include "globals.h"
#include "bflib_basics.h"

#include "thing_data.h"
#include "thing_stats.h"
#include "thing_list.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_graphics.h"
#include "dungeon_data.h"
#include "config_creature.h"
#include "gui_topmsg.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_update_dead_creature(struct Thing *thing);
DLLIMPORT struct Thing *_DK_create_dead_creature(struct Coord3d *pos, unsigned short model, unsigned short a1, unsigned short owner, long explevel);
DLLIMPORT struct Thing *_DK_destroy_creature_and_create_corpse(struct Thing *thing, long a1);
/******************************************************************************/


/******************************************************************************/
long update_dead_creature(struct Thing *thing)
{
    struct Coord3d pos;
    SYNCDBG(18,"Starting");
    return _DK_update_dead_creature(thing);
}

TbBool update_dead_creatures_list(struct Dungeon *dungeon, struct Thing *thing)
{
  struct CreatureStorage *cstore;
  struct CreatureControl *cctrl;
  long i;
  SYNCDBG(18,"Starting");
  cctrl = creature_control_get_from_thing(thing);
  if ((dungeon == NULL) || creature_control_invalid(cctrl))
  {
    WARNLOG("Invalid victim or dungeon");
    return false;
  }
  // Check if the creature of same type is in list
  i = dungeon->dead_creatures_count-1;
  while (i >= 0)
  {
    cstore = &dungeon->dead_creatures[i];
    if ((cstore->model == thing->model) && (cstore->explevel == cctrl->explevel))
    {
      // This creature is already in list
      SYNCDBG(18,"Already in list");
      return false;
    }
    i--;
  }
  // Find a slot for the new creature
  if (dungeon->dead_creatures_count < DEAD_CREATURES_MAX_COUNT)
  {
    i = dungeon->dead_creatures_count;
    dungeon->dead_creatures_count++;
  } else
  {
    i = dungeon->dead_creature_idx;
    dungeon->dead_creature_idx++;
    if (dungeon->dead_creature_idx >= DEAD_CREATURES_MAX_COUNT)
      dungeon->dead_creature_idx = 0;
  }
  cstore = &dungeon->dead_creatures[i];
  cstore->model = thing->model;
  cstore->explevel = cctrl->explevel;
  SYNCDBG(19,"Finished");
  return true;
}

struct Thing *create_dead_creature(struct Coord3d *pos, unsigned short model, unsigned short a1, unsigned short owner, long explevel)
{
    struct Thing *thing;
    unsigned long k;
    //return _DK_create_dead_creature(pos, model, a1, owner, explevel);
    if (!i_can_allocate_free_thing_structure(TAF_FreeEffectIfNoSlots))
    {
        ERRORDBG(3,"Cannot create dead creature model %d for player %d. There are too many things allocated.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing = allocate_free_thing_structure(TAF_FreeEffectIfNoSlots);
    if (thing->index == 0) {
        ERRORDBG(3,"Should be able to allocate dead creature %d for player %d, but failed.",(int)model,(int)owner);
        erstat_inc(ESE_NoFreeThings);
        return INVALID_THING;
    }
    thing->class_id = 4;
    thing->model = model;
    thing->field_1D = thing->index;
    thing->owner = owner;
    thing->byte_13 = explevel;
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = 0;
    thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);
    thing->field_56 = 0;
    thing->field_58 = 0;
    thing->field_5A = 0;
    thing->field_5C = 0;
    thing->field_20 = 16;
    thing->field_23 = 204;
    thing->field_24 = 51;
    thing->field_22 = 0;
    thing->field_25 |= 0x08;
    thing->field_9 = game.play_gameturn;
    if (creatures[model].field_7)
      thing->field_4F |= 0x30;
    add_thing_to_its_class_list(thing);
    place_thing_in_mapwho(thing);
    switch (a1)
    {
    case 2:
        thing->active_state = CrSt_ImpArrivesAtDigOrMine1;
        k = get_creature_anim(thing, 17);
        set_thing_draw(thing, k, 256, 300, 0, 0, 2);
        break;
    default:
        thing->active_state = CrSt_ImpDoingNothing;
        k = get_creature_anim(thing, 15);
        set_thing_draw(thing, k, 128, 300, 0, 0, 2);
        thing->health = 3 * get_lifespan_of_animation(thing->field_44, thing->field_3E);
        play_creature_sound(thing, 9, 3, 0);
        break;
    }
    thing->field_46 = (300 * (long)thing->byte_13) / 20 + 300;
    return thing;
}

struct Thing *destroy_creature_and_create_corpse(struct Thing *thing, long a1)
{
    struct CreatureControl *cctrl;
    struct PlayerInfo *player;
    struct Thing *deadtng;
    struct Coord3d pos;
    TbBool memf1;
    long owner;
    long crmodel;
    long explevel;
    long prev_idx;

    //return _DK_destroy_creature_and_create_corpse(thing, a1);
    crmodel = thing->model;
    memf1 = ((thing->field_0 & 0x20) != 0);
    pos.x.val = thing->mappos.x.val;
    pos.y.val = thing->mappos.y.val;
    pos.z.val = thing->mappos.z.val;
    owner = thing->owner;
    prev_idx = thing->index;
    cctrl = creature_control_get_from_thing(thing);
    explevel = cctrl->explevel;
    player = NULL;
    remove_creature_score_from_owner(thing);
    delete_thing_structure(thing, 0);
    deadtng = create_dead_creature(&pos, crmodel, a1, owner, explevel);
    if (thing_is_invalid(deadtng))
    {
        ERRORLOG("Could not create dead thing.");
        return INVALID_THING;
    }
    set_flag_byte(&deadtng->field_0, 0x20, memf1);
    if (owner != game.neutral_player_num)
    {
        // Update thing index inside player struct
        player = get_player(owner);
        if (player->controlled_thing_idx == prev_idx)
        {
            player->controlled_thing_idx = deadtng->index;
            player->field_31 = deadtng->field_9;
        }
    }
    return deadtng;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
