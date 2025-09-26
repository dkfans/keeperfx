/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_strings.c
 *     List of language-specific strings support.
 * @par Purpose:
 *     Support of configuration files for game strings.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     19 Nov 2011 - 01 Aug 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_strings.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_guibtns.h"

#include "config_keeperfx.h"
#include "config_campaigns.h"
#include "game_merge.h"
#include "lvl_filesdk1.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char *gui_strings_data;
char *gui_strings[GUI_STRINGS_COUNT];
/******************************************************************************/
TbBool reset_strings(char **strings, int max)
{
    char** text_arr = strings;
    int text_idx = max;
    while (text_idx >= 0)
    {
        *text_arr = "";
        text_arr++;
        text_idx--;
  }
  return true;
}

TbBool create_strings_list(char **strings,char *strings_data,char *strings_data_end, int max)
{
    char** text_arr = strings;
    int text_idx = max;
    char* text_ptr = strings_data;
    while (text_idx >= 0)
    {
        if (text_ptr >= strings_data_end)
        {
            break;
        }
        *text_arr = text_ptr;
        text_arr++;
        char chr_prev;
        do
        {
            chr_prev = *text_ptr;
            text_ptr++;
    } while ((chr_prev != '\0') && (text_ptr < strings_data_end));
    text_idx--;
  }
  return (text_idx < max);
}

/**
 * Loads the language-specific strings data for game interface.
 */
TbBool setup_gui_strings_data(void)
{
  SYNCDBG(8,"Starting");

  char* fname = prepare_file_fmtpath(FGrp_FxData, "gtext_%s.dat", get_language_lwrstr(install_info.lang_id));
  long filelen = LbFileLengthRnc(fname);
  if (filelen <= 0)
  {
    ERRORLOG("GUI Strings file does not exist or can't be opened");
    SYNCLOG("Strings file name is \"%s\"",fname);
    return false;
  }
  gui_strings_data = (char *)calloc(filelen + 256, 1);
  if (gui_strings_data == NULL)
  {
    ERRORLOG("Can't allocate memory for GUI Strings data");
    SYNCLOG("Strings file name is \"%s\"",fname);
    return false;
  }
  char* strings_data_end = gui_strings_data + filelen + 255;
  long loaded_size = LbFileLoadAt(fname, gui_strings_data);
  if (loaded_size < 16)
  {
    ERRORLOG("GUI Strings file couldn't be loaded or is too small");
    return false;
  }
  // Resetting all values to empty strings
  reset_strings(gui_strings, GUI_STRINGS_COUNT-1);
  // Analyzing strings data and filling correct values
  short result = create_strings_list(gui_strings, gui_strings_data, strings_data_end, GUI_STRINGS_COUNT-1);
  SYNCDBG(19,"Finished");
  return result;
}

TbBool free_gui_strings_data(void)
{
  // Resetting all values to empty strings
  reset_strings(gui_strings, GUI_STRINGS_COUNT-1);
  // Freeing memory
  free(gui_strings_data);
  gui_strings_data = NULL;
  return true;
}

/**
 * Loads the language-specific strings data for the current campaign.
 */
TbBool setup_campaign_strings_data(struct GameCampaign *campgn)
{
  SYNCDBG(18,"Starting");
  char* fname = prepare_file_path(FGrp_Main, campgn->strings_fname);
  long filelen = LbFileLengthRnc(fname);
  if (filelen <= 0)
  {
    ERRORLOG("Campaign Strings file %s does not exist or can't be opened", campgn->strings_fname);
    return false;
  }
  campgn->strings_data = (char *)calloc(filelen + 256, 1);
  if (campgn->strings_data == NULL)
  {
    ERRORLOG("Can't allocate memory for Campaign Strings data");
    return false;
  }
  char* strings_data_end = campgn->strings_data + filelen + 255;
  long loaded_size = LbFileLoadAt(fname, campgn->strings_data);
  if (loaded_size < 16)
  {
    ERRORLOG("Campaign Strings file couldn't be loaded or is too small");
    return false;
  }
  // Resetting all values to empty strings
  reset_strings(campgn->strings, STRINGS_MAX);
  // Analyzing strings data and filling correct values
  TbBool result = create_strings_list(campgn->strings, campgn->strings_data, strings_data_end, STRINGS_MAX);
  SYNCDBG(19,"Finished");
  return result;
}

const char * gui_string(unsigned int index)
{
    static char string_invalid[64];

    if (index >= GUI_STRINGS_COUNT)
    {
        snprintf(string_invalid, sizeof(string_invalid), "untranslated <%d>", index);
        return string_invalid;
    }
    return gui_strings[index];
}

const char * cmpgn_string(unsigned int index)
{
    if (index >= STRINGS_MAX)
    {
        return gui_string(index - STRINGS_MAX);
    }
    if (*campaign.strings[index] != '\0')
    {
        return campaign.strings[index];
    }
    return gui_string(index);
}

const char * get_string(TextStringId stridx)
{
    if (stridx <= STRINGS_MAX)
    {
        if (level_strings[stridx] != NULL)
        {
            if (*level_strings[stridx] != '\0')
            {
                return level_strings[stridx];
            }
        }
        return cmpgn_string(stridx);
    }
    else
        return gui_string(stridx-STRINGS_MAX);
}

unsigned long count_strings(char *strings, int size)
{
    unsigned long result = 0;
    char *s = strings;
    char *end = strings + size;
    while (s <= end)
    {
        if (*s == '\0')
        {
            result++;
        }
        s++;
    }
    return result;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
