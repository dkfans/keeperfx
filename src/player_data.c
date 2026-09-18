/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_data.c
 *     Player data structures definitions.
 * @par Purpose:
 *     Defines functions for player-related structures support.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "observer.h"
#include "player_data.h"

#include "globals.h"
#include "bflib_basics.h"

#include "config_players.h"
#include "config_powerhands.h"
#include "player_instances.h"
#include "config_players.h"
#include "game_legacy.h"
#include "local_camera.h"
#include "net_game.h"
#include "engine_redraw.h"
#include "frontend.h"
#include "thing_objects.h"
#include "power_hand.h"
#include "gui_msgs.h"
#include "kfx/renderer/RendererManager.h"
#include "post_inc.h"

/******************************************************************************/
TbPixel player_path_colours[]  =     {131, 90, 163, 181,  20,   4, 106,  52,  42};
TbPixel player_room_colours[]  =     {132, 92, 164, 183,  21, 132, 108,  54,  44};
TbPixel player_flash_colours[] =     {133, 94, 167, 142,  31,  15, 110,  54,  46};
TbPixel player_highlight_colours[] = {31,  31,  31,  31,  31,  31,  31,  31,  31};
TbPixel possession_hit_colours[] =   {133, 89, 167, 141,  31,  31, 110,  54,  46};

unsigned short const player_cubes[] = {0x00C0, 0x00C1, 0x00C2, 0x00C3, 0x00C7, 0x00C6 };

struct PlayerInfo bad_player;

struct LocalState local_state;
struct UserState bad_user_state;

/** The current player's number. */
unsigned char my_player_number;
/******************************************************************************/

enum LocalViewMode get_dungeon_view_mode(const struct UserState *ustate)
{
    if (ustate->prefs[UPref_FrontView] != 0)
        return PVM_FrontView;
    return (ustate->prefs[UPref_Wibble] != 0) ? PVM_IsoWibbleView : PVM_IsoStraightView;
}

int32_t get_player_dungeon_yaw(const struct PlayerInfo *player)
{
    const struct UserState *ustate = get_player_user_state(player);
    if (user_state_invalid(ustate))
        return 0;
    return ustate->dungeon_camera.yaw[ustate->prefs[UPref_FrontView] != 0];
}


unsigned char get_player_view_type(const struct PlayerInfo *player)
{
    const struct UserState *ustate = get_player_user_state(player);
    if (user_state_invalid(ustate))
        return PVT_DungeonTop;
    return ustate->view_type;
}

/** Which of a user's local cameras shows this player's view. */
unsigned char get_player_active_camera_index(const struct PlayerInfo *player)
{
    switch (get_player_view_type(player))
    {
    case PVT_CreatureContrl:
    case PVT_CreaturePasngr:
        return CamIV_FirstPerson;
    case PVT_MapScreen:
    case PVT_MapFadeOut:
        return CamIV_Parchment;
    default:
        return (get_player_user_state(player)->prefs[UPref_FrontView] != 0) ? CamIV_FrontView : CamIV_Isometric;
    }
}


struct PlayerInfo *get_player_f(PlayerNumber plyr_idx,const char *func_name)
{
    if ((plyr_idx >= 0) && (plyr_idx < PLAYERS_COUNT))
    {
        return &game.players[plyr_idx];
    }
    if (plyr_idx == game.neutral_player_num) // Suppress error for never existing but valid neutral 'player'
    {
        SYNCDBG(3, "%s: Tried to get neutral player!",func_name);
    }
    else
    {
        ERRORMSG("%s: Tried to get non-existing player %d!",func_name,(int)plyr_idx);
    }
    return INVALID_PLAYER;
}

TbBool player_invalid(const struct PlayerInfo *player)
{
    if (player == &local_observer_player) {
        return false;
    }
    if (player == INVALID_PLAYER)
        return true;
    return (player < &game.players[0]);
}

/**
 * Returns if the given player is in use on current map.
 * @param player The player to check.
 * @return True if the player is in use, false otherwise.
 */
TbBool player_exists(const struct PlayerInfo *player)
{
    if (player_invalid(player))
        return false;
    return ((player->allocflags & PlaF_Allocated) != 0);
}

