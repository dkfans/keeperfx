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
#include "player_computer.h"
#include "thing_data.h"
#include "game_merge.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const struct NamedCommand compp_common_commands[] = {
  {"COMPUTERASSISTS", 1},
  {"CHECKSCOUNT",     3},
  {"EVENTSCOUNT",     4},
  {"COMPUTERSCOUNT",  5},
  {"SKIRMISHFIRST",   6}, /*new*/
  {"SKIRMISHLAST",    7}, /*new*/
  {NULL,              0},
  };

const struct NamedCommand compp_check_commands[] = {
  {"NAME",            1},
  {"MNEMONIC",        2},
  {"VALUES",          3},
  {"FUNCTIONS",       4},
  {"PARAMS",          5},
  {NULL,              0},
  };

const struct NamedCommand compp_event_commands[] = {
  {"NAME",            1},
  {"MNEMONIC",        2},
  {"VALUES",          3},
  {"FUNCTIONS",       4},
  {"PROCESS",         5},
  {"PARAMS",          6},
  {NULL,              0},
  };

const struct NamedCommand compp_computer_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"PROCESSES",       3},
  {"CHECKS",          4},
  {"EVENTS",          5},
  {"NAMETEXTID",      6},
  {"TOOLTIPTEXTID",   7},
  {NULL,              0},
  };

const char keeper_compplayer_file[]="keepcompp.cfg";

/******************************************************************************/
ComputerName computer_check_names[COMPUTER_CHECKS_TYPES_COUNT];
struct ComputerCheck computer_checks[COMPUTER_CHECKS_TYPES_COUNT];
struct ComputerCheckMnemonic computer_check_config_list[COMPUTER_CHECKS_TYPES_COUNT];

ComputerName computer_event_names[COMPUTER_EVENTS_TYPES_COUNT];
struct ComputerEvent computer_events[COMPUTER_EVENTS_TYPES_COUNT];
struct ComputerEventMnemonic computer_event_config_list[COMPUTER_EVENTS_TYPES_COUNT];

ComputerName ComputerProcessListsNames[COMPUTER_MODELS_COUNT];
struct ComputerProcessTypes ComputerProcessLists[COMPUTER_MODELS_COUNT];

// Moved to the header file so that it can be used in player_computer.c for .skirmish_first and .skirmish_last
// which indicate the range for computer model AIs available for assignment
struct ComputerPlayerConfig comp_player_conf;


struct NamedCommand process_mnemonic_desc[COMPUTER_PROCESS_TYPES_COUNT];
struct NamedCommand process_names_desc[COMPUTER_PROCESS_TYPES_COUNT];

/******************************************************************************/

static const struct NamedField compp_process_named_fields[] = {
  //name           //pos    //field                                   //default //min     //max    //NamedCommand
  {"NAME",        -1, field(comp_player_conf.process_types[0].name),          0, LONG_MIN,ULONG_MAX, process_names_desc,         value_name,    assign_null},
  {"MNEMONIC",     0, field(comp_player_conf.process_types[0].mneumonic),     0, LONG_MIN,ULONG_MAX, NULL,                       value_name,    assign_null},
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
  process_names_desc,
  COMPUTER_PROCESS_TYPES_COUNT,
  sizeof(comp_player_conf.process_types[0]),
  comp_player_conf.process_types,
  {"keepcompp.cfg","INVALID"},
};
/******************************************************************************/

int get_computer_process_config_list_index_mnem(const char *mnemonic)
{
    for (int i = 1; i <= comp_player_conf.processes_count; i++)
    {
        if (strcasecmp(comp_player_conf.process_types[i].mneumonic, mnemonic) == 0)
            return i;
  }
  return 0;
}

