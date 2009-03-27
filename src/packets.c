/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets.c
 *     Packet processing routines.
 * @par Purpose:
 *     Functions for creating and executing packets.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     30 Jan 2009 - 11 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "packets.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_keybrd.h"
#include "bflib_vidraw.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "kjm_input.h"
#include "front_simple.h"
#include "frontend.h"
#include "vidmode.h"
#include "config.h"
#include "player_instances.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_process_pause_packet(long a1, long a2);
DLLIMPORT void _DK_process_map_packet_clicks(long idx);
DLLIMPORT void _DK_process_quit_packet(struct PlayerInfo *, int);
DLLIMPORT void _DK_process_dungeon_control_packet_clicks(long idx);
DLLIMPORT void _DK_process_players_dungeon_control_packet_control(long idx);
DLLIMPORT void _DK_process_packets(void);
DLLIMPORT char _DK_process_players_global_packet_action(long idx);
DLLIMPORT void _DK_process_players_dungeon_control_packet_action(long idx);
DLLIMPORT void _DK_process_players_creature_control_packet_control(long idx);
DLLIMPORT void _DK_process_players_creature_control_packet_action(long idx);
DLLIMPORT unsigned long _DK_get_packet_save_checksum(void);
DLLIMPORT void _DK_load_packets_for_turn(long gameturn);
DLLIMPORT void _DK_open_new_packet_file_for_save(void);
DLLIMPORT void _DK_open_packet_file_for_load(char *fname);
DLLIMPORT void _DK_set_packet_action(struct Packet *pckt,unsigned char,short,short,short,short);

/******************************************************************************/
#define PACKET_START_POS (sizeof(struct PacketSaveHead))
#define PACKET_TURN_SIZE (NET_PLAYERS_COUNT*sizeof(struct Packet) + sizeof(TbBigChecksum))

/******************************************************************************/
void set_packet_action(struct Packet *pckt, unsigned char pcktype, unsigned short par1, unsigned short par2, unsigned short par3, unsigned short par4)
{
  pckt->field_6 = par1;
  pckt->field_8 = par2;
  pckt->field_5 = pcktype;
}

void set_packet_control(struct Packet *pckt, unsigned long val)
{
  pckt->field_E |= val;
}

void clear_packets(void)
{
  int i;
  for (i=0; i < PACKETS_COUNT; i++)
    memset(&game.packets[i], 0, sizeof(struct Packet));
}

short set_packet_pause_toggle(void)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  if (my_player_number >= PLAYERS_COUNT)
    return false;
  player=&(game.players[my_player_number]);
  if (player->packet_num >= PACKETS_COUNT)
    return false;
  pckt = &game.packets[player->packet_num];
  set_packet_action(pckt, PckT_TogglePause, 0, 0, 0, 0);
  return true;
}

unsigned long compute_players_checksum(void)
{
  struct PlayerInfo *player;
  struct Coord3d *mappos;
  int i;
  unsigned long sum;
  sum = 0;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i%PLAYERS_COUNT]);
    if (((player->field_0 & 0x01) != 0) && ((player->field_0 & 0x40) == 0))
    {
        mappos = &(player->camera->mappos);
        sum += player->field_4B1 + player->instance_num
                   + mappos->x.val + mappos->z.val + mappos->y.val;
    }
  }
  return sum;
}

/*
 * Checks if all active players packets have same checksums.
 * @return Returns false if all checksums are same; true if there's mismatch.
 */
short checksums_different(void)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  TbChecksum checksum;
  unsigned short is_set;
  int i;
  is_set = false;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i]);
    if (((player->field_0 & 0x01) != 0) && ((player->field_0 & 0x40) == 0))
    {
        pckt = &game.packets[player->packet_num];
        if (!is_set)
        {
          checksum = pckt->chksum;
          is_set = true;
        } else
        if (checksum != pckt->chksum)
        {
          return true;
        }
    }
  }
  return false;
}

void process_dungeon_control_packet_clicks(long idx)
{
  static const char *func_name="process_dungeon_control_packet_clicks";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_dungeon_control_packet_clicks(idx);
}

TbBigChecksum get_packet_save_checksum(void)
{
  return _DK_get_packet_save_checksum();
}

