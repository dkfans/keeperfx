/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_utils.c
 *     Player data structures definitions.
 * @par Purpose:
 *     Defines functions for player-related structures support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_utils.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_sound.h"

#include "player_data.h"
#include "player_instances.h"
#include "player_computer.h"
#include "dungeon_data.h"
#include "power_hand.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "front_simple.h"
#include "front_lvlstats.h"
#include "gui_soundmsgs.h"
#include "gui_frontmenu.h"
#include "config_settings.h"
#include "game_saves.h"
#include "game_legacy.h"
#include "frontend.h"
#include "magic.h"
#include "engine_redraw.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_map.h"
#include "keeperfx.hpp"

/******************************************************************************/
/******************************************************************************/
DLLIMPORT long _DK_take_money_from_dungeon(short a1, long a2, unsigned char a3);
DLLIMPORT long _DK_update_dungeon_generation_speeds(void);
DLLIMPORT void _DK_calculate_dungeon_area_scores(void);
DLLIMPORT void _DK_post_init_players(void);
DLLIMPORT void _DK_init_player(struct PlayerInfo *player, int a2);
DLLIMPORT void _DK_init_keeper_map_exploration(struct PlayerInfo *player);
DLLIMPORT long _DK_wander_point_initialise(struct Wander *wandr, long plyr_idx, long a3);
DLLIMPORT long _DK_wp_check_map_pos_valid(struct Wander *wandr, long a1);
DLLIMPORT long _DK_wander_point_update(struct Wander *wandr);
DLLIMPORT void _DK_process_player_states(void);
/******************************************************************************/
TbBool player_has_won(PlayerNumber plyr_idx)
{
  struct PlayerInfo *player;
  player = get_player(plyr_idx);
  if (player_invalid(player))
    return false;
  return (player->victory_state == VicS_WonLevel);
}

TbBool player_has_lost(PlayerNumber plyr_idx)
{
  struct PlayerInfo *player;
  player = get_player(plyr_idx);
  if (player_invalid(player))
    return false;
  return (player->victory_state == VicS_LostLevel);
}

/**
 * Returns whether given player has no longer any chance to win.
 * @param plyr_idx
 * @return
 */
TbBool player_cannot_win(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    if (plyr_idx == game.neutral_player_num)
        return true;
    player = get_player(plyr_idx);
    if (!player_exists(player))
        return true;
    if (player->victory_state == VicS_LostLevel)
        return true;
    struct Thing *heartng;
    heartng = get_player_soul_container(player->id_number);
    if (!thing_exists(heartng) || (heartng->active_state == ObSt_BeingDestroyed))
        return true;
    return false;
}

void set_player_as_won_level(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  if (player->victory_state != VicS_Undecided)
  {
      WARNLOG("Player fate is already decided to %d",(int)player->victory_state);
      return;
  }
  if (is_my_player(player))
    frontstats_initialise();
  player->victory_state = VicS_WonLevel;
  dungeon = get_dungeon(player->id_number);
  // Computing player score
  dungeon->lvstats.player_score = compute_player_final_score(player, dungeon->max_gameplay_score);
  dungeon->lvstats.allow_save_score = 1;
  if ((game.system_flags & GSF_NetworkActive) == 0)
    player->field_4EB = game.play_gameturn + 300;
  if (is_my_player(player))
  {
    if (lord_of_the_land_in_prison_or_tortured())
    {
        SYNCLOG("Lord Of The Land kept captive. Torture tower unlocked.");
        player->field_3 |= 0x10;
    }
    output_message(SMsg_LevelWon, 0, true);
  }
}

void set_player_as_lost_level(struct PlayerInfo *player)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    if (player->victory_state != VicS_Undecided)
    {
        WARNLOG("Victory state already set to %d",(int)player->victory_state);
        return;
    }
    if (is_my_player(player))
      frontstats_initialise();
    player->victory_state = VicS_LostLevel;
    dungeon = get_dungeon(player->id_number);
    // Computing player score
    dungeon->lvstats.player_score = compute_player_final_score(player, dungeon->max_gameplay_score);
    if (is_my_player(player))
    {
      output_message(SMsg_LevelFailed, 0, true);
      turn_off_all_menus();
      clear_transfered_creature();
    }
    clear_things_in_hand(player);
    dungeon->num_things_in_hand = 0;
    if (player_uses_call_to_arms(player->id_number))
        turn_off_call_to_arms(player->id_number);
    if (player_uses_power_sight(player->id_number))
    {
        thing = thing_get(dungeon->sight_casted_thing_idx);
        delete_thing_structure(thing, 0);
        dungeon->sight_casted_thing_idx = 0;
    }
    if (is_my_player(player))
        gui_set_button_flashing(0, 0);
    set_player_mode(player, 1);
    set_player_state(player, 1, 0);
    if ((game.system_flags & GSF_NetworkActive) == 0)
        player->field_4EB = game.play_gameturn + 300;
    if ((game.system_flags & GSF_NetworkActive) != 0)
        reveal_whole_map(player);
    if ((dungeon->computer_enabled & 0x01) != 0)
        toggle_computer_player(player->id_number);
}

