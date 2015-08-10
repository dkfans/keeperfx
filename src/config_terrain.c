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
#include "config_strings.h"
#include "config_creature.h"
#include "game_legacy.h"

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
  {"TOOLTIPTEXTID",   2},
  {NULL,              0},
};

const struct NamedCommand terrain_room_commands[] = {
  {"NAME",            1},
  {"COST",            2},
  {"HEALTH",          3},
  {"PROPERTIES",      4},
  {"SLABASSIGN",      5},
  {"CREATURECREATION",6},
  {"MESSAGES",        7},
  {"NAMETEXTID",      8},
  {"TOOLTIPTEXTID",   9},
  {"SYMBOLSPRITES",  10},
  {"POINTERSPRITES", 11},
  {"PANELTABINDEX",  12},
  {"TOTALCAPACITY",  13},
  {"USEDCAPACITY",   14},
  {"AMBIENTSNDSAMPLE",15},
  {NULL,              0},
};

const struct NamedCommand  terrain_room_properties_commands[] = {
  {"HAS_NO_ENSIGN",     1},
  {"CANNOT_VANDALIZE",  2},
  {"BUILD_TO_BROKE",    3},
  {NULL,                0},
};

/* Room capacity computation, using functions from room_data.c */

extern void count_slabs_all_only(struct Room *room);
extern void count_slabs_all_wth_effcncy(struct Room *room);
extern void count_slabs_div2_wth_effcncy(struct Room *room);
extern void count_gold_slabs_wth_effcncy(struct Room *room);

const struct NamedCommand terrain_room_total_capacity_func_type[] = {
  {"slabs_all_only",          1},
  {"slabs_all_wth_effcncy",   2},
  {"slabs_div2_wth_effcncy",  3},
  {"gold_slabs_wth_effcncy",  4},
  {"none",                    5},
  {NULL,                      0},
};

Room_Update_Func terrain_room_total_capacity_func_list[] = {
  NULL,
  count_slabs_all_only,
  count_slabs_all_wth_effcncy,
  count_slabs_div2_wth_effcncy,
  count_gold_slabs_wth_effcncy,
  NULL,
  NULL,
};

/* Room usage computation, using functions from room_data.c */

extern void count_gold_hoardes_in_room(struct Room *room);
extern void count_books_in_room(struct Room *room);
extern void count_workers_in_room(struct Room *room);
extern void count_crates_in_room(struct Room *room);
extern void count_bodies_in_room(struct Room *room);
extern void count_food_in_room(struct Room *room);
extern void count_lair_occupants(struct Room *room);

const struct NamedCommand terrain_room_used_capacity_func_type[] = {
  {"gold_hoardes_in_room", 1},
  {"books_in_room",        2},
  {"workers_in_room",      3},
  {"crates_in_room",       4},
  {"bodies_in_room",       5},
  {"food_in_room",         6},
  {"lair_occupants",       7},
  {"none",                 8},
  {NULL,                   0},
};

