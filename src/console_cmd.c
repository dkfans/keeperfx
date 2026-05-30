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
#include "pre_inc.h"
#include "console_cmd.h"
#include "globals.h"

#include "actionpt.h"
#include "bflib_datetm.h"
#include "bflib_sound.h"
#include "bflib_sndlib.h"
#include "config.h"
#include "config_keeperfx.h"
#include "config_campaigns.h"
#include "config_effects.h"
#include "config_magic.h"
#include "config_rules.h"
#include "config_settings.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "creature_instances.h"
#include "creature_jobs.h"
#include "creature_states.h"
#include "creature_states_hero.h"
#include "dungeon_data.h"
#include "frontend.h"
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_tabs.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "gui_boxmenu.h"
#include "gui_msgs.h"
#include "gui_soundmsgs.h"
#include "keeperfx.hpp"
#include "lvl_script_lib.h"
#include "map_blocks.h"
#include "map_columns.h"
#include "map_utils.h"
#include "packets.h"
#include "player_computer.h"
#include "player_instances.h"
#include "config_players.h"
#include "player_utils.h"
#include "room_data.h"
#include "room_treasure.h"
#include "room_util.h"
#include "slab_data.h"
#include "thing_factory.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "thing_navigate.h"
#include "thing_physics.h"
#include "version.h"
#include "frontmenu_ingame_map.h"
#include <string.h>
#include <math.h>
#include "lua_base.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__MINGW32__)

// Copied from stack overflow because MingW doesn't provide it.
char *strsep(char ** stringp, const char * delim) {
    char * rv = *stringp;
    if (rv) {
        *stringp += strcspn(*stringp, delim);
        if (**stringp) {
            *(*stringp)++ = '\0'; // Null-terminate at the next delimiter
        } else {
            *stringp = 0;
        }
    }
    return rv;
}
#endif

char *strsep_param_with_space(char ** stringp) {
    if (stringp == NULL || *stringp == NULL)
        return NULL;

    const char * delim = " ";
    *stringp += strspn(*stringp, delim); // Skip leading delimiters

    if (**stringp == 0) {
        *stringp = NULL;
        return NULL;
    }

    return strsep(stringp, delim);
}


enum AutoCompletionFlags {
    ACF_DoDefaultCompletion,
    ACF_ExactMatchIndex, // get key index when only one full match, without completion
    ACF_UniquePrefixIndex // get key index when only one prefix match, without completion
};

#define AUTO_COMP_RET_COMPLETE_PARTIAL -1
#define AUTO_COMP_RET_COMPLETE_UNIQUE -2
#define AUTO_COMP_RET_NOT_FOUND -3 // zero possibility
#define AUTO_COMP_RET_AMBIGUOUS -4 // multiple possibilities
int do_complete_from_candidates(const char *key_list[], int key_cnt, char *completion_str, size_t completion_len, size_t completion_size, enum AutoCompletionFlags check_flag, char **poss_str_ret);
int do_complete_from_prioritized_candidates(const char *key_list[], int primary_cnt, int secondary_cnt, int prio_threshold_len, char *completion_str, size_t completion_len, size_t completion_size, char **poss_str_ret);
void do_param1_completion_for_name_command(PlayerNumber plyr_idx, char *args_str, size_t args_size, const char *desc_str, struct NamedCommand *primary_name_desc, struct NamedCommand *secondary_name_desc, int prio_threshold_len, TbBool add_random);


extern void render_set_sprite_debug(int level);
extern TbBool process_players_global_packet_action(PlayerNumber plyr_idx);

static struct GuiBoxOption cmd_comp_procs_data[COMPUTER_PROCESSES_COUNT + 3] = {
  {"!", 1, NULL, NULL, 0, 0, 0, 0, 0, 0, 0, 0 }
};

static struct NamedCommand creature_model_command_aliases[] = {
    {"beetle", 24},
    {"mistress", 20},
    {"biledemon", 22},
    {"hound", 27},
    {"priestess", 9},
    {"warlock", 21},
    {"sorcerer", 21},
    {"reaper", 14},
    {"hornedreaper", 14},
    {"dwarf", 5},
    {"mountaindwarf", 5},
    {"spirit", 31},
    {"floatingspirit", 31},
    {"any_creature", CREATURE_ANY},
    {"any", CREATURE_ANY},
    {"evil_creature", 250},
    {"evil", 250},
    {"good_creature", 249},
    {"good", 249},
    {"hero", 249},
    {NULL, 0}
};

static struct NamedCommand trap_model_command_aliases[] = {
    {"gas", 3},
    {"poison", 3},
    {"poisongas", 3},
    {"word", 5},
    {"wordofpower", 5},
    {NULL, 0}
};

static struct NamedCommand door_model_command_aliases[] = {
    {"wooden", 1},
    {"iron", 3},
    {"magical", 4},
    {NULL, 0}
};


static char cmd_comp_procs_label[COMPUTER_PROCESSES_COUNT + 2][COMMAND_WORD_LEN + 8];

struct GuiBoxOption cmd_comp_checks_data[COMPUTER_CHECKS_COUNT + 1] = { 0 };
static char cmd_comp_checks_label[COMPUTER_CHECKS_COUNT][COMMAND_WORD_LEN + 8];

struct GuiBoxOption cmd_comp_events_data[COMPUTER_EVENTS_COUNT + 1] = { 0 };

static TbBool script_set_pool(PlayerNumber player_idx, const char *creature, const char *num);

static char cmd_comp_events_label[COMPUTER_EVENTS_COUNT][COMMAND_WORD_LEN + 8];

static PlayerNumber get_player_number_for_command(char *msg);
static char get_door_number_for_command(char* msg);
static char get_trap_number_for_command(char* msg);
static long get_creature_model_for_command(char *msg);

static long cmd_comp_procs_click(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, int32_t *args)
{
    struct Computer2 *comp;
    comp = get_computer_player(args[0]);
    struct ComputerProcess* cproc = &comp->processes[args[1]];

    if (flag_is_set(cproc->flags, ComProc_Unkn0020))
        message_add_fmt(MsgType_Player, args[0], "resuming %s", cproc->name);
    else
        message_add_fmt(MsgType_Player, args[0], "suspending %s", cproc->name);

    toggle_flag(cproc->flags, ComProc_Unkn0020); // Suspend, but do not update running time
    return 1;
}

static long cmd_comp_procs_update(struct GuiBox *gbox, struct GuiBoxOption *goptn, int32_t *args)
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

    snprintf(cmd_comp_procs_label[i], sizeof(cmd_comp_procs_label[0]), "comp=%d, wait=%u", 0, comp->gameturn_wait);
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
    //gui_cheat_box_2
    int i = 0;
    struct Computer2 *comp;
    comp = get_computer_player(plyr_idx);
    for (; i < max_count; i++)
    {
        unsigned long flags = get_flags(comp, i);
        const char *name = get_name(comp, i);
        if (name == NULL) {
            snprintf(label_list[i], sizeof(label_list[i]), "%02lx %s", flags, "(null2)");
        } else {
            snprintf(label_list[i], sizeof(label_list[i]), "%02lx %s", flags, name);
        }
        data_list[i].label = label_list[i];

        data_list[i].is_enabled = 1;
        data_list[i].callback = click_fn;
        data_list[i].acb_param1 = plyr_idx;
        data_list[i].max_count = max_count;
        data_list[i].cb_param1 = plyr_idx;
        data_list[i].option_index = i;
    }
    data_list[i].label = "!";
    data_list[i].is_enabled = 0;
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

static const char *get_event_name(struct Computer2 *comp, int i)
{
    return comp->events[i].name;
}
static unsigned long  get_event_flags(struct Computer2 *comp, int i)
{
    return 0;
}

