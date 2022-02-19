/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_library.c
 *     Library room maintain functions.
 * @par Purpose:
 *     Functions to create and use libraries.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 05 Jun 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_library.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_stats.h"
#include "thing_navigate.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "creature_states_rsrch.h"
#include "magic.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Thing *create_spell_in_library(struct Room *room, ThingModel tngmodel, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if (room->kind != RoK_LIBRARY) {
        SYNCDBG(4,"Cannot add spell to %s owned by player %d",room_code_name(room->kind),(int)room->owner);
        return INVALID_THING;
    }
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(stl_x);
    pos.y.val = subtile_coord_center(stl_y);
    pos.z.val = 0;
    struct Thing* spelltng = create_object(&pos, tngmodel, room->owner, -1);
    if (thing_is_invalid(spelltng))
    {
        return INVALID_THING;
    }
    // Neutral thing do not need any more processing
    if (is_neutral_thing(spelltng))
    {
        return spelltng;
    }
    if (!add_item_to_room_capacity(room, true))
    {
        destroy_object(spelltng);
        return INVALID_THING;
    }
    if (!add_power_to_player(book_thing_to_power_kind(spelltng), room->owner))
    {
        remove_item_from_room_capacity(room);
        destroy_object(spelltng);
        return INVALID_THING;
    }
    return spelltng;
}

TbBool remove_spell_from_library(struct Room *room, struct Thing *spelltng, PlayerNumber new_owner)
{
    if ( (room->kind != RoK_LIBRARY) || (spelltng->owner != room->owner) ) {
        SYNCDBG(4,"Spell %s owned by player %d found in a %s owned by player %d, instead of proper library",thing_model_name(spelltng),(int)spelltng->owner,room_code_name(room->kind),(int)room->owner);
        return false;
    }
    if (!remove_item_from_room_capacity(room))
        return false;
    if (thing_is_spellbook(spelltng))
    {
        remove_power_from_player(book_thing_to_power_kind(spelltng), room->owner);
    }
    return true;
}

EventIndex update_library_object_pickup_event(struct Thing *creatng, struct Thing *picktng)
{
    EventIndex evidx;
    struct PlayerInfo* player;
    if (thing_is_spellbook(picktng))
    {
        evidx = event_create_event_or_update_nearby_existing_event(
            picktng->mappos.x.val, picktng->mappos.y.val,
            EvKind_SpellPickedUp, creatng->owner, picktng->index);
        // Only play speech message if new event was created
        if (evidx > 0)
        {
            if ( (is_my_player_number(picktng->owner)) && (!is_my_player_number(creatng->owner)) )
            {
                output_message(SMsg_SpellbookStolen, 0, true);
            } 
            else if ( (is_my_player_number(creatng->owner)) && (!is_my_player_number(picktng->owner)) )
            {
                player = get_my_player();
                if (picktng->owner == game.neutral_player_num)
                {
                   if (creatng->index != player->influenced_thing_idx)
                   {                       
                        output_message(SMsg_DiscoveredSpell, 0, true);
                   }                   
                }
                else
                {
                   if (creatng->index != player->influenced_thing_idx)
                   {       
                        output_message(SMsg_SpellbookTaken, 0, true);
                   }
                }
            }
        }
    } else
    if (thing_is_special_box(picktng))
    {
        evidx = event_create_event_or_update_nearby_existing_event(
            picktng->mappos.x.val, picktng->mappos.y.val,
            EvKind_DnSpecialFound, creatng->owner, picktng->index);
        // Only play speech message if new event was created
        if (evidx > 0)
        {
          if (is_my_player_number(creatng->owner) && !is_my_player_number(picktng->owner))
          {
              player = get_my_player();
              if (creatng->index != player->influenced_thing_idx)
              {    
                output_message(SMsg_DiscoveredSpecial, 0, true);
              }
          }
        }
    } else
    {
        WARNLOG("Strange pickup (model %d) - no event",(int)picktng->model);
        evidx = 0;
    }
    return evidx;
}

void init_dungeons_research(void)
{
    for (int i = 0; i < DUNGEONS_COUNT; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        dungeon->current_research_idx = get_next_research_item(dungeon);
    }
}

TbBool remove_all_research_from_player(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    dungeon->research_num = 0;
    dungeon->research_override = 1;
    return true;
}

