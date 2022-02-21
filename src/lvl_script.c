/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script.c
 *     Level script commands support.
 * @par Purpose:
 *     Load, recognize and maintain the level script.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     12 Feb 2009 - 11 Apr 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include <math.h>

#include "lvl_script.h"

#include "globals.h"
#include "bflib_memory.h"
#include "player_instances.h"
#include "player_data.h"
#include "player_utils.h"
#include "thing_effects.h"
#include "lvl_filesdk1.h"
#include "keeperfx.hpp"

#include "lvl_script_conditions.h"
#include "lvl_script_value.h"
#include "lvl_script_commands_old.h"
#include "lvl_script_commands.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/


const struct CommandDesc *get_next_word(char **line, char *param, int *para_level, const struct CommandDesc *cmdlist_desc)
{
    char chr;
    SCRIPTDBG(12,"Starting");
    const struct CommandDesc* cmnd_desc = NULL;
    // Find start of an item to read
    unsigned int pos = 0;
    param[pos] = '\0';
    while (1)
    {
        chr = **line;
        // letter or number
        if ((isalnum(chr)) || (chr == '-'))
            break;
        // operator
        if ((chr == '\"') || (chr == '=') || (chr == '!') || (chr == '<') || (chr == '>') || (chr == '~'))
            break;
        // end of line
        if ((chr == '\r') || (chr == '\n') || (chr == '\0'))
        {
            (*para_level) = -1;
            return NULL;
        }
        // paraenesis open
        if (chr == '(') {
            (*para_level)++;
        } else
        // paraenesis close
        if (chr == ')') {
            (*para_level)--;
        }
        (*line)++;
    }

    chr = **line;
    // Text string
    if (isalpha(chr))
    {
        // Read the parameter
        while (isalnum(chr) || (chr == '_') || (chr == '[') || (chr == ']') || (chr == ':'))
        {
            param[pos] = chr;
            pos++;
            (*line)++;
            chr = **line;
            if (pos+1 >= MAX_TEXT_LENGTH) break;
        }
        param[pos] = '\0';
        strupr(param);
        // Check if it's a command
        int i = 0;
        cmnd_desc = NULL;
        while (cmdlist_desc[i].textptr != NULL)
        {
            if (strcmp(param, cmdlist_desc[i].textptr) == 0)
            {
                cmnd_desc = &cmdlist_desc[i];
                break;
            }
            i++;
        }
    } else
    // Number string
    if (isdigit(chr) || (chr == '-'))
    {
        if (chr == '-')
        {
          param[pos] = chr;
          pos++;
          (*line)++;
        }
        chr = **line;
        if (!isdigit(chr))
        {
          SCRPTERRLOG("Unexpected '-' not followed by a number");
          return NULL;
        }
        while ( isdigit(chr) )
        {
          param[pos] = chr;
          pos++;
          (*line)++;
          chr = **line;
          if (pos+1 >= MAX_TEXT_LENGTH) break;
        }
    } else
    // Multiword string taken into quotes
    if (chr == '\"')
    {
        (*line)++;
        chr = **line;
        while ((chr != '\0') && (chr != '\n') && (chr != '\r'))
        {
          if (chr == '\"')
          {
            (*line)++;
            break;
          }
          param[pos] = chr;
          pos++;
          (*line)++;
          chr = **line;
          if (pos+1 >= MAX_TEXT_LENGTH) break;
      }
    } else
    // Other cases - only operators are left
    {
        param[pos] = chr;
        pos++;
        (*line)++;
        switch (chr)
        {
        case '!':
            chr = **line;
            if (chr != '=')
            {
                SCRPTERRLOG("Expected '=' after '!'");
                return NULL;
            }
            param[pos] = chr;
            pos++;
            (*line)++;
            break;
        case '>':
        case '<':
            chr = **line;
            if (chr == '=')
            {
              param[pos] = chr;
              pos++;
              (*line)++;
            }
            break;
        case '=':
            chr = **line;
            if (chr != '=')
            {
              SCRPTERRLOG("Expected '=' after '='");
              return 0;
            }
            param[pos] = chr;
            pos++;
            (*line)++;
            break;
        default:
            break;
        }
    }
    chr = **line;
    if ((chr == '\0') || (chr == '\r')  || (chr == '\n'))
        *para_level = -1;
    param[pos] = '\0';
    return cmnd_desc;
}



/**
 * Returns if the command is 'preloaded'. Preloaded commands are initialized
 * before the whole level data is loaded.
 */
TbBool script_is_preloaded_command(long cmnd_index)
{
  switch (cmnd_index)
  {
  case Cmd_SWAP_CREATURE:
  case Cmd_LEVEL_VERSION:
      return true;
  default:
      return false;
  }
}

