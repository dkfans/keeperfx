/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_entrance.c
 *     Entrance maintain functions.
 * @par Purpose:
 *     Functions to create and use entrances.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "room_entrance.h"
#include "keeperfx.hpp"
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "room_data.h"
#include "room_lair.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "dungeon_stats.h"
#include "player_utils.h"
#include "thing_data.h"
#include "thing_navigate.h"
#include "creature_states.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "post_inc.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Thing *create_creature_at_entrance(struct Room * room, ThingModel crkind)
{
    struct Coord3d pos;
    pos.x.val = room->central_stl_x;
    pos.y.val = room->central_stl_y;
    pos.z.val = subtile_coord(1,0);
    struct Thing* creatng = create_creature(&pos, crkind, room->owner);
    if (thing_is_invalid(creatng)) {
        ERRORLOG("Cannot create creature %s for player %d entrance",creature_code_name(crkind),(int)room->owner);
        return INVALID_THING;
    }
    struct Dungeon* dungeon = get_dungeon(room->owner);
    if (!dungeon_invalid(dungeon))
    {
        if (dungeon->creature_entrance_level > 0)
        {
            set_creature_level(creatng, dungeon->creature_entrance_level);
        }
    }
    mark_creature_joined_dungeon(creatng);
    if (!find_random_valid_position_for_thing_in_room(creatng, room, &pos)) {
        ERRORLOG("Cannot find a valid place in player %d entrance to create creature %s",(int)room->owner,creature_code_name(crkind));
        delete_thing_structure(creatng, 0);
        return INVALID_THING;
    }
    move_thing_in_map(creatng, &pos);
    if (room->owner != game.neutral_player_num)
    {
        dungeon->lvstats.creatures_attracted++;
    }
    struct Thing* heartng = get_player_soul_container(room->owner);
    TRACE_THING(heartng);
    if (thing_exists(heartng))
    {
        if (setup_person_move_to_position(creatng, heartng->mappos.x.stl.num, heartng->mappos.y.stl.num, 0)) {
            creatng->continue_state = CrSt_CreaturePresentToDungeonHeart;
        } else {
            heartng = INVALID_THING;
        }
    }
    if (!thing_exists(heartng))
    {
        set_start_state(creatng);
    }
    return creatng;
}

TbBool generation_due_for_dungeon(struct Dungeon * dungeon)
{
    if (!creature_count_below_map_limit(0))
    {
        SYNCDBG(9, "At map limit");
        return false;
    }

    if ( (game.armageddon_cast_turn == 0) || (game.conf.rules[game.armageddon_caster_idx].magic.armageddon_count_down + game.armageddon_cast_turn > game.play_gameturn) )
    {
        if ( (dungeon->turns_between_entrance_generation != -1) &&
             (game.play_gameturn - dungeon->last_entrance_generation_gameturn >= dungeon->turns_between_entrance_generation) ) {
            SYNCDBG(9,"Due confirmed");
            return true;
        }
    }
    SYNCDBG(9,"Due negative");
    return false;
}

TbBool generation_available_to_dungeon(const struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    if (!dungeon_has_room_of_role(dungeon, RoRoF_CrPoolSpawn))
        return false;
    if (((game.conf.rules[game.armageddon_caster_idx].magic.armageddon_count_down + game.armageddon_cast_turn) > game.play_gameturn) && (game.armageddon_cast_turn > 0)) //No new creatures during armageddon
        return false;
    return ((long)dungeon->num_active_creatrs < (long)dungeon->max_creatures_attracted);
}

long calculate_attractive_room_quantity(RoomKind room_kind, PlayerNumber plyr_idx, int crmodel)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    long slabs_count = get_room_slabs_count(plyr_idx, room_kind);
    long used_fraction;
    switch (room_kind)
    {
    case RoK_LAIR:
        // Add one attractiveness per 2 unused slabs in the room
        used_fraction = get_room_kind_used_capacity_fraction(plyr_idx, room_kind);
        return (slabs_count * (256-used_fraction)) / 256 / 2 - (long)dungeon->owned_creatures_of_model[crmodel];
    case RoK_DUNGHEART:
    case RoK_BRIDGE:
        // Add one attractiveness per 9 slabs of such room
        return slabs_count / 9 - (long)dungeon->owned_creatures_of_model[crmodel];
    case RoK_ENTRANCE:
    case RoK_LIBRARY:
    case RoK_PRISON:
    case RoK_TORTURE:
    case RoK_TRAINING:
    case RoK_SCAVENGER:
    case RoK_TEMPLE:
    case RoK_GRAVEYARD:
    case RoK_BARRACKS:
    case RoK_GUARDPOST:
        // Add one attractiveness per 3 slabs of such room
        return slabs_count / 3 - (long)dungeon->owned_creatures_of_model[crmodel];
    case RoK_WORKSHOP:
    case RoK_GARDEN:
        // Add one attractiveness per 4 slabs of such room
        return slabs_count / 4 - (long)dungeon->owned_creatures_of_model[crmodel];
    case RoK_TREASURE:
        // Add one attractiveness per 3 used slabs in the room
        used_fraction = get_room_kind_used_capacity_fraction(plyr_idx, room_kind);
        return (slabs_count * used_fraction) / 256 / 3;
    case RoK_NONE:
    default:
        return 0;
    }
}

