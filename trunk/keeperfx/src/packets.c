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
 * @date     30 Jan 2009 - 11 Oct 2012
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
#include "bflib_math.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_keybrd.h"
#include "bflib_vidraw.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_network.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "bflib_planar.h"

#include "kjm_input.h"
#include "front_simple.h"
#include "front_landview.h"
#include "front_network.h"
#include "frontmenu_net.h"
#include "frontend.h"
#include "vidmode.h"
#include "config.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_terrain.h"
#include "config_settings.h"
#include "player_instances.h"
#include "player_data.h"
#include "player_states.h"
#include "player_utils.h"
#include "thing_physics.h"
#include "thing_doors.h"
#include "thing_effects.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_creature.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_groups.h"
#include "dungeon_data.h"
#include "tasks_list.h"
#include "power_specials.h"
#include "power_hand.h"
#include "room_util.h"
#include "room_workshop.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "magic.h"
#include "map_blocks.h"
#include "light_data.h"
#include "gui_draw.h"
#include "gui_topmsg.h"
#include "gui_frontmenu.h"
#include "gui_soundmsgs.h"
#include "gui_parchment.h"
#include "net_game.h"
#include "net_sync.h"
#include "game_legacy.h"
#include "engine_redraw.h"
#include "frontmenu_ingame_tabs.h"
#include "vidfade.h"

#include "keeperfx.hpp"

#include "music_player.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PACKET_TURN_SIZE (NET_PLAYERS_COUNT*sizeof(struct Packet) + sizeof(TbBigChecksum))
struct Packet bad_packet;
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
#pragma region PlayerActionPacketHelpers
// Fill packet with player action date, used in single player part of game.
void set_packet_action(struct Packet *pckt, unsigned char pcktype, unsigned short par1, unsigned short par2)
{
    pckt->actn_par1 = par1;
    pckt->actn_par2 = par2;
    pckt->action = pcktype;
}

// Fill packet with player action date, can be used in multiple player game.
void set_players_packet_action(struct PlayerInfo *player, unsigned char pcktype, unsigned short par1, unsigned short par2)
{
    struct Packet *pckt;
    pckt = get_packet_direct(player->packet_num);
    pckt->actn_par1 = par1;
    pckt->actn_par2 = par2;
    pckt->action = pcktype;
}

unsigned char get_players_packet_action(struct PlayerInfo *player)
{
    struct Packet *pckt;
    pckt = get_packet_direct(player->packet_num);
    return pckt->action;
}
#pragma endregion

#pragma region PlayerControlPacketHelpers
void set_packet_control(struct Packet *pckt, unsigned long flag)
{
  pckt->control_flags |= flag;
}

void set_players_packet_control(struct PlayerInfo *player, unsigned long flag)
{
  struct Packet *pckt;
  pckt = get_packet_direct(player->packet_num);
  pckt->control_flags |= flag;
}

void unset_packet_control(struct Packet *pckt, unsigned long flag)
{
  pckt->control_flags &= ~flag;
}

void unset_players_packet_control(struct PlayerInfo *player, unsigned long flag)
{
  struct Packet *pckt;
  pckt = get_packet_direct(player->packet_num);
  pckt->control_flags &= ~flag;
}
#pragma endregion

void set_players_packet_position(struct PlayerInfo *player, long x, long y)
{
    struct Packet *pckt;
    pckt = get_packet_direct(player->packet_num);
    pckt->pos_x = x;
    pckt->pos_y = y;
}

/**
 * Gives a pointer for the player's packet.
 * @param plyr_idx The player index for which we want the packet.
 * @return Returns Packet pointer. On error, returns a dummy structure.
 */
struct Packet *get_packet(long plyr_idx)
{
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    if (player_invalid(player))
        return INVALID_PACKET;
    if (player->packet_num >= PACKETS_COUNT)
        return INVALID_PACKET;
    return &game.packets[player->packet_num];
}

/**
 * Gives a pointer to packet of given index.
 * @param pckt_idx Packet index in the array. Note that it may differ from player index.
 * @return Returns Packet pointer. On error, returns a dummy structure.
 */
struct Packet *get_packet_direct(long pckt_idx)
{
    if ((pckt_idx < 0) || (pckt_idx >= PACKETS_COUNT))
        return INVALID_PACKET;
    return &game.packets[pckt_idx];
}

void clear_packets(void)
{
    int i;
    for (i=0; i < PACKETS_COUNT; i++) {
        LbMemorySet(&game.packets[i], 0, sizeof(struct Packet));
    }
}

short set_packet_pause_toggle(void)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    player = get_my_player();
    if (player_invalid(player))
        return false;
    if (player->packet_num >= PACKETS_COUNT)
        return false;
    pckt = get_packet_direct(player->packet_num);
    set_packet_action(pckt, PckA_TogglePause, 0, 0);
    return true;
}

TbBigChecksum compute_player_checksum(struct PlayerInfo *player)
{
    struct Coord3d *mappos;
    TbBigChecksum sum;
    sum = 0;
    if (((player->allocflags & PlaF_CompCtrl) == 0) && (player->acamera != NULL))
    {
        mappos = &(player->acamera->mappos);
        sum += (TbBigChecksum)player->instance_remain_rurns + (TbBigChecksum)player->instance_num;
        sum += (TbBigChecksum)mappos->x.val + (TbBigChecksum)mappos->z.val + (TbBigChecksum)mappos->y.val;
    }
    return sum;
}

TbBigChecksum compute_players_checksum(void)
{
    struct PlayerInfo *player;
    int i;
    TbBigChecksum sum;
    sum = 0;
    for (i=0; i<PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if (player_exists(player))
      {
          sum += compute_player_checksum(player);
      }
    }
    return sum;
}

void player_packet_checksum_add(PlayerNumber plyr_idx, TbBigChecksum sum)
{
    struct Packet *pckt;
    pckt = get_packet(plyr_idx);
    pckt->chksum += sum;
    SYNCDBG(9,"Checksum increase %06lX",(unsigned long)sum);
}

