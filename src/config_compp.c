/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_compp.c
 *     Computer player configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for computer player.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     25 May 2009 - 06 Jan 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_compp.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "config_strings.h"
#include "player_computer.h"
#include "thing_data.h"
#include "game_merge.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static TbBool load_computer_player_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_keepcomp_file_data = {
  .filename = "keepcompp.cfg",
  .load_func = load_computer_player_config_file,
  .post_load_func = NULL,
};


/******************************************************************************/
struct ComputerPlayerConfig comp_player_conf;

static TbBool computer_type_clear_processes(struct ComputerType *cpt);
static TbBool computer_type_clear_checks(struct ComputerType *cpt);
static short computer_type_clear_events(struct ComputerType *cpt);

static int64_t value_processes(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);
static int64_t value_checks(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);
static int64_t value_events(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);
int64_t value_process_mnemonic(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);

static int get_computer_process_config_list_index_mnem(const char *mnemonic);
static int get_computer_check_config_list_index_mnem(const char *mnemonic);
static int get_computer_event_config_list_index_mnem(const char *mnemonic);

static int computer_type_add_process(struct ComputerType *cpt, unsigned char cproc_idx);
static int computer_type_add_check(struct ComputerType *cpt, unsigned char check_idx);
static int computer_type_add_event(struct ComputerType *cpt, unsigned char event_idx);

/******************************************************************************/

static const struct NamedField compp_common_named_fields[] = {
  {"ComputerAssists",        0, field(comp_player_conf.computer_assist_types[0]), 0, 0,COMPUTER_MODELS_COUNT, NULL, value_default, assign_default},
  {"ComputerAssists",        1, field(comp_player_conf.computer_assist_types[1]), 0, 0,COMPUTER_MODELS_COUNT, NULL, value_default, assign_default},
  {"ComputerAssists",        2, field(comp_player_conf.computer_assist_types[2]), 0, 0,COMPUTER_MODELS_COUNT, NULL, value_default, assign_default},
  {"ComputerAssists",        3, field(comp_player_conf.computer_assist_types[3]), 0, 0,COMPUTER_MODELS_COUNT, NULL, value_default, assign_default},
  {"SkirmishFirst",          0, field(comp_player_conf.skirmish_first),           0, 0,COMPUTER_MODELS_COUNT, NULL, value_default, assign_default},
  {"SkirmishLast",           0, field(comp_player_conf.skirmish_last),            0, 0,COMPUTER_MODELS_COUNT, NULL, value_default, assign_default},
  {"DefaultComputerAssist",  0, field(comp_player_conf.player_assist_default),    0, 0,COMPUTER_MODELS_COUNT, NULL, value_default, assign_default},
  {NULL},
};

const struct NamedFieldSet compp_common_named_fields_set = {
  NULL,
  "common",
  compp_common_named_fields,
  NULL,
  0,
  0,
  NULL,
};

