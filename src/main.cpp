/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file main.cpp
 * @author KeeperFX Team
 * @date 01 Aug 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"

#include "platform.h"
#include "kfx/platform/PlatformManager.h"
#include "kfx/renderer/RendererManager.h"
#include "keeperfx.hpp"

#include "bflib_coroutine.h"
#include "bflib_math.h"
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
#include "config_sounds.h"
#include "bflib_mouse.h"
#include "bflib_mshandler.hpp"
#include "bflib_filelst.h"
#include "net_exchange_gameplay.h"
#include "net_lobby.h"
#include "net_resync.h"
#include "bflib_planar.h"

#include "ariadne_update.h"
#include "api.h"
#include "custom_sprites.h"
#include "version.h"
#include "front_simple.h"
#include "frontend.h"
#include "front_input.h"
#include "frontmenu_net.h"
#include "gui_parchment.h"
#include "gui_frontmenu.h"
#include "gui_msgs.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "kjm_input.h"
#include "packets.h"
#include "config.h"
#include "config_slabsets.h"
#include "config_strings.h"
#include "config_campaigns.h"
#include "front_landview.h"
#include "config_terrain.h"
#include "config_objects.h"
#include "config_magic.h"
#include "config_creature.h"
#include "config_compp.h"
#include "config_effects.h"
#include "lua_triggers.h"
#include "lvl_script.h"
#include "lvl_filesdk1.h"
#include "thing_list.h"
#include "player_instances.h"
#include "player_utils.h"
#include "config_players.h"
#include "player_computer.h"
#include "game_heap.h"
#include "game_saves.h"
#include "engine_render.h"
#include "engine_lenses.h"
#include "engine_camera.h"
#include "local_camera.h"
#include "engine_arrays.h"
#include "engine_textures.h"
#include "engine_redraw.h"
#include "front_easter.h"
#include "front_fmvids.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_creature.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_doors.h"
#include "thing_traps.h"
#include "thing_navigate.h"
#include "thing_shots.h"
#include "thing_factory.h"
#include "slab_data.h"
#include "room_data.h"
#include "room_entrance.h"
#include "room_util.h"
#include "map_columns.h"
#include "map_ceiling.h"
#include "map_events.h"
#include "map_utils.h"
#include "map_blocks.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "creature_states_combt.h"
#include "creature_states_mood.h"
#include "lens_api.h"
#include "light_data.h"
#include "magic_powers.h"
#include "power_process.h"
#include "power_hand.h"
#include "game_merge.h"
#include "gui_topmsg.h"
#include "gui_boxmenu.h"
#include "gui_soundmsgs.h"
#include "gui_frontbtns.h"
#include "frontmenu_ingame_tabs.h"
#include "frontmenu_ingame_evnt.h"
#include "sounds.h"
#include "vidfade.h"
#include "config_settings.h"
#include "config_keeperfx.h"
#include "game_legacy.h"
#include "room_list.h"
#include "steam_api.hpp"
#include "game_loop.h"
#include "net_input_lag.h"
#include "moonphase.h"
#include "frontmenu_ingame_map.h"
#include "room_library.h"
#include <cstdint>
#include "timer.h"

#ifdef FUNCTESTING
  #include "ftests/ftest.h"
#endif

#include "post_inc.h"

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif


short default_loc_player = 0;
struct StartupParameters start_params;
char autostart_multiplayer_campaign[80] = "";
int autostart_multiplayer_level = 0;
int autostart_multiplayer_users_expected = 2;
int32_t turns_per_second;
unsigned char *blue_palette;
unsigned char *red_palette;
unsigned char *dog_palette;
unsigned char *vampire_palette;
unsigned char exit_keeper;
unsigned char quit_game;
unsigned char is_running_under_wine = false;
int continue_game_option_available;

int FatalError;
int32_t define_key_scroll_offset;
uint32_t time_last_played_demo;
short drag_menu_x;
short drag_menu_y;
int32_t pointer_x;
int32_t pointer_y;
int32_t block_pointed_at_x;
int32_t block_pointed_at_y;
int32_t pointed_at_frac_x;
int32_t pointed_at_frac_y;
int32_t top_pointed_at_x;
int32_t top_pointed_at_y;
int32_t top_pointed_at_frac_x;
int32_t top_pointed_at_frac_y;
char level_name[88];
char top_of_breed_list;
/** Amount of different creature kinds the local player has. Used for creatures tab in panel menu. */
char no_of_breeds_owned;
int32_t optimised_lights;
int32_t total_lights;
unsigned char do_lights;
struct Thing *thing_pointed_at;
struct Map *me_pointed_at;
char *level_names_data;
char *end_level_names_data;
unsigned char *frontend_backup_palette;
unsigned char zoom_to_heart_palette[768];
unsigned char EngineSpriteDrawUsingAlpha;
unsigned char temp_pal[768];
unsigned char *lightning_palette;


#ifdef __cplusplus
extern "C" {
#endif

TbBool force_player_num = false;

/******************************************************************************/


/******************************************************************************/

int32_t fps_limit_current = 0;
int32_t fps_limit_main = 0; // -1 if auto
int32_t fps_limit_secondary = 0;
long double process_frame_time = 0;
long double time_since_last_draw = 0;
long double multiplayer_clock_adjust = 1;
long double host_packet_received = 1;

/******************************************************************************/

void setup_stuff(void)
{
    setup_texture_block_mem();
    init_fades_table();
    init_alpha_table();
}

TbBool all_dungeons_destroyed(const struct PlayerInfo *win_player)
{
    long win_plyr_idx;
    long i;
    win_plyr_idx = win_player->id_number;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      if (i == win_plyr_idx)
        continue;
      if (!player_is_friendly_or_defeated(i,win_plyr_idx))
        return false;
    }
    SYNCDBG(1,"Returning true for player %ld",win_plyr_idx);
    return true;
}

void init_censorship(void)
{
  if ( censorship_enabled() )
  {
    // Modification for Dark Mistress
      set_creature_model_graphics(20, 14, 48);
  }
}

void init_keeper(void)
{
    SYNCDBG(8,"Starting");
    engine_init();
    init_fp_td_animation_conversion_tables();
    init_colours();
    init_spiral_steps();
    init_key_to_strings();
    // Load configs which may have per-campaign part, and even be modified within a level
    recheck_all_mod_exist();
    init_custom_sprites(SPRITE_LAST_LEVEL);
    load_stats_files();
    check_and_auto_fix_stats();
    init_creature_scores();
    init_top_texture_to_cube_table();
    game.neutral_player_num = PLAYER_NEUTRAL;
    poly_pool_end = &poly_pool[sizeof(poly_pool)-128];
    lbDisplay.GlassMap = pixmap.ghost;
    RendererSetDrawColour(colours[15][15][15]);
    game.comp_player_aggressive  = (comp_player_conf.player_assist_default == comp_player_conf.computer_assist_types[0]);
    game.comp_player_defensive   = (comp_player_conf.player_assist_default == comp_player_conf.computer_assist_types[1]);
    game.comp_player_construct   = (comp_player_conf.player_assist_default == comp_player_conf.computer_assist_types[2]);
    game.comp_player_creatrsonly = (comp_player_conf.player_assist_default == comp_player_conf.computer_assist_types[3]);
    game.creatures_tend_imprison = 0;
    game.creatures_tend_flee = 0;
    game.operation_flags |= GOF_ShowPanel;
    game.view_mode_flags |= (GNFldD_StatusPanelDisplay | GNFldD_RoomFlameProcessing);
    init_censorship();
    SYNCDBG(9,"Finished");
}

/**
 * Initial video setup - loads only most important files to show startup screens.
 */
TbBool initial_setup(void)
{
    SYNCDBG(6,"Starting");
    // setting this will force video mode change, even if previous one is same
    MinimalResolutionSetup = true;
    // Set size of static textures buffer
    game_load_files[1].SLength = max((ulong)TEXTURE_BLOCKS_STAT_COUNT_A*block_dimension*block_dimension,(ulong)LANDVIEW_MAP_WIDTH*LANDVIEW_MAP_HEIGHT);
    if (LbDataLoadAllV2(game_load_files))
    {
        ERRORLOG("Unable to load game_load_files");
        return false;
    }
    load_pointer_file(0);
    update_screen_mode_data(320, 200);
    clear_game();
    RendererAddDrawFlags(0x4000u);
    return true;
}

/**
 * Displays 'legal' screens, intro and initializes basic game data.
 * If true is returned, then all files needed for startup were loaded,
 * and there should be the loading screen visible.
 * @return Returns true on success, false on error which makes the
 *   gameplay impossible (usually files loading failure).
 * @note The current screen resolution at end of this function may vary.
 */