// has a computer or human keeper (possibly defeated)
// Not a "zombie," hero, neutral, nor unused slot
TbBool is_active_keeper(const struct PlayerInfo *player)
{
    return player_exists(player) && (player->player_type == PT_Keeper);
}

TbBool is_my_player(const struct PlayerInfo *player)
{
    return player == get_my_player();
}

struct PlayerInfo *get_my_player(void)
{
    if (observer_is_active()) {
        return &local_observer_player;
    }
    return get_player(my_player_number);
}

const struct PlayerInfo *get_displayed_player(void)
{
    if (observer_is_active() && !is_observer_camera_active()) {
        return get_player(my_player_number);
    }
    return get_my_player();
}

TbBool is_my_player_number(PlayerNumber plyr_num)
{
    struct PlayerInfo *player = &game.players[my_player_number % PLAYERS_COUNT];
    return plyr_num == player->id_number && is_my_player(player);
}

TbBool is_player_displayed(PlayerNumber plyr_num)
{
    if (get_my_player() == &local_observer_player) {
        return plyr_num == my_player_number;
    }
    return is_my_player_number(plyr_num);
}

// returns user's UserState, or INVALID_USER_STATE.
struct UserState *get_user_state(NetUserId user)
{
    if (network_is_active() && user == netstate.my_id && user >= MAX_NET_USERS && user < MAX_NET_CONNECTIONS) {
        return &local_observer_user_state;
    }
    if ((user < 0) || (user >= MAX_NET_USERS))
        return INVALID_USER_STATE;
    return &game.user_states[user];
}

NetUserId get_player_primary_user(const struct PlayerInfo *player)
{
    if (player == &local_observer_player) {
        return get_local_user();
    }
    if ((player == NULL) || player_invalid(player))
        return -1;
    for (NetUserId user = 0; user < MAX_NET_USERS; ++user) {
        if (game.user_states[user].player_id == player->id_number) {
            return user;
        }
    }
    return -1;
}

struct UserState *get_player_user_state(const struct PlayerInfo *player)
{
    if (player == &local_observer_player) {
        return &local_observer_user_state;
    }
    return get_user_state(get_player_primary_user(player));
}

struct UserState *get_local_user_state(void)
{
    if (observer_is_active()) {
        return &local_observer_user_state;
    }
    return get_user_state(get_local_user());
}

uint32_t get_local_minimap_zoom(void)
{
    const struct UserState *ustate = get_local_user_state();
    if (user_state_invalid(ustate))
        return MINIMAP_ZOOM_DEFAULT;
    return clamp(ustate->prefs[UPref_MinimapZoom], MINIMAP_ZOOM_MIN, MINIMAP_ZOOM_MAX);
}

TbBool user_state_invalid(const struct UserState *ustate)
{
    if (ustate == INVALID_USER_STATE)
        return true;
    return (ustate == NULL);
}

int32_t user_zoom_min(const struct UserState *ustate, TbBool front_view)
{
    if (user_state_invalid(ustate))
        return front_view ? frontview_zoom_distance_setting : max(CAMERA_ZOOM_MIN, zoom_distance_setting);
    if (front_view)
        return ustate->prefs[UPref_MaxZoomFrontview];
    return max(CAMERA_ZOOM_MIN, (int32_t)ustate->prefs[UPref_MaxZoomIso]);
}

TbBool player_is_roaming(PlayerNumber plyr_num)
{
    struct PlayerInfo* player = get_player(plyr_num);
    return (player->player_type == PT_Roaming);
}

TbBool player_is_keeper(PlayerNumber plyr_num)
{
    struct PlayerInfo* player = get_player(plyr_num);
    return (player->player_type == PT_Keeper);
}

TbBool player_is_neutral(PlayerNumber plyr_num)
{
    struct PlayerInfo* player = get_player(plyr_num);
    return (player->player_type == PT_Neutral);
}

/**
 * Informs if player plyr1_idx considers player plyr2_idx as enemy.
 * Note that if the players are not enemies, it doesn't necessarily mean they're friends.
 * @param origin_plyr_idx Index of the player who asks for an enemy.
 * @param check_plyr_idx Index of the player who could be enemy.
 * @return True if the players are enemies; false otherwise.
 */