long compute_player_final_score(struct PlayerInfo *player, long gameplay_score)
{
    long i;
    if (((game.system_flags & GSF_NetworkActive) != 0)
      || !is_singleplayer_level(game.loaded_level_number)) {
        i = 2 * gameplay_score;
    } else {
        i = gameplay_score + 10 * gameplay_score * array_index_for_singleplayer_level(game.loaded_level_number) / 100;
    }
    if (player_has_lost(player->id_number))
        i /= 2;
    return i;
}

long take_money_from_dungeon(PlayerNumber plyr_idx, long a2, unsigned char a3)
{
  return _DK_take_money_from_dungeon(plyr_idx, a2, a3);
}

long update_dungeon_generation_speeds(void)
{
    int plr_idx;
    int n;
    //return _DK_update_dungeon_generation_speeds();
    // Get value of generation
    n = 0;
    for (plr_idx=0; plr_idx<PLAYERS_COUNT; plr_idx++)
    {
        struct PlayerInfo *player;
        player = get_player(plr_idx);
        if (player_exists(player) && (player->field_2C == 1))
        {
            struct Dungeon *dungeon;
            dungeon = get_players_dungeon(player);
            if (dungeon->field_AE9[1] > n)
                n = dungeon->field_AE9[0];
        }
    }
    // Update the values
    if (game.generate_speed == -1)
    {
        for (plr_idx=0; plr_idx<PLAYERS_COUNT; plr_idx++)
        {
            struct PlayerInfo *player;
            player = get_player(plr_idx);
            if (player_exists(player) && (player->field_2C == 1))
            {
                struct Dungeon *dungeon;
                dungeon = get_players_dungeon(player);
                dungeon->turns_between_entrance_generation = 0;
            }
        }
    } else
    {
        for (plr_idx=0; plr_idx<PLAYERS_COUNT; plr_idx++)
        {
            struct PlayerInfo *player;
            player = get_player(plr_idx);
            if (player_exists(player) && (player->field_2C == 1))
            {
                struct Dungeon *dungeon;
                dungeon = get_players_dungeon(player);
                dungeon->turns_between_entrance_generation = n * game.generate_speed / dungeon->field_AE9[0];
            }
        }
    }
    return 1;
}

void calculate_dungeon_area_scores(void)
{
  _DK_calculate_dungeon_area_scores();
}

void init_player_music(struct PlayerInfo *player)
{
    LevelNumber lvnum;
    lvnum = get_loaded_level_number();
    game.audiotrack = ((lvnum - 1) % -4) + 3;
    randomize_sound_font();
}

void init_keeper_map_exploration(struct PlayerInfo *player)
{
  _DK_init_keeper_map_exploration(player);
}

void init_player_as_single_keeper(struct PlayerInfo *player)
{
    unsigned short idx;
    struct InitLight ilght;
    memset(&ilght, 0, sizeof(struct InitLight));
    player->field_4CD = 0;
    ilght.field_0 = 2560;
    ilght.field_2 = 48;
    ilght.field_3 = 5;
    ilght.is_dynamic = 1;
    idx = light_create_light(&ilght);
    player->field_460 = idx;
    if (idx != 0) {
        light_set_light_never_cache(idx);
    } else {
        WARNLOG("Cannot allocate light to player %d.",(int)player->id_number);
    }
}