static long cmd_comp_checks_click(struct GuiBox *gbox, struct GuiBoxOption *goptn, unsigned char btn, int32_t *args)
{
    struct Computer2 *comp;
    comp = get_computer_player(args[0]);
    struct ComputerCheck* ccheck = &comp->checks[args[1]];

    if (flag_is_set(ccheck->flags, ComChk_Unkn0001))
        message_add_fmt(MsgType_Player, args[0], "resuming %s", ccheck->name);
    else
        message_add_fmt(MsgType_Player, args[0], "suspending %s", ccheck->name);

    ccheck->flags ^= ComChk_Unkn0001;
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

static void str_replace(char *str, int from, int to)
{
    for (char *p = strchr(str, from); p != NULL; p = strchr(p+1, from))
    {
        *p = to;
    }
}

static TbBool cmd_magic_instance(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * creature_str = strsep_param_with_space(&args);
    if (creature_str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as creature");
        return false;
    }
    char * slot_str = strsep_param_with_space(&args);
    if (slot_str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as slot");
        return false;
    }
    char * instance_str = strsep_param_with_space(&args);
    if (instance_str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 3 as instance");
        return false;
    }
    str_replace(creature_str, '.', '_');
    str_replace(instance_str, '.', '_');
    int creature = get_id(creature_desc, creature_str);
    if (creature == -1) {
        targeted_message_add(MsgType_Player, 10, plyr_idx, GUI_MESSAGES_DELAY, "Invalid creature");
        return false;
    }
    int slot = atoi(slot_str);
    if (slot < 0 || slot > 9) {
        targeted_message_add(MsgType_Player, 10, plyr_idx, GUI_MESSAGES_DELAY, "Invalid slot");
        return false;
    }
    int instance = get_id(instance_desc, instance_str);
    if (instance == -1) {
        instance = atoi(instance_str);
    }
    if (instance <= 0) {
        targeted_message_add(MsgType_Player, 10, plyr_idx, GUI_MESSAGES_DELAY, "Invalid instance");
        return false;
    }
    struct CreatureModelConfig* crconf = creature_stats_get(creature);
    crconf->learned_instance_id[slot] = instance;
    for (long i = 0; i < THINGS_COUNT; i++) {
        struct Thing * thing = thing_get(i);
        if ((thing->alloc_flags & TAlF_Exists) != 0) {
            if (thing->class_id == TCls_Creature) {
                creature_increase_available_instances(thing);
            }
        }
    }
    return true;
}

void param_completion_for_magic_instance(PlayerNumber plyr_idx, char *args_str, size_t args_size)
{
    char *pr1_str = NULL, *pr2_str = NULL, *pr3_str = NULL, *pr4_str = NULL;
    size_t pr1_len = 0, pr2_len = 0, pr3_len = 0;
    pr1_str = args_str + strspn(args_str, " ");

    pr2_str = strchr(pr1_str, ' ');
    if (pr2_str == NULL) {
        pr1_len = strlen(pr1_str);
    } else {
        pr1_len = pr2_str - pr1_str;
        pr2_str += strspn(pr2_str, " ");

        pr3_str = strchr(pr2_str, ' ');
    }

    if (pr3_str == NULL) {
        if (pr2_str != NULL) {
            pr2_len = strlen(pr2_str);
        }
    } else {
        pr2_len = pr3_str - pr2_str;
        pr3_str += strspn(pr3_str, " ");

        pr4_str = strchr(pr3_str, ' ');
    }

    if (pr4_str == NULL) {
        if (pr3_str != NULL) {
            pr3_len = strlen(pr3_str);
        }
    } else {
        pr3_len = pr4_str - pr3_str;
        pr4_str += strspn(pr4_str, " ");
    }

    const int suggested_key_max = 2048;
    const char *suggested_key_list[suggested_key_max];
    memset(suggested_key_list, 0, sizeof(suggested_key_list));
    int suggested_key_cnt = 0;

    long i = 0;
    for (i=0; creature_desc[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
    {
        suggested_key_list[suggested_key_cnt++] = creature_desc[i].name;
    }

    {
        char *poss_str = NULL;
        enum AutoCompletionFlags check_flag = pr2_str != NULL ? ACF_ExactMatchIndex : ACF_DoDefaultCompletion ;
        int ret = do_complete_from_candidates(suggested_key_list, suggested_key_cnt, pr1_str, pr1_len, args_size-(pr1_str-args_str), check_flag, &poss_str);
        if (poss_str != NULL) {
            if (ret == AUTO_COMP_RET_AMBIGUOUS)
                targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 1: Possible creature model: %s", poss_str);
            free(poss_str);
            return;
        }

        if (ret == AUTO_COMP_RET_COMPLETE_PARTIAL || ret == AUTO_COMP_RET_COMPLETE_UNIQUE)
            return;
        if (ret == AUTO_COMP_RET_NOT_FOUND) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 1: Unsupported creature model");
            return;
        }
        if (ret < 0)
            return;
    }


    if (pr2_str == NULL)
        return;
    (void)pr2_len;

    if (pr3_str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 2: Auto completion for magic shot is not supported");
        return;
    }


    memset(suggested_key_list, 0, sizeof(suggested_key_list));
    suggested_key_cnt = 0;

    for (i=0; instance_desc[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
    {
        suggested_key_list[suggested_key_cnt++] = instance_desc[i].name;
    }

    {
        char *poss_str = NULL;
        enum AutoCompletionFlags check_flag = pr4_str != NULL ? ACF_ExactMatchIndex : ACF_DoDefaultCompletion ;
        int ret = do_complete_from_candidates(suggested_key_list, suggested_key_cnt, pr3_str, pr3_len, args_size-(pr3_str-args_str), check_flag, &poss_str);
        if (poss_str != NULL) {
            if (ret == AUTO_COMP_RET_AMBIGUOUS)
                targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 3: Possible magic instance: %s", poss_str);
            free(poss_str);
            return;
        }

        if (ret == AUTO_COMP_RET_COMPLETE_PARTIAL || ret == AUTO_COMP_RET_COMPLETE_UNIQUE )
            return;
        if (ret == AUTO_COMP_RET_NOT_FOUND) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 3: Unsupported magic instance");
            return;
        }
        if (ret < 0)
            return;
    }


    if (pr4_str == NULL)
        return;

    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Auto completion is only supported for the first 3 parameters now");
}


TbBool cmd_stats(PlayerNumber plyr_idx, char * args)
{
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "turn fps is %d, draw fps is %d", turns_per_second, turns_per_second_draw_current);
    return true;
}

TbBool cmd_fps_turn(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        turns_per_second = start_params.num_fps;
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Framerate/Turn is %d fps", turns_per_second);
    } else {
        turns_per_second = atoi(pr1str);
    }
    return true;
}

TbBool cmd_fps_draw(PlayerNumber plyr_idx, char * args)
{
    parse_draw_fps_config_val(args, &turns_per_second_draw_main, &turns_per_second_draw_secondary);
    redetect_screen_refresh_rate_for_draw();
    return true;
}

TbBool cmd_frametime(PlayerNumber plyr_idx, char * args)
{
    if (debug_display_frametime == 1) { // If already displaying, then turn off
        debug_display_frametime = 0;
    } else {
        debug_display_frametime = 1;
    }
    return true;
}

TbBool cmd_frametime_max(PlayerNumber plyr_idx, char * args)
{
    if (debug_display_frametime == 2) { // If already displaying, then turn off
        debug_display_frametime = 0;
    } else {
        debug_display_frametime = 2;
    }
    return true;
}

TbBool cmd_network_stats(PlayerNumber plyr_idx, char * args)
{
    if (debug_display_network_stats != 0) {
        debug_display_network_stats = 0;
    } else {
        debug_display_network_stats = 1;
    }
    return true;
}

TbBool cmd_quit(PlayerNumber plyr_idx, char * args)
{
    quit_game = 1;
    exit_keeper = 1;
    return true;
}

TbBool cmd_time(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    char * pr2str = strsep_param_with_space(&args);
    GameTurn turn = (pr1str != NULL) ? (GameTurn) atoi(pr1str) : get_gameturn();
    long frames = (pr2str != NULL) ? (long) atoi(pr2str) : turns_per_second;
    show_game_time_taken(frames, turn);
    return true;
}

TbBool cmd_timer_toggle(PlayerNumber plyr_idx, char * args)
{
    game_flags2 ^= GF2_Timer;
    struct PlayerInfo * player = get_player(plyr_idx);
    if ( (player->victory_state == VicS_WonLevel) && (timer_enabled()) && (TimerGame) ) {
        struct Dungeon * dungeon = get_my_dungeon();
        TimerTurns = dungeon->lvstats.hopes_dashed;
    }
    return true;
}

TbBool cmd_timer_switch(PlayerNumber plyr_idx, char * args)
{
    TimerGame ^= 1;
    struct PlayerInfo * player = get_player(plyr_idx);
    if ( (player->victory_state == VicS_WonLevel) && (timer_enabled()) && (TimerGame) ) {
        struct Dungeon * dungeon = get_my_dungeon();
        TimerTurns = dungeon->lvstats.hopes_dashed;
    }
    return true;
}

TbBool cmd_turn(PlayerNumber plyr_idx, char * args)
{
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "turn %ld", get_gameturn());
    return true;
}

TbBool cmd_pause(PlayerNumber plyr_idx, char * args)
{
    toggle_flag(game.operation_flags, GOF_Paused);
    return true;
}

TbBool cmd_step(PlayerNumber plyr_idx, char * args)
{
    game.frame_step = true;
    clear_flag(game.operation_flags, GOF_Paused);
    return true;
}

TbBool cmd_game_save(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    long slot_num = (pr1str != NULL) ? atoi(pr1str) : 0;
    if (slot_num < 0 || slot_num >= TOTAL_SAVE_SLOTS_COUNT)
    {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "slot_num [%d] exceeds [%d,%d)", slot_num, 0, TOTAL_SAVE_SLOTS_COUNT);
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str != NULL) {
        fill_game_catalogue_slot(slot_num, pr2str);
    }
    set_flag(game.operation_flags, GOF_Paused); // games are saved in a paused state
    TbBool result = save_game(slot_num);
    if (result) {
        output_message(SMsg_GameSaved, 0);
    } else {
        ERRORLOG("Error in save!");
        create_error_box(GUIStr_ErrorSaving);
    }
    clear_flag(game.operation_flags, GOF_Paused); // unpause after save attempt
    return result;
}

TbBool cmd_game_load(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    long slot_num = (pr1str != NULL) ? atoi(pr1str) : 0;
    if (slot_num < 0 || slot_num >= TOTAL_SAVE_SLOTS_COUNT)
    {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "slot_num [%d] exceeds [%d,%d)", slot_num, 0, TOTAL_SAVE_SLOTS_COUNT);
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    TbBool Pause = (pr2str != NULL) ? atoi(pr2str) : false;
    if (is_save_game_loadable(slot_num)) {
        if (load_game(slot_num)) {
            set_flag_value(game.operation_flags, GOF_Paused, Pause); // unpause, because games are saved whilst paused
            return true;
        } else {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Unable to load game %d", slot_num);
        }
    } else {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Unable to load game %d", slot_num);
    }
    return false;
}

TbBool cmd_cls(PlayerNumber plyr_idx, char * args)
{
    zero_messages();
    return true;
}

TbBool cmd_ver(PlayerNumber plyr_idx, char * args)
{
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, PRODUCT_VERSION);
    return true;
}

TbBool cmd_volume(PlayerNumber plyr_idx, char * args)
{
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "%s: %d %s: %d", get_string(340), settings.sound_volume, get_string(341), settings.music_volume);
    return true;
}

TbBool cmd_volume_sound(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL || !parameter_is_number(pr1str)) {
        if (pr1str == NULL)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        else
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "parameter 1 requires a number");
        return false;
    }
    settings.sound_volume = atoi(pr1str);
    if (settings.sound_volume > 127) {
        settings.sound_volume = 127;
    }
    save_settings();
    SetSoundMasterVolume(settings.sound_volume);
    return true;
}

TbBool cmd_volume_music(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL || !parameter_is_number(pr1str)) {
        if (pr1str == NULL)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        else
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "parameter 1 requires a number");
        return false;
    }
    settings.music_volume = atoi(pr1str);
    if (settings.music_volume > 127) {
        settings.music_volume = 127;
    }
    save_settings();
    set_music_volume(settings.music_volume);
    return true;
}

TbBool cmd_compuchat(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        return false;
    } else if ((strcasecmp(pr1str, "scarce") == 0) || (strcasecmp(pr1str, "1") == 0)) {
        for (int i = 0; i < PLAYERS_COUNT; i++) {
            if (!player_is_keeper(i)) {
                continue;
            }
            struct Computer2 *comp = get_computer_player(i);
            if (player_exists(get_player(i)) && (!computer_player_invalid(comp))) {
                targeted_message_add(MsgType_Player, i, plyr_idx, GUI_MESSAGES_DELAY, "Ai model %d", (int) comp->model);
            }
        }
        game.computer_chat_flags = CChat_TasksScarce;
    } else if ((strcasecmp(pr1str, "frequent") == 0) || (strcasecmp(pr1str, "2") == 0)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "%s", pr1str);
        game.computer_chat_flags = CChat_TasksScarce | CChat_TasksFrequent;
    } else {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "none");
        game.computer_chat_flags = CChat_None;
    }
    return true;
}

TbBool cmd_comp_procs(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as player idx");
        return false;
    }
    int id = atoi(pr1str);
    if (id < 0 || id >= PLAYERS_COUNT) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "player idx [%d] exceeds [%d,%d]", id, 0, PLAYERS_COUNT-1);
        return false;
    }
    int i = cmd_comp_list(id, COMPUTER_PROCESSES_COUNT,
        cmd_comp_procs_data, cmd_comp_procs_label,
        &get_process_name, &get_process_flags,
        &cmd_comp_procs_click);
    cmd_comp_procs_data[0].active_cb = &cmd_comp_procs_update;
    cmd_comp_procs_data[i].label = "======";
    cmd_comp_procs_data[i].is_enabled = 1;
    i++;
    cmd_comp_procs_data[i].label = cmd_comp_procs_label[COMPUTER_PROCESSES_COUNT];
    cmd_comp_procs_data[i].is_enabled = 1;
    i++;
    cmd_comp_procs_data[i].label = "!";
    cmd_comp_procs_data[i].is_enabled = 0;
    gui_cheat_box_2 = gui_create_box(my_mouse_x, 20, cmd_comp_procs_data);
    return true;
}