static long calculate_excess_attraction_for_creature(ThingModel crmodel, PlayerNumber plyr_idx)
{
    SYNCDBG(11, "Starting");

    struct CreatureModelConfig* stats = creature_stats_get(crmodel);
    long excess_attraction = 0;
    for (int i = 0; i < ENTRANCE_ROOMS_COUNT; i++)
    {
        RoomKind room_kind = stats->entrance_rooms[i];
        if ((room_kind != RoK_NONE) && (stats->entrance_slabs_req[i] > 0)) {
            // First room adds fully to attraction, second adds only 1/2, third adds 1/3
            excess_attraction += calculate_attractive_room_quantity(room_kind, plyr_idx, crmodel) / (i+1);
        }
    }
    return excess_attraction;
}

long count_player_available_creatures_of_model(PlayerNumber plyr_idx, ThingModel crmodel)
{
    struct Dungeon *dungeon = get_dungeon(plyr_idx);
    long count = 0;
    for (ThingModel i = 0; i < CREATURE_TYPES_MAX; i++)
    {
        if (!creature_model_matches_model(i, plyr_idx, crmodel))
            continue;

        if (creature_will_generate_for_dungeon(dungeon, i))
        {
            count+= game.pool.crtr_kind[i];
        }
    }
    return min(count, dungeon->max_creatures_attracted - (long)dungeon->num_active_creatrs);
}

TbBool creature_will_generate_for_dungeon(const struct Dungeon * dungeon, ThingModel crmodel)
{
    SYNCDBG(11, "Starting for creature model %s", creature_code_name(crmodel));

    if (game.pool.crtr_kind[crmodel] <= 0) {
        SYNCDBG(11, "The %s is not in pool", creature_code_name(crmodel));
        return false;
    }

    // Not allowed creatures can never be attracted
    if (!dungeon->creature_allowed[crmodel]) {
        SYNCDBG(11, "The %s is not allowed for player %d", creature_code_name(crmodel),(int)dungeon->owner);
        return false;
    }

    // Enabled creatures don't need additional conditions to be met
    if (dungeon->creature_force_enabled[crmodel] > dungeon->creature_models_joined[crmodel]) {
        SYNCDBG(11, "The %s is forced for player %d", creature_code_name(crmodel),(int)dungeon->owner);
        return true;
    }

    // Typical way is to allow creatures which meet attraction conditions
    struct CreatureModelConfig* stats = creature_stats_get(crmodel);

    // Check if we've got rooms of enough size for attraction
    for (int i = 0; i < ENTRANCE_ROOMS_COUNT; ++i)
    {
        RoomKind room_kind = stats->entrance_rooms[i];

        if (room_kind != RoK_NONE) {
            int slabs_count = get_room_slabs_count(dungeon->owner, room_kind);

            if (slabs_count < stats->entrance_slabs_req[i]) {
                SYNCDBG(11, "The %s needs more %s space for player %d", creature_code_name(crmodel),room_code_name(room_kind),(int)dungeon->owner);
                return false;
            }
        }
    }

    return true;
}

TbBool remove_creature_from_generate_pool(ThingModel crmodel)
{
    if (game.pool.crtr_kind[crmodel] > INT32_MIN)
    {
        game.pool.crtr_kind[crmodel]--;
        return true;
    }
    WARNLOG("Could not remove creature %s from the creature pool", creature_code_name(crmodel));
    return false;
}

static int calculate_creature_to_generate_for_dungeon(const struct Dungeon * dungeon)
{
    //cumulative frequency
    long crmodel;

    SYNCDBG(9,"Starting");

    long cum_freq = 0;
    long gen_count = 0;
    long crtr_freq[CREATURE_TYPES_MAX];
    crtr_freq[0] = 0;
    for (crmodel = 1; crmodel < game.conf.crtr_conf.model_count; crmodel++)
    {
        if (creature_will_generate_for_dungeon(dungeon, crmodel))
        {
            struct CreatureModelConfig* crconf = creature_stats_get(crmodel);

            gen_count += 1;

            long score = (long)crconf->entrance_score + calculate_excess_attraction_for_creature(crmodel, dungeon->owner);
            if (score < 1) {
                score = 1;
            }
            cum_freq += score;
            crtr_freq[crmodel] = cum_freq;
        } else
        {
            crtr_freq[crmodel] = 0;
        }
    }

    SYNCDBG(19,"Getting random out of %d creature models",(int)gen_count);
    // Select a creature kind to generate based on score we've got for every kind
    // Scores define a chance of being generated.
    if (gen_count > 0)
    {
        if (cum_freq > 0)
        {
            long rnd = PLAYER_RANDOM(dungeon->owner, cum_freq);

            crmodel = 1;
            while (rnd >= crtr_freq[crmodel])
            {
                crmodel++;
                if (crmodel >= game.conf.crtr_conf.model_count) {
                    ERRORLOG("Internal problem; got outside of cummulative range.");
                    return 0;
                }
            }

            return crmodel;
        } else
        {
            ERRORLOG("Bad configuration; creature available but no scores for randomization.");
        }
    }

    return 0;
}

