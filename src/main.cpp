
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
#include "bflib_sprfnt.h"
#include "bflib_fileio.h"
#include "bflib_sndlib.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"
#include "bflib_mouse.h"

#include "frontend.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "kjm_input.h"

// Max length of the command line
#define CMDLN_MAXLEN 259
// Max length of any processed string
#define MAX_TEXT_LENGTH 4096
char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];
unsigned char palette_buf[768];
int map_subtiles_x=255;
int map_subtiles_y=255;

char onscreen_msg_text[255]="";
int onscreen_msg_turns = 0;

char window_class_name[128]="Bullfrog Shell";
short default_loc_player=0;
const char keeper_config_file[]="keeperfx.cfg";

// Boxes used for cheat menu
struct GuiBox *gui_box=NULL;
struct GuiBox *gui_cheat_box=NULL;

struct GuiBox *first_box=NULL;
struct GuiBox *last_box=NULL;
struct GuiBox gui_boxes[3];
struct TbSprite *font_sprites=NULL;
struct TbSprite *end_font_sprites=NULL;
struct DraggingBox dragging_box;

#define TRACE LbNetLog

struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE", &_DK_palette, NULL, PALETTE_SIZE, 0, 0},
    {"*SCRATCH", &scratch, NULL, 0x10000, 1, 0},
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
  {"FRONTEND_RES=",  6},
  {"INGAME_RES=",    7},
  {NULL,             0},
  };

//static
TbClockMSec last_loop_time=0;

#ifdef __cplusplus
extern "C" {
#endif
DLLIMPORT void _DK_set_packet_action(struct Packet *pckt,unsigned char,short,short,short,short);
DLLIMPORT void __cdecl _DK_reset_eye_lenses(void);
DLLIMPORT void __cdecl _DK_reset_heap_manager(void);
DLLIMPORT void __cdecl _DK_reset_heap_memory(void);
DLLIMPORT int _DK_LoadMcgaData(void);
DLLIMPORT void _DK_initialise_eye_lenses(void);
DLLIMPORT void _DK_setup_eye_lens(long nlens);
DLLIMPORT void _DK_setup_heap_manager(void);
DLLIMPORT int _DK_setup_heap_memory(void);
DLLIMPORT long _DK_light_create_light(struct InitLight *ilght);
DLLIMPORT void _DK_light_set_light_never_cache(long idx);
DLLIMPORT void _DK_reset_player_mode(struct PlayerInfo *player, unsigned char a2);
DLLIMPORT void _DK_init_keeper_map_exploration(struct PlayerInfo *player);
DLLIMPORT void _DK_init_player_cameras(struct PlayerInfo *player);
DLLIMPORT void _DK_pannel_map_update(long x, long y, long w, long h);
#ifdef __cplusplus
}
#endif

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

short show_onscreen_msg_va(int nturns, const char *fmt_str, va_list arg)
{
  vsprintf(onscreen_msg_text, fmt_str, arg);
  LbSyncLog("Onscreen message: %s\n",onscreen_msg_text);
  onscreen_msg_turns = nturns;
  return 1;
}

