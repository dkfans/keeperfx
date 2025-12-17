/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file main_game.c
 * @author KeeperFX Team
 * @date 24 Sep 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "keeperfx.hpp"

#include "bflib_coroutine.h"
#include "bflib_datetm.h"
#include "bflib_math.h"
#include "bflib_sound.h"

#include "config_compp.h"
#include "config_settings.h"
#include "creature_states_combt.h"
#include "dungeon_data.h"
#include "engine_lenses.h"
#include "engine_redraw.h"
#include "engine_textures.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_map.h"
#include "game_heap.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "kjm_input.h"
#include "lvl_filesdk1.h"
#include "lua_base.h"
#include "lua_triggers.h"
#include "net_resync.h"
#include "room_library.h"
#include "room_list.h"
#include "power_specials.h"
#include "player_data.h"
#include "player_instances.h"
#include "player_utils.h"
#include "vidfade.h"
#include "vidmode.h"
#include "custom_sprites.h"
#include "gui_boxmenu.h"
#include "sounds.h"
#include "api.h"
#include "net_resync.h"

#ifdef FUNCTESTING
  #include "ftests/ftest.h"
#endif

#include "post_inc.h"

extern TbBool force_player_num;
extern TbBool IMPRISON_BUTTON_DEFAULT;
extern TbBool FLEE_BUTTON_DEFAULT;
extern unsigned long features_enabled;

extern void setup_players_count();
extern void set_skip_heart_zoom_feature(TbBool enable);

CoroutineLoopState set_not_has_quit(CoroutineLoop *context);
TbBool luascript_loaded = false;
/**
 * Resets timers and flags of all players into default (zeroed) state.
 * Also enables spells which are always enabled by default.
 */
void reset_script_timers_and_flags(void)
{
    struct Dungeon *dungeon;
    int plyr_idx;
    int k;
    TbBool freeplay = is_map_pack();
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        add_power_to_player(PwrK_HAND, plyr_idx);
        add_power_to_player(PwrK_SLAP, plyr_idx);
        add_power_to_player(PwrK_POSSESS, plyr_idx);
        dungeon = get_dungeon(plyr_idx);
        for (k=0; k<TURN_TIMERS_COUNT; k++)
        {
            memset(&dungeon->turn_timers[k], 0, sizeof(struct TurnTimer));
            dungeon->turn_timers[k].state = 0;
        }
        for (k=0; k<SCRIPT_FLAGS_COUNT; k++)
        {
            dungeon->script_flags[k] = 0;
            if (freeplay)
            {
                intralvl.campaign_flags[plyr_idx][k] = 0;
            }
        }
    }
}

void init_player_types()
{
    for (size_t plr_idx = 0; plr_idx < PLAYERS_COUNT; plr_idx++)
    {
        struct PlayerInfo *player;
        player = get_player(plr_idx);
        switch (plr_idx)
        {
        case PLAYER_GOOD:
            player->allocflags |= PlaF_Allocated;
            player->allocflags |= PlaF_CompCtrl;
            player->player_type = PT_Roaming;
            player->id_number = plr_idx;
            break;
        case PLAYER_NEUTRAL:
            player->player_type = PT_Neutral;
            break;
        default:
            player->player_type = PT_Keeper;
            break;
        }
    }
}

/******************************************************************************/
void init_lookups(void)
{
    long i;
    SYNCDBG(8,"Starting");
    for (i=0; i < THINGS_COUNT; i++)
    {
        game.things.lookup[i] = &game.things_data[i];
    }
    game.things.end = &game.things_data[THINGS_COUNT];

    memset(&game.persons, 0, sizeof(struct Persons));
    for (i=0; i < CREATURES_COUNT; i++)
    {
        game.persons.cctrl_lookup[i] = &game.cctrl_data[i];
    }
    game.persons.cctrl_end = &game.cctrl_data[CREATURES_COUNT];

    for (i=0; i < COLUMNS_COUNT; i++)
    {
        game.columns.lookup[i] = &game.columns_data[i];
    }
    game.columns.end = &game.columns_data[COLUMNS_COUNT];
}

