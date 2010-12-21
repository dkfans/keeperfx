/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_terrain.c
 *     Slabs and rooms configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for terrain elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config_terrain.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_terrain_file[]="terrain.cfg";

const struct NamedCommand terrain_common_commands[] = {
  {"SLABSCOUNT",      1},
  {"ROOMSCOUNT",      2},
  {NULL,              0},
  };

const struct NamedCommand terrain_slab_commands[] = {
  {"NAME",            1},
  {NULL,              0},
  };

const struct NamedCommand terrain_room_commands[] = {
  {"NAME",            1},
  {"COST",            2},
  {"HEALTH",          3},
  {NULL,              0},
  };

const struct NamedCommand terrain_health_commands[] = {
  {"DIRT",            1},
  {"GOLD",            2},
  {"PRETTY",          3},
  {"FLOOR",           4},
  {"ROOM",            5},
  {"DOOR_WOODEN",     6},
  {"DOOR_BRACE",      7},
  {"DOOR_STEEL",      8},
  {"DOOR_MAGIC",      9},
  {NULL,              0},
  };

/******************************************************************************/
struct SlabsConfig slab_conf;
struct NamedCommand slab_desc[TERRAIN_ITEMS_MAX];
struct NamedCommand room_desc[TERRAIN_ITEMS_MAX];

//TODO identify all slab attributes and store them in config file
struct SlabAttr slab_attrs[] = {
  {324, 4, 0, 48, 0, 0, 0,  0, 1, 0, 0, 0, 0}, // [0]
  {326, 4, 1, 17, 0, 0, 0,  0, 1, 0, 0, 1, 0},
  {325, 4, 0, 24, 0, 0, 1,  0, 1, 0, 0, 1, 0},
  {325, 4, 0, 24, 0, 0, 1,  0, 1, 0, 0, 1, 0},
  {329, 4, 2, 48, 0, 0, 3,  0, 1, 0, 0, 1, 0},
  {329, 4, 2, 48, 0, 0, 3,  0, 1, 0, 0, 1, 0},
  {329, 4, 2, 48, 0, 0, 3,  0, 1, 0, 0, 1, 0},
  {329, 4, 2, 48, 0, 0, 3,  0, 1, 0, 0, 1, 0},
  {329, 4, 2, 48, 0, 0, 3,  0, 1, 0, 0, 1, 0},
  {330, 4, 2, 24, 0, 0, 3,  0, 1, 0, 0, 1, 0},
  {331, 0, 2,  0, 0, 0, 0,  1, 1, 1, 0, 0, 0}, // [10]
  {332, 0, 3,  0, 0, 0, 2,  2, 1, 1, 0, 0, 0},
  {327, 0, 2,  0, 0, 1, 0,  3, 1, 0, 0, 0, 1},
  {328, 0, 2,  0, 0, 2, 0,  4, 1, 1, 0, 0, 2},
  {201, 4, 4, 18, 2, 0, 4,  5, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3,  5, 1, 0, 0, 1, 0}, // [15]
  {201, 4, 4, 18, 2, 0, 4,  6, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3,  6, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4,  7, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3,  7, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4,  8, 1, 1, 0, 0, 0}, // [20]
  {201, 4, 2, 48, 0, 0, 3,  8, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4,  9, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3,  9, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4, 10, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3, 10, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4, 11, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3, 11, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4, 12, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3, 12, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4, 13, 1, 1, 0, 0, 0}, // [30]
  {201, 4, 2, 48, 0, 0, 3, 13, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4, 14, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3, 14, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4, 15, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3, 15, 1, 0, 0, 1, 0}, // [35]
  {201, 4, 4, 18, 2, 0, 4, 16, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3, 16, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4, 17, 1, 1, 0, 0, 0},
  {201, 4, 2, 48, 0, 0, 3, 17, 1, 0, 0, 1, 0},
  {201, 4, 4, 18, 2, 0, 4, 18, 1, 1, 0, 0, 0}, // [40]
  {201, 4, 2, 48, 0, 0, 3, 18, 1, 0, 0, 1, 0},
  {590, 4, 5, 80, 0, 0, 5,  2, 1, 1, 1, 0, 0},
  {590, 4, 5, 80, 0, 0, 5,  2, 1, 1, 1, 0, 0},
  {591, 4, 6, 80, 0, 0, 5,  2, 1, 1, 1, 0, 0},
  {591, 4, 6, 80, 0, 0, 5,  2, 1, 1, 1, 0, 0}, // [45]
  {592, 4, 7, 80, 0, 0, 5,  2, 1, 1, 1, 0, 0},
  {592, 4, 7, 80, 0, 0, 5,  2, 1, 1, 1, 0, 0},
  {593, 4, 8, 80, 0, 0, 5,  2, 1, 1, 1, 0, 0},
  {593, 4, 8, 80, 0, 0, 5,  2, 1, 1, 1, 0, 0},
  {201, 4, 2,  0, 0, 0, 5,  0, 1, 0, 1, 0, 0}, // [50]
  {201, 4, 4, 18, 2, 0, 5,  1, 1, 1, 1, 0, 3},
  {544, 4, 1, 17, 0, 0, 5,  0, 1, 0, 1, 1, 0},
  {201, 4, 4, 18, 2, 0, 5,  1, 1, 1, 1, 0, 0},
  {201, 4, 2, 48, 0, 0, 5,  0, 1, 0, 1, 0, 0},
  {201, 4, 2, 48, 0, 0, 5,  0, 1, 0, 1, 0, 0}, // [55]
  {201, 4, 2, 48, 0, 0, 5,  0, 1, 0, 1, 0, 0},
  {201, 4, 2, 48, 0, 0, 5,  0, 1, 0, 1, 0, 0},
};
/******************************************************************************/
struct SlabAttr *get_slab_kind_attrs(long slab_kind)
{
    if ((slab_kind < 0) || (slab_kind >= sizeof(slab_attrs)/sizeof(slab_attrs[0])))
        return &slab_attrs[0];
    return &slab_attrs[slab_kind];
}