short show_onscreen_msg(int nturns, const char *fmt_str, ...)
{
    short result;
    va_list val;
    va_start(val, fmt_str);
    result=show_onscreen_msg_va(nturns, fmt_str, val);
    va_end(val);
    return result;
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
      mdir=keeper_runtime_directory;
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

short load_configuration(void)
{
  static const char *func_name="load_configuration";
  //return _DK_load_configuration();
  char *fname;
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
  fname=prepare_file_path(FGrp_Main,keeper_config_file);
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
      case 6: // FRONTEND_RES
          for (i=0; i<3; i++)
          {
            while ((buf[pos] == ' ') || (buf[pos] == '\t'))
              pos++;
            k=0;
            while (((unsigned char)buf[pos] > 32) && (buf[pos] != '\t') && (k<31))
            {
              word_buf[k]=buf[pos];
              pos++;k++;
            }
            word_buf[k]='\0';
            k=LbRecogniseVideoModeString(word_buf);
            if (k<=0) continue;
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
            while ((buf[pos] == ' ') || (buf[pos] == '\t'))
              pos++;
            k=0;
            while (((unsigned char)buf[pos] > 32) && (buf[pos] != '\t') && (k<31))
            {
              word_buf[k]=buf[pos];
              pos++;k++;
            }
            word_buf[k]='\0';
            k=LbRecogniseVideoModeString(word_buf);
            if (k<=0) continue;
            set_game_vidmode(i,k);
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

void reset_eye_lenses(void)
{
  _DK_reset_eye_lenses();
}

int LoadMcgaData(void)
{
  return _DK_LoadMcgaData();
}

void initialise_eye_lenses(void)
{
  _DK_initialise_eye_lenses();
}

void setup_eye_lens(long nlens)
{
  _DK_setup_eye_lens(nlens);
}

void reinitialise_eye_lens(long nlens)
{
  initialise_eye_lenses();
  if ((game.flags_cd & 0x02) && (nlens>0))
  {
      game.numfield_1B = 0;
      setup_eye_lens(nlens);
  }
}

void setup_heap_manager(void)
{
  _DK_setup_heap_manager();
}

int setup_heap_memory(void)
{
  return _DK_setup_heap_memory();
}

void reset_heap_manager(void)
{
  _DK_reset_heap_manager();
}

void reset_heap_memory(void)
{
  _DK_reset_heap_memory();
}

long light_create_light(struct InitLight *ilght)
{
  return _DK_light_create_light(ilght);
}

void light_set_light_never_cache(long idx)
{
  _DK_light_set_light_never_cache(idx);
}

void init_player_as_single_keeper(struct PlayerInfo *player)
{
  unsigned short idx;
  struct InitLight ilght;
  memset(&ilght, 0, sizeof(struct InitLight));
  player->field_4CD = 0;
  ilght.field_0 = 0x0A00;
  ilght.field_2 = 48;
  ilght.field_3 = 5;
  ilght.field_11 = 1;
  idx = light_create_light(&ilght);
  player->field_460 = idx;
  light_set_light_never_cache(idx);
}

void reset_player_mode(struct PlayerInfo *player, unsigned char a2)
{
  _DK_reset_player_mode(player, a2);
}

void init_keeper_map_exploration(struct PlayerInfo *player)
{
  _DK_init_keeper_map_exploration(player);
}

void init_player_cameras(struct PlayerInfo *player)
{
  _DK_init_player_cameras(player);
}

void init_player_start(struct PlayerInfo *player)
{
  struct Dungeon *dungeon;
  struct Thing *thing;
  thing = game.things_lookup[game.field_149E1A];
  while (thing > game.things_lookup[0])
  {
    if ((game.objects_config[thing->field_1A].field_6) && (thing->owner == player->field_2B))
    {
      dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
      dungeon->field_0 = thing->field_1B;
      memcpy(&dungeon->mappos,&thing->mappos,sizeof(struct Coord3d));
      break;
    }
    thing = game.things_lookup[thing->field_67];
  }
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
      net_player_info[k].field_20 = 0;
      strcpy(net_player_info[k].field_0, "");
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

TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long a2)
{
  return _DK_LbNetwork_ChangeExchangeBuffer(buf, a2);
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

/*
 * Copies the given RAW image at center of screen buffer and swaps video
 * buffers to make the image visible.
 * @return Returns true on success.
 */
short copy_raw8_image_buffer(unsigned char *dst_buf,const int scanline,const int nlines,const int spx,const int spy,const unsigned char *src_buf,const int src_width,const int src_height,const int m)
{
  int w,h,i,k;
  unsigned char *dst;
  const unsigned char *src;
  w=0;
  h=0;
  // Clearing top of the canvas
  if (spy>0)
  {
    for (h=0; h<spy; h++)
    {
      dst = dst_buf + (h)*scanline;
      memset(dst,0,scanline);
    }
    // Clearing bottom of the canvas
    // (Note: it must be done before drawing, to make sure we won't overwrite last line)
    for (h=nlines-spy; h<nlines; h++)
    {
      dst = dst_buf + (h)*scanline;
      memset(dst,0,scanline);
    }
  }
  // Now drawing
  for (h=0; h<src_height; h++)
  {
    src = src_buf + h*src_width;
    for (k=0; k<m; k++)
    {
      if (spy+m*h+k<0) continue;
      if (spy+m*h+k>=nlines) break;
      dst = dst_buf + (spy+m*h+k)*scanline + spx;
      for (w=0; w<src_width; w++)
      {
        for (i=0;i<m;i++)
        {
            dst[m*w+i] = src[w];
        }
      }
    }
  }
  return true;
}

/*
 * Copies the given RAW image at center of screen buffer and swaps video
 * buffers to make the image visible.
 * @return Returns true on success.
 */
short copy_raw8_image_to_screen_center(const unsigned char *buf,const int img_width,const int img_height)
{
  struct TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(lbDisplay.ScreenMode);
  int w,h,m,i,k;
  int spx,spy;
  unsigned char *dst;
  const unsigned char *src;
  // Only 8bpp supported for now
  if (mdinfo->BitsPerPixel != 8)
    return false;
  w=0;
  h=0;
  for (m=0;m<5;m++)
  {
    w+=img_width;
    h+=img_height;
    if (w > mdinfo->Width) break;
    if (h > mdinfo->Height) break;
  }
  // The image width can't be larger than video resolution
  if (m<1)
  {
    if (w > mdinfo->Width)
    {
      LbSyncLog("The %dx%d image does not fit on %dx%d screen, skipped.\n", img_width, img_height,mdinfo->Width,mdinfo->Height);
      return false;
    }
    m=1;
  }
  // Locking screen
  if (LbScreenLock()!=1)
    return false;
  // Starting point coords
  spx = (mdinfo->Width-m*img_width)>>1;
  spy = (mdinfo->Height-m*img_height)>>1;
  copy_raw8_image_buffer(lbDisplay.WScreen,mdinfo->Width,mdinfo->Height,
      spx,spy,buf,img_width,img_height,m);
  perform_any_screen_capturing();
  LbScreenUnlock();
  LbScreenSwap();
  return true;
}

short show_rawimage_screen(unsigned char *raw,unsigned char *pal,int width,int height,TbClockMSec tmdelay)
{
  static const char *func_name="show_rawimage_screen";
      if (height>lbDisplay.PhysicalScreenHeight)
           height=lbDisplay.PhysicalScreenHeight;
      LbPaletteSet(pal);
      TbClockMSec end_time;
      end_time = LbTimerClock() + tmdelay;
      TbClockMSec tmdelta;
      tmdelta = tmdelay/100;
      if (tmdelta>100) tmdelta=100;
      if (tmdelta<10) tmdelta=10;
      while (LbTimerClock() < end_time)
      {
          LbWindowsControl();
          copy_raw8_image_to_screen_center(raw,width,height);
          if (is_key_pressed(KC_SPACE,KM_DONTCARE) ||
              is_key_pressed(KC_ESCAPE,KM_DONTCARE) ||
              is_key_pressed(KC_RETURN,KM_DONTCARE) ||
              is_mouse_pressed_lrbutton())
          {
              clear_key_pressed(KC_SPACE);
              clear_key_pressed(KC_ESCAPE);
              clear_key_pressed(KC_RETURN);
              clear_mouse_pressed_lrbutton();
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
        TbClockMSec last_loop_time;
        last_loop_time = LbTimerClock();
        while ( LbPaletteFade(pal, n, flg) < n )
        {
          if (!is_key_pressed(KC_SPACE,KM_DONTCARE) &&
              !is_key_pressed(KC_ESCAPE,KM_DONTCARE) &&
              !is_key_pressed(KC_RETURN,KM_DONTCARE) &&
              !is_mouse_pressed_lrbutton())
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
        TbClockMSec last_loop_time;
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

int LbTextStringWidth(const char *str)
{
  return _DK_LbTextStringWidth(str);
}

int LbTextStringHeight(const char *str)
{
  //return _DK_LbTextStringHeight(str);
  int i,h,lines;
  lines=1;
  if ((lbFontPtr==NULL) || (str==NULL))
    return 0;
  for (i=0;i<MAX_TEXT_LENGTH;i++)
  {
    if (str[i]=='\0') break;
    if (str[i]==10) lines++;
  }
  h = 0;
  if (lbFontPtr != NULL)
    h = lbFontPtr[1].SHeight;
  return h*lines;
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
  char *fname;
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
      fname=prepare_file_path(FGrp_StdData,"loading.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      check_cd_in_drive();
      fname=prepare_file_path(FGrp_StdData,"loading.pal");
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      LbScreenClear(0);
      dproceed=copy_raw8_image_to_screen_center(buf,img_width,img_height);
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
  char *fname;
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
      fname=prepare_file_path(FGrp_FxData,"loading_640.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      check_cd_in_drive();
      fname=prepare_file_path(FGrp_FxData,"loading_640.pal");
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      LbScreenClear(0);
      dproceed=copy_raw8_image_to_screen_center(buf,img_width,img_height);
  }
  free(buf);
  return dproceed;
}

short display_loading_screen(void)
{
  struct TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(lbDisplay.ScreenMode);
  short done=false;
  if (mdinfo->Width >= 640)
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
    if ( setup_screen_mode_minimal(get_movies_vidmode()) )
    {
      LbMouseChangeSprite(0);
      LbScreenClear(0);
      LbScreenSwap();
    } else
    {
      error(func_name, 2356, "Can't enter movies video mode to play a Smacker file");
      result=0;
    }
  }
  if (result)
  {
    // Fail in playing shouldn't set result=0, because result=0 means fatal error.
    if (play_smk_(filename, 0, movie_flags | 0x100) == 0)
    {
      error(func_name, 2357, "Smacker play error");
      result=0;
    }
  }
  if (nstate>-2)
  {
    if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
    {
      error(func_name, 2358, "Can't re-enter frontend video mode after playing Smacker file");
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

char game_is_busy_doing_gui_string_input(void)
{
  return _DK_game_is_busy_doing_gui_string_input();
}

void set_packet_action(struct Packet *pckt, unsigned char pcktype, unsigned short par1, unsigned short par2, unsigned short par3, unsigned short par4)
{
  pckt->field_6 = par1;
  pckt->field_8 = par2;
  pckt->field_5 = pcktype;
}

/*
 *  Reacts on a keystoke by sending text message packets.
 */
short get_players_message_inputs(void)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if (is_key_pressed(KC_RETURN,KM_NONE))
  {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, PckT_PlyrMsgEnd, 0, 0, 0, 0);
      clear_key_pressed(KC_RETURN);
      return true;
  }
  lbFontPtr = winfont;
  int msg_width = pixel_size * LbTextStringWidth(player->strfield_463);
  if ( (is_key_pressed(KC_BACK,KM_DONTCARE)) || (msg_width < 450) )
  {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt,PckT_PlyrMsgChar,lbInkey,key_modifiers,0,0);
      clear_key_pressed(lbInkey);
      return true;
  }
  return false;
}

/*
 * Captures the screen to make a gameplay movie or screenshot image.
 * @return Returns 0 if no event was handled, nonzero otherwise.
 */
short get_screen_capture_inputs(void)
{
  short result=false;
  if (is_key_pressed(KC_M,KM_SHIFT))
  {
    if (game.numfield_A & 0x08)
      movie_record_stop();
    else
      movie_record_start();
    clear_key_pressed(KC_M);
    result=true;
  }
  if (is_key_pressed(KC_C,KM_SHIFT))
  {
    game.numfield_A |= 0x10;
    clear_key_pressed(KC_C);
    result=true;
  }
  return result;
}

short get_global_inputs(void)
{
  if ( game_is_busy_doing_gui_string_input() && (input_button==NULL) )
    return false;
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  struct Packet *pckt;
  long keycode;
  if ((player->field_0 & 0x04) != 0)
  {
    get_players_message_inputs();
    return true;
  }
  if ((player->field_452 == 1) && ((game.numfield_A & 0x01) != 0))
  {
      if (is_key_pressed(KC_RETURN,KM_NONE))
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, PckT_PlyrMsgBegin, 0, 0, 0, 0);
        clear_key_pressed(KC_RETURN);
        return true;
      }
  }
  // Code for debugging purposes
  if ( is_key_pressed(KC_D,KM_ALT) )
  {
    LbSyncLog("REPORT for gameturn %d\n",game.seedchk_random_used);
    // Timing report
    LbSyncLog("Now time is %d, last loop time was %d, clock is %d, requested fps is %d\n",LbTimerClock(),last_loop_time,clock(),game.num_fps);
  }

  int idx;
  for (idx=KC_F1;idx<=KC_F8;idx++)
  {
      if ( is_key_pressed(idx,KM_ALT) )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, PckT_PlyrFastMsg, idx-KC_F1, 0, 0, 0);
        clear_key_pressed(idx);
        return true;
      }
  }
  if ( (player->field_4B0 != 14) && (player->field_4B0 != 15) && (input_button==0) )
  {
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 22, 0, 0, 0, 0);
        clear_key_pressed(keycode);
        return true;
      }
  }
  if ((game.numfield_C & 0x01) != 0)
      return true;
  if (is_key_pressed(KC_ADD,KM_CONTROL))
  {
      game.timingvar1 += 2;
      if (game.timingvar1 < 0)
          game.timingvar1 = 0;
      else
      if ( game.timingvar1 > 64 )
          game.timingvar1 = 64;
      clear_key_pressed(KC_ADD);
  }
  if (is_key_pressed(KC_SUBTRACT,KM_CONTROL))
  {
      game.timingvar1 -= 2;
        if (game.timingvar1 < 0)
        game.timingvar1 = 0;
      else
      if ( game.timingvar1 > 64 )
        game.timingvar1 = 64;
      clear_key_pressed(KC_SUBTRACT);
  }
  if (is_key_pressed(KC_SUBTRACT,KM_NONE))
  {
      if ( player->minimap_zoom < 0x0800 )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, PckT_SetMinimapConf, 2 * player->minimap_zoom, 0, 0, 0);
      }
      clear_key_pressed(KC_SUBTRACT);
      return true;
  }
  if (is_key_pressed(KC_ADD,KM_NONE))
  {
      if ( player->minimap_zoom > 0x0080 )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, PckT_SetMinimapConf, player->minimap_zoom >> 1, 0, 0, 0);
      }
      clear_key_pressed(KC_ADD);
      return true;
  }
  if (is_key_pressed(KC_R,KM_ALT))
  {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, PckT_SwitchScrnRes, 0, 0, 0, 0);
      clear_key_pressed(KC_R);
      return true;
  }
  if (is_key_pressed(KC_SPACE,KM_NONE))
  {
      if ( player->field_29 )
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 5, 0, 0, 0, 0);
        clear_key_pressed(KC_SPACE);
        return true;
      }
  }
  get_screen_capture_inputs();

  if ( _DK_is_game_key_pressed(29, &keycode, 0) )
  {
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 111, 0, 0, 0, 0);
      clear_key_pressed(keycode);
  }
  return false;
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
  memset(&net_player_info[0], 0, sizeof(struct TbNetworkPlayerInfo));
  if ( LbNetwork_Init(srvidx, _DK_net_guid, maxplayrs, &_DK_net_screen_packet, 0xCu, &net_player_info[0], init_data) )
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
  char *fname;

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
  if ( LbScreenSetup(get_frontend_vidmode(), LEGAL_WIDTH, LEGAL_HEIGHT, _DK_palette, 1, 0) != 1 )
  {
      error(func_name, 1912, "Frontend screen mode setup error.");
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
      fname=prepare_file_path(FGrp_FxData,"bullfrog.smk");
      result=play_smacker_file(fname, -2);
      if ( !result )
        error(func_name, 1483, "Unable to play new moon movie");
  }

  result = 1;
  // The 320x200 mode is required only for the intro;
  // loading and no CD screens can run in both 320x2?0 and 640x4?0.
  if ( result && (!game.no_intro) )
  {
    int mode_ok = LbScreenSetup(get_movies_vidmode(), 320, 200, _DK_palette, 2, 0);
    if (mode_ok != 1)
    {
      error(func_name, 1500, "Can't enter movies screen mode to play intro");
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
      fname=prepare_file_path(FGrp_LoData,"intromix.smk");
      result=play_smacker_file(fname, -2);
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
      strings_data = (char *)LbMemoryAlloc(filelen + 256);
      if ( strings_data == NULL )
      {
        error(func_name, 1509, "Can't allocate memory for Strings data");
        result = 0;
      }
      text_end = strings_data+filelen+255;
  }
  if ( result )
  {
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
        _DK_set_gamma(settings.gamma_correction, 0);
        SetRedbookVolume(settings.redbook_volume);
        SetSoundMasterVolume(settings.sound_volume);
        SetMusicMasterVolume(settings.sound_volume);
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
    _DK_left_button_held_x = GetMouseX();
    _DK_left_button_held_y = lbDisplay.MMouseY * pixel_size;
  }
  if (_DK_left_button_held)
  {
    if (!lbDisplay.MLeftButton)
    {
      _DK_left_button_released = 1;
      _DK_left_button_held = 0;
      _DK_left_button_released_x = GetMouseX();
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
    _DK_right_button_held_x = GetMouseX();
    _DK_right_button_held_y = lbDisplay.MMouseY * pixel_size;
  }
  if ( _DK_right_button_held )
  {
    if ( !lbDisplay.MRightButton )
    {
      _DK_right_button_released = 1;
      _DK_right_button_held = 0;
      _DK_right_button_released_x = GetMouseX();
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
    switch (eastegg03_cntr)
    {
    case 0:
    case 4:
      if ( lbInkey==KC_S )
      {
       eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 1:
    case 3:
      if ( lbInkey==KC_K )
      {
       eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 2:
      if ( lbInkey==KC_E )
      {
       eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 5:
      if ( lbInkey==KC_I )
      {
       eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 6:
      if ( lbInkey==KC_S )
      {
        eastegg03_cntr++;
        lbInkey=0;
        //'Your pants are definitely too tight'
        output_message(94, 0, 1);
      }
      break;
    }
  } else
  {
    eastegg03_cntr = 0;
  }
  if (lbInkey!=0)
  {
    eastegg03_cntr = 0;
  }
}

void write_debug_packets(void)
{
  FILE *file;
  file = fopen("keeperd.pck", "w");
  fwrite(game.packets, 1, sizeof(struct Packet)*PACKETS_COUNT, file);
  fflush(file);
  fclose(file);
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
  for(idx=0;idx<ACTIVE_MENUS_COUNT;idx++)
  {
    struct GuiMenu *gmnu;
    gmnu=&active_menus[idx];
    if (gmnu->field_1 != 2)
        continue;
    if (gmnu->flgfield_1D == 0)
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
  const short gmax=8;//sizeof(active_menus)/sizeof(struct GuiMenu);
  struct GuiMenu *gmnu;
  for (idx=0;idx<gmax;idx++)
  {
    gmnu=&active_menus[idx];
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
  int mouse_x = GetMouseX();
  int mouse_y = GetMouseY();
  int x = gbtn->pos_x;
  int y = gbtn->pos_y;
  if ( (mouse_x >= x) && (mouse_x < x + gbtn->width)
    && (mouse_y >= y) && (mouse_y < y + gbtn->height)
    && (gbtn->field_0 & 0x04) )
    return true;
  return false;
}

inline void reset_scrolling_tooltip(void)
{
    _DK_tooltip_scroll_offset = 0;
    _DK_tooltip_scroll_timer = 25;
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
    if ((thing->field_18==0) && (player->field_2B != thing->owner))
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
      _DK_tool_tip_box.pos_x = GetMouseX();
      _DK_tool_tip_box.pos_y = GetMouseY()+86;
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
    _DK_tool_tip_box.pos_x = GetMouseX();
    _DK_tool_tip_box.field_0 = 1;
    _DK_tool_tip_box.field_809 = 5;
    _DK_tool_tip_box.pos_y = GetMouseY() + 86;
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
      _DK_tool_tip_box.pos_x = GetMouseX();
      _DK_tool_tip_box.field_809 = 5;
      _DK_tool_tip_box.pos_y = GetMouseY() + 86;
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
    _DK_tool_tip_box.pos_x = GetMouseX();
    _DK_tool_tip_box.pos_y = GetMouseY() + 86;
    _DK_tool_tip_box.field_809 = 5;
    return true;
  }
  if (!settings.tooltips_on)
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
      _DK_tool_tip_box.pos_x = GetMouseX();
      _DK_tool_tip_box.field_809 = 5;
      _DK_tool_tip_box.pos_y = GetMouseY() + 86;
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
  if (!settings.tooltips_on)
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
    _DK_tool_tip_box.pos_x = GetMouseX();
    _DK_tool_tip_box.field_809 = 2;
    _DK_tool_tip_box.pos_y = GetMouseY() + 86;
  } else
  {
    _DK_help_tip_time++;
  }
  return true;
}

short setup_room_tooltips(struct Coord3d *pos)
{
  if (!settings.tooltips_on)
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
      _DK_tool_tip_box.pos_x = GetMouseX();
      _DK_tool_tip_box.pos_y = GetMouseY() + 86 + 20*widener;
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

short check_cd_in_drive(void)
{
    static const char *func_name="check_cd_in_drive";
//  _DK_check_cd_in_drive(); return;
  char *fname;
  const int img_width = 320;
  const int img_height = 200;
  short was_locked = LbScreenIsLocked();
  fname=prepare_file_path(FGrp_LoData,"dkwind00.dat");
  if ( LbFileExists(fname) )
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
  unsigned int i;
  counter=0;
  while ( !exit_keeper )
  {
      if ( LbFileExists(fname) )
        break;
      for (i=0;i<10;i++)
      {
        copy_raw8_image_to_screen_center(nocd_raw,img_width,img_height);
        while ( (!_DK_LbIsActive()) && (!exit_keeper) && (!quit_game) )
        {
          if (!LbWindowsControl())
          {
            exit_keeper = 1;
            break;
          }
        }
        if (is_key_pressed(KC_Q,KM_DONTCARE))
        {
          error(func_name, 77, "User requested quit, giving up");
          clear_key_pressed(KC_Q);
          exit_keeper = 1;
          break;
        }
        LbSleepFor(100);
      }
      // One 'counter' cycle lasts approx. 1 second.
      counter++;
      if (counter>300)
      {
          error(func_name, 79, "Wait time too long, giving up");
          exit_keeper = 1;
      }
  }
  LbSyncLog("Finished waiting for CD after %lu seconds\n",counter);
  _DK_LbDataFreeAll(nocd_load_files);
  if ( was_locked )
    LbScreenLock();
  return (!exit_keeper);
}

short get_gui_inputs(short gameplay_on)
{
  static const char *func_name="get_gui_inputs";
  //LbSyncLog("%s: Starting\n", func_name);
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
    const short gmax=86;//sizeof(active_buttons)/sizeof(struct GuiButton);
    _DK_drag_menu_x = -999;
    _DK_drag_menu_y = -999;
    int idx;
    for (idx=0;idx<gmax;idx++)
    {
      struct GuiButton *gbtn = &active_buttons[idx];
      if ((gbtn->field_0 & 1) && (gbtn->gbtype == 6))
          gbtn->field_1 = 0;
    }
  }

  int gidx;
  gidx = point_is_over_gui_menu(GetMouseX(), GetMouseY());
  if ( gidx == -1 )
    _DK_busy_doing_gui = 0;
  else
    _DK_busy_doing_gui = 1;
  int fmmenu_idx = first_monopoly_menu();

  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  int gmbtn_idx = -1;
  struct GuiButton *gbtn;
  // Sweep through buttons
  for (gidx=0; gidx<ACTIVE_BUTTONS_COUNT; gidx++)
  {
    gbtn = &active_buttons[gidx];
    if ((gbtn->field_0 & 1)==0)
      continue;
    if (!active_menus[gbtn->gmenu_idx].flgfield_1D)
      continue;
    Gf_Btn_Callback callback;
    callback = gbtn->field_17;
    if ( callback != NULL )
      callback(gbtn);
    if ( ((gbtn->field_1B & 0x4000u)!=0) || _DK_mouse_is_over_small_map(player->mouse_x,player->mouse_y) )
      continue;

    if ( check_if_mouse_is_over_button(gbtn) && (input_button==NULL)
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
      int mouse_x = GetMouseX();
      int btnsize;
      btnsize = gbtn->scr_pos_x + ((gbtn->slide_val)*(gbtn->width-64) >> 8);
      if ((mouse_x>(btnsize+22)) && (mouse_x<=(btnsize+44)))
      {
        int mouse_y = GetMouseY();
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
  if (input_button!=NULL)
  {
    _DK_busy_doing_gui = 1;
    if (_DK_get_button_area_input(input_button,input_button->id_num) != 0)
        result = 1;
  }
  if ((over_slider_button!=-1) && (_DK_left_button_released))
  {
      _DK_left_button_released = 0;
      if (gmbtn_idx!=-1)
        active_buttons[gmbtn_idx].field_1 = 0;
      over_slider_button = -1;
      do_sound_menu_click();
  }

  if (gmbtn_idx!=-1)
  {
    gbtn = &active_buttons[gmbtn_idx];
    if ((active_menus[gbtn->gmenu_idx].field_1 == 2) && ((gbtn->field_1B & 0x8000u)==0))
    {
      if (_DK_tool_tip_box.gbutton == gbtn)
      {
        if ((_DK_tool_tip_time>10) || (player->field_453==12))
        {
          _DK_busy_doing_gui = 1;
          if ( gbtn->field_13 != gui_area_text )
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
        _DK_tool_tip_box.pos_x = GetMouseX();
        _DK_tool_tip_box.pos_y = GetMouseY()+86;
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
    gbtn = &active_buttons[over_slider_button];
    int mouse_x = GetMouseX();
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
    gbtn = &active_buttons[gmbtn_idx];
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
    gbtn = &active_buttons[gidx];
    if (gbtn->field_0 & 1)
      if ( ((gmbtn_idx==-1) || (gmbtn_idx!=gidx)) && (gbtn->gbtype!=3) && (gbtn!=input_button) )
      {
        gbtn->field_0 &= 0xEFu;
        gbtn->field_1 = 0;
        gbtn->field_2 = 0;
      }
  }
  if ( gmbtn_idx != -1 )
  {
    Gf_Btn_Callback callback;
    gbtn = &active_buttons[gmbtn_idx];
    if ((gbtn->field_1) && (_DK_left_button_released))
    {
      callback = gbtn->click_event;
      result = 1;
      if ((callback!=NULL) || (gbtn->field_0 & 0x02) || (gbtn->field_2F!=0) || (gbtn->gbtype==3))
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
        struct Coord3d mappos;
        if ( screen_to_map(player->camera,GetMouseX(),GetMouseY(),&mappos) )
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
    set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  struct Thing *thing;
  thing = game.things_lookup[player->field_2F];
  if ((player->field_31 != thing->field_9) || ((thing->field_0 & 1)==0) )
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    set_packet_action(pckt, 32, player->field_2F,0,0,0);
    return true;
  }
  if (is_key_pressed(KC_TAB,KM_NONE))
  {
    clear_key_pressed(KC_TAB);
    toggle_gui_overlay_map();
  }
  return false;
}

void save_settings(void)
{
  _DK_save_settings();
}

void toggle_tooltips(void)
{
  const char *statstr;
  settings.tooltips_on = !settings.tooltips_on;
  if (settings.tooltips_on)
  {
    do_sound_menu_click();
    statstr = "on";
  } else
  {
    statstr = "off";
  }
  show_onscreen_msg(2*game.num_fps, "Tooltips %s", statstr);
  save_settings();
}

short gui_box_is_not_valid(struct GuiBox *gbox)
{
  return (gbox->field_0 & 0x01) == 0;
}

void gui_insert_box_at_list_top(struct GuiBox *gbox)
{
  static const char *func_name="gui_insert_box_at_list_top";
  if (gbox->field_0 & 0x02)
  {
    error(func_name, 425, "GuiBox is already in list");
    return;
  }
  gbox->field_0 |= 0x02;
  gbox->next_box = first_box;
  if (first_box != NULL)
      first_box->prev_box = gbox;
  else
      last_box = gbox;
  first_box = gbox;
}

struct GuiBox *gui_allocate_box_structure(void)
{
  int i;
  struct GuiBox *gbox;
  for (i=1;i<3;i++)
  {
    gbox = &gui_boxes[i];
    if (gui_box_is_not_valid(gbox))
    {
      gbox->field_1 = i;
      gbox->field_0 |= 0x01;
      gui_insert_box_at_list_top(gbox);
      return gbox;
    }
  }
  return NULL;
}

long gui_calculate_box_width(struct GuiBox *gbox)
{
  struct GuiBoxOption *goptn;
  int w,maxw;
  maxw = 0;
  goptn = gbox->optn_list;
  while (goptn->label[0] != '!')
  {
    w = pixel_size * LbTextStringWidth(goptn->label);
    if (w > maxw)
      maxw = w;
    goptn++;
  }
  return maxw+16;
}

long gui_calculate_box_height(struct GuiBox *gbox)
{
  struct GuiBoxOption *goptn;
  int i;
  i = 0;
  goptn = gbox->optn_list;
  while (goptn->label[0] != '!')
  {
    i++;
    goptn++;
  }
  return i*(pixel_size*LbTextStringHeight("Wp")+2) + 16;
}

void gui_remove_box_from_list(struct GuiBox *gbox)
{
  static const char *func_name="gui_remove_box_from_list";
  if ((gbox->field_0 & 0x02) == 0)
  {
    error(func_name, 460, "Cannot remove box from list when it is not in one!");
    return;
  }
  gbox->field_0 &= 0xFDu;
  if ( gbox->prev_box )
      gbox->prev_box->next_box = gbox->next_box;
  else
      first_box = gbox->next_box;
  if ( gbox->next_box )
      gbox->next_box->prev_box = gbox->prev_box;
  else
      last_box = gbox->prev_box;
  gbox->prev_box = 0;
  gbox->next_box = 0;
}

void gui_delete_box(struct GuiBox *gbox)
{
  gui_remove_box_from_list(gbox);
  memset(gbox, 0, sizeof(struct GuiBox));
}

struct GuiBox *gui_create_box(long x, long y, struct GuiBoxOption *optn_list)
{
  struct GuiBox *gbox;
  gbox = gui_allocate_box_structure();
  if (gbox == NULL)
    return NULL;
  gbox->optn_list = optn_list;
  gbox->pos_x=x;
  gbox->pos_y=y;
  gbox->width=gui_calculate_box_width(gbox);
  gbox->height=gui_calculate_box_height(gbox);
  return gbox;
}

/*
 * Toggles cheat menu. It should not allow cheats in Network mode.
 */
void toggle_main_cheat_menu(void)
{
  long mouse_x = GetMouseX();
  long mouse_y = GetMouseY();
  if ( (gui_box==NULL) || (gui_box_is_not_valid(gui_box)) )
  {
    gui_box=gui_create_box(mouse_x,mouse_y,gui_main_cheat_list);
  } else
  {
    gui_delete_box(gui_box);
    gui_box=NULL;
  }
}

void set_menu_visible_on(long menu_id)
{
  const short gmax=86;//sizeof(active_buttons)/sizeof(struct GuiButton);
  long menu_num;
  menu_num = menu_id_to_number(menu_id);
  if ( menu_num == -1 )
    return;
  active_menus[menu_num].flgfield_1D = 1;
  int idx;
  for (idx=0;idx<gmax;idx++)
  {
    struct GuiButton *gbtn = &active_buttons[idx];
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
  active_menus[menu_num].flgfield_1D = 0;
}

unsigned long toggle_status_menu(short visib)
{
  return _DK_toggle_status_menu(visib);
}

void zoom_from_map(void)
{
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ( (game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320) )
  {
      toggle_status_menu((game.numfield_C & 0x40) != 0);
      struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 120,1,0,0,0);
  } else
  {
      struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 80,6,0,0,0);
  }
}

short get_map_action_inputs()
{
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  long keycode;
  long mouse_x = GetMouseX();
  long mouse_y = GetMouseY();
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
      set_packet_action(pckt, 81,mappos_x,mappos_y,0,0);
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
    if (pckt->field_5)
      return true;
    if (is_key_pressed(KC_F8,KM_NONE))
    {
      clear_key_pressed(KC_F8);
      toggle_tooltips();
    }
    if (is_key_pressed(KC_NUMPADENTER,KM_NONE))
    {
      clear_key_pressed(KC_NUMPADENTER);
      toggle_main_cheat_menu();
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
      creature_query1_on = active_menus[gmnu_idx].flgfield_1D;
    set_menu_visible_off(31);
    // Menu no 35
    gmnu_idx=menu_id_to_number(35);
    if (gmnu_idx != -1)
      creature_query2_on = active_menus[gmnu_idx].flgfield_1D;
    set_menu_visible_off(31);
    return 1;
  }
}

void setup_engine_window(long x1, long y1, long x2, long y2)
{
  long cx1,cy1,cx2,cy2;
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (game.numfield_C & 0x20)
  {
    if (x1 >= 140)
    {
      cx1 = x1;
      if (x1 > MyScreenWidth)
        cx1 = MyScreenWidth;
    } else
    {
      cx1 = 140;
    }
  } else
  {
    if (x1 >= 0)
    {
      cx1 = x1;
      if (x1 > MyScreenWidth)
        cx1 = MyScreenWidth;
    } else
    {
      cx1 = 0;
    }
  }
  if (y1 >= 0)
  {
    cy1 = y1;
    if (y1 > MyScreenHeight)
      cy1 = MyScreenHeight;
  } else
  {
    cy1 = 0;
  }
  if (x2 >= 0)
  {
    cx2 = x2;
    if (cx1 + cx2 > MyScreenWidth)
      cx2 = MyScreenWidth - cx1;
  } else
  {
     cx2 = 0;
  }
  if (y2 >= 0)
  {
    cy2 = y2;
    if (cy1+y2 > MyScreenHeight)
      cy2 = MyScreenHeight - cy1;
  } else
  {
    cy2 = 0;
  }
  player->field_448 = cx1;
  player->field_44A = cy1;
  player->field_444 = cx2;
  player->field_446 = cy2;
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
    toggle_status_menu(is_visbl);
  if ( (game.numfield_D & 0x20) && (game.numfield_C & 0x20) )
    setup_engine_window(140, 0, MyScreenWidth, MyScreenHeight);
  else
    setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
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

short checksums_different(void)
{
  struct PlayerInfo *player;
  int i;
  int first = -1;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i]);
    if (((player->field_0 & 0x01) != 0) && ((player->field_0 & 0x40) == 0))
    {
        if (first == -1)
        {
          first = game.packets[player->field_B].field_4;
          continue;
        }
        if (game.packets[player->field_B].field_4 != first)
        {
          return true;
        }
    }
  }
  return false;
}

short get_creature_control_action_inputs(void)
{
  if ( ((game.numfield_C & 0x01)==0) || (game.numfield_C & 0x80) )
    get_gui_inputs(1);
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  long keycode;
  if (is_key_pressed(KC_NUMPADENTER,KM_DONTCARE))
  {
      clear_key_pressed(KC_NUMPADENTER);
      // Toggle cheat menu
      if ( (gui_box==NULL) || (gui_box_is_not_valid(gui_box)) )
      {
        gui_box=gui_create_box(200,20,gui_instance_option_list);
/*
        player->unknownbyte  |= 0x08;
        game.unknownbyte |= 0x08;
*/
      } else
      {
        gui_delete_box(gui_box);
        gui_box=NULL;
/*
        player->unknownbyte &= 0xF7;
        game.unknownbyte &= 0xF7;
*/
      }
      return 1;
  }
  if (is_key_pressed(KC_F12,KM_DONTCARE))
  {
      clear_key_pressed(KC_F12);
      // Cheat sub-menus
      if ( (gui_cheat_box==NULL) || (gui_box_is_not_valid(gui_cheat_box)) )
      {
        gui_cheat_box=gui_create_box(150,20,gui_creature_cheat_option_list);
/*
        player->unknownbyte  |= 0x08;
*/
      } else
      {
        gui_delete_box(gui_cheat_box);
        gui_cheat_box=NULL;
/*
        player->unknownbyte &= 0xF7;
*/
      }
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
      set_packet_action(pckt, 33, player->field_2F,0,0,0);
    }
  }
  if ( is_key_pressed(KC_TAB,KM_NONE) )
  {
    clear_key_pressed(KC_TAB);
    toggle_gui();
  }
  int numkey;
  numkey = -1;
  {
    for (keycode=KC_1;keycode<=KC_0;keycode++)
    {
      if ( is_key_pressed(keycode,KM_NONE) )
      {
        clear_key_pressed(keycode);
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
          set_packet_action(pckt, 39, instnce,0,0,0);
          break;
        }
        num_avail++;
      }
    }
  }
  return false;
}

void set_player_instance(struct PlayerInfo *player, long a2, int a3)
{
  _DK_set_player_instance(player, a2, a3);
}

long get_dungeon_control_action_inputs(void)
{
  return _DK_get_dungeon_control_action_inputs();
}

void get_level_lost_inputs(void)
{
  static const char *func_name="get_level_lost_inputs";
  //LbSyncLog("%s: Starting\n", func_name);
//  _DK_get_level_lost_inputs();
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  long keycode;
  if ( player->field_0 & 0x04 )
  {
    get_players_message_inputs();
    return;
  }
  if ((game.numfield_A & 0x01) != 0)
  {
    if (is_key_pressed(KC_RETURN,KM_NONE))
    {
      struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 13, 0,0,0,0);
      clear_key_pressed(KC_RETURN);
      return;
    }
  }
  if (is_key_pressed(KC_SPACE,KM_NONE))
  {
    struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
    set_packet_action(pckt, 5, 0,0,0,0);
    clear_key_pressed(KC_SPACE);
  }
  if ( player->field_452 == 4 )
  {
    int screen_x = GetMouseX() - 150;
    int screen_y = GetMouseY() - 56;
    if ( _DK_is_game_key_pressed(31, &keycode, 0) )
    {
      lbKeyOn[keycode] = 0;
      if (((game.numfield_A & 0x01) == 0) && (lbDisplay.PhysicalScreenWidth <= 320))
      {
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 80, 6,0,0,0);
      }
      else
      {
        toggle_status_menu((game.numfield_C & 0x40) != 0);
        struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 120, 1,0,0,0);
      }
    } else
    if ( _DK_right_button_released )
    {
        _DK_right_button_released = 0;
        if ( (game.numfield_A & 0x01) || lbDisplay.PhysicalScreenWidth > 320 )
        {
          toggle_status_menu((game.numfield_C & 0x40) != 0);
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          set_packet_action(pckt, 120, 1,0,0,0);
        }
        else
        {
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          set_packet_action(pckt, 80, 6,0,0,0);
        }
    } else
    if ( _DK_left_button_released )
    {
        int actn_x = 3*screen_x/4 + 1;
        int actn_y = 3*screen_y/4 + 1;
        if  ((actn_x >= 0) && (actn_x < map_subtiles_x) && (actn_y >= 0) && (actn_y < map_subtiles_y))
        {
          struct Packet *pckt=&game.packets[player->field_B%PACKETS_COUNT];
          set_packet_action(pckt, 81, actn_x,actn_y,0,0);
          _DK_left_button_released = 0;
        }
    }
  } else
  if ( player->field_452 == 1 )
  {
    if (is_key_pressed(KC_TAB,KM_DONTCARE))
    {
        if ((player->field_37 == 2) || (player->field_37 == 5))
        {
          clear_key_pressed(KC_TAB);
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
              if (toggle_status_menu(0))
                game.numfield_C |= 0x40;
              else
                game.numfield_C &= 0xBF;
              set_packet_action(pckt, 119, 4,0,0,0);
        } else
        {
              set_packet_action(pckt, 80, 5,0,0,0);
        }
        _DK_turn_off_roaming_menus();
      }
    }
  }
  if (is_key_pressed(KC_ESCAPE,KM_DONTCARE))
  {
    clear_key_pressed(KC_ESCAPE);
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
      inp_done=menu_is_active(38);
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
          set_player_instance(player, 10, 0);
        } else
        {
          inp_done=_DK_get_small_map_inputs(player->mouse_x, player->mouse_y, player->minimap_zoom / (3-pixel_size));
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
          set_packet_action(pckt, 33, player->field_2F,0,0,0);
      } else
      {
        set_packet_action(pckt, 33, player->field_2F,0,0,0);
      }
      break;
    case 3:
      set_packet_action(pckt, 32, player->field_2F,0,0,0);
      break;
    case 4:
      if ( menu_is_active(38) )
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
  static const char *func_name="get_inputs";
  //LbSyncLog("%s: Starting\n", func_name);
  //return _DK_get_inputs();
  long keycode;
  if (game.flags_cd & 0x01)
  {
    _DK_load_packets_for_turn(game.gameturn);
    game.gameturn++;
    if (is_key_pressed(KC_SPACE,KM_DONTCARE) ||
        is_key_pressed(KC_ESCAPE,KM_DONTCARE) ||
        is_key_pressed(KC_RETURN,KM_DONTCARE) ||
        (lbKeyOn[KC_LALT] && lbKeyOn[KC_X]) ||
        left_button_clicked)
    {
      clear_key_pressed(KC_SPACE);
      clear_key_pressed(KC_ESCAPE);
      clear_key_pressed(KC_RETURN);
      lbKeyOn[KC_X] = 0;
      left_button_clicked = 0;
      quit_game = 1;
    }
    //LbSyncLog("%s: Loading packets finished\n", func_name);
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
    //LbSyncLog("%s: Quit packet posted\n", func_name);
    return false;
  }
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (player->field_0 & 0x80)
  {
    struct Packet *pckt = &game.packets[player->field_B%PACKETS_COUNT];
    pckt->field_A = 127;
    pckt->field_C = 127;
    if ((input_button==NULL) && (game.numfield_C & 0x01))
    {
      if ( _DK_is_game_key_pressed(30, &keycode, 0) )
      {
        lbKeyOn[keycode] = 0;
        set_packet_action(pckt, 22, 0,0,0,0);
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
  if (input_button!=NULL)
    return false;
  struct Packet *pckt;
  switch ( player->field_452 )
  {
  case 1:
      if (!inp_handled)
        inp_handled=get_dungeon_control_action_inputs();
      _DK_get_dungeon_control_nonaction_inputs();
      get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 2:
      if (!inp_handled)
        inp_handled = get_creature_control_action_inputs();
      _DK_get_creature_control_nonaction_inputs();
      get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 3:
      if ( inp_handled )
      {
        get_player_gui_clicks();
        _DK_get_packet_control_mouse_clicks();
        return true;
      } else
      if ( get_creature_passenger_action_inputs() )
      {
        _DK_get_packet_control_mouse_clicks();
        return true;
      } else
      {
        get_player_gui_clicks();
        _DK_get_packet_control_mouse_clicks();
        return false;
      }
  case 4:
      if (!inp_handled)
        inp_handled = get_map_action_inputs();
      _DK_get_map_nonaction_inputs();
      get_player_gui_clicks();
      _DK_get_packet_control_mouse_clicks();
      return inp_handled;
  case 5:
      if (player->field_37==6)
        return false;
      if ( !(game.numfield_A & 0x01) )
        game.numfield_C &= 0xFE;
      if ( toggle_status_menu(0) )
        player->field_1 |= 0x01;
      else
        player->field_1 &= 0xFE;
      pckt = &game.packets[player->field_B%PACKETS_COUNT];
      set_packet_action(pckt, 80, 4,0,0,0);
      return false;
  case 6:
      if (player->field_37 != 7)
      {
        pckt = &game.packets[player->field_B%PACKETS_COUNT];
        set_packet_action(pckt, 80, 1,0,0,0);
      }
      return false;
  default:
      //LbSyncLog("%s: Default exit\n", func_name);
      return false;
  }
}

void input(void)
{
  static const char *func_name="input";
  //LbSyncLog("%s: Starting\n", func_name);
  //_DK_input();return;
  update_key_modifiers();
  if ((input_button) && (lbInkey>0))
  {
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
  //LbSyncLog("%s: Finished\n", func_name);
}

int LbNetwork_Exchange(struct Packet *pckt)
{
  return _DK_LbNetwork_Exchange(pckt);
}

unsigned long get_packet_save_checksum(void)
{
  return _DK_get_packet_save_checksum();
}

void clear_game(void)
{
  _DK_clear_game();
}

void resync_game(void)
{
  return _DK_resync_game();
}

void free_swipe_graphic(void)
{
  _DK_free_swipe_graphic();
}

void process_quit_packet(struct PlayerInfo *player, int a2)
{
  _DK_process_quit_packet(player,a2);
}

void frontend_save_continue_game(long lv_num, int a2)
{
  _DK_frontend_save_continue_game(lv_num,a2);
}

void frontstats_initialise(void)
{
  _DK_frontstats_initialise();
}

void message_add(char c)
{
  _DK_message_add(c);
}

void light_stat_light_map_clear_area(long x1, long y1, long x2, long y2)
{
  _DK_light_stat_light_map_clear_area(x1, y1, x2, y2);
}

void light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2)
{
  _DK_light_signal_stat_light_update_in_area(x1, y1, x2, y2);
}

void light_set_lights_on(char state)
{
  if (state)
  {
    game.field_46149 = 10;
    game.field_4614D = 1;
  } else
  {
    game.field_46149 = 32;
    game.field_4614D = 0;
  }
  light_stat_light_map_clear_area(0, 0, 255, 255);
  light_signal_stat_light_update_in_area(0, 0, 255, 255);
}

void process_pause_packet(long a1, long a2)
{
  _DK_process_pause_packet(a1, a2);
}

void change_engine_window_relative_size(long w_delta, long h_delta)
{
  struct PlayerInfo *myplyr;
  myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
  setup_engine_window(myplyr->field_448, myplyr->field_44A,
    myplyr->field_444+w_delta, myplyr->field_446+h_delta);
}

void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal)
{
  _DK_PaletteSetPlayerPalette(player, pal);
}

short set_gamma(char corrlvl, short do_set)
{
  static const char *func_name="set_gamma";
  char *fname;
  short result=1;
  if (corrlvl < 0)
    corrlvl = 0;
  else
  if (corrlvl > 4)
    corrlvl = 4;
  settings.gamma_correction = corrlvl;
  fname=prepare_file_fmtpath(FGrp_StdData,"pal%05d.dat",settings.gamma_correction);
  if (!LbFileExists(fname))
  {
    LbSyncLog("Palette file \"%s\" doesn't exist.\n", fname);
    result = 0;
  }
  if (result)
  {
    result = (LbFileLoadAt(fname, _DK_palette) != -1);
  }
  if ((result)&&(do_set))
  {
    struct PlayerInfo *myplyr;
    myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
    PaletteSetPlayerPalette(myplyr, _DK_palette);
  }
  if (!result)
    error(func_name, 3720, "Can't load palette file.");
  return result;
}

void centre_engine_window(void)
{
  long x1,y1;
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (game.numfield_C & 0x20)
    x1 = (MyScreenWidth-player->field_444-140) / 2 + 140;
  else
    x1 = (MyScreenWidth-player->field_444) / 2;
  y1 = (MyScreenHeight-player->field_446) / 2;
  setup_engine_window(x1, y1, player->field_444, player->field_446);
}

void set_engine_view(struct PlayerInfo *player, long val)
{
  _DK_set_engine_view(player, val);
}

void toggle_creature_tendencies(struct PlayerInfo *player, short val)
{
  _DK_toggle_creature_tendencies(player, val);
}

void set_player_state(struct PlayerInfo *player, short a1, long a2)
{
  _DK_set_player_state(player, a1, a2);
}

void set_player_mode(struct PlayerInfo *player, long val)
{
  _DK_set_player_mode(player, val);
}

void turn_off_query(short a)
{
  _DK_turn_off_query(a);
}

void turn_off_call_to_arms(long a)
{
  _DK_turn_off_call_to_arms(a);
}

long event_move_player_towards_event(struct PlayerInfo *player, long var)
{
  return _DK_event_move_player_towards_event(player,var);
}

long battle_move_player_towards_battle(struct PlayerInfo *player, long var)
{
  return _DK_battle_move_player_towards_battle(player, var);
}

long place_thing_in_power_hand(struct Thing *thing, long var)
{
  return _DK_place_thing_in_power_hand(thing, var);
}

short dump_held_things_on_map(unsigned int plyridx, long a2, long a3, short a4)
{
  return _DK_dump_held_things_on_map(plyridx, a2, a3, a4);
}

void magic_use_power_armageddon(unsigned int plridx)
{
  _DK_magic_use_power_armageddon(plridx);
}

long set_autopilot_type(unsigned int plridx, long aptype)
{
  return _DK_set_autopilot_type(plridx, aptype);
}

short magic_use_power_obey(unsigned short plridx)
{
  return _DK_magic_use_power_obey(plridx);
}

void turn_off_sight_of_evil(long plridx)
{
  _DK_turn_off_sight_of_evil(plridx);
}

void turn_off_event_box_if_necessary(long plridx, char val)
{
  _DK_turn_off_event_box_if_necessary(plridx, val);
}

void level_lost_go_first_person(long plridx)
{
  static const char *func_name="level_lost_go_first_person";
//  LbSyncLog("%s: Starting.\n",func_name);
  _DK_level_lost_go_first_person(plridx);
//  LbSyncLog("%s: Finished.\n",func_name);
}

void go_on_then_activate_the_event_box(long plridx, long val)
{
  _DK_go_on_then_activate_the_event_box(plridx, val);
}

void directly_cast_spell_on_thing(long plridx, unsigned char a2, unsigned short a3, long a4)
{
  _DK_directly_cast_spell_on_thing(plridx, a2, a3, a4);
}

void event_delete_event(long plridx, long num)
{
  _DK_event_delete_event(plridx, num);
}

void lose_level(struct PlayerInfo *player)
{
  _DK_lose_level(player);
}

void complete_level(struct PlayerInfo *player)
{
  _DK_complete_level(player);
}

void  toggle_ally_with_player(long plyridx, unsigned int allyidx)
{
  struct PlayerInfo *player=&(game.players[plyridx%PLAYERS_COUNT]);
  player->field_2A ^= (1 << allyidx);
}

void reveal_whole_map(struct PlayerInfo *player)
{
  //TODO
}

char process_players_global_packet_action(long plyridx)
{
  //TODO: add commands from beta
  //return _DK_process_players_global_packet_action(plyridx);
  struct PlayerInfo *player;
  struct PlayerInfo *myplyr;
  struct Packet *pckt;
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Room *room;
  int i;
  player=&(game.players[plyridx%PLAYERS_COUNT]);
  pckt=&game.packets[player->field_B%PACKETS_COUNT];
  switch (pckt->field_5)
  {
    case 1:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (my_player_number == plyridx)
      {
        frontstats_initialise();
        if ((game.numfield_A & 0x01) == 0)
        {
          int i,lv_num;
          i = myplyr->field_29;
          clear_game();
          myplyr->field_29 = i;
          if (myplyr->field_29 == 1)
            lv_num = game.level_number+1;
          else
            lv_num = game.level_number;
          frontend_save_continue_game(lv_num, 1);
        }
        free_swipe_graphic();
      }
      player->field_6 |= 0x02;
      process_quit_packet(player, 0);
      return 1;
    case 3:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (my_player_number == plyridx)
      {
        frontstats_initialise();
        if ((game.numfield_A & 0x01) == 0)
        {
          int i,lv_num;
          i = myplyr->field_29;
          clear_game();
          myplyr->field_29 = i;
          if (player->field_29 == 1)
            lv_num = game.level_number+1;
          else
            lv_num = game.level_number;
          frontend_save_continue_game(lv_num, 1);
        }
      }
      player->field_6 |= 0x02;
      process_quit_packet(player, 1);
      return 1;
    case 4:
      return 1;
    case 5:
      if (my_player_number == plyridx)
      {
        frontstats_initialise();
        free_swipe_graphic();
      }
      if ((game.numfield_A & 0x01) != 0)
      {
        process_quit_packet(player, 0);
        return 0;
      }
      switch (player->field_29)
      {
      case 1:
          complete_level(player);
          break;
      case 2:
          lose_level(player);
          break;
      }
      player->field_0 &= 0xFEu;
      if (my_player_number == plyridx)
      {
        unsigned int k;
        // Save some of the data from clearing
        myplyr = &(game.players[my_player_number%PLAYERS_COUNT]);
        dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
        i = myplyr->field_29;
        // The block started at field_1197 ends before field_131F
        memcpy(scratch, &dungeon->field_1197, 392);
        k = (myplyr->field_3 & 0x10) >> 4;
        // clear all data
        clear_game();
        // Restore saved data
        myplyr->field_29 = i;
        memcpy(&dungeon->field_1197, scratch, 392);
        myplyr->field_3 ^= (myplyr->field_3 ^ (k << 4)) & 0x10;
        if ((game.numfield_A & 0x01) == 0)
          frontend_save_continue_game(game.level_number, 1);
      }
      return 0;
    case PckT_PlyrMsgBegin:
      player->field_0 |= 0x04;
      return 0;
    case PckT_PlyrMsgEnd:
      player->field_0 &= 0xFBu;
      if (player->strfield_463[0] != '\0')
        message_add(player->field_2B);
      memset(player->strfield_463, 0, 64);
      return 0;
    case 20:
      if (my_player_number == plyridx)
        light_set_lights_on(game.field_4614D == 0);
      return 1;
    case PckT_SwitchScrnRes:
      if (my_player_number == plyridx)
        switch_to_next_video_mode();
      return 1;
    case 22:
      process_pause_packet((game.numfield_C & 0x01) == 0, pckt->field_6);
      return 1;
    case 24:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (myplyr->field_2B == plyridx)
      {
        settings.video_cluedo_mode = pckt->field_6;
        save_settings();
      }
      player->field_4DA = pckt->field_6;
      return 0;
    case 25:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (myplyr->field_2B == player->field_2B)
      {
        change_engine_window_relative_size(pckt->field_6, pckt->field_8);
        centre_engine_window();
      }
      return 0;
    case 26:
      player->field_90 = pckt->field_6 << 8;
      player->field_BA = pckt->field_6 << 8;
      player->field_3C = pckt->field_6 << 8;
      player->field_92 = pckt->field_8 << 8;
      player->field_BC = pckt->field_8 << 8;
      player->field_3E = pckt->field_8 << 8;
      return 0;
    case 27:
      if (myplyr->field_2B == player->field_2B)
      {
        set_gamma(pckt->field_6, 1);
        save_settings();
      }
      return 0;
    case PckT_SetMinimapConf:
      player->minimap_zoom = pckt->field_6;
      return 0;
    case 29:
      player->field_97 = pckt->field_6;
      player->field_C1 = pckt->field_6;
      player->field_43 = pckt->field_6;
      return 0;
    case 36:
      set_player_state(player, pckt->field_6, pckt->field_8);
      return 0;
    case 37:
      set_engine_view(player, pckt->field_6);
      return 0;
    case 55:
      toggle_creature_tendencies(player, pckt->field_6);
      return 0;
    case 60:
//      game.???[my_player_number].cheat_mode = 1;
      show_onscreen_msg(2*game.num_fps, "Cheat mode activated by player %d", plyridx);
      return 1;
    case 61:
      //TODO: remake from beta
/*
      if (word_5E674A != 0)
        word_5E674A = 0;
      else
        word_5E674A = 15;
*/
      return 1;
    case 62:
      //TODO: remake from beta
      return 0;
    case 63:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      reveal_whole_map(myplyr);
      return 0;
    case 64:
      //TODO: remake from beta
      return 0;
    case 65:
      //TODO: remake from beta
      return 0;
    case 66:
      //TODO: remake from beta
      return 0;
    case 67:
      //TODO: remake from beta
      return 0;
    case 68:
      //TODO: remake from beta
      return 0;
    case 69:
      //TODO: remake from beta
      return 0;
    case 80:
      set_player_mode(player, pckt->field_6);
      return 0;
    case 81:
      myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
      player->field_90 = pckt->field_6 << 8;
      player->field_BA = pckt->field_6 << 8;
      player->field_3C = pckt->field_6 << 8;
      player->field_92 = pckt->field_8 << 8;
      player->field_BC = pckt->field_8 << 8;
      player->field_3E = pckt->field_8 << 8;
      player->field_97 = 0;
      player->field_C1 = 0;
      player->field_43 = 0;
      if ((game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320))
      {
        if (my_player_number == plyridx)
          toggle_status_menu((game.numfield_C & 0x40) >> 6);
        set_player_mode(player, 1);
      } else
      {
        set_player_mode(player, 6);
      }
      return 0;
    case 82:
      process_pause_packet(pckt->field_6, pckt->field_8);
      return 1;
    case 83:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      event_move_player_towards_event(player, pckt->field_6);
      return 0;
    case 84:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      room = &game.rooms[pckt->field_6];
      player->field_E4 = room->field_8 << 8;
      player->field_E6 = room->field_9 << 8;
      set_player_instance(player, 16, 0);
      if (player->field_453 == 2)
        set_player_state(player, 2, room->field_A);
      return 0;
    case 85:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      thing = game.things_lookup[pckt->field_6];
      player->field_E4 = thing->mappos.x.val;
      player->field_E6 = thing->mappos.y.val;
      set_player_instance(player, 16, 0);
      if ((player->field_453 == 16) || (player->field_453 == 18))
        set_player_state(player, 16, thing->field_1A);
      return 0;
    case 86:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      thing = game.things_lookup[pckt->field_6];
      player->field_E4 = thing->mappos.x.val;
      player->field_E6 = thing->mappos.y.val;
      set_player_instance(player, 16, 0);
      if ((player->field_453 == 16) || (player->field_453 == 18))
        set_player_state(player, 18, thing->field_1A);
      return 0;
    case 87:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      player->field_E4 = pckt->field_6;
      player->field_E6 = pckt->field_8;
      set_player_instance(player, 16, 0);
      return 0;
    case 88:
      game.numfield_D ^= (game.numfield_D ^ (0x04 * ((game.numfield_D & 0x04) == 0))) & 0x04;
      return 0;
    case 89:
      turn_off_call_to_arms(plyridx);
      return 0;
    case 90:
      if (game.dungeon[plyridx].field_63 < 8)
        place_thing_in_power_hand(game.things_lookup[pckt->field_6], plyridx);
      return 0;
    case 91:
      dump_held_things_on_map(plyridx, pckt->field_6, pckt->field_8, 1);
      return 0;
    case 92:
      if (game.event[pckt->field_6].field_B == 3)
      {
        turn_off_event_box_if_necessary(plyridx, pckt->field_6);
      } else
      {
        event_delete_event(plyridx, pckt->field_6);
      }
      return 0;
    case 97:
      magic_use_power_obey(plyridx);
      return 0;
    case 98:
      magic_use_power_armageddon(plyridx);
      return 0;
    case 99:
      turn_off_query(plyridx);
      return 0;
    case 104:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      battle_move_player_towards_battle(player, pckt->field_6);
      return 0;
    case 106:
      if (player->field_453 == 15)
        turn_off_query(plyridx);
      dungeon = &(game.dungeon[plyridx]);
      switch (pckt->field_6)
      {
      case 5:
          if (dungeon->field_5D8)
          {
            struct Thing *thing;
            thing = game.things_lookup[dungeon->field_5D8];
            player->field_E4 = thing->mappos.x.val;
            player->field_E6 = thing->mappos.y.val;
            set_player_instance(player, 16, 0);
          }
          break;
      case 6:
          if (dungeon->field_884)
          {
            player->field_E4 = ((unsigned long)dungeon->field_881) << 8;
            player->field_E6 = ((unsigned long)dungeon->field_882) << 8;
            set_player_instance(player, 16, 0);
          }
          break;
      }
      if ( spell_data[pckt->field_6].field_0 )
      {
        for (i=0; i<21; i++)
        {
          if (spell_data[i].field_4 == player->field_453)
          {
            set_player_state(player, spell_data[pckt->field_6].field_4, 0);
            break;
          }
        }
      }
      return 0;
    case PckT_PlyrFastMsg:
      //show_onscreen_msg(game.num_fps, "Message from player %d", plyridx);
      output_message(pckt->field_6+110, 0, 1);
      return 0;
    case 109:
      set_autopilot_type(plyridx, pckt->field_6);
      return 0;
    case 110:
      level_lost_go_first_person(plyridx);
      return 0;
    case 111:
      if (game.dungeon[plyridx].field_63)
      {
        i = game.dungeon[plyridx].field_33;
        thing=game.things_lookup[i%THINGS_COUNT];
        dump_held_things_on_map(plyridx, thing->mappos.x.stl.num, thing->mappos.y.stl.num, 1);
      }
      return 0;
    case PckT_SpellSOEDis:
      turn_off_sight_of_evil(plyridx);
      return 0;
    case 115:
      go_on_then_activate_the_event_box(plyridx, pckt->field_6);
      return 0;
    case 116:
      turn_off_event_box_if_necessary(plyridx, game.dungeon[plyridx].field_1173);
      game.dungeon[plyridx].field_1173 = 0;
      return 0;
    case 117:
      i = player->field_4D2 / 4;
      if (i > 8) i = 8;
      directly_cast_spell_on_thing(plyridx, pckt->field_6, pckt->field_8, i);
      return 0;
    case PckT_PlyrToggleAlly:
      toggle_ally_with_player(plyridx, pckt->field_6);
      return 0;
    case 119:
      player->field_4B5 = player->camera->field_6;
      set_player_mode(player, pckt->field_6);
      return 0;
    case 120:
      set_player_mode(player, pckt->field_6);
      set_engine_view(player, player->field_4B5);
      return 0;
    default:
      return 0;
  }
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
  LbFileSeek(game.packet_save_fp, 0, 2);
  // Note: originally only 48 bytes were saved; I guess it was a mistake (now 55 are saved).
  for (i=0; i<PACKETS_COUNT; i++)
    memcpy(pckt_buf[sizeof(struct Packet)*i], &game.packets[i], sizeof(struct Packet));
  LbFileWrite(game.packet_save_fp, &pckt_buf, sizeof(struct Packet)*PACKETS_COUNT);
  if ( !LbFileFlush(game.packet_save_fp) )
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
    chr = key_to_ascii(pcktd->field_6, pcktd->field_8);
    chpos = strlen(player->strfield_463);
    if (pcktd->field_6 == KC_BACK)
    {
      if (chpos>0)
        player->strfield_463[chpos-1] = '\0';
    } else
    if ((chr >= 'a') && (chr <= 'z') ||
        (chr >= 'A') && (chr <= 'Z') ||
        (chr >= '0') && (chr <= '9') || (chr == ' ')  || (chr == '!') ||
        (chr == '.'))
    {
      if (chpos < 63)
      {
        player->strfield_463[chpos] = toupper(chr);
        player->strfield_463[chpos+1] = '\0';
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
    set_player_instance(player, 8, 0);
  }
}

void process_players_packet(long idx)
{
  static const char *func_name="process_players_packet";
  struct PlayerInfo *player;
  struct Packet *pckt;
  player=&(game.players[idx%PLAYERS_COUNT]);
  pckt = &game.packets[player->field_B%PACKETS_COUNT];
  //LbSyncLog("%s: Processing packet %d of type %d.\n",func_name,idx,(int)pckt->field_5);
  player->field_4 = (pckt->field_10 & 0x20) >> 5;
  player->field_5 = (pckt->field_10 & 0x40) >> 6;
  if ( (player->field_0 & 0x04) && (pckt->field_5 == PckT_PlyrMsgChar))
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
  static const char *func_name="process_packets";
  //_DK_process_packets();return;

  int i,j,k;
  struct Packet *pckt;
  struct PlayerInfo *player;
  // Do the network data exchange
  lbDisplay.DrawColour = colours[15][15][15];
  // Exchange packets with the network
  if (game.flagfield_14EA4A != 2)
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
          player->field_0 |= 0x40;
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
//Debug code, to find packet errors
//write_debug_packets();
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
    LbSyncLog("%s: Resyncing.\n",func_name);
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

short LbIsActive(void)
{
  return _DK_LbIsActive();
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
      {
          process_player_research(i);
      }
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
      {
          process_player_manufacturing(i);
      }
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
  int i;
  anim_counter = (anim_counter+1) % 8;
  short result=true;
  for (i=0; i<TEXTURE_BLOCKS_ANIM_COUNT; i++)
  {
        short j = game.texture_animation[8*i+anim_counter];
        if ((j>=0) && (j<TEXTURE_BLOCKS_STAT_COUNT))
        {
          block_ptrs[TEXTURE_BLOCKS_STAT_COUNT+i] = block_ptrs[j];
        } else
        {
          result=false;
        }
  }
  return result;
}

void draw_slab64k(long pos_x, long pos_y, long width, long height)
{
  _DK_draw_slab64k(pos_x, pos_y, width, height);
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
  static const char *func_name="update";
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

    for (i=0; i<=game.field_14E496; i++)
    {
      short *v7; // edx@16
      v7 = (short *)&game.dungeon[i];
      memset(v7 + 50, 0, 0x3C0u);
      memset(v7 + 530, 0, 0xC0u);
      memset(v7 + 626, 0, 0xC0u);
//      memset(game.dungeon[i].field_64, 0, 960);
//      memset(game.dungeon[i].field_424, 0, 12);
//      memset(game.dungeon[i].field_4E4, 0, 12);
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
  if ((game.seedchk_random_used != 0) && ((game.seedchk_random_used % 0x4E20) == 0))
  {
      if (seed_check_random(0x7D0u, &game.field_14BB4A, func_name, 4345) == 0)
      {
        if (seed_check_random(10, &game.field_14BB4E, func_name, 4346) == 7)
        {
          output_message(94, 0, 1);// 'Your pants are definitely too tight'
        } else
        {
          output_message((game.field_14BB4E % 10) + 91, 0, 1);
        }
      }
  }
  game.field_14EA4B = 0;
}

void draw_tooltip(void)
{
  _DK_draw_tooltip();
}

long map_fade_in(long a)
{
  return _DK_map_fade_in(a);
}

long map_fade_out(long a)
{
  return _DK_map_fade_out(a);
}

void draw_2d_map(void)
{
  _DK_draw_2d_map();
}

void draw_eastegg(void)
{
  static char text[255];
  int i;
  _DK_LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  if (eastegg03_cntr > 6)
  {
      unsigned char pos;
      eastegg03_cntr++;
      lbFontPtr = winfont;
      sprintf(text, "Dene says a big 'Hello' to Goth Buns, Tarts and Barbies");
      lbDisplay.DrawFlags = 64;
      for (i=0; i<30; i+=2)
      {
        pos = game.seedchk_random_used - i;
        lbDisplay.DrawColour = pos;
        LbTextDraw((lbCosTable[16*(pos&0x7F)] / 512 + 120) / pixel_size,
          (lbSinTable[32*(pos&0x3F)] / 512 + 200) / pixel_size, text);
      }
      lbDisplay.DrawFlags &= 0xFFBFu;
      pos=game.seedchk_random_used;
      LbTextDraw((lbCosTable[16*(pos&0x7F)] / 512 + 120) / pixel_size,
        (lbSinTable[32*(pos&0x3F)] / 512 + 200) / pixel_size, text);
      if (eastegg03_cntr > 1000)
        eastegg03_cntr = 0;
  }
  _DK_draw_eastegg();
}

void process_pointer_graphic(void)
{
  _DK_process_pointer_graphic();
}

void message_draw(void)
{
  _DK_message_draw();
}

void redraw_creature_view(void)
{
  _DK_redraw_creature_view();
}

void redraw_isometric_view(void)
{
  _DK_redraw_isometric_view();
}

void redraw_frontview(void)
{
  _DK_redraw_frontview();
}

void draw_zoom_box(void)
{
  _DK_draw_zoom_box();
}

struct GuiBox *gui_get_highest_priority_box(void)
{
  return first_box;
}

struct GuiBox *gui_get_lowest_priority_box(void)
{
  return last_box;
}

struct GuiBox *gui_get_next_highest_priority_box(struct GuiBox *gbox)
{
  return gbox->prev_box;
}

struct GuiBox *gui_get_next_lowest_priority_box(struct GuiBox *gbox)
{
  return gbox->next_box;
}

struct GuiBox *gui_get_box_point_over(long x, long y)
{
//TODO from beta
  return NULL;
}

struct GuiBoxOption *gui_get_box_option_point_over(struct GuiBox *gbox, long x, long y)
{
//TODO from beta
  return NULL;
}

void gui_draw_box(struct GuiBox *gbox)
{
  //LbSyncLog("Drawing box, first optn \"%s\"\n",gbox->optn_list->label);
  struct GuiBox *gbox_over;
  struct GuiBoxOption *goptn_over;
  struct GuiBoxOption *goptn;
  long lnheight;
  long pos_x,pos_y;
  _DK_LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);

  goptn_over = NULL;
  gbox_over = gui_get_box_point_over(GetMouseX(), GetMouseY());
  if (gbox_over != NULL)
  {
    goptn_over = gui_get_box_option_point_over(gbox_over, GetMouseX(), GetMouseY());
  }

  lnheight = pixel_size * LbTextStringHeight("Wp") + 2;
  pos_y = gbox->pos_y + 8;
  pos_x = gbox->pos_x + 8;
  if (gbox != gui_get_highest_priority_box())
  {
    lbDisplay.DrawFlags |= 0x0004;
    LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[6][0][0]);
    if (lbDisplay.DrawFlags & 0x0010)
    {
      LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[0][0][0]);
    } else
    {
      lbDisplay.DrawFlags ^= 0x0010;
      LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[0][0][0]);
      lbDisplay.DrawFlags ^= 0x0010;
    }
    lbDisplay.DrawFlags ^= 0x0004;
    lbDisplay.DrawColour = colours[3][3][3];
    goptn = gbox->optn_list;
    while (goptn->label[0] != '!')
    {
      if (goptn->active_cb != NULL)
        goptn->field_26 = (goptn->active_cb)(gbox, goptn, &goptn->field_D);
      else
        goptn->field_26 = 1;
      if (!goptn->field_26)
        lbDisplay.DrawColour = colours[0][0][0];
      else
        lbDisplay.DrawColour = colours[3][3][3];
      if (LbScreenIsLocked())
      {
        LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, goptn->label);
      }
      goptn++;
      pos_y += lnheight;
    }
  } else
  {
    lbDisplay.DrawFlags |= 0x0004;
    LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[12][0][0]);
    if (lbDisplay.DrawFlags & 0x0010)
    {
      LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[2][0][0]);
    } else
    {
      lbDisplay.DrawFlags ^= 0x0010;
      LbDrawBox(gbox->pos_x/pixel_size, gbox->pos_y/pixel_size, gbox->width/pixel_size, gbox->height/pixel_size, colours[2][0][0]);
      lbDisplay.DrawFlags ^= 0x0010;
    }
    lbDisplay.DrawFlags ^= 0x0004;
    goptn = gbox->optn_list;
    while (goptn->label[0] != '!')
    {
      if (goptn->active_cb != NULL)
        goptn->field_26 = (goptn->active_cb)(gbox, goptn, &goptn->field_D);
      else
        goptn->field_26 = 1;
      if (!goptn->field_26)
        lbDisplay.DrawColour = colours[0][0][0];
      else
      if ((gbox == gbox_over) && (goptn == goptn_over) && (gbox != dragging_box.gbox) ||
           (gbox != NULL) && (goptn->field_25 != 0))
        lbDisplay.DrawColour = colours[15][15][15];
      else
        lbDisplay.DrawColour = colours[9][9][9];
      if (LbScreenIsLocked())
      {
        LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, goptn->label);
      }
      goptn++;
      pos_y += lnheight;
    }
  }
}

void gui_draw_all_boxes(void)
{
  struct GuiBox *gbox;
  lbDisplay.DrawFlags = 0x0040;
  lbFontPtr = font_sprites;
  gbox = gui_get_lowest_priority_box();
  while (gbox != NULL)
  {
    gui_draw_box(gbox);
    gbox = gui_get_next_highest_priority_box(gbox);
  }
}

void draw_map_level_name(void)
{
  char *lv_name;
  lv_name = NULL;
  if (is_campaign_level(game.numfield_14A83D))
  {
    lv_name = strings[game.numfield_14A83D+201];
  } else
  if ( is_bonus_level(game.numfield_14A83D) )
  {
    lv_name = strings[430];
  } else
  if (is_multiplayer_level(game.numfield_14A83D))
  {
    lv_name = level_name;
  }
  if (lv_name != NULL)
  {
    lbFontPtr = winfont;
    lbDisplay.DrawFlags = 0;
    _DK_LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    LbTextDraw((MyScreenWidth-pixel_size*LbTextStringWidth(lv_name))/2 / pixel_size, 32 / pixel_size, lv_name);
  }
}

void redraw_parchment_view(void)
{
  char *fname;
  if ( !parchment_loaded )
  {
    if (lbDisplay.PhysicalScreenWidth < 640)
    {
      fname=prepare_file_path(FGrp_StdData,"gmap.raw");
      LbFileLoadAt(fname, poly_pool);
    } else
    {
      fname=prepare_file_path(FGrp_StdData,"gmaphi.raw");
      LbFileLoadAt(fname, hires_parchment);
    }
    parchment_loaded = 1;
  }
  draw_map_parchment();
  draw_2d_map();
  draw_gui();
  gui_draw_all_boxes();
  draw_zoom_box();
  draw_map_level_name();
  draw_tooltip();
}

void redraw_display(void)
{
  //_DK_redraw_display();return;
  static char text[255];
  struct PlayerInfo *player;
  int i;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);

  player->field_6 &= 0xFEu;
  if (game.flagfield_14EA4A == 1)
    return;

  if ( grabbed_small_map == 2 )
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  else
    process_pointer_graphic();
  switch (player->field_37)
  {
  case 1:
      redraw_creature_view();
      parchment_loaded = 0;
      break;
  case 2:
      redraw_isometric_view();
      parchment_loaded = 0;
      break;
  case 3:
      redraw_parchment_view();
      break;
  case 5:
      redraw_frontview();
      parchment_loaded = 0;
      break;
  case 6:
      parchment_loaded = 0;
      player->field_4BD = map_fade_in(player->field_4BD);
      break;
  case 7:
      parchment_loaded = 0;
      player->field_4BD = map_fade_out(player->field_4BD);
      break;
  default:
      break;
  }
  _DK_LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  lbFontPtr = winfont;
  lbDisplay.DrawFlags &= 0xFFBFu;
  _DK_LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  if (player->field_0 & 0x04)
  {
      sprintf(text, ">%s_", player->strfield_463);
      LbTextDraw(148/pixel_size, 8/pixel_size, text);
  }
  if ( draw_spell_cost )
  {
      long pos_x,pos_y;
      unsigned short drwflags_mem;
      drwflags_mem = lbDisplay.DrawFlags;
      _DK_LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
      lbDisplay.DrawFlags = 0;
      lbFontPtr = winfont;
      sprintf(text, "%d", draw_spell_cost);
      pos_y = GetMouseY() - pixel_size*LbTextStringHeight(text)/2 - 2;
      pos_x = GetMouseX() - pixel_size*LbTextStringWidth(text)/2;
      LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, text);
      lbDisplay.DrawFlags = drwflags_mem;
      draw_spell_cost = 0;
  }
  if ( is_bonus_level(game.numfield_14A83D) )
    message_draw();
  if (((game.numfield_C & 0x01) != 0) && ((game.numfield_C & 0x80) == 0))
  {
        lbFontPtr = winfont;
        long pos_x,pos_y;
        long w,h;
        int i;
        i = 0;
        if (lbFontPtr != NULL)
          i = lbFontPtr[1].SWidth;
        w = pixel_size * (LbTextStringWidth(strings[320]) + 2*i);
        i = player->field_37;
        if ((i == 2) || (i == 5) || (i == 1))
          pos_x = player->field_448 + (MyScreenWidth-w-player->field_448)/2;
        else
          pos_x = (MyScreenWidth-w)/2;
        pos_y=16;
        i = 0;
        if (lbFontPtr != NULL)
          i = lbFontPtr[1].SHeight;
        lbDisplay.DrawFlags = 0x0100;
        h = pixel_size*i + pixel_size*i/2;
        _DK_LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
        draw_slab64k(pos_x, pos_y, w, h);
        LbTextDraw(0/pixel_size, 0/pixel_size, strings[320]);
        _DK_LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  }
  if (game.field_150356 != 0)
  {
    long pos_x,pos_y;
    long w,h;
    int i;
    if (game.armageddon+game.field_150356 <= game.seedchk_random_used)
    {
      i = 0;
      if ( game.field_15035A - game.field_1517EC <= game.seedchk_random_used )
        i = game.field_15035A - game.seedchk_random_used;
    } else
    {
      i = game.seedchk_random_used - game.field_150356 - game.armageddon;
    }
    lbFontPtr = winfont;
    sprintf(text, " %s %03d", strings[646], i/2);
    i = 0;
    if (lbFontPtr != NULL)
      i = lbFontPtr[1].SWidth;
    w = pixel_size*LbTextStringWidth(text) + 6*i;
    pos_x = MyScreenWidth - w - 16;
    pos_y = 16;
    i = 0;
    if (lbFontPtr != NULL)
      i = lbFontPtr[1].SHeight;
    lbDisplay.DrawFlags = 0x0100;
    h = pixel_size*i + pixel_size*i/2;
    _DK_LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
    draw_slab64k(pos_x, pos_y, w, h);
    LbTextDraw(0/pixel_size, 0/pixel_size, text);
    _DK_LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  }
  draw_eastegg();
}

