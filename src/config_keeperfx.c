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
#include "bflib_fmvids.h"
#include "config_campaigns.h"
#include "engine_render.h"
#include "frontend.h"
#include "front_simple.h"
#include "gui_draw.h"
#include "scrcapt.h"
#include "sounds.h"
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
TbBool exit_on_lua_error = false;
TbBool FLEE_BUTTON_DEFAULT = false;
TbBool IMPRISON_BUTTON_DEFAULT = false;

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
  {"UKR", Lang_Ukrainian},
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
  {"GUI_BLINK_RATE",      15},
  {"NEUTRAL_FLASH_RATE",  16},
  {"FREEZE_GAME_ON_FOCUS_LOST"     , 17},
  {"UNLOCK_CURSOR_WHEN_GAME_PAUSED", 18},
  {"LOCK_CURSOR_IN_POSSESSION"     , 19},
  {"PAUSE_MUSIC_WHEN_GAME_PAUSED"  , 20},
  {"MUTE_AUDIO_ON_FOCUS_LOST"      , 21},
  {"STARTUP"                       , 22},
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
  {"EXIT_ON_LUA_ERROR"             , 35},
  {"TURNS_PER_SECOND"              , 36},
  {"FLEE_BUTTON_DEFAULT"           , 37},
  {"IMPRISON_BUTTON_DEFAULT"       , 38},
  {"FRAMES_PER_SECOND"             , 39},
  {"TAG_MODE_TOGGLING"             , 40},
  {"DEFAULT_TAG_MODE"              , 41},
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

  const struct NamedCommand startup_parameters[] = {
  {"LEGAL",                   1},
  {"FX",                      2},
  {"BULLFROG",                3}, // hidden
  {"EA",                      4}, // hidden
  {"INTRO",                   5},
  {NULL,                      0},
  };

  const struct NamedCommand tag_modes[] = {
  {"SINGLE",   1},
  {"DRAG",     2},
  {"PRESET",   3}, //legacy
  {"REMEMBER", 3},
  {NULL,       0},
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

void set_skip_heart_zoom_feature(TbBool enable)
{
  if (enable)
    features_enabled |= Ft_SkipHeartZoom;
  else
    features_enabled &= ~Ft_SkipHeartZoom;
}

TbBool get_skip_heart_zoom_feature(void)
{
  return ((features_enabled & Ft_SkipHeartZoom) != 0);
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

static void load_file_configuration(const char *fname, const char *sname, const char *config_textname, unsigned short flags)
{
  long len = LbFileLengthRnc(fname);
  if (len < 2)
  {
    if ((flags & CnfLd_IgnoreErrors) == 0)
      WARNMSG("%s file \"%s\" doesn't exist or is too small.", config_textname,sname);
    return;
  }
  if (len > 65536)
  {
    WARNMSG("%s file \"%s\" is too large.",config_textname,sname);
    return;
  }
  char* buf = (char*)calloc(len + 256, 1);
  if (buf == NULL)
    return;
  // Loading file data
  len = LbFileLoadAt(fname, buf);
  if (len>0)
  {
    SYNCDBG(7,"Processing %s file, %ld bytes",config_textname,len);
    buf[len] = '\0';
    // Set text line number - we don't have blocks so we need to initialize it manually
    text_line_number = 1;
    int32_t pos = 0;
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(conf_commands,cmd_num)
    while (pos<len)
    {
      // Finding command number in this line
      int i = 0, n = 0;
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
      case 15: // GUI_BLINK_RATE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              i = atoi(word_buf);
          }
          if (i < 1)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.", COMMAND_TEXT(cmd_num), config_textname);
              break;
          }
          else if (i > 160)
          {
              CONFWRNLOG("Value %d out of range for \"%s\" command of %s file. Set to 160.", i, COMMAND_TEXT(cmd_num), config_textname);
              i = 160;
          }
          gui_blink_rate = i;
          break;
      case 16: // NEUTRAL_FLASH_RATE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              i = atoi(word_buf);
          }
          if (i < 1)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.", COMMAND_TEXT(cmd_num), config_textname);
              break;
          }
          else if (i > 160)
          {
              CONFWRNLOG("Value %d out of range for \"%s\" command of %s file. Set to 160.",i, COMMAND_TEXT(cmd_num), config_textname);
              i = 160;
          }
          neutral_flash_rate = i;
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
        case 22: // STARTUP
          start_params.startup_flags = 0;
          while (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = get_id(startup_parameters, word_buf);
            switch (k)
            {
              case 1: // LEGAL
                set_flag(start_params.startup_flags, SFlg_Legal);
                n++;
                break;
              case 2: // FX
                set_flag(start_params.startup_flags, SFlg_FX);
                n++;
                break;
              case 3: // BULLFROG
                set_flag(start_params.startup_flags, SFlg_Bullfrog);
                n++;
                break;
              case 4: // EA
                set_flag(start_params.startup_flags, SFlg_EA);
                n++;
                break;
              case 5: // INTRO
                set_flag(start_params.startup_flags, SFlg_Intro);
                n++;
                break;
              default:
                CONFWRNLOG("Incorrect value of \"%s\" parameter \"%s\" in %s file.",
                   COMMAND_TEXT(cmd_num), word_buf, config_textname);
                break;
              }
          }
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
      case 35: // EXIT_ON_LUA_ERROR
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          exit_on_lua_error = (i == 1);
          break;
      case 36: // TURNS_PER_SECOND
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              i = atoi(word_buf);
          }
          if ((i >= 0) && (i <= INT32_MAX))
          {
              if (!start_params.overrides[Clo_GameTurns])
              {
                  start_params.num_fps = i;
              }
          }
          else {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.", COMMAND_TEXT(cmd_num), config_textname);
          }
          break;
      case 37: // FLEE_BUTTON_DEFAULT
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
                break;
          }
          if (i == 1) {
              FLEE_BUTTON_DEFAULT = true;
          } else {
              FLEE_BUTTON_DEFAULT = false;
          }
          break;
      case 38: // IMPRISON_BUTTON_DEFAULT
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          if (i == 1) {
              IMPRISON_BUTTON_DEFAULT = true;
          } else {
              IMPRISON_BUTTON_DEFAULT = false;
          }
          break;
      case 39: // FRAMES_PER_SECOND
          if (!start_params.overrides[Clo_FramesPerSecond] && get_conf_parameter_whole(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              i = parse_draw_fps_config_val(word_buf, &start_params.num_fps_draw_main, &start_params.num_fps_draw_secondary);
              if (i <= 0)
                  CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.", COMMAND_TEXT(cmd_num), config_textname);
          }
          break;
      case 40: // TAG_MODE_TOGGLING
          i = recognize_conf_parameter(buf,&pos,len,logicval_type);
          if (i <= 0)
          {
              CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",
                COMMAND_TEXT(cmd_num),config_textname);
            break;
          }
          right_click_tag_mode_toggle = (i == 1);
          break;
      case 41: // DEFAULT_TAG_MODE
          i = recognize_conf_parameter(buf,&pos,len,tag_modes);
          if (i <= 0)
          {
            CONFWRNLOG("Couldn't recognize \"%s\" command parameter in %s file.",COMMAND_TEXT(cmd_num),config_textname);
          }
          else
          {
            default_tag_mode = i;
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
  SYNCDBG(7,"%s loaded", config_textname);
  // Freeing
  free(buf);

}

