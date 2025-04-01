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
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

const char keeper_compplayer_file[]="keepcompp.cfg";

/******************************************************************************/
struct ComputerPlayerConfig comp_player_conf;

static TbBool computer_type_clear_processes(struct ComputerTypes *cpt);
static TbBool computer_type_clear_checks(struct ComputerTypes *cpt);
static short computer_type_clear_events(struct ComputerTypes *cpt);

static int64_t value_processes(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
static int64_t value_checks(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);
static int64_t value_events(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src);

static int get_computer_process_config_list_index_mnem(const char *mnemonic);
static int get_computer_check_config_list_index_mnem(const char *mnemonic);
static int get_computer_event_config_list_index_mnem(const char *mnemonic);

static int computer_type_add_process(struct ComputerTypes *cpt, struct ComputerProcess *cproc);
static int computer_type_add_check(struct ComputerTypes *cpt, struct ComputerCheck *check);
static int computer_type_add_event(struct ComputerTypes *cpt, struct ComputerEvent *event);

/******************************************************************************/

static const struct NamedField compp_common_named_fields[] = {
  {"ComputerAssists",  0, field(comp_player_conf.computer_assist_types[0]), 0, LONG_MIN,ULONG_MAX, NULL, value_default, assign_default},
  {"ComputerAssists",  1, field(comp_player_conf.computer_assist_types[1]), 0, LONG_MIN,ULONG_MAX, NULL, value_default, assign_default},
  {"ComputerAssists",  2, field(comp_player_conf.computer_assist_types[2]), 0, LONG_MIN,ULONG_MAX, NULL, value_default, assign_default},
  {"ComputerAssists",  3, field(comp_player_conf.computer_assist_types[3]), 0, LONG_MIN,ULONG_MAX, NULL, value_default, assign_default},
  {"SkirmishFirst",    0, field(comp_player_conf.skirmish_first),           0, LONG_MIN,ULONG_MAX, NULL, value_default, assign_default},
  {"SkirmishLast",     0, field(comp_player_conf.skirmish_last),            0, LONG_MIN,ULONG_MAX, NULL, value_default, assign_default},
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
  {"keepcompp.cfg","INVALID"},
};

static const struct NamedField compp_process_named_fields[] = {
  //name           //pos    //field                                   //default //min     //max    //NamedCommand
  {"NAME",        -1, field(comp_player_conf.process_types[0].name),          0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"MNEMONIC",     0, field(comp_player_conf.process_types[0].mnemonic),      0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"VALUES",       0, field(comp_player_conf.process_types[0].priority     ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       1, field(comp_player_conf.process_types[0].confval_2    ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       2, field(comp_player_conf.process_types[0].confval_3    ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       3, field(comp_player_conf.process_types[0].confval_4    ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       4, field(comp_player_conf.process_types[0].confval_5    ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"FUNCTIONS",    0, field(comp_player_conf.process_types[0].func_check   ), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"FUNCTIONS",    1, field(comp_player_conf.process_types[0].func_setup   ), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"FUNCTIONS",    2, field(comp_player_conf.process_types[0].func_task    ), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"FUNCTIONS",    3, field(comp_player_conf.process_types[0].func_complete), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"FUNCTIONS",    4, field(comp_player_conf.process_types[0].func_pause   ), 0, LONG_MIN,ULONG_MAX, computer_process_func_type, value_default, assign_default},
  {"PARAMS",       0, field(comp_player_conf.process_types[0].param_1      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       1, field(comp_player_conf.process_types[0].param_2      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       2, field(comp_player_conf.process_types[0].param_3      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       3, field(comp_player_conf.process_types[0].last_run_turn), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       4, field(comp_player_conf.process_types[0].param_5      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
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
  {"keepcompp.cfg","INVALID"},
};

static const struct NamedField compp_check_named_fields[] = {
  //name           //pos    //field                                   //default //min     //max    //NamedCommand
  {"NAME",        -1, field(comp_player_conf.check_types[0].name          ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"MNEMONIC",     0, field(comp_player_conf.check_types[0].mnemonic      ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
  {"VALUES",       0, field(comp_player_conf.check_types[0].flags         ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"VALUES",       1, field(comp_player_conf.check_types[0].turns_interval), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"FUNCTIONS",    0, field(comp_player_conf.check_types[0].func          ), 0, LONG_MIN,ULONG_MAX, computer_check_func_type,   value_default, assign_default},
  {"PARAMS",       0, field(comp_player_conf.check_types[0].param1        ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       1, field(comp_player_conf.check_types[0].param2        ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       2, field(comp_player_conf.check_types[0].param3        ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
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
  {"keepcompp.cfg","INVALID"},
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
  {"PARAMS",       0, field(comp_player_conf.event_types[0].param1             ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       1, field(comp_player_conf.event_types[0].param2             ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
  {"PARAMS",       2, field(comp_player_conf.event_types[0].param3             ), 0, LONG_MIN,ULONG_MAX, NULL,                       value_default, assign_default},
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
  {"keepcompp.cfg","INVALID"},
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
  {"keepcompp.cfg","INVALID"},
};

/******************************************************************************/
int64_t value_processes(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src)
{
  char word_buf[COMMAND_WORD_LEN];
  struct ComputerTypes* cpt = get_computer_type_template(idx);
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
      if (computer_type_add_process(cpt, &comp_player_conf.process_types[process_idx]) < 0)
      {
          NAMFIELDWRNLOG("failed to add process %s for [%s%d].",word_buf, named_fields_set->block_basename, idx);
      }
  }
  return 0;
}

int64_t value_checks(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src)
{
    char word_buf[COMMAND_WORD_LEN];
    struct ComputerTypes* cpt = get_computer_type_template(idx);
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
        if (computer_type_add_check(cpt, &comp_player_conf.check_types[check_idx]) < 0)
        {
            NAMFIELDWRNLOG("failed to add check %s for [%s%d].",word_buf, named_fields_set->block_basename, idx);
        }
    }
    return 0;
}

int64_t value_events(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, unsigned char src)
{
  char word_buf[COMMAND_WORD_LEN];
  struct ComputerTypes* cpt = get_computer_type_template(idx);
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
    if (computer_type_add_event(cpt, &comp_player_conf.event_types[event_idx]) < 0)
    {
        NAMFIELDWRNLOG("failed to add event %s for [%s%d].",word_buf, named_fields_set->block_basename, idx);
    }
  }
  return 0;
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

struct ComputerTypes *get_computer_type_template(long cpt_idx)
{
    if ((cpt_idx < 0) || (cpt_idx >= COMPUTER_MODELS_COUNT))
        cpt_idx = 0;
  return &comp_player_conf.computer_types[cpt_idx];
}

static TbBool computer_type_clear_processes(struct ComputerTypes *cpt)
{
    for (int i = 0; i < COMPUTER_PROCESSES_COUNT; i++)
    {
        cpt->processes[i] = NULL;
  }
  return true;
}

static int computer_type_add_process(struct ComputerTypes *cpt, struct ComputerProcess *cproc)
{
    for (int i = 0; i < COMPUTER_PROCESSES_COUNT; i++)
    {
        if (cpt->processes[i] == NULL)
        {
            cpt->processes[i] = cproc;
            return i;
        }
  }
  return -1;
}

static TbBool computer_type_clear_checks(struct ComputerTypes *cpt)
{
    for (int i = 0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        memset(&cpt->checks[i], 0, sizeof(struct ComputerCheck));
  }
  return true;
}

static int computer_type_add_check(struct ComputerTypes *cpt, struct ComputerCheck *check)
{
    for (int i = 0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        if (cpt->checks[i].name[0] == '\0')
        {
            memcpy(&cpt->checks[i], check, sizeof(struct ComputerCheck));
            return i;
        }
  }
  return -1;
}

short computer_type_clear_events(struct ComputerTypes *cpt)
{
    for (int i = 0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        memset(&cpt->events[i], 0, sizeof(struct ComputerEvent));
  }
  return true;
}

static int computer_type_add_event(struct ComputerTypes *cpt, struct ComputerEvent *event)
{
    for (int i = 0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        if (cpt->events[i].name[0] == '\0')
        {
            memcpy(&cpt->events[i], event, sizeof(struct ComputerEvent));
            return i;
        }
  }
  return -1;
}

TbBool load_computer_player_config(unsigned short flags)
{
    SYNCDBG(8, "Starting");
    static const char *textname = "Computer Player";
    // Load the config file
    const char* fname = prepare_file_path(FGrp_FxData, keeper_compplayer_file);
    long len = LbFileLengthRnc(fname);
    if (len < 2)
    {
        ERRORLOG("Computer Player file \"%s\" doesn't exist or is too small.",keeper_compplayer_file);
        return false;
    }
    if (len > 65536)
    {
        ERRORLOG("Computer Player file \"%s\" is too large.",keeper_compplayer_file);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
      return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    if (len>0)
    {
        parse_named_field_block(buf, len, textname, flags,"common",  compp_common_named_fields,&compp_common_named_fields_set, 0);
        parse_named_field_blocks(buf, len, textname, flags, &compp_process_named_fields_set);
        parse_named_field_blocks(buf, len, textname, flags, &compp_check_named_fields_set);
        parse_named_field_blocks(buf, len, textname, flags, &compp_event_named_fields_set);
        parse_named_field_blocks(buf, len, textname, flags, &compp_computer_named_fields_set);
    }
    //Freeing and exiting
    free(buf);
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