/*
 * Checks if the game screen needs redrawing.
 */
short keeper_check_if_shall_draw(void)
{
  static TbClockMSec prev_time1=0;
  static TbClockMSec cntr_time1=0;
  static TbClockMSec prev_time2=0;
  static TbClockMSec cntr_time2=0;
  if ((game.numfield_C & 0x01) != 0)
    return true;
  if ( (game.turns_fastforward==0) && (!game.numfield_149F38) )
  {
          unsigned long curr_time;
          curr_time = clock();
          cntr_time2++;
          if ( curr_time-prev_time2 >= 1000 )
          {
              double time_fdelta = 1000.0*((double)(cntr_time2))/(curr_time-prev_time2);
              prev_time2 = curr_time;
              game.time_delta = (unsigned long)(time_fdelta*256.0);
              cntr_time2 = 0;
          }
          if ( (game.timingvar1!=0) && (game.seedchk_random_used % game.timingvar1) )
          {
            return false;
          }
  } else
  if ( ((game.seedchk_random_used & 0x3F)==0) ||
       ((game.numfield_149F38) && ((game.seedchk_random_used & 7)==0)) )
  {
            TbClockMSec curr_time;
            curr_time = clock();
            if ((curr_time-prev_time1) < 5000)
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
      return false;
  }
  return true;
}

