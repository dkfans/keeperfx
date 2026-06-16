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

#include "config_mods.h"
#include "config_keeperfx.h"
#include "config_campaigns.h"
#include "config_translation.h"
#include "game_merge.h"
#include "lvl_filesdk1.h"
#include "kfx/modding/mod_api.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char *gui_strings_data_list[MOD_ITEM_MAX*MOD_ITEM_TYPE_CNT+1] = {0};
int gui_strings_data_count = 0;
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

static TbBool load_gui_strings_data_from_file(const char *fname, unsigned short flags)
{
  if (gui_strings_data_count >= sizeof(gui_strings_data_list)/sizeof(gui_strings_data_list[0]))
    return false;

  long filelen = LbFileLengthRnc(fname);
  if (filelen <= 0)
  {
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("GUI Strings file does not exist or can't be opened");
      SYNCLOG("Strings file name is \"%s\"",fname);
    }
    return false;
  }
  char *gui_strings_data = (char *)calloc(filelen + 256, 1);
  if (gui_strings_data == NULL)
  {
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("Can't allocate memory for GUI Strings data");
      SYNCLOG("Strings file name is \"%s\"",fname);
    }
    return false;
  }
  char* strings_data_end = gui_strings_data + filelen + 255;
  long loaded_size = LbFileLoadAt(fname, gui_strings_data);
  if (loaded_size < 16)
  {
    free(gui_strings_data);
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("GUI Strings file couldn't be loaded or is too small");
    }
    return false;
  }

  gui_strings_data_list[gui_strings_data_count] = gui_strings_data;
  gui_strings_data_count++;

  // Analyzing strings data and filling correct values
  fill_strings_list(gui_strings, gui_strings_data, strings_data_end, GUI_STRINGS_COUNT-1);

  return true;
}

static const ModLocation s_gui_str_locs[] = {
  { ModTier_AfterBase, (short)FGrp_FxData, ModLifetime_Startup,
    offsetof(struct ModExistState, fx_data), ModRes_Directory, NULL },
};
static KfxModHandle s_gui_str_walker = NULL;

static void on_gui_str_fxdata_dir(const char *fxdata_dir, void *userdata)
{
  char fname[512];
  snprintf(fname, sizeof(fname), "%s/gtext_%s.dat", fxdata_dir, get_language_lwrstr(install_info.lang_id));
  if (!load_gui_strings_data_from_file(fname, CnfLd_IgnoreErrors) && install_info.lang_id != Lang_English)
  {
    snprintf(fname, sizeof(fname), "%s/gtext_%s.dat", fxdata_dir, get_language_lwrstr(Lang_English));
    load_gui_strings_data_from_file(fname, CnfLd_IgnoreErrors);
  }
}

/**
 * Loads the language-specific strings data for game interface.
 */
TbBool setup_gui_strings_data(void)
{
  SYNCDBG(8,"Starting");

  // Resetting all values to empty strings
  reset_strings(gui_strings, GUI_STRINGS_COUNT-1);

  char* fname = prepare_file_fmtpath(FGrp_FxData, "gtext_%s.dat", get_language_lwrstr(install_info.lang_id));
  if (!load_gui_strings_data_from_file(fname, 0))
    return false;

  // Default only one dat file as base and must exist.
  // So for mods, ignore mod section stage, keep only the mod order.

  if (s_gui_str_walker == NULL)
    s_gui_str_walker = kfx_mod_create_walker(s_gui_str_locs, sizeof(s_gui_str_locs) / sizeof(s_gui_str_locs[0]));
  kfx_mod_visit(s_gui_str_walker, NULL, on_gui_str_fxdata_dir, NULL);

  SYNCDBG(19,"Finished");
  return true;
}

TbBool free_gui_strings_data(void)
{
  // Resetting all values to empty strings
  reset_strings(gui_strings, GUI_STRINGS_COUNT-1);
  // Freeing memory
  for (int i=0; i<gui_strings_data_count; i++)
  {
    free(gui_strings_data_list[i]);
    gui_strings_data_list[i] = NULL;
  }
  gui_strings_data_count = 0;
  return true;
}


TbBool load_campaign_strings_data_from_file(const char *fname, unsigned short flags, struct GameCampaign *campgn)
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
  char *strings_data = (char *)calloc(filelen + 256, 1);
  if (strings_data == NULL)
  {
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("Can't allocate memory for Campaign Strings data");
    }
    return false;
  }
  char* strings_data_end = strings_data + filelen + 255;
  long loaded_size = LbFileLoadAt(fname, strings_data);
  if (loaded_size < 16)
  {
    free(strings_data);
    if ((flags & CnfLd_IgnoreErrors) == 0)
    {
      ERRORLOG("Campaign Strings file couldn't be loaded or is too small");
    }
    return false;
  }

  campgn->strings_data_list[campgn->strings_data_count] = strings_data;
  campgn->strings_data_count++;

  // Analyzing strings data and filling correct values
  fill_strings_list(campgn->strings, strings_data, strings_data_end, STRINGS_MAX);

  return true;
}

static const ModLocation s_camp_str_locs[] = {
  { ModTier_AfterBase,     (short)FGrp_Main, ModLifetime_Startup,
    offsetof(struct ModExistState, mod_dir), ModRes_Directory, NULL },
  { ModTier_AfterCampaign, (short)FGrp_Main, ModLifetime_Campaign,
    offsetof(struct ModExistState, mod_dir), ModRes_Directory, NULL },
  { ModTier_AfterMap,      (short)FGrp_Main, ModLifetime_Level,
    offsetof(struct ModExistState, mod_dir), ModRes_Directory, NULL },
};
static KfxModHandle s_camp_str_walker = NULL;

static void on_camp_str_mod_dir(const char *mod_dir_path, void *userdata)
{
  struct GameCampaign *campgn = (struct GameCampaign *)userdata;
  char fname[512];
  snprintf(fname, sizeof(fname), "%s/%s", mod_dir_path, campgn->strings_fname);
  if (!load_campaign_strings_data_from_file(fname, CnfLd_IgnoreErrors, campgn))
  {
    if (campgn->strings_fname_eng[0] != 0 && strcmp(campgn->strings_fname_eng, campgn->strings_fname) != 0)
    {
      snprintf(fname, sizeof(fname), "%s/%s", mod_dir_path, campgn->strings_fname_eng);
      load_campaign_strings_data_from_file(fname, CnfLd_IgnoreErrors, campgn);
    }
  }
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
  if (!load_campaign_strings_data_from_file(fname, 0, campgn))
  {
    // if the current language of campaign is not translated, then try eng.
    if (campgn->strings_fname_eng[0] == 0 || strcmp(campgn->strings_fname_eng, campgn->strings_fname) == 0)
      return false;

    fname = prepare_file_path(FGrp_Main, campgn->strings_fname_eng);
    if (!load_campaign_strings_data_from_file(fname, 0, campgn))
      return false;
  }

  // Default only one dat file as base and must exist.
  // So for mods, ignore mod section stage, keep only the mod order.

  if (s_camp_str_walker == NULL)
    s_camp_str_walker = kfx_mod_create_walker(s_camp_str_locs, sizeof(s_camp_str_locs) / sizeof(s_camp_str_locs[0]));
  kfx_mod_visit(s_camp_str_walker, NULL, on_camp_str_mod_dir, campgn);

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
