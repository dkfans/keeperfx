/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_instances.c
 *     Player instances support and switching code.
 * @par Purpose:
 *     Supports various states of a player, and switching between them.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "player_instances.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_datetm.h"
#include "bflib_math.h"
#include "bflib_sound.h"
#include "bflib_planar.h"

#include "creature_control.h"
#include "creature_states.h"
#include "creature_graphics.h"
#include "creature_instances.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_powerhands.h"
#include "thing_stats.h"
#include "light_data.h"
#include "thing_effects.h"
#include "thing_navigate.h"
#include "thing_traps.h"
#include "thing_physics.h"
#include "front_simple.h"
#include "frontend.h"
#include "power_hand.h"
#include "player_utils.h"
#include "config_players.h"
#include "room_workshop.h"
#include "magic_powers.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "engine_arrays.h"
#include "engine_camera.h"
#include "local_camera.h"
#include "engine_redraw.h"
#include "sounds.h"
#include "config_settings.h"
#include "config_terrain.h"
#include "game_legacy.h"
#include "config_magic.h"
#include "thing_shots.h"
#include "bflib_inputctrl.h"
#include "map_blocks.h"
#include "lua_triggers.h"
#include "lens_api.h"

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long pinstfs_hand_grab(struct PlayerInfo *player, int32_t *n);
long pinstfe_hand_grab(struct PlayerInfo *player, int32_t *n);
long pinstfs_hand_drop(struct PlayerInfo *player, int32_t *n);
long pinstfe_hand_drop(struct PlayerInfo *player, int32_t *n);
long pinstfs_hand_whip(struct PlayerInfo *player, int32_t *n);
long pinstfe_hand_whip(struct PlayerInfo *player, int32_t *n);
long pinstfs_hand_whip_end(struct PlayerInfo *player, int32_t *n);
long pinstfe_hand_whip_end(struct PlayerInfo *player, int32_t *n);
long pinstfs_direct_control_creature(struct PlayerInfo *player, int32_t *n);
long pinstfs_passenger_control_creature(struct PlayerInfo *player, int32_t *n);
long pinstfm_control_creature(struct PlayerInfo *player, int32_t *n);
long pinstfe_direct_control_creature(struct PlayerInfo *player, int32_t *n);
long pinstfe_passenger_control_creature(struct PlayerInfo *player, int32_t *n);
long pinstfs_direct_leave_creature(struct PlayerInfo *player, int32_t *n);
long pinstfm_leave_creature(struct PlayerInfo *player, int32_t *n);
long pinstfs_passenger_leave_creature(struct PlayerInfo *player, int32_t *n);
long pinstfe_leave_creature(struct PlayerInfo *player, int32_t *n);
long pinstfs_query_creature(struct PlayerInfo *player, int32_t *n);
long pinstfs_unquery_creature(struct PlayerInfo *player, int32_t *n);
long pinstfs_zoom_to_heart(struct PlayerInfo *player, int32_t *n);
long pinstfm_zoom_to_heart(struct PlayerInfo *player, int32_t *n);
long pinstfe_zoom_to_heart(struct PlayerInfo *player, int32_t *n);
long pinstfs_zoom_out_of_heart(struct PlayerInfo *player, int32_t *n);
long pinstfm_zoom_out_of_heart(struct PlayerInfo *player, int32_t *n);
long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, int32_t *n);
long pinstfm_control_creature_fade(struct PlayerInfo *player, int32_t *n);
long pinstfe_control_creature_fade(struct PlayerInfo *player, int32_t *n);
long pinstfs_fade_to_map(struct PlayerInfo *player, int32_t *n);
long pinstfm_fade_to_map(struct PlayerInfo *player, int32_t *n);
long pinstfe_fade_to_map(struct PlayerInfo *player, int32_t *n);
long pinstfs_fade_from_map(struct PlayerInfo *player, int32_t *n);
long pinstfm_fade_from_map(struct PlayerInfo *player, int32_t *n);
long pinstfe_fade_from_map(struct PlayerInfo *player, int32_t *n);
long pinstfs_zoom_to_position(struct PlayerInfo *player, int32_t *n);
long pinstfm_zoom_to_position(struct PlayerInfo *player, int32_t *n);
long pinstfe_zoom_to_position(struct PlayerInfo *player, int32_t *n);

struct PlayerInstanceInfo player_instance_info[PLAYER_INSTANCES_COUNT] = {
  { 0, 0, NULL,                                 NULL,                           NULL,                                {0}, {0}, 0, 0}, // PI_Unset
  { 3, 1, pinstfs_hand_grab,                    NULL,                           pinstfe_hand_grab,                   {0}, {0}, 0, 0}, // PI_Grab
  { 3, 1, pinstfs_hand_drop,                    NULL,                           pinstfe_hand_drop,                   {0}, {0}, 0, 0}, // PI_Drop
  { 4, 0, pinstfs_hand_whip,                    NULL,                           pinstfe_hand_whip,                   {0}, {0}, 0, 0}, // PI_Whip
  { 5, 0, pinstfs_hand_whip_end,                NULL,                           pinstfe_hand_whip_end,               {0}, {0}, 0, 0}, // PI_WhipEnd
  {12, 1, pinstfs_direct_control_creature,      pinstfm_control_creature,       pinstfe_direct_control_creature,     {0}, {0}, 0, 0}, // PI_DirctCtrl
  {12, 1, pinstfs_passenger_control_creature,   pinstfm_control_creature,       pinstfe_passenger_control_creature,  {0}, {0}, 0, 0}, // PI_PsngrCtrl
  {12, 1, pinstfs_direct_leave_creature,        pinstfm_leave_creature,         pinstfe_leave_creature,              {0}, {0}, 0, 0}, // PI_DirctCtLeave
  {12, 1, pinstfs_passenger_leave_creature,     pinstfm_leave_creature,         pinstfe_leave_creature,              {0}, {0}, 0, 0}, // PI_PsngrCtLeave
  { 0, 1, pinstfs_query_creature,               NULL,                           NULL,                                {0}, {0}, 0, 0}, // PI_QueryCrtr
  { 0, 1, pinstfs_unquery_creature,             NULL,                           NULL,                                {0}, {0}, 0, 0}, // PI_UnqueryCrtr
  {16, 1, pinstfs_zoom_to_heart,                pinstfm_zoom_to_heart,          pinstfe_zoom_to_heart,               {0}, {0}, 0, 0}, // PI_HeartZoom
  {16, 1, pinstfs_zoom_out_of_heart,            pinstfm_zoom_out_of_heart,      pinstfe_zoom_out_of_heart,           {0}, {0}, 0, 0}, // PI_HeartZoomOut
  {12, 1, NULL,                                 pinstfm_control_creature_fade,  pinstfe_control_creature_fade,       {0}, {0}, 0, 0}, // PI_CrCtrlFade
  { 8, 1, pinstfs_fade_to_map,                  pinstfm_fade_to_map,            pinstfe_fade_to_map,                 {0}, {0}, 0, 0}, // PI_MapFadeTo
  { 8, 1, pinstfs_fade_from_map,                pinstfm_fade_from_map,          pinstfe_fade_from_map,               {0}, {0}, 0, 0}, // PI_MapFadeFrom
  {-1, 1, pinstfs_zoom_to_position,             pinstfm_zoom_to_position,       pinstfe_zoom_to_position,            {0}, {0}, 0, 0}, // PI_ZoomToPos
  { 0, 0, NULL,                                 NULL,                           NULL,                                {0}, {0}, 0, 0}, // PI_UnusedSlot17
  { 0, 0, NULL,                                 NULL,                           NULL,                                {0}, {0}, 0, 0}, // PI_UnusedSlot18
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
long pinstfs_hand_grab(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->hand_thing_idx);
    if (!thing_is_invalid(thing))
    {
        set_power_hand_graphic(player->id_number, HndA_Pickup);
    }
    return 0;
}