TbBool cmd_comp_events(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as player idx");
        return false;
    }
    int id = atoi(pr1str);
    if (id < 0 || id >= PLAYERS_COUNT) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "player idx [%d] exceeds [%d,%d]", id, 0, PLAYERS_COUNT-1);
        return false;
    }
    cmd_comp_list(id, COMPUTER_EVENTS_COUNT,
        cmd_comp_events_data, cmd_comp_events_label,
        &get_event_name, &get_event_flags, NULL);
    cmd_comp_events_data[0].active_cb = NULL;
    gui_cheat_box_2 = gui_create_box(my_mouse_x, 20, cmd_comp_events_data);
    return true;
}

TbBool cmd_comp_checks(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as player idx");
        return false;
    }
    int id = atoi(pr1str);
    if (id < 0 || id >= PLAYERS_COUNT) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "player idx [%d] exceeds [%d,%d]", id, 0, PLAYERS_COUNT-1);
        return false;
    }
    cmd_comp_list(id, COMPUTER_CHECKS_COUNT,
        cmd_comp_checks_data, cmd_comp_checks_label,
        &get_check_name, &get_check_flags, &cmd_comp_checks_click);
    cmd_comp_checks_data[0].active_cb = NULL;
    gui_cheat_box_2 = gui_create_box(my_mouse_x, 20, cmd_comp_checks_data);
    return true;
}

TbBool cmd_reveal(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    int r = 0;
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str != NULL) {
        r = atol(pr1str);
    }
    if (r > 0) {
        int radius_offset = r / 2;
        struct Packet * pckt = get_packet_direct(player->packet_num);
        MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
        MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
        clear_dig_for_map_rect(player->id_number,
                                subtile_slab(stl_x - radius_offset),
                                subtile_slab(stl_x + r - radius_offset),
                                subtile_slab(stl_y - radius_offset),
                                subtile_slab(stl_y + r - radius_offset)
        );
        reveal_map_rect(player->id_number, stl_x - radius_offset, stl_x + r - radius_offset, stl_y - radius_offset, stl_y + r - radius_offset);
        panel_map_update(stl_x - radius_offset, stl_x + r - radius_offset, stl_y - radius_offset, stl_y + r - radius_offset);
    } else {
        reveal_whole_map(player);
    }
    return true;
}

TbBool cmd_conceal(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    int r = 0;
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str != NULL) {
        r = atol(pr1str);
    }
    if (r > 0) {
        int radius_offset = r / 2;
        struct Packet * pckt = get_packet_direct(player->packet_num);
        MapSubtlCoord stl_x = coord_subtile((pckt->pos_x));
        MapSubtlCoord stl_y = coord_subtile((pckt->pos_y));
        conceal_map_area(player->id_number, stl_x - radius_offset, stl_x + r - radius_offset, stl_y - radius_offset, stl_y + r - radius_offset, false);
    } else {
        conceal_map_area(player->id_number, 0, game.map_subtiles_x - 1, 0, game.map_subtiles_y - 1, false);
    }
    return true;
}

TbBool cmd_comp_kill(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as player idx");
        return false;
    }
    int id = atoi(pr1str);
    if (id < 0 || id > PLAYERS_COUNT) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "player idx [%d] exceeds [%d,%d]", id, 0, PLAYERS_COUNT);
        return false;
    }
    struct Thing * thing = get_player_soul_container(id);
    thing->health = 0;
    return true;
}

TbBool cmd_player_score(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr1str);
    struct Dungeon * dungeon = get_dungeon(id);
    if (dungeon_invalid(dungeon)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "dungeon is invalid");
        return false;
    }
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Player %d score: %ld", id,
                            dungeon->total_score);
    return true;
}

TbBool cmd_player_flag(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr1str);
    struct Dungeon * dungeon = get_dungeon(id);
    if (dungeon_invalid(dungeon)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "dungeon is invalid");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    unsigned char flg_id = (pr2str != NULL) ? atoi(pr2str) : 0;
    if (flg_id >= SCRIPT_FLAGS_COUNT) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "script flag [%d] exceeds");
        return false;
    }
    char * pr3str = strsep_param_with_space(&args);
    if (pr3str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Player %d flag %d value: %d", id,
                                flg_id, dungeon->script_flags[flg_id]);
    } else {
        dungeon->script_flags[flg_id] = atoi(pr3str);
    }
    return true;
}

TbBool cmd_comp_me(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        return false;
    }
    if (!setup_a_computer_player(plyr_idx, atoi(pr1str))) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "unable to set assistant");
        return false;
    }
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "computer assistant is %d", atoi(pr1str));
    return true;
}


TbBool cmd_give_trap(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    long id = get_trap_number_for_command(pr1str);
    if (id <= 0 || id > game.conf.trapdoor_conf.trap_types_count) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "trap number [%d] exceeds (%d,%d]", id, 0, game.conf.trapdoor_conf.trap_types_count);
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    unsigned char num = (pr2str != NULL) ? atoi(pr2str) : 1;
    set_trap_buildable_and_add_to_amount(plyr_idx, id, 1, num);
    update_trap_tab_to_config();
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "done!");
    return true;
}

void param_completion_for_give_trap(PlayerNumber plyr_idx, char *args_str, size_t args_size)
{
    do_param1_completion_for_name_command(plyr_idx, args_str, args_size, "trap model", trap_desc, trap_model_command_aliases, -1, false);
}


TbBool cmd_give_door(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    long id = get_door_number_for_command(pr1str);
    if (id <= 0 || id > game.conf.trapdoor_conf.door_types_count) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "door number [%d] exceeds (%d, %d])", id, 0, game.conf.trapdoor_conf.door_types_count);
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    unsigned char num = (pr2str != NULL) ? atoi(pr2str) : 1;
    set_door_buildable_and_add_to_amount(plyr_idx, id, 1, num);
    update_trap_tab_to_config();
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "done!");
    return true;
}

void param_completion_for_give_door(PlayerNumber plyr_idx, char *args_str, size_t args_size)
{
    do_param1_completion_for_name_command(plyr_idx, args_str, args_size, "door model", door_desc, door_model_command_aliases, -1, false);
}


TbBool cmd_map_pool(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as creature model");
        return true;
    }
    long kind = get_id(creature_desc, pr1str);
    if (kind == -1) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Invalid creature: %s", pr1str);
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Pool count %s: %d", pr1str, game.pool.crtr_kind[kind]);
        return true;
    }
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Set %s pool count: %d", pr1str, atoi(pr2str));
    return script_set_pool(plyr_idx, pr1str, pr2str);
}

TbBool cmd_create_gold(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL || !parameter_is_number(pr1str)) {
        if (pr1str == NULL)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        else
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "parameter 1 requires a number");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    struct Thing * thing = create_gold_pot_at(pckt->pos_x, pckt->pos_y, plyr_idx);
    if (thing_is_invalid(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "coordinate thing is invalid");
        return false;
    }
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        move_creature_to_nearest_valid_position(thing);
    }
    thing->valuable.gold_stored = atoi(pr1str);
    add_gold_to_pile(thing, 0);
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    struct Room * room = subtile_room_get(stl_x, stl_y);
    if (room_exists(room)) {
        if (room_role_matches(room->kind, RoRoF_GoldStorage)) {
            count_gold_hoardes_in_room(room);
        }
    }
    return true;
}

TbBool cmd_look(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL || plyr_idx != my_player_number) {
        if (plyr_idx != my_player_number)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "unknown error");
        else
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        return false;
    }
    make_uppercase(pr1str);
    long room_id = get_id(room_desc, pr1str);
    if (room_id != -1) {
        go_to_my_next_room_of_type(room_id);
        process_players_global_packet_action(plyr_idx); // Dirty hack
        return true;
    }
    long crmodel = get_id(creature_desc, pr1str);
    if(crmodel != -1) {
        go_to_next_creature_of_model_and_gui_job(crmodel, CrGUIJob_Any, TPF_OrderedPick);
        process_players_global_packet_action(plyr_idx); // Dirty hack
        return true;
    }
    TbMapLocation loc = {0};
    if (!get_map_location_id(pr1str, &loc)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "failed to get map location");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    MapSubtlCoord stl_x, stl_y;
    find_map_location_coords(loc, &stl_x, &stl_y, plyr_idx, __func__);
    if (stl_x == 0 && stl_y == 0) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "subtile coord is invalid");
        return false;
    }
    set_players_packet_action(player, PckA_ZoomToPosition, subtile_coord_center(stl_x), subtile_coord_center(stl_y), 0, 0);
    process_players_global_packet_action(plyr_idx); // Dirty hack
    return true;
}

TbBool cmd_create_object(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as object model");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    struct Coord3d pos = {0};
    pos.x.stl.num = coord_subtile(pckt->pos_x);
    pos.y.stl.num = coord_subtile(pckt->pos_y);
    if (subtile_coords_invalid(pos.x.stl.num, pos.y.stl.num)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "subtile coord is invalid");
        return false;
    }
    long ObjModel = get_rid(object_desc, pr1str);
    if (ObjModel == -1) {
        if (parameter_is_number(pr1str)) {
            ObjModel = atoi(pr1str);
        }
    }
    if (ObjModel < 0) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "object model is invalid");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr2str);
    struct Thing * thing = create_object(&pos, ObjModel, id, -1);
    if (!thing_is_object(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "thing is not object");
        return false;
    }
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        move_creature_to_nearest_valid_position(thing);
    }
    return true;
}


TbBool cmd_create_creature(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as creature model");
        return false;
    }
    long crmodel = get_creature_model_for_command(pr1str);
    if (crmodel == -1) {
        if (parameter_is_number(pr1str)) {
            crmodel = atoi(pr1str);
        }
    }
    if ((crmodel <= CREATURE_ANY) && (crmodel >= 249)) {
        TbBool evil = false;
        TbBool good = false;
        if (crmodel == 250) {
            evil = true;
        } else if (crmodel == 249) {
            good = true;
        }
        while (1) {
            crmodel = GAME_RANDOM(game.conf.crtr_conf.model_count) + 1;
            // Accept only evil creatures
            struct CreatureModelConfig* crconf = creature_stats_get(crmodel);
            if ((crconf->model_flags & CMF_IsSpectator) != 0) {
                continue;
            }
            if (evil)  {
                if ((crconf->model_flags & CMF_IsEvil) != 0) {
                    break;
                }
            } else if (good) {
                if ((crconf->model_flags & CMF_IsEvil) == 0) {
                    break;
                }
            } else {
                break;
            }
        }
    }
    if (crmodel <= 0 || crmodel >= game.conf.crtr_conf.model_count) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "creature model [%d] exceeds (%d, %d)", crmodel, 0, game.conf.crtr_conf.model_count);
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    if (subtile_coords_invalid(stl_x, stl_y)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "subtile coord is invalid");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    int level = (pr2str != NULL) ? (atoi(pr2str) - 1) : 0;
    char * pr3str = strsep_param_with_space(&args);
    unsigned int count = (pr3str != NULL) ? atoi(pr3str) : 1;
    char * pr4str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr4str);
    struct Coord3d pos = {0};
    pos.x.stl.num = stl_x;
    pos.y.stl.num = stl_y;
    unsigned int i;
    for (i = 0; i < count; i++) {
        struct Thing *creatng = create_creature(&pos, crmodel, id);
        if (thing_is_creature(creatng)) {
            set_creature_level(creatng, level);
            if (thing_in_wall_at(creatng, &creatng->mappos)) {
                move_creature_to_nearest_valid_position(creatng);
            }
        }
    }
    return true;
}