TbBool players_are_enemies(PlayerNumber origin_plyr_idx, PlayerNumber check_plyr_idx)
{
    // Player can't be his own enemy
    if (origin_plyr_idx == check_plyr_idx)
        return false;
    // And neutral player can't be enemy
    if ((origin_plyr_idx == game.neutral_player_num) || (check_plyr_idx == game.neutral_player_num))
        return false;

        
    struct PlayerInfo* origin_player = get_player(origin_plyr_idx);
    struct PlayerInfo* check_player = get_player(check_plyr_idx);
    // Inactive or invalid players are not enemies, as long as they're not heroes
    // (heroes are normally NOT existing keepers)
    if (!player_exists(origin_player) && (!player_is_roaming(origin_plyr_idx)))
        return false;
    if (!player_exists(check_player) && (!player_is_roaming(check_plyr_idx)))
        return false;
    // And if they're valid, living players - get result from alliances table
    return !flag_is_set(origin_player->allied_players, to_flag(check_plyr_idx));
}

/**
 * Informs if players plyr1_idx and plyr2_idx are mutual allies.
 * If the players are not mutual allies, one side can still consider they're friends.
 * @param plyr1_idx Index of the first player.
 * @param plyr2_idx Index of the second player.
 * @return True if the players are mutual allies; false otherwise.
 */
TbBool players_are_mutual_allies(PlayerNumber plyr1_idx, PlayerNumber plyr2_idx)
{
    // Player is always his own ally
    if (plyr1_idx == plyr2_idx)
        return true;
    // And neutral player can't be allied
    if ((plyr1_idx == game.neutral_player_num) || (plyr2_idx == game.neutral_player_num))
        return false;
    struct PlayerInfo* player1 = get_player(plyr1_idx);
    struct PlayerInfo* player2 = get_player(plyr2_idx);
    // Inactive or invalid players are not allies
    if (!player_exists(player1))
        return false;
    if (!player_exists(player2))
        return false;
    return (flag_is_set(player1->allied_players, to_flag(plyr2_idx)) && flag_is_set(player2->allied_players, to_flag(plyr1_idx)));
}

/**
 * Informs if players plyr1_idx and plyr2_idx creatures are tolerating each other.
 * This is similar to mutual alliance, but differs in conditions on nonexisting and neutral player.
 * @param plyr1_idx Index of the first player.
 * @param plyr2_idx Index of the second player.
 * @return True if the players creatures are tolerating each other; false otherwise.
 */
TbBool players_creatures_tolerate_each_other(PlayerNumber plyr1_idx, PlayerNumber plyr2_idx)
{
    // Player is always tolerating fellow creatures
    if (plyr1_idx == plyr2_idx)
        return true;
    // And neutral player creatures are like fellow creatures
    if ((plyr1_idx == game.neutral_player_num) || (plyr2_idx == game.neutral_player_num))
        return true;
    struct PlayerInfo* player1 = get_player(plyr1_idx);
    struct PlayerInfo* player2 = get_player(plyr2_idx);
    // Check if we're allied
    return (flag_is_set(player1->allied_players, to_flag(plyr2_idx)) && flag_is_set(player2->allied_players, to_flag(plyr1_idx)));
}

TbBool player_allied_with(const struct PlayerInfo *player, PlayerNumber ally_idx)
{
    if ((ally_idx < 0) || (ally_idx >= PLAYERS_COUNT))
    {
        WARNLOG("Tried to get non-existing player!");
        return false;
    }
    return flag_is_set(player->allied_players, to_flag(ally_idx));
}

/**
 * Checks if given player is either friendly to origin player or defeated.
 * @param check_plyr_idx
 * @param origin_plyr_idx
 * @return
 */
TbBool player_is_friendly_or_defeated(PlayerNumber check_plyr_idx, PlayerNumber origin_plyr_idx)
{
    // Handle neutral player at first, because we can't get PlayerInfo nor Dungeon for it
    if ((origin_plyr_idx == game.neutral_player_num) || (check_plyr_idx == game.neutral_player_num))
        return true;
    struct PlayerInfo* player = get_player(check_plyr_idx);
    struct PlayerInfo* win_player = get_player(origin_plyr_idx);
    if (player_exists(player))
    {
        if ( (!player_allied_with(win_player, check_plyr_idx)) || (!player_allied_with(player, origin_plyr_idx)) )
        {
            if (player_has_heart(check_plyr_idx))
              return false;
        }
    }
    return true;
}

