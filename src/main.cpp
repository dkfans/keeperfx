
#include <windows.h>
#include <winbase.h>
#include <math.h>
#include "keeperfx.h"

#include "bflib_dernc.h"
#include "bflib_pom.h"
#include "bflib_memory.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_bufrw.h"
#include "bflib_sprite.h"
#include "bflib_fileio.h"
#include "bflib_sndlib.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"

#include "frontend.h"

#define CMDLN_MAXLEN 259
char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];
unsigned char palette_buf[768];
int map_subtiles_x=255;
int map_subtiles_y=255;

char window_class_name[128]="Bullfrog Shell";
short default_loc_player=0;
short screenshot_format=1;
const char keeper_config_file[]="keeperfx.cfg";

#define pointer_x _DK_pointer_x
#define pointer_y _DK_pointer_y
#define top_pointed_at_x _DK_top_pointed_at_x
#define top_pointed_at_y _DK_top_pointed_at_y
#define block_pointed_at_x _DK_block_pointed_at_x
#define block_pointed_at_y _DK_block_pointed_at_y
#define top_pointed_at_frac_x _DK_top_pointed_at_frac_x
#define top_pointed_at_frac_y _DK_top_pointed_at_frac_y
#define pointed_at_frac_x _DK_pointed_at_frac_x
#define pointed_at_frac_y _DK_pointed_at_frac_y
#define load_game_scroll_offset _DK_load_game_scroll_offset
#define save_game_catalogue _DK_save_game_catalogue
#define datafiles_path _DK_datafiles_path
#define exchangeBuffer _DK_exchangeBuffer
#define exchangeSize _DK_exchangeSize
#define maximumPlayers _DK_maximumPlayers
#define localPlayerInfoPtr _DK_localPlayerInfoPtr
#define localDataPtr _DK_localDataPtr
#define compositeBuffer _DK_compositeBuffer
#define sequenceNumber _DK_sequenceNumber
#define timeCount _DK_timeCount
#define maxTime _DK_maxTime
#define runningTwoPlayerModel _DK_runningTwoPlayerModel
#define waitingForPlayerMapResponse _DK_waitingForPlayerMapResponse
#define compositeBufferSize _DK_compositeBufferSize
#define basicTimeout _DK_basicTimeout
#define noOfEnumeratedDPlayServices _DK_noOfEnumeratedDPlayServices
#define clientDataTable _DK_clientDataTable
#define receiveCallbacks _DK_receiveCallbacks
#define TRACE LbNetLog

struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE", &_DK_palette, NULL, PALETTE_SIZE, 0, 0},
    {"*SCRATCH", &_DK_scratch, NULL, 0x10000, 1, 0},
    {"", NULL, NULL, 0, 0, 0}, };

unsigned char *nocd_raw;
unsigned char *nocd_pal;

struct TbLoadFiles nocd_load_files[] = {
    {"data/nocd.raw", &nocd_raw, NULL, 0, 0, 0},
    {"data/nocd.pal", &nocd_pal, NULL, 0, 0, 0},
    {"", NULL, NULL, 0, 0, 0}, };

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
  {"INSTALL_PATH=",  1},
  {"INSTALL_TYPE=",  2},
  {"LANGUAGE=",      3},
  {"KEYBOARD=",      4},
  {"SCREENSHOT=",    5},
  {NULL,             0},
  };


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

short load_configuration(void)
{
  static const char *func_name="load_configuration";
  //return _DK_load_configuration();
  char fname[255];
  char *buf;
  long len,pos;
  int i,cmd_num;
  // Preparing config file name and checking the file
  sprintf(fname, "%s/%s", keeper_runtime_directory,keeper_config_file);
  strcpy(install_info.inst_path,"");
  install_info.field_9A = 0;
  if (!LbFileExists(fname))
    return false;
  buf = (char *)LbMemoryAlloc(4096); // configuration file size restriction
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
      cmd_num = 0;
      while (conf_commands[i].num > 0)
      {
        if (strnicmp(buf+pos, conf_commands[i].name, strlen(conf_commands[i].name)) == 0)
        {
          pos += strlen(conf_commands[i].name);
          cmd_num = conf_commands[i].num;
          break;
        }
        i++;
      }
      // Now store the config item in correct place
      switch (cmd_num)
      {
      case 1: // INSTALL_PATH
          for (i=0;i<149;i++) //sizeof(inst_path)-1
          {
              if ((buf[pos]==10)||(buf[pos]==13)||(buf[pos]<7))
                break;
              install_info.inst_path[i]=buf[pos];
              pos++;
          }
          while (i>0)
          {
            if ((install_info.inst_path[i]!='\\') && (install_info.inst_path[i]!='/') &&
                (install_info.inst_path[i]>32))
              break;
            i--;
          }
          install_info.inst_path[i+1]='\0';
          break;
      case 2: // INSTALL_TYPE
          // This command is just skipped...
          pos += strlen("MAX");
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
      default:
          LbSyncLog("Unrecognized command in config file, starting on byte %d.\n",pos);
      }
      // Skip to end of the line
      while (pos<len)
      {
        if ((buf[pos]==10) || (buf[pos]==13)) break;
        pos++;
      }
      // Go to start of next line
      while (pos<len)
      {
        if (buf[pos]>32) break;
        pos++;
      }
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

/*TODO: requires clientDataTable and maximumPlayers
short check_host_id(unsigned long *src_id)
{
  short result;
  struct ClientDataEntry *cent;
  unsigned int i;
  result = 0;
  for (i=0; i<maximumPlayers; i++)
  {
    cent = &clientDataTable[i];
    if ( (cent->field_4) && (cent->field_0 == (*src_id)))
    {
      result = 1;
      (*src_id) = i;
    }
  }
  return result;
}*/

/*TODO: requires exported variables
void * __stdcall MultiPlayerCallback(unsigned long src_id, unsigned long seq_length, unsigned long seq_num, void *buf)
{
//  if ( dword_538E48 ) - variable from DK Premium
//    TRACE("GOT A REQUEST MultiPlayerCallback\n");
  if (hostId == localPlayerId)
  {
    if (seq_length != exchangeSize)
    {
      TRACE("  WARNING network: UserDataMsgCallback() invalid length: %d\n", seq_length);
      return NULL;
    }
    if (src_id == localPlayerId)
    {
      TRACE("  WARNING network: UserDataMsgCallback() host got data from itself\n");
      return NULL;
    }
    if (!check_host_id(&src_id))
    {
      TRACE("  WARNING network: UserDataMsgCallback() invalid id: %d\n", src_id);
      return NULL;
    }
    if ((seq_num==sequenceNumber) || (seq_num==15))
    {
      TRACE("  WARNING network: UserDataMsgCallback() Unexpected sequence number: Got %d, expected %d\n",seq_num,sequenceNumber);
      return NULL;
    }
    clientDataTable[src_id].field_8 = 1;
    return (unsigned char *)exchangeBuffer + exchangeSize*src_id;
  } else
  if ( exchangeSize*maximumPlayers == seq_length )
  {
    if ( hostId != src_id )
    {
      TRACE("  WARNING network: UserDataMsgCallback() data is not from host\n");
      return NULL;
    }
    if ( !check_host_id(&src_id) )
    {
      TRACE("  WARNING network: UserDataMsgCallback() invalid id: %d\n", src_id);
      return NULL;
    }
    if ( sequenceNumber == 15 )
    {
      sequenceNumber = seq_num;
    } else
    if ( seq_num != sequenceNumber )
    {
      TRACE("  WARNING network: UserDataMsgCallback() Unexpected sequence number: Got %d, expected %d\n",seq_num,sequenceNumber);
      return NULL;
    }
    gotCompositeData = 1;
    return (unsigned char *)exchangeBuffer;
  } else
  {
    if ( seq_length != exchangeSize )
    {
      TRACE("  WARNING network: UserDataMsgCallback() invalid length: %d\n", seq_length);
      return NULL;
    }
    if ( src_id == localPlayerId )
    {
      TRACE("  WARNING network: UserDataMsgCallback() client acting as host got data from itself\n");
      return NULL;
    }
    if ( !check_host_id(&src_id) )
    {
      TRACE("  WARNING network: UserDataMsgCallback() invalid id: %d\n", src_id);
      return NULL;
    }
    clientDataTable[src_id].field_8 = 1;
    return (unsigned char *)exchangeBuffer + exchangeSize*src_id;
  }
}*/

/*TODO: requires ServiceProvider and HostDataCollection
void __stdcall MultiPlayerReqExDataMsgCallback(unsigned long src_id, unsigned long seqNum, void *buf)
{
//  if ( dword_538E48 ) - variable from DK Premium
//    TRACE("GOT A REQUEST MultiPlayerReqExDataMsgCallback\n");
  if (localDataPtr == NULL)
  {
    TRACE("  WARNING network: RequestExchangeDataMsgCallback() NULL data pointer\n");
    return;
  }
  if (sequenceNumber == 15)
  {
    sequenceNumber = seqNum;
  } else
  if (sequenceNumber != seqNum)
  {
    TRACE("  WARNING network: RequestExchangeDataMsgCallback() unexpected sequence number, got %d, expected %d\n",seqNum,sequenceNumber);
    return;
  }
  ServiceProvider::EncodeMessageStub(localDataPtr, exchangeSize-4, 0, sequenceNumber);
  if ( HostDataCollection(src_id, localDataPtr) )
  {
    TRACE("  WARNING network: RequestExchangeDataMsgCallback() failure on SP::Send()\n");
    return;
  }
}*/

TbError GenericIPXInit(struct _GUID guid)
{
  //TODO
  return -1;
}

TbError GenericSerialInit(struct _GUID guid,void *)
{
  //TODO
  return -1;
}

TbError GenericModemInit(struct _GUID guid,void *)
{
  //TODO
  return -1;
}

TbError DPlayInit(struct _GUID guid,struct _GUID *cliguid)
{
  //TODO
  return -1;
}

TbError LbNetwork_Init(unsigned long srvcp,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data)
{
  return _DK_LbNetwork_Init(srvcp,guid,maxplayrs,exchng_buf,exchng_size,locplayr,init_data);
  //TODO
  long dpidx;
  exchangeBuffer = exchng_buf;
  exchangeSize = exchng_size;
  maximumPlayers = maxplayrs;
  localPlayerInfoPtr = locplayr;
  localDataPtr = 0;
  compositeBuffer = 0;
  sequenceNumber = 0;
  timeCount = 0;
  maxTime = 0;
  runningTwoPlayerModel = 0;
  waitingForPlayerMapResponse = 0;
  compositeBufferSize = 0;
  basicTimeout = 250;
//  receiveCallbacks_offs24 = NULL; //probably not needed
//  receiveCallbacks.multi_player = MultiPlayerCallback;
//  receiveCallbacks.mp_req_exdata_msg = MultiPlayerReqExDataMsgCallback;
  if (exchng_size > 0)
  {
    compositeBufferSize = maxplayrs * exchng_size;
    compositeBuffer = LbMemoryAlloc(compositeBufferSize);
    if (compositeBuffer == NULL)
    {
      TRACE("WARNING network: LbNetwork_Init() failure on compositeBuffer allocation\n");
      return -1;
    }
  }
  int k;

  memset(clientDataTable, 0, sizeof(clientDataTable));
  for(k=0; k<maximumPlayers; k++)
    clientDataTable[k].field_4 = 0;

  for (k=0; k<maximumPlayers; k++)
  {
      _DK_net_player_info[k].field_20 = 0;
      strcpy(_DK_net_player_info[k].field_0, "");
  }
  switch (srvcp)
  {
  case 0:
      TRACE("Selecting Win95 Serial SP\n");
      if ( GenericSerialInit(guid, init_data) )
      {
        TRACE("WARNING network: LbNetwork_Init() failure on GenericSerialInit()\n");
        return -1;
      }
      break;
  case 1:
      TRACE("Selecting Win95 Modem SP\n");
      if ( GenericModemInit(guid, init_data) )
      {
        TRACE("WARNING network: LbNetwork_Init() failure on GenericModemInit()\n");
        return -1;
      }
      break;
  case 2:
      TRACE("Selecting Win95 IPX SP\n");
      if ( GenericIPXInit(guid) )
      {
        TRACE("WARNING network: LbNetwork_Init() failure on GenericIPXInit()\n");
        return -1;
      }
      break;
  case 3:
  default:
      dpidx = srvcp-3;
      TRACE("Selecting a Direct Play SP\n");
      if ((dpidx<0) || (dpidx >= noOfEnumeratedDPlayServices))
      {
          TRACE("WARNING network: LbNetwork_Init() bad DPlay service index\n");
          return -1;
      }
      if ( DPlayInit(guid, &clientGuidTable[dpidx]) )
      {
        TRACE("WARNING network: LbNetwork_Init() failure on DPlayInit()\n");
        return -1;
      }
      break;
  }
  return 0;
}

short inline calculate_moon_phase(short add_to_log)
{
  //Moon phase calculation
  _DK_phase_of_moon=PhaseOfMoon::Calculate();
  if ((_DK_phase_of_moon > -0.05) && (_DK_phase_of_moon < 0.05))
  {
    if (add_to_log)
      LbSyncLog("Full moon %.4f\n", _DK_phase_of_moon);
    game.is_full_moon = 1;
  } else
  {
    if (add_to_log)
      LbSyncLog("Moon phase %.4f\n", _DK_phase_of_moon);
    game.is_full_moon = 0;
  }
  return game.is_full_moon;
}

short show_rawimage_screen(unsigned char *raw,unsigned char *pal,int width,int height,long tmdelay)
{
  static const char *func_name="show_rawimage_screen";
      if (height>lbDisplay.PhysicalScreenHeight)
           height=lbDisplay.PhysicalScreenHeight;
      LbPaletteSet(pal);
      long end_time;
      end_time = LbTimerClock() + tmdelay;
      long tmdelta;
      tmdelta = tmdelay/100;
      if (tmdelta>100) tmdelta=100;
      if (tmdelta<5) tmdelta=5;
      while ( LbTimerClock() < end_time )
      {
          LbWindowsControl();
          if ( LbScreenLock() == 1 )
          {
              unsigned char *raw_line;
              unsigned char *scrn_line;
              raw_line = raw;
              scrn_line = lbDisplay.WScreen;
              int i;
              for ( i = 0; i < height; i++ )
              {
                  memcpy(scrn_line, raw_line, width);
                  raw_line += width;
                  scrn_line += lbDisplay.GraphicsScreenWidth;
              }
              LbScreenUnlock();
          }
          LbScreenSwap();
          if ( lbKeyOn[KC_SPACE] || lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_RETURN] ||
               lbDisplay.LeftButton || lbDisplay.RightButton )
          {
              lbKeyOn[KC_SPACE] = 0;
              lbKeyOn[KC_ESCAPE] = 0;
              lbKeyOn[KC_RETURN] = 0;
              lbDisplay.LeftButton = 0;
              lbDisplay.RightButton = 0;
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
        while ( LbPaletteFade(pal, n, flg) < n )
        {
          if ( !lbKeyOn[KC_SPACE] && !lbKeyOn[KC_ESCAPE] && !lbKeyOn[KC_RETURN] &&
               !lbDisplay.LeftButton && !lbDisplay.RightButton )
          {
            LbSleepUntil(last_loop_time+50);
          }
          last_loop_time = LbTimerClock();
        }
    } else
    if ( pal != NULL )
    {
        LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        LbPaletteSet(palette_buf);
    }
}

void ProperForcedFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg)
{
    if (flg==Lb_PALETTE_FADE_OPEN)
    {
        LbPaletteFade(pal, n, flg);
        return;
    }
    if ( _DK_lbUseSdk )
    {
        long last_loop_time;
        last_loop_time = LbTimerClock();
        while ( LbPaletteFade(pal, n, flg) < n )
        {
          LbSleepUntil(last_loop_time+50);
          last_loop_time = LbTimerClock();
        }
    } else
    if ( pal != NULL )
    {
        LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        LbPaletteSet(palette_buf);
    }
}