void init_player(struct PlayerInfo *player, short no_explore)
{
    SYNCDBG(5,"Starting");
    //_DK_init_player(player, no_explore); return;
    player->mouse_x = 10;
    player->mouse_y = 12;
    player->minimap_zoom = 256;
    player->field_4D1 = player->id_number;
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
    player->continue_work_state = PSt_CtrlDungeon;
    player->work_state = PSt_CtrlDungeon;
    player->field_14 = 2;
    player->palette = engine_palette;
    if (is_my_player(player))
    {
        set_flag_byte(&game.numfield_C,0x40,true);
        set_gui_visible(true);
        init_gui();
        turn_on_menu(GMnu_MAIN);
        turn_on_menu(GMnu_ROOM);
    }
    switch (game.game_kind)
    {
    case GKind_NetworkGame:
        init_player_as_single_keeper(player);
        init_player_start(player, false);
        reset_player_mode(player, 1);
        if ( !no_explore )
          init_keeper_map_exploration(player);
        break;
    case GKind_KeeperGame:
        if (player->field_2C != 1)
        {
          ERRORLOG("Non Keeper in Keeper game");
          break;
        }
        init_player_as_single_keeper(player);
        init_player_start(player, false);
        reset_player_mode(player, 1);
        init_keeper_map_exploration(player);
        break;
    default:
        ERRORLOG("How do I set up this player?");
        break;
    }
    init_player_cameras(player);
    pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
    player->mp_message_text[0] = '\0';
    if (is_my_player(player))
    {
        init_player_music(player);
    }
    // By default, player is his own ally
    player->allied_players = (1 << player->id_number);
    player->field_10 = 0;
}

void init_players(void)
{
    struct PlayerInfo *player;
    int i;
    for (i=0;i<PLAYERS_COUNT;i++)
    {
        player = get_player(i);
        player->field_0 ^= (player->field_0 ^ ((game.packet_save_head.field_C & (1 << i)) >> i)) & 1;
        if (player_exists(player))
        {
            player->id_number = i;
            player->field_0 ^= (player->field_0 ^ (((game.packet_save_head.field_D & (1 << i)) >> i) << 6)) & 0x40;
            if ((player->field_0 & 0x40) == 0)
            {
              game.field_14E495++;
              player->field_2C = 1;
              game.game_kind = GKind_KeeperGame;
              init_player(player, 0);
            }
        }
    }
}

TbBool wp_check_map_pos_valid(struct Wander *wandr, SubtlCodedCoords stl_num)
{
    SYNCDBG(16,"Starting");
    //return _DK_wp_check_map_pos_valid(wandr, stl_num);
    MapSubtlCoord stl_x,stl_y;
    long plyr_bit;
    stl_x = stl_num_decode_x(stl_num);
    stl_y = stl_num_decode_y(stl_num);
    plyr_bit = wandr->plyr_bit;
    if (wandr->store_revealed)
    {
        struct Map *mapblk;
        mapblk = get_map_block_at_pos(stl_num);
        // Add only tiles which are revealed to the wandering player, unless it's heroes - for them, add all
        if ((((1 << game.hero_player_num) & plyr_bit) != 0) || map_block_revealed_bit(mapblk, plyr_bit))
        {
            if (((mapblk->flags & 0x10) == 0) && ((get_navigation_map(stl_x, stl_y) & 0x10) == 0))
            {
                return true;
            }
        }
    } else
    {
        struct Map *mapblk;
        mapblk = get_map_block_at_pos(stl_num);
        // Add only tiles which are not revealed to the wandering player, unless it's heroes - for them, do nothing
        if ((((1 << game.hero_player_num) & plyr_bit) == 0) && !map_block_revealed_bit(mapblk, plyr_bit))
        {
            if (((mapblk->flags & 0x10) == 0) && ((get_navigation_map(stl_x, stl_y) & 0x10) == 0))
            {
                struct Thing *heartng;
                heartng = get_player_soul_container(wandr->plyr_idx);
                if (!thing_is_invalid(heartng))
                {
                    struct Coord3d dstpos;
                    dstpos.x.val = subtile_coord_center(stl_x);
                    dstpos.y.val = subtile_coord_center(stl_y);
                    dstpos.z.val = subtile_coord(1,0);
                    if (navigation_points_connected(&heartng->mappos, &dstpos))
                      return true;
                }
            }
        }
    }
    return false;
}

TbBool wander_point_add(struct Wander *wandr, SubtlCodedCoords stl_num)
{
    unsigned long i;
    i = wandr->point_insert_idx;
    wandr->points[i].stl_x = stl_num_decode_x(stl_num);
    wandr->points[i].stl_y = stl_num_decode_y(stl_num);
    wandr->point_insert_idx = (i + 1) % WANDER_POINTS_COUNT;
    if (wandr->points_count < WANDER_POINTS_COUNT)
      wandr->points_count++;
    return true;
}