static void init_level(void)
{
    SYNCDBG(6,"Starting");
    struct IntralevelData transfer_mem;
    //memcpy(&transfer_mem,&game.intralvl.transferred_creature,sizeof(struct CreatureStorage));
    memcpy(&transfer_mem,&intralvl,sizeof(struct IntralevelData));
    game.flags_gui = GGUI_SoloChatEnabled;
    clear_flag(game.system_flags, GSF_RunAfterVictory);
    free_swipe_graphic();
    game.loaded_swipe_idx = -1;
    game.play_gameturn = 0;
    game.paused_at_gameturn = false;
    game_flags2 &= (GF2_PERSISTENT_FLAGS | GF2_Timer);
    clear_game();
    reset_heap_manager();
    lens_mode = 0;
    setup_heap_manager();

    luascript_loaded = open_lua_script(get_selected_level_number());
    // Load configs which may have per-campaign part, and can even be modified within a level
    recheck_all_mod_exist();
    init_custom_sprites(get_selected_level_number());
    load_stats_files();
    check_and_auto_fix_stats();

    // We should do this after 'load stats'
    update_room_tab_to_config();
    update_powers_tab_to_config();
    update_trap_tab_to_config();

    init_creature_scores();

    init_player_types();
    light_set_lights_on(1);
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];

    erstats_clear();
    init_dungeons();
    setup_panel_colors();
    init_map_size(get_selected_level_number());
    clear_messages();
    init_seeds();
    
    sync_various_data();
    
    // Load the actual level files
    TbBool script_preloaded = preload_script(get_selected_level_number());
    if (!load_map_file(get_selected_level_number()))
    {
        // TODO: whine about missing file to screen
        JUSTMSG("Unable to load level %lu from %s", get_selected_level_number(), campaign.name);
        return;
    }
    else
    {
        if (script_preloaded == false && luascript_loaded == false)
        {
            show_onscreen_msg(200,"%s: No Script %lu", get_string(GUIStr_Error), get_selected_level_number());
            JUSTMSG("Unable to load script level %lu from %s", get_selected_level_number(), campaign.name);
        }
    }
    init_navigation();
    snprintf(game.campaign_fname, sizeof(game.campaign_fname), "%s", campaign.fname);
    light_set_lights_on(1);
    {
        for (size_t i = 0; i < PLAYERS_COUNT; i++)
        {
            if(player_is_roaming(i))
            {
                struct PlayerInfo *player;
                player = get_player(i);
                init_player_start(player, false);
            }
        }
    }
    game.view_mode_flags |= GNFldD_ComputerPlayerProcessing;
    //memcpy(&game.intralvl.transferred_creature,&transfer_mem,sizeof(struct CreatureStorage));
    memcpy(&intralvl,&transfer_mem,sizeof(struct IntralevelData));
    event_initialise_all();
    battle_initialise();
    ambient_sound_prepare();
    zero_messages();
    game.armageddon_cast_turn = 0;
    game.armageddon_over_turn = 0;
    clear_messages();
    game.creatures_tend_imprison = 0;
    game.creatures_tend_flee = 0;
    memset(game.pay_day_progress, 0, sizeof(game.pay_day_progress));
    game.chosen_room_kind = 0;
    game.chosen_room_spridx = 0;
    game.chosen_room_tooltip = 0;
    set_chosen_power_none();
    game.manufactr_element = 0;
    game.manufactr_spridx = 0;
    game.manufactr_tooltip = 0;
    reset_postal_instance_cache();
    JUSTMSG("Started level %lu from %s", get_selected_level_number(), campaign.name);

    api_event("GAME_STARTED");
}

static void post_init_level(void)
{
    SYNCDBG(8,"Starting");
    if (game.packet_save_enable)
        open_new_packet_file_for_save();
    calculate_dungeon_area_scores();
    init_animating_texture_maps();
    reset_creature_max_levels();
    clear_creature_pool();
    setup_computer_players2();
    load_script(get_loaded_level_number());
    lua_on_game_start();
    init_dungeons_research();
    init_dungeons_essential_position();
    if (!is_map_pack())
    {
        create_transferred_creatures_on_level();
    }
    update_dungeons_scores();
    update_dungeon_generation_speeds();
    init_traps();
    init_all_creature_states();
    init_keepers_map_exploration();
    SYNCDBG(9,"Finished");
}

/******************************************************************************/

