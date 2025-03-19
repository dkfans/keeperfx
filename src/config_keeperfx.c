/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_keeperfx.c
 * @par Purpose:
 *     load the main keeperfx.cfg config file.
 * @par Comment:
 *     None.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_keeperfx.h"

#include <stdarg.h>
#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_mouse.h"
#include "bflib_sound.h"
#include "sounds.h"
#include "engine_render.h"
#include "bflib_fmvids.h"

#include "config_campaigns.h"
#include "front_simple.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "moonphase.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/


static const char keeper_config_file[]="keeperfx.cfg";

char cmd_char = '!';
unsigned short AtmosRepeat = 1013;
unsigned short AtmosStart = 1014;
unsigned short AtmosEnd = 1034;
TbBool AssignCpuKeepers = 0;
struct InstallInfo install_info;
char keeper_runtime_directory[152];
short api_enabled = false;
uint16_t api_port = 5599;
unsigned long features_enabled = 0;

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
  {"PNG", 1},
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
  {"FREEZE_GAME_ON_FOCUS_LOST"     , 17},
  {"UNLOCK_CURSOR_WHEN_GAME_PAUSED", 18},
  {"LOCK_CURSOR_IN_POSSESSION"     , 19},
  {"PAUSE_MUSIC_WHEN_GAME_PAUSED"  , 20},
  {"MUTE_AUDIO_ON_FOCUS_LOST"      , 21},
  {"DISABLE_SPLASH_SCREENS"        , 22},
  {"SKIP_HEART_ZOOM"               , 23},
  {"CURSOR_EDGE_CAMERA_PANNING"    , 24},
  {"DELTA_TIME"                    , 25},
  {"CREATURE_STATUS_SIZE"          , 26},
  {"MAX_ZOOM_DISTANCE"             , 27},
  {"DISPLAY_NUMBER"                , 28},
  {"MUSIC_FROM_DISK"               , 29},
  {"HAND_SIZE"                     , 30},
  {"LINE_BOX_SIZE"                 , 31},
  {"COMMAND_CHAR"                  , 32},
  {"API_ENABLED"                   , 33},
  {"API_PORT"                      , 34},
  {NULL,                   0},
  };

  const struct NamedCommand vidscale_type[] = {
  {"OFF",          0}, // No scaling of Smacker Video
  {"DISABLED",     0},
  {"FALSE",        0},
  {"NO",           0},
  {"0",            0},
  {"FIT",          SMK_FullscreenFit}, // Fit to fullscreen, using letterbox and pillarbox as necessary
  {"ON",           SMK_FullscreenFit}, // Duplicate of FIT, for legacy reasons
  {"ENABLED",      SMK_FullscreenFit},
  {"TRUE",         SMK_FullscreenFit},
  {"YES",          SMK_FullscreenFit},
  {"1",            SMK_FullscreenFit},
  {"STRETCH",      SMK_FullscreenStretch}, // Stretch to fullscreen - ignores aspect ratio difference between source and destination
  {"CROP",         SMK_FullscreenCrop}, // Fill fullscreen and crop - no letterbox or pillarbox
  {"4BY3",         SMK_FullscreenFit | SMK_FullscreenStretch}, // [Aspect Ratio correction mode] - stretch 320x200 to 4:3 (i.e. increase height by 1.2)
  {"PIXELPERFECT", SMK_FullscreenFit | SMK_FullscreenCrop}, // integer multiple scale only (FIT)
  {"4BY3PP",       SMK_FullscreenFit | SMK_FullscreenStretch | SMK_FullscreenCrop}, // integer multiple scale only (4BY3)
  {NULL,           0},
  };

unsigned int vid_scale_flags = SMK_FullscreenFit;


/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

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

