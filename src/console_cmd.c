/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file console_cmd.c
 *     
 * @par Purpose:
 *     Define various console commans
 * @par Comment:
 *     None
 * @author   Sim
 * @date     07 Jul 2020 - 07 Jul 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "console_cmd.h"
#include "globals.h"

#include "bflib_datetm.h"
#include "dungeon_data.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "gui_boxmenu.h"
#include "gui_msgs.h"
#include "keeperfx.hpp"
#include "player_computer.h"
#include "slab_data.h"

#ifdef __cplusplus
extern "C" {
#endif

static struct GuiBoxOption cmd_comp_procs_data[COMPUTER_PROCESSES_COUNT + 3] = {
  {"!", 1, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0 }
  };

static char cmd_comp_procs_label[COMPUTER_PROCESSES_COUNT + 2][COMMAND_WORD_LEN + 8];

struct GuiBoxOption cmd_comp_checks_data[COMPUTER_CHECKS_COUNT + 1] = { 0 };
static char cmd_comp_checks_label[COMPUTER_CHECKS_COUNT][COMMAND_WORD_LEN + 8];

struct GuiBoxOption cmd_comp_events_data[COMPUTER_EVENTS_COUNT + 1] = { 0 };
  static char cmd_comp_events_label[COMPUTER_EVENTS_COUNT][COMMAND_WORD_LEN + 8];

static long cmd_comp_procs_click(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *args)
{
    struct Computer2 *comp;
    comp = get_computer_player(args[0]);
    struct ComputerProcess* cproc = &comp->processes[args[1]];

    if (cproc->flags & ComProc_Unkn0020)
        message_add_fmt(args[0], "resuming %s", cproc->name?cproc->name:"(null)");
    else
        message_add_fmt(args[0], "suspending %s", cproc->name?cproc->name:"(null)");
    
    cproc->flags ^= ComProc_Unkn0020; // Suspend, but do not update running time
    return 1;
}

static long cmd_comp_procs_update(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *args)
{
    struct Computer2 *comp = get_computer_player(args[0]);
    int i = 0;

    for (; i < args[1]; i++)
    {
        struct ComputerProcess* cproc = &comp->processes[i];
        if (cproc != NULL)
        {
            char *label = (char*)goptn[i].label;
            sprintf(label, "%02lx", cproc->flags);
            label[2] = ' ';
        }
    }

    sprintf(cmd_comp_procs_label[i], "comp=%d, wait=%ld", 0, comp->gameturn_wait);
    return 1;
}

static long cmd_comp_checks_update(struct GuiBox *gbox, struct GuiBoxOption *goptn, long *args)
{
    struct Computer2 *comp = get_computer_player(args[0]);
    int i = 0;

    for (; i < args[1]; i++)
    {
        struct ComputerCheck* check = &comp->checks[i];
        if (check != NULL)
        {
            char *label = (char*)goptn[i].label;
            sprintf(label, "%02lx", check->flags);
            label[2] = ' ';
        }
    }
    return 1;
}

int cmd_comp_list(PlayerNumber plyr_idx, int max_count,
    struct GuiBoxOption *data_list, char label_list[][COMMAND_WORD_LEN + 8],
    const char *(*get_name)(struct Computer2 *, int),
    unsigned long (*get_flags)(struct Computer2 *, int),
    Gf_OptnBox_4Callback click_fn
    )
{
    close_creature_cheat_menu();
    //gui_cheat_box
    int i = 0;
    struct Computer2 *comp;
    comp = get_computer_player(plyr_idx);
    for (; i < max_count; i++)
    {
        unsigned long flags = get_flags(comp, i); 
        const char *name = get_name(comp, i);
        if (name == NULL)
            sprintf(label_list[i], "%02lx %s", flags, "(null2)");  
        else
          sprintf(label_list[i], "%02lx %s", flags, name);
        data_list[i].label = label_list[i];

        data_list[i].numfield_4 = 1;
        data_list[i].callback = click_fn;
        data_list[i].field_D = plyr_idx;
        data_list[i].field_11 = max_count;
        data_list[i].field_19 = plyr_idx;
        data_list[i].field_1D = i;
    }
    data_list[i].label = "!";
    data_list[i].numfield_4 = 0;
    return i;
}

static const char *get_process_name(struct Computer2 *comp, int i)
{
    return comp->processes[i].name;
}
static unsigned long  get_process_flags(struct Computer2 *comp, int i)
{
    return comp->processes[i].flags;
}
static void cmd_comp_procs(PlayerNumber plyr_idx)
{
    int i = cmd_comp_list(plyr_idx, COMPUTER_PROCESSES_COUNT, 
        cmd_comp_procs_data, cmd_comp_procs_label,
        &get_process_name, &get_process_flags,
        &cmd_comp_procs_click);

    cmd_comp_procs_data[0].active_cb = &cmd_comp_procs_update;
    
    cmd_comp_procs_data[i].label = "======";
    cmd_comp_procs_data[i].numfield_4 = 1;
    i++;
    cmd_comp_procs_data[i].label = cmd_comp_procs_label[COMPUTER_PROCESSES_COUNT];
    cmd_comp_procs_data[i].numfield_4 = 1;
    i++;
    cmd_comp_procs_data[i].label = "!";
    cmd_comp_procs_data[i].numfield_4 = 0;

    gui_cheat_box = gui_create_box(my_mouse_x, 20, cmd_comp_procs_data);
}

static const char *get_event_name(struct Computer2 *comp, int i)
{
    return comp->events[i].name;
}
static unsigned long  get_event_flags(struct Computer2 *comp, int i)
{
    return 0;
}
static void cmd_comp_events(PlayerNumber plyr_idx)
{
    cmd_comp_list(plyr_idx, COMPUTER_EVENTS_COUNT, 
        cmd_comp_events_data, cmd_comp_events_label,
        &get_event_name, &get_event_flags, NULL);
    cmd_comp_events_data[0].active_cb = NULL;

    gui_cheat_box = gui_create_box(my_mouse_x, 20, cmd_comp_events_data);
}


static long cmd_comp_checks_click(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, long *args)
{
    struct Computer2 *comp;
    comp = get_computer_player(args[0]);
    struct ComputerCheck* cproc = &comp->checks[args[1]];

    if (cproc->flags & ComChk_Unkn0001)
        message_add_fmt(args[0], "resuming %s", cproc->name?cproc->name:"(null)");
    else
        message_add_fmt(args[0], "suspending %s", cproc->name?cproc->name:"(null)");
    
    cproc->flags ^= ComChk_Unkn0001;
    return 1;
}
static const char *get_check_name(struct Computer2 *comp, int i)
{
    return comp->checks[i].name;
}
static unsigned long  get_check_flags(struct Computer2 *comp, int i)
{
    return comp->checks[i].flags;
}

static void cmd_comp_checks(PlayerNumber plyr_idx)
{
    cmd_comp_list(plyr_idx, COMPUTER_CHECKS_COUNT, 
        cmd_comp_checks_data, cmd_comp_checks_label,
        &get_check_name, &get_check_flags, &cmd_comp_checks_click);
    cmd_comp_checks_data[0].active_cb = NULL;

    gui_cheat_box = gui_create_box(my_mouse_x, 20, cmd_comp_checks_data);
}

static char *cmd_strtok(char *tail)
{
    char* next = strchr(tail,' ');
    if (next == NULL)
        return next;
    next[0] = '\0';
    next++; // it was space
    while (*next == ' ')
      next++;
    return next;
}

TbBool cmd_exec(PlayerNumber plyr_idx, char *msg)
{
    SYNCDBG(2,"Command %d: %s",(int)plyr_idx, msg);
    const char * parstr = msg + 1;
    const char * pr2str = cmd_strtok(msg + 1);
    if (strcmp(parstr, "stats") == 0)
    {
      message_add_fmt(plyr_idx, "Now time is %d, last loop time was %d",LbTimerClock(),last_loop_time);
      message_add_fmt(plyr_idx, "clock is %d, requested fps is %d",clock(),game.num_fps);
      return true;
    } else if (strcmp(parstr, "quit") == 0)
    {
        if (is_my_player_number(plyr_idx))
        {
            quit_game = 1;
            exit_keeper = 1;
        }
        return true;
    } else if (strcmp(parstr, "show.turn") == 0)
    {
        message_add_fmt(plyr_idx, "turn %ld", game.play_gameturn);
        return true;
    } else if (strcmp(parstr, "show.ticks") == 0)
    {
        if (is_my_player_number(plyr_idx))
        {
            game.flags_gui ^= GGUI_ShowTickTime;
        }
        return true;
    } else if ((game.flags_font & FFlg_AlexCheat) != 0)
    {
        if (strcmp(parstr, "compuchat") == 0)
        {
            if (pr2str == NULL)
                return false;

            if (!is_my_player_number(plyr_idx))
                return false;

            if ((strcmp(pr2str,"scarce") == 0) || (strcmp(pr2str,"1") == 0))
            {
                for (int i = 0; i < PLAYERS_COUNT; i++)
                {
                    if ((i == game.hero_player_num)
                        || (plyr_idx == game.neutral_player_num))
                        continue;
                    struct Computer2* comp = get_computer_player(i);
                    if (player_exists(get_player(i)) && (!computer_player_invalid(comp)))
                        message_add_fmt(i, "Ai model %d", (int)comp->model);
                }
                gameadd.computer_chat_flags = CChat_TasksScarce;
            } else
            if ((strcmp(pr2str,"frequent") == 0) || (strcmp(pr2str,"2") == 0))
            {
                message_add_fmt(plyr_idx, "%s", pr2str);
                gameadd.computer_chat_flags = CChat_TasksScarce|CChat_TasksFrequent;
            } else
            {
                message_add(plyr_idx, "none");
                gameadd.computer_chat_flags = CChat_None;
            }
            return true;
        } else if (strcmp(parstr, "comp.procs") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            if (!is_my_player_number(plyr_idx))
                return false;
            cmd_comp_procs(id);
            return true;
        } else if (strcmp(parstr, "comp.events") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            if (!is_my_player_number(plyr_idx))
                return false;
            cmd_comp_events(id);
            return true;
        } else if (strcmp(parstr, "comp.checks") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            if (!is_my_player_number(plyr_idx))
                return false;
            cmd_comp_checks(id);
            return true;
        } else if (strcmp(parstr, "reveal") == 0)
        {
            if (!is_my_player_number(plyr_idx))
                return false;
            struct PlayerInfo* player = get_my_player();
            reveal_whole_map(player);
            return true;
        } else if (strcmp(parstr, "comp.kill") == 0)
        {
            if (pr2str == NULL)
                return false;
            int id = atoi(pr2str);
            if (id < 0 || id > PLAYERS_COUNT)
                return false;
            if (!is_my_player_number(plyr_idx))
                return false;
            struct Thing* thing = get_player_soul_container(id);
            thing->health = 0;
        } else if (strcmp(parstr, "comp.me") == 0)
        {
            if (!is_my_player_number(plyr_idx))
                return false;
            struct PlayerInfo* player = get_player(plyr_idx);
            if (pr2str == NULL)
                return false;
            if (!setup_a_computer_player(plyr_idx, atoi(pr2str))) {
                message_add(plyr_idx, "unable to set assistant");
            } else
                message_add_fmt(plyr_idx, "computer assistant is %d", atoi(pr2str));
            return true;
        } else if (strcmp(parstr, "give.trap") == 0)
        {
            if (!is_my_player_number(plyr_idx))
                return false;
            int id = atoi(pr2str);
            if (id <= 0 || id > trapdoor_conf.trap_types_count)
                return false;
            command_add_value(Cmd_TRAP_AVAILABLE, plyr_idx, id, 1, 1);
            update_trap_tab_to_config();
            message_add(plyr_idx, "done!");
            return true;
        } else if (strcmp(parstr, "give.door") == 0)
        {
            if (!is_my_player_number(plyr_idx))
                return false;
            int id = atoi(pr2str);
            if (id <= 0 || id > trapdoor_conf.door_types_count)
                return false;
            script_process_value(Cmd_DOOR_AVAILABLE, plyr_idx, id, 1, 1);
            update_trap_tab_to_config();
            message_add(plyr_idx, "done!");
            return true;
        }
    }
    return false;
}


#ifdef __cplusplus
}
#endif
