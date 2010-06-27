/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_hand.c
 *     power_hand support functions.
 * @par Purpose:
 *     Functions to power_hand.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "power_hand.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"

#include "power_specials.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "creature_graphics.h"
#include "config_creature.h"
#include "player_instances.h"
#include "kjm_input.h"
#include "front_input.h"
#include "frontend.h"
#include "gui_draw.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT unsigned long _DK_object_is_pickable_by_hand(struct Thing *thing, long a2);
DLLIMPORT void _DK_set_power_hand_offset(struct PlayerInfo *player, struct Thing *thing);
DLLIMPORT struct Thing *_DK_process_object_being_picked_up(struct Thing *thing, long a2);
DLLIMPORT void _DK_set_power_hand_graphic(long a1, long a2, long a3);
DLLIMPORT long _DK_dump_thing_in_power_hand(struct Thing *thing, long a2);
DLLIMPORT void _DK_draw_power_hand(void);
DLLIMPORT long _DK_prepare_thing_for_power_hand(unsigned short tng_idx, long plyr_idx);
DLLIMPORT void _DK_draw_mini_things_in_hand(long x, long y);
DLLIMPORT void _DK_create_power_hand(unsigned char a1);
DLLIMPORT struct Thing *_DK_get_nearest_thing_for_hand_or_slap(unsigned char a1, long a2, long a3);
DLLIMPORT struct Thing *_DK_get_nearest_thing_for_slap(unsigned char plyr_idx, long x, long y);
DLLIMPORT void _DK_process_things_in_dungeon_hand(void);
DLLIMPORT long _DK_place_thing_in_power_hand(struct Thing *thing, long var);
DLLIMPORT short _DK_dump_held_things_on_map(unsigned char a1, long a2, long a3, short a4);

/******************************************************************************/
unsigned long object_is_pickable_by_hand(struct Thing *thing, long a2)
{
  return _DK_object_is_pickable_by_hand(thing, a2);
}

short thing_is_pickable_by_hand(struct PlayerInfo *player,struct Thing *thing)
{
  if (thing_is_invalid(thing))
    return false;
  if (((thing->field_0 & 0x01) == 0) || (thing->field_9 != player->field_440))
    return false;
  // All creatures can be picked
  if (thing->class_id == TCls_Creature)
    return true;
  // Some objects can be picked
  if ((thing->class_id == TCls_Object) && object_is_pickable_by_hand(thing, player->id_number))
    return true;
  // Other things are not pickable
  return false;
}

void set_power_hand_offset(struct PlayerInfo *player, struct Thing *thing)
{
  _DK_set_power_hand_offset(player, thing);
}

struct Thing *process_object_being_picked_up(struct Thing *thing, long plyr_idx)
{
  struct PlayerInfo *player;
  struct Thing *picktng;
  struct Coord3d pos;
  long i;
  switch (thing->model)
  {
    case 3:
    case 6:
    case 43:
      i = thing->long_13;
      if (i != 0)
      {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val + 128;
        create_price_effect(&pos, thing->owner, i);
      }
      picktng = thing;
      break;
    case 10:
      i = UNSYNC_RANDOM(3);
      thing_play_sample(thing, 109+i, 100, 0, 3, 0, 2, 256);
      i = convert_td_iso(122);
      set_thing_draw(thing, i, 256, -1, -1, 0, 2);
      remove_food_from_food_room_if_possible(thing);
      picktng = thing;
      break;
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
      picktng = create_gold_for_hand_grab(thing, plyr_idx);
      break;
    case 86:
    case 87:
    case 88:
    case 89:
    case 90:
    case 91:
    case 92:
    case 93:
      player = get_player(plyr_idx);
      activate_dungeon_special(thing, player);
      picktng = NULL;
      break;
    default:
      ERRORLOG("Picking up invalid object");
      picktng = NULL;
      break;
  }
  return picktng;
}

void set_power_hand_graphic(long plyr_idx, long a2, long a3)
{
  struct PlayerInfo *player;
  struct Thing *thing;
  player = get_player(plyr_idx);
  if (player->field_10 >= game.play_gameturn)
  {
    if ((a2 == 786) || (a2 == 787))
      player->field_10 = 0;
  }
  if (player->field_10 < game.play_gameturn)
  {
    if (player->field_C != a2)
    {
      player->field_C = a2;
      thing = thing_get(player->hand_thing_idx);
      if ((a2 == 782) || (a2 == 781))
      {
        set_thing_draw(thing, a2, a3, 300, 0, 0, 2);
      } else
      {
        set_thing_draw(thing, a2, a3, 300, 1, 0, 2);
      }
      thing = get_first_thing_in_power_hand(player);
      set_power_hand_offset(player,thing);
    }
  }
}

TbBool power_hand_is_empty(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  return (dungeon->things_in_hand[0] == 0);
}