TbBool is_feature_on(unsigned long feature)
{
  return ((features_enabled & feature) != 0);
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
  snprintf(lang_str, 4, "%s", src);
  make_lowercase(lang_str);
  return lang_str;
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
  // Set default runtime directory and load the config file
  strcpy(keeper_runtime_directory,".");
  // Config file variables
  const char* sname; // Filename
  const char* fname; // Filepath
  // Check if custom config file is set '-config <file>'
  if (start_params.overrides[Clo_ConfigFile])
  {
    // Check if config override contains either '\\' or '/'
    // This means we'll use the absolute path to the config file
    if (strchr(start_params.config_file, '\\') != NULL || strchr(start_params.config_file, '/') != NULL) {
        // Get filename
        const char *backslash = strrchr(start_params.config_file, '\\');
        const char *slash = strrchr(start_params.config_file, '/');
        const char *last_separator = backslash > slash ? backslash : slash;
        sname = last_separator ? last_separator + 1 : start_params.config_file;
        // Get filepath
        fname = start_params.config_file; // Absolute path
    } else {
        sname = start_params.config_file;
        fname = prepare_file_path(FGrp_Main, sname);
    }
  }
  else
  {
    sname = keeper_config_file;
    fname = prepare_file_path(FGrp_Main, sname);
  }

  long len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    WARNMSG("%s file \"%s\" doesn't exist or is too small.",config_textname,sname);
    return false;
  }
  if (len > 65536)
  {
    WARNMSG("%s file \"%s\" is too large.",config_textname,sname);
    return false;
  }
  char* buf = (char*)calloc(len + 256, 1);
  if (buf == NULL)
    return false;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  if (len>0)
  {
    SYNCDBG(7,"Processing %s file, %ld bytes",config_textname,len);
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
                set_failsafe_vidmode((TbScreenMode)k);
                break;
            case 1:
                set_movies_vidmode((TbScreenMode)k);
                break;
            case 2:
                set_frontend_vidmode((TbScreenMode)k);
                break;
            }
          }
          break;
      case 7: // INGAME_RES
          for (i=0; i<MAX_GAME_VIDMODE_COUNT; i++)
          {
            if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
            {
              k = LbRegisterVideoModeString(word_buf);
              if (k > 0)
                set_game_vidmode((uint)i,(TbScreenMode)k);
              else
                  CONFWRNLOG("Couldn't recognize video mode %d in \"%s\" command of %s file.",
                    i+1,COMMAND_TEXT(cmd_num),config_textname);
            } else
            {
              if (i > 0)
                set_game_vidmode((uint)i,Lb_SCREEN_MODE_INVALID);
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
          if (i < 0)
          {
            CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i > 0) {
            features_enabled |= Ft_Resizemovies;
            vid_scale_flags = i;
          }
          else {
            features_enabled &= ~Ft_Resizemovies;
          }
          break;
      case 15: // MUSIC_TRACKS
          // obsolete, no longer needed
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
        case 22: //DISABLE_SPLASH_SCREENS
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_SkipSplashScreens;
          else
              features_enabled &= ~Ft_SkipSplashScreens;
          break;
        case 23: //SKIP_HEART_ZOOM
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_SkipHeartZoom;
          else
              features_enabled &= ~Ft_SkipHeartZoom;
          break;
        case 24: //CURSOR_EDGE_CAMERA_PANNING
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled &= ~Ft_DisableCursorCameraPanning;
          else
              features_enabled |= Ft_DisableCursorCameraPanning;
          break;
        case 25: //DELTA_TIME
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_DeltaTime;
          else
              features_enabled &= ~Ft_DeltaTime;
          break;
      case 26: // CREATURE_STATUS_SIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = atoi(word_buf);
          }
          if ((i >= 0) && (i <= 32768)) {
              creature_status_size = i;
          } else {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case 27: // MAX_ZOOM_DISTANCE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = atoi(word_buf);
          }
          if ((i >= 0) && (i <= 32768)) {
              if (i > 100) {i = 100;}
              zoom_distance_setting = LbLerp(4100, CAMERA_ZOOM_MIN, (float)i/100.0);
              frontview_zoom_distance_setting = LbLerp(16384, FRONTVIEW_CAMERA_ZOOM_MIN, (float)i/100.0);
          } else {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case 28: // DISPLAY_NUMBER
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = atoi(word_buf);
          }
          if ((i >= 0) && (i <= 32768)) {
              display_id = ((i == 0) ? 0 : (i - 1));
          } else {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case 29: // MUSIC_FROM_DISK
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1)
              features_enabled |= Ft_NoCdMusic;
          else
              features_enabled &= ~Ft_NoCdMusic;
          break;
      case 30: // HAND_SIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = atoi(word_buf);
          }
          if ((i >= 0) && (i <= SHRT_MAX)) {
              global_hand_scale = i/100.0;
          } else {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case 31: // LINE_BOX_SIZE
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = atoi(word_buf);
          }
          if ((i >= 0) && (i <= 32768)) {
              line_box_size = i;
          } else {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case 32: // COMMAND_CHAR
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              cmd_char = word_buf[0];
          }
          break;
      case 33: // API_ENABLED
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          api_enabled = (i == 1);
          break;
      case 34: // API_PORT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            i = atoi(word_buf);
          }
          if ((i >= 0) && (i <= UINT16_MAX)) {
              api_port = i;
          } else {
              CONFWRNLOG("Invalid API port '%s' in %s file.",COMMAND_TEXT(cmd_num),config_textname);
          }
          break;
      case ccr_comment:
          break;
      case ccr_endOfFile:
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
  free(buf);
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

/** CmdLine overrides allow settings from the command line to override the default settings, or those set in the config file.
 *
 * See enum CmdLineOverrides and struct StartupParameters -> TbBool overrides[CMDLINE_OVERRIDES].
 */
void process_cmdline_overrides(void)
{
  // Use CD for music rather than OGG files
  if (start_params.overrides[Clo_CDMusic])
  {
    features_enabled &= ~Ft_NoCdMusic;
  }
}

/******************************************************************************/