/*
 * Makes last updates to the video buffer, and swaps buffers to show
 * the new image.
 */
short keeper_screen_swap(void)
{
/*  // For resolution 640x480, move the graphics data 40 lines lower
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
    }*/
  LbScreenSwap();
  return 1;
}

/*
 * Waits until the next game turn. Delay is usually controlled by
 * num_fps variable.
 */
short keeper_wait_for_next_turn(void)
{
  if (game.numfield_D & 0x10)
  {
      // No idea when such situation occurs
      TbClockMSec sleep_end = last_loop_time + 1000;
      LbSleepUntil(sleep_end);
      last_loop_time = LbTimerClock();
      return 1;
  }
  if (game.timingvar1 == 0)
  {
      // Standard delaying system
      TbClockMSec sleep_end = last_loop_time + 1000/game.num_fps;
      LbSleepUntil(sleep_end);
      last_loop_time = LbTimerClock();
      return 1;
  }
  return 0;
}

/*
 * Redraws the game display buffer.
 */
short keeper_screen_redraw(void)
{
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  LbScreenClear(0);
  if ( LbScreenLock() == 1 )
  {
    setup_engine_window(player->field_448, player->field_44A,
        player->field_444, player->field_446);
    redraw_display();
    LbScreenUnlock();
    return 1;
  }
  return 0;
}