short thing_is_special(struct Thing *thing)
{
  return (thing->field_1F==1) && (_DK_object_to_special[thing->field_1A]);
}

short copy_lowres_image_to_screen(const unsigned char *buf,const int img_width,const int img_height)
{
  if (LbScreenLock()!=1)
    return false;
  int w,h;
  unsigned char *dst;
  const unsigned char *src;
  if ((lbDisplay.ScreenMode!=Lb_SCREEN_MODE_640_400_8) &&
      (lbDisplay.ScreenMode!=Lb_SCREEN_MODE_640_480_8))
  {
    dst = lbDisplay.WScreen;
    src = buf;
    for (h=img_height;h>0;h--)
    {
          memcpy(dst, src, img_width);
          src += img_width;
          dst += lbDisplay.GraphicsScreenWidth;
    }
  } else
  {
        if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
        {
          dst = lbDisplay.WScreen + 40*lbDisplay.GraphicsScreenWidth;
        } else
        {
          dst = lbDisplay.WScreen;
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
          memcpy(dst+lbDisplay.GraphicsScreenWidth-640, dst-640, 640);
          dst += (lbDisplay.GraphicsScreenWidth<<1)-640;
        }
  }
  LbScreenUnlock();
  LbScreenSwap();
  return true;
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
  LbPaletteSet(palette_buf);

  check_cd_in_drive();
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
      sprintf(fname, "%s\\%s\\%s",keeper_runtime_directory, "data", "loading.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",keeper_runtime_directory,"data","loading.pal");
      check_cd_in_drive();
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      LbScreenClear(0);
      dproceed=copy_lowres_image_to_screen(buf,img_width,img_height);
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
  LbPaletteSet(palette_buf);

  check_cd_in_drive();
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
      sprintf(fname, "%s\\%s\\%s",keeper_runtime_directory, "fxdata", "loading_640.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      sprintf(fname, "%s\\%s\\%s",keeper_runtime_directory,"fxdata","loading_640.pal");
      check_cd_in_drive();
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      LbScreenClear(0);
      dproceed = (LbScreenLock()==1);
  }
  if ( dproceed )
  {
      int w,h;
      unsigned char *dst;
      unsigned char *src;
      int cp_height;
      if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
      {
          src = buf;
          cp_height=480;
      } else
      {
          src = buf + 40*img_width;
          cp_height=400;
      }
      dst = lbDisplay.WScreen;
      for (h=cp_height;h>0;h--)
      {
          memcpy(dst, src, img_width);
          src += img_width;
          dst += lbDisplay.GraphicsScreenWidth;
      }
      LbScreenUnlock();
      LbScreenSwap();
  }
  free(buf);
  return dproceed;
}

short display_loading_screen(void)
{
    short done=false;
    if ((lbDisplay.ScreenMode==Lb_SCREEN_MODE_640_400_8) ||
        (lbDisplay.ScreenMode==Lb_SCREEN_MODE_640_480_8))
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
  if ( SoundDisabled )
    movie_flags |= 0x01;
  short result=1;

  if ((result)&&(nstate>-2))
  {
    if ( _DK_setup_screen_mode_minimal(Lb_SCREEN_MODE_320_200_8) )
    {
      LbMouseChangeSprite(0);
      LbScreenClear(0);
      LbScreenSwap();
    } else
      result=0;
  }
  if (result)
  {
    // Fail in playing shouldn't set result=0, because result=0 means fatal error.
    if ( play_smk_(filename, 0, movie_flags | 0x100)==0 )
    {
      error(func_name, 2357, "Smacker play error");
      result=0;
    }
  }
  if (nstate>-2)
  {
    if ( !_DK_setup_screen_mode_minimal(Lb_SCREEN_MODE_640_480_8) )
    {
      _DK_FatalError = 1;
      exit_keeper = 1;
      return 0;
    }
  } else
  {
    memset(_DK_frontend_palette, 0, PALETTE_SIZE);
  }
  LbScreenClear(0);
  LbScreenSwap();
  LbPaletteSet(_DK_frontend_palette);
  if ( nstate >= 0 )
    frontend_set_state(nstate);
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
  lbDisplay.MiddleButton = 0;
  if (nstate>-2)
    _DK_LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
  return result;
}

short sound_emitter_in_use(long emidx)
{
  return (emidx!=0) && (_DK_emitter[emidx].flags & 1);
}

char game_is_busy_doing_gui_string_input(void)
{
  return _DK_game_is_busy_doing_gui_string_input();
}

short get_global_inputs()
{
  if ( game_is_busy_doing_gui_string_input() && (_DK_input_button==NULL) )
    return false;
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  struct Packet *pckt;
  long keycode;
  if ( (player->field_0 & 0x04) != 0 )
  {
    if ( (key_modifiers==KM_NONE) && (lbKeyOn[KC_RETURN]) )
    {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt, 14, 0, 0, 0, 0);
      lbKeyOn[KC_RETURN] = 0;
      lbInkey = 0;
      return true;
    }
    lbFontPtr = winfont;
    int msg_width = pixel_size * _DK_LbTextStringWidth(player->strfield_463);
    if ( (lbInkey == 14) || (msg_width < 450) )
    {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt,121,lbInkey,0,0,0);
      lbKeyOn[lbInkey] = 0;
      lbInkey = 0;
    }
    return true;
  }
  if ( player->field_452 == 1 )
  {
      if ( ((game.numfield_A & 0x01) != 0) && (key_modifiers==KM_NONE) && (lbKeyOn[KC_RETURN]) )
      {
        lbKeyOn[KC_RETURN] = 0;
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 13, 0, 0, 0, 0);
        return true;
      }
  }
  int idx;
  for (idx=KC_F1;idx<=KC_F8;idx++)
  {
      if ( (key_modifiers==KM_ALT) && (lbKeyOn[idx]) )
      {
        lbKeyOn[idx] = 0;
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 108, idx-KC_F1, 0, 0, 0);
        return true;
      }
  }
  if ( (player->field_4B0 != 14) && (player->field_4B0 != 15) && (input_button==0) )
  {
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        lbKeyOn[keycode] = 0;
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 22, 0, 0, 0, 0);
        return true;
      }
  }
  if ((game.numfield_C & 0x01) != 0)
      return true;
  if ( (key_modifiers==KM_CONTROL) && (lbKeyOn[KC_ADD]))
  {
        lbKeyOn[KC_ADD] = 0;
        game.timingvar1 += 2;
        if (game.timingvar1 < 0)
          game.timingvar1 = 0;
        else
        if ( game.timingvar1 > 64 )
          game.timingvar1 = 64;
  }
  if ( (key_modifiers==KM_CONTROL) && (lbKeyOn[KC_SUBTRACT]))
  {
      lbKeyOn[KC_SUBTRACT] = 0;
      game.timingvar1 -= 2;
        if (game.timingvar1 < 0)
        game.timingvar1 = 0;
      else
      if ( game.timingvar1 > 64 )
        game.timingvar1 = 64;
  }
  if ( (key_modifiers==KM_NONE) && (lbKeyOn[KC_SUBTRACT]))
  {
      lbKeyOn[KC_SUBTRACT] = 0;
      if ( player->field_450 < 0x800u )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 28, 2 * player->field_450, 0, 0, 0);
      }
  }
  if ( (key_modifiers==KM_NONE) && (lbKeyOn[KC_ADD]))
  {
      lbKeyOn[KC_ADD] = 0;
      if ( player->field_450 > 0x80u )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 28, player->field_450 >> 1, 0, 0, 0);
      }
  }
  if ( (key_modifiers==KM_ALT) && (lbKeyOn[KC_R]) )
  {
      lbKeyOn[KC_R] = 0;
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt, 21, 0, 0, 0, 0);
      return true;
  }
  if ( (key_modifiers==KM_NONE) && (lbKeyOn[KC_SPACE]))
  {
      if ( player->field_29 )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 5, 0, 0, 0, 0);
        lbKeyOn[KC_SPACE] = 0;
        return true;
      }
  }
  if ( lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT] )
  {
      if ( lbKeyOn[KC_M] )
      {
        lbKeyOn[KC_M] = 0;
        if ( game.numfield_A & 0x08 )
        {
          game.numfield_A &= 0xF7u;
          anim_stop();
        } else
        if ( anim_record() )
        {
          game.numfield_A |= 0x08;
        }
      }
      if ( lbKeyOn[KC_C] )
      {
        lbKeyOn[KC_C] = 0;
        game.numfield_A |= 0x10;
      }
  }
  if ( _DK_is_game_key_pressed(29, &keycode, 0) )
  {
      lbKeyOn[keycode] = 0;
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt, 111, 0, 0, 0, 0);
  }
  return false;
}