long pinstfe_hand_grab(struct PlayerInfo *player, int32_t *n)
{
    SYNCDBG(8,"Starting");
    struct Thing* dsttng = thing_get(player->influenced_thing_idx);
    if (dsttng->creation_turn != player->influenced_thing_creation) {
        WARNLOG("The thing index %d is no longer the same",(int)player->influenced_thing_idx);
        player->influenced_thing_creation = 0;
        player->influenced_thing_idx = 0;
        return 0;
    }
    player->influenced_thing_creation = 0;
    player->influenced_thing_idx = 0;
    if (magic_use_available_power_on_thing(player->id_number, PwrK_HAND, 0,dsttng->mappos.x.stl.num, dsttng->mappos.y.stl.num, dsttng, PwMod_Default) == Lb_FAIL) {
        WARNLOG("Cannot pick up %s index %d",thing_model_name(dsttng),(int)dsttng->index);
        return 0;
    }
    struct Thing* handtng = thing_get(player->hand_thing_idx);
    if (thing_exists(handtng))
    {
        set_power_hand_graphic(player->id_number, HndA_Pickup);
    }
    return 0;
}

long pinstfs_hand_drop(struct PlayerInfo *player, int32_t *n)
{
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct Thing* thing = thing_get(player->hand_thing_idx);
    player->influenced_thing_idx = dungeon->things_in_hand[0];
    player->influenced_thing_creation = thing->creation_turn;
    if (thing_exists(thing))
    {
        set_power_hand_graphic(player->id_number, HndA_Pickup);
    }
    return 0;
}

long pinstfe_hand_drop(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->hand_thing_idx);
    if (thing_exists(thing))
    {
        set_power_hand_graphic(player->id_number, HndA_Hover);
    }
    player->influenced_thing_idx = 0;
    player->influenced_thing_creation = 0;
    return 0;
}

long pinstfs_hand_whip(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->hand_thing_idx);
    if (thing_exists(thing))
    {
        set_power_hand_graphic(player->id_number, HndA_Slap);
    }
    return 0;
}

long pinstfe_hand_whip(struct PlayerInfo *player, int32_t *n)
{
    struct PowerConfigStats* powerst = get_power_model_stats(PwrK_SLAP);
    struct Thing* thing = thing_get(player->influenced_thing_idx);
    struct TrapConfigStats *trapst;
    struct ShotConfigStats* shotst;
    if (!thing_exists(thing) || (thing->creation_turn != player->influenced_thing_creation) || (!thing_slappable(thing, player->id_number)))
    {
        player->influenced_thing_creation = 0;
        player->influenced_thing_idx = 0;
        return 0;
  }
  switch (thing->class_id)
  {
  case TCls_Creature:
  {
      struct Coord3d pos;
      if (creature_under_spell_effect(thing, CSAfF_Freeze))
      {
          kill_creature(thing, INVALID_THING, thing->owner, CrDed_Default);
      } else
      {
          slap_creature(player, thing);
          pos.x.val = thing->mappos.x.val;
          pos.y.val = thing->mappos.y.val;
          pos.z.val = thing->mappos.z.val + (thing->clipbox_size_z >> 1);
          if ( creature_model_bleeds(thing->model) )
              create_effect(&pos, TngEff_HitBleedingUnit, thing->owner);
          thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
          struct Camera* cam = player->acamera;
          if (cam != NULL)
          {
            thing->veloc_base.x.val += distance_with_angle_to_coord_x(64, cam->rotation_angle_x);
            thing->veloc_base.y.val += distance_with_angle_to_coord_y(64, cam->rotation_angle_x);
          }
      }
      break;
  }
  case TCls_Shot:
      shotst = get_shot_model_stats(thing->model);
      if (shotst->model_flags & ShMF_Boulder)
      {
          thing->move_angle_xy = player->acamera->rotation_angle_x;
          if (thing->model != ShM_SolidBoulder) // TODO CONFIG shot model dependency, make config option instead.
          {
              thing->health -= game.conf.rules[thing->owner].game.boulder_reduce_health_slap;
          }
      }
      else
      {
          detonate_shot(thing,true);
      }
      break;
  case TCls_Trap:
      trapst = get_trap_model_stats(thing->model);
      if ((trapst->slappable > 0) && trap_is_active(thing))
      {
          activate_trap_by_slap(player, thing);

          struct Dungeon* dungeon = get_dungeon(thing->owner);
          if (!dungeon_invalid(dungeon))
          {
              dungeon->trap_info.activated[thing->trap.flag_number]++;
              if (thing->trap.flag_number > 0)
              {
                  memcpy(&dungeon->last_trap_event_location, &thing->mappos, sizeof(struct Coord3d));
              }
          }
          process_trap_charge(thing);
      }
      break;
      case TCls_Object:
      {
          struct Thing* efftng;
          if (object_is_slappable(thing, player->id_number))
          {
            efftng = create_effect(&thing->mappos, TngEff_Dummy, thing->owner);
            if (!thing_is_invalid(efftng))
              thing_play_sample(efftng, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 3, FULL_LOUDNESS);
            slap_object(thing);
          }
          break;
      }
  }
  set_player_instance(player, PI_WhipEnd, false);
  return 0;
}

long pinstfs_hand_whip_end(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->hand_thing_idx);
    if (thing_exists(thing))
    {
        set_power_hand_graphic(player->id_number, HndA_SideSlap);
    }
    return 0;
}

long pinstfe_hand_whip_end(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->hand_thing_idx);
    if (thing_exists(thing))
    {
        set_power_hand_graphic(player->id_number, HndA_SideHover);
    }
    return 0;
}