/**
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
  checksum = 0;
  is_set = false;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
    {
        pckt = get_packet_direct(player->packet_num);
        if (!is_set)
        {
          checksum = pckt->chksum;
          is_set = true;
        } 
        else if (checksum != pckt->chksum)
        {
          return true;
        }
    }
  }
  return false;
}

void update_double_click_detection(long plyr_idx)
{
  struct Packet *pckt;
  pckt = get_packet(plyr_idx);
  if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
  {
    if (packet_left_button_click_space_count[plyr_idx] < 5)
      packet_left_button_double_clicked[plyr_idx] = 1;
    packet_left_button_click_space_count[plyr_idx] = 0;
  }
  if ((pckt->control_flags & (PCtr_LBtnClick|PCtr_LBtnHeld)) == 0)
  {
    if (packet_left_button_click_space_count[plyr_idx] < LONG_MAX)
      packet_left_button_click_space_count[plyr_idx]++;
  }
}

struct Room *keeper_build_room(long stl_x,long stl_y,long plyr_idx,long rkind)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Room *room;
    struct RoomStats *rstat;
    struct Coord3d pos;
    MapCoord x,y;
    player = get_player(plyr_idx);
    dungeon = get_players_dungeon(player);
    rstat = room_stats_get_for_kind(rkind);
    x = ((player->field_4A4+1) / 2) + slab_subtile(subtile_slab_fast(stl_x),0);
    y = ((player->field_4A4+1) / 2) + slab_subtile(subtile_slab_fast(stl_y),0);
    room = player_build_room_at(x, y, plyr_idx, rkind);
    if (!room_is_invalid(room))
    {
        dungeon->camera_deviate_jump = 192;
        set_coords_to_slab_center(&pos,subtile_slab_fast(stl_x),subtile_slab_fast(stl_y));
        create_price_effect(&pos, plyr_idx, rstat->cost);
    }
    return room;
}

TbBool process_dungeon_control_packet_spell_overcharge(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Packet *pckt;
    player = get_player(plyr_idx);
    dungeon = get_players_dungeon(player);
    pckt = get_packet_direct(player->packet_num);
    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
      switch (player->work_state)
      {
      case PSt_CallToArms:
          if (player_uses_power_call_to_arms(plyr_idx))
            player->field_4D2 = (dungeon->cta_splevel << 2);
          else
            update_power_overcharge(player, PwrK_CALL2ARMS);
          break;
      case PSt_CaveIn:
          update_power_overcharge(player, PwrK_CAVEIN);
          break;
      case PSt_SightOfEvil:
          update_power_overcharge(player, PwrK_SIGHT);
          break;
      case PSt_Lightning:
          update_power_overcharge(player, PwrK_LIGHTNING);
          break;
      case PSt_SpeedUp:
          update_power_overcharge(player, PwrK_SPEEDCRTR);
          break;
      case PSt_Armour:
          update_power_overcharge(player, PwrK_PROTECT);
          break;
      case PSt_Conceal:
          update_power_overcharge(player, PwrK_CONCEAL);
          break;
      case PSt_Heal:
          update_power_overcharge(player, PwrK_HEALCRTR);
          break;
      default:
          player->field_4D2++;
          break;
      }
      return true;
    }
    if ((pckt->control_flags & PCtr_LBtnRelease) == 0)
    {
        player->field_4D2 = 0;
        return false;
    }
    return false;
}

TbBool player_sell_room_at_subtile(long plyr_idx, long stl_x, long stl_y)
{
    struct Room *room;
    struct RoomStats *rstat;
    struct Coord3d pos;
    long revenue;
    room = subtile_room_get(stl_x, stl_y);
    if (room_is_invalid(room))
    {
        ERRORLOG("No room to delete at subtile (%d,%d)",(int)stl_x,(int)stl_y);
        return false;
    }
    //TODO CONFIG sell revenue percentage should be inside config files
    rstat = room_stats_get_for_room(room);
    revenue = compute_value_percentage(rstat->cost, ROOM_SELL_REVENUE_PERCENT);
    if (room->owner != game.neutral_player_num)
    {
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(room->owner);
        dungeon->rooms_destroyed++;
        dungeon->camera_deviate_jump = 192;
    }
    delete_room_slab(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y), 0);
    if (is_my_player_number(plyr_idx))
        play_non_3d_sample(115);
    if (revenue != 0)
    {
        set_coords_to_slab_center(&pos,subtile_slab_fast(stl_x),subtile_slab_fast(stl_y));
        create_price_effect(&pos, plyr_idx, revenue);
        player_add_offmap_gold(plyr_idx, revenue);
    }
    return true;
}

TbBool process_dungeon_control_packet_sell_operation(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    struct SlabMap *slb;
    MapSubtlCoord stl_x,stl_y;
    MapCoord x,y;
    player = get_player(plyr_idx);
    pckt = get_packet_direct(player->packet_num);
    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
        {
            player->field_4AF = 0;
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    x = ((unsigned short)pckt->pos_x);
    y = ((unsigned short)pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);
    player->field_4A4 = 1;
    if (is_my_player(player))
    {
      if (!game_is_busy_doing_gui())
        tag_cursor_blocks_sell_area(player->id_number, stl_x, stl_y, player->field_4A4);
    }
    if ((pckt->control_flags & PCtr_LBtnClick) == 0)
    {
      if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
      {
          player->field_4AF = 0;
          unset_packet_control(pckt, PCtr_LBtnRelease);
      }
      return false;
    }
    slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_owner(slb) != plyr_idx)
    {
        WARNLOG("Player %d can't sell item on %s owned by player %d at subtile (%d,%d).",(int)plyr_idx,slab_code_name(slb->kind),(int)slabmap_owner(slb),(int)stl_x,(int)stl_y);
        unset_packet_control(pckt, PCtr_LBtnClick);
        return false;
    }
    // Trying to sell room
    if (subtile_is_sellable_room(plyr_idx, stl_x, stl_y))
    {
        player_sell_room_at_subtile(plyr_idx, stl_x, stl_y);
    } else
    // Trying to sell door
    if (player_sell_door_at_subtile(plyr_idx, stl_x, stl_y))
    {
        // Nothing to do here - door already sold
    } else
    // Trying to sell trap
    if (player_sell_trap_at_subtile(plyr_idx, stl_x, stl_y))
    {
        // Nothing to do here - trap already sold
    } else
    {
        WARNLOG("Nothing to do for player %d request",(int)plyr_idx);
    }
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

TbBool process_dungeon_power_hand_state(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    struct Packet *pckt;
    MapSubtlCoord stl_x,stl_y;
    MapCoord x,y;
    long i;
    player = get_player(plyr_idx);
    pckt = get_packet_direct(player->packet_num);
    x = ((unsigned short)pckt->pos_x);
    y = ((unsigned short)pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);

    player->field_3 &= ~0x02;
    if ((player->field_455 != P454_Unkn0) && (player->field_455 != P454_Unkn3))
    {
      if (player->instance_num != PI_Grab) {
          delete_power_hand(player->id_number);
      }
      return false;
    }
    thing = get_nearest_thing_for_hand_or_slap(plyr_idx, x, y);
    if (!thing_is_invalid(thing))
    {
      SYNCDBG(19,"Thing %d under hand at (%d,%d)",(int)thing->index,(int)thing->mappos.x.stl.num,(int)thing->mappos.y.stl.num);
      if (player->hand_thing_idx == 0)
        create_power_hand(player->id_number);
      player->thing_under_hand = thing->index;
    }
    thing = get_first_thing_in_power_hand(player);
    if (!thing_is_invalid(thing))
    {
      if (player->hand_thing_idx == 0) {
        create_power_hand(player->id_number);
      }
      i = thing_is_creature_special_digger(thing);
      if (can_drop_thing_here(stl_x, stl_y, player->id_number, i)
        || !can_dig_here(stl_x, stl_y, player->id_number))
      {
        tag_cursor_blocks_thing_in_hand(player->id_number, stl_x, stl_y, i, player->field_4A4);
      } else
      {
        player->field_3 |= 0x02;
        tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->field_4A4);
      }
    }
    if (player->hand_thing_idx != 0)
    {
      if ((player->instance_num != PI_Grab) && (player->instance_num != PI_Drop) &&
          (player->instance_num != PI_Whip) && (player->instance_num != PI_WhipEnd))
      {
        thing = get_first_thing_in_power_hand(player);
        if ((player->thing_under_hand != 0) || thing_is_invalid(thing))
        {
          set_power_hand_graphic(plyr_idx, 782, 256);
          if (!thing_is_invalid(thing))
            thing->field_4F |= TF4F_Unknown01;
        } else
        if ((thing->class_id == TCls_Object) && object_is_gold_pile(thing))
        {
          set_power_hand_graphic(plyr_idx, 781, 256);
          thing->field_4F &= ~TF4F_Unknown01;
        } else
        {
          set_power_hand_graphic(plyr_idx, 784, 256);
          thing->field_4F &= ~TF4F_Unknown01;
        }
      }
    }
    return true;
}

TbBool process_dungeon_control_packet_dungeon_build_room(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    MapSubtlCoord stl_x,stl_y;
    MapCoord x,y;
    long i;
    player = get_player(plyr_idx);
    pckt = get_packet_direct(player->packet_num);
    x = ((unsigned short)pckt->pos_x);
    y = ((unsigned short)pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);

    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
      if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
      {
        player->field_4AF = 0;
        unset_packet_control(pckt, PCtr_LBtnRelease);
      }
      return false;
    }
    player->field_4A4 = 1;
    if (is_my_player(player))
      gui_room_type_highlighted = player->chosen_room_kind;
    i = tag_cursor_blocks_place_room(player->id_number, stl_x, stl_y, player->field_4A4);
    if ((pckt->control_flags & PCtr_LBtnClick) == 0)
    {
      if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
      {
        player->field_4AF = 0;
        unset_packet_control(pckt, PCtr_LBtnRelease);
      }
      return false;
    }
    if (i == 0)
    {
      if (is_my_player(player))
      {
        play_non_3d_sample(119);
        unset_packet_control(pckt, PCtr_LBtnClick);
      }
      return false;
    }
    keeper_build_room(stl_x,stl_y,plyr_idx,player->chosen_room_kind);
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

TbBool process_dungeon_control_packet_dungeon_place_trap(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    MapSubtlCoord stl_x,stl_y;
    MapCoord x,y;
    long i;
    player = get_player(plyr_idx);
    pckt = get_packet_direct(player->packet_num);
    x = ((unsigned short)pckt->pos_x);
    y = ((unsigned short)pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);

    if ((pckt->control_flags & PCtr_MapCoordsValid) == 0)
    {
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
        {
          player->field_4AF = 0;
          unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        return false;
    }
    player->field_4A4 = 1;
    i = tag_cursor_blocks_place_trap(player->id_number, stl_x, stl_y);
    if ((pckt->control_flags & PCtr_LBtnClick) == 0)
    {
      if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && (player->field_4AF != 0))
      {
        player->field_4AF = 0;
        unset_packet_control(pckt, PCtr_LBtnRelease);
      }
      return false;
    }
    if (i == 0)
    {
        if (is_my_player(player))
            play_non_3d_sample(119);
        unset_packet_control(pckt, PCtr_LBtnClick);
        return false;
    }
    if (!player_place_trap_at(stl_x, stl_y, plyr_idx, player->chosen_trap_kind))
    {
        unset_packet_control(pckt, PCtr_LBtnClick);
        return false;
    }
    unset_packet_control(pckt, PCtr_LBtnClick);
    return true;
}

TbBool process_dungeon_control_packet_dungeon_control(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *thing;
    struct Packet *pckt;
    MapSubtlCoord stl_x,stl_y;
    MapSubtlCoord cx,cy;
    MapCoord x,y;
    long i;
    player = get_player(plyr_idx);
    dungeon = get_players_dungeon(player);
    pckt = get_packet_direct(player->packet_num);
    x = ((unsigned short)pckt->pos_x);
    y = ((unsigned short)pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);

    cx = stl_slab_starting_subtile(stl_x);
    cy = stl_slab_starting_subtile(stl_y);
    if ((pckt->control_flags & PCtr_LBtnAnyAction) == 0)
      player->field_455 = P454_Unkn0;
    player->field_454 = (unsigned short)(pckt->field_10 & 0x1E) >> 1;

    process_dungeon_power_hand_state(plyr_idx);

    if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
    {
      if (is_my_player(player) && !game_is_busy_doing_gui())
      {
        if (player->field_454 == P454_Unkn1)
          tag_cursor_blocks_dig(player->id_number, stl_x, stl_y, player->field_4A4);
      }
      if ((pckt->control_flags & PCtr_LBtnClick) != 0)
      {
        player->field_4AB = stl_x;
        player->field_4AD = stl_y;
        player->field_4AF = 1;
        player->field_455 = player->field_454;
        switch (player->field_454)
        {
        case P454_Unkn1:
          i = get_subtile_number(stl_slab_center_subtile(player->field_4AB),stl_slab_center_subtile(player->field_4AD));
          if (find_from_task_list(plyr_idx,i) != -1)
              player->allocflags |= PlaF_Unknown20;
          else
              player->allocflags &= ~PlaF_Unknown20;
          break;
        case P454_Unkn2:
          thing = get_door_for_position(player->field_4AB, player->field_4AD);
          if (thing_is_invalid(thing))
          {
            ERRORLOG("Door thing not found at map pos (%d,%d)",(int)player->field_4AB,(int)player->field_4AD);
            break;
          }
          if (thing->byte_18)
            unlock_door(thing);
          else
            lock_door(thing);
          break;
        case P454_Unkn3:
          if (player->thing_under_hand == 0)
          {
            i = get_subtile_number(stl_slab_center_subtile(player->field_4AB),stl_slab_center_subtile(player->field_4AD));
            if (find_from_task_list(plyr_idx,i) != -1)
                player->allocflags |= PlaF_Unknown20;
            else
                player->allocflags &= ~PlaF_Unknown20;
            player->field_3 |= 0x01;
          }
          break;
        }
        unset_packet_control(pckt, PCtr_LBtnClick);
      }
      if ((pckt->control_flags & PCtr_RBtnClick) != 0)
      {
        player->field_4AB = stl_x;
        player->field_4AD = stl_y;
        player->field_4AF = 1;
        unset_packet_control(pckt, PCtr_RBtnClick);
      }
    }

    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
        if (player->field_455 == P454_Unkn0)
        {
            player->field_455 = player->field_454;
            if (player->field_454 == P454_Unkn1)
            {
                i = get_subtile_number(stl_slab_center_subtile(stl_x), stl_slab_center_subtile(stl_y));
                if (find_from_task_list(plyr_idx, i) != -1)
                    player->allocflags |= PlaF_Unknown20;
                else
                    player->allocflags &= ~PlaF_Unknown20;
            }
        }
        if (player->field_4AF != 0)
        {
            if ((player->field_454 == player->field_455) && !lbDisplayEx.isDragMovingCamera)
            {
                if (player->field_455 == P454_Unkn1)
                {
                    if ((player->allocflags & PlaF_Unknown20) != 0)
                    {
                        untag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
                    }
                    else if (dungeon->field_E8F < 300)
                    {
                        tag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
                    }
                    else if (is_my_player(player))
                    {
                        output_message(SMsg_WorkerJobsLimit, 500, true);
                    }
                }
                else if ((player->field_455 == P454_Unkn3) && ((player->field_3 & 0x01) != 0))
                {
                    if ((player->allocflags & PlaF_Unknown20) != 0)
                    {
                        untag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
                    }
                    else if (dungeon->field_E8F < 300)
                    {
                        if (can_dig_here(stl_x, stl_y, player->id_number))
                        {
                            tag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
                        }
                    }
                    else if (is_my_player(player))
                    {
                        output_message(SMsg_WorkerJobsLimit, 500, true);
                    }
                }
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
    }
    if ((pckt->control_flags & PCtr_RBtnHeld) != 0)
    {
        if (player->field_4AF != 0)
            unset_packet_control(pckt, PCtr_RBtnRelease);
    }
    if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
    {
        if (player->field_455 == P454_Unkn0)
        player->field_455 = player->field_454;
      if (player->field_4AF != 0)
      {
        thing = thing_get(player->thing_under_hand);
        if ((player->thing_under_hand != 0) && (player->boolfield_4 != 0)
          && (dungeon->things_in_hand[0] != player->thing_under_hand))
        {
            set_player_state(player, PSt_CtrlDirect, 0);
            if (magic_use_available_power_on_thing(plyr_idx, PwrK_POSSESS, 0, stl_x, stl_y, thing) == Lb_FAIL) {
                set_player_state(player, player->continue_work_state, 0);
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        } else
        if ((player->thing_under_hand != 0) && (player->boolfield_5 != 0)
          && (dungeon->things_in_hand[0] != player->thing_under_hand)
          && can_thing_be_queried(thing, plyr_idx) )
        {
          if (player->thing_under_hand != player->controlled_thing_idx)
          {
            if (is_my_player(player))
            {
              turn_off_all_panel_menus();
              turn_on_menu(GMnu_CREATURE_QUERY1);
            }
            player->influenced_thing_idx = player->thing_under_hand;
            set_player_state(player, PSt_CreatrQuery, 0);
            set_player_instance(player, PI_QueryCrtr, 0);
          }
          unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        else if ((player->field_454 == player->field_455) && !lbDisplayEx.isDragMovingCamera)
        {
          if (player->field_454 == P454_Unkn1)
          {
            if ((player->allocflags & PlaF_Unknown20) != 0)
            {
              untag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
            } else
            if (300-dungeon->field_E8F >= 9)
            {
              tag_blocks_for_digging_in_rectangle_around(cx, cy, plyr_idx);
            } else
            if (is_my_player(player))
            {
              output_message(SMsg_WorkerJobsLimit, 500, true);
            }
          } else
          if (player->field_454 == P454_Unkn3)
          {
            if (player->thing_under_hand != 0) {
                // TODO SPELLS it's not a good idea to use this directly; change to magic_use_available_power_on_*()
                magic_use_power_hand(plyr_idx, stl_x, stl_y, 0);
            }
          }
        }
        player->field_4AF = 0;
        unset_packet_control(pckt, PCtr_LBtnRelease);
        player->field_455 = P454_Unkn0;
        player->field_3 &= ~0x01;
      }
    }

    if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
    {
      if (player->field_4AF != 0)
      {
        if (!power_hand_is_empty(player))
        {
          if (dump_first_held_thing_on_map(player->id_number, stl_x, stl_y, 1)) {
              player->field_4AF = 0;
              unset_packet_control(pckt, PCtr_RBtnRelease);
          }
        } else
        {
          if (player->field_454 == P454_Unkn3) {
              thing = get_nearest_thing_for_slap(plyr_idx, subtile_coord_center(stl_x), subtile_coord_center(stl_y));
              magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, 0, stl_x, stl_y, thing);
          }
          player->field_4AF = 0;
          unset_packet_control(pckt, PCtr_RBtnRelease);
        }
      }
    }
    return true;
}

TbBool process_dungeon_control_packet_clicks(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    struct Thing *thing;
    MapSubtlCoord stl_x,stl_y;
    short influence_own_creatures;
    TbBool ret;
    MapCoord x,y;
    PowerKind pwkind;
    long i,k;

    player = get_player(plyr_idx);
    pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6,"Starting for state %d",(int)player->work_state);
    player->field_4A4 = 1;
    packet_left_button_double_clicked[plyr_idx] = 0;
    if ((pckt->control_flags & PCtr_Unknown4000) != 0)
      return false;
    ret = true;

    process_dungeon_control_packet_spell_overcharge(plyr_idx);
    if ((pckt->control_flags & PCtr_RBtnHeld) != 0)
    {
      player->field_4D6++;
    } else
    if ((pckt->control_flags & PCtr_RBtnRelease) == 0)
    {
      player->field_4D6 = 0;
    }
    update_double_click_detection(plyr_idx);
    player->thing_under_hand = 0;
    x = ((unsigned short)pckt->pos_x);
    y = ((unsigned short)pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);
    influence_own_creatures = false;

    switch (player->work_state)
    {
    case PSt_CtrlDungeon:
        influence_own_creatures = 1;
        process_dungeon_control_packet_dungeon_control(plyr_idx);
        break;
    case PSt_BuildRoom:
        process_dungeon_control_packet_dungeon_build_room(plyr_idx);
        break;
    case PSt_MkGoodDigger:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            create_owned_special_digger(x, y, game.hero_player_num);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_MkGoodCreatr:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            create_random_hero_creature(x, y, game.hero_player_num, CREATURE_MAX_LEVEL);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_MkGoldPot:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            create_gold_pot_at(x, y, player->id_number);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_CallToArms:
    case PSt_CaveIn:
    case PSt_SightOfEvil:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            pwkind = player_state_to_power_kind[player->work_state];
            i = get_power_overcharge_level(player);
            magic_use_available_power_on_subtile(plyr_idx, pwkind, i, stl_x, stl_y, PwCast_None);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_Slap:
        influence_own_creatures = 1;
        thing = get_creature_near_to_be_keeper_power_target(x, y, PwrK_SLAP, plyr_idx);
        if (thing_is_invalid(thing)) {
            player->thing_under_hand = 0;
        } else {
            player->thing_under_hand = thing->index;
        }
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            magic_use_available_power_on_thing(plyr_idx, PwrK_SLAP, 0, stl_x, stl_y, thing);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_CtrlPassngr:
    case PSt_FreeCtrlPassngr:
        influence_own_creatures = 1;
        if (player->work_state == PSt_CtrlPassngr)
            thing = get_creature_near_and_owned_by(x, y, plyr_idx);
        else
            thing = get_creature_near_and_owned_by(x, y, -1);
        if (thing_is_invalid(thing))
            player->thing_under_hand = 0;
        else
            player->thing_under_hand = thing->index;
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
          if (player->thing_under_hand > 0)
          {
            player->influenced_thing_idx = player->thing_under_hand;
            set_player_instance(player, PI_PsngrCtrl, 0);
            unset_packet_control(pckt, PCtr_LBtnRelease);
          }
        }
        if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
        {
          if (player->instance_num != PI_PsngrCtrl)
          {
            set_player_state(player, player->continue_work_state, 0);
            unset_packet_control(pckt, PCtr_RBtnRelease);
          }
        }
        break;
    case PSt_CtrlDirect:
    case PSt_FreeCtrlDirect:
        influence_own_creatures = 1;
        if (player->work_state == PSt_CtrlDirect)
            thing = get_creature_near_for_controlling(plyr_idx, x, y);
        else
            thing = get_creature_near_for_controlling(plyr_idx, x, y); // TODO Change to allow controlling any owner
        if (thing_is_invalid(thing))
          player->thing_under_hand = 0;
        else
          player->thing_under_hand = thing->index;
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
          if (player->thing_under_hand > 0)
          {
              magic_use_available_power_on_thing(plyr_idx, PwrK_POSSESS, 0, stl_x, stl_y, thing);
              unset_packet_control(pckt, PCtr_LBtnRelease);
          }
        }
        if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
        {
          if (player->instance_num != PI_DirctCtrl)
          {
            set_player_state(player, player->continue_work_state, 0);
            unset_packet_control(pckt, PCtr_RBtnRelease);
          }
        }
        break;
    case PSt_CreatrQuery:
    case PSt_CreatrInfo:
        influence_own_creatures = 1;
        thing = get_creature_near_and_owned_by(x, y, plyr_idx);
        if (thing_is_invalid(thing))
            player->thing_under_hand = 0;
        else
            player->thing_under_hand = thing->index;
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
          if (player->thing_under_hand > 0)
          {
            if (player->controlled_thing_idx != player->thing_under_hand)
            {
              if (is_my_player(player))
              {
                turn_off_all_panel_menus();
                initialise_tab_tags_and_menu(GMnu_CREATURE_QUERY1);
                turn_on_menu(GMnu_CREATURE_QUERY1);
              }
              player->influenced_thing_idx = player->thing_under_hand;
              set_player_instance(player, PI_QueryCrtr, 0);
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
          }
        }
        if (player->work_state == PSt_CreatrInfo)
        {
          thing = thing_get(player->controlled_thing_idx);
          if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
          {
            if (is_my_player(player))
            {
              turn_off_query_menus();
              turn_on_main_panel_menu();
            }
            set_player_instance(player, PI_UnqueryCrtr, 0);
            unset_packet_control(pckt, PCtr_RBtnRelease);
          } else
          if (creature_is_dying(thing))
          {
            set_player_instance(player, PI_UnqueryCrtr, 0);
            if (is_my_player(player))
            {
              turn_off_query_menus();
              turn_on_main_panel_menu();
            }
          }
        }
        break;
    case PSt_OrderCreatr:
        influence_own_creatures = 1;
        thing = get_creature_near_and_owned_by(x, y, plyr_idx);
        if (thing_is_invalid(thing))
            player->thing_under_hand = 0;
        else
            player->thing_under_hand = thing->index;
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
          if ((player->controlled_thing_idx > 0) && (player->controlled_thing_idx < THINGS_COUNT))
          {
            if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
            {
              thing = thing_get(player->controlled_thing_idx);
              if (!setup_person_move_to_position(thing, stl_x, stl_y, NavRtF_Default))
                  WARNLOG("Move %s order failed",thing_model_name(thing));
              thing->continue_state = CrSt_ManualControl;
            }
          } else
          {
            thing = get_creature_near(x, y);
            if (!thing_is_invalid(thing))
            {
                set_selected_creature(player, thing);
                initialise_thing_state(thing, CrSt_ManualControl);
                if (creature_is_group_member(thing)) {
                    make_group_member_leader(thing);
                }
            }
          }
          unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        if ((pckt->control_flags & PCtr_RBtnRelease) != 0)
        {
          if ((player->controlled_thing_idx > 0) && (player->controlled_thing_idx < THINGS_COUNT))
          {
            thing = thing_get(player->controlled_thing_idx);
            set_start_state(thing);
            clear_selected_thing(player);
          }
          unset_packet_control(pckt, PCtr_RBtnRelease);
        }
        break;
    case PSt_MkBadCreatr:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            create_random_evil_creature(x, y, plyr_idx, CREATURE_MAX_LEVEL);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_PlaceTrap:
        process_dungeon_control_packet_dungeon_place_trap(plyr_idx);
        break;
    case PSt_Lightning:
        player->thing_under_hand = 0;
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            i = get_power_overcharge_level(player);
            magic_use_available_power_on_subtile(plyr_idx, PwrK_LIGHTNING, i, stl_x, stl_y, PwCast_None);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_PlaceDoor:
        if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
        {
            player->field_4A4 = 1;
            // Make the frame around active slab
            i = tag_cursor_blocks_place_door(player->id_number, stl_x, stl_y);
            if ((pckt->control_flags & PCtr_LBtnClick) != 0)
            {
              k = get_slab_number(subtile_slab_fast(stl_x), subtile_slab_fast(stl_y));
              delete_room_slabbed_objects(k);
              packet_place_door(stl_x, stl_y, player->id_number, player->chosen_door_kind, i);
            }
            unset_packet_control(pckt, PCtr_LBtnClick);
        }
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
            if (player->field_4AF != 0)
            {
              player->field_4AF = 0;
              unset_packet_control(pckt, PCtr_LBtnRelease);
            }
        }
        break;
    case PSt_SpeedUp:
    case PSt_Armour:
    case PSt_Conceal:
    case PSt_Heal:
        influence_own_creatures = true;
        pwkind = player_state_to_power_kind[player->work_state];
        thing = get_creature_near_to_be_keeper_power_target(x, y, pwkind, plyr_idx);
        if (thing_is_invalid(thing))
        {
            player->thing_under_hand = 0;
            break;
        }
        player->thing_under_hand = thing->index;
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
            i = get_power_overcharge_level(player);
            magic_use_available_power_on_thing(plyr_idx, pwkind, i, stl_x, stl_y, thing);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_Sell:
        process_dungeon_control_packet_sell_operation(plyr_idx);
        break;
    case PSt_CreateDigger:
    case PSt_DestroyWalls:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            pwkind = player_state_to_power_kind[player->work_state];
            i = get_power_overcharge_level(player);
            magic_use_available_power_on_subtile(plyr_idx, pwkind, i, stl_x, stl_y, PwCast_None);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_CastDisease:
        pwkind = player_state_to_power_kind[player->work_state];
        thing = get_creature_near_to_be_keeper_power_target(x, y, pwkind, plyr_idx);
        if (thing_is_invalid(thing))
        {
            player->thing_under_hand = 0;
            break;
        }
        player->thing_under_hand = thing->index;
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
            i = get_power_overcharge_level(player);
            magic_use_available_power_on_thing(plyr_idx, pwkind, i, stl_x, stl_y, thing);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_TurnChicken:
        influence_own_creatures = true;
        pwkind = player_state_to_power_kind[player->work_state];
        thing = get_creature_near_to_be_keeper_power_target(x, y, pwkind, plyr_idx);
        if (thing_is_invalid(thing))
        {
            player->thing_under_hand = 0;
            break;
        }
        player->thing_under_hand = thing->index;
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
            i = get_power_overcharge_level(player);
            magic_use_available_power_on_thing(plyr_idx, pwkind, i, stl_x, stl_y, thing);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_FreeDestroyWalls:
        if (((pckt->control_flags & PCtr_LBtnRelease) != 0) && ((pckt->control_flags & PCtr_MapCoordsValid) != 0))
        {
            i = get_power_overcharge_level(player);
            magic_use_power_destroy_walls(plyr_idx, stl_x, stl_y, i, PwMod_CastForFree);
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    case PSt_FreeTurnChicken:
    case PSt_FreeCastDisease:
        pwkind = player_state_to_power_kind[player->work_state];
        thing = get_creature_near_to_be_keeper_power_target(x, y, pwkind, plyr_idx);
        if (thing_is_invalid(thing))
        {
            player->thing_under_hand = 0;
            break;
        }
        player->thing_under_hand = thing->index;
        if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
        {
            i = get_power_overcharge_level(player);
            switch (pwkind)
            {
            case PwrK_DISEASE:
                magic_use_power_disease(plyr_idx, thing, stl_x, stl_y, i, PwMod_CastForFree);
                break;
            case PwrK_CHICKEN:
                magic_use_power_chicken(plyr_idx, thing, stl_x, stl_y, i, PwMod_CastForFree);
                break;
            }
            unset_packet_control(pckt, PCtr_LBtnRelease);
        }
        break;
    default:
        ERRORLOG("Unrecognized player %d work state: %d", (int)plyr_idx, (int)player->work_state);
        ret = false;
        break;
    }
    // resetting position variables - they may have been changed
    x = ((unsigned short)pckt->pos_x);
    y = ((unsigned short)pckt->pos_y);
    stl_x = coord_subtile(x);
    stl_y = coord_subtile(y);
    if (player->thing_under_hand == 0)
    {
      if ((x != 0) && (y != 0))  //originally was (y == 0), but it was probably a mistake
      {
          thing = get_queryable_object_near(x, y, plyr_idx);
          if (!thing_is_invalid(thing)) {
              player->thing_under_hand = thing->index;
          }
      }
    }
    if (((pckt->control_flags & PCtr_HeldAnyButton) != 0) && (influence_own_creatures))
    {
      if ((player->field_455 == P454_Unkn0) || (player->field_455 == P454_Unkn3))
        stop_creatures_around_hand(plyr_idx, stl_x, stl_y);
    }
    return ret;
}

TbBigChecksum get_thing_simple_checksum(const struct Thing *tng)
{
    return (ulong)tng->mappos.x.val + (ulong)tng->mappos.y.val + (ulong)tng->mappos.z.val
         + (ulong)tng->move_angle_xy + (ulong)tng->owner;
}

  TbBigChecksum get_packet_save_checksum(void)
  {
      TbBigChecksum sum;
      struct Thing *tng;
      long tng_idx;
      sum = 0;
      for (tng_idx=0; tng_idx < THINGS_COUNT; tng_idx++)
      {
          tng = thing_get(tng_idx);
          if ((tng->alloc_flags & TAlF_Exists) != 0)
          {
              // It would be nice to completely ignore effects, but since
              // thing indices are used in packets, lack of effect may cause desync too.
              if ((tng->class_id != TCls_AmbientSnd) && (tng->class_id != TCls_EffectElem))
              {
                  sum += get_thing_simple_checksum(tng);
              }
          }
      }
      return sum;
}

TbBool reinit_packets_after_load(void)
{
    game.packet_save_enable = false;
    game.packet_load_enable = false;
    game.packet_save_fp = -1;
    game.packet_fopened = 0;
    return true;
}

TbBool open_new_packet_file_for_save(void)
{
    struct PlayerInfo *player;
    struct CatalogueEntry centry;
    int i;
    // Filling the header
    SYNCMSG("Starting packet saving, turn %lu",(unsigned long)game.play_gameturn);
    game.packet_save_head.field_0 = 0;
    game.packet_save_head.level_num = get_loaded_level_number();
    game.packet_save_head.field_8 = 0;
    game.packet_save_head.field_C = 0;
    game.packet_save_head.field_D = 0;
    game.packet_save_head.chksum_available = game.packet_checksum_verify;
    for (i=0; i<PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player))
        {
            game.packet_save_head.field_C |= (1 << i) & 0xff;
            if ((player->allocflags & PlaF_CompCtrl) != 0)
              game.packet_save_head.field_D |= (1 << i) & 0xff;
        }
    }
    LbFileDelete(game.packet_fname);
    game.packet_save_fp = LbFileOpen(game.packet_fname, Lb_FILE_MODE_NEW);
    if (game.packet_save_fp == -1)
    {
        ERRORLOG("Cannot open keeper packet file for save, \"%s\".",game.packet_fname);
        game.packet_fopened = 0;
        return false;
    }
    fill_game_catalogue_entry(&centry,"Packet file");
    if (!save_packet_chunks(game.packet_save_fp,&centry))
    {
        WARNMSG("Cannot write to packet file, \"%s\".",game.packet_fname);
        LbFileClose(game.packet_save_fp);
        game.packet_fopened = 0;
        game.packet_save_fp = -1;
        return false;
    }
    game.packet_fopened = 1;
    return true;
}

void load_packets_for_turn(GameTurn nturn)
{
    struct Packet *pckt;
    TbChecksum pckt_chksum;
    TbBigChecksum tot_chksum;
    short done;
    long i;
    SYNCDBG(19,"Starting");
    const int turn_data_size = PACKET_TURN_SIZE;
    unsigned char pckt_buf[PACKET_TURN_SIZE+4];
    pckt = get_packet(my_player_number);
    pckt_chksum = pckt->chksum;
    if (nturn >= game.turns_stored)
    {
        ERRORDBG(18,"Out of turns to load from Packet File");
        erstat_inc(ESE_CantReadPackets);
        return;
    }

    if (LbFileRead(game.packet_save_fp, &pckt_buf, turn_data_size) == -1)
    {
        ERRORDBG(18,"Cannot read turn data from Packet File");
        erstat_inc(ESE_CantReadPackets);
        return;
    }
    game.packet_file_pos += turn_data_size;
    for (i=0; i < NET_PLAYERS_COUNT; i++)
      LbMemoryCopy(&game.packets[i], &pckt_buf[i*sizeof(struct Packet)], sizeof(struct Packet));
    tot_chksum = llong(&pckt_buf[NET_PLAYERS_COUNT*sizeof(struct Packet)]);
    if ((game.numfield_C & 0x01) != 0)
    {
        done = false;
        while (!done)
        {
            for (i=0; i<NET_PLAYERS_COUNT; i++)
            {
              pckt = get_packet_direct(i);
              if ((pckt->action != 0) || (pckt->control_flags != 0))
              {
                done = true;
                break;
              }
            }
            game.pckt_gameturn++;
            if (LbFileRead(game.packet_save_fp, &pckt_buf, turn_data_size) == -1)
            {
              ERRORLOG("Cannot read turn data from Packet File");
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
    if (game.packet_checksum_verify)
    {
        pckt = get_packet(my_player_number);
        if (get_packet_save_checksum() != tot_chksum)
        {
            ERRORLOG("PacketSave checksum - Out of sync (GameTurn %d)", game.play_gameturn);
            if (!is_onscreen_msg_visible())
              show_onscreen_msg(game.num_fps, "Out of sync");
        } else
        if (pckt->chksum != pckt_chksum)
        {
            ERRORLOG("Opps we are really Out Of Sync (GameTurn %d)", game.play_gameturn);
            if (!is_onscreen_msg_visible())
              show_onscreen_msg(game.num_fps, "Out of sync");
        }
    }
}

void process_pause_packet(long curr_pause, long new_pause)
{
  struct PlayerInfo *player;
  TbBool can;
  long i;
  can = true;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player) && (player->field_2C == 1))
    {
        if ((player->allocflags & PlaF_CompCtrl) == 0)
        {
            if ((player->instance_num == PI_MapFadeTo)
             || (player->instance_num == PI_MapFadeFrom)
             || (player->instance_num == PI_CrCtrlFade)
             || (player->instance_num == PI_DirctCtrl)
             || (player->instance_num == PI_PsngrCtrl)
             || (player->instance_num == PI_DirctCtLeave)
             || (player->instance_num == PI_PsngrCtLeave))
            {
              can = false;
              break;
            }
        }
    }
  }
  if ( can )
  {
      player = get_my_player();
      set_flag_byte(&game.numfield_C, 0x01, curr_pause);
      if ((game.numfield_C & 0x01) != 0)
          set_flag_byte(&game.numfield_C, 0x80, new_pause);
      else
          set_flag_byte(&game.numfield_C, 0x01, false);
      if ( !SoundDisabled )
      {
        if ((game.numfield_C & 0x01) != 0)
        {
          SetSoundMasterVolume(settings.sound_volume >> 1);
          SetMusicPlayerVolume(settings.redbook_volume >> 1);
          SetMusicMasterVolume(settings.sound_volume >> 1);
        } else
        {
          SetSoundMasterVolume(settings.sound_volume);
          SetMusicPlayerVolume(settings.redbook_volume);
          SetMusicMasterVolume(settings.sound_volume);
        }
      }
      if ((game.numfield_C & 0x01) != 0)
      {
          if ((player->field_3 & 0x08) != 0)
          {
              PaletteSetPlayerPalette(player, engine_palette);
              player->field_3 &= ~0x08;
          }
      }
  }
}

void process_players_dungeon_control_packet_control(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    struct Camera *cam;
    unsigned long zoom_min,zoom_max;
    SYNCDBG(6,"Starting");
    player = get_player(plyr_idx);
    pckt = get_packet_direct(player->packet_num);
    cam = player->acamera;
    long inter_val;
    switch (cam->viewType)
    {
    case CAMERA_VIEW_EMPTY:
        inter_val = 2560000 / cam->zoom;
        break;
    case CAMERA_VIEW_PARCHMENT:
        inter_val = 12800000 / cam->zoom;
        break;
    default:
        inter_val = 256;
        break;
    }
    if (pckt->field_10 & PCAdV_Unknown01)
      inter_val *= 3;

    if ((pckt->control_flags & PCtr_MoveUp) != 0)
        view_set_camera_y_inertia(cam, -inter_val*3/4, -inter_val, true);
    if ((pckt->control_flags & PCtr_MoveDown) != 0)
        view_set_camera_y_inertia(cam, inter_val*3/4, inter_val, true);
    if ((pckt->control_flags & PCtr_MoveLeft) != 0)
        view_set_camera_x_inertia(cam, -inter_val*3/4, -inter_val, true);
    if ((pckt->control_flags & PCtr_MoveRight) != 0)
        view_set_camera_x_inertia(cam, inter_val*3/4, inter_val, true);
    if ((pckt->control_flags & PCtr_ViewRotateCCW) != 0)
    {
        switch (cam->viewType)
        {
        case CAMERA_VIEW_EMPTY:
             view_set_camera_rotation_inertia(cam, 48, 64, true);
            break;
        case CAMERA_VIEW_PARCHMENT:
            cam->orient_a = (cam->orient_a + 512) & 0x7FF;
            break;
        }
    }
    if ((pckt->control_flags & PCtr_ViewRotateCW) != 0)
    {
        switch (cam->viewType)
        {
        case CAMERA_VIEW_EMPTY:
            view_set_camera_rotation_inertia(cam, -24, -64, true);
            break;
        case CAMERA_VIEW_PARCHMENT:
            cam->orient_a = (cam->orient_a - 512) & 0x7FF;
            break;
        }
    }
    zoom_min = CAMERA_ZOOM_MIN;
    zoom_max = CAMERA_ZOOM_MAX;
    if (pckt->control_flags & PCtr_ViewZoomIn)
    {
        switch (cam->viewType)
        {
        case CAMERA_VIEW_EMPTY:
            view_zoom_camera_in(cam, zoom_max, zoom_min);
            update_camera_zoom_bounds(cam, zoom_max, zoom_min);
            break;
        default:
            view_zoom_camera_in(cam, zoom_max, zoom_min);
            break;
        }
    }
    if (pckt->control_flags & PCtr_ViewZoomOut)
    {
        switch (cam->viewType)
        {
        case CAMERA_VIEW_EMPTY:
            view_zoom_camera_out(cam, zoom_max, zoom_min);
            update_camera_zoom_bounds(cam, zoom_max, zoom_min);
            break;
        default:
            view_zoom_camera_out(cam, zoom_max, zoom_min);
            break;
        }
    }
    process_dungeon_control_packet_clicks(plyr_idx);
    set_mouse_light(player);
}

/**
 * Modifies a message string according to a key pressed.
 * @param message The message buffer.
 * @param maxlen Max length of the string (message buffer size - 1).
 * @param key The key which will affect the message.
 * @param kmodif Modifier keys pressed with the key.
 * @return Gives true if the message was modified by that key, false if it stayed without change.
 * @note We shouldn't use regional settings in this function, otherwise players playing different language
 * versions may get different messages from each other.
 */