short setup_game(void)
{
  struct CPU_INFO cpu_info; // CPU status variable
  short result;
  // Do only a very basic setup
  cpu_detect(&cpu_info);
  SYNCMSG("CPU %s type %d family %d model %d stepping %d features %08lx",cpu_info.vendor,
      (int)cpu_get_type(&cpu_info),(int)cpu_get_family(&cpu_info),(int)cpu_get_model(&cpu_info),
      (int)cpu_get_stepping(&cpu_info),cpu_info.feature_edx);
  if (cpu_info.BrandString)
  {
      SYNCMSG("%s", &cpu_info.brand[0]);
  }
  SYNCMSG("Build image base: %p", PlatformManager_GetImageBase());
  SYNCMSG("Operating System: %s", PlatformManager_GetOSVersion());

  const auto wine_version = PlatformManager_GetWineVersion();
  if (wine_version) {
        SYNCMSG("Running on Wine v%s", wine_version);
        is_running_under_wine = true;
        const auto wine_host = PlatformManager_GetWineHost();
        SYNCMSG("Wine Host: %s", wine_host);
  }

  // Enable features that require more than 32 megs of memory
  features_enabled |= Ft_HiResCreatr;
  // Enable features that require more than 16 megs of memory
  features_enabled |= Ft_EyeLens;
  features_enabled |= Ft_HiResVideo;
  features_enabled |= Ft_BigPointer;
  features_enabled |= Ft_AdvAmbSound;

  // Default feature settings (in case the options are absent from keeperfx.cfg)
  features_enabled &= ~Ft_FreezeOnLoseFocus; // don't freeze the game, if the game window loses focus
  features_enabled &= ~Ft_UnlockCursorOnPause; // don't unlock the mouse cursor from the window, if the user pauses the game
  features_enabled |= Ft_LockCursorInPossession; // lock the mouse cursor to the window, when the user enters possession mode (when the cursor is already unlocked)
  features_enabled |= Ft_RelativeMouseMode; // use SDL relative ("raw") mouse mode; set RELATIVE_MOUSE_MODE=OFF for the grab-and-warp scheme
  features_enabled &= ~Ft_PauseMusicOnGamePause; // don't pause the music, if the user pauses the game
  features_enabled &= ~Ft_MuteAudioOnLoseFocus; // don't mute the audio, if the game window loses focus
  features_enabled &= ~Ft_SkipHeartZoom; // don't skip the dungeon heart zoom in
  features_enabled &= ~Ft_DisableCursorCameraPanning; // don't disable cursor camera panning
  features_enabled |= Ft_DeltaTime; // enable delta time
  features_enabled |= Ft_NoCdMusic; // use music files (OGG) rather than CD music

  // Configuration file
  if ( !load_configuration() )
  {
      ERRORLOG("Configuration load error.");
      return 0;
  }

  #ifdef FUNCTESTING
    start_params.startup_flags &= ~SFlg_Legal;
    start_params.startup_flags &= ~SFlg_FX;
    features_enabled |= Ft_SkipHeartZoom;
  #endif

  // Process CmdLine overrides
  process_cmdline_overrides();

  LbIKeyboardOpen();

  if (LbDataLoadAll(legal_load_files) != 0)
  {
      ERRORLOG("Error on allocation/loading of legal_load_files.");
      return 0;
  }

  // Setup polyscans
  setup_bflib_render();

  // View the legal screen
  if (!setup_screen_mode_zero(get_frontend_vidmode()))
  {
      ERRORLOG("Unable to set display mode for legal screen");
      return 0;
  }

  if (flag_is_set(start_params.startup_flags, SFlg_Legal))
  {
      if (is_ar_wider_than_original(LbGraphicsScreenWidth(), LbGraphicsScreenHeight()))
      {
        result = init_actv_bitmap_screen(RBmp_SplashLegalWide);
      } else {
        result = init_actv_bitmap_screen(RBmp_SplashLegal);
      }
       if ( result )
      {
          result = show_actv_bitmap_screen(3000);
          free_actv_bitmap_screen();
      } else
          SYNCLOG("Legal image skipped");
  }
  else
  {
      // Make the white screen into a black screen faster
      draw_clear_screen();
  }

  // Now do more setup
  // Prepare the Game structure
  clear_complete_game();
  // Moon phase calculation
  calculate_moon_phase(true,true);
  // Start the sound system
  if (!init_sound())
    WARNMSG("Sound system disabled.");
  // Note: for some reason, signal handlers must be installed AFTER
  // init_sound(). This will probably change when we'll move sound
  // to SDL - then we'll put that line earlier, before setup_game().
  LbErrorParachuteInstall();
  // View second splash screen
  if (flag_is_set(start_params.startup_flags, SFlg_FX))
  {
      result = init_actv_bitmap_screen(RBmp_SplashFx);
      if ( result == 1 )
      {
          result = show_actv_bitmap_screen(4000);
          free_actv_bitmap_screen();
      } else
          SYNCLOG("startup_fx image skipped");
  }

  draw_clear_screen();
  // View Bullfrog company logo animation when new moon
  if ( ( is_new_moon ) || (flag_is_set(start_params.startup_flags, SFlg_Bullfrog)) )
    if (!start_params.no_intro)
    {
        result = moon_video();
        if ( !result ) {
            ERRORLOG("Unable to play new moon movie");
        }
    }

  result = 1;
  // Setup the intro video mode
  if (result && (!start_params.no_intro) )
  {
      if (!setup_screen_mode_zero(get_movies_vidmode()))
      {
        ERRORLOG("Can't enter movies screen mode to play intro");
        result=0;
      }
  }

  if (result == 1)
  {
      draw_clear_screen();
      if (wait_for_installation_files())
      {
          //result = -1; // Helps with better warning message later
      }
      if (!start_params.no_intro)
      {
         if (flag_is_set(start_params.startup_flags, SFlg_EA))
         {
             ea_video();
         }
         if (flag_is_set(start_params.startup_flags, SFlg_Intro))
         {
            result = intro_replay();
         }
      }
  }

  game.frame_skip = start_params.frame_skip;
  redetect_screen_refresh_rate_for_draw();

  // Intro problems shouldn't force the game to quit,
  // so we're re-setting the result flag
  if (result == 0)
      result = 1;

  if (result == 1)
  {
      display_loading_screen();
  }
  LbDataFreeAll(legal_load_files);

  if (result == 1)
  {
      if ( !initial_setup() )
        result = 0;
  }

  if (result == 1)
  {
    load_settings();
    if ( !setup_gui_strings_data() )
      result = 0;
  }

  if (result == 1)
  {
      init_keeper();
      set_gamma(settings.gamma_correction, 0);
      set_music_volume(settings.music_volume);
      SetSoundMasterVolume(settings.sound_volume);
      setup_mesh_randomizers();
      setup_stuff();
  }

  return result;
}

/** Returns if cursor for given player is at top of the dungeon in 3D view.
 *  Cursor placed at top of dungeon is marked by green/red "volume box";
 *   if there's no volume box, cursor should be of the field behind it
 *   (the exact field in a line of view through cursor). If cursor is at top
 *   of view, then pointed map field is a bit lower than the line of view
 *   through cursor.
 *
 * @param player
 * @return
 */
TbBool players_cursor_is_at_top_of_view(struct PlayerInfo *player)
{
    switch (player->work_state)
    {
    case PSt_BuildRoom:
    case PSt_PlaceDoor:
    case PSt_PlaceTrap:
    case PSt_SightOfEvil:
    case PSt_Sell:
    case PSt_PlaceTerrain:
    case PSt_MkDigger:
        return true;

    case PSt_OrderCreatr:
        return (player->controlled_thing_idx > 0);

    case PSt_CtrlDungeon:
        switch (player->primary_cursor_state)
        {
            case CSt_DefaultArrow:
                return false;

            case CSt_PickAxe:
            case CSt_DoorKey:
                return true;

            case CSt_PowerHand:
                return (player->thing_under_hand == 0)
                    || (! power_hand_is_empty(player));
        }
    }
    return false;
}

TbBool engine_point_to_map(struct Camera *camera, long screen_x, long screen_y, int32_t *map_x, int32_t *map_y)
{
    struct PlayerInfo *player = get_my_player();
    *map_x = 0;
    *map_y = 0;
    if ( (pointer_x >= 0) && (pointer_y >= 0)
      && (pointer_x < (player->engine_window_width/pixel_size))
      && (pointer_y < (player->engine_window_height/pixel_size)) )
    {
        if ( players_cursor_is_at_top_of_view(player) )
        {
              *map_x = subtile_coord(top_pointed_at_x,top_pointed_at_frac_x);
              *map_y = subtile_coord(top_pointed_at_y,top_pointed_at_frac_y);
        } else
        {
              *map_x = subtile_coord(block_pointed_at_x,pointed_at_frac_x);
              *map_y = subtile_coord(block_pointed_at_y,pointed_at_frac_y);
        }
        // Clipping coordinates
        if (*map_y < 0)
          *map_y = 0;
        else
        if (*map_y > subtile_coord(game.map_subtiles_y,-1))
          *map_y = subtile_coord(game.map_subtiles_y,-1);
        if (*map_x < 0)
          *map_x = 0;
        else
        if (*map_x > subtile_coord(game.map_subtiles_x,-1))
          *map_x = subtile_coord(game.map_subtiles_x,-1);
        return true;
    }
    return false;
}