void open_new_packet_file_for_save(void)
{
  static const char *func_name="open_new_packet_file_for_save";
  struct PlayerInfo *player;
  int i;
  //_DK_open_new_packet_file_for_save(); return;
  // Filling the header
  game.packet_save_head.field_0 = 0;
  game.packet_save_head.level_num = game.level_file_number;
  game.packet_save_head.field_8 = 0;
  game.packet_save_head.field_C = 0;
  game.packet_save_head.field_D = 0;
  game.packet_save_head.chksum = game.packet_checksum;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i%PLAYERS_COUNT]);
    if (player->field_0 & 0x01)
    {
      game.packet_save_head.field_C |= (1 << i) & 0xff;
      if (player->field_0 & 0x40)
        game.packet_save_head.field_D |= (1 << i) & 0xff;
    }
  }
  LbFileDelete(game.packet_fname);
  game.packet_save_fp = LbFileOpen(game.packet_fname, Lb_FILE_MODE_NEW);
  if (game.packet_save_fp == -1)
  {
    error(func_name, 3774, "Cannot open keeper packet file for save");
    return;
  }
  game.packet_fopened = 1;
  LbFileWrite(game.packet_save_fp, &game.packet_save_head, sizeof(struct PacketSaveHead));
}

void load_packets_for_turn(long nturn)
{
  static const char *func_name="load_packets_for_turn";
  struct PlayerInfo *player;
  struct Packet *pckt;
  TbChecksum pckt_chksum;
  TbBigChecksum tot_chksum;
  long data_size;
  short done;
  char *text;
  long i;
  const int turn_data_size = PACKET_TURN_SIZE;
  unsigned char pckt_buf[PACKET_TURN_SIZE+4];
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
  pckt_chksum = pckt->chksum;
  if (nturn >= game.field_149F30)
  {
    error(func_name, 3973, "Out of turns to load from Packet File");
    return;
  }

  data_size = PACKET_START_POS + turn_data_size*nturn;
  if (data_size != game.packet_file_pos)
  {
    error(func_name, 3879, "Packet Loading Seek Offset is wrong");
    LbFileSeek(game.packet_save_fp, data_size, Lb_FILE_SEEK_BEGINNING);
    game.packet_file_pos = data_size;
  }
  if (LbFileRead(game.packet_save_fp, &pckt_buf, turn_data_size) == -1)
  {
    error(func_name, 3879, "Cannot read turn data from Packet File");
    return;
  }
  game.packet_file_pos += turn_data_size;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
    LbMemoryCopy(&game.packets[i], &pckt_buf[i*sizeof(struct Packet)], sizeof(struct Packet));
  tot_chksum = llong(&pckt_buf[NET_PLAYERS_COUNT*sizeof(struct Packet)]);
  if (game.numfield_C & 0x01)
  {
      done = false;
      while (!done)
      {
        for (i=0; i<NET_PLAYERS_COUNT; i++)
        {
          pckt = &game.packets[i];
          if ((pckt->field_5 != 0) || (pckt->field_E != 0))
          {
            done = true;
            break;
          }
        }
        data_size += turn_data_size;
        game.pckt_gameturn++;
        if (data_size != game.packet_file_pos)
        {
          error(func_name, 3914, "Packet Saving Seek Offset is wrong");
          LbFileSeek(game.packet_save_fp, data_size, Lb_FILE_SEEK_BEGINNING);
          game.packet_file_pos = data_size;
        }
        if (LbFileRead(game.packet_save_fp, &pckt_buf, turn_data_size) == -1)
        {
          error(func_name, 3451, "Cannot read turn data from Packet File");
          return;
        }
        game.packet_file_pos += turn_data_size;
        for (i=0; i < NET_PLAYERS_COUNT; i++)
          LbMemoryCopy(&game.packets[i], &pckt_buf[i*sizeof(struct Packet)], sizeof(struct Packet));
        tot_chksum = llong(&pckt_buf[NET_PLAYERS_COUNT*sizeof(struct Packet)]);
      }
  }
  if (game.turns_fastforward > 0)
      game.turns_fastforward--;
  if (game.packet_checksum)
  {
      player=&(game.players[my_player_number%PLAYERS_COUNT]);
      pckt = &game.packets[player->packet_num%PACKETS_COUNT];
      if (get_packet_save_checksum() != tot_chksum)
      {
        text = buf_sprintf("PacketSave checksum - Out of sync (GameTurn %d)", game.seedchk_random_used);
        error(func_name, 3947, text);
        if (!is_onscreen_msg_visible())
          show_onscreen_msg(game.num_fps, "Out of sync");
      } else
      if (pckt->chksum != pckt_chksum)
      {
        text = buf_sprintf("Opps we are really Out Of Sync (GameTurn %d)", game.seedchk_random_used);
        error(func_name, 3955, text);
        if (!is_onscreen_msg_visible())
          show_onscreen_msg(game.num_fps, "Out of sync");
      }
  }
}

