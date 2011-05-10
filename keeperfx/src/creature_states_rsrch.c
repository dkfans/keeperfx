/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_rsrch.c
 *     Creature state machine functions for their job in various rooms.
 * @par Purpose:
 *     Defines elements of states[] array, containing valid creature states.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "creature_states_rsrch.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_rules.h"
#include "config_terrain.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "room_data.h"
#include "room_jobs.h"
#include "gui_soundmsgs.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT short _DK_at_research_room(struct Thing *thing);
DLLIMPORT long _DK_process_research_function(struct Thing *thing);
DLLIMPORT short _DK_researching(struct Thing *thing);
DLLIMPORT void _DK_force_complete_current_research(long plyr_idx);
DLLIMPORT long _DK_get_next_research_item(struct Dungeon *dungeon);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
short at_research_room(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    //return _DK_at_research_room(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_80 = 0;
    if (thing->owner == game.neutral_player_num)
    {
        set_start_state(thing);
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    dungeon = get_dungeon(thing->owner);
    if ((crstat->research_value <= 0) || (dungeon->field_F78 < 0))
    {
        if ( (thing->owner != game.neutral_player_num) && (dungeon->field_F78 < 0) )
        {
            if ( is_my_player_number(dungeon->owner) )
                output_message(SMsg_NoMoreReseach, 500, true);
        }
        set_start_state(thing);
        return 0;
    }
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room))
    {
        set_start_state(thing);
        return 0;
    }
    if ((room->kind != RoK_LIBRARY) || (room->owner != thing->owner))
    {
        WARNLOG("Room %s owned by player %d is invalid for %s",room_code_name(room->kind),(int)room->owner,thing_model_name(thing));
        set_start_state(thing);
        return 0;
    }
    if (!add_creature_to_work_room(thing, room))
    {
        set_start_state(thing);
        return 0;
    }
    if ( !setup_random_head_for_room(thing, room, 0) )
    {
        ERRORLOG("The %s can not move in research room", thing_model_name(thing));
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    thing->continue_state = CrSt_Researching;
    cctrl->field_82 = 0;
    cctrl->byte_9A = 3;
    return 1;
}

TbBool research_needed(struct ResearchVal *rsrchval, struct Dungeon *dungeon)
{
    if (dungeon->research_num == 0)
        return -1;
    switch (rsrchval->rtyp)
    {
   case RsCat_Power:
        if ( (dungeon->magic_resrchable[rsrchval->rkind]) && (dungeon->magic_level[rsrchval->rkind] == 0) )
        {
            return true;
        }
        break;
    case RsCat_Room:
        if ( (dungeon->room_resrchable[rsrchval->rkind]) && (dungeon->room_buildable[rsrchval->rkind] == 0) )
        {
            return true;
        }
        break;
    case RsCat_Creature:
        if ( (dungeon->creature_allowed[rsrchval->rkind]) && (dungeon->creature_enabled[rsrchval->rkind] == 0) )
        {
            return true;
        }
        break;
    case RsCat_None:
        break;
    default:
        ERRORLOG("Illegal research type %d while processing player research",(int)rsrchval->rtyp);
        break;
    }
    return false;
}

long get_next_research_item(struct Dungeon *dungeon)
{
    struct ResearchVal *rsrchval;
    long resnum;
    //return _DK_get_next_research_item(dungeon);
    if (dungeon->research_num == 0)
        return -1;
    for (resnum = 0; resnum < dungeon->research_num; resnum++)
    {
        rsrchval = &dungeon->research[resnum];
        switch (rsrchval->rtyp)
        {
       case RsCat_Power:
            if ( (dungeon->magic_resrchable[rsrchval->rkind]) && (dungeon->magic_level[rsrchval->rkind] == 0) )
            {
                return resnum;
            }
            break;
        case RsCat_Room:
            if ( (dungeon->room_resrchable[rsrchval->rkind]) && (dungeon->room_buildable[rsrchval->rkind] == 0) )
            {
                return resnum;
            }
            break;
        case RsCat_Creature:
            WARNLOG("Creature research skipped - not implemented");
            break;
        case RsCat_None:
            break;
        default:
            ERRORLOG("Illegal research type %d while getting next research item",(int)rsrchval->rtyp);
            break;
        }
    }
    return -1;
}

struct ResearchVal *get_players_current_research_val(long plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if ((dungeon->field_F78 < 0) || (dungeon->field_F78 >= DUNGEON_RESEARCH_COUNT))
        return NULL;
    return &dungeon->research[dungeon->field_F78];
}