void startup_saved_packet_game(void)
{
    struct CatalogueEntry centry;
    clear_packets();
    open_packet_file_for_load(game.packet_fname,&centry);
    if (!change_campaign(centry.campaign_fname))
    {
        ERRORLOG("Unable to load campaign associated with packet file");
    }
    set_selected_level_number(game.packet_save_head.level_num);
    lbDisplay.DrawColour = colours[15][15][15];
    game.pckt_gameturn = 0;
#if (BFDEBUG_LEVEL > 0)
    SYNCDBG(0,"Initialising level %d", (int)get_selected_level_number());
    SYNCMSG("Packet Loading Active (File contains %lu turns)", game.turns_stored);
    SYNCMSG("Packet Checksum Verification %s",game.packet_checksum_verify ? "Enabled" : "Disabled");
    SYNCMSG("Fast Forward through %lu game turns", game.turns_fastforward);
    if (game.turns_packetoff != -1)
        SYNCMSG("Packet Quit at %lu", game.turns_packetoff);
    if (game.packet_load_enable)
    {
      if (game.log_things_end_turn != game.log_things_start_turn)
        SYNCMSG("Logging things, game turns %lu -> %lu", game.log_things_start_turn, game.log_things_end_turn);
    }
    SYNCMSG("Packet file prepared on KeeperFX %d.%d.%d.%d",(int)game.packet_save_head.game_ver_major,(int)game.packet_save_head.game_ver_minor,
        (int)game.packet_save_head.game_ver_release,(int)game.packet_save_head.game_ver_build);
#endif
    if ((game.packet_save_head.game_ver_major != VER_MAJOR) || (game.packet_save_head.game_ver_minor != VER_MINOR)
        || (game.packet_save_head.game_ver_release != VER_RELEASE) || (game.packet_save_head.game_ver_build != VER_BUILD)) {
        WARNLOG("Packet file was created with different version of the game; this rarely works");
    }
    game.game_kind = GKind_LocalGame;
    if (!flag_is_set(game.packet_save_head.players_exist, to_flag(game.local_plyr_idx))
        || flag_is_set(game.packet_save_head.players_comp, to_flag(game.local_plyr_idx)))
        my_player_number = 0;
    else
        my_player_number = game.local_plyr_idx;
    settings.isometric_view_zoom_level = game.packet_save_head.isometric_view_zoom_level;
    settings.frontview_zoom_level = game.packet_save_head.frontview_zoom_level;
    settings.isometric_tilt = game.packet_save_head.isometric_tilt;
    settings.highlight_mode = game.packet_save_head.highlight_mode;
    IMPRISON_BUTTON_DEFAULT = game.packet_save_head.default_imprison_tendency;
    FLEE_BUTTON_DEFAULT = game.packet_save_head.default_flee_tendency;
    set_skip_heart_zoom_feature(game.packet_save_head.skip_heart_zoom);
    init_level();
    setup_zombie_players();//TODO GUI What about packet file from network game? No zombies there..
    init_players();
    if (game.active_players_count == 1)
        game.game_kind = GKind_LocalGame;
    if (game.turns_stored < game.turns_fastforward)
        game.turns_fastforward = game.turns_stored;
    post_init_level();
    post_init_players();
    set_selected_level_number(0);
    struct PlayerInfo* player = get_my_player();
    set_engine_view(player, rotate_mode_to_view_mode(game.packet_save_head.video_rotate_mode));
}

static CoroutineLoopState startup_network_game_tail(CoroutineLoop *context);

void startup_network_game(CoroutineLoop *context, TbBool local)
{
    SYNCDBG(0,"Starting up network game");
    stop_streamed_samples();
    unsigned int flgmem;
    struct PlayerInfo *player;
    setup_count_players();
    player = get_my_player();
    flgmem = player->is_active;
    if (local && (campaign.human_player >= 0) && (!force_player_num))
    {
        default_loc_player = campaign.human_player;
        game.local_plyr_idx = default_loc_player;
        my_player_number = default_loc_player;
    }
    init_level();
    player = get_my_player();
    player->is_active = flgmem;
    //if (game.flagfield_14EA4A == 2) //was wrong because init_level sets this to 2. global variables are evil (though perhaps that's why they were chosen for DK? ;-))
    TbBool ShouldAssignCpuKeepers = 0;
    if (local)
    {
        game.game_kind = GKind_LocalGame;
        init_players_local_game();
        if (AssignCpuKeepers || campaign.assignCpuKeepers) {
            ShouldAssignCpuKeepers = 1;
        }
    } else
    {
        game.game_kind = GKind_MultiGame;
        init_players_network_game(context);

        // Fix desyncs when two players have a different zoom distance cfg setting
        // This temporary solution just disregards their cfg value and sets it here
        int max_zoom_in_multiplayer = 60;
        zoom_distance_setting = LbLerp(4100, CAMERA_ZOOM_MIN, (float)max_zoom_in_multiplayer/100.0);
        frontview_zoom_distance_setting = LbLerp(16384, FRONTVIEW_CAMERA_ZOOM_MIN, (float)max_zoom_in_multiplayer/100.0);
    }
    setup_count_players(); // It is reset by init_level
    int args[COROUTINE_ARGS] = {ShouldAssignCpuKeepers, 0};
    coroutine_add_args(context, &startup_network_game_tail, args);
}

