#include "keeperfx.hpp"

#include "bflib_coroutine.h"
#include "bflib_datetm.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_sound.h"

#include "config_compp.h"
#include "dungeon_data.h"
#include "engine_lenses.h"
#include "engine_redraw.h"
#include "engine_textures.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "game_heap.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "gui_topmsg.h"
#include "gui_soundmsgs.h"
#include "kjm_input.h"
#include "lvl_filesdk1.h"
#include "net_sync.h"
#include "room_library.h"
#include "room_list.h"
#include "power_specials.h"
#include "player_utils.h"
#include "vidfade.h"
#include "vidmode.h"
#include "custom_sprites.h"

extern TbBool force_player_num;

extern void setup_players_count();

CoroutineLoopState set_not_has_quit(CoroutineLoop *context);

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

void init_good_player_as(PlayerNumber plr_idx)
{
    struct PlayerInfo *player;
    game.hero_player_num = plr_idx;
    player = get_player(plr_idx);
    player->allocflags |= PlaF_Allocated;
    player->allocflags |= PlaF_CompCtrl;
    player->id_number = game.hero_player_num;
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
    //_DK_init_level(); return;
    //LbMemoryCopy(&transfer_mem,&game.intralvl.transferred_creature,sizeof(struct CreatureStorage));
    LbMemoryCopy(&transfer_mem,&intralvl,sizeof(struct IntralevelData));
    game.flags_gui = GGUI_SoloChatEnabled;
    set_flag_byte(&game.system_flags, GSF_RunAfterVictory, false);
    game.action_rand_seed = 1;
    free_swipe_graphic();
    game.loaded_swipe_idx = -1;
    game.play_gameturn = 0;
    game_flags2 &= (GF2_PERSISTENT_FLAGS | GF2_Timer);
    clear_game();
    reset_heap_manager();
    lens_mode = 0;
    setup_heap_manager();

    // Load configs which may have per-campaign part, and can even be modified within a level
    load_computer_player_config(CnfLd_Standard);
    init_custom_sprites(get_selected_level_number());
    load_stats_files();
    check_and_auto_fix_stats();

    // We should do this after 'load stats'
    update_room_tab_to_config();
    update_powers_tab_to_config();
    update_trap_tab_to_config();

    init_creature_scores();

    init_good_player_as(hero_player_number);
    light_set_lights_on(1);
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];

    erstats_clear();
    init_dungeons();
    // Load the actual level files
    preload_script(get_selected_level_number());
    load_map_file(get_selected_level_number());

    init_navigation();
    clear_messages();
    LbStringCopy(game.campaign_fname,campaign.fname,sizeof(game.campaign_fname));
#ifdef AUTOTESTING
    if (start_params.autotest_flags & ATF_FixedSeed)
    {
      game.action_rand_seed = 1;
      game.unsync_rand_seed = 1;
      srand(1);
    }
    else
#else
    // Initialize unsynchronized random seed (the value may be different
    // on computers in MP, as it shouldn't affect game actions)
    game.unsync_rand_seed = (unsigned long)LbTimeSec();