TbBool force_complete_current_research(long plyr_idx)
{
    struct Dungeon *dungeon;
    struct ResearchVal *rsrchval;
    long resnum;
    //_DK_force_complete_current_research(plyr_idx);
    dungeon = get_dungeon(plyr_idx);
    rsrchval = get_players_current_research_val(plyr_idx);
    if (rsrchval != NULL)
    {
        if ( research_needed(rsrchval, dungeon) ) {
            dungeon->field_1193 = rsrchval->req_amount << 8;
            return true;
        }
    }
    resnum = get_next_research_item(dungeon);
    dungeon->field_F78 = resnum;
    rsrchval = get_players_current_research_val(plyr_idx);
    if (rsrchval != NULL)
    {
        dungeon->field_1193 = rsrchval->req_amount << 8;
        return true;
    }
    return false;
}

long process_research_function(struct Thing *thing)
{
  return _DK_process_research_function(thing);
}

short researching(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    struct CreatureStats *crstat;
    struct Dungeon *dungeon;
    struct Room *room;
    long i;
    //return _DK_researching(thing);
    dungeon = get_dungeon(thing->owner);
    cctrl = creature_control_get_from_thing(thing);
    if (thing->owner == game.neutral_player_num)
    {
        ERRORLOG("Neutral %s can't do research",thing_model_name(thing));
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    crstat = creature_stats_get_from_thing(thing);
    if ( (crstat->research_value == 0) || (dungeon->field_F78 < 0) )
    {
        if ( (thing->owner != game.neutral_player_num) && (dungeon->field_F78 < 0) )
        {
            if ( is_my_player_number(dungeon->owner) )
                output_message(SMsg_NoMoreReseach, 500, true);
        }
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    // Get and verify working room
    room = get_room_thing_is_on(thing);
    if (room_is_invalid(room) || (cctrl->work_room_id != room->index))
    {
        WARNLOG("Room %s index %d is is not %s work room %d",room_code_name(room->kind),(int)room->index,thing_model_name(thing),(int)cctrl->work_room_id);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    if ( (room->kind != RoK_LIBRARY) || (room->owner != thing->owner) )
    {
        WARNLOG("Room %s owned by player %d is invalid for %s owned by played %d",room_code_name(room->kind),(int)room->owner,thing_model_name(thing),(int)thing->owner);
        remove_creature_from_work_room(thing);
        set_start_state(thing);
        return 0;
    }
    if (room->used_capacity > room->total_capacity)
    {
      if ( is_my_player_number(room->owner) )
          output_message(SMsg_LibraryTooSmall, 0, true);
      remove_creature_from_work_room(thing);
      set_start_state(thing);
      return 0;
    }
    process_research_function(thing);
    cctrl = creature_control_get_from_thing(thing);
    if ( (game.play_gameturn - dungeon->field_AE5 < 50)
      && ((game.play_gameturn + thing->index) & 0x03) == 0)
    {
      external_set_thing_state(thing, 127);
      cctrl->field_282 = 50;
      cctrl->long_9A = 0;
      return 1;
    }
    if (cctrl->instance_id != CrInst_NULL)
      return 1;
    cctrl->field_82++;
    // Shall we do some "Standing and thinking"
    if (cctrl->field_82 <= 128)
    {
      if (cctrl->byte_9A == 3)
      {
          // Do some random thinking
          if ((cctrl->field_82 % 16) == 0)
          {
              i = ACTION_RANDOM(1024) - 512;
              cctrl->long_9B = ((long)thing->field_52 + i) & 0x7FF;
              cctrl->byte_9A = 4;
          }
      } else
      {
          // Look at different direction while thinking
          if ( creature_turn_to_face_angle(thing, cctrl->long_9B) < 56 )
          {
              cctrl->byte_9A = 3;
          }
      }
      return 1;
    }
    // Finished "Standing and thinking" - make "new idea" effect and go to next position
    if ( !setup_random_head_for_room(thing, room, 0) )
    {
        ERRORLOG("Cannot move %s in %s room", thing_model_name(thing),room_code_name(room->kind));
        set_start_state(thing);
        return 1;
    }
    thing->continue_state = CrSt_Researching;
    cctrl->field_82 = 0;
    cctrl->byte_9A = 3;
    if (cctrl->explevel < 3)
    {
        create_effect(&thing->mappos, 54, thing->owner);
    } else
    if (cctrl->explevel < 6)
    {
        create_effect(&thing->mappos, 55, thing->owner);
    } else
    {
        create_effect(&thing->mappos, 56, thing->owner);
    }
    return 1;
}

/******************************************************************************/