void param_completion_for_create_creature(PlayerNumber plyr_idx, char *args_str, size_t args_size)
{
    do_param1_completion_for_name_command(plyr_idx, args_str, args_size, "creature model", creature_desc, creature_model_command_aliases, 4, true);
}


TbBool cmd_create_thing(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as class");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as model");
        return false;
    }
    short tngclass = -1;
    short tngmodel = -1;
    if (strcasecmp(pr1str, "object") == 0) {
        tngclass = TCls_Object;
        tngmodel = get_rid(object_desc, pr2str);
    } else if (strcasecmp(pr1str, "corpse") == 0) {
        tngclass = TCls_DeadCreature;
        tngmodel = get_creature_model_for_command(pr2str);
    } else if (strcasecmp(pr1str, "creature") == 0) {
        tngclass = TCls_Creature;
        tngmodel = get_creature_model_for_command(pr2str);
    } else if (strcasecmp(pr1str, "trap") == 0) {
        tngclass = TCls_Trap;
        tngmodel = get_trap_number_for_command(pr2str);
    } else if (strcasecmp(pr1str, "door") == 0) {
        tngclass = TCls_Door;
        tngmodel = get_door_number_for_command(pr2str);
    } else if (strcasecmp(pr1str, "effect") == 0) {
        tngclass = TCls_Effect;
        tngmodel = get_rid(effect_desc, pr2str);
    } else if (strcasecmp(pr1str, "shot") == 0) {
        tngclass = TCls_Shot;
        tngmodel = get_rid(shot_desc, pr2str);
    }
    if (tngclass < 0) {
        if (parameter_is_number(pr1str)) {
            tngclass = atoi(pr1str);
        }
    }
    if (tngmodel < 0) {
        if (parameter_is_number(pr2str)) {
            tngmodel = atoi(pr2str);
        }
    }
    if ((tngclass < 0) || (tngmodel < 0)) {
        if (tngclass < 0)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "class is invalid");
        else
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "model is invalid");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    struct Coord3d pos = {0};
    pos.x.stl.num = coord_subtile(pckt->pos_x);
    pos.y.stl.num = coord_subtile(pckt->pos_y);
    if (subtile_coords_invalid(pos.x.stl.num, pos.y.stl.num)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "subtile coord is invalid");
        return false;
    }
    pos.z.val = get_floor_height(pos.x.stl.num, pos.y.stl.num);
    char * pr3str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr3str);
    struct Thing * thing = create_thing(&pos, tngclass, tngmodel, id, -1);
    if (thing_is_invalid(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "thing is invalid");
        return false;
    }
    if (thing_in_wall_at(thing, &thing->mappos))
    {
        move_creature_to_nearest_valid_position(thing);
    }
    return true;
}

void param_completion_for_create_thing(PlayerNumber plyr_idx, char *args_str, size_t args_size)
{
    char *pr1_str = NULL, *pr2_str = NULL, *pr3_str = NULL;
    size_t pr1_len = 0, pr2_len = 0;
    pr1_str = args_str + strspn(args_str, " ");

    char str_buf[256] = {0};

    pr2_str = strchr(pr1_str, ' ');
    if (pr2_str == NULL) {
        pr1_len = strlen(pr1_str);
    } else {
        pr1_len = pr2_str - pr1_str;
        pr2_str += strspn(pr2_str, " ");

        pr3_str = strchr(pr2_str, ' ');
    }

    if (pr3_str == NULL) {
        if (pr2_str != NULL) {
            pr2_len = strlen(pr2_str);
        }
    } else {
        pr2_len = pr3_str - pr2_str;
        pr3_str += strspn(pr3_str, " ");
    }



    const int suggested_key_max = 2048;
    const char *suggested_key_list[suggested_key_max];
    memset(suggested_key_list, 0, sizeof(suggested_key_list));
    int suggested_key_cnt = 0;
    int primary_cnt = 0, secondary_cnt = 0;

    struct NamedCommand thing_command_aliases[] = {
        {"object", TCls_Object},
        {"corpse", TCls_DeadCreature},
        {"creature", TCls_Creature},
        {"trap", TCls_Trap},
        {"door", TCls_Door},
        {"effect", TCls_Effect},
        {"shot", TCls_Shot},
        {NULL, 0}
    };

    long i = 0;
    for (i=0; thing_command_aliases[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
    {
        suggested_key_list[suggested_key_cnt++] = thing_command_aliases[i].name;
    }

    memset(str_buf, 0, sizeof(str_buf));
    int copy_len =  pr1_len < (sizeof(str_buf) - 1) ? pr1_len : (sizeof(str_buf) - 1);
    memcpy(str_buf, pr1_str, copy_len);
    TbBool is_pr1_num = parameter_is_number(str_buf);
    if (pr2_str == NULL) {
        if (is_pr1_num) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 1: Auto completion for numeric thing class is not supported");
            return;
        }
    }

    int tngclass = -1;
    if (is_pr1_num) {
        tngclass = atoi(str_buf);
    } else {
        char *poss_str = NULL;
        enum AutoCompletionFlags check_flag = pr2_str != NULL ? ACF_ExactMatchIndex : ACF_DoDefaultCompletion ;
        int ret = do_complete_from_candidates(suggested_key_list, suggested_key_cnt, pr1_str, pr1_len, args_size-(pr1_str-args_str), check_flag, &poss_str);
        if (poss_str != NULL) {
            if (ret == AUTO_COMP_RET_AMBIGUOUS)
                targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 1: Possible thing class: %s", poss_str);
            free(poss_str);
            return;
        }

        if (ret == AUTO_COMP_RET_COMPLETE_PARTIAL || ret == AUTO_COMP_RET_COMPLETE_UNIQUE )
            return;
        if (ret == AUTO_COMP_RET_NOT_FOUND) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 1: Unsupported thing class");
            return;
        }
        if (ret < 0)
            return;

        tngclass = thing_command_aliases[ret].num;
    }

    if (pr2_str == NULL) {
        return;
    }



    memset(suggested_key_list, 0, sizeof(suggested_key_list));
    suggested_key_cnt = 0;
    primary_cnt = 0, secondary_cnt = 0;

    struct NamedCommand *model_command = NULL;
    int prio_threshold_len = -1;
    switch (tngclass) {
    case TCls_Object:
        model_command = object_desc;
        break;
    case TCls_DeadCreature:
    case TCls_Creature:
        suggested_key_list[suggested_key_cnt++] = "RANDOM";
        for (i = 0; creature_desc[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
        {
            suggested_key_list[suggested_key_cnt++] = creature_desc[i].name;
        }
        primary_cnt = suggested_key_cnt;

        // refer to get_creature_model_for_command function
        for (i=0; creature_model_command_aliases[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
        {
            suggested_key_list[suggested_key_cnt++] = creature_model_command_aliases[i].name;
        }
        secondary_cnt = suggested_key_cnt - primary_cnt;

        prio_threshold_len = 4;
        break;
    case TCls_Trap:
        for (i = 0; trap_desc[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
        {
            suggested_key_list[suggested_key_cnt++] = trap_desc[i].name;
        }
        primary_cnt = suggested_key_cnt;

        // refer to get_trap_number_for_command function
        for (i=0; trap_model_command_aliases[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
        {
            suggested_key_list[suggested_key_cnt++] = trap_model_command_aliases[i].name;
        }
        secondary_cnt = suggested_key_cnt - primary_cnt;
        break;
    case TCls_Door:
        for (i = 0; door_desc[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
        {
            suggested_key_list[suggested_key_cnt++] = door_desc[i].name;
        }
        primary_cnt = suggested_key_cnt;

        // refer to get_door_number_for_command function
        for (i=0; door_model_command_aliases[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
        {
            suggested_key_list[suggested_key_cnt++] = door_model_command_aliases[i].name;
        }
        secondary_cnt = suggested_key_cnt - primary_cnt;
        break;
    case TCls_Effect:
        model_command = effect_desc;
        break;
    case TCls_Shot:
        model_command = shot_desc;
        break;
    default:
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Unknown thing class");
        return;
    }

    if (model_command != NULL) {
        for (i = 0; model_command[i].name != NULL && suggested_key_cnt < suggested_key_max; i++) {
            suggested_key_list[suggested_key_cnt++] = model_command[i].name;
        }
        primary_cnt = suggested_key_cnt;
        secondary_cnt = 0;
    }

    memset(str_buf, 0, sizeof(str_buf));
    copy_len =  pr2_len < (sizeof(str_buf) - 1) ? pr2_len : (sizeof(str_buf) - 1);
    memcpy(str_buf, pr2_str, copy_len);
    TbBool is_pr2_num = parameter_is_number(str_buf);
    if (pr3_str == NULL) {
        if (is_pr2_num) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 2: Auto completion for numeric thing model is not supported");
            return;
        }
    }

    if (!is_pr2_num) {
        char *poss_str = NULL;
        int ret = 0;
        if (pr3_str == NULL) {
            ret = do_complete_from_prioritized_candidates(suggested_key_list, primary_cnt, secondary_cnt, prio_threshold_len, pr2_str, pr2_len, args_size-(pr2_str-args_str), &poss_str);
        } else {
            ret = do_complete_from_candidates(suggested_key_list, suggested_key_cnt, pr2_str, pr2_len, args_size-(pr2_str-args_str), ACF_ExactMatchIndex, &poss_str);
        }
        if (poss_str != NULL) {
            if (ret == AUTO_COMP_RET_AMBIGUOUS)
                targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 2: Possible thing model: %s", poss_str);
            free(poss_str);
            return;
        }

        if (ret == AUTO_COMP_RET_COMPLETE_PARTIAL || ret == AUTO_COMP_RET_COMPLETE_UNIQUE )
            return;
        if (ret == AUTO_COMP_RET_NOT_FOUND) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 2: Unsupported thing model");
            return;
        }
        if (ret < 0)
            return;
    }

    if (pr3_str == NULL) {
        return;
    }

    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Auto completion is only supported for the first 2 parameters now");
}


TbBool cmd_place_slab(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as slbkind");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    MapSlabCoord slb_x = subtile_slab(stl_x);
    MapSlabCoord slb_y = subtile_slab(stl_y);
    struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "subtile coord is invalid");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    PlayerNumber id = (pr2str == NULL) ? slabmap_owner(slb) : get_player_number_for_command(pr2str);
    short slbkind = get_rid(slab_desc, pr1str);
    if (slbkind < 0) {
        long rid = get_rid(room_desc, pr1str);
        if (rid > 0) {
            struct RoomConfigStats *roomst = get_room_kind_stats(rid);
            slbkind = roomst->assigned_slab;
        } else {
            if (strcasecmp(pr1str, "Earth") == 0) {
                slbkind = rand() % (4 - 2) + 2;
            } else if ((strcasecmp(pr1str, "Reinforced") == 0) || (strcasecmp(pr1str, "Fortified") == 0)) {
                slbkind = rand() % (9 - 4) + 4;
            } else if (strcasecmp(pr1str, "Claimed") == 0) {
                slbkind = SlbT_CLAIMED;
            } else if ((strcasecmp(pr1str, "Rock") == 0) || (strcasecmp(pr1str, "Impenetrable") == 0)) {
                slbkind = SlbT_ROCK;
            } else if (parameter_is_number(pr1str)) {
                slbkind = atoi(pr1str);
            }
        }
    }
    if (slbkind < 0 || slbkind > game.conf.slab_conf.slab_types_count) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "slbkind [%d] exceeds [%d,%d]", slbkind, 0, game.conf.slab_conf.slab_types_count);
        return false;
    }
    if (subtile_is_room(stl_x, stl_y)) {
        delete_room_slab(slb_x, slb_y, true);
    }
    if (slab_kind_is_animated(slbkind)) {
        place_animating_slab_type_on_map(slbkind, 0, stl_x, stl_y, id);
    } else {
        place_slab_type_on_map(slbkind, stl_x, stl_y, id, 0);
    }
    do_slab_efficiency_alteration(slb_x, slb_y);
    return true;
}

TbBool cmd_room_available(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    TbBool available = (pr2str == NULL) ? 1 : atoi(pr2str);
    char * pr3str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr3str);
    long roomid;
    if (strcasecmp(pr1str, "all") == 0) {
        for (roomid = RoK_TREASURE; roomid <= RoK_GUARDPOST; roomid++) {
            if (roomid != RoK_DUNGHEART) {
                set_room_available(id, roomid, available, available);
            }
        }
    } else {
        roomid = get_rid(room_desc, pr1str);
        if (roomid <= 0) {
            if (strcasecmp(pr1str, "Hatchery") == 0) {
                roomid = RoK_GARDEN;
            } else if ((strcasecmp(pr1str, "Guard") == 0) || (strcasecmp(pr1str, "GuardPost") == 0)) {
                roomid = RoK_GUARDPOST;
            } else {
                roomid = atoi(pr1str);
            }
        }
        set_room_available(id, roomid, available, available);
    }
    update_room_tab_to_config();
    return true;
}


TbBool cmd_give_power(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        return false;
    }
    if (strcasecmp(pr1str, "all") == 0) {
        for (PowerKind pw = game.conf.magic_conf.power_types_count - 1; pw > PwrK_HAND; pw--) {
            if ( (pw == PwrK_PICKUPCRTR) || (pw == PwrK_PICKUPGOLD) || (pw == PwrK_PICKUPFOOD) ) {
                continue;
            }
            if (!set_power_available(plyr_idx, pw, 1, 1)) {
                WARNLOG("Setting power %s availability for player %d failed.", power_code_name(pw), plyr_idx);
            }
        }
        update_powers_tab_to_config();
        return true;
    }
    long power = get_rid(power_desc, pr1str);
    if (power < 0) {
        power = atoi(pr1str);
    }
    if (!set_power_available(plyr_idx, power, 1, 1)) {
        WARNLOG("Setting power %s availability for player %d failed.", power_code_name(power), plyr_idx);
        return false;
    }
    update_powers_tab_to_config();
    return true;
}

void param_completion_for_give_power(PlayerNumber plyr_idx, char *args_str, size_t args_size)
{
    struct NamedCommand power_model_command_aliases[] = {
        {"all", 0},
        {NULL, 0}
    };
    do_param1_completion_for_name_command(plyr_idx, args_str, args_size, "power model", power_desc, power_model_command_aliases, -1, false);
}


TbBool cmd_player_heart_health(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr1str);
    struct Thing * thing = get_player_soul_container(id);
    if (!thing_is_dungeon_heart(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "thing is not dungeon heart");
        return false;
    }
    struct ObjectConfigStats* objst = get_object_model_stats(thing->model);
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        float percent = ((float) thing->health / (float)objst->health) * 100;
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY,
                                "Player %d heart health: %ld (%.2f per cent)", id, thing->health, percent);
        return true;
    }
    HitPoints Health = atoi(pr2str);
    thing->health = Health;
    if (thing->health <= 0) {
        struct Dungeon * dungeon = get_dungeon(plyr_idx);
        dungeon->lvstats.keeper_destroyed[id]++;
        dungeon->lvstats.keepers_destroyed++;
    }
    return true;
}

TbBool cmd_creature_available(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    long crmodel = get_creature_model_for_command(pr1str);
    char * pr2str = strsep_param_with_space(&args);
    TbBool available = (pr2str == NULL) ? 1 : atoi(pr2str);
    char * pr3str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr3str);
    if (!set_creature_available(id, crmodel, available, available)) {
        WARNLOG("Setting creature %s availability for player %d failed.", creature_code_name(crmodel),
                (int) id);
        return false;
    }
    return true;
}

TbBool cmd_creature_add_health(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL || !parameter_is_number(pr1str)) {
        if (pr1str == NULL)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        else
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "parameter 1 requires a number");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    thing->health += atoi(pr1str);
    return true;
}

