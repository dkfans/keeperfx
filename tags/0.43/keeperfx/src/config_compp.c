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
#include "config_compp.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "player_computer.h"
#include "thing_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const struct NamedCommand compp_common_commands[] = {
  {"COMPUTERASSISTS", 1},
  {"PROCESSESCOUNT",  2},
  {"CHECKSCOUNT",     3},
  {"EVENTSCOUNT",     4},
  {"COMPUTERSCOUNT",  5},
  {NULL,              0},
  };

const struct NamedCommand compp_process_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PARAMS",          4},
  {"MNEMONIC",        5},
  {NULL,              0},
  };

const struct NamedCommand compp_check_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PARAMS",          4},
  {"MNEMONIC",        5},
  {NULL,              0},
  };

const struct NamedCommand compp_event_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"FUNCTIONS",       3},
  {"PROCESS",         4},
  {"PARAMS",          5},
  {"MNEMONIC",        6},
  {NULL,              0},
  };

const struct NamedCommand compp_computer_commands[] = {
  {"NAME",            1},
  {"VALUES",          2},
  {"PROCESSES",       3},
  {"CHECKS",          4},
  {"EVENTS",          5},
  {NULL,              0},
  };

const char keeper_compplayer_file[]="keepcompp.cfg";

/******************************************************************************/
DLLIMPORT struct ComputerProcessTypes _DK_ComputerProcessLists[14];
//#define ComputerProcessLists _DK_ComputerProcessLists
/******************************************************************************/
ComputerName computer_check_names[COMPUTER_CHECKS_TYPES_COUNT];
struct ComputerCheck computer_checks[COMPUTER_CHECKS_TYPES_COUNT];
struct ComputerCheckMnemonic computer_check_config_list[COMPUTER_CHECKS_TYPES_COUNT];

ComputerName computer_event_names[COMPUTER_EVENTS_TYPES_COUNT];
struct ComputerEvent computer_events[COMPUTER_EVENTS_TYPES_COUNT];
struct ComputerEventMnemonic computer_event_config_list[COMPUTER_EVENTS_TYPES_COUNT];

ComputerName ComputerProcessListsNames[COMPUTER_PROCESS_LISTS_COUNT];
struct ComputerProcessTypes ComputerProcessLists[COMPUTER_PROCESS_LISTS_COUNT];

struct ComputerPlayerConfig comp_player_conf;

/******************************************************************************/

int get_computer_process_config_list_index_prc(struct ComputerProcess *process)
{
  int i;
  for (i=1; i <= comp_player_conf.processes_count; i++)
  {
    if (computer_process_config_list[i].process == process)
      return i;
  }
  return 0;
}

