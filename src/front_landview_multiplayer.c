/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_landview_multiplayer.c
 *     Network land view, where the user can select map for multiplayer.
 * @par Purpose:
 *     Functions for displaying and maintaining the network land view.
 * @par Comment:
 *     Split from front_landview.c to isolate multiplayer-specific behavior.
 * @author   KeeperFX Team
 * @date     05 May 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "front_landview.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_datetm.h"
#include "bflib_mouse.h"
#include "bflib_network.h"
#include "bflib_network_exchange.h"
#include "bflib_sndlib.h"
#include "bflib_sound.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"

#include "config_campaigns.h"
#include "config_settings.h"
#include "front_network.h"
#include "front_simple.h"
#include "frontend.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "keeperfx.hpp"
#include "room_list.h"
#include "vidfade.h"
#include "vidmode.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/* Keep network landview animation and slap timing local to this unit. */
#define NETLAND_SLAP_SOUND 75
#define NETLAND_SLAP_MISS_SOUND 26
#define NETLAND_SLAP_MISS_SOUND_VARIANTS 6
#define NETLAND_HAND_ANIM_SPEED 20
#define NETLAND_HAND_LIMP_FRAME_COUNT ((int32_t)(sizeof(hand_limp_xoffset) / sizeof(hand_limp_xoffset[0])))

struct NetLandPlayersState {
    int32_t unselected_players;
    LevelNumber selected_level_number;
    TbBool is_selected;
};

enum NetLandSlapFrame {
    NetLandSlap_StartFrame = 9,
    NetLandSlap_HitFrame = 13,
    NetLandSlap_EndFrame = 16,
};

struct NetLandRemoteSlap {
    TbClockMSec start;
    int32_t x;
    int32_t y;
    TbBool active;
};

struct NetLandLocalState {
    TbClockMSec limp_start;
    TbClockMSec local_slap_anim_start;
    int32_t limp_x;
    int32_t limp_y;
    int32_t local_slap_send_frame;
    int32_t slap_miss_wait;
};

extern LevelNumber mouse_over_lvnum;
void draw_map_screen(void);
void draw_map_level_ensigns(void);
const struct TbSprite * get_map_ensign(long idx);
void set_all_ensigns_state(unsigned short nstate);
void unload_map_and_window(void);
TbBool load_map_and_window(LevelNumber lvnum);
void frontmap_start_music(void);
void frontmap_zoom_skip_init(LevelNumber lvnum);
void frontmap_zoom_in_init(LevelNumber lvnum);
TbBool frontmap_input_active_ensign(long curr_mx, long curr_my);
TbBool frontmap_update_zoom(void);

TbPixel net_player_colours[] = { 251, 58, 182, 11 };
const int32_t hand_limp_xoffset[] = { 32, 31, 30, 29, 28, 27, 26, 24, 22, 19, 15, 9 };
const int32_t hand_limp_yoffset[] = { -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0 };

long fe_net_level_selected;
static struct NetLandLocalState net_map_local;
static struct NetLandRemoteSlap net_map_remote_slap[NET_PLAYERS_COUNT];
struct ScreenPacket net_screen_packet[NET_PLAYERS_COUNT];
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
static void update_net_ensigns_visibility(void)
{
    SYNCDBG(18, "Starting");
    set_all_ensigns_state(LvSt_Hidden);
    long lvnum = first_multiplayer_level();
    while (lvnum > 0) {
        struct LevelInformation* lvinfo = get_level_info(lvnum);
        if (lvinfo != NULL) {
            lvinfo->state = LvSt_Visible;
        }
        lvnum = next_multiplayer_level(lvnum);
    }
}

static TbBool is_connected_screen_packet(const struct ScreenPacket *nspck)
{
    return (nspck->networkstatus_flags & NetStat_PlayerConnected) != 0;
}