TbBool research_overriden_for_player(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    return (dungeon->research_override != 0);
}

TbBool clear_research_for_all_players(void)
{
    for (int plyr_idx = 0; plyr_idx < DUNGEONS_COUNT; plyr_idx++)
    {
        struct Dungeon* dungeon = get_dungeon(plyr_idx);
        dungeon->research_num = 0;
        dungeon->research_override = 0;
    }
    return true;
}

TbBool research_needed(const struct ResearchVal *rsrchval, const struct Dungeon *dungeon)
{
    if (dungeon->research_num == 0)
        return false;
    switch (rsrchval->rtyp)
    {
   case RsCat_Power:
        if ( (dungeon->magic_resrchable[rsrchval->rkind]) && (dungeon->magic_level[rsrchval->rkind] == 0) )
        {
            return true;
        }
        break;
    case RsCat_Room:
        if ((dungeon->room_buildable[rsrchval->rkind] & 1) == 0)
        {
            // Is available for research
            if (dungeon->room_resrchable[rsrchval->rkind] == 1)
                return true;
            // Is available for research and reseach instantly completes when the room is first captured
            else if (dungeon->room_resrchable[rsrchval->rkind] == 2)
                return true;
            // Is not available for research until the room is first captured
            else if ( (dungeon->room_resrchable[rsrchval->rkind] == 4) && (dungeon->room_buildable[rsrchval->rkind] & 2))
                return true;
        }
        break;
    case RsCat_Creature:
        if ((dungeon->creature_allowed[rsrchval->rkind]) && (dungeon->creature_force_enabled[rsrchval->rkind] == 0))
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

TbBool add_research_to_player(PlayerNumber plyr_idx, long rtyp, long rkind, long amount)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long i = dungeon->research_num;
    if (i >= DUNGEON_RESEARCH_COUNT)
    {
      ERRORLOG("Too much research (%d items) for player %d", i, plyr_idx);
      return false;
    }
    struct ResearchVal* resrch = &dungeon->research[i];
    resrch->rtyp = rtyp;
    resrch->rkind = rkind;
    resrch->req_amount = amount;
    dungeon->research_num++;
    return true;
}

TbBool add_research_to_all_players(long rtyp, long rkind, long amount)
{
    TbBool result = true;
    SYNCDBG(17, "Adding type %d, kind %d, amount %d", rtyp, rkind, amount);
    for (long i = 0; i < PLAYERS_COUNT; i++)
    {
        result &= add_research_to_player(i, rtyp, rkind, amount);
  }
  return result;
}

TbBool update_players_research_amount(PlayerNumber plyr_idx, long rtyp, long rkind, long amount)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    for (long i = 0; i < dungeon->research_num; i++)
    {
        struct ResearchVal* resrch = &dungeon->research[i];
        if ((resrch->rtyp == rtyp) && (resrch->rkind = rkind))
        {
            resrch->req_amount = amount;
        }
        return true;
  }
  return false;
}

TbBool update_or_add_players_research_amount(PlayerNumber plyr_idx, long rtyp, long rkind, long amount)
{
  if (update_players_research_amount(plyr_idx, rtyp, rkind, amount))
    return true;
  return add_research_to_player(plyr_idx, rtyp, rkind, amount);
}