TbBool message_text_key_add(char * message, long maxlen, TbKeyCode key, TbKeyMods kmodif)
{
    char chr;
    int chpos;
    chr = key_to_ascii(key, kmodif);
    chpos = strlen(message);
    if (key == KC_BACK)
    {
      if (chpos>0) {
          message[chpos-1] = '\0';
          return true;
      }
    } else
    if (((chr >= 'a') && (chr <= 'z')) ||
        ((chr >= 'A') && (chr <= 'Z')) ||
        ((chr >= '0') && (chr <= '9')) ||
        (chr == ' ')  || (chr == '!') || (chr == ':') || (chr == ';') ||
        (chr == '(') || (chr == ')') || (chr == '.'))
    {
        if (chpos < maxlen)
        {
            message[chpos] = chr;
            message[chpos+1] = '\0';
            return true;
        }
    }
    return false;
}

void process_players_message_character(struct PlayerInfo *player)
{
  struct Packet *pcktd;
  pcktd = get_packet(player->id_number);
  if (pcktd->actn_par1 > 0)
  {
      message_text_key_add(player->mp_message_text, PLAYER_MP_MESSAGE_LEN, pcktd->actn_par1, pcktd->actn_par2);
  }
}

void process_quit_packet(struct PlayerInfo *player, short complete_quit)
{
    struct PlayerInfo *swplyr;
    struct PlayerInfo *myplyr;
    short winning_quit;
    long plyr_count;
    int i;

    plyr_count = 0;
    myplyr = get_my_player();
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
      winning_quit = winning_player_quitting(player, &plyr_count);
      if (winning_quit)
      {
        // Set other players as losers
        for (i=0; i < PLAYERS_COUNT; i++)
        {
          swplyr = get_player(i);
          if (player_exists(swplyr))
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
          myplyr->field_3 |= 0x10;
      } else
      {
        if (!winning_quit)
        {
          if (player->victory_state != VicS_Undecided)
          {
            player->allocflags &= ~PlaF_Allocated;
          } else
          {
            player->allocflags |= PlaF_CompCtrl;
            toggle_computer_player(player->id_number);
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
          swplyr = get_player(i);
          if (player_exists(swplyr))
          {
            swplyr->allocflags &= ~PlaF_Allocated;
            swplyr->flgfield_6 |= PlaF6_PlyrHasQuit;
          }
        }
      } else
      {
        player->allocflags &= ~PlaF_Allocated;
      }
      return;
    }
    player->allocflags &= ~PlaF_Allocated;
    if (player == myplyr)
    {
        quit_game = 1;
        if (complete_quit)
          exit_keeper = 1;
    }
}

TbBool process_players_global_packet_action(PlayerNumber plyr_idx)
{
  //TODO PACKET add commands from beta
  struct PlayerInfo *player;
  struct PlayerInfo *myplyr;
  struct Packet *pckt;
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Room *room;
  int i;
  SYNCDBG(6,"Starting");
  player = get_player(plyr_idx);
  pckt = get_packet_direct(player->packet_num);
  switch (pckt->action)
  {
  case PckA_Unknown001:
      if (is_my_player(player))
      {
        turn_off_all_menus();
        frontend_save_continue_game(true);
        free_swipe_graphic();
      }
      player->flgfield_6 |= PlaF6_PlyrHasQuit;
      process_quit_packet(player, 0);
      return 1;
  case PckA_Unknown003:
      if (is_my_player(player))
      {
        turn_off_all_menus();
        frontend_save_continue_game(true);
      }
      player->flgfield_6 |= PlaF6_PlyrHasQuit;
      process_quit_packet(player, 1);
      return 1;
  case PckA_Unknown004:
      return 1;
  case PckA_FinishGame:
      if (is_my_player(player))
      {
        turn_off_all_menus();
        free_swipe_graphic();
      }
      if ((game.system_flags & GSF_NetworkActive) != 0)
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
      player->allocflags &= ~PlaF_Allocated;
      if (is_my_player(player))
      {
        frontend_save_continue_game(false);
      }
      return 0;
  case PckA_PlyrMsgBegin:
      player->allocflags |= PlaF_NewMPMessage;
      return 0;
  case PckA_PlyrMsgEnd:
      player->allocflags &= ~PlaF_NewMPMessage;
      if (player->mp_message_text[0] != '\0')
        message_add(player->id_number, player->mp_message_text);
      LbMemorySet(player->mp_message_text, 0, PLAYER_MP_MESSAGE_LEN);
      return 0;
  case PckA_ToggleLights:
      if (is_my_player(player))
      {
          light_set_lights_on(game.lish.field_4614D == 0);
      }
      return 1;
  case PckA_SwitchScrnRes:
      if (is_my_player(player))
      {
          switch_to_next_video_mode();
      }
      return 1;
  case PckA_TogglePause:
      process_pause_packet((game.numfield_C & 0x01) == 0, pckt->actn_par1);
      return 1;
  case PckA_SetCluedo:
      if (is_my_player(player))
      {
          settings.video_cluedo_mode = pckt->actn_par1;
          save_settings();
      }
      player->video_cluedo_mode = pckt->actn_par1;
      return 0;
  case PckA_Unknown025:
      if (is_my_player(player))
      {
        change_engine_window_relative_size(pckt->actn_par1, pckt->actn_par2);
        centre_engine_window();
      }
      return 0;
  case PckA_BookmarkLoad:
      set_player_cameras_position(player, subtile_coord_center(pckt->actn_par1), subtile_coord_center(pckt->actn_par2));
      return 0;
  case PckA_SetGammaLevel:
      if (is_my_player(player))
      {
        set_gamma(pckt->actn_par1, 1);
        save_settings();
      }
      return 0;
  case PckA_SetMinimapConf:
      player->minimap_zoom = pckt->actn_par1;
      return 0;
  case PckA_Unknown029:
      player->cameras[2].orient_a = pckt->actn_par1;
      player->cameras[3].orient_a = pckt->actn_par1;
      player->cameras[0].orient_a = pckt->actn_par1;
      return 0;
  case PckA_SetPlyrState:
      set_player_state(player, pckt->actn_par1, pckt->actn_par2);
      return 0;
  case PckA_SwitchView:
      set_engine_view(player, pckt->actn_par1);
      return 0;
  case PckA_ToggleTendency:
      toggle_creature_tendencies(player, pckt->actn_par1);
      if (is_my_player(player)) {
          dungeon = get_players_dungeon(player);
          game.creatures_tend_imprison = ((dungeon->creature_tendencies & 0x01) != 0);
          game.creatures_tend_flee = ((dungeon->creature_tendencies & 0x02) != 0);
      }
      return 0;
  case PckA_CheatEnter:
//      game.???[my_player_number].cheat_mode = 1;
      show_onscreen_msg(2*game.num_fps, "Cheat mode activated by player %d", plyr_idx);
      return 1;
  case PckA_CheatAllFree:
      make_all_creatures_free();
      make_all_rooms_free();
      make_all_powers_cost_free();
      return 1;
  case PckA_CheatCrtSpells:
      //TODO: remake from beta
      return 0;
  case PckA_CheatRevealMap:
      myplyr = get_my_player();
      reveal_whole_map(myplyr);
      return 0;
  case PckA_CheatCrAllSpls:
      //TODO: remake from beta
      return 0;
  case PckA_Unknown065:
      //TODO: remake from beta
      return 0;
  case PckA_CheatAllMagic:
      make_available_all_researchable_powers(my_player_number);
      return 0;
  case PckA_CheatAllRooms:
      make_available_all_researchable_rooms(my_player_number);
      return 0;
  case PckA_Unknown068:
      //TODO: remake from beta
      return 0;
  case PckA_Unknown069:
      //TODO: remake from beta
      return 0;
  case PckA_CheatAllResrchbl:
      make_all_powers_researchable(my_player_number);
      make_all_rooms_researchable(my_player_number);
      return 0;
  case PckA_Unknown080:
      set_player_mode(player, pckt->actn_par1);
      return 0;
  case PckA_ZoomFromMap:
      set_player_cameras_position(player, subtile_coord_center(pckt->actn_par1), subtile_coord_center(pckt->actn_par2));
      player->cameras[2].orient_a = 0;
      player->cameras[3].orient_a = 0;
      player->cameras[0].orient_a = 0;
      if (((game.system_flags & GSF_NetworkActive) != 0)
          || (lbDisplay.PhysicalScreenWidth > 320))
      {
        if (is_my_player_number(plyr_idx))
          toggle_status_menu((game.numfield_C & 0x40) != 0);
        set_player_mode(player, PVT_DungeonTop);
      } else
      {
        set_player_mode(player, PVT_MapFadeOut);
      }
      return 0;
  case PckA_Unknown082:
      process_pause_packet(pckt->actn_par1, pckt->actn_par2);
      return 1;
  case PckA_Unknown083:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      event_move_player_towards_event(player, pckt->actn_par1);
      return 0;
  case PckA_ZoomToRoom:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      room = room_get(pckt->actn_par1);
      player->zoom_to_pos_x = subtile_coord_center(room->central_stl_x);
      player->zoom_to_pos_y = subtile_coord_center(room->central_stl_y);
      set_player_instance(player, PI_ZoomToPos, 0);
      if (player->work_state == PSt_BuildRoom) {
          set_player_state(player, PSt_BuildRoom, room->kind);
      }
      return 0;
  case PckA_ZoomToTrap:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      thing = thing_get(pckt->actn_par1);
      player->zoom_to_pos_x = thing->mappos.x.val;
      player->zoom_to_pos_y = thing->mappos.y.val;
      set_player_instance(player, PI_ZoomToPos, 0);
      if ((player->work_state == PSt_PlaceTrap) || (player->work_state == PSt_PlaceDoor)) {
          set_player_state(player, PSt_PlaceTrap, thing->model);
      }
      return 0;
  case PckA_ZoomToDoor:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      thing = thing_get(pckt->actn_par1);
      player->zoom_to_pos_x = thing->mappos.x.val;
      player->zoom_to_pos_y = thing->mappos.y.val;
      set_player_instance(player, PI_ZoomToPos, 0);
      if ((player->work_state == PSt_PlaceTrap) || (player->work_state == PSt_PlaceDoor)) {
          set_player_state(player, PSt_PlaceDoor, thing->model);
      }
      return 0;
  case PckA_Unknown087:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      player->zoom_to_pos_x = pckt->actn_par1;
      player->zoom_to_pos_y = pckt->actn_par2;
      set_player_instance(player, PI_ZoomToPos, 0);
      return 0;
  case PckA_Unknown088:
      game.numfield_D ^= (game.numfield_D ^ (0x04 * ((game.numfield_D & 0x04) == 0))) & 0x04;
      return 0;
  case PckA_PwrCTADis:
      turn_off_power_call_to_arms(plyr_idx);
      return 0;
  case PckA_UsePwrHandPick:
      thing = thing_get(pckt->actn_par1);
      magic_use_available_power_on_thing(plyr_idx, PwrK_HAND, 0,thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing);
      return 0;
  case PckA_UsePwrHandDrop:
      dump_first_held_thing_on_map(plyr_idx, pckt->actn_par1, pckt->actn_par2, 1);
      return 0;
  case PckA_Unknown092:
      if (game.event[pckt->actn_par1].kind == 3)
      {
        turn_off_event_box_if_necessary(plyr_idx, pckt->actn_par1);
      } else
      {
        event_delete_event(plyr_idx, pckt->actn_par1);
      }
      return 0;
  case PckA_UsePwrObey:
      magic_use_available_power_on_level(plyr_idx, PwrK_OBEY, 0);
      return 0;
  case PckA_UsePwrArmageddon:
      magic_use_available_power_on_level(plyr_idx, PwrK_ARMAGEDDON, 0);
      return 0;
  case PckA_Unknown099:
      turn_off_query(plyr_idx);
      return 0;
  case PckA_Unknown104:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      battle_move_player_towards_battle(player, pckt->actn_par1);
      return 0;
  case PckA_ZoomToSpell:
      if (player->work_state == PSt_CreatrInfo)
        turn_off_query(plyr_idx);
      {
          struct Coord3d locpos;
          if (find_power_cast_place(plyr_idx, pckt->actn_par1, &locpos))
          {
              player->zoom_to_pos_x = locpos.x.val;
              player->zoom_to_pos_y = locpos.y.val;
              set_player_instance(player, PI_ZoomToPos, 0);
          }
      }
      if (!power_is_instinctive(pckt->actn_par1))
      {
          const struct PowerConfigStats *powerst;
          powerst = get_power_model_stats(pckt->actn_par1);
          i = get_power_index_for_work_state(player->work_state);
          if (i > 0)
            set_player_state(player, powerst->work_state, 0);
      }
      return 0;
  case PckA_PlyrFastMsg:
      //show_onscreen_msg(game.num_fps, "Message from player %d", plyr_idx);
      output_message(SMsg_EnemyHarassments+pckt->actn_par1, 0, true);
      return 0;
  case PckA_SetComputerKind:
      set_autopilot_type(plyr_idx, pckt->actn_par1);
      return 0;
  case PckA_GoSpectator:
      level_lost_go_first_person(plyr_idx);
      return 0;
  case PckA_DumpHeldThingToOldPos:
      dungeon = get_players_num_dungeon(plyr_idx);
      if (!power_hand_is_empty(player))
      {
          thing = get_first_thing_in_power_hand(player);
          dump_first_held_thing_on_map(plyr_idx, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 1);
      }
      return false;
  case PckA_PwrSOEDis:
      turn_off_power_sight_of_evil(plyr_idx);
      return false;
  case PckA_EventBoxActivate:
      go_on_then_activate_the_event_box(plyr_idx, pckt->actn_par1);
      return false;
  case PckA_EventBoxClose:
      dungeon = get_players_num_dungeon(plyr_idx);
      turn_off_event_box_if_necessary(plyr_idx, dungeon->visible_event_idx);
      dungeon->visible_event_idx = 0;
      return false;
  case PckA_UsePwrOnThing:
      i = get_power_overcharge_level(player);
      directly_cast_spell_on_thing(plyr_idx, pckt->actn_par1, pckt->actn_par2, i);
      return 0;
  case PckA_PlyrToggleAlly:
      toggle_ally_with_player(plyr_idx, pckt->actn_par1);
      return 0;
  case PckA_Unknown119:
      if (player->acamera != NULL)
          player->field_4B5 = player->acamera->viewType;
      set_player_mode(player, pckt->actn_par1);
      return false;
  case PckA_Unknown120:
      set_player_mode(player, pckt->actn_par1);
      set_engine_view(player, player->field_4B5);
      return false;
    default:
      return false;
  }
}

void process_players_map_packet_control(long plyr_idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    SYNCDBG(6,"Starting");
    player = get_player(plyr_idx);
    pckt = get_packet_direct(player->packet_num);
    // Get map coordinates
    process_map_packet_clicks(plyr_idx);
    player->cameras[2].mappos.x.val = pckt->pos_x;
    player->cameras[2].mappos.y.val = pckt->pos_y;
    set_mouse_light(player);
    SYNCDBG(8,"Finished");
}

void process_map_packet_clicks(long plyr_idx)
{
    SYNCDBG(7,"Starting");
    struct Packet *pckt;
    packet_left_button_double_clicked[plyr_idx] = 0;
    pckt = get_packet(plyr_idx);
    if ((pckt->control_flags & PCtr_Unknown4000) == 0)
    {
        update_double_click_detection(plyr_idx);
    }
    SYNCDBG(8,"Finished");
}

// Apply local or received packets to game.
void process_players_packet(long idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    player = get_player(idx);
    pckt = get_packet_direct(player->packet_num);
    SYNCDBG(6, "Processing player %d packet of type %d.", idx, (int)pckt->action);
    player->boolfield_4 = ((pckt->field_10 & 0x20) != 0);
    player->boolfield_5 = ((pckt->field_10 & 0x40) != 0);
    if (((player->allocflags & PlaF_NewMPMessage) != 0) && (pckt->action == PckA_PlyrMsgChar))
    {
        process_players_message_character(player);
    }
    else if (!process_players_global_packet_action(idx))
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
  SYNCDBG(8,"Finished");
}

void process_players_creature_passenger_packet_action(long idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    SYNCDBG(6,"Starting");
    player = get_player(idx);
    pckt = get_packet_direct(player->packet_num);
    if (pckt->action == PckA_PasngrCtrlExit)
    {
        player->influenced_thing_idx = pckt->actn_par1;
        set_player_instance(player, PI_PsngrCtLeave, 0);
    }
    SYNCDBG(8,"Finished");
}

TbBool process_players_dungeon_control_packet_action(long plyr_idx)
{
    SYNCDBG(6, "Starting");
    struct PlayerInfo *player;
    struct Packet *pckt;
    struct Camera *cam;

    player = get_player(plyr_idx);
    pckt = get_packet_direct(player->packet_num);
    cam = player->acamera;

    switch (pckt->action)
    {
    case PckA_HoldAudience:
        magic_use_available_power_on_level(plyr_idx, PwrK_HOLDAUDNC, 0);
        break;
    case PckA_UseSpecialBox:
        activate_dungeon_special(thing_get(pckt->actn_par1), player);
        break;
    case PckA_ResurrectCrtr:
        resurrect_creature(thing_get(pckt->actn_par1), (pckt->actn_par2) & 0x0F, (pckt->actn_par2 >> 4) & 0xFF,
            (pckt->actn_par2 >> 12) & 0x0F);
        break;
    case PckA_TransferCreatr:
        transfer_creature(thing_get(pckt->actn_par1), thing_get(pckt->actn_par2), plyr_idx);
        break;
    case PckA_ToggleComputer:
        toggle_computer_player(plyr_idx);
        break;    
    case PckA_DragMoveCameraLeftUp:
        view_set_camera_x_inertia(cam, -pckt->actn_par1, 1000000/* no upper bound */, false);
        view_set_camera_y_inertia(cam, -pckt->actn_par2, 1000000/* no upper bound */, false);
        break;    
    case PckA_DragMoveCameraLeftDown:
        view_set_camera_x_inertia(cam, -pckt->actn_par1, 1000000/* no upper bound */, false);
        view_set_camera_y_inertia(cam, pckt->actn_par2, 1000000/* no upper bound */, false);
        break;
    case PckA_DragMoveCameraRightUp:
        view_set_camera_x_inertia(cam, pckt->actn_par1, 1000000/* no upper bound */, false);
        view_set_camera_y_inertia(cam, -pckt->actn_par2, 1000000/* no upper bound */, false);
        break;
    case PckA_DragMoveCameraRightDown:
        view_set_camera_x_inertia(cam, pckt->actn_par1, 1000000/* no upper bound */, false);
        view_set_camera_y_inertia(cam, pckt->actn_par2, 1000000/* no upper bound */, false);
        break;
    case PckA_DragRotateCameraCW:
        view_set_camera_rotation_inertia(cam, pckt->actn_par1, 1000000/* no upper bound */, false);
        break;
    case PckA_DragRotateCameraCCW:
        view_set_camera_rotation_inertia(cam, -pckt->actn_par1, 1000000/* no upper bound */, false);
        break;
    default:
        return false;
    }
    return true;
}

void process_players_creature_control_packet_control(long idx)
{
    struct PlayerInfo *player;
    struct Packet *pckt;
    struct Thing *cctng;
    struct CreatureControl *ccctrl;
    struct InstanceInfo *inst_inf;
    struct CreatureStats *crstat;
    long speed_limit;
    long angle_limit,angle;
    long i,k,n;

    SYNCDBG(6,"Starting");
    player = get_player(idx);
    pckt = get_packet_direct(player->packet_num);
    cctng = thing_get(player->controlled_thing_idx);
    if (cctng->class_id != TCls_Creature)
        return;
    ccctrl = creature_control_get_from_thing(cctng);
    if (creature_is_dying(cctng))
        return;
    if ((ccctrl->stateblock_flags != 0) || (cctng->active_state == CrSt_CreatureUnconscious))
        return;
    speed_limit = get_creature_speed(cctng);
    if ((pckt->control_flags & PCtr_MoveUp) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->move_speed = compute_controlled_speed_increase(ccctrl->move_speed, speed_limit);
            ccctrl->flgfield_1 |= CCFlg_Unknown40;
        } else
        {
            ERRORLOG("No creature to increase speed");
        }
    }
    if ((pckt->control_flags & PCtr_MoveDown) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->move_speed = compute_controlled_speed_decrease(ccctrl->move_speed, speed_limit);
            ccctrl->flgfield_1 |= CCFlg_Unknown40;
        } else
        {
            ERRORLOG("No creature to decrease speed");
        }
    }
    if ((pckt->control_flags & PCtr_MoveLeft) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->orthogn_speed = compute_controlled_speed_increase(ccctrl->orthogn_speed, speed_limit);
            ccctrl->flgfield_1 |= CCFlg_Unknown80;
        } else
        {
            ERRORLOG("No creature to increase speed");
        }
    }
    if ((pckt->control_flags & PCtr_MoveRight) != 0)
    {
        if (!creature_control_invalid(ccctrl))
        {
            ccctrl->orthogn_speed = compute_controlled_speed_decrease(ccctrl->orthogn_speed, speed_limit);
            ccctrl->flgfield_1 |= CCFlg_Unknown80;
        } else
        {
            ERRORLOG("No creature to decrease speed");
        }
    }

    if ((pckt->control_flags & PCtr_LBtnRelease) != 0)
    {
        if (ccctrl->instance_id == CrInst_NULL)
        {
            if (creature_instance_is_available(cctng, ccctrl->field_1E8))
            {
                if (creature_instance_has_reset(cctng, ccctrl->field_1E8))
                {
                    i = ccctrl->field_1E8;
                    inst_inf = creature_instance_info_get(i);
                    n = get_human_controlled_creature_target(cctng, inst_inf->field_1D);
                    set_creature_instance(cctng, i, 1, n, 0);
                }
            }
        }
    }
    if ((pckt->control_flags & PCtr_LBtnHeld) != 0)
    {
        i = ccctrl->field_1E8;
        inst_inf = creature_instance_info_get(i);
        if (inst_inf->field_1A)
        {
            k = ccctrl->instance_id;
            if ((k == CrInst_NULL) || (k == i))
            {
              n = get_human_controlled_creature_target(cctng, inst_inf->field_1D);
              set_creature_instance(cctng, i, 1, n, 0);
            }
        }
    }
    crstat = creature_stats_get_from_thing(cctng);
    i = pckt->pos_y;
    if (i < 5)
      i = 5;
    else
    if (i > 250)
      i = 250;
    k = i - 127;
    angle = (pckt->pos_x - 127) / player->field_14;
    if (angle != 0)
    {
      if (angle < -32)
          angle = -32;
      else
      if (angle > 32)
          angle = 32;
      ccctrl->field_6C += 56 * angle / 32;
    }
    angle_limit = crstat->max_angle_change;
    if (angle_limit < 1)
        angle_limit = 1;
    angle = ccctrl->field_6C;
    if (angle < -angle_limit)
        angle = -angle_limit;
    else
    if (angle > angle_limit)
        angle = angle_limit;
    cctng->move_angle_xy = (cctng->move_angle_xy + angle) & LbFPMath_AngleMask;
    cctng->move_angle_z = (227 * k / 127) & LbFPMath_AngleMask;
    ccctrl->field_CC = 170 * angle / angle_limit;
    ccctrl->field_6C = 4 * angle / 8;
}

