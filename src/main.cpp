
#include <windows.h>
#include <winbase.h>
#include <math.h>
//#include <mmsystem.h>
#include "keeperfx.h"

#include "bflib_dernc.h"
#include "bflib_pom.h"
#include "bflib_memory.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_sprite.h"
#include "bflib_fileio.h"

#define CMDLN_MAXLEN 259
char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];
unsigned char palette_buf[768];

char window_class_name[128]="Bullfrog Shell";

struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE", &_DK_palette, NULL, PALETTE_SIZE, 0, 0},
    {"*SCRATCH", &_DK_scratch, NULL, 0x10000, 1, 0},
    {"", NULL, NULL, 0, 0, 0}, };

long gf_change_player_state(struct GuiBox *gbox, struct GuiBoxOption *goptn, char, long *tag)
{}

struct GuiBoxOption gui_main_cheat_list[] = { //gui_main_option_list in beta
    {"Null mode",              1, 0, gf_change_player_state, 0, 0, 0,  0, 0, 0, 0},
    {"Place tunneller mode",   1, 0, gf_change_player_state, 0, 0, 0,  3, 0, 0, 0},
    {"Place creature mode",    1, 0, gf_change_player_state, 0, 0, 0, 14, 0, 0, 0},
    {"Place hero mode",        1, 0, gf_change_player_state, 0, 0, 0,  4, 0, 0, 0},
    {"Destroy walls mode",     1, 0, gf_change_player_state, 0, 0, 0, 25, 0, 0, 0},
    {"Disease mode",           1, 0, gf_change_player_state, 0, 0, 0, 26, 0, 0, 0},
    {"Peter mode",	           1, 0, gf_change_player_state, 0, 0, 0, 27, 0, 0, 0},
    {"",                       2, 0,                   NULL, 0, 0, 0,  0, 0, 0, 0},
    {"Passenger control mode", 1, 0, gf_change_player_state, 0, 0, 0, 10, 0, 0, 0},
    {"Direct control mode",    1, 0, gf_change_player_state, 0, 0, 0, 11, 0, 0, 0},
    {"Order creature mode",    1, 0, gf_change_player_state, 0, 0, 0, 13, 0, 0, 0},
    {"",                       2, 0,                   NULL, 0,	0, 0,  0, 0, 0, 0},
    {"!",                      0, 0,                   NULL, 0, 0, 0,  0, 0, 0, 0},
};

// Dummy music support functions
int __stdcall _DK_PlayRedbookTrack(int)
{}
int __stdcall _DK_MonitorStreamedSoundTrack(void)
{}
int __stdcall _DK_StopRedbookTrack(void)
{}
int __stdcall _DK_StopMusic(void)
{}
// Dummy sound support functions
int __stdcall _DK_StopStreamedSample(void)
{}

int frontend_set_state(long nstate);

void error(const char *codefile,const int ecode,const char *message)
{
  LbErrorLog("In source %s:\n %5d - %s\n",codefile,ecode,message);
}

void LbSetIcon(unsigned short nicon)
{
  _DK_icon_index=nicon;
}

void LbDataLoadSetModifyFilenameFunction(ModDL_Fname_Func nmodify_dl_filename_func)
{
  _DK_modify_data_load_filename_function=nmodify_dl_filename_func;
}

short inline calculate_moon_phase(short add_to_log)
{
  //Moon phase calculation
  _DK_phase_of_moon=PhaseOfMoon::Calculate();
  if ((_DK_phase_of_moon > -0.05) && (_DK_phase_of_moon < 0.05))
  {
    if (add_to_log)
      LbSyncLog("Full moon %.4f\n", _DK_phase_of_moon);
    _DK_game.is_full_moon = 1;
  } else
  {
    if (add_to_log)
      LbSyncLog("Moon phase %.4f\n", _DK_phase_of_moon);
    _DK_game.is_full_moon = 0;
  }
  return _DK_game.is_full_moon;
}

short show_rawimage_screen(unsigned char *raw,unsigned char *pal,int width,int height,long tmdelay)
{
      if (height>_DK_lbDisplay.PhysicalScreenHeight)
           height=_DK_lbDisplay.PhysicalScreenHeight;
      _DK_LbPaletteSet(pal);
      long end_time;
      end_time = LbTimerClock() + tmdelay;
      long tmdelta;
      tmdelta = tmdelay/100;
      if (tmdelta>100) tmdelta=100;
      if (tmdelta<5) tmdelta=5;
      while ( LbTimerClock() < end_time )
      {
          _DK_LbWindowsControl();
          if ( _DK_LbScreenLock() == 1 )
          {
              unsigned char *raw_line;
              unsigned char *scrn_line;
              raw_line = raw;
              scrn_line = _DK_lbDisplay.WScreen;
              int i;
              for ( i = 0; i < height; i++ )
              {
                  memcpy(scrn_line, raw_line, width);
                  raw_line += width;
                  scrn_line += _DK_lbDisplay.GraphicsScreenWidth;
              }
              _DK_LbScreenUnlock();
          }
          _DK_LbScreenSwap();
          if ( _DK_lbKeyOn[KC_SPACE] || _DK_lbKeyOn[KC_ESCAPE] || _DK_lbKeyOn[KC_RETURN] ||
               _DK_lbDisplay.LeftButton || _DK_lbDisplay.RightButton )
          {
              _DK_lbKeyOn[KC_SPACE] = 0;
              _DK_lbKeyOn[KC_ESCAPE] = 0;
              _DK_lbKeyOn[KC_RETURN] = 0;
              _DK_lbDisplay.LeftButton = 0;
              _DK_lbDisplay.RightButton = 0;
              break;
          }
          LbSleepFor(tmdelta);
      }
}

short LbPaletteStopOpenFade(void)
{
    _DK_lbfade_open=0;
    return 1;
}

void ProperFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg)
{
    if ( _DK_lbUseSdk )
    {
        long last_loop_time;
        last_loop_time = LbTimerClock();
        while ( _DK_LbPaletteFade(pal, n, flg) < n )
        {
          if ( !_DK_lbKeyOn[KC_SPACE] && !_DK_lbKeyOn[KC_ESCAPE] && !_DK_lbKeyOn[KC_RETURN] &&
               !_DK_lbDisplay.LeftButton && !_DK_lbDisplay.RightButton )
          {
            LbSleepUntil(last_loop_time+50);
          }
          last_loop_time = LbTimerClock();
        }
    } else
    if ( pal != NULL )
    {
        _DK_LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        _DK_LbPaletteSet(palette_buf);
    }
}

void ProperForcedFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg)
{
    if (flg==Lb_PALETTE_FADE_OPEN)
    {
        _DK_LbPaletteFade(pal, n, flg);
        return;
    }
    if ( _DK_lbUseSdk )
    {
        long last_loop_time;
        last_loop_time = LbTimerClock();
        while ( _DK_LbPaletteFade(pal, n, flg) < n )
        {
          LbSleepUntil(last_loop_time+50);
          last_loop_time = LbTimerClock();
        }
    } else
    if ( pal != NULL )
    {
        _DK_LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        _DK_LbPaletteSet(palette_buf);
    }
}

void inline fade_in(void)
{
  ProperFadePalette(_DK_frontend_palette, 8, Lb_PALETTE_FADE_OPEN);
}

void inline fade_out(void)
{
  ProperFadePalette(0, 8, Lb_PALETTE_FADE_CLOSED);
  _DK_LbScreenClear(0);
}

/*
 * Displays the 320x200 loading screen.
 * Will work properly even on 640x4?0 resolutions.
 * @return Returns true on success.
 */