Room_Update_Func terrain_room_used_capacity_func_list[] = {
  NULL,
  count_gold_hoardes_in_room,
  count_books_in_room,
  count_workers_in_room,
  count_crates_in_room,
  count_bodies_in_room,
  count_food_in_room,
  count_lair_occupants,
  NULL,
  NULL,
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

//TODO CONFIG identify all slab attributes and store them in config file
struct SlabAttr slab_attrs[] = {
  {0, 4, 0, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_Unclaimed,      0, 1, 0, 0, 0, 0}, // [0] HARD
  {0, 4, 1, SlbAtFlg_Blocking|SlbAtFlg_Valuable,SlbAtFlg_None,   0, SlbAtCtg_Unclaimed,      0, 1, 0, 0, 1, 0}, // [1] GOLD
  {0, 4, 0, SlbAtFlg_Blocking|SlbAtFlg_Digable, SlbAtFlg_None,   0, SlbAtCtg_FriableDirt,    0, 1, 0, 0, 1, 0}, // [2] DIRT
  {0, 4, 0, SlbAtFlg_Blocking|SlbAtFlg_Digable, SlbAtFlg_None,   0, SlbAtCtg_FriableDirt,    0, 1, 0, 0, 1, 0}, // [3] TORCH_DIRT
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  0, 1, 0, 0, 1, 0}, // [4] DRAPE_WALL
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  0, 1, 0, 0, 1, 0}, // [5] TORCH_WALL
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  0, 1, 0, 0, 1, 0}, // [6] TWINS_WALL
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  0, 1, 0, 0, 1, 0}, // [7] WOMAN_WALL
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  0, 1, 0, 0, 1, 0}, // [8] PAIR_WALL
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Digable, SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  0, 1, 0, 0, 1, 0}, // [9] DAMAGED_WALL
  {0, 0, 2, SlbAtFlg_None,                      SlbAtFlg_None,   0, SlbAtCtg_Unclaimed,      1, 1, 1, 0, 0, 0}, // [10] PATH
  {0, 0, 3, SlbAtFlg_None,                      SlbAtFlg_None,   0, SlbAtCtg_FortifiedGround,2, 1, 1, 0, 0, 0}, // [11]
  {0, 0, 2, SlbAtFlg_None,                      SlbAtFlg_None,   1, SlbAtCtg_Unclaimed,      3, 1, 0, 0, 0, 1}, // [12] LAVA
  {0, 0, 2, SlbAtFlg_None,                      SlbAtFlg_None,   2, SlbAtCtg_Unclaimed,      4, 1, 1, 0, 0, 2}, // [13] WATER
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,   5, 1, 1, 0, 0, 0}, // [14] ENTRANCE_ZONE
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  5, 1, 0, 0, 1, 0}, // [15] SLAB15
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,   6, 1, 1, 0, 0, 0}, // [16] TREASURY_AREA
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  6, 1, 0, 0, 1, 0}, // [17] SLAB17
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,   7, 1, 1, 0, 0, 0}, // [18]
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  7, 1, 0, 0, 1, 0}, // [19] SLAB19
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,   8, 1, 1, 0, 0, 0}, // [20]
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  8, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,   9, 1, 1, 0, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall,  9, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  10, 1, 1, 0, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 10, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  11, 1, 1, 0, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 11, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  12, 1, 1, 0, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 12, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  13, 1, 1, 0, 0, 0}, // [30]
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 13, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  14, 1, 1, 0, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 14, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  15, 1, 1, 0, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 15, 1, 0, 0, 1, 0}, // [35]
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  16, 1, 1, 0, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 16, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  17, 1, 1, 0, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 17, 1, 0, 0, 1, 0},
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_RoomInterior,  18, 1, 1, 0, 0, 0}, // [40] BARRACK_AREA
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_FortifiedWall, 18, 1, 0, 0, 1, 0}, // [41] SLAB41
  {0, 4, 5, SlbAtFlg_Blocking|SlbAtFlg_IsDoor,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       2, 1, 1, 1, 0, 0}, // [42] DOOR_WOODEN
  {0, 4, 5, SlbAtFlg_Blocking|SlbAtFlg_IsDoor,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       2, 1, 1, 1, 0, 0},
  {0, 4, 6, SlbAtFlg_Blocking|SlbAtFlg_IsDoor,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       2, 1, 1, 1, 0, 0},
  {0, 4, 6, SlbAtFlg_Blocking|SlbAtFlg_IsDoor,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       2, 1, 1, 1, 0, 0}, // [45] DOOR_BRACE2
  {0, 4, 7, SlbAtFlg_Blocking|SlbAtFlg_IsDoor,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       2, 1, 1, 1, 0, 0},
  {0, 4, 7, SlbAtFlg_Blocking|SlbAtFlg_IsDoor,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       2, 1, 1, 1, 0, 0},
  {0, 4, 8, SlbAtFlg_Blocking|SlbAtFlg_IsDoor,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       2, 1, 1, 1, 0, 0}, // [48] DOOR_MAGIC
  {0, 4, 8, SlbAtFlg_Blocking|SlbAtFlg_IsDoor,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       2, 1, 1, 1, 0, 0},
  {0, 4, 2, SlbAtFlg_None,                      SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       0, 1, 0, 1, 0, 0}, // [50]
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_Obstacle,       1, 1, 1, 1, 0, 3}, // [51] BRIDGE_FRAME
  {0, 4, 1, SlbAtFlg_Blocking|SlbAtFlg_Valuable,SlbAtFlg_None,   0, SlbAtCtg_Unclaimed,      0, 1, 0, 1, 1, 0}, // [52] GEMS
  {0, 4, 4, SlbAtFlg_Blocking|SlbAtFlg_IsRoom,  SlbAtFlg_IsRoom, 0, SlbAtCtg_Obstacle,       1, 1, 1, 1, 0, 0}, // [53] GUARD_AREA
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       0, 1, 0, 1, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       0, 1, 0, 1, 0, 0}, // [55]
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       0, 1, 0, 1, 0, 0},
  {0, 4, 2, SlbAtFlg_Blocking|SlbAtFlg_Filled,  SlbAtFlg_None,   0, SlbAtCtg_Obstacle,       0, 1, 0, 1, 0, 0},
};
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct SlabAttr *get_slab_kind_attrs(SlabKind slab_kind)
{
    if ((slab_kind < 0) || (slab_kind >= sizeof(slab_attrs)/sizeof(slab_attrs[0])))
        return &slab_attrs[0];
    return &slab_attrs[slab_kind];
}

