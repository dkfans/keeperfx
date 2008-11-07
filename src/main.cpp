
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
#include "bflib_sndlib.h"

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

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 1 byte long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_byte(unsigned char *flags,short mask,short value)
{
  if (value) value=mask;
  *flags = (*flags ^ (unsigned char)value) & mask ^ *flags;
}

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 4 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_dword(unsigned long *flags,unsigned long mask,unsigned long value)
{
  if (value) value=mask;
  *flags = (*flags ^ (unsigned long)value) & mask ^ *flags;
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

short thing_is_special(struct Thing *thing)
{
  return (thing->field_1F==1) && (_DK_object_to_special[thing->field_1A]);
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

short sound_emitter_in_use(long emidx)
{
  return (emidx!=0) && (_DK_emitter[emidx].flags & 1);
}

void play_non_3d_sample(long sample_idx)
{
  static const char *func_name="play_non_3d_sample";
  if ( _DK_SoundDisabled )
    return;
  if ( GetCurrentSoundMasterVolume() <= 0 )
    return;
  if (_DK_Non3DEmitter!=0)
    if ( sound_emitter_in_use(_DK_Non3DEmitter) == 0 )
    {
      error(func_name, 263, "Non 3d Emitter has been deleted!");
      _DK_Non3DEmitter = 0;
    }
  if (_DK_Non3DEmitter!=0)
  {
    _DK_S3DAddSampleToEmitterPri(_DK_Non3DEmitter, sample_idx, 0, 100, 256, 0, 3, 8, 2147483646);
  } else
  {
    _DK_Non3DEmitter = _DK_S3DCreateSoundEmitterPri(0, 0, 0, sample_idx, 0, 100, 256, 0, 8, 2147483646);
  }
}

short setup_network_service(int srvidx)
{
//  return _DK_setup_network_service(srvidx);
  struct SerialInitData *init_data;
  long maxplayrs;

  switch (srvidx)
  {
  case 0:
      maxplayrs = 2;
      init_data = &_DK_net_serial_data;
      _DK_game.flags_font |= 0x10;
      LbSyncLog("Initializing %d-players serial network.\n",maxplayrs);
      break;
  case 1:
      maxplayrs = 2;
      init_data = &_DK_net_modem_data;
      _DK_game.flags_font |= 0x10;
      LbSyncLog("Initializing %d-players modem network.\n",maxplayrs);
      break;
  case 2:
  default:
      maxplayrs = 4;
      init_data = NULL;
      _DK_game.flags_font &= 0xEFu;
      LbSyncLog("Initializing %d-players type %d network.\n",maxplayrs,srvidx);
      break;
  }

  memset(&_DK_net_player_info, 0, sizeof(struct TbNetworkPlayerInfo));
  if ( _DK_LbNetwork_Init(srvidx, _DK_net_guid, maxplayrs, &_DK_net_screen_packet, 0xCu, &_DK_net_player_info, init_data) )
  {
    if (srvidx != 0)
      _DK_process_network_error(-800);
    return 0;
  }
  _DK_net_service_index_selected = srvidx;
  if ( _DK_game.flags_font & 0x10 )
    ;//rndseed_nullsub();
  frontend_set_state(5);
  return 1;
}

void frontnet_service_update(void)
{
  _DK_frontnet_service_update();
}

void frontnet_session_update(void)
{
  _DK_frontnet_session_update();
}

void frontnet_modem_update(void)
{
  _DK_frontnet_modem_update();
}

void frontnet_serial_update(void)
{
  _DK_frontnet_serial_update();
}

void frontnet_start_update(void)
{
  _DK_frontnet_start_update();
}

/*
 * Displays 'legal' screens, intro and initializes basic game data.
 * If true is returned, then all files needed for startup were loaded,
 * and there should be the loading screen visible.
 * @return Returns true on success, false on error which makes the 
 *   gameplay impossible (usually files loading failure).
 * @note The current screen resolution at end of this function may vary.
 */
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
  // Intro problems shouldn't force the game to quit,
  // so we're re-setting the result flag
  result = 1;

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
        SetRedbookVolume(_DK_settings.redbook_volume);
        SetSoundMasterVolume(_DK_settings.sound_volume);
        SetMusicMasterVolume(_DK_settings.sound_volume);
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

void input_eastegg(void)
{
  static const char *func_name="input_eastegg";
  //_DK_input_eastegg();return;
  // Maintain the FECKOFF cheat
  if ( _DK_lbKeyOn[KC_LSHIFT] )
  {
    switch (_DK_game.eastegg01_cntr)
    {
    case 0:
      if ( _DK_lbKeyOn[KC_F] )
        _DK_game.eastegg01_cntr++;
      break;
    case 1:
      if ( _DK_lbKeyOn[KC_E] )
        _DK_game.eastegg01_cntr++;
      break;
    case 2:
      if ( _DK_lbKeyOn[KC_C] )
        _DK_game.eastegg01_cntr++;
      break;
    case 3:
      if ( _DK_lbKeyOn[KC_K] )
        _DK_game.eastegg01_cntr++;
      break;
    case 4:
      if ( _DK_lbKeyOn[KC_O] )
        _DK_game.eastegg01_cntr++;
      break;
    case 5:
    case 6:
      if ( _DK_lbKeyOn[KC_F] )
        _DK_game.eastegg01_cntr++;
      break;
    }
  } else
  {
    _DK_game.eastegg01_cntr = 0;
  }
  // Maintain the JLW cheat
  if ( _DK_game.flags_font & 0x20 )
  {
    if ( _DK_lbKeyOn[KC_LSHIFT] && _DK_lbKeyOn[KC_RSHIFT] )
    {
      switch (_DK_game.eastegg02_cntr)
      {
      case 0:
        if ( _DK_lbKeyOn[KC_J] )
        {
          play_non_3d_sample(159);
          _DK_game.eastegg02_cntr++;
        }
        break;
      case 1:
        if ( _DK_lbKeyOn[KC_L] )
        {
          play_non_3d_sample(159);
          _DK_game.eastegg02_cntr++;
        }
        break;
      case 2:
        if ( _DK_lbKeyOn[KC_W] )
        {
          play_non_3d_sample(159);
          _DK_game.eastegg02_cntr++;
        }
        break;
      }
    } else
    {
      _DK_game.eastegg02_cntr = 0;
    }
  }
  // Maintain the SKEKSIS cheat
  if ( _DK_lbKeyOn[KC_LSHIFT] )
  {
    switch (_DK_eastegg03_cntr)
    {
    case 0:
    case 4:
      if ( _DK_lbInkey==KC_S )
      {
        _DK_eastegg03_cntr++;
       _DK_lbInkey=0;
      }
      break;
    case 1:
    case 3:
      if ( _DK_lbInkey==KC_K )
      {
        _DK_eastegg03_cntr++;
       _DK_lbInkey=0;
      }
      break;
    case 2:
      if ( _DK_lbInkey==KC_E )
      {
        _DK_eastegg03_cntr++;
       _DK_lbInkey=0;
      }
      break;
    case 5:
      if ( _DK_lbInkey==KC_I )
      {
        _DK_eastegg03_cntr++;
       _DK_lbInkey=0;
      }
      break;
    case 6:
      if ( _DK_lbInkey==KC_S )
      {
        _DK_eastegg03_cntr++;
        _DK_lbInkey=0;
        //'Your pants are definitely too tight'
        _DK_output_message(94, 0, 1);
      }
      break;
    }
  } else
  {
    _DK_eastegg03_cntr = 0;
  }
  if (_DK_lbInkey!=0)
  {
    _DK_eastegg03_cntr = 0;
  }
}

/*
 * Checks if the given screen point is over a gui menu.
 * @param x,y Screen coordinates to check.
 * @return Returns index of the menu, or -1 if there's no menu on this point.
 */
int point_is_over_gui_menu(long x, long y)
{
  int idx;
  int gidx = -1;
  const short gmax=8;//sizeof(_DK_active_menus)/sizeof(struct GuiMenu);
  for(idx=0;idx<gmax;idx++)
  {
    struct GuiMenu *gmnu;
    gmnu=&_DK_active_menus[idx];
    if (gmnu->field_1 != 2)
        continue;
    if ( gmnu->flgfield_1D == 0 )
        continue;
    short gx = gmnu->pos_x;
    if ((x >= gx) && (x < gx+gmnu->width))
    {
        short gy = gmnu->pos_y;
        if ((y >= gy) && (y < gy+gmnu->height))
            gidx=idx;
    }
  }
  return gidx;
}

int first_monopoly_menu(void)
{
  int idx;
  const short gmax=8;//sizeof(_DK_active_menus)/sizeof(struct GuiMenu);
  struct GuiMenu *gmnu;
  for (idx=0;idx<gmax;idx++)
  {
    gmnu=&_DK_active_menus[idx];
    if ((gmnu->field_1!=0) && (gmnu->flgfield_1E!=0))
        return idx;
  }
  return -1;
}

/*
 * Checks if mouse pointer is currently over a specific button.
 * @return Returns true it mouse is over the button.
 */
short check_if_mouse_is_over_button(struct GuiButton *gbtn)
{
  int mouse_x = _DK_lbDisplay.MMouseX*_DK_pixel_size;
  int mouse_y = _DK_GetMouseY();
  int x = gbtn->pos_x;
  int y = gbtn->pos_y;
  if ( (mouse_x >= x) && (mouse_x < x + gbtn->width)
    && (mouse_y >= y) && (mouse_y < y + gbtn->height)
    && (gbtn->field_0 & 4) )
    return true;
  return false;
}

inline void reset_scrolling_tooltip(void)
{
    _DK_tooltip_scroll_offset = 0;
    _DK_tooltip_scroll_timer = 25;
}

inline void do_sound_menu_click(void)
{
  play_non_3d_sample(61);
}

short setup_trap_tooltips(struct Coord3d *pos)
{
    struct Thing *thing=_DK_get_trap_for_position(_DK_map_to_slab[pos->x.stl.num],_DK_map_to_slab[pos->y.stl.num]);
    if (thing==NULL) return false;
    struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
    if ((thing->field_18==0) && (player->field_2B != thing->field_6))
      return false;
    if ( thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = thing;
    }
    if ((_DK_help_tip_time > 20) || (player->field_453 == 12))
    {
      _DK_tool_tip_box.field_0 = 1;
      int stridx=_DK_trap_data[thing->field_1A].field_C;
      sprintf(_DK_tool_tip_box.text,"%s",_DK_strings[stridx]);
      _DK_tool_tip_box.pos_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
      _DK_tool_tip_box.pos_y = _DK_GetMouseY()+86;
      _DK_tool_tip_box.field_809 = 4;
    } else
    {
      _DK_help_tip_time++;
    }
    return true;
}

short setup_object_tooltips(struct Coord3d *pos)
{
  char text[2048];
  struct Thing *thing = NULL;
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  thing = _DK_game.things_lookup[player->field_35];
  if ((thing!=NULL)&&(!thing_is_special(thing)))
      thing = NULL;
  if (thing==NULL)
    thing = _DK_get_special_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    if ( (void *)thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = thing;
    }
    int stridx=_DK_specials_text[_DK_object_to_special[thing->field_1A]];
    sprintf(_DK_tool_tip_box.text,"%s",_DK_strings[stridx]);
    _DK_tool_tip_box.pos_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
    _DK_tool_tip_box.field_0 = 1;
    _DK_tool_tip_box.field_809 = 5;
    _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
    return true;
  }
  thing = _DK_get_spellbook_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    if ( (void *)thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = (void *)thing;
    }
    int stridx = _DK_spell_data[_DK_object_to_magic[thing->field_1A]].field_D;
    if (stridx>0)
    {
      sprintf(_DK_tool_tip_box.text,"%s",_DK_strings[stridx]);
      _DK_tool_tip_box.field_0 = 1;
      _DK_tool_tip_box.pos_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
      _DK_tool_tip_box.field_809 = 5;
      _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
    }
    return 1;
  }
  thing = _DK_get_crate_at_position(pos->x.stl.num, pos->y.stl.num);
  if ( thing )
  {
    if ( (void *)thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = (void *)thing;
    }
    _DK_tool_tip_box.field_0 = 1;
    int objidx = thing->field_1A;
    int stridx;
    if ( _DK_workshop_object_class[objidx] == 8 )
      stridx = _DK_trap_data[_DK_object_to_door_or_trap[objidx]].field_C;
    else
      stridx = _DK_door_names[_DK_object_to_door_or_trap[objidx]];
    sprintf(_DK_tool_tip_box.text,"%s",_DK_strings[stridx]);
    _DK_tool_tip_box.pos_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
    _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
    _DK_tool_tip_box.field_809 = 5;
    return true;
  }
  if (!_DK_settings.tooltips_on)
    return false;
  thing = _DK_get_nearest_object_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    int objidx = thing->field_1A;
    int crtridx;
    if ( objidx == 49 )
    {
      sprintf(text,"%s",_DK_strings[545]);
    } else
    if (crtridx = _DK_objects[objidx].field_13)
    {
      int stridx=_DK_creature_data[crtridx].field_3;
      sprintf(text, "%s %s", _DK_strings[stridx], _DK_strings[609]);
    } else
    {
      return 0;
    }

    if ( (void *)thing != _DK_tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      _DK_tool_tip_box.target = (void *)thing;
    }
    if ( (_DK_help_tip_time>20) || (player->field_453==12))
    {
      _DK_tool_tip_box.field_0 = 1;
      strcpy(_DK_tool_tip_box.text, text);
      _DK_tool_tip_box.pos_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
      _DK_tool_tip_box.field_809 = 5;
      _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
    } else
    {
      _DK_help_tip_time++;
    }
    return true;
  }
  return false;
}