/*
 * Draws the crucial warning messages on screen.
 * Requires the screen to be locked before.
 */
short draw_onscreen_direct_messages(void)
{
  static const char *func_name="draw_onscreen_direct_messages";
  static char text[255];
  unsigned int msg_pos;
  // Display in-game message for debug purposes
  if (onscreen_msg_turns > 0)
  {
      if ( LbScreenIsLocked() )
        LbTextDraw(260/pixel_size, 0/pixel_size, onscreen_msg_text);
      onscreen_msg_turns--;
  }
  msg_pos = 200;
  if (game.numfield_A & 0x02)
  {
      sprintf(text, "OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
      error(func_name, 413, text);
      if ( LbScreenIsLocked() )
        LbTextDraw(300/pixel_size, msg_pos/pixel_size, "OUT OF SYNC");
      msg_pos += 20;
  }
  if (game.numfield_A & 0x04)
  {
      sprintf(text, "SEED OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
      error(func_name, 427, text);
      if ( LbScreenIsLocked() )
        LbTextDraw(300/pixel_size, msg_pos/pixel_size, "SEED OUT OF SYNC");
      msg_pos += 20;
  }
  return 1;
}

void keeper_gameplay_loop(void)
{
    static const char *func_name="keeper_gameplay_loop";
    short do_draw;
    struct PlayerInfo *player;
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    PaletteSetPlayerPalette(player, _DK_palette);
    if (game.numfield_C & 0x02)
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

      // Check if we should redraw screen in this turn
      do_draw = keeper_check_if_shall_draw() || (!LbIsActive());

      update_mouse();
      input_eastegg();
      input();
      update();

      if ( do_draw )
        keeper_screen_redraw();

      do {
        if ( !LbWindowsControl() )
        {
          if ((game.numfield_A & 0x01) == 0)
          {
            exit_keeper = 1;
            break;
          }
          LbSyncLog("%s - Alex's point reached\n",func_name);
        }
        if ( (game.numfield_A & 0x01) || LbIsActive() )
          break;
      } while ((!exit_keeper) && (!quit_game));

      // Direct information/error messages
      if ( LbScreenLock() == 1 )
      {
        if ( do_draw )
          perform_any_screen_capturing();
        draw_onscreen_direct_messages();
        LbScreenUnlock();
      }

      // Music and sound control
      if ( !SoundDisabled )
        if ((game.turns_fastforward==0) && (!game.numfield_149F38))
        {
            MonitorStreamedSoundTrack();
            _DK_process_sound_heap();
        }

      // Move the graphics window to center of screen buffer and swap screen
      if ( do_draw )
        keeper_screen_swap();

      // Make delay if the machine is too fast
      if ( (!game.field_149E81) || (game.turns_fastforward==0) )
        keeper_wait_for_next_turn();

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

short continue_game_available()
{
  char buf[12];
  char *fname;
  static short continue_needs_checking_file = 1;
  check_cd_in_drive();
  fname=prepare_file_path(FGrp_Save,"continue.sav");
  if (LbFileLength(fname) != sizeof(struct Game))
    return false;
  if ( continue_needs_checking_file )
  {
    TbFileHandle fh=LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
    if ( fh != -1 )
    {
            LbFileRead(fh, buf, 10);
            LbFileClose(fh);
            if (buf[0])
              game.level_number = (unsigned char)buf[0];
    }
    continue_needs_checking_file = 0;
  }
  if (game.level_number > 20)
    return false;
  else
    return true;
}

void intro(void)
{
    char *fname;
    fname=prepare_file_path(FGrp_LoData,"intromix.smk");
    LbSyncLog("Playing intro movie \"%s\"\n",fname);
    play_smacker_file(fname, 1);
}

void outro(void)
{
    char *fname;
    fname=prepare_file_path(FGrp_LoData,"outromix.smk");
    LbSyncLog("Playing outro movie \"%s\"\n",fname);
    play_smacker_file(fname, 17);
}

unsigned long seed_check_random(unsigned long range, unsigned long *seed, const char *func_name, unsigned long place)
{
  if (range == 0)
    return 0;
  unsigned long i;
  i = 9377 * (*seed) + 9439;
  *seed = _lrotr(i, 13);
  i = (*seed) % range;
/*
  if (byte_5642DD & 0x01)
  {
      if (a2 == &dword_5E6742)
        LbSyncLog("%s: place %d, val %d\n", func_name, place, i);
  }
*/
  return i;
}

void open_packet_file_for_load(char *fname)
{
  _DK_open_packet_file_for_load(fname); return;
}

void init_level(void)
{
  _DK_init_level();
}

void pannel_map_update(long x, long y, long w, long h)
{
  _DK_pannel_map_update(x, y, w, h);
}

void init_player(struct PlayerInfo *player, int a2)
{
  static const char *func_name="init_player";
  //_DK_init_player(player, a2); return;
  player->mouse_x = 10;
  player->mouse_y = 12;
  player->minimap_zoom = 256;
  player->field_4D1 = player->field_2B;
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  player->field_456 = 1;
  player->field_453 = 1;
  player->field_14 = 2;
  player->field_4C9 = _DK_palette;
  if (player == &game.players[my_player_number])
  {
    game.numfield_C |= 0x0040;
    set_gui_visible(1);
    init_gui();
    turn_on_menu(1);
    turn_on_menu(2);
  }
  switch (game.flagfield_14EA4A)
  {
  case 2:
    init_player_as_single_keeper(player);
    init_player_start(player);
    reset_player_mode(player, 1);
    if ( !a2 )
      init_keeper_map_exploration(player);
    break;
  case 5:
    if (player->field_2C != 1)
    {
      error(func_name, 290, "Non Keeper in Keeper game");
      break;
    }
    init_player_as_single_keeper(player);
    init_player_start(player);
    reset_player_mode(player, 1);
    init_keeper_map_exploration(player);
    break;
  default:
    error(func_name, 309, "How do I set up this player?");
    break;
  }
  init_player_cameras(player);
  pannel_map_update(0, 0, 256, 256);
  player->strfield_463[0] = '\0';
  if (player == &game.players[my_player_number])
  {
    game.audiotrack = ((game.numfield_14A83D - 1) % -4) + 3;
    StopMusic();
    switch (seed_check_random(3, &game.field_14BB4E, func_name, 363))
    {
    case 0:
      if (LoadAwe32Soundfont("bullfrog"))
        StartMusic(1, 127);
      break;
    case 1:
      if (LoadAwe32Soundfont("atmos1"))
        StartMusic(1, 127);
      break;
    case 2:
      if (LoadAwe32Soundfont("atmos2"))
        StartMusic(1, 127);
      break;
    }
  }
  player->field_2A = (1 << player->field_2B);
  player->field_10 = 0;
}

void post_init_level(void)
{
  _DK_post_init_level(); return;
}

void post_init_players(void)
{
  _DK_post_init_players(); return;
}

void startup_saved_packet_game(void)
{
  //_DK_startup_saved_packet_game(); return;
  struct PlayerInfo *player;
  int i;
  memset(game.packets, 0, sizeof(struct Packet)*PACKETS_COUNT);
  open_packet_file_for_load(game.packet_fname);
  game.numfield_14A83D = game.packet_save_head.field_4;
  lbDisplay.DrawColour = colours[15][15][15];
  game.gameturn = 0;
  LbSyncLog("Initialising level %d\n", game.numfield_14A83D);
  LbSyncLog("Packet Loading Active (File contains %d turns)\n", game.field_149F30);
  if ( game.packet_checksum )
    LbSyncLog("Packet Checksum Active\n");
  LbSyncLog("Fast Forward through %d game turns\n", game.turns_fastforward);
  if (game.numfield_149F42 != -1)
    LbSyncLog("Packet Quit at %d\n", game.numfield_149F42);
  if ( game.field_149E81 )
  {
    if (game.numfield_149F3E != game.numfield_149F3A)
      LbSyncLog("Logging things, game turns %d -> %d\n", game.numfield_149F3A, game.numfield_149F3E);
  }
  game.flagfield_14EA4A = 2;
  if (!(game.packet_save_head.field_C & (1 << game.numfield_149F46))
    || (game.packet_save_head.field_D & (1 << game.numfield_149F46)) )
    my_player_number = 0;
  else
    my_player_number = game.numfield_149F46;
  init_level();
  for (i=0;i<PLAYERS_COUNT;i++)
  {
    player=&(game.players[i]);
    player->field_0 ^= (player->field_0 ^ ((game.packet_save_head.field_C & (1 << i)) >> i)) & 1;
    if (player->field_0 & 0x01)
    {
      player->field_2B = i;
      player->field_0 ^= (player->field_0 ^ (((game.packet_save_head.field_D & (1 << i)) >> i) << 6)) & 0x40;
      if ((player->field_0 & 0x40) == 0)
      {
        game.field_14E495++;
        player->field_2C = 1;
        game.flagfield_14EA4A = 5;
        init_player(player, 0);
      }
    }
  }
  if (game.field_14E495 == 1)
    game.flagfield_14EA4A = 2;
  if (game.field_149F30 < game.turns_fastforward)
    game.turns_fastforward = game.field_149F30;
  post_init_level();
  post_init_players();
}

void faststartup_saved_packet_game(void)
{
    reenter_video_mode();
    startup_saved_packet_game();
    game.players[my_player_number%PLAYERS_COUNT].field_6 &= 0xFDu;
}

short frontend_is_player_allied(long plyr1, long plyr2)
{
  return _DK_frontend_is_player_allied(plyr1, plyr2);
}

long script_support_setup_player_as_computer_keeper(unsigned char plyridx, long a2)
{
  return _DK_script_support_setup_player_as_computer_keeper(plyridx, a2);
}

void startup_network_game(void)
{
  static const char *func_name="startup_network_game";
  LbSyncLog("Starting up network game.\n");
  //_DK_startup_network_game(); return;
  int i,k;
  short is_set;
  short checksum_bad;
  int checksum_mem;
  unsigned int flgmem;
  struct PlayerInfo *player;
  struct Packet *pckt;
  struct Thing *thing;
  if (game.flagfield_14EA4A == 2)
  {
    game.field_14E495 = 1;
  } else
  {
    game.field_14E495 = 0;
    for (i=0; i<NET_PLAYERS_COUNT; i++)
    {
      if (net_player_info[i].field_20)
        game.field_14E495++;
    }
  }
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  i = player->field_2C;
  init_level();
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_2C = i;
  if (game.flagfield_14EA4A == 2)
  {
    player->field_2B = my_player_number;
    player->field_0 |= 0x01;
    player->field_4B5 = (-(settings.field_3 < 1u) & 0xFD) + 5;
    init_player(player, 0);
  } else
  {
    if (LbNetwork_ChangeExchangeBuffer(game.packets, 17))
      error(func_name, 119, "Unable to reinitialise ExchangeBuffer");
    flgmem = game.players[my_player_number].field_2C;
    is_set = 0;
    k = 0;
    for (i=0; i<NET_PLAYERS_COUNT; i++)
    {
      player=&(game.players[i%PLAYERS_COUNT]);
      if ( net_player_info[i].field_20 )
      {
        player->field_B = i;
        if ((!is_set) && (my_player_number == i))
        {
          is_set = 1;
          my_player_number = k;
        }
        k++;
      }
    }
    memset(game.packets, 0, sizeof(struct Packet)*PACKETS_COUNT);
    pckt = &game.packets[my_player_number];
    pckt->field_5 = 10;
    pckt->field_6 = flgmem;
    pckt->field_8 = settings.field_3;
    if ( LbNetwork_Exchange(pckt) )
      error(func_name, 156, "LbNetwork_Exchange failed");
    k = 0;
    for (i=0; i<NET_PLAYERS_COUNT; i++)
    {
      pckt = &game.packets[i];
      if ((net_player_info[i].field_20) && (pckt->field_5 == 10))
      {
          player = &(game.players[k]);
          player->field_2B = k;
          player->field_0 |= 0x01;
          player->field_4B5 = (-(pckt->field_8 < 1u) & 0xFD) + 5;
          player->field_2C = pckt->field_6;
          init_player(player, 0);
          strncpy(player->field_15,enum_players_callback[i].field_0,sizeof(struct TbNetworkCallbackData));
          k++;
      }
    }
  checksum_mem = 0;
  for (i=1; i<THINGS_COUNT; i++)
  {
      thing=game.things_lookup[i];
      if (thing->field_0 & 0x01)
      {
        checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
      }
  }
    player=&(game.players[my_player_number%PLAYERS_COUNT]);
    memset(game.packets, 0, sizeof(struct Packet)*PACKETS_COUNT);
    pckt = &game.packets[player->field_B];
    pckt->field_4 = checksum_mem + game.field_14BB4A;
    pckt->field_5 = 12;
    pckt->field_6 = 0;
    pckt->field_8 = 0;
    if (LbNetwork_Exchange(pckt))
      error(func_name, 210, "LbNetwork_Exchange failed");
    checksum_bad = 0;
    checksum_mem = -1;
    for (i=0; i<PLAYERS_COUNT; i++)
    {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) && ((player->field_0 & 0x40) == 0))
      {
          pckt = &game.packets[player->field_B];
          if (checksum_mem == -1)
          {
            checksum_mem = pckt->field_4;
          } else
          if (pckt->field_4 != checksum_mem)
          {
            checksum_bad = 1;
          }
      }
    }
    if ( checksum_bad )
      error(func_name, 219, "Checksums different");
    for (i=0; i<PLAYERS_COUNT; i++)
    {
      player=&(game.players[i]);
      if ((i != my_player_number) && (player->field_0 & 0x01))
      {
          if (frontend_is_player_allied(my_player_number, i))
          {
            toggle_ally_with_player(my_player_number, i);
            toggle_ally_with_player(i, my_player_number);
          }
      }
    }
  }
  if (fe_computer_players)
  {
    for (i=0; i<PLAYERS_COUNT; i++)
    {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) == 0)
      {
        thing = game.things_lookup[game.field_149E1A];
        while (thing > game.things_lookup[0])
        {
          if ((game.objects_config[thing->field_1A].field_6) && (thing->owner == i))
          {
            script_support_setup_player_as_computer_keeper(i, 0);
            break;
          }
          thing = game.things_lookup[thing->field_67];
        }
      }
    }
  }
  post_init_level();
  post_init_players();
  if ((game.field_149E81) && (game.numfield_149F47))
  {
      open_packet_file_for_load(game.packet_fname);
      game.gameturn = 0;
  }
  memset(game.packets, 0, sizeof(struct Packet)*PACKETS_COUNT);
}