struct Thing *get_first_thing_in_power_hand(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  dungeon = get_dungeon(player->id_number);
  return thing_get(dungeon->things_in_hand[0]);
}

/** Silently removes a thing from player's power hand.
 */
TbBool remove_thing_from_power_hand(struct Thing *thing, long plyr_idx)
{
  struct Dungeon *dungeon;
  long i;
  dungeon = get_dungeon(plyr_idx);
  for (i = 0; i < dungeon->field_63; i++)
  {
    if (dungeon->things_in_hand[i] == thing->index)
    {
      for ( ; i < dungeon->field_63-1; i++)
      {
        dungeon->things_in_hand[i] = dungeon->things_in_hand[i+1];
      }
      dungeon->field_63--;
      dungeon->things_in_hand[dungeon->field_63] = 0;
      return true;
    }
  }
  return false;
}

/** Puts a thing into player's power hand.
 */
TbBool dump_thing_in_power_hand(struct Thing *thing, long plyr_idx)
{
  struct Dungeon *dungeon;
  long i;
  //return _DK_dump_thing_in_power_hand(thing, plyr_idx);
  dungeon = get_dungeon(plyr_idx);
  if (dungeon->field_63 >= MAX_THINGS_IN_HAND)
    return false;
  // Move all things in list up, to free position 0
  for (i = MAX_THINGS_IN_HAND-1; i > 0; i--)
  {
    dungeon->things_in_hand[i] = dungeon->things_in_hand[i-1];
  }
  dungeon->field_63++;
  dungeon->things_in_hand[0] = thing->index;
  if (thing->class_id == TCls_Creature)
    remove_all_traces_of_combat(thing);
  if (is_my_player_number(thing->owner))
  {
    if (thing->class_id == TCls_Creature)
      play_creature_sound(thing, CrSnd_HandPick, 3, 1);
  }
  return true;
}

void place_thing_in_limbo(struct Thing *thing)
{
  remove_thing_from_mapwho(thing);
  thing->field_4F |= 0x01;
  thing->field_0 |= 0x10;
}