int get_computer_process_config_list_index_mnem(const char *mnemonic)
{
  int i;
  for (i=1; i <= comp_player_conf.processes_count; i++)
  {
    if (stricmp(computer_process_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

int get_computer_check_config_list_index_mnem(const char *mnemonic)
{
  int i;
  const int arr_size = (int)(sizeof(computer_check_config_list)/sizeof(computer_check_config_list[0]));
  for (i=1; i < arr_size; i++)
  {
    if (stricmp(computer_check_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

int get_computer_event_config_list_index_mnem(const char *mnemonic)
{
  int i;
  const int arr_size = (int)(sizeof(computer_event_config_list)/sizeof(computer_event_config_list[0]));
  for (i=1; i < arr_size; i++)
  {
    if (strcasecmp(computer_event_config_list[i].name, mnemonic) == 0)
      return i;
  }
  return 0;
}

struct ComputerProcessTypes *get_computer_process_type_template(long cpt_idx)
{
    if ((cpt_idx < 0) || (cpt_idx >= COMPUTER_PROCESS_LISTS_COUNT))
        cpt_idx = 0;
  return &ComputerProcessLists[cpt_idx];
}

TbBool computer_type_clear_processes(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_PROCESSES_COUNT; i++)
  {
    cpt->processes[i] = NULL;
  }
  return true;
}

int computer_type_add_process(struct ComputerProcessTypes *cpt, struct ComputerProcess *process)
{
  int i;
  for (i=0; i<COMPUTER_PROCESSES_COUNT; i++)
  {
      if (cpt->processes[i] == NULL)
      {
        cpt->processes[i] = process;
        return i;
      }
  }
  return -1;
}

short computer_type_clear_checks(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_CHECKS_COUNT; i++)
  {
    LbMemorySet(&cpt->checks[i], 0, sizeof(struct ComputerCheck));
  }
  return true;
}

int computer_type_add_check(struct ComputerProcessTypes *cpt, struct ComputerCheck *check)
{
  int i;
  for (i=0; i<COMPUTER_CHECKS_COUNT; i++)
  {
      if (cpt->checks[i].name == NULL)
      {
        LbMemoryCopy(&cpt->checks[i], check, sizeof(struct ComputerCheck));
        return i;
      }
  }
  return -1;
}

short computer_type_clear_events(struct ComputerProcessTypes *cpt)
{
  int i;
  for (i=0; i<COMPUTER_EVENTS_COUNT; i++)
  {
    LbMemorySet(&cpt->events[i], 0, sizeof(struct ComputerEvent));
  }
  return true;
}

int computer_type_add_event(struct ComputerProcessTypes *cpt, struct ComputerEvent *event)
{
  int i;
  for (i=0; i<COMPUTER_EVENTS_COUNT; i++)
  {
      if (cpt->events[i].name == NULL)
      {
        LbMemoryCopy(&cpt->events[i], event, sizeof(struct ComputerEvent));
        return i;
      }
  }
  return -1;
}

short init_computer_process_lists(void)
{
  struct ComputerProcessTypes *cpt;
  int i;
  for (i=0; i<COMPUTER_PROCESS_LISTS_COUNT; i++)
  {
    cpt = &ComputerProcessLists[i];
    LbMemorySet(cpt, 0, sizeof(struct ComputerProcessTypes));
    LbMemorySet(ComputerProcessListsNames[i], 0, LINEMSG_SIZE);
  }
  for (i=0; i<COMPUTER_PROCESS_LISTS_COUNT-1; i++)
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
    long pos;
    int k,n;
    int cmd_num;
    // Block name and parameter word store variables
    char block_buf[COMMAND_WORD_LEN];
    char word_buf[COMMAND_WORD_LEN];
    // Initialize block data
    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        comp_player_conf.processes_count = 1;
        comp_player_conf.checks_count = 1;
        comp_player_conf.events_count = 1;
        comp_player_conf.computers_count = 1;
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
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(compp_common_commands,cmd_num)
    while (pos<len)
    {
        // Finding command number in this line
        cmd_num = recognize_conf_command(buf,&pos,len,compp_common_commands);
        // Now store the config item in correct place
        if (cmd_num == -3) break; // if next block starts
        n = 0;
        switch (cmd_num)
        {
        case 1: // COMPUTERASSISTS
  //TODO DLL_CLEANUP make it work when AI structures from DLL will no longer be used
            break;
        case 2: // PROCESSESCOUNT
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if ((k > 0) && (k <= COMPUTER_PROCESS_TYPES_COUNT))
              {
                  comp_player_conf.processes_count = k;
                n++;
              }
            }
            if (n < 1)
            {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%s] block of %s file.",
                  COMMAND_TEXT(cmd_num),block_buf,config_textname);
            }
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
              if ((k > 0) && (k <= COMPUTER_PROCESS_LISTS_COUNT))
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

short parse_computer_player_process_blocks(char *buf,long len)
{
  struct ComputerProcess *process;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variable
  char block_buf[32];
  char word_buf[32];
  for (i=1; i <= comp_player_conf.processes_count; i++)
  {
    sprintf(block_buf,"process%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    process = computer_process_config_list[i].process;
    process->parent = NULL;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_process_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          //For now, let's leave default names.
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_4 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_8 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_C = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_10 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_14 = k;
            n++;
          }
          if (n < 5)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_check = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_setup = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_task = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_complete = computer_process_func_list[k];
              n++;
          }
          k = recognize_conf_parameter(buf,&pos,len,computer_process_func_type);
          if (k > 0)
          {
              process->func_pause = computer_process_func_list[k];
              n++;
          }
          if (n < 5)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PARAMS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_30 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_34 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_38 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_3C = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->field_40 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            process->flags = k;
            n++;
          }
          if (n < 6)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 5: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_process_config_list[i].name,sizeof(computer_process_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short parse_computer_player_check_blocks(char *buf,long len)
{
  struct ComputerCheck *check;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[32];
  char word_buf[32];
  // Initialize the checks array
  const int arr_size = sizeof(computer_check_config_list)/sizeof(computer_check_config_list[0]);
  for (i=0; i < arr_size; i++)
  {
    check = &computer_checks[i];
    computer_check_config_list[i].name[0] = '\0';
    computer_check_config_list[i].check = check;
    check->name = computer_check_names[i];
    LbMemorySet(computer_check_names[i], 0, LINEMSG_SIZE);
  }
  strcpy(computer_check_names[0],"INCORRECT CHECK");
  // Load the file
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"check%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    check = computer_check_config_list[i].check;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_check_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,check->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->flags = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->turns_interval = k;
            n++;
          }
          if (n < 2)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
          k = recognize_conf_parameter(buf,&pos,len,computer_check_func_type);
          if (k > 0)
          {
              check->func = computer_check_func_list[k];
              n++;
          }
          if (n < 1)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PARAMS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param1 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param2 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param3 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            check->param4 = k;
            n++;
          }
          if (n < 4)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 5: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_check_config_list[i].name,sizeof(computer_check_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short parse_computer_player_event_blocks(char *buf,long len)
{
  struct ComputerEvent *cevent;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variables
  char block_buf[32];
  char word_buf[32];
  // Initialize the events array
  const int arr_size = (int)(sizeof(computer_event_config_list)/sizeof(computer_event_config_list[0]));
  for (i=0; i < arr_size; i++)
  {
    cevent = &computer_events[i];
    computer_event_config_list[i].name[0] = '\0';
    computer_event_config_list[i].event = cevent;
    cevent->name = computer_event_names[i];
    LbMemorySet(computer_event_names[i], 0, LINEMSG_SIZE);
  }
  strcpy(computer_event_names[0],"INCORRECT EVENT");
  // Load the file
  for (i=1; i < arr_size; i++)
  {
    sprintf(block_buf,"event%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    cevent = computer_event_config_list[i].event;
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_event_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,cevent->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->cetype = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cevent->field_8 = k;
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
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // FUNCTIONS
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
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","FUNCTIONS",block_buf);
          }
          break;
      case 4: // PROCESS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_process_config_list_index_mnem(word_buf);
            if (k > 0)
            {
              cevent->process = computer_process_config_list[k].process;
            } else
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","PROCESS",word_buf);
            }
          }
          break;
      case 5: // PARAMS
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
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","PARAMS",block_buf);
          }
          break;
      case 6: // MNEMONIC
          if (get_conf_parameter_whole(buf,&pos,len,computer_event_config_list[i].name,sizeof(computer_event_config_list[i].name)) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","MNEMONIC",block_buf);
            break;
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

short write_computer_player_check_to_log(struct ComputerCheck *check)
{
  JUSTMSG("[checkXX]");
  JUSTMSG("Name = %s",check->name);
  JUSTMSG("Mnemonic = %s","XX");
  JUSTMSG("Values = %d %d",check->flags,check->turns_interval);
  JUSTMSG("Functions = %x",check->func);
  JUSTMSG("Params = %d %d %d %d",check->param1,check->param2,check->param3,check->param4);
  return true;
}

short write_computer_player_event_to_log(struct ComputerEvent *event)
{
  JUSTMSG("[eventXX]");
  JUSTMSG("Name = %s",event->name);
  JUSTMSG("Mnemonic = %s","XX");
  JUSTMSG("Values = %d %d %d",event->cetype,event->field_8,event->test_interval);
  JUSTMSG("Functions = %x %x",event->func_event,event->func_test);
  JUSTMSG("Params = %d %d %d %d",event->param1,event->param2,event->param3,event->last_test_gameturn);
  return true;
}

short parse_computer_player_computer_blocks(char *buf,long len)
{
  struct ComputerProcessTypes *cpt;
  long pos;
  int i,k,n;
  int cmd_num;
  // Block name and parameter word store variable
  char block_buf[32];
  char word_buf[32];
  const int arr_size = (int)(sizeof(ComputerProcessLists)/sizeof(ComputerProcessLists[0]))-1;
  for (i=0; i < arr_size; i++)
  {
    sprintf(block_buf,"computer%d",i);
    pos = 0;
    k = find_conf_block(buf,&pos,len,block_buf);
    if (k < 0)
    {
      WARNMSG("Block [%s] not found in Computer Player file.",block_buf);
      continue;
    }
    cpt = &ComputerProcessLists[i];
    while (pos<len)
    {
      // Finding command number in this line
      cmd_num = recognize_conf_command(buf,&pos,len,compp_computer_commands);
      // Now store the config item in correct place
      if (cmd_num == -3) break; // if next block starts
      n = 0;
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_whole(buf,&pos,len,cpt->name,LINEMSG_SIZE) <= 0)
          {
            WARNMSG("Couldn't read \"%s\" parameter in [%s] block of Computer file.","NAME",block_buf);
            break;
          }
          break;
      case 2: // VALUES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_4 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_8 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_C = k;
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
            cpt->field_14 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_18 = k;
            n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            cpt->field_1C = k;
            n++;
          }
          if (n < 7)
          {
            WARNMSG("Couldn't recognize all \"%s\" parameters in [%s] block of Computer file.","VALUES",block_buf);
          }
          break;
      case 3: // PROCESSES
          computer_type_clear_processes(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_process_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","PROCESSES",word_buf);
              continue;
            }
            n = computer_type_add_process(cpt, computer_process_config_list[k].process);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","PROCESSES",word_buf);
          }
          break;
      case 4: // CHECKS
          computer_type_clear_checks(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_check_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","CHECKS",word_buf);
              continue;
            }
            n = computer_type_add_check(cpt, computer_check_config_list[k].check);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","CHECKS",word_buf);
          }
          break;
      case 5: // EVENTS
          computer_type_clear_events(cpt);
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_computer_event_config_list_index_mnem(word_buf);
            if (k <= 0)
            {
              WARNMSG("Couldn't recognize \"%s\" parameter \"%s\" in Computer file.","EVENTS",word_buf);
              continue;
            }
            n = computer_type_add_event(cpt, computer_event_config_list[k].event);
            if (n < 0)
              WARNMSG("Couldn't add \"%s\" list element \"%s\" when reading Computer file.","EVENTS",word_buf);
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          WARNMSG("Unrecognized command in Computer Player file, starting on byte %d.",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  return 1;
}

TbBool load_computer_player_config(unsigned short flags)
{
    static const char *textname = "Computer Player";
    const char *fname;
    char *buf;
    long len;
    init_computer_process_lists();
    // Load the config file
    fname = prepare_file_path(FGrp_FxData,keeper_compplayer_file);
    len = LbFileLengthRnc(fname);
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
    buf = (char *)LbMemoryAlloc(len+256);
    if (buf == NULL)
      return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    if (len>0)
    {
        parse_computer_player_common_blocks(buf,len, textname, flags);
        parse_computer_player_process_blocks(buf,len);
        parse_computer_player_check_blocks(buf,len);
        parse_computer_player_event_blocks(buf,len);
        parse_computer_player_computer_blocks(buf,len);
    }
    //Freeing and exiting
    LbMemoryFree(buf);
    // Hack to synchronize local structure with the one inside DLL.
    // Remove when it's not needed anymore.
    LbMemoryCopy(_DK_ComputerProcessLists,ComputerProcessLists,13*sizeof(struct ComputerProcessTypes));
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