void process_players_creature_control_packet_action(long plyr_idx)
{
  struct CreatureControl *cctrl;
  struct InstanceInfo *inst_inf;
  struct PlayerInfo *player;
  struct Thing *thing;
  struct Packet *pckt;
  long i,k;
  SYNCDBG(6,"Starting");
  player = get_player(plyr_idx);
  pckt = get_packet_direct(player->packet_num);
  switch (pckt->action)
  {
  case PckA_Unknown033:
      player->influenced_thing_idx = pckt->actn_par1;
      set_player_instance(player, PI_DirctCtLeave, 0);
      break;
  case PckA_CtrlCrtrSetInstnc:
      thing = thing_get(player->controlled_thing_idx);
      if (thing_is_invalid(thing))
        break;
      cctrl = creature_control_get_from_thing(thing);
      if (creature_control_invalid(cctrl))
        break;
      i = pckt->actn_par1;
      inst_inf = creature_instance_info_get(i);
      if (!inst_inf->field_0)
      {
        cctrl->field_1E8 = i;
      } else
      if (cctrl->instance_id == CrInst_NULL)
      {
        if (creature_instance_is_available(thing,i) && creature_instance_has_reset(thing, pckt->actn_par1))
        {
          i = pckt->actn_par1;
          inst_inf = creature_instance_info_get(i);
          k = get_human_controlled_creature_target(thing, inst_inf->field_1D);
          set_creature_instance(thing, i, 1, k, 0);
          if (plyr_idx == my_player_number) {
              instant_instance_selected(i);
          }
        }
      }
      break;
  }
}