TbBool screen_to_map(struct Camera *camera, int32_t screen_x, int32_t screen_y, struct Coord3d *mappos)
{
    TbBool result;
    int32_t x;
    int32_t y;
    SYNCDBG(19,"Starting");
    result = false;
    if (camera != NULL)
    {
      switch (camera->view_mode)
      {
        case PVM_CreatureView:
        case PVM_IsoWibbleView:
        case PVM_FrontView:
        case PVM_IsoStraightView:
          // 3D view mode
          result = engine_point_to_map(camera,screen_x,screen_y,&x,&y);
          break;
        case PVM_ParchmentView: //map mode
          result = point_to_overhead_map(camera,screen_x/pixel_size,screen_y/pixel_size,&x,&y);
          break;
        default:
          result = false;
          break;
      }
    }
    if ( result )
    {
      mappos->x.val = x;
      mappos->y.val = y;
    }
    if ( mappos->x.val > ((game.map_subtiles_x<<8)-1) )
      mappos->x.val = ((game.map_subtiles_x<<8)-1);
    if ( mappos->y.val > ((game.map_subtiles_y<<8)-1) )
      mappos->y.val = ((game.map_subtiles_y<<8)-1);
    SYNCDBG(19,"Finished");
    return result;
}

void update_creatr_model_activities_list(TbBool forced)
{
    struct Dungeon *dungeon = get_my_dungeon();
    ThingModel crmodel;
    int num_breeds = no_of_breeds_owned;
    TbBool changed = false;

    // Add to breed activities
    for (crmodel = 1; crmodel < game.conf.crtr_conf.model_count; crmodel++)
    {
        if ((dungeon->owned_creatures_of_model[crmodel] > 0)
            && (crmodel != get_players_spectator_model(my_player_number)))
        {
            TbBool found = false;
            for (int i = 0; i < num_breeds; i++)
            {
                if (breed_activities[i] == crmodel)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                changed = true;
                breed_activities[num_breeds] = crmodel;
                num_breeds++;
            }
        }
    }

    // Remove from breed activities
    for (crmodel = 1; crmodel < game.conf.crtr_conf.model_count; crmodel++)
    {
        if ((dungeon->owned_creatures_of_model[crmodel] <= 0)
          && (crmodel != get_players_special_digger_model(my_player_number)))
        {
            for (int i = 0; i < num_breeds; i++)
            {
                if (breed_activities[i] == crmodel)
                {
                    for (; i < num_breeds-1;  i++) {
                        breed_activities[i] = breed_activities[i+1];
                    }
                    changed = true;
                    num_breeds--;
                    breed_activities[i] = 0;
                    break;
                }
            }
        }
        no_of_breeds_owned = num_breeds;
    }

    // Reorder breed activities to ensure diggers are correctly positioned
    if (changed || forced)
    {
        struct CreatureModelConfig* crconf;
        ThingModel temp;
        int write_idx = 1;
        for (int i = 1; i < num_breeds; i++)
        {
            crconf = creature_stats_get(breed_activities[i]);
            if (any_flag_is_set(crconf->model_flags, (CMF_IsDiggingCreature | CMF_IsSpecDigger)))
            {
                temp = breed_activities[i];
                memmove(&breed_activities[write_idx + 1], &breed_activities[write_idx], (i - write_idx) * sizeof(ThingModel));
                breed_activities[write_idx] = temp;
                write_idx++;
            }
        }
    }
}

void toggle_hero_health_flowers(void)
{
    const char *statstr;
    toggle_flag(game.mode_flags, MFlg_NoHeroHealthFlower);
    if (game.mode_flags & MFlg_NoHeroHealthFlower)
    {
      statstr = "off";
    } else
    {
      do_sound_menu_click();
      statstr = "on";
    }
    show_onscreen_msg(2*turns_per_second, "Hero health flowers %s", statstr);
}

void reset_gui_based_on_player_mode(void)
{
    struct PlayerInfo *player = get_my_player();
    if (player->view_type == PVT_CreatureContrl)
    {
        turn_on_menu(vid_change_query_menu);
        if (player->victory_state == VicS_LostLevel)
        {
            turn_off_query_menus();
        }
    }
    else if (player->view_type == PVT_CreaturePasngr)
    {
        turn_on_menu(vid_change_query_menu);
        turn_off_query_menus();
    }
    else
    {
        turn_on_menu(GMnu_MAIN);
        if (game.active_panel_mnu_idx > 0)
        {
            initialise_tab_tags(game.active_panel_mnu_idx);
            if ( (player->work_state == PSt_CreatrInfo) || (player->work_state == PSt_CreatrInfoAll) )
            {
                turn_on_menu(vid_change_query_menu);
            }
            else
            {
                turn_on_menu(game.active_panel_mnu_idx);
            }
            MenuNumber mnuidx = menu_id_to_number(GMnu_MAIN);
            if (mnuidx != MENU_INVALID_ID) {
                setup_radio_buttons(&active_menus[mnuidx]);
            }
        }
        else
        {
            turn_on_menu(GMnu_ROOM);
        }
    }
    set_gui_visible(true);
}

void reinit_tagged_blocks_for_player(PlayerNumber plyr_idx)
{
    // Clear tagged blocks
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    for (stl_y=0; stl_y < game.map_subtiles_y; stl_y++)
    {
        for (stl_x=0; stl_x < game.map_subtiles_x; stl_x++)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x, stl_y);
            mapblk->flags &= ~SlbAtFlg_Unexplored;
            mapblk->flags &= ~SlbAtFlg_TaggedValuable;
        }
    }
    // Reinit with data from current players dungeon
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    int task_idx;
    for (task_idx = 0; task_idx < dungeon->highest_task_number; task_idx++)
    {
        struct MapTask  *mtask;
        mtask = &dungeon->task_list[task_idx];
        MapSubtlCoord taskstl_x;
        MapSubtlCoord taskstl_y;
        taskstl_x = stl_num_decode_x(mtask->coords);
        taskstl_y = stl_num_decode_y(mtask->coords);
        switch (mtask->kind)
        {
        case 2:
            for (stl_y = taskstl_y - 1; stl_y <= taskstl_y + 1; stl_y++)
            {
                for (stl_x = taskstl_x - 1; stl_x <= taskstl_x + 1; stl_x++)
                {
                    struct Map *mapblk;
                    mapblk = get_map_block_at(stl_x, stl_y);
                    mapblk->flags |= SlbAtFlg_TaggedValuable;
                }
            }
            break;
        case 1:
        case 3:
            for (stl_y = taskstl_y - 1; stl_y <= taskstl_y + 1; stl_y++)
            {
                for (stl_x = taskstl_x - 1; stl_x <= taskstl_x + 1; stl_x++)
                {
                    struct Map *mapblk;
                    mapblk = get_map_block_at(stl_x, stl_y);
                    mapblk->flags |= SlbAtFlg_Unexplored;
                }
            }
            break;
        default:
            break;
        }
    }
}

void instant_instance_selected(CrInstance check_inst_id)
{
    struct PlayerInfo *player;
    player = get_player(my_player_number);
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    struct CreatureModelConfig *crconf;
    crconf = creature_stats_get_from_thing(ctrltng);
    long i;
    long k;
    int avail_pos;
    int match_avail_pos;
    avail_pos = 0;
    match_avail_pos = 0;
    for (i=0; i < CREATURE_MAX_LEVEL; i++)
    {
        k = crconf->learned_instance_id[i];
        if (creature_instance_is_available(ctrltng, k))
        {
            if (k == check_inst_id) {
                match_avail_pos = avail_pos;
                break;
            }
            avail_pos++;
        }
    }
    first_person_instance_top_half_selected = match_avail_pos < 6 && (first_person_instance_top_half_selected || match_avail_pos < 4);
}