static void set_screen_packet_position(struct ScreenPacket *nspck, int32_t x, int32_t y)
{
    nspck->stored_data1 = x;
    nspck->stored_data2 = y;
}

static void set_remote_slap_position(struct NetLandRemoteSlap *remote_slap, const struct ScreenPacket *nspck)
{
    remote_slap->x = nspck->stored_data1;
    remote_slap->y = nspck->stored_data2;
}

static void set_packet_slap_frame(struct ScreenPacket *nspck, int32_t slap_frame)
{
    screen_packet_set_action(nspck, NetAct_Slapping);
    nspck->action_par1 = slap_frame;
}

void frontnetmap_unload(void)
{
    unload_map_and_window();
    free_font(&map_font);
    free_spritesheet(&map_flag);
    free_spritesheet(&map_hand);
    fe_network_active = 0;
    stop_music();
    set_music_volume(settings.music_volume);
}

static int32_t get_hand_limp_frame(TbClockMSec now)
{
    return NETLAND_HAND_LIMP_FRAME_COUNT - 1 - ((now - net_map_local.limp_start) * NETLAND_HAND_ANIM_SPEED) / 1000;
}

static void stop_hand_limp(void)
{
    LbMouseSetPosition(net_map_local.limp_x + hand_limp_xoffset[0] - map_info.screen_shift_x, net_map_local.limp_y + hand_limp_yoffset[0] - map_info.screen_shift_y);
    net_map_local.limp_start = 0;
}

static int32_t get_local_hand_limp_frame(TbClockMSec now)
{
    if (net_map_local.limp_start == 0) {
        return -1;
    }
    int32_t frame = get_hand_limp_frame(now);
    if (frame >= 0) {
        return frame;
    }
    stop_hand_limp();
    return -1;
}

static int32_t get_slap_anim_frame(TbClockMSec slap_anim_start, TbClockMSec now)
{
    if (slap_anim_start == 0) {
        return 0;
    }
    int32_t frame = NetLandSlap_StartFrame + ((now - slap_anim_start) * NETLAND_HAND_ANIM_SPEED) / 1000;
    if (frame > NetLandSlap_EndFrame) {
        return 0;
    }
    return frame;
}

static const struct TbSprite *get_hand_sprite_for_packet(const struct ScreenPacket *nspck, int32_t anim_frame, int32_t *x, int32_t *y)
{
    int32_t frame;
    switch (screen_packet_action(nspck)) {
    case NetAct_Limping:
        frame = clamp(nspck->action_par2, 0, NETLAND_HAND_LIMP_FRAME_COUNT - 1);
        *x = nspck->stored_data1 + hand_limp_xoffset[frame];
        *y = nspck->stored_data2 + hand_limp_yoffset[frame];
        return get_sprite(map_hand, 21);
    case NetAct_Slapping:
        *x = nspck->stored_data1 - 31;
        *y = nspck->stored_data2 - 27;
        return get_sprite(map_hand, nspck->action_par1);
    default:
        if (nspck->action_par1 == SINGLEPLAYER_NOTSTARTED) {
            *x = nspck->stored_data1 - 19;
            *y = nspck->stored_data2 - 25;
            return get_sprite(map_hand, 1 + (anim_frame % 8));
        }
        *x = nspck->stored_data1 - 20;
        *y = nspck->stored_data2 - 14;
        return get_sprite(map_hand, 17 + (anim_frame % 4));
    }
}