long pinstfs_passenger_control_creature(struct PlayerInfo *player, int32_t *n)
{
  player->allocflags |= PlaF_MouseInputDisabled;
  if (is_my_player(player))
  {
    player->palette_fade_step_possession = 1;
    turn_off_all_window_menus();
    turn_off_menu(GMnu_CREATURE_QUERY1);
    turn_off_menu(GMnu_CREATURE_QUERY2);
  }
  struct Camera* cam = player->acamera;
  player->allocflags |= PlaF_KeyboardInputDisabled;
  player->dungeon_camera_zoom = get_camera_zoom(cam);
  // Play possession sound
  if (is_my_player(player))
  {
      struct PowerConfigStats* powerst = get_power_model_stats(PwrK_POSSESS);
      play_non_3d_sample(powerst->select_sound_idx);
  }
  return 0;
}

long pinstfs_direct_control_creature(struct PlayerInfo *player, int32_t *n)
{
    // Reset state of the thing being possessed
    struct Thing* thing = thing_get(player->influenced_thing_idx);
    if (thing_can_be_controlled_as_controller(thing))
    {
        SYNCDBG(8,"Cleaning up state %s of %s index %d",creature_state_code_name(thing->active_state),thing_model_name(thing),(int)thing->index);
        initialise_thing_state(thing, CrSt_ManualControl);
        LbGrabMouseCheck(MG_OnPossessionEnter);
    }
    return pinstfs_passenger_control_creature(player, n);
}

long pinstfm_control_creature(struct PlayerInfo *player, int32_t *n)
{
    struct Camera* cam = player->acamera;
    if (cam == NULL)
        return 0;
    struct Thing* thing = thing_get(player->influenced_thing_idx);
    if (!thing_exists(thing) || (thing->class_id == TCls_DeadCreature) || creature_is_dying(thing))
    {
        set_camera_zoom(cam, player->dungeon_camera_zoom);
        if (is_my_player(player))
            PaletteSetPlayerPalette(player, engine_palette);
        player->influenced_thing_idx = 0;
        player->influenced_thing_creation = 0;
        player->allocflags &= ~PlaF_KeyboardInputDisabled;
        player->allocflags &= ~PlaF_MouseInputDisabled;
        set_player_instance(player, PI_Unset, true);
        return 0;
    }
    if (player->view_mode != PVM_FrontView)
    {
        view_zoom_camera_in(cam, 30000, 0);
        // Compute new camera angle
        long mv_a = (thing->move_angle_xy - cam->rotation_angle_x) & ANGLE_MASK;
        if (mv_a > DEGREES_180)
          mv_a -= DEGREES_360;
        if (mv_a < -DEGREES_30)
        {
            mv_a = -DEGREES_30;
        } else
        if (mv_a > DEGREES_30)
        {
            mv_a = DEGREES_30;
        }
        cam->rotation_angle_x += mv_a;
        cam->rotation_angle_x &= ANGLE_MASK;
        // Now mv_a becomes a circle radius
        mv_a = get_creature_eye_height(thing) + thing->mappos.z.val;
        long mv_x = thing->mappos.x.val + distance_with_angle_to_coord_x(mv_a, cam->rotation_angle_x) - (MapCoordDelta)cam->mappos.x.val;
        long mv_y = thing->mappos.y.val + distance_with_angle_to_coord_y(mv_a, cam->rotation_angle_x) - (MapCoordDelta)cam->mappos.y.val;
        if (mv_x < -128)
        {
            mv_x = -128;
        } else
        if (mv_x > 128)
        {
            mv_x = 128;
        }
        if (mv_y < -128)
        {
            mv_y = -128;
        } else
        if (mv_y > 128)
        {
            mv_y = 128;
        }
        cam->mappos.x.val += mv_x;
        cam->mappos.y.val += mv_y;
        if (cam->rotation_angle_x < 0)
        {
          cam->rotation_angle_x += DEGREES_360;
        }
        if (cam->rotation_angle_x >= DEGREES_360)
        {
          cam->rotation_angle_x -= DEGREES_360;
        }
        set_local_camera_destination(player);
    }
    return 0;
}

long pinstfe_direct_control_creature(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->influenced_thing_idx);
    if (thing_exists(thing) && (thing->creation_turn == player->influenced_thing_creation))
    {
        if (!control_creature_as_controller(player, thing)) {
            thing = INVALID_THING;
        }
    }
    if (!thing_exists(thing))
    {
        set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
        if (is_my_player(player)) {
            PaletteSetPlayerPalette(player, engine_palette);
        }
        player->allocflags &= ~PlaF_KeyboardInputDisabled;
        player->allocflags &= ~PlaF_MouseInputDisabled;
        return 0;
    }
    set_player_instance(player, PI_CrCtrlFade, false);
    TbBool my_player = (is_my_player(player));
    if (thing->class_id == TCls_Creature)
    {
        load_swipe_graphic_for_creature(thing);
        if (my_player)
        {
            if (creature_under_spell_effect(thing, CSAfF_Freeze))
            {
                PaletteSetPlayerPalette(player, blue_palette);
            }
        }
        creature_choose_first_available_instance(thing);
    }
    if (my_player)
    {
        turn_on_menu(GMnu_CREATURE_QUERY1);
    }
    return 0;
}

long pinstfe_passenger_control_creature(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->influenced_thing_idx);
    if (thing_exists(thing))
    {
        load_swipe_graphic_for_creature(thing);
        control_creature_as_passenger(player, thing);
    }
    set_player_instance(player, PI_CrCtrlFade, false);
    if (is_my_player(player))
    {
        if (thing->class_id == TCls_Creature)
        {
            turn_on_menu(GMnu_CREATURE_QUERY1);
        }
    }
    return 0;
}

long pinstfs_direct_leave_creature(struct PlayerInfo *player, int32_t *n)
{
  if (player->influenced_thing_idx == 0)
  {
    set_player_instance(player, PI_Unset, true);
    return 0;
  }
  player->allocflags |= PlaF_MouseInputDisabled;
  struct Thing* thing = thing_get(player->influenced_thing_idx);
  reset_creature_eye_lens(thing);
  if (is_my_player(player))
  {
      PaletteSetPlayerPalette(player, engine_palette);
      player->palette_fade_step_possession = 11;
      turn_off_all_window_menus();
      turn_off_query_menus();
      turn_on_main_panel_menu();
      set_flag_value(game.operation_flags, GOF_ShowPanel, (game.operation_flags & GOF_ShowGui) != 0);
      LbGrabMouseCheck(MG_OnPossessionLeave);
  }
  thing = thing_get(player->influenced_thing_idx);
  leave_creature_as_controller(player, thing);
  player->allocflags |= PlaF_KeyboardInputDisabled;
  player->influenced_thing_idx = 0;
  player->influenced_thing_creation = 0;
  light_turn_light_on(player->cursor_light_idx);
  play_non_3d_sample(177);
  return 0;
}

long pinstfm_leave_creature(struct PlayerInfo *player, int32_t *n)
{
    if (player->view_mode != PVM_FrontView)
    {
        view_zoom_camera_out(player->acamera, 30000, 0);
        if (get_camera_zoom(player->acamera) < player->dungeon_camera_zoom) {
            set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
        }
        set_local_camera_destination(player);
    }
    return 0;
}

