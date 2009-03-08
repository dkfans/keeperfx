/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config.c
 *     Configuration and campaign files support.
 * @par Purpose:
 *     Support for multiple campaigns, switching between levels mechanisms,
       and loading of CFG files.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     30 Jan 2009 - 11 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "config.h"

#include <stdarg.h>
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "scrcapt.h"
#include "vidmode.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_config_file[]="keeperfx.cfg";
const char keeper_campaign_file[]="keepcmpgn.cfg";

const struct LanguageType lang_type[] = {
  {"ENG", 1},
  {"FRE", 2},
  {"GER", 3},
  {"ITA", 4},
  {"SPA", 5},
  {"SWE", 6},
  {"POL", 7},
  {"DUT", 8},
  {NULL,  0},
  };

const struct ConfigCommand scrshot_type[] = {
  {"HSI", 1},
  {"BMP", 2},
  {NULL,  0},
  };

const struct ConfigCommand conf_commands[] = {
  {"INSTALL_PATH",  1},
  {"INSTALL_TYPE",  2},
  {"LANGUAGE",      3},
  {"KEYBOARD",      4},
  {"SCREENSHOT",    5},
  {"FRONTEND_RES",  6},
  {"INGAME_RES",    7},
  {NULL,            0},
  };

const struct ConfigCommand cmpgn_commands[] = {
  {"NAME",          1},
  {"SINGLE_LEVELS", 2},
  {"MULTI_LEVELS",  3},
  {"BONUS_LEVELS",  4},
  {"EXTRA_LEVELS",  5},
  {NULL,            0},
  };

unsigned long features_enabled = 0;
struct GameCampaign campaign;

/******************************************************************************/
short update_features(unsigned long mem_size)
{
  short result;
  result = false;
  if (mem_size >= 32)
  {
    result = true;
    features_enabled |= Ft_HiResCreatr;
  }
  if (mem_size >= 16)
  {
    features_enabled |= Ft_EyeLens;
    features_enabled |= Ft_HiResVideo;
    features_enabled |= Ft_BigPointer;
  }
  LbSyncLog("Memory-demanding features %s.\n",result?"enabled":"disabled");
  return result;
}

int recognize_conf_command(const char *buf,long *pos,long buflen,const struct ConfigCommand *commands)
{
  int i,cmdname_len;
  if ((*pos) >= buflen) return -1;
  // Skipping starting spaces
  while ((buf[*pos] == ' ') || (buf[*pos] == '\t') || (buf[*pos] == '\n') || (buf[*pos] == '\r') || (buf[*pos] == 26) || ((unsigned char)buf[*pos] < 7))
  {
    (*pos)++;
    if ((*pos) >= buflen) return -1;
  }
  // Checking if this line is a comment
  if (buf[*pos] == ';')
    return 0;
  // Finding command number
  i = 0;
  while (commands[i].num > 0)
  {
    cmdname_len = strlen(commands[i].name);
    if ((*pos)+cmdname_len > buflen)
      continue;
    // Find a matching command
    if (strnicmp(buf+(*pos), commands[i].name, cmdname_len) == 0)
    {
      (*pos) += cmdname_len;
      // if we're not at end of input buffer..
      if ((*pos) < buflen)
      {
         // make sure it's whole command, not just start of different one
        if ((buf[(*pos)] != ' ') && (buf[(*pos)] != '\t')
         && (buf[(*pos)] != '=')  && ((unsigned char)buf[(*pos)] >= 7))
        {
           (*pos) -= cmdname_len;
           i++;
           continue;
        }
        // Skipping spaces between command and parameters
        while ((buf[*pos] == ' ') || (buf[*pos] == '\t')
         || (buf[*pos] == '=')  || ((unsigned char)buf[*pos] < 7))
        {
          (*pos)++;
          if ((*pos) >= buflen) break;
        }
      }
      return commands[i].num;
    }
    i++;
  }
  return -2;
}

int get_conf_parameter_whole(const char *buf,long *pos,long buflen,char *dst,long dstlen)
{
  int i;
  if ((*pos) >= buflen) return 0;
  // Skipping spaces after previous parameter
  while ((buf[*pos] == ' ') || (buf[*pos] == '\t'))
  {
    (*pos)++;
    if ((*pos) >= buflen) return 0;
  }
  for (i=0; i+1 < dstlen; i++)
  {
    if ((buf[*pos]=='\r') || (buf[*pos]=='\n') || ((unsigned char)buf[*pos] < 7))
      break;
    dst[i]=buf[*pos];
    (*pos)++;
    if ((*pos) >= buflen) break;
  }
  dst[i]='\0';
  return i;
}

