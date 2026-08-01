/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file game_loop.c
 *     Module which contains functions from the main game loop.
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
#include "room_workshop.h"
#include "map_columns.h"
#include "creature_states.h"
#include "magic_powers.h"
#include "game_merge.h"
#include "sounds.h"
#include "game_legacy.h"
#include "game_loop.h"
#include "lua_triggers.h"


#include "kjm_input.h"
#include "moonphase.h"
#include "vidmode.h"
#include "frontend.h"
#include "bflib_mouse.h"
#include "front_simple.h"
#include "bflib_datetm.h"
#include "bflib_inputctrl.h"
#include "bflib_sndlib.h"
#include "vidfade.h"
#include "config_keeperfx.h"
#include "api.h"
#include "player_instances.h"
#include "lvl_filesdk1.h"
#include "config_sounds.h"
#include "lens_api.h"
#include "scrcapt.h"
#include "frontmenu_ingame_evnt.h"
#include "engine_redraw.h"
#include "bflib_crash.h"
#include "gui_topmsg.h"
#include "front_easter.h"
#include "front_input.h"
#include "net_exchange_gameplay.h"
#include "timer.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void startup_network_game(CoroutineLoop *context, TbBool local);
extern void faststartup_network_game(CoroutineLoop *context);
extern CoroutineLoopState set_not_has_quit(CoroutineLoop *context);
void update_frontend_delta_time();
void update_gameplay_delta_time();
bool use_delta_time();

/******************************************************************************/
static short do_draw;
static long double average_frame_draw_time = 1;

float interpolate_time = 0;

