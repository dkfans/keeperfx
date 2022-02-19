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
#include "bflib_math.h"
#include "bflib_sound.h"

#include "player_data.h"
#include "player_instances.h"
#include "player_states.h"
#include "player_computer.h"
#include "dungeon_data.h"
#include "power_hand.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "frontmenu_ingame_evnt.h"
#include "front_simple.h"
#include "front_lvlstats.h"
#include "gui_msgs.h"
#include "gui_soundmsgs.h"
#include "gui_frontmenu.h"
#include "config_settings.h"
#include "config_terrain.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "ariadne_wallhug.h"
#include "game_saves.h"
#include "game_legacy.h"
#include "frontend.h"
#include "magic.h"
#include "engine_redraw.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_map.h"
#include "keeperfx.hpp"
#include "kjm_input.h"

/******************************************************************************/
/******************************************************************************/
DLLIMPORT void _DK_fill_in_explored_area(unsigned char plyr_idx, short stl_x, short stl_y);
/******************************************************************************/
TbBool player_has_won(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_invalid(player))
        return false;
    return (player->victory_state == VicS_WonLevel);
}

TbBool player_has_lost(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
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
    if (plyr_idx == game.neutral_player_num)
        return true;
    struct PlayerInfo* player = get_player(plyr_idx);
    if (!player_exists(player))
        return true;
    if (player->victory_state == VicS_LostLevel)
        return true;
    struct Thing* heartng = get_player_soul_container(player->id_number);
    if (!thing_exists(heartng) || (heartng->active_state == ObSt_BeingDestroyed))
        return true;
    return false;
}

void set_player_as_won_level(struct PlayerInfo *player)
{
  if (player->victory_state != VicS_Undecided)
  {
      //WARNLOG("Player fate is already decided to %d",(int)player->victory_state);
      return;
  }
  if (is_my_player(player))
    frontstats_initialise();
  player->victory_state = VicS_WonLevel;
  struct Dungeon* dungeon = get_dungeon(player->id_number);
  if ( timer_enabled() )
  {
    if (TimerGame)
    {
        TimerTurns = dungeon->lvstats.hopes_dashed;
        update_time();
    }
    else
    {
        show_real_time_taken();
    }
    struct GameTime GameT = get_game_time(dungeon->lvstats.hopes_dashed, game.num_fps);
    SYNCMSG("Won level %ld. Total turns taken: %ld (%02d:%02d:%02d at %d fps). Real time elapsed: %02d:%02d:%02d:%03d.", game.loaded_level_number, dungeon->lvstats.hopes_dashed, GameT.Hours, GameT.Minutes, GameT.Seconds, game.num_fps, Timer.Hours, Timer.Minutes, Timer.Seconds, Timer.MSeconds);
  }
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
        player->additional_flags |= PlaAF_UnlockedLordTorture;
    }
    output_message(SMsg_LevelWon, 0, true);
  }
}