int get_conf_parameter_single(const char *buf,long *pos,long buflen,char *dst,long dstlen)
{
  int i;
  if ((*pos) >= buflen) return 0;
  // Skipping spaces after previous parameter
  while ((buf[*pos] == ' ') || (buf[*pos] == '\t'))
  {
    (*pos)++;
    if ((*pos) >= buflen) return 0;
  }
  for (i=0; i+1 < dstlen; i++)
  {
    if ((buf[*pos] == ' ') || (buf[*pos] == '\t') || (buf[*pos] == '\r')
     || (buf[*pos] == '\n') || ((unsigned char)buf[*pos] < 7))
      break;
    dst[i]=buf[*pos];
    (*pos)++;
    if ((*pos) >= buflen) break;
  }
  dst[i]='\0';
  return i;
}

short skip_conf_to_next_line(const char *buf,long *pos,long buflen)
{
  // Skip to end of the line
  while ((*pos)<buflen)
  {
    if ((buf[*pos]=='\r') || (buf[*pos]=='\n')) break;
    (*pos)++;
  }
  // Go to start of next line
  while ((*pos)<buflen)
  {
    if ((unsigned char)buf[*pos] > 32) break;
    (*pos)++;
  }
  return ((*pos)<buflen);
}

short prepare_diskpath(char *buf,long buflen)
{
  int i;
  i = strlen(buf)-1;
  if (i >= buflen) i = buflen-1;
  if (i<0) return false;
  while (i>0)
  {
    if ((buf[i]!='\\') && (buf[i]!='/') &&
      ((unsigned char)(buf[i]) > 32))
      break;
    i--;
  }
  buf[i+1]='\0';
  return true;
}