long pinstfs_passenger_leave_creature(struct PlayerInfo *player, int32_t *n)
{
  if (player->influenced_thing_idx == 0)
  {
      set_player_instance(player, PI_Unset, true);
      return 0;
  }
  player->allocflags |= PlaF_MouseInputDisabled;
  struct Thing* thing = thing_get(player->influenced_thing_idx);
  reset_creature_eye_lens(thing);
  if (is_my_player(player))
  {
    PaletteSetPlayerPalette(player, engine_palette);
    player->palette_fade_step_possession = 11;
    turn_off_all_window_menus();
    turn_off_query_menus();
    turn_off_all_panel_menus();
    turn_on_main_panel_menu();
    set_flag_value(game.operation_flags, GOF_ShowPanel, (game.operation_flags & GOF_ShowGui) != 0);
  }
  leave_creature_as_passenger(player, thing);
  player->allocflags |= PlaF_KeyboardInputDisabled;
  player->influenced_thing_idx = 0;
  player->influenced_thing_creation = 0;
  light_turn_light_on(player->cursor_light_idx);
  play_non_3d_sample(177);
  return 0;
}

long pinstfe_leave_creature(struct PlayerInfo *player, int32_t *n)
{
  set_camera_zoom(player->acamera, player->dungeon_camera_zoom);
  if (is_my_player(player)) {
    PaletteSetPlayerPalette(player, engine_palette);
  }
  player->allocflags &= ~PlaF_KeyboardInputDisabled;
  player->allocflags &= ~PlaF_MouseInputDisabled;
  return 0;
}

long pinstfs_query_creature(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->influenced_thing_idx);
    player->dungeon_camera_zoom = get_camera_zoom(player->acamera);
    set_selected_creature(player, thing);
    unsigned char state = ( (player->work_state == PSt_QueryAll) || (player->work_state == PSt_CreatrInfoAll) ) ? PSt_CreatrInfoAll : PSt_CreatrInfo;
    set_player_state(player, state, 0);
    return 0;
}

long pinstfs_unquery_creature(struct PlayerInfo *player, int32_t *n)
{
    set_player_state(player, PSt_CtrlDungeon, 0);
    clear_selected_thing(player);
    return 0;
}

long pinstfs_zoom_to_heart(struct PlayerInfo *player, int32_t *n)
{
    SYNCDBG(6,"Starting for player %d",(int)player->id_number);
    if (is_my_player_number(player->id_number)) {
        LbPaletteDataFillWhite(zoom_to_heart_palette);
    }
    light_turn_light_off(player->cursor_light_idx);
    struct Thing* thing = get_player_soul_container(player->id_number);
    ThingModel spectator_breed = get_players_spectator_model(player->id_number);
    struct Coord3d mappos;
    mappos.x.val = thing->mappos.x.val;
    mappos.y.val = thing->mappos.y.val + subtile_coord(7,0);
    mappos.z.val = thing->mappos.z.val + subtile_coord(1,0);
    thing = create_and_control_creature_as_controller(player, spectator_breed, &mappos);
    if (!thing_is_invalid(thing))
    {
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        cctrl->creature_control_flags |= CCFlg_NoCompControl;
        player->allocflags |= PlaF_KeyboardInputDisabled;
        player->allocflags |= PlaF_MouseInputDisabled;
        game.view_mode_flags |= GNFldD_CreaturePasngr;
    }
    return 0;
}

long pinstfm_zoom_to_heart(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    if (thing_exists(thing))
    {
        struct Coord3d pos;
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val - subtile_coord(7, 0) / 16;
        pos.z.val = thing->mappos.z.val + (thing->solid_size_z / 2);
        move_thing_in_map(thing, &pos);
  }
  if (is_my_player_number(player->id_number)) {
      if (player->instance_remain_turns <= 8)
        LbPaletteFade(zoom_to_heart_palette, 8, Lb_PALETTE_FADE_OPEN);
  }
  return 0;
}


long pinstfe_zoom_to_heart(struct PlayerInfo *player, int32_t *n)
{
    set_player_instance(player, PI_HeartZoomOut, false);
    if (is_my_player_number(player->id_number))
        LbPaletteStopOpenFade();
    return 0;
}

long pinstfs_zoom_out_of_heart(struct PlayerInfo *player, int32_t *n)
{
    struct Thing* thing = thing_get(player->controlled_thing_idx);
    if (thing_exists(thing))
        leave_creature_as_controller(player, thing);
    set_player_mode(player, PVT_DungeonTop);
    struct Camera* cam = player->acamera;
    if (cam == NULL)
        return 0;
    thing = get_player_soul_container(player->id_number);
    if (!thing_exists(thing))
    {
        cam->mappos.x.val = subtile_coord_center(game.map_subtiles_x / 2);
        cam->mappos.y.val = subtile_coord_center(game.map_subtiles_y / 2);
        cam->zoom = 24000;
        cam->rotation_angle_x = 0;
        return 0;
  }
  cam->mappos.x.val = thing->mappos.x.val;
  if (player->view_mode == PVM_FrontView)
  {
    cam->mappos.y.val = thing->mappos.y.val;
    cam->zoom = player->frontview_zoom_level;
  } else
  {
    cam->mappos.y.val = thing->mappos.y.val - (thing->clipbox_size_z >> 1) -  thing->mappos.z.val;
    cam->zoom = 24000;
  }
  cam->rotation_angle_x = 0;
  sync_local_camera(player);
  if (!TimerNoReset)
  {
     timerstarttime = LbTimerClock();
  }
  TimerFreeze = false;
  return 0;
}

long pinstfm_zoom_out_of_heart(struct PlayerInfo *player, int32_t *n)
{
    if (player->view_mode != PVM_FrontView)
    {
        struct Camera* cam = player->acamera;
        struct Thing* thing = get_player_soul_container(player->id_number);
        long deltax;
        long deltay;
        unsigned long addval;
        if (cam != NULL)
        {
          cam->zoom -= (24000 - player->isometric_view_zoom_level) / 16;
          cam->rotation_angle_x += DEGREES_2_8125;
          addval = (thing->clipbox_size_z >> 1);
          deltax = distance_with_angle_to_coord_x((long)thing->mappos.z.val+addval, cam->rotation_angle_x);
          deltay = distance_with_angle_to_coord_y((long)thing->mappos.z.val+addval, cam->rotation_angle_x);
        } else
        {
          addval = (thing->clipbox_size_z >> 1);
          deltax = addval;
          deltay = -addval;
        }
        struct Camera* dstcam = &player->cameras[CamIV_Isometric];
        dstcam->mappos.x.val = thing->mappos.x.val + deltax;
        dstcam->mappos.y.val = thing->mappos.y.val + deltay;
        dstcam = &player->cameras[CamIV_FrontView];
        dstcam->mappos.x.val = thing->mappos.x.val + deltax;
        dstcam->mappos.y.val = thing->mappos.y.val + deltay;
        set_local_camera_destination(player);
    }
    if (is_my_player_number(player->id_number) && (player->instance_remain_turns >= 8))
        LbPaletteFade(engine_palette, 8, Lb_PALETTE_FADE_OPEN);
    return 0;
}

