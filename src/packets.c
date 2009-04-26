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
  pckt->action = pcktype;
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
  set_packet_action(pckt, PckA_TogglePause, 0, 0, 0, 0);
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
    if (((player->field_0 & 0x01) != 0) && ((player->field_0 & 0x40) == 0) && (player->acamera != NULL))
    {
        mappos = &(player->acamera->mappos);
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

void process_dungeon_control_packet_clicks(long plyr_idx)
{
  static const char *func_name="process_dungeon_control_packet_clicks";
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Packet *pckt;
  struct CreatureControl *cctrl;
  struct Thing *thing;
  long stl_x,stl_y;
  short v172;
  struct Coord3d pos;
  long x,y;
  long i,k;
  //_DK_process_dungeon_control_packet_clicks(plyr_idx); return;
  player = &(game.players[plyr_idx%PLAYERS_COUNT]);
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting for state %d\n",func_name,(int)player->field_453);
#endif
  player->field_4A4 = 1;
  packet_left_button_double_clicked[plyr_idx] = 0;
  if ((pckt->field_E & 0x4000) != 0)
    return;

//TODO: finish rewriting!!!
// HACK to make not rewritten part work
  switch (player->field_453)
  {
  case 1:
  case 2:
  case 16:
  case 23:
      _DK_process_dungeon_control_packet_clicks(plyr_idx);
      return;
  }

  if ((pckt->field_E & 0x0400) != 0)
  {
    switch (player->field_453)
    {
    case 6:
        if (dungeon->field_884)
          player->field_4D2 = (dungeon->field_883 << 2);
        else
          update_spell_overcharge(player, 6);
        break;
    case 7:
        update_spell_overcharge(player, 7);
        break;
    case 8:
        update_spell_overcharge(player, 5);
        break;
    case 17:
        update_spell_overcharge(player, 10);
        break;
    case 19:
        update_spell_overcharge(player, 11);
        break;
    case 20:
        update_spell_overcharge(player, 12);
        break;
    case 21:
        update_spell_overcharge(player, 13);
        break;
    case 22:
        update_spell_overcharge(player, 8);
        break;
    default:
        player->field_4D2++;
        break;
    }
  } else
  if ((pckt->field_E & 0x1000) == 0)
  {
    player->field_4D2 = 0;
  }
  if ((pckt->field_E & 0x0800) != 0)
  {
    player->field_4D6++;
  } else
  if ((pckt->field_E & 0x2000) == 0)
  {
    player->field_4D6 = 0;
  }
  if ((pckt->field_E & 0x1000) != 0)
  {
    if (packet_left_button_click_space_count[plyr_idx] < 5)
      packet_left_button_double_clicked[plyr_idx] = 1;
    packet_left_button_click_space_count[plyr_idx] = 0;
  }
  i = (unsigned short)pckt->field_E;
  if (((i >> 8) & 5) == 0)
  {
    if (packet_left_button_click_space_count[plyr_idx] < LONG_MAX)
      packet_left_button_click_space_count[plyr_idx]++;
  }
  player->field_35 = 0;
  x = ((unsigned short)pckt->field_A);
  y = ((unsigned short)pckt->field_C);
  stl_x = x/(map_subtiles_x+1);
  stl_y = y/(map_subtiles_y+1);
  v172 = false;
  switch (player->field_453)
  {
  case 1:
//........
      break;
  case 2:
//........
      break;
  case 3:
      if (((pckt->field_E & 0x1000) == 0) || ((pckt->field_E & 0x8000) == 0))
        break;
      pos.x.val = x;
      pos.y.val = y;
      pos.z.val = 0;
      thing = create_creature(&pos, 8, plyr_idx);
      if (thing_is_invalid(thing))
      {
        pckt->field_E &= 0xEFFFu;
        break;
      }
      pos.z.val = get_thing_height_at(thing, &pos);
      if (thing_in_wall_at(thing, &pos))
      {
        delete_thing_structure(thing, 0);
        pckt->field_E &= 0xEFFFu;
      } else
      {
        thing->mappos.x.val = pos.x.val;
        thing->mappos.y.val = pos.y.val;
        thing->mappos.z.val = pos.z.val;
        remove_first_creature(thing);
        set_first_creature(thing);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 4:
      if (((pckt->field_E & 0x1000) == 0) || ((pckt->field_E & 0x8000) == 0))
        break;
      create_random_hero_creature(x, y, game.field_14E496, CREATURE_MAX_LEVEL);
      pckt->field_E &= 0xEFFFu;
      break;
  case 6:
      if (((pckt->field_E & 0x1000) != 0) && ((pckt->field_E & 0x8000) != 0))
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_call_to_arms(plyr_idx, stl_x, stl_y, i, 0);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 7:
      if (((pckt->field_E & 0x1000) != 0) && ((pckt->field_E & 0x8000) != 0))
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_cave_in(plyr_idx, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 8:
      if (((pckt->field_E & 0x1000) != 0) && ((pckt->field_E & 0x8000) != 0))
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_sight(plyr_idx, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 9:
      v172 = 1;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
      if (thing_is_invalid(thing))
        player->field_35 = 0;
      else
        player->field_35 = thing->field_1B;
      if (((pckt->field_E & 0x1000) != 0) && ((pckt->field_E & 0x8000) != 0))
      {
        magic_use_power_slap(plyr_idx, stl_x, stl_y);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 10:
      v172 = 1;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
      if (thing_is_invalid(thing))
        player->field_35 = 0;
      else
        player->field_35 = thing->field_1B;

      if ((pckt->field_E & 0x1000) != 0)
      {
        if (player->field_35 > 0)
        {
          player->field_43E = player->field_35;
          set_player_instance(player, 6, 0);
          pckt->field_E &= 0xEFFFu;
        }
      }
      if ((pckt->field_E & 0x2000) != 0)
      {
        if (player->instance_num != 6)
        {
          set_player_state(player, player->field_456, 0);
          pckt->field_E &= 0xDFFFu;
        }
      }
      break;
  case 11:
      v172 = 1;
      thing = get_creature_near_for_controlling(plyr_idx, x, y);
      if (thing_is_invalid(thing))
        player->field_35 = 0;
      else
        player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        if (player->field_35 > 0)
        {
          player->field_43E = player->field_35;
          set_player_instance(player, 5, 0);
          pckt->field_E &= 0xEFFFu;
        }
      }
      if ((pckt->field_E & 0x2000) != 0)
      {
        if (player->instance_num != 5)
        {
          set_player_state(player, player->field_456, 0);
          pckt->field_E &= 0xDFFFu;
        }
      }
      break;
  case 12:
  case 15:
      v172 = 1;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
      if (thing_is_invalid(thing))
        player->field_35 = 0;
      else
        player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        if (player->field_35 > 0)
        {
          if (player->field_2F != player->field_35)
          {
            if (is_my_player(player))
            {
              turn_off_all_panel_menus();
              initialise_tab_tags_and_menu(31);
              turn_on_menu(31);
            }
            player->field_43E = player->field_35;
            set_player_instance(player, 9, 0);
          }
          pckt->field_E &= 0xEFFFu;
        }
      }
      if (player->field_453 == 15)
      {
        thing = game.things_lookup[player->field_2F%THINGS_COUNT];
        if ((pckt->field_E & 0x2000) != 0)
        {
          if (is_my_player(player))
          {
            turn_off_query_menus();
            turn_on_main_panel_menu();
          }
          set_player_instance(player, 10, 0);
          pckt->field_E &= 0xDFFFu;
        } else
        if (thing->health < 0)
        {
          set_player_instance(player, 10, 0);
          if (is_my_player(player))
          {
            turn_off_query_menus();
            turn_on_main_panel_menu();
          }
        }
      }
      break;
  case 13:
      v172 = 1;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
      if (thing_is_invalid(thing))
        player->field_35 = 0;
      else
        player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        if ((player->field_2F > 0) && (player->field_2F < THINGS_COUNT))
        {
          if ((pckt->field_E & 0x8000) != 0)
          {
            thing = game.things_lookup[player->field_2F];
            setup_person_move_to_position(thing, stl_x, stl_y, 0);
            thing->field_8 = 122;
          }
        } else
        {
          thing = get_creature_near(x, y);
          if (!thing_is_invalid(thing))
          {
            player->field_2F = thing->field_1B;
            initialise_thing_state(thing, 122);
            cctrl = game.persons.cctrl_lookup[thing->field_64%CREATURES_COUNT];
            i = cctrl->field_7A;// & 0xFFF;
            if (i > 0)
              make_group_member_leader(thing);
          }
        }
        pckt->field_E &= 0xEFFFu;
      }
      if ((pckt->field_E & 0x2000) != 0)
      {
        if ((player->field_2F > 0) && (player->field_2F < THINGS_COUNT))
        {
          thing = game.things_lookup[player->field_2F];
          set_start_state(thing);
          player->field_2F = 0;
        }
        pckt->field_E &= 0xDFFFu;
      }
      break;
  case 14:
      if (((pckt->field_E & 0x1000) == 0) || ((pckt->field_E & 0x8000) == 0))
        break;
      create_random_evil_creature(x, y, plyr_idx, CREATURE_MAX_LEVEL);
      pckt->field_E &= 0xEFFFu;
      break;
  case 16:
//........
      break;
  case 17:
      player->field_35 = 0;
      if (((pckt->field_E & 0x1000) != 0) && ((pckt->field_E & 0x8000) != 0))
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_lightning(plyr_idx, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 18:
      if ((pckt->field_E & 0x8000) != 0)
      {
        player->field_4A4 = 1;
        if ((pckt->field_E & 0x0100) != 0)
        {
          i = tag_cursor_blocks_place_door(player->field_2B, stl_x, stl_y);
          k = map_tiles_x * map_to_slab[stl_y] + map_to_slab[stl_x];
          delete_room_slabbed_objects(k);
          packet_place_door(stl_x, stl_y, player->field_2B, player->field_4A6, i);
        }
        pckt->field_E &= 0xFEFFu;
      }
      if ((pckt->field_E & 0x1000) != 0)
      {
        if ( player->field_4AF )
        {
          player->field_4AF = 0;
          pckt->field_E &= 0xEFFFu;
        }
      }
      break;
  case 19:
      v172 = true;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
      if (thing_is_invalid(thing))
      {
        player->field_35 = 0;
        break;
      }
      player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_speed(plyr_idx, thing, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 20:
      v172 = true;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
      if (thing_is_invalid(thing))
      {
        player->field_35 = 0;
        break;
      }
      player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_armour(plyr_idx, thing, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 21:
      v172 = true;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
      if (thing_is_invalid(thing))
      {
        player->field_35 = 0;
        break;
      }
      player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_conceal(plyr_idx, thing, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 22:
      v172 = true;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_owned_by, plyr_idx);
      if (thing_is_invalid(thing))
      {
        player->field_35 = 0;
        break;
      }
      player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_heal(plyr_idx, thing, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 23:
//........
      break;
  case 24:
      if (((pckt->field_E & 0x1000) != 0) && ((pckt->field_E & 0x8000) != 0))
      {
        magic_use_power_imp(plyr_idx, stl_x, stl_y);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 25:
      if (((pckt->field_E & 0x1000) != 0) && ((pckt->field_E & 0x8000) != 0))
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_destroy_walls(plyr_idx, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 26:
      thing = get_creature_near_with_filter(x, y, creature_near_filter_is_enemy_of_and_not_imp, plyr_idx);
      if (thing_is_invalid(thing))
      {
        player->field_35 = 0;
        break;
      }
      player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_disease(plyr_idx, thing, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  case 27:
      v172 = true;
      thing = get_creature_near_with_filter(x, y, creature_near_filter_not_imp, plyr_idx);
      if (thing_is_invalid(thing))
      {
        player->field_35 = 0;
        break;
      }
      player->field_35 = thing->field_1B;
      if ((pckt->field_E & 0x1000) != 0)
      {
        i = get_spell_overcharge_level(player);
        magic_use_power_chicken(plyr_idx, thing, stl_x, stl_y, i);
        pckt->field_E &= 0xEFFFu;
      }
      break;
  default:
      break;
  }
  // resetting position variables - they may have been changed
  x = ((unsigned short)pckt->field_A);
  y = ((unsigned short)pckt->field_C);
  stl_x = x/(map_subtiles_x+1);
  stl_y = y/(map_subtiles_y+1);
  if (!player->field_35)
  {
    if ((x != 0) && (y == 0))  // what this condition is supposed to mean!?
    {
      thing = get_queryable_object_near(x, y, plyr_idx);
      if (!thing_is_invalid(thing))
        player->field_35 = thing->field_1B;
    }
  }
  i = (unsigned short)pckt->field_E;
  if (((i >> 8) & 0x0C) && (v172))
  {
    if ((player->field_455 == 0) || (player->field_455 == 3))
      stop_creatures_around_hand(plyr_idx, stl_x, stl_y);
  }
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
  game.packet_save_head.level_num = get_loaded_level_number();
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
          if ((pckt->action != 0) || (pckt->field_E != 0))
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
  cam = player->acamera;
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
  if (pcktd->field_6 > 0)
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

void process_quit_packet(struct PlayerInfo *player, short complete_quit)
{
  //_DK_process_quit_packet(player,a2);
  struct PlayerInfo *swplyr;
  struct PlayerInfo *myplyr;
  short winning_quit;
  long plyr_count;
  int i;

  plyr_count = 0;
  myplyr = &game.players[my_player_number%PLAYERS_COUNT];
  if ((game.numfield_A & 0x01) != 0)
  {
    winning_quit = winning_player_quitting(player, &plyr_count);
    if (winning_quit)
    {
      // Set other players as losers
      for (i=0; i < PLAYERS_COUNT; i++)
      {
        swplyr = &(game.players[i]);
        if ((swplyr->field_0 & 0x01) != 0)
        {
          if (swplyr->field_2C == 1)
            if (swplyr->victory_state == VicS_Undecided)
              swplyr->victory_state = VicS_WonLevel;
        }
      }
    }

    if ((player == myplyr) || (frontend_should_all_players_quit()))
    {
      if ((!winning_quit) || (plyr_count <= 1))
        LbNetwork_Stop();
      else
        myplyr->field_3 |= 0x10u;
    } else
    {
      if (!winning_quit)
      {
        if (player->victory_state != VicS_Undecided)
        {
          player->field_0 &= 0xFE;
        } else
        {
          player->field_0 |= 0x40;
          toggle_computer_player(player->field_2B);
        }
        if (player == myplyr)
        {
          quit_game = 1;
          if (complete_quit)
            exit_keeper = 1;
        }
        return;
      } else
      if (plyr_count <= 1)
        LbNetwork_Stop();
      else
        myplyr->field_3 |= 0x10u;
    }
    quit_game = 1;
    if (complete_quit)
      exit_keeper = 1;
    if (frontend_should_all_players_quit())
    {
      for (i=0; i < PLAYERS_COUNT; i++)
      {
        swplyr = &(game.players[i]);
        if ((swplyr->field_0 & 0x01) != 0)
        {
          swplyr->field_0 &= 0xFEu;
          swplyr->field_6 |= 0x02u;
        }
      }
    } else
    {
      player->field_0 &= 0xFEu;
    }
    return;
  }
  player->field_0 &= 0xFEu;
  if (player == myplyr)
  {
    quit_game = 1;
    if (complete_quit)
      exit_keeper = 1;
  }
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
  switch (pckt->action)
  {
    case 1:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (my_player_number == plyridx)
      {
        turn_off_all_menus();
        frontend_save_continue_game(true);
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
        frontend_save_continue_game(true);
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
      switch (player->victory_state)
      {
      case VicS_WonLevel:
          complete_level(player);
          break;
      case VicS_LostLevel:
          lose_level(player);
          break;
      default:
          resign_level(player);
          break;
      }
      player->field_0 &= 0xFEu;
      if (my_player_number == plyridx)
      {
        frontend_save_continue_game(false);
      }
      return 0;
    case PckA_PlyrMsgBegin:
      player->field_0 |= 0x04;
      return 0;
    case PckA_PlyrMsgEnd:
      player->field_0 &= 0xFBu;
      if (player->strfield_463[0] != '\0')
        message_add(player->field_2B);
      memset(player->strfield_463, 0, 64);
      return 0;
    case PckA_ToggleLights:
      if (my_player_number == plyridx)
        light_set_lights_on(game.field_4614D == 0);
      return 1;
    case PckA_SwitchScrnRes:
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
      set_player_cameras_position(player, pckt->field_6 << 8, pckt->field_8 << 8);
      return 0;
    case PckA_SetGammaLevel:
      if (myplyr->field_2B == player->field_2B)
      {
        set_gamma(pckt->field_6, 1);
        save_settings();
      }
      return 0;
    case PckA_SetMinimapConf:
      player->minimap_zoom = pckt->field_6;
      return 0;
    case 29:
      player->cameras[2].orient_a = pckt->field_6;
      player->cameras[3].orient_a = pckt->field_6;
      player->cameras[0].orient_a = pckt->field_6;
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
      set_player_cameras_position(player, pckt->field_6 << 8, pckt->field_8 << 8);
      player->cameras[2].orient_a = 0;
      player->cameras[3].orient_a = 0;
      player->cameras[0].orient_a = 0;
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
      if (game.event[pckt->field_6].kind == 3)
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
    case PckA_PlyrFastMsg:
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
    case PckA_SpellSOEDis:
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
    case PckA_PlyrToggleAlly:
      toggle_ally_with_player(plyridx, pckt->field_6);
      return 0;
    case 119:
      if (player->acamera != NULL)
        player->field_4B5 = player->acamera->field_6;
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
  player->cameras[2].mappos.x.val = (x << 8) + 1920;
  player->cameras[2].mappos.y.val = (y << 8) + 1920;
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
    LbSyncLog("%s: Processing player %d packet of type %d.\n",func_name,idx,(int)pckt->action);
#endif
  player->field_4 = (pckt->field_10 & 0x20) >> 5;
  player->field_5 = (pckt->field_10 & 0x40) >> 6;
  if ( (player->field_0 & 0x04) && (pckt->action == PckA_PlyrMsgChar))
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
  if (pckt->action == 32)
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
  switch (pckt->action)
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
  struct CreatureControl *cctrl;
  struct PlayerInfo *player;
  struct Thing *thing;
  struct Packet *pckt;
  long i,k;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_process_players_creature_control_packet_action(idx); return;
  player = &(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
  switch (pckt->action)
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
      if (thing_is_invalid(thing))
        break;
      cctrl = game.persons.cctrl_lookup[thing->field_64%CREATURES_COUNT];
      if ((cctrl == NULL) || (cctrl == game.persons.cctrl_lookup[0]))
        break;
      i = pckt->field_6;
      if (!instance_info[i].field_0)
      {
        cctrl->field_1E8 = i;
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