short setup_land_tooltips(struct Coord3d *pos)
{
  if (!_DK_settings.tooltips_on)
    return false;
  int slab_idx = _DK_map_to_slab[pos->x.stl.num] + 85*_DK_map_to_slab[pos->y.stl.num];
  int attridx = _DK_game.slabmap[slab_idx].field_0;
  int stridx = _DK_slab_attrs[attridx].field_0;
  if (stridx==201)
    return false;
  if ((void *)attridx != _DK_tool_tip_box.target)
  {
    _DK_help_tip_time = 0;
    _DK_tool_tip_box.target = (void *)attridx;
  }
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  if ((_DK_help_tip_time>20) || (player->field_453==12))
  {
    _DK_tool_tip_box.field_0 = 1;
    sprintf(_DK_tool_tip_box.text, "%s", _DK_strings[stridx]);
    _DK_tool_tip_box.pos_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
    _DK_tool_tip_box.field_809 = 2;
    _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86;
  } else
  {
    _DK_help_tip_time++;
  }
  return true;
}

short setup_room_tooltips(struct Coord3d *pos)
{
  if (!_DK_settings.tooltips_on)
    return false;
  int slab_idx = _DK_map_to_slab[pos->x.stl.num] + 85*_DK_map_to_slab[pos->y.stl.num];
  struct Room *room;
  room = &_DK_game.rooms[_DK_game.slabmap[slab_idx].field_3];
  if (room==NULL)
    return false;
  int stridx=_DK_room_data[room->field_A].field_13;
  if ( stridx == 201 )
    return false;
  if ( room != _DK_tool_tip_box.target )
  {
    _DK_help_tip_time = 0;
    _DK_tool_tip_box.target = room;
  }
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  int widener=0;
  if ( (_DK_help_tip_time>20) || (player->field_453==12) )
  {
      if ( room->field_A >= 2 )
      {
        if ( (room->field_A<=14) || (room->field_A==16) )
          widener = 0;
      }
      sprintf(_DK_tool_tip_box.text, "%s", _DK_strings[stridx]);
      _DK_tool_tip_box.field_0 = 1;
      _DK_tool_tip_box.pos_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
      _DK_tool_tip_box.pos_y = _DK_GetMouseY() + 86 + 20*widener;
      _DK_tool_tip_box.field_809 = 1;
  } else
  {
    _DK_help_tip_time++;
  }
  return true;
}