void clear_players(void)
{
    observer_reset();
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = &game.players[i];
        memset(player, 0, sizeof(struct PlayerInfo));
        player->id_number = PLAYERS_COUNT;
        switch (i)
        {
        case PLAYER_GOOD:
            player->player_type = PT_Roaming;
            break;
        case PLAYER_NEUTRAL:
            player->player_type = PT_Neutral;
            break;
        default:
            player->player_type = PT_Keeper;
            break;
        }
    }
    memset(&bad_player, 0, sizeof(struct PlayerInfo));
    bad_player.id_number = PLAYERS_COUNT;
    memset(game.user_states, 0, sizeof(game.user_states));
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        game.user_states[user].player_id = PLAYER_NONE;
    }
    memset(&local_state, 0, sizeof(local_state));
    memset(&bad_user_state, 0, sizeof(bad_user_state));
    bad_user_state.player_id = PLAYER_NONE;
    game.human_players_count = 0;
    //game.game_kind = GKind_LocalGame;
}

void toggle_ally_with_player(PlayerNumber plyr_idx, PlayerNumber ally_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_invalid(player))
        return;
    toggle_flag(player->allied_players, to_flag(ally_idx)); // toggle player ally_idx in player plyridx's allies list
}

TbBool set_ally_with_player(PlayerNumber plyr_idx, PlayerNumber ally_idx, TbBool make_ally)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_invalid(player))
        return false;
    if ((ally_idx < 0) || (ally_idx >= PLAYERS_COUNT))
        return false;
    if (make_ally)
        set_flag(player->allied_players, to_flag(ally_idx)); // add player ally_idx to player plyridx's allies list
    else // enemy
        clear_flag(player->allied_players, to_flag(ally_idx)); // remove player ally_idx from player plyridx's allies list

    return true;
}

TbBool is_player_ally_locked(PlayerNumber plyr_idx, PlayerNumber ally_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_invalid(player))
        return false;

    if ((ally_idx < 0) || (ally_idx >= PLAYERS_COUNT))
        return false;

    return flag_is_set(player->players_with_locked_ally_status, to_flag(ally_idx)); // returns true if player ally_idx's ally status is locked for player plyridx
}

void set_player_ally_locked(PlayerNumber plyr_idx, PlayerNumber ally_idx, TbBool lock_alliance)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_invalid(player))
        return;

    if ((ally_idx < 0) || (ally_idx >= PLAYERS_COUNT))
        return;

    if (lock_alliance)
        set_flag(player->players_with_locked_ally_status, to_flag(ally_idx)); // lock ally player's ally status with player plyridx
    else
        clear_flag(player->players_with_locked_ally_status, to_flag(ally_idx)); // unlock ally player's ally status with player plyridx
}