TbBool cmd_creature_sub_health(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL || !parameter_is_number(pr1str)) {
        if (pr1str == NULL)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        else
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "parameter 1 requires a number");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    thing->health -= atoi(pr1str);
    return true;
}

TbBool cmd_send_digger_to(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    struct PlayerInfo * player = get_player(plyr_idx); // requesting player
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    ThingModel model = get_players_special_digger_model(thing->owner);
    PlayerNumber id = get_player_number_for_command(pr1str);
    player = get_player(id); // target player
    if (!player_exists(player)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "player no exist");
        return false;
    }
    struct Coord3d pos = {0};
    if (thing_is_creature(thing)) {
        if (thing->model == model) {
            if (get_random_position_in_dungeon_for_creature(id, CrWaS_WithinDungeon, thing, &pos)) {
                targeted_message_add(MsgType_Player,plyr_idx, plyr_idx, GUI_MESSAGES_DELAY,"%s %d will dig to %s", thing_model_name(thing), thing->index, player_code_name(id));
                return send_tunneller_to_point_in_dungeon(thing, id, &pos);
            }
        }
    }
    thing = find_players_next_creature_of_breed_and_gui_job(get_players_special_digger_model(thing->owner), -1, plyr_idx, TPF_None);
    if (thing_is_invalid(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "thing is invalid");
        return false;
    }
    if (get_random_position_in_dungeon_for_creature(id, CrWaS_WithinDungeon, thing, &pos)) {
        targeted_message_add(MsgType_Player,plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "%s %d will dig to %s", thing_model_name(thing),thing->index, player_code_name(id));
        return send_tunneller_to_point_in_dungeon(thing, id, &pos);
    }
    return false;
}

TbBool cmd_set_creature_instance(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as instance");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    unsigned char inst = atoi(pr1str);
    set_creature_instance(thing, inst, 0, 0);
    return true;
}

TbBool cmd_set_creature_state(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as state");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    unsigned char state = atoi(pr1str);
    if (can_change_from_state_to(thing, thing->active_state, state))
    {
        return internal_set_thing_state(thing, state);
    }
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "unable to set state");
    return false;
}

TbBool cmd_set_creature_job(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as job");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    unsigned char new_job = atoi(pr1str);
    if (!creature_can_do_job_for_player(thing, thing->owner, 1LL << new_job, JobChk_None))
    {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Cannot do job %d.", new_job);
        return false;
    }
    return send_creature_to_job_for_player(thing, thing->owner, 1LL << new_job);
}

TbBool cmd_mapwho_info(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    struct Coord3d pos = {0};
    if (pr1str == NULL) {
        struct PlayerInfo * player = get_player(plyr_idx);
        struct Packet * pckt = get_packet_direct(player->packet_num);
        pos.x.val = pckt->pos_x;
        pos.y.val = pckt->pos_y;
    } else {
        char * pr2str = strsep_param_with_space(&args);
        if (pr2str == NULL) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as stl_y");
            return false;
        }
        pos.x.stl.num = atoi(pr1str);
        pos.y.stl.num = atoi(pr2str);
    }
    if ((pos.x.stl.num >= game.map_subtiles_x) || (pos.y.stl.num >= game.map_subtiles_y)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "invalid location");
        return false;
    }
    struct Map * block = get_map_block_at(pos.x.stl.num, pos.y.stl.num);
    short thing_id = (short) get_mapwho_thing_index(block);
    struct Thing * thing = thing_get(thing_id);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "first_thing:%d %s", thing_id,
                            thing_class_and_model_name(thing->class_id, thing->model));
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "flags: %02x,filled: %d, wib: %d, col: %04ld", block->flags,
                            block->filled_subtiles, block->wibble_value, block->col_idx);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "mapwho: %04ld, rev: %d", block->mapwho, block->revealed);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "stl_x: %d, stl_y:%d", pos.x.stl.num,
                            pos.y.stl.num);
    return true;
}

TbBool cmd_thing_info(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (thing_is_invalid(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or thing is invalid");
        return false;
    }
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "next_on_map: %d, next_of_class: %d",
                            thing->next_on_mapblk, thing->next_of_class);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "health: %d", thing->health);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "pos: %d %d %d", thing->mappos.x.stl.num,
                            thing->mappos.y.stl.num,
                            thing->mappos.z.stl.num);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "%s",
                            thing_class_and_model_name(thing->class_id, thing->model));
    return true;
}

TbBool cmd_creature_attack_heart(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as player number");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    PlayerNumber id = get_player_number_for_command(pr1str);
    struct Thing * heartng = get_player_soul_container(id);
    if (!thing_is_dungeon_heart(heartng)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "thing is not dungeon heart");
        return false;
    }
    TRACE_THING(heartng);
    set_creature_object_combat(thing, heartng);
    return true;
}

TbBool cmd_player_gold_add(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    PlayerNumber id = get_player_number_for_command(pr1str);
    struct PlayerInfo * player = get_player(id);
    if (!player_exists(player)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "player no exist");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as gold amount");
        return false;
    }
    player_add_offmap_gold(id, atoi(pr2str));
    return true;
}

TbBool cmd_cursor_pos(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    struct Coord3d pos = {0};
    pos.x.val = pckt->pos_x;
    pos.y.val = pckt->pos_y;
    pos.z.val = get_floor_height_at(&pos);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Cursor at %d, %d, %d",
                            (int) pos.x.stl.num, (int) pos.y.stl.num, (int) pos.z.stl.num);
    return true;
}

TbBool cmd_get_thing(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    MapSubtlCoord stl_x = coord_subtile(pckt->pos_x);
    MapSubtlCoord stl_y = coord_subtile(pckt->pos_y);
    struct Thing * thing = (pr1str != NULL) ? thing_get(atoi(pr1str)) : get_nearest_thing_at_position(stl_x, stl_y);
    if (thing_is_invalid(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "thing is invalid");
        return false;
    }
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Got thing ID %d %s",
                            thing->index,
                            thing_class_and_model_name(thing->class_id, thing->model));
    player->influenced_thing_idx = thing->index;
    player->influenced_thing_creation = thing->creation_turn;
    return true;
}

TbBool cmd_thing_show_id(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    render_set_sprite_debug((pr1str != NULL) ? atoi(pr1str) : 1);
    return true;
}