long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, int32_t *n)
{
  if (is_my_player(player)) {
    LbPaletteStopOpenFade();
  }
  struct Camera* cam = player->acamera;
  if ((player->view_mode != PVM_FrontView) && (cam != NULL))
  {
    cam->zoom = player->isometric_view_zoom_level;
    cam->rotation_angle_x = DEGREES_45;
    set_local_camera_destination(player);
  }
  light_turn_light_on(player->cursor_light_idx);
  player->allocflags &= ~PlaF_KeyboardInputDisabled;
  player->allocflags &= ~PlaF_MouseInputDisabled;
  game.view_mode_flags &= ~GNFldD_CreaturePasngr;
  if (is_my_player(player)) {
    PaletteSetPlayerPalette(player, engine_palette);
  }
  return 0;
}

long pinstfm_control_creature_fade(struct PlayerInfo *player, int32_t *n)
{
  player->allocflags |= PlaF_MouseInputDisabled;
  return 0;
}

long pinstfe_control_creature_fade(struct PlayerInfo *player, int32_t *n)
{
  if (is_my_player(player))
  {
    if ((player->additional_flags & PlaAF_FreezePaletteIsActive) != 0)
      PaletteSetPlayerPalette(player, blue_palette);
    else
      PaletteSetPlayerPalette(player, engine_palette);
  }
  player->allocflags &= ~PlaF_KeyboardInputDisabled;
  light_turn_light_off(player->cursor_light_idx);
  player->allocflags &= ~PlaF_MouseInputDisabled;
  return 0;
}

long pinstfs_fade_to_map(struct PlayerInfo *player, int32_t *n)
{
    struct Camera* cam = player->acamera;
    player->palette_fade_step_map = 0;
    player->allocflags |= PlaF_MouseInputDisabled;
    player->view_mode_restore = cam->view_mode;
    if (is_my_player(player))
    {
        player->tooltips_restore = settings.tooltips_on; // store tooltips setting before starting the fade
        settings.tooltips_on = false; // don't show tooltips during the fade
        player->status_menu_restore = toggle_status_menu(0); // store current status menu visibility, and hide the status menu (when the map is visible)
  }
  set_engine_view(player, PVM_ParchFadeIn);
  return 0;

}

long pinstfm_fade_to_map(struct PlayerInfo *player, int32_t *n)
{
  return 0;
}

long pinstfe_fade_to_map(struct PlayerInfo *player, int32_t *n)
{
  set_player_mode(player, PVT_MapScreen);
  if (is_my_player(player))
    settings.tooltips_on = player->tooltips_restore; // restore tooltips setting after the fade is completed
  player->allocflags &= ~PlaF_MouseInputDisabled;
  return 0;
}

long pinstfs_fade_from_map(struct PlayerInfo *player, int32_t *n)
{
  player->allocflags |= PlaF_MouseInputDisabled;
  if (is_my_player(player))
  {
    player->tooltips_restore = settings.tooltips_on; // store tooltips setting before starting the fade
    settings.tooltips_on = false; // don't show tooltips during the fade
    game.operation_flags &= ~GOF_ShowPanel;
  }
  player->palette_fade_step_map = 32;
  set_player_mode(player, PVT_DungeonTop);
  set_engine_view(player, PVM_ParchFadeOut);
  return 0;
}

long pinstfm_fade_from_map(struct PlayerInfo *player, int32_t *n)
{
  return 0;
}

long pinstfe_fade_from_map(struct PlayerInfo *player, int32_t *n)
{
    struct PlayerInfo* myplyr = get_player(my_player_number);
    set_engine_view(player, player->view_mode_restore);
    if (player->id_number == myplyr->id_number) {
        settings.tooltips_on = player->tooltips_restore; // restore tooltips setting after the fade is completed
        toggle_status_menu(player->status_menu_restore); // restore the status menu visiblity now that the map is no longer visible
    }
    player->allocflags &= ~PlaF_MouseInputDisabled;
    return 0;
}

void set_player_zoom_to_position(struct PlayerInfo *player,struct Coord3d *pos)
{
    // Make sure we are in the normal Dungeon Top view
    if(player->view_type != PVT_DungeonTop)
        return;

    // Make sure we are not in some weird instance
    if(player->instance_num == PI_DirctCtrl ||
       player->instance_num == PI_PsngrCtrl ||
       player->instance_num == PI_HeartZoom ||
       player->instance_num == PI_HeartZoomOut ||
       player->instance_num == PI_CrCtrlFade ||
       player->instance_num == PI_MapFadeTo ||
       player->instance_num == PI_MapFadeFrom ||
       player->instance_num == PI_ZoomToPos ||
       player->instance_num == PI_UnusedSlot17 ||
       player->instance_num == PI_UnusedSlot18)
        return;

    // Set zoom position
    player->zoom_to_pos_x = pos->x.val;
    player->zoom_to_pos_y = pos->y.val;

    // Make player zoom to location
    set_player_instance(player, PI_ZoomToPos, 0);
}

long pinstfs_zoom_to_position(struct PlayerInfo *player, int32_t *n)
{
    player->controlled_thing_idx = 0;
    player->controlled_thing_creatrn = 0;
    player->allocflags |= PlaF_MouseInputDisabled;
    player->allocflags |= PlaF_KeyboardInputDisabled;
    struct Camera* cam = player->acamera;
    int dt_x = (player->zoom_to_pos_x - (int)cam->mappos.x.val) / 8;
    int dt_y = (player->zoom_to_pos_y - (int)cam->mappos.y.val) / 8;
    if (dt_x < 0)
    {
      if (dt_x > -256)
        dt_x = -256;
    } else
    {
      if (dt_x < 256)
        dt_x = 256;
    }
    player->zoom_to_movement_x = dt_x;
    if (dt_y < 0)
    {
        if (dt_y > -256)
          dt_y = -256;
    } else
    {
        if (dt_y < 256)
          dt_y = 256;
    }
    player->zoom_to_movement_y = dt_y;
    return 0;
}

long pinstfm_zoom_to_position(struct PlayerInfo *player, int32_t *n)
{
    MapCoord x, y;
    struct Camera* cam = player->acamera;
    cam->inertia_x = 0;
    cam->inertia_y = 0;
    if (abs(cam->mappos.x.val - player->zoom_to_pos_x) >= abs(player->zoom_to_movement_x))
      x = player->zoom_to_movement_x + cam->mappos.x.val;
    else
      x = player->zoom_to_pos_x;
    if (abs(cam->mappos.y.val - player->zoom_to_pos_y) >= abs(player->zoom_to_movement_y))
      y = player->zoom_to_movement_y + cam->mappos.y.val;
    else
      y = player->zoom_to_pos_y;
    if ((player->zoom_to_pos_x == x) && (player->zoom_to_pos_y == y))
        player->instance_remain_turns = 0;
    cam->mappos.x.val = x;
    cam->mappos.y.val = y;
    set_local_camera_destination(player);
    return 0;
}