static const struct NamedField compp_process_named_fields[] = {
  //name           //pos    //field                                   //default //min     //max    //NamedCommand
  {"NAME",        -1, field(comp_player_conf.process_types[0].name),          0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"MNEMONIC",     0, field(comp_player_conf.process_types[0].mnemonic),      0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"VALUES",       0, field(comp_player_conf.process_types[0].priority     ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       1, field(comp_player_conf.process_types[0].process_configuration_value_2    ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       2, field(comp_player_conf.process_types[0].process_configuration_value_3    ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       3, field(comp_player_conf.process_types[0].process_configuration_value_4    ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       4, field(comp_player_conf.process_types[0].process_configuration_value_5    ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"FUNCTIONS",    0, field(comp_player_conf.process_types[0].func_check   ), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"FUNCTIONS",    1, field(comp_player_conf.process_types[0].func_setup   ), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"FUNCTIONS",    2, field(comp_player_conf.process_types[0].func_task    ), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"FUNCTIONS",    3, field(comp_player_conf.process_types[0].func_complete), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"FUNCTIONS",    4, field(comp_player_conf.process_types[0].func_pause   ), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"PARAMS",       0, field(comp_player_conf.process_types[0].process_parameter_1      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       1, field(comp_player_conf.process_types[0].process_parameter_2      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       2, field(comp_player_conf.process_types[0].process_parameter_3      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       3, field(comp_player_conf.process_types[0].last_run_turn), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       4, field(comp_player_conf.process_types[0].process_parameter_5      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       5, field(comp_player_conf.process_types[0].flags        ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {NULL},
};

const struct NamedFieldSet compp_process_named_fields_set = {
  &comp_player_conf.processes_count,
  "process",
  compp_process_named_fields,
  NULL,
  COMPUTER_PROCESS_TYPES_COUNT,
  sizeof(comp_player_conf.process_types[0]),
  comp_player_conf.process_types,
};

static const struct NamedField compp_check_named_fields[] = {
  //name           //pos    //field                                   //default //min     //max    //NamedCommand
  {"NAME",        -1, field(comp_player_conf.check_types[0].name          ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"MNEMONIC",     0, field(comp_player_conf.check_types[0].mnemonic      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"VALUES",       0, field(comp_player_conf.check_types[0].flags         ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       1, field(comp_player_conf.check_types[0].turns_interval), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"FUNCTIONS",    0, field(comp_player_conf.check_types[0].func          ), 0, LONG_MIN,ULONG_MAX, computer_check_func_type,   value_default, assign_default},
  {"PARAMS",       0, field(comp_player_conf.check_types[0].primary_parameter        ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       1, field(comp_player_conf.check_types[0].secondary_parameter        ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       2, field(comp_player_conf.check_types[0].tertiary_parameter        ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       3, field(comp_player_conf.check_types[0].last_run_turn ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {NULL},
};


const struct NamedFieldSet compp_check_named_fields_set = {
  &comp_player_conf.checks_count,
  "check",
  compp_check_named_fields,
  NULL,
  COMPUTER_CHECKS_TYPES_COUNT,
  sizeof(comp_player_conf.check_types[0]),
  comp_player_conf.check_types,
};

static const struct NamedField compp_event_named_fields[] = {
  //name           //pos    //field                                   //default //min     //max    //NamedCommand
  {"NAME",        -1, field(comp_player_conf.event_types[0].name               ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"MNEMONIC",     0, field(comp_player_conf.event_types[0].mnemonic           ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"VALUES",       0, field(comp_player_conf.event_types[0].cetype             ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       1, field(comp_player_conf.event_types[0].mevent_kind        ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       1, field(comp_player_conf.event_types[0].test_interval      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"FUNCTIONS",    0, field(comp_player_conf.event_types[0].func_event         ), 0, LONG_MIN,ULONG_MAX, computer_event_func_type,   value_default, assign_default},
  {"FUNCTIONS",    0, field(comp_player_conf.event_types[0].func_test          ), 0, LONG_MIN,ULONG_MAX, computer_event_test_func_type,   value_default, assign_default},
  {"PROCESS",      0, field(comp_player_conf.event_types[0].process            ), 0, LONG_MIN,ULONG_MAX, NULL,              value_process_mnemonic, assign_default},
  {"PARAMS",       0, field(comp_player_conf.event_types[0].primary_parameter             ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       1, field(comp_player_conf.event_types[0].secondary_parameter             ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       2, field(comp_player_conf.event_types[0].tertiary_parameter             ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       3, field(comp_player_conf.event_types[0].last_test_gameturn ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {NULL},
};


const struct NamedFieldSet compp_event_named_fields_set = {
  &comp_player_conf.events_count,
  "event",
  compp_event_named_fields,
  NULL,
  COMPUTER_EVENTS_TYPES_COUNT,
  sizeof(comp_player_conf.event_types[0]),
  comp_player_conf.event_types,
};



static const struct NamedField compp_computer_named_fields[] = {
  //name           //pos    //field                                                    //default //min     //max    //NamedCommand
  {"NAME",             -1, field(comp_player_conf.computer_types[0].name),                     0, LONG_MIN,ULONG_MAX, NULL,         value_name,    assign_null},
  {"TOOLTIPTEXTID",     0, field(comp_player_conf.computer_types[0].tooltip_stridx),GUIStr_Empty, LONG_MIN,ULONG_MAX, NULL,         value_default, assign_default},
  {"ASSISTANTICON",     0, field(comp_player_conf.computer_types[0].sprite_idx),               0, LONG_MIN,ULONG_MAX, NULL,         value_icon,    assign_icon},
  {"VALUES",            0, field(comp_player_conf.computer_types[0].dig_stack_size ),          0, LONG_MIN,ULONG_MAX, NULL,         value_default, assign_default},
  {"VALUES",            1, field(comp_player_conf.computer_types[0].processes_time ),          0, LONG_MIN,ULONG_MAX, NULL,         value_default, assign_default},
  {"VALUES",            2, field(comp_player_conf.computer_types[0].click_rate),               0, LONG_MIN,ULONG_MAX, NULL,         value_default, assign_default},
  {"VALUES",            3, field(comp_player_conf.computer_types[0].max_room_build_tasks),     0, LONG_MIN,ULONG_MAX, NULL,         value_default, assign_default},
  {"VALUES",            4, field(comp_player_conf.computer_types[0].turn_begin         ),      0, LONG_MIN,ULONG_MAX, NULL,         value_default, assign_default},
  {"VALUES",            5, field(comp_player_conf.computer_types[0].sim_before_dig     ),      0, LONG_MIN,ULONG_MAX, NULL,         value_default, assign_default},
  {"VALUES",            5, field(comp_player_conf.computer_types[0].drop_delay         ),      0, LONG_MIN,ULONG_MAX, NULL,         value_default, assign_default},
  {"PROCESSES",        -1, field(comp_player_conf.computer_types[0].processes          ),      0, LONG_MIN,ULONG_MAX, NULL,         value_processes, assign_null},
  {"CHECKS",           -1, field(comp_player_conf.computer_types[0].checks             ),      0, LONG_MIN,ULONG_MAX, NULL,         value_checks, assign_null},
  {"EVENTS",           -1, field(comp_player_conf.computer_types[0].events             ),      0, LONG_MIN,ULONG_MAX, NULL,         value_events, assign_null},
  {NULL},
};


const struct NamedFieldSet compp_computer_named_fields_set = {
  &comp_player_conf.computers_count,
  "computer",
  compp_computer_named_fields,
  NULL,
  COMPUTER_MODELS_COUNT,
  sizeof(comp_player_conf.computer_types[0]),
  comp_player_conf.computer_types,
};

/******************************************************************************/
int64_t value_processes(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
  char word_buf[COMMAND_WORD_LEN];
  struct ComputerType* cpt = get_computer_type_template(idx);
  computer_type_clear_processes(cpt);

  long pos = 0;
  long len = strlen(value_text);
  while (get_conf_parameter_single(value_text,&pos,len,word_buf,sizeof(word_buf)) > 0)
  {
      int process_idx = get_computer_process_config_list_index_mnem(word_buf);
      if (process_idx <= 0)
      {
          NAMFIELDWRNLOG("process %s not recognized for [%s%d].",word_buf, named_fields_set->block_basename, idx);
          continue;
      }
      if (computer_type_add_process(cpt, process_idx) < 0)
      {
          NAMFIELDWRNLOG("failed to add process %s for [%s%d].",word_buf, named_fields_set->block_basename, idx);
      }
  }
  return 0;
}

int64_t value_checks(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    char word_buf[COMMAND_WORD_LEN];
    struct ComputerType* cpt = get_computer_type_template(idx);
    computer_type_clear_checks(cpt);

    long pos = 0;
    long len = strlen(value_text);
    while (get_conf_parameter_single(value_text,&pos,len,word_buf,sizeof(word_buf)) > 0)
    {
        int check_idx = get_computer_check_config_list_index_mnem(word_buf);
        if (check_idx <= 0)
        {
            NAMFIELDWRNLOG("check %s not recognized for [%s%d].",word_buf, named_fields_set->block_basename, idx);
            continue;
        }
        if (computer_type_add_check(cpt, check_idx) < 0)
        {
            NAMFIELDWRNLOG("failed to add check %s for [%s%d].",word_buf, named_fields_set->block_basename, idx);
        }
    }
    return 0;
}

int64_t value_events(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
  char word_buf[COMMAND_WORD_LEN];
  struct ComputerType* cpt = get_computer_type_template(idx);
  computer_type_clear_events(cpt);

  long pos = 0;
  long len = strlen(value_text);
  while (get_conf_parameter_single(value_text,&pos,len,word_buf,sizeof(word_buf)) > 0)
  {
    int event_idx = get_computer_event_config_list_index_mnem(word_buf);
    if (event_idx <= 0)
    {
        NAMFIELDWRNLOG("event %s not recognized for [%s%d].",word_buf, named_fields_set->block_basename, idx);
        continue;
    }
    if (computer_type_add_event(cpt, event_idx) < 0)
    {
        NAMFIELDWRNLOG("failed to add event %s for [%s%d].",word_buf, named_fields_set->block_basename, idx);
    }
  }
  return 0;
}

int64_t value_process_mnemonic(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    return get_computer_process_config_list_index_mnem(value_text);
}

/******************************************************************************/

static int get_computer_process_config_list_index_mnem(const char *mnemonic)
{
    for (int i = 1; i <= comp_player_conf.processes_count; i++)
    {
        if (strcasecmp(comp_player_conf.process_types[i].mnemonic, mnemonic) == 0)
            return i;
  }
  return 0;
}

static int get_computer_check_config_list_index_mnem(const char *mnemonic)
{
  const int arr_size = (int)(sizeof(comp_player_conf.check_types)/sizeof(comp_player_conf.check_types[0]));
  for (int i = 1; i < arr_size; i++)
  {
    if (strcasecmp(comp_player_conf.check_types[i].mnemonic, mnemonic) == 0)
      return i;
  }
  return 0;
}

static int get_computer_event_config_list_index_mnem(const char *mnemonic)
{
  const int arr_size = (int)(sizeof(comp_player_conf.event_types)/sizeof(comp_player_conf.event_types[0]));
  for (int i = 1; i < arr_size; i++)
  {
    if (strcasecmp(comp_player_conf.event_types[i].mnemonic, mnemonic) == 0)
      return i;
  }
  return 0;
}

struct ComputerType *get_computer_type_template(long cpt_idx)
{
    if ((cpt_idx < 0) || (cpt_idx >= COMPUTER_MODELS_COUNT))
        cpt_idx = 0;
  return &comp_player_conf.computer_types[cpt_idx];
}

static TbBool computer_type_clear_processes(struct ComputerType *cpt)
{
    memset(&cpt->processes, 0, sizeof(cpt->processes));
    return true;
}

static int computer_type_add_process(struct ComputerType *cpt, unsigned char cproc_idx)
{
    for (int i = 0; i < COMPUTER_PROCESSES_COUNT; i++)
    {
        if (cpt->processes[i] == 0)
        {
            cpt->processes[i] = cproc_idx;
            return i;
        }
    }
    return -1;
}

static TbBool computer_type_clear_checks(struct ComputerType *cpt)
{
    memset(&cpt->checks, 0, sizeof(cpt->checks));
    return true;
}

static int computer_type_add_check(struct ComputerType *cpt, unsigned char check_idx)
{
    for (int i = 0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        if (cpt->checks[i] == 0)
        {
            cpt->checks[i] = check_idx;
            return i;
        }
  }
  return -1;
}

short computer_type_clear_events(struct ComputerType *cpt)
{

    memset(&cpt->events, 0, sizeof(cpt->events));
    return true;
}

static int computer_type_add_event(struct ComputerType *cpt, unsigned char event_idx)
{
    for (int i = 0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        if (cpt->events[i] == 0)
        {
            cpt->events[i] = event_idx;
            return i;
        }
    }
    return -1;
}

static TbBool load_computer_player_config_file(const char *fname, unsigned short flags)
{
    SYNCDBG(8, "Starting");
    // Load the config file
    long len = LbFileLengthRnc(fname);
    if (len < 2)
    {
        if (!flag_is_set(flags,CnfLd_IgnoreErrors))
          ERRORLOG("Computer Player file \"%s\" doesn't exist or is too small.",fname);
        return false;
    }
    if (len > 65536)
    {
        ERRORLOG("Computer Player file \"%s\" is too large.",fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
      return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    if (len>0)
    {
        parse_named_field_block(buf, len, fname, flags,"common",  compp_common_named_fields,&compp_common_named_fields_set, 0);
        parse_named_field_blocks(buf, len, fname, flags, &compp_process_named_fields_set);
        parse_named_field_blocks(buf, len, fname, flags, &compp_check_named_fields_set);
        parse_named_field_blocks(buf, len, fname, flags, &compp_event_named_fields_set);
        parse_named_field_blocks(buf, len, fname, flags, &compp_computer_named_fields_set);
    }
    //Freeing and exiting
    free(buf);
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