static CoroutineLoopState startup_network_game_tail(CoroutineLoop *context)
{
    TbBool ShouldAssignCpuKeepers = coroutine_args(context)[0];
    if (fe_computer_players || ShouldAssignCpuKeepers)
    {
        SYNCDBG(5,"Setting up uninitialized players as computer players");
        setup_computer_players();
    } else
    {
        SYNCDBG(5,"Setting up uninitialized players as zombie players");
        setup_zombie_players();
    }
    post_init_level();
    post_init_players();
    post_init_packets();
    set_selected_level_number(0);

#ifdef FUNCTESTING
    set_flag(start_params.functest_flags, FTF_LevelLoaded);
#endif

    return CLS_CONTINUE;
}

/******************************************************************************/

void faststartup_network_game(CoroutineLoop *context)
{
    struct PlayerInfo *player;
    SYNCDBG(3,"Starting");
    reenter_video_mode();
    my_player_number = default_loc_player;
    game.game_kind = GKind_LocalGame;
    if (!is_campaign_loaded())
    {
        if (!change_campaign(""))
            ERRORLOG("Unable to load campaign");
    }
    player = get_my_player();
    player->is_active = 1;
    startup_network_game(context, true);
    coroutine_add(context, &set_not_has_quit);
}

CoroutineLoopState set_not_has_quit(CoroutineLoop *context)
{
    get_my_player()->display_flags &= ~PlaF6_PlyrHasQuit;
    return CLS_CONTINUE;
}

void faststartup_saved_packet_game(void)
{
    reenter_video_mode();
    startup_saved_packet_game();
    {
        struct PlayerInfo *player;
        player = get_my_player();
        player->display_flags &= ~PlaF6_PlyrHasQuit;
    }
    set_gui_visible(false);
    clear_flag(game.operation_flags, GOF_ShowPanel);
}

/******************************************************************************/

/**
 * Clears the Game structure completely, and copies startup parameters
 * from start_params structure.
 */
void clear_complete_game(void)
{
    memset(&game, 0, sizeof(struct Game));
    memset(&intralvl, 0, sizeof(struct IntralevelData));
    game.turns_packetoff = -1;
    game.local_plyr_idx = default_loc_player;
    game.packet_checksum_verify = start_params.packet_checksum_verify;
    game.packet_load_initialized = 0;
    // Set levels to 0, as we may not have the campaign loaded yet
    set_continue_level_number(first_singleplayer_level());
    if ((start_params.operation_flags & GOF_SingleLevel) != 0)
        set_selected_level_number(start_params.selected_level_number);
    else
        set_selected_level_number(first_singleplayer_level());
    game_num_fps = start_params.num_fps;
    game_num_fps_draw_current = 0;
    game_num_fps_draw_main = start_params.num_fps_draw_main;
    game_num_fps_draw_secondary = start_params.num_fps_draw_secondary;
    game.mode_flags = start_params.mode_flags;
    game.easter_eggs_enabled = start_params.easter_egg;
    set_flag_value(game.system_flags, GSF_AllowOnePlayer, start_params.one_player);
    game.computer_chat_flags = start_params.computer_chat_flags;
    game.operation_flags = start_params.operation_flags;
    snprintf(game.packet_fname,150, "%s", start_params.packet_fname);
    game.packet_save_enable = start_params.packet_save_enable;
    game.packet_load_enable = start_params.packet_load_enable;
    my_player_number = default_loc_player;
}

void init_seeds()
{
#if FUNCTESTING
    if (flag_is_set(start_params.functest_flags, FTF_Enabled))
    {
        ftest_srand();
    }
    else
#endif
    {
        // Unsynced seeds - these values will be different per-player in multiplayer
        unsigned long calender_time = (unsigned long)LbTimeSec();
        game.unsync_random_seed = calender_time * 9007 + 9011;  // Use prime multipliers for different seeds
        game.sound_random_seed = calender_time * 7919 + 7927;

        // If doing -packetload then use the replay's stored seed
        if ((game.packet_save_head.action_seed != 0) && (game.packet_load_enable == true)) {
            game.action_random_seed = game.packet_save_head.action_seed;
        } else {
            game.action_random_seed = calender_time * 9311 + 9319;
        }

        game.ai_random_seed = game.action_random_seed * 9377 + 9391;
        game.player_random_seed = game.action_random_seed * 9473 + 9479;
        
        initial_replay_seed = game.action_random_seed;
        lua_set_random_seed(game.action_random_seed);
    }
}