void toggle_gui_overlay_map(void)
{
  unsigned char mask;
  if ((_DK_game.numfield_C & 0x20) == 0)
    mask=0x20;
  else
    mask=0;
  _DK_game.numfield_C = (_DK_game.numfield_C & 0xDFu) | mask;
}

short screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos)
{
//TODO: put parts into subfunctions, check them with beta
  short result = 0;
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  int x,y;
  switch ( camera->field_6 )
  {
    case 1:
    case 2:
    case 5:
      //put in engine_point_to_map
      x = 0;
      y = 0;
      result = false;
      if ( (_DK_pointer_x >= 0) && (_DK_pointer_y >= 0)
        && (_DK_pointer_x < (player->field_444/_DK_pixel_size))
        && (_DK_pointer_y < (player->field_446/_DK_pixel_size)) )
      {
        int i = player->field_453;
        if ( ((i == 1) && (player->field_454)) || (i == 2) ||
           (i == 18) || (i == 16) || (i == 8) || (i == 23) )
        {
          x = ( _DK_top_pointed_at_x << 8) + _DK_top_pointed_at_frac_x;
          y = ( _DK_top_pointed_at_y << 8) + _DK_top_pointed_at_frac_y;
        } else
        {
          x = (_DK_block_pointed_at_x << 8) + _DK_pointed_at_frac_x;
          y = (_DK_block_pointed_at_y << 8) + _DK_pointed_at_frac_y;
        }

        if ( y < 0 )
          y = 0;
        else
        if ( y > 0xFEFFu )
          y = 0xFEFFu;
        if ( x < 0 )
          x = 0;
        else
        if ( x > 0xFEFFu )
          x = 0xFEFFu;
        result = 1;
      }
      break;
    case 3:
      // put in point_to_overhead_map
      x = 0;
      y = 0;
      result = false;
      if ((screen_x >= 150) && (screen_x < 490)
         && (screen_y >= 56) && (screen_y < 396))
      {
        x = 768 * (screen_x - 150) / 4 + 384;
        y = 768 * (screen_y - 56) / 4 + 384;
        result = ((x >= 0) && (x < 65536) && (y >= 0) && (y < 65536));
      }
      break;
    default:
      result = false;
      break;
  }
  if ( result )
  {
    mappos->x.val = x;
    mappos->y.val = y;
  }
  if ( mappos->x.val > 0xFEFFu )
    mappos->x.val = 0xFEFFu;
  if ( mappos->y.val > 0xFEFFu )
    mappos->y.val = 0xFEFFu;
  return result;
}

short is_key_pressed(long key, long kmodif)
{
  if ( (kmodif==-1) || (kmodif==_DK_key_modifiers) )
    return _DK_lbKeyOn[key];
  return 0;
}

