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

#include "game_loop.h"

#include <winbase.h>
#include <math.h>
#include <string>
#include "keeperfx.hpp"

#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_keybrd.h"
#include "bflib_inputctrl.h"
#include "bflib_datetm.h"
#include "bflib_sprfnt.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_sndlib.h"
#include "bflib_cpu.h"
#include "bflib_crash.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"
#include "bflib_mouse.h"
#include "bflib_filelst.h"
#include "bflib_network.h"

#include "version.h"
#include "front_simple.h"
#include "frontend.h"
#include "front_input.h"
#include "gui_draw.h"
#include "gui_tooltips.h"
#include "gui_parchment.h"
#include "gui_frontmenu.h"
#include "gui_msgs.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "kjm_input.h"
#include "packets.h"
#include "config.h"
#include "config_strings.h"
#include "config_campaigns.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "config_objects.h"
#include "config_rules.h"
#include "config_lenses.h"
#include "config_magic.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_crtrmodel.h"
#include "config_compp.h"
#include "config_effects.h"
#include "lvl_script.h"
#include "lvl_filesdk1.h"
#include "thing_list.h"
#include "player_instances.h"
#include "player_utils.h"
#include "player_states.h"
#include "player_computer.h"
#include "game_heap.h"
#include "game_saves.h"
#include "engine_render.h"
#include "engine_lenses.h"
#include "engine_camera.h"
#include "engine_arrays.h"
#include "engine_textures.h"
#include "engine_redraw.h"
#include "front_landview.h"
#include "front_lvlstats.h"
#include "front_easter.h"
#include "front_fmvids.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_creature.h"
#include "thing_corpses.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_doors.h"
#include "thing_traps.h"
#include "thing_shots.h"
#include "thing_navigate.h"
#include "thing_factory.h"
#include "slab_data.h"
#include "room_data.h"
#include "room_entrance.h"
#include "room_jobs.h"
#include "room_util.h"
#include "room_library.h"
#include "map_columns.h"
#include "map_events.h"
#include "map_utils.h"
#include "map_blocks.h"
#include "ariadne_wallhug.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "creature_states_rsrch.h"
#include "creature_states_lair.h"
#include "creature_states_mood.h"
#include "lens_api.h"
#include "light_data.h"
#include "magic.h"
#include "power_process.h"
#include "power_hand.h"
#include "power_specials.h"
#include "game_merge.h"
#include "gui_topmsg.h"
#include "gui_boxmenu.h"
#include "gui_soundmsgs.h"
#include "gui_frontbtns.h"
#include "frontmenu_ingame_tabs.h"
#include "ariadne.h"
#include "net_game.h"
#include "sounds.h"
#include "vidfade.h"
#include "KeeperSpeech.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "room_list.h"
#include "game_loop.h"
#include "map_data.h"

#include "music_player.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

static void powerful_magic_breaking_sparks(struct Thing* breaktng)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(breaktng->mappos.x.stl.num + ACTION_RANDOM(11) - 5);
    pos.y.val = subtile_coord_center(breaktng->mappos.y.stl.num + ACTION_RANDOM(11) - 5);
    pos.z.val = get_floor_height_at(&pos);
    draw_lightning(&breaktng->mappos, &pos, 96, 60);
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
            if (ACTION_RANDOM(96) < (dungeon->heart_destroy_turn << 6) / 32 + 32) {
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
            efftng->byte_16 = 8;
        efftng = create_effect(central_pos, TngEff_WoPExplosion, plyr_idx);
        if (!thing_is_invalid(efftng))
            efftng->byte_16 = 8;
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
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/******************************************************************************/