/**
 * Stores up to given amount of wander points into given wander structure.
 * If required, selects several evenly distributed points from the input array.
 * @param wandr
 * @param stl_num_list
 * @param stl_num_count
 * @param max_to_store
 * @return
 */
TbBool store_wander_points_up_to(struct Wander *wandr, const SubtlCodedCoords stl_num_list[], long stl_num_count, long max_to_store)
{
    long i;
    if (stl_num_count > max_to_store)
    {
        double realidx,delta;
        if (wandr->max_found_per_check <= 0)
            return 1;
        wandr->point_insert_idx %= WANDER_POINTS_COUNT;
        delta = ((double)stl_num_count) / max_to_store;
        realidx = 0.1; // A little above zero to avoid float rounding errors
        for (i = 0; i < max_to_store; i++)
        {
            wander_point_add(wandr, stl_num_list[(unsigned int)(realidx)]);
            realidx += delta;
        }
    } else
    {
        // Otherwise, add all points to the wander array
        for (i = 0; i < stl_num_count; i++)
        {
            wander_point_add(wandr, stl_num_list[i]);
        }
    }
    return true;
}

long wander_point_initialise(struct Wander *wandr, PlayerNumber plyr_idx, long store_revealed)
{
    //return _DK_wander_point_initialise(wandr, plyr_idx, store_revealed);
    wandr->store_revealed = store_revealed;
    wandr->plyr_idx = plyr_idx;
    wandr->point_insert_idx = 0;
    wandr->last_checked_slb_num = 0;
    wandr->plyr_bit = (1 << plyr_idx);
    wandr->num_check_per_run = 20;
    wandr->max_found_per_check = 4;
    wandr->wdrfield_14 = 0;

    SubtlCodedCoords *stl_num_list;
    long stl_num_list_count;
    SlabCodedCoords slb_num;
    stl_num_list_count = 0;
    stl_num_list = (SubtlCodedCoords *)scratch;
    while (1)
    {
        MapSlabCoord slb_x,slb_y;
        SubtlCodedCoords stl_num;
        slb_x = slb_num_decode_x(slb_num);
        slb_y = slb_num_decode_y(slb_num);
        stl_num = get_subtile_number(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (wp_check_map_pos_valid(wandr, stl_num))
        {
            if (stl_num_list_count >= 0x10000/sizeof(SubtlCodedCoords)-1)
                break;
            stl_num_list[stl_num_list_count] = stl_num;
            stl_num_list_count++;
        }
        slb_num++;
        if (slb_num >= map_tiles_x*map_tiles_y) {
            break;
        }
    }
    // Check if we have found anything
    if (stl_num_list_count <= 0)
        return 1;
    // If we have too many points, use only some of them
    store_wander_points_up_to(wandr, stl_num_list, stl_num_list_count, WANDER_POINTS_COUNT);
    return 1;
}

#define LOCAL_LIST_SIZE 20
long wander_point_update(struct Wander *wandr)
{
    SubtlCodedCoords stl_num_list[LOCAL_LIST_SIZE];
    long stl_num_list_count;
    SlabCodedCoords slb_num;
    long i;
    SYNCDBG(6,"Starting");
    //return _DK_wander_point_update(wandr);
    // Find up to 20 numbers (starting where we ended last time) and store them in local array
    slb_num = wandr->last_checked_slb_num;
    stl_num_list_count = 0;
    for (i = 0; i < wandr->num_check_per_run; i++)
    {
        MapSlabCoord slb_x,slb_y;
        SubtlCodedCoords stl_num;
        slb_x = slb_num_decode_x(slb_num);
        slb_y = slb_num_decode_y(slb_num);
        stl_num = get_subtile_number(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
        if (wp_check_map_pos_valid(wandr, stl_num))
        {
            if (stl_num_list_count >= LOCAL_LIST_SIZE)
                break;
            stl_num_list[stl_num_list_count] = stl_num;
            stl_num_list_count++;
            if ((wandr->wdrfield_14 != 0) && (stl_num_list_count == wandr->max_found_per_check))
            {
                slb_num = (wandr->num_check_per_run + wandr->last_checked_slb_num) % (map_tiles_x*map_tiles_y);
                break;
            }
        }
        slb_num++;
        if (slb_num >= map_tiles_x*map_tiles_y) {
            slb_num = 0;
        }
    }
    wandr->last_checked_slb_num = slb_num;
    // Check if we have found anything
    if (stl_num_list_count <= 0)
        return 1;
    // If we have too many points, use only some of them
    store_wander_points_up_to(wandr, stl_num_list, stl_num_list_count, wandr->max_found_per_check);
    return 1;
}
#undef LOCAL_LIST_SIZE

void post_init_player(struct PlayerInfo *player)
{
    switch (game.game_kind)
    {
    case 3:
        break;
    case 2:
    case 5:
        wander_point_initialise(&player->wandr1, player->id_number, 1);
        wander_point_initialise(&player->wandr2, player->id_number, 0);
        break;
    default:
        if ((player->field_0 & 0x40) == 0)
          ERRORLOG("Invalid GameMode");
        break;
    }
}

void post_init_players(void)
{
    PlayerNumber plyr_idx;
    //_DK_post_init_players(); return;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo *player;
        player = get_player(plyr_idx);
        if ((player->field_0 & 0x01) != 0) {
            post_init_player(player);
        }
    }
}

void init_players_local_game(void)
{
    struct PlayerInfo *player;
    SYNCDBG(4,"Starting");
    player = get_my_player();
    player->id_number = my_player_number;
    player->field_0 |= 0x01;
    if (settings.field_3 < 1)
      player->field_4B5 = 2;
    else
      player->field_4B5 = 5;
    init_player(player, 0);
}

void process_player_states(void)
{
    SYNCDBG(6,"Starting");
    _DK_process_player_states();
}

void process_players(void)
{
    int i;
    struct PlayerInfo *player;
    SYNCDBG(5,"Starting");
    process_player_instances();
    process_player_states();
    for (i=0; i<PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player) && (player->field_2C == 1))
        {
            SYNCDBG(6,"Doing updates for player %d",i);
            wander_point_update(&player->wandr1);
            wander_point_update(&player->wandr2);
            update_power_sight_explored(player);
            update_player_objectives(i);
        }
    }
    SYNCDBG(17,"Finished");
}