void play_non_3d_sample(long sample_idx)
{
  static const char *func_name="play_non_3d_sample";
  if ( SoundDisabled )
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
      game.flags_font |= 0x10;
      LbSyncLog("Initializing %d-players serial network.\n",maxplayrs);
      break;
  case 1:
      maxplayrs = 2;
      init_data = &_DK_net_modem_data;
      game.flags_font |= 0x10;
      LbSyncLog("Initializing %d-players modem network.\n",maxplayrs);
      break;
  case 2:
      maxplayrs = 4;
      init_data = NULL;
      game.flags_font &= 0xEFu;
      LbSyncLog("Initializing %d-players IPX network.\n",maxplayrs);
      break;
  default:
      maxplayrs = 4;
      init_data = NULL;
      game.flags_font &= 0xEFu;
      LbSyncLog("Initializing %d-players type %d network.\n",maxplayrs,srvidx);
      break;
  }
  memset(&_DK_net_player_info[0], 0, sizeof(struct TbNetworkPlayerInfo));
  if ( LbNetwork_Init(srvidx, _DK_net_guid, maxplayrs, &_DK_net_screen_packet, 0xCu, &_DK_net_player_info[0], init_data) )
  {
    if (srvidx != 0)
      _DK_process_network_error(-800);
    return 0;
  }
  _DK_net_service_index_selected = srvidx;
  if ( game.flags_font & 0x10 )
    ;//rndseed_nullsub();
  frontend_set_state(5);
  return 1;
}

/*
 * Updates CPU and memory status variables.
 */