void faststartup_network_game(void)
{
    reenter_video_mode();
    my_player_number = default_loc_player;
    game.flagfield_14EA4A = 2;
    game.players[my_player_number].field_2C = 1;
    game.numfield_14A83D = game.numfield_16;
    startup_network_game();
    game.players[my_player_number].field_6 &= 0xFDu;
}

short is_bonus_level(long levidx)
{
    if ((levidx>=100)&&(levidx<106))
        return true;
    return false;
}

short is_campaign_level(long levidx)
{
    if ((levidx>=1)&&(levidx<21))
        return true;
    return false;
}

short is_multiplayer_level(long levidx)
{
    if ((levidx>=50)&&(levidx<80))
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

  if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
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
  memset(scratch, 0, 0x300u);
  LbPaletteSet(scratch);
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
    old_mouse_over_button = frontend_mouse_over_button;
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
  reenter_video_mode();

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
        startup_network_game();
        break;
  case 8:
        game.numfield_14A83D = game.numfield_16;
        game.numfield_A |= 0x01;
        game.flagfield_14EA4A = 5;
        game.players[my_player_number].field_2C = 1;
        startup_network_game();
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
        game.flags_cd |= 0x01;
        startup_saved_packet_game();
        break;
  }
  game.players[my_player_number].field_6 &= 0xFDu;
}

