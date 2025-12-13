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
#include "pre_inc.h"
#include "keeperfx.hpp"

#include "bflib_math.h"
#include "bflib_keybrd.h"
#include "bflib_sound.h"
#include "thing_list.h"
#include "player_computer.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_objects.h"
#include "room_data.h"
#include "room_library.h"
#include "room_workshop.h"
#include "map_columns.h"
#include "creature_states.h"
#include "magic_powers.h"
#include "game_merge.h"
#include "sounds.h"
#include "game_legacy.h"
#include "game_loop.h"
#include "lua_triggers.h"
#include "post_inc.h"

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
    struct ObjectConfigStats* objst = get_object_model_stats(breaktng->model);
    pos.x.val = subtile_coord_center(breaktng->mappos.x.stl.num + GAME_RANDOM(11) - 5);
    pos.y.val = subtile_coord_center(breaktng->mappos.y.stl.num + GAME_RANDOM(11) - 5);
    pos.z.val = get_floor_height_at(&pos);
    draw_lightning(&breaktng->mappos, &pos, objst->effect.spacing, objst->effect.beam);
    if (!S3DEmitterIsPlayingSample(breaktng->snd_emitter_id, objst->effect.sound_idx, 0)) {
        thing_play_sample(breaktng, objst->effect.sound_idx + SOUND_RANDOM(objst->effect.sound_range), NORMAL_PITCH, -1, 3, 1, 6, FULL_LOUDNESS);
    }
}

void initialise_devastate_dungeon_from_heart(PlayerNumber plyr_idx)
{
    lua_on_dungeon_destroyed(plyr_idx);

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
            dungeon->devastation_centr_x = game.map_subtiles_x / 2;
            dungeon->devastation_centr_y = game.map_subtiles_y / 2;
        }
    }
}

void process_dungeon_destroy(struct Thing* heartng)
{
    if (heartng->owner == game.neutral_player_num)
        return;
    long plyr_idx = heartng->owner;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    struct Thing* soultng = thing_get(dungeon->free_soul_idx);
    struct ObjectConfigStats* objst = get_object_model_stats(heartng->model);
    struct CreatureControl* sctrl;
    if (dungeon->heart_destroy_state == 0)
    {
        return;
    }
    if (heartng->index != dungeon->dnheart_idx)
    {
        return;
    }
    TbBool no_backup = !(dungeon->backup_heart_idx > 0);
    powerful_magic_breaking_sparks(heartng);
    struct Coord3d* central_pos = &heartng->mappos;
    switch (dungeon->heart_destroy_state)
    {
    case 1:
        if (no_backup)
        {
            initialise_devastate_dungeon_from_heart(plyr_idx);
        }
        else
        {
            if ((dungeon->heart_destroy_turn == 10) && (dungeon->free_soul_idx == 0))
            {
                if (!thing_exists(soultng))
                {
                    soultng = create_creature(central_pos, get_players_spectator_model(plyr_idx), plyr_idx);
                }
                if (!thing_is_invalid(soultng))
                {
                    dungeon->num_active_creatrs--;
                    dungeon->owned_creatures_of_model[soultng->model]--;
                    sctrl = creature_control_get_from_thing(soultng);
                    set_flag(sctrl->creature_state_flags,TF2_Spectator);
                    dungeon->free_soul_idx = soultng->index;
                    short xplevel = 0;
                    if (dungeon->lvstats.player_score > 1000)
                    {
                        xplevel = min(((dungeon->lvstats.player_score - 1000) / 10), (CREATURE_MAX_LEVEL - 1));
                    }
                    set_creature_level(soultng, xplevel);
                    initialise_thing_state(soultng, CrSt_CreatureWantsAHome);
                }
            }
            else if (dungeon->heart_destroy_turn == 20)
            {
                // Sets soultng to be invisible for a short amount of time.
                sctrl = creature_control_get_from_thing(soultng);
                set_flag(sctrl->spell_flags, CSAfF_Invisibility);
                sctrl->force_visible = 0;
            }
            else if (dungeon->heart_destroy_turn == 25)
            {
                struct Thing* bheartng = thing_get(dungeon->backup_heart_idx);
                if (thing_is_creature_spectator(soultng))
                {
                    struct Coord3d movepos = bheartng->mappos;
                    movepos.z.val = get_ceiling_height_at(&movepos);
                    move_thing_in_map(soultng, &movepos);
                }
            }
            else if (dungeon->heart_destroy_turn == 28)
            {
                // Clears soultng invisibility.
                sctrl = creature_control_get_from_thing(soultng);
                clear_flag(sctrl->spell_flags, CSAfF_Invisibility);
                sctrl->force_visible = 0;
            }
            else if (dungeon->heart_destroy_turn == 30)
            {
                dungeon->free_soul_idx = 0;
                delete_thing_structure(soultng, 0);
            }
        }
        dungeon->heart_destroy_turn++;
        if (dungeon->heart_destroy_turn < 32)
        {
            if (GAME_RANDOM(96) < (dungeon->heart_destroy_turn << 6) / 32 + 32) {
                create_used_effect_or_element(central_pos, objst->effect.particle, plyr_idx, heartng->index);
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
            create_used_effect_or_element(central_pos, objst->effect.particle, plyr_idx, heartng->index);
        }
        else
        { // Got to next phase
            dungeon->heart_destroy_state = 3;
            dungeon->heart_destroy_turn = 0;
        }
        break;
    case 3:
        // Drop all held things, by keeper
        if ((dungeon->num_things_in_hand > 0) && ((game.conf.rules[plyr_idx].game.classic_bugs_flags & ClscBug_NoHandPurgeOnDefeat) == 0))
        {
            if (no_backup)
                dump_all_held_things_on_map(plyr_idx, central_pos->x.stl.num, central_pos->y.stl.num);
        }
        // Drop all held things, by computer player
        struct Computer2* comp;
        comp = get_computer_player(plyr_idx);
        if (no_backup)
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
        efftng = create_used_effect_or_element(central_pos, objst->effect.explosion1, plyr_idx, heartng->index);
        if (!thing_is_invalid(efftng))
            efftng->shot_effect.hit_type = THit_HeartOnlyNotOwn;
        efftng = create_used_effect_or_element(central_pos, objst->effect.explosion2, plyr_idx, heartng->index);
        if (!thing_is_invalid(efftng))
            efftng->shot_effect.hit_type = THit_HeartOnlyNotOwn;
        destroy_dungeon_heart_room(plyr_idx, heartng);
        delete_thing_structure(heartng, 0);
    }
    { // If there is another heart owned by this player, set it to "working" heart
        struct PlayerInfo* player;
        player = get_player(plyr_idx);
        init_player_start(player, true);
        if (player_has_heart(plyr_idx) && (dungeon->heart_destroy_turn <= 0))
        {
            // If another heart was found, stop the process
            dungeon->devastation_turn = 0;
        }
        else
        {
            if (game.heart_lost_display_message)
            {
                if (is_my_player_number(dungeon->owner))
                {
                    const char* objective = (game.heart_lost_quick_message) ? game.quick_messages[game.heart_lost_message_id] : get_string(game.heart_lost_message_id);
                    process_objective(objective, game.heart_lost_message_target, 0, 0);
                }
            }
            // If this is the last heart the player had, finish him
            setup_all_player_creatures_and_diggers_leave_or_die(plyr_idx);
            player->allied_players = to_flag(player->id_number);
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
    for (i = 0; i < PLAYERS_COUNT; i++)
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