static void get_hand_packet(PlayerNumber plyr_idx, struct ScreenPacket *nspck, int32_t slap_frame, TbClockMSec now)
{
    if (!is_my_player_number(plyr_idx)) {
        struct NetLandRemoteSlap *remote_slap = &net_map_remote_slap[(int32_t)plyr_idx];
        *nspck = net_screen_packet[(int32_t)plyr_idx];
        TbBool packet_slap = screen_packet_action(nspck) == NetAct_Slapping;
        if (packet_slap) {
            // Hold onto a remote slap until the sender clears it, so network delay doesn't accidentally turn one slap into multiple.
            if (!remote_slap->active) {
                remote_slap->start = now;
                remote_slap->active = true;
            }
            set_remote_slap_position(remote_slap, nspck);
        } else {
            remote_slap->active = false;
        }
        if (remote_slap->start != 0) {
            int32_t display_frame = get_slap_anim_frame(remote_slap->start, now);
            if (display_frame != 0) {
                if (!packet_slap && nspck->action_par1 == SINGLEPLAYER_NOTSTARTED) {
                    set_remote_slap_position(remote_slap, nspck);
                }
                set_packet_slap_frame(nspck, display_frame);
                set_screen_packet_position(nspck, remote_slap->x, remote_slap->y);
                return;
            }
            remote_slap->start = 0;
        }
        if (packet_slap && remote_slap->active) {
            screen_packet_set_action(nspck, NetAct_None);
            nspck->action_par1 = SINGLEPLAYER_NOTSTARTED;
        }
        return;
    }
    memset(nspck, 0, sizeof(struct ScreenPacket));
    nspck->networkstatus_flags = NetStat_PlayerConnected;
    nspck->action_par1 = fe_net_level_selected;
    set_screen_packet_position(nspck, GetMouseX()*16/units_per_pixel_landview + map_info.screen_shift_x, GetMouseY()*16/units_per_pixel_landview + map_info.screen_shift_y);
    int32_t frame = get_local_hand_limp_frame(now);
    if (frame >= 0) {
        screen_packet_set_action(nspck, NetAct_Limping);
        nspck->action_par2 = frame;
        set_screen_packet_position(nspck, net_map_local.limp_x, net_map_local.limp_y);
        return;
    }
    if (slap_frame > 0) {
        set_packet_slap_frame(nspck, slap_frame);
        return;
    }
    if (nspck->action_par1 > 0) {
        const struct TbSprite* spr = get_map_ensign(1);
        struct LevelInformation* lvinfo = get_level_info(nspck->action_par1);
        if (lvinfo != NULL) {
            set_screen_packet_position(nspck, lvinfo->ensign_x + my_player_number * ((int32_t)spr->SWidth), lvinfo->ensign_y - 48);
        }
    }
}

static void draw_netmap_players_hands(void)
{
  struct ScreenPacket nspck;
  const char *plyr_nam;
  const struct TbSprite *spr;
  TbPixel colr;
  TbClockMSec now;
  int32_t x, y;
  int32_t w;
  int32_t h;
  int32_t i;
  int32_t anim_frame;

  now = LbTimerClock();
  anim_frame = now / 150;
  for (i=0; i < NET_PLAYERS_COUNT; i++) {
      int32_t slap_frame;

      if (!is_connected_screen_packet(&net_screen_packet[i])) {
        continue;
      }
      slap_frame = 0;
      if (i == my_player_number) {
          slap_frame = get_slap_anim_frame(net_map_local.local_slap_anim_start, now);
      }
      get_hand_packet(i, &nspck, slap_frame, now);
      plyr_nam = network_player_name(i);
      colr = net_player_colours[i];
      spr = get_hand_sprite_for_packet(&nspck, anim_frame, &x, &y);
      x -= (long)map_info.screen_shift_x;
      y -= (long)map_info.screen_shift_y;
      LbSpriteDrawResized(scale_value_landview(x), scale_value_landview(y), units_per_pixel_landview, spr);
      w = LbTextStringWidth(plyr_nam);
      if (w > 0) {
        lbDisplay.DrawFlags = 0;
        h = LbTextHeight(level_name);
        y += 32;
        x += 32;
        LbDrawBox(scale_value_landview(x-4), scale_value_landview(y), scale_value_landview(w+8), scale_value_landview(h), colr);
        LbTextDrawResized(scale_value_landview(x), scale_value_landview(y), units_per_pixel_landview, plyr_nam);
      }
  }
}