short zoom_to_next_annoyed_creature(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *thing;
    player = get_my_player();
    dungeon = get_players_num_dungeon(my_player_number);
    dungeon->zoom_annoyed_creature_idx = find_next_annoyed_creature(player->id_number,dungeon->zoom_annoyed_creature_idx);
    thing = thing_get(dungeon->zoom_annoyed_creature_idx);
    if (!thing_exists(thing))
    {
      return false;
    }
    set_players_packet_action(player, PckA_ZoomToPosition, thing->mappos.x.val, thing->mappos.y.val, 0, 0);
    return true;
}

TbBool toggle_computer_player(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player = get_player(plyr_idx);
    struct Dungeon *dungeon = get_players_dungeon(player);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    if ((dungeon->computer_enabled & 0x01) == 0)
    {
        dungeon->computer_enabled |= 0x01;
    } else
    {
        dungeon->computer_enabled &= ~0x01;
    }
    struct Computer2 *comp;
    comp = get_computer_player(player->id_number);
    computer_force_dump_held_things_on_map(comp, &dungeon->essential_pos);
    return true;
}

void reinit_level_after_load(void)
{
    struct PlayerInfo *player;
    int i;
    SYNCDBG(6,"Starting");
    // Reinit structures from within the game
    player = get_my_player();
    player->lens_palette = 0;
    player->main_palette = engine_palette;
    init_navigation();
    reinit_packets_after_load();
    game.easter_eggs_enabled = start_params.easter_egg;
    parchment_loaded = 0;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player))
        {
            set_engine_view(player, player->view_mode);
            update_panel_color_player_color(player->id_number, get_dungeon(i)->color_idx);
        }
    }
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];
    update_room_tab_to_config();
    update_powers_tab_to_config();
    update_trap_tab_to_config();
    load_texture_map_file(game.texture_id, get_loaded_level_number(), get_level_fgroup(get_loaded_level_number()));
    init_animating_texture_maps();
    init_gui();
    reset_gui_based_on_player_mode();
    erstats_clear();
    player = get_my_player();
    reinit_tagged_blocks_for_player(player->id_number);
    restore_computer_player_after_load();
    sound_reinit_after_load();
    update_panel_colors();
    reset_postal_instance_cache();
}

/**
 * Sets to defaults some basic parameters which are
 * later copied into Game structure.
 */
TbBool set_default_startup_parameters(void)
{
    memset(&start_params, 0, sizeof(struct StartupParameters));
    start_params.startup_flags = (SFlg_Legal|SFlg_FX|SFlg_Intro);
    start_params.packet_checksum_verify = 1;
    // Set levels to 0, as we may not have the campaign loaded yet
    start_params.selected_level_number = 0;
    start_params.num_fps = 20;
    start_params.one_player = 1;
    start_params.computer_chat_flags = CChat_None;
    clear_flag(start_params.mode_flags, MFlg_IsDemoMode);
    set_flag(start_params.mode_flags, MFlg_DemoMode);
    return true;
}

void clear_map(void)
{
    clear_mapmap();
    clear_slabs();
    clear_columns();
}

void clear_things_and_persons_data(void)
{
    struct Thing *thing;
    long i;
    memset(game.thing_lists, 0, sizeof(game.thing_lists));
    game.ambient_sound_thing_idx = 0;
    game.nodungeon_creatr_list_start = 0;
    for (i=0; i < THINGS_COUNT; i++)
    {
        thing = &game.things_data[i];
        memset(thing, 0, sizeof(struct Thing));
        thing->owner = PLAYERS_COUNT;
        thing->mappos.x.val = subtile_coord_center(game.map_subtiles_x/2);
        thing->mappos.y.val = subtile_coord_center(game.map_subtiles_y/2);

        // Create the list of free indices (skip index 0 since that's INVALID_THING
        if (i > 0) {
            if (i < SYNCED_THINGS_COUNT) {
                game.synced_free_things[SYNCED_THINGS_COUNT-1-i] = i;
            } else if (i < THINGS_COUNT) {
                game.unsynced_free_things[THINGS_COUNT-1-i] = i;
            }
        }
    }
    game.synced_free_things_count = SYNCED_THINGS_COUNT-1; // 1 to 8191. Note: COUNT macros aren't real representations of how many things there should be, all of them are off by 1.
    game.unsynced_free_things_count = UNSYNCED_THINGS_COUNT-1; // 8192 to 12287

    for (i=0; i < CREATURES_COUNT; i++)
    {
      memset(&game.cctrl_data[i], 0, sizeof(struct CreatureControl));
    }
}

void clear_computer(void)
{
    long i;
    SYNCDBG(8,"Starting");
    for (i=0; i < COMPUTER_TASKS_COUNT; i++)
    {
        memset(&game.computer_task[i], 0, sizeof(struct ComputerTask));
    }
    for (i=0; i < GOLD_LOOKUP_COUNT; i++)
    {
        memset(&game.gold_lookup[i], 0, sizeof(struct GoldLookup));
    }
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        memset(&game.computer[i], 0, sizeof(struct Computer2));
    }
}



void clear_players_for_save(void)
{
    struct PlayerInfo *player;
    unsigned short saved_player_id;
    unsigned short saved_is_active;
    unsigned short saved_allocation_flags;
    struct Camera cammem;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      saved_player_id = player->id_number;
      saved_is_active = player->is_active;
      saved_allocation_flags = player->allocflags;
      memcpy(&cammem,&player->cameras[CamIV_FirstPerson],sizeof(struct Camera));
      memset(player, 0, sizeof(struct PlayerInfo));
      player->id_number = saved_player_id;
      player->is_active = saved_is_active;
      set_flag_value(player->allocflags, PlaF_Allocated, ((saved_allocation_flags & PlaF_Allocated) != 0));
      set_flag_value(player->allocflags, PlaF_CompCtrl, ((saved_allocation_flags & PlaF_CompCtrl) != 0));
      memcpy(&player->cameras[CamIV_FirstPerson],&cammem,sizeof(struct Camera));
      set_player_active_camera(player, CamIV_FirstPerson);
    }
}

void delete_all_thing_structures(void)
{
    long i;
    struct Thing *thing;
    for (i=1; i < THINGS_COUNT; i++)
    {
      thing = thing_get(i);
      if (thing_exists(thing)) {
          delete_thing_structure(thing, 1);
      }
        if (i < SYNCED_THINGS_COUNT) {
            game.synced_free_things[SYNCED_THINGS_COUNT-1-i] = i;
        } else if (i < THINGS_COUNT) {
            game.unsynced_free_things[THINGS_COUNT-1-i] = i;
        }
    }
    game.synced_free_things_count = SYNCED_THINGS_COUNT-1;
    game.unsynced_free_things_count = UNSYNCED_THINGS_COUNT-1;
}

void delete_all_structures(void)
{
    SYNCDBG(6,"Starting");
    delete_all_thing_structures();
    delete_all_control_structures();
    delete_all_room_structures();
    delete_all_action_point_structures();
    light_initialise();
    SYNCDBG(16,"Done");
}

/**
 * Clears game structures at end of level.
 * Also used as part of clearing before new level is loaded.
 */
void clear_game_for_summary(void)
{
    SYNCDBG(6,"Starting");
    delete_all_structures();
    clear_shadow_limits(&game.lish);
    clear_stat_light_map();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_random_seed = 0;
    game.ai_random_seed = 0;
    game.player_random_seed = 0;
    game.operation_flags &= ~GOF_Paused;
    clear_columns();
    clear_action_points();
    clear_players();
    clear_dungeons();
}

void clear_game(void)
{
    SYNCDBG(6,"Starting");
    clear_game_for_summary();
    game.music_track = 0;
    clear_map();
    clear_computer();
    clear_script();
    clear_events();
    clear_things_and_persons_data();
    ceiling_set_info(12, 4, 1);
    init_animating_texture_maps();
    clear_slabsets();
    game.skip_initial_input_turns = 0;
    initialize_packet_history();
}

void clear_game_for_save(void)
{
    SYNCDBG(6,"Starting");
    delete_all_structures();
    light_initialise();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_random_seed = 0;
    game.ai_random_seed = 0;
    game.player_random_seed = 0;
    clear_columns();
    clear_players_for_save();
    clear_dungeons();
}

void reset_creature_max_levels(void)
{
    int i;
    int k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(i);
        for (k=1; k < game.conf.crtr_conf.model_count; k++)
        {
            dungeon->creature_max_level[k] = CREATURE_MAX_LEVEL+1;
        }
    }
}

void change_engine_window_relative_size(long w_delta, long h_delta)
{
    struct PlayerInfo *myplyr;
    myplyr=get_my_player();
    setup_engine_window(myplyr->engine_window_x, myplyr->engine_window_y,
        myplyr->engine_window_width+w_delta, myplyr->engine_window_height+h_delta);
}

