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
#include "bflib_math.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_mouse.h"
#include "bflib_sound.h"
#include "sounds.h"

#include "config_campaigns.h"
#include "front_simple.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "music_player.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_config_file[]="keeperfx.cfg";
int max_track = 7;
unsigned short AtmosRepeat = 1013;
unsigned short AtmosStart = 1014;
unsigned short AtmosEnd = 1034;
TbBool AssignCpuKeepers = 0;

/**
 * Language 3-char abbreviations.
 * These are selected from ISO 639-2/B naming standard.
 */
const struct NamedCommand lang_type[] = {
  {"ENG", Lang_English},
  {"FRE", Lang_French},
  {"GER", Lang_German},
  {"ITA", Lang_Italian},
  {"SPA", Lang_Spanish},
  {"SWE", Lang_Swedish},
  {"POL", Lang_Polish},
  {"DUT", Lang_Dutch},
  {"HUN", Lang_Hungarian},
  {"KOR", Lang_Korean},
  {"DAN", Lang_Danish},
  {"NOR", Lang_Norwegian},
  {"CZE", Lang_Czech},
  {"ARA", Lang_Arabic},
  {"RUS", Lang_Russian},
  {"JPN", Lang_Japanese},
  {"CHI", Lang_ChineseInt}, // Simplified Chinese
  {"CHT", Lang_ChineseTra}, // Traditional Chinese (not from ISO 639-2/B)
  {"POR", Lang_Portuguese},
  {"HIN", Lang_Hindi},
  {"BEN", Lang_Bengali},
  {"JAV", Lang_Javanese},
  {"LAT", Lang_Latin}, // Classic Latin
  {NULL,  Lang_Unset},
  };

const struct NamedCommand scrshot_type[] = {
  {"HSI", 1},
  {"BMP", 2},
  {NULL,  0},
  };

const struct NamedCommand atmos_volume[] = {
  {"LOW",     64},
  {"MEDIUM", 128},
  {"HIGH",   255},
  {NULL,  0},
  };

const struct NamedCommand atmos_freq[] = {
  {"LOW",    3200},
  {"MEDIUM",  800},
  {"HIGH",    400},
  {NULL,  0},
  };

const struct NamedCommand conf_commands[] = {
  {"INSTALL_PATH",         1},
  {"INSTALL_TYPE",         2},
  {"LANGUAGE",             3},
  {"KEYBOARD",             4},
  {"SCREENSHOT",           5},
  {"FRONTEND_RES",         6},
  {"INGAME_RES",           7},
  {"CENSORSHIP",           8},
  {"POINTER_SENSITIVITY",  9},
  {"ATMOSPHERIC_SOUNDS",  10},
  {"ATMOS_VOLUME",        11},
  {"ATMOS_FREQUENCY",     12},
  {"ATMOS_SAMPLES",       13},
  {"RESIZE_MOVIES",       14},
  {"MUSIC_TRACKS",        15},
  {"WIBBLE",              16},
  {"FREEZE_GAME_ON_FOCUS_LOST"     , 17},
  {"UNLOCK_CURSOR_WHEN_GAME_PAUSED", 18},
  {"LOCK_CURSOR_IN_POSSESSION"     , 19},
  {"PAUSE_MUSIC_WHEN_GAME_PAUSED"  , 20},
  {"MUTE_AUDIO_ON_FOCUS_LOST"      , 21},
  {NULL,                   0},
  };

const struct NamedCommand logicval_type[] = {
  {"ENABLED",  1},
  {"DISABLED", 2},
  {"ON",       1},
  {"OFF",      2},
  {"TRUE",     1},
  {"FALSE",    2},
  {"YES",      1},
  {"NO",       2},
  {"1",        1},
  {"0",        2},
  {NULL,       0},
  };

  const struct NamedCommand wibble_type[] = {
  {"ON",             1},
  {"OFF",            2},
  {"LIQUIDONLY",     3},
  {NULL,             0},
  };

  const struct NamedCommand vidscale_type[] = {
  {"OFF",          256}, // = 0x100 = No scaling of Smacker Video
  {"DISABLED",     256},
  {"FALSE",        256},
  {"NO",           256},
  {"0",            256},
  {"FIT",           16}, // = 0x10 = SMK_FullscreenFit - fit to fullscreen, using letterbox and pillarbox as necessary
  {"ON",            16}, // Duplicate of FIT, for legacy reasons
  {"ENABLED",       16},
  {"TRUE",          16},
  {"YES",           16},
  {"1",             16},
  {"STRETCH",       32}, // = 0x20 = SMK_FullscreenStretch  - stretch to fullscreen - ignores aspect ratio difference between source and destination
  {"CROP",          64}, // = 0x40 = SMK_FullscreenCrop - fill fullscreen and crop - no letterbox or pillarbox
  {"4BY3",          48}, // = 0x10 & 0x20 = [Aspect Ratio correction mode] - stretch 320x200 to 4:3 (i.e. increase height by 1.2)
  {"PIXELPERFECT",  80}, // = 0x10 & 0x40 = integer multiple scale only (FIT)
  {"4BY3PP",       112}, // = 0x10 & 0x20 & 0x40 = integer multiple scale only (4BY3)
  {NULL,             0},
  };
unsigned int vid_scale_flags = 0;

unsigned long features_enabled = 0;
/** Line number, used when loading text files. */
unsigned long text_line_number;

short is_full_moon = 0;
short is_near_full_moon = 0;
short is_new_moon = 0;
short is_near_new_moon = 0;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
/**
 * Updates enabled features flags. Returns true if ALL features are enabled.
 * @param mem_size Amount of memory available for the game.
 * @return
 */
TbBool update_features(unsigned long uf_mem_size)
{
    short result = false;
    if (uf_mem_size >= 32)
    {
        result = true;
        features_enabled |= Ft_HiResCreatr;
  }
  if (uf_mem_size >= 16)
  {
    features_enabled |= Ft_EyeLens;
    features_enabled |= Ft_HiResVideo;
    features_enabled |= Ft_BigPointer;
    features_enabled |= Ft_AdvAmbSound;
  }
  SYNCMSG("Memory-demanding features %s.",result?"enabled":"disabled");
  return result;
}

/**
 * Returns if the censorship is on. This mostly affects blood.
 * Originally, censorship was on for german language.
 */
TbBool censorship_enabled(void)
{
  return ((features_enabled & Ft_Censorship) != 0);
}

/**
 * Returns if Athmospheric sound is on.
 */
TbBool atmos_sounds_enabled(void)
{
  return ((features_enabled & Ft_Atmossounds) != 0);
}

/**
 * Returns if Resize Movie is on.
 */
TbBool resize_movies_enabled(void)
{
  return ((features_enabled & Ft_Resizemovies) != 0);
}

/**
 * Returns if the wibble effect is on.
 */
TbBool wibble_enabled(void)
{
  return ((features_enabled & Ft_Wibble) != 0);
}

#include "game_legacy.h" // it would be nice to not have to include this
/**
 * Returns if we should freeze the game, if the game window loses focus.
 */
TbBool freeze_game_on_focus_lost(void)
{
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        return false;
    }
  return ((features_enabled & Ft_FreezeOnLoseFocus) != 0);
}