#endif
    if (!SoundDisabled)
    {
        game.field_14BB54 = (UNSYNC_RANDOM(67) % 3 + 1);
        game.field_14BB55 = 0;
    }
    light_set_lights_on(1);
    {
        struct PlayerInfo *player;
        player = get_player(game.hero_player_num);
        init_player_start(player, false);
    }
    game.numfield_D |= GNFldD_Unkn04;
    //LbMemoryCopy(&game.intralvl.transferred_creature,&transfer_mem,sizeof(struct CreatureStorage));
    LbMemoryCopy(&intralvl,&transfer_mem,sizeof(struct IntralevelData));
    event_initialise_all();
    battle_initialise();
    ambient_sound_prepare();
    zero_messages();
    game.armageddon_cast_turn = 0;
    game.armageddon_field_15035A = 0;
    init_messages();
    game.creatures_tend_imprison = 0;
    game.creatures_tend_flee = 0;
    game.pay_day_progress = 0;
    game.chosen_room_kind = 0;
    game.chosen_room_spridx = 0;
    game.chosen_room_tooltip = 0;
    set_chosen_power_none();
    game.manufactr_element = 0;
    game.manufactr_spridx = 0;
    game.manufactr_tooltip = 0;
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
    init_dungeons_research();
    init_dungeons_essential_position();
    if (!is_map_pack())
    {
        create_transferred_creature_on_level();
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
    SYNCMSG("Packet Loading Active (File contains %d turns)", game.turns_stored);
    SYNCMSG("Packet Checksum Verification %s",game.packet_checksum_verify ? "Enabled" : "Disabled");
    SYNCMSG("Fast Forward through %d game turns", game.turns_fastforward);
    if (game.turns_packetoff != -1)
        SYNCMSG("Packet Quit at %d", game.turns_packetoff);
    if (game.packet_load_enable)
    {
      if (game.log_things_end_turn != game.log_things_start_turn)
        SYNCMSG("Logging things, game turns %d -> %d", game.log_things_start_turn, game.log_things_end_turn);
    }
    SYNCMSG("Packet file prepared on KeeperFX %d.%d.%d.%d",(int)game.packet_save_head.game_ver_major,(int)game.packet_save_head.game_ver_minor,
        (int)game.packet_save_head.game_ver_release,(int)game.packet_save_head.game_ver_build);
#endif
    if ((game.packet_save_head.game_ver_major != VER_MAJOR) || (game.packet_save_head.game_ver_minor != VER_MINOR)
        || (game.packet_save_head.game_ver_release != VER_RELEASE) || (game.packet_save_head.game_ver_build != VER_BUILD)) {
        WARNLOG("Packet file was created with different version of the game; this rarely works");
    }
    game.game_kind = GKind_LocalGame;
    if (!(game.packet_save_head.players_exist & (1 << game.local_plyr_idx))
        || (game.packet_save_head.players_comp & (1 << game.local_plyr_idx)))
        my_player_number = 0;
    else
        my_player_number = game.local_plyr_idx;
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
    if (is_key_pressed(KC_LALT, KMod_NONE))
    {
        struct PlayerInfo* player = get_my_player();
        set_engine_view(player, PVM_FrontView);
    }
}

static CoroutineLoopState startup_network_game_tail(CoroutineLoop *context);

void startup_network_game(CoroutineLoop *context, TbBool local)
{
    SYNCDBG(0,"Starting up network game");
    //_DK_startup_network_game(); return;
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
    get_my_player()->flgfield_6 &= ~PlaF6_PlyrHasQuit;
    return CLS_CONTINUE;
}

void faststartup_saved_packet_game(void)
{
    reenter_video_mode();
    startup_saved_packet_game();
    {
        struct PlayerInfo *player;
        player = get_my_player();
        player->flgfield_6 &= ~PlaF6_PlyrHasQuit;
    }
    set_gui_visible(false);
    set_flag_byte(&game.operation_flags,GOF_ShowPanel,false);
}

/******************************************************************************/

/**
 * Clears the Game structure completely, and copies startup parameters
 * from start_params structure.
 */
void clear_complete_game(void)
{
    memset(&game, 0, sizeof(struct Game));
    memset(&gameadd, 0, sizeof(struct GameAdd));
    memset(&intralvl, 0, sizeof(struct IntralevelData));
    game.turns_packetoff = -1;
    game.local_plyr_idx = default_loc_player;
    game.packet_checksum_verify = start_params.packet_checksum_verify;
    game.numfield_1503A2 = -1;
    game.flags_font = start_params.flags_font;
    game.numfield_149F47 = 0;
    // Set levels to 0, as we may not have the campaign loaded yet
    set_continue_level_number(first_singleplayer_level());
    if ((start_params.operation_flags & GOF_SingleLevel) != 0)
        set_selected_level_number(start_params.selected_level_number);
    else
        set_selected_level_number(first_singleplayer_level());
    game.num_fps = start_params.num_fps;
    game.flags_cd = start_params.flags_cd;
    game.no_intro = start_params.no_intro;
    set_flag_byte(&game.system_flags,GSF_AllowOnePlayer,start_params.one_player);
    gameadd.computer_chat_flags = start_params.computer_chat_flags;
    game.operation_flags = start_params.operation_flags;
    strncpy(game.packet_fname,start_params.packet_fname,150);
    game.packet_save_enable = start_params.packet_save_enable;
    game.packet_load_enable = start_params.packet_load_enable;
    my_player_number = default_loc_player;
}