void game_loop(void)
{
  //_DK_game_loop(); return;
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
        set_player_instance(player, 11, 0);
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
    LbMouseChangeSpriteAndHotspot(0, 0, 0);
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
    reset_eye_lenses();
    if ( game.packet_fopened )
    {
      _DK_LbFileClose(game.packet_save_fp);
      game.packet_fopened = 0;
      game.packet_save_fp = 0;
    }
  } // end while
  // Stop the movie recording if it's on
  if ( game.numfield_A & 0x08 )
    movie_record_stop();
}

short reset_game(void)
{
  _DK_IsRunningUnmark();
  _DK_LbMouseSuspend();
  LbIKeyboardClose();
  LbScreenReset();
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
  if (endpos==NULL)
      endpos=strrchr( keeper_runtime_directory, '/');
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

void close_video_context(void)
{
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
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
  static const char *func_name="LbBullfrogMain";
  //return _DK_LbBullfrogMain(argc, argv);
  short retval=0;
  LbErrorLogSetup("/", log_file_name, 5);
  strcpy(window_class_name, PROGRAM_NAME);
  LbTimerInit();
  LbSetIcon(110);
  srand(LbTimerClock());

  retval=process_command_line(argc,argv);
  if ( retval < 1 )
  {
      static const char *msg_text="Command line parameters analysis failed.\n";
      close_video_context();
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
  close_video_context();
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

  return 0;
}