/**
 * Returns if we should unlock the mouse cursor from the window, if the user pauses the game.
 */
TbBool unlock_cursor_when_game_paused(void)
{
  return ((features_enabled & Ft_UnlockCursorOnPause) != 0);
}

/**
 * Returns if we should lock the mouse cursor to the window, when the user enters possession mode (when the cursor is already unlocked).
 */
TbBool lock_cursor_in_possession(void)
{
  return ((features_enabled & Ft_LockCursorInPossession) != 0);
}

/**
 * Returns if we should pause the music, if the user pauses the game.
 */
TbBool pause_music_when_game_paused(void)
{
  return ((features_enabled & Ft_PauseMusicOnGamePause) != 0);
}

/**
 * Returns if we should mute the game audio, if the game window loses focus.
 */
TbBool mute_audio_on_focus_lost(void)
{
  return ((features_enabled & Ft_MuteAudioOnLoseFocus) != 0);
}
  
/**
 * Returns if the liquid wibble effect is on.
 */
TbBool liquid_wibble_enabled(void)
{
  return ((features_enabled & Ft_LiquidWibble) != 0);
}

TbBool is_feature_on(unsigned long feature)
{
  return ((features_enabled & feature) != 0);
}

TbBool skip_conf_to_next_line(const char *buf,long *pos,long buflen)
{
  // Skip to end of the line
  while ((*pos) < buflen)
  {
    if ((buf[*pos]=='\r') || (buf[*pos]=='\n')) break;
    (*pos)++;
  }
  // Go to start of next line
  while ((*pos) < buflen)
  {
    if ((unsigned char)buf[*pos] > 32) break;
    if (buf[*pos]=='\n')
      text_line_number++;
    (*pos)++;
  }
  return ((*pos) < buflen);
}

TbBool skip_conf_spaces(const char *buf, long *pos, long buflen)
{
  while ((*pos) < buflen)
  {
    if ((buf[*pos]!=' ') && (buf[*pos]!='\t') && (buf[*pos] != 26) && ((unsigned char)buf[*pos] >= 7)) break;
    (*pos)++;
  }
  return ((*pos) < buflen);
}

/**
 * Searches for start of INI file block with given name.
 * Starts at position given with pos, and sets it to position of block data.
 * @return Returns 1 if the block is found, -1 if buffer exceeded.
 */
short find_conf_block(const char *buf,long *pos,long buflen,const char *blockname)
{
  text_line_number = 1;
  int blname_len = strlen(blockname);
  while ((*pos)+blname_len+2 < buflen)
  {
    // Skipping starting spaces
    if (!skip_conf_spaces(buf,pos,buflen))
      break;
    // Checking if this line is start of a block
    if (buf[*pos] != '[')
    {
      skip_conf_to_next_line(buf,pos,buflen);
      continue;
    }
    (*pos)++;
    // Skipping any spaces
    if (!skip_conf_spaces(buf,pos,buflen))
      break;
    if ((*pos)+blname_len+2 >= buflen)
      break;
    if (strncasecmp(&buf[*pos],blockname,blname_len) != 0)
    {
      skip_conf_to_next_line(buf,pos,buflen);
      continue;
    }
    (*pos)+=blname_len;
    // Skipping any spaces
    if (!skip_conf_spaces(buf,pos,buflen))
      break;
    if (buf[*pos] != ']')
    {
      skip_conf_to_next_line(buf,pos,buflen);
      continue;
    }
    skip_conf_to_next_line(buf,pos,buflen);
    return 1;
  }
  return -1;
}

/**
 * Recognizes config command and returns its number, or negative status code.
 * @param buf
 * @param pos
 * @param buflen
 * @param commands
 * @return If positive integer is returned, it is the command number recognized in the line.
 * If 0 is returned, that means the current line did not contained any command and should be skipped.
 * If -1 is returned, that means we've reached end of file.
 * If -2 is returned, that means the command wasn't recognized.
 * If -3 is returned, that means we've reached end of the INI block.
 */