struct SlabAttr *get_slab_attrs(const struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return &slab_attrs[0];
    return get_slab_kind_attrs(slb->kind);
}

struct SlabConfigStats *get_slab_kind_stats(SlabKind slab_kind)
{
    if ((slab_kind < 0) || (slab_kind >= slab_conf.slab_types_count))
        return &slab_conf.slab_cfgstats[0];
    return &slab_conf.slab_cfgstats[slab_kind];
}

struct SlabConfigStats *get_slab_stats(struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return &slab_conf.slab_cfgstats[0];
    return get_slab_kind_stats(slb->kind);
}

/**
 * Returns Code Name (name to use in script file) of given slab kind.
 */
const char *slab_code_name(SlabKind slbkind)
{
    const char *name;
    name = get_conf_parameter_text(slab_desc,slbkind);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

struct RoomConfigStats *get_room_kind_stats(RoomKind room_kind)
{
    if ((room_kind < 0) || (room_kind >= slab_conf.room_types_count))
        return &slab_conf.room_cfgstats[0];
    return &slab_conf.room_cfgstats[room_kind];
}

/**
 * Returns Code Name (name to use in script file) of given room kind.
 */
const char *room_code_name(RoomKind rkind)
{
    const char *name;
    name = get_conf_parameter_text(room_desc,rkind);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

TbBool parse_terrain_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        slab_conf.slab_types_count = 1;
        slab_conf.room_types_count = 1;
    }
    // Find the block
    sprintf(block_buf,"common");
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
        if ((flags & CnfLd_AcceptPartial) == 0)
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

TbBool parse_terrain_slab_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct SlabAttr *slbattr;
    struct SlabConfigStats *slabst;
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize the array
    int arr_size;
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(slab_conf.slab_cfgstats)/sizeof(slab_conf.slab_cfgstats[0]);
        for (i=0; i < arr_size; i++)
        {
            slabst = &slab_conf.slab_cfgstats[i];
            LbMemorySet(slabst->code_name, 0, COMMAND_WORD_LEN);
            slabst->tooltip_stridx = GUIStr_Empty;
            if (i < slab_conf.slab_types_count)
            {
                slab_desc[i].name = slabst->code_name;
                slab_desc[i].num = i;
            } else
            {
                slab_desc[i].name = NULL;
                slab_desc[i].num = 0;
            }
        }
        arr_size = sizeof(slab_attrs)/sizeof(slab_attrs[0]);
        for (i=0; i < arr_size; i++)
        {
            slbattr = get_slab_kind_attrs(i);
            slbattr->tooltip_stridx = GUIStr_Empty;
        }
    }
    // Parse every numbered block within range
    arr_size = slab_conf.slab_types_count;
    for (i=0; i < arr_size; i++)
    {
      sprintf(block_buf,"slab%d",i);
      pos = 0;
      k = find_conf_block(buf,&pos,len,block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0) {
              WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
              return false;
          }
          continue;
      }
      slbattr = get_slab_kind_attrs(i);
      slabst = &slab_conf.slab_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(terrain_slab_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,terrain_slab_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        if ((flags & CnfLd_ListOnly) != 0) {
            // In "List only" mode, accept only name command
            if (cmd_num > 1) {
                cmd_num = 0;
            }
        }
        n = 0;
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_single(buf,&pos,len,slabst->code_name,COMMAND_WORD_LEN) <= 0)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 2: // TOOLTIPTEXTID
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k > 0)
                {
                    slbattr->tooltip_stridx = k;
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
    }
    // Block health - will be later integrated with slab blocks
      sprintf(block_buf,"block_health");
      pos = 0;
      k = find_conf_block(buf,&pos,len,block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0)
              WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
          return false;
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