short get_gui_inputs(short gameplay_on)
{
//  return _DK_get_gui_inputs(gameplay_on);
  static short doing_tooltip;
  static char over_slider_button=-1;
  doing_tooltip = 0;
  _DK_update_breed_activities();
  _DK_battle_creature_over = 0;
  _DK_gui_room_type_highlighted = -1;
  _DK_gui_door_type_highlighted = -1;
  _DK_gui_trap_type_highlighted = -1;
  _DK_gui_creature_type_highlighted = -1;
  if ( gameplay_on )
    _DK_maintain_my_battle_list();
  if ( !_DK_lbDisplay.MLeftButton )
  {
    const short gmax=86;//sizeof(_DK_active_buttons)/sizeof(struct GuiButton);
    _DK_drag_menu_x = -999;
    _DK_drag_menu_y = -999;
    int idx;
    for (idx=0;idx<gmax;idx++)
    {
      struct GuiButton *gbtn = &_DK_active_buttons[idx];
      if ((gbtn->field_0 & 1) && (gbtn->gbtype == 6))
          gbtn->field_1 = 0;
    }
  }

  int gidx;
  gidx = point_is_over_gui_menu(_DK_lbDisplay.MMouseX * _DK_pixel_size, _DK_GetMouseY());
  if ( gidx == -1 )
    _DK_busy_doing_gui = 0;
  else
    _DK_busy_doing_gui = 1;
  int fmmenu_idx = first_monopoly_menu();

  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  int gmbtn_idx = -1;
  struct GuiButton *gbtn;
  // Sweep through buttons
  for (gidx=0;gidx<86;gidx++)
  {
    gbtn = &_DK_active_buttons[gidx];
    if ((gbtn->field_0 & 1)==0)
      continue;
    if (!_DK_active_menus[gbtn->gmenu_idx].flgfield_1D)
      continue;
    Gf_Btn_Callback callback;
    callback = gbtn->field_17;
    if ( callback != NULL )
      callback(gbtn);
    if ( ((gbtn->field_1C & 0x40)!=0) || _DK_mouse_is_over_small_map(player->mouse_x,player->mouse_y) )
      continue;

    if ( check_if_mouse_is_over_button(gbtn) && (_DK_input_button==NULL)
      || (gbtn->gbtype==6) && (gbtn->field_1!=0) )
    {
      if ( (fmmenu_idx==-1) || (gbtn->gmenu_idx==fmmenu_idx) )
      {
        gmbtn_idx = gidx;
        gbtn->field_0 |= 0x10;
        _DK_busy_doing_gui = 1;
        callback = gbtn->field_F;
        if ( callback != NULL )
          callback(gbtn);
        if ( gbtn->gbtype == 6 )
          break;
        if ( gbtn->gbtype != Lb_SLIDER )
          over_slider_button = -1;
      } else
      {
        gbtn->field_0 &= 0xEFu;
      }
    } else
    {
      gbtn->field_0 &= 0xEFu;
    }
    if ( gbtn->gbtype == Lb_SLIDER )
    {
      int mouse_x = _DK_lbDisplay.MMouseX*_DK_pixel_size;
      int btnsize;
      btnsize = gbtn->field_1D + ((gbtn->slide_val)*(gbtn->width-64) >> 8);
      if ((mouse_x>(btnsize+22)) && (mouse_x<=(btnsize+44)))
      {
        int mouse_y = _DK_GetMouseY();
        if ((mouse_y>gbtn->pos_y) && (mouse_y<=(gbtn->pos_y+gbtn->height)))
        {
          if ( _DK_left_button_clicked )
          {
            _DK_left_button_clicked = 0;
            over_slider_button = gidx;
            do_sound_menu_click();
          }
        }
      }
    }
  }  // end for

  short result = 0;
  if (_DK_input_button!=NULL)
  {
    _DK_busy_doing_gui = 1;
    if (_DK_get_button_area_input(_DK_input_button,_DK_input_button->id_num) != 0)
        result = 1;
  }
  if ((over_slider_button!=-1) && (_DK_left_button_released))
  {
      _DK_left_button_released = 0;
      if (gmbtn_idx!=-1)
        _DK_active_buttons[gmbtn_idx].field_1 = 0;
      over_slider_button = -1;
      do_sound_menu_click();
  }

  if (gmbtn_idx!=-1)
  {
    gbtn = &_DK_active_buttons[gmbtn_idx];
    if ((_DK_active_menus[gbtn->gmenu_idx].field_1 == 2) && ((gbtn->field_1C & 0x80)==0))
    {
      if (_DK_tool_tip_box.gbutton == gbtn)
      {
        if ((_DK_tool_tip_time>10) || (player->field_453==12))
        {
          _DK_busy_doing_gui = 1;
          if ( gbtn->field_13 != _DK_gui_area_text )
            _DK_setup_gui_tooltip(gbtn);
        } else
        {
          _DK_tool_tip_time++;
          doing_tooltip = 1;
          _DK_busy_doing_gui = 1;
        }
      } else
      {
        _DK_tool_tip_time = 0;
        _DK_tool_tip_box.gbutton = gbtn;
        _DK_tool_tip_box.pos_x = _DK_lbDisplay.MMouseX*_DK_pixel_size;
        _DK_tool_tip_box.pos_y = _DK_GetMouseY()+86;
        _DK_tool_tip_box.field_809 = 0;
      }
    } else
    {
      _DK_tool_tip_time = 0;
      _DK_tool_tip_box.gbutton = NULL;
    }
  } else
  {
    _DK_tool_tip_time = 0;
    _DK_tool_tip_box.gbutton = NULL;
  }

  if ( over_slider_button != -1 )
  {
    gbtn = &_DK_active_buttons[over_slider_button];
    int mouse_x = _DK_lbDisplay.MMouseX*_DK_pixel_size;
    gbtn->field_1 = 1;
    int slide_start,slide_end;
    slide_start = gbtn->pos_x+32;
    slide_end = gbtn->pos_x+gbtn->width-32;
    if (mouse_x > slide_start)
    {
      if (mouse_x < slide_end)
      {
        if ( gbtn->width>64 )
            gbtn->slide_val = ((mouse_x-slide_start) << 8) / (gbtn->width-64);
        else
            gbtn->slide_val = ((mouse_x-gbtn->pos_x) << 8) / (gbtn->width+1);
      } else
      {
        gbtn->slide_val = 255;
      }
    } else
    {
      gbtn->slide_val = 0;
    }
    *gbtn->field_33 = (gbtn->slide_val) * (gbtn->field_2D+1) >> 8;
    Gf_Btn_Callback callback;
    callback = gbtn->click_event;
    if ( callback != NULL )
      callback(gbtn);
    return 1;
  }

  if ( gmbtn_idx != -1 )
  {
    gbtn = &_DK_active_buttons[gmbtn_idx];
    Gf_Btn_Callback callback;
    if ( _DK_lbDisplay.MLeftButton )
    {
      result = 1;
      callback = gbtn->click_event;
      if ((callback!=NULL) || (((gbtn->field_0 & 2)!=0) || (gbtn->field_2F!=0) || (gbtn->gbtype==3)))
        if ((gbtn->field_0 & 8)!=0)
        {
          switch (gbtn->gbtype)
          {
          case 1:
            if ( (gbtn->field_1>5) && (callback!=NULL) )
              callback(gbtn);
            else
              gbtn->field_1++;
            break;
          case 6:
            if (callback!=NULL)
              callback(gbtn);
            break;
          }
        }
    } else
    if ( _DK_lbDisplay.MRightButton )
    {
      result = 1;
      callback = gbtn->rclick_event;
      if ((callback != NULL) && ((gbtn->field_0 & 8)!=0))
      {
        switch (gbtn->gbtype)
        {
        case 1:
          if ( (gbtn->field_2>5) && (callback!=NULL) )
            callback(gbtn);
          else
            gbtn->field_2++;
          break;
        case 6:
          if (callback!=NULL)
            callback(gbtn);
          break;
        }
      }
    }
    if ( _DK_left_button_clicked )
    {
      result = 1;
      if (_DK_game.field_1516F3 != 0)
      {
        if (gbtn->id_num == _DK_game.field_1516F3)
          _DK_game.field_1516F3 = 0;
      }
      callback = gbtn->click_event;
      if ((callback!=NULL) || (gbtn->field_0 & 2) || (gbtn->field_2F) || (gbtn->gbtype==3) )
      {
        _DK_left_button_clicked = 0;
        _DK_gui_last_left_button_pressed_id = gbtn->id_num;
        _DK_do_button_click_actions(gbtn, &gbtn->field_1, callback);
      }
    } else
    if ( _DK_right_button_clicked )
    {
      result = 1;
      if (_DK_game.field_1516F3 != 0)
      {
        if (gbtn->id_num == _DK_game.field_1516F3)
          _DK_game.field_1516F3 = 0;
      }
      callback = gbtn->rclick_event;
      if ((callback!=NULL))
      {
        _DK_right_button_clicked = 0;
        _DK_gui_last_right_button_pressed_id = gbtn->id_num;
        _DK_do_button_click_actions(gbtn, &gbtn->field_2, callback);
      }
    }
  }

  for (gidx=0;gidx<86;gidx++)
  {
    gbtn = &_DK_active_buttons[gidx];
    if (gbtn->field_0 & 1)
      if ( ((gmbtn_idx==-1) || (gmbtn_idx!=gidx)) && (gbtn->gbtype!=3) && (gbtn!=_DK_input_button) )
      {
        gbtn->field_0 &= 0xEFu;
        gbtn->field_1 = 0;
        gbtn->field_2 = 0;
      }
  }
  if ( gmbtn_idx != -1 )
  {
    Gf_Btn_Callback callback;
    gbtn = &_DK_active_buttons[gmbtn_idx];
    if ((gbtn->field_1) && (_DK_left_button_released))
    {
      callback = gbtn->click_event;
      result = 1;
      if ((callback!=NULL) || (gbtn->field_0 & 2) || (gbtn->field_2F!=0) || (gbtn->gbtype==3))
      {
        _DK_left_button_released = 0;
        _DK_do_button_release_actions(gbtn, &gbtn->field_1, callback);
      }
    } else
    if ((gbtn->field_2) && (_DK_right_button_released))
    {
      callback = gbtn->rclick_event;
      result = 1;
      if ( callback!=NULL )
      {
        _DK_right_button_released = 0;
        _DK_do_button_release_actions(gbtn, &gbtn->field_2, callback);
      }
    }
  }
  if ((gameplay_on) && (_DK_tool_tip_time==0) && (!_DK_busy_doing_gui))
  {
        int mouse_x = _DK_lbDisplay.MMouseX*_DK_pixel_size;
        int mouse_y = _DK_GetMouseY();
        struct Coord3d mappos;
        if ( screen_to_map(player->camera,mouse_x,mouse_y,&mappos) )
        {
          int bblock_x=mappos.x.stl.num;
          int bblock_y=mappos.y.stl.num;
          unsigned short bitval;
          // Get the top four bits
          bitval = (_DK_game.map[bblock_x+bblock_y*256].field_3) >> 12;
          if ((1 << player->field_2B) & (bitval))
          {
            if (player->field_37 != 1)
            {
              short shown;
              shown=setup_trap_tooltips(&mappos);
              if (!shown)
                shown=setup_object_tooltips(&mappos);
              if (!shown)
                shown=setup_land_tooltips(&mappos);
              if (!shown)
                shown=setup_room_tooltips(&mappos);
              if (!shown)
              {
                _DK_help_tip_time = 0;
                _DK_tool_tip_box.target = NULL;
              }
            }
          }
        }
  }
  if ( _DK_tool_tip_box.field_0 == 0 )
    reset_scrolling_tooltip();
  return result;
}