void set_user_work_state(NetUserId user, short nwrk_state, int32_t chosen_kind)
{
  struct UserState* ustate = get_user_state(user);
  if (user_state_invalid(ustate))
    return;
  struct PlayerInfo* player = get_player(get_net_user_player_number(user));
  SYNCDBG(6,"User %d (player %d) state %s to %s",(int)user,(int)player->id_number,player_state_code_name(ustate->work_state),player_state_code_name(nwrk_state));
  // Selecting the same state again - update only 2nd parameter
  if (ustate->work_state == nwrk_state)
  {
    switch ( ustate->work_state )
    {
    case PSt_BuildRoom:
        ustate->chosen_room_kind = chosen_kind;
        break;
    case PSt_PlaceTrap:
        ustate->chosen_trap_kind = chosen_kind;
        break;
    case PSt_PlaceDoor:
        ustate->chosen_door_kind = chosen_kind;
        break;
    case PSt_CastPowerOnSubtile:
    case PST_CastPowerOnTarget:
    case PSt_CreateDigger:
    case PSt_SightOfEvil:
    case PSt_CallToArms:
        ustate->chosen_power_kind = chosen_kind;
        break;
    case PSt_CtrlDirect:
    case PSt_CtrlPassngr:
    case PSt_FreeCtrlPassngr:
    case PSt_FreeCtrlDirect:
        ustate->chosen_power_kind = PwrK_POSSESS;
        break;
    case PSt_FreeDestroyWalls:
        ustate->chosen_power_kind = PwrK_DESTRWALLS;
        break;
    case PSt_FreeCastDisease:
        ustate->chosen_power_kind = PwrK_DISEASE;
        break;
    case PSt_FreeTurnChicken:
        ustate->chosen_power_kind = PwrK_CHICKEN;
        break;
    }
    return;
  }
  ustate->continue_work_state = ustate->work_state;
  ustate->work_state = nwrk_state;
  if ((ustate->work_state != PSt_CreatrQuery) && (ustate->work_state != PSt_CreatrInfo)
     && (ustate->work_state != PSt_QueryAll) && (ustate->work_state != PSt_CreatrInfoAll)
     && (ustate->work_state != PSt_CtrlDirect) && (ustate->work_state != PSt_CtrlPassngr)
     && (ustate->work_state != PSt_FreeCtrlDirect) && (ustate->work_state != PSt_FreeCtrlPassngr))
  {
      clear_selected_thing(player);
  }
  switch (ustate->work_state)
  {
  case PSt_CtrlDungeon:
      ustate->full_slab_cursor = 1;
      ustate->chosen_power_kind = PwrK_None; //Cleanup for spells. Traps, doors and rooms do not require cleanup.
      break;
  case PSt_BuildRoom:
      ustate->chosen_room_kind = chosen_kind;
      break;
  case PSt_HoldInHand:
      create_power_hand(user);
      break;
  case PSt_Slap:
  {
      {
          struct Coord3d pos;
          pos.x.val = 0;
          pos.y.val = 0;
          pos.z.val = 0;
          struct Thing* thing = create_object(&pos, ObjMdl_PowerHand, player->id_number, -1);
          if (thing_is_invalid(thing))
          {
              ustate->hand_thing_idx = 0;
              break;
          }
          ustate->hand_thing_idx = thing->index;
          set_power_hand_graphic(user, HndA_SideHover);
          place_thing_in_limbo(thing);
          break;
      }
  }
  case PSt_PlaceTrap:
      ustate->chosen_trap_kind = chosen_kind;
      break;
  case PSt_PlaceDoor:
      ustate->chosen_door_kind = chosen_kind;
      break;
  case PSt_CastPowerOnSubtile:
  case PST_CastPowerOnTarget:
  case PSt_CallToArms:
  case PSt_SightOfEvil:
  case PSt_CreateDigger:
      ustate->chosen_power_kind = chosen_kind;
      break;
  case PSt_MkGoodCreatr:
        clear_messages_from_player(MsgType_Player, ustate->cheatselection.chosen_player);
        ustate->cheatselection.chosen_player = PLAYER_GOOD;
        break;
  case PSt_MkBadCreatr:
  case PSt_MkDigger:
        clear_messages_from_player(MsgType_Player, ustate->cheatselection.chosen_player);
        ustate->cheatselection.chosen_player = player->id_number;
        break;
  case PSt_FreeCtrlPassngr:
  case PSt_FreeCtrlDirect:
  case PSt_CtrlPassngr:
  case PSt_CtrlDirect:
        ustate->chosen_power_kind = PwrK_POSSESS;
        break;
  case PSt_FreeDestroyWalls:
      ustate->chosen_power_kind = PwrK_DESTRWALLS;
      break;
  case PSt_FreeCastDisease:
      ustate->chosen_power_kind = PwrK_DISEASE;
      break;
  case PSt_FreeTurnChicken:
      ustate->chosen_power_kind = PwrK_CHICKEN;
      break;
   default:
      break;
  }
}

/**
 * Sets a user's view type.
 *
 * @param user The user whose view type will be set.
 * @param nview The new view type.
 */