short update_memory_constraits(void)
{
  struct _MEMORYSTATUS msbuffer;
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
  return true;
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
  // CPU status variable
  struct CPU_INFO cpu_info;
  char filename[DISKPATH_SIZE];

  // Do only a very basic setup
  _DK_get_cpu_info(&cpu_info);
  update_memory_constraits();

  // Configuration file
  if ( !load_configuration() )
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
  if ( LbScreenSetup(Lb_SCREEN_MODE_640_480_8, LEGAL_WIDTH, LEGAL_HEIGHT, _DK_palette, 1, 0) != 1 )
  {
      error(func_name, 1912, "Screen mode setup error.");
      return 0;
  }

  unsigned char *legal_raw;

  if ( result )
  {
      LbScreenClear(0);
      LbScreenSwap();
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
      LbScreenClear(0);
      LbScreenSwap();
  }

  // View Bullfrog company logo animation when new moon
  if ((_DK_phase_of_moon < -0.95) || (_DK_phase_of_moon > 0.95))
  if ( !game.no_intro )
  {
      sprintf(filename, "fxdata/%s","bullfrog.smk");
      result=play_smacker_file(filename, -2);
      if ( !result )
        error(func_name, 1483, "Unable to play new moon movie");
  }

  result = 1;
  // The 320x200 mode is required only for the intro;
  // loading and no CD screens can run in both 320x2?0 and 640x4?0.
  if ( result && (!game.no_intro) )
  {
    int mode_ok = LbScreenSetup(Lb_SCREEN_MODE_320_200_8, 320, 200, _DK_palette, 2, 0);
    if (mode_ok != 1)
    {
      error(func_name, 1500, "Can't enter 320x200 screen mode");
      result=0;
    }
  }

  if ( result )
  {
      LbScreenClear(0);
      LbScreenSwap();
      result=check_cd_in_drive();
  }
  if ( result && (!game.no_intro) )
  {
      sprintf(filename, "%s/ldata/%s",install_info.inst_path,"intromix.smk");
      result=play_smacker_file(filename, -2);
  }
  if ( !result )
      LbSyncLog("%s - some problem prevented playing intro\n", func_name);
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
      strings_data = (char *)LbMemoryAlloc(filelen + 256);
      if ( strings_data == NULL )
      {
        exit(1);
      }
      text_end = strings_data+filelen+255;
      int loaded_size=LbFileLoadAt("data/text.dat", strings_data);
      if (loaded_size<2*STRINGS_MAX)
      {
          error(func_name, 1501, "Strings data too small");
          result = 0;
      }
  }
  if ( result )
  {
      char **text_arr = strings;
      int text_idx;
      char *text_ptr;
      text_idx = STRINGS_MAX;
      text_ptr = strings_data;
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
  if ( lbDisplay.LeftButton )
  {
    _DK_left_button_held = 1;
    _DK_left_button_held_x = lbDisplay.MMouseX * pixel_size;
    _DK_left_button_held_y = lbDisplay.MMouseY * pixel_size;
  }
  if (_DK_left_button_held)
  {
    if (!lbDisplay.MLeftButton)
    {
      _DK_left_button_released = 1;
      _DK_left_button_held = 0;
      _DK_left_button_released_x = lbDisplay.MMouseX * pixel_size;
      _DK_left_button_released_y = lbDisplay.MMouseY * pixel_size;
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
  if (lbDisplay.RightButton)
  {
    _DK_right_button_held = 1;
    _DK_right_button_held_x = lbDisplay.MMouseX * pixel_size;
    _DK_right_button_held_y = lbDisplay.MMouseY * pixel_size;
  }
  if ( _DK_right_button_held )
  {
    if ( !lbDisplay.MRightButton )
    {
      _DK_right_button_released = 1;
      _DK_right_button_held = 0;
      _DK_right_button_released_x = lbDisplay.MMouseX * pixel_size;
      _DK_right_button_released_y = lbDisplay.MMouseY * pixel_size;
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
  _DK_left_button_clicked = lbDisplay.LeftButton;
  _DK_left_button_clicked_x = lbDisplay.MouseX * pixel_size;
  _DK_left_button_clicked_y = lbDisplay.MouseY * pixel_size;
}

void update_right_button_clicked(void)
{
  _DK_right_button_clicked = lbDisplay.RightButton;
  _DK_right_button_clicked_x = lbDisplay.MouseX * pixel_size;
  _DK_right_button_clicked_y = lbDisplay.MouseY * pixel_size;
}

void update_mouse(void)
{
  update_left_button_released();
  update_right_button_released();
  update_left_button_clicked();
  update_right_button_clicked();
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
}

void output_message(long idx, long a, long b)
{
  _DK_output_message(idx, a, b);
}

void input_eastegg(void)
{
  static const char *func_name="input_eastegg";
  //_DK_input_eastegg();return;
  // Maintain the FECKOFF cheat
  if ( lbKeyOn[KC_LSHIFT] )
  {
    switch (game.eastegg01_cntr)
    {
    case 0:
      if ( lbKeyOn[KC_F] )
        game.eastegg01_cntr++;
      break;
    case 1:
      if ( lbKeyOn[KC_E] )
        game.eastegg01_cntr++;
      break;
    case 2:
      if ( lbKeyOn[KC_C] )
        game.eastegg01_cntr++;
      break;
    case 3:
      if ( lbKeyOn[KC_K] )
        game.eastegg01_cntr++;
      break;
    case 4:
      if ( lbKeyOn[KC_O] )
        game.eastegg01_cntr++;
      break;
    case 5:
    case 6:
      if ( lbKeyOn[KC_F] )
        game.eastegg01_cntr++;
      break;
    }
  } else
  {
    game.eastegg01_cntr = 0;
  }
  // Maintain the JLW cheat
  if ( game.flags_font & 0x20 )
  {
    if ( lbKeyOn[KC_LSHIFT] && lbKeyOn[KC_RSHIFT] )
    {
      switch (game.eastegg02_cntr)
      {
      case 0:
        if ( lbKeyOn[KC_J] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      case 1:
        if ( lbKeyOn[KC_L] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      case 2:
        if ( lbKeyOn[KC_W] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      }
    } else
    {
      game.eastegg02_cntr = 0;
    }
  }
  // Maintain the SKEKSIS cheat
  if ( lbKeyOn[KC_LSHIFT] )
  {
    switch (_DK_eastegg03_cntr)
    {
    case 0:
    case 4:
      if ( lbInkey==KC_S )
      {
        _DK_eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 1:
    case 3:
      if ( lbInkey==KC_K )
      {
        _DK_eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 2:
      if ( lbInkey==KC_E )
      {
        _DK_eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 5:
      if ( lbInkey==KC_I )
      {
        _DK_eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 6:
      if ( lbInkey==KC_S )
      {
        _DK_eastegg03_cntr++;
        lbInkey=0;
        //'Your pants are definitely too tight'
        output_message(94, 0, 1);
      }
      break;
    }
  } else
  {
    _DK_eastegg03_cntr = 0;
  }
  if (lbInkey!=0)
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
  int mouse_x = lbDisplay.MMouseX*pixel_size;
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

long S3DSetSoundReceiverPosition(int pos_x, int pos_y, int pos_z)
{
  return _DK_S3DSetSoundReceiverPosition(pos_x, pos_y, pos_z);
}

long S3DSetSoundReceiverOrientation(int ori_a, int ori_b, int ori_c)
{
  return _DK_S3DSetSoundReceiverOrientation(ori_a, ori_b, ori_c);
}

short setup_trap_tooltips(struct Coord3d *pos)
{
    struct Thing *thing=_DK_get_trap_for_position(_DK_map_to_slab[pos->x.stl.num],_DK_map_to_slab[pos->y.stl.num]);
    if (thing==NULL) return false;
    struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
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
      sprintf(_DK_tool_tip_box.text,"%s",strings[stridx]);
      _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
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
  static char text[2048];
  struct Thing *thing = NULL;
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  thing = game.things_lookup[player->field_35];
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
    sprintf(_DK_tool_tip_box.text,"%s",strings[stridx]);
    _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
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
      sprintf(_DK_tool_tip_box.text,"%s",strings[stridx]);
      _DK_tool_tip_box.field_0 = 1;
      _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
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
    sprintf(_DK_tool_tip_box.text,"%s",strings[stridx]);
    _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
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
      sprintf(text,"%s",strings[545]);
    } else
    if (crtridx = _DK_objects[objidx].field_13)
    {
      int stridx=_DK_creature_data[crtridx].field_3;
      sprintf(text, "%s %s", strings[stridx], strings[609]);
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
      _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
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
  int attridx = game.slabmap[slab_idx].field_0;
  int stridx = _DK_slab_attrs[attridx].field_0;
  if (stridx==201)
    return false;
  if ((void *)attridx != _DK_tool_tip_box.target)
  {
    _DK_help_tip_time = 0;
    _DK_tool_tip_box.target = (void *)attridx;
  }
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ((_DK_help_tip_time>20) || (player->field_453==12))
  {
    _DK_tool_tip_box.field_0 = 1;
    sprintf(_DK_tool_tip_box.text, "%s", strings[stridx]);
    _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
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
  room = &game.rooms[game.slabmap[slab_idx].field_3];
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
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  int widener=0;
  if ( (_DK_help_tip_time>20) || (player->field_453==12) )
  {
      if ( room->field_A >= 2 )
      {
        if ( (room->field_A<=14) || (room->field_A==16) )
          widener = 0;
      }
      sprintf(_DK_tool_tip_box.text, "%s", strings[stridx]);
      _DK_tool_tip_box.field_0 = 1;
      _DK_tool_tip_box.pos_x = lbDisplay.MMouseX * pixel_size;
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
  if ((game.numfield_C & 0x20) == 0)
    mask=0x20;
  else
    mask=0;
  game.numfield_C = (game.numfield_C & 0xDFu) | mask;
}

short engine_point_to_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  *map_x = 0;
  *map_y = 0;
  if ( (_DK_pointer_x >= 0) && (_DK_pointer_y >= 0)
    && (_DK_pointer_x < (player->field_444/pixel_size))
    && (_DK_pointer_y < (player->field_446/pixel_size)) )
  {
    int i;
    i = player->field_453;
    if ( ((i == 1) && (player->field_454)) || (i == 2) ||
      (i == 18) || (i == 16) || (i == 8) || (i == 23) )
    {
          *map_x = ( top_pointed_at_x << 8) + top_pointed_at_frac_x;
          *map_y = ( top_pointed_at_y << 8) + top_pointed_at_frac_y;
    } else
    {
          *map_x = (block_pointed_at_x << 8) + pointed_at_frac_x;
          *map_y = (block_pointed_at_y << 8) + pointed_at_frac_y;
    }
    // Clipping coordinates
    if ( *map_y < 0 )
      *map_y = 0;
    else
    if ( *map_y > 0xFEFFu )
      *map_y = 0xFEFFu;
    if ( *map_x < 0 )
      *map_x = 0;
    else
    if ( *map_x > 0xFEFFu )
      *map_x = 0xFEFFu;
    return true;
  }
  return false;
}

short point_to_overhead_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  *map_x = 0;
  *map_y = 0;
  if ((screen_x >= 150) && (screen_x < 490)
    && (screen_y >= 56) && (screen_y < 396))
  {
    *map_x = 3*256 * (screen_x-150) / 4 + 384;
    *map_y = 3*256 * (screen_y-56) / 4 + 384;
    return ((*map_x >= 0) && (*map_x < (map_subtiles_x+1)<<8) && (*map_y >= 0) && (*map_y < (map_subtiles_y+1)<<8));
  }
  return false;
}

short screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos)
{
//return _DK_screen_to_map(camera,screen_x,screen_y,mappos);
  short result;
  long x,y;
  switch ( camera->field_6 )
  {
    case 1:
    case 2:
    case 5:
      // 3D view mode
      result=engine_point_to_map(camera,screen_x,screen_y,&x,&y);
      break;
    case 3: //map mode
      result=point_to_overhead_map(camera,screen_x,screen_y,&x,&y);
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
    return lbKeyOn[key];
  return 0;
}

short check_cd_in_drive(void)
{
    static const char *func_name="check_cd_in_drive";
//  _DK_check_cd_in_drive(); return;
  static char src_fname[256];
  const int img_width = 320;
  const int img_height = 200;
  short was_locked = LbScreenIsLocked();
  sprintf(src_fname, "%s/%s/%s", install_info.inst_path, "ldata","dkwind00.dat");
  if ( LbFileExists(src_fname) )
    return true;
  if ( was_locked )
    LbScreenUnlock();
  if ( _DK_LbDataLoadAll(nocd_load_files) )
  {
      error(func_name, 78, "Unable to test for CD in drive");
      return false;
  }
  LbSyncLog("CD not found in drive, waiting\n");
  LbScreenClear(0);
  LbPaletteSet(nocd_pal);
  unsigned long counter;
  counter=0;
  while ( !exit_keeper )
  {
      if ( LbFileExists(src_fname) )
        break;
      copy_lowres_image_to_screen(nocd_raw,img_width,img_height);
      while ( (!_DK_LbIsActive()) && (!exit_keeper) && (!quit_game) )
      {
        if (!LbWindowsControl())
          exit_keeper = 1;
      }
      if (counter>300)
      {
          error(func_name, 79, "Wait time too long, giving up");
          exit_keeper = 1;
      }
      if ( lbKeyOn[KC_Q] )
      {
          error(func_name, 77, "User requested quit, giving up");
          lbKeyOn[KC_Q] = 0;
          exit_keeper = 1;
      }
      LbSleepFor(1000);
      counter++;
  }
  LbSyncLog("Finished waiting for CD after %lu seconds\n",counter);
  _DK_LbDataFreeAll(nocd_load_files);
  if ( was_locked )
    LbScreenLock();
  return (!exit_keeper);
}

short get_gui_inputs(short gameplay_on)
{
  //return _DK_get_gui_inputs(gameplay_on);
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
  if ( !lbDisplay.MLeftButton )
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
  gidx = point_is_over_gui_menu(lbDisplay.MMouseX * pixel_size, _DK_GetMouseY());
  if ( gidx == -1 )
    _DK_busy_doing_gui = 0;
  else
    _DK_busy_doing_gui = 1;
  int fmmenu_idx = first_monopoly_menu();

  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
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
      int mouse_x = lbDisplay.MMouseX*pixel_size;
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
        _DK_tool_tip_box.pos_x = lbDisplay.MMouseX*pixel_size;
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
    int mouse_x = lbDisplay.MMouseX*pixel_size;
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
    if ( lbDisplay.MLeftButton )
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
    if ( lbDisplay.MRightButton )
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
      if (game.field_1516F3 != 0)
      {
        if (gbtn->id_num == game.field_1516F3)
          game.field_1516F3 = 0;
      }
      callback = gbtn->click_event;
      if ((callback!=NULL) || (gbtn->field_0 & 2) || (gbtn->field_2F) || (gbtn->gbtype==3) )
      {
        _DK_left_button_clicked = 0;
        _DK_gui_last_left_button_pressed_id = gbtn->id_num;
        do_button_click_actions(gbtn, &gbtn->field_1, callback);
      }
    } else
    if ( _DK_right_button_clicked )
    {
      result = 1;
      if (game.field_1516F3 != 0)
      {
        if (gbtn->id_num == game.field_1516F3)
          game.field_1516F3 = 0;
      }
      callback = gbtn->rclick_event;
      if ((callback!=NULL))
      {
        _DK_right_button_clicked = 0;
        _DK_gui_last_right_button_pressed_id = gbtn->id_num;
        do_button_click_actions(gbtn, &gbtn->field_2, callback);
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
        do_button_release_actions(gbtn, &gbtn->field_1, callback);
      }
    } else
    if ((gbtn->field_2) && (_DK_right_button_released))
    {
      callback = gbtn->rclick_event;
      result = 1;
      if ( callback!=NULL )
      {
        _DK_right_button_released = 0;
        do_button_release_actions(gbtn, &gbtn->field_2, callback);
      }
    }
  }
  if ((gameplay_on) && (_DK_tool_tip_time==0) && (!_DK_busy_doing_gui))
  {
        int mouse_x = lbDisplay.MMouseX*pixel_size;
        int mouse_y = _DK_GetMouseY();
        struct Coord3d mappos;
        if ( screen_to_map(player->camera,mouse_x,mouse_y,&mappos) )
        {
          int bblock_x=mappos.x.stl.num;
          int bblock_y=mappos.y.stl.num;
          unsigned short bitval;
          // Get the top four bits
          bitval = (game.map[bblock_x+bblock_y*(map_subtiles_x+1)].field_3) >> 12;
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
  if ( ((game.numfield_C & 0x01)==0) || (game.numfield_C & 0x80) )
      get_gui_inputs(1);
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ( !player->field_2F )
    return false;
  if (_DK_right_button_released)
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    _DK_set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  struct Thing *thing;
  thing = game.things_lookup[player->field_2F];
  if ((player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0) )
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    _DK_set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  if ( is_key_pressed(KC_TAB,0) )
  {
    lbKeyOn[KC_TAB] = 0;
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
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ( (game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320) )
  {
      _DK_toggle_status_menu((game.numfield_C & 0x40) != 0);
      struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt, 120,1,0,0,0);
  } else
  {
      struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt, 80,6,0,0,0);
  }
}

short get_map_action_inputs()
{
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  long keycode;
  long mouse_x = lbDisplay.MMouseX * pixel_size;
  long mouse_y = _DK_GetMouseY();
  int mappos_x = 3 * (mouse_x - 150) / 4 + 1;
  int mappos_y = 3 * (mouse_y - 56) / 4 + 1;
  if ((mappos_x >= 0) && (mappos_x < map_subtiles_x) && (mappos_y >= 0) && (mappos_y < map_subtiles_y) )
  {
    if ( _DK_left_button_clicked )
    {
      _DK_left_button_clicked = 0;
    }
    if ( _DK_left_button_released )
    {
      struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
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
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    if ( pckt->field_5 )
      return true;
    if ( lbKeyOn[KC_F8] )
    {
      lbKeyOn[KC_F8] = 0;
      toggle_tooltips();
    }
    if ( lbKeyOn[KC_NUMPADENTER] )
    {
      lbKeyOn[KC_NUMPADENTER] = 0;
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
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
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
    game.numfield_C |= 0x20;
  else
    game.numfield_C &= 0xDF;
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  unsigned char is_visbl = ((game.numfield_C & 0x20)!=0);
  if (player->field_452 == 2)
    toggle_first_person_menu(is_visbl);
  else
    _DK_toggle_status_menu(is_visbl);
  if ( (game.numfield_D & 0x20) && (game.numfield_C & 0x20) )
    _DK_setup_engine_window(140, 0, MyScreenWidth, MyScreenHeight);
  else
    _DK_setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
}

void toggle_gui(void)
{
  int visible=((game.numfield_C & 0x20)==0);
  set_gui_visible(visible);
}

long creature_instance_is_available(struct Thing *thing, long inum)
{
  struct CreatureControl *crctrl;
  crctrl = game.creature_control_lookup[thing->field_64];
  return crctrl->instances[inum];
}

short toggle_computer_player(int idx)
{
  if (game.dungeon[idx].computer_enabled & 0x01)
    game.dungeon[idx].computer_enabled &= 0xFE;
  else
    game.dungeon[idx].computer_enabled |= 0x01;
  return 1;
}

unsigned short checksums_different(void)
{
  struct PlayerInfo *player;
  int i;
  int first = -1;
  unsigned short chk = 0;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i]);
    if (((player->field_0 & 0x01) != 0) && ((player->field_0 & 0x40) == 0))
    {
        if (first == -1)
        {
          first = game.packets[player->field_B].field_4;
        } else
        if (game.packets[player->field_B].field_4 != first)
          chk = 1;
    }
  }
  return chk;
}

short get_creature_control_action_inputs(void)
{
  if ( ((game.numfield_C & 0x01)==0) || (game.numfield_C & 0x80) )
    get_gui_inputs(1);
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  long keycode;
  if ( lbKeyOn[KC_NUMPADENTER] )
  {
      lbKeyOn[KC_NUMPADENTER] = 0;
      // Toggle cheat menu
/*
      if ( (_DK_gui_box==NULL) || (_DK_gui_box_is_not_valid(_DK_gui_box)) )
      {
        gui_box=gui_create_box(?,?,_DK_gui_instance_option_list); / 200,20 or 20,200
        player->unknownbyte  |= 0x08;
        game.unknownbyte |= 0x08;
      } else
      {
        gui_delete_box(_DK_gui_box);
        player->unknownbyte &= 0xF7;
        game.unknownbyte &= 0xF7;
      }
*/
      return 1;
  }
  if ( lbKeyOn[KC_F12] )
  {
      lbKeyOn[KC_F12] = 0;
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
      thing = game.things_lookup[player->field_2F];
      if ( (player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0)
         || (thing->field_7 == 67) )
        make_packet = true;
    }
    if (make_packet)
    {
      struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_right_button_released = 0;
      _DK_set_packet_action(pckt, 33, player->field_2F,0,0,0);
    }
  }
  if ( is_key_pressed(KC_TAB,0) )
  {
    lbKeyOn[KC_TAB] = 0;
    toggle_gui();
  }
  int numkey;
  numkey = -1;
  {
    for (keycode=KC_1;keycode<=KC_0;keycode++)
    {
      if ( is_key_pressed(keycode,0) )
      {
        lbKeyOn[keycode] = 0;
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
      thing = game.things_lookup[player->field_2F];
      int instnce = game.creature_stats[thing->field_1A].field_80[idx];
      if ( creature_instance_is_available(thing,instnce) )
      {
        if ( numkey == num_avail )
        {
          struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
          _DK_set_packet_action(pckt, 39, instnce,0,0,0);
          break;
        }
        num_avail++;
      }
    }
  }
  return false;
}

void get_level_lost_inputs(void)
{
//  _DK_get_level_lost_inputs();
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  long keycode;
  if ( player->field_0 & 4 )
  {
    if (is_key_pressed(KC_RETURN,0))
    {
      struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt, 14, 0,0,0,0);
      lbInkey = 0;
      lbKeyOn[KC_RETURN] = 0;
    } else
    {
      lbFontPtr = _DK_winfont;
      if ( (lbInkey == KC_BACK) || (pixel_size*_DK_LbTextStringWidth(player->strfield_463) < 450) )
      {
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 121, lbInkey,0,0,0);
        lbKeyOn[lbInkey] = 0;
        lbInkey = 0;
      }
    }
    return;
  }
  if ((game.numfield_A & 0x01) != 0)
  {
    if (is_key_pressed(KC_RETURN,0))
    {
      struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt, 13, 0,0,0,0);
      lbKeyOn[KC_RETURN] = 0;
      return;
    }
  }
  if (is_key_pressed(KC_SPACE,0))
  {
    struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
    _DK_set_packet_action(pckt, 5, 0,0,0,0);
    lbKeyOn[KC_SPACE] = 0;
  }


  if ( player->field_452 == 4 )
  {
    int screen_x = lbDisplay.MMouseX * pixel_size - 150;
    int screen_y = _DK_GetMouseY() - 56;
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      if (((game.numfield_A & 0x01) == 0) && (lbDisplay.PhysicalScreenWidth <= 320))
      {
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 80, 6,0,0,0);
      }
      else
      {
        _DK_toggle_status_menu((game.numfield_C & 0x40) != 0);
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        _DK_set_packet_action(pckt, 120, 1,0,0,0);
      }
    } else
    if ( _DK_right_button_released )
    {
        _DK_right_button_released = 0;
        if ( (game.numfield_A & 0x01) || lbDisplay.PhysicalScreenWidth > 320 )
        {
          _DK_toggle_status_menu((game.numfield_C & 0x40) != 0);
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          _DK_set_packet_action(pckt, 120, 1,0,0,0);
        }
        else
        {
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          _DK_set_packet_action(pckt, 80, 6,0,0,0);
        }
    } else
    if ( _DK_left_button_released )
    {
        int actn_x = 3*screen_x/4 + 1;
        int actn_y = 3*screen_y/4 + 1;
        if  ((actn_x >= 0) && (actn_x < map_subtiles_x) && (actn_y >= 0) && (actn_y < map_subtiles_y))
        {
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          _DK_set_packet_action(pckt, 81, actn_x,actn_y,0,0);
          _DK_left_button_released = 0;
        }
    }
  } else
  if ( player->field_452 == 1 )
  {
    if ( lbKeyOn[KC_TAB] )
    {
        if ((player->field_37 == 2) || (player->field_37 == 5))
        {
          lbKeyOn[KC_TAB] = 0;
          toggle_gui();
        }
    } else
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      if (player->field_37 != 7)
      {
        _DK_turn_off_all_window_menus();
        game.numfield_C = (game.numfield_C ^ (unsigned __int8)(2 * game.numfield_C)) & 0x40 ^ game.numfield_C;
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        if ((game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320))
        {
              if (_DK_toggle_status_menu(0))
                game.numfield_C |= 0x40;
              else
                game.numfield_C &= 0xBF;
              _DK_set_packet_action(pckt, 119, 4,0,0,0);
        } else
        {
              _DK_set_packet_action(pckt, 80, 5,0,0,0);
        }
        _DK_turn_off_roaming_menus();
      }
    }
  }
  if ( lbKeyOn[KC_ESCAPE] )
  {
    lbKeyOn[KC_ESCAPE] = 0;
    if ( _DK_a_menu_window_is_active() )
      _DK_turn_off_all_window_menus();
    else
      _DK_turn_on_menu(8);
  }
  struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
  struct Thing *thing;
  short inp_done=false;
  switch (player->field_452)
  {
    case 1:
      inp_done=_DK_menu_is_active(38);
      if ( !inp_done )
      {
        if ((game.numfield_C & 0x20) != 0)
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
          inp_done=_DK_get_small_map_inputs(player->mouse_x, player->mouse_y, player->field_450 / (3-pixel_size));
          if ( !inp_done )
            _DK_get_bookmark_inputs();
          _DK_get_dungeon_control_nonaction_inputs();
        }
      }
      break;
    case 2:
      thing = game.things_lookup[player->field_2F];
      if (thing->field_1F == 5)
      {
        struct CreatureControl *crctrl;
        crctrl = game.creature_control_lookup[thing->field_64];
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
        if ((game.numfield_C & 0x20) != 0)
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
  long keycode;
  if ( game.flags_cd & 0x01 )
  {
    _DK_load_packets_for_turn(game.gameturn);
    game.gameturn++;
    if ( lbKeyOn[KC_ESCAPE] || lbKeyOn[KC_SPACE] || lbKeyOn[KC_RETURN]
       || (lbKeyOn[KC_LALT] && lbKeyOn[KC_X]) || _DK_left_button_clicked )
    {
      lbKeyOn[KC_ESCAPE] = 0;
      lbKeyOn[KC_SPACE] = 0;
      lbKeyOn[KC_RETURN] = 0;
      lbKeyOn[KC_X] = 0;
      _DK_left_button_clicked = 0;
      quit_game = 1;
    }
    return false;
  }
  if ( game.field_149E81 )
  {
    _DK_load_packets_for_turn(game.gameturn);
    game.gameturn++;
    if ( lbKeyOn[KC_LALT] && lbKeyOn[KC_X] )
    {
      lbKeyOn[KC_X] = 0;
      if ( game.numfield_A & 0x01 )
        _DK_LbNetwork_Stop();
      quit_game = 1;
      exit_keeper = 1;
    }
    return false;
  }
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ( player->field_0 & 0x80 )
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    pckt->field_A = 127;
    pckt->field_C = 127;
    if ((_DK_input_button==NULL) && (game.numfield_C & 0x01))
    {
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        lbKeyOn[keycode] = 0;
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
    thing = game.things_lookup[player->field_2F];
    if ( (thing <= game.things_lookup[0]) || (thing->field_1F != 5) )
    {
      get_level_lost_inputs();
      return true;
    }
    struct CreatureControl *crctrl;
    crctrl = game.creature_control_lookup[thing->field_64];
    if ((crctrl->field_2 & 2) == 0)
    {
      get_level_lost_inputs();
      return true;
    }
  }
  short inp_handled = false;
  if ( !(game.numfield_C & 0x01) || (game.numfield_C & 0x80) )
    inp_handled = get_gui_inputs(1);
  if ( !inp_handled )
    inp_handled = get_global_inputs();
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
      if ( !(game.numfield_A & 0x01) )
        game.numfield_C &= 0xFE;
      if ( _DK_toggle_status_menu(0) )
        player->field_1 |= 0x01;
      else
        player->field_1 &= 0xFE;
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      _DK_set_packet_action(pckt, 80, 4,0,0,0);
      return false;
  case 6:
      if (player->field_37 != 7)
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
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
  if ( lbKeyOn[KC_LSHIFT] || lbKeyOn[KC_RSHIFT] )
    key_mods |= KM_SHIFT;
  if ( lbKeyOn[KC_LCONTROL] || lbKeyOn[KC_RCONTROL] )
    key_mods |= KM_CONTROL;
  if ( lbKeyOn[KC_LALT] || lbKeyOn[KC_RALT] )
    key_mods |= KM_ALT;
  _DK_key_modifiers = key_mods;
  if ( _DK_input_button )
  {
    if ( lbInkey )
      lbKeyOn[lbInkey] = 0;
  }

  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
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

int LbNetwork_Exchange(struct Packet *pckt)
{
  return _DK_LbNetwork_Exchange(pckt);
}

unsigned long get_packet_save_checksum(void)
{
  return _DK_get_packet_save_checksum();
}

void resync_game(void)
{
  return _DK_resync_game();
}

char process_players_global_packet_action(long idx)
{
  return _DK_process_players_global_packet_action(idx);
}

void process_players_dungeon_control_packet_control(long idx)
{
  _DK_process_players_dungeon_control_packet_control(idx);
}

void process_players_dungeon_control_packet_action(long idx)
{
  _DK_process_players_dungeon_control_packet_action(idx);
}

void process_players_creature_control_packet_control(long idx)
{
  _DK_process_players_creature_control_packet_control(idx);
}

void process_players_creature_control_packet_action(long idx)
{
  _DK_process_players_creature_control_packet_action(idx);
}

void save_packets(void)
{
  static const char *func_name="save_packets";
  unsigned char *pckt_buf[sizeof(struct Packet)*PACKETS_COUNT];
  unsigned long chksum;
  int i;
  if (game.packet_checksum)
    chksum = get_packet_save_checksum();
  else
    chksum = 0;
  LbFileSeek(game.packet_save, 0, 2);
  // Note: originally only 48 bytes were saved; I guess it was a mistake (now 55 are saved).
  for (i=0; i<PACKETS_COUNT; i++)
    memcpy(pckt_buf[sizeof(struct Packet)*i], &game.packets[i], sizeof(struct Packet));
  LbFileWrite(game.packet_save, &pckt_buf, sizeof(struct Packet)*PACKETS_COUNT);
  if ( !LbFileFlush(game.packet_save) )
    error(func_name, 3821, "Unable to flush PacketSave File");
}

void process_players_message_character(struct PlayerInfo *player)
{
  struct Packet *pcktd;
  struct PlayerInfo *playerd;
  char chr;
  int chpos;
  playerd = &(game.players[player->field_2B%PLAYERS_COUNT]);
  pcktd = &game.packets[playerd->field_B%PACKETS_COUNT];
  if (pcktd->field_6 >= 0)
  {
    chr = lbInkeyToAscii[pcktd->field_6];
    chpos = strlen(player->strfield_463) - 1;
    if (pcktd->field_6 == 14)
    {
      if (chpos>0)
        player->strfield_463[chpos-1] = 0;
    } else
    if ((chr & 0x80) == 0)
    {
      if ((chr >= 'a') && (chr <= 'z') ||
          (chr >= 'A') && (chr <= 'Z') ||
          (chr >= '0') && (chr <= '9') || (chr == ' '))
      {
        if (chpos < 62)
        {
          player->strfield_463[chpos] = toupper(chr);
          player->strfield_463[chpos+1] = '\0';
        }
      }
    }
  }
}

void process_map_packet_clicks(long idx)
{
  _DK_process_map_packet_clicks(idx);
}

void set_mouse_light(struct PlayerInfo *player)
{
  _DK_set_mouse_light(player);
}

void process_players_map_packet_control(long idx)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  unsigned short x,y;
  player=&(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->field_B%PACKETS_COUNT];
  x = (3*pckt->field_A - 450)/4 - 6;
  y = (3*pckt->field_C - 168)/4 - 6;
  process_map_packet_clicks(idx);
  player->field_90 = (x << 8) + 1920;
  player->field_92 = (y << 8) + 1920;
  set_mouse_light(player);
}

void process_players_creature_passenger_packet_action(long idx)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  player=&(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->field_B%PACKETS_COUNT];
  if (pckt->field_5 == 32)
  {
    player->field_43E = pckt->field_6;
    _DK_set_player_instance(player, 8, 0);
  }
}

void process_players_packet(long idx)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  player=&(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->field_B%PACKETS_COUNT];
  player->field_4 = (pckt->field_10 & 0x20) >> 5;
  player->field_5 = (pckt->field_10 & 0x40) >> 6;
  if ( (player->field_0 & 0x04) && (pckt->field_5 == 121))
  {
     process_players_message_character(player);
  } else
  if ( !process_players_global_packet_action(idx) )
  {
      switch (player->field_452)
      {
      case 1:
        process_players_dungeon_control_packet_control(idx);
        process_players_dungeon_control_packet_action(idx);
        break;
      case 2:
        process_players_creature_control_packet_control(idx);
        process_players_creature_control_packet_action(idx);
        break;
      case 3:
        process_players_creature_passenger_packet_action(idx);
        break;
      case 4:
        process_players_map_packet_control(idx);
        break;
      default:
        break;
      }
  }
}

void process_packets(void)
{
  //_DK_process_packets();return;
  static const char *func_name="process_packets";
  int i,j,k;
  struct Packet *pckt;
  struct PlayerInfo *player;
  // Do the network data exchange
  lbDisplay.DrawColour = colours[15][15][15];
  // Exchange packets with the network
  if ( game.flagfield_14EA4A != 2 )
  {
    player=&(game.players[my_player_number%PLAYERS_COUNT]);
    j=0;
    for (i=0; i<4; i++)
    {
      if (net_player_info[i].field_20 != 0)
        j++;
    }
    if ( !game.field_149E81 || game.numfield_149F47 )
    {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      if (LbNetwork_Exchange(pckt) != 0)
      {
        error(func_name, 426, "LbNetwork_Exchange failed");
      }
    }
    k=0;
    for (i=0; i<4; i++)
    {
      if (net_player_info[i].field_20 != 0)
        k++;
    }
    if (j != k)
    {
      for (i=0; i<4; i++)
      {
        player=&(game.players[i]);
        if (net_player_info[player->field_B].field_20 == 0)
        {
          player->field_0 |= 0x40u;
          toggle_computer_player(i);
        }
      }
    }
  }
  // Setting checksum problem flags
  switch (checksums_different())
  {
  case 1:
    game.numfield_A |= 0x02;
    game.numfield_A &= 0xFB;
    break;
  case 2:
    game.numfield_A |= 0x04;
    game.numfield_A &= 0xFD;
    break;
  case 3:
    game.numfield_A |= 0x04;
    game.numfield_A |= 0x02;
    break;
  default:
    game.numfield_A &= 0xFD;
    game.numfield_A &= 0xFB;
    break;
  }
  // Write packets into file, if requested
  if ((game.field_149E80) && (game.packet_fopened))
    save_packets();
  // Process the packets
  for (i=0; i<PACKETS_COUNT; i++)
  {
    player=&(game.players[i]);
    if (((player->field_0 & 0x01) != 0) && ((player->field_0 & 0x40) == 0))
      process_players_packet(i);
  }
  // Clear all packets
  for (i=0; i<PACKETS_COUNT; i++)
    memset(&game.packets[i], 0, sizeof(struct Packet));
  if ((game.numfield_A & 0x02) || (game.numfield_A & 0x04))
  {
    // Note: the message is now displayed in keeper_gameplay_loop()
    //sprintf(text, "OUT OF SYNC (GameTurn %d)", game_seedchk_random_used);
    resync_game();
  }
}

void update_things(void)
{
  _DK_update_things();
}

void process_rooms(void)
{
  _DK_process_rooms();
}

void process_dungeons(void)
{
  _DK_process_dungeons();
}

void process_messages(void)
{
  _DK_process_messages();
}

void find_nearest_rooms_for_ambient_sound(void)
{
  _DK_find_nearest_rooms_for_ambient_sound();
}

void light_render_area(int startx, int starty, int endx, int endy)
{
  _DK_light_render_area(startx, starty, endx, endy);
}

void process_3d_sounds(void)
{
  _DK_process_3d_sounds();
}

void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal)
{
  _DK_PaletteSetPlayerPalette(player, pal);
}

void process_player_research(int plr_idx)
{
  _DK_process_player_research(plr_idx);
}

long process_player_manufacturing(int plr_idx)
{
  return _DK_process_player_manufacturing(plr_idx);
}

void event_process_events(void)
{
  _DK_event_process_events();
}

void update_all_events(void)
{
  _DK_update_all_events();
}

void process_computer_players2(void)
{
  _DK_process_computer_players2();
}

void process_level_script(void)
{
  _DK_process_level_script();
}

long process_action_points(void)
{
  return _DK_process_action_points();
}

long PaletteFadePlayer(struct PlayerInfo *player)
{
  return _DK_PaletteFadePlayer(player);
}

void message_update(void)
{
  _DK_message_update();
}

void process_player_instance(struct PlayerInfo *player)
{
  _DK_process_player_instance(player);
}

void process_player_instances(void)
{
  //_DK_process_player_instances();return;
  int i;
  struct PlayerInfo *player;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i]);
    if (player->field_0 & 0x01)
      process_player_instance(player);
  }
}