int get_computer_check_config_list_index_mnem(const char *mnemonic)
{
  const int arr_size = (int)(sizeof(computer_check_config_list)/sizeof(computer_check_config_list[0]));
  for (int i = 1; i < arr_size; i++)
  {
    if (strcasecmp(computer_check_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

int get_computer_event_config_list_index_mnem(const char *mnemonic)
{
  const int arr_size = (int)(sizeof(computer_event_config_list)/sizeof(computer_event_config_list[0]));
  for (int i = 1; i < arr_size; i++)
  {
    if (strcasecmp(computer_event_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

struct ComputerProcessTypes *get_computer_process_type_template(long cpt_idx)
{
    if ((cpt_idx < 0) || (cpt_idx >= COMPUTER_MODELS_COUNT))
        cpt_idx = 0;
  return &ComputerProcessLists[cpt_idx];
}

TbBool computer_type_clear_processes(struct ComputerProcessTypes *cpt)
{
    for (int i = 0; i < COMPUTER_PROCESSES_COUNT; i++)
    {
        cpt->processes[i] = NULL;
  }
  return true;
}

int computer_type_add_process(struct ComputerProcessTypes *cpt, struct ComputerProcess *cproc)
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

short computer_type_clear_checks(struct ComputerProcessTypes *cpt)
{
    for (int i = 0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        memset(&cpt->checks[i], 0, sizeof(struct ComputerCheck));
  }
  return true;
}

int computer_type_add_check(struct ComputerProcessTypes *cpt, struct ComputerCheck *check)
{
    for (int i = 0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        if (cpt->checks[i].name == NULL)
        {
            memcpy(&cpt->checks[i], check, sizeof(struct ComputerCheck));
            return i;
        }
  }
  return -1;
}

short computer_type_clear_events(struct ComputerProcessTypes *cpt)
{
    for (int i = 0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        memset(&cpt->events[i], 0, sizeof(struct ComputerEvent));
  }
  return true;
}

int computer_type_add_event(struct ComputerProcessTypes *cpt, struct ComputerEvent *event)
{
    for (int i = 0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        if (cpt->events[i].name == NULL)
        {
            memcpy(&cpt->events[i], event, sizeof(struct ComputerEvent));
            return i;
        }
  }
  return -1;
}

short init_computer_process_lists(void)
{
  struct ComputerProcessTypes *cpt;
  int i;
  for (i=0; i<COMPUTER_MODELS_COUNT; i++)
  {
    cpt = &ComputerProcessLists[i];
    memset(cpt, 0, sizeof(struct ComputerProcessTypes));
    memset(ComputerProcessListsNames[i], 0, LINEMSG_SIZE);
  }
  // Changing this to not subtract 1. This is possibly the bug for the highest computer model assignment
  // not appropriately being applied.
  for (i=0; i<COMPUTER_MODELS_COUNT; i++)
  {
    cpt = &ComputerProcessLists[i];
    cpt->name = ComputerProcessListsNames[i];
    computer_type_clear_processes(cpt);
    computer_type_clear_checks(cpt);
  }
  return true;
}

TbBool parse_computer_player_common_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variables
    // Initialize block data
    if (!flag_is_set(flags,CnfLd_AcceptPartial))
    {
        comp_player_conf.processes_count = 1;
        comp_player_conf.checks_count = 1;
        comp_player_conf.events_count = 1;
        comp_player_conf.computers_count = 1;
    }
    // Find the block
    char block_buf[COMMAND_WORD_LEN];
    sprintf(block_buf, "common");
    long pos = 0;
    int k = find_conf_block(buf, &pos, len, block_buf);
    if (k < 0)
    {
        if (!flag_is_set(flags,CnfLd_IgnoreErrors))
            WARNMSG("Block [%s] not found in %s file.",block_buf,config_textname);
        return false;
    }
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(compp_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, compp_common_commands);
        // Now store the config item in correct place
        if (cmd_num == ccr_endOfBlock) break; // if next block starts
        int n = 0;
        char word_buf[COMMAND_WORD_LEN];
        switch (cmd_num)
        {
        case 1: // COMPUTERASSISTS
  //TODO DLL_CLEANUP make it work when AI structures from DLL will no longer be used
            break;
        case 3: // CHECKSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= COMPUTER_CHECKS_TYPES_COUNT))
              {
                  comp_player_conf.checks_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // EVENTSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= COMPUTER_EVENTS_TYPES_COUNT))
              {
                  comp_player_conf.events_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // COMPUTERSCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= COMPUTER_MODELS_COUNT))
              {
                  comp_player_conf.computers_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 6: // SKIRMISHFIRST
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= COMPUTER_MODELS_COUNT))
              {
                  comp_player_conf.skirmish_first = k;
                  n++;
              }
            }
            break;
        case 7: // SKIRMISHLAST
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= COMPUTER_MODELS_COUNT))
              {
                  comp_player_conf.skirmish_last = k;
                  n++;
              }
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

short parse_computer_player_check_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct ComputerCheck *ccheck;
    int i;
    // Block name and parameter word store variables
    // Initialize the checks array
    const int arr_size = sizeof(computer_check_config_list)/sizeof(computer_check_config_list[0]);
    if(!flag_is_set(flags,CnfLd_AcceptPartial))
    {
      for (i=0; i < arr_size; i++)
      {
        ccheck = &computer_checks[i];
        computer_check_config_list[i].name[0] = '\0';
        computer_check_config_list[i].check = ccheck;
        ccheck->name = computer_check_names[i];
        LbMemorySet(computer_check_names[i], 0, LINEMSG_SIZE);
      }
    }

    strcpy(computer_check_names[0],"INCORRECT CHECK");
    // Load the file
    for (i=1; i < arr_size; i++)
    {
        char block_buf[32];
        sprintf(block_buf, "check%d", i);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if(!flag_is_set(flags,CnfLd_IgnoreErrors))
              WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
            continue;
      }
      ccheck = computer_check_config_list[i].check;
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(compp_check_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, compp_check_commands);
        // Now store the config item in correct place
        if (cmd_num == ccr_endOfBlock) break; // if next block starts
        if (flag_is_set(flags,CnfLd_ListOnly)) {
            // In "List only" mode, accept only name command
            if (cmd_num > 2) {
                cmd_num = 0;
            }
        }
        int n = 0;
        char word_buf[32];
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_whole(buf,&pos,len,ccheck->name,LINEMSG_SIZE) <= 0)
            {
                CONFWRNLOG("Could not read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
                break;
            }
            break;
        case 2: // MNEMONIC
            if (get_conf_parameter_whole(buf,&pos,len,computer_check_config_list[i].name,sizeof(computer_check_config_list[i].name)) <= 0)
            {
                CONFWRNLOG("Could not read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
                break;
            }
            break;
        case 3: // VALUES
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              ccheck->flags = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              ccheck->turns_interval = k;
              n++;
            }
            if (n < 2)
            {
                CONFWRNLOG("Could not recognize all of \"%s\" parameters in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // FUNCTIONS
            k = recognize_conf_parameter(buf,&pos,len,computer_check_func_type);
            if (k > 0)
            {
                ccheck->func = computer_check_func_list[k];
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Could not recognize all of \"%s\" parameters in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // PARAMS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              ccheck->param1 = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              ccheck->param2 = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              ccheck->param3 = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              ccheck->last_run_turn = k;
              n++;
            }
            if (n < 4)
            {
                CONFWRNLOG("Could not recognize all of \"%s\" parameters in [%s] block of %s file.",
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
    }
    return 1;
}

short parse_computer_player_event_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    struct ComputerEvent *cevent;
    int i;
    // Block name and parameter word store variables
    // Initialize the events array
    const int arr_size = (int)(sizeof(computer_event_config_list)/sizeof(computer_event_config_list[0]));
    if(!flag_is_set(flags,CnfLd_AcceptPartial))
    {
      for (i=0; i < arr_size; i++)
      {
        cevent = &computer_events[i];
        computer_event_config_list[i].name[0] = '\0';
        computer_event_config_list[i].event = cevent;
        cevent->name = computer_event_names[i];
        LbMemorySet(computer_event_names[i], 0, LINEMSG_SIZE);
      }
      strcpy(computer_event_names[0],"INCORRECT EVENT");
    }

    // Load the file
    for (i=1; i < arr_size; i++)
    {
        char block_buf[32];
        sprintf(block_buf, "event%d", i);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if(!flag_is_set(flags,CnfLd_IgnoreErrors))
              WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
            continue;
      }
      cevent = computer_event_config_list[i].event;
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(compp_event_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, compp_event_commands);
        // Now store the config item in correct place
        if (cmd_num == ccr_endOfBlock) break; // if next block starts
        if (flag_is_set(flags,CnfLd_ListOnly)) {
            // In "List only" mode, accept only name command
            if (cmd_num > 2) {
                cmd_num = 0;
            }
        }
        int n = 0;
        char word_buf[32];
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_whole(buf,&pos,len,cevent->name,LINEMSG_SIZE) <= 0)
            {
                CONFWRNLOG("Could not read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
                break;
            }
            break;
        case 2: // MNEMONIC
            if (get_conf_parameter_whole(buf,&pos,len,computer_event_config_list[i].name,sizeof(computer_event_config_list[i].name)) <= 0)
            {
                CONFWRNLOG("Could not read \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
                break;
            }
            break;
        case 3: // VALUES
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cevent->cetype = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cevent->mevent_kind = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cevent->test_interval = k;
              n++;
            }
            if (n < 3)
            {
                CONFWRNLOG("Could not recognize all of \"%s\" parameters in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 4: // FUNCTIONS
            k = recognize_conf_parameter(buf,&pos,len,computer_event_func_type);
            if (k > 0)
            {
                cevent->func_event = computer_event_func_list[k];
                n++;
            }
            k = recognize_conf_parameter(buf,&pos,len,computer_event_test_func_type);
            if (k > 0)
            {
                cevent->func_test = computer_event_test_func_list[k];
                n++;
            }
            if (n < 2)
            {
                CONFWRNLOG("Could not recognize all of \"%s\" parameters in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 5: // PROCESS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_computer_process_config_list_index_mnem(word_buf);
              if (k > 0)
              {
                cevent->process = &comp_player_conf.process_types[k];
              } else
              {
                  CONFWRNLOG("Could not recognize \"%s\" parameter in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),block_buf,config_textname);
              }
            }
            break;
        case 6: // PARAMS
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cevent->param1 = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cevent->param2 = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cevent->param3 = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cevent->last_test_gameturn = k;
              n++;
            }
            if (n < 4)
            {
                CONFWRNLOG("Could not recognize all of \"%s\" parameters in [%s] block of %s file.",
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
    }
    return 1;
}

short write_computer_player_check_to_log(struct ComputerCheck *ccheck)
{
  JUSTMSG("[checkXX]");
  JUSTMSG("Name = %s",ccheck->name);
  JUSTMSG("Mnemonic = %s","XX");
  JUSTMSG("Values = %lu %ld",ccheck->flags,ccheck->turns_interval);
  JUSTMSG("Functions = %p",ccheck->func);
  JUSTMSG("Params = %ld %ld %ld %ld",ccheck->param1,ccheck->param2,ccheck->param3,ccheck->last_run_turn);
  return true;
}

short write_computer_player_event_to_log(const struct ComputerEvent *event)
{
  JUSTMSG("[eventXX]");
  JUSTMSG("Name = %s",event->name);
  JUSTMSG("Mnemonic = %s","XX");
  JUSTMSG("Values = %lu %lu %ld",event->cetype,event->mevent_kind,event->test_interval);
  JUSTMSG("Functions = %p %p",event->func_event,event->func_test);
  JUSTMSG("Params = %ld %ld %ld %ld",event->param1,event->param2,event->param3,event->last_test_gameturn);
  return true;
}

short parse_computer_player_computer_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
    // Block name and parameter word store variable
    // The -1 was possibly making the array size 1-too-small. It was originally
    // const int arr_size = (int)(sizeof(ComputerProcessLists)/sizeof(ComputerProcessLists[0]))-1;
    const int arr_size = (int)(sizeof(ComputerProcessLists)/sizeof(ComputerProcessLists[0]));
    for (int i = 0; i < arr_size; i++)
    {
        struct ComputerProcessTypes* cpt;
        char block_buf[32];
        sprintf(block_buf, "computer%d", i);
        long pos = 0;
        int k = find_conf_block(buf, &pos, len, block_buf);
        if (k < 0)
        {
            if (!flag_is_set(flags,CnfLd_AcceptPartial))
            {
                WARNMSG("Block [%s] not found in %s file.", block_buf, config_textname);
            }
            else
            {
                cpt = &ComputerProcessLists[i];
                computer_type_clear_events(cpt);
                for (int j = 0; j < COMPUTER_EVENTS_TYPES_COUNT; j++)
                {
                    if ((strcmp(computer_event_config_list[j].event->name, "UNUSED") != 0) && (computer_event_config_list[j].event->name != NULL))
                    {
                        computer_type_add_event(cpt, computer_event_config_list[j].event);
                    }
                }
            }
            continue;
        }
      cpt = &ComputerProcessLists[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(compp_computer_commands,cmd_num)
      while (pos<len)
      {
        // Finding command number in this line
        int cmd_num = recognize_conf_command(buf, &pos, len, compp_computer_commands);
        // Now store the config item in correct place
        if (cmd_num == ccr_endOfBlock) break; // if next block starts
        if (flag_is_set(flags,CnfLd_ListOnly)) {

            // In "List only" mode, accept only name command
            if (cmd_num > 1) {
                cmd_num = 0;
            }
        }
        int n = 0;
        char word_buf[32];
        switch (cmd_num)
        {
        case 1: // NAME
            if (get_conf_parameter_whole(buf,&pos,len,cpt->name,LINEMSG_SIZE) <= 0)
            {
                CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
                break;
            }
            break;
        case 2: // VALUES
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cpt->dig_stack_size = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cpt->processes_time = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cpt->click_rate = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cpt->max_room_build_tasks = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cpt->turn_begin = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cpt->sim_before_dig = k;
              n++;
            }
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              cpt->drop_delay = k;
              n++;
            }
            if (n < 7)
            {
                CONFWRNLOG("Could not recognize all of \"%s\" parameters in [%s] block of %s file.",
                    COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 3: // PROCESSES
            computer_type_clear_processes(cpt);
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_computer_process_config_list_index_mnem(word_buf);
              if (k <= 0)
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                continue;
              }
              n = computer_type_add_process(cpt, &comp_player_conf.process_types[k]);
              if (n < 0) {
                  CONFWRNLOG("Could not add \"%s\" list element \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 4: // CHECKS
            computer_type_clear_checks(cpt);
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_computer_check_config_list_index_mnem(word_buf);
              if (k <= 0)
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                  continue;
              }
              n = computer_type_add_check(cpt, computer_check_config_list[k].check);
              if (n < 0)
              {
                  CONFWRNLOG("Could not add \"%s\" list element \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 5: // EVENTS
            computer_type_clear_events(cpt);
            while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = get_computer_event_config_list_index_mnem(word_buf);
              if (k <= 0)
              {
                  CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
                  continue;
              }
              n = computer_type_add_event(cpt, computer_event_config_list[k].event);
              if (n < 0)
              {
                  CONFWRNLOG("Could not add \"%s\" list element \"%s\" in [%s] block of %s file.",
                      COMMAND_TEXT(cmd_num),word_buf,block_buf,config_textname);
              }
            }
            break;
        case 6: // NAMETEXTID
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                  //TODO CONFIG Add when ComputerProcessTypes can be changed
                  //cpt->name_stridx = k;
                  n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
            break;
        case 7: // TOOLTIPTEXTID
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
              {
                  //TODO CONFIG Add when ComputerProcessTypes can be changed
                  //cpt->tooltip_stridx = k;
                  n++;
              }
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
    }
    return 1;
}

TbBool load_computer_player_config_file(const char *textname, const char *fname, unsigned short flags)
{
    SYNCDBG(8, "Starting");
    if (!flag_is_set(flags,CnfLd_AcceptPartial))
    {
        init_computer_process_lists();
    }
    // Load the config file
    long len = LbFileLengthRnc(fname);
    if (len < 2)
    {
        if (!flag_is_set(flags,CnfLd_IgnoreErrors))
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
        parse_computer_player_common_blocks(buf, len, textname, flags);
        parse_named_field_blocks(buf, len, textname, flags, &compp_process_named_fields_set);
        parse_computer_player_check_blocks(buf, len, textname, flags);
        parse_computer_player_event_blocks(buf, len, textname, flags);
        parse_computer_player_computer_blocks(buf, len, textname, flags);
    }
    //Freeing and exiting
    free(buf);
    return true;
}


TbBool load_computer_player_config(unsigned short flags)
{
    static const char config_global_textname[] = "global Computer Player config";
    static const char config_campgn_textname[] = "campaign Computer Player config";
    static const char config_level_textname[] = "level Computer Player config";
    char* fname = prepare_file_path(FGrp_FxData, keeper_compplayer_file);
    TbBool result = load_computer_player_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,keeper_compplayer_file);
    if (strlen(fname) > 0)
    {
        load_computer_player_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), keeper_compplayer_file);
    if (strlen(fname) > 0)
    {
        load_computer_player_config_file(config_level_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