void draw_power_hand(void)
{
  SYNCDBG(7,"Starting");
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct CreatureControl *cctrl;
  struct CreaturePickedUpOffset *pickoffs;
  struct Thing *thing;
  struct Thing *picktng;
  struct Room *room;
  struct RoomData *rdata;
  long x,y;
  //_DK_draw_power_hand(); return;
  player = get_my_player();
  dungeon = get_dungeon(player->id_number);
  if ((player->field_6 & 0x01) != 0)
    return;
  if (game.small_map_state == 2)
    return;
  lbDisplay.DrawFlags = 0x00;
  if (player->view_type != 1)
    return;
  if (((game.numfield_C & 0x20) != 0) && (game.small_map_state != 2)
    && mouse_is_over_small_map(player->mouse_x, player->mouse_y) )
  {
    x = game.hand_over_subtile_x;
    y = game.hand_over_subtile_y;
    room = subtile_room_get(x,y);
    if ((!room_is_invalid(room)) && (subtile_revealed(x, y, player->id_number)))
    {
      rdata = room_data_get_for_room(room);
      draw_gui_panel_sprite_centered(GetMouseX()+24, GetMouseY()+32, rdata->numfield_1);
    }
    if ((!power_hand_is_empty(player)) && (game.small_map_state == 1))
    {
      draw_mini_things_in_hand(GetMouseX()+10, GetMouseY()+10);
    }
    return;
  }
  if (game_is_busy_doing_gui())
  {
    draw_mini_things_in_hand(GetMouseX()+10, GetMouseY()+10);
    return;
  }
  thing = thing_get(player->hand_thing_idx);
  if (thing_is_invalid(thing))
    return;
  if (player->field_10 > game.play_gameturn)
  {
    process_keeper_sprite((GetMouseX()+60) / pixel_size, (GetMouseY()+40)/pixel_size,
      thing->field_44, 0, thing->field_48, 64 / pixel_size);
    draw_mini_things_in_hand(GetMouseX()+60, GetMouseY());
    return;
  }
  if ((player->field_3 & 0x02) != 0)
  {
    draw_mini_things_in_hand(GetMouseX()+18, GetMouseY());
    return;
  }
  if (player->work_state != 5)
  {
    if ( (player->work_state != 1)
      || (player->field_455 != 3) && ((player->work_state != 1) || (player->field_455) || (player->field_454 != 3)) )
    {
      if ((player->instance_num != 1) && (player->instance_num != 2))
      {
        if (player->work_state == PSt_Slap)
        {
          process_keeper_sprite((GetMouseX()+70) / pixel_size, (GetMouseY()+46) / pixel_size,
              thing->field_44, 0, thing->field_48, 64 / pixel_size);
        } else
        if (player->work_state == 1)
        {
          if ((player->field_455 == 2) || (player->field_454 == 2))
          {
            draw_mini_things_in_hand(GetMouseX()+18, GetMouseY());
          }
        }
        return;
      }
    }
  }
  picktng = get_first_thing_in_power_hand(player);
  if ((!thing_is_invalid(picktng)) && ((picktng->field_4F & 0x01) == 0))
  {
    switch (picktng->class_id)
    {
    case TCls_Creature:
        cctrl = creature_control_get_from_thing(picktng);
        if ((cctrl->field_AD & 0x02) == 0)
        {
            pickoffs = get_creature_picked_up_offset(picktng);
            x = GetMouseX() + pickoffs->delta_x;
            y = GetMouseY() + pickoffs->delta_y;
            if (creatures[picktng->model].field_7 )
              EngineSpriteDrawUsingAlpha = 1;
            process_keeper_sprite(x / pixel_size, y / pixel_size,
                picktng->field_44, 0, picktng->field_48, 64 / pixel_size);
            EngineSpriteDrawUsingAlpha = 0;
        } else
        {
            x = GetMouseX()+11;
            y = GetMouseY()+56;
            process_keeper_sprite(x / pixel_size, y / pixel_size,
                picktng->field_44, 0, picktng->field_48, 64 / pixel_size);
        }
        break;
    case TCls_Object:
        if (object_is_mature_food(picktng))
        {
          x = GetMouseX()+11;
          y = GetMouseY()+56;
          process_keeper_sprite(x / pixel_size, y / pixel_size,
              picktng->field_44, 0, picktng->field_48, 64 / pixel_size);
          break;
        } else
        if ((picktng->class_id == TCls_Object) && object_is_gold_pile(picktng))
          break;
    default:
        x = GetMouseX();
        y = GetMouseY();
        process_keeper_sprite(x / pixel_size, y / pixel_size,
              picktng->field_44, 0, picktng->field_48, 64 / pixel_size);
        break;
    }
  }
  if (player->field_C == 784)
  {
    x = GetMouseX()+58;
    y = GetMouseY()+6;
    process_keeper_sprite(x / pixel_size, y / pixel_size,
        thing->field_44, 0, thing->field_48, 64 / pixel_size);
    draw_mini_things_in_hand(GetMouseX()+60, GetMouseY());
  } else
  {
    x = GetMouseX()+60;
    y = GetMouseY()+40;
    process_keeper_sprite(x / pixel_size, y / pixel_size,
        thing->field_44, 0, thing->field_48, 64 / pixel_size);
    draw_mini_things_in_hand(GetMouseX()+60, GetMouseY());
  }
}

void get_nearest_thing_for_hand_or_slap_on_map_block(long *near_distance, struct Thing **near_thing,struct Map *mapblk, long plyr_idx, long x, long y)
{
  struct Thing *thing;
  long i;
  unsigned long k;
  k = 0;
  i = get_mapwho_thing_index(mapblk);
  while (i != 0)
  {
    thing = thing_get(i);
    if (thing_is_invalid(thing))
    {
      ERRORLOG("Jump to invalid thing detected");
      break;
    }
    i = thing->field_2;
    // Begin per-loop code
    if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0) && (thing->field_7 != 67))
    {
      if (can_thing_be_picked_up_by_player(thing, plyr_idx) || thing_slappable(thing, plyr_idx))
      {
        if (*near_distance > 2 * abs(y-thing->mappos.y.stl.pos))
        {
          *near_distance = 2 * abs(y-thing->mappos.y.stl.pos);
          *near_thing = thing;
        }
      }
    }
    // End of per-loop code
    k++;
    if (k > THINGS_COUNT)
    {
      ERRORLOG("Infinite loop detected when sweeping things list");
      break;
    }
  }
}

struct Thing *get_nearest_thing_for_slap(PlayerNumber plyr_idx, MapCoord x, MapCoord y)
{
    return _DK_get_nearest_thing_for_slap(plyr_idx, x, y);
}

struct Thing *get_nearest_thing_for_hand_or_slap(PlayerNumber plyr_idx, MapCoord x, MapCoord y)
{
  long near_distance;
  struct Thing *near_thing;
  struct Map *mapblk;
  long sx,sy;
  int around;
  //return _DK_get_nearest_thing_for_hand_or_slap(plyr_idx, x, y);
  near_distance = LONG_MAX;
  near_thing = NULL;
  for (around=4; around < sizeof(small_around_pos)/sizeof(small_around_pos[0]); around++)
  {
    sx = (x >> 8) + stl_num_decode_x(small_around_pos[around]);
    sy = (y >> 8) + stl_num_decode_y(small_around_pos[around]);
    mapblk = get_map_block_at(sx, sy);
    if (!map_block_invalid(mapblk))
    {
      if (map_block_revealed(mapblk, plyr_idx))
      {
        get_nearest_thing_for_hand_or_slap_on_map_block(&near_distance,&near_thing,mapblk, plyr_idx, x, y);
      }
    }
  }
  return near_thing;
}