void set_user_view_type(NetUserId user, unsigned short nview)
{
  struct UserState* ustate = get_user_state(user);
  if (user_state_invalid(ustate))
    return;
  struct PlayerInfo* player = get_player(get_net_user_player_number(user));
  const TbBool is_local = (user == get_local_user());
  if (is_local && local_state.view_type == nview)
    local_state.view_type = PVT_None;
  if (user_state_invalid(ustate) || (ustate->view_type == nview))
    return;
  const TbBool leaving_map = (ustate->view_type == PVT_MapScreen) || (ustate->view_type == PVT_MapFadeOut);
  ustate->view_type = nview;
  ustate->init_flags &= ~UsrIF_CreaturePassengerMode;
  if (is_local)
  {
    // GameUI::IsActiveForCurrentView() decides fresh each frame whether to
    // submit sidebar content based on view_type, but a transition that also
    // kicks off a palette fade (e.g. entering/leaving the parchment map) can
    // have its now-correct submission get stuck behind a fade-preserve
    // window that keeps replaying the last frame. Force one real flip so the
    // change actually lands on the read-side buffer before any such window
    // starts.
    RendererForceUIFlipNextFrame();
    game.view_mode_flags &= ~GNFldD_CreaturePasngr;
    game.view_mode_flags |= GNFldD_CreatureViewMode;
    stop_all_things_playing_samples();
  }
  switch (nview)
  {
  case PVT_DungeonTop:
  {
      update_engine_view(player, leaving_map);
      if (is_local) {
        if (local_state.view_type == PVT_None) {
          toggle_status_menu((game.operation_flags & GOF_ShowPanel) != 0);
        }
        setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
      }
      break;
  }
  case PVT_CreatureContrl:
  case PVT_CreaturePasngr:
      update_engine_view(player, leaving_map);
      if (is_local)
      {
        game.view_mode_flags &= ~GNFldD_CreatureViewMode;
        setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
      }
      break;
  case PVT_MapScreen:
      if (is_local && local_state.view_type == PVT_None) {
        toggle_status_menu(0);
      }
      ustate->continue_work_state = ustate->work_state;
      update_engine_view(player, leaving_map);
      break;
  case PVT_MapFadeIn:
      set_player_instance(player, PI_MapFadeTo, 0);
      if (player->instance_num == PI_MapFadeTo)
          player->instance_remain_turns = max((int32_t)ustate->prefs[UPref_MapFade], 1);
      break;
  case PVT_MapFadeOut:
      set_player_instance(player, PI_MapFadeFrom, 0);
      if (player->instance_num == PI_MapFadeFrom)
          player->instance_remain_turns = max((int32_t)ustate->prefs[UPref_MapFade], 1);
      break;
  }
}

void reset_user_view_type(NetUserId user, unsigned short nview)
{
  struct UserState* ustate = get_user_state(user);
  if (user_state_invalid(ustate))
    return;
  struct PlayerInfo* player = get_player(get_net_user_player_number(user));
  const TbBool is_local = (user == get_local_user());
  const TbBool leaving_map = (ustate->view_type == PVT_MapScreen) || (ustate->view_type == PVT_MapFadeOut);
  if (!user_state_invalid(ustate))
    ustate->view_type = nview;
  switch (nview)
  {
    case PVT_DungeonTop:
      ustate->work_state = ustate->continue_work_state;
      update_engine_view(player, leaving_map);
      if (is_local)
        game.view_mode_flags &= ~GNFldD_CreatureViewMode;
      break;
    case PVT_CreatureContrl:
    case PVT_CreaturePasngr:
      ustate->work_state = ustate->continue_work_state;
      update_engine_view(player, leaving_map);
      if (is_local)
        game.view_mode_flags |= GNFldD_CreatureViewMode;
      break;
    case PVT_MapScreen:
      ustate->work_state = ustate->continue_work_state;
      update_engine_view(player, leaving_map);
      if (is_local)
        game.view_mode_flags &= ~GNFldD_CreatureViewMode;
      break;
    default:
      break;
  }
}

/** Front view leaves the wibble preference as given, for when the user returns to iso. */
void rotate_mode_to_dungeon_view(unsigned char mode, TbBool *front_view, TbBool *wibble)
{
    if (mode > 2)
    {
        ERRORLOG("Unrecognised video rotate mode: %u", mode);
        mode = 0;
    }
    *front_view = (mode == 2);
    if (mode != 2)
        *wibble = (mode == 0);
}

unsigned char get_player_color_idx(PlayerNumber plyr_idx)
{
    //neutral has no dungeon to store this in
    if(plyr_idx == PLAYER_NEUTRAL)
        return plyr_idx;
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    return dungeon->color_idx;
}
/******************************************************************************/