void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal)
{
    if (pal == blue_palette) // if the requested palette is the Freeze palette
    {
      if ((player->additional_flags & PlaAF_FreezePaletteIsActive) != 0)
        return; // Freeze palette is already on
      player->additional_flags |= PlaAF_FreezePaletteIsActive; // flag Freeze palette is active
    } else
    {
      player->additional_flags &= ~PlaAF_FreezePaletteIsActive; // flag Freeze palette is not active
    }
    if ( (player->lens_palette == 0) || ((pal != player->main_palette) && (pal == player->lens_palette)) )
    {
        player->main_palette = pal;
        player->palette_fade_step_pain = 0;
        player->palette_fade_step_possession = 0;
        if (is_my_player(player))
        {
            LbScreenWaitVbi();
            RendererPaletteSet(pal);
        }
    }
}

TbBool set_gamma(char corrlvl, TbBool do_set)
{
    char *fname;
    TbBool result = true;
    if (corrlvl < 0)
      corrlvl = 0;
    else
    if (corrlvl > 4)
      corrlvl = 4;
    settings.gamma_correction = corrlvl;
    fname=prepare_file_fmtpath(FGrp_StdData,"pal%05d.dat",settings.gamma_correction);
    if (!LbFileExists(fname))
    {
      WARNMSG("Palette file \"%s\" doesn't exist.", fname);
      result = false;
    }
    if (result)
    {
      result = (LbFileLoadAt(fname, engine_palette) != -1);
    }
    if ((result) && (do_set))
    {
      struct PlayerInfo *myplyr;
      myplyr=get_my_player();
      PaletteSetPlayerPalette(myplyr, engine_palette);
    }
    if (!result)
      ERRORLOG("Can't load palette file.");
    return result;
}

void centre_engine_window(void)
{
    long window_center_x;
    long window_center_y;
    struct PlayerInfo *player=get_my_player();
    if ((game.operation_flags & GOF_ShowGui) != 0)
      window_center_x = (MyScreenWidth-player->engine_window_width-status_panel_width) / 2 + status_panel_width;
    else
      window_center_x = (MyScreenWidth-player->engine_window_width) / 2;
    window_center_y = (MyScreenHeight-player->engine_window_height) / 2;
    setup_engine_window(window_center_x, window_center_y, player->engine_window_width, player->engine_window_height);
}

void turn_off_query(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    set_player_instance(player, PI_UnqueryCrtr, 0);
}

long filter_creatures_owned_by_keepers(const struct Thing *thing, MaxTngFilterParam, long)
{
    if (player_is_keeper(thing->owner)) {
        return INT32_MAX;
    }
    return -1;
}

void level_lost_go_first_person(PlayerNumber plyr_idx)
{
    struct CreatureControl *cctrl;
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *thing;
    ThingModel spectator_breed;
    SYNCDBG(6,"Starting for player %d",(int)plyr_idx);
    player = get_player(plyr_idx);
    dungeon = get_dungeon(player->id_number);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Unable to get player %d dungeon",(int)plyr_idx);
        return;
    }
    spectator_breed = get_players_spectator_model(plyr_idx);
    player->dungeon_camera_zoom = get_camera_zoom(get_player_active_camera(player));
    struct CompoundTngFilterParam param = {};
    param.class_id = TCls_Creature;
    struct Thing *spawn_creatng = get_random_thing_of_class_with_filter(filter_creatures_owned_by_keepers, &param, plyr_idx);
    if (!thing_exists(spawn_creatng)) {
        return;
    }
    struct Coord3d mappos = spawn_creatng->mappos;
    thing = create_and_control_creature_as_controller(player, spectator_breed, &mappos);
    if (thing_is_invalid(thing)) {
        ERRORLOG("Unable to create spectator creature");
        return;
    }
    move_creature_to_nearest_valid_position(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->creature_control_flags |= CCFlg_NoCompControl;
    SYNCDBG(8,"Finished");
}

void set_general_information(int32_t msg_id, PlayerNumber plyr_idx, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y)
{
    set_general_information_with_icon(
        msg_id,
        plyr_idx,
        target,
        x,
        y,
        -1);
}

void set_general_information_with_icon(int32_t msg_id, PlayerNumber plyr_idx, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y, short icon_idx)
{
    struct PlayerInfo *player = get_player(plyr_idx);
    MapCoord pos_x = 0;
    MapCoord pos_y = 0;
    find_map_location_coords(target, &x, &y, plyr_idx, __func__);
    if ((x != 0) || (y != 0))
    {
        pos_y = subtile_coord_center(y);
        pos_x = subtile_coord_center(x);
    }
    struct Event* event = event_create_event(pos_x, pos_y, EvKind_Information, player->id_number, -msg_id);
    if (!event_is_invalid(event))
        event->icon_idx = icon_idx;
}

void set_quick_information_with_icon(int32_t msg_id, PlayerNumber plyr_idx, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y, short icon_idx)
{
    struct PlayerInfo *player = get_player(plyr_idx);
    MapCoord pos_x = 0;
    MapCoord pos_y = 0;
    find_map_location_coords(target, &x, &y, plyr_idx, __func__);
    if ((x != 0) || (y != 0))
    {
        pos_y = subtile_coord_center(y);
        pos_x = subtile_coord_center(x);
    }
    struct Event* event = event_create_event(pos_x, pos_y, EvKind_QuickInformation, player->id_number, -msg_id);
    if (!event_is_invalid(event))
        event->icon_idx = icon_idx;
}

void set_quick_information(int32_t msg_id, PlayerNumber plyr_idx, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y)
{
    set_quick_information_with_icon(
        msg_id,
        plyr_idx,
        target,
        x,
        y,
        -1);
}

void set_general_objective(int32_t msg_id, PlayerNumber plyr_idx, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y)
{
    set_general_objective_with_icon(msg_id, plyr_idx, target, x, y, -1);
}

void set_general_objective_with_icon(int32_t msg_id, PlayerNumber plyr_idx, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y, short icon_idx)
{
    process_objective_with_icon(get_string(msg_id), plyr_idx, target, x, y, icon_idx);
}

void process_objective(const char *msg_text, PlayerNumber plyr_idx, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y)
{
    process_objective_with_icon(msg_text, plyr_idx, target, x, y, -1);
}

void process_objective_with_icon(const char *msg_text, PlayerNumber plyr_idx, TbMapLocation target, MapSubtlCoord x, MapSubtlCoord y, short icon_idx)
{
    struct PlayerInfo *player = get_player(plyr_idx);
    find_map_location_coords(target, &x, &y, plyr_idx, __func__);
    set_level_objective(player->id_number, msg_text);
    display_objectives_with_icon(player->id_number, x, y, icon_idx);
}

short winning_player_quitting(struct PlayerInfo *player, int32_t *plyr_count)
{
    struct PlayerInfo *swplyr;
    int i;
    int k;
    int n;
    if (player->victory_state == VicS_LostLevel)
    {
      return 0;
    }
    k = 0;
    n = 0;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      swplyr = get_player(i);
      if (player_exists(swplyr))
      {
        if (swplyr->is_active == 1)
        {
          k++;
          if (swplyr->victory_state == VicS_LostLevel)
            n++;
        }
      }
    }
    *plyr_count = k;
    return ((k - n) == 1);
}

short lose_level(struct PlayerInfo *player)
{
    if (!is_my_player(player))
        return false;
    if (network_is_active())
    {
        LbNetwork_Stop();
    }
    quit_game = 1;
    return true;
}

short resign_level(struct PlayerInfo *player)
{
    if (!is_my_player(player))
        return false;
    if (network_is_active())
    {
        LbNetwork_Stop();
    }
    quit_game = 1;
    return true;
}

short complete_level(struct PlayerInfo *player)
{
    SYNCDBG(6,"Starting");
    if (!is_my_player(player))
        return false;
    if (network_is_active())
    {
        LbNetwork_Stop();
        quit_game = 1;
        return true;
    }
    LevelNumber lvnum;
    lvnum = get_continue_level_number();
    if (get_loaded_level_number() == lvnum)
    {
        SYNCDBG(7,"Progressing the campaign");
        move_campaign_to_next_level();
    }
    quit_game = 1;
    return true;
}

static void set_mouse_light(struct PlayerInfo *player, TbBool valid, struct Coord3d pos)
{
    const int idx = player->cursor_light_idx;
    if (idx == 0)
        return;

    if (valid)
    {
        pos.z.val = get_floor_height_at(&pos);
        light_turn_light_on(idx);
        light_set_light_position(idx, &pos);

        if (is_my_player(player))
            game.mouse_light_pos = pos;
    }
    else
    {
        light_turn_light_off(idx);
    }
}

