/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_data.c
 *     Rooms support functions.
 * @par Purpose:
 *     Functions to create and maintain the game rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     17 Apr 2009 - 14 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_data.h"
#include "globals.h"

#include "bflib_basics.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
RoomKind look_through_rooms[] = {
    RoK_DUNGHEART, RoK_TREASURE, RoK_LAIR,      RoK_GARDEN,
    RoK_LIBRARY,   RoK_TRAINING, RoK_WORKSHOP,  RoK_SCAVENGER,
    RoK_PRISON,    RoK_TEMPLE,   RoK_TORTURE,   RoK_GRAVEYARD,
    RoK_BARRACKS,  RoK_BRIDGE,   RoK_GUARDPOST, RoK_ENTRANCE,
    RoK_DUNGHEART, RoK_UNKN17,};

struct RoomInfo room_info[] = {
  { 0,  0,  0},
  { 0,  0,  0},
  {29, 57,  0},
  {33, 61,  0},
  {37, 65,  0},
  {35, 63,  0},
  {39, 67, 85},
  { 0,  0,  0},
  {47, 75, 86},
  {49, 77,156},
  {45, 73,155},
  {43, 71, 45},
  {41, 69,  0},
  {31, 59,  0},
  {51, 79,  0},
  {53, 81,  0},
  {55, 83,  0},
};

/******************************************************************************/
DLLIMPORT void _DK_set_room_capacity(struct Room *room, long capac);
DLLIMPORT void _DK_set_room_efficiency(struct Room *room);
DLLIMPORT struct Room *_DK_link_adjacent_rooms_of_type(unsigned char owner, long x, long y, unsigned char rkind);
DLLIMPORT struct Room *_DK_create_room(unsigned char a1, unsigned char a2, unsigned short a3, unsigned short a4);
DLLIMPORT void _DK_create_room_flag(struct Room *room);
DLLIMPORT struct Room *_DK_allocate_free_room_structure(void);
DLLIMPORT unsigned short _DK_i_can_allocate_free_room_structure(void);

/******************************************************************************/
struct Room *room_get(long room_idx)
{
  if ((room_idx < 1) || (room_idx > ROOMS_COUNT))
    return &game.rooms[0];
  return &game.rooms[room_idx];
}

struct Room *subtile_room_get(long stl_x, long stl_y)
{
  struct SlabMap *slb;
  slb = get_slabmap_for_subtile(stl_x,stl_y);
  if (slabmap_block_invalid(slb))
    return &game.rooms[0];
  return room_get(slb->room_index);
}

TbBool room_is_invalid(const struct Room *room)
{
  return (room <= &game.rooms[0]) || (room == NULL);
}

struct RoomData *room_data_get_for_kind(long room_kind)
{
  if ((room_kind < 1) || (room_kind > ROOM_TYPES_COUNT))
    return &room_data[0];
  return &room_data[room_kind];
}

struct RoomData *room_data_get_for_room(const struct Room *room)
{
  if ((room->kind < 1) || (room->kind > ROOM_TYPES_COUNT))
    return &room_data[0];
  return &room_data[room->kind];
}

long get_room_look_through(RoomKind rkind)
{
  const long arr_length = sizeof(look_through_rooms)/sizeof(look_through_rooms[0]);
  long i;
  for (i=0; i < arr_length; i++)
  {
    if (look_through_rooms[i] == rkind)
      return i;
  }
  return -1;
}

long get_room_slabs_count(long plyr_idx, unsigned short rkind)
{
  static const char *func_name="get_room_slabs_count";
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned long k;
  long i,n;
  int count;
  dungeon = &(game.dungeon[plyr_idx%DUNGEONS_COUNT]);
  count = 0;
  i = dungeon->room_kind[rkind];
  k = 0;
  while (i>0)
  {
    if (i >= ROOMS_COUNT)
    {
      error(func_name,1953,"Jump out of rooms array bounds detected");
      break;
    }
    room = &game.rooms[i];
    if (room_is_invalid(room))
      break;
    i = room->field_6;
    count += room->field_3B;
    k++;
    if (k > ROOMS_COUNT)
    {
      error(func_name,7641,"Infinite loop detected when sweeping rooms list");
      break;
    }
  }
  return count;

}