void process_player_research(PlayerNumber plyr_idx)
{
    //_DK_process_player_research(plyr_idx); return;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (!player_has_room_of_role(plyr_idx, RoRoF_Research)) {
        return;
    }
    struct ResearchVal* rsrchval = get_players_current_research_val(plyr_idx);
    if (rsrchval == NULL)
    {
        // If no current research - try to set one for next time the function is run
        dungeon->current_research_idx = get_next_research_item(dungeon);
        return;
    }
    if (!research_needed(rsrchval, dungeon))
    {
        dungeon->current_research_idx = get_next_research_item(dungeon);
        rsrchval = get_players_current_research_val(plyr_idx);
    }
    if (rsrchval == NULL) {
        // No new research
        return;
    }
    if ((rsrchval->req_amount << 8) > dungeon->research_progress) {
        // Research in progress - not completed
        return;
    }
    struct Room *room;
    struct Coord3d pos;
    switch (rsrchval->rtyp)
    {
    case RsCat_Power:
    {
        if (dungeon->magic_resrchable[rsrchval->rkind])
        {
            PowerKind pwkind = rsrchval->rkind;
            room = find_room_with_spare_room_item_capacity(plyr_idx, RoK_LIBRARY);
            struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
            if (powerst->artifact_model < 1) {
                ERRORLOG("Tried to research power with no associated artifact");
                break;
            }
            if (room_is_invalid(room)) {
                WARNLOG("Player %d has no %s with capacity for %s artifact, delaying creation",(int)plyr_idx,room_code_name(RoK_LIBRARY),power_code_name(pwkind));
                return;
            }
            pos.x.val = 0;
            pos.y.val = 0;
            pos.z.val = 0;
            struct Thing* spelltng = create_object(&pos, powerst->artifact_model, plyr_idx, -1);
            if (thing_is_invalid(spelltng))
            {
                ERRORLOG("Could not create %s artifact",power_code_name(pwkind));
                return;
            }
            room = find_random_room_for_thing_with_spare_room_item_capacity(spelltng, plyr_idx, RoK_LIBRARY, 0);
            if (room_is_invalid(room))
            {
                ERRORLOG("There should be %s for %s artifact, but not found",room_code_name(RoK_LIBRARY),power_code_name(pwkind));
                delete_thing_structure(spelltng, 0);
                return;
            }
            if (!find_random_valid_position_for_thing_in_room_avoiding_object(spelltng, room, &pos))
            {
                ERRORLOG("Could not find position in %s for %s artifact",room_code_name(room->kind),power_code_name(pwkind));
                delete_thing_structure(spelltng, 0);
                return;
            }
            pos.z.val = get_thing_height_at(spelltng, &pos);
            if (add_power_to_player(pwkind, plyr_idx))
            {
                move_thing_in_map(spelltng, &pos);
                add_item_to_room_capacity(room, true);
                event_create_event(spelltng->mappos.x.val, spelltng->mappos.y.val, EvKind_NewSpellResrch, spelltng->owner, pwkind);
                create_effect(&pos, TngEff_ResearchComplete, spelltng->owner);
            }
            else
            {
                dungeon->magic_level[pwkind]++;
            }
            if (is_my_player_number(plyr_idx))
            {
                output_message(SMsg_ResearchedSpell, 0, true);
            }
        }
        break;
    }
    case RsCat_Room:
        if (dungeon->room_resrchable[rsrchval->rkind])
        {
            RoomKind rkind;
            rkind = rsrchval->rkind;
            event_create_event(0, 0, EvKind_NewRoomResrch, plyr_idx, rkind);
            dungeon->room_buildable[rkind] |= 3; // Player may build room and may research it again
            if (is_my_player_number(plyr_idx))
                output_message(SMsg_ResearchedRoom, 0, true);
            room = find_room_with_spare_room_item_capacity(plyr_idx, RoK_LIBRARY);
            if (!room_is_invalid(room))
            {
                pos.x.val = subtile_coord_center(room->central_stl_x);
                pos.y.val = subtile_coord_center(room->central_stl_y);
                pos.z.val = get_floor_height_at(&pos);
                create_effect(&pos, TngEff_ResearchComplete, room->owner);
            }
        }
        break;
    case RsCat_Creature:
        if (dungeon->creature_allowed[rsrchval->rkind])
        {
            ThingModel crkind;
            crkind = rsrchval->rkind;
            dungeon->creature_force_enabled[crkind]++;
        }
        break;
    default:
        ERRORLOG("Illegal research type %d while processing player %d research",(int)rsrchval->rtyp,(int)plyr_idx);
        break;
    }
    dungeon->research_progress -= (rsrchval->req_amount << 8);
    dungeon->last_research_complete_gameturn = game.play_gameturn;

    dungeon->current_research_idx = get_next_research_item(dungeon);
    dungeon->lvstats.things_researched++;
    return;
}

void research_found_room(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
     // Player got room to build instantly
    if ((dungeon->room_resrchable[rkind] == 2)
        || (dungeon->room_resrchable[rkind] == 3)
        )
    {
        dungeon->room_buildable[rkind] = 3;
    }
    else
    {
        // Player may research room then it is claimed
        dungeon->room_buildable[rkind] |= 2; 
    }
}
/******************************************************************************/
