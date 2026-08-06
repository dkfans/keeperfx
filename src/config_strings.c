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
#include "kfx_memory.h"
#include "pre_inc.h"
#include "config_strings.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_guibtns.h"
#include "bflib_text.h"

#include "config_mods.h"
#include "config_keeperfx.h"
#include "config_campaigns.h"
#include "config_translation.h"
#include "game_merge.h"
#include "lvl_filesdk1.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

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

TbBool fill_strings_list(char **strings,char *strings_data,char *strings_data_end, int max)
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

    // Compatible with mods. Do not replace empty string.
    if (*text_ptr != '\0')
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

  // Resetting all values to empty strings
  reset_strings(gui_strings, GUI_STRINGS_COUNT-1);

  char* fname = get_game_file_path_fmt(FGrp_FxData, "gtext_%s.dat", get_language_lwrstr(install_info.lang_id));
  if (!fname || !load_gui_strings_data_from_file(fname, 0))
    return false;

  SYNCDBG(19,"Finished");
  return true;
}

TbBool free_gui_strings_data(void)
{
  // Resetting all values to empty strings
  reset_strings(gui_strings, GUI_STRINGS_COUNT-1);
  return true;
}

TbBool load_campaign_strings_data_from_file(const char *fname, unsigned short flags, struct GameCampaign *campgn, uint8_t lang_id)
{
  if (campgn->strings_data_count >= sizeof(campgn->strings_data_list)/sizeof(campgn->strings_data_list[0]))
    return false;

  long filelen = LbFileLengthRnc(fname);
  if (filelen <= 0)
  {
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("Campaign Strings file %s does not exist or can't be opened", fname);
    }
    return false;
  }
  char *raw_data = (char *)KfxCalloc(filelen + 256, 1);
  if (raw_data == NULL)
  {
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("Can't allocate memory for Campaign Strings data");
    }
    return false;
  }
  long loaded_size = LbFileLoadAt(fname, raw_data);
  if (loaded_size < 16)
  {
    KfxFree(raw_data);
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("Campaign Strings file couldn't be loaded or is too small");
    }
    return false;
  }

  size_t out_buf_size = (size_t)loaded_size * 4 + 1;
  char *strings_data = (char *)calloc(out_buf_size, 1);
  if (strings_data == NULL)
  {
    free(raw_data);
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("Can't allocate memory for UTF-8 Campaign Strings data");
    }
    return false;
  }

  size_t utf8_size = convert_codepage_to_utf8_buffer(raw_data, (size_t)loaded_size, strings_data, out_buf_size, lang_id);
  free(raw_data);
  if (utf8_size == 0)
  {
    free(strings_data);
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("Campaign Strings file couldn't be converted to UTF-8");
    }
    return false;
  }

  char* strings_data_end = strings_data + utf8_size;

  campgn->strings_data_list[campgn->strings_data_count] = strings_data;
  campgn->strings_data_count++;

  // Analyzing strings data and filling correct values
  fill_strings_list(campgn->strings, strings_data, strings_data_end, STRINGS_MAX);

  return true;
}

/**
 * Loads the language-specific strings data for the current campaign.
 */
TbBool setup_campaign_strings_data(struct GameCampaign *campgn)
{
  SYNCDBG(18,"Starting");

  // Resetting all values to empty strings
  reset_strings(campgn->strings, STRINGS_MAX);

  char* fname = prepare_file_path(FGrp_Main, campgn->strings_fname);
  if (!load_campaign_strings_data_from_file(fname, 0, campgn, campgn->strings_lang))
  {
    // if the current language of campaign is not translated, then try eng.
    if (campgn->strings_fname_eng[0] == 0 || strcmp(campgn->strings_fname_eng, campgn->strings_fname) == 0)
      return false;

    fname = prepare_file_path(FGrp_Main, campgn->strings_fname_eng);
    if (!load_campaign_strings_data_from_file(fname, 0, campgn, Lang_English))
      return false;
  }

  SYNCDBG(19,"Finished");
  return true;
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
    if (stridx < 0)
    {
      return "invalid string id";
    }
    if (stridx < TRANSLATION_STRINGS_START)
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
    else if (stridx < GUI_STRINGS_START )
        return get_translation_file_string(stridx);
    else
        return gui_string(stridx - GUI_STRINGS_START);
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