void frontnetmap_draw(void)
{
    SYNCDBG(8,"Starting");
    LbTextSetFont(map_font);
    LbTextSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
    if ((map_info.fadeflags & MLInfoFlg_Zooming) != 0) {
        frontzoom_to_point(map_info.hotspot_imgpos_x, map_info.hotspot_imgpos_y, map_info.fade_pos);
        compressed_window_draw();
    } else {
        draw_map_screen();
        draw_map_level_ensigns();
        draw_netmap_players_hands();
        draw_map_level_descriptions();
        compressed_window_draw();
    }
}

void frontnetmap_input(void)
{
  TbBool can_select;
  TbClockMSec now;
  struct LevelInformation* lvinfo;

  if (lbKeyOn[KC_ESCAPE]) {
      fe_net_level_selected = LEVELNUMBER_ERROR;
      lbKeyOn[KC_ESCAPE] = 0;
      SYNCLOG("Escaped from level selection");
      return;
  }

  now = LbTimerClock();
  if (get_local_hand_limp_frame(now) >= 0) {
      return;
  }
  can_select = (get_slap_anim_frame(net_map_local.local_slap_anim_start, now) == 0) && (net_map_local.local_slap_send_frame == 0);

  if (right_button_clicked) {
      right_button_clicked = 0;
      if (fe_net_level_selected != SINGLEPLAYER_NOTSTARTED) {
        lvinfo = get_level_info(fe_net_level_selected);
        if (lvinfo != NULL) {
          LbMouseSetPosition(scale_value_landview(lvinfo->ensign_x - (int32_t)map_info.screen_shift_x),
              scale_value_landview(lvinfo->ensign_y - (int32_t)map_info.screen_shift_y));
        }
        fe_net_level_selected = SINGLEPLAYER_NOTSTARTED;
      } else if (can_select) {
          net_map_local.local_slap_anim_start = now;
          net_map_local.local_slap_send_frame = NetLandSlap_StartFrame;
          can_select = false;
      }
  }

  if (fe_net_level_selected != SINGLEPLAYER_NOTSTARTED) {
      return;
  }

  net_level_hilighted = SINGLEPLAYER_NOTSTARTED;
  frontmap_input_active_ensign(GetMouseX(), GetMouseY());
  if (mouse_over_lvnum > 0) {
    net_level_hilighted = mouse_over_lvnum;
  }
  if ((net_level_hilighted > 0) && can_select && left_button_clicked) {
      fe_net_level_selected = net_level_hilighted;
      left_button_clicked = 0;
      set_level_name_text(fe_net_level_selected, NULL);
      SYNCLOG("Selected level %d with description \"%s\"",(int)fe_net_level_selected,level_name);
  }
  check_mouse_scroll();
  update_velocity();
}