long place_thing_in_power_hand(struct Thing *thing, long var)
{
  return _DK_place_thing_in_power_hand(thing, var);
}

short dump_held_things_on_map(unsigned int plyridx, long a2, long a3, short a4)
{
  return _DK_dump_held_things_on_map(plyridx, a2, a3, a4);
}

void clear_things_in_hand(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  long i;
  dungeon = get_dungeon(player->id_number);
  for (i=0; i < MAX_THINGS_IN_HAND; i++)
    dungeon->things_in_hand[i] = 0;
}

void process_things_in_dungeon_hand(void)
{
  _DK_process_things_in_dungeon_hand();
}

void draw_mini_things_in_hand(long x, long y)
{
  _DK_draw_mini_things_in_hand(x, y);
}

struct Thing *create_power_hand(PlayerNumber owner)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    struct Thing *grabtng;
    struct Coord3d pos;
    //_DK_create_power_hand(owner);
    pos.x.val = 0;
    pos.y.val = 0;
    pos.z.val = 0;
    thing = create_object(&pos, 37, owner, -1);
    if (thing_is_invalid(thing))
        return NULL;
    player = get_player(owner);
    player->hand_thing_idx = thing->index;
    player->field_C = 0;
    grabtng = get_first_thing_in_power_hand(player);
    if (thing_is_invalid(thing))
    {
      set_power_hand_graphic(owner, 782, 256);
    } else
    if ((grabtng->class_id == TCls_Object) && object_is_gold_pile(grabtng))
    {
        set_power_hand_graphic(owner, 781, 256);
    } else
    {
        set_power_hand_graphic(owner, 784, 256);
    }
    place_thing_in_limbo(thing);
    return thing;
}

void delete_power_hand(PlayerNumber owner)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    long hand_idx;
    player = get_player(owner);
    hand_idx = player->hand_thing_idx;
    if (hand_idx == 0)
        return;
    player->hand_thing_idx = 0;
    thing = thing_get(hand_idx);
    delete_thing_structure(thing, 0);
}

long prepare_thing_for_power_hand(unsigned short tng_idx, long plyr_idx)
{
    return _DK_prepare_thing_for_power_hand(tng_idx, plyr_idx);
}

void add_creature_to_sacrifice_list(long plyr_idx, long model, long explevel)
{
  struct Dungeon *dungeon;
  SYNCDBG(6,"Player %ld sacrificed creture model %ld exp level %d",plyr_idx,model,explevel);
  if ((plyr_idx < 0) || (plyr_idx >= DUNGEONS_COUNT))
  {
    ERRORLOG("How can this player sacrifice a creature?");
    return;
  }
  if ((model < 0) || (model >= CREATURE_TYPES_COUNT))
  {
    ERRORLOG("Tried to sacrifice invalid creature model.");
    return;
  }
  dungeon = get_dungeon(plyr_idx);
  dungeon->creature_sacrifice[model]++;
  dungeon->creature_sacrifice_exp[model] += explevel+1;
  dungeon->lvstats.creatures_sacrificed++;
}

TbBool magic_use_power_hand(long plyr_idx, unsigned short a2, unsigned short a3, unsigned short tng_idx)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Thing *thing;
  //return _DK_magic_use_power_hand(plyr_idx, a2, a3, tng_idx);
  dungeon = get_dungeon(plyr_idx);
  player = get_player(plyr_idx);
  if (dungeon->field_63 >= 8)
    return false;
  thing = thing_get(tng_idx);
  if (thing_is_invalid(thing))
  {
    thing = NULL;
  } else
  if (!can_thing_be_picked_up_by_player(thing, plyr_idx))
  {
      thing = NULL;
  }
  if (thing == NULL)
  {
      if (player->thing_under_hand > 0)
          thing = thing_get(player->thing_under_hand);
  }
  if (thing_is_invalid(thing))
      return false;
  if (!can_thing_be_picked_up_by_player(thing, plyr_idx))
  {
      return false;
  }
  if (thing->class_id != TCls_Object)
  {
      prepare_thing_for_power_hand(thing->index, plyr_idx);
      return true;
  }
  if (is_dungeon_special(thing))
  {
      activate_dungeon_special(thing, player);
      return false;
  }
  if ( object_is_pickable_by_hand(thing, plyr_idx) )
  {
      prepare_thing_for_power_hand(thing->index, plyr_idx);
      return true;
  }
  return false;
}


/******************************************************************************/
#ifdef __cplusplus
}
#endif