short get_creature_passenger_action_inputs(void)
{
  if ( ((_DK_game.numfield_C & 0x01)==0) || (_DK_game.numfield_C & 0x80) )
      get_gui_inputs(1);
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  if ( !player->field_2F )
    return false;
  if (_DK_right_button_released)
  {
    struct Packet *pckt = &_DK_game.packets[player->field_B];
    _DK_set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  struct Thing *thing;
  thing = _DK_game.things_lookup[player->field_2F];
  if ((player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0) )
  {
    struct Packet *pckt = &_DK_game.packets[player->field_B];
    _DK_set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  if ( is_key_pressed(KC_TAB,0) )
  {
    _DK_lbKeyOn[KC_TAB] = 0;
    toggle_gui_overlay_map();
  }
  return false;
}

long menu_id_to_number(short menu_id)
{
  int idx;
  const short gmax=8;//sizeof(_DK_active_menus)/sizeof(struct GuiMenu);
  for(idx=0;idx<gmax;idx++)
  {
    struct GuiMenu *gmnu;
    gmnu=&_DK_active_menus[idx];
    if ((gmnu->field_1 != 0) && (gmnu->field_0 == menu_id))
      return idx;
  }
  return -1;
}

void toggle_tooltips(void)
{
  _DK_settings.tooltips_on = !_DK_settings.tooltips_on;
  if (_DK_settings.tooltips_on)
    do_sound_menu_click();
  _DK_save_settings();
}

void set_menu_visible_on(long menu_id)
{
  const short gmax=86;//sizeof(_DK_active_buttons)/sizeof(struct GuiButton);
  long menu_num;
  menu_num = menu_id_to_number(menu_id);
  if ( menu_num == -1 )
    return;
  _DK_active_menus[menu_num].flgfield_1D = 1;
  int idx;
  for (idx=0;idx<gmax;idx++)
  {
    struct GuiButton *gbtn = &_DK_active_buttons[idx];
    if (gbtn->field_0 & 1)
    {
      Gf_Btn_Callback callback;
      callback = gbtn->field_17;
      if ((gbtn->gmenu_idx == menu_num) && (callback != NULL))
        callback(gbtn);
    }
  }
}

void set_menu_visible_off(long menu_id)
{
  long menu_num;
  menu_num = menu_id_to_number(menu_id);
  if ( menu_num == -1 )
    return;
  _DK_active_menus[menu_num].flgfield_1D = 0;
}

void zoom_from_map(void)
{
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  if ( (_DK_game.numfield_A & 0x01) || (_DK_lbDisplay.PhysicalScreenWidth > 320) )
  {
      _DK_toggle_status_menu((_DK_game.numfield_C & 0x40) != 0);
      struct Packet *pckt = &_DK_game.packets[player->field_B];
      _DK_set_packet_action(pckt, 120,1,0,0,0);
  } else
  {
      struct Packet *pckt = &_DK_game.packets[player->field_B];
      _DK_set_packet_action(pckt, 80,6,0,0,0);
  }
}

short get_map_action_inputs()
{
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  long mouse_x = _DK_lbDisplay.MMouseX * _DK_pixel_size;
  long mouse_y = _DK_GetMouseY();
  int mappos_x = 3 * (mouse_x - 150) / 4 + 1;
  int mappos_y = 3 * (mouse_y - 56) / 4 + 1;
  if ((mappos_x >= 0) && (mappos_x < 255) && (mappos_y >= 0) && (mappos_y < 255) )
  {
    if ( _DK_left_button_clicked )
    {
      _DK_left_button_clicked = 0;
    }
    if ( _DK_left_button_released )
    {
      struct Packet *pckt = &_DK_game.packets[player->field_B];
      _DK_set_packet_action(pckt, 81,mappos_x,mappos_y,0,0);
      _DK_left_button_released = 0;
      return true;
    }
  }

  if ( _DK_right_button_clicked )
    _DK_right_button_clicked = 0;
  if ( _DK_right_button_released )
  {
    _DK_right_button_released = 0;
    zoom_from_map();
    return true;
  } else
  {
    struct Packet *pckt = &_DK_game.packets[player->field_B];
    if ( pckt->field_5 )
      return true;
    if ( _DK_lbKeyOn[KC_F8] )
    {
      _DK_lbKeyOn[KC_F8] = 0;
      toggle_tooltips();
    }
    if ( _DK_lbKeyOn[KC_NUMPADENTER] )
    {
      _DK_lbKeyOn[KC_NUMPADENTER] = 0;
      // Toggle cheat menu
/*
      if ( (_DK_gui_box==NULL) || (_DK_gui_box_is_not_valid(_DK_gui_box)) )
      {
        gui_create_box(?,?,_DK_gui_main_option_list);
      } else
      {
        gui_delete_box(_DK_gui_box);
      }
*/
    }
    long keycode;
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      _DK_lbKeyOn[keycode] = 0;
      _DK_turn_off_all_window_menus();
      zoom_from_map();
      return true;
    }
    return false;
  }
}

short toggle_first_person_menu(short visible)
{
  static char creature_query1_on = 0;
  static char creature_query2_on = 0;
  if (visible)
  {
    if ( creature_query1_on )
        set_menu_visible_on(31);
    else
    if ( creature_query2_on )
      set_menu_visible_on(35);
    return 1;
  } else
  {
    long gmnu_idx;
    // Menu no 31
    gmnu_idx=menu_id_to_number(31);
    if (gmnu_idx != -1)
      creature_query1_on = _DK_active_menus[gmnu_idx].flgfield_1D;
    else
      creature_query1_on = _DK_default_menu.flgfield_1D;
    set_menu_visible_off(31);
    // Menu no 35
    gmnu_idx=menu_id_to_number(35);
    if (gmnu_idx != -1)
      creature_query2_on = _DK_active_menus[gmnu_idx].flgfield_1D;
    else
      creature_query2_on = _DK_default_menu.flgfield_1D;
    set_menu_visible_off(31);
    return 1;
  }
}

void set_gui_visible(unsigned long visible)
{
  if (visible)
    _DK_game.numfield_C |= 0x20;
  else
    _DK_game.numfield_C &= 0xDF;
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  unsigned char is_visbl = ((_DK_game.numfield_C & 0x20)!=0);
  if (player->field_452 == 2)
    toggle_first_person_menu(is_visbl);
  else
    _DK_toggle_status_menu(is_visbl);
  if ( (_DK_game.numfield_D & 0x20) && (_DK_game.numfield_C & 0x20) )
    _DK_setup_engine_window(140, 0, _DK_MyScreenWidth, _DK_MyScreenHeight);
  else
    _DK_setup_engine_window(0, 0, _DK_MyScreenWidth, _DK_MyScreenHeight);
}

void toggle_gui(void)
{
  int visible=((_DK_game.numfield_C & 0x20)==0);
  set_gui_visible(visible);
}

long creature_instance_is_available(struct Thing *thing, long inum)
{
  struct CreatureControl *crctrl;
  crctrl = _DK_game.creature_control_lookup[thing->field_64];
  return crctrl->instances[inum];
}

short get_creature_control_action_inputs(void)
{
  if ( ((_DK_game.numfield_C & 0x01)==0) || (_DK_game.numfield_C & 0x80) )
    get_gui_inputs(1);
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  if ( _DK_lbKeyOn[KC_NUMPADENTER] )
  {
      _DK_lbKeyOn[KC_NUMPADENTER] = 0;
      // Toggle cheat menu
/*
      if ( (_DK_gui_box==NULL) || (_DK_gui_box_is_not_valid(_DK_gui_box)) )
      {
        gui_box=gui_create_box(?,?,_DK_gui_instance_option_list); / 200,20 or 20,200
        player->unknownbyte  |= 0x08;
        _DK_game.unknownbyte |= 0x08;
      } else
      {
        gui_delete_box(_DK_gui_box);
        player->unknownbyte &= 0xF7;
        _DK_game.unknownbyte &= 0xF7;
      }
*/
      return 1;
  }
  if ( _DK_lbKeyOn[KC_F12] )
  {
      _DK_lbKeyOn[KC_F12] = 0;
      // Cheat sub-menus
/*
      if ( (_DK_gui_cheat_box==NULL) || (_DK_gui_box_is_not_valid(_DK_gui_cheat_box)) )
      {
        ...
      } else
      {
        gui_delete_box(_DK_gui_cheat_box);
        player->unknownbyte &= 0xF7;
      }
*/
  }

  if ( player->field_2F )
  {
    short make_packet = _DK_right_button_released;
    if (!make_packet)
    {
      struct Thing *thing;
      thing = _DK_game.things_lookup[player->field_2F];
      if ( (player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0)
         || (thing->field_7 == 67) )
        make_packet = true;
    }
    if (make_packet)
    {
      struct Packet *pckt = &_DK_game.packets[player->field_B];
      _DK_right_button_released = 0;
      _DK_set_packet_action(pckt, 33, player->field_2F,0,0,0);
    }
  }
  if ( is_key_pressed(KC_TAB,0) )
  {
    _DK_lbKeyOn[KC_TAB] = 0;
    toggle_gui();
  }
  int numkey;
  numkey = -1;
  {
    long keycode;
    for (keycode=KC_1;keycode<=KC_0;keycode++)
    {
      if ( is_key_pressed(keycode,0) )
      {
        _DK_lbKeyOn[keycode] = 0;
        numkey = keycode-2;
        break;
      }
    }
  }
  if ( numkey != -1 )
  {
    int idx;
    int num_avail = 0;
    for (idx=0;idx<10;idx++)
    {
      struct Thing *thing;
      thing = _DK_game.things_lookup[player->field_2F];
      int instnce = _DK_game.creature_stats[thing->field_1A].field_80[idx];
      if ( creature_instance_is_available(thing,instnce) )
      {
        if ( numkey == num_avail )
        {
          struct Packet *pckt = &_DK_game.packets[player->field_B];
          _DK_set_packet_action(pckt, 39, instnce,0,0,0);
          break;
        }
        num_avail++;
      }
    }
  }
  return false;
}

void turn_off_menu(short mnu_idx)
{
  _DK_turn_off_menu(mnu_idx);
}

void get_level_lost_inputs(void)
{
//  _DK_get_level_lost_inputs();
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  if ( player->field_0 & 4 )
  {
    if (is_key_pressed(KC_RETURN,0))
    {
      struct Packet *pckt=&_DK_game.packets[player->field_B];
      _DK_set_packet_action(pckt, 14, 0,0,0,0);
      _DK_lbInkey = 0;
      _DK_lbKeyOn[KC_RETURN] = 0;
    } else
    {
      _DK_lbFontPtr = _DK_winfont;
      if ( (_DK_lbInkey == KC_BACK) || (_DK_pixel_size*_DK_LbTextStringWidth(player->field_463) < 450) )
      {
        struct Packet *pckt=&_DK_game.packets[player->field_B];
        _DK_set_packet_action(pckt, 121, _DK_lbInkey,0,0,0);
        _DK_lbKeyOn[_DK_lbInkey] = 0;
        _DK_lbInkey = 0;
      }
    }
    return;
  }
  if ((_DK_game.numfield_A & 1) != 0)
  {
    if (is_key_pressed(KC_RETURN,0))
    {
      struct Packet *pckt=&_DK_game.packets[player->field_B];
      _DK_set_packet_action(pckt, 13, 0,0,0,0);
      _DK_lbKeyOn[KC_RETURN] = 0;
      return;
    }
  }
  if (is_key_pressed(KC_SPACE,0))
  {
    struct Packet *pckt=&_DK_game.packets[player->field_B];
    _DK_set_packet_action(pckt, 5, 0,0,0,0);
    _DK_lbKeyOn[KC_SPACE] = 0;
  }


  if ( player->field_452 == 4 )
  {
    long keycode;
    int screen_x = _DK_lbDisplay.MMouseX * _DK_pixel_size - 150;
    int screen_y = _DK_GetMouseY() - 56;
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      _DK_lbKeyOn[keycode] = 0;
      if (((_DK_game.numfield_A & 0x01) == 0) && (_DK_lbDisplay.PhysicalScreenWidth <= 320))
      {
        struct Packet *pckt=&_DK_game.packets[player->field_B];
        _DK_set_packet_action(pckt, 80, 6,0,0,0);
      }
      else
      {
        _DK_toggle_status_menu((_DK_game.numfield_C & 0x40) != 0);
        struct Packet *pckt=&_DK_game.packets[player->field_B];
        _DK_set_packet_action(pckt, 120, 1,0,0,0);
      }
    } else
    if ( _DK_right_button_released )
    {
        _DK_right_button_released = 0;
        if ( (_DK_game.numfield_A & 1) || _DK_lbDisplay.PhysicalScreenWidth > 320 )
        {
          _DK_toggle_status_menu((_DK_game.numfield_C & 0x40) != 0);
          struct Packet *pckt=&_DK_game.packets[player->field_B];
          _DK_set_packet_action(pckt, 120, 1,0,0,0);
        }
        else
        {
          struct Packet *pckt=&_DK_game.packets[player->field_B];
          _DK_set_packet_action(pckt, 80, 6,0,0,0);
        }
    } else
    if ( _DK_left_button_released )
    {
        int actn_x = 3*screen_x/4 + 1;
        int actn_y = 3*screen_y/4 + 1;
        if  ((actn_x >= 0) && (actn_x < 255) && (actn_y >= 0) && (actn_y < 255))
        {
          struct Packet *pckt=&_DK_game.packets[player->field_B];
          _DK_set_packet_action(pckt, 81, actn_x,actn_y,0,0);
          _DK_left_button_released = 0;
        }
    }
  } else
  if ( player->field_452 == 1 )
  {
    long keycode;
    if ( _DK_lbKeyOn[KC_TAB] )
    {
        if ((player->field_37 == 2) || (player->field_37 == 5))
        {
          _DK_lbKeyOn[KC_TAB] = 0;
          toggle_gui();
        }
    } else
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      _DK_lbKeyOn[keycode] = 0;
      if (player->field_37 != 7)
      {
        _DK_turn_off_all_window_menus();
        _DK_game.numfield_C = (_DK_game.numfield_C ^ (unsigned __int8)(2 * _DK_game.numfield_C)) & 0x40 ^ _DK_game.numfield_C;
        struct Packet *pckt=&_DK_game.packets[player->field_B];
        if ((_DK_game.numfield_A & 1) || (_DK_lbDisplay.PhysicalScreenWidth > 320))
        {
              if (_DK_toggle_status_menu(0))
                _DK_game.numfield_C |= 0x40;
              else
                _DK_game.numfield_C &= 0xBF;
              _DK_set_packet_action(pckt, 119, 4,0,0,0);
        } else
        {
              _DK_set_packet_action(pckt, 80, 5,0,0,0);
        }
        _DK_turn_off_roaming_menus();
      }
    }
  }
  if ( _DK_lbKeyOn[KC_ESCAPE] )
  {
    _DK_lbKeyOn[KC_ESCAPE] = 0;
    if ( _DK_a_menu_window_is_active() )
      _DK_turn_off_all_window_menus();
    else
      _DK_turn_on_menu(8);
  }
  struct Packet *pckt=&_DK_game.packets[player->field_B];
  struct Thing *thing;
  short inp_done=false;
  switch (player->field_452)
  {
    case 1:
      inp_done=_DK_menu_is_active(38);
      if ( !inp_done )
      {
        if ((_DK_game.numfield_C & 0x20) != 0)
        {
          _DK_initialise_tab_tags_and_menu(3);
          _DK_turn_off_all_panel_menus();
          _DK_turn_on_menu(38);
        }
      }
      inp_done=get_gui_inputs(1);
      if ( !inp_done )
      {
        if (player->field_453 == 15)
        {
          _DK_set_player_instance(player, 10, 0);
        } else
        {
          inp_done=_DK_get_small_map_inputs(player->mouse_x, player->mouse_y, player->field_450 / (3-_DK_pixel_size));
          if ( !inp_done )
            _DK_get_bookmark_inputs();
          _DK_get_dungeon_control_nonaction_inputs();
        }
      }
      break;
    case 2:
      thing = _DK_game.things_lookup[player->field_2F];
      if (thing->field_1F == 5)
      {
        struct CreatureControl *crctrl;
        crctrl = _DK_game.creature_control_lookup[thing->field_64];
        if ((crctrl->field_2 & 0x02) == 0)
          _DK_set_packet_action(pckt, 33, player->field_2F,0,0,0);
      } else
      {
        _DK_set_packet_action(pckt, 33, player->field_2F,0,0,0);
      }
      break;
    case 3:
      _DK_set_packet_action(pckt, 32, player->field_2F,0,0,0);
      break;
    case 4:
      if ( _DK_menu_is_active(38) )
      {
        if ((_DK_game.numfield_C & 0x20) != 0)
          turn_off_menu(38);
      }
      break;
    default:
      return;
  }
}