short load_configuration(void)
{
  static const char *func_name="load_configuration";
  //return _DK_load_configuration();
  const char *fname;
  char *buf;
  long len,pos;
  int cmd_num;
  // Variables to use when recognizing parameters
  char word_buf[32];
  char *bufpt;
  int i,k;
  // Preparing config file name and checking the file
  strcpy(install_info.inst_path,"");
  install_info.field_9A = 0;
  // Set default rundime directory and load the config file
  strcpy(keeper_runtime_directory,".");
  fname = prepare_file_path(FGrp_Main,keeper_config_file);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    LbWarnLog("Config file \"%s\" doesn't exist or is too small.\n",keeper_config_file);
    return false;
  }
  if (len > 65536)
  {
    LbWarnLog("Config file \"%s\" is too large.\n",keeper_config_file);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  if (len>0)
  {
    pos = 0;
    while (pos<len)
    {
      // Finding command number in this line
      i = 0;
      cmd_num = recognize_conf_command(buf,&pos,len,conf_commands);
      // Now store the config item in correct place
      switch (cmd_num)
      {
      case 1: // INSTALL_PATH
          i=get_conf_parameter_whole(buf,&pos,len,install_info.inst_path,sizeof(install_info.inst_path));
          if (i <= 0)
          {
            LbWarnLog("Couldn't read \"%s\" command parameter in config file.\n","INSTALL_PATH");
            break;
          }
          prepare_diskpath(install_info.inst_path,sizeof(install_info.inst_path));
          break;
      case 2: // INSTALL_TYPE
          // This command is just skipped...
          break;
      case 3: // LANGUAGE
          i = 0;
          while (lang_type[i].num > 0)
          {
            if (strnicmp(buf+pos, lang_type[i].name, strlen(lang_type[i].name)) == 0)
            {
              pos += strlen(lang_type[i].name);
              install_info.field_96 = lang_type[i].num;
              break;
            }
            i++;
          }
          break;
      case 4: // KEYBOARD
          // Works only in DK Premium
          break;
      case 5: // SCREENSHOT
          i = 0;
          while (scrshot_type[i].num > 0)
          {
            if (strnicmp(buf+pos, scrshot_type[i].name, strlen(scrshot_type[i].name)) == 0)
            {
              pos += strlen(scrshot_type[i].name);
              screenshot_format = scrshot_type[i].num;
              break;
            }
            i++;
          }
          break;
      case 6: // FRONTEND_RES
          for (i=0; i<3; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
              k = LbRecogniseVideoModeString(word_buf);
            else
              k = -1;
            if (k<=0)
            {
               LbWarnLog("Couldn't recognize video mode %d in \"%s\" command of config file.\n",i+1,"FRONTEND_RES");
               continue;
            }
            switch (i)
            {
            case 0:
                set_failsafe_vidmode(k);
                break;
            case 1:
                set_movies_vidmode(k);
                break;
            case 2:
                set_frontend_vidmode(k);
                break;
            }
          }
          break;
      case 7: // INGAME_RES
          for (i=0; i<7; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = LbRecogniseVideoModeString(word_buf);
              if (k > 0)
                set_game_vidmode(i,k);
              else
                LbWarnLog("Couldn't recognize video mode %d in \"%s\" command of config file.\n",i+1,"INGAME_RES");
            } else
            {
              if (i > 0)
                set_game_vidmode(i,Lb_SCREEN_MODE_INVALID);
              else
                LbWarnLog("Video modes list empty in \"%s\" command of config file.\n","INGAME_RES");
              break;
            }
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          LbWarnLog("Unrecognized command in config file, starting on byte %d.\n",pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  switch (install_info.field_96)
  {
  case 1:
      LbKeyboardSetLanguage(1);
      break;
  case 2:
      LbKeyboardSetLanguage(2);
      break;
  case 3:
      LbKeyboardSetLanguage(3);
      break;
  case 4:
      LbKeyboardSetLanguage(4);
      break;
  case 5:
      LbKeyboardSetLanguage(5);
      break;
  case 6:
      LbKeyboardSetLanguage(6);
      break;
  case 7:
      LbKeyboardSetLanguage(7);
      break;
  case 8:
      LbKeyboardSetLanguage(8);
      break;
  default:
      break;
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return (install_info.field_96>0) && (install_info.inst_path[0]!='\0');
}

char *prepare_file_path_buf(char *ffullpath,short fgroup,const char *fname)
{
  const char *mdir;
  const char *sdir;
  switch (fgroup)
  {
  case FGrp_StdData:
      mdir=keeper_runtime_directory;
      sdir="data";
      break;
  case FGrp_LrgData:
      mdir=keeper_runtime_directory;
      sdir="data";
      break;
  case FGrp_FxData:
      mdir=keeper_runtime_directory;
      sdir="fxdata";
      break;
  case FGrp_LoData:
      mdir=install_info.inst_path;
      sdir="ldata";
      break;
  case FGrp_HiData:
      mdir=keeper_runtime_directory;
      sdir="hdata";
      break;
  case FGrp_Levels:
      mdir=install_info.inst_path;
      sdir="levels";
      break;
  case FGrp_Save:
      mdir=keeper_runtime_directory;
      sdir="save";
      break;
  case FGrp_SShots:
      mdir=keeper_runtime_directory;
      sdir="scrshots";
      break;
  case FGrp_StdSound:
      mdir=keeper_runtime_directory;
      sdir="sound";
      break;
  case FGrp_LrgSound:
      mdir=keeper_runtime_directory;
      sdir="sound";
      break;
  case FGrp_Main:
      mdir=keeper_runtime_directory;
      sdir=NULL;
      break;
  case FGrp_Campgn:
      mdir=keeper_runtime_directory;
      sdir="fxdata";
      break;
  default:
      mdir="./";
      sdir=NULL;
      break;
  }
  if (sdir != NULL)
    sprintf(ffullpath,"%s/%s/%s",mdir,sdir,fname);
  else
    sprintf(ffullpath,"%s/%s",mdir,fname);
  return ffullpath;
}

char *prepare_file_path(short fgroup,const char *fname)
{
  static char ffullpath[2048];
  return prepare_file_path_buf(ffullpath,fgroup,fname);
}

char *prepare_file_path_va(short fgroup, const char *fmt_str, va_list arg)
{
  static char ffullpath[2048];
  char fname[255];
  vsprintf(fname, fmt_str, arg);
  return prepare_file_path_buf(ffullpath,fgroup,fname);
}

char *prepare_file_fmtpath(short fgroup, const char *fmt_str, ...)
{
  char *result;
  va_list val;
  va_start(val, fmt_str);
  result=prepare_file_path_va(fgroup, fmt_str, val);
  va_end(val);
  return result;
}

short clear_campaign(struct GameCampaign *campgn)
{
  int i;
  memset (campgn->name,0,LINEMSG_SIZE);
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    campgn->single_levels[i]=0;
    campgn->multi_levels[i]=0;
    campgn->bonus_levels[i]=0;
    campgn->extra_levels[i]=0;
  }
  campgn->single_levels[0]=1;
  return true;
}

short load_campaign(const char *cmpgn_fname,struct GameCampaign *campgn)
{
  static const char *func_name="load_campaign";
  char *fname;
  char *buf;
  long len,pos;
  int cmd_num;
  // Variables to use when recognizing parameters
  char word_buf[32];
  int i,k;
  // Preparing campaign file name and checking the file
  clear_campaign(campgn);
  fname=prepare_file_path(FGrp_Campgn,cmpgn_fname);
  len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    LbWarnLog("Campaign file \"%s\" doesn't exist or is too small.\n",cmpgn_fname);
    return false;
  }
  if (len > 65536)
  {
    LbWarnLog("Campaign file \"%s\" is too large.\n",cmpgn_fname);
    return false;
  }
  buf = (char *)LbMemoryAlloc(len+256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  if (len>0)
  {
    pos = 0;
    while (pos<len)
    {
      // Finding command number in this line
      i = 0;
      cmd_num = recognize_conf_command(buf,&pos,len,cmpgn_commands);
      // Now store the config item in correct place
      switch (cmd_num)
      {
      case 1: // NAME
          i=get_conf_parameter_whole(buf,&pos,len,campgn->name,LINEMSG_SIZE);
          if (i <= 0)
            LbWarnLog("Couldn't read \"%s\" command parameter in config file.\n","NAME");
          break;
      case 2: // SINGLE_LEVELS
          for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
                campgn->single_levels[i] = k;
              else
                LbWarnLog("Couldn't recognize level in \"%s\" command of campaign file.\n","SINGLE_LEVELS");
            } else
            {
              if (i > 0)
                campgn->single_levels[i] = 0;
              else
                LbWarnLog("Levels list empty in \"%s\" command of campaign file.\n","SINGLE_LEVELS");
              break;
            }
          }
          break;
      case 3: // MULTI_LEVELS
          for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
                campgn->multi_levels[i] = k;
              else
                LbWarnLog("Couldn't recognize level in \"%s\" command of campaign file.\n","MULTI_LEVELS");
            } else
            {
              if (i > 0)
                campgn->multi_levels[i] = 0;
              else
                LbWarnLog("Levels list empty in \"%s\" command of campaign file.\n","MULTI_LEVELS");
              break;
            }
          }
          break;
      case 4: // BONUS_LEVELS
          for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k >= 0) // Some bonus levels may not exist
                campgn->bonus_levels[i] = k;
              else
                LbWarnLog("Couldn't recognize level in \"%s\" command of campaign file.\n","BONUS_LEVELS");
            } else
            {
              if (i > 0)
                campgn->bonus_levels[i] = 0;
              else
                LbWarnLog("Levels list empty in \"%s\" command of campaign file.\n","BONUS_LEVELS");
              break;
            }
          }
          break;
      case 5: // EXTRA_LEVELS
          for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = atoi(word_buf);
              if (k > 0)
                campgn->extra_levels[i] = k;
              else
                LbWarnLog("Couldn't recognize level in \"%s\" command of campaign file.\n","EXTRA_LEVELS");
            } else
            {
              if (i > 0)
                campgn->extra_levels[i] = 0;
              else
                LbWarnLog("Levels list empty in \"%s\" command of campaign file.\n","EXTRA_LEVELS");
              break;
            }
          }
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          LbWarnLog("Unrecognized command in campaign file \"%s\", starting on byte %d.\n",cmpgn_fname,pos);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
  }
  //Freeing and exiting
  LbMemoryFree(buf);
  return true;
}