void process_pause_packet(long a1, long a2)
{
  _DK_process_pause_packet(a1, a2);
}

void process_players_dungeon_control_packet_control(long idx)
{
  static const char *func_name="process_players_dungeon_control_packet_control";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_process_players_dungeon_control_packet_control(idx); return;
  struct PlayerInfo *player;
  struct Packet *pckt;
  struct Camera *cam;
  unsigned long zoom_min,zoom_max;

  player = &(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
  cam = player->camera;
  long inter_val;
  switch (cam->field_6)
  {
  case 2:
      inter_val = 2560000 / cam->field_17;
      break;
  case 5:
      inter_val = 12800000 / cam->field_17;
      break;
  default:
      inter_val = 256;
      break;
  }
  if (pckt->field_10 & 0x01)
    inter_val *= 3;

  if (pckt->field_E & 0x04)
    view_set_camera_y_inertia(cam, -inter_val/4, -inter_val);
  if (pckt->field_E & 0x08)
    view_set_camera_y_inertia(cam, inter_val/4, inter_val);
  if (pckt->field_E & 0x10)
    view_set_camera_x_inertia(cam, -inter_val/4, -inter_val);
  if (pckt->field_E & 0x20)
    view_set_camera_x_inertia(cam, inter_val/4, inter_val);
  if (pckt->field_E & 0x02)
  {
    switch (cam->field_6)
    {
    case 2:
         view_set_camera_rotation_inertia(cam, 16, 64);
        break;
    case 5:
        cam->orient_a = (cam->orient_a + 512) & 0x7FF;
        break;
    }
  }
  if (pckt->field_E & 0x01)
  {
    switch (cam->field_6)
    {
    case 2:
        view_set_camera_rotation_inertia(cam, -16, -64);
        break;
    case 5:
        cam->orient_a = (cam->orient_a - 512) & 0x7FF;
        break;
    }
  }
  zoom_min = scale_camera_zoom_to_screen(CAMERA_ZOOM_MIN);
  zoom_max = scale_camera_zoom_to_screen(CAMERA_ZOOM_MAX);
  if (pckt->field_E & 0x40)
  {
    switch (cam->field_6)
    {
    case 2:
        view_zoom_camera_in(cam, zoom_max, zoom_min);
        update_camera_zoom_bounds(cam, zoom_max, zoom_min);
        break;
    default:
        view_zoom_camera_in(cam, zoom_max, zoom_min);
        break;
    }
  }
  if (pckt->field_E & 0x80)
  {
    switch (cam->field_6)
    {
    case 2:
        view_zoom_camera_out(cam, zoom_max, zoom_min);
        update_camera_zoom_bounds(cam, zoom_max, zoom_min);
        break;
    default:
        view_zoom_camera_out(cam, zoom_max, zoom_min);
        break;
    }
  }
  process_dungeon_control_packet_clicks(idx);
  set_mouse_light(player);
}

void process_players_message_character(struct PlayerInfo *player)
{
  struct Packet *pcktd;
  struct PlayerInfo *playerd;
  char chr;
  int chpos;
  playerd = &(game.players[player->field_2B%PLAYERS_COUNT]);
  pcktd = &game.packets[playerd->packet_num%PACKETS_COUNT];
  if (pcktd->field_6 >= 0)
  {
    chr = key_to_ascii(pcktd->field_6, pcktd->field_8);
    chpos = strlen(player->strfield_463);
    if (pcktd->field_6 == KC_BACK)
    {
      if (chpos>0)
        player->strfield_463[chpos-1] = '\0';
    } else
    if ((chr >= 'a') && (chr <= 'z') ||
        (chr >= 'A') && (chr <= 'Z') ||
        (chr >= '0') && (chr <= '9') || (chr == ' ')  || (chr == '!') ||
        (chr == '.'))
    {
      if (chpos < 63)
      {
        player->strfield_463[chpos] = toupper(chr);
        player->strfield_463[chpos+1] = '\0';
      }
    }
  }
}

void process_quit_packet(struct PlayerInfo *player, int a2)
{
  _DK_process_quit_packet(player,a2);
}

char process_players_global_packet_action(long plyridx)
{
  static const char *func_name="process_players_global_packet_action";
  //TODO: add commands from beta
  //return _DK_process_players_global_packet_action(plyridx);
  struct PlayerInfo *player;
  struct PlayerInfo *myplyr;
  struct Packet *pckt;
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Room *room;
  int i;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[plyridx%PLAYERS_COUNT]);
  pckt=&game.packets[player->packet_num%PACKETS_COUNT];
  switch (pckt->field_5)
  {
    case 1:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (my_player_number == plyridx)
      {
        turn_off_all_menus();
        update_continue_game();
        free_swipe_graphic();
      }
      player->field_6 |= 0x02;
      process_quit_packet(player, 0);
      return 1;
    case 3:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (my_player_number == plyridx)
      {
        turn_off_all_menus();
        update_continue_game();
      }
      player->field_6 |= 0x02;
      process_quit_packet(player, 1);
      return 1;
    case 4:
      return 1;
    case 5:
      if (my_player_number == plyridx)
      {
        turn_off_all_menus();
        free_swipe_graphic();
      }
      if ((game.numfield_A & 0x01) != 0)
      {
        process_quit_packet(player, 0);
        return 0;
      }
      switch (player->field_29)
      {
      case 1:
          complete_level(player);
          break;
      case 2:
          lose_level(player);
          break;
      }
      player->field_0 &= 0xFEu;
      if (my_player_number == plyridx)
      {
        unsigned int k;
        // Save some of the data from clearing
        myplyr = &(game.players[my_player_number%PLAYERS_COUNT]);
        dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
        i = myplyr->field_29;
        // The block started at field_1197 ends before field_131F
        memcpy(scratch, &dungeon->lvstats, sizeof(struct LevelStats));
        k = (myplyr->field_3 & 0x10) >> 4;
        // clear all data
        clear_game_for_save();
        // Restore saved data
        myplyr->field_29 = i;
        memcpy(&dungeon->lvstats, scratch, sizeof(struct LevelStats));
        myplyr->field_3 ^= (myplyr->field_3 ^ (k << 4)) & 0x10;
        if (((game.numfield_A & 0x01) == 0) && (!game.packet_load_enable))
          frontend_save_continue_game(game.level_number, true);
      }
      return 0;
    case PckT_PlyrMsgBegin:
      player->field_0 |= 0x04;
      return 0;
    case PckT_PlyrMsgEnd:
      player->field_0 &= 0xFBu;
      if (player->strfield_463[0] != '\0')
        message_add(player->field_2B);
      memset(player->strfield_463, 0, 64);
      return 0;
    case PckT_ToggleLights:
      if (my_player_number == plyridx)
        light_set_lights_on(game.field_4614D == 0);
      return 1;
    case PckT_SwitchScrnRes:
      if (my_player_number == plyridx)
        switch_to_next_video_mode();
      return 1;
    case 22:
      process_pause_packet((game.numfield_C & 0x01) == 0, pckt->field_6);
      return 1;
    case 24:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (myplyr->field_2B == plyridx)
      {
        settings.video_cluedo_mode = pckt->field_6;
        save_settings();
      }
      player->field_4DA = pckt->field_6;
      return 0;
    case 25:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (myplyr->field_2B == player->field_2B)
      {
        change_engine_window_relative_size(pckt->field_6, pckt->field_8);
        centre_engine_window();
      }
      return 0;
    case 26:
      player->field_90 = pckt->field_6 << 8;
      player->field_BA = pckt->field_6 << 8;
      player->cam_mappos.x.val = pckt->field_6 << 8;
      player->field_92 = pckt->field_8 << 8;
      player->field_BC = pckt->field_8 << 8;
      player->cam_mappos.y.val = pckt->field_8 << 8;
      return 0;
    case PckT_SetGammaLevel:
      if (myplyr->field_2B == player->field_2B)
      {
        set_gamma(pckt->field_6, 1);
        save_settings();
      }
      return 0;
    case PckT_SetMinimapConf:
      player->minimap_zoom = pckt->field_6;
      return 0;
    case 29:
      player->field_97 = pckt->field_6;
      player->field_C1 = pckt->field_6;
      player->field_43 = pckt->field_6;
      return 0;
    case 36:
      set_player_state(player, pckt->field_6, pckt->field_8);
      return 0;
    case 37:
      set_engine_view(player, pckt->field_6);
      return 0;
    case 55:
      toggle_creature_tendencies(player, pckt->field_6);
      return 0;
    case 60:
//      game.???[my_player_number].cheat_mode = 1;
      show_onscreen_msg(2*game.num_fps, "Cheat mode activated by player %d", plyridx);
      return 1;
    case 61:
      //TODO: remake from beta
/*
      if (word_5E674A != 0)
        word_5E674A = 0;
      else
        word_5E674A = 15;
*/
      return 1;
    case 62:
      //TODO: remake from beta
      return 0;
    case 63:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      reveal_whole_map(myplyr);
      return 0;
    case 64:
      //TODO: remake from beta
      return 0;
    case 65:
      //TODO: remake from beta
      return 0;
    case 66:
      //TODO: remake from beta
      return 0;
    case 67:
      //TODO: remake from beta
      return 0;
    case 68:
      //TODO: remake from beta
      return 0;
    case 69:
      //TODO: remake from beta
      return 0;
    case 80:
      set_player_mode(player, pckt->field_6);
      return 0;
    case 81:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      player->field_90 = pckt->field_6 << 8;
      player->field_BA = pckt->field_6 << 8;
      player->cam_mappos.x.val = pckt->field_6 << 8;
      player->field_92 = pckt->field_8 << 8;
      player->field_BC = pckt->field_8 << 8;
      player->cam_mappos.y.val = pckt->field_8 << 8;
      player->field_97 = 0;
      player->field_C1 = 0;
      player->field_43 = 0;
      if ((game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320))
      {
        if (my_player_number == plyridx)
          toggle_status_menu((game.numfield_C & 0x40) != 0);
        set_player_mode(player, 1);
      } else
      {
        set_player_mode(player, 6);
      }
      return 0;
    case 82:
      process_pause_packet(pckt->field_6, pckt->field_8);
      return 1;
    case 83:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      event_move_player_towards_event(player, pckt->field_6);
      return 0;
    case 84:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      room = &game.rooms[pckt->field_6];
      player->field_E4 = room->field_8 << 8;
      player->field_E6 = room->field_9 << 8;
      set_player_instance(player, 16, 0);
      if (player->field_453 == 2)
        set_player_state(player, 2, room->kind);
      return 0;
    case 85:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      thing = game.things_lookup[pckt->field_6];
      player->field_E4 = thing->mappos.x.val;
      player->field_E6 = thing->mappos.y.val;
      set_player_instance(player, 16, 0);
      if ((player->field_453 == 16) || (player->field_453 == 18))
        set_player_state(player, 16, thing->model);
      return 0;
    case 86:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      thing = game.things_lookup[pckt->field_6];
      player->field_E4 = thing->mappos.x.val;
      player->field_E6 = thing->mappos.y.val;
      set_player_instance(player, 16, 0);
      if ((player->field_453 == 16) || (player->field_453 == 18))
        set_player_state(player, 18, thing->model);
      return 0;
    case 87:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      player->field_E4 = pckt->field_6;
      player->field_E6 = pckt->field_8;
      set_player_instance(player, 16, 0);
      return 0;
    case 88:
      game.numfield_D ^= (game.numfield_D ^ (0x04 * ((game.numfield_D & 0x04) == 0))) & 0x04;
      return 0;
    case 89:
      turn_off_call_to_arms(plyridx);
      return 0;
    case 90:
      if (game.dungeon[plyridx].field_63 < 8)
        place_thing_in_power_hand(game.things_lookup[pckt->field_6], plyridx);
      return 0;
    case 91:
      dump_held_things_on_map(plyridx, pckt->field_6, pckt->field_8, 1);
      return 0;
    case 92:
      if (game.event[pckt->field_6].field_B == 3)
      {
        turn_off_event_box_if_necessary(plyridx, pckt->field_6);
      } else
      {
        event_delete_event(plyridx, pckt->field_6);
      }
      return 0;
    case 97:
      magic_use_power_obey(plyridx);
      return 0;
    case 98:
      magic_use_power_armageddon(plyridx);
      return 0;
    case 99:
      turn_off_query(plyridx);
      return 0;
    case 104:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      battle_move_player_towards_battle(player, pckt->field_6);
      return 0;
    case 106:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      dungeon = &(game.dungeon[plyridx]);
      switch (pckt->field_6)
      {
      case 5:
          if (dungeon->field_5D8)
          {
            struct Thing *thing;
            thing = game.things_lookup[dungeon->field_5D8];
            player->field_E4 = thing->mappos.x.val;
            player->field_E6 = thing->mappos.y.val;
            set_player_instance(player, 16, 0);
          }
          break;
      case 6:
          if (dungeon->field_884)
          {
            player->field_E4 = ((unsigned long)dungeon->field_881) << 8;
            player->field_E6 = ((unsigned long)dungeon->field_882) << 8;
            set_player_instance(player, 16, 0);
          }
          break;
      }
      if ( spell_data[pckt->field_6].field_0 )
      {
        for (i=0; i<21; i++)
        {
          if (spell_data[i].field_4 == player->field_453)
          {
            set_player_state(player, spell_data[pckt->field_6].field_4, 0);
            break;
          }
        }
      }
      return 0;
    case PckT_PlyrFastMsg:
      //show_onscreen_msg(game.num_fps, "Message from player %d", plyridx);
      output_message(pckt->field_6+110, 0, 1);
      return 0;
    case 109:
      set_autopilot_type(plyridx, pckt->field_6);
      return 0;
    case 110:
      level_lost_go_first_person(plyridx);
      return 0;
    case 111:
      if (game.dungeon[plyridx].field_63)
      {
        i = game.dungeon[plyridx].field_33;
        thing=game.things_lookup[i%THINGS_COUNT];
        dump_held_things_on_map(plyridx, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 1);
      }
      return 0;
    case PckT_SpellSOEDis:
      turn_off_sight_of_evil(plyridx);
      return 0;
    case 115:
      go_on_then_activate_the_event_box(plyridx, pckt->field_6);
      return 0;
    case 116:
      turn_off_event_box_if_necessary(plyridx, game.dungeon[plyridx].field_1173);
      game.dungeon[plyridx].field_1173 = 0;
      return 0;
    case 117:
      i = player->field_4D2 / 4;
      if (i > 8) i = 8;
      directly_cast_spell_on_thing(plyridx, pckt->field_6, pckt->field_8, i);
      return 0;
    case PckT_PlyrToggleAlly:
      toggle_ally_with_player(plyridx, pckt->field_6);
      return 0;
    case 119:
      player->field_4B5 = player->camera->field_6;
      set_player_mode(player, pckt->field_6);
      return 0;
    case 120:
      set_player_mode(player, pckt->field_6);
      set_engine_view(player, player->field_4B5);
      return 0;
    default:
      return 0;
  }
}