long wander_point_update(struct Wander *wandr)
{
  return _DK_wander_point_update(wandr);
}

void update_player_camera(struct PlayerInfo *player)
{
  _DK_update_player_camera(player);
}

void update_research(void)
{
  int i;
  struct PlayerInfo *player;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) && (player->field_2C == 1))
          process_player_research(i);
  }
}

void update_manufacturing(void)
{
  int i;
  struct PlayerInfo *player;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) && (player->field_2C == 1))
          process_player_manufacturing(i);
  }
}

void update_all_players_cameras(void)
{
  int i;
  struct PlayerInfo *player;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i]);
    if ((player->field_0 & 0x01) && ((player->field_0 & 0x40) == 0))
    {
          update_player_camera(player);
    }
  }
}

#define LIGHT_MAX_RANGE 30
void update_light_render_area(void)
{
  int subtile_x,subtile_y;
  int delta_x,delta_y;
  int startx,endx,starty,endy;
  struct PlayerInfo *player;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (player->field_37 >= 1)
    if ((player->field_37 <= 2) || (player->field_37 == 5))
    {
        game.field_14BB5D = LIGHT_MAX_RANGE;
        game.field_14BB59 = LIGHT_MAX_RANGE;
    }
  delta_x=abs(game.field_14BB59);
  delta_y=abs(game.field_14BB5D);
  // Prepare the area constraits
  subtile_y = (player->camera->pos_y >> 8);
  subtile_x = (player->camera->pos_x >> 8);
//LbSyncLog("LghtRng %d,%d CamTil %d,%d\n",game.field_14BB59,game.field_14BB5D,tile_x,tile_y);
  if (subtile_y > delta_y)
  {
    starty = subtile_y - delta_y;
    if (starty > map_subtiles_y) starty = map_subtiles_y;
  } else
    starty = 0;
  if (subtile_x > delta_x)
  {
    startx = subtile_x - delta_x;
    if (startx > map_subtiles_x) startx = map_subtiles_x;
  } else
    startx = 0;
  endy = subtile_y + delta_y;
  if (endy < starty) endy = starty;
  if (endy > map_subtiles_y) endy = map_subtiles_y;
  endx = subtile_x + delta_x;
  if (endx < startx) endx = startx;
  if (endx > map_subtiles_x) endx = map_subtiles_x;
  // Set the area
  light_render_area(startx, starty, endx, endy);
}