int recognize_conf_command(const char *buf,long *pos,long buflen,const struct NamedCommand commands[])
{
    SYNCDBG(19,"Starting");
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
    // Checking if this line is start of a block
    if (buf[*pos] == '[')
        return -3;
    // Finding command number
    int i = 0;
    while (commands[i].num > 0)
    {
        int cmdname_len = strlen(commands[i].name);
        if ((*pos)+cmdname_len > buflen) {
            i++;
            continue;
        }
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

int get_conf_parameter_quoted(const char *buf,long *pos,long buflen,char *dst,long dstlen)
{
    int i;
    TbBool esc = false;
    if ((*pos) >= buflen) return 0;
    // Skipping spaces after previous parameter
    while ((buf[*pos] == ' ') || (buf[*pos] == '\t'))
    {
        (*pos)++;
        if ((*pos) >= buflen) return 0;
    }
    // first quote
    if (buf[*pos] != '"')
        return 0;
    (*pos)++;

    for (i=0; i+1 < dstlen;)
    {
        if ((*pos) >= buflen) {
            return 0; // End before quote
        }
        if (!esc)
        {
            if (buf[*pos] == '\\')
            {
                esc = true;
                (*pos)++;
                continue;
            }
            else if (buf[*pos] == '"')
            {
                (*pos)++;
                break;
            }
        }
        else
        {
            esc = false;
        }
        dst[i++]=buf[*pos];
        (*pos)++;
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
        if ((*pos) >= buflen) {
            i++;
            break;
        }
    }
    dst[i]='\0';
    return i;
}

int get_conf_list_int(const char *buf, const char **state, int *dst)
{
    int len = -1;
    if (*state == NULL)
    {
        if (1 != sscanf(buf, " %d%n", dst, &len))
        {
            return 0;
        }
        *state = buf + len;
        return 1;
    }
    else
    {
        if (1 != sscanf(*state, " , %d%n", dst, &len))
        {
            return 0;
        }
        *state = *state + len;
        return 1;
    }
}
/**
 * Returns parameter num from given NamedCommand array, or 0 if not found.
 */
int recognize_conf_parameter(const char *buf,long *pos,long buflen,const struct NamedCommand commands[])
{
  if ((*pos) >= buflen) return 0;
  // Skipping spaces after previous parameter
  while ((buf[*pos] == ' ') || (buf[*pos] == '\t'))
  {
    (*pos)++;
    if ((*pos) >= buflen) return 0;
  }
  int i = 0;
  while (commands[i].name != NULL)
  {
      int par_len = strlen(commands[i].name);
      if (strncasecmp(&buf[(*pos)], commands[i].name, par_len) == 0)
      {
          // If EOLN found, finish and return position before the EOLN
          if ((buf[(*pos)+par_len] == '\n') || (buf[(*pos)+par_len] == '\r'))
          {
            (*pos) += par_len;
            return commands[i].num;
          }
          // If non-EOLN blank char, finish and return position after the char
          if ((buf[(*pos)+par_len] == ' ') || (buf[(*pos)+par_len] == '\t')
           || ((unsigned char)buf[(*pos)+par_len] < 7))
          {
            (*pos) += par_len+1;
            return commands[i].num;
          }
      }
      i++;
  }
  return 0;
}

/**
 * Returns name of a config parameter with given number, or empty string.
 */
const char *get_conf_parameter_text(const struct NamedCommand commands[],int num)
{
    long i = 0;
    while (commands[i].name != NULL)
    {
        //SYNCLOG("\"%s\", %d %d",commands[i].name,commands[i].num,num);
        if (commands[i].num == num)
            return commands[i].name;
        i++;
  }
  return lbEmptyString;
}

/**
 * Returns current language string.
 */
const char *get_current_language_str(void)
{
  return get_conf_parameter_text(lang_type,install_info.lang_id);
}

/**
 * Returns copy of the requested language string in lower case.
 */
const char *get_language_lwrstr(int lang_id)
{
    const char* src = get_conf_parameter_text(lang_type, lang_id);
#if (BFDEBUG_LEVEL > 0)
  if (strlen(src) != 3)
      WARNLOG("Bad text code for language index %d",(int)lang_id);
#endif
  static char lang_str[4];
  strncpy(lang_str, src, 4);
  lang_str[3] = '\0';
  strlwr(lang_str);
  return lang_str;
}

/**
 * Returns ID of given item using NamedCommands list.
 * Similar to recognize_conf_parameter(), but for use only if the buffer stores
 * one word, ended with "\0".
 * If not found, returns -1.
 */
long get_id(const struct NamedCommand *desc, const char *itmname)
{
  if ((desc == NULL) || (itmname == NULL))
    return -1;
  for (long i = 0; desc[i].name != NULL; i++)
  {
    if (strcasecmp(desc[i].name, itmname) == 0)
      return desc[i].num;
  }
  return -1;
}

/**
 * Returns ID of given item using NamedCommands list, or any item if the string is 'RANDOM'.
 * Similar to recognize_conf_parameter(), but for use only if the buffer stores
 * one word, ended with "\0".
 * If not found, returns -1.
 */
long get_rid(const struct NamedCommand *desc, const char *itmname)
{
  long i;
  if ((desc == NULL) || (itmname == NULL))
    return -1;
  for (i=0; desc[i].name != NULL; i++)
  {
    if (strcasecmp(desc[i].name, itmname) == 0)
      return desc[i].num;
  }
  if (strcasecmp("RANDOM", itmname) == 0)
  {
      i = (rand() % i);
      return desc[i].num;
  }
  return -1;
}

TbBool prepare_diskpath(char *buf,long buflen)
{
    int i = strlen(buf) - 1;
    if (i >= buflen)
        i = buflen - 1;
    if (i < 0)
        return false;
    while (i > 0)
    {
        if ((buf[i] != '\\') && (buf[i] != '/') &&
            ((unsigned char)(buf[i]) > 32))
            break;
        i--;
  }
  buf[i+1]='\0';
  return true;
}

short load_configuration(void)
{
  static const char config_textname[] = "Config";
  // Variables to use when recognizing parameters
  SYNCDBG(4,"Starting");
  // Preparing config file name and checking the file
  strcpy(install_info.inst_path,"");
  install_info.field_9A = 0;
  // Set default runtime directory and load the config file
  strcpy(keeper_runtime_directory,".");
  const char* fname = prepare_file_path(FGrp_Main, keeper_config_file);
  long len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("%s file \"%s\" doesn't exist or is too small.",config_textname,keeper_config_file);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("%s file \"%s\" is too large.",config_textname,keeper_config_file);
    return false;
  }
  char* buf = (char*)LbMemoryAlloc(len + 256);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  if (len>0)
  {
    SYNCDBG(7,"Processing %s file, %d bytes",config_textname,len);
    buf[len] = '\0';
    // Set text line number - we don't have blocks so we need to initialize it manually
    text_line_number = 1;
    long pos = 0;
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(conf_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int i = 0;
      int cmd_num = recognize_conf_command(buf, &pos, len, conf_commands);
      // Now store the config item in correct place
      int k;
      char word_buf[32];
      switch (cmd_num)
      {
      case 1: // INSTALL_PATH
          i = get_conf_parameter_whole(buf,&pos,len,install_info.inst_path,sizeof(install_info.inst_path));
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't read \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          prepare_diskpath(install_info.inst_path,sizeof(install_info.inst_path));
          break;
      case 2: // INSTALL_TYPE
          // This command is just skipped...
          break;
      case 3: // LANGUAGE
          i = recognize_conf_parameter(buf,&pos,len,lang_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          install_info.lang_id = i;
          break;
      case 4: // KEYBOARD
          // Works only in DK Premium
          break;
      case 5: // SCREENSHOT
          i = recognize_conf_parameter(buf,&pos,len,scrshot_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          screenshot_format = i;
          break;
      case 6: // FRONTEND_RES
          for (i=0; i<3; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
              k = LbRegisterVideoModeString(word_buf);
            else
              k = -1;
            if (k<=0)
            {
                CONFWRNLOG("Couldn't recognize video mode %d in \"%s\" command of %s file.",
                   i+1,COMMAND_TEXT(cmd_num),config_textname);
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
          for (i=0; i<max_game_vidmode_count(); i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = LbRegisterVideoModeString(word_buf);
              if (k > 0)
                set_game_vidmode(i,k);
              else
                  CONFWRNLOG("Couldn't recognize video mode %d in \"%s\" command of %s file.",
                    i+1,COMMAND_TEXT(cmd_num),config_textname);
            } else
            {
              if (i > 0)
                set_game_vidmode(i,Lb_SCREEN_MODE_INVALID);
              else
                  CONFWRNLOG("Video modes list empty in \"%s\" command of %s file.",
                    COMMAND_TEXT(cmd_num),config_textname);
              break;
            }
          }
          break;
      case 8: // CENSORSHIP
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_Censorship;
          else
              features_enabled &= ~Ft_Censorship;
          break;
      case 9: // POINTER_SENSITIVITY
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = atoi(word_buf);
          }
          if ((i >= 0) && (i <= 1000)) {
              base_mouse_sensitivity = i*256/100;
          } else {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case 10: // Atmospheric sound
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_Atmossounds;
          else
              features_enabled &= ~Ft_Atmossounds;
          break;
      case 11: // Atmospheric Sound Volume
          i = recognize_conf_parameter(buf,&pos,len,atmos_volume);
          if (i <= 0)
          {
            CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          else 
          {
            atmos_sound_volume = i;
            break;
          }
      case 12: // Atmospheric Sound Frequency - Chance of 1 in X
          i = recognize_conf_parameter(buf,&pos,len,atmos_freq);
          if (i <= 0)
          {
            CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          else
          {
            atmos_sound_frequency = i;
            break;
          }
      case 13: // Atmos_samples
          for (i=0; i<3; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
              k = atoi(word_buf);
            else
              k = -1;
            if (k<=0)
            {
                CONFWRNLOG("Couldn't recognize setting %d in \"%s\" command of %s file.",
                   i+1,COMMAND_TEXT(cmd_num),config_textname);
               continue;
            }
            switch (i)
            {
            case 0:
                AtmosStart = k;
                break;
            case 1:
                AtmosEnd = k;
                break;
            case 2:
                AtmosRepeat = k;
                break;
            }
          }
          break;
      case 14: // Resize Movies
          i = recognize_conf_parameter(buf,&pos,len,vidscale_type);
          if (i <= 0 || i > 256)
          {
            CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i < 256) {
            features_enabled |= Ft_Resizemovies;
            vid_scale_flags = i;
          }
          else {
            features_enabled &= ~Ft_Resizemovies;
          }
          break;
      case 15: // MUSIC_TRACKS
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = atoi(word_buf);
          }
          if ((i > 0) && (i <= 50)) {
              max_track = i;
          } else {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case 16: // WIBBLE
          i = recognize_conf_parameter(buf,&pos,len,wibble_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1) // WIBBLE ON
          {
              features_enabled |= Ft_Wibble;
              features_enabled |= Ft_LiquidWibble;
          }
          else if (i == 3) // LIQUID ONLY
          {
              features_enabled &= ~Ft_Wibble;
              features_enabled |= Ft_LiquidWibble;
          }
          else // WIBBLE OFF
          {
              features_enabled &= ~Ft_Wibble;
              features_enabled &= ~Ft_LiquidWibble;
          }
          break;
      case 17: // FREEZE_GAME_ON_FOCUS_LOST
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_FreezeOnLoseFocus;
          else
              features_enabled &= ~Ft_FreezeOnLoseFocus;
          break;
      case 18: // UNLOCK_CURSOR_WHEN_GAME_PAUSED
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_UnlockCursorOnPause;
          else
              features_enabled &= ~Ft_UnlockCursorOnPause;
          break;
      case 19: // LOCK_CURSOR_IN_POSSESSION
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_LockCursorInPossession;
          else
              features_enabled &= ~Ft_LockCursorInPossession;
          break;
      case 20: // PAUSE_MUSIC_WHEN_GAME_PAUSED
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_PauseMusicOnGamePause;
          else
              features_enabled &= ~Ft_PauseMusicOnGamePause;
          break;
      case 21: // MUTE_AUDIO_ON_FOCUS_LOST
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_MuteAudioOnLoseFocus;
          else
              features_enabled &= ~Ft_MuteAudioOnLoseFocus;
          break;
      case 0: // comment
          break;
      case -1: // end of buffer
          break;
      default:
          CONFWRNLOG("Unrecognized command in %s file.",config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
  }
  SYNCDBG(7,"Config loaded");
  // Freeing
  LbMemoryFree(buf);
  // Updating game according to loaded settings
  switch (install_info.lang_id)
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
  // Returning if the setting are valid
  return (install_info.lang_id > 0) && (install_info.inst_path[0] != '\0');
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
  case FGrp_Music:
      mdir=keeper_runtime_directory;
      sdir="music";
      break;
  case FGrp_VarLevels:
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
  case FGrp_AtlSound:
      if (campaign.speech_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=keeper_runtime_directory;
      sdir=campaign.speech_location;
      break;
  case FGrp_Main:
      mdir=keeper_runtime_directory;
      sdir=NULL;
      break;
  case FGrp_Campgn:
      mdir=keeper_runtime_directory;
      sdir="campgns";
      break;
  case FGrp_CmpgLvls:
      if (campaign.levels_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.levels_location;
      break;
  case FGrp_CmpgCrtrs:
      if (campaign.creatures_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.creatures_location;
      break;
  case FGrp_CmpgConfig:
      if (campaign.configs_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.configs_location;
      break;
  case FGrp_CmpgMedia:
      if (campaign.media_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.media_location;
      break;
  case FGrp_LandView:
      if (campaign.land_location[0] == '\0') {
          mdir=NULL; sdir=NULL;
          break;
      }
      mdir=install_info.inst_path;
      sdir=campaign.land_location;
      break;
  case FGrp_CrtrData:
      mdir=keeper_runtime_directory;
      sdir="creatrs";
      break;
  default:
      mdir="./";
      sdir=NULL;
      break;
  }
  if (mdir == NULL)
      ffullpath[0] = '\0';
  else
  if (sdir == NULL)
      sprintf(ffullpath,"%s/%s",mdir,fname);
  else
      sprintf(ffullpath,"%s/%s/%s",mdir,sdir,fname);
  return ffullpath;
}

char *prepare_file_path(short fgroup,const char *fname)
{
  static char ffullpath[2048];
  return prepare_file_path_buf(ffullpath,fgroup,fname);
}

char *prepare_file_path_va(short fgroup, const char *fmt_str, va_list arg)
{
  char fname[255];
  vsprintf(fname, fmt_str, arg);
  static char ffullpath[2048];
  return prepare_file_path_buf(ffullpath, fgroup, fname);
}

char *prepare_file_fmtpath(short fgroup, const char *fmt_str, ...)
{
  va_list val;
  va_start(val, fmt_str);
  char* result = prepare_file_path_va(fgroup, fmt_str, val);
  va_end(val);
  return result;
}

short file_group_needs_cd(short fgroup)
{
  switch (fgroup)
  {
  case FGrp_LoData:
  case FGrp_VarLevels:
      return true;
  default:
      return false;
  }
}

/**
 * Returns the folder specified by LEVELS_LOCATION
 */
short get_level_fgroup(LevelNumber lvnum)
{
    return FGrp_CmpgLvls;
}

/**
 * Loads data file into allocated buffer.
 * @return Returns NULL if the file doesn't exist or is smaller than ldsize;
 * on success, returns a buffer which should be freed after use,
 * and sets ldsize into its size.
 */
unsigned char *load_data_file_to_buffer(long *ldsize, short fgroup, const char *fmt_str, ...)
{
  // Prepare file name
  va_list arg;
  va_start(arg, fmt_str);
  char fname[255];
  vsprintf(fname, fmt_str, arg);
  char ffullpath[2048];
  prepare_file_path_buf(ffullpath, fgroup, fname);
  va_end(arg);
  // Load the file
  if (file_group_needs_cd(fgroup))
  {
    if (!wait_for_cd_to_be_available())
      return NULL;
   }
   long fsize = LbFileLengthRnc(ffullpath);
   if (fsize < *ldsize)
   {
       WARNMSG("File \"%s\" doesn't exist or is too small.", fname);
       return NULL;
  }
  unsigned char* buf = LbMemoryAlloc(fsize + 16);
  if (buf == NULL)
  {
    WARNMSG("Can't allocate %ld bytes to load \"%s\".",fsize,fname);
    return NULL;
  }
  fsize = LbFileLoadAt(ffullpath,buf);
  if (fsize < *ldsize)
  {
    WARNMSG("Reading file \"%s\" failed.",fname);
    LbMemoryFree(buf);
    return NULL;
  }
  LbMemorySet(buf+fsize, '\0', 15);
  *ldsize = fsize;
  return buf;
}

short calculate_moon_phase(short do_calculate,short add_to_log)
{
  //Moon phase calculation
  if (do_calculate)
  {
    phase_of_moon = (float)LbMoonPhase();
  }
  if ((phase_of_moon > -0.05) && (phase_of_moon < 0.05))
  {
    if (add_to_log)
      SYNCMSG("Full moon %.4f", phase_of_moon);
    is_full_moon = 1;
    is_near_full_moon = 0;
    is_new_moon = 0;
    is_near_new_moon = 0;
  } else
  if ((phase_of_moon > -0.1) && (phase_of_moon < 0.1))
  {
    if (add_to_log)
      SYNCMSG("Near Full moon %.4f", phase_of_moon);
    is_full_moon = 0;
    is_near_full_moon = 1;
    is_new_moon = 0;
    is_near_new_moon = 0;
  } else
  if ((phase_of_moon < -0.95) || (phase_of_moon > 0.95))
  {
    if (add_to_log)
      SYNCMSG("New moon %.4f", phase_of_moon);
    is_full_moon = 0;
    is_near_full_moon = 0;
    is_new_moon = 1;
    is_near_new_moon = 0;
  } else
  if ((phase_of_moon < -0.9) || (phase_of_moon > 0.9))
  {
    if (add_to_log)
      SYNCMSG("Near new moon %.4f", phase_of_moon);
    is_full_moon = 0;
    is_near_full_moon = 0;
    is_new_moon = 0;
    is_near_new_moon = 1;
  } else
  {
    if (add_to_log)
      SYNCMSG("Moon phase %.4f", phase_of_moon);
    is_full_moon = 0;
    is_near_full_moon = 0;
    is_new_moon = 0;
    is_near_new_moon = 0;
  }
//!CHEAT! always show extra levels
//  is_full_moon = 1; is_new_moon = 1;
  return is_full_moon;
}

void load_or_create_high_score_table(void)
{
  if (!load_high_score_table())
  {
     SYNCMSG("High scores table bad; creating new one.");
     create_empty_high_score_table();
     save_high_score_table();
  }
}

TbBool load_high_score_table(void)
{
    char* fname = prepare_file_path(FGrp_Save, campaign.hiscore_fname);
    long arr_size = campaign.hiscore_count * sizeof(struct HighScore);
    if (arr_size <= 0)
    {
        LbMemoryFree(campaign.hiscore_table);
        campaign.hiscore_table = NULL;
        return true;
    }
    if (campaign.hiscore_table == NULL)
        campaign.hiscore_table = (struct HighScore *)LbMemoryAlloc(arr_size);
    if (LbFileLengthRnc(fname) != arr_size)
        return false;
    if (campaign.hiscore_table == NULL)
        return false;
    if (LbFileLoadAt(fname, campaign.hiscore_table) == arr_size)
        return true;
    return false;
}

TbBool save_high_score_table(void)
{
    char* fname = prepare_file_path(FGrp_Save, campaign.hiscore_fname);
    long fsize = campaign.hiscore_count * sizeof(struct HighScore);
    if (fsize <= 0)
        return true;
    if (campaign.hiscore_table == NULL)
        return false;
    // Save the file
    if (LbFileSaveAt(fname, campaign.hiscore_table, fsize) == fsize)
        return true;
    return false;
}

/**
 * Generates new high score table if previous can't be loaded.
 */
TbBool create_empty_high_score_table(void)
{
  int i;
  int npoints = 100 * VISIBLE_HIGH_SCORES_COUNT;
  int nlevel = 1 * VISIBLE_HIGH_SCORES_COUNT;
  long arr_size = campaign.hiscore_count * sizeof(struct HighScore);
  if (campaign.hiscore_table == NULL)
    campaign.hiscore_table = (struct HighScore *)LbMemoryAlloc(arr_size);
  if (campaign.hiscore_table == NULL)
    return false;
  for (i=0; i < VISIBLE_HIGH_SCORES_COUNT; i++)
  {
    if (i >= campaign.hiscore_count) break;
    sprintf(campaign.hiscore_table[i].name, "Bullfrog");
    campaign.hiscore_table[i].score = npoints;
    campaign.hiscore_table[i].lvnum = nlevel;
    npoints -= 100;
    nlevel -= 1;
  }
  while (i < campaign.hiscore_count)
  {
    campaign.hiscore_table[i].name[0] = '\0';
    campaign.hiscore_table[i].score = 0;
    campaign.hiscore_table[i].lvnum = 0;
    i++;
  }
  return true;
}

/**
 * Adds new entry to high score table. Returns its index.
 */
int add_high_score_entry(unsigned long score, LevelNumber lvnum, const char *name)
{
    int dest_idx;
    // If the table is not initiated - return
    if (campaign.hiscore_table == NULL)
    {
        WARNMSG("Can't add entry to uninitiated high score table");
        return false;
    }
    // Determining position of the new entry to keep table sorted with decreasing scores
    for (dest_idx=0; dest_idx < campaign.hiscore_count; dest_idx++)
    {
        if (campaign.hiscore_table[dest_idx].score < score)
            break;
        if (campaign.hiscore_table[dest_idx].lvnum <= 0)
            break;
    }
    // Find different entry which has duplicates with higher score - the one we can overwrite with no consequence
    // Don't allow replacing first 10 scores - they are visible to the player, and shouldn't be touched
    int overwrite_idx;
    for (overwrite_idx = campaign.hiscore_count-1; overwrite_idx >= 10; overwrite_idx--)
    {
        LevelNumber last_lvnum = campaign.hiscore_table[overwrite_idx].lvnum;
        if (last_lvnum <= 0) {
            // Found unused slot
            break;
        }
        int k;
        for (k=overwrite_idx-1; k >= 0; k--)
        {
            if (campaign.hiscore_table[k].lvnum == last_lvnum) {
                // Found a duplicate entry for that level with higher score
                break;
            }
        }
        if (k >= 0)
            break;
    }
    SYNCDBG(4,"New high score entry index %d, overwrite index %d",(int)dest_idx,(int)overwrite_idx);
    // In case nothing was found to overwrite
    if (overwrite_idx < 10) {
        overwrite_idx = campaign.hiscore_count;
    }
    // If index from sorting is outside array, make it equal to index to overwrite
    if (dest_idx >= campaign.hiscore_count)
        dest_idx = overwrite_idx;
    // And overwrite index - if not found, make it point to last entry in array
    if (overwrite_idx >= campaign.hiscore_count)
        overwrite_idx = campaign.hiscore_count-1;
    // If the new score is too poor, and there's not enough space for it, and we couldn't find slot to overwrite, return
    if (dest_idx >= campaign.hiscore_count) {
        WARNMSG("Can't add entry to high score table - it's full and has no duplicating levels");
        return -1;
    }
    // Now, we need to shift entries between destination position and position to overwrite
    if (overwrite_idx > dest_idx)
    {
        // Moving entries down
        for (int k = overwrite_idx - 1; k >= dest_idx; k--)
        {
            memcpy(&campaign.hiscore_table[k+1],&campaign.hiscore_table[k],sizeof(struct HighScore));
        }
    } else
    {
        // Moving entries up
        for (int k = overwrite_idx; k < dest_idx; k++)
        {
            memcpy(&campaign.hiscore_table[k],&campaign.hiscore_table[k+1],sizeof(struct HighScore));
        }
    }
    // Preparing the new entry
    strncpy(campaign.hiscore_table[dest_idx].name, name, HISCORE_NAME_LENGTH);
    campaign.hiscore_table[dest_idx].score = score;
    campaign.hiscore_table[dest_idx].lvnum = lvnum;
    return dest_idx;
}

/**
 * Returns highest score value for given level.
 */
unsigned long get_level_highest_score(LevelNumber lvnum)
{
    for (int idx = 0; idx < campaign.hiscore_count; idx++)
    {
        if (campaign.hiscore_table[idx].lvnum == lvnum)
            return campaign.hiscore_table[idx].score;
  }
  return 0;
}

struct LevelInformation *get_level_info(LevelNumber lvnum)
{
  return get_campaign_level_info(&campaign, lvnum);
}

struct LevelInformation *get_or_create_level_info(LevelNumber lvnum, unsigned long lvoptions)
{
    struct LevelInformation* lvinfo = get_campaign_level_info(&campaign, lvnum);
    if (lvinfo != NULL)
    {
        lvinfo->options |= lvoptions;
        return lvinfo;
  }
  lvinfo = new_level_info_entry(&campaign, lvnum);
  if (lvinfo != NULL)
  {
    lvinfo->options |= lvoptions;
    return lvinfo;
  }
  return NULL;
}

/**
 * Returns first level info structure in the array.
 */
struct LevelInformation *get_first_level_info(void)
{
  if (campaign.lvinfos == NULL)
    return NULL;
  return &campaign.lvinfos[0];
}

/**
 * Returns last level info structure in the array.
 */
struct LevelInformation *get_last_level_info(void)
{
  if ((campaign.lvinfos == NULL) || (campaign.lvinfos_count < 1))
    return NULL;
  return &campaign.lvinfos[campaign.lvinfos_count-1];
}

/**
 * Returns next level info structure in the array.
 * Note that it's not always corresponding to next campaign level; use
 * get_level_info() to get information for specific level. This function
 * is used for sweeping through all level info entries.
 */
struct LevelInformation *get_next_level_info(struct LevelInformation *previnfo)
{
  if (campaign.lvinfos == NULL)
    return NULL;
  if (previnfo == NULL)
    return NULL;
  int i = previnfo - &campaign.lvinfos[0];
  i++;
  if (i >= campaign.lvinfos_count)
    return NULL;
  return &campaign.lvinfos[i];
}

/**
 * Returns previous level info structure in the array.
 * Note that it's not always corresponding to previous campaign level; use
 * get_level_info() to get information for specific level. This function
 * is used for reverse sweeping through all level info entries.
 */
struct LevelInformation *get_prev_level_info(struct LevelInformation *nextinfo)
{
  if (campaign.lvinfos == NULL)
    return NULL;
  if (nextinfo == NULL)
    return NULL;
  int i = nextinfo - &campaign.lvinfos[0];
  i--;
  if (i < 0)
    return NULL;
  return &campaign.lvinfos[i];
}

short set_level_info_string_index(LevelNumber lvnum, char *stridx, unsigned long lvoptions)
{
    if (campaign.lvinfos == NULL)
        init_level_info_entries(&campaign, 0);
    struct LevelInformation* lvinfo = get_or_create_level_info(lvnum, lvoptions);
    if (lvinfo == NULL)
        return false;
    int k = atoi(stridx);
    if (k > 0)
    {
        lvinfo->name_stridx = k;
        return true;
    }
  return false;
}

short set_level_info_text_name(LevelNumber lvnum, char *name, unsigned long lvoptions)
{
    if (campaign.lvinfos == NULL)
        init_level_info_entries(&campaign, 0);
    struct LevelInformation* lvinfo = get_or_create_level_info(lvnum, lvoptions);
    if (lvinfo == NULL)
        return false;
    strncpy(lvinfo->name, name, LINEMSG_SIZE - 1);
    lvinfo->name[LINEMSG_SIZE - 1] = '\0';
    if ((lvoptions & LvOp_IsFree) != 0)
    {
        lvinfo->ensign_x += ((LANDVIEW_MAP_WIDTH >> 4) * (LbSinL(lvnum * LbFPMath_PI / 16) >> 6)) >> 10;
        lvinfo->ensign_y -= ((LANDVIEW_MAP_HEIGHT >> 4) * (LbCosL(lvnum * LbFPMath_PI / 16) >> 6)) >> 10;
  }
  return true;
}

TbBool reset_credits(struct CreditsItem *credits)
{
    for (long i = 0; i < CAMPAIGN_CREDITS_COUNT; i++)
    {
        LbMemorySet(&credits[i], 0, sizeof(struct CreditsItem));
        credits[i].kind = CIK_None;
  }
  return true;
}

TbBool parse_credits_block(struct CreditsItem *credits,char *buf,char *buf_end)
{
  // Block name and parameter word store variables
  char block_buf[32];
  // Find the block
  sprintf(block_buf,"credits");
  long len = buf_end - buf;
  long pos = 0;
  int k = find_conf_block(buf, &pos, len, block_buf);
  if (k < 0)
  {
    WARNMSG("Block [%s] not found in Credits file.",block_buf);
    return 0;
  }
  int n = 0;
  while (pos<len)
  {
    if ((buf[pos] != 0) && (buf[pos] != '[') && (buf[pos] != ';'))
    {
      credits[n].kind = CIK_EmptyLine;
      credits[n].font = 2;
      char word_buf[32];
      switch (buf[pos])
      {
      case '*':
        pos++;
        if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          k = atol(word_buf);
        else
          k = 0;
        if (k > 0)
        {
          credits[n].kind = CIK_StringId;
          credits[n].font = 1;
          credits[n].num = k;
        }
        break;
      case '&':
        pos++;
        if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          k = atoi(word_buf);
        else
          k = 0;
        if (k > 0)
        {
          credits[n].kind = CIK_StringId;
          credits[n].font = 2;
          credits[n].num = k;
        }
        break;
      case '%':
        pos++;
        credits[n].kind = CIK_DirectText;
        credits[n].font = 0;
        credits[n].str = &buf[pos];
        break;
      case '#':
        pos++;
        credits[n].kind = CIK_DirectText;
        credits[n].font = 1;
        credits[n].str = &buf[pos];
        break;
      default:
        credits[n].kind = CIK_DirectText;
        credits[n].font = 2;
        credits[n].str = &buf[pos];
        break;
      }
      n++;
    }
    // Finishing the line
    while (pos < len)
    {
      if (buf[pos] < 32) break;
      pos++;
    }
    if (buf[pos] == '\r')
    {
      buf[pos] = '\0';
      pos+=2;
    } else
    {
      buf[pos] = '\0';
      pos++;
    }
  }
  if (credits[0].kind == CIK_None)
    WARNMSG("Credits list empty after parsing [%s] block of Credits file.", block_buf);
  return true;
}

/**
 * Loads the credits data for the current campaign.
 */
TbBool setup_campaign_credits_data(struct GameCampaign *campgn)
{
  SYNCDBG(18,"Starting");
  char* fname = prepare_file_path(FGrp_LandView, campgn->credits_fname);
  long filelen = LbFileLengthRnc(fname);
  if (filelen <= 0)
  {
    ERRORLOG("Campaign Credits file \"%s\" does not exist or can't be opened",campgn->credits_fname);
    return false;
  }
  campgn->credits_data = (char *)LbMemoryAlloc(filelen + 256);
  if (campgn->credits_data == NULL)
  {
    ERRORLOG("Can't allocate memory for Campaign Credits file \"%s\"",campgn->credits_fname);
    return false;
  }
  char* credits_data_end = campgn->credits_data + filelen + 255;
  short result = true;
  long loaded_size = LbFileLoadAt(fname, campgn->credits_data);
  if (loaded_size < 4)
  {
    ERRORLOG("Campaign Credits file \"%s\" couldn't be loaded or is too small",campgn->credits_fname);
    result = false;
  }
  // Resetting all values to unused
  reset_credits(campgn->credits);
  // Analyzing credits data and filling correct values
  if (result)
  {
    result = parse_credits_block(campgn->credits, campgn->credits_data, credits_data_end);
    if (!result)
      WARNMSG("Parsing credits file \"%s\" credits block failed",campgn->credits_fname);
  }
  SYNCDBG(19,"Finished");
  return result;
}

short is_bonus_level(LevelNumber lvnum)
{
  if (lvnum < 1) return false;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.bonus_levels[i] == lvnum)
    {
        SYNCDBG(7,"Level %ld identified as bonus",lvnum);
        return true;
    }
  }
  SYNCDBG(7,"Level %ld not recognized as bonus",lvnum);
  return false;
}

short is_extra_level(LevelNumber lvnum)
{
  if (lvnum < 1) return false;
  for (int i = 0; i < EXTRA_LEVELS_COUNT; i++)
  {
      if (campaign.extra_levels[i] == lvnum)
      {
          SYNCDBG(7,"Level %ld identified as extra",lvnum);
          return true;
      }
  }
  SYNCDBG(7,"Level %ld not recognized as extra",lvnum);
  return false;
}

/**
 * Returns index for Game->bonus_levels associated with given single player level.
 * Gives -1 if there's no store place for the level.
 */
int storage_index_for_bonus_level(LevelNumber bn_lvnum)
{
    if (bn_lvnum < 1)
        return -1;
    int k = 0;
    for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
    {
        if (campaign.bonus_levels[i] == bn_lvnum)
            return k;
        if (campaign.bonus_levels[i] != 0)
            k++;
  }
  return -1;
}

/**
 * Returns index for Campaign->bonus_levels associated with given bonus level.
 * If the level is not found, returns -1.
 */
int array_index_for_bonus_level(LevelNumber bn_lvnum)
{
  if (bn_lvnum < 1) return -1;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.bonus_levels[i] == bn_lvnum)
        return i;
  }
  return -1;
}

/**
 * Returns index for Campaign->extra_levels associated with given extra level.
 * If the level is not found, returns -1.
 */
int array_index_for_extra_level(LevelNumber ex_lvnum)
{
  if (ex_lvnum < 1) return -1;
  for (int i = 0; i < EXTRA_LEVELS_COUNT; i++)
  {
    if (campaign.extra_levels[i] == ex_lvnum)
        return i;
  }
  return -1;
}

/**
 * Returns index for Campaign->single_levels associated with given singleplayer level.
 * If the level is not found, returns -1.
 */
int array_index_for_singleplayer_level(LevelNumber sp_lvnum)
{
  if (sp_lvnum < 1) return -1;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == sp_lvnum)
        return i;
  }
  return -1;
}

/**
 * Returns index for Campaign->multi_levels associated with given multiplayer level.
 * If the level is not found, returns -1.
 */
int array_index_for_multiplayer_level(LevelNumber mp_lvnum)
{
  if (mp_lvnum < 1) return -1;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.multi_levels[i] == mp_lvnum)
        return i;
  }
  return -1;
}

/**
 * Returns index for Campaign->freeplay_levels associated with given freeplay level.
 * If the level is not found, returns -1.
 */
int array_index_for_freeplay_level(LevelNumber fp_lvnum)
{
  if (fp_lvnum < 1) return -1;
  for (int i = 0; i < FREE_LEVELS_COUNT; i++)
  {
    if (campaign.freeplay_levels[i] == fp_lvnum)
        return i;
  }
  return -1;
}

/**
 * Returns bonus level number for given singleplayer level number.
 * If no bonus found, returns 0.
 */
LevelNumber bonus_level_for_singleplayer_level(LevelNumber sp_lvnum)
{
    int i = array_index_for_singleplayer_level(sp_lvnum);
    if (i >= 0)
        return campaign.bonus_levels[i];
    return 0;
}

/**
 * Returns first single player level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber first_singleplayer_level(void)
{
    long lvnum = campaign.single_levels[0];
    if (lvnum > 0)
        return lvnum;
    return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns last single player level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber last_singleplayer_level(void)
{
    int i = campaign.single_levels_count;
    if ((i > 0) && (i <= CAMPAIGN_LEVELS_COUNT))
        return campaign.single_levels[i - 1];
    return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns first multi player level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber first_multiplayer_level(void)
{
  long lvnum = campaign.multi_levels[0];
  if (lvnum > 0)
    return lvnum;
  return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns last multi player level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber last_multiplayer_level(void)
{
    int i = campaign.multi_levels_count;
    if ((i > 0) && (i <= CAMPAIGN_LEVELS_COUNT))
        return campaign.multi_levels[i - 1];
    return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns first free play level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber first_freeplay_level(void)
{
  long lvnum = campaign.freeplay_levels[0];
  if (lvnum > 0)
    return lvnum;
  return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns last free play level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber last_freeplay_level(void)
{
    int i = campaign.freeplay_levels_count;
    if ((i > 0) && (i <= FREE_LEVELS_COUNT))
        return campaign.freeplay_levels[i - 1];
    return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns first extra level number.
 * On error, returns SINGLEPLAYER_NOTSTARTED.
 */
LevelNumber first_extra_level(void)
{
    for (long lvidx = 0; lvidx < campaign.extra_levels_index; lvidx++)
    {
        long lvnum = campaign.extra_levels[lvidx];
        if (lvnum > 0)
            return lvnum;
  }
  return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns the extra level number. Gives SINGLEPLAYER_NOTSTARTED if no such level,
 * LEVELNUMBER_ERROR on error.
 */
LevelNumber get_extra_level(unsigned short elv_kind)
{
    int i = elv_kind;
    i--;
    if ((i < 0) || (i >= EXTRA_LEVELS_COUNT))
        return LEVELNUMBER_ERROR;
    LevelNumber lvnum = campaign.extra_levels[i];
    SYNCDBG(5, "Extra level kind %d has number %ld", (int)elv_kind, lvnum);
    if (lvnum > 0)
    {
        return lvnum;
  }
  return SINGLEPLAYER_NOTSTARTED;
}

/**
 * Returns the next single player level. Gives SINGLEPLAYER_FINISHED if
 * last level was won, LEVELNUMBER_ERROR on error.
 */
LevelNumber next_singleplayer_level(LevelNumber sp_lvnum)
{
  if (sp_lvnum == SINGLEPLAYER_FINISHED) return SINGLEPLAYER_FINISHED;
  if (sp_lvnum == SINGLEPLAYER_NOTSTARTED) return first_singleplayer_level();
  if (sp_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == sp_lvnum)
    {
      if (i+1 >= CAMPAIGN_LEVELS_COUNT)
        return SINGLEPLAYER_FINISHED;
      if (campaign.single_levels[i+1] <= 0)
        return SINGLEPLAYER_FINISHED;
      return campaign.single_levels[i+1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the previous single player level. Gives SINGLEPLAYER_NOTSTARTED if
 * first level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber prev_singleplayer_level(LevelNumber sp_lvnum)
{
  if (sp_lvnum == SINGLEPLAYER_NOTSTARTED) return SINGLEPLAYER_NOTSTARTED;
  if (sp_lvnum == SINGLEPLAYER_FINISHED) return last_singleplayer_level();
  if (sp_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == sp_lvnum)
    {
      if (i < 1)
        return SINGLEPLAYER_NOTSTARTED;
      if (campaign.single_levels[i-1] <= 0)
        return SINGLEPLAYER_NOTSTARTED;
      return campaign.single_levels[i-1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the next multi player level. Gives SINGLEPLAYER_FINISHED if
 * last level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber next_multiplayer_level(LevelNumber mp_lvnum)
{
  if (mp_lvnum == SINGLEPLAYER_FINISHED) return SINGLEPLAYER_FINISHED;
  if (mp_lvnum == SINGLEPLAYER_NOTSTARTED) return first_multiplayer_level();
  if (mp_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.multi_levels[i] == mp_lvnum)
    {
      if (i+1 >= CAMPAIGN_LEVELS_COUNT)
        return SINGLEPLAYER_FINISHED;
      if (campaign.multi_levels[i+1] <= 0)
        return SINGLEPLAYER_FINISHED;
      return campaign.multi_levels[i+1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the previous multi player level. Gives SINGLEPLAYER_NOTSTARTED if
 * first level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber prev_multiplayer_level(LevelNumber mp_lvnum)
{
  if (mp_lvnum == SINGLEPLAYER_NOTSTARTED) return SINGLEPLAYER_NOTSTARTED;
  if (mp_lvnum == SINGLEPLAYER_FINISHED) return last_multiplayer_level();
  if (mp_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.multi_levels[i] == mp_lvnum)
    {
      if (i < 1)
        return SINGLEPLAYER_NOTSTARTED;
      if (campaign.multi_levels[i-1] <= 0)
        return SINGLEPLAYER_NOTSTARTED;
      return campaign.multi_levels[i-1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the next extra level. Gives SINGLEPLAYER_FINISHED if
 * last level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber next_extra_level(LevelNumber ex_lvnum)
{
  if (ex_lvnum == SINGLEPLAYER_FINISHED) return SINGLEPLAYER_FINISHED;
  if (ex_lvnum == SINGLEPLAYER_NOTSTARTED) return first_extra_level();
  if (ex_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < EXTRA_LEVELS_COUNT; i++)
  {
    if (campaign.extra_levels[i] == ex_lvnum)
    {
      i++;
      while (i < EXTRA_LEVELS_COUNT)
      {
        if (campaign.extra_levels[i] > 0)
          return campaign.extra_levels[i];
        i++;
      }
      return SINGLEPLAYER_FINISHED;
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the next freeplay level. Gives SINGLEPLAYER_FINISHED if
 * last level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber next_freeplay_level(LevelNumber fp_lvnum)
{
  if (fp_lvnum == SINGLEPLAYER_FINISHED) return SINGLEPLAYER_FINISHED;
  if (fp_lvnum == SINGLEPLAYER_NOTSTARTED) return first_freeplay_level();
  if (fp_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < FREE_LEVELS_COUNT; i++)
  {
    if (campaign.freeplay_levels[i] == fp_lvnum)
    {
      if (i+1 >= FREE_LEVELS_COUNT)
        return SINGLEPLAYER_FINISHED;
      if (campaign.freeplay_levels[i+1] <= 0)
        return SINGLEPLAYER_FINISHED;
      return campaign.freeplay_levels[i+1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns the previous freeplay level. Gives SINGLEPLAYER_NOTSTARTED if
 * first level was given, LEVELNUMBER_ERROR on error.
 */
LevelNumber prev_freeplay_level(LevelNumber fp_lvnum)
{
  if (fp_lvnum == SINGLEPLAYER_NOTSTARTED) return SINGLEPLAYER_NOTSTARTED;
  if (fp_lvnum == SINGLEPLAYER_FINISHED) return last_freeplay_level();
  if (fp_lvnum < 1) return LEVELNUMBER_ERROR;
  for (int i = 0; i < FREE_LEVELS_COUNT; i++)
  {
    if (campaign.freeplay_levels[i] == fp_lvnum)
    {
      if (i < 1)
        return SINGLEPLAYER_NOTSTARTED;
      if (campaign.freeplay_levels[i-1] <= 0)
        return SINGLEPLAYER_NOTSTARTED;
      return campaign.freeplay_levels[i-1];
    }
  }
  return LEVELNUMBER_ERROR;
}

/**
 * Returns if the level is a single player campaign level,
 * or special non-existing level at start/end of campaign.
 */
short is_singleplayer_like_level(LevelNumber lvnum)
{
  if ((lvnum == SINGLEPLAYER_FINISHED) || (lvnum == SINGLEPLAYER_NOTSTARTED))
    return true;
  return is_singleplayer_level(lvnum);
}

/**
 * Returns if the level is a single player campaign level.
 */
short is_singleplayer_level(LevelNumber lvnum)
{
  if (lvnum < 1)
  {
    SYNCDBG(17,"Level index %ld is not correct",lvnum);
    return false;
  }
  for (int i = 0; i < CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.single_levels[i] == lvnum)
    {
      SYNCDBG(17,"Level %ld identified as SP",lvnum);
      return true;
    }
  }
  SYNCDBG(17,"Level %ld not recognized as SP",lvnum);
  return false;
}

short is_multiplayer_level(LevelNumber lvnum)
{
  int i;
  if (lvnum < 1) return false;
  for (i=0; i<CAMPAIGN_LEVELS_COUNT; i++)
  {
    if (campaign.multi_levels[i] == lvnum)
    {
        SYNCDBG(17,"Level %ld identified as MP",lvnum);
        return true;
    }
  }
  // Original MP checking - remove when it's not needed anymore
  if (net_number_of_levels <= 0)
    return false;
  for (i=0; i < net_number_of_levels; i++)
  {
      struct NetLevelDesc* lvdesc = &net_level_desc[i];
      if (lvdesc->lvnum == lvnum)
      {
          SYNCDBG(17, "Level %ld identified as MP with old description", lvnum);
          return true;
    }
  }
  SYNCDBG(17,"Level %ld not recognized as MP",lvnum);
  return false;
}

/**
 * Returns if the level is 'campaign' level.
 * All levels mentioned in campaign file are campaign levels. Campaign and
 * freeplay levels are exclusive.
 */
short is_campaign_level(LevelNumber lvnum)
{
  if (is_singleplayer_level(lvnum) || is_bonus_level(lvnum)
   || is_extra_level(lvnum) || is_multiplayer_level(lvnum))
    return true;
  return false;
}

/**
 * Returns if the level is 'free play' level, which should be visible
 * in list of levels.
 */
short is_freeplay_level(LevelNumber lvnum)
{
  if (lvnum < 1) return false;
  for (int i = 0; i < FREE_LEVELS_COUNT; i++)
  {
    if (campaign.freeplay_levels[i] == lvnum)
    {
        SYNCDBG(18,"%d is freeplay",lvnum);
        return true;
    }
  }
  SYNCDBG(18,"%d is NOT freeplay",lvnum);
  return false;
}
/******************************************************************************/