TbBool cmd_thing_health(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (thing_is_invalid(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or thing is invalid");
        return false;
    }
    if (pr1str != NULL) {
        thing->health = atoi(pr1str);
    } else {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Thing ID: %d health: %d", thing->index, thing->health);
    }
    return true;
}

TbBool cmd_move_thing(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (thing_is_invalid(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or thing is invalid");
        return false;
    }
    struct Coord3d pos = {0};
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str != NULL) {
        pos.x.stl.num = atoi(pr1str);
        pos.y.stl.num = atoi(pr2str);
        if ((pos.x.stl.num >= game.map_subtiles_x) || (pos.y.stl.num >= game.map_subtiles_y)) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "invalid location");
            return false;
        }
        char * pr3str = strsep_param_with_space(&args);
        if (pr3str != NULL) {
            pos.z.stl.num = atoi(pr3str);
        } else {
            pos.z.val = get_floor_height_at(&pos);
        }
    } else {
        struct Packet * pckt = get_packet_direct(player->packet_num);
        pos.x.val = pckt->pos_x;
        pos.y.val = pckt->pos_y;
        pos.z.val = get_floor_height_at(&pos);
    }
    move_thing_in_map(thing, &pos);
    return true;
}

TbBool cmd_destroy_thing(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (thing_is_invalid(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or thing is invalid");
        return false;
    }
    destroy_object(thing);
    return true;
}

TbBool cmd_get_room(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    MapSubtlCoord stl_x = coord_subtile((pckt->pos_x));
    MapSubtlCoord stl_y = coord_subtile((pckt->pos_y));
    struct Room * room = (pr1str != NULL) ? room_get(atoi(pr1str)) : subtile_room_get(stl_x, stl_y);
    if (!room_exists(room)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "room no exist");
        return false;
    }
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Got room ID %d", room->index);
    player->influenced_thing_idx = room->index;
    player->influenced_thing_creation = room->creation_turn;
    return true;
}

TbBool cmd_room_health(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Room * room = room_get(player->influenced_thing_idx);
    if (room_is_invalid(room)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not room");
        return false;
    } else if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Room ID %d health: %d", room->index, room->health);
        return true;
    }
    room-> health = atoi(pr1str);
    return true;
}

TbBool cmd_slab_health(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Packet * pckt = get_packet_direct(player->packet_num);
    MapSubtlCoord stl_x = coord_subtile((pckt->pos_x));
    MapSubtlCoord stl_y = coord_subtile((pckt->pos_y));
    struct SlabMap * slb = get_slabmap_for_subtile(stl_x, stl_y);
    if (slabmap_block_invalid(slb)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "slabmap block is invalid");
        return false;
    } else if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Slab health: %d", slb->health);
        return true;
    }
    slb->health = atoi(pr1str);
    return true;
}

TbBool cmd_creature_pool_add(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as creature amount");
        return false;
    }
    long crmodel = get_creature_model_for_command(pr1str);
    if (crmodel == -1) {
        crmodel = atoi(pr1str);
    }
    if (crmodel == 0) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "invalid creature model");
        return false;
    }
    game.pool.crtr_kind[crmodel] += atoi(pr2str);
    return true;
}

TbBool cmd_creature_pool_sub(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as creature amount");
        return false;
    }
    long crmodel = get_creature_model_for_command(pr1str);
    if (crmodel == -1) {
        crmodel = atoi(pr1str);
    }
    if (crmodel == 0) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "invalid creature model");
        return false;
    }
    game.pool.crtr_kind[crmodel] -= atoi(pr2str);
    return true;
}

TbBool cmd_creature_level(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as creature level");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    set_creature_level(thing, (atoi(pr1str)-1));
    return true;
}

TbBool cmd_freeze_creature(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    thing_play_sample(thing, 50, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    // Not sure how to handle this yet, for now simply hardcode the intended spell kind with a number.
    apply_spell_effect_to_thing(thing, 3, 8, plyr_idx); // 3 was 'SplK_Freeze' in the enum.
    return true;
}

TbBool cmd_slow_creature(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    thing_play_sample(thing, 50, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    // Not sure how to handle this yet, for now simply hardcode the intended spell kind with a number.
    apply_spell_effect_to_thing(thing, 12, 8, plyr_idx); // 12 was 'SplK_Slow' in the enum.
    return true;
}

TbBool cmd_set_music(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        return false;
    }
    int track = atoi(pr1str);
    if (track < 0) {
        return play_music(pr1str);
    } else {
        return play_music_track(track);
    }
}

TbBool cmd_zoom_to_slabcoord(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as slb_x");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as slb_y");
        return false;
    }
    MapSlabCoord slb_x = atoi(pr1str);
    MapSlabCoord slb_y = atoi(pr2str);
    MapSubtlCoord stl_x = slab_subtile(slb_x, 0);
    MapSubtlCoord stl_y = slab_subtile(slb_y, 0);
    if (subtile_coords_invalid(stl_x, stl_y)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Slab coordinates specified are invalid");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    player->zoom_to_pos_x = subtile_coord_center(stl_x);
    player->zoom_to_pos_y = subtile_coord_center(stl_y);
    set_player_instance(player, PI_ZoomToPos, 0);
    return true;
}

TbBool cmd_zoom_to_subtilecoord(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as stl_x");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as stl_y");
        return false;
    }
    MapSubtlCoord stl_x = atoi(pr1str);
    MapSubtlCoord stl_y = atoi(pr2str);
    if (subtile_coords_invalid(stl_x, stl_y)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Subtile coordinates specified are invalid");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    player->zoom_to_pos_x = subtile_coord_center(stl_x);
    player->zoom_to_pos_y = subtile_coord_center(stl_y);
    set_player_instance(player, PI_ZoomToPos, 0);
    return true;
}

TbBool cmd_toggle_classic_bug(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as bug name");
        return false;
    }
    char bug = get_rid(rules_game_classicbugs_commands, pr1str);
    if (bug == -1) {
        bug = atoi(pr1str);
    }
    unsigned long flg = (bug > 2) ? (1 << (bug - 1)) : bug;
    toggle_flag(game.conf.rules[plyr_idx].game.classic_bugs_flags, flg);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "%s %s", get_conf_parameter_text(rules_game_classicbugs_commands, bug), ((game.conf.rules[plyr_idx].game.classic_bugs_flags & flg) != 0) ? "enabled" : "disabled");
    return true;
}

TbBool cmd_get_action_point_pos(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as actionpoint number");
        return false;
    }
    long num = atoi(pr1str);
    ActionPointId idx = action_point_number_to_index(num);
    if (!action_point_exists_idx(idx)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "actionpoint no exist");
        return false;
    }
    struct ActionPoint * actionpt = action_point_get(idx);
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Action Point idx: %ld num: %ld X: %d Y: %d", idx, num, actionpt->mappos.x.stl.num, actionpt->mappos.y.stl.num);
    return true;
}

TbBool cmd_zoom_to_action_point(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as actionpoint number");
        return false;
    }
    long num = atoi(pr1str);
    ActionPointId idx = action_point_number_to_index(num);
    if (!action_point_exists_idx(idx)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "actionpoint no exist");
        return false;
    }
    struct ActionPoint * actionpt = action_point_get(idx);
    struct PlayerInfo * player = get_player(plyr_idx);
    player->zoom_to_pos_x = subtile_coord_center(actionpt->mappos.x.stl.num);
    player->zoom_to_pos_y = subtile_coord_center(actionpt->mappos.y.stl.num);
    set_player_instance(player, PI_ZoomToPos, 0);
    return true;
}

TbBool cmd_reset_action_point(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as actionpoint number");
        return false;
    }
    long num = atoi(pr1str);
    ActionPointId idx = action_point_number_to_index(num);
    if (!action_point_exists_idx(idx)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "actionpoint no exist");
        return false;
    }
    char * pr2str = strsep_param_with_space(&args);
    PlayerNumber player_idx = (pr2str == NULL) ? ALL_PLAYERS : atoi(pr2str);
    return action_point_reset_idx(idx, player_idx);
}

TbBool cmd_zoom_to_hero_gate(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as hero gate number");
        return false;
    }
    unsigned char hg = atoi(pr1str);
    struct Thing * thing = find_hero_gate_of_number(hg);
    if (!thing_is_object(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "thing is not object");
        return false;
    } else if (!object_is_hero_gate(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "thing is not hero gate");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    player->zoom_to_pos_x = subtile_coord_center(thing->mappos.x.stl.num);
    player->zoom_to_pos_y = subtile_coord_center(thing->mappos.y.stl.num);
    set_player_instance(player, PI_ZoomToPos, 0);
    return true;
}

TbBool cmd_sound_test(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        return false;
    }
    play_non_3d_sample(atoi(pr1str));
    return true;
}

TbBool cmd_speech_test(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1");
        return false;
    }
    play_speech_sample(atoi(pr1str));
    return true;
}

TbBool cmd_player_colour(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as player");
        return false;
    }
    int plr_start;
    int plr_end;
    PlayerNumber plr_range_id = get_player_number_for_command(pr1str);
    get_players_range(plr_range_id, &plr_start, &plr_end);

    char * pr2str = strsep_param_with_space(&args);
    if (pr2str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 2 as color");
        return false;
    }
    char colour_idx = get_rid(cmpgn_human_player_options, pr2str);
    if (plr_start >= 0)
    {
            for (PlayerNumber plyr_id = plr_start; plyr_id < plr_end; plyr_id++)
            {
                if (plyr_id == PLAYER_NEUTRAL)
                {
                    continue;
                }
                set_player_colour(plyr_id, (unsigned char)colour_idx);
            }
            return true;
    }
    targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "player number is invalid");
    return false;
}

TbBool cmd_possession_lock(PlayerNumber plyr_idx, char * args)
{
    struct PlayerInfo * player = get_player(plyr_idx);
    player->possession_lock = true;
    return true;
}

TbBool cmd_possession_unlock(PlayerNumber plyr_idx, char * args)
{
    struct PlayerInfo * player = get_player(plyr_idx);
    player->possession_lock = false;
    return true;
}

TbBool cmd_string_show(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    long msg_id = (pr1str != NULL) ? atoi(pr1str) : 0;
    if (msg_id >= 0)
    {
        set_general_information(msg_id, plyr_idx, 0, 0, 0);
    }
    return true;
}

TbBool cmd_quick_show(PlayerNumber plyr_idx, char * args)
{
    char * pr1str = strsep_param_with_space(&args);
    long msg_id = (pr1str != NULL) ? atoi(pr1str) : 0;
    if (msg_id >= 0)
    {
        set_quick_information(msg_id, plyr_idx, 0, 0, 0);
    }
    return true;
}

TbBool cmd_lua(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    execute_lua_code_from_console(args);
    return true;
}

TbBool cmd_luatypedump(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    generate_lua_types_file();
    return true;
}