TbBool open_packet_file_for_load(char *fname, struct CatalogueEntry *centry)
{
    int i;
    LbMemorySet(centry, 0, sizeof(struct CatalogueEntry));
    strcpy(game.packet_fname, fname);
    game.packet_save_fp = LbFileOpen(game.packet_fname, Lb_FILE_MODE_READ_ONLY);
    if (game.packet_save_fp == -1)
    {
        ERRORLOG("Cannot open keeper packet file for load");
        game.packet_fopened = 0;
        return false;
    }
    i = load_game_chunks(game.packet_save_fp,centry);
    if ((i != GLoad_PacketStart) && (i != GLoad_PacketContinue))
    {
        LbFileClose(game.packet_save_fp);
        game.packet_save_fp = -1;
        game.packet_fopened = 0;
        WARNMSG("Couldn't correctly read packet file \"%s\" header.",fname);
        return false;
    }
    game.packet_file_pos = LbFilePosition(game.packet_save_fp);
    game.turns_stored = (LbFileLengthHandle(game.packet_save_fp) - game.packet_file_pos) / PACKET_TURN_SIZE;
    if ((game.packet_checksum_verify) && (!game.packet_save_head.chksum_available))
    {
        WARNMSG("PacketSave checksum not available, checking disabled.");
        game.packet_checksum_verify = false;
    }
    if (game.log_things_start_turn == -1)
    {
        game.log_things_start_turn = 0;
        game.log_things_end_turn = game.turns_stored + 1;
    }
    game.packet_fopened = 1;
    return true;
}

