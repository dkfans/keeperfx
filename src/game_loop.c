/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_loop.c
 *     Module which contains functions from the main game loop.
 * @author   Loobinex
 * @date     14 Jul 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "keeperfx.hpp"

#include "bflib_math.h"
#include "bflib_keybrd.h"
#include "bflib_sound.h"
#include "thing_list.h"
#include "player_computer.h"
#include "thing_effects.h"
#include "room_data.h"
#include "room_library.h"
#include "room_workshop.h"
#include "map_columns.h"
#include "creature_states.h"
#include "magic.h"
#include "game_merge.h"
#include "sounds.h"
#include "game_legacy.h"
#include "game_loop.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/*
 *  Dungeon core destruction
 */
/******************************************************************************/

static void powerful_magic_breaking_sparks(struct Thing* breaktng)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(breaktng->mappos.x.stl.num + UNSYNC_RANDOM(11) - 5);
    pos.y.val = subtile_coord_center(breaktng->mappos.y.stl.num + UNSYNC_RANDOM(11) - 5);
    pos.z.val = get_floor_height_at(&pos);
    draw_lightning(&breaktng->mappos, &pos, 96, TngEffElm_ElectricBall3);
    if (!S3DEmitterIsPlayingSample(breaktng->snd_emitter_id, 157, 0)) {
        thing_play_sample(breaktng, 157, NORMAL_PITCH, -1, 3, 1, 6, FULL_LOUDNESS);
    }
}

void initialise_devastate_dungeon_from_heart(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon->devastation_turn == 0)
    {
        struct Thing* heartng;
        heartng = get_player_soul_container(plyr_idx);
        if (thing_exists(heartng)) {
            dungeon->devastation_turn = 1;
            dungeon->devastation_centr_x = heartng->mappos.x.stl.num;
            dungeon->devastation_centr_y = heartng->mappos.y.stl.num;
        }
        else {
            dungeon->devastation_turn = 1;
            dungeon->devastation_centr_x = map_subtiles_x / 2;
            dungeon->devastation_centr_y = map_subtiles_y / 2;
        }
    }
}

void process_dungeon_destroy(struct Thing* heartng)
{
    struct Dungeon* dungeon;
    long plyr_idx;
    plyr_idx = heartng->owner;
    //_DK_process_dungeon_destroy(heartng); return;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon->heart_destroy_state == 0) {
        return;
    }
    powerful_magic_breaking_sparks(heartng);
    const struct Coord3d* central_pos;
    central_pos = &heartng->mappos;
    switch (dungeon->heart_destroy_state)
    {
    case 1:
        initialise_devastate_dungeon_from_heart(plyr_idx);
        dungeon->heart_destroy_turn++;
        if (dungeon->heart_destroy_turn < 32)
        {
            if (UNSYNC_RANDOM(96) < (dungeon->heart_destroy_turn << 6) / 32 + 32) {
                create_effect(central_pos, TngEff_HearthCollapse, plyr_idx);
            }
        }
        else
        { // Got to next phase
            dungeon->heart_destroy_state = 2;
            dungeon->heart_destroy_turn = 0;
        }
        break;
    case 2:
        dungeon->heart_destroy_turn++;
        if (dungeon->heart_destroy_turn < 32)
        {
            create_effect(central_pos, TngEff_HearthCollapse, plyr_idx);
        }
        else
        { // Got to next phase
            dungeon->heart_destroy_state = 3;
            dungeon->heart_destroy_turn = 0;
        }
        break;
    case 3:
        // Drop all held things, by keeper
        if ((dungeon->num_things_in_hand > 0) && ((gameadd.classic_bugs_flags & ClscBug_NoHandPurgeOnDefeat) == 0))
        {
            dump_all_held_things_on_map(plyr_idx, central_pos->x.stl.num, central_pos->y.stl.num);
        }
        // Drop all held things, by computer player
        struct Computer2* comp;
        comp = get_computer_player(plyr_idx);
        computer_force_dump_held_things_on_map(comp, central_pos);
        // Now if players things are still in hand - they must be kept by enemies
        // Got to next phase
        dungeon->heart_destroy_state = 4;
        dungeon->heart_destroy_turn = 0;
        break;
    case 4:
        // Final phase - destroy the heart, both pedestal room and container thing
    {
        struct Thing* efftng;
        efftng = create_effect(central_pos, TngEff_Explosion4, plyr_idx);
        if (!thing_is_invalid(efftng))
            efftng->hit_type = THit_HeartOnlyNotOwn;
        efftng = create_effect(central_pos, TngEff_WoPExplosion, plyr_idx);
        if (!thing_is_invalid(efftng))
            efftng->hit_type = THit_HeartOnlyNotOwn;
        if (gameadd.heart_lost_display_message)
        {
            if (is_my_player_number(heartng->owner))
            {
                const char *objective = (gameadd.heart_lost_quick_message) ? gameadd.quick_messages[gameadd.heart_lost_message_id] : get_string(gameadd.heart_lost_message_id);
                process_objective(objective, gameadd.heart_lost_message_target, 0, 0);
            }
        }
        destroy_dungeon_heart_room(plyr_idx, heartng);
        delete_thing_structure(heartng, 0);
    }
    { // If there is another heart owned by this player, set it to "working" heart
        struct PlayerInfo* player;
        player = get_player(plyr_idx);
        init_player_start(player, true);
        if (player_has_heart(plyr_idx))
        {
            // If another heart was found, stop the process
            dungeon->devastation_turn = 0;
        }
        else
        {
            // If this is the last heart the player had, finish him
            setup_all_player_creatures_and_diggers_leave_or_die(plyr_idx);
            player->allied_players = (1 << player->id_number);
        }
    }
    dungeon->heart_destroy_state = 0;
    dungeon->heart_destroy_turn = 0;
    break;
    }
}

/******************************************************************************/

void update_research(void)
{
    int i;
    struct PlayerInfo *player;
    SYNCDBG(6,"Starting");
    for (i=0; i<PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player) && (player->is_active == 1))
        {
            process_player_research(i);
        }
    }
}

void update_manufacturing(void)
{
    int i;
    struct PlayerInfo *player;
    SYNCDBG(16,"Starting");
    for (i=0; i<PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player) && (player->is_active == 1))
        {
            process_player_manufacturing(i);
        }
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/******************************************************************************/