static void load_configuration_for_mod_one(const struct ModConfigItem *mod_item)
{
    char mod_dir[256] = {0}, config_textname[256] = {0};
    sprintf(mod_dir, "%s/%s", MODS_DIR_NAME, mod_item->name);
    sprintf(config_textname, "Mod config '%s'", mod_item->name);

    char *fname = prepare_file_fmtpath_mod(mod_dir, FGrp_Main, "%s", keeper_config_file);
    load_file_configuration(fname, keeper_config_file, config_textname, CnfLd_IgnoreErrors);
}

static void load_configuration_for_mod_list(const struct ModConfigItem *mod_items, long mod_cnt)
{
    for (long i=0; i<mod_cnt; i++)
    {
        const struct ModConfigItem *mod_item = mod_items + i;
        if (mod_item->state.mod_dir == 0)
            continue;

        load_configuration_for_mod_one(mod_item);
    }
}

void load_configuration_for_mod_all(void)
{
    if (mods_conf.after_base_cnt > 0)
    {
        load_configuration_for_mod_list(mods_conf.after_base_item, mods_conf.after_base_cnt);
    }

    if (mods_conf.after_campaign_cnt > 0)
    {
        load_configuration_for_mod_list(mods_conf.after_campaign_item, mods_conf.after_campaign_cnt);
    }

    if (mods_conf.after_map_cnt > 0)
    {
        load_configuration_for_mod_list(mods_conf.after_map_item, mods_conf.after_map_cnt);
    }
}

short load_configuration(void)
{
  // Variables to use when recognizing parameters
  SYNCDBG(4,"Starting");
  // Preparing config file name and checking the file
  strcpy(install_info.inst_path,"");
  // Set default runtime directory and load the config file
  strcpy(keeper_runtime_directory,".");
  // Config file variables
  const char* sname; // Filename
  const char* fname; // Filepath

  if (start_params.ignore_mods == false)
  {
      load_mods_order_config_file();
      recheck_all_mod_exist();
  }
  else
  {
      SYNCMSG("Mod loading skipped");
  }

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

  const char *config_textname = "Base config";
  load_file_configuration(fname, sname, config_textname, 0);

  load_configuration_for_mod_all();

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

int parse_draw_fps_config_val(const char *arg, long *fps_draw_main, long *fps_draw_secondary)
{
  int cnt = 0, val1 = 0, val2 = 0;
  long len = strlen(arg);
  int32_t pos = 0;
  char word_buf[32];
  for (int i=0; i<2; i++)
  {
    if (get_conf_parameter_single(arg,&pos,len,word_buf,sizeof(word_buf)) <= 0)
      break;

    switch (i)
    {
    case 0:
      if (strcasecmp(word_buf, "auto") == 0) {
        val1=-1;
        cnt++;
      } else {
        val1 = atoi(word_buf);
        if (val1 >= 0){
          cnt++;
        } else {
          i=2; // jump out for loop
          break;
        }
      }
      break;
    case 1:
        val2 = atoi(word_buf);
        if (val2 >= 0){
          cnt++;
        } else {
          i=2; // jump out for loop
          break;
        }
      break;
    }
  }

  if (cnt > 0) {
    *fps_draw_main = val1;
  }
  if (cnt > 1) {
    *fps_draw_secondary = val2;
  }

  return cnt;
}


/******************************************************************************/