extern int32_t fps_limit_secondary;
extern long double process_frame_time;
extern long double time_since_last_draw;
extern long double multiplayer_clock_adjust;
extern long double host_packet_received;


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
    if (!S3DEmitterIsPlayingSample(breaktng->snd_emitter_id, objst->effect.sound_idx)) {
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
    if (!no_backup)
    {
        struct Thing* backup = thing_get(dungeon->backup_heart_idx);
        if (!thing_is_dungeon_heart(backup))
        {
            ERRORLOG("%s had invalid backup heart %s during heart destruction", player_code_name(plyr_idx), thing_model_name(backup));
            dungeon->backup_heart_idx = 0;
            backup = find_players_backup_dungeon_heart(dungeon->owner);
            if (thing_is_dungeon_heart(backup))
            {
                dungeon->backup_heart_idx = backup->index;
            }
            else
            {
                no_backup = true;
            }
        }
    }
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
                destroy_object(soultng);
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
        if ((dungeon->num_things_in_hand > 0) && ((game.conf.rules[plyr_idx].gameplay.classic_bugs_flags & ClscBug_NoHandPurgeOnDefeat) == 0))
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
        destroy_object(heartng);
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
                    process_objective(objective,dungeon->owner, game.heart_lost_message_target, 0, 0);
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





// Using Alt-F4, or similar operating system close requests
static void force_application_close()
{
    extern int frontend_menu_state;

    if (frontend_menu_state == 0)
    {
        struct PlayerInfo* player = get_my_player();
        if (player != INVALID_PLAYER)
        {
            set_players_packet_action(player, PckA_ForceApplicationClose, 0, 0, 0, 0);
        }
        else
        {
            exit_keeper = 1;
        }
    }
    else
    {
        exit_keeper = 1;
    }
}

static bool keeper_wait_for_screen_focus()
{
    do {
        if ( !poll_inputs() )
        {
          force_application_close();
          break;
        }
        if (LbIsActive())
          return true;
        if (network_is_active())
          return true;
        if (!freeze_game_on_focus_lost())
          return true;
        LbSleepFor(50);
        update_gameplay_delta_time();
        game.process_turn_time = 1.0;
        time_since_last_draw = 1.0;
    } while ((!exit_keeper) && (!quit_game));
    return false;
}

static void find_frame_rate(void)
{
    static TbClockMSec prev_time2=0;
    static TbClockMSec cntr_time2=0;
    unsigned long curr_time;
    curr_time = LbTimerClock();
    cntr_time2++;
    if (curr_time-prev_time2 >= 1000)
    {
        double time_fdelta = 1000.0*((double)(cntr_time2))/(curr_time-prev_time2);
        prev_time2 = curr_time;
        game.time_delta = (unsigned long)(time_fdelta*256.0);
        cntr_time2 = 0;
    }
}

static void packet_load_find_frame_rate(unsigned long incr)
{
    static TbClockMSec start_time=0;
    static TbClockMSec extra_frames=0;
    TbClockMSec curr_time;
    curr_time = LbTimerClock();
    if ((curr_time-start_time) < 5000)
    {
        extra_frames += incr;
    } else
    {
        double time_fdelta = 1000.0*((double)(extra_frames+incr))/(curr_time-start_time);
        start_time = curr_time;
        game.time_delta = (unsigned long)(time_fdelta*256.0);
        extra_frames = 0;
    }
}

/**
 * Checks if the game screen needs redrawing.
 */
static short display_should_be_updated_this_turn(void)
{
    if ((game.operation_flags & GOF_Paused) != 0)
      return true;
    if ( (game.turns_fastforward == 0) && (!game.packet_loading_in_progress) )
    {
      find_frame_rate();
      if ( (game.frame_skip == 0) || ((get_gameturn() % game.frame_skip) == 0) )
        return true;
    } else
    if ( ((get_gameturn() & 0x3F)==0) ||
         ((game.packet_loading_in_progress) && ((get_gameturn() & 7)==0)) )
    {
      packet_load_find_frame_rate(64);
      return true;
    }
    return false;
}

// this one isn't static for now, because it's used in the network code
// if networking had its own thread, it wouldn't need the yield that calls this function, but for now it does
void gameplay_loop_draw()
{
    if (use_delta_time())
        do_draw = true;

    update_gameplay_delta_time();

    if (game.process_turn_time > 1.0 && time_since_last_draw < 1.0)
        do_draw = false;

    // Frame rate limiter
    if (fps_limit_current > 0)
    {
        frametime_start_measurement(Frametime_Sleep);
        if (process_frame_time < 1.0)
        {
            if (game.process_turn_time < 1.0)
                SDL_Delay(1);
            do_draw = false;
        }
        else
        {
            process_frame_time = min(1.L, process_frame_time - 1.L);
        }
        frametime_end_measurement(Frametime_Sleep);
    }

    // Floats are used a lot in the drawing related functions. But keep in mind integers are typically preferred for logic related functions.
    frametime_start_measurement(Frametime_Draw);

    // Update lights
    update_light_render_area();

    if (quit_game || exit_keeper) {
        do_draw = false;
    }
    if ( do_draw ) {
        if (frametime_enabled())
            framerate_measurement_capture(Framerate_Draw);
        game.delta_time = min(time_since_last_draw, 1.L);
        time_since_last_draw = 0;
        interpolate_time = min(max(game.process_turn_time, 0.L), 1.L);
        keeper_screen_redraw();
    }
    keeper_wait_for_screen_focus();
    // Direct information/error messages
    if (LbScreenLock() == Lb_SUCCESS) {
        if ( do_draw ) {
            perform_any_screen_capturing();
        }
        draw_onscreen_direct_messages();
        LbScreenUnlock();
    }
    // Move the graphics window to center of screen buffer and swap screen
    if ( do_draw ) {
        LbScreenSwap();
    }
    frametime_end_measurement(Frametime_Draw);

    if ( do_draw ) {
        update_gameplay_delta_time();
        const long double delta = time_since_last_draw - average_frame_draw_time;
        average_frame_draw_time += delta * max(average_frame_draw_time, .05L) / 20;
    }
}

static void gameplay_loop_logic()
{
    if(flag_is_set(start_params.debug_flags, DFlg_PauseAtGameTurn))
    {
        static GameTurn previous_gameturn = 0;
        if(get_gameturn() >= start_params.pause_at_gameturn && get_gameturn() != previous_gameturn)
        {
            if(!game.paused_at_gameturn)
            {
                game.paused_at_gameturn = true;

                game.frame_skip = 0;
                if(game.packet_load_enable)
                {
                    disable_packet_mode();
                }
                set_packet_pause_toggle();
            }
        }
        previous_gameturn = get_gameturn();
    }

    if (use_delta_time())
    {
        update_gameplay_delta_time();
        if (game.input_lag_turns == 0 && network_is_active())
        {
            // Aim to exchange network packets before the turn ends.  If drawing
            // another frame could miss this deadline, skip it.
            // In a 3-4 player game, clients must be 2 frames early.
            const int frames = 1 + (netstate.my_id != SERVER_ID && game.active_players_count > 2);
            const long double offset = frames * average_frame_draw_time * multiplayer_clock_adjust * max(game.frame_skip, 1);
            if (game.process_turn_time + offset < 1.0)
                return;
        }
        else
        {
            if (game.process_turn_time < 1.0)
                return;
        }
    }

    frametime_start_measurement(Frametime_Logic);
    if (frametime_enabled())
        framerate_measurement_capture(Framerate_Logic);

#ifdef FUNCTESTING
    if(flag_is_set(start_params.functest_flags, FTF_Enabled))
    {
        FTestFrameworkState ftstate = ftest_update(NULL);
        if(ftstate == FTSt_InvalidState || ftstate == FTSt_TestsCompletedSuccessfully)
        {
            quit_game = true;
            exit_keeper = true;
            return;
        }
    }
#endif // FUNCTESTING
    do_draw = display_should_be_updated_this_turn() || (!LbIsActive());
    poll_inputs();
    input_eastegg();
    input();
    exchange_packets();

    update_gameplay_delta_time();
    if (game.process_turn_time > turns_per_second + 1)
        game.process_turn_time = turns_per_second + 1;

    // Adjust client time scaling
    if (netstate.my_id != SERVER_ID && network_is_active())
    {
        if (game.input_lag_turns == 0)
        {
            // Adjust the clock rate so that the host packet is received at
            // process_turn_time == 1.0 (on average).  If it is received later,
            // reduce the scaling factor (< 1.0) so that the next turn takes a
            // little longer in real time.  Vice-versa if it is early.

            multiplayer_clock_adjust = 1 + (1 - host_packet_received) / 20;
        }
        else
        {
            const long double tick_ns_one_turn = 1e9L / turns_per_second;
            const long double tick_ns_adjusted_turn = tick_ns_one_turn + multiplayer_speed_adjustment_ns;
            assert (tick_ns_adjusted_turn > 0);
            multiplayer_clock_adjust = tick_ns_one_turn / tick_ns_adjusted_turn;
        }
    }
    else multiplayer_clock_adjust = 1.0;
    host_packet_received = 1.0;

    while (game.process_turn_time < 1.0)
    {
        gameplay_loop_draw();
        update_gameplay_delta_time();
    }
    game.process_turn_time -= 1.0;

    update();

    frametime_end_measurement(Frametime_Logic);

    if(game.frame_step)
    {
        game.frame_step = false;
        set_packet_pause_toggle();
    }
}

static void gameplay_loop_network()
{
    if (! network_is_active())
        return;

    network_update(game.packets, sizeof(struct Packet));
}

static TbBool keeper_wait_for_next_turn(void)
{
    const long double tick_ns_one_sec = 1000000000.0;
    long double tick_ns_one_frame = -1;
    if ((game.view_mode_flags & GNFldD_WaitSleepMode) != 0)
    {
        // No idea when such situation occurs
        tick_ns_one_frame = tick_ns_one_sec;
    }
    if (game.frame_skip >= 0)
    {
        // Standard delaying system
        int32_t num_fps = turns_per_second;
        if (game.frame_skip > 0)
            num_fps *= game.frame_skip;

        tick_ns_one_frame = tick_ns_one_sec/num_fps;
    }

    if (tick_ns_one_frame >= 0) {
        static long double tick_ns_last_turn = 0;

        long double tick_ns_cur = get_time_tick_ns();
        long double tick_ns_used = tick_ns_cur - tick_ns_last_turn;
        long double tick_ns_delay = tick_ns_one_frame - tick_ns_used;
        if (multiplayer_speed_adjustment_ns != 0) {
            tick_ns_delay += multiplayer_speed_adjustment_ns;
        }

        long double tick_ns_end = tick_ns_cur;
        // tick_ns_used: every level, initialized_time_point will be reset, so tick_ns_used may be less than 0 when enter level for the non-first time, Skip it directly to solve the problem.
        if (tick_ns_delay > 0 && tick_ns_used >= 0) {
            tick_ns_end = tick_ns_cur + tick_ns_delay;
            LbSleepUntilExt(tick_ns_end);
        }
        tick_ns_last_turn = tick_ns_end;
        return true;
    }

    return false;
}

static void gameplay_loop_timestep()
{
    if (! use_delta_time()) {
        frametime_start_measurement(Frametime_Sleep);
        // Make delay if the machine is too fast
        if ( (!game.packet_load_enable) || (game.turns_fastforward == 0) ) {
            keeper_wait_for_next_turn();
        }
        frametime_end_measurement(Frametime_Sleep);
    }
}

static void keeper_gameplay_loop(void)
{
    struct PlayerInfo *player;
    SYNCDBG(5,"Starting");
    player = get_my_player();
    PaletteSetPlayerPalette(player, engine_palette);
    if ((game.operation_flags & GOF_SingleLevel) != 0) {
        initialise_eye_lenses();
    }
    SYNCDBG(0,"Entering the gameplay loop for level %d",(int)get_loaded_level_number());
    LbErrorParachuteUpdate(); // For some reasone parachute keeps changing; Remove when won't be needed anymore

    initial_time_point();
    LbSleepExtInit();

    //the main gameplay loop starts
    while ((!quit_game) && (!exit_keeper))
    {
        frametime_start_measurement(Frametime_FullFrame);
        if (frametime_enabled())
            framerate_measurement_capture(Framerate_FullFrame);
        gameplay_loop_logic();
        gameplay_loop_draw();
        gameplay_loop_network();
        gameplay_loop_timestep();

        frametime_end_measurement(Frametime_FullFrame);
    } // end while
    SYNCDBG(0,"Gameplay loop finished after %lu turns",(unsigned long)get_gameturn());

    // Reset the game kind because we are not in a game anymore at this point
    game.game_kind = GKind_Unset;

    api_event("GAME_ENDED");
}


static TbBool should_use_delta_time_on_menu()
{
    switch (frontend_menu_state) {
        case FeSt_MAIN_MENU:
        case FeSt_FELOAD_GAME:
        case FeSt_NET_SERVICE: /**< Network service selection, where player can select Serial/Modem/IPX/TCP IP/1 player. */
        case FeSt_NET_SESSION: /**< Network session selection screen, where list of games is displayed, with possibility to join or create own game. */
        case FeSt_NET_START: /**< Network game start screen (the menu with chat), when created new session or joined existing session. */
        case FeSt_LEVEL_STATS:
        case FeSt_HIGH_SCORES:
        case FeSt_FEDEFINE_KEYS:
        case FeSt_FEOPTIONS:
        case FeSt_LEVEL_SELECT:
        case FeSt_CAMPAIGN_SELECT:
        case FeSt_MAPPACK_SELECT:
        case FeSt_MP_MAPPACK_SELECT:
        case FeSt_LAND_VIEW:
        case FeSt_NETLAND_VIEW:
        case FeSt_TORTURE:
            return true;
        default:
            return false;
    }
}

static void faststartup_saved_packet_game(void)
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

static TbBool wait_at_frontend(void)
{
    struct PlayerInfo *player;
    // This is an improvised coroutine-like stuff
    CoroutineLoop loop;
    memset(&loop, 0, sizeof(loop));

    SYNCDBG(0,"Falling into frontend menu.");
    // Moon phase calculation
    calculate_moon_phase(true,false);
    update_extra_levels_visibility();
    // Returning from Demo Mode
    if (game.mode_flags & MFlg_IsDemoMode)
    {
      close_packet_file();
      game.packet_load_enable = 0;
    }
    game.save_game_slot = -1;
    // Make sure campaigns are loaded
    if (!load_campaigns_list(&campaigns_list ,FGrp_Campgn ,"campaigns","campgn_order.txt"))
    {
      ERRORLOG("No valid campaign files found");
      exit_keeper = 1;
      return true;
    }
    // Make sure mappacks are loaded
    if (!load_campaigns_list(&mappacks_list,FGrp_VarLevels,"mappacks","mappck_order.txt"))
    {
      WARNMSG("No valid mappack files found");
    }
    if (!load_campaigns_list(&mp_mappacks_list,FGrp_MpLevels,"multiplayer mappacks","mp_mappck_order.txt"))
    {
      WARNMSG("No valid multiplayer mappack files found");
    }
    //Set level number and campaign (for single level mode: GOF_SingleLevel)
    if ((start_params.operation_flags & GOF_SingleLevel) != 0)
    {
        TbBool result = false;
        if (start_params.selected_campaign[0] != '\0')
        {
            result = change_campaign(CampgnT_Default, start_params.selected_campaign);
        }
        if (!result) {
            if (!change_campaign(CampgnT_Default,"")) {
                WARNMSG("Unable to load default campaign for the specified level CMD Line parameter");
            }
            else if (start_params.selected_campaign[0] != '\0') { // only show this log message if the user actually specified a campaign
                WARNMSG("Unable to load campaign associated with the specified level CMD Line parameter, default loaded.");
            }
            else {
                JUSTLOG("No campaign specified. Default campaign loaded for selected level (%u).", start_params.selected_level_number);
            }
        }
        set_selected_level_number(start_params.selected_level_number);
        //game.selected_level_number = start_params.selected_level_number;
    }
    else
    {
        set_selected_level_number(first_singleplayer_level());
    }
    // Init load/save catalogue
    initialise_load_game_slots();

    #ifdef FUNCTESTING
    if(flag_is_set(start_params.functest_flags, FTF_Enabled)) //override for functional tests
    {
        FTestFrameworkState ft_prev_state = FTSt_InvalidState;
        FTestFrameworkState ft_current_state = ftest_update(&ft_prev_state);

        TbBool user_aborted_tests = ft_prev_state == FTSt_TestIsProcessingActions && ft_current_state == FTSt_TestIsProcessingActions;
        if(user_aborted_tests)
        {
            FTEST_FAIL_TEST("User aborted tests");
        }

        if(ft_current_state == FTSt_InvalidState || ft_current_state == FTSt_TestsCompletedSuccessfully || user_aborted_tests)
        {
            quit_game = true;
            exit_keeper = true;
            return true;
        }
        faststartup_network_game(&loop);
        coroutine_process(&loop);
        return true;
    }
    #endif

    // Prepare to enter PacketLoad game
    if ((game.packet_load_enable) && (!game.packet_load_initialized))
    {
      faststartup_saved_packet_game();
      return true;
    }
    // Load single-player level directly from command line arguments (-server and -connect bypass this, autoloading a multiplayer map is handled elsewhere)
    if ((game.operation_flags & GOF_SingleLevel) != 0 && !(game_flags2 & (GF2_Connect | GF2_Server)))
    {
      faststartup_network_game(&loop);
      coroutine_process(&loop);
      return true;
    }

    if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
    {
      FatalError = 1;
      exit_keeper = 1;
      return true;
    }
    LbScreenClear(0);
    LbScreenSwap();
    if (frontend_load_data() != Lb_SUCCESS)
    {
      ERRORLOG("Unable to load frontend data");
      exit_keeper = 1;
      return true;
    }
    memset(scratch, 0, PALETTE_SIZE);
    LbPaletteSet(scratch);
    frontend_set_state(get_startup_menu_state());

    // Once the Mouse Sprite initialization is complete, the sprite's position needs to be reset because it defaults to (0, 0).
    // Note that we cannot use LbMoveGameCursorToHostCursor for this, because the buffer position may remain unchanged.
    LbMouseSetPositionInitial(lbDisplay.MMouseX, lbDisplay.MMouseY);

    try_restore_frontend_error_box();

    poll_inputs();
    clear_mouse_pressed_lrbutton();

    short finish_menu = 0;
    clear_flag(game.mode_flags, MFlg_DemoMode);
    // TODO move to separate function
    // Begin the frontend loop
    long fe_last_loop_time = LbTimerClock();
    do
    {
      if (!poll_inputs())
      {
        force_application_close();
        SYNCDBG(0,"Windows Control exit condition invoked");
        break;
      }
      update_mouse();
      update_key_modifiers();
      old_mouse_over_button = frontend_mouse_over_button;
      frontend_mouse_over_button = 0;

      frontend_input();
      if ( exit_keeper )
      {
        SYNCDBG(0,"Frontend Input exit condition invoked");
        break; // end while
      }

      frontend_update(&finish_menu);
      if ( exit_keeper )
      {
        SYNCDBG(0,"Frontend Update exit condition invoked");
        break; // end while
      }

      if ((!finish_menu) && (LbIsActive()))
      {
        frontend_draw();
        LbScreenSwap();
      }

      if (!SoundDisabled)
      {
        process_3d_sounds();
        MonitorStreamedSoundTrack();
      }

      if (fade_palette_in)
      {
        fade_in();
        fade_palette_in = 0;
      } else {
        if (is_feature_on(Ft_DeltaTime) == true && should_use_delta_time_on_menu()) {
          update_frontend_delta_time();
        } else {
          int32_t frame_time;
          frame_time = max(1, 1000 / turns_per_second);
          game.delta_time = 1;
          LbSleepUntil(fe_last_loop_time + frame_time);
        }
      }
      fe_last_loop_time = LbTimerClock();

      api_update_server();

    } while (!finish_menu);

    LbPaletteFade(0, 8, Lb_PALETTE_FADE_CLOSED);
    LbScreenClear(0);
    LbScreenSwap();
    FrontendMenuState prev_state;
    prev_state = frontend_menu_state;
    frontend_set_state(FeSt_INITIAL);
    if (exit_keeper)
    {
      player = get_my_player();
      player->display_flags &= ~PlaF6_PlyrHasQuit;
      return true;
    }
    reenter_video_mode();

    display_loading_screen();

    short flgmem;
    switch (prev_state)
    {
    case FeSt_START_KPRLEVEL:
          my_player_number = default_loc_player;
          game.game_kind = GKind_LocalGame;
          clear_flag(game.system_flags, GSF_NetworkActive);
          player = get_my_player();
          player->is_active = 1;
          startup_network_game(&loop, true);
          break;
    case FeSt_START_MPLEVEL:
          set_flag(game.system_flags, GSF_NetworkActive);
          skip_high_score_screen = 1;
          game.game_kind = GKind_MultiGame;
          player = get_my_player();
          player->is_active = 1;
          startup_network_game(&loop, false);
          break;
    case FeSt_LOAD_GAME:
          flgmem = game.save_game_slot;
          clear_flag(game.system_flags, GSF_NetworkActive);
          LbScreenClear(0);
          LbScreenSwap();
          if (!load_game(game.save_game_slot))
          {
              ERRORLOG("Loading game %d failed; quitting.",(int)game.save_game_slot);
              quit_game = 1;
          }
          game.save_game_slot = flgmem;
          break;
    case FeSt_PACKET_DEMO:
          game.mode_flags |= MFlg_IsDemoMode;
          startup_saved_packet_game();
          set_gui_visible(false);
          clear_flag(game.operation_flags, GOF_ShowPanel);
          break;
    }

    coroutine_add(&loop, &set_not_has_quit);
    coroutine_process(&loop);
    if (loop.error)
    {
        frontend_set_state(FeSt_INITIAL);
        return false;
    }
    return true;
}

void game_loop(void)
{
#if (BFDEBUG_LEVEL > 0)
    unsigned long playtime = 0;
#endif
    SYNCDBG(0,"Entering gameplay loop.");

    while ( !exit_keeper )
    {
      update_mouse();
      while (!wait_at_frontend())
      {
          if (exit_keeper)
              break;
      }
      if ( exit_keeper )
        break;

      int32_t mspos_x_bak = lbDisplay.MMouseX;
      int32_t mspos_y_bak = lbDisplay.MMouseY;

      if (game.game_kind == GKind_LocalGame)
      {
        if (game.save_game_slot == -1)
        {
            if (is_feature_on(Ft_SkipHeartZoom) == false) {
                for (int i = 0; i < PLAYERS_COUNT; i++) {
                    struct PlayerInfo *player = get_player(i);
                    if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
                        set_player_instance(player, PI_HeartZoom, 0);
                    }
                }
            } else {
                if (!game.packet_load_enable) {
                    toggle_status_menu(1); // Required when skipping PI_HeartZoom
                }
            }
        } else
        {
          game.save_game_slot = -1;
        }
      } else {
          for (int i = 0; i < PLAYERS_COUNT; i++) {
              struct PlayerInfo *player = get_player(i);
              if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0)) {
                  set_player_instance(player, PI_HeartZoom, 0);
              }
          }
      }

      // Try to keep the mouse position unchanged when entering the level.
      // The main considerations are:
      // 1. SKIP_HEART_ZOOM: the mouse icon position will be reset to the top-left corner (0, 0), but the actual mouse position remains unchanged.
      // 2. PI_HeartZoom: the mouse will be moved to the center of the screen.
      LbMouseSetPosition(mspos_x_bak, mspos_y_bak);

      unsigned long starttime;