long pinstfe_zoom_to_position(struct PlayerInfo *player, int32_t *n)
{
    player->allocflags &= ~PlaF_MouseInputDisabled;
    player->allocflags &= ~PlaF_KeyboardInputDisabled;
    if ( (player->work_state == PSt_CreatrInfo) || (player->work_state == PSt_CreatrInfoAll) )
    {
        player->controlled_thing_idx = player->influenced_thing_idx;
    }
    return 0;
}

void set_player_instance(struct PlayerInfo *player, long ninum, TbBool force)
{
    long inum = player->instance_num;
    if (inum >= PLAYER_INSTANCES_COUNT)
        inum = 0;
    if ((inum == 0) || (player_instance_info[inum].instance_state != 1) || (force))
    {
        player->instance_num = ninum%PLAYER_INSTANCES_COUNT;
        struct PlayerInstanceInfo* inst_info = &player_instance_info[player->instance_num];
        player->instance_remain_turns = inst_info->length_turns;
        InstncInfo_Func callback = inst_info->start_cb;
        if (callback != NULL) {
            callback(player, &inst_info->start_callback_parameters[0]);
        }
    }
}

void process_player_instance(struct PlayerInfo *player)
{
    struct PlayerInstanceInfo *inst_info;
    InstncInfo_Func callback;
    SYNCDBG(16,"Starting for instance %d",(int)player->instance_num);
    if (player->instance_num <= 0) {
        return;
    }
    if (player->instance_remain_turns > 0)
    {
        player->instance_remain_turns--;
        inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
        callback = inst_info->maintain_cb;
        if (callback != NULL) {
            callback(player, &inst_info->maintain_end_callback_parameter);
        }
    }
    if (player->instance_remain_turns == 0)
    {
        inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
        player->instance_num = PI_Unset;
        callback = inst_info->end_cb;
        if (callback != NULL) {
            callback(player, &inst_info->maintain_end_callback_parameter);
        }
    }
}

void process_player_instances(void)
{
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        if (player_exists(player))
          process_player_instance(player);
    }
    SYNCDBG(9,"Finished");
}

void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing)
{
    SYNCDBG(7,"Starting for player %d within %s index %d",(int)player->id_number,thing_model_name(thing),(int)thing->index);
    if (((thing->owner != player->id_number) && (player->work_state != PSt_FreeCtrlDirect))
      || (thing->index != player->controlled_thing_idx))
    {
        set_player_instance(player, PI_Unset, 1);
        set_player_mode(player, PVT_DungeonTop);
        player->allocflags &= ~PlaF_CreaturePassengerMode;
        set_engine_view(player, player->view_mode_restore);
        player->cameras[CamIV_Isometric].mappos.x.val = subtile_coord_center(game.map_subtiles_x/2);
        player->cameras[CamIV_Isometric].mappos.y.val = subtile_coord_center(game.map_subtiles_y/2);
        player->cameras[CamIV_FrontView].mappos.x.val = subtile_coord_center(game.map_subtiles_x/2);
        player->cameras[CamIV_FrontView].mappos.y.val = subtile_coord_center(game.map_subtiles_y/2);
        sync_local_camera(player);
        clear_selected_thing(player);
        return;
    }
    clear_selected_thing(player);
    set_player_mode(player, PVT_DungeonTop);
    if (is_my_player(player)) {
        setup_eye_lens(0);
    }
    thing->alloc_flags &= ~TAlF_IsControlled;
    thing->rendering_flags &= ~TRF_Invisible;
    player->allocflags &= ~PlaF_CreaturePassengerMode;
    set_engine_view(player, player->view_mode_restore);
    long i = player->acamera->rotation_angle_x;
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    long k = thing->mappos.z.val + get_creature_eye_height(thing);
    player->cameras[CamIV_Isometric].mappos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(k,i);
    player->cameras[CamIV_Isometric].mappos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(k,i);
    player->cameras[CamIV_FrontView].mappos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(k,i);
    player->cameras[CamIV_FrontView].mappos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(k,i);
    sync_local_camera(player);
    if (thing->class_id == TCls_Creature)
    {
        set_start_state(thing);
        cctrl->max_speed = calculate_correct_creature_maxspeed(thing);
        if ((cctrl->creature_state_flags & TF2_Spectator) != 0) {
          delete_thing_structure(thing, 0);
        } else {
          disband_creatures_group(thing);
        }
    }
    if ((thing->light_id != 0) && (!crconf->illuminated) && (!creature_under_spell_effect(thing, CSAfF_Light)))
    {
        light_delete_light(thing->light_id);
        thing->light_id = 0;
    }
}

void leave_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing)
{
  SYNCDBG(7,"Starting for player %d within %s index %d",(int)player->id_number,thing_model_name(thing),(int)thing->index);
  if (((thing->owner != player->id_number) && (player->work_state != PSt_FreeCtrlPassngr))
    || (thing->index != player->controlled_thing_idx))
  {
    set_player_instance(player, PI_Unset, 1);
    set_player_mode(player, PVT_DungeonTop);
    player->allocflags &= ~PlaF_CreaturePassengerMode;
    set_engine_view(player, player->view_mode_restore);
    player->cameras[CamIV_Isometric].mappos.x.val = subtile_coord_center(game.map_subtiles_x/2);
    player->cameras[CamIV_Isometric].mappos.y.val = subtile_coord_center(game.map_subtiles_y/2);
    player->cameras[CamIV_FrontView].mappos.x.val = subtile_coord_center(game.map_subtiles_x/2);
    player->cameras[CamIV_FrontView].mappos.y.val = subtile_coord_center(game.map_subtiles_y/2);
    sync_local_camera(player);
    clear_selected_thing(player);
    return;
  }
  set_player_mode(player, PVT_DungeonTop);
  thing->rendering_flags &= ~TRF_Invisible;
  player->allocflags &= ~PlaF_CreaturePassengerMode;
  set_engine_view(player, player->view_mode_restore);
  long i = player->acamera->rotation_angle_x;
  long k = thing->mappos.z.val + get_creature_eye_height(thing);
  player->cameras[CamIV_Isometric].mappos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(k,i);
  player->cameras[CamIV_Isometric].mappos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(k,i);
  player->cameras[CamIV_FrontView].mappos.x.val = thing->mappos.x.val + distance_with_angle_to_coord_x(k,i);
  player->cameras[CamIV_FrontView].mappos.y.val = thing->mappos.y.val + distance_with_angle_to_coord_y(k,i);
  sync_local_camera(player);
  clear_selected_thing(player);
}