void post_init_packets(void)
{
    struct CatalogueEntry centry;
    SYNCDBG(6,"Starting");
    if ((game.packet_load_enable) && (game.numfield_149F47))
    {
        open_packet_file_for_load(game.packet_fname,&centry);
        game.pckt_gameturn = 0;
    }
    clear_packets();
}

short save_packets(void)
{
    const int turn_data_size = PACKET_TURN_SIZE;
    unsigned char pckt_buf[PACKET_TURN_SIZE+4];
    TbBigChecksum chksum;
    int i;
    SYNCDBG(6,"Starting");
    if (game.packet_checksum_verify)
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
      ERRORLOG("Packet file write error");
    }
    if ( !LbFileFlush(game.packet_save_fp) )
    {
      ERRORLOG("Unable to flush PacketSave File");
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
        game.packet_save_fp = -1;
    }
}

void dump_memory_to_file(const char * fname, const char * buf, size_t len)
{
    FILE *file;
    file = fopen(fname, "w");
    fwrite(buf, 1, len, file);
    fflush(file);
    fclose(file);
}

void write_debug_packets(void)
{
    //note, changed this to be more general and to handle multiplayer where there can
    //be several players writing to same directory if testing on local machine
    char filename[32];
    snprintf(filename, sizeof(filename), "%s%u.%s", "keeperd", my_player_number, "pck");
    dump_memory_to_file(filename, (char*) game.packets, sizeof(game.packets));
}