TbBool cmd_cheat_menu(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    char * pr1str = strsep_param_with_space(&args);
    if (pr1str == NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require parameter 1 as menu type");
        return false;
    }

    int menu_type = -1;
    if (strcmp(pr1str, "0") == 0 || strcasecmp(pr1str, "none") == 0)
        menu_type = 0;
    else if (strcmp(pr1str, "1") == 0 || strcasecmp(pr1str, "main") == 0)
        menu_type = 1;
    else if (strcmp(pr1str, "2") == 0 || strcasecmp(pr1str, "creature") == 0)
        menu_type = 2;
    else if (strcmp(pr1str, "3") == 0 || strcasecmp(pr1str, "instance") == 0)
        menu_type = 3;
    else if (strcmp(pr1str, "4") == 0 || strcasecmp(pr1str, "secondary") == 0)
        menu_type = 4;

    if (menu_type < 0) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "menu type is invalid. possible values: 0/1/2/3/4, none/main/creature/instance/secondary");
        return false;
    }

    if (menu_type != 1)
	{
        close_main_cheat_menu();
	}
    if (menu_type != 2)
	{
        close_creature_cheat_menu();
	}
    if (menu_type != 3)
	{
        close_instance_cheat_menu();
	}
	if (menu_type != 4)
	{
        close_secondary_cheat_menu();
	}

    if (menu_type == 1)
	{
        toggle_main_cheat_menu();
	}
    else if (menu_type == 2)
	{
        toggle_creature_cheat_menu();
	}
    else if (menu_type == 3)
	{
        toggle_instance_cheat_menu();
	}
	else if (menu_type == 4)
	{
        toggle_secondary_cheat_menu();
	}

    return true;
}

TbBool cmd_chicken_creature(PlayerNumber plyr_idx, char * args)
{
    if (game.easter_eggs_enabled == false) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "require 'cheat mode'");
        return false;
    }
    struct PlayerInfo * player = get_player(plyr_idx);
    struct Thing * thing = thing_get(player->influenced_thing_idx);
    if (!thing_is_creature(thing)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "no thing selected or not creature");
        return false;
    }
    struct PowerConfigStats *powerst = get_power_model_stats(PwrK_CHICKEN);
    thing_play_sample(thing, powerst->select_sound_idx, NORMAL_PITCH, 0, 3, 0, 4, FULL_LOUDNESS);
    // Not sure how to handle this yet, for now simply hardcode the intended spell kind with a number.
    apply_spell_effect_to_thing(thing, 27, 8, plyr_idx); // 27 was 'SplK_Chicken' in the enum.
    return true;
}



struct ConsoleCommand {
    const char * name;
    TbBool (* function)(PlayerNumber, char *);
    void (* param_auto_completion)(PlayerNumber, char *, size_t);
};

static const struct ConsoleCommand console_commands[] = {
    { "stats", cmd_stats, NULL },
    { "fps", cmd_fps_turn, NULL },
    { "fps.draw", cmd_fps_draw, NULL },
    { "frametime", cmd_frametime, NULL },
    { "ft", cmd_frametime, NULL },
    { "frametime.max", cmd_frametime_max, NULL },
    { "ft.max", cmd_frametime_max, NULL },
    { "netstats", cmd_network_stats, NULL },
    { "quit", cmd_quit, NULL },
    { "time", cmd_time, NULL },
    { "timer.toggle", cmd_timer_toggle, NULL },
    { "timer.switch", cmd_timer_switch, NULL },
    { "turn", cmd_turn, NULL },
    { "pause", cmd_pause, NULL },
    { "step", cmd_step, NULL },
    { "game.save", cmd_game_save, NULL },
    { "game.load", cmd_game_load, NULL },
    { "cls", cmd_cls, NULL },
    { "ver", cmd_ver, NULL },
    { "volume", cmd_volume, NULL },
    { "volume.sound", cmd_volume_sound, NULL },
    { "volume.sfx", cmd_volume_sound, NULL },
    { "volume.music", cmd_volume_music, NULL },
    { "volume.soundtrack", cmd_volume_music, NULL },
    { "compuchat", cmd_compuchat, NULL },
    { "comp.procs", cmd_comp_procs, NULL },
    { "comp.events", cmd_comp_events, NULL },
    { "comp.checks", cmd_comp_checks, NULL },
    { "reveal", cmd_reveal, NULL },
    { "conceal", cmd_conceal, NULL },
    { "comp.kill", cmd_comp_kill, NULL },
    { "player.score", cmd_player_score, NULL },
    { "player.flag", cmd_player_flag, NULL },
    { "comp.me", cmd_comp_me, NULL },
    { "magic.instance", cmd_magic_instance, param_completion_for_magic_instance },
    { "give.trap", cmd_give_trap, param_completion_for_give_trap },
    { "trap.give", cmd_give_trap, param_completion_for_give_trap },
    { "give.door", cmd_give_door, param_completion_for_give_door },
    { "door.give", cmd_give_door, param_completion_for_give_door },
    { "map.pool", cmd_map_pool, NULL },
    { "creature.pool", cmd_map_pool, NULL },
    { "gold.create", cmd_create_gold, NULL },
    { "create.gold", cmd_create_gold, NULL },
    { "look", cmd_look, NULL },
    { "object.create", cmd_create_object, NULL },
    { "create.object", cmd_create_object, NULL },
    { "creature.create", cmd_create_creature, param_completion_for_create_creature },
    { "create.creature", cmd_create_creature, param_completion_for_create_creature },
    { "thing.create", cmd_create_thing, param_completion_for_create_thing },
    { "create.thing", cmd_create_thing, param_completion_for_create_thing },
    { "slab.place", cmd_place_slab, NULL },
    { "place.slab", cmd_place_slab, NULL },
    { "room.available", cmd_room_available, NULL },
    { "power.give", cmd_give_power, param_completion_for_give_power },
    { "spell.give", cmd_give_power, param_completion_for_give_power },
    { "player.heart.health", cmd_player_heart_health, NULL },
    { "creature.available", cmd_creature_available, NULL },
    { "creature.addhealth", cmd_creature_add_health, NULL },
    { "creature.health.add", cmd_creature_add_health, NULL },
    { "creature.subhealth", cmd_creature_sub_health, NULL },
    { "creature.health.sub", cmd_creature_sub_health, NULL },
    { "digger.sendto", cmd_send_digger_to, NULL },
    { "creature.instance.set", cmd_set_creature_instance, NULL },
    { "creature.state.set", cmd_set_creature_state, NULL },
    { "creature.job.set", cmd_set_creature_job, NULL },
    { "mapwho.info", cmd_mapwho_info, NULL },
    { "thing.info", cmd_thing_info, NULL },
    { "creature.attackheart", cmd_creature_attack_heart, NULL },
    { "player.addgold", cmd_player_gold_add, NULL },
    { "player.gold.add", cmd_player_gold_add, NULL },
    { "cursor.pos", cmd_cursor_pos, NULL },
    { "thing.get", cmd_get_thing, NULL },
    { "thing.show_id", cmd_thing_show_id, NULL },
    { "thing.health", cmd_thing_health, NULL },
    { "thing.move", cmd_move_thing, NULL },
    { "thing.destroy", cmd_destroy_thing, NULL },
    { "room.get", cmd_get_room, NULL },
    { "room.health", cmd_room_health, NULL },
    { "slab.health", cmd_slab_health, NULL },
    { "creature.pool.add", cmd_creature_pool_add, NULL },
    { "creature.pool.sub", cmd_creature_pool_sub, NULL },
    { "creature.pool.remove", cmd_creature_pool_sub, NULL },
    { "creature.level", cmd_creature_level, NULL },
    { "creature.freeze", cmd_freeze_creature, NULL },
    { "creature.slow", cmd_slow_creature, NULL },
    { "music.set", cmd_set_music, NULL },
    { "zoomto.slabcoord", cmd_zoom_to_slabcoord, NULL },
    { "zoomto.subtilecoord", cmd_zoom_to_subtilecoord, NULL },
    { "bug.toggle", cmd_toggle_classic_bug, NULL },
    { "actionpoint.pos", cmd_get_action_point_pos, NULL },
    { "actionpoint.zoomto", cmd_zoom_to_action_point, NULL },
    { "zoomto.actionpoint", cmd_zoom_to_action_point, NULL },
    { "actionpoint.reset", cmd_reset_action_point, NULL },
    { "herogate.zoomto", cmd_zoom_to_hero_gate, NULL },
    { "zoomto.herogate", cmd_zoom_to_hero_gate, NULL },
    { "sound.test", cmd_sound_test, NULL },
    { "speech.test", cmd_speech_test, NULL },
    { "player.color", cmd_player_colour, NULL },
    { "player.colour", cmd_player_colour, NULL },
    { "possession.lock", cmd_possession_lock, NULL },
    { "possession.unlock", cmd_possession_unlock, NULL },
    { "string.show", cmd_string_show, NULL },
    { "quick.show", cmd_quick_show, NULL },
    { "lua", cmd_lua, NULL },
    { "luatypedump", cmd_luatypedump, NULL },
    { "cheat.menu", cmd_cheat_menu, NULL },
    { "creature.chicken", cmd_chicken_creature, NULL },
};
static const int console_command_count = sizeof(console_commands) / sizeof(*console_commands);



int do_complete_from_candidates(const char *key_list[], int key_cnt, char *completion_str, size_t completion_len, size_t completion_size, enum AutoCompletionFlags check_flag, char **poss_str_ret)
{
    int same_count = 0;
    int *same_idx = NULL;
    if (check_flag != ACF_ExactMatchIndex)
        same_idx = (int *)calloc(key_cnt, sizeof(int));

    for (int i = 0; i < key_cnt; ++i)
    {
        if (key_list[i][0] == 0)
            continue;

        if (strncasecmp(completion_str, key_list[i], completion_len) != 0)
            continue;

        if (check_flag == ACF_ExactMatchIndex) {
            // do parameter auto completion
            if (strlen(key_list[i]) == completion_len) {
                return i;
            }
            continue;
        }

        same_idx[same_count] = i;
        same_count++;
    }

    if (same_count == 0 || check_flag == ACF_ExactMatchIndex || check_flag == ACF_UniquePrefixIndex) {
        int ret = AUTO_COMP_RET_NOT_FOUND;
        if (check_flag == ACF_UniquePrefixIndex) {
            if (same_count == 1)
                ret = same_idx[0];
            else if (same_count > 1)
                ret = AUTO_COMP_RET_AMBIGUOUS;
        }

        if (same_idx)
            free(same_idx);
        return ret;
    }

    int end_flag = 0;
    int auto_len = 0;

    // calculate the length that can be automatically completed.
    while(1)
    {
        int last_char = -1;
        for (int i=0; i<same_count; i++)
        {
            int idx = same_idx[i];
            int cur_char = tolower(key_list[idx][completion_len+auto_len]);
            if (cur_char == 0) {
                end_flag = 1;
                break;
            }

            if (last_char < 0)
                last_char = cur_char;
            else if (last_char != cur_char) {
                end_flag = 1;
                break;
            }
        }
        if (end_flag)
            break;
        auto_len++;
    }

    int ret = 0;
    if (auto_len != 0)
    {
        int idx = same_idx[0];
        int len = (completion_size - 1) < (completion_len + auto_len) ? (completion_size - 1) : (completion_len + auto_len);
        memcpy(completion_str, key_list[idx], len); // cover all, uniform capitalization.
        completion_str[len] = 0;
        completion_len = len;
        ret = AUTO_COMP_RET_COMPLETE_PARTIAL;
    }

    if (same_count == 1) {
        if (completion_len + 1 < completion_size) {
            completion_str[completion_len] = ' ';
            completion_len++;
            completion_str[completion_len] = 0;
            ret = AUTO_COMP_RET_COMPLETE_UNIQUE;
        }
    }

    if (ret == 0) {
        if (poss_str_ret != NULL)
        {
            // multiple possibilities, list these
            char *poss_str = (char *)calloc(same_count, 64);
            for (int i=0; i<same_count; i++)
            {
                int idx = same_idx[i];
                if (i != 0)
                    strcat(poss_str, ", ");
                strcat(poss_str, key_list[idx]);
            }
            *poss_str_ret = poss_str;
        }
        ret = AUTO_COMP_RET_AMBIGUOUS;
    }

    free(same_idx);

    return ret;
}