short display_lores_loading_screen(void)
{
  static const char *func_name="display_lores_loading_screen";
  const int img_width = 320;
  const int img_height = 200;
  char fname[256];
  unsigned char *buf=NULL;

  memset(palette_buf, 0, 768);
  _DK_LbPaletteSet(palette_buf);

  _DK_check_cd_in_drive();
  short dproceed=true;
  if (dproceed)
  {
      buf=(unsigned char *)malloc(img_width*img_height);
      if (buf==NULL)
      {
          error(func_name, 1056, "Cannot allocate bitmap buffer");
          dproceed = false;
      }
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",_DK_keeper_runtime_directory, "data", "loading.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",_DK_keeper_runtime_directory,"data","loading.pal");
      _DK_check_cd_in_drive();
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      _DK_LbScreenClear(0);
      dproceed = (_DK_LbScreenLock()==1);
  }
  if ( dproceed )
  {
      int w,h;
      unsigned char *dst;
      unsigned char *src;
      if ((_DK_lbDisplay.ScreenMode!=Lb_SCREEN_MODE_640_400_8) &&
          (_DK_lbDisplay.ScreenMode!=Lb_SCREEN_MODE_640_480_8))
      {
        dst = _DK_lbDisplay.WScreen;
        src = buf;
        for (h=img_height;h>0;h--)
        {
          memcpy(dst, src, img_width);
          src += img_width;
          dst += _DK_lbDisplay.GraphicsScreenWidth;
        }
      } else
      {
        if ( _DK_lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
        {
          dst = _DK_lbDisplay.WScreen + 40*_DK_lbDisplay.GraphicsScreenWidth;
        } else
        {
          dst = _DK_lbDisplay.WScreen;
        }
        src = buf;
        for (h=img_height;h>0;h--)
        {
          for (w=img_width;w>0;w--)
          {
            dst[0] = *src;
            dst[1] = *src;
            dst += 2;
            src++;
          }
          memcpy(dst+_DK_lbDisplay.GraphicsScreenWidth-640, dst-640, 640);
          dst += (_DK_lbDisplay.GraphicsScreenWidth<<1)-640;
        }
      }
      _DK_LbScreenUnlock();
      _DK_LbScreenSwap();
  }
  free(buf);
  return dproceed;
}

/*
 * Displays the 640x480 loading screen.
 * Will work properly only on high resolutions.
 * @return Returns true on success.
 */
short display_hires_loading_screen(void)
{
  static const char *func_name="display_hires_loading_screen";
  const int img_width = 640;
  const int img_height = 480;
  char fname[256];
  unsigned char *buf=NULL;

  memset(palette_buf, 0, 768);
  _DK_LbPaletteSet(palette_buf);

  _DK_check_cd_in_drive();
  short dproceed=true;
  if (dproceed)
  {
      buf=(unsigned char *)malloc(img_width*img_height);
      if (buf==NULL)
      {
          error(func_name, 1056, "Cannot allocate bitmap buffer");
          dproceed = false;
      }
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",_DK_keeper_runtime_directory, "fxdata", "loading_640.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",_DK_keeper_runtime_directory,"fxdata","loading_640.pal");
      _DK_check_cd_in_drive();
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      _DK_LbScreenClear(0);
      dproceed = (_DK_LbScreenLock()==1);
  }
  if ( dproceed )
  {
      int w,h;
      unsigned char *dst;
      unsigned char *src;
      int cp_height;
      if ( _DK_lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
      {
          src = buf;
          cp_height=480;
      } else
      {
          src = buf + 40*img_width;
          cp_height=400;
      }
      dst = _DK_lbDisplay.WScreen;
      for (h=cp_height;h>0;h--)
      {
          memcpy(dst, src, img_width);
          src += img_width;
          dst += _DK_lbDisplay.GraphicsScreenWidth;
      }
      _DK_LbScreenUnlock();
      _DK_LbScreenSwap();
  }
  free(buf);
  return dproceed;
}

short display_loading_screen(void)
{
    short done=false;
    if ((_DK_lbDisplay.ScreenMode==Lb_SCREEN_MODE_640_400_8) ||
        (_DK_lbDisplay.ScreenMode==Lb_SCREEN_MODE_640_480_8))
    {
        done=display_hires_loading_screen();
    }
    if (!done)
    {
        done=display_lores_loading_screen();
    }
    LbPaletteStopOpenFade();
    ProperForcedFadePalette(palette_buf, 64, Lb_PALETTE_FADE_CLOSED);
    return done;
}


/*
 * Plays a smacker animation file and sets frontend state to nstate.
 * @param nstate Frontend state; -1 means no change, -2 means don't even
 *    change screen mode.
 * @return Returns false if fatal error occured and probram execution should end.
 */
short play_smacker_file(char *filename, int nstate)
{
  static const char *func_name="play_smacker_file";
  unsigned int movie_flags = 0;
  if ( _DK_SoundDisabled )
    movie_flags |= 1;
  short result=1;

  if ((result)&&(nstate>-2))
  {
    if ( _DK_setup_screen_mode_minimal(1) )
    {
      _DK_LbMouseChangeSprite(0);
      _DK_LbScreenClear(0);
      _DK_LbScreenSwap();
    } else
      result=0;
  }
  if (result)
  {
    // Fail in playing shouldn't set result=0, because result=0 means fatal error.
    if ( !_DK_play_smk_(filename, 0, movie_flags | 0x100) )
      error(func_name, 2357, "Smacker error");
  }
  if (nstate>-2)
  {
    if ( !_DK_setup_screen_mode_minimal(13) )
    {
      _DK_FatalError = 1;
      _DK_exit_keeper = 1;
      return 0;
    }
  } else
  {
    memset(_DK_frontend_palette, 0, PALETTE_SIZE);
  }
  _DK_LbScreenClear(0);
  _DK_LbScreenSwap();
  _DK_LbPaletteSet(_DK_frontend_palette);
  if ( nstate >= 0 )
    frontend_set_state(nstate);
  _DK_lbDisplay.LeftButton = 0;
  _DK_lbDisplay.RightButton = 0;
  _DK_lbDisplay.MiddleButton = 0;
  if (nstate>-2)
    _DK_LbMouseSetPosition(_DK_lbDisplay.PhysicalScreenWidth/2, _DK_lbDisplay.PhysicalScreenHeight/2);
  return 1;
}

short setup_game(void)
{
  static const char *func_name="setup_game";
  // CPU and memory status
  struct CPU_INFO cpu_info;
  struct _MEMORYSTATUS msbuffer;
  char filename[DISKPATH_SIZE];

  // Do only a very basic setup
  
  _DK_get_cpu_info(&cpu_info);
  msbuffer.dwLength = 32;
  GlobalMemoryStatus(&msbuffer);
  if ( msbuffer.dwTotalPhys <= (8*1024*1024) )
      _DK_mem_size = 8;
  else
  if ( msbuffer.dwTotalPhys <= (16*1024*1024) )
      _DK_mem_size = 16;
  else
  if ( msbuffer.dwTotalPhys <= (24*1024*1024) )
      _DK_mem_size = 24;
  else
      _DK_mem_size = 32;
  LbSyncLog("PhysicalMemory %d\n", _DK_mem_size);

  // Configuration file
  if ( !_DK_load_configuration() )
  {
      error(func_name, 1912, "Configuration load error.");
      return 0;
  }

  //Moon phase calculation
  calculate_moon_phase(true);

  LbIKeyboardOpen();

  if ( _DK_LbDataLoadAll(legal_load_files) != 0 )
  {
      error(func_name, 1911, "Error on allocation/loading of legal_load_files.");
      return 0;
  }

  short result=1;
  short need_clean;

  // This should be closed in int initial_setup(void)

  // View the legal screen

  memset(_DK_palette, 0, PALETTE_SIZE);
  if ( _DK_LbScreenSetup(Lb_SCREEN_MODE_640_480_8, LEGAL_WIDTH, LEGAL_HEIGHT, _DK_palette, 1, 0) != 1 )
  {
      error(func_name, 1912, "Screen mode setup error.");
      return 0;
  }

  unsigned char *legal_raw;

  if ( result )
  {
      _DK_LbScreenClear(0);
      _DK_LbScreenSwap();
      legal_raw = LbMemoryAlloc(LEGAL_WIDTH*LEGAL_HEIGHT);
      if (legal_raw == NULL)
      {
          error(func_name, 1456, "Unable to open allocate screen buffer");
          result=0;
      }
  }

  if ( result )
  {
      int loaded_size=LbFileLoadAt("data/legal.raw", legal_raw);
      if ( loaded_size != LEGAL_WIDTH*LEGAL_HEIGHT )
      {
          error(func_name, 1457, "Unable to load legal image data");
          result=0;
      }
  }
  if ( result )
  {
      int loaded_size=LbFileLoadAt("data/legal.pal", _DK_palette);
      if ( loaded_size != PALETTE_SIZE )
      {
          error(func_name, 1458, "Unable to load legal image palette");
          result=0;
      }
  }

  if ( result )
      show_rawimage_screen(legal_raw,_DK_palette,LEGAL_WIDTH,LEGAL_HEIGHT,3000);
  else
      LbSyncLog("%s - Legal image skipped\n", func_name);

  // Now do more setup

  _DK_init_sound();

  // View second legal screen

  result = (legal_raw!=NULL);

  if ( result )
  {
      int loaded_size=LbFileLoadAt("fxdata/startup_fx.raw", legal_raw);
      if ( loaded_size != LEGAL_WIDTH*LEGAL_HEIGHT )
      {
          error(func_name, 1477, "Unable to open startup_fx Screen");
          result=0;
      }
  }
  if ( result )
  {
      int loaded_size=LbFileLoadAt("fxdata/startup_fx.pal", _DK_palette);
      if ( loaded_size != PALETTE_SIZE )
      {
          error(func_name, 1484, "Unable to open startup_fx Palette");
          result=0;
      }
  }

  if ( result )
      show_rawimage_screen(legal_raw,_DK_palette,LEGAL_WIDTH,LEGAL_HEIGHT,4000);
  else
      LbSyncLog("%s - startup_fx image skipped\n", func_name);

  {
      LbMemoryFree(legal_raw);
      memset(_DK_palette, 0, PALETTE_SIZE);
      _DK_LbScreenClear(0);
      _DK_LbScreenSwap();
  }

  result = 1;
  // The 320x200 mode is required only for the intro;
  // loading and no CD screens can run in both 320x2?0 and 640x4?0.
  if ( result && (!_DK_game.no_intro) )
  {
    int mode_ok = _DK_LbScreenSetup(Lb_SCREEN_MODE_320_200_8, 320, 200, _DK_palette, 2, 0);
    if (mode_ok != 1)
    {
      error(func_name, 1500, "Can't enter 320x200 screen mode");
      result=0;
    }
  }

  if ( result )
  {
        _DK_LbScreenClear(0);
        _DK_LbScreenSwap();
        _DK_check_cd_in_drive();
  }
  if ( result && (!_DK_game.no_intro) )
  {
      sprintf(filename, "%s/ldata/%s",_DK_datafiles_path,"intromix.smk");
      play_smacker_file(filename, -2);
/*
      int movie_flags;
      movie_flags = 0;
      if ( _DK_SoundDisabled )
          movie_flags = 1;
      if ( !_DK_play_smk_(filename, 0, movie_flags | 0x100) )
          error(func_name, 1555, "Smacker error while playing intro");
      memset(_DK_palette, 0, PALETTE_SIZE);
      _DK_LbPaletteSet(_DK_palette);
      _DK_LbScreenClear(0);
      _DK_LbScreenSwap();
*/
  }
  if ( result )
  {
        display_loading_screen();
  }
  _DK_LbDataFreeAll(legal_load_files);

  char *text_end;
  if ( result )
  {
      long filelen;
      filelen = LbFileLengthRnc("data/text.dat");
      _DK_strings_data = (char *)LbMemoryAlloc(filelen + 256);
      if ( _DK_strings_data == NULL )
      {
        exit(1);
      }
      text_end = _DK_strings_data+filelen+255;
      int loaded_size=LbFileLoadAt("data/text.dat", _DK_strings_data);
      if (loaded_size<2*STRINGS_MAX)
      {
          error(func_name, 1501, "Strings data too small");
          result = 0;
      }
  }
  if ( result )
  {
      char **text_arr = _DK_strings;
      int text_idx;
      char *text_ptr;
      text_idx = STRINGS_MAX;
      text_ptr = _DK_strings_data;
      while (text_idx>=0)
      {
        if (text_ptr>=text_end)
        {
            result = 0;
            break;
        }
        *text_arr = text_ptr;
        text_arr++;
        char chr_prev;
        do {
            chr_prev = *text_ptr;
            text_ptr++;
        } while ((chr_prev!='\0')&&(text_ptr<text_end));
        text_idx--;
      }
  }

  if ( result )
  {
      _DK_IsRunningMark();
      if ( !_DK_initial_setup() )
      {
        error(func_name, 1502, "Initial setup failed");
        result = 0;
      }
  }
  if ( result )
  {
      _DK_load_settings();
      if ( !_DK_setup_game_sound_heap() )
      {
        error(func_name, 1503, "Sound heap setup failed");
        result = 0;
      }
  }

  if ( result )
  {
        _DK_init_creature_scores();
        if ( cpu_info.field_0 )
        {
            if ( ((cpu_info.field_0>>8) & 0x0Fu) >= 0x06 )
              _DK_set_cpu_mode(1);
        }
        _DK_set_gamma(_DK_settings.gamma_correction, 0);
        _DK_SetRedbookVolume(_DK_settings.redbook_volume);
        _DK_SetSoundMasterVolume(_DK_settings.sound_volume);
        _DK_SetMusicMasterVolume(_DK_settings.sound_volume);
        _DK_setup_3d();
        _DK_setup_stuff();
        _DK_init_lookups();
        result = 1;
  }
  return result;
}

void update_left_button_released(void)
{
  _DK_left_button_released = 0;
  _DK_left_button_double_clicked = 0;
  if ( _DK_lbDisplay.LeftButton )
  {
    _DK_left_button_held = 1;
    _DK_left_button_held_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
    _DK_left_button_held_y = _DK_lbDisplay.MMouseY * _DK_pixel_size;
  }
  if (_DK_left_button_held)
  {
    if (!_DK_lbDisplay.MLeftButton)
    {
      _DK_left_button_released = 1;
      _DK_left_button_held = 0;
      _DK_left_button_released_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
      _DK_left_button_released_y = _DK_lbDisplay.MMouseY * _DK_pixel_size;
      if ( _DK_left_button_click_space_count < 5 )
      {
        _DK_left_button_double_clicked = 1;
        _DK_left_button_double_clicked_x = _DK_left_button_released_x;
        _DK_left_button_double_clicked_y = _DK_left_button_released_y;
      }
      _DK_left_button_click_space_count = 0;
    }
  } else
  {
    if ( _DK_left_button_click_space_count < 2147483647 )
      _DK_left_button_click_space_count++;
  }
}

void update_right_button_released(void)
{
  _DK_right_button_released = 0;
  _DK_right_button_double_clicked = 0;
  if (_DK_lbDisplay.RightButton)
  {
    _DK_right_button_held = 1;
    _DK_right_button_held_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
    _DK_right_button_held_y = _DK_lbDisplay.MMouseY * _DK_pixel_size;
  }
  if ( _DK_right_button_held )
  {
    if ( !_DK_lbDisplay.MRightButton )
    {
      _DK_right_button_released = 1;
      _DK_right_button_held = 0;
      _DK_right_button_released_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
      _DK_right_button_released_y = _DK_lbDisplay.MMouseY * _DK_pixel_size;
      if ( _DK_right_button_click_space_count < 5 )
      {
        _DK_right_button_double_clicked = 1;
        _DK_right_button_double_clicked_x = _DK_right_button_released_x;
        _DK_right_button_double_clicked_y = _DK_right_button_released_y;
      }
      _DK_right_button_click_space_count = 0;
    }
  } else
  {
    if ( _DK_right_button_click_space_count < 2147483647 )
      _DK_right_button_click_space_count++;
  }
}

void update_left_button_clicked(void)
{
  _DK_left_button_clicked = _DK_lbDisplay.LeftButton;
  _DK_left_button_clicked_x = _DK_lbDisplay.MouseX * _DK_pixel_size;
  _DK_left_button_clicked_y = _DK_lbDisplay.MouseY * _DK_pixel_size;
}

void update_right_button_clicked(void)
{
  _DK_right_button_clicked = _DK_lbDisplay.RightButton;
  _DK_right_button_clicked_x = _DK_lbDisplay.MouseX * _DK_pixel_size;
  _DK_right_button_clicked_y = _DK_lbDisplay.MouseY * _DK_pixel_size;
}

void update_mouse(void)
{
  update_left_button_released();
  update_right_button_released();
  update_left_button_clicked();
  update_right_button_clicked();
  _DK_lbDisplay.LeftButton = 0;
  _DK_lbDisplay.RightButton = 0;
}

void keeper_gameplay_loop(void)
{
    static const char *func_name="keeper_gameplay_loop";
    static char text[255];
    static unsigned long last_loop_time=0;
    static unsigned long prev_time1=0;
    static unsigned long cntr_time1=0;
    static unsigned long prev_time2=0;
    static unsigned long cntr_time2=0;
    struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
    _DK_PaletteSetPlayerPalette(player, _DK_palette);
    if ( _DK_game.numfield_C & 2 )
      _DK_initialise_eye_lenses();
    LbSyncLog("Entering the gameplay loop for level %d\n",(int)_DK_game.level_number);

    //the main gameplay loop starts
    while ( (!_DK_quit_game)&&(!_DK_exit_keeper) )
    {
      if ( _DK_game.flags_font & 0x10 )
      {
        if ( _DK_game.seedchk_random_used == 4 )
          ;//rndseed_nullsub();
      }

      // Some timing (which I don't understand; but it affects graphics)
      short all_ok;
      all_ok = 1;
      if ( !(_DK_game.numfield_C & 1) )
      {
        if ( (!_DK_game.numfield_149F34) && (!_DK_game.numfield_149F38) )
        {
          unsigned long curr_time;
          curr_time = clock();
          cntr_time2++;
          if ( curr_time-prev_time2 >= 1000 )
          {
              double time_fdelta = 1000.0*((double)(cntr_time2))/((double)(curr_time-prev_time2));
              prev_time2 = curr_time;
              _DK_game.time_delta = (unsigned long)(time_fdelta*256.0);
              cntr_time2 = 0;
          }
          if ( _DK_game.timingvar1 && _DK_game.seedchk_random_used % _DK_game.timingvar1 )
          {
            all_ok = 0;
          }
        } else
        if ( ((_DK_game.seedchk_random_used & 0x3F)==0) ||
           ((_DK_game.numfield_149F38) && ((_DK_game.seedchk_random_used & 7)==0)) )
        {
            unsigned long curr_time;
            curr_time = clock();
            if ( (curr_time-prev_time1) < 5000 )
            {
              cntr_time1 += 64;
            } else
            {
              double time_fdelta = 1000.0*((double)(cntr_time1+64))/(curr_time-prev_time1);
              prev_time1 = curr_time;
              _DK_game.time_delta = (unsigned long)(time_fdelta*256.0);
              cntr_time1 = 0;
            }
        } else
        {
            all_ok = 0;
        }
      }
      if (!all_ok)
          all_ok = !_DK_LbIsActive();

      update_mouse();
      _DK_input_eastegg();
      _DK_input();
      _DK_update();

      if ( all_ok )
      {
        _DK_LbScreenClear(0);
        if ( _DK_LbScreenLock() == 1 )
        {
          _DK_setup_engine_window(player->field_448, player->field_44A,
                player->field_444, player->field_446);
          _DK_redraw_display();
          _DK_LbScreenUnlock();
        }
      }
      do {
        if ( !_DK_LbWindowsControl() )
        {
          if ( !(_DK_game.numfield_A & 1) )
          {
            _DK_exit_keeper = 1;
            break;
          }
          sprintf(text, "alex");
          LbSyncLog("%s - %s\n",func_name,text);
        }
        if ( (_DK_game.numfield_A&1) || _DK_LbIsActive() )
          break;
      } while ( (!_DK_exit_keeper) && (!_DK_quit_game) );

      if ( _DK_game.numfield_A & 0x10 )
      {
        _DK_game.numfield_A &= 0xEFu;
        _DK_cumulative_screen_shot();
      }
      if ( _DK_game.numfield_A & 8 )
        _DK_anim_record_frame(_DK_lbDisplay.WScreen, _DK_palette);

      // Direct information/error messages
      if ( _DK_LbScreenLock() == 1 )
      {
        // Display message for debug purposes
        //_DK_LbTextDraw(200/_DK_pixel_size, 50/_DK_pixel_size, text);text[0]='\0';
        if ( _DK_game.numfield_A & 2 )
        {
          sprintf(text, "OUT OF SYNC (GameTurn %7d)", _DK_game.seedchk_random_used);
          error(func_name, 413, text);
          if ( _DK_lbDisplay.WScreen != NULL )
            _DK_LbTextDraw(300/_DK_pixel_size, 200/_DK_pixel_size, "OUT OF SYNC");
        }
        if ( _DK_game.numfield_A & 4 )
        {
          sprintf(text, "SEED OUT OF SYNC (GameTurn %7d)", _DK_game.seedchk_random_used);
          error(func_name, 427, text);
          if ( _DK_lbDisplay.WScreen != NULL)
            _DK_LbTextDraw(300/_DK_pixel_size, 220/_DK_pixel_size, "SEED OUT OF SYNC");
        }
        _DK_LbScreenUnlock();
      }

      // Music and sound control
      if ( !_DK_SoundDisabled )
        if ((!_DK_game.numfield_149F34) && (!_DK_game.numfield_149F38))
        {
            _DK_MonitorStreamedSoundTrack();
            _DK_process_sound_heap();
        }

      // Move the graphics window to center of screen buffer and swap screen
      if ( all_ok )
      {
        // For resolution 640x480, move the graphics data 40 lines lower
        if ( _DK_lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
          if ( _DK_LbScreenLock() == 1 )
          {
            int i;
            int scrmove_x=0;
            int scrmove_y=40;
            int scanline_len=640;
            for (i=400;i>=0;i--)
              memcpy(_DK_lbDisplay.WScreen+scanline_len*(i+scrmove_y)+scrmove_x, _DK_lbDisplay.WScreen+scanline_len*i, scanline_len-scrmove_x);
            memset(_DK_lbDisplay.WScreen, 0, scanline_len*scrmove_y);
            _DK_LbScreenUnlock();
          }
        _DK_LbScreenSwap();
      }

      // Make delay if the machine is too fast
      if ( (!_DK_game.field_149E81) || (!_DK_game.numfield_149F34) )
      {
        if ( _DK_game.numfield_D & 0x10 )
        {
          unsigned long sleep_end = last_loop_time + 1000;
          LbSleepUntil(sleep_end);
          last_loop_time = LbTimerClock();
        } else
        if ( !_DK_game.timingvar1 )
        {
          unsigned long sleep_end = last_loop_time + 1000/_DK_game.num_fps;
          LbSleepUntil(sleep_end);
          last_loop_time = LbTimerClock();
        }
      }

      if ( _DK_game.numfield_149F42 == _DK_game.seedchk_random_used )
        _DK_exit_keeper = 1;
    } // end while
    LbSyncLog("Gameplay loop finished after %u turns\n",_DK_game.seedchk_random_used);
}

void initialise_load_game_slots(void)
{
  _DK_load_game_save_catalogue(_DK_save_game_catalogue);
  _DK_number_of_saved_games = 0;
  int entry_idx;
  for (entry_idx=0;entry_idx<8;entry_idx++)
  {
    if ( _DK_save_game_catalogue[entry_idx].used )
      _DK_number_of_saved_games++;
    entry_idx++;
  }
}

void load_game_update(void)
{
    if ((_DK_number_of_saved_games>0) && (_DK_load_game_scroll_offset>=0))
    {
        if ( _DK_number_of_saved_games - 1 < _DK_load_game_scroll_offset )
          _DK_load_game_scroll_offset = _DK_number_of_saved_games - 1;
    } else
    {
        _DK_load_game_scroll_offset = 0;
    }
}

void define_key_input(void)
{
    if ( _DK_lbInkey == 1 )
    {
          _DK_defining_a_key = 0;
          _DK_lbInkey = 0;
    } else
    if ( _DK_lbInkey )
    {
            short ctrl_state = 0;
            if ( _DK_lbKeyOn[KC_LCONTROL] || (_DK_lbKeyOn[KC_RCONTROL]) )
              ctrl_state = 1;
            short shift_state = 0;
            if ( _DK_lbKeyOn[KC_LSHIFT] || (_DK_lbKeyOn[KC_RSHIFT]) )
              shift_state = 1;
            if ( _DK_set_game_key(_DK_defining_a_key_id, _DK_lbInkey, shift_state, ctrl_state) )
              _DK_defining_a_key = 0;
            _DK_lbInkey = 0;
    }
}

void frontend_load_high_score_table(void)
{
    static const char *hiscores_fname="data\\hiscores.dat";
    if ( LbFileLoadAt(hiscores_fname, _DK_high_score_table) != sizeof(_DK_high_score_table) )
    {
        int i;
        int npoints = 1000;
        int nlevel = 10;
        for (i=0;i<10;i++)
        {
            sprintf(_DK_high_score_table[i].name, "Bullfrog");
            _DK_high_score_table[i].score=npoints;
            _DK_high_score_table[i].level=nlevel;
            npoints -= 100;
            nlevel -= 1;
        }
        LbFileSaveAt(hiscores_fname, _DK_high_score_table, sizeof(_DK_high_score_table));
    }
}

void add_score_to_high_score_table(void)
{
    struct Dungeon *dungeon=&(_DK_game.dungeon[_DK_my_player_number]);
    // Determining position of the new entry
    int idx;
    long new_score=dungeon->player_score;
    for (idx=0;idx<10;idx++)
    {
        if (_DK_high_score_table[idx].score <= new_score )
          break;
    }
    // If the new score is poor, return
    if (idx>9) return;
    // Moving entries down
    int k;
    for (k=8;k>=idx;k--)
    {
        memcpy(&_DK_high_score_table[k+1],&_DK_high_score_table[k],sizeof(struct HighScore));
    }
    // Preparing the new entry
    _DK_high_score_entry_input_active = idx;
    _DK_high_score_entry[0] = '\0';
    _DK_high_score_entry_index = 0;
    _DK_high_score_table[idx].score = new_score;
    _DK_high_score_table[idx].level = _DK_game.level_number - 1;
}

void frontend_load_data_from_cd(void)
{
    LbDataLoadSetModifyFilenameFunction(_DK_mdlf_for_cd);
}

void frontstory_load(void)
{
    static const char *func_name="frontstory_load";
    _DK_check_cd_in_drive();
    frontend_load_data_from_cd();
    if ( _DK_LbDataLoadAll(_DK_frontstory_load_files) )
    {
        error(func_name, 2790, "Unable to Load FRONT STORY FILES");
    } else
    {
        LbDataLoadSetModifyFilenameFunction(_DK_mdlf_default);
        _DK_LbSpriteSetupAll(_DK_frontstory_setup_sprites);
        _DK_LbPaletteSet(_DK_frontend_palette);
        srand(LbTimerClock());
        _DK_frontstory_text_no = rand() % 26 + 803;
    }
}
void inline frontstory_unload(void)
{
    _DK_LbDataFreeAll(_DK_frontstory_load_files);
}

short continue_game_available()
{
      static char buf[255];
      static short continue_needs_checking_file = 1;
      sprintf(buf, "%s\\%s\\%s",_DK_keeper_runtime_directory,"save","continue.sav");
      _DK_check_cd_in_drive();
      if ( LbFileLength(buf) != sizeof(struct Game) )
          return false;
      if ( continue_needs_checking_file )
      {
          TbFileHandle fh=LbFileOpen(buf,Lb_FILE_MODE_READ_ONLY);
          if ( fh != -1 )
          {
            LbFileRead(fh, buf, 10);
            LbFileClose(fh);
            if ( buf[0] )
              _DK_game.level_number = (unsigned char)buf[0];
          }
          continue_needs_checking_file = 0;
      }
      if ( _DK_game.level_number > 20 )
          return false;
      else
          return true;
}

int frontend_set_state(long nstate)
{
    static const char *func_name="frontend_set_state";
    static char text[255];
    //_DK_frontend_set_state(nstate);return nstate;
  switch ( _DK_frontend_menu_state )
  {
    case 0:
      _DK_init_gui();
      sprintf(text, "%s\\%s\\front.pal", _DK_datafiles_path,"ldata");
      _DK_check_cd_in_drive();
      if ( LbFileLoadAt(text, &_DK_frontend_palette) != 768 )
        error(func_name, 1323, "Unable to load FRONTEND PALETTE");
      _DK_check_cd_in_drive();
      frontend_load_high_score_table();
      _DK_LbMouseSetPosition(_DK_lbDisplay.PhysicalScreenWidth>>1, _DK_lbDisplay.PhysicalScreenHeight>>1);
      update_mouse();
      break;
    case 1: // main menu state
      _DK_turn_off_menu(18);
      break;
    case 2:
      _DK_turn_off_menu(19);
      break;
    case 3:
      _DK_frontmap_unload();
      _DK_frontend_load_data();
      break;
    case 4:
      _DK_turn_off_menu(20);
      break;
    case 5:
      _DK_turn_off_menu(21);
      break;
    case 6:
      _DK_turn_off_menu(22);
      break;
    case 12:
    case 29:
      frontstory_unload();
      break;
    case 13:
      if ( !(_DK_game.flags_cd & 0x10) )
        _DK_StopRedbookTrack();
      break;
    case 15:
      _DK_turn_off_menu(23);
      _DK_frontnet_modem_reset();
      break;
    case 16:
      _DK_turn_off_menu(24);
      _DK_frontnet_serial_reset();
      break;
    case 17:
      _DK_StopStreamedSample();
      _DK_turn_off_menu(25);
      break;
    case 18:
      _DK_turn_off_menu(26);
      break;
    case 19:
      _DK_fronttorture_unload();
      _DK_frontend_load_data();
      break;
    case 24:
      _DK_frontnetmap_unload();
      _DK_frontend_load_data();
      break;
    case 26:
      _DK_turn_off_menu(36);
      _DK_save_settings();
      break;
    case 27:
      _DK_turn_off_menu(39);
      if ( !(_DK_game.flags_cd & 0x10) )
        _DK_StopRedbookTrack();
      break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 14: //demo state (intro/credits)
    case 21:
    case 25:
      break;
    default:
      error(func_name, 1444, "Unhandled FRONTEND previous state");
      break;
  }
  if ( _DK_frontend_menu_state )
    fade_out();
  _DK_fade_palette_in = 1;
  LbSyncLog("Frontend state change from %u into %u\n",_DK_frontend_menu_state,nstate);
  switch ( nstate )
  {
    case 0:
      _DK_LbMouseChangeSpriteAndHotspot(0, 0, 0);
      break;
    case 1:
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_continue_game_option_available = continue_game_available();
      _DK_turn_on_menu(18);
      // change when all references to frontend_set_state() are rewritten
      //time_last_played_demo = LbTimerClock();
      _DK_time_last_played_demo=timeGetTime();
      _DK_last_mouse_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
      _DK_last_mouse_y = _DK_GetMouseY();
      _DK_fe_high_score_table_from_main_menu = 1;
      _DK_game.numfield_A &= 0xFEu;
      break;
    case 2:
      _DK_turn_on_menu(19);
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      break;
    case 3:
      if ( !_DK_frontmap_load() )
        nstate = 7;
      break;
    case 4:
      _DK_turn_on_menu(20);
      _DK_frontnet_service_setup();
      break;
    case 5:
      _DK_turn_on_menu(21);
      _DK_frontnet_session_setup();
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_game.numfield_A &= 0xFEu;
      break;
    case 6:
      _DK_turn_on_menu(22);
      _DK_frontnet_start_setup();
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_game.numfield_A |= 1;
      break;
    case 7:
    case 9:
    case 10:
    case 11:
    case 14:
    case 21:
    case 25:
      _DK_fade_palette_in = 0;
      break;
    case 8:
      if ( _DK_game.flags_font & 0x10 )
        ;//rndseed_nullsub();
      _DK_fade_palette_in = 0;
      break;
    case 12:
    case 29:
      frontstory_load();
      break;
    case 13:
      _DK_credits_offset = _DK_lbDisplay.PhysicalScreenHeight;
      _DK_credits_end = 0;
      _DK_LbTextSetWindow(0, 0, _DK_lbDisplay.PhysicalScreenWidth, _DK_lbDisplay.PhysicalScreenHeight);
      _DK_lbDisplay.DrawFlags = 256;
      break;
    case 15:
      _DK_turn_on_menu(23);
      _DK_frontnet_modem_setup();
      break;
    case 16:
      _DK_turn_on_menu(24);
      _DK_frontnet_serial_setup();
      break;
    case 17:
      _DK_turn_on_menu(25);
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_frontstats_set_timer(); // note: rewrite this in pack with frontstats_update
      break;
    case 18:
      _DK_turn_on_menu(26);
      if ( _DK_game.dungeon[_DK_my_player_number].allow_save_score )
      {
        _DK_game.dungeon[_DK_my_player_number].allow_save_score = false;
        add_score_to_high_score_table();
      }
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_lbInkey = 0;
      break;
    case 19:
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_fronttorture_load();
      break;
    case 24:
      _DK_LbMouseChangeSpriteAndHotspot(&_DK_frontend_sprite[1], 0, 0);
      _DK_frontnetmap_load();
      break;
    case 26:
      _DK_defining_a_key = 0;
      _DK_define_key_scroll_offset = 0;
      _DK_turn_on_menu(36);
      break;
    case 27:
      _DK_turn_on_menu(39);
      break;
    default:
      error(func_name, 1609, "Unhandled FRONTEND new state");
      break;
  }
  _DK_frontend_menu_state = nstate;
  return _DK_frontend_menu_state;
}


short end_input(void)
{
    if ( _DK_lbKeyOn[KC_SPACE] )
    {
        _DK_lbKeyOn[KC_SPACE] = 0;
        frontend_set_state(1);
    } else
    if ( _DK_lbKeyOn[KC_RETURN] )
    {
        _DK_lbKeyOn[KC_RETURN] = 0;
        frontend_set_state(1);
    } else
    if ( _DK_lbKeyOn[KC_ESCAPE] )
    {
        _DK_lbKeyOn[KC_ESCAPE] = 0;
        frontend_set_state(1);
    } else
    if ( _DK_left_button_clicked )
    {
        _DK_left_button_clicked = 0;
        frontend_set_state(1);
    } else
        return false;
    return true;
}

void frontcredits_input(void)
{
    _DK_credits_scroll_speed = 1;
    int speed;
    if ( _DK_lbKeyOn[KC_DOWN] )
    {
        speed = _DK_frontend_font[1][32].SHeight;
        _DK_credits_scroll_speed = speed;
    } else
    if ((_DK_lbKeyOn[KC_UP]) && (_DK_credits_offset<=0))
    {
        speed = -_DK_frontend_font[1][32].SHeight;
        if ( speed <= _DK_credits_offset )
          speed = _DK_credits_offset;
        _DK_credits_scroll_speed = speed;
    }
}

void frontend_input(void)
{
    switch ( _DK_frontend_menu_state )
    {
      case 1:
        if ((_DK_lbDisplay.MMouseX*_DK_pixel_size != _DK_last_mouse_x) ||
             (_DK_GetMouseY() != _DK_last_mouse_y))
        {
          _DK_last_mouse_x = _DK_lbDisplay.MMouseX*_DK_pixel_size;
          _DK_last_mouse_y = _DK_GetMouseY();
          //time_last_played_demo = LbTimerClock();
          _DK_time_last_played_demo = timeGetTime();
        }
        _DK_get_gui_inputs(0);
        break;
      case 3:
        _DK_frontmap_input();
        break;
      case 6:
        _DK_get_gui_inputs(0);
        _DK_frontnet_start_input();
        break;
      case 12:
      case 29:
        if ( _DK_lbKeyOn[KC_SPACE] )
        {
          _DK_lbKeyOn[KC_SPACE] = 0;
          frontend_set_state(1);
        } else
        if ( _DK_lbKeyOn[KC_RETURN] )
        {
            _DK_lbKeyOn[KC_RETURN] = 0;
            frontend_set_state(1);
        } else
        if ( _DK_lbKeyOn[KC_ESCAPE] )
        {
            _DK_lbKeyOn[KC_ESCAPE] = 0;
            frontend_set_state(1);
        } else
        if ( _DK_left_button_clicked )
        {
            _DK_left_button_clicked = 0;
            frontend_set_state(1);
        }
        break;
      case 13:
        if (!end_input())
        {
          if ( _DK_credits_end )
            frontend_set_state(1);
        }
        frontcredits_input();
        break;
      case 18:
        _DK_get_gui_inputs(0);
         _DK_frontend_high_score_table_input();
        break;
      case 19:
        _DK_fronttorture_input();
        break;
      case 24:
        _DK_frontnetmap_input();
        break;
      case 26:
        if ( !_DK_defining_a_key )
          _DK_get_gui_inputs(0);
        else
          define_key_input();
        break;
      default:
        _DK_get_gui_inputs(0);
        break;
    } // end switch
}

void intro(void)
{
    char text[255];
    sprintf(text, "%s/ldata/%s", _DK_datafiles_path, "intromix.smk");
    LbSyncLog("Playing video \"%s\"\n",text);
    play_smacker_file(text, 1);
}

void outro(void)
{
    char text[255];
    sprintf(text, "%s/ldata/%s", _DK_datafiles_path, "outromix.smk");
    LbSyncLog("Playing video \"%s\"\n",text);
    play_smacker_file(text, 17);
}

void frontend_copy_background(void)
{
    unsigned char *wscrn = _DK_lbDisplay.WScreen;
    unsigned char *sscrn = _DK_frontend_background;
    int qwidth = _DK_lbDisplay.PhysicalScreenWidth >> 2;
    int i;
    for ( i=0; i<_DK_lbDisplay.PhysicalScreenHeight; i++ )
    {
        memcpy(wscrn, sscrn, 4*qwidth);
        memcpy(wscrn+4*qwidth, sscrn+4*qwidth, _DK_lbDisplay.PhysicalScreenWidth & 0x03);
        sscrn += 640;
        wscrn += _DK_lbDisplay.GraphicsScreenWidth;
    }
}

int __cdecl frontstory_draw()
{
  frontend_copy_background();
  _DK_LbTextSetWindow(70, 70, 500, 340);
  _DK_lbFontPtr = _DK_frontstory_font;
  _DK_lbDisplay.DrawFlags = 256;
  _DK_LbTextDraw(0,0,_DK_strings[_DK_frontstory_text_no]);
}

void draw_defining_a_key_box(void)
{
    _DK_draw_text_box(_DK_strings[470]);
}

struct TbBirthday {
    unsigned char day;
    unsigned char month;
    char *name;
    };

const struct TbBirthday team_birthdays[] = {
    {13,1,"Mark Healey"},
    {21,3,"Jonty Barnes"},
    {3,5,"Simon Carter"},
    {5,5,"Peter Molyneux"},
    {13,11,"Alex Peters"},
    {1,12,"Dene Carter"},
    {25,5,"Tomasz Lis"},
    {0,0,NULL},
    };

char *get_team_birthday()
{
  struct TbDate curr_date;
  LbDate(&curr_date);
  int i;
  for (i=0;team_birthdays[i].day!=0;i++)
  {
      if ((team_birthdays[i].day==curr_date.Day) &&
          (team_birthdays[i].month==curr_date.Month))
      {
          return team_birthdays[i].name;
      }
  }
  return NULL;
}

int __cdecl frontbirthday_draw()
{
  frontend_copy_background();
  _DK_LbTextSetWindow(70, 70, 500, 340);
  _DK_lbFontPtr = _DK_frontstory_font;
  _DK_lbDisplay.DrawFlags = 256;
  char *name=get_team_birthday();
  if ( name != NULL )
  {
      unsigned short line_pos = 0;
      if ( _DK_lbFontPtr != NULL )
          line_pos = _DK_lbFontPtr[1].SHeight;
      _DK_LbTextDraw(0, 170-line_pos, _DK_strings[885]);
      _DK_LbTextDraw(0, 170, name);
  } else
  {
      frontend_set_state(11);
  }
}

short frontend_draw(void)
{
    short result=1;
    switch (_DK_frontend_menu_state)
    {
    case 11:
        intro();
        return 0;
    case 14:
        _DK_demo();
        return 0;
    case 21:
        outro();
        return 0;
    }

    if ( _DK_LbScreenLock() != 1 )
        return result;

    switch ( _DK_frontend_menu_state )
    {
    case 1:
    case 2:
    case 4:
    case 5:
    case 15:
    case 16:
    case 17:
    case 18:
    case 20:
    case 27:
        _DK_draw_gui();
        break;
    case 3:
        _DK_frontmap_draw();
        break;
    case 6:
        _DK_draw_gui();
        break;
    case 12:
        frontstory_draw();
        break;
    case 13:
        _DK_frontcredits_draw();
        break;
    case 19:
        _DK_fronttorture_draw();
        break;
    case 24:
        _DK_frontnetmap_draw();
        break;
    case 26:
        _DK_draw_gui();
        if ( _DK_defining_a_key )
            draw_defining_a_key_box();
        break;
    case 29:
        frontbirthday_draw();
        break;
    default:
        break;
    }
    _DK_LbScreenUnlock();
    return result;
}

void frontend_update(short *finish_menu)
{
    switch ( _DK_frontend_menu_state )
    {
      case 1:
        _DK_frontend_button_info[8].field_2 = (_DK_continue_game_option_available?1:3);
        //this uses original timing function for compatibility with frontend_set_state()
        //if ( abs(LbTimerClock()-time_last_played_demo) > 30000 )
        if ( abs(timeGetTime()-_DK_time_last_played_demo) > 30000 )
          frontend_set_state(14);
        break;
      case 2:
        load_game_update();
        break;
      case 3:
        *finish_menu = _DK_frontmap_update();
        break;
      case 4:
        _DK_frontnet_service_update();
        break;
      case 5:
        _DK_frontnet_session_update();
        break;
      case 6:
        _DK_frontnet_start_update();
        break;
      case 7:
      case 8:
      case 10:
      case 25:
        *finish_menu = 1;
        break;
      case 9:
        *finish_menu = 1;
        _DK_exit_keeper = 1;
        break;
      case 13:
        if ( !(_DK_game.flags_cd & 0x10) )
          _DK_PlayRedbookTrack(7);
        break;
      case 15:
        _DK_frontnet_modem_update();
        break;
      case 16:
        _DK_frontnet_serial_update();
        break;
      case 17:
        _DK_frontstats_update(); // rewrite with frontstats_set_timer
        break;
      case 19:
        _DK_fronttorture_update();
        break;
      case 24:
        *finish_menu = _DK_frontnetmap_update();
        break;
      case 27:
        if ( !(_DK_game.flags_cd & 0x10) )
          _DK_PlayRedbookTrack(3);
        break;
      default:
        break;
    }
}

void faststartup_saved_packet_game(void)
{
    int scrmode=(-((unsigned int)(_DK_settings.field_B - 1) < 1) & 0xFFF4) + 13;
    if (_DK_setup_screen_mode(scrmode))
        return;
    if ( _DK_settings.field_B != 13 )
    {
        _DK_FatalError = 1;
        _DK_exit_keeper = 1;
        return;
    }
    if ( !_DK_setup_screen_mode(1) )
    {
        _DK_FatalError = 1;
        _DK_exit_keeper = 1;
        return;
    }
    _DK_settings.field_B = 1;
    _DK_save_settings();
    _DK_startup_saved_packet_game();
    _DK_game.players[_DK_my_player_number].field_6 &= 0xFDu;
}

void faststartup_network_game(void)
{
    int scrmode=(-((unsigned int)(_DK_settings.field_B - 1) < 1) & 0xFFF4) + 13;
    if ( !_DK_setup_screen_mode(scrmode) )
    {
      if ( _DK_settings.field_B != 13 )
      {
        _DK_FatalError = 1;
        _DK_exit_keeper = 1;
        return;
      }
      if ( !_DK_setup_screen_mode(1) )
      {
        _DK_FatalError = 1;
        _DK_exit_keeper = 1;
        return;
      }
      _DK_settings.field_B = 1;
      _DK_save_settings();
    }
    _DK_my_player_number = 0;
    _DK_game.flagfield_14EA4A = 2;
    _DK_game.players[_DK_my_player_number].field_2C = 1;
    _DK_game.numfield_14A83D = _DK_game.numfield_16;
    _DK_startup_network_game();
    _DK_game.players[_DK_my_player_number].field_6 &= 0xFDu;
}

short is_bonus_level(long levidx)
{
    if ((levidx>=100)&&(levidx<106))
        return true;
    return false;
}

int setup_old_network_service()
{
    return _DK_setup_network_service(_DK_net_service_index_selected);
}

int get_startup_menu_state(void)
{
  static const char *func_name="get_startup_menu_state";
  if ( _DK_game.flags_cd & 0x40 )
  {
    if (_DK_game.is_full_moon)
    {
        LbSyncLog("%s: Full moon state selected\n",func_name);
        return 12;
    } else
    if ( get_team_birthday() != NULL )
    {
        LbSyncLog("%s: Birthday state selected\n",func_name);
        return 29;
    } else
    {
        LbSyncLog("%s: Standard startup state selected\n",func_name);
        return 1;
    }
  } else
  {
    LbSyncLog("%s: Player-based state selected\n",func_name);
    struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
    if ( !(_DK_game.numfield_A & 1) )
    {
      if ( (player->field_6 & 2) || (!player->field_29) )
      {
        return 3;
      } else
      if ( _DK_game.flags_cd & 1 )
      {
        _DK_game.flags_cd &= 0xFEu;
        return 1;
      } else
      if ( player->field_29 == 1 )
      {
          if ( _DK_game.level_number <= 20 )
          {
            if ( player->field_3 & 0x10 )
            {
                player->field_3 &= 0xEF;
                return 19;
            } else
            if ( is_bonus_level(_DK_game.numfield_14A83D) )
            {
                return 3;
            } else
            {
                return 17;
            }
          } else
          if ( is_bonus_level(_DK_game.numfield_14A83D) )
          {
              return 3;
          } else
          {
              return 21;
          }
      } else
      if ( player->field_29 == 3 )
      {
          return 17;
      } else
      if ( (_DK_game.numfield_14A83D < 50) || (_DK_game.numfield_14A83D > 79) )
      {
          return 3;
      } else
      {
          return 1;
      }
    } else
    {
      if ( !(player->field_3 & 0x10) )
      {
        if ( !(player->field_6 & 2) )
        {
          return 17;
        } else
        if ( setup_old_network_service() )
        {
          return 5;
        } else
        {
          return 1;
        }
      } else
      {
        player->field_3 &= 0xEF;
        return 19;
      }
    }
  }
  error(func_name, 978, "Unresolved menu state");
  return 1;
}
void wait_at_frontend(void)
{
  static const char *func_name="wait_at_frontend";

  //Moon phase calculation
  calculate_moon_phase(false);
  if ( _DK_game.flags_cd & 1 )
    _DK_game.field_149E81 = 0;
  _DK_game.numfield_15 = -1;

  initialise_load_game_slots();

  if ( _DK_game.field_149E81 && !_DK_game.numfield_149F47 )
  {
    faststartup_saved_packet_game();
    return;
  }

  if ( _DK_game.numfield_C & 2 )
  {
    faststartup_network_game();
    return;
  }

  if ( !_DK_setup_screen_mode_minimal(13) )
  {
    _DK_FatalError = 1;
    _DK_exit_keeper = 1;
    return;
  }
  _DK_LbScreenClear(0);
  _DK_LbScreenSwap();
  if ( !_DK_frontend_load_data() )
  {
    error(func_name, 738, "Unable to load frontend data");
    _DK_exit_keeper = 1;
    return;
  }
  memset(_DK_scratch, 0, 0x300u);
  _DK_LbPaletteSet(_DK_scratch);
  frontend_set_state(get_startup_menu_state());

  short finish_menu = 0;
  _DK_game.flags_cd &= 0xBFu;

  // Begin the frontend loop
  long last_loop_time = LbTimerClock();
  do
  {
    if ( (!_DK_LbWindowsControl()) && ((_DK_game.numfield_A&1)==0) )
    {
      _DK_exit_keeper = 1;
      LbSyncLog("%s: Normal exit condition 1 invoked\n",func_name);
      break;
    }
//LbSyncLog("update_mouse\n");
    update_mouse();
    _DK_old_mouse_over_button = _DK_frontend_mouse_over_button;
    _DK_frontend_mouse_over_button = 0;

//LbSyncLog("frontend_input\n");
    frontend_input();
    if ( _DK_exit_keeper )
    {
      LbSyncLog("%s: Normal exit condition 2 invoked\n",func_name);
      break; // end while
    }

//LbSyncLog("frontend_update\n");
    frontend_update(&finish_menu);
    if ( _DK_exit_keeper )
    {
      LbSyncLog("%s: Normal exit condition 3 invoked\n",func_name);
      break; // end while
    }

    if ( !finish_menu && _DK_LbIsActive() )
    {
//LbSyncLog("frontend_draw\n");
      frontend_draw();
      _DK_LbScreenSwap();
    }

    if ( !_DK_SoundDisabled )
    {
      _DK_process_3d_sounds();
      _DK_process_sound_heap();
      _DK_MonitorStreamedSoundTrack();
    }

    if ( _DK_fade_palette_in )
    {
//LbSyncLog("fade_in\n");
      fade_in();
      _DK_fade_palette_in = 0;
    } else
    {
      LbSleepUntil(last_loop_time + 30);
    }
    last_loop_time = LbTimerClock();
  } while (!finish_menu);

  _DK_LbPaletteFade(0, 8, Lb_PALETTE_FADE_CLOSED);
  _DK_LbScreenClear(0);
  _DK_LbScreenSwap();
  short prev_state = _DK_frontend_menu_state;
  frontend_set_state(0);
  if ( _DK_exit_keeper )
  {
    _DK_game.players[_DK_my_player_number].field_6 &= 0xFDu;
    return;
  }
  int scrmode=(-((unsigned int)(_DK_settings.field_B - 1) < 1) & 0xFFF4) + 13;
  if ( !_DK_setup_screen_mode(scrmode) )
  {
    if ((_DK_settings.field_B==13) && _DK_setup_screen_mode(1))
    {
      _DK_settings.field_B = 1;
      _DK_save_settings();
    } else
    {
      _DK_FatalError = 1;
      _DK_exit_keeper = 1;
      return;
    }
  }

  display_loading_screen();
  short flgmem;
  switch ( prev_state )
  {
      case 7:
        _DK_my_player_number = 0;
        _DK_game.flagfield_14EA4A = 2;
        _DK_game.numfield_A &= 0xFEu;
        _DK_game.players[_DK_my_player_number].field_2C = 1;
        _DK_game.numfield_14A83D = _DK_game.numfield_16;
        _DK_startup_network_game();
        break;
      case 8:
        _DK_game.numfield_14A83D = _DK_game.numfield_16;
        _DK_game.numfield_A |= 1;
        _DK_game.flagfield_14EA4A = 5;
        _DK_game.players[_DK_my_player_number].field_2C = 1;
        _DK_startup_network_game();
        break;
      case 10:
        flgmem = _DK_game.numfield_15;
        _DK_game.numfield_A &= 0xFEu;
        if ( _DK_game.numfield_15 == -2 )
        {
          error(func_name, 1012, "Why are we here");
          _DK_game.numfield_15 = flgmem;
        } else
        {
          _DK_LbScreenClear(0);
          _DK_LbScreenSwap();
          _DK_load_game(_DK_game.numfield_15);
          _DK_game.numfield_15 = flgmem;
        }
        break;
      case 25:
        _DK_game.flags_cd |= 1;
        _DK_startup_saved_packet_game();
        break;
  }
  _DK_game.players[_DK_my_player_number].field_6 &= 0xFDu;
}

void game_loop(void)
{
  unsigned long random_seed;
  unsigned long playtime;
  playtime = 0;
  random_seed = 0;
  while ( !_DK_exit_keeper )
  {
    update_mouse();
    wait_at_frontend();
//    _DK_wait_at_frontend();
    if ( _DK_exit_keeper )
      return;
    struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
    if ( _DK_game.flagfield_14EA4A == 2 )
    {
      if ( _DK_game.numfield_15 == -1 )
      {
        _DK_set_player_instance(player, 11, 0);
      } else
      {
        _DK_game.numfield_15 = -1;
        _DK_game.numfield_C &= 0xFE;
      }
    }
    unsigned long starttime;
    unsigned long endtime;
    starttime = LbTimerClock();
    _DK_game.dungeon[_DK_my_player_number].time1 = timeGetTime();//starttime;
    _DK_game.dungeon[_DK_my_player_number].time2 = timeGetTime();//starttime;
    _DK_LbScreenClear(0);
    _DK_LbScreenSwap();
    keeper_gameplay_loop();
    _DK_LbMouseChangeSpriteAndHotspot(0, 0, 0);
    _DK_LbScreenClear(0);
    _DK_LbScreenSwap();
    _DK_StopRedbookTrack();
    _DK_StopMusic();
    _DK_frontstats_initialise();
    _DK_delete_all_structures();
    _DK_clear_mapwho();
    endtime = LbTimerClock();
    _DK_quit_game = 0;
    if ( _DK_game.numfield_C & 2 )
        _DK_exit_keeper=true;
    playtime += endtime-starttime;
    LbSyncLog("Play time is %d seconds\n",playtime>>10);
    random_seed += _DK_game.seedchk_random_used;
    _DK_reset_eye_lenses();
    if ( _DK_game.packet_fopened )
    {
      _DK_LbFileClose(_DK_game.packet_fhandle);
      _DK_game.packet_fopened = 0;
      _DK_game.packet_fhandle = 0;
    }
  } // end while
//     _DK_game_loop();
}

short reset_game(void)
{
  _DK_IsRunningUnmark();
  _DK_LbMouseSuspend();
  LbIKeyboardClose();
  _DK_LbScreenReset();
  _DK_LbDataFreeAll(_DK_game_load_files);
  _DK_LbMemoryFree(_DK_strings_data);
  _DK_strings_data = NULL;
  _DK_FreeAudio();
  return _DK_LbMemoryReset();
}

short process_command_line(unsigned short argc, char *argv[])
{
  char fullpath[CMDLN_MAXLEN+1];
  strncpy(fullpath, argv[0], CMDLN_MAXLEN);

  sprintf( _DK_keeper_runtime_directory, fullpath);
  char *endpos=strrchr( _DK_keeper_runtime_directory, '\\');
  if (endpos!=NULL)
      *endpos='\0';

  _DK_SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain
  _DK_LbErrorLogSetup(0, 0, 1);

  _DK_game.numfield_149F42 = -1;
  _DK_game.numfield_149F46 = 0;
  _DK_game.packet_checksum = 1;
  _DK_game.numfield_1503A2 = -1;
  _DK_game.flags_font &= 0xFEu;
  _DK_game.numfield_149F47 = 0;
  _DK_game.numfield_16 = 0;
  _DK_game.num_fps = 20;
  _DK_game.flags_cd = (_DK_game.flags_cd & 0xFE) | 0x40;

  long level_num = -1;

  unsigned short narg;
  narg = 1;
  while ( narg < argc )
  {
      char *par;
      par = argv[narg];
      if ( (par==NULL) || ((par[0] != '-') && (par[0] != '/')) )
          return -1;
      char parstr[CMDLN_MAXLEN+1];
      char pr2str[CMDLN_MAXLEN+1];
      strncpy(parstr, par+1, CMDLN_MAXLEN);
      if ( narg+1 < argc )
        strncpy(pr2str,  argv[narg+1], CMDLN_MAXLEN);
      else
        pr2str[0]='\0';

      if ( stricmp(parstr, "nointro") == 0 )
      {
        _DK_game.no_intro = 1;
      } else
      if ( stricmp(parstr, "nocd") == 0 )
      {
          _DK_game.flags_cd |= 0x10;
      } else
      if ( stricmp(parstr, "1player") == 0 )
      {
          _DK_game.one_player = 1;
      } else
      if ( (stricmp(parstr, "s") == 0) || (stricmp(parstr, "nosound") == 0) )
      {
          _DK_SoundDisabled = 1;
      } else
      if ( stricmp(parstr, "fps") == 0 )
      {
          narg++;
          _DK_game.num_fps = atoi(pr2str);
      } else
      if ( stricmp(parstr, "usersfont") == 0 )
      {
          _DK_game.flags_font |= 0x40;
      } else
      if ( stricmp(parstr,"level") == 0 )
      {
        _DK_game.numfield_C |= 2;
        level_num = atoi(pr2str);
        narg++;
      } else
      if ( stricmp(parstr,"q") == 0 )
      {
         _DK_game.numfield_C |= 2;
      } else
      if ( stricmp(parstr,"columnconvert") == 0 )
      {
         _DK_game.numfield_C |= 8;
      } else
      if ( stricmp(parstr,"lightconvert") == 0 )
      {
         _DK_game.numfield_C |= 0x10;
      } else
      if ( stricmp(parstr, "alex") == 0 )
      {
          _DK_game.flags_font |= 0x20;
      }
      narg++;
  }

  if ( level_num == -1 )
    level_num = 1;
  _DK_game.numfield_16 = level_num;
  _DK_game.level_number = level_num;
  if ( (_DK_game.numfield_C & 2) == 0 )
    _DK_game.level_number = 1;
  _DK_my_player_number = 0;
  return 0;
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
  static const char *func_name="LbBullfrogMain";
  short retval=0;
  LbErrorLogSetup("/", "keeperfx.log", 5);
  strcpy(window_class_name, PROGRAM_NAME);
  LbSetIcon(110);
  srand(LbTimerClock());

  retval=process_command_line(argc,argv);

  if ( retval > -1 )
  {
      retval=setup_game();
      if ( retval )
        game_loop();
      reset_game();
/*
          MessageBox(NULL, "Finished OK.",
             "Dungeon Keeper Fan eXpansion", MB_OK | MB_ICONINFORMATION);
*/
      if ( !retval )
      {
        static const char *msg_text="Setting up game failed.\n";
        error(func_name, 2, msg_text);
        MessageBox(NULL, msg_text, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
      } else
      {
        LbSyncLog("%s finished properly.\n",func_name);
      }
  } else
  {
      static const char *msg_text="Command line parameters analysis failed.\n";
      error(func_name, 1, msg_text);
      MessageBox(NULL, msg_text, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
  }
  LbErrorLogClose();
  return 0;
}

void get_cmdln_args(unsigned short &argc, char *argv[])
{
  char *ptr;
  const char *cmndln_orig;
  cmndln_orig = GetCommandLineA();
  strncpy(cmndline, cmndln_orig, CMDLN_MAXLEN);
  ptr = cmndline;
  bf_argc = 0;
  while ( *ptr != '\0' )
  {
      if ((*ptr == '\t') || (*ptr == ' '))
      {
          ptr++;
          continue;
      }
      if (*ptr == '\"')
      {
          ptr++;
          bf_argv[bf_argc] = ptr;
          bf_argc++;
          while (*ptr != '\0')
          {
            if (*ptr == '\"')
            {
              *ptr++ = '\0';
              break;
            }
            ptr++;
          }
      } else
      {
          bf_argv[bf_argc] = ptr;
          bf_argc++;
          while (*ptr != '\0')
          {
            if ((*ptr == '\t') || (*ptr == ' '))
            {
              *ptr++ = '\0';
              break;
            }
            ptr++;
          }
      }
  }
}

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
  static const char *func_name="WinMain";
  _DK_hInstance = hThisInstance;
  _DK_lpDDC = NULL;

  get_cmdln_args(bf_argc, bf_argv);

  if (sizeof(struct Game)!=SIZEOF_Game)
  {
      static const char *msg_text="Bad compilation - struct Game has wrong size!\n";
      error(func_name, 1, msg_text);
      MessageBox(NULL, msg_text, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
      return 0;
  }

/*  MessageBox(NULL, "KeeperFX is a Dungeon Keeper mod. It requires original product.",
     "Dungeon Keeper Fan eXpansion", MB_OK | MB_ICONINFORMATION);*/

  LbBullfrogMain(bf_argc, bf_argv);

//  LbFileSaveAt("!tmp_file", &_DK_game, sizeof(struct Game));

  if ( _DK_lpDDC != NULL )
  {
    TDDrawBaseVTable *vtable=_DK_lpDDC->vtable;
    //Note: __thiscall is for functions with no arguments same as __fastcall,
    // so we're using __fastcall and passing the "this" pointer manually
    vtable->reset_screen(_DK_lpDDC);
    if ( _DK_lpDDC != NULL )
    {
      vtable=_DK_lpDDC->vtable;
      // Destructor requires a flag passed in AX
      asm volatile ("  movl $1,%eax\n");
      vtable->dt(_DK_lpDDC);
    }
  }
    return 0;
}