TbBool parse_terrain_room_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct RoomData *rdata;
    struct RoomStats *rstat;
    struct RoomConfigStats *roomst;
    long pos;
    int i,k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize the rooms array
    int arr_size;
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        arr_size = sizeof(slab_conf.room_cfgstats)/sizeof(slab_conf.room_cfgstats[0]);
        for (i=0; i < arr_size; i++)
        {
            roomst = &slab_conf.room_cfgstats[i];
            LbMemorySet(roomst->code_name, 0, COMMAND_WORD_LEN);
            roomst->name_stridx = GUIStr_Empty;
            roomst->tooltip_stridx = GUIStr_Empty;
            roomst->creature_creation_model = 0;
            roomst->bigsym_sprite_idx = 0;
            roomst->medsym_sprite_idx = 0;
            roomst->pointer_sprite_idx = 0;
            roomst->panel_tab_idx = 0;
            roomst->ambient_snd_smp_id = 0;
            roomst->msg_needed = 0;
            roomst->msg_too_small = 0;
            roomst->msg_no_route = 0;
            if (i < slab_conf.room_types_count)
            {
                room_desc[i].name = roomst->code_name;
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
    }
    // Parse every numbered block within range
    arr_size = slab_conf.room_types_count;
    for (i=0; i < arr_size; i++)
    {
      sprintf(block_buf,"room%d",i);
      pos = 0;
      k = find_conf_block(buf,&pos,len,block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0) {
              WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
              return false;
          }
          continue;
      }
      rstat = &game.room_stats[i];
      roomst = &slab_conf.room_cfgstats[i];
      rdata = room_data_get_for_kind(i);
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(terrain_room_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,terrain_room_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        if ((flags & CnfLd_ListOnly) != 0) {
            // In "List only" mode, accept only name command
            if (cmd_num > 1) {
                cmd_num = 0;
            }
        }
        n = 0;
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_single(buf,&pos,len,roomst->code_name,COMMAND_WORD_LEN) > 0)
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
        case 4: // PROPERTIES
            roomst->flags = RoCFlg_None;
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = get_id(terrain_room_properties_commands, word_buf);
                switch (k)
                {
                case 1: // HAS_NO_ENSIGN
                    roomst->flags |= RoCFlg_NoEnsign;
                    n++;
                    break;
                case 2: // CANNOT_VANDALIZE
                    roomst->flags |= RoCFlg_CantVandalize;
                    n++;
                    break;
                case 3: // BUILD_TO_BROKE
                    roomst->flags |= RoCFlg_BuildToBroke;
                    n++;
                    break;
                default:
                    CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                        COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                    break;
                }
            }
            break;
        case 5: // SLABASSIGN
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(slab_desc, word_buf);
              if (k >= 0)
              {
                  roomst->assigned_slab = k;
                  rdata->assigned_slab = k;
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // CREATURECREATION
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_id(creature_desc, word_buf);
              if (k >= 0)
              {
                  roomst->creature_creation_model = k;
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 7: // MESSAGES
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                roomst->msg_needed = k;
                n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                roomst->msg_too_small = k;
                n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                roomst->msg_no_route = k;
                n++;
            }
            if (n < 3)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 8: // NAMETEXTID
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                  roomst->name_stridx = k;
                  rdata->name_stridx = k;
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 9: // TOOLTIPTEXTID
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                  roomst->tooltip_stridx = k;
                  rdata->tooltip_stridx = k;
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 10: // SYMBOLSPRITES
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k >= 0)
                {
                    roomst->bigsym_sprite_idx = k;
                    n++;
                }
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k >= 0)
                {
                    roomst->medsym_sprite_idx = k;
                    rdata->medsym_sprite_idx = k;
                    n++;
                }
            }
            if (n < 2)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 11: // POINTERSPRITES
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                if (k >= 0)
                {
                    roomst->pointer_sprite_idx = k;
                    n++;
                }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 12: // PANELTABINDEX
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  roomst->panel_tab_idx = k;
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 13: // TOTALCAPACITY
            k = recognize_conf_parameter(buf,&pos,len,terrain_room_total_capacity_func_type);
            if (k > 0)
            {
                rdata->update_total_capacity = terrain_room_total_capacity_func_list[k];
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 14: // USEDCAPACITY
            k = recognize_conf_parameter(buf,&pos,len,terrain_room_used_capacity_func_type);
            if (k > 0)
            {
                rdata->update_storage_in_room = terrain_room_used_capacity_func_list[k];
                n++;
            }
            k = recognize_conf_parameter(buf,&pos,len,terrain_room_used_capacity_func_type);
            if (k > 0)
            {
                rdata->update_workers_in_room = terrain_room_used_capacity_func_list[k];
                n++;
            }
            if (n < 2)
            {
                CONFWRNLOG("Couldn't recognize all of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 15: // AMBIENTSNDSAMPLE
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  roomst->ambient_snd_smp_id = k;
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
    }
    return true;
}

TbBool load_terrain_config_file(const char *textname, const char *fname, unsigned short flags)
{
    char *buf;
    long len;
    TbBool result;
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    if (len > MAX_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" is too large.",textname,fname);
        return false;
    }
    buf = (char *)LbMemoryAlloc(len+256);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    result = (len > 0);
    // Parse blocks of the config file
    if (result)
    {
        result = parse_terrain_common_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" common blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_terrain_slab_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" slab blocks failed.",textname,fname);
    }
    if (result)
    {
        result = parse_terrain_room_blocks(buf, len, textname, flags);
        if ((flags & CnfLd_AcceptPartial) != 0)
            result = true;
        if (!result)
            WARNMSG("Parsing %s file \"%s\" room blocks failed.",textname,fname);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    return result;
}

TbBool load_terrain_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global terrain config";
    static const char config_campgn_textname[] = "campaign terrain config";
    char *fname;
    TbBool result;
    fname = prepare_file_path(FGrp_FxData,conf_fname);
    result = load_terrain_config_file(config_global_textname,fname,flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_terrain_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Zeroes all the costs for all rooms.
 */
TbBool make_all_rooms_free(void)
{
    struct RoomStats *rstat;
    long rkind;
    for (rkind=0; rkind < slab_conf.room_types_count; rkind++)
    {
        rstat = &game.room_stats[rkind];
        rstat->cost = 0;
    }
    return true;
}

/**
 * Makes all rooms to be available to research for the player.
 */
TbBool make_all_rooms_researchable(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    long rkind;
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot do; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    for (rkind=0; rkind < slab_conf.room_types_count; rkind++)
    {
        dungeon->room_resrchable[rkind] = 1;
    }
    return true;
}

/**
 * Sets room availability state.
 */
TbBool set_room_available(PlayerNumber plyr_idx, RoomKind room_idx, long resrch, long avail)
{
    struct Dungeon *dungeon;
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot do; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    if ((room_idx < 0) || (room_idx >= ROOM_TYPES_COUNT))
    {
        ERRORLOG("Can't add incorrect room %d to player %d",(int)room_idx, (int)plyr_idx);
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
TbBool is_room_available(PlayerNumber plyr_idx, RoomKind room_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build rooms
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((room_idx < 0) || (room_idx >= ROOM_TYPES_COUNT))
    {
      ERRORLOG("Incorrect room %d (player %d)",(int)room_idx, (int)plyr_idx);
      return false;
    }
    if (dungeon->room_buildable[room_idx]) {
        return true;
    }
    return false;
}

/**
 * Makes all the rooms, which are researchable, to be instantly available.
 */
TbBool make_available_all_researchable_rooms(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    long i;
    SYNCDBG(0,"Starting");
    dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot do; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    for (i=0; i < ROOM_TYPES_COUNT; i++)
    {
        if (dungeon->room_resrchable[i])
        {
            dungeon->room_buildable[i] = 1;
        }
    }
    return true;
}

/**
 * Returns if given slab kind is indestructible - cannot be damaged by digging nor anything else.
 * @param slbkind The slab kind to be checked.
 * @return True if the slab cannot be damaged, false otherwise.
 * @note Being indestructible does not necessarily means the slab cannot be marked for digging, or just sold.
 */
TbBool slab_kind_is_indestructible(RoomKind slbkind)
{
    return (slbkind == SlbT_ROCK) || (slbkind == SlbT_GEMS) || (slbkind == SlbT_ENTRANCE);
}

/**
 * Returns if given slab kind is a reinforced wall, fortified by a player.
 * @param slbkind The slab kind to be checked.
 * @return True if the slab is a fortified wall, false otherwise.
 */
TbBool slab_kind_is_fortified_wall(RoomKind slbkind)
{
    return (slbkind == SlbT_WALLDRAPE) || (slbkind == SlbT_WALLTORCH) ||
           (slbkind == SlbT_WALLWTWINS) || (slbkind == SlbT_WALLWWOMAN) ||
           (slbkind == SlbT_WALLPAIRSHR);
}

/**
 * Returns if given slab kind is a friable, unfortified and unowned dirt.
 * @param slbkind The slab kind to be checked.
 * @return True if the slab is an unowned dirt, false otherwise.
 */
TbBool slab_kind_is_friable_dirt(RoomKind slbkind)
{
    return (slbkind == SlbT_EARTH) || (slbkind == SlbT_TORCHDIRT);
}

TbBool slab_kind_is_door(SlabKind slbkind)
{
    if ((slbkind >= SlbT_DOORWOOD1) && (slbkind <= SlbT_DOORMAGIC2))
    {
        return true;
    }
    return false;
}

TbBool slab_kind_is_nonmagic_door(SlabKind slbkind)
{
    if ((slbkind >= SlbT_DOORWOOD1) && (slbkind <= SlbT_DOORIRON2))
    {
        return true;
    }
    return false;
}

/**
 * Returns if given slab type represents liquid slab.
 * Liquid slabs can be used to build bridges, and other rooms cannot be built on them.
 * @param slbkind
 * @return
 */
TbBool slab_kind_is_liquid(SlabKind slbkind)
{
    if ((slbkind == SlbT_WATER) || (slbkind == SlbT_LAVA))
        return true;
    return false;
}

/**
 * Returns if given slab type represents room slab.
 * @param slbkind The slab kind to be checked.
 * @return True if given slab kind is assigned to a room, false otherwise.
 */
TbBool slab_kind_is_room(SlabKind slbkind)
{
    return (slab_corresponding_room(slbkind) != 0);
}

/** Returns creature model to be created by given room kind.
 * @param rkind The room kind to be checked.
 * @return
 */
ThingModel get_room_create_creature_model(RoomKind rkind)
{
    const struct RoomConfigStats *roomst;
    roomst = get_room_kind_stats(rkind);
    return roomst->creature_creation_model;
}

TbBool enemies_may_work_continuously_in_room(RoomKind rkind)
{
    return (get_jobs_enemies_may_do_continuously_in_room(rkind) != Job_NULL);
}

TbBool room_role_matches(RoomKind rkind, RoomRole rrole)
{
    //TODO CONFIG Place this in room config data
    switch (rkind)
    {
    case RoK_ENTRANCE:
        return ((rrole & (RoRoF_CrPoolSpawn|RoRoF_CrPoolLeave)) != 0);
    case RoK_TREASURE:
        return ((rrole & (RoRoF_GoldStorage)) != 0);
    case RoK_LIBRARY:
        return ((rrole & (RoRoF_PowersStorage|RoRoF_Research)) != 0);
    case RoK_PRISON:
        return ((rrole & (RoRoF_Prison|RoRoF_CrConditSpawn)) != 0);
    case RoK_TORTURE:
        return ((rrole & (RoRoF_Torture|RoRoF_CrConditSpawn)) != 0);
    case RoK_TRAINING:
        return ((rrole & (RoRoF_CrTrainExp)) != 0);
    case RoK_DUNGHEART:
        return ((rrole & (RoRoF_KeeperStorage)) != 0);
    case RoK_WORKSHOP:
        return ((rrole & (RoRoF_CratesStorage|RoRoF_CratesManufctr)) != 0);
    case RoK_SCAVENGER:
        return ((rrole & (RoRoF_CrScavenge|RoRoF_CrConditSpawn)) != 0);
    case RoK_TEMPLE:
        return ((rrole & (RoRoF_CrSacrifice|RoRoF_CrPurifySpell|RoRoF_CrHappyPray|RoRoF_CrConditSpawn)) != 0);
    case RoK_GRAVEYARD:
        return ((rrole & (RoRoF_DeadStorage|RoRoF_CrConditSpawn)) != 0);
    case RoK_BARRACKS:
        return ((rrole & (RoRoF_CrMakeGroup)) != 0);
    case RoK_GARDEN:
        return ((rrole & (RoRoF_FoodStorage|RoRoF_FoodSpawn)) != 0);
    case RoK_LAIR:
        return ((rrole & (RoRoF_LairStorage|RoRoF_CrHealSleep)) != 0);
    case RoK_BRIDGE:
        return ((rrole & (RoRoF_PassWater|RoRoF_PassLava)) != 0);
    case RoK_GUARDPOST:
        return ((rrole & (RoRoF_CrGuard)) != 0);
    default:
        break;
    }
    return false;
}

TbBool room_has_surrounding_flames(RoomKind rkind)
{
    //TODO CONFIG Place this in room config data
    return (rkind != RoK_DUNGHEART);
}

/**
 * Returns if given room kind cannot be vandalized (it's either indestructible or crucial for the game).
 * @param rkind The room kind to be checked.
 * @return True if given room kind cannot be vandalized or accidently destroyed, false otherwise.
 */
TbBool room_cannot_vandalise(RoomKind rkind)
{
    struct RoomConfigStats *roomst;
    roomst = get_room_kind_stats(rkind);
    return ((roomst->flags & RoCFlg_CantVandalize) != 0);
}

/**
 * Returns if given room kind is by definition not buildable.
 * @param rkind The room kind to be checked.
 * @return True if given room kind is unconditionally not buildable, false otherwise.
 */
TbBool room_never_buildable(RoomKind rkind)
{
    //TODO CONFIG Place this in room config data
    return (rkind == RoK_DUNGHEART) || (rkind == RoK_ENTRANCE);
}

/**
 * Returns if given room kind can have an informational ensign regarding health and effectiveness.
 * @param rkind The room kind to be checked.
 * @return True if given room kind should have the flag, false otherwise.
 */
TbBool room_can_have_ensign(RoomKind rkind)
{
    struct RoomConfigStats *roomst;
    roomst = get_room_kind_stats(rkind);
    return ((roomst->flags & RoCFlg_NoEnsign) == 0);
}

/**
 * Returns slab kind which corresponds to given room kind.
 * @param rkind The room kind to be checked.
 * @return The corresponding slab kind index.
 */
SlabKind room_corresponding_slab(RoomKind rkind)
{
    struct RoomConfigStats *roomst;
    roomst = get_room_kind_stats(rkind);
    return roomst->assigned_slab;
}

/**
 * Returns room kind which corresponds to given slab kind.
 * @param slbkind The slab kind to be checked.
 * @return The corresponding room kind index.
 */
RoomKind slab_corresponding_room(SlabKind slbkind)
{
    struct RoomConfigStats *roomst;
    RoomKind rkind;
    for (rkind=0; rkind < slab_conf.room_types_count; rkind++)
    {
        roomst = get_room_kind_stats(rkind);
        if (roomst->assigned_slab == slbkind)
            return rkind;
    }
    return 0;
}
/******************************************************************************/