TbBool is_thing_passenger_controlled(const struct Thing *thing)
{
    if (!thing_exists(thing))
        return false;
    if (is_neutral_thing(thing))
        return false;
    struct PlayerInfo* player = get_player(thing->owner);
    if ((player->work_state != PSt_CtrlPassngr) && (player->work_state != PSt_FreeCtrlPassngr))
        return false;
    switch (player->instance_num)
    {
    case PI_PsngrCtrl:
        return (thing->index == player->influenced_thing_idx);
    case PI_CrCtrlFade:
        return (thing->index == player->controlled_thing_idx);
    case PI_PsngrCtLeave:
        return (thing->index == player->influenced_thing_idx);
    case PI_Unset:
    case PI_Whip: // Whip can be used at any time by comp. assistant
    case PI_WhipEnd:
        return (thing->index == player->controlled_thing_idx);
    default:
        ERRORLOG("Bad player %d instance %d",(int)thing->owner,(int)player->instance_num);
        break;
    }
    return false;
}

TbBool is_thing_directly_controlled(const struct Thing *thing)
{
    if (!thing_exists(thing))
        return false;
    if (is_neutral_thing(thing))
        return false;
    struct PlayerInfo* player = get_player(thing->owner);
    if ((player->work_state != PSt_CtrlDirect) && (player->work_state != PSt_FreeCtrlDirect))
    {
        return false;
    }
    switch (player->instance_num)
    {
    case PI_DirctCtrl:
        return (thing->index == player->influenced_thing_idx);
    case PI_CrCtrlFade:
        return (thing->index == player->controlled_thing_idx);
    case PI_DirctCtLeave:
        return (thing->index == player->influenced_thing_idx);
    case PI_Unset:
    case PI_Whip: // Whip can be used at any time by comp. assistant
    case PI_WhipEnd:
        return (thing->index == player->controlled_thing_idx);
    case PI_PsngrCtLeave: // Leaving the possessed creature
        break;
    default:
        ERRORLOG("Bad player %d instance %d",(int)thing->owner,(int)player->instance_num);
        break;
    }
    return false;
}

TbBool is_thing_some_way_controlled(const struct Thing *thing)
{
    if (!thing_exists(thing))
        return false;
    if (is_neutral_thing(thing))
        return false;
    struct PlayerInfo* player = get_player(thing->owner);
    return (player->controlled_thing_idx == thing->index);
}

TbBool set_selected_thing_f(struct PlayerInfo *player, struct Thing *thing, const char *func_name)
{
    if (thing_exists(thing))
    {
        player->controlled_thing_idx = thing->index;
        player->controlled_thing_creatrn = thing->creation_turn;
        return true;
    }
    ERRORLOG("%s: Cannot select %s index %d",func_name,thing_model_name(thing),(int)thing->index);
    return false;
}

TbBool set_selected_creature_f(struct PlayerInfo *player, struct Thing *thing, const char *func_name)
{
    if (thing_is_creature(thing))
    {
        player->controlled_thing_idx = thing->index;
        player->controlled_thing_creatrn = thing->creation_turn;
        return true;
    }
    ERRORLOG("%s: Cannot select %s index %d",func_name,thing_model_name(thing),(int)thing->index);
    return false;
}

TbBool clear_selected_thing(struct PlayerInfo *player)
{
    player->controlled_thing_idx = 0;
    player->controlled_thing_creatrn = 0;
    return true;
}

/** Builds room for the given player at given coords.
 * Takes money from dungeon, builds the room and updates stats.
 * Makes proper sound messages on failure.
 *
 * @param stl_x Target X coord to build on.
 * @param stl_y Target Y coord to build on.
 * @param plyr_idx Player owning the room.
 * @param rkind Kind of the room.
 * @return Returns room struct, or invalid room on error.
 */
struct Room *player_build_room_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, RoomKind rkind)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    struct Dungeon* dungeon = get_players_dungeon(player);
    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    // Check if we are allowed to build the room
    if (!is_room_available(plyr_idx, rkind)) {
        // It shouldn't be possible to select unavailable room
        WARNLOG("Player %d tried to build %s which is unavailable at (%d,%d)",(int)plyr_idx,room_code_name(rkind),(int)stl_x,(int)stl_y);
        if (is_my_player(player))
            play_non_3d_sample(119);
        return INVALID_ROOM;
    }
    if (!can_build_room_at_slab(plyr_idx, rkind, subtile_slab(stl_x), subtile_slab(stl_y))) {
        // It shouldn't be possible to select unavailable room
        WARNLOG("Player %d tried to build %s on a forbidden subtile (%d,%d)",(int)plyr_idx,
            room_code_name(rkind),(int)stl_x,(int)stl_y);
        if (is_my_player(player))
        {
            if (!player->roomspace.is_active)
            {
                play_non_3d_sample(119);
            }
        }
        return INVALID_ROOM;
    }
    // Check if there's a place for new room
    if (!i_can_allocate_free_room_structure())
    {
      if (is_my_player(player))
        play_non_3d_sample(119);
      return INVALID_ROOM;
    }
    if (player->boxsize == 0)
    {
        player->boxsize++;
    }
    if (dungeon->total_money_owned >= roomst->cost * player->boxsize)
    {
        if (take_money_from_dungeon(plyr_idx, roomst->cost, 1) < 0)
        {
            if (is_my_player(player))
                output_message(SMsg_GoldNotEnough, 0);
            return INVALID_ROOM;
        }
        if (player->boxsize > 0)
        {
        player->boxsize--;
        }
    }
    else
    {
        if (is_my_player(player))
            output_message(SMsg_GoldNotEnough, 0);
        return INVALID_ROOM;
    }
    struct Room* room = place_room(plyr_idx, rkind, stl_x, stl_y);
    if (!room_is_invalid(room))
    {
        if (room_role_matches(rkind, RoRoF_PassWater | RoRoF_PassLava))
        {
            if ((player->allocflags & PlaF_CompCtrl) != 0)
            {
                //Computer players need sight to build more bridge tiles
                set_explored_around(subtile_slab(stl_x), subtile_slab(stl_y), plyr_idx);
            }
            dungeon->lvstats.bridges_built++;
        }
      if (is_my_player(player))
      {
          play_non_3d_sample(77);
          if (player->boxsize > 1)
          {
              play_non_3d_sample(959);
              play_non_3d_sample(856);
          }
      }
    }
    return room;
}