void write_debug_screenpackets(void)
{
    char filename[32];
    snprintf(filename, sizeof(filename), "%s%u.%s", "keeperd", my_player_number, "spck");
    dump_memory_to_file(filename, (char*) net_screen_packet, sizeof(net_screen_packet));
}

// Process local and received packets.
void process_packets(void)
{
    int i, j, k;
    struct Packet *pckt;
    struct PlayerInfo *player;
    SYNCDBG(5, "Starting");

    lbDisplay.DrawColour = colours[15][15][15];

    // Exchange packets with the network
    if (game.game_type != GameType_LocalGame)
    {
        player = get_my_player();
        j = 0;
        for (i = 0; i < 4; i++)
        {
            if (is_network_player_active(i))
            {
                j++;
            }
        }

        if (!game.packet_load_enable || game.numfield_149F47)
        {
            pckt = get_packet_direct(player->packet_num);
            if (LbNetwork_Exchange(pckt) != Lb_OK)
            {
                ERRORLOG("LbNetwork_Exchange failed");
            }
        }
        k = 0;
        for (i = 0; i < 4; i++)
        {
            if (is_network_player_active(i))
                k++;
        }
        if (j != k)
        {
            for (i = 0; i < 4; i++)
            {
                player = get_player(i);
                if (is_network_player_active(player->packet_num))
                {
                    player->allocflags |= PlaF_CompCtrl;
                    toggle_computer_player(i);
                }
            }
        }
    }

    // Setting checksum problem flags
    switch (checksums_different())
    {
    case 1: // GSF_NetGameNoSync
        set_flag_byte(&game.system_flags, GSF_NetGameNoSync, true);
        set_flag_byte(&game.system_flags, GSF_NetSeedNoSync, false);
        break;
    case 2: // GSF_NetSeedNoSync
        set_flag_byte(&game.system_flags, GSF_NetGameNoSync, false);
        set_flag_byte(&game.system_flags, GSF_NetSeedNoSync, true);
        break;
    case 3: // GSF_NetGameNoSync and GSF_NetSeedNoSync
        set_flag_byte(&game.system_flags, GSF_NetGameNoSync, true);
        set_flag_byte(&game.system_flags, GSF_NetSeedNoSync, true);
        break;
    default: // Everything at sync
        set_flag_byte(&game.system_flags, GSF_NetGameNoSync, false);
        set_flag_byte(&game.system_flags, GSF_NetSeedNoSync, false);
        break;
    }
    // Write packets into file, if requested
    if ((game.packet_save_enable) && (game.packet_fopened))
        save_packets();
    //Debug code, to find packet errors
#if DEBUG_NETWORK_PACKETS
    write_debug_packets();
#endif
    // Process the packets
    for (i = 0; i < PACKETS_COUNT; i++)
    {
        player = get_player(i);
        if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
            process_players_packet(i);
    }
    // Clear all packets
    clear_packets();
    if (((game.system_flags & GSF_NetGameNoSync) != 0)
        || ((game.system_flags & GSF_NetSeedNoSync) != 0))
    {
        SYNCDBG(0, "Resyncing");
        resync_game();
    }
    SYNCDBG(7, "Finished");
}