#if (BFDEBUG_LEVEL > 0)
      unsigned long endtime;
#endif
      struct Dungeon *dungeon;
      // get_my_dungeon() can't be used here because players are not initialized yet
      dungeon = get_dungeon(my_player_number);
      starttime = LbTimerClock();
      dungeon->lvstats.start_time = starttime;
      dungeon->lvstats.end_time = starttime;
      if (!TimerNoReset)
      {
          if (is_feature_on(Ft_SkipHeartZoom))
          {
              timerstarttime = starttime;
          }
          else
          {
              TimerFreeze = true;
          }
          memset(&Timer, 0, sizeof(Timer));
      }
      LbScreenClear(0);
      LbScreenSwap();
      game.frame_skip = 0;
      keeper_gameplay_loop();
      set_pointer_graphic_none();
      LbScreenClear(0);
      LbScreenSwap();
      stop_atmos_sounds();
      stop_music(true);
      stop_streamed_samples();
      free_level_strings_data();
      turn_off_all_menus();
      delete_all_structures();
      clear_mapwho();
      // Reset sounds back to the fxdata baseline so the main menu (and any
      // subsequent campaign/freeplay selection) hears unmodified defaults.
      sound_reset_to_fxdata_baseline();
#if (BFDEBUG_LEVEL > 0)
      endtime = LbTimerClock();
#endif
      quit_game = 0;
      if ((game.operation_flags & GOF_SingleLevel) != 0)
          exit_keeper=true;
#if (BFDEBUG_LEVEL > 0)
      playtime += endtime-starttime;
#endif
      SYNCDBG(0,"Play time is %lu seconds",playtime>>10);
      reset_eye_lenses();
      close_packet_file();
      game.packet_load_enable = false;
      game.packet_save_enable = false;
    } // end while

    // Stop the movie recording if it's on
    if ((game.system_flags & GSF_CaptureMovie) != 0) {
        movie_record_stop();
    }
    ShutDownSDLAudio();
    SYNCDBG(7,"Done");
}



/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/******************************************************************************/