TbBool player_place_trap_without_check_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel, TbBool free)
{
    struct TrapConfigStats* trap_cfg = get_trap_model_stats(tngmodel);
    struct Coord3d pos;
    if (trap_cfg->place_on_subtile)
    {
        set_coords_to_subtile_center(&pos, stl_x, stl_y, 1);
    }
    else
    {
        set_coords_to_slab_center(&pos, subtile_slab(stl_x), subtile_slab(stl_y));
    }
    delete_room_slabbed_objects(get_slab_number(subtile_slab(stl_x), subtile_slab(stl_y)));
    struct Thing* traptng = create_trap(&pos, tngmodel, plyr_idx);
    if (thing_is_invalid(traptng))
    {
        return false;
    }
    traptng->mappos.z.val = get_thing_height_at(traptng, &traptng->mappos);
    traptng->trap.revealed = 0;
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (free)
    {
        rearm_trap(traptng);
        if (!dungeon_invalid(dungeon))
            dungeon->lvstats.traps_armed++;
    }
    else
    {
        remove_workshop_item_from_amount_placeable(plyr_idx, TCls_Trap, tngmodel);
        if (placing_offmap_workshop_item(plyr_idx, TCls_Trap, tngmodel))
        {
            remove_workshop_item_from_amount_stored(plyr_idx, TCls_Trap, tngmodel, WrkCrtF_NoStored);
            rearm_trap(traptng);
            if (!dungeon_invalid(dungeon))
                dungeon->lvstats.traps_armed++;
        }
        else if (trap_cfg->instant_placement)
        {
            remove_workshop_item_from_amount_stored(plyr_idx, TCls_Trap, tngmodel, WrkCrtF_NoOffmap);
            remove_workshop_object_from_player(plyr_idx, trap_crate_object_model(tngmodel));
            rearm_trap(traptng);
            if (!dungeon_invalid(dungeon))
                dungeon->lvstats.traps_armed++;
        }
    }
    if (!dungeon_invalid(dungeon))
        dungeon->camera_deviate_jump = 192;
    if (is_my_player_number(plyr_idx))
    {
        play_non_3d_sample(trap_cfg->place_sound_idx);
    }

    lua_on_trap_placed(traptng);
    return true;
}

TbBool player_place_trap_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel)
{
    if (!is_trap_placeable(plyr_idx, tngmodel))
    {
        WARNLOG("Player %d tried to build %s but has none to place", (int)plyr_idx, trap_code_name(tngmodel));
        return false;
    }
    return player_place_trap_without_check_at(stl_x, stl_y, plyr_idx, tngmodel,false);
}

TbBool player_place_door_without_check_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel,TbBool free)
{
    unsigned char orient = find_door_angle(stl_x, stl_y, plyr_idx);
    struct Coord3d pos;
    set_coords_to_slab_center(&pos, subtile_slab(stl_x), subtile_slab(stl_y));
    struct Thing *door = create_door(&pos, tngmodel, orient, plyr_idx, 0);
    if (thing_is_invalid(door))
    {
        return false;
    }
    do_slab_efficiency_alteration(subtile_slab(stl_x), subtile_slab(stl_y));
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (!free)
    {
        int crate_source = remove_workshop_item_from_amount_stored(plyr_idx, TCls_Door, tngmodel, WrkCrtF_Default);
        switch (crate_source)
        {
        case WrkCrtS_Offmap:
            remove_workshop_item_from_amount_placeable(plyr_idx, TCls_Door, tngmodel);
            break;
        case WrkCrtS_Stored:
            remove_workshop_item_from_amount_placeable(plyr_idx, TCls_Door, tngmodel);
            remove_workshop_object_from_player(plyr_idx, door_crate_object_model(tngmodel));
            break;
        default:
            if (!dungeon_invalid(dungeon))
            {
                WARNLOG("Placeable door %s amount for player %d was incorrect; fixed", door_code_name(tngmodel), (int)dungeon->owner);
                dungeon->mnfct_info.door_amount_placeable[tngmodel] = 0;
            }
            break;
        }
    }
    if (!dungeon_invalid(dungeon))
        dungeon->camera_deviate_jump = 192;
    if (is_my_player_number(plyr_idx))
    {
        struct DoorConfigStats* door_cfg = get_door_model_stats(tngmodel);
        play_non_3d_sample(door_cfg->place_sound_idx);
    }
    return true;
}

TbBool player_place_door_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel)
{
    if (!is_door_placeable(plyr_idx, tngmodel)) {
        WARNLOG("Player %d tried to build %s but has none to place",(int)plyr_idx,door_code_name(tngmodel));
        return false;
    }
    return player_place_door_without_check_at(stl_x, stl_y, plyr_idx, tngmodel,0);
}

TbBool is_thing_directly_controlled_by_player(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (!thing_exists(thing))
        return false;
     struct PlayerInfo* player = get_player(plyr_idx);
     if (player_invalid(player))
     {
         ERRORLOG("Bad player: %d", plyr_idx);
         return false;
     }
     else
     {
        if ((player->work_state != PSt_CtrlDirect) && (player->work_state != PSt_FreeCtrlDirect) && (player->work_state != PSt_CtrlDungeon))
        {
            return false;
        }
        switch (player->instance_num)
        {
            case PI_DirctCtrl:
            case PI_HeartZoom:
            case PI_HeartZoomOut:
            case PI_Drop:
            {
                if ((thing->alloc_flags & TAlF_IsControlled) != 0)
                {
                    if (player->view_type == PVT_CreatureContrl)
                    {
                        return ( (thing->index == player->influenced_thing_idx) || (get_creature_model_flags(thing) & CMF_IsSpectator) );
                    }
                }
                return false;
            }
            case PI_CrCtrlFade:
                return (thing->index == player->controlled_thing_idx);
            case PI_DirctCtLeave:
                return (thing->index == player->influenced_thing_idx);
            case PI_Unset:
            case PI_Whip: // Whip can be used at any time by comp. assistant
            case PI_WhipEnd:
                return (thing->index == player->controlled_thing_idx);
            case PI_PsngrCtLeave: // Leaving the possessed creature
                break;
        }
     }
    return false;
}

TbBool is_thing_passenger_controlled_by_player(const struct Thing *thing, PlayerNumber plyr_idx)
{
    if (!thing_exists(thing))
        return false;
     struct PlayerInfo* player = get_player(plyr_idx);
     if (player_invalid(player))
     {
         ERRORLOG("Bad player: %d", plyr_idx);
         return false;
     }
    else
    {
        if ((player->work_state != PSt_CtrlPassngr) && (player->work_state != PSt_FreeCtrlPassngr))
            return false;
        switch (player->instance_num)
        {
        case PI_PsngrCtrl:
            return ( (thing->index == player->influenced_thing_idx) && (player->view_type == PVT_CreaturePasngr) );
        case PI_CrCtrlFade:
            return (thing->index == player->controlled_thing_idx);
        case PI_PsngrCtLeave:
            return (thing->index == player->influenced_thing_idx);
        case PI_Unset:
        case PI_Whip: // Whip can be used at any time by comp. assistant
        case PI_WhipEnd:
            return (thing->index == player->controlled_thing_idx);
        default:
            ERRORLOG("Bad player %d instance %d",plyr_idx,(int)player->instance_num);
            break;
        }
    }
    return false;
}
/******************************************************************************/