void process_frontend_packets(void)
{
  struct ScreenPacket *nspckt;
  struct PlayerInfo *player;
  long i,k;
  unsigned short c;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    net_screen_packet[i].field_4 &= 0xFE;
  }
  nspckt = &net_screen_packet[my_player_number];
  set_flag_byte(&nspckt->field_4, 0x01, true);
  nspckt->field_5 = frontend_alliances;
  set_flag_byte(&nspckt->field_4, 0x01, true);
  nspckt->field_4 ^= ((nspckt->field_4 ^ (fe_computer_players << 1)) & 0x06);
  nspckt->field_6 = VersionMajor;
  nspckt->field_8 = VersionMinor;
  if (LbNetwork_Exchange(nspckt))
    ERRORLOG("LbNetwork_Exchange failed");
  if (frontend_should_all_players_quit())
  {
    i = frontnet_number_of_players_in_session();
    if (players_currently_in_session < i)
    {
      players_currently_in_session = i;
    }
    if (players_currently_in_session > i)
    {
      if (frontend_menu_state == FeSt_NET_SESSION)
      {
          if (LbNetwork_Stop())
          {
            ERRORLOG("LbNetwork_Stop() failed");
            return;
          }
          frontend_set_state(FeSt_MAIN_MENU);
      } else
      if (frontend_menu_state == FeSt_NET_START)
      {
          if (LbNetwork_Stop())
          {
            ERRORLOG("LbNetwork_Stop() failed");
            return;
          }
          if (setup_network_service(net_service_index_selected))
          {
            frontend_set_state(FeSt_NET_SESSION);
          } else
          {
            frontend_set_state(FeSt_MAIN_MENU);
          }
      }
    }
  }
#if DEBUG_NETWORK_PACKETS
  write_debug_screenpackets();
#endif
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    nspckt = &net_screen_packet[i];
    player = get_player(i);
    if ((nspckt->field_4 & 0x01) != 0)
    {
      switch (nspckt->field_4 >> 3)
      {
      case 2:
        add_message(i, (char *)&nspckt->param1);
        break;
      case 3:
        if (!validate_versions())
        {
          versions_different_error();
          break;
        }
        fe_network_active = 1;
        frontend_set_state(FeSt_NETLAND_VIEW);
        break;
      case 4:
        frontend_set_alliance(nspckt->param1, nspckt->param2);
        break;
      case 7:
        fe_computer_players = nspckt->param1;
        break;
      case 8:
        k = strlen(player->mp_message_text);
        if (nspckt->param1 == KC_BACK)
        {
          if (k > 0)
          {
            k--;
            player->mp_message_text[k] = '\0';
          }
        } else
        if (nspckt->param1 == KC_RETURN)
        {
          if (k > 0)
          {
            add_message(i, player->mp_message_text);
            k = 0;
            player->mp_message_text[k] = '\0';
          }
        } else
        {
          c = key_to_ascii(nspckt->param1, nspckt->param2);
          if ((c != 0) && (frontend_font_char_width(1,c) > 1) && (k < 62))
          {
            player->mp_message_text[k] = c;
            k++;
            player->mp_message_text[k] = '\0';
          }
        }
        if (frontend_font_string_width(1,player->mp_message_text) >= 420)
        {
          if (k > 0)
          {
            k--;
            player->mp_message_text[k] = '\0';
          }
        }
        break;
      default:
        break;
      }
      if (frontend_alliances == -1)
      {
        if (nspckt->field_5 != -1)
          frontend_alliances = nspckt->field_5;
      }
      if (fe_computer_players == 2)
      {
        k = ((nspckt->field_4 & 0x06) >> 1);
        if (k != 2)
          fe_computer_players = k;
      }
      player->field_4E7 = nspckt->field_8 + (nspckt->field_6 << 8);
    }
    nspckt->field_4 &= 0x07;
  }
  if (frontend_alliances == -1)
    frontend_alliances = 0;
  for (i=0; i < NET_PLAYERS_COUNT; i++)
  {
    nspckt = &net_screen_packet[i];
    if ((nspckt->field_4 & 0x01) == 0)
    {
      if (frontend_is_player_allied(my_player_number, i))
        frontend_set_alliance(my_player_number, i);
    }
  }
}

/******************************************************************************/