void update_flames_nearest_camera(struct Camera *camera)
{
  _DK_update_flames_nearest_camera(camera);
}

void update_footsteps_nearest_camera(struct Camera *camera)
{
  _DK_update_footsteps_nearest_camera(camera);
}

void process_player_states(void)
{
  _DK_process_player_states();
}

void process_players(void)
{
  int i;
  struct PlayerInfo *player;
  process_player_instances();
  process_player_states();
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) && (player->field_2C == 1))
      {
          wander_point_update(&player->wandr1);
          wander_point_update(&player->wandr2);
          _DK_update_power_sight_explored(player);
          if (game.numfield_A & 0x01)
          {
            if ((!player->field_4EB) && (player->field_29))
              player->field_4EB = game.seedchk_random_used+1;
          }
          if (player->field_4EB == game.seedchk_random_used)
          {
            switch (player->field_29)
            {
            case 1:
              if (i == my_player_number)
                _DK_set_level_objective(strings[0]);
              _DK_display_objectives(player->field_2B, 0, 0);
              break;
            case 2:
              if (i == my_player_number)
                _DK_set_level_objective(strings[335]);
              _DK_display_objectives(player->field_2B, 0, 0);
              break;
            }
          }
      }
  }
}

short update_animating_texture_maps(void)
{
  int i,k;
  anim_counter = (anim_counter+1) % -8;
  k = (anim_counter+1) % -8;
  for (i=0; i<48; i++)
  {
        short j = game.field_14A83F[k+8*i];
        block_ptrs[592-48+i] = block_ptrs[j];
  }
  return true;
}

long prepare_hsi_screenshot(unsigned char *buf)
{
  static const char *func_name="prepare_hsi_screenshot";
  unsigned char palette[768];
  long pos,i;
  int w,h;
  short lock_mem;
  pos=0;
  w=MyScreenWidth/pixel_size;
  h=MyScreenHeight/pixel_size;

  write_int8_buf(buf+pos,'m');pos++;
  write_int8_buf(buf+pos,'h');pos++;
  write_int8_buf(buf+pos,'w');pos++;
  write_int8_buf(buf+pos,'a');pos++;
  write_int8_buf(buf+pos,'n');pos++;
  write_int8_buf(buf+pos,'h');pos++;
  // pos=6
  write_int16_be_buf(buf+pos, 4);pos+=2;
  write_int16_be_buf(buf+pos, w);pos+=2;
  write_int16_be_buf(buf+pos, h);pos+=2;
  write_int16_be_buf(buf+pos, 256);pos+=2;
  // pos=14
  write_int16_be_buf(buf+pos, 256);pos+=2;
  write_int16_be_buf(buf+pos, 256);pos+=2;
  write_int16_be_buf(buf+pos, 256);pos+=2;
  // pos=20
  for (i=0; i<6; i++)
  {
    write_int16_be_buf(buf+pos, 0);pos+=2;
  }
  LbPaletteGet(palette);
  for (i=0; i<768; i+=3)
  {
    write_int8_buf(buf+pos,4*palette[i+0]);pos++;
    write_int8_buf(buf+pos,4*palette[i+1]);pos++;
    write_int8_buf(buf+pos,4*palette[i+2]);pos++;
  }
  lock_mem = LbScreenIsLocked();
  if (!lock_mem)
  {
    if (LbScreenLock()!=1)
    {
      error(func_name, 3852,"Can't lock canvas");
      LbMemoryFree(buf);
      return 0;
    }
  }
  for (i=0; i<h; i++)
  {
    memcpy(buf+pos, lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth*i, w);
    pos += w;
  }
  if (!lock_mem)
    LbScreenUnlock();
  return pos;
}

long prepare_bmp_screenshot(unsigned char *buf)
{
  static const char *func_name="prepare_bmp_screenshot";
  unsigned char palette[768];
  long pos,i,j;
  int width,height;
  short lock_mem;
  long data_len,pal_len;
  pos=0;
  width=MyScreenWidth/pixel_size;
  height=MyScreenHeight/pixel_size;
  write_int8_buf(buf+pos,'B');pos++;
  write_int8_buf(buf+pos,'M');pos++;
  int padding_size=4-(width&3);
  data_len = (width+padding_size)*height;
  pal_len = 256*4;
  write_int32_le_buf(buf+pos, data_len+pal_len+0x36);pos+=4;
  write_int32_le_buf(buf+pos, 0);pos+=4;
  write_int32_le_buf(buf+pos, pal_len+0x36);pos+=4;
  write_int32_le_buf(buf+pos, 40);pos+=4;
  write_int32_le_buf(buf+pos, width);pos+=4;
  write_int32_le_buf(buf+pos, height);pos+=4;
  write_int16_le_buf(buf+pos, 1);pos+=2;
  write_int16_le_buf(buf+pos, 8);pos+=2;
  write_int32_le_buf(buf+pos, 0);pos+=4;
  write_int32_le_buf(buf+pos, 0);pos+=4;
  write_int32_le_buf(buf+pos, 0);pos+=4;
  write_int32_le_buf(buf+pos, 0);pos+=4;
  write_int32_le_buf(buf+pos, 0);pos+=4;
  write_int32_le_buf(buf+pos, 0);pos+=4;
  LbPaletteGet(palette);
  for (i=0; i<768; i+=3)
  {
      unsigned int cval;
      cval=(unsigned int)4*palette[i+2];
      if (cval>255) cval=255;
      write_int8_buf(buf+pos,cval);pos++;
      cval=(unsigned int)4*palette[i+1];
      if (cval>255) cval=255;
      write_int8_buf(buf+pos,cval);pos++;
      cval=(unsigned int)4*palette[i+0];
      if (cval>255) cval=255;
      write_int8_buf(buf+pos,cval);pos++;
      write_int8_buf(buf+pos,0);pos++;
  }
  lock_mem = LbScreenIsLocked();
  if (!lock_mem)
  {
    if (LbScreenLock()!=1)
    {
      error(func_name, 3852,"Can't lock canvas");
      LbMemoryFree(buf);
      return 0;
    }
  }
  for (i=0; i<height; i++)
  {
    memcpy(buf+pos, lbDisplay.WScreen + lbDisplay.GraphicsScreenWidth*(height-i-1), width);
    pos += width;
    if ((padding_size&3) > 0)
      for (j=0; j < padding_size; j++)
      {
        write_int8_buf(buf+pos,0);pos++;
      }
  }
  if (!lock_mem)
    LbScreenUnlock();
  return pos;
}