TbBool generate_creature_at_random_entrance(struct Dungeon * dungeon, ThingModel crmodel)
{
    SYNCDBG(9,"Starting");

    struct Room* room = pick_random_room_of_role(dungeon->owner, RoRoF_CrPoolSpawn);
    if (room_is_invalid(room))
    {
        ERRORLOG("Could not get a random entrance for player %d",(int)dungeon->owner);
        return false;
    }
    struct Thing* creatng = create_creature_at_entrance(room, crmodel);
    if (thing_is_invalid(creatng)) {
        return false;
    }
    remove_creature_from_generate_pool(crmodel);
    return true;
}

void generate_creature_for_dungeon(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    ThingModel crmodel = calculate_creature_to_generate_for_dungeon(dungeon);

    if (crmodel > 0)
    {
        struct CreatureModelConfig* crconf = creature_stats_get(crmodel);
        long lair_space = calculate_free_lair_space(dungeon);

        // Creature cannot enter dungeon unless player has enough gold
        if ((long)crconf->pay > dungeon->total_money_owned)
        {
            SYNCDBG(8,"The %s will not come as player %d has less than %d gold",creature_code_name(crmodel),(int)dungeon->owner,(int)crconf->pay);
            if (is_my_player_number(dungeon->owner)) {
                output_message(SMsg_GoldLow, MESSAGE_DURATION_TREASURY);
            }
        }
        else if (lair_space >= 0)
        {
            // Creatures can only enter the dungeon if your Lair has space for them. But one homeless creature is also allowed.
            if (lair_space > 0) {
                SYNCDBG(8,"The %s will come to player %d",creature_code_name(crmodel),(int)dungeon->owner);
            } else {
                SYNCDBG(8,"The %s will come to player %d even though lair is full",creature_code_name(crmodel),(int)dungeon->owner);
            }
            generate_creature_at_random_entrance(dungeon, crmodel);
        }
        else
        {
            // Lair is over capacity
            SYNCDBG(8,"The %s will not come as player %d has lair capacity exceeded",creature_code_name(crmodel),(int)dungeon->owner);
        }
        // Notify player they're out of lair space and play important EvKind_NoMoreLivingSet event for Computer Player.
        if (lair_space <= 0)
        {
            RoomKind rkind = find_first_available_roomkind_with_role(dungeon->owner, RoRoF_LairStorage);
            if (rkind == RoK_NONE) {
                rkind = find_first_roomkind_with_role(RoRoF_LairStorage);
            }
            if (dungeon_has_room_of_role(dungeon, RoRoF_LairStorage)) {
                event_create_event_or_update_nearby_existing_event(0, 0, EvKind_NoMoreLivingSet, dungeon->owner, 0);
                output_room_message(dungeon->owner, rkind, OMsg_RoomTooSmall);
            } else {
                output_room_message(dungeon->owner, rkind, OMsg_RoomNeeded);
            }
        }
    }
    else
    {
        SYNCDBG(9,"There is no creature for player %d",(int)dungeon->owner);
    }
}

void process_entrance_generation(void)
{
    SYNCDBG(8,"Starting");
    TbBool due = false;
    for (long i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* plyr = get_player(i);
        if (!player_exists(plyr)) {
            continue;
        }
        struct Dungeon* dungeon = get_players_dungeon(plyr);
        if (!dungeon_invalid(dungeon))
        {
            if (generation_due_for_dungeon(dungeon))
            {
                due = true;
                if ((plyr->is_active) && (plyr->victory_state != VicS_LostLevel) )
                {
                    if (generation_available_to_dungeon(dungeon)) {
                        generate_creature_for_dungeon(dungeon);
                    }
                    dungeon->last_entrance_generation_gameturn = game.play_gameturn;
                    dungeon->portal_scavenge_boost = 0;
                }
            }
        }
    }
    if (due)
    {
        if (game.armageddon_cast_turn == 0)
        {
            update_dungeons_scores();
            update_dungeon_generation_speeds();
        }
    }
}
/******************************************************************************/
TbBool update_creature_pool_state(void)
{
    int i;
    game.pool.is_empty = true;
    for (i=1; i < game.conf.crtr_conf.model_count; i++)
    {
        if (game.pool.crtr_kind[i] > 0)
        { game.pool.is_empty = false; break; }
    }
    return true;
}

void add_creature_to_pool(ThingModel kind, int32_t amount)
{
    kind %= game.conf.crtr_conf.model_count;

    if (amount > 0 && game.pool.crtr_kind[kind] > INT32_MAX - amount)
    {
        game.pool.crtr_kind[kind] = INT32_MAX;
    }
    else if (amount < 0 && game.pool.crtr_kind[kind] < INT32_MIN - amount)
    {
        game.pool.crtr_kind[kind] = INT32_MIN;
    }
    else
    {
        game.pool.crtr_kind[kind] += amount;
    }
}