int do_complete_from_prioritized_candidates(const char *key_list[], int primary_cnt, int secondary_cnt, int prio_threshold_len, char *completion_str, size_t completion_len, size_t completion_size, char **poss_str_ret)
{
    int ret = 0;
    int i = 0;
    char *poss_str = NULL;
    for (i=0; i<2; i++) {
        ret = do_complete_from_candidates(key_list, primary_cnt+secondary_cnt, completion_str, completion_len, completion_size, ACF_DoDefaultCompletion, &poss_str);
        if (poss_str != NULL) {
            if (ret == AUTO_COMP_RET_AMBIGUOUS) {
                if (prio_threshold_len >= 0 && completion_len >= prio_threshold_len && primary_cnt > 0 && secondary_cnt > 0) {

                    // split up the list to primary names and secondary names, and if there's only a primary and a secondary name left, pick the primary name.
                    // to avoid unexpected completion, a minimum length is specified as prio_threshold_len.
                    // `create.creature` command short length issue Example: `he` for `HELL_HOUND` and `hero`, `mo` for `MONK` and `mountaindwarf`.

                    int ret1 = do_complete_from_candidates(key_list, primary_cnt, completion_str, completion_len, completion_size, ACF_UniquePrefixIndex, NULL);
                    int ret2 = do_complete_from_candidates(key_list+primary_cnt, secondary_cnt, completion_str, completion_len, completion_size, ACF_UniquePrefixIndex, NULL);
                    if (ret1 >= 0 && ret2 >= 0) {
                        free(poss_str);
                        poss_str = NULL;

                        const char *auto_str = key_list[ret1];
                        int auto_len = (int)strlen(auto_str);
                        int len = (int)completion_size-1 < auto_len ? (int)completion_size-1 : auto_len;
                        memcpy(completion_str, auto_str, len); // cover all, uniform capitalization.
                        completion_str[len] = 0;
                        completion_len = len;

                        // primary/secondary completion successfully, whether it was the first time or the second time, it was considered a success.
                        ret = AUTO_COMP_RET_COMPLETE_PARTIAL;

                        if (completion_len + 1 < completion_size) {
                            completion_str[completion_len] = ' ';
                            completion_len++;
                            completion_str[completion_len] = 0;
                            ret = AUTO_COMP_RET_COMPLETE_UNIQUE;
                        }
                    } else if (i == 1) {
                        free(poss_str);
                        poss_str = NULL;

                        // another primary/secondary completion failed, ignore it and use the first time result, which must be AUTO_COMP_RET_COMPLETE_PARTIAL.
                        ret = AUTO_COMP_RET_COMPLETE_PARTIAL;
                    }
                }
            }
        }

        if (i == 0 && ret == AUTO_COMP_RET_COMPLETE_PARTIAL && prio_threshold_len >= 0 && (int)completion_len < prio_threshold_len) {
            completion_len = strlen(completion_str);
            if ((int)completion_len >= prio_threshold_len) {
                // after the first default completion, completion_len exceeds the prio_threshold_len limit, try another primary/secondary completion
                continue;
            }
        }

        break;
    }

    if (poss_str_ret != NULL && poss_str != NULL)
        *poss_str_ret = poss_str;

    return ret;
}

void do_param1_completion_for_name_command(PlayerNumber plyr_idx, char *args_str, size_t args_size, const char *desc_str, struct NamedCommand *primary_name_desc, struct NamedCommand *secondary_name_desc, int prio_threshold_len, TbBool add_random)
{
    char *pr1_str = args_str + strspn(args_str, " ");
    char *pr2_str = strchr(pr1_str, ' ');
    if (pr2_str != NULL) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Auto completion is only supported for parameter 1 as %s", desc_str);
        return;
    }

    size_t pr1_len = strlen(pr1_str);
    if (parameter_is_number(pr1_str)) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 1: Auto completion for numeric %s is not supported", desc_str);
        return;
    }

    const int suggested_key_max = 2048;
    const char *suggested_key_list[suggested_key_max];
    memset(suggested_key_list, 0, sizeof(suggested_key_list));
    int suggested_key_cnt = 0;
    int primary_cnt  = 0, secondary_cnt = 0;

    long i = 0;
    if (add_random)
        suggested_key_list[suggested_key_cnt++] = "RANDOM";
    if (primary_name_desc != NULL) {
        for (i = 0; primary_name_desc[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
        {
            suggested_key_list[suggested_key_cnt++] = primary_name_desc[i].name;
        }
    }
    primary_cnt = suggested_key_cnt;

    if (secondary_name_desc != NULL) {
        for (i=0; secondary_name_desc[i].name != NULL && suggested_key_cnt < suggested_key_max; i++)
        {
            suggested_key_list[suggested_key_cnt++] = secondary_name_desc[i].name;
        }
    }
    secondary_cnt = suggested_key_cnt - primary_cnt;

    char *poss_str = NULL;
    int ret = do_complete_from_prioritized_candidates(suggested_key_list, primary_cnt, secondary_cnt, prio_threshold_len, pr1_str, pr1_len, args_size-(pr1_str-args_str), &poss_str);

    if (poss_str != NULL) {
        if (ret == AUTO_COMP_RET_AMBIGUOUS)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 1: Possible %s: %s", desc_str, poss_str);
        free(poss_str);
    }
    if (ret == AUTO_COMP_RET_COMPLETE_PARTIAL || ret == AUTO_COMP_RET_COMPLETE_UNIQUE )
        return;
    if (ret == AUTO_COMP_RET_NOT_FOUND) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Parameter 1: Unsupported %s", desc_str);
        return;
    }
}

void cmd_auto_completion(PlayerNumber plyr_idx, char *cmd_str, size_t cmd_size)
{
    SYNCDBG(2, "Command auto completion %s", cmd_str);

    char *space = strchr(cmd_str, ' ');
    size_t cmd_len = 0;
    if (space == NULL)
        cmd_len = strlen(cmd_str);
    else
        cmd_len = space - cmd_str;

    const int suggested_key_max = 2048;
    int suggested_key_cnt = 0;
    const char *suggested_key_list[suggested_key_max];
    memset(suggested_key_list, 0, sizeof(suggested_key_list));

    long i;
    for (i = 0; i < console_command_count && suggested_key_cnt < suggested_key_max; i++)
    {
        suggested_key_list[suggested_key_cnt++] = console_commands[i].name;
    }

    char *poss_str = NULL;
    enum AutoCompletionFlags check_flag = space != NULL ? ACF_ExactMatchIndex : ACF_DoDefaultCompletion ;
    int ret = do_complete_from_candidates(suggested_key_list, suggested_key_cnt, cmd_str, cmd_len, cmd_size, check_flag, &poss_str);

    if (poss_str != NULL) {
        if (ret == AUTO_COMP_RET_AMBIGUOUS)
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Possible commands: %s", poss_str);
        free(poss_str);
    }
    if (ret == AUTO_COMP_RET_COMPLETE_PARTIAL || ret == AUTO_COMP_RET_COMPLETE_UNIQUE )
        return;
    if (ret == AUTO_COMP_RET_NOT_FOUND) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Unsupported command");
        return;
    }
    if (ret >= 0) {
        // do parameter auto completion
        if (console_commands[ret].param_auto_completion != NULL) {
            console_commands[ret].param_auto_completion(plyr_idx, space+1, cmd_size-(space+1-cmd_str));
        } else {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "Command '%s' does not support parameter auto completion", console_commands[ret].name);
        }
    }
}



TbBool cmd_exec(PlayerNumber plyr_idx, char * args)
{
    SYNCDBG(2, "Command %d: %s",(int)plyr_idx, args);
    const char * command = strsep_param_with_space(&args);
    if (command == NULL) {
        if (game.easter_eggs_enabled == true) {
            targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "command is empty");
        }
        return false;
    }
    // NOTE: execution can be optimized by pre-sorting commands by name and performing binary search
    for (int i = 0; i < console_command_count; ++i) {
        if (strcasecmp(command, console_commands[i].name) == 0) {
            return console_commands[i].function(plyr_idx, args);
        }
    }
    if (game.easter_eggs_enabled == true) {
        targeted_message_add(MsgType_Player, plyr_idx, plyr_idx, GUI_MESSAGES_DELAY, "unsupported command");
    }
    return false;
}

static TbBool script_set_pool(PlayerNumber plyr_idx, const char *creature, const char *str_num)
{
    if (creature == NULL)
        return false;
    long kind = get_id(creature_desc, creature);
    if (kind == -1)
    {
        if (0 == strcasecmp(creature, "EMPTY"))
        {
            clear_creature_pool();
            return true;
        }
        targeted_message_add(MsgType_Player, 10, plyr_idx, GUI_MESSAGES_DELAY, "Invalid creature");
        return false;
    }
    int num = atoi(str_num);
    if (num < 0)
        return false;
    game.pool.crtr_kind[kind] = num;
    return true;
}

static long get_creature_model_for_command(char *msg)
{
    if (msg == NULL || msg[0] == 0)
        return -1;

    long rid = get_rid(creature_desc, msg);
    if (rid >= 1)
    {
        return rid;
    }
    else
    {
        long i = 0;
        for (i=0; creature_model_command_aliases[i].name != NULL; i++)
        {
            if (strcasecmp(msg, creature_model_command_aliases[i].name) == 0)
                return creature_model_command_aliases[i].num;
        }

        return -1;
    }
}

static PlayerNumber get_player_number_for_command(char *msg)
{
    PlayerNumber id = (msg == NULL || *msg == 0) ? my_player_number : get_rid(cmpgn_human_player_options, msg);
    if (id == -1)
    {
        id = get_rid(player_desc, msg);
        if (id == -1)
        {
            if (strcasecmp(msg, "neutral") == 0)
            {
                id = game.neutral_player_num;
            }
            else
            {
                id = atoi(msg);
            }
        }
    }
    return id;
}

static char get_trap_number_for_command(char* msg)
{
    if (msg == NULL || msg[0] == 0)
        return -1;

    char id = get_id(trap_desc, msg);
    if (id < 0)
    {
        long i = 0;
        for (i=0; trap_model_command_aliases[i].name != NULL; i++)
        {
            if (strcasecmp(msg, trap_model_command_aliases[i].name) == 0)
                return trap_model_command_aliases[i].num;
        }

       if (parameter_is_number(msg))
        {
            id = atoi(msg);
        }
    }
    return id;
}

static char get_door_number_for_command(char* msg)
{
    if (msg == NULL || msg[0] == 0)
        return -1;

    long id = get_id(door_desc, msg);
    if (id < 0)
    {
        long i = 0;
        for (i=0; door_model_command_aliases[i].name != NULL; i++)
        {
            if (strcasecmp(msg, door_model_command_aliases[i].name) == 0)
                return door_model_command_aliases[i].num;
        }

        if (parameter_is_number(msg))
        {
            id = atoi(msg);
        }
    }
    return id;
}

#ifdef __cplusplus
}
#endif