void process_players_map_packet_control(long idx)
{
  static const char *func_name="process_players_map_packet_control";
  struct PlayerInfo *player;
  struct Packet *pckt;
  unsigned short x,y;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
  x = (3*pckt->field_A - 450)/4 - 6;
  y = (3*pckt->field_C - 168)/4 - 6;
  process_map_packet_clicks(idx);
  player->field_90 = (x << 8) + 1920;
  player->field_92 = (y << 8) + 1920;
  set_mouse_light(player);
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void process_map_packet_clicks(long idx)
{
  static const char *func_name="process_map_packet_clicks";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_map_packet_clicks(idx);
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void process_players_packet(long idx)
{
  static const char *func_name="process_players_packet";
  struct PlayerInfo *player;
  struct Packet *pckt;
  player=&(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Processing player %d packet of type %d.\n",func_name,idx,(int)pckt->field_5);
#endif
  player->field_4 = (pckt->field_10 & 0x20) >> 5;
  player->field_5 = (pckt->field_10 & 0x40) >> 6;
  if ( (player->field_0 & 0x04) && (pckt->field_5 == PckT_PlyrMsgChar))
  {
     process_players_message_character(player);
  } else
  if ( !process_players_global_packet_action(idx) )
  {
      switch (player->view_type)
      {
      case PVT_DungeonTop:
        process_players_dungeon_control_packet_control(idx);
        process_players_dungeon_control_packet_action(idx);
        break;
      case PVT_CreatureContrl:
        process_players_creature_control_packet_control(idx);
        process_players_creature_control_packet_action(idx);
        break;
      case PVT_CreaturePasngr:
        process_players_creature_passenger_packet_action(idx);
        break;
      case PVT_MapScreen:
        process_players_map_packet_control(idx);
        break;
      default:
        break;
      }
  }
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void process_players_creature_passenger_packet_action(long idx)
{
  static const char *func_name="process_players_creature_passenger_packet_action";
  struct PlayerInfo *player;
  struct Packet *pckt;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
  if (pckt->field_5 == 32)
  {
    player->field_43E = pckt->field_6;
    set_player_instance(player, 8, 0);
  }
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void process_players_dungeon_control_packet_action(long idx)
{
  static const char *func_name="process_players_dungeon_control_packet_action";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_process_players_dungeon_control_packet_action(idx); return;
  struct PlayerInfo *player;
  struct Packet *pckt;
  player = &(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
  switch (pckt->field_5)
  {
    case 41:
      magic_use_power_hold_audience(idx);
      break;
    case 93:
      activate_dungeon_special(game.things_lookup[pckt->field_6%THINGS_COUNT], player);
      break;
    case 95:
      resurrect_creature(game.things_lookup[pckt->field_6%THINGS_COUNT],
        (pckt->field_8) & 0x0F, (pckt->field_8 >> 4) & 0xFF, (pckt->field_8 >> 12) & 0x0F);
      break;
    case 96:
      transfer_creature(game.things_lookup[pckt->field_6%THINGS_COUNT],
          game.things_lookup[pckt->field_8%THINGS_COUNT], idx);
      break;
    case 107:
      toggle_computer_player(idx);
      break;
  }
}

void process_players_creature_control_packet_control(long idx)
{
  static const char *func_name="process_players_creature_control_packet_control";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_players_creature_control_packet_control(idx);
}

void process_players_creature_control_packet_action(long idx)
{
  static const char *func_name="process_players_creature_control_packet_action";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_process_players_creature_control_packet_action(idx); return;
  struct CreatureControl *cctrl;
  struct PlayerInfo *player;
  struct Thing *thing;
  struct Packet *pckt;
  long i,k;

  player = &(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
  switch (pckt->field_5)
  {
  case 33:
      player->field_43E = pckt->field_6;
      set_player_instance(player, 7, 0);
      break;
  case 39:
      thing = NULL;
      i = player->field_2F;
      if ((i>0) && (i<THINGS_COUNT))
        thing = game.things_lookup[i];
      if ((thing == NULL) || (thing == game.things_lookup[0]))
        break;
      cctrl = game.persons.cctrl_lookup[thing->field_64%CREATURES_COUNT];
      if ((cctrl == NULL) || (cctrl == game.persons.cctrl_lookup[0]))
        break;
      i = pckt->field_6;
      if (!instance_info[i].field_0)
      {
        cctrl->instances[74] = i;
      } else
      if (cctrl->field_D2 == 0)
      {
        if (creature_instance_is_available(thing,i) && creature_instance_has_reset(thing, pckt->field_6))
        {
          i = pckt->field_6;
          k = get_human_controlled_creature_target(thing, instance_info[i].field_1D);
          set_creature_instance(thing, i, 1, k, 0);
          if (idx == my_player_number)
            instant_instance_selected(i);
        }
      }
      break;
  }
}

void open_packet_file_for_load(char *fname)
{
  static const char *func_name="open_packet_file_for_load";
  //_DK_open_packet_file_for_load(fname); return;
  strcpy(game.packet_fname, fname);
  game.packet_save_fp = LbFileOpen(game.packet_fname, Lb_FILE_MODE_READ_ONLY);
  if (game.packet_save_fp == -1)
  {
    error(func_name, 3835, "Cannot open keeper packet file for load");
    return;
  }
  LbFileRead(game.packet_save_fp, &game.packet_save_head, sizeof(struct PacketSaveHead));
  game.packet_file_pos = PACKET_START_POS;
  game.field_149F30 = (LbFileLengthRnc(fname) - PACKET_START_POS) / PACKET_TURN_SIZE;
  if ((game.packet_checksum) && (!game.packet_save_head.chksum))
  {
      LbWarnLog("PacketSave checksum not available, checking disabled.\n");
      game.packet_checksum = false;
  }
  if (game.numfield_149F3A == -1)
  {
    game.numfield_149F3A = 0;
    game.numfield_149F3E = game.field_149F30 + 1;
  }
  game.packet_fopened = 1;
}

void post_init_packets(void)
{
  if ((game.packet_load_enable) && (game.numfield_149F47))
  {
      open_packet_file_for_load(game.packet_fname);
      game.pckt_gameturn = 0;
  }
  clear_packets();
}

short save_packets(void)
{
  static const char *func_name="save_packets";
  const int turn_data_size = PACKET_TURN_SIZE;
  unsigned char pckt_buf[PACKET_TURN_SIZE+4];
  TbBigChecksum chksum;
  int i;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  if (game.packet_checksum)
    chksum = get_packet_save_checksum();
  else
    chksum = 0;
  LbFileSeek(game.packet_save_fp, 0, Lb_FILE_SEEK_END);
  // Prepare data in the buffer
  for (i=0; i<NET_PLAYERS_COUNT; i++)
    LbMemoryCopy(&pckt_buf[i*sizeof(struct Packet)], &game.packets[i], sizeof(struct Packet));
  LbMemoryCopy(&pckt_buf[NET_PLAYERS_COUNT*sizeof(struct Packet)], &chksum, sizeof(TbBigChecksum));
  // Write buffer into file
  if (LbFileWrite(game.packet_save_fp, &pckt_buf, turn_data_size) != turn_data_size)
  {
    error(func_name, 3818, "Packet file write error");
  }
  if ( !LbFileFlush(game.packet_save_fp) )
  {
    error(func_name, 3821, "Unable to flush PacketSave File");
    return false;
  }
  return true;
}

void close_packet_file(void)
{
  if ( game.packet_fopened )
  {
    LbFileClose(game.packet_save_fp);
    game.packet_fopened = 0;
    game.packet_save_fp = 0;
  }
}

void write_debug_packets(void)
{
  FILE *file;
  file = fopen("keeperd.pck", "w");
  fwrite(game.packets, 1, sizeof(struct Packet)*PACKETS_COUNT, file);
  fflush(file);
  fclose(file);
}

void process_packets(void)
{
  static const char *func_name="process_packets";
  //_DK_process_packets();return;

  int i,j,k;
  struct Packet *pckt;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  // Do the network data exchange
  lbDisplay.DrawColour = colours[15][15][15];
  // Exchange packets with the network
  if (game.flagfield_14EA4A != 2)
  {
    player=&(game.players[my_player_number%PLAYERS_COUNT]);
    j=0;
    for (i=0; i<4; i++)
    {
      if (net_player_info[i].field_20 != 0)
        j++;
    }
    if ( !game.packet_load_enable || game.numfield_149F47 )
    {
      pckt = &game.packets[player->packet_num%PACKETS_COUNT];
      if (LbNetwork_Exchange(pckt) != 0)
      {
        error(func_name, 426, "LbNetwork_Exchange failed");
      }
    }
    k=0;
    for (i=0; i<4; i++)
    {
      if (net_player_info[i].field_20 != 0)
        k++;
    }
    if (j != k)
    {
      for (i=0; i<4; i++)
      {
        player=&(game.players[i]);
        if (net_player_info[player->packet_num%NET_PLAYERS_COUNT].field_20 == 0)
        {
          player->field_0 |= 0x40;
          toggle_computer_player(i);
        }
      }
    }
  }
  // Setting checksum problem flags
  switch (checksums_different())
  {
  case 1:
    game.numfield_A |= 0x02;
    game.numfield_A &= 0xFB;
    break;
  case 2:
    game.numfield_A |= 0x04;
    game.numfield_A &= 0xFD;
    break;
  case 3:
    game.numfield_A |= 0x04;
    game.numfield_A |= 0x02;
    break;
  default:
    game.numfield_A &= 0xFD;
    game.numfield_A &= 0xFB;
    break;
  }
  // Write packets into file, if requested
  if ((game.packet_save_enable) && (game.packet_fopened))
    save_packets();
//Debug code, to find packet errors
//write_debug_packets();
  // Process the packets
  for (i=0; i<PACKETS_COUNT; i++)
  {
    player=&(game.players[i]);
    if (((player->field_0 & 0x01) != 0) && ((player->field_0 & 0x40) == 0))
      process_players_packet(i);
  }
  // Clear all packets
  for (i=0; i<PACKETS_COUNT; i++)
    memset(&game.packets[i], 0, sizeof(struct Packet));
  if ((game.numfield_A & 0x02) || (game.numfield_A & 0x04))
  {
  #if (BFDEBUG_LEVEL > 0)
    LbSyncLog("%s: Resyncing.\n",func_name);
  #endif
    resync_game();
  }
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}



/******************************************************************************/
#ifdef __cplusplus
}
#endif