short get_inputs(void)
{
  //return _DK_get_inputs();
  if ( _DK_game.flags_cd & 0x01 )
  {
    _DK_load_packets_for_turn(_DK_game.gameturn);
    _DK_game.gameturn++;
    if ( _DK_lbKeyOn[KC_ESCAPE] || _DK_lbKeyOn[KC_SPACE] || _DK_lbKeyOn[KC_RETURN]
       || (_DK_lbKeyOn[KC_LALT] && _DK_lbKeyOn[KC_X]) || _DK_left_button_clicked )
    {
      _DK_lbKeyOn[KC_ESCAPE] = 0;
      _DK_lbKeyOn[KC_SPACE] = 0;
      _DK_lbKeyOn[KC_RETURN] = 0;
      _DK_lbKeyOn[KC_X] = 0;
      _DK_left_button_clicked = 0;
      _DK_quit_game = 1;
    }
    return false;
  }
  if ( _DK_game.field_149E81 )
  {
    _DK_load_packets_for_turn(_DK_game.gameturn);
    _DK_game.gameturn++;
    if ( _DK_lbKeyOn[KC_LALT] && _DK_lbKeyOn[KC_X] )
    {
      _DK_lbKeyOn[KC_X] = 0;
      if ( _DK_game.numfield_A & 0x01 )
        _DK_LbNetwork_Stop();
      _DK_quit_game = 1;
      _DK_exit_keeper = 1;
    }
    return false;
  }
  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  if ( player->field_0 & 0x80 )
  {
    struct Packet *pckt = &_DK_game.packets[player->field_B];
    pckt->field_A = 127;
    pckt->field_C = 127;
    if ((_DK_input_button==NULL) && (_DK_game.numfield_C & 0x01))
    {
      long keycode;
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        _DK_lbKeyOn[keycode] = 0;
        _DK_set_packet_action(pckt, 22, 0,0,0,0);
      }
    }
    return false;
  }
  if (player->field_29 == 2)
  {
    if (player->field_2C != 1)
    {
      get_level_lost_inputs();
      return true;
    }
    struct Thing *thing;
    thing = _DK_game.things_lookup[player->field_2F];
    if ( (thing <= _DK_game.things_lookup[0]) || (thing->field_1F != 5) )
    {
      get_level_lost_inputs();
      return true;
    }
    struct CreatureControl *crctrl;
    crctrl = _DK_game.creature_control_lookup[thing->field_64];
    if ((crctrl->field_2 & 2) == 0)
    {
      get_level_lost_inputs();
      return true;
    }
  }
  short inp_handled = false;
  if ( !(_DK_game.numfield_C & 0x01) || (_DK_game.numfield_C & 0x80) )
    inp_handled = get_gui_inputs(1);
  if ( !inp_handled )
    inp_handled = _DK_get_global_inputs();
  if (_DK_input_button!=NULL)
    return false;
  struct Packet *pckt;
  switch ( player->field_452 )
  {
  case 1:
      if (!inp_handled)
        inp_handled=_DK_get_dungeon_control_action_inputs();
      _DK_get_dungeon_control_nonaction_inputs();
      _DK_get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 2:
      if (!inp_handled)
        inp_handled = get_creature_control_action_inputs();
      _DK_get_creature_control_nonaction_inputs();
      _DK_get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 3:
      if ( inp_handled )
      {
        _DK_get_player_gui_clicks();
        _DK_get_packet_control_mouse_clicks();
        return true;
      } else
      if ( get_creature_passenger_action_inputs() )
      {
        _DK_get_packet_control_mouse_clicks();
        return true;
      } else
      {
        _DK_get_player_gui_clicks();
        _DK_get_packet_control_mouse_clicks();
        return false;
      }
  case 4:
      if (!inp_handled)
        inp_handled = get_map_action_inputs();
      _DK_get_map_nonaction_inputs();
      _DK_get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 5:
      if (player->field_37==6)
        return false;
      if ( !(_DK_game.numfield_A & 0x01) )
        _DK_game.numfield_C &= 0xFE;
      if ( _DK_toggle_status_menu(0) )
        player->field_1 |= 0x01;
      else
        player->field_1 &= 0xFE;
      pckt = &_DK_game.packets[player->field_B];
      _DK_set_packet_action(pckt, 80, 4,0,0,0);
      return false;
  case 6:
      if (player->field_37 != 7)
      {
        pckt = &_DK_game.packets[player->field_B];
        _DK_set_packet_action(pckt, 80, 1,0,0,0);
      }
      return false;
  default:
      return false;
  }
}