short load_default_campaign(void)
{
  return load_campaign(keeper_campaign_file,&campaign);
}

short is_bonus_level(long levidx)
{
  int i;
  if (levidx < 1) return false;
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.bonus_levels[i] == levidx)
        return true;
    if (campaign.extra_levels[i] == levidx)
        return true;
  }
  return false;
}

/*
 * Returns index for Game->bonus_levels associated with given single player level.
 */
int array_index_for_levels_bonus(long levidx)
{
  int i,k;
  if (levidx < 1) return -1;
  k=0;
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == levidx)
        return k;
    if (campaign.bonus_levels[i] != 0)
        k++;
  }
  return -1;
}

long first_singleplayer_level(void)
{
  return campaign.single_levels[0];
}

/*
 * Returns the next single player level. Gives -2 if last level was won,
 * -1 on error.
 */
long next_singleplayer_level(long levidx)
{
  int i;
  if (levidx < 1) return -1;
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == levidx)
    {
      if (i+1 >= CAMPAIGN_LEVELS_COUNT)
        return -2;
      if (campaign.single_levels[i+1] <= 0)
        return -2;
      return campaign.single_levels[i+1];
    }
  }
  return false;
}

short is_singleplayer_level(long levidx)
{
  static const char *func_name="is_singleplayer_level";
  int i;
  if (levidx < 1)
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Level index %ld is not correct\n",func_name,levidx);
  #endif
    return false;
  }
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == levidx)
    {
  #if (BFDEBUG_LEVEL > 7)
      LbSyncLog("%s: Level %ld identified as SP\n",func_name,levidx);
  #endif
        return true;
    }
  }
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Level %ld not recognized as SP\n",func_name,levidx);
#endif
  return false;
}

short is_original_singleplayer_level(long levidx)
{
    if ((levidx>=1)&&(levidx<21))
        return true;
    return false;
}

short is_multiplayer_level(long levidx)
{
  int i;
  if (levidx < 1) return false;
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.multi_levels[i] == levidx)
        return true;
  }
  return false;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
