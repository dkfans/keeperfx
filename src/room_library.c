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
#include "pre_inc.h"
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
#include "post_inc.h"

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
    if (!room_role_matches(room->kind,RoRoF_PowersStorage)) {
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
    if (thing_is_spellbook(spelltng))
    {
        if (!add_item_to_room_capacity(room, true))
        {
            destroy_object(spelltng);
            return INVALID_THING;
        }
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
    if ( (!room_role_matches(room->kind,RoRoF_PowersStorage)) || (spelltng->owner != room->owner) ) {
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
        WARNLOG("Strange pickup (%s) - no event",thing_class_and_model_name(picktng->class_id, picktng->model));
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
      ERRORLOG("Too much research (%ld items) for player %d", i, plyr_idx);
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
    SYNCDBG(17, "Adding type %ld, kind %ld, amount %ld", rtyp, rkind, amount);
    for (long i = 0; i < PLAYERS_COUNT; i++)
    {
        result &= add_research_to_player(i, rtyp, rkind, amount);
  }
  return result;
}

TbBool update_players_research_amount(PlayerNumber plyr_idx, long rtyp, long rkind, long amount)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    short n = 0;
    for (long i = 0; i < dungeon->research_num; i++)
    {
        struct ResearchVal* resrch = &dungeon->research[i];
        if ((resrch->rtyp == rtyp) && (resrch->rkind == rkind))
        {
            resrch->req_amount = amount;
        }
        n++;
    }
    if (n > 0)
        return true;
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
            room = find_room_of_role_with_spare_room_item_capacity(plyr_idx, RoRoF_PowersStorage);
            struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
            if (powerst->artifact_model < 1) {
                ERRORLOG("Tried to research power with no associated artifact");
                break;
            }
            if (room_is_invalid(room)) {
                WARNLOG("Player %d has no %s with capacity for %s artifact, delaying creation",(int)plyr_idx,room_role_code_name(RoRoF_PowersStorage),power_code_name(pwkind));
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
            room = find_random_room_of_role_for_thing_with_spare_room_item_capacity(spelltng, plyr_idx, RoRoF_PowersStorage, 0);
            if (room_is_invalid(room))
            {
                ERRORLOG("There should be %s for %s artifact, but not found",room_role_code_name(RoRoF_PowersStorage),power_code_name(pwkind));
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
                if (thing_is_spellbook(spelltng))
                {
                    add_item_to_room_capacity(room, true);
                }
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
            room = find_room_of_role_with_spare_room_item_capacity(plyr_idx, RoRoF_PowersStorage);
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
            dungeon->creature_force_enabled[rsrchval->rkind]++;
        }
        else 
        {          
            room = find_room_of_role_with_spare_room_item_capacity(plyr_idx, RoRoF_PowersStorage);
            if (!room_is_invalid(room))
            {
                pos.x.val = subtile_coord_center(room->central_stl_x);
                pos.y.val = subtile_coord_center(room->central_stl_y);
                pos.z.val = get_floor_height_at(&pos);
                create_effect(&pos, TngEff_ResearchComplete, room->owner);
            }
            dungeon->creature_allowed[rsrchval->rkind]++;
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

void reposition_all_books_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    struct Dungeon* dungeon;
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return;
    unsigned long k = 0;
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
        // Per thing code
        if (thing_is_spellbook(thing))
        {
            ThingModel objkind = thing->model;
            PowerKind spl_idx = book_thing_to_power_kind(thing);
            if ((spl_idx > 0) && ((thing->alloc_flags & TAlF_IsDragged) == 0))
            {
                if (game.play_gameturn > 10) //Function is used to place books in rooms before dungeons are intialized
                {
                    dungeon = get_players_num_dungeon(room->owner);
                    if (dungeon->magic_level[spl_idx] < 2)
                    {
                        if (!store_reposition_entry(rrepos, objkind)) {
                            WARNLOG("Too many things to reposition in %s.", room_code_name(room->kind));
                        }
                    }
                    if (!is_neutral_thing(thing))
                    {
                        remove_power_from_player(spl_idx, room->owner);
                        dungeon = get_dungeon(room->owner);
                        dungeon->magic_resrchable[spl_idx] = 1;
                    }
                }
                else
                {
                    if (!store_reposition_entry(rrepos, objkind))
                    {
                        WARNLOG("Too many things to reposition in %s.", room_code_name(room->kind));
                    }
                    if (!is_neutral_thing(thing))
                    {
                        remove_power_from_player(spl_idx, room->owner);
                    }
                }
                delete_thing_structure(thing, 0);
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
}

TbBool recreate_repositioned_book_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    if ((rrepos->used < 0) || (room->used_capacity >= room->total_capacity)) {
        return false;
    }
    for (int ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
    {
        if (rrepos->models[ri] != 0)
        {
            struct Thing* objtng = create_spell_in_library(room, rrepos->models[ri], stl_x, stl_y);
            if (!thing_is_invalid(objtng))
            {
                rrepos->used--;
                rrepos->models[ri] = 0;
                return true;
            }
        }
    }
    return false;
}

int position_books_in_room_with_capacity(PlayerNumber plyr_idx, RoomKind rkind, struct RoomReposition* rrepos)
{
    struct Room* room = find_room_of_role_with_spare_room_item_capacity(plyr_idx, RoRoF_PowersStorage);
    struct Coord3d pos;
    unsigned long k = 0;
    int i = room->index;
    int count = 0;
    while (i != 0)
    {
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        // Per-room code
        pos.x.val = subtile_coord_center(room->central_stl_x);
        pos.y.val = subtile_coord_center(room->central_stl_y);
        pos.z.val = get_floor_height_at(&pos);

        for (int ri = 0; ri < ROOM_REPOSITION_COUNT; ri++)
        {
            if (rrepos->models[ri] != 0)
            {
                struct Thing* spelltng = create_spell_in_library(room, rrepos->models[ri], room->central_stl_x, room->central_stl_y);
                if (!thing_is_invalid(spelltng))
                {
                    if (!find_random_valid_position_for_thing_in_room_avoiding_object(spelltng, room, &pos))
                    {
                        SYNCDBG(7, "Could not find position in %s for %s artifact", room_code_name(room->kind), object_code_name(spelltng->model));
                        if (!is_neutral_thing(spelltng))
                        {
                            remove_power_from_player(book_thing_to_power_kind(spelltng), plyr_idx);
                        }
                        delete_thing_structure(spelltng, 0);
                    }
                    else
                    {
                        pos.z.val = get_thing_height_at(spelltng, &pos);
                        if (!thing_exists(spelltng))
                        {
                            ERRORLOG("Attempt to reposition non-existing book.");
                            return false;
                        }
                        move_thing_in_map(spelltng, &pos);
                        create_effect(&pos, TngEff_RoomSparkeLarge, spelltng->owner);
                        rrepos->used--;
                        rrepos->models[ri] = 0;
                        count++;
                    }
                }
            }
        }
        if (rrepos->used <= 0)
        {
            SYNCDBG(7,"Nothing left to reposition")
            break;
        }
        room = find_room_of_role_with_spare_room_item_capacity(plyr_idx, RoRoF_PowersStorage);
        if (room_is_invalid(room))
        {
            SYNCLOG("Could not find any spare %s capacity for %d remaining books", room_role_code_name(RoRoF_PowersStorage), rrepos->used);
            i = 0;
            break;
        }
        i = room->index;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return count;
}

int check_books_on_subtile_for_reposition_in_room(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    if (map_block_invalid(mapblk))
        return -2; // do nothing
    struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
    if ((roomst->storage_height >= 0) && (get_floor_filled_subtiles_at(stl_x, stl_y) != roomst->storage_height)) {
        return -1; // re-create all
    }
    int matching_things_at_subtile = 0;
    unsigned long k = 0;
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
        // Per thing code
        if (thing_is_spellbook(thing))
        {
            PowerKind spl_idx = book_thing_to_power_kind(thing);
            if ((spl_idx > 0) && ((thing->alloc_flags & TAlF_IsDragged) == 0) && ((thing->owner == room->owner) || game.play_gameturn < 10))//Function is used to integrate preplaced books at map startup too.
            {
                // If exceeded capacity of the library
                if (room->used_capacity > room->total_capacity)
                {
                    SYNCDBG(7,"Room %d type %s capacity %d exceeded; space used is %d", room->index, room_code_name(room->kind), (int)room->total_capacity, (int)room->used_capacity);
                    struct Dungeon* dungeon = get_players_num_dungeon(room->owner);
                    if (dungeon->magic_level[spl_idx] <= 1)
                    { 
                        // We have a single copy, but nowhere to place it. -1 will handle the rest.
                        return -1;
                    }
                    else // We have more than one copy, so we can just delete the book.
                    {
                        if (!is_neutral_thing(thing))
                        {
                            remove_power_from_player(spl_idx, thing->owner);
                        }
                        SYNCLOG("Deleting from %s of player %d duplicate object %s", room_code_name(room->kind), (int)thing->owner, object_code_name(thing->model));
                        delete_thing_structure(thing, 0);
                    }

                } else // we have capacity to spare, so it can stay unless it's stuck
                if (thing_in_wall_at(thing, &thing->mappos)) 
                {
                    if (position_over_floor_level(thing, &thing->mappos)) //If it's inside the floors, simply move it up and count it.
                    {
                        matching_things_at_subtile++; 
                    }
                    else
                    {
                        return -1; // If it's inside the wall or cannot be moved up, recreate all items.
                    }
                } else
                {
                    matching_things_at_subtile++;
                }
            }
        }
        // Per thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    if (matching_things_at_subtile == 0)
    {
        if (room->used_capacity == room->total_capacity) // When 0 is returned, it would try to place books at this subtile. When at capacity already, return -2 to refuse that.
        {
            return -2;
        }
    }
    return matching_things_at_subtile; // Increase used capacity
}

void count_and_reposition_books_in_room_on_subtile(struct Room *room, MapSubtlCoord stl_x, MapSubtlCoord stl_y, struct RoomReposition * rrepos)
{
    int matching_things_at_subtile = check_books_on_subtile_for_reposition_in_room(room, stl_x, stl_y);
    if (matching_things_at_subtile > 0) {
        // This subtile contains spells
        SYNCDBG(19,"Got %d matching things at (%d,%d)",(int)matching_things_at_subtile,(int)stl_x,(int)stl_y);
        room->used_capacity += matching_things_at_subtile;
    } else
    {
        switch (matching_things_at_subtile)
        {
        case -2:
            // No matching things, but also cannot recreate anything on this subtile
            break;
        case -1:
            // All matching things are to be removed from the subtile and stored for re-creation
            reposition_all_books_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        case 0:
            // There are no matching things there, something can be re-created
            recreate_repositioned_book_in_room_on_subtile(room, stl_x, stl_y, rrepos);
            break;
        default:
            WARNLOG("Invalid value returned by reposition check");
            break;
        }
    }
}

/**
 * Updates count of books (used capacity) in a library.
 * Also repositions spellbooks which are in solid rock.
 * @param room The room to be recomputed and repositioned.
 */
void count_books_in_room(struct Room *room)
{
    SYNCDBG(17,"Starting for %s",room_code_name(room->kind));
    struct RoomReposition rrepos;
    init_reposition_struct(&rrepos);
    // Making two loops guarantees that no rrepos things will be lost
    for (long n = 0; n < 2; n++)
    {
        // The correct count should be taken from last sweep
        room->used_capacity = 0;
        room->capacity_used_for_storage = 0;
        unsigned long k = 0;
        unsigned long i = room->slabs_list;
        while (i > 0)
        {
            MapSubtlCoord slb_x = slb_num_decode_x(i);
            MapSubtlCoord slb_y = slb_num_decode_y(i);
            // Per-slab code
            for (long dy = 0; dy < STL_PER_SLB; dy++)
            {
                for (long dx = 0; dx < STL_PER_SLB; dx++)
                {
                    count_and_reposition_books_in_room_on_subtile(room, slab_subtile(slb_x,dx), slab_subtile(slb_y,dy), &rrepos);
                }
            }
            // Per-slab code ends
            i = get_next_slab_number_in_room(i);
            k++;
            if (k > room->slabs_count)
            {
                ERRORLOG("Infinite loop detected when sweeping room slabs");
                break;
            }
        }
    }
    if (rrepos.used > 0) 
    {
        int move_count = position_books_in_room_with_capacity(room->owner, room->kind, &rrepos);
        if (move_count > 0)
        {
            if (rrepos.used > 0)
            {
                SYNCLOG("The %s capacity wasn't enough, %d moved, but %d items belonging to player %d dropped",
                    room_code_name(room->kind), move_count, (int)rrepos.used, (int)room->owner);
            }
            else
            {
                SYNCDBG(7,"Moved %d items belonging to player %d to different %s",
                    move_count, (int)room->owner, room_code_name(room->kind));
            }
        }
        else
        {
            SYNCLOG("No %s capacity available to move, %d items belonging to player %d dropped",
                room_code_name(room->kind), (int)rrepos.used, (int)room->owner);
        }      
    }
    room->capacity_used_for_storage = room->used_capacity;
}
/******************************************************************************/