void cumulative_screen_shot(void)
{
  static const char *func_name="cumulative_screen_shot";
  //_DK_cumulative_screen_shot();return;
  static long frame_number=0;
  char fname[255];
  const char *fext;
  int w,h;
  switch (screenshot_format)
  {
  case 1:
    fext="raw";
    break;
  case 2:
    fext="bmp";
    break;
  default:
    error(func_name, 3849,"Screenshot format incorrectly set.");
    return;
  }
  long i;
  unsigned char *buf;
  long ssize;
  for (i=frame_number; i<10000; i++)
  {
    sprintf(fname, "scrshots/scr%05d.%s", i, fext);
    if (!LbFileExists(fname)) break;
  }
  frame_number = i;
  if (frame_number >= 10000)
  {
    error(func_name, 3850,"No free filename");
    return;
  }
  sprintf(fname, "scrshots/scr%05d.%s", frame_number, fext);

  w=MyScreenWidth/pixel_size;
  h=MyScreenHeight/pixel_size;

  buf = LbMemoryAlloc((w+3)*h+2048);
  if (buf == NULL)
  {
    error(func_name, 3851,"Can't allocate buffer");
    return;
  }
  switch (screenshot_format)
  {
  case 1:
    ssize=prepare_hsi_screenshot(buf);
    break;
  case 2:
    ssize=prepare_bmp_screenshot(buf);
    break;
  default:
    ssize=0;
    break;
  }
  if (ssize>0)
    LbFileSaveAt(fname, buf, ssize);
  LbMemoryFree(buf);
  frame_number++;
}

void update_player_sounds(void)
{
  int k;
  struct Camera *camera;
  if ((game.numfield_C & 0x01) == 0)
  {
    camera=game.players[my_player_number%PLAYERS_COUNT].camera;
    process_messages();
    if (!SoundDisabled)
    {
      if ((game.flags_cd & 0x10) == 0)
      {
        if (game.audiotrack > 0)
          PlayRedbookTrack(game.audiotrack);
      }
      S3DSetSoundReceiverPosition(camera->pos_x,camera->pos_y,camera->pos_z);
      S3DSetSoundReceiverOrientation(camera->orient_a,camera->orient_b,camera->orient_c);
    }
    game.seedchk_random_used++;
  }
  find_nearest_rooms_for_ambient_sound();
  process_3d_sounds();
  k = (game.field_1517E2-game.seedchk_random_used) / 2;
  if (is_bonus_level(game.numfield_14A83D) )
  {
    if ((game.field_1517E2!=game.seedchk_random_used) ||
        (game.field_1517E2>game.seedchk_random_used) && ((k<=100) && ((k % 10) == 0) ||
        (k<=300) && ((k % 50)==0) || ((k % 250)==0)) )
      play_non_3d_sample(89);
  }
}

void update(void)
{
  //_DK_update();return;
  struct PlayerInfo *player;
  int i,k;

  if ((game.numfield_C & 0x01) == 0)
    update_light_render_area();

  process_packets();
  if (quit_game)
  {
    return;
  }
  if (game.flagfield_14EA4A == 1)
  {
    game.field_14EA4B = 0;
    return;
  }

  if ((game.numfield_C & 0x01) == 0)
  {
    player=&(game.players[my_player_number%PLAYERS_COUNT]);
    if (player->field_3 & 0x08)
    {
      PaletteSetPlayerPalette(player, _DK_palette);
      player->field_3 &= 0xF7u;
    }

    for (i=0; i<DUNGEONS_COUNT; i++)
    {
      memset(game.dungeon[i].field_64, 0, 960);
      memset(game.dungeon[i].field_424, 0, 12);
      memset(game.dungeon[i].field_4E4, 0, 12);
    }

    game.creature_pool_empty = 1;
    for (i=1; i<32; i++)
    {
      if (game.creature_pool[i] > 0)
      { game.creature_pool_empty = 0; break; }
    }

    if ((game.seedchk_random_used & 0x01) != 0)
    {
      update_animating_texture_maps();
    }
    update_things();
    process_rooms();
    process_dungeons();
    update_research();
    update_manufacturing();
    event_process_events();
    update_all_events();
    process_level_script();
    if (game.numfield_D & 0x04)
      process_computer_players2();
    process_players();
    process_action_points();
    player=&(game.players[my_player_number%PLAYERS_COUNT]);
    if (player->field_37 == 1)
      update_flames_nearest_camera(player->camera);
    update_footsteps_nearest_camera(player->camera);
    PaletteFadePlayer(player);
    _DK_process_armageddon();
  }

  message_update();
  update_all_players_cameras();
  update_player_sounds();

  // Rare message easter egg
  if ((game.seedchk_random_used) && ((game.seedchk_random_used % 0x4E20) == 0))
  {
      game.field_14BB4A = _lrotr(9377*game.field_14BB4A+9439, 13);
      if ( !(game.field_14BB4A % 0x7D0u) )
      {
        game.field_14BB4E = _lrotr(9377*game.field_14BB4E+9439, 13);
        if (game.field_14BB4E % 0xAu == 7)
          k = 94;// 'Your pants are definitely too tight'
        else
          k = game.field_14BB4E % 0xAu + 91;
        output_message(k, 0, 1);
      }
  }
  game.field_14EA4B = 0;
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
    struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
    PaletteSetPlayerPalette(player, _DK_palette);
    if ( game.numfield_C & 0x02 )
      _DK_initialise_eye_lenses();
    LbSyncLog("Entering the gameplay loop for level %d\n",(int)game.level_number);

    //the main gameplay loop starts
    while ( (!quit_game)&&(!exit_keeper) )
    {
      if ( game.flags_font & 0x10 )
      {
        if ( game.seedchk_random_used == 4 )
          ;//rndseed_nullsub();
      }

      // Some timing (which I don't understand; but it affects graphics)
      short do_draw;
      do_draw = true;
      if ( !(game.numfield_C & 0x01) )
      {
        if ( (!game.numfield_149F34) && (!game.numfield_149F38) )
        {
          unsigned long curr_time;
          curr_time = clock();
          cntr_time2++;
          if ( curr_time-prev_time2 >= 1000 )
          {
              double time_fdelta = 1000.0*((double)(cntr_time2))/((double)(curr_time-prev_time2));
              prev_time2 = curr_time;
              game.time_delta = (unsigned long)(time_fdelta*256.0);
              cntr_time2 = 0;
          }
          if ( (game.timingvar1!=0) && (game.seedchk_random_used % game.timingvar1) )
          {
            do_draw = false;
          }
        } else
        if ( ((game.seedchk_random_used & 0x3F)==0) ||
           ((game.numfield_149F38) && ((game.seedchk_random_used & 7)==0)) )
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
              game.time_delta = (unsigned long)(time_fdelta*256.0);
              cntr_time1 = 0;
            }
        } else
        {
            do_draw = false;
        }
      }
      if (!do_draw)
          do_draw = !_DK_LbIsActive();

      update_mouse();
      input_eastegg();
      input();
      update();

      if ( do_draw )
      {
        LbScreenClear(0);
        if ( LbScreenLock() == 1 )
        {
          _DK_setup_engine_window(player->field_448, player->field_44A,
                player->field_444, player->field_446);
          _DK_redraw_display();
          LbScreenUnlock();
        }
      }
      do {
        if ( !LbWindowsControl() )
        {
          if ( !(game.numfield_A & 0x01) )
          {
            exit_keeper = 1;
            break;
          }
          sprintf(text, "alex");
          LbSyncLog("%s - %s\n",func_name,text);
        }
        if ( (game.numfield_A & 0x01) || _DK_LbIsActive() )
          break;
      } while ( (!exit_keeper) && (!quit_game) );

      if ( game.numfield_A & 0x10 )
      {
        game.numfield_A &= 0xEFu;
        cumulative_screen_shot();
      }

      // Direct information/error messages
      if ( LbScreenLock() == 1 )
      {
        if ( game.numfield_A & 0x08 )
        {
          if (anim_record_frame(lbDisplay.WScreen, _DK_palette))
            _DK_LbTextDraw(600/pixel_size, 4/pixel_size, "REC");
        }
        // Display in-game message for debug purposes
        //_DK_LbTextDraw(200/pixel_size, 8/pixel_size, text);text[0]='\0';
        if ( game.numfield_A & 0x02 )
        {
          sprintf(text, "OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
          error(func_name, 413, text);
          if ( lbDisplay.WScreen != NULL )
            _DK_LbTextDraw(300/pixel_size, 200/pixel_size, "OUT OF SYNC");
        }
        if ( game.numfield_A & 0x04 )
        {
          sprintf(text, "SEED OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
          error(func_name, 427, text);
          if ( lbDisplay.WScreen != NULL)
            _DK_LbTextDraw(300/pixel_size, 220/pixel_size, "SEED OUT OF SYNC");
        }
        LbScreenUnlock();
      }

      // Music and sound control
      if ( !SoundDisabled )
        if ((!game.numfield_149F34) && (!game.numfield_149F38))
        {
            MonitorStreamedSoundTrack();
            _DK_process_sound_heap();
        }

      // Move the graphics window to center of screen buffer and swap screen
      if ( do_draw )
      {
        // For resolution 640x480, move the graphics data 40 lines lower
        if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
          if ( LbScreenLock() == 1 )
          {
            int i;
            int scrmove_x=0;
            int scrmove_y=40;
            int scanline_len=640;
            for (i=400;i>=0;i--)
              memcpy(lbDisplay.WScreen+scanline_len*(i+scrmove_y)+scrmove_x, lbDisplay.WScreen+scanline_len*i, scanline_len-scrmove_x);
            memset(lbDisplay.WScreen, 0, scanline_len*scrmove_y);
            LbScreenUnlock();
          }
        LbScreenSwap();
      }

      // Make delay if the machine is too fast
      if ( (!game.field_149E81) || (!game.numfield_149F34) )
      {
        if ( game.numfield_D & 0x10 )
        {
          unsigned long sleep_end = last_loop_time + 1000;
          LbSleepUntil(sleep_end);
          last_loop_time = LbTimerClock();
        } else
        if ( game.timingvar1 == 0 )
        {
          unsigned long sleep_end = last_loop_time + 1000/game.num_fps;
          LbSleepUntil(sleep_end);
          last_loop_time = LbTimerClock();
        }
      }

      if ( game.numfield_149F42 == game.seedchk_random_used )
        exit_keeper = 1;
    } // end while
    LbSyncLog("Gameplay loop finished after %u turns\n",game.seedchk_random_used);
}

void initialise_load_game_slots(void)
{
  _DK_load_game_save_catalogue(save_game_catalogue);
  number_of_saved_games = 0;
  int entry_idx;
  for (entry_idx=0;entry_idx<8;entry_idx++)
  {
    if ( save_game_catalogue[entry_idx].used )
      number_of_saved_games++;
  }
}

void define_key_input(void)
{
    if ( lbInkey == 1 )
    {
          _DK_defining_a_key = 0;
          lbInkey = 0;
    } else
    if ( lbInkey )
    {
            short ctrl_state = 0;
            if ( lbKeyOn[KC_LCONTROL] || (lbKeyOn[KC_RCONTROL]) )
              ctrl_state = 1;
            short shift_state = 0;
            if ( lbKeyOn[KC_LSHIFT] || (lbKeyOn[KC_RSHIFT]) )
              shift_state = 1;
            if ( _DK_set_game_key(_DK_defining_a_key_id, lbInkey, shift_state, ctrl_state) )
              _DK_defining_a_key = 0;
            lbInkey = 0;
    }
}

short continue_game_available()
{
      static char buf[255];
      static short continue_needs_checking_file = 1;
      sprintf(buf, "%s\\%s\\%s",keeper_runtime_directory,"save","continue.sav");
      check_cd_in_drive();
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
              game.level_number = (unsigned char)buf[0];
          }
          continue_needs_checking_file = 0;
      }
      if ( game.level_number > 20 )
          return false;
      else
          return true;
}

void intro(void)
{
    char text[255];
    sprintf(text, "%s/ldata/%s", install_info.inst_path, "intromix.smk");
    LbSyncLog("Playing video \"%s\"\n",text);
    play_smacker_file(text, 1);
}