void input(void)
{
  static const char *func_name="input";
  //_DK_input();return;
  // Set key modifiers based on the pressed key codes
  unsigned short key_mods=0;
  if ( _DK_lbKeyOn[KC_LSHIFT] || _DK_lbKeyOn[KC_RSHIFT] )
    key_mods |= 0x01;
  if ( _DK_lbKeyOn[KC_LCONTROL] || _DK_lbKeyOn[KC_RCONTROL] )
    key_mods |= 0x02;
  if ( _DK_lbKeyOn[KC_LALT] || _DK_lbKeyOn[KC_RALT] )
    key_mods |= 0x04;
  _DK_key_modifiers = key_mods;
  if ( _DK_input_button )
  {
    if ( _DK_lbInkey )
      _DK_lbKeyOn[_DK_lbInkey] = 0;
  }

  struct PlayerInfo *player=&(_DK_game.players[_DK_my_player_number]);
  int idx=player->field_B;
  struct Packet *pckt=&_DK_game.packets[idx];
  if (_DK_is_game_key_pressed(27, 0, 0) != 0)
    pckt->field_10 |= 0x20;
  else
    pckt->field_10 &= 0xDFu;
  if (_DK_is_game_key_pressed(28, 0, 0) != 0)
    pckt->field_10 |= 0x40;
  else
    pckt->field_10 &= 0xBFu;

  get_inputs();
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
    if ( _DK_game.numfield_C & 0x02 )
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
      if ( !(_DK_game.numfield_C & 0x01) )
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
      input_eastegg();
      input();
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
        // Display in-game message for debug purposes
        //_DK_LbTextDraw(200/_DK_pixel_size, 8/_DK_pixel_size, text);text[0]='\0';
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
            MonitorStreamedSoundTrack();
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
      turn_off_menu(18);
      break;
    case 2:
      turn_off_menu(19);
      break;
    case 3:
      _DK_frontmap_unload();
      _DK_frontend_load_data();
      break;
    case 4:
      turn_off_menu(20);
      break;
    case 5: // Network play mode
      turn_off_menu(21);
      break;
    case 6:
      turn_off_menu(22);
      break;
    case 12:
    case 29:
      frontstory_unload();
      break;
    case 13:
      if ( !(_DK_game.flags_cd & 0x10) )
        StopRedbookTrack();
      break;
    case 15:
      turn_off_menu(23);
      _DK_frontnet_modem_reset();
      break;
    case 16:
      turn_off_menu(24);
      _DK_frontnet_serial_reset();
      break;
    case 17:
      StopStreamedSample();
      turn_off_menu(25);
      break;
    case 18:
      turn_off_menu(26);
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
      turn_off_menu(36);
      _DK_save_settings();
      break;
    case 27:
      turn_off_menu(39);
      if ( !(_DK_game.flags_cd & 0x10) )
        StopRedbookTrack();
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
        get_gui_inputs(0);
        break;
      case 3:
        _DK_frontmap_input();
        break;
      case 6:
        get_gui_inputs(0);
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
        get_gui_inputs(0);
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
          get_gui_inputs(0);
        else
          define_key_input();
        break;
      default:
        get_gui_inputs(0);
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
    const char *name;
    };