void set_player_as_lost_level(struct PlayerInfo *player)
{
    if (player->victory_state != VicS_Undecided)
    {
        // Suppress redundant warnings
        if ((game.system_flags & GSF_RunAfterVictory) == 0)
        {
            WARNLOG("Victory state already set to %d",(int)player->victory_state);
        }
        return;
    }
    SYNCLOG("Player %d lost",(int)player->id_number);
    if (is_my_player(player))
    {
        frontstats_initialise();
    }
    player->victory_state = VicS_LostLevel;
    struct Dungeon* dungeon = get_dungeon(player->id_number);
    // Computing player score
    dungeon->lvstats.player_score = compute_player_final_score(player, dungeon->max_gameplay_score);
    if (is_my_player(player))
    {
        output_message(SMsg_LevelFailed, 0, true);
        turn_off_all_menus();
        clear_transfered_creature();
    }
    if ((gameadd.classic_bugs_flags & ClscBug_NoHandPurgeOnDefeat) == 0) {
        clear_things_in_hand(player);
        dungeon->num_things_in_hand = 0;
    }
    if (player_uses_power_call_to_arms(player->id_number))
        turn_off_power_call_to_arms(player->id_number);
    if (player_uses_power_sight(player->id_number))
    {
        struct Thing* thing = thing_get(dungeon->sight_casted_thing_idx);
        delete_thing_structure(thing, 0);
        dungeon->sight_casted_thing_idx = 0;
    }
    if (is_my_player(player))
        gui_set_button_flashing(0, 0);
    set_player_mode(player, PVT_DungeonTop);
    set_player_state(player, PSt_CtrlDungeon, 0);
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

/**
 * Takes money from hoards stored in given room.
 * @param room The room which contains gold hoards.
 * @param amount_take Amount of gold to be taken.
 * @return Gives amount of gold taken from room.
 */
GoldAmount take_money_from_room(struct Room *room, GoldAmount amount_take)
{
    GoldAmount amount = amount_take;
    // Remove gold from room border slabs
    unsigned long k = 0;
    unsigned long slbnum = room->slabs_list;
    while (slbnum > 0)
    {
        struct SlabMap* slb = get_slabmap_direct(slbnum);
        if (slabmap_block_invalid(slb)) {
            ERRORLOG("Jump to invalid room slab detected");
            break;
        }
        // Per-slab code starts
        MapSlabCoord slb_x = slb_num_decode_x(slbnum);
        MapSlabCoord slb_y = slb_num_decode_y(slbnum);
        MapSubtlCoord stl_x = slab_subtile_center(slb_x);
        MapSubtlCoord stl_y = slab_subtile_center(slb_y);
        if (slab_is_area_outer_border(slb_x, slb_y))
        {
            struct Thing* hrdtng = find_gold_hoard_at(stl_x, stl_y);
            if (!thing_is_invalid(hrdtng)) {
                amount -= remove_gold_from_hoarde(hrdtng, room, amount);
            }
        }
        if (amount <= 0)
          break;
        // Per-slab code ends
        slbnum = get_next_slab_number_in_room(slbnum);
        k++;
        if (k > map_tiles_x * map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    if (amount <= 0)
        return amount_take-amount;
    // Remove gold from room center only if borders are clear
    k = 0;
    slbnum = room->slabs_list;
    while (slbnum > 0)
    {
        struct SlabMap* slb = get_slabmap_direct(slbnum);
        if (slabmap_block_invalid(slb)) {
            ERRORLOG("Jump to invalid room slab detected");
            break;
        }
        // Per-slab code starts
        MapSubtlCoord stl_x = slab_subtile_center(slb_num_decode_x(slbnum));
        MapSubtlCoord stl_y = slab_subtile_center(slb_num_decode_y(slbnum));
        {
            struct Thing* hrdtng = find_gold_hoard_at(stl_x, stl_y);
            if (!thing_is_invalid(hrdtng)) {
                amount -= remove_gold_from_hoarde(hrdtng, room, amount);
            }
        }
        if (amount <= 0)
          break;
        // Per-slab code ends
        slbnum = get_next_slab_number_in_room(slbnum);
        k++;
        if (k > map_tiles_x * map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    return amount_take-amount;
}

long take_money_from_dungeon_f(PlayerNumber plyr_idx, GoldAmount amount_take, TbBool only_whole_sum, const char *func_name)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        WARNLOG("%s: Cannot take gold from player %d with no dungeon",func_name,(int)plyr_idx);
        return -1;
    }
    GoldAmount take_remain = amount_take;
    GoldAmount total_money = dungeon->total_money_owned;
    if (take_remain <= 0) {
        WARNLOG("%s: No gold needed to be taken from player %d",func_name,(int)plyr_idx);
        return 0;
    }
    if (take_remain > total_money)
    {
        SYNCDBG(7,"%s: Player %d has only %d gold, cannot get %d from him",func_name,(int)plyr_idx,(int)total_money,(int)take_remain);
        if ((only_whole_sum) || (total_money == 0)) {
            return -1;
        }
        take_remain = dungeon->total_money_owned;
        amount_take = dungeon->total_money_owned;
    }
    GoldAmount offmap_money = dungeon->offmap_money_owned;
    if (offmap_money > 0)
    {
        if (take_remain <= offmap_money)
        {
            dungeon->offmap_money_owned -= take_remain;
            dungeon->total_money_owned -= take_remain;
            return amount_take;
        }
        take_remain -= offmap_money;
        dungeon->total_money_owned -= offmap_money;
        dungeon->offmap_money_owned = 0;
    }
    long i = dungeon->room_kind[RoK_TREASURE];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
          ERRORLOG("Jump to invalid room detected");
          break;
        }
        i = room->next_of_owner;
        // Per-room code
        if (room->capacity_used_for_storage > 0)
        {
            take_remain -= take_money_from_room(room, take_remain);
            if (take_remain <= 0)
            {
                if (is_my_player_number(plyr_idx))
                {
                  if ((total_money >= 1000) && (total_money - amount_take < 1000)) {
                      output_message(SMsg_GoldLow, MESSAGE_DELAY_TREASURY, true);
                  }
                }
                return amount_take;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    WARNLOG("%s: Player %d could not give %d gold, %d was missing; his total gold was %d",func_name,(int)plyr_idx,(int)amount_take,(int)take_remain,(int)total_money);
    return -1;
}

long update_dungeon_generation_speeds(void)
{
    int plyr_idx;
    // Get value of generation
    int max_manage_score = 0;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        if (player_exists(player) && (player->is_active == 1))
        {
            struct Dungeon* dungeon = get_players_dungeon(player);
            if (dungeon->total_score > max_manage_score)
                max_manage_score = dungeon->manage_score;
        }
    }
    // Update the values
    for (plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        if (player_exists(player) && (player->is_active == 1))
        {
            struct Dungeon* dungeon = get_players_dungeon(player);
            if (dungeon->manage_score > 0)
                dungeon->turns_between_entrance_generation = max_manage_score * game.generate_speed / dungeon->manage_score;
            else
                dungeon->turns_between_entrance_generation = game.generate_speed;
        }
    }

    return 1;
}

void calculate_dungeon_area_scores(void)
{
    //_DK_calculate_dungeon_area_scores();
    // Zero dungeon areas
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
        if (!dungeon_invalid(dungeon))
        {
            dungeon->total_area = 0;
            dungeon->room_manage_area = 0;
        }
    }
    // Compute new values for dungeon areas
    for (MapSlabCoord slb_y = 0; slb_y < map_tiles_y; slb_y++)
    {
        for (MapSlabCoord slb_x = 0; slb_x < map_tiles_x; slb_x++)
        {
            SlabCodedCoords slb_num = get_slab_number(slb_x, slb_y);
            struct SlabMap* slb = get_slabmap_direct(slb_num);
            const struct SlabAttr* slbattr = get_slab_attrs(slb);
            if (slbattr->category == SlbAtCtg_RoomInterior)
            {
                struct Dungeon *dungeon;
                if (slabmap_owner(slb) != game.neutral_player_num) {
                    dungeon = get_players_num_dungeon(slabmap_owner(slb));
                } else {
                    dungeon = INVALID_DUNGEON;
                }
                if (!dungeon_invalid(dungeon))
                {
                    dungeon->total_area++;
                    dungeon->room_manage_area++;
                }
            } else
            if (slbattr->category == SlbAtCtg_FortifiedGround)
            {
                struct Dungeon *dungeon;
                if (slabmap_owner(slb) != game.neutral_player_num) {
                    dungeon = get_players_num_dungeon(slabmap_owner(slb));
                } else {
                    dungeon = INVALID_DUNGEON;
                }
                if (!dungeon_invalid(dungeon))
                {
                    dungeon->total_area++;
                }
            }
        }
    }
}

void init_player_music(struct PlayerInfo *player)
{
    LevelNumber lvnum = get_loaded_level_number();
    game.audiotrack = 3 + ((lvnum - 1) % 4);
}

TbBool map_position_has_sibling_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, SlabKind slbkind, PlayerNumber plyr_idx)
{
    for (int n = 0; n < SMALL_AROUND_LENGTH; n++)
    {
        int dx = small_around[n].delta_x;
        int dy = small_around[n].delta_y;
        struct SlabMap* slb = get_slabmap_block(slb_x + dx, slb_y + dy);
        if ((slb->kind == slbkind) && (slabmap_owner(slb) == plyr_idx)) {
            return true;
        }
    }
    return false;
}

TbBool map_position_initially_explored_for_player(PlayerNumber plyr_idx, MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
    struct Map* mapblk = get_map_block_at(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    // All owned ground is visible
    if (slabmap_owner(slb) == plyr_idx) {
        return true;
    }
    // All Rocks are visible
    if (slb->kind == SlbT_ROCK) {
        return true;
    }
    // Neutral entrances are visible
    struct Room* room = room_get(slb->room_index);
    if (((mapblk->flags & SlbAtFlg_IsRoom) != 0) && (room->kind == RoK_ENTRANCE) && (slabmap_owner(slb) == game.neutral_player_num)) {
        return true;
    }
    // Slabs with specific flag are visible
    if ((mapblk->flags & SlbAtFlg_Valuable) != 0) {
        return true;
    }
    // Area around entrances is visible
    if (map_position_has_sibling_slab(slb_x, slb_y, SlbT_ENTRANCE, game.neutral_player_num)) {
        return true;
    }
    return false;
}

void fill_in_explored_area(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    _DK_fill_in_explored_area(plyr_idx, stl_x, stl_y); return;
}

void init_keeper_map_exploration_by_terrain(struct PlayerInfo *player)
{
    //_DK_init_keeper_map_exploration(player); return;
    struct Thing* heartng = get_player_soul_container(player->id_number);
    if (thing_exists(heartng)) {
        fill_in_explored_area(player->id_number, heartng->mappos.x.stl.num, heartng->mappos.y.stl.num);
    }
    for (MapSlabCoord slb_y = 0; slb_y < map_tiles_y; slb_y++)
    {
        for (MapSlabCoord slb_x = 0; slb_x < map_tiles_x; slb_x++)
        {
            if (map_position_initially_explored_for_player(player->id_number, slb_x, slb_y)) {
                set_slab_explored(player->id_number, slb_x, slb_y);
            }
        }
    }
}

TbBool check_map_explored_at_current_pos(struct Thing *creatng)
{
    check_map_explored(creatng, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num);
    return true;
}

void init_keeper_map_exploration_by_creatures(struct PlayerInfo *player)
{
    do_to_players_all_creatures_of_model(player->id_number, CREATURE_ANY, check_map_explored_at_current_pos);
}

void init_player_as_single_keeper(struct PlayerInfo *player)
{
    struct InitLight ilght;
    memset(&ilght, 0, sizeof(struct InitLight));
    player->field_4CD = 0;
    ilght.radius = 2560;
    ilght.intensity = 48;
    ilght.field_3 = 5;
    ilght.is_dynamic = 1;
    unsigned short idx = light_create_light(&ilght);
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
    player->minimap_pos_x = 11;
    player->minimap_pos_y = 11;
    player->minimap_zoom = settings.minimap_zoom;
    player->field_4D1 = player->id_number;
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
    player->continue_work_state = PSt_CtrlDungeon;
    player->work_state = PSt_CtrlDungeon;
    player->field_14 = 2;
    player->main_palette = engine_palette;
    if (is_my_player(player))
    {
        set_flag_byte(&game.operation_flags,GOF_ShowPanel,true);
        set_gui_visible(true);
        init_gui();
        turn_on_menu(GMnu_MAIN);
        turn_on_menu(GMnu_ROOM);
    }
    switch (game.game_kind)
    {
    case GKind_LocalGame:
        init_player_as_single_keeper(player);
        init_player_start(player, false);
        reset_player_mode(player, PVT_DungeonTop);
        if ( !no_explore ) {
          init_keeper_map_exploration_by_terrain(player);
          init_keeper_map_exploration_by_creatures(player);
        }
        break;
    case GKind_MultiGame:
        if (player->is_active != 1)
        {
          ERRORLOG("Non Keeper in Keeper game");
          break;
        }
        init_player_as_single_keeper(player);
        init_player_start(player, false);
        reset_player_mode(player, PVT_DungeonTop);
        init_keeper_map_exploration_by_terrain(player);
        init_keeper_map_exploration_by_creatures(player);
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
    player->hand_busy_until_turn = 0;
}

void init_players(void)
{
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if ((game.packet_save_head.players_exist & (1 << i)) != 0)
            player->allocflags |= PlaF_Allocated;
        else
            player->allocflags &= ~PlaF_Allocated;
        if (player_exists(player))
        {
            player->id_number = i;
            if ((game.packet_save_head.players_comp & (1 << i)) != 0)
                player->allocflags |= PlaF_CompCtrl;
            else
                player->allocflags &= ~PlaF_CompCtrl;
            if ((player->allocflags & PlaF_CompCtrl) == 0)
            {
              game.active_players_count++;
              player->is_active = 1;
              game.game_kind = GKind_MultiGame;
              init_player(player, 0);
            }
        }
    }
}

TbBool wp_check_map_pos_valid(struct Wander *wandr, SubtlCodedCoords stl_num)
{
    SYNCDBG(16,"Starting");
    MapSubtlCoord stl_x = stl_num_decode_x(stl_num);
    MapSubtlCoord stl_y = stl_num_decode_y(stl_num);
    if (wandr->wandr_slot == CrWaS_WithinDungeon)
    {
        struct Map* mapblk = get_map_block_at_pos(stl_num);
        // Add only tiles which are revealed to the wandering player, unless it's heroes - for them, add all
        if ((wandr->plyr_idx == game.hero_player_num) || map_block_revealed(mapblk, wandr->plyr_idx))
        {
            struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
            if (((mapblk->flags & SlbAtFlg_Blocking) == 0) && ((get_navigation_map(stl_x, stl_y) & NAVMAP_UNSAFE_SURFACE) == 0)
             && players_creatures_tolerate_each_other(wandr->plyr_idx,slabmap_owner(slb)))
            {
                return true;
            }
        }
    } else
    {
        struct Map* mapblk = get_map_block_at_pos(stl_num);
        // Add only tiles which are not revealed to the wandering player, unless it's heroes - for them, do nothing
        if ((wandr->plyr_idx != game.hero_player_num) && !map_block_revealed(mapblk, wandr->plyr_idx))
        {
            if (((mapblk->flags & SlbAtFlg_Blocking) == 0) && ((get_navigation_map(stl_x, stl_y) & NAVMAP_UNSAFE_SURFACE) == 0))
            {
                struct Thing* heartng = get_player_soul_container(wandr->plyr_idx);
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
    unsigned long i = wandr->point_insert_idx;
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
        if (wandr->max_found_per_check <= 0)
            return 1;
        wandr->point_insert_idx %= WANDER_POINTS_COUNT;
        double delta = ((double)stl_num_count) / max_to_store;
        double realidx = 0.1; // A little above zero to avoid float rounding errors
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

long wander_point_initialise(struct Wander *wandr, PlayerNumber plyr_idx, unsigned char wandr_slot)
{
    wandr->wandr_slot = wandr_slot;
    wandr->plyr_idx = plyr_idx;
    wandr->point_insert_idx = 0;
    wandr->last_checked_slb_num = 0;
    wandr->plyr_bit = (1 << plyr_idx);
    wandr->num_check_per_run = 20;
    wandr->max_found_per_check = 4;
    wandr->wdrfield_14 = 0;

    long stl_num_list_count = 0;
    SubtlCodedCoords* stl_num_list = (SubtlCodedCoords*)scratch;
    SlabCodedCoords slb_num = 0;
    while (1)
    {
        MapSlabCoord slb_x = slb_num_decode_x(slb_num);
        MapSlabCoord slb_y = slb_num_decode_y(slb_num);
        SubtlCodedCoords stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
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
    SYNCDBG(6,"Starting");
    // Find up to 20 numbers (starting where we ended last time) and store them in local array
    SlabCodedCoords slb_num = wandr->last_checked_slb_num;
    long stl_num_list_count = 0;
    for (long i = 0; i < wandr->num_check_per_run; i++)
    {
        MapSlabCoord slb_x = slb_num_decode_x(slb_num);
        MapSlabCoord slb_y = slb_num_decode_y(slb_num);
        SubtlCodedCoords stl_num = get_subtile_number_at_slab_center(slb_x, slb_y);
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
    case GKind_Unknown3:
        break;
    case GKind_LocalGame:
    case GKind_MultiGame:
        wander_point_initialise(&player->wandr_within, player->id_number, CrWaS_WithinDungeon);
        wander_point_initialise(&player->wandr_outside, player->id_number, CrWaS_OutsideDungeon);
        break;
    default:
        if ((player->allocflags & PlaF_CompCtrl) == 0) {
            ERRORLOG("Invalid GameMode");
        }
        break;
    }
}

void post_init_players(void)
{
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        if ((player->allocflags & PlaF_Allocated) != 0) {
            post_init_player(player);
        }
    }
}

void init_players_local_game(void)
{
    SYNCDBG(4,"Starting");
    struct PlayerInfo* player = get_my_player();
    player->id_number = my_player_number;
    player->allocflags |= PlaF_Allocated;
    if (settings.video_rotate_mode < 1)
      player->view_mode_restore = PVM_IsometricView;
    else
      player->view_mode_restore = PVM_FrontView;
    init_player(player, 0);
}

void process_player_states(void)
{
    SYNCDBG(6,"Starting");
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
        {
            if ( (player->work_state == PSt_CreatrInfo) || (player->work_state == PSt_CreatrInfoAll) )
            {
                struct Thing* thing = thing_get(player->controlled_thing_idx);
                struct Camera* cam = player->acamera;
                if ((cam != NULL) && thing_exists(thing)) {
                    cam->mappos.x.val = thing->mappos.x.val;
                    cam->mappos.y.val = thing->mappos.y.val;
                }
            }
        }
    }
}

void process_players(void)
{
    SYNCDBG(5,"Starting");
    update_roomspaces();
    process_player_instances();
    process_player_states();
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player) && (player->is_active == 1))
        {
            SYNCDBG(6,"Doing updates for player %d",i);
            wander_point_update(&player->wandr_within);
            wander_point_update(&player->wandr_outside);
            update_power_sight_explored(player);
            update_player_objectives(i);
        }
    }
    TbBigChecksum sum = 0;
    sum += compute_players_checksum();
    sum += game.action_rand_seed;
    player_packet_checksum_add(my_player_number,sum,"players");
    SYNCDBG(17,"Finished");
}

TbBool player_sell_trap_at_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Thing *thing;
    struct Coord3d pos;
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    long sell_value = 0;
    if (is_key_pressed(KC_LALT, KMod_DONTCARE))
    {
        thing = get_trap_for_position(stl_x, stl_y);
        if (thing_is_invalid(thing))
        {
            return false;
        }
        set_coords_to_subtile_center(&pos,stl_x,stl_y,1);
        remove_trap_on_subtile(stl_x, stl_y, &sell_value);
    }
    else
    {
        thing = get_trap_for_slab_position(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
        if (thing_is_invalid(thing))
        {
            return false;
        }
        set_coords_to_slab_center(&pos,slb_x,slb_y);
        remove_traps_around_subtile(slab_subtile_center(slb_x), slab_subtile_center(slb_y), &sell_value);
    }

	struct DungeonAdd* dungeonadd = get_dungeonadd(thing->owner);
	dungeonadd->traps_sold++;
	dungeonadd->manufacture_gold += sell_value;

    struct Dungeon* dungeon = get_players_num_dungeon(thing->owner);
    if (is_my_player_number(plyr_idx))
    {
        play_non_3d_sample(115);
    }
    dungeon->camera_deviate_jump = 192;
    if (sell_value != 0)
    {
        create_price_effect(&pos, plyr_idx, sell_value);
        player_add_offmap_gold(plyr_idx,sell_value);
    } else
    {
        WARNLOG("Sold traps at (%d,%d) which didn't cost anything",(int)stl_x,(int)stl_y);
    }
    // Add the trap location to related computer player, in case we'll want to place a trap again
    struct Computer2* comp = get_computer_player(plyr_idx);
    if (!computer_player_invalid(comp))
    {
        add_to_trap_location(comp, &pos);
    }
    return true;
}

TbBool player_sell_door_at_subtile(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    MapSubtlCoord cstl_x = stl_slab_center_subtile(stl_x);
    MapSubtlCoord cstl_y = stl_slab_center_subtile(stl_y);
    struct Thing* thing = get_door_for_position(cstl_x, cstl_y);
    if (thing_is_invalid(thing))
    {
        return false;
    }

	struct Dungeon* dungeon = get_players_num_dungeon(thing->owner);
	dungeon->camera_deviate_jump = 192;
    long sell_value = compute_value_percentage(gameadd.doors_config[thing->model].selling_value, gameadd.door_sale_percent);

	struct DungeonAdd* dungeonadd = get_dungeonadd(thing->owner);
	dungeonadd->doors_sold++;
	dungeonadd->manufacture_gold += sell_value;

    destroy_door(thing);
    if (is_my_player_number(plyr_idx))
        play_non_3d_sample(115);
    struct Coord3d pos;
    set_coords_to_slab_center(&pos,subtile_slab_fast(stl_x),subtile_slab_fast(stl_y));
    if (sell_value != 0)
    {
        create_price_effect(&pos, plyr_idx, sell_value);
        player_add_offmap_gold(plyr_idx, sell_value);
    }
    { // Add the trap location to related computer player, in case we'll want to place a trap again
        struct Computer2* comp = get_computer_player(plyr_idx);
        if (!computer_player_invalid(comp)) {
            add_to_trap_location(comp, &pos);
        }
    }
    return true;
}

void compute_and_update_player_payday_total(PlayerNumber plyr_idx)
{
    SYNCDBG(15,"Starting for player %d",(int)plyr_idx);
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    dungeon->creatures_total_pay = compute_player_payday_total(dungeon);
}

PlayerNumber get_selected_player_for_cheat(PlayerNumber defplayer)
{
        if (is_key_pressed(KC_NUMPAD0, KMod_DONTCARE))
        {
            return 0;
        }
        else if (is_key_pressed(KC_NUMPAD1, KMod_DONTCARE))
        {
            return 1;
        }
        else if (is_key_pressed(KC_NUMPAD2, KMod_DONTCARE))
        {
            return 2;
        }
        else if (is_key_pressed(KC_NUMPAD3, KMod_DONTCARE))
        {
            return 3;
        }
        else if (is_key_pressed(KC_NUMPAD4, KMod_DONTCARE))
        {
            return game.hero_player_num;
        }
        else if (is_key_pressed(KC_NUMPAD5, KMod_DONTCARE))
        {
            return game.neutral_player_num;
        }
        else
        {
            return defplayer;
        }
}
/******************************************************************************/