struct SlabAttr *get_slab_attrs(struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return &slab_attrs[0];
    return get_slab_kind_attrs(slb->slab);
}

TbBool parse_terrain_common_blocks(char *buf,long len,const char *config_textname)
{
  long pos;
  int k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize block data
  slab_conf.slab_types_count = 1;
  slab_conf.room_types_count = 1;
  // Find the block
  sprintf(block_buf,"common");
  pos = 0;
  k = find_conf_block(buf,&pos,len,block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
    return false;
  }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(terrain_common_commands,cmd_num)
  while (pos<len)
  {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_common_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // SLABSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= TERRAIN_ITEMS_MAX))
            {
              slab_conf.slab_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 2: // ROOMSCOUNT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if ((k > 0) && (k <= TERRAIN_ITEMS_MAX))
            {
              slab_conf.room_types_count = k;
              n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
  }
#undef COMMAND_TEXT
  return true;
}

TbBool parse_terrain_slab_blocks(char *buf,long len,const char *config_textname)
{
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the array
  int arr_size = sizeof(slab_conf.slab_names)/sizeof(slab_conf.slab_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(slab_conf.slab_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < slab_conf.slab_types_count)
    {
      slab_desc[i].name = slab_conf.slab_names[i].text;
      slab_desc[i].num = i;
    } else
    {
      slab_desc[i].name = NULL;
      slab_desc[i].num = 0;
    }
  }
  arr_size = slab_conf.slab_types_count;
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"slab%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      continue;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(terrain_slab_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_slab_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,slab_conf.slab_names[i].text,COMMAND_WORD_LEN) <= 0)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
  }
  // Block health - will be later integrated with slab blocks
    sprintf(block_buf,"block_health");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
    } else
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(terrain_health_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_health_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7:
      case 8:
      case 9:
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            game.block_health[cmd_num-1] = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
  return true;
}

TbBool parse_terrain_room_blocks(char *buf,long len,const char *config_textname)
{
  struct RoomStats *rstat;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[COMMAND_WORD_LEN];
  char word_buf[COMMAND_WORD_LEN];
  // Initialize the rooms array
  int arr_size = sizeof(slab_conf.room_names)/sizeof(slab_conf.room_names[0]);
  for (i=0; i < arr_size; i++)
  {
    LbMemorySet(slab_conf.room_names[i].text, 0, COMMAND_WORD_LEN);
    if (i < slab_conf.room_types_count)
    {
      room_desc[i].name = slab_conf.room_names[i].text;
      room_desc[i].num = i;
    } else
    {
      room_desc[i].name = NULL;
      room_desc[i].num = 0;
    }
  }
  arr_size = slab_conf.room_types_count;
  for (i=0; i < arr_size; i++)
  {
    rstat = &game.room_stats[i];
    rstat->cost = 0;
    rstat->health = 0;
  }
  // Load the file
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"room%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
      continue;
    }
    rstat = &game.room_stats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(terrain_room_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,terrain_room_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,slab_conf.room_names[i].text,COMMAND_WORD_LEN) > 0)
          {
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 2: // COST
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            rstat->cost = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 3: // HEALTH
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            rstat->health = k;
            n++;
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                COMMAND_TEXT(cmd_num),block_buf,config_textname);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%s] block of %s file.",
              cmd_num,block_buf,config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
  }
  return true;
}