void update_local_mouse_light(void)
{
    SYNCDBG(6,"Starting");
    struct PlayerInfo *player = get_my_player();

    // Avoid glitching during level intro or possess animation
    if (player->instance_num != PI_Unset)
        return;
    // ... or when watching a replay
    if (game.packet_load_enable)
        return;
    // ... or during text input (save menu)
    if (game_is_busy_doing_gui_string_input())
        return;

    struct Camera *cam = get_local_camera(get_player_active_camera(player));
    struct Coord3d pos;
    const TbBool valid = screen_to_map(cam, GetMouseX(), GetMouseY(), &pos);

    set_mouse_light(player, valid, pos);

    if (player->cursor_light_idx != 0)
        light_reset_interpolation(player->cursor_light_idx);
}

void update_mouse_light(struct PlayerInfo *player)
{
    SYNCDBG(6,"Starting");
    const struct Packet *pckt = nullptr;

    if (is_my_player(player))
        pckt = get_history_packet(player->packet_num, get_gameturn());
    if (pckt == nullptr)
        pckt = get_packet_direct(player->packet_num);

    const TbBool valid = (pckt->control_flags & PCtr_MapCoordsValid) != 0;
    struct Coord3d pos;
    pos.x.val = pckt->pos_x;
    pos.y.val = pckt->pos_y;
    set_mouse_light(player, valid, pos);
}

void update_block_pointed(int i,long x, long x_frac, long y, long y_frac)
{
    struct Map *mapblk;
    struct Column *colmn;
    short visible;
    unsigned int smask;
    long k;

    if (i > 0)
    {
      mapblk = get_map_block_at(x,y);
      visible = map_block_revealed(mapblk, my_player_number);
      if ((!visible) || (get_mapblk_column_index(mapblk) > 0))
      {
        if (visible)
          k = get_mapblk_column_index(mapblk);
        else
          k = game.unrevealed_column_idx;
        colmn = get_column(k);
        smask = colmn->solidmask;
        if ((temp_cluedo_mode) && (smask != 0))
        {
          if (visible)
            k = get_mapblk_column_index(mapblk);
          else
            k = game.unrevealed_column_idx;
          colmn = get_column(k);
          if (colmn->solidmask >= 8)
          {
            if ( (!visible) || (((mapblk->flags & SlbAtFlg_IsRoom) == 0)) )
              smask &= 3;
          }
        }
        if (smask & (1 << (i-1)))
        {
          pointed_at_frac_x = x_frac;
          pointed_at_frac_y = y_frac;
          block_pointed_at_x = x;
          block_pointed_at_y = y;
          me_pointed_at = mapblk;
        }
        if (((!temp_cluedo_mode) && (i == 5)) || ((temp_cluedo_mode) && (i == 2)))
        {
          top_pointed_at_frac_x = x_frac;
          top_pointed_at_frac_y = y_frac;
          top_pointed_at_x = x;
          top_pointed_at_y = y;
        }
      }
    } else
    {
        mapblk = get_map_block_at(x,y);
        floor_pointed_at_x = x;
        floor_pointed_at_y = y;
        block_pointed_at_x = x;
        block_pointed_at_y = y;
        pointed_at_frac_x = x_frac;
        pointed_at_frac_y = y_frac;
        me_pointed_at = mapblk;
    }
}

void update_blocks_pointed(void)
{
    int32_t x;
    int32_t y;
    int32_t x_frac;
    int32_t y_frac;
    int64_t hori_ptr_y;
    int64_t vert_ptr_y;
    int64_t hori_hdelta_y;
    int64_t vert_hdelta_y;
    int64_t hori_ptr_x;
    int64_t vert_ptr_x;
    int64_t hvdiv_x;
    int64_t hvdiv_y;
    int64_t lltmp;
    int64_t k;
    int i;
    SYNCDBG(19,"Starting");
    if ((!vert_offset[1]) && (!hori_offset[1]))
    {
        block_pointed_at_x = 0;
        block_pointed_at_y = 0;
        me_pointed_at = INVALID_MAP_BLOCK;//get_map_block_at(0,0);
    } else
    {
        hori_ptr_y = (int64_t)hori_offset[0] * (pointer_y - y_init_off);
        vert_ptr_y = (int64_t)vert_offset[0] * (pointer_y - y_init_off);
        hori_hdelta_y = (int64_t)hori_offset[0] * ((long)high_offset[1] >> 8);
        vert_hdelta_y = (int64_t)vert_offset[0] * ((long)high_offset[1] >> 8);
        vert_ptr_x = ((int64_t)vert_offset[1] * (pointer_x - x_init_off)) >> 1;
        hori_ptr_x = ((int64_t)hori_offset[1] * (pointer_x - x_init_off)) >> 1;
        lltmp = hori_offset[0] * (int64_t)vert_offset[1] - vert_offset[0] * (int64_t)hori_offset[1];
        hvdiv_x = (lltmp >> 11);
        if (hvdiv_x == 0) hvdiv_x = 1;
        lltmp = vert_offset[0] * (int64_t)hori_offset[1] - hori_offset[0] * (int64_t)vert_offset[1];
        hvdiv_y = (lltmp >> 11);
        if (hvdiv_y == 0) hvdiv_y = 1;
        for (i=0; i < 8; i++)
        {
          k = (vert_ptr_x - (vert_ptr_y >> 1)) / hvdiv_x;
          x_frac = (k & 3) << 6;
          x = k >> 2;
          k = (hori_ptr_x - (hori_ptr_y >> 1)) / hvdiv_y;
          y_frac = (k & 3) << 6;
          y = k >> 2;
          if ((x >= 0) && (x < game.map_subtiles_x) && (y >= 0) && (y < game.map_subtiles_y))
          {
              update_block_pointed(i,x,x_frac,y,y_frac);
          }
          hori_ptr_y -= hori_hdelta_y;
          vert_ptr_y -= vert_hdelta_y;
        }
    }
    SYNCDBG(19,"Finished");
}