void outro(void)
{
    char text[255];
    sprintf(text, "%s/ldata/%s", install_info.inst_path, "outromix.smk");
    LbSyncLog("Playing video \"%s\"\n",text);
    play_smacker_file(text, 17);
}

void faststartup_saved_packet_game(void)
{
    int scrmode=(-((unsigned int)(_DK_settings.field_B - 1) < 1) & 0xFFF4) + 13;
    if (_DK_setup_screen_mode(scrmode))
        return;
    if ( _DK_settings.field_B != 13 )
    {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return;
    }
    if ( !_DK_setup_screen_mode(1) )
    {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return;
    }
    _DK_settings.field_B = 1;
    _DK_save_settings();
    _DK_startup_saved_packet_game();
    game.players[my_player_number%PLAYERS_COUNT].field_6 &= 0xFDu;
}

void faststartup_network_game(void)
{
    int scrmode=(-((unsigned int)(_DK_settings.field_B - 1) < 1) & 0xFFF4) + 13;
    if ( !_DK_setup_screen_mode(scrmode) )
    {
      if ( _DK_settings.field_B != 13 )
      {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return;
      }
      if ( !_DK_setup_screen_mode(1) )
      {
        _DK_FatalError = 1;
        exit_keeper = 1;
        return;
      }
      _DK_settings.field_B = 1;
      _DK_save_settings();
    }
    my_player_number = default_loc_player;
    game.flagfield_14EA4A = 2;
    game.players[my_player_number].field_2C = 1;
    game.numfield_14A83D = game.numfield_16;
    _DK_startup_network_game();
    game.players[my_player_number].field_6 &= 0xFDu;
}

short is_bonus_level(long levidx)
{
    if ((levidx>=100)&&(levidx<106))
        return true;
    return false;
}

int setup_old_network_service(void)
{
    return setup_network_service(_DK_net_service_index_selected);
}

void wait_at_frontend(void)
{
  static const char *func_name="wait_at_frontend";
  //_DK_wait_at_frontend(); return;
  LbSyncLog("Falling into frontend menu.\n");

  //Moon phase calculation
  calculate_moon_phase(false);
  if ( game.flags_cd & 0x01 )
    game.field_149E81 = 0;
  game.numfield_15 = -1;

  initialise_load_game_slots();

  if ( (game.field_149E81) && (!game.numfield_149F47) )
  {
    faststartup_saved_packet_game();
    return;
  }

  if ( game.numfield_C & 0x02 )
  {
    faststartup_network_game();
    return;
  }

  if ( !_DK_setup_screen_mode_minimal(13) )
  {
    _DK_FatalError = 1;
    exit_keeper = 1;
    return;
  }
  LbScreenClear(0);
  LbScreenSwap();
  if ( !_DK_frontend_load_data() )
  {
    error(func_name, 738, "Unable to load frontend data");
    exit_keeper = 1;
    return;
  }
  memset(_DK_scratch, 0, 0x300u);
  LbPaletteSet(_DK_scratch);
  frontend_set_state(get_startup_menu_state());

  short finish_menu = 0;
  game.flags_cd &= 0xBFu;

  // Begin the frontend loop
  long last_loop_time = LbTimerClock();
  do
  {
    if ( (!LbWindowsControl()) && ((game.numfield_A & 0x01)==0) )
    {
      exit_keeper = 1;
      LbSyncLog("%s: Windows Control exit condition invoked\n",func_name);
      break;
    }
//LbSyncLog("update_mouse\n");
    update_mouse();
    _DK_old_mouse_over_button = frontend_mouse_over_button;
    frontend_mouse_over_button = 0;

//LbSyncLog("frontend_input\n");
    frontend_input();
    if ( exit_keeper )
    {
      LbSyncLog("%s: Frontend Input exit condition invoked\n",func_name);
      break; // end while
    }

//LbSyncLog("frontend_update\n");
    frontend_update(&finish_menu);
    if ( exit_keeper )
    {
      LbSyncLog("%s: Frontend Update exit condition invoked\n",func_name);
      break; // end while
    }

    if ( !finish_menu && _DK_LbIsActive() )
    {
//LbSyncLog("frontend_draw\n");
      frontend_draw();
      LbScreenSwap();
    }

    if ( !SoundDisabled )
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

  LbPaletteFade(0, 8, Lb_PALETTE_FADE_CLOSED);
  LbScreenClear(0);
  LbScreenSwap();
  short prev_state = _DK_frontend_menu_state;
  frontend_set_state(0);
  if ( exit_keeper )
  {
    game.players[my_player_number].field_6 &= 0xFDu;
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
      exit_keeper = 1;
      return;
    }
  }

  display_loading_screen();
  short flgmem;
  switch ( prev_state )
  {
      case 7:
        my_player_number = default_loc_player;
        game.flagfield_14EA4A = 2;
        game.numfield_A &= 0xFEu;
        game.players[my_player_number].field_2C = 1;
        game.numfield_14A83D = game.numfield_16;
        _DK_startup_network_game();
        break;
      case 8:
        game.numfield_14A83D = game.numfield_16;
        game.numfield_A |= 0x01;
        game.flagfield_14EA4A = 5;
        game.players[my_player_number].field_2C = 1;
        _DK_startup_network_game();
        break;
      case 10:
        flgmem = game.numfield_15;
        game.numfield_A &= 0xFEu;
        if ( game.numfield_15 == -2 )
        {
          error(func_name, 1012, "Why are we here");
          game.numfield_15 = flgmem;
        } else
        {
          LbScreenClear(0);
          LbScreenSwap();
          _DK_load_game(game.numfield_15);
          game.numfield_15 = flgmem;
        }
        break;
      case 25:
        game.flags_cd |= 1;
        _DK_startup_saved_packet_game();
        break;
  }
  game.players[my_player_number].field_6 &= 0xFDu;
}

void game_loop(void)
{
  //_DK_game_loop();
  unsigned long random_seed;
  unsigned long playtime;
  playtime = 0;
  random_seed = 0;
  LbSyncLog("Entering gameplay loop.\n");
  while ( !exit_keeper )
  {
    update_mouse();
    wait_at_frontend();
    if ( exit_keeper )
      break;
    struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
    if ( game.flagfield_14EA4A == 2 )
    {
      if ( game.numfield_15 == -1 )
      {
        _DK_set_player_instance(player, 11, 0);
      } else
      {
        game.numfield_15 = -1;
        game.numfield_C &= 0xFE;
      }
    }
    unsigned long starttime;
    unsigned long endtime;
    starttime = LbTimerClock();
    game.dungeon[my_player_number%DUNGEONS_COUNT].time1 = timeGetTime();//starttime;
    game.dungeon[my_player_number%DUNGEONS_COUNT].time2 = timeGetTime();//starttime;
    LbScreenClear(0);
    LbScreenSwap();
    keeper_gameplay_loop();
    _DK_LbMouseChangeSpriteAndHotspot(0, 0, 0);
    LbScreenClear(0);
    LbScreenSwap();
    StopRedbookTrack();
    StopMusic();
    _DK_frontstats_initialise();
    _DK_delete_all_structures();
    _DK_clear_mapwho();
    endtime = LbTimerClock();
    quit_game = 0;
    if ( game.numfield_C & 0x02 )
        exit_keeper=true;
    playtime += endtime-starttime;
    LbSyncLog("Play time is %d seconds\n",playtime>>10);
    random_seed += game.seedchk_random_used;
    _DK_reset_eye_lenses();
    if ( game.packet_fopened )
    {
      _DK_LbFileClose(game.packet_save);
      game.packet_fopened = 0;
      game.packet_save = 0;
    }
  } // end while
  // Stop the movie recording if it's on
  if ( game.numfield_A & 0x08 )
  {
    game.numfield_A &= 0xF7u;
    anim_stop();
  }
}

short reset_game(void)
{
  _DK_IsRunningUnmark();
  _DK_LbMouseSuspend();
  LbIKeyboardClose();
  _DK_LbScreenReset();
  _DK_LbDataFreeAll(_DK_game_load_files);
  _DK_LbMemoryFree(strings_data);
  strings_data = NULL;
  FreeAudio();
  return _DK_LbMemoryReset();
}

short process_command_line(unsigned short argc, char *argv[])
{
  static const char *func_name="process_command_line";
  char fullpath[CMDLN_MAXLEN+1];
  strncpy(fullpath, argv[0], CMDLN_MAXLEN);

  sprintf( keeper_runtime_directory, fullpath);
  char *endpos=strrchr( keeper_runtime_directory, '\\');
  if (endpos!=NULL)
      *endpos='\0';

  SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain
  _DK_LbErrorLogSetup(0, 0, 1);

  game.numfield_149F42 = -1;
  game.numfield_149F46 = 0;
  game.packet_checksum = 1;
  game.numfield_1503A2 = -1;
  game.flags_font &= 0xFEu;
  game.numfield_149F47 = 0;
  game.numfield_16 = 0;
  game.num_fps = 20;
  game.flags_cd = (game.flags_cd & 0xFE) | 0x40;

  short bad_param = 0;
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
        game.no_intro = 1;
      } else
      if ( stricmp(parstr, "nocd") == 0 )
      {
          game.flags_cd |= 0x10;
      } else
      if ( stricmp(parstr, "1player") == 0 )
      {
          game.one_player = 1;
      } else
      if ( (stricmp(parstr, "s") == 0) || (stricmp(parstr, "nosound") == 0) )
      {
          SoundDisabled = 1;
      } else
      if ( stricmp(parstr, "fps") == 0 )
      {
          narg++;
          game.num_fps = atoi(pr2str);
      } else
      if ( stricmp(parstr, "human") == 0 )
      {
          narg++;
          default_loc_player = atoi(pr2str);
      } else
      if ( stricmp(parstr, "usersfont") == 0 )
      {
          game.flags_font |= 0x40;
      } else
      if ( stricmp(parstr,"level") == 0 )
      {
        game.numfield_C |= 0x02;
        level_num = atoi(pr2str);
        narg++;
      } else
      if ( stricmp(parstr,"q") == 0 )
      {
         game.numfield_C |= 0x02;
      } else
      if ( stricmp(parstr,"columnconvert") == 0 )
      {
         game.numfield_C |= 0x08;
      } else
      if ( stricmp(parstr,"lightconvert") == 0 )
      {
         game.numfield_C |= 0x10;
      } else
      if ( stricmp(parstr, "alex") == 0 )
      {
          game.flags_font |= 0x20;
      } else
      {
        LbSyncLog("Warning - Unrecognized command line parameter '%s'.\n",parstr);
        bad_param=narg;
      }
      narg++;
  }

  if ( level_num == -1 )
    level_num = 1;
  game.numfield_16 = level_num;
  game.level_number = level_num;
  if ( (game.numfield_C & 0x02) == 0 )
    game.level_number = 1;
  my_player_number = default_loc_player;
  return (bad_param==0);
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
  static const char *func_name="LbBullfrogMain";
  short retval=0;
  LbErrorLogSetup("/", log_file_name, 5);
  strcpy(window_class_name, PROGRAM_NAME);
  LbSetIcon(110);
  srand(LbTimerClock());

  retval=process_command_line(argc,argv);
  if ( retval < 1 )
  {
      static const char *msg_text="Command line parameters analysis failed.\n";
      error_dialog_fatal(func_name, 1, msg_text);
      LbErrorLogClose();
      return 0;
  }

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
      error_dialog_fatal(func_name, 2, msg_text);
  } else
  {
      LbSyncLog("%s finished properly.\n",func_name);
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
  struct PlayerInfo *player=&(game.players[0]);
      static char msg_text[255];
      sprintf(msg_text,"Position of the first Player is %06x, first Camera is %06x bytes.\n",((int)player) - ((int)&_DK_game),((int)&(player->camera)) - ((int)player));
      error_dialog(func_name, 1, msg_text);
      return 0;
  }*/

//TODO: delete when won't be needed anymore
  memcpy(_DK_menu_list,menu_list,sizeof(menu_list));


  if (sizeof(struct Game)!=SIZEOF_Game)
  {
      static char msg_text[255];
      sprintf(msg_text,"Bad compilation - struct Game has wrong size!\nThe difference is %d bytes.\n",sizeof(struct Game)-SIZEOF_Game);
      error_dialog(func_name, 1, msg_text);
      return 0;
  }

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