void set_room_capacity(struct Room *room, long capac)
{
  _DK_set_room_capacity(room, capac);
}

void set_room_efficiency(struct Room *room)
{
  _DK_set_room_efficiency(room);
}

void delete_room_structure(struct Room *room)
{
  //_DK_delete_room_structure(room); return;
  struct Dungeon *dungeon;
  unsigned short *wptr;
  if (room == NULL)
    return;
  if (room->field_0 & 0x01)
  {
    if (game.field_14E497 != room->owner)
    {
        dungeon = &(game.dungeon[room->owner%DUNGEONS_COUNT]);
        wptr = &dungeon->room_kind[room->kind];
        if (room->index == *wptr)
        {
          *wptr = room->field_6;
          game.rooms[room->field_6].field_4 = 0;
        }
        else
        {
          game.rooms[room->field_6].field_4 = room->field_4;
          game.rooms[room->field_4].field_6 = room->field_6;
        }
    }
    memset(room, 0, sizeof(struct Room));
  }
}

void delete_all_room_structures(void)
{
  struct Room *room;
  long i;
  for (i=1; i < ROOMS_COUNT; i++)
  {
    room = &game.rooms[i];
    delete_room_structure(room);
  }
}

struct Room *link_adjacent_rooms_of_type(unsigned char owner, long x, long y, unsigned char rkind)
{
  static const char *func_name="link_adjacent_rooms_of_type";
  // TODO: rework! may lead to hang on map borders
  return _DK_link_adjacent_rooms_of_type(owner, x, y, rkind);
}

struct Room *create_room(unsigned char owner, unsigned char rkind, unsigned short x, unsigned short y)
{
  static const char *func_name="create_room";
  struct Dungeon *dungeon;
  struct MapOffset *sstep;
  struct SlabMap *slb;
  struct Room *room;
  unsigned long tot_x,tot_y;
  unsigned long cx,cy;
  long slb_x,slb_y;
  long i;
  //room = _DK_create_room(owner, rkind, x, y); return room;
  room = link_adjacent_rooms_of_type(owner, x, y, rkind);
  if (room != NULL)
  {
    room->field_3B = 0;
    i = room->field_37;
    while (i > 0)
    {
      room->field_3B++;
      slb_x = i % map_tiles_x;
      slb_y = i / map_tiles_x;
      if (slb_y >= map_tiles_y)
      {
        LbErrorLog("Index out of range when sweeping Slabs in %s.\n",func_name);
        break;
      }
      slb = get_slabmap_block(slb_x,slb_y);
      slb->room_index = room->index;
      i = slb->field_1;
    }
  } else
  {
    if ( !i_can_allocate_free_room_structure() )
      return NULL;
    room = allocate_free_room_structure();
    room->owner = owner;
    room->kind = rkind;
    if (rkind == 1)
    {
      if ((game.field_14E938 > 0) && (game.field_14E938 < ROOMS_COUNT))
      {
        room->field_19 = game.field_14E938;
        game.rooms[game.field_14E938].field_17 = room->index;
      }
      game.field_14E938 = room->index;
      game.field_14E93A++;
    }
    if (owner != game.field_14E497)
    {
      dungeon = &(game.dungeon[owner%DUNGEONS_COUNT]);
      i = dungeon->room_kind[room->kind%ROOM_TYPES_COUNT];
      room->field_6 = i;
      game.rooms[i].field_4 = room->index;
      dungeon->room_kind[room->kind%ROOM_TYPES_COUNT] = room->index;
      dungeon->room_slabs_count[room->kind%ROOM_TYPES_COUNT]++;
    }
    slb_x = map_to_slab[x%(map_subtiles_x+1)];
    slb_y = map_to_slab[y%(map_subtiles_y+1)];
    i = map_tiles_x*slb_y + slb_x;
    room->field_37 = i;
    room->field_39 = i;
    slb = &game.slabmap[i];
    slb->field_1 = 0;
    room->field_3B = 0;
    i = room->field_37;
    while (i > 0)
    {
      room->field_3B++;
      slb_x = i % map_tiles_x;
      slb_y = (i / map_tiles_x);
      if (slb_y >= map_tiles_y)
      {
        LbErrorLog("Index out of range when sweeping Slabs in %s.\n",func_name);
        break;
      }
      slb = get_slabmap_block(slb_x,slb_y);
      slb->room_index = room->index;
      i = slb->field_1;
    }
    create_room_flag(room);
  }
  tot_x = 0;
  tot_y = 0;
  i = room->field_37;
  while (i > 0)
  {
      slb_x = (i % map_tiles_x);
      slb_y = (i / map_tiles_x);
      if (slb_y >= map_tiles_y)
      {
        LbErrorLog("Index out of range when sweeping Slabs in %s.\n",func_name);
        break;
      }
      slb = get_slabmap_block(slb_x,slb_y);
      tot_x += slb_x;
      tot_y += slb_y;
      i = slb->field_1;
  }
  if (room->field_3B > 1)
  {
    tot_x /= room->field_3B;
    tot_y /= room->field_3B;
  }
  for (i=0; i < 256; i++)
  {
    sstep = &spiral_step[i];
    cx = 3 * (tot_x + sstep->h) + 1;
    cy = 3 * (tot_y + sstep->v) + 1;
    slb_x = map_to_slab[cx%(map_subtiles_x+1)];
    slb_y = map_to_slab[cy%(map_subtiles_y+1)];
    slb = get_slabmap_block(slb_x,slb_y);
    if (&game.rooms[slb->room_index] == room)
    {
      room->field_8 = cx;
      room->field_9 = cy;
      break;
    }
  }
  return room;
}