TbBool load_terrain_config(const char *conf_fname,unsigned short flags)
{
  static const char config_textname[] = "Terrain config";
  char *fname;
  char *buf;
  long len;
  TbBool result;
  SYNCDBG(0,"Reading %s file \"%s\".",config_textname,conf_fname);
  fname = prepare_file_path(FGrp_FxData,conf_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("%s file \"%s\" doesn't exist or is too small.",config_textname,conf_fname);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("%s file \"%s\" is too large.",config_textname,conf_fname);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  result = (len > 0);
  if (result)
  {
    result = parse_terrain_common_blocks(buf, len, config_textname);
    if (!result)
      WARNMSG("Parsing %s file \"%s\" common blocks failed.",config_textname,conf_fname);
  }
  if (result)
  {
    result = parse_terrain_slab_blocks(buf, len, config_textname);
    if (!result)
      WARNMSG("Parsing %s file \"%s\" slab blocks failed.",config_textname,conf_fname);
  }
  if (result)
  {
    result = parse_terrain_room_blocks(buf, len, config_textname);
    if (!result)
      WARNMSG("Parsing %s file \"%s\" room blocks failed.",config_textname,conf_fname);
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return result;
}

/**
 * Zeroes all the costs for all rooms.
 */
TbBool make_all_rooms_free(void)
{
  struct RoomStats *rstat;
  long i;
  for (i=0; i < slab_conf.room_types_count; i++)
  {
    rstat = &game.room_stats[i];
    rstat->cost = 0;
  }
  return true;
}

/**
 * Makes all rooms to be available to research for the player.
 */
TbBool make_all_rooms_researchable(long plyr_idx)
{
  struct Dungeon *dungeon;
  long i;
  dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
      return false;
  for (i=0; i < slab_conf.room_types_count; i++)
  {
    dungeon->room_resrchable[i] = 1;
  }
  return true;
}

/**
 * Sets room availability state.
 */
TbBool set_room_available(long plyr_idx, long room_idx, long resrch, long avail)
{
  struct Dungeon *dungeon;
  // note that we can't get_players_num_dungeon() because players
  // may be uninitialized yet when this is called.
  dungeon = get_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
      return false;
  if ((room_idx < 0) || (room_idx >= ROOM_TYPES_COUNT))
  {
    ERRORLOG("Can't add incorrect room %ld to player %ld",room_idx, plyr_idx);
    return false;
  }
  dungeon->room_resrchable[room_idx] = resrch;
  if (resrch != 0)
    dungeon->room_buildable[room_idx] = avail;
  else
    dungeon->room_buildable[room_idx] = 0;
  return true;
}

/**
 * Returns if the room can be built by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if the player has enough money or map position is on correct spot.
 */
TbBool is_room_available(long plyr_idx, long room_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon))
        return false;
    // Player must have dungeon heart to build rooms
    if (dungeon->dnheart_idx <= 0)
    {
        return false;
    }
    if ((room_idx < 0) || (room_idx >= ROOM_TYPES_COUNT))
    {
      ERRORLOG("Incorrect room %ld (player %ld)",room_idx, plyr_idx);
      return false;
    }
    if (dungeon->room_buildable[room_idx])
      return true;
    return false;
}

/*
 * Makes all the rooms, which are researchable, to be instantly available.
 */
TbBool make_available_all_researchable_rooms(long plyr_idx)
{
  struct Dungeon *dungeon;
  long i;
  SYNCDBG(0,"Starting");
  dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon))
      return false;
  for (i=0; i < ROOM_TYPES_COUNT; i++)
  {
    if (dungeon->room_resrchable[i])
    {
      dungeon->room_buildable[i] = 1;
    }
  }
  return true;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