void engine(struct PlayerInfo *player, struct Camera *cam)
{
    TbGraphicsWindow grwnd;
    TbGraphicsWindow ewnd;
    unsigned short flg_mem;

    SYNCDBG(9,"Starting");

    flg_mem = RendererGetDrawFlags();
    update_engine_settings(player);
    mx = cam->mappos.x.val;
    my = cam->mappos.y.val;
    mz = cam->mappos.z.val;
    pointer_x = (GetMouseX() - player->engine_window_x) / pixel_size;
    pointer_y = (GetMouseY() - player->engine_window_y) / pixel_size;
    lens = cam->horizontal_fov * scale_value_by_horizontal_resolution(4) / pixel_size;
    if (lens_mode == 0)
        update_blocks_pointed();
    update_local_mouse_light();
    LbScreenStoreGraphicsWindow(&grwnd);
    store_engine_window(&ewnd,pixel_size);
    view_height_over_2 = ewnd.height/2;
    view_width_over_2 = ewnd.width/2;
    LbScreenSetGraphicsWindow(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
    setup_vecs(lbDisplay.GraphicsWindowPtr, 0, lbDisplay.GraphicsScreenWidth,
        ewnd.width, ewnd.height);
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    draw_view(cam, 0);
    RendererSetDrawFlags(flg_mem);
    thing_being_displayed = 0;
    LbScreenLoadGraphicsWindow(&grwnd);
}

void redetect_screen_refresh_rate_for_draw()
{
    fps_limit_current = 0;

    if (fps_limit_main == -1) {
        if (fps_limit_secondary > 0)
            fps_limit_current = fps_limit_secondary;

        int refresh_rate = PlatformManager_GetDisplayRefreshRate();
        if (refresh_rate > 0) {
            fps_limit_current = refresh_rate;
        }

    } else if (fps_limit_main > 0) {
        fps_limit_current = fps_limit_main;
    }
}

bool use_delta_time()
{
    // Always enable interpolation in multiplayer games.
    return is_feature_on(Ft_DeltaTime) || network_is_active();
}

void update_frontend_delta_time()
{
    static int64_t prev = 0;
    const int64_t now = get_time_tick_ns();
    const int64_t ns = now - prev;
    prev = now;
    const long double dt = ns / 1e9L * turns_per_second;
    game.delta_time = min(max(dt, 0.L), 1.L);
}

void update_gameplay_delta_time()
{
    if (use_delta_time()) {
        static int64_t prev = 0;
        const int64_t now = get_time_tick_ns();
        const int64_t ns = now - prev;
        prev = now;

        const long double seconds = max(ns / 1e9L, 0.L);
        const long double turns = seconds * turns_per_second;
        const long double frames = seconds * fps_limit_current;

        game.process_turn_time += turns * multiplayer_clock_adjust * max(game.frame_skip, 1);

        // This sets game.delta_time, which is used to pace locally-displayed
        // things (eg. tooltip scroll speed).  It should not be affected by
        // multiplayer clock adjustment or frameskip.
        time_since_last_draw += turns;

        // Like process_turn_time, but for the video frame rate.
        process_frame_time += frames;
    } else {
        // Set to 1 so that these variables don't affect anything. (if something is multiplied by 1 it doesn't change)
        time_since_last_draw = 1;
        game.delta_time = 1;
        game.process_turn_time = 1;
        process_frame_time = 1;
    }
}

void gameplay_loop_draw();

extern "C" void network_yield_draw_gameplay()
{
    gameplay_loop_draw();
}

extern "C" void update_velocity(void);
extern "C" void check_mouse_scroll(void);
extern "C" void fronttorture_update(void);

extern "C" void network_yield_draw_frontend()
{
    update_frontend_delta_time();
    if (frontend_menu_state == FeSt_NETLAND_VIEW) {
        check_mouse_scroll();
        update_velocity();
    }
    if (frontend_menu_state == FeSt_TORTURE) {
        fronttorture_update();
    }
    if (frontend_menu_state == FeSt_NET_START) {
        poll_inputs();
        frontnet_start_input();
    }
    frontend_draw();
    RendererPresentFrame();
}

TbBool can_thing_be_queried(struct Thing *thing, PlayerNumber plyr_idx)
{
    if ( (!thing_is_creature(thing)) || !( (thing->owner == plyr_idx) || (creature_is_kept_in_custody_by_player(thing, plyr_idx)) ) || (thing->alloc_flags & TAlF_IsInLimbo) || (thing->state_flags & TF1_InCtrldLimbo) || (thing->active_state == CrSt_CreatureUnconscious) )
    {
        return false;
    }
    unsigned char state = (thing->active_state == CrSt_MoveToPosition) ? thing->continue_state : thing->active_state;
    if ( (state == CrSt_CreatureSacrifice) || (state == CrSt_CreatureBeingSacrificed) || (state == CrSt_CreatureBeingSummoned) )
    {
        return false;
    }
    else
    {
        return true;
    }
}

static short process_command_line(unsigned short argc, char *argv[])
{
  char fullpath[CMDLN_MAXLEN+1];
  snprintf(fullpath, CMDLN_MAXLEN, "%s", argv[0]);
  snprintf(keeper_runtime_directory, sizeof(keeper_runtime_directory), "%s", fullpath);
  char *endpos = strrchr( keeper_runtime_directory, '\\');
  if (endpos==NULL)
      endpos=strrchr( keeper_runtime_directory, '/');
  if (endpos!=NULL)
      *endpos='\0';
  else
      strcpy(keeper_runtime_directory, ".");

  AssignCpuKeepers = 0;
  SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain

  set_default_startup_parameters();

  short bad_param;
  LevelNumber level_num;
  bad_param = 0;
  unsigned short narg;
  level_num = LEVELNUMBER_ERROR;
  TbBool one_player_mode = 0;
  narg = 1;
  char bad_params[TEXT_BUFFER_LENGTH] = "\0";
  while ( narg < argc )
  {
      char *par;
      par = argv[narg];
      if ( (par == NULL) || ((par[0] != '-') && (par[0] != '/')) )
          return -1;
      char parstr[CMDLN_MAXLEN+1];
      char pr2str[CMDLN_MAXLEN+1];
      char pr3str[CMDLN_MAXLEN+1];
      snprintf(parstr, CMDLN_MAXLEN, "%s", par + 1);
      if (narg + 1 < argc)
      {
          snprintf(pr2str, CMDLN_MAXLEN, "%s", argv[narg + 1]);
          if (narg + 2 < argc)
              snprintf(pr3str, CMDLN_MAXLEN, "%s", argv[narg + 2]);
          else
              pr3str[0]='\0';
      }
      else
      {
          pr2str[0]='\0';
          pr3str[0]='\0';
      }

      if (strcasecmp(parstr, "nointro") == 0)
      {
        start_params.no_intro = true;
      } else
      if (strcasecmp(parstr, "nocd") == 0) // kept for legacy reasons
      {
          WARNLOG("The -nocd commandline parameter is no longer functional. Game music from CD is a setting in keeperfx.cfg instead.");
      } else
      if (strcasecmp(parstr, "columnconvert") == 0) //todo remove once it's no longer in the launcher
      {
          WARNLOG("The -%s commandline parameter is no longer functional.", parstr);
      }
      else
      if (strcasecmp(parstr, "cd") == 0)
      {
          start_params.overrides[Clo_CDMusic] = true;
      } else
      if (strcasecmp(parstr, "1player") == 0)
      {
          start_params.one_player = true;
          one_player_mode = true;
      } else
      if ((strcasecmp(parstr, "s") == 0) || (strcasecmp(parstr, "nosound") == 0))
      {
          SoundDisabled = true;
      } else
      if (strcasecmp(parstr, "fps") == 0)
      {
          narg++;
          start_params.num_fps = atoi(pr2str);
          start_params.overrides[Clo_GameTurns] = true;
      } else
      if (strcasecmp(parstr, "fps_draw") == 0)
      {
          narg++;
	  if (parse_draw_fps_config_val(pr2str, &start_params.num_fps_draw_main, &start_params.num_fps_draw_secondary) > 0)
            start_params.overrides[Clo_FramesPerSecond] = true;
      } else
      if (strcasecmp(parstr, "human") == 0)
      {
          narg++;
          default_loc_player = atoi(pr2str);
          force_player_num = true;
      } else
      if (strcasecmp(parstr, "vidsmooth") == 0)
      {
          smooth_on = true;
      } else
      if ( strcasecmp(parstr,"level") == 0 )
      {
        set_flag(start_params.operation_flags, GOF_SingleLevel);
        level_num = atoi(pr2str);
        autostart_multiplayer_level = atoi(pr2str);
        narg++;
      } else
      if ( strcasecmp(parstr,"campaign") == 0 )
      {
        strcpy(start_params.selected_campaign, pr2str);
        strcpy(autostart_multiplayer_campaign, pr2str);
        narg++;
      } else
      if ( strcasecmp(parstr,"altinput") == 0 )
      {
          SYNCLOG("Mouse auto reset disabled");
          lbMouseGrab = false;
      }
      else if (strcasecmp(parstr,"packetload") == 0)
      {
         if (start_params.packet_save_enable)
            WARNMSG("PacketSave disabled to enable PacketLoad.");
         start_params.packet_load_enable = true;
         start_params.packet_save_enable = false;
         snprintf(start_params.packet_fname, sizeof(start_params.packet_fname), "%s", pr2str);
         set_flag(start_params.debug_flags, DFlg_ShowGameTurns | DFlg_FrameStep);
         narg++;
      } else
      if (strcasecmp(parstr,"packetsave") == 0)
      {
         if (start_params.packet_load_enable)
            WARNMSG("PacketLoad disabled to enable PacketSave.");
         start_params.packet_load_enable = false;
         start_params.packet_save_enable = true;
         snprintf(start_params.packet_fname, sizeof(start_params.packet_fname), "%s", pr2str);
         narg++;
      } else
      if (strcasecmp(parstr,"pause_at_gameturn") == 0)
      {
         set_flag(start_params.debug_flags, DFlg_ShowGameTurns | DFlg_FrameStep | DFlg_PauseAtGameTurn);
         start_params.pause_at_gameturn = atoi(pr2str);
         narg++;
      } else
      if (strcasecmp(parstr,"q") == 0)
      {
         set_flag(start_params.operation_flags, GOF_SingleLevel);
      } else
      if (strcasecmp(parstr,"lightconvert") == 0)
      {
         WARNLOG("The -%s commandline parameter is no longer functional.", parstr); //todo remove once it's no longer in the launcher
      } else
      if (strcasecmp(parstr, "dbgshots") == 0)
      {
          set_flag(start_params.debug_flags, DFlg_ShotsDamage);
      } else
      if (strcasecmp(parstr, "dbgpathfind") == 0)
      {
          set_flag(start_params.debug_flags, DFlg_CreatrPaths);
      } else
      if (strcasecmp(parstr, "show_game_turns") == 0)
      {
          set_flag(start_params.debug_flags, DFlg_ShowGameTurns);
      } else
      if (strcasecmp(parstr, "mplog") == 0)
      {
          detailed_multiplayer_logging = true;
      } else
      if (strcasecmp(parstr, "netstats") == 0)
      {
          debug_display_network_stats = 1;
      } else
      if (strcasecmp(parstr, "compuchat") == 0)
      {
          if (strcasecmp(pr2str,"scarce") == 0) {
              start_params.computer_chat_flags = CChat_TasksScarce;
          } else
          if (strcasecmp(pr2str,"frequent") == 0) {
              start_params.computer_chat_flags = CChat_TasksScarce|CChat_TasksFrequent;
          } else {
              start_params.computer_chat_flags = CChat_None;
          }
          narg++;
      } else
      if (strcasecmp(parstr, "sessions") == 0) {
          narg++;
          LbNetwork_InitSessionsFromCmdLine(pr2str);
      } else
      if (strcasecmp(parstr, "nomods") == 0) {
          start_params.ignore_mods = true;
      } else
      if (strcasecmp(parstr,"alex") == 0)
      {
         start_params.easter_egg = true;
      }
      else if (strcasecmp(parstr,"connect") == 0)
      {
          narg++;
          LbNetwork_InitSessionsFromCmdLine(pr2str);
          game_flags2 |= GF2_Connect;
      }
      else if (strcasecmp(parstr,"waitusers") == 0)
      {
          autostart_multiplayer_users_expected = clamp(atoi(pr2str), MIN_NET_USERS, MAX_NET_USERS);
          narg++;
      }
      else if (strcasecmp(parstr,"server") == 0)
      {
          game_flags2 |= GF2_Server;
          int port = atoi(pr2str);
          if (port > 0)
          {
              LbNetwork_SetServerPort(port);
              narg++;
          }
      }
      else if (strcasecmp(parstr, "nick") == 0)
      {
          if (pr2str[0])
          {
              snprintf(net_player_name, sizeof(net_player_name), "%s", pr2str);
              snprintf(tmp_net_player_name, sizeof(net_player_name), "%s", pr2str);
              narg++;
          }
          else
          {
              WARNMSG("No player name given after -nick");
          }
      }
      else if (strcasecmp(parstr,"frameskip") == 0)
      {
         start_params.frame_skip = atoi(pr2str);
         narg++;
      } else
      if (strcasecmp(parstr,"framestep") == 0)
      {
         set_flag(start_params.debug_flags, DFlg_ShowGameTurns | DFlg_FrameStep);
      }
      else if (strcasecmp(parstr, "timer") == 0)
      {
          game_flags2 |= GF2_Timer;
          if (strcasecmp(pr2str, "game") == 0)
          {
              TimerGame = true;
              narg++;
          }
          else if (strcasecmp(pr2str, "continuous") == 0)
          {
              TimerNoReset = true;
              narg++;
          }
      }
      else if ( strcasecmp(parstr,"config") == 0 )
      {
        strcpy(start_params.config_file, pr2str);
        start_params.overrides[Clo_ConfigFile] = true;
        narg++;
      }
      else if ( strcasecmp(parstr,"Bullfrog") == 0 ) // force playing the Bullfrog video
      {
        set_flag(start_params.startup_flags, SFlg_Bullfrog);
      }
      else if ( strcasecmp(parstr,"ea") == 0 ) // force playing the EA video
      {
        set_flag(start_params.startup_flags, SFlg_EA);
      }
      else if (strcasecmp(parstr, "ftests") == 0)
      {
#ifdef FUNCTESTING
        if(ftest_parse_arg(pr2str)) // handle arg on ftest build
#else
        if(strlen(pr2str) > 0 && pr2str[0] != '-') // ignore arg on regular build
#endif // FUNCTESTING
        {
            ++narg;
        }

#ifdef FUNCTESTING
        set_flag(start_params.functest_flags, FTF_Enabled);
#else
        WARNLOG("Flag '%s' disabled for release builds.", parstr);
#endif // FUNCTESTING
      }
      else if (strcasecmp(parstr, "log") == 0)
      {
          narg++;
      }
      else if(strcasecmp(parstr, "exitonfailedtest") == 0)
      {
#ifdef FUNCTESTING
        set_flag(start_params.functest_flags, FTF_ExitOnTestFailure);
#else
       WARNLOG("Flag '%s' disabled for release builds.", parstr);
#endif // FUNCTESTING
      }
      else if(strcasecmp(parstr, "includelongtests") == 0)
      {
#ifdef FUNCTESTING
        set_flag(start_params.functest_flags, FTF_IncludeLongTests);
#else
       WARNLOG("Flag '%s' disabled for release builds.", parstr);
#endif // FUNCTESTING
      }
      else
      {
        // append bad parstr to bad_params string
        char param_buffer[128] = "";
        snprintf(param_buffer, sizeof(param_buffer), "%s%s", strnlen(bad_params, TEXT_BUFFER_LENGTH) > 0 ? ", " : "" , parstr);
        str_append(bad_params, sizeof(bad_params), param_buffer);
        bad_param=narg;
      }
      narg++;
  }

  if (level_num == LEVELNUMBER_ERROR)
  {
      if (first_singleplayer_level() > 0)
      {
          level_num = first_singleplayer_level();
      }
      else
      {
          level_num = 1;
      }
  }
  else {
      if (one_player_mode) {
          AssignCpuKeepers = 1;
      }
  }
  start_params.selected_level_number = level_num;
  my_player_number = default_loc_player;

#ifdef FUNCTESTING
  ftest_init(); // initialise test framework on ftest build
#endif

  if(bad_param != 0)
  {
    char message[TEXT_BUFFER_LENGTH];
    snprintf(message, sizeof(message), "Incorrect command line parameters: '%s'.\nPlease correct your Run options.", bad_params);
    warning_dialog(__func__, 0, message);
  }

  return (bad_param==0);
}

static const char* determine_log_filename(unsigned short argument_count, char *argument_values[])
{
    for (int argument_index = 1; argument_index < argument_count; argument_index++) {
        if (argument_values[argument_index] && (argument_values[argument_index][0] == '-' || argument_values[argument_index][0] == '/')) {
            char* argument_name = argument_values[argument_index] + 1;
            if (strcasecmp(argument_name, "log") == 0 && argument_index + 1 < argument_count) {
                remove("keeperfx.log");
                return argument_values[argument_index + 1];
            }
        }
    }
    return log_file_name;
}

static short reset_game(void)
{
    SYNCDBG(6,"Starting");

    LbMouseSuspend();
    LbIKeyboardClose();
    RendererResetScreen(false);
    LbDataFreeAllV2(game_load_files);
    free_gui_strings_data();
    free_level_strings_data();
    FreeAudio();
    return 1;
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
    short retval;
    retval=0;

    // Determine correct log file based on command line flags
    const char* selected_log_file_name = determine_log_filename(argc, argv);
    LbErrorLogSetup("/", selected_log_file_name, 5);

    retval = process_command_line(argc,argv);
    if (retval < 1)
    {
        LbErrorLogClose();
        return 0;
    }

    retval = true;
    retval &= (LbTimerInit() != Lb_FAIL);
    retval &= (RendererScreenInitialize() != Lb_FAIL);
    retval &= (RendererInit(RENDERER_SOFTWARE) != 0);
    LbSetTitle(PROGRAM_NAME);
    LbSetIcon(1);
    RendererSetDoubleBuffering(true);
    srand(LbTimerClock());

#ifdef FUNCTESTING
    ftest_srand();
#endif // FUNCTESTING

    if (!retval)
    {
        static const char *msg_text="Basic engine initialization failed.\n";
        error_dialog_fatal(__func__, 1, msg_text);
        LbErrorLogClose();
        return 0;
    }

    retval = setup_game();
    if (retval == 1)
    {
        steam_api_init();
    }
    if (retval == 1)
    {
        if (is_dbc_language(install_info.lang_id))
        {            
            dbc_initialized = 1;
        }
        load_unifont_files();
    }
    if ( retval == 1 )
    {
        api_init_server();
        game_loop();
    }
    reset_game();
    RendererResetScreen(true);
    RendererShutdown();
    if ( retval == 0 )
    {
        static const char *msg_text="Setting up game failed.\n";
        error_dialog_fatal(__func__, 2, msg_text);
    } else
    if (retval == -1)
    {
        static const char* msg_text = " Game files which have to be copied from original DK are not present.\n\n";
        error_dialog_fatal(__func__, 2, msg_text);
    }
    else
    {
        SYNCDBG(0,"finished properly");
    }

    steam_api_shutdown();
    LbErrorLogClose();
    return 0;
}

int kfxmain(int argc, char *argv[])
{
  try {
  LbBullfrogMain(argc, argv);
  } catch (...)
  {
      error_dialog(__func__, 1, "Exception raised!");
      return 1;
  }

#ifdef FUNCTESTING
  TbBool should_report_failure = flag_is_set(start_params.functest_flags, FTF_TestFailed) && flag_is_set(start_params.functest_flags, FTF_ExitOnTestFailure);
  if(flag_is_set(start_params.functest_flags, FTF_Enabled) && (flag_is_set(start_params.functest_flags, FTF_Abort) || should_report_failure))
  {
      return -1;
  }
#endif

  return 0;
}

#ifdef __cplusplus
}
#endif