const struct TbBirthday team_birthdays[] = {
    {13,1,"Mark Healey"},
    {21,3,"Jonty Barnes"},
    {3,5,"Simon Carter"},
    {5,5,"Peter Molyneux"},
    {13,11,"Alex Peters"},
    {1,12,"Dene Carter"},
    {25,5,"Tomasz Lis"},
    {29,11,"Michael Chateauneuf"},
    {0,0,NULL},
    };

const char *get_team_birthday()
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

void frontbirthday_draw()
{
  frontend_copy_background();
  _DK_LbTextSetWindow(70, 70, 500, 340);
  _DK_lbFontPtr = _DK_frontstory_font;
  _DK_lbDisplay.DrawFlags = 256;
  const char *name=get_team_birthday();
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
    // In-Menu information, for debugging messages
    //char text[255];
    //sprintf(text, "time %7d, mode %d",LbTimerClock(),_DK_frontend_menu_state);
    //_DK_lbDisplay.DrawFlags=0;_DK_LbTextSetWindow(0,0,640,200);_DK_lbFontPtr = _DK_frontend_font[0];
    //_DK_LbTextDraw(200/_DK_pixel_size, 8/_DK_pixel_size, text);text[0]='\0';
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
        frontnet_service_update();
        break;
      case 5:
        frontnet_session_update();
        break;
      case 6:
        frontnet_start_update();
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
          PlayRedbookTrack(7);
        break;
      case 15:
        frontnet_modem_update();
        break;
      case 16:
        frontnet_serial_update();
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
          PlayRedbookTrack(3);
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
    return setup_network_service(_DK_net_service_index_selected);
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

  if ( _DK_game.numfield_C & 0x02 )
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
      MonitorStreamedSoundTrack();
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
    StopRedbookTrack();
    StopMusic();
    _DK_frontstats_initialise();
    _DK_delete_all_structures();
    _DK_clear_mapwho();
    endtime = LbTimerClock();
    _DK_quit_game = 0;
    if ( _DK_game.numfield_C & 0x02 )
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
  FreeAudio();
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
        _DK_game.numfield_C |= 0x02;
        level_num = atoi(pr2str);
        narg++;
      } else
      if ( stricmp(parstr,"q") == 0 )
      {
         _DK_game.numfield_C |= 0x02;
      } else
      if ( stricmp(parstr,"columnconvert") == 0 )
      {
         _DK_game.numfield_C |= 0x08;
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
  if ( (_DK_game.numfield_C & 0x02) == 0 )
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
//  return _DK_WinMain(hThisInstance,hPrevInstance,lpszArgument,nFunsterStil);
  static const char *func_name="WinMain";
  _DK_hInstance = hThisInstance;
  _DK_lpDDC = NULL;

  get_cmdln_args(bf_argc, bf_argv);

/*  {
  struct PlayerInfo *player=&(_DK_game.players[0]);
      static char msg_text[255];
      sprintf(msg_text,"Position of the first Player is %06x, first Camera is %06x bytes.\n",((int)player) - ((int)&_DK_game),((int)&(player->camera)) - ((int)player));
      error(func_name, 1, msg_text);
      MessageBox(NULL, msg_text, PROGRAM_FULL_NAME, MB_OK | MB_ICONERROR);
      return 0;
  }*/


  if (sizeof(struct Game)!=SIZEOF_Game)
  {
      static char msg_text[255];
      sprintf(msg_text,"Bad compilation - struct Game has wrong size!\nThe difference is %d bytes.\n",sizeof(struct Game)-SIZEOF_Game);
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