#define get_players_range(plr_range_id, plr_start, plr_end) get_players_range_f(plr_range_id, plr_start, plr_end, __func__, text_line_number)
long get_players_range_f(long plr_range_id, int *plr_start, int *plr_end, const char *func_name, long ln_num)
{
    if (plr_range_id < 0)
    {
        return -1;
    }
    if (plr_range_id == ALL_PLAYERS)
    {
        *plr_start = 0;
        *plr_end = PLAYERS_COUNT;
        return plr_range_id;
    } else
    if (plr_range_id == PLAYER_GOOD)
    {
        *plr_start = game.hero_player_num;
        *plr_end = game.hero_player_num+1;
        return plr_range_id;
    } else
    if (plr_range_id == PLAYER_NEUTRAL)
    {
        *plr_start = game.neutral_player_num;
        *plr_end = game.neutral_player_num+1;
        return plr_range_id;
    } else
    if (plr_range_id < PLAYERS_COUNT)
    {
        *plr_start = plr_range_id;
        *plr_end = (*plr_start) + 1;
        return plr_range_id;
    }
    return -2;
}

static void process_fx_line(struct ScriptFxLine *fx_line)
{
    fx_line->partial_steps += fx_line->steps_per_turn;
    for (;fx_line->partial_steps >= FX_LINE_TIME_PARTS; fx_line->partial_steps -= FX_LINE_TIME_PARTS)
    {
        fx_line->here.z.val = get_floor_height_at(&fx_line->here);
        if (fx_line->here.z.val < FILLED_COLUMN_HEIGHT)
        {
          if (fx_line->effect > 0)
          {
            create_effect(&fx_line->here, fx_line->effect, 5); // Owner - neutral
          } else if (fx_line->effect < 0)
          {
            create_effect_element(&fx_line->here, -fx_line->effect, 5); // Owner - neutral
          }
        }

        fx_line->step++;
        if (fx_line->step >= fx_line->total_steps)
        {
          fx_line->used = false;
          break;
        }

        int remain_t = fx_line->total_steps - fx_line->step;

        int bx = fx_line->from.x.val * remain_t + fx_line->cx * fx_line->step;
        int by = fx_line->from.y.val * remain_t + fx_line->cy * fx_line->step;
        int dx = fx_line->cx * remain_t + fx_line->to.x.val * fx_line->step;
        int dy = fx_line->cy * remain_t + fx_line->to.y.val * fx_line->step;

        fx_line->here.x.val = (bx * remain_t + dx * fx_line->step) / fx_line->total_steps / fx_line->total_steps;
        fx_line->here.y.val = (by * remain_t + dy * fx_line->step) / fx_line->total_steps / fx_line->total_steps;
    }
}

#define get_players_range_from_str(plrname, plr_start, plr_end) get_players_range_from_str_f(plrname, plr_start, plr_end, __func__, text_line_number)
long get_players_range_from_str_f(const char *plrname, int *plr_start, int *plr_end, const char *func_name, long ln_num)
{
    long plr_range_id = get_rid(player_desc, plrname);
    if (plr_range_id == -1)
    {
        plr_range_id = get_rid(cmpgn_human_player_options, plrname);
    }
    switch (get_players_range_f(plr_range_id, plr_start, plr_end, func_name, ln_num))
    {
    case -1:
        ERRORMSG("%s(line %lu): Invalid player name, '%s'",func_name,ln_num, plrname);
        *plr_start = 0;
        *plr_end = 0;
        return -1;
    case -2:
        ERRORMSG("%s(line %lu): Player '%s' out of range",func_name,ln_num, plrname);
        *plr_start = 0;
        *plr_end = 0;
        return -2;
    default:
        break;
    }
    return plr_range_id;
}

#define get_player_id(plrname, plr_range_id) get_player_id_f(plrname, plr_range_id, __func__, text_line_number)
TbBool get_player_id_f(const char *plrname, long *plr_range_id, const char *func_name, long ln_num)
{
    *plr_range_id = get_rid(player_desc, plrname);
    if (*plr_range_id == -1)
    {
      *plr_range_id = get_rid(cmpgn_human_player_options, plrname);
      if (*plr_range_id == -1)
      {
        ERRORMSG("%s(line %lu): Invalid player name, '%s'",func_name,ln_num, plrname);
        return false;
      }
    }
    return true;
}