TbBool frontnetmap_load(void)
{
    SYNCDBG(8,"Starting");
    if (fe_network_active) {
      if (LbNetwork_EnableNewPlayers(0)) {
        ERRORLOG("Unable to prohibit new players joining exchange");
      }
    }
    frontend_load_data_from_cd();
    game.selected_level_number = 0;
    if (!load_map_and_window(0)) {
        frontend_load_data_reset();
        return false;
    }
    switch (campaign.land_markers) {
    case LndMk_PINPOINTS:
        map_flag = load_spritesheet("ldata/netflag_pin.dat", "ldata/netflag_pin.tab");
        break;
    default:
        ERRORLOG("Unsupported land markers type %d",(int)campaign.land_markers);
        // Fall Through
    case LndMk_ENSIGNS:
        map_flag = load_spritesheet("ldata/netflag_ens.dat", "ldata/netflag_ens.tab");
        break;
    }
    map_font = load_spritesheet("ldata/netfont.dat", "ldata/netfont.tab");
    map_hand = load_spritesheet("ldata/maphand.dat", "ldata/maphand.tab");
    if (!map_flag || !map_font || !map_hand) {
      ERRORLOG("Unable to load MAP SCREEN sprites");
      return false;
    }
    frontend_load_data_reset();
    //TODO NETWORK Don't allow campaigns besides original - we don't have per-campaign MP yet
    change_campaign("");
    frontmap_zoom_skip_init(SINGLEPLAYER_NOTSTARTED);
    fe_net_level_selected = SINGLEPLAYER_NOTSTARTED;
    net_level_hilighted = SINGLEPLAYER_NOTSTARTED;
    set_pointer_graphic_none();
    LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
    map_sound_fade = FULL_LOUDNESS;
    lbDisplay.DrawFlags = 0;
    set_music_volume(settings.music_volume);
    frontmap_start_music();
    if (fe_network_active) {
        net_number_of_players = 0;
        for (long i = 0; i < NET_PLAYERS_COUNT; i++) {
            struct ScreenPacket* nspck = &net_screen_packet[i];
            if (is_connected_screen_packet(nspck)) {
              net_number_of_players++;
            }
        }
    } else {
      memset(net_screen_packet, 0, sizeof(net_screen_packet));
      net_number_of_players = 1;
    }
    memset(&net_map_local, 0, sizeof(net_map_local));
    memset(net_map_remote_slap, 0, sizeof(net_map_remote_slap));
    update_net_ensigns_visibility();
    return true;
}

static TbBool frontmap_exchange_screen_packet(void)
{
    struct ScreenPacket* nspck = &net_screen_packet[my_player_number];
    TbClockMSec now = LbTimerClock();
    if (net_map_local.local_slap_send_frame > NetLandSlap_EndFrame) {
        net_map_local.local_slap_send_frame = 0;
    }
    get_hand_packet(my_player_number, nspck, net_map_local.local_slap_send_frame, now);
    if (screen_packet_action(nspck) == NetAct_Slapping) {
        if (net_map_local.local_slap_send_frame == NetLandSlap_HitFrame) {
            net_map_local.slap_miss_wait = 2;
        }
        net_map_local.local_slap_send_frame++;
    }
    if (fe_network_active) {
      if (LbNetwork_Exchange(NETMSG_FRONTEND, nspck, &net_screen_packet, sizeof(struct ScreenPacket))) {
          ERRORLOG("LbNetwork_Exchange failed");
          return false;
      }
    }
    return true;
}

