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
#include "pre_inc.h"
#include "config_terrain.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"
#include "config_strings.h"
#include "config_creature.h"
#include "game_legacy.h"
#include "custom_sprites.h"
#include "frontmenu_ingame_tabs.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static int64_t value_synergy(const struct NamedField* named_field,const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);

static void assign_update_room_tab       (const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
static void assign_icon_update_room_tab  (const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
static void assign_reinitialise_rooms    (const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
static void assign_recalculate_effeciency(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
/******************************************************************************/

const char keeper_terrain_file[]="terrain.cfg";

static const struct NamedCommand terrain_flags[] = {
    {"VALUABLE",          1},
    {"IS_ROOM",           2},
    {"UNEXPLORED",        3},
    {"DIGGABLE",          4},
    {"BLOCKING",          5},
    {"FILLED",            6},
    {"IS_DOOR",           7},
    {"TAGGED_VALUABLE",   8},
    {NULL,                0},
};

static const struct NamedCommand terrain_room_properties_commands[] = {
    {"HAS_NO_ENSIGN",     RoCFlg_NoEnsign},
    {"CANNOT_VANDALIZE",  RoCFlg_CantVandalize},
    {"BUILD_TILL_BROKE",  RoCFlg_BuildTillBroke},
    {"CANNOT_BE_SOLD",    RoCFlg_CannotBeSold},
    {"CANNOT_BE_CLAIMED", RoCFlg_CannotBeClaimed},
    {NULL,                0},
};

const struct NamedCommand terrain_room_total_capacity_func_type[] = {
    {"slabs_all_only",          1},
    {"slabs_all_wth_effcncy",   2},
    {"slabs_no_min_wth_effcncy",3},
    {"slabs_div2_wth_effcncy",  4},
    {"slabs_div2_nomin_effcncy",5},
    {"slabs_mul2_wth_effcncy",  6},
    {"slabs_pow2_wth_effcncy",  7},
    {"gold_slabs_wth_effcncy",  8},
    {"gold_slabs_full",         9},
    {"gold_slabs_div2",        10},
    {"none",                   11},
    {NULL,                      0},
  };

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
  
static const struct NamedField terrain_slab_named_fields[] = {
    //name                //field                                                        //default      //min     //max    //NamedCommand
    {"NAME",              0, field(game.conf.slab_conf.slab_cfgstats[0].code_name),                     0, LONG_MIN,ULONG_MAX, slab_desc,     value_name,           NULL},
    {"TOOLTIPTEXTID",     0, field(game.conf.slab_conf.slab_cfgstats[0].tooltip_stridx),     GUIStr_Empty, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"BLOCKFLAGSHEIGHT",  0, field(game.conf.slab_conf.slab_cfgstats[0].block_flags_height),            0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"BLOCKHEALTHINDEX",  0, field(game.conf.slab_conf.slab_cfgstats[0].block_health_index),            0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"BLOCKFLAGS",       -1, field(game.conf.slab_conf.slab_cfgstats[0].block_flags),                   0, LONG_MIN,ULONG_MAX, terrain_flags, value_flagsfieldshift,NULL},
    {"NOBLOCKFLAGS",     -1, field(game.conf.slab_conf.slab_cfgstats[0].noblck_flags),                  0, LONG_MIN,ULONG_MAX, terrain_flags, value_flagsfieldshift,NULL},
    {"FILLSTYLE",         0, field(game.conf.slab_conf.slab_cfgstats[0].fill_style),                    0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"CATEGORY",          0, field(game.conf.slab_conf.slab_cfgstats[0].category),                      0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"SLBID",             0, field(game.conf.slab_conf.slab_cfgstats[0].slb_id),                        0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"WIBBLE",            0, field(game.conf.slab_conf.slab_cfgstats[0].wibble),                        0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"ISSAFELAND",        0, field(game.conf.slab_conf.slab_cfgstats[0].is_safe_land),                  0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"ISDIGGABLE",        0, field(game.conf.slab_conf.slab_cfgstats[0].is_diggable),                   0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"WLBTYPE",           0, field(game.conf.slab_conf.slab_cfgstats[0].wlb_type),                      0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"ANIMATED",          0, field(game.conf.slab_conf.slab_cfgstats[0].animated),                      0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"ISOWNABLE",         0, field(game.conf.slab_conf.slab_cfgstats[0].is_ownable),                    0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {"INDESTRUCTIBLE",    0, field(game.conf.slab_conf.slab_cfgstats[0].indestructible),                0, LONG_MIN,ULONG_MAX, NULL,          value_default,        NULL},
    {NULL},
};

const struct NamedFieldSet terrain_slab_named_fields_set = {
    &game.conf.slab_conf.slab_types_count,
    "slab",
    terrain_slab_named_fields,
    slab_desc,
    TERRAIN_ITEMS_MAX,
    sizeof(game.conf.slab_conf.slab_cfgstats[0]),
    game.conf.slab_conf.slab_cfgstats,
    {"terrain.cfg","INVALID_SCRIPT"},
};

static const struct NamedField terrain_room_named_fields[] = {
    //name           //pos    //field                                                               //default //min     //max    //NamedCommand
    {"NAME",              0, field(game.conf.slab_conf.room_cfgstats[0].code_name),                     0, LONG_MIN,ULONG_MAX,      room_desc,                            value_name,      NULL},
    {"COST",              0, field(game.conf.slab_conf.room_cfgstats[0].cost),                          0, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   NULL},
    {"HEALTH",            0, field(game.conf.slab_conf.room_cfgstats[0].health),                        0, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   NULL},
    {"PROPERTIES",       -1, field(game.conf.slab_conf.room_cfgstats[0].flags),                         0, LONG_MIN,RoCFlg_ListEnd, terrain_room_properties_commands,     value_flagsfield,NULL},
    {"SLABASSIGN",        0, field(game.conf.slab_conf.room_cfgstats[0].assigned_slab),                 0, LONG_MIN,ULONG_MAX,      slab_desc,                            value_default,   NULL},
    {"CREATURECREATION",  0, field(game.conf.slab_conf.room_cfgstats[0].creature_creation_model),       0, LONG_MIN,ULONG_MAX,      creature_desc,                        value_default,   NULL},
    {"MESSAGES",          0, field(game.conf.slab_conf.room_cfgstats[0].msg_needed),                    0, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   NULL},
    {"MESSAGES",          1, field(game.conf.slab_conf.room_cfgstats[0].msg_too_small),                 0, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   NULL},
    {"MESSAGES",          2, field(game.conf.slab_conf.room_cfgstats[0].msg_no_route),                  0, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   NULL},
    {"NAMETEXTID",        0, field(game.conf.slab_conf.room_cfgstats[0].name_stridx),        GUIStr_Empty, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   NULL},
    {"TOOLTIPTEXTID",     0, field(game.conf.slab_conf.room_cfgstats[0].tooltip_stridx),     GUIStr_Empty, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   assign_update_room_tab},
    {"SYMBOLSPRITES",     0, field(game.conf.slab_conf.room_cfgstats[0].bigsym_sprite_idx),             0, LONG_MIN,ULONG_MAX,      NULL,                                 value_icon,      assign_icon},
    {"SYMBOLSPRITES",     1, field(game.conf.slab_conf.room_cfgstats[0].medsym_sprite_idx),             0, LONG_MIN,ULONG_MAX,      NULL,                                 value_icon,      assign_icon_update_room_tab},
    {"POINTERSPRITES",    0, field(game.conf.slab_conf.room_cfgstats[0].pointer_sprite_idx),            0, LONG_MIN,ULONG_MAX,      NULL,                                 value_icon,      assign_icon_update_room_tab},
    {"PANELTABINDEX",     0, field(game.conf.slab_conf.room_cfgstats[0].panel_tab_idx),                 0,        0,       32,      NULL,                                 value_default,   assign_update_room_tab},
    {"TOTALCAPACITY",     0, field(game.conf.slab_conf.room_cfgstats[0].update_total_capacity_idx),     0, LONG_MIN,ULONG_MAX,      terrain_room_total_capacity_func_type,value_default,   assign_reinitialise_rooms},
    {"USEDCAPACITY",      0, field(game.conf.slab_conf.room_cfgstats[0].update_storage_in_room_idx),    0, LONG_MIN,ULONG_MAX,      terrain_room_used_capacity_func_type, value_default,   NULL},
    {"USEDCAPACITY",      1, field(game.conf.slab_conf.room_cfgstats[0].update_workers_in_room_idx),    0, LONG_MIN,ULONG_MAX,      terrain_room_used_capacity_func_type, value_default,   assign_reinitialise_rooms},
    {"SLABSYNERGY",       0, field(game.conf.slab_conf.room_cfgstats[0].synergy_slab),                  0, LONG_MIN,ULONG_MAX,      slab_desc,                            value_synergy,   assign_recalculate_effeciency},
    {"AMBIENTSNDSAMPLE",  0, field(game.conf.slab_conf.room_cfgstats[0].ambient_snd_smp_id),            0, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   NULL},
    {"ROLES",            -1, field(game.conf.slab_conf.room_cfgstats[0].roles),                         0, LONG_MIN,ULONG_MAX,      room_roles_desc,                      value_flagsfield,NULL},
    {"STORAGEHEIGHT",     0, field(game.conf.slab_conf.room_cfgstats[0].storage_height),                0, LONG_MIN,ULONG_MAX,      NULL,                                 value_default,   NULL},
    {NULL},
};

const struct NamedFieldSet terrain_room_named_fields_set = {
    &game.conf.slab_conf.room_types_count,
    "room",
    terrain_room_named_fields,
    room_desc,
    TERRAIN_ITEMS_MAX,
    sizeof(game.conf.slab_conf.room_cfgstats[0]),
    game.conf.slab_conf.room_cfgstats,
    {"terrain.cfg","SET_ROOM_CONFIGURATION"}
};

static void assign_update_room_tab(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src)
{
    int64_t old_value = get_named_field_value(named_field,named_fields_set,idx);
    if (value == old_value)
    {
        return;
    }    

    assign_named_field_value_direct(named_field,value,named_fields_set,idx,src);
    update_room_tab_to_config();
}

static void assign_icon_update_room_tab(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src)
{
    int64_t old_value = get_named_field_value(named_field,named_fields_set,idx);
    if (value == old_value)
    {
        return;
    }    

    assign_icon(named_field,value,named_fields_set,idx,src);
    update_room_tab_to_config();
}

static void assign_reinitialise_rooms(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src)
{
    int64_t old_value = get_named_field_value(named_field,named_fields_set,idx);
    if (value == old_value)
    {
        return;
    }    

    assign_named_field_value_direct(named_field,value,named_fields_set,idx, src);
    reinitialise_rooms_of_kind(idx);
}

static void assign_recalculate_effeciency(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src)
{
    int64_t old_value = get_named_field_value(named_field,named_fields_set,idx);
    if (value == old_value)
    {
        return;
    }    

    assign_named_field_value_direct(named_field,value,named_fields_set,idx, src);
    recalculate_effeciency_for_rooms_of_kind(idx);
}



const struct NamedCommand room_roles_desc[] = {
  {"ROOM_ROLE_KEEPER_STORAGE", RoRoF_KeeperStorage},
  {"ROOM_ROLE_LAIR_STORAGE",   RoRoF_LairStorage},
  {"ROOM_ROLE_GOLD_STORAGE",   RoRoF_GoldStorage},
  {"ROOM_ROLE_FOOD_STORAGE",   RoRoF_FoodStorage},
  {"ROOM_ROLE_CRATES_STORAGE", RoRoF_CratesStorage},
  {"ROOM_ROLE_POWERS_STORAGE", RoRoF_PowersStorage},
  {"ROOM_ROLE_PRISON",         RoRoF_Prison},
  {"ROOM_ROLE_DEAD_STORAGE",   RoRoF_DeadStorage},
  {"ROOM_ROLE_POOL_SPAWN",     RoRoF_CrPoolSpawn},
  {"ROOM_ROLE_CONDITIONAL_SPAWN",RoRoF_CrConditSpawn},
  {"ROOM_ROLE_SACRIFICE",      RoRoF_CrSacrifice},
  {"ROOM_ROLE_PURIFY_SPELLS",  RoRoF_CrPurifySpell},
  {"ROOM_ROLE_FOOD_SPAWN",     RoRoF_FoodSpawn},
  {"ROOM_ROLE_CRATES_MANUFACTURE",RoRoF_CratesManufctr},
  {"ROOM_ROLE_RESEARCH",       RoRoF_Research},
  {"ROOM_ROLE_TORTURE",        RoRoF_Torture},
  {"ROOM_ROLE_HAPPY_PRAY",     RoRoF_CrHappyPray},
  {"ROOM_ROLE_HEAL_SLEEP",     RoRoF_CrHealSleep},
  {"ROOM_ROLE_SCAVENGE",       RoRoF_CrScavenge},
  {"ROOM_ROLE_TRAIN_EXP",      RoRoF_CrTrainExp},
  {"ROOM_ROLE_MAKE_GROUP",     RoRoF_CrMakeGroup},
  {"ROOM_ROLE_GUARD",          RoRoF_CrGuard},
  {"ROOM_ROLE_POOL_LEAVE",     RoRoF_CrPoolLeave},
  {"ROOM_ROLE_PASS_WATER",     RoRoF_PassWater},
  {"ROOM_ROLE_PASS_LAVA",      RoRoF_PassLava},
  {"ROOM_ROLE_NONE",           RoRoF_None},
  {NULL,                       0},
};

/* Room capacity computation, using functions from room_data.c */

extern void count_slabs_all_only(struct Room *room);
extern void count_slabs_all_wth_effcncy(struct Room *room);
extern void count_slabs_no_min_wth_effcncy(struct Room *room);
extern void count_slabs_div2_wth_effcncy(struct Room *room);
extern void count_slabs_div2_nomin_effcncy(struct Room *room);
extern void count_slabs_mul2_wth_effcncy(struct Room *room);
extern void count_slabs_pow2_wth_effcncy(struct Room *room);
extern void count_gold_slabs_wth_effcncy(struct Room *room);
extern void count_gold_slabs_full(struct Room *room);
extern void count_gold_slabs_div2(struct Room* room);

Room_Update_Func terrain_room_total_capacity_func_list[] = {
  NULL,
  count_slabs_all_only,
  count_slabs_all_wth_effcncy,
  count_slabs_no_min_wth_effcncy,
  count_slabs_div2_wth_effcncy,
  count_slabs_div2_nomin_effcncy,
  count_slabs_mul2_wth_effcncy,
  count_slabs_pow2_wth_effcncy,
  count_gold_slabs_wth_effcncy,
  count_gold_slabs_full,
  count_gold_slabs_div2,
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
struct NamedCommand slab_desc[TERRAIN_ITEMS_MAX];
struct NamedCommand room_desc[TERRAIN_ITEMS_MAX];

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

struct SlabConfigStats *get_slab_kind_stats(SlabKind slab_kind)
{
    if (slab_kind >= game.conf.slab_conf.slab_types_count)
        return &game.conf.slab_conf.slab_cfgstats[0];
    return &game.conf.slab_conf.slab_cfgstats[slab_kind];
}

struct SlabConfigStats *get_slab_stats(const struct SlabMap *slb)
{
    if (slabmap_block_invalid(slb))
        return &game.conf.slab_conf.slab_cfgstats[0];
    return get_slab_kind_stats(slb->kind);
}

/**
 * Returns Code Name (name to use in script file) of given slab kind.
 */
const char *slab_code_name(SlabKind slbkind)
{
    const char* name = get_conf_parameter_text(slab_desc, slbkind);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given room role.
 */
const char *room_role_code_name(RoomRole rrole)
{
    const char* name = get_conf_parameter_text(room_roles_desc, rrole);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

struct RoomConfigStats *get_room_kind_stats(RoomKind room_kind)
{
    if (room_kind >= game.conf.slab_conf.room_types_count)
        return &game.conf.slab_conf.room_cfgstats[0];
    return &game.conf.slab_conf.room_cfgstats[room_kind];
}

/**
 * Returns Code Name (name to use in script file) of given room kind.
 */
const char *room_code_name(RoomKind rkind)
{
    const char* name = get_conf_parameter_text(room_desc, rkind);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

static int64_t value_synergy(const struct NamedField* named_field,const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src)
{
    if (strcasecmp(value_text, "none") == 0) {
        return -1;
    } else {
        return value_default(named_field, value_text, named_fields_set, idx, src);
    }
}

TbBool parse_block_health_block(char *buf, long len, const char *config_textname, unsigned short flags)
{
    long pos = 0;
    int k = 0;
    int n = 0;
    int cmd_num = 0;
    char word_buf[COMMAND_WORD_LEN];
    // Block health - will be later integrated with slab blocks
      char block_buf[COMMAND_WORD_LEN];
      sprintf(block_buf,"block_health");
      pos = 0;
      k = find_conf_block(buf,&pos,len,block_buf);
      if (k < 0)
      {
          if ((flags & CnfLd_AcceptPartial) == 0)
              WARNMSG("aaBlock [%s] not found in %s file.",block_buf,config_textname);
          return false;
      } else
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(terrain_health_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,terrain_health_commands);
        // Now store the config item in correct place
        if (cmd_num == ccr_endOfBlock) break; // if next block starts
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
        case ccr_comment:
            break;
        case ccr_endOfFile:
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

TbBool load_terrain_config_file(const char *textname, const char *fname, unsigned short flags)
{
    SYNCDBG(0,"%s %s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",textname,fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    
    // Parse blocks of the config file
    parse_named_field_blocks(buf, len, textname, flags, &terrain_slab_named_fields_set);
            
    parse_block_health_block(buf, len, textname, flags);

    parse_named_field_blocks(buf, len, textname, flags, &terrain_room_named_fields_set);

    //Freeing and exiting
    free(buf);
    return result;
}

TbBool load_terrain_config(const char *conf_fname, unsigned short flags)
{
    static const char config_global_textname[] = "global terrain config";
    static const char config_campgn_textname[] = "campaign terrain config";
    static const char config_level_textname[] = "level terrain config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_terrain_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_terrain_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), conf_fname);
    if (strlen(fname) > 0)
    {
        load_terrain_config_file(config_level_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/**
 * Zeroes all the costs for all rooms.
 */
TbBool make_all_rooms_free(void)
{
    for (long rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
        roomst->cost = 0;
    }
    return true;
}

/**
 * Makes all rooms to be available to research for the player.
 */
TbBool make_all_rooms_researchable(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot do; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    for (long rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        dungeon->room_resrchable[rkind] = 1;
    }
    return true;
}

/**
 * Sets room availability state.
 */
TbBool set_room_available(PlayerNumber plyr_idx, RoomKind rkind, long resrch, long avail)
{
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    struct Computer2* comp = INVALID_COMPUTER_PLAYER;
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot do; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    if (rkind >= game.conf.slab_conf.room_types_count)
    {
        ERRORLOG("Can't add incorrect room %d to player %d",(int)rkind, (int)plyr_idx);
        return false;
    }
    dungeon->room_resrchable[rkind] = resrch;
    // This doesnt reset if player has room in the past
    if (resrch != 0)
        dungeon->room_buildable[rkind] |= (avail? 1 : 0 );
    else
        dungeon->room_buildable[rkind] &= ~1;

    if (dungeon->room_buildable[rkind] & 1)
    {
        comp = get_computer_player(plyr_idx);
        if (comp != INVALID_COMPUTER_PLAYER)
        {
            reactivate_build_process(comp, rkind);
        }
    }

    return true;
}

/**
 * Returns if the room can be built by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if the player has enough money or map position is on correct spot.
 */
TbBool is_room_available(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build rooms
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if (rkind >= game.conf.slab_conf.room_types_count)
    {
      ERRORLOG("Incorrect room %d (player %d)",(int)rkind, (int)plyr_idx);
      return false;
    }
    if (dungeon->room_buildable[rkind] & 1) {
        return true;
    }
    return false;
}

/**
 * Returns if the room can be or already is obtained by a player.
 */
TbBool is_room_obtainable(PlayerNumber plyr_idx, RoomKind rkind)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even has a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build rooms
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if (rkind >= game.conf.slab_conf.room_types_count) {
        ERRORLOG("Incorrect room %u (player %d)",rkind, plyr_idx);
        return false;
    }
    return ( (dungeon->room_buildable[rkind]) || (dungeon->room_resrchable[rkind]) );
}

/**
 * Returns if a room that has role can be built by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if the player has enough money or map position is on correct spot.
 */
RoomKind find_first_available_roomkind_with_role(PlayerNumber plyr_idx, RoomRole rrole)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return RoK_NONE;
    }
    // Player must have dungeon heart to build rooms
    if (!player_has_heart(plyr_idx)) {
        return RoK_NONE;
    }

    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if (room_role_matches(rkind, rrole))
        {
            if (dungeon->room_buildable[rkind] & 1)
            {
                return rkind;
            }
        }
    }
    return RoK_NONE;
}

/**
 * Returns if a room that has role can be built by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if the player has enough money or map position is on correct spot.
 */
TbBool is_room_of_role_available(PlayerNumber plyr_idx, RoomRole rrole)
{
    if (find_first_available_roomkind_with_role(plyr_idx, rrole) > RoK_NONE)
    {
        return true;
    }
    return false;
}

/**
 * Makes all the rooms, which are researchable, to be instantly available.
 */
TbBool make_available_all_researchable_rooms(PlayerNumber plyr_idx)
{
    SYNCDBG(0,"Starting");
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot do; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    for (long i = 0; i < game.conf.slab_conf.room_types_count; i++)
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
    struct SlabConfigStats* slabst = get_slab_kind_stats(slbkind);
    return (slabst->indestructible);
}

/**
 * Returns if given slab kind is a reinforced wall modified by the room besides it.
 * @param slbkind The slab kind to be checked.
 * @return True if the slab is a fortified wall next to a room, false otherwise.
 */
TbBool slab_kind_is_room_wall(RoomKind slbkind)
{
    struct SlabConfigStats* slabst = get_slab_kind_stats(slbkind);
    return ((slabst->category == SlbAtCtg_FortifiedWall) && (slabst->slb_id != 0));
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
    struct SlabConfigStats* slabst = get_slab_kind_stats(slbkind);
    return (slabst->block_flags & (SlbAtFlg_IsDoor));
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

/**
 * Returns if given slab type represents room slab.
 * @param slbkind The slab kind to be checked.
 * @return True if given slab kind is assigned to a room, false otherwise.
 */
TbBool slab_kind_has_torches(SlabKind slbkind)
{
    if ((slbkind == SlbT_WALLTORCH) || (slbkind == SlbT_TORCHDIRT))
        return true;
    return false;
}

/** Returns creature model to be created by given room kind.
 * @param rkind The room kind to be checked.
 * @return
 */
ThingModel get_room_create_creature_model(RoomKind rkind)
{
    const struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    return roomst->creature_creation_model;
}

TbBool enemies_may_work_in_room(RoomKind rkind)
{
    return (get_jobs_enemies_may_do_in_room(rkind) != Job_NULL);
}

RoomRole get_room_roles(RoomKind rkind)
{
    const struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    return roomst->roles;
}

TbBool room_role_matches(RoomKind rkind, RoomRole rrole)
{
    return ((rrole & get_room_roles(rkind)) != 0);
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
    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
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
    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    return ((roomst->flags & RoCFlg_NoEnsign) == 0);
}

/**
 * Returns slab kind which corresponds to given room kind.
 * @param rkind The room kind to be checked.
 * @return The corresponding slab kind index.
 */
SlabKind room_corresponding_slab(RoomKind rkind)
{
    struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
    return roomst->assigned_slab;
}

/**
 * Returns room kind which corresponds to given slab kind.
 * @param slbkind The slab kind to be checked.
 * @return The corresponding room kind index.
 */
RoomKind slab_corresponding_room(SlabKind slbkind)
{
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
        if (roomst->assigned_slab == slbkind)
            return rkind;
    }
    return 0;
}

/**
 * Returns room kind which corresponds to given role.
 * @param rrole The slab kind to be checked.
 * @return The corresponding room kind index.
 */
RoomKind find_first_roomkind_with_role(RoomRole rrole)
{
    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,rrole))
        {
            return rkind;
        }
    }
    return 0;
}
/******************************************************************************/