static TbBool script_command_param_to_number(char type_chr, struct ScriptLine *scline, int idx, TbBool extended)
{
    switch (toupper(type_chr))
    {
    case 'N':
    {
        char* text;
        scline->np[idx] = strtol(scline->tp[idx], &text, 0);
        if (text != &scline->tp[idx][strlen(scline->tp[idx])]) {
            SCRPTWRNLOG("Numerical value \"%s\" interpreted as %ld", scline->tp[idx], scline->np[idx]);
        }
        break;
    }
    case 'P':{
        long plr_range_id;
        if (!get_player_id(scline->tp[idx], &plr_range_id)) {
            return false;
        }
        scline->np[idx] = plr_range_id;
        };break;
    case 'C':{
        long crtr_id = get_rid(creature_desc, scline->tp[idx]);
        if (extended)
        {
            if (crtr_id == -1)
            {
                if (0 == strcmp(scline->tp[idx], "ANY_CREATURE"))
                {
                    crtr_id = 0;
                }
            }
        }
        if (crtr_id == -1)
        {
            SCRPTERRLOG("Unknown creature, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = crtr_id;
        };break;
    case 'R':{
        long room_id = get_rid(room_desc, scline->tp[idx]);
        if (room_id == -1)
        {
            SCRPTERRLOG("Unknown room kind, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = room_id;
        };break;
    case 'S': {
        long slab_id = get_rid(slab_desc, scline->tp[idx]);
        if (slab_id == -1)
        {
            SCRPTERRLOG("Unknown slab kind, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = slab_id;
    }; break;
    case 'L':{
        TbMapLocation loc;
        if (!get_map_location_id(scline->tp[idx], &loc)) {
            return false;
        }
        scline->np[idx] = loc;
        };break;
    case 'O':{
        long opertr_id = get_rid(comparison_desc, scline->tp[idx]);
        if (opertr_id == -1) {
            SCRPTERRLOG("Unknown operator, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = opertr_id;
        };break;
    case 'X': {
        long prop_id = get_rid(creatmodel_properties_commands, scline->tp[idx]);
        if (prop_id == -1)
        {
            SCRPTERRLOG("Unknown creature property kind, \"%s\"", scline->tp[idx]);
            return false;
        }
        scline->np[idx] = prop_id;
    }; break;
    case 'A':
        break;
    case '!': // extended sign
        return true;
    default:
        return false;
    }
    return true;
}

static TbBool is_condition_met(unsigned char cond_idx)
{
    if (cond_idx >= CONDITIONS_COUNT)
    {
      if (cond_idx == CONDITION_ALWAYS)
          return true;
      else
          return false;
    }
    unsigned long i = gameadd.script.conditions[cond_idx].status;
    return ((i & 0x01) != 0);
}

TbBool script_command_param_to_text(char type_chr, struct ScriptLine *scline, int idx)
{
    switch (toupper(type_chr))
    {
    case 'N':
        itoa(scline->np[idx], scline->tp[idx], 10);
        break;
    case 'P':
        strcpy(scline->tp[idx], player_code_name(scline->np[idx]));
        break;
    case 'C':
        strcpy(scline->tp[idx], creature_code_name(scline->np[idx]));
        break;
    case 'R':
        strcpy(scline->tp[idx], room_code_name(scline->np[idx]));
        break;
    case 'L':
        get_map_location_code_name(scline->np[idx], scline->tp[idx]);
        break;
    case 'A':
        break;
    case '!': // extended sign
        return true;
    default:
        return false;
    }
    return true;
}

static int count_required_parameters(const char *args)
{
    int required = 0;
    for (int i = 0; i < COMMANDDESC_ARGS_COUNT; i++)
    {
        char chr = args[i];
        if (isupper(chr)) // Required arguments have upper-case type letters
        {
            required++;
        }
        else if (chr == '!')
        {
            continue;
        }
        else
            break;
    }
    return required;
}

int script_recognize_params(char **line, const struct CommandDesc *cmd_desc, struct ScriptLine *scline, int *para_level, int expect_level)
{
    int dst, src;
    for (dst = 0, src = 0; dst <= COMMANDDESC_ARGS_COUNT; dst++, src++)
    {
        TbBool extended = false;
        char chr = cmd_desc->args[src];
        if (*para_level < expect_level)
            break;
        if (chr == '!')
        {
            dst--;
            continue;
        }
        // Read the next parameter
        const struct CommandDesc *funcmd_desc;
        {
            char* funline = *line;
            int funpara_level = *para_level;
            char funcmd_buf[MAX_TEXT_LENGTH];
            LbMemorySet(funcmd_buf, 0, MAX_TEXT_LENGTH);
            funcmd_desc = get_next_word(&funline, funcmd_buf, &funpara_level, subfunction_desc);
            if (funpara_level < expect_level+1) {
                // Break the loop keeping variables as if the parameter wasn't read
                break;
            }
            if (funpara_level > (*para_level)+(dst > 0 ? 0 : 1)) {
                SCRPTWRNLOG("Unexpected paraenesis in parameter %d of command \"%s\"", dst + 1, scline->tcmnd);
            }
            *line = funline;
            *para_level = funpara_level;
            if (!isalpha(chr))
            {
                // Don't show parameter index - it may be bad, as we're decreasing dst to not overflow cmd_desc->args
                SCRPTWRNLOG("Excessive parameter of command \"%s\", value \"%s\"; ignoring", scline->tcmnd, funcmd_buf);
                dst--;
                continue;
            }
            // Access tp[dst] only if we're sure dst < COMMANDDESC_ARGS_COUNT
            LbMemoryCopy(scline->tp[dst], funcmd_buf, MAX_TEXT_LENGTH);
        }
        if (funcmd_desc != NULL)
        {
            struct ScriptLine* funscline = (struct ScriptLine*)LbMemoryAlloc(sizeof(struct ScriptLine));
            if (funscline == NULL) {
                SCRPTERRLOG("Can't allocate buffer to recognize line");
                return -1;
            }
            LbMemorySet(funscline, 0, sizeof(struct ScriptLine));
            LbMemoryCopy(funscline->tcmnd, scline->tp[dst], MAX_TEXT_LENGTH);
            int args_count = script_recognize_params(line, funcmd_desc, funscline, para_level, *para_level);
            if (args_count < 0)
            {
                LbMemoryFree(funscline);
                return -1;
            }
            // Count valid args
            if (args_count < COMMANDDESC_ARGS_COUNT)
            {
                int required = count_required_parameters(funcmd_desc->args);
                if (args_count < required)
                {
                  SCRPTERRLOG("Not enough parameters for \"%s\", got only %d", funcmd_desc->textptr,(int)args_count);
                  LbMemoryFree(funscline);
                  return -1;
                }
            }
            switch (funcmd_desc->index)
            {
            case Cmd_RANDOM:
            case Cmd_DRAWFROM:{
                // Create array of value ranges
                long range_total = 0;
                int fi;
                struct MinMax ranges[COMMANDDESC_ARGS_COUNT];
                if (level_file_version > 0)
                {
                    chr = cmd_desc->args[src];
                    int ri;
                    for (fi = 0, ri = 0; fi < COMMANDDESC_ARGS_COUNT; fi++, ri++)
                    {
                        if (funscline->tp[fi][0] == '\0') {
                            break;
                        }
                        if (toupper(chr) == 'A')
                        {
                            // Values which do not support range
                            if (strcmp(funscline->tp[fi],"~") == 0) {
                                SCRPTERRLOG("Parameter %d of function \"%s\" within command \"%s\" does not support range", fi+1, funcmd_desc->textptr, scline->tcmnd);
                                LbMemoryFree(funscline);
                                return -1;
                            }
                            // Values of that type cannot define ranges, as we cannot interpret them
                            ranges[ri].min = fi;
                            ranges[ri].max = fi;
                            range_total += 1;
                        } else
                        if ((ri > 0) && (strcmp(funscline->tp[fi],"~") == 0))
                        {
                            // Second step of defining range
                            ri--;
                            fi++;
                            if (!script_command_param_to_number(chr, funscline, fi, false)) {
                                SCRPTERRLOG("Parameter %d of function \"%s\" within command \"%s\" has unexpected range end value; discarding command", fi+1, funcmd_desc->textptr, scline->tcmnd);
                                LbMemoryFree(funscline);
                                return -1;
                            }
                            ranges[ri].max = funscline->np[fi];
                            if (ranges[ri].max < ranges[ri].min) {
                                SCRPTWRNLOG("Range definition in argument of function \"%s\" within command \"%s\" should have lower value first", funcmd_desc->textptr, scline->tcmnd);
                                ranges[ri].max = ranges[ri].min;
                            }
                            range_total += ranges[ri].max - ranges[ri].min; // +1 was already added
                        } else
                        {
                            // Single value or first step of defining range
                            if (!script_command_param_to_number(chr, funscline, fi, false)) {
                                SCRPTERRLOG("Parameter %d of function \"%s\" within command \"%s\" has unexpected value; discarding command", fi+1, funcmd_desc->textptr, scline->tcmnd);
                                LbMemoryFree(funscline);
                                return -1;
                            }
                            ranges[ri].min = funscline->np[fi];
                            ranges[ri].max = funscline->np[fi];
                            range_total += 1;
                        }
                    }
                } else
                {
                    // Old RANDOM command accepts only one range, and gives only numbers
                    fi = 0;
                    {
                        ranges[fi].min = atol(funscline->tp[0]);
                        ranges[fi].max = atol(funscline->tp[1]);
                    }
                    if (ranges[fi].max < ranges[fi].min) {
                        SCRPTWRNLOG("Range definition in argument of function \"%s\" within command \"%s\" should have lower value first", funcmd_desc->textptr, scline->tcmnd);
                        ranges[fi].max = ranges[fi].min;
                    }
                    range_total += ranges[fi].max - ranges[fi].min + 1;
                    fi++;
                }
                if (range_total <= 0) {
                    SCRPTERRLOG("Arguments of function \"%s\" within command \"%s\" define no values to select from", funcmd_desc->textptr, scline->tcmnd);
                    break;
                }
                if ((funcmd_desc->index != Cmd_RANDOM) && (level_file_version == 0)) {
                    SCRPTERRLOG("The function \"%s\" used within command \"%s\" is not supported in old level format", funcmd_desc->textptr, scline->tcmnd);
                    break;
                }
                // The new RANDOM command stores values to allow selecting different one every turn during gameplay
                if ((funcmd_desc->index == Cmd_RANDOM) && (level_file_version > 0))
                {
                    //TODO RANDOM make implementation - store ranges as variable to be used for selecting random value during gameplay
                    SCRPTERRLOG("The function \"%s\" used within command \"%s\" is not supported yet", funcmd_desc->textptr, scline->tcmnd);
                    break;
                }
                // DRAWFROM support - select random index now
                long range_index = rand() % range_total;
                // Get value from ranges array
                range_total = 0;
                for (fi=0; fi < COMMANDDESC_ARGS_COUNT; fi++)
                {
                    if ((range_index >= range_total) && (range_index <= range_total + ranges[fi].max - ranges[fi].min)) {
                        chr = cmd_desc->args[src];
                        if (toupper(chr) == 'A') {
                            strcpy(scline->tp[dst], funscline->tp[ranges[fi].min]);
                        } else {
                            scline->np[dst] = ranges[fi].min + range_index - range_total;
                            // Set text value for that number
                            script_command_param_to_text(chr, scline, dst);
                        }
                        break;
                    }
                    range_total += ranges[fi].max - ranges[fi].min + 1;
                }
                SCRPTLOG("Function \"%s\" returned value \"%s\"", funcmd_desc->textptr, scline->tp[dst]);
                };break;
            case Cmd_IMPORT:
            {
                long player_id = get_id(player_desc, funscline->tp[0]);
                if (player_id >= PLAYERS_FOR_CAMPAIGN_FLAGS)
                {
                    SCRPTERRLOG("Cannot fetch flag values for player, '%s'", funscline->tp[0]);
                    strcpy(scline->tp[dst], "0");
                    break;
                }
                long flag_id = get_id(campaign_flag_desc, funscline->tp[1]);
                if (flag_id == -1)
                {
                    SCRPTERRLOG("Unknown campaign flag name, '%s'", funscline->tp[1]);
                    strcpy(scline->tp[dst], "0");
                    break;
                }
                SCRPTLOG("Function \"%s\" returned value \"%ld\"", funcmd_desc->textptr,
                    intralvl.campaign_flags[player_id][flag_id]);
                ltoa(intralvl.campaign_flags[player_id][flag_id], scline->tp[dst], 10);
                break;
            }
            default:
                SCRPTWRNLOG("Parameter value \"%s\" is a command which isn't supported as function", scline->tp[dst]);
                break;
            }
            LbMemoryFree(funscline);
        }
        if (scline->tp[dst][0] == '\0') {
          break;
        }
        if (*para_level > expect_level+2) {
            SCRPTWRNLOG("Parameter %d of command \"%s\", value \"%s\", is at too high paraenesis level %d", dst + 1, scline->tcmnd, scline->tp[dst], (int)*para_level);
        }
        chr = cmd_desc->args[src];
        if (cmd_desc->args[src + 1] == '+')
        {
            // All other parameters will be same
            src -= 1;
        }
        if (cmd_desc->args[src + 1] == '!') //extended set (dst.e. ANY_CREATURE)
        {
            extended = true;
        }
        if (!script_command_param_to_number(chr, scline, dst, extended)) {
            SCRPTERRLOG("Parameter %d of command \"%s\", type %c, has unexpected value; discarding command", dst + 1, scline->tcmnd, chr);
            return -1;
        }
    }
    return dst;
}

long script_scan_line(char *line,TbBool preloaded)
{
    const struct CommandDesc *cmd_desc;
    SCRIPTDBG(12,"Starting");
    struct ScriptLine* scline = (struct ScriptLine*)LbMemoryAlloc(sizeof(struct ScriptLine));
    if (scline == NULL)
    {
      SCRPTERRLOG("Can't allocate buffer to recognize line");
      return 0;
    }
    int para_level = 0;
    LbMemorySet(scline, 0, sizeof(struct ScriptLine));
    if (next_command_reusable > 0)
        next_command_reusable--;
    if (level_file_version > 0)
    {
        cmd_desc = get_next_word(&line, scline->tcmnd, &para_level, command_desc);
    } else
    {
        cmd_desc = get_next_word(&line, scline->tcmnd, &para_level, dk1_command_desc);
    }
    if (cmd_desc == NULL)
    {
        if (isalnum(scline->tcmnd[0])) {
          SCRPTERRLOG("Invalid command, '%s' (lev ver %d)", scline->tcmnd,level_file_version);
        }
        LbMemoryFree(scline);
        return 0;
    }
    SCRIPTDBG(12,"Executing command %lu",cmd_desc->index);
    // Handling comments
    if (cmd_desc->index == Cmd_REM)
    {
        LbMemoryFree(scline);
        return 0;
    }
    scline->command = cmd_desc->index;
    // selecting only preloaded/not preloaded commands
    if (script_is_preloaded_command(cmd_desc->index) != preloaded)
    {
        LbMemoryFree(scline);
        return 0;
    }
    // Recognizing parameters
    int args_count = script_recognize_params(&line, cmd_desc, scline, &para_level, 0);
    if (args_count < 0)
    {
        LbMemoryFree(scline);
        return -1;
    }
    if (args_count < COMMANDDESC_ARGS_COUNT)
    {
        int required = count_required_parameters(cmd_desc->args);
        if (args_count < required) // Required arguments have upper-case type letters
        {
            SCRPTERRLOG("Not enough parameters for \"%s\", got only %d", cmd_desc->textptr,(int)args_count);
            LbMemoryFree(scline);
            return -1;
        }
    }
    script_add_command(cmd_desc, scline);
    LbMemoryFree(scline);
    SCRIPTDBG(13,"Finished");
    return 0;
}

short clear_script(void)
{
    LbMemorySet(&game.script, 0, sizeof(struct LevelScriptOld));
    LbMemorySet(&gameadd.script, 0, sizeof(struct LevelScript));
    gameadd.script.next_string = gameadd.script.strings;
    set_script_current_condition(CONDITION_ALWAYS);
    text_line_number = 1;
    return true;
}

short clear_quick_messages(void)
{
    for (long i = 0; i < QUICK_MESSAGES_COUNT; i++)
        LbMemorySet(gameadd.quick_messages[i], 0, MESSAGE_TEXT_LEN);
    return true;
}

static char* process_multiline_comment(char *buf, char *buf_end)
{
    for (char *p = buf; p < buf_end - 1; p++)
    {
        if ((*p == ' ') || (*p == 9)) // Tabs or spaces
            continue;
        if (p[0] == '/') // /
        {
            if (p[1] != '*') // /*
                break;
            p += 2;
            for (; p < buf_end - 1; p++)
            {
                if ((p[0] == '*') && (p[1] == '/'))
                {
                    buf = p + 2;
                    break;
                }
            }
            break;
        }
        else
            break;
    }
    return buf;
}

short preload_script(long lvnum)
{
  SYNCDBG(7,"Starting");
  set_script_current_condition(CONDITION_ALWAYS);
  next_command_reusable = 0;
  text_line_number = 1;
  level_file_version = DEFAULT_LEVEL_VERSION;
  clear_quick_messages();
  // Load the file
  long script_len = 1;
  char* script_data = (char*)load_single_map_file_to_buffer(lvnum, "txt", &script_len, LMFF_None);
  if (script_data == NULL)
    return false;
  // Process the file lines
  char* buf = script_data;
  char* buf_end = script_data + script_len;
  while (buf < buf_end)
  {
      // Check for long comment
      buf = process_multiline_comment(buf, buf_end);
    // Find end of the line
    int lnlen = 0;
    while (&buf[lnlen] < buf_end)
    {
      if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
        break;
      lnlen++;
    }
    // Get rid of the next line characters
    buf[lnlen] = 0;
    lnlen++;
    if (&buf[lnlen] < buf_end)
    {
      if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
        lnlen++;
    }
    //SCRPTLOG("Analyse");
    // Analyze the line
    script_scan_line(buf, true);
    // Set new line start
    text_line_number++;
    buf += lnlen;
  }
  LbMemoryFree(script_data);
  SYNCDBG(8,"Finished");
  return true;
}

short load_script(long lvnum)
{
    SYNCDBG(7,"Starting");

    // Clear script data
    gui_set_button_flashing(0, 0);
    clear_script();
    set_script_current_condition(CONDITION_ALWAYS);
    next_command_reusable = 0;
    text_line_number = 1;
    game.bonus_time = 0;
    game.flags_gui &= ~GGUI_CountdownTimer;
    game.flags_cd |= MFlg_DeadBackToPool;
    reset_creature_max_levels();
    reset_script_timers_and_flags();
    if ((game.operation_flags & GOF_ColumnConvert) != 0)
    {
        convert_old_column_file(lvnum);
        game.operation_flags &= ~GOF_ColumnConvert;
    }
    // Load the file
    long script_len = 1;
    char* script_data = (char*)load_single_map_file_to_buffer(lvnum, "txt", &script_len, LMFF_None);
    if (script_data == NULL)
      return false;
    // Process the file lines
    char* buf = script_data;
    char* buf_end = script_data + script_len;
    while (buf < buf_end)
    {
        buf = process_multiline_comment(buf, buf_end);
      // Find end of the line
      int lnlen = 0;
      while (&buf[lnlen] < buf_end)
      {
        if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
          break;
        lnlen++;
      }
      // Get rid of the next line characters
      buf[lnlen] = 0;
      lnlen++;
      if (&buf[lnlen] < buf_end)
      {
        if ((buf[lnlen] == '\r') || (buf[lnlen] == '\n'))
          lnlen++;
      }
      // Analyze the line
      script_scan_line(buf, false);
      // Set new line start
      text_line_number++;
      buf += lnlen;
    }
    LbMemoryFree(script_data);
    if (gameadd.script.win_conditions_num == 0)
      WARNMSG("No WIN GAME conditions in script file.");
    if (get_script_current_condition() != CONDITION_ALWAYS)
      WARNMSG("Missing ENDIF's in script file.");
    JUSTLOG("Used script resources: %d/%d tunneller triggers, %d/%d party triggers, %d/%d script values, %d/%d IF conditions, %d/%d party definitions",
        (int)gameadd.script.tunneller_triggers_num,TUNNELLER_TRIGGERS_COUNT,
        (int)gameadd.script.party_triggers_num,PARTY_TRIGGERS_COUNT,
        (int)gameadd.script.values_num,SCRIPT_VALUES_COUNT,
        (int)gameadd.script.conditions_num,CONDITIONS_COUNT,
        (int)gameadd.script.creature_partys_num,CREATURE_PARTYS_COUNT);
    return true;
}

/**
 * Returns if the action point condition was activated.
 * Action point index and player to be activated should be stored inside condition.
 */
TbBool process_activation_status(struct Condition *condt)
{
    TbBool new_status;
    int plr_start;
    int plr_end;
    if (get_players_range(condt->plyr_range, &plr_start, &plr_end) < 0)
    {
        WARNLOG("Invalid player range %d in CONDITION command %d.",(int)condt->plyr_range,(int)condt->variabl_type);
        return false;
    }
    {
        new_status = false;
        for (long i = plr_start; i < plr_end; i++)
        {
            new_status = action_point_activated_by_player(condt->variabl_idx,i);
            if (new_status) break;
        }
    }
    return new_status;
}

static void add_to_party_process(struct ScriptContext *context)
{
    struct PartyTrigger* pr_trig = context->pr_trig;
    add_member_to_party(pr_trig->party_id, pr_trig->creatr_id, pr_trig->crtr_level, pr_trig->carried_gold, pr_trig->objectv, pr_trig->countdown);
}

static void process_party(struct PartyTrigger* pr_trig)
{
    struct ScriptContext context = {0};
    long n = pr_trig->creatr_id;

    context.pr_trig = pr_trig;

    switch (pr_trig->flags & TrgF_COMMAND_MASK)
    {
    case TrgF_ADD_TO_PARTY:
        add_to_party_process(&context);
        break;
    case TrgF_DELETE_FROM_PARTY:
        delete_member_from_party(pr_trig->party_id, pr_trig->creatr_id, pr_trig->crtr_level);
        break;
    case TrgF_CREATE_OBJECT:
        n |= ((pr_trig->crtr_level & 7) << 7);
        SYNCDBG(6, "Adding object %d at location %d", (int)n, (int)pr_trig->location);
        script_process_new_object(n, pr_trig->location, pr_trig->carried_gold);
        break;
    case TrgF_CREATE_PARTY:
        SYNCDBG(6, "Adding player %d party %d at location %d", (int)pr_trig->plyr_idx, (int)n, (int)pr_trig->location);
        script_process_new_party(&gameadd.script.creature_partys[n],
            pr_trig->plyr_idx, pr_trig->location, pr_trig->ncopies);
        break;
    case TrgF_CREATE_CREATURE:
        SCRIPTDBG(6, "Adding creature %d", n);
        script_process_new_creatures(pr_trig->plyr_idx, n, pr_trig->location,
            pr_trig->ncopies, pr_trig->carried_gold, pr_trig->crtr_level);
        break;
    }
}

void process_check_new_creature_partys(void)
{
    for (long i = 0; i < gameadd.script.party_triggers_num; i++)
    {
        struct PartyTrigger* pr_trig = &gameadd.script.party_triggers[i];
        if ((pr_trig->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(pr_trig->condit_idx))
            {
                process_party(pr_trig);
                if ((pr_trig->flags & TrgF_REUSABLE) == 0)
                    set_flag_byte(&pr_trig->flags, TrgF_DISABLED, true);
            }
      }
    }
}

void process_check_new_tunneller_partys(void)
{
    for (long i = 0; i < gameadd.script.tunneller_triggers_num; i++)
    {
        struct TunnellerTrigger* tn_trig = &gameadd.script.tunneller_triggers[i];
        if ((tn_trig->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(tn_trig->condit_idx))
            {
                long k = tn_trig->party_id;
                if (k > 0)
                {
                    long n = tn_trig->plyr_idx;
                    SCRIPTDBG(6, "Adding tunneler party %d", k);
                    struct Thing* thing = script_process_new_tunneler(n, tn_trig->location, tn_trig->heading,
                        tn_trig->crtr_level, tn_trig->carried_gold);
                    if (!thing_is_invalid(thing))
                    {
                        struct Thing* grptng = script_process_new_party(&gameadd.script.creature_partys[k - 1], n, tn_trig->location, 1);
                        if (!thing_is_invalid(grptng))
                        {
                            add_creature_to_group_as_leader(thing, grptng);
                        }
                        else
                        {
                            WARNLOG("No party created, only lone %s", thing_model_name(thing));
                        }
                    }
                }
                else
                {
                    SCRIPTDBG(6, "Adding tunneler, heading %d", tn_trig->heading);
                    script_process_new_tunneler(tn_trig->plyr_idx, tn_trig->location, tn_trig->heading,
                        tn_trig->crtr_level, tn_trig->carried_gold);
                }
                if ((tn_trig->flags & TrgF_REUSABLE) == 0)
                    tn_trig->flags |= TrgF_DISABLED;
            }
      }
    }
}

void process_win_and_lose_conditions(PlayerNumber plyr_idx)
{
    long i;
    long k;
    struct PlayerInfo* player = get_player(plyr_idx);
    if ((game.system_flags & GSF_NetworkActive) != 0)
      return;
    for (i=0; i < gameadd.script.win_conditions_num; i++)
    {
      k = gameadd.script.win_conditions[i];
      if (is_condition_met(k)) {
          SYNCDBG(8,"Win condition %d (cond. %d) met for player %d.",(int)i,(int)k,(int)plyr_idx);
          set_player_as_won_level(player);
      }
    }
    for (i=0; i < gameadd.script.lose_conditions_num; i++)
    {
      k = gameadd.script.lose_conditions[i];
      if (is_condition_met(k)) {
          SYNCDBG(8,"Lose condition %d (cond. %d) met for player %d.",(int)i,(int)k,(int)plyr_idx);
          set_player_as_lost_level(player);
      }
    }
}


void process_values(void)
{
    for (long i = 0; i < gameadd.script.values_num; i++)
    {
        struct ScriptValue* value = &gameadd.script.values[i];
        if ((value->flags & TrgF_DISABLED) == 0)
        {
            if (is_condition_met(value->condit_idx))
            {
                script_process_value(value->valtype, value->plyr_range, value->arg0, value->arg1, value->arg2, value);
                if ((value->flags & TrgF_REUSABLE) == 0)
                  set_flag_byte(&value->flags, TrgF_DISABLED, true);
            }
        }
    }

    for (int i = 0; i < gameadd.active_fx_lines; i++)
    {
        if (gameadd.fx_lines[i].used)
        {
            process_fx_line(&gameadd.fx_lines[i]);
        }
    }
    for (int i = gameadd.active_fx_lines; i > 0; i--)
    {
        if (gameadd.fx_lines[i-1].used)
        {
            break;
        }
        gameadd.active_fx_lines--;
    }
}


void process_level_script(void)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo *player;
  player = get_my_player();
  // Do NOT stop executing scripts after winning if the RUN_AFTER_VICTORY(1) script command has been issued
  if ((player->victory_state == VicS_Undecided) || (game.system_flags & GSF_RunAfterVictory))
  {
      process_conditions();
      process_check_new_creature_partys();
    //script_process_messages(); is not here, but it is in beta - check why
      process_check_new_tunneller_partys();
      process_values();
      process_win_and_lose_conditions(my_player_number); //player->id_number may be uninitialized yet
    //  show_onscreen_msg(8, "Flags %d %d %d %d %d %d", game.dungeon[0].script_flags[0],game.dungeon[0].script_flags[1],
    //    game.dungeon[0].script_flags[2],game.dungeon[0].script_flags[3],game.dungeon[0].script_flags[4],game.dungeon[0].script_flags[5]);
  }
  SYNCDBG(19,"Finished");
}


/******************************************************************************/

/******************************************************************************/
#ifdef __cplusplus
}
#endif