void create_room_flag(struct Room *room)
{
  static const char *func_name="create_room_flag";
  struct Thing *thing;
  struct Coord3d pos;
  long x,y;
  //_DK_create_room_flag(room);
  x = 3 * (room->field_37 % map_tiles_x) + 1;
  y = 3 * (room->field_37 / map_tiles_x) + 1;
  if ((room->kind != 7) && (room->kind != 1) && (room->kind != 16) && (room->kind != 15))
  {
    pos.z.val = 512;
    pos.x.val = x << 8;
    pos.y.val = y << 8;
    thing = find_base_thing_on_mapwho(1, 25, x, y);
    if (thing == NULL)
    {
      thing = create_object(&pos, 25, room->owner, -1);
    }
    if (thing == NULL)
    {
      error(func_name, 5541, "Cannot create room_flag");
      return;
    }
    thing->word_13.w0 = room->index;
  }
}

struct Room *allocate_free_room_structure(void)
{
  return _DK_allocate_free_room_structure();
}

unsigned short i_can_allocate_free_room_structure(void)
{
  return _DK_i_can_allocate_free_room_structure();
}

void reinitialise_treaure_rooms(void)
{
  static const char *func_name="reinitialise_treaure_rooms";
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned int i,k,n;
  for (n=0; n < DUNGEONS_COUNT; n++)
  {
    dungeon = &(game.dungeon[n]);
    i = dungeon->room_kind[RoK_TREASURE];
    k = 0;
    while (i > 0)
    {
      if (i > ROOMS_COUNT)
      {
        error(func_name,478,"Jump out of rooms array detected");
        break;
      }
      room = &game.rooms[i];
      if (room_is_invalid(room))
        break;
      i = room->field_6;
      set_room_capacity(room, 1);
      k++;
      if (k > ROOMS_COUNT)
      {
        error(func_name,479,"Infinite loop detected when sweeping rooms list");
        break;
      }
    }
  }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