TbBool player_sell_trap_at_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    MapSlabCoord slb_x,slb_y;
    long sell_value;
    thing = get_trap_for_slab_position(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
    if (thing_is_invalid(thing))
    {
        return false;
    }
    dungeon = get_players_num_dungeon(thing->owner);
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    sell_value = 0;
    remove_traps_around_subtile(slab_subtile_center(slb_x), slab_subtile_center(slb_y), &sell_value);
    if (is_my_player_number(plyr_idx))
        play_non_3d_sample(115);
    dungeon->camera_deviate_jump = 192;
    struct Coord3d pos;
    set_coords_to_slab_center(&pos,slb_x,slb_y);
    if (sell_value != 0)
    {
        create_price_effect(&pos, plyr_idx, sell_value);
        player_add_offmap_gold(plyr_idx,sell_value);
    } else
    {
        WARNLOG("Sold traps at (%d,%d) which didn't cost anything",(int)stl_x,(int)stl_y);
    }
    { // Add the trap location to related computer player, in case we'll want to place a trap again
        struct Computer2 *comp;
        comp = get_computer_player(plyr_idx);
        if (!computer_player_invalid(comp)) {
            add_to_trap_location(comp, &pos);
        }
    }
    return true;
}

TbBool player_sell_door_at_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    MapSubtlCoord x,y;
    long i;
    x = stl_slab_center_subtile(stl_x);
    y = stl_slab_center_subtile(stl_y);
    thing = get_door_for_position(x, y);
    if (thing_is_invalid(thing))
    {
        return false;
    }
    dungeon = get_players_num_dungeon(thing->owner);
    dungeon->camera_deviate_jump = 192;
    i = game.doors_config[thing->model].selling_value;
    destroy_door(thing);
    if (is_my_player_number(plyr_idx))
        play_non_3d_sample(115);
    struct Coord3d pos;
    set_coords_to_slab_center(&pos,subtile_slab_fast(stl_x),subtile_slab_fast(stl_y));
    if (i != 0)
    {
        create_price_effect(&pos, plyr_idx, i);
        player_add_offmap_gold(plyr_idx, i);
    }
    { // Add the trap location to related computer player, in case we'll want to place a trap again
        struct Computer2 *comp;
        comp = get_computer_player(plyr_idx);
        if (!computer_player_invalid(comp)) {
            add_to_trap_location(comp, &pos);
        }
    }
    return true;
}
/******************************************************************************/