static TbBool frontnetmap_update_players(struct NetLandPlayersState * nmps)
{
    memset(scratch, 0, PALETTE_SIZE);
    struct ScreenPacket* my_nspck = &net_screen_packet[my_player_number];
    TbBool slap_hit_confirmed = false;
    int32_t leading_votes = -1;
    for (int32_t i = 0; i < NET_PLAYERS_COUNT; i++) {
        struct ScreenPacket* nspck = &net_screen_packet[i];
        unsigned char action;
        TbBool remote_player;
        if (!is_connected_screen_packet(nspck)) {
          continue;
        }
        remote_player = i != my_player_number;
        if (remote_player && !network_player_active(i)) {
            LbNetwork_EnableNewPlayers(1);
            frontend_set_state(FeSt_NET_START);
            return false;
        }
        if (nspck->action_par1 == LEVELNUMBER_ERROR) {
            if (fe_network_active) {
              if (LbNetwork_EnableNewPlayers(1)) {
                ERRORLOG("Unable to enable new players joining exchange");
              }
              frontend_set_state(FeSt_NET_START);
            } else {
              frontend_set_state(FeSt_MAIN_MENU);
            }
            return false;
        }
        action = screen_packet_action(nspck);
        if ((nspck->action_par1 == SINGLEPLAYER_NOTSTARTED) || (action == NetAct_Slapping)) {
            nmps->unselected_players++;
        } else {
            LevelNumber pckt_lvnum = nspck->action_par1;
            scratch[pckt_lvnum]++;
            if (scratch[pckt_lvnum] == leading_votes) {
                nmps->is_selected = false;
            } else if (scratch[pckt_lvnum] > leading_votes) {
                nmps->selected_level_number = pckt_lvnum;
                leading_votes = scratch[pckt_lvnum];
                nmps->is_selected = true;
            }
        }
        if ((action == NetAct_Slapping) && (nspck->action_par1 == NetLandSlap_HitFrame) && remote_player && (screen_packet_action(my_nspck) != NetAct_Limping)) {
            int32_t hand_x;
            int32_t hand_y;
            const struct TbSprite *spr = get_hand_sprite_for_packet(nspck, 0, &hand_x, &hand_y);
            if ((my_nspck->stored_data1 - 7 < hand_x + spr->SWidth) && (hand_x < my_nspck->stored_data1 + 23)
              && (my_nspck->stored_data2 - 13 < hand_y + spr->SHeight) && (hand_y < my_nspck->stored_data2 + 7)) {
                net_map_local.limp_start = LbTimerClock();
                fe_net_level_selected = SINGLEPLAYER_NOTSTARTED;
                net_map_local.local_slap_anim_start = 0;
                net_map_local.local_slap_send_frame = 0;
                net_map_local.slap_miss_wait = 0;
                net_map_local.limp_x = nspck->stored_data1;
                net_map_local.limp_y = nspck->stored_data2;
                play_non_3d_sample(NETLAND_SLAP_SOUND);
                SYNCLOG("Slapped out of level");
            }
        }
        if (remote_player && (net_map_local.slap_miss_wait > 0) && (screen_packet_action(my_nspck) == NetAct_Slapping)) {
            int32_t hand_x;
            int32_t hand_y;
            const struct TbSprite *spr = get_hand_sprite_for_packet(my_nspck, 0, &hand_x, &hand_y);
            if ((nspck->stored_data1 - 7 < hand_x + spr->SWidth) && (hand_x < nspck->stored_data1 + 23)
              && (nspck->stored_data2 - 13 < hand_y + spr->SHeight) && (hand_y < nspck->stored_data2 + 7)) {
                slap_hit_confirmed = true;
            }
        }
    }
    if (net_map_local.slap_miss_wait > 0) {
        if (slap_hit_confirmed) {
            net_map_local.slap_miss_wait = 0;
        } else {
            net_map_local.slap_miss_wait--;
            if (net_map_local.slap_miss_wait == 0) {
                play_non_3d_sample(NETLAND_SLAP_MISS_SOUND + SOUND_RANDOM(NETLAND_SLAP_MISS_SOUND_VARIANTS));
            }
        }
    }
    return true;
}

TbBool frontnetmap_update(void)
{
    SYNCDBG(8,"Starting");
    set_music_volume((map_sound_fade * settings.music_volume) / FULL_LOUDNESS);

    struct NetLandPlayersState nmps = { 0, SINGLEPLAYER_NOTSTARTED, false };
    if ((map_info.fadeflags & MLInfoFlg_Zooming) != 0) {
        if (frontmap_update_zoom()) {
          SYNCDBG(8,"Zoom end");
          return true;
        }
    } else {
        frontmap_exchange_screen_packet();
        frontnetmap_update_players(&nmps);
    }
    if ((!nmps.unselected_players) && (nmps.selected_level_number > 0) && (nmps.is_selected)) {
        set_selected_level_number(nmps.selected_level_number);
        set_level_name_text(nmps.selected_level_number, NULL);
        if (fe_network_active < 1) {
            map_info.state_trigger = FeSt_START_KPRLEVEL;
        } else {
            map_info.state_trigger = FeSt_START_MPLEVEL;
        }
        frontmap_zoom_in_init(nmps.selected_level_number);
        if (!fe_network_active) {
            fe_computer_players = 1;
        }
    }
    SYNCDBG(8,"Normal end");
    return false;
}

/******************************************************************************/
