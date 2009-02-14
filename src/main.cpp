
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
#include "front_input.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "kjm_input.h"
#include "packets.h"
#include "config.h"

// Max length of the command line
#define CMDLN_MAXLEN 259
char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];
unsigned char palette_buf[768];

short is_new_moon = 0;

// Map size variables
int map_subtiles_x = 255;
int map_subtiles_y = 255;
int map_tiles_x = 85;
int map_tiles_y = 85;

const long VersionMajor = 1;
const long VersionMinor = 12;

char onscreen_msg_text[255]="";
int onscreen_msg_turns = 0;

char window_class_name[128]="Bullfrog Shell";
short default_loc_player = 0;
short hero_player = 4;

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

//DLLIMPORT extern struct TbLoadFiles _DK_legal_load_files[];
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

//static 
TbClockMSec last_loop_time=0;

#ifdef __cplusplus
extern "C" {
#endif
DLLIMPORT void _DK_load_level_file(long lvnum);
DLLIMPORT void _DK_delete_all_structures(void);
DLLIMPORT void _DK_clear_mapwho(void);
DLLIMPORT void _DK_light_initialise(void);
DLLIMPORT void _DK_clear_game(void);
DLLIMPORT void _DK_clear_game_for_save(void);
DLLIMPORT long _DK_update_cave_in(struct Thing *thing);
DLLIMPORT void _DK_update_thing_animation(struct Thing *thing);
DLLIMPORT long _DK_update_thing(struct Thing *thing);
DLLIMPORT void _DK_update_creatures_not_in_list(void);
DLLIMPORT long _DK_get_thing_checksum(struct Thing *thing);
DLLIMPORT long _DK_update_things_in_list(struct StructureList *list);
DLLIMPORT long _DK_update_thing_sound(struct Thing *thing);
DLLIMPORT void _DK_update_power_sight_explored(struct PlayerInfo *player);
DLLIMPORT void _DK_init_messages(void);
DLLIMPORT void _DK_battle_initialise(void);
DLLIMPORT void _DK_event_initialise_all(void);
DLLIMPORT void _DK_add_thing_to_list(struct Thing *thing, struct StructureList *list);
DLLIMPORT struct Thing *_DK_allocate_free_thing_structure(unsigned char a1);
DLLIMPORT unsigned char _DK_i_can_allocate_free_thing_structure(unsigned char a1);
DLLIMPORT void _DK_message_add(char c);
DLLIMPORT void _DK_toggle_creature_tendencies(struct PlayerInfo *player, char val);
DLLIMPORT long _DK_event_move_player_towards_event(struct PlayerInfo *player, long var);
DLLIMPORT void _DK_turn_off_call_to_arms(long a);
DLLIMPORT long _DK_place_thing_in_power_hand(struct Thing *thing, long var);
DLLIMPORT short _DK_magic_use_power_obey(unsigned short plridx);
DLLIMPORT void _DK_set_player_state(struct PlayerInfo *player, unsigned char a1, long a2);
DLLIMPORT void _DK_event_delete_event(long plridx, long num);
DLLIMPORT long _DK_set_autopilot_type(long plridx, long aptype);
DLLIMPORT void _DK_set_player_mode(struct PlayerInfo *player, long val);
DLLIMPORT short _DK_dump_held_things_on_map(unsigned char a1, long a2, long a3, short a4);
DLLIMPORT void _DK_turn_off_sight_of_evil(long plridx);
DLLIMPORT void _DK_go_on_then_activate_the_event_box(long plridx, long val);
DLLIMPORT void _DK_output_message(int, int, int);
DLLIMPORT void _DK_directly_cast_spell_on_thing(unsigned char plridx, unsigned char a2, unsigned short a3, long a4);
DLLIMPORT void _DK_lose_level(struct PlayerInfo *player);
DLLIMPORT long _DK_magic_use_power_armageddon(unsigned char val);
DLLIMPORT long _DK_battle_move_player_towards_battle(struct PlayerInfo *player, long var);
DLLIMPORT void _DK_level_lost_go_first_person(long plridx);
DLLIMPORT void __cdecl _DK_set_player_instance(struct PlayerInfo *playerinf, long, int);
DLLIMPORT TbError _DK_LbNetwork_Exchange(struct Packet *pckt);
DLLIMPORT void _DK_resync_game(void);
DLLIMPORT void __cdecl _DK_set_gamma(char, int);
DLLIMPORT void _DK_complete_level(struct PlayerInfo *player);
DLLIMPORT void _DK_free_swipe_graphic(void);
DLLIMPORT void _DK_draw_sound_stuff(void);
DLLIMPORT void _DK_draw_bonus_timer(void);
DLLIMPORT void _DK_draw_power_hand(void);
DLLIMPORT void _DK_update_explored_flags_for_power_sight(struct PlayerInfo *player);
DLLIMPORT void _DK_engine(struct Camera *cam);
DLLIMPORT void _DK_smooth_screen_area(unsigned char *a1, long a2, long a3, long a4, long a5, long a6);
DLLIMPORT void _DK_remove_explored_flags_for_power_sight(struct PlayerInfo *player);
DLLIMPORT void _DK_DrawBigSprite(long x, long y, struct BigSprite *bigspr, struct TbSprite *sprite);
DLLIMPORT void _DK_draw_gold_total(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT void _DK_pannel_map_draw(long x, long y, long zoom);
DLLIMPORT void _DK_draw_overlay_things(long zoom);
DLLIMPORT void _DK_draw_overlay_compass(long a1, long a2);
DLLIMPORT unsigned char _DK_find_first_battle_of_mine(unsigned char idx);
DLLIMPORT void _DK_set_engine_view(struct PlayerInfo *player, long a2);
DLLIMPORT void _DK_startup_network_game(void);
DLLIMPORT void _DK_reinit_level_after_load(void);
DLLIMPORT void _DK_reinit_tagged_blocks_for_player(unsigned char idx);
DLLIMPORT void _DK_reset_gui_based_on_player_mode(void);
DLLIMPORT long _DK_load_texture_map_file(long a1, unsigned char a2);
DLLIMPORT void _DK_init_animating_texture_maps(void);
DLLIMPORT void _DK_init_lookups(void);
DLLIMPORT long _DK_init_navigation(void);
DLLIMPORT long _DK_load_game(long);
DLLIMPORT void _DK_sound_reinit_after_load(void);
DLLIMPORT void _DK_restore_computer_player_after_load(void);
DLLIMPORT struct Thing *_DK_create_effect(struct Coord3d *pos, unsigned short a2, unsigned char a3);
DLLIMPORT void _DK_delete_thing_structure(struct Thing *thing, long a2);
DLLIMPORT void _DK_make_safe(struct PlayerInfo *player);
DLLIMPORT struct Thing *_DK_create_creature(struct Coord3d *pos, unsigned short a1, unsigned short a2);
DLLIMPORT void _DK_set_creature_level(struct Thing *thing, long nlvl);
DLLIMPORT void _DK_remove_events_thing_is_attached_to(struct Thing *thing);
DLLIMPORT unsigned long _DK_steal_hero(struct PlayerInfo *player, struct Coord3d *pos);
DLLIMPORT void _DK_creature_increase_level(struct Thing *thing);
DLLIMPORT void _DK_clear_slab_dig(long a1, long a2, char a3);
DLLIMPORT void _DK_magic_use_power_hold_audience(unsigned char idx);
DLLIMPORT void _DK_activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player);
DLLIMPORT void _DK_resurrect_creature(struct Thing *thing, unsigned char a2, unsigned char a3, unsigned char a4);
DLLIMPORT void _DK_transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char a3);
DLLIMPORT long _DK_thing_is_special(Thing *thing);
DLLIMPORT int __cdecl _DK_play_smacker_file(char *fname, int);
DLLIMPORT void __cdecl _DK_wait_for_cd_to_be_available(void);
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
DLLIMPORT void _DK_view_set_camera_y_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_view_set_camera_x_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_view_zoom_camera_in(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_set_camera_zoom(struct Camera *cam, long val);
DLLIMPORT void _DK_view_zoom_camera_out(struct Camera *cam, long a2, long a3);
DLLIMPORT long _DK_get_camera_zoom(struct Camera *camera);
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

void reset_eye_lenses(void)
{
  _DK_reset_eye_lenses();
}

void view_set_camera_y_inertia(struct Camera *cam, long a2, long a3)
{
  _DK_view_set_camera_y_inertia(cam, a2, a3);
}

void view_set_camera_x_inertia(struct Camera *cam, long a2, long a3)
{
  _DK_view_set_camera_x_inertia(cam, a2, a3);
}

void view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3)
{
  _DK_view_set_camera_rotation_inertia(cam, a2, a3);
}

void view_zoom_camera_in(struct Camera *cam, long a2, long a3)
{
  _DK_view_zoom_camera_in(cam, a2, a3);
}

void set_camera_zoom(struct Camera *cam, long val)
{
  _DK_set_camera_zoom(cam, val);
}

void view_zoom_camera_out(struct Camera *cam, long a2, long a3)
{
  _DK_view_zoom_camera_out(cam, a2, a3);
}

long get_camera_zoom(struct Camera *camera)
{
  return _DK_get_camera_zoom(camera);
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

unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl)
{
  // Note: I don't know if the zoom may be scaled for current resolution,
  // as there may be different resolution on another computer if playing MP game.
  return ((zoom_lvl*units_per_pixel) >> 4)*pixel_size;
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
  static const char *func_name="init_player_start";
  struct Dungeon *dungeon;
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[2].field_4;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->field_67;
    if ((game.objects_config[thing->field_1A].field_6) && (thing->owner == player->field_2B))
    {
      dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
      dungeon->field_0 = thing->field_1B;
      memcpy(&dungeon->mappos,&thing->mappos,sizeof(struct Coord3d));
      break;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void clear_slab_dig(long a1, long a2, char a3)
{
  _DK_clear_slab_dig(a1, a2, a3);
}

TbError LbNetwork_Init(unsigned long srvcp,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data)
{
  return _DK_LbNetwork_Init(srvcp,guid,maxplayrs,exchng_buf,exchng_size,locplayr,init_data);
}

TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long a2)
{
  return _DK_LbNetwork_ChangeExchangeBuffer(buf, a2);
}

void LbNetwork_ChangeExchangeTimeout(unsigned long tmout)
{
//  exchangeTimeout = 1000 * tmout;
}

short inline calculate_moon_phase(short do_calculate,short add_to_log)
{
  //Moon phase calculation
  if (do_calculate)
  {
    phase_of_moon=PhaseOfMoon::Calculate();
  }
  if ((phase_of_moon > -0.05) && (phase_of_moon < 0.05))
  {
    if (add_to_log)
      LbSyncLog("Full moon %.4f\n", phase_of_moon);
    game.is_full_moon = 1;
    is_new_moon = 0;
  } else
  if ((phase_of_moon < -0.95) || (phase_of_moon > 0.95))
  {
    if (add_to_log)
      LbSyncLog("New moon %.4f\n", phase_of_moon);
    game.is_full_moon = 0;
    is_new_moon = 1;
  } else
  {
    if (add_to_log)
      LbSyncLog("Moon phase %.4f\n", phase_of_moon);
    game.is_full_moon = 0;
    is_new_moon = 0;
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

short thing_is_special(struct Thing *thing)
{
  if ((thing->field_1F != 1) || (thing->field_1A >= OBJECT_TYPES_COUNT))
    return false;
  return (object_to_special[thing->field_1A] > 0);
}

void update_thing_animation(struct Thing *thing)
{
  _DK_update_thing_animation(thing);
}

long update_thing(struct Thing *thing)
{
  return _DK_update_thing(thing);
}

void update_creatures_not_in_list(void)
{
  _DK_update_creatures_not_in_list();
}

long get_thing_checksum(struct Thing *thing)
{
  return _DK_get_thing_checksum(thing);
}

unsigned long update_things_in_list(struct StructureList *list)
{
  static const char *func_name="update_things_in_list";
  struct Thing *thing;
  unsigned long k;
  unsigned long sum;
  int i;
  //return _DK_update_things_in_list(list);
  sum = 0;
  i = list->field_4;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->field_67;
    if ((thing->field_0 & 0x40) == 0)
    {
        if ((thing->field_0 & 0x10) != 0)
          update_thing_animation(thing);
        else
          update_thing(thing);
    }
    sum += get_thing_checksum(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return sum;
}

long update_thing_sound(struct Thing *thing)
{
  return _DK_update_thing_sound(thing);
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
  wait_for_cd_to_be_available();
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
      wait_for_cd_to_be_available();
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

  wait_for_cd_to_be_available();
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
      wait_for_cd_to_be_available();
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
    LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
  return result;
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
  calculate_moon_phase(true,true);

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
  if ( is_new_moon )
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
      result=wait_for_cd_to_be_available();
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
        set_gamma(settings.gamma_correction, 0);
        SetRedbookVolume(settings.redbook_volume);
        SetSoundMasterVolume(settings.sound_volume);
        SetMusicMasterVolume(settings.sound_volume);
        _DK_setup_3d();
        _DK_setup_stuff();
        init_lookups();
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
  left_button_clicked = lbDisplay.LeftButton;
  left_button_clicked_x = lbDisplay.MouseX * pixel_size;
  left_button_clicked_y = lbDisplay.MouseY * pixel_size;
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

void init_messages(void)
{
  _DK_init_messages();
}

void zero_messages(void)
{
  int i;
  game.active_messages_count = 0;
  for (i=0; i<3; i++)
  {
    memset(&game.messages[i], 0, sizeof(struct GuiMessage));
  }
}

void battle_initialise(void)
{
  _DK_battle_initialise();
}

void event_initialise_all(void)
{
  _DK_event_initialise_all();
}

void add_thing_to_list(struct Thing *thing, struct StructureList *list)
{
  _DK_add_thing_to_list(thing, list);
}

struct Thing *allocate_free_thing_structure(unsigned char a1)
{
  return _DK_allocate_free_thing_structure(a1);
}

unsigned char i_can_allocate_free_thing_structure(unsigned char a1)
{
  return _DK_i_can_allocate_free_thing_structure(a1);
}

struct Thing *create_ambient_sound(struct Coord3d *pos, unsigned short a2, unsigned short owner)
{
  static const char *func_name="create_ambient_sound";
  struct Thing *thing;
  if ( !i_can_allocate_free_thing_structure(1) )
  {
    error(func_name, 3698, "'Told You So' error! Cannot create ambient sound because there are too many fucking things allocated.");
    return NULL;
  }
  thing = allocate_free_thing_structure(1);
  thing->field_1F = 12;
  thing->field_1A = a2;
  thing->field_1D = thing->field_1B;
  memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
  thing->owner = owner;
  thing->field_4F |= 0x01;
  add_thing_to_list(thing, &game.thing_lists[9]);
  return thing;
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
  char *text;
  struct Thing *thing = NULL;
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  thing = game.things_lookup[player->field_35];
  if (thing == game.things_lookup[0])
      thing = NULL;
  if ((thing!=NULL)&&(!thing_is_special(thing)))
      thing = NULL;
  if (thing==NULL)
    thing = _DK_get_special_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    if ((void *)thing != _DK_tool_tip_box.target)
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
      text=buf_sprintf("%s",strings[545]);
    } else
    if (crtridx = _DK_objects[objidx].field_13)
    {
      int stridx=_DK_creature_data[crtridx].field_3;
      text=buf_sprintf("%s %s", strings[stridx], strings[609]);
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

void reveal_whole_map(struct PlayerInfo *player)
{
  unsigned short nflag;
  unsigned int x,y,i;

  nflag = (1 << player->field_2B);
  for (y=0; y<map_tiles_y; y++)
    for (x=0; x<map_tiles_x; x++)
    {
      clear_slab_dig(x, y, player->field_2B);
    }
  for (i=0; i<65536; i++)
  {
    x = (game.map[i].field_3 >> 12) | nflag;
    game.map[i].field_3 |= (x & 0x0F) << 12;
  }
  pannel_map_update(0, 0, 256, 256);
}

void increase_level(struct PlayerInfo *player)
{
  static const char *func_name="increase_level";
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct CreatureControl *cctrl;
  int i;
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);

  i = dungeon->field_2D;
  while (i > 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4576,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    creature_increase_level(thing);
    if (thing->field_64 >= CREATURES_COUNT)
      break;
    cctrl = game.creature_control_lookup[thing->field_64];
    if (cctrl < game.creature_control_data)
      break;
    i = cctrl->thing_idx;
  }

  i = dungeon->field_2F;
  while (i > 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4575,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    creature_increase_level(thing);
    if (thing->field_64 >= CREATURES_COUNT)
      break;
    cctrl = game.creature_control_lookup[thing->field_64];
    if (cctrl < game.creature_control_data)
      break;
    i = cctrl->thing_idx;
  }
}

void start_resurrect_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct Dungeon *dungeon;
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  if (dungeon->field_139F != 0)
  {
    if (player == &game.players[my_player_number])
    {
      dungeon_special_selected = thing->field_1B;
      resurrect_creature_scroll_offset = 0;
      turn_off_menu(27);
      turn_on_menu(28);
    }
  }
}

void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct Dungeon *dungeon;
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  if (dungeon->field_919 != 0)
  {
    if (player == &game.players[my_player_number])
    {
      dungeon_special_selected = thing->field_1B;
      transfer_creature_scroll_offset = 0;
      turn_off_menu(27);
      turn_on_menu(29);
    }
  }
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

void restore_computer_player_after_load(void)
{
  _DK_restore_computer_player_after_load();
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

short wait_for_cd_to_be_available(void)
{
    static const char *func_name="wait_for_cd_to_be_available";
//  _DK_wait_for_cd_to_be_available(); return;
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
    _DK_drag_menu_x = -999;
    _DK_drag_menu_y = -999;
    int idx;
    for (idx=0;idx<ACTIVE_BUTTONS_COUNT;idx++)
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

    if ( check_if_mouse_is_over_button(gbtn) && (!game_is_busy_doing_gui())
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
          if ( left_button_clicked )
          {
            left_button_clicked = 0;
            over_slider_button = gidx;
            do_sound_menu_click();
          }
        }
      }
    }
  }  // end for

  short result = 0;
  if (game_is_busy_doing_gui())
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
    if ( left_button_clicked )
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
        left_button_clicked = 0;
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

  for (gidx=0;gidx<ACTIVE_BUTTONS_COUNT;gidx++)
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

void setup_engine_window(long x1, long y1, long x2, long y2)
{
  static const char *func_name="setup_engine_window";
  long cx1,cy1,cx2,cy2;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Setting coords %d,%d,%d,%d\n",func_name,x1,y1,x2,y2);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
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

long creature_instance_is_available(struct Thing *thing, long inum)
{
  struct CreatureControl *crctrl;
  crctrl = game.creature_control_lookup[thing->field_64];
  return crctrl->instances[inum];
}

struct Thing *create_creature(struct Coord3d *pos, unsigned short a1, unsigned short a2)
{
  return _DK_create_creature(pos, a1, a2);
}

void creature_increase_level(struct Thing *thing)
{
  _DK_creature_increase_level(thing);
}

void set_creature_level(struct Thing *thing, long nlvl)
{
  _DK_set_creature_level(thing, nlvl);
}

void remove_events_thing_is_attached_to(struct Thing *thing)
{
  _DK_remove_events_thing_is_attached_to(thing);
}

unsigned long steal_hero(struct PlayerInfo *player, struct Coord3d *pos)
{
  return _DK_steal_hero(player, pos);
}

void make_safe(struct PlayerInfo *player)
{
  _DK_make_safe(player);
}

void reset_gui_based_on_player_mode(void)
{
  _DK_reset_gui_based_on_player_mode();
}

void sound_reinit_after_load(void)
{
  _DK_sound_reinit_after_load();
}

void reinit_tagged_blocks_for_player(unsigned char idx)
{
  _DK_reinit_tagged_blocks_for_player(idx);
}

short activate_bonus_level(struct PlayerInfo *player)
{
  static const char *func_name="activate_bonus_level";
  int i;
  short result;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  game.flags_font |= 0x02;
  i = array_index_for_levels_bonus(game.numfield_14A83D);
  if ((i>=0) && (i<5))
  {
    LbSyncLog("Used index %d to store bonus award for level %d\n",i,game.numfield_14A83D);
    game.bonus_levels[i] = 1;
    result = true;
  } else
  {
    LbSyncLog("Couldn't find index to store bonus award for level %d\n",game.numfield_14A83D);
    error(func_name, 517, "No Bonus level assigned to this level");
    result = false;
  }
/*  switch (game.numfield_14A83D)
  {
  case 8:
      game.bonus_levels[0] = 1;
      break;
  case 9:
      game.bonus_levels[1] = 1;
      break;
  case 15:
      game.bonus_levels[2] = 1;
      break;
  case 17:
      game.bonus_levels[3] = 1;
      break;
  case 18:
      game.bonus_levels[4] = 1;
      break;
  default:
      error(func_name, 517, "No Bonus level assigned to this level");
      break;
  }*/
  game.numfield_C &= 0xFFFDu;
  return result;
}

void multiply_creatures(struct PlayerInfo *player)
{
  static const char *func_name="multiply_creatures";
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Thing *tncopy;
  struct CreatureControl *cctrl;
  int i;

  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  i = dungeon->field_2D;
  while (i > 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4573,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    tncopy = create_creature(&thing->mappos, thing->field_1A, player->field_2B);
    if (tncopy == NULL)
      break;
    set_creature_level(tncopy, thing->field_23);
    tncopy->field_5E = thing->field_5E;
    if (thing->field_64 >= CREATURES_COUNT)
      break;
    cctrl = game.creature_control_lookup[thing->field_64];
    if (cctrl < game.creature_control_data)
      break;
    i = cctrl->thing_idx;
  }

  i = dungeon->field_2F;
  while (i >= 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4572,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    tncopy = create_creature(&thing->mappos, thing->field_1A, player->field_2B);
    if (tncopy == NULL)
      break;
    set_creature_level(tncopy, thing->field_23);
    tncopy->field_5E = thing->field_5E;
    if (thing->field_64 >= CREATURES_COUNT)
      break;
    cctrl = game.creature_control_lookup[thing->field_64];
    if (cctrl < game.creature_control_data)
      break;
    i = cctrl->thing_idx;
  }
}

short toggle_computer_player(int idx)
{
  if (game.dungeon[idx].computer_enabled & 0x01)
    game.dungeon[idx].computer_enabled &= 0xFE;
  else
    game.dungeon[idx].computer_enabled |= 0x01;
  return 1;
}

void set_player_instance(struct PlayerInfo *player, long a2, int a3)
{
  _DK_set_player_instance(player, a2, a3);
}

short save_version_compatible(long filesize,struct Game *header)
{
  // Native version
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor == VersionMinor))
    return true;
  // Compatibility list
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor <= 8) &&
    (VersionMinor == 10))
    return true;
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor <= 10) &&
    (VersionMinor == 12))
    return true;
  return false;
}

short load_texture_map_file(long tmapidx, unsigned char a2)
{
  static const char *func_name="load_texture_map_file";
  //return _DK_load_texture_map_file(tmapidx, a2);
  char *fname;
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  fname = prepare_file_fmtpath(FGrp_StdData,"tmapa%03d.dat",tmapidx);
  wait_for_cd_to_be_available();
  if (!LbFileExists(fname))
  {
    LbWarnLog("Texture file \"%s\" doesn't exits.\n",fname);
    return 0;
  }
  // The texture file has always over 500kb
  if (LbFileLoadAt(fname, block_mem) < 65536)
  {
    LbWarnLog("Texture file \"%s\" can't be loaded or is too small.\n",fname);
    return 0;
  }
  return 1;
}

void reinit_level_after_load(void)
{
  static const char *func_name="reinit_level_after_load";
  //_DK_reinit_level_after_load(); return;
  struct PlayerInfo *player;
  int i;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_7 = 0;
  init_lookups();
  init_navigation();
  parchment_loaded = 0;
  for (i=0; i<5; i++)
  {
    player = &(game.players[i%PLAYERS_COUNT]);
    if (player->field_0 & 0x01)
      set_engine_view(player, player->field_37);
  }
  start_rooms = &game.rooms[1];
  end_rooms = &game.rooms[ROOMS_COUNT];
  load_texture_map_file(game.field_14AB41, 2);
  init_animating_texture_maps();
  init_gui();
  reset_gui_based_on_player_mode();
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  reinit_tagged_blocks_for_player(player->field_2B);
  restore_computer_player_after_load();
  sound_reinit_after_load();
}

short load_game(long num)
{
  static const char *func_name="load_game";
  //return _DK_load_game(num);
  char *fname;
  TbFileHandle handle;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  unsigned char buffer[10];
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  reset_eye_lenses();
  fname = prepare_file_fmtpath(FGrp_Save,"game%04d.sav",num);
  wait_for_cd_to_be_available();
  handle = LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
  if (handle == -1)
  {
    LbWarnLog("Cannot open saved game file \"%s\".\n",fname);
    game.save_catalogue[num].used = 0;
    fname = prepare_file_path(FGrp_Save,"save.cat");
    LbFileSaveAt(fname, &game.save_catalogue, sizeof(struct CatalogueEntry)*SAVE_SLOTS_COUNT);
    return 0;
  }
  if (LbFileRead(handle, &buffer, 10) != 10)
  {
    LbFileClose(handle);
    game.save_catalogue[num].used = 0;
    fname = prepare_file_path(FGrp_Save,"save.cat");
    LbFileSaveAt(fname, &game.save_catalogue, sizeof(struct CatalogueEntry)*SAVE_SLOTS_COUNT);
    return 0;
  }
  LbFileClose(handle);
  if (!save_version_compatible(LbFileLength(fname),(struct Game *)buffer))
  {
    LbWarnLog("Saved game file \"%s\" has incompatible version; restarting level.\n",fname);
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    player->field_7 = 0;
    my_player_number = default_loc_player;
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    game.flagfield_14EA4A = 2;
    game.numfield_A &= 0xFEu;
    player->field_2C = 1;
    game.numfield_14A83D = buffer[0];
    game.level_number = buffer[0];
    startup_network_game();
    return 1;
  }
  if (LbFileLoadAt(fname, &game) != sizeof(struct Game))
  {
    LbWarnLog("Couldn't correctly load saved game \"%s\".\n",fname);
    return 0;
  }
  init_lookups();
  reinit_level_after_load();
  output_message(102, 0, 1);
  pannel_map_update(0, 0, 256, 256);
  calculate_moon_phase(false,false);
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_3 &= 0xF7;
  player->field_3 &= 0xFB;
  player->field_4C1 = 0;
  player->field_4C5 = 0;
  player->field_7 = 0;
  PaletteSetPlayerPalette(player, _DK_palette);
  reinitialise_eye_lens(game.numfield_1B);
  if (player->field_29 != 0)
  {
    frontstats_initialise();
    dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
    dungeon->player_score = 0;
    dungeon->allow_save_score = 1;
  }
  game.field_1516FF = 0;
  return 1;
}

int LbNetwork_Exchange(struct Packet *pckt)
{
  return _DK_LbNetwork_Exchange(pckt);
}

void light_initialise(void)
{
  _DK_light_initialise();
}

void delete_all_structures(void)
{
  _DK_delete_all_structures();
}

void clear_mapwho(void)
{
  _DK_clear_mapwho();
}

void clear_columns(void)
{
  int i;
  for (i=0; i < COLUMNS_COUNT; i++)
  {
    memset(&game.columns[i], 0, sizeof(struct Column));
  }
}

void clear_players(void)
{
  int i;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    memset(&game.players[i], 0, sizeof(struct PlayerInfo));
  }
}

void clear_players_for_save(void)
{
  struct PlayerInfo *player;
  unsigned short mem1,mem2,memflg;
  int i;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player=&(game.players[i%PLAYERS_COUNT]);
    mem1 = player->field_2B;
    mem2 = player->field_2C;
    memflg = player->field_0;
    memset(player, 0, sizeof(struct PlayerInfo));
    player->field_2B = mem1;
    player->field_2C = mem2;
    set_flag_byte(&player->field_0,0x01,((memflg & 0x01) != 0));
    set_flag_byte(&player->field_0,0x40,((memflg & 0x40) != 0));
  }
}

void clear_dungeons(void)
{
  int i;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    memset(&game.dungeon[i], 0, sizeof(struct Dungeon));
  }
}

void clear_game(void)
{
  //_DK_clear_game(); return;
  delete_all_structures();
  light_initialise();
  clear_mapwho();
  game.field_14E938 = 0;
  game.field_14BB4A = 0;
  game.numfield_C &= 0xFFFB;
  clear_columns();
  clear_players();
  clear_dungeons();
}

void clear_game_for_save(void)
{
  //_DK_clear_game_for_save(); return;
  delete_all_structures();
  light_initialise();
  clear_mapwho();
  game.field_14E938 = 0;
  game.field_14BB4A = 0;
  game.numfield_C &= 0xFFFB;
  clear_columns();
  clear_players_for_save();
  clear_dungeons();
}

void init_good_player_as(long plr_idx)
{
  struct PlayerInfo *player;
  game.field_14E496 = plr_idx;
  player = &(game.players[plr_idx%PLAYERS_COUNT]);
  player->field_0 |= 0x01;
  player->field_0 |= 0x40;
  player->field_2B = game.field_14E496;
}

void resync_game(void)
{
  return _DK_resync_game();
}

void free_swipe_graphic(void)
{
  _DK_free_swipe_graphic();
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
    LbWarnLog("Palette file \"%s\" doesn't exist.\n", fname);
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

void reset_scroll_window(void)
{
  game.field_1512D9 = 0;
  game.field_1512DD = 0;
  game.field_1512DE = 0;
  game.field_1512E2 = 0;
}

unsigned char find_first_battle_of_mine(unsigned char idx)
{
  return _DK_find_first_battle_of_mine(idx);
}

void level_lost_go_first_person(long plridx)
{
  static const char *func_name="level_lost_go_first_person";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_level_lost_go_first_person(plridx);
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void go_on_then_activate_the_event_box(long plridx, long evidx)
{
  static const char *func_name="go_on_then_activate_the_event_box";
  //_DK_go_on_then_activate_the_event_box(plridx, val);
  struct Dungeon *dungeon;
  struct Event *event;
  struct Thing *thing;
  char *text;
  int i;
  short other_off;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  dungeon = &(game.dungeon[plridx%DUNGEONS_COUNT]);
  dungeon->field_1173 = evidx;
  event = &game.event[evidx];
  if (plridx == my_player_number)
  {
    i = event_button_info[event->field_B].field_6;
    if (i != 201)
      strcpy(game.text_info, strings[i%STRINGS_MAX]);
  }
  if (event->field_B == 2)
    dungeon->field_1174 = find_first_battle_of_mine(plridx);
  if (plridx == my_player_number)
  {
    other_off = 0;
    switch ( event->field_B )
    {
    case 1:
    case 4:
        other_off = 1;
        turn_on_menu(16);
        break;
    case 2:
        turn_off_menu(16);
        turn_on_menu(34);
        break;
    case 3:
        strcpy(game.text_info, game.field_1506D9);
        for (i=13; i>=0; i--)
        {
          // Not sure about the bound being 121
          if (game.event[dungeon->field_13A7[i] % 121].field_B == 3)
          {
            other_off = 1;
            turn_on_menu(16);
            new_objective = 0;
            break;
          }
        }
        break;
    case 5:
        other_off = 1;
        i = room_data[event->field_C].field_13;
        text=buf_sprintf("%s:\n %s",game.text_info,strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 6:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = creature_data[thing->field_1A % CREATURE_TYPES_COUNT].field_3;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 7:
        other_off = 1;
        i = spell_data[event->field_C % (SPELL_TYPES_COUNT+1)].field_D;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 8:
        other_off = 1;
        i = trap_data[event->field_C % TRAP_TYPES_COUNT].field_C;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 9:
        other_off = 1;
        i = door_names[event->field_C % DOOR_TYPES_COUNT];
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 10:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = creature_data[thing->field_1A % CREATURE_TYPES_COUNT].field_3;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 11:
    case 13:
        other_off = 1;
        turn_on_menu(16);
        break;
    case 12:
        other_off = 1;
        text = buf_sprintf("%s:\n %d", game.text_info, event->field_C);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 14:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = spell_data[object_to_magic[thing->field_1A % OBJECT_TYPES_COUNT]].field_D;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 15:
        other_off = 1;
        i = room_data[event->field_C].field_13;
        text = buf_sprintf("%s:\n %s",game.text_info,strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 16:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = creature_data[thing->field_1A % CREATURE_TYPES_COUNT].field_3;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 17:
    case 18:
    case 19:
    case 20:
    case 22:
    case 23:
        other_off = 1;
        turn_on_menu(16);
        break;
    case 21:
        i = (long)event->field_C;
        if (i < 0)
        {
          i = -i;
          event->field_C = i;
        }
        strncpy(game.field_150AD9, strings[i%STRINGS_MAX], sizeof(game.field_150AD9)-1);
        strncpy(game.text_info, game.field_150AD9, sizeof(game.text_info)-1);
        other_off = 1;
        turn_on_menu(16);
        break;
    case 24:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = trap_data[object_to_door_or_trap[thing->field_1A % OBJECT_TYPES_COUNT]].field_C;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
      case 25:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = door_names[object_to_door_or_trap[thing->field_1A % OBJECT_TYPES_COUNT]];
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
      case 26:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = specials_text[object_to_special[thing->field_1A % OBJECT_TYPES_COUNT]];
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    default:
        text = buf_sprintf("Undefined event type: %d", (int)event->field_B);
        error(func_name, 9100, text);
        break;
    }
    reset_scroll_window();
    if (other_off)
    {
      turn_off_menu(34);
      turn_off_menu(27);
      turn_off_menu(28);
      turn_off_menu(29);
    }
  }
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
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
  static const char *func_name="complete_level";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_complete_level(player);
}

long init_navigation(void)
{
  return _DK_init_navigation();
}

void init_lookups(void)
{
  _DK_init_lookups();
}

void  toggle_ally_with_player(long plyridx, unsigned int allyidx)
{
  struct PlayerInfo *player=&(game.players[plyridx%PLAYERS_COUNT]);
  player->field_2A ^= (1 << allyidx);
}

void set_mouse_light(struct PlayerInfo *player)
{
  static const char *func_name="set_mouse_light";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_set_mouse_light(player);
}

/*
 * Scales camera zoom level on resolution change. If prev_units_per_pixel_size
 * is zero, then the zoom level will be only clipped, without any scaling.
 */
void keep_camera_zoom_level(struct Camera *cam,unsigned long prev_units_per_pixel_size)
{
  long zoom_val;
  unsigned long zoom_min,zoom_max;
  zoom_min = scale_camera_zoom_to_screen(CAMERA_ZOOM_MIN);
  zoom_max = scale_camera_zoom_to_screen(CAMERA_ZOOM_MAX);
  zoom_val = get_camera_zoom(cam);
  // Note: I don't know if the zoom may be scaled for current resolution,
  // as there may be different resolution on another computer if playing MP game.
  if (prev_units_per_pixel_size > 0)
    zoom_val = zoom_val*units_per_pixel*pixel_size/prev_units_per_pixel_size;
  if (zoom_val < zoom_min)
  {
    zoom_val = zoom_min;
  } else
  if (zoom_val > zoom_max)
  {
    zoom_val = zoom_max;
  }
  set_camera_zoom(cam, zoom_val);
}

/*
 * Scales local player camera zoom level on resolution change. If prev_units_per_pixel_size
 * is zero, then the zoom level will be only clipped, without any scaling.
 */
void keep_local_camera_zoom_level(unsigned long prev_units_per_pixel_size)
{
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  keep_camera_zoom_level(player->camera,prev_units_per_pixel_size);
}

/*
 * Conducts clipping to zoom level of given camera, based on current screen mode.
 */
void update_camera_zoom_bounds(struct Camera *cam,unsigned long zoom_max,unsigned long zoom_min)
{
  static const char *func_name="update_camera_zoom_bounds";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  long zoom_val;
  zoom_val = get_camera_zoom(cam);
  if (zoom_val < zoom_min)
  {
    zoom_val = zoom_min;
  } else
  if (zoom_val > zoom_max)
  {
    zoom_val = zoom_max;
  }
  set_camera_zoom(cam, zoom_val);
}

void magic_use_power_hold_audience(unsigned char idx)
{
  _DK_magic_use_power_hold_audience(idx);
}

void delete_thing_structure(struct Thing *thing, long a2)
{
  _DK_delete_thing_structure(thing, a2);
}

struct Thing *create_effect(struct Coord3d *pos, unsigned short a2, unsigned char a3)
{
  return _DK_create_effect(pos, a2, a3);
}

void create_special_used_effect(struct Coord3d *pos, long a2)
{
  create_effect(pos, 67, a2);
}

void activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player)
{
  static const char *func_name="activate_dungeon_special";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_activate_dungeon_special(thing, player); return;
  short used;
  struct Coord3d pos;
  int spkindidx;

  // Gathering data which we'll need if the special is used and disposed.
  memcpy(&pos,&thing->mappos,sizeof(struct Coord3d));
  spkindidx = thing->field_1A - 86;
  used = 0;
  if ((thing->field_0 & 0x01) && is_dungeon_special(thing))
  {
    switch (thing->field_1A)
    {
        case 86:
          reveal_whole_map(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 87:
          start_resurrect_creature(player, thing);
          break;
        case 88:
          start_transfer_creature(player, thing);
          break;
        case 89:
          if (steal_hero(player, &thing->mappos))
          {
            remove_events_thing_is_attached_to(thing);
            used = 1;
            delete_thing_structure(thing, 0);
          }
          break;
        case 90:
          multiply_creatures(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 91:
          increase_level(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 92:
          make_safe(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 93:
          activate_bonus_level(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        default:
          error(func_name, 360, buf_sprintf("Invalid dungeon special (Model %d)", thing->field_1A));
          break;
      }
      if ( used )
      {
        if (player == &game.players[my_player_number])
          output_message(special_desc[spkindidx].field_8, 0, 1);
        create_special_used_effect(&pos, player->field_2B);
      }
  }
}

void resurrect_creature(struct Thing *thing, unsigned char a2, unsigned char a3, unsigned char a4)
{
  _DK_resurrect_creature(thing, a2, a3, a4);
}

void transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char a3)
{
  _DK_transfer_creature(tng1, tng2, a3);
}

void process_rooms(void)
{
  static const char *func_name="process_rooms";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_rooms();
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void process_dungeons(void)
{
  static const char *func_name="process_dungeons";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_dungeons();
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void process_messages(void)
{
  static const char *func_name="process_messages";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_messages();
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
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
  _DK_process_level_script(); return;
  
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
  static const char *func_name="process_player_instances";
  //_DK_process_player_instances();return;
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
/*  if (lpDDC != NULL)
    return lpDDC->numfield_1C;
  return -1;*/
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
  static const char *func_name="update_research";
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
  static const char *func_name="update_manufacturing";
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
  static const char *func_name="update_all_players_cameras";
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
  static const char *func_name="update_light_render_area";
  int subtile_x,subtile_y;
  int delta_x,delta_y;
  int startx,endx,starty,endy;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
  subtile_y = player->camera->mappos.y.stl.num;
  subtile_x = player->camera->mappos.x.stl.num;
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
  static const char *func_name="process_player_states";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_player_states();
}

void update_power_sight_explored(struct PlayerInfo *player)
{
  _DK_update_power_sight_explored(player);
}

void update_player_objectives(int plridx)
{
  static const char *func_name="update_player_objectives";
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting for player %d\n",func_name,plridx);
#endif
  player=&(game.players[plridx]);
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
        if (plridx == my_player_number)
          _DK_set_level_objective(strings[0]);
        display_objectives(player->field_2B, 0, 0);
        break;
    case 2:
        if (plridx == my_player_number)
          _DK_set_level_objective(strings[335]);
        display_objectives(player->field_2B, 0, 0);
        break;
    }
  }
}

void process_players(void)
{
  static const char *func_name="process_players";
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  process_player_instances();
  process_player_states();
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) && (player->field_2C == 1))
      {
  #if (BFDEBUG_LEVEL > 6)
      LbSyncLog("%s: Doing updates for player %d\n",func_name,i);
  #endif
          wander_point_update(&player->wandr1);
          wander_point_update(&player->wandr2);
          update_power_sight_explored(player);
          update_player_objectives(i);
      }
  }
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Finished\n",func_name);
#endif
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

/*
 * Updates sounds of things from given StructureList.
 * Returns amount of items in the list.
 */
unsigned long update_things_sounds_in_list(struct StructureList *list)
{
  static const char *func_name="update_things_sounds_in_list";
  struct Thing *thing;
  unsigned long k;
  int i;
  i = list->field_4;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->field_67;
    update_thing_sound(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return k;
}

long update_cave_in(struct Thing *thing)
{
  return _DK_update_cave_in(thing);
}

/*
 * Updates cave in things, using proper StructureList.
 * Returns amount of items in the list.
 */
unsigned long update_cave_in_things(void)
{
  static const char *func_name="update_cave_in_things";
  struct Thing *thing;
  unsigned long k;
  int i;
  i = game.thing_lists[10].field_4;
  k = 0;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4576,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->field_67;
    update_cave_in(thing);
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4577,"Infinite loop detected when sweeping things list");
      break;
    }
  }
  return k;
}

unsigned long compute_players_checksum(void)
{
  struct PlayerInfo *player;
  struct Coord3d *mappos;
  int i;
  unsigned long sum;
  sum = 0;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i%PLAYERS_COUNT]);
    if (((player->field_0 & 0x01) != 0) && ((player->field_0 & 0x40) == 0))
    {
        mappos = &(player->camera->mappos);
        sum += player->field_4B1 + player->field_4B0
                   + mappos->x.val + mappos->z.val + mappos->y.val;
    }
  }
  return sum;
}


void update_things(void)
{
  static const char *func_name="update_things";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_update_things();
  unsigned long sum;
  struct PlayerInfo *player;
  optimised_lights = 0;
  total_lights = 0;
  do_lights = game.field_4614D;
  sum = 0;
  sum += update_things_in_list(&game.thing_lists[0]);
  update_creatures_not_in_list();
  sum += update_things_in_list(&game.thing_lists[7]);
  sum += update_things_in_list(&game.thing_lists[1]);
  sum += update_things_in_list(&game.thing_lists[2]);
  sum += update_things_in_list(&game.thing_lists[5]);
  sum += update_things_in_list(&game.thing_lists[3]);
  sum += update_things_in_list(&game.thing_lists[4]);
  sum += update_things_in_list(&game.thing_lists[6]);
  sum += update_things_in_list(&game.thing_lists[8]);
  update_things_sounds_in_list(&game.thing_lists[9]);
  update_cave_in_things();
  sum += compute_players_checksum();
  sum += game.field_14BB4A;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  game.packets[player->field_B%PACKETS_COUNT].field_4 = sum;
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
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
      S3DSetSoundReceiverPosition(camera->mappos.x.val,camera->mappos.y.val,camera->mappos.z.val);
      S3DSetSoundReceiverOrientation(camera->orient_a,camera->orient_b,camera->orient_c);
    }
    game.seedchk_random_used++;
  }
  find_nearest_rooms_for_ambient_sound();
  process_3d_sounds();
  k = (game.field_1517E2-game.seedchk_random_used) / 2;
  if (is_bonus_level(game.numfield_14A83D) )
  {
    if ((game.field_1517E2 == game.seedchk_random_used) ||
        (game.field_1517E2 > game.seedchk_random_used) && ((k<=100) && ((k % 10) == 0) ||
        (k<=300) && ((k % 50)==0) || ((k % 250)==0)) )
      play_non_3d_sample(89);
  }
}

void update(void)
{
  static const char *func_name="update";
  //_DK_update();return;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  int i,k;
#if (BFDEBUG_LEVEL > 4)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
      dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
      memset((char *)dungeon->field_64, 0, 480 * sizeof(short));
      memset((char *)dungeon->field_424, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
      memset((char *)dungeon->field_4E4, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
    }

    game.creature_pool_empty = 1;
    for (i=1; i<CREATURE_TYPES_COUNT; i++)
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
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void draw_tooltip(void)
{
  static const char *func_name="draw_tooltip";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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

/*
 * Strange name to hide easter eggs ;). Displays easter egg messages on screen.
 */
void draw_sound_stuff(void)
{
  static const char *func_name="draw_sound_stuff";
  char *text;
  int i;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  if (eastegg03_cntr > 6)
  {
      unsigned char pos;
      eastegg03_cntr++;
      lbFontPtr = winfont;
      text=buf_sprintf("Dene says a big 'Hello' to Goth Buns, Tarts and Barbies");
      lbDisplay.DrawFlags = 0x40;
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
  _DK_draw_sound_stuff();
}

void process_pointer_graphic(void)
{
  _DK_process_pointer_graphic();
}

void draw_bonus_timer(void)
{
  _DK_draw_bonus_timer(); return;
}

void message_draw(void)
{
  static const char *func_name="message_draw";
  int i,h;
  long x,y;
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  lbFontPtr = winfont;
  if (lbFontPtr != NULL)
    h = lbFontPtr[1].SHeight;
  else
    h = 0;
  x = 148;
  y = 28;
  for (i=0; i < game.active_messages_count; i++)
  {
      LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
      lbDisplay.DrawFlags &= 0xFFBFu;
      LbTextDraw((x+32)/pixel_size, y/pixel_size, game.messages[i].text);
      LbSpriteDraw(x/pixel_size, y/pixel_size, &gui_panel_sprites[488+game.messages[i].field_40]);
      y += pixel_size * h;
  }
}

void draw_power_hand(void)
{
  static const char *func_name="draw_power_hand";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_draw_power_hand();
}

void redraw_creature_view(void)
{
  static const char *func_name="redraw_creature_view";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_redraw_creature_view();
}

void smooth_screen_area(unsigned char *scrbuf, long x, long y, long w, long h, long scanln)
{
  static const char *func_name="smooth_screen_area";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  long sx,sy,sw,sh,i,k;
  unsigned char *buf;
  unsigned char *lnbuf;
  unsigned int ghpos;
  sx = x/pixel_size;
  sy = y/pixel_size;
  sw = w/pixel_size;
  sh = h/pixel_size;
  lnbuf = scrbuf + scanln*sy + sx;
  for (i = sh-sy-1; i>0; i--)
  {
    buf = lnbuf;
    for (k = sw-sx-1; k>0; k--)
    {
        ghpos = (buf[0] << 8) + buf[1];
        ghpos = (buf[scanln] << 8) + ghost[ghpos];
        buf[0] = ghpos;
        buf++;
    }
    lnbuf += scanln;
  }
}

void make_camera_deviations(struct PlayerInfo *player,struct Dungeon *dungeon)
{
  static const char *func_name="make_camera_deviations";
  long x,y;
  x = player->camera->mappos.x.val;
  y = player->camera->mappos.y.val;
  if (dungeon->field_EA0)
  {
    x += seed_check_random(80, &game.field_14BB4E, func_name, 8653) - 40;
    y += seed_check_random(80, &game.field_14BB4E, func_name, 8654) - 40;
  }
  if (dungeon->field_EA4)
  {
    x += ( (dungeon->field_EA4 * lbSinTable[player->camera->orient_a] >> 8) >> 8);
    y += (-(dungeon->field_EA4 * lbCosTable[player->camera->orient_a] >> 8) >> 8);
  }
  if ((dungeon->field_EA0) || (dungeon->field_EA4))
  {
    // bounding position
    if (x < 0)
    {
      x = 0;
    } else
    if (x > 65535)
    {
      x = 65535;
    }
    if (y < 0)
    {
      y = 0;
    } else
    if (y > 65535)
    {
      y = 65535;
    }
    // setting deviated position
    player->camera->mappos.x.val = x;
    player->camera->mappos.y.val = y;
  }
}

void redraw_isometric_view(void)
{
  static const char *func_name="redraw_isometric_view";
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Coord3d pos;
  int i;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_redraw_isometric_view(); return;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  memcpy(&pos,&player->camera->mappos,sizeof(struct Coord3d));
  if (player->field_45F != 1)
    player->field_45F = 1;
  dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
  // Camera position modifications
  make_camera_deviations(player,dungeon);
  update_explored_flags_for_power_sight(player);
  if (game.flags_font & 0x08)
  {
    setup_engine_window(player->field_448, player->field_44A,
        player->field_444 >> 1, player->field_446 >> 1);
  }
  engine((struct Camera *)&player->cam_mappos);
  if (game.flags_font & 0x08)
  {
    setup_engine_window(player->field_448, player->field_44A,
      player->field_444, player->field_446);
  }
  if (smooth_on)
    smooth_screen_area(lbDisplay.WScreen, 140, 0, MyScreenWidth, MyScreenHeight, lbDisplay.GraphicsScreenWidth);
  remove_explored_flags_for_power_sight(player);
  if (game.numfield_C & 0x20)
  {
    lbDisplay.DrawColour = colours[15][15][15];
    lbDisplay.DrawFlags = 0;
    DrawBigSprite(0, 0, &status_panel, gui_panel_sprites);
    draw_gold_total(player->field_2B, 60, 134, dungeon->field_AF9);
    if (pixel_size < 3)
      i = (player->minimap_zoom) / (3-pixel_size);
    else
      i = player->minimap_zoom;
    pannel_map_draw(player->mouse_x, player->mouse_y, i);
    draw_overlay_things(i);
  }
  draw_gui();
  if ( game.numfield_C & 0x20)
    draw_overlay_compass(player->mouse_x, player->mouse_y);
  message_draw();
  draw_power_hand();
  draw_tooltip();
  memcpy(&player->camera->mappos,&pos,sizeof(struct Coord3d));
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void redraw_frontview(void)
{
  static const char *func_name="redraw_frontview";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_redraw_frontview();
}

void draw_zoom_box(void)
{
  _DK_draw_zoom_box();
}

void update_explored_flags_for_power_sight(struct PlayerInfo *player)
{
  _DK_update_explored_flags_for_power_sight(player);
}

void engine(struct Camera *cam)
{
  _DK_engine(cam);
}

void remove_explored_flags_for_power_sight(struct PlayerInfo *player)
{
  _DK_remove_explored_flags_for_power_sight(player);
}

void DrawBigSprite(long x, long y, struct BigSprite *bigspr, struct TbSprite *sprite)
{
  _DK_DrawBigSprite(x, y, bigspr, sprite);
}

void draw_gold_total(unsigned char a1, long a2, long a3, long a4)
{
  _DK_draw_gold_total(a1, a2, a3, a4);
}

void pannel_map_draw(long x, long y, long zoom)
{
  _DK_pannel_map_draw(x, y, zoom);
}

void draw_overlay_things(long zoom)
{
  static const char *func_name="draw_overlay_things";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_draw_overlay_things(zoom);
}

void draw_overlay_compass(long a1, long a2)
{
  _DK_draw_overlay_compass(a1, a2);
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
  static const char *func_name="gui_draw_box";
#if (BFDEBUG_LEVEL > 6)
  LbSyncLog("%s: Drawing box, first optn \"%s\"\n",func_name,gbox->optn_list->label);
#endif
  struct GuiBox *gbox_over;
  struct GuiBoxOption *goptn_over;
  struct GuiBoxOption *goptn;
  long lnheight;
  long pos_x,pos_y;
  LbTextSetWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);

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
  static const char *func_name="gui_draw_all_boxes";
  struct GuiBox *gbox;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
  int x,y,w,h;
  // Retrieving name
  lv_name = NULL;
  if (is_original_singleplayer_level(game.numfield_14A83D))
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
  // Retrieving position
  x = 0;
  y = 0;
  w = 640;//MyScreenWidth;
  h = MyScreenHeight;
  // Drawing
  if (lv_name != NULL)
  {
    lbFontPtr = winfont;
    lbDisplay.DrawFlags = 0;
    LbTextSetWindow(x/pixel_size, y/pixel_size, w/pixel_size, h/pixel_size);
    LbTextDraw((w-pixel_size*LbTextStringWidth(lv_name))/2 / pixel_size, 32 / pixel_size, lv_name);
  }
}

void redraw_parchment_view(void)
{
  static const char *func_name="redraw_parchment_view";
  char *fname;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
  static const char *func_name="redraw_display";
  //_DK_redraw_display();return;
  char *text;
  struct PlayerInfo *player;
  int i;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);

  player->field_6 &= 0xFEu;
  if (game.flagfield_14EA4A == 1)
    return;

  if (grabbed_small_map == 2)
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
  LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  lbFontPtr = winfont;
  lbDisplay.DrawFlags &= 0xFFBFu;
  LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  if (player->field_0 & 0x04)
  {
      text=buf_sprintf( ">%s_", player->strfield_463);
      LbTextDraw(148/pixel_size, 8/pixel_size, text);
  }
  if ( draw_spell_cost )
  {
      long pos_x,pos_y;
      unsigned short drwflags_mem;
      drwflags_mem = lbDisplay.DrawFlags;
      LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
      lbDisplay.DrawFlags = 0;
      lbFontPtr = winfont;
      text = buf_sprintf("%d", draw_spell_cost);
      pos_y = GetMouseY() - pixel_size*LbTextStringHeight(text)/2 - 2;
      pos_x = GetMouseX() - pixel_size*LbTextStringWidth(text)/2;
      LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, text);
      lbDisplay.DrawFlags = drwflags_mem;
      draw_spell_cost = 0;
  }
  if ( is_bonus_level(game.numfield_14A83D) )
    draw_bonus_timer();
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
        LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
        draw_slab64k(pos_x, pos_y, w, h);
        LbTextDraw(0/pixel_size, 0/pixel_size, strings[320]);
        LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
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
    text=buf_sprintf(" %s %03d", strings[646], i/2);
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
    LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
    draw_slab64k(pos_x, pos_y, w, h);
    LbTextDraw(0/pixel_size, 0/pixel_size, text);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  }
  draw_sound_stuff();
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void find_frame_rate(void)
{
  static TbClockMSec prev_time2=0;
  static TbClockMSec cntr_time2=0;
  unsigned long curr_time;
  curr_time = LbTimerClock();
  cntr_time2++;
  if ( curr_time-prev_time2 >= 1000 )
  {
    double time_fdelta = 1000.0*((double)(cntr_time2))/(curr_time-prev_time2);
    prev_time2 = curr_time;
    game.time_delta = (unsigned long)(time_fdelta*256.0);
    cntr_time2 = 0;
  }
}

void packet_load_find_frame_rate(unsigned long incr)
{
  static TbClockMSec start_time=0;
  static TbClockMSec extra_frames=0;
  TbClockMSec curr_time;
  curr_time = LbTimerClock();
  if ((curr_time-start_time) < 5000)
  {
    extra_frames += incr;
  } else
  {
    double time_fdelta = 1000.0*((double)(extra_frames+incr))/(curr_time-start_time);
    start_time = curr_time;
    game.time_delta = (unsigned long)(time_fdelta*256.0);
    extra_frames = 0;
  }
}

/*
 * Checks if the game screen needs redrawing.
 */
short display_should_be_updated_this_turn(void)
{
  if ((game.numfield_C & 0x01) != 0)
    return true;
  if ( (game.turns_fastforward==0) && (!game.numfield_149F38) )
  {
    find_frame_rate();
    if ( (game.timingvar1==0) || ((game.seedchk_random_used % game.timingvar1)==0))
      return true;
  } else
  if ( ((game.seedchk_random_used & 0x3F)==0) ||
       ((game.numfield_149F38) && ((game.seedchk_random_used & 7)==0)) )
  {
    packet_load_find_frame_rate(64);
    return true;
  }
  return false;
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
  static const char *func_name="keeper_screen_redraw";
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
  char *text;
  unsigned int msg_pos;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
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
      text=buf_sprintf("OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
      error(func_name, 413, text);
      if ( LbScreenIsLocked() )
        LbTextDraw(300/pixel_size, msg_pos/pixel_size, "OUT OF SYNC");
      msg_pos += 20;
  }
  if (game.numfield_A & 0x04)
  {
      text=buf_sprintf("SEED OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
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
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    PaletteSetPlayerPalette(player, _DK_palette);
    if (game.numfield_C & 0x02)
      initialise_eye_lenses();
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Entering the gameplay loop for level %d\n",(int)game.level_number);
#endif

    //the main gameplay loop starts
    while ( (!quit_game)&&(!exit_keeper) )
    {
      if ( game.flags_font & 0x10 )
      {
        if ( game.seedchk_random_used == 4 )
          LbNetwork_ChangeExchangeTimeout(0);
      }

      // Check if we should redraw screen in this turn
      do_draw = display_should_be_updated_this_turn() || (!LbIsActive());

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
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Gameplay loop finished after %u turns\n",game.seedchk_random_used);
#endif
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
  static const char *func_name="continue_game_available";
  unsigned char buf[12];
  char *fname;
  static short continue_needs_checking_file = 1;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  wait_for_cd_to_be_available();
  fname=prepare_file_path(FGrp_Save,"continue.sav");
  if (LbFileLength(fname) != sizeof(struct Game))
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: No correct .SAV file; there's no continue\n",func_name);
  #endif
    return false;
  }
  if ( continue_needs_checking_file )
  {
    TbFileHandle fh=LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
    if ( fh != -1 )
    {
      LbFileRead(fh, buf, 10);
      LbFileClose(fh);
      if (((struct Game *)buf)->continue_level > 0)
        game.level_number = ((struct Game *)buf)->continue_level;
    }
    continue_needs_checking_file = 0;
  }
  if (is_singleplayer_level(game.level_number))
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Returning that continue is available\n",func_name);
  #endif
    return true;
  } else
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: The level in .SAV isn't correct continue level\n",func_name);
  #endif
    return false;
  }
}

void intro(void)
{
    char *fname;
    fname=prepare_file_path(FGrp_LoData,"intromix.smk");
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Playing intro movie \"%s\"\n",fname);
#endif
    play_smacker_file(fname, 1);
}

void outro(void)
{
    char *fname;
    fname=prepare_file_path(FGrp_LoData,"outromix.smk");
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Playing outro movie \"%s\"\n",fname);
#endif
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

void init_dungeons(void)
{
  int i,k;
  struct Dungeon *dungeon;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = &(game.dungeon[game.field_14E496%DUNGEONS_COUNT]);
    dungeon->field_B01[i%DUNGEONS_COUNT] = game.field_14E946;
    dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
    dungeon->field_B01[game.field_14E496%DUNGEONS_COUNT] = game.field_14E946;
    dungeon->field_918 = 0;
    dungeon->field_919 = 0;
    dungeon->field_2D = 0;
    dungeon->field_2F = 0;
    dungeon->field_E9F = i;
    dungeon->field_105C = game.field_14EB8C;
    dungeon->field_139F = 0;
    dungeon->field_13A3 = 0;
    for (k=0; k < DUNGEONS_COUNT; k++)
    {
      if (k == i)
        dungeon->field_B01[k] = game.field_14E94A;
      else
        dungeon->field_B01[k] = game.field_14E946;
    }
    memset(dungeon->field_1489, 0, 32);
  }
}

void load_level_file(long lvnum)
{
  _DK_load_level_file(lvnum);
}

void load_map_file(long lvidx)
{
  load_level_file(lvidx);
}

void clear_messages(void)
{
  int i;
  for (i=0; i<MESSAGE_QUEUE_COUNT; i++)
  {
    memset(&message_queue[i], 0, sizeof(struct MessageQueueEntry));
  }
}

void init_dungeon_owner(unsigned short owner)
{
  static const char *func_name="init_dungeon_owner";
  struct Dungeon *dungeon;
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[2].field_4;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds deteted");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->field_67;
    if ((game.objects_config[thing->field_1A].field_6) && (thing->owner == owner))
    {
      dungeon = &(game.dungeon[owner%DUNGEONS_COUNT]);
      dungeon->field_0 = thing->field_1B;
      break;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void init_level(void)
{
  static const char *func_name="init_level";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  struct Thing *thing;
  struct Coord3d pos;
  unsigned short mem1,mem2;
  int i,k;
  //_DK_init_level(); return;

  mem2 = game.field_1517E6;
  mem1 = game.field_1517E7;
  memset(&pos,0,sizeof(struct Coord3d));
  game.field_14BB4A = 1;
  free_swipe_graphic();
  game.field_1516FF = -1;
  game.seedchk_random_used = 0;
  clear_game();
  reset_heap_manager();
  setup_heap_manager();
  init_good_player_as(hero_player);
  light_set_lights_on(1);
  start_rooms = &game.rooms[1];
  end_rooms = &game.rooms[150];
  init_dungeons();
  load_map_file(game.numfield_14A83D);
  init_navigation();
  clear_messages();
  game.field_14BB4E = (unsigned long)LbTimeSec();
  if (!SoundDisabled)
  {
    game.field_14BB54 = (seed_check_random(67, &game.field_14BB4E, func_name, 5712) % 3 + 1);
    game.field_14BB55 = 0;
  }
  light_set_lights_on(1);
  init_dungeon_owner(game.field_14E496);
  game.numfield_D |= 0x04;
  game.field_1517E6 = mem2;
  game.field_1517E7 = mem1;
  event_initialise_all();
  battle_initialise();

  thing = create_ambient_sound(&pos, 1, game.field_14E497);
  if (thing != NULL)
    game.field_14E906 = thing->field_1B;
  else
    error(func_name, 481, "Could not create ambient sound object");
  zero_messages();
  game.field_150356 = 0;
  game.field_15035A = 0;
  init_messages();
  game.field_1517FB = 0;
  game.field_1517FC = 0;
  game.field_15033A = 0;
  game.field_151801 = 0;
  game.field_151805 = 0;
  game.field_151809 = 0;
  game.chosen_spell_type = 0;
  game.chosen_spell_look = 0;
  game.chosen_spell_tooltip = 0;
  game.numfield_151819 = 0;
  game.numfield_15181D = 0;
  game.numfield_151821 = 0;
}

void pannel_map_update(long x, long y, long w, long h)
{
  _DK_pannel_map_update(x, y, w, h);
}

void init_player_music(struct PlayerInfo *player)
{
  static const char *func_name="init_player_music";
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

void init_player(struct PlayerInfo *player, int no_explore)
{
  static const char *func_name="init_player";
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_init_player(player, no_explore); return;
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
    if ( !no_explore )
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
    init_player_music(player);
  }
  player->field_2A = (1 << player->field_2B);
  player->field_10 = 0;
}

void init_players(void)
{
  struct PlayerInfo *player;
  int i;
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
}

void post_init_level(void)
{
  _DK_post_init_level(); return;
}

void post_init_players(void)
{
  _DK_post_init_players(); return;
}

void init_animating_texture_maps(void)
{
  _DK_init_animating_texture_maps(); return;
}

void startup_saved_packet_game(void)
{
  //_DK_startup_saved_packet_game(); return;
  struct PlayerInfo *player;
  int i;
  clear_packets();
  open_packet_file_for_load(game.packet_fname);
  game.numfield_14A83D = game.packet_save_head.field_4;
  lbDisplay.DrawColour = colours[15][15][15];
  game.gameturn = 0;
#if (BFDEBUG_LEVEL > 0)
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
#endif
  game.flagfield_14EA4A = 2;
  if (!(game.packet_save_head.field_C & (1 << game.numfield_149F46))
    || (game.packet_save_head.field_D & (1 << game.numfield_149F46)) )
    my_player_number = 0;
  else
    my_player_number = game.numfield_149F46;
  init_level();
  init_players();
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

void setup_computer_players(void)
{
  static const char *func_name="setup_computer_players";
  struct PlayerInfo *player;
  struct Thing *thing;
  int plr_idx;
  int i,k;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  for (plr_idx=0; plr_idx<PLAYERS_COUNT; plr_idx++)
  {
      player=&(game.players[plr_idx]);
      if ((player->field_0 & 0x01) == 0)
      {
        k = 0;
        i = game.thing_lists[2].field_4;
        while (i>0)
        {
          if (i >= THINGS_COUNT)
          {
            error(func_name,4578,"Jump out of things array bounds deteted");
            break;
          }
          thing = game.things_lookup[i];
          if ((thing == game.things_lookup[0]) || (thing == NULL))
            break;
          i = thing->field_67;
          if ((game.objects_config[thing->field_1A].field_6) && (thing->owner == plr_idx))
          {
            script_support_setup_player_as_computer_keeper(plr_idx, 0);
            break;
          }
          k++;
          if (k > THINGS_COUNT)
          {
            error(func_name,4579,"Infinite loop detected when sweeping things list");
            break;
          }
        }
      }
  }
}

void setup_alliances(void)
{
  int i;
  struct PlayerInfo *player;
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

/*
 * Exchanges verification packets between all players.
 * @return Returns true if all players return same checksum.
 */
short perform_checksum_verification()
{
  static const char *func_name="perform_checksum_verification";
  struct PlayerInfo *player;
  struct Packet *pckt;
  struct Thing *thing;
  unsigned long checksum_mem;
  short result;
  int i;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  result = true;
  checksum_mem = 0;
  for (i=1; i<THINGS_COUNT; i++)
  {
      thing = game.things_lookup[i];
      if ((thing == game.things_lookup[0]) || (thing == NULL))
        continue;
      if (thing->field_0 & 0x01)
      {
        checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
      }
  }
  clear_packets();
  pckt = &game.packets[player->field_B];
  set_packet_action(pckt, 12, 0, 0, 0, 0);
  pckt->field_4 = checksum_mem + game.field_14BB4A;
  if (LbNetwork_Exchange(pckt))
  {
    error(func_name, 210, "Network exchange failed on level checksum verification");
    result = false;
  }
  if ( checksums_different() )
  {
    error(func_name, 219, "Level checksums different for network players");
    result = false;
  }
  return result;
}

void post_init_packets(void)
{
  if ((game.field_149E81) && (game.numfield_149F47))
  {
      open_packet_file_for_load(game.packet_fname);
      game.gameturn = 0;
  }
  clear_packets();
}

short setup_select_player_number(void)
{
  static const char *func_name="setup_select_player_number";
  struct PlayerInfo *player;
  short is_set;
  int i,k;
  is_set = 0;
  k = 0;
#if (BFDEBUG_LEVEL > 6)
  LbSyncLog("%s: Starting\n",func_name);
#endif
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
  return is_set;
}

void setup_exchange_player_number(void)
{
  static const char *func_name="setup_exchange_player_number";
  struct PlayerInfo *player;
  struct Packet *pckt;
  int i,k;
#if (BFDEBUG_LEVEL > 6)
  LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  clear_packets();
  pckt = &game.packets[my_player_number];
  set_packet_action(pckt, 10, player->field_2C, settings.field_3, 0, 0);
  pckt = &game.packets[my_player_number];
  if ( LbNetwork_Exchange(pckt) )
      error(func_name, 156, "LbNetwork_Exchange failed");
  k = 0;
  for (i=0; i<NET_PLAYERS_COUNT; i++)
  {
      pckt = &game.packets[i];
      if ((net_player_info[i].field_20) && (pckt->field_5 == 10))
      {
          player = &(game.players[k%PLAYERS_COUNT]);
          player->field_2B = k;
          player->field_0 |= 0x01;
          if (pckt->field_8 < 1u)
            player->field_4B5 = 2;
          else
            player->field_4B5 = 5;
          player->field_2C = pckt->field_6;
          init_player(player, 0);
          strncpy(player->field_15,enum_players_callback[i].field_0,sizeof(struct TbNetworkCallbackData));
          k++;
      }
  }
}
void init_players_local_game(void)
{
  static const char *func_name="init_players_local_game";
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 4)
  LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_2B = my_player_number;
  player->field_0 |= 0x01;
  if (settings.field_3 < 1u)
    player->field_4B5 = 2;
  else
    player->field_4B5 = 5;
  init_player(player, 0);
}

void init_players_network_game(void)
{
  static const char *func_name="init_players_network_game";
  int i,k;
  struct PlayerInfo *player;
  struct Packet *pckt;
#if (BFDEBUG_LEVEL > 4)
  LbSyncLog("%s: Starting\n",func_name);
#endif
  if (LbNetwork_ChangeExchangeBuffer(game.packets, sizeof(struct Packet)))
      error(func_name, 119, "Unable to reinitialise ExchangeBuffer");
  setup_select_player_number();
  setup_exchange_player_number();
  perform_checksum_verification();
  setup_alliances();
}

void setup_count_players(void)
{
  int i;
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
}

void startup_network_game(void)
{
  static const char *func_name="startup_network_game";
#if (BFDEBUG_LEVEL > 0)
  LbSyncLog("Starting up network game.\n");
#endif
  //_DK_startup_network_game(); return;
  unsigned int flgmem;
  struct PlayerInfo *player;
  setup_count_players();
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  flgmem = player->field_2C;
  init_level();
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_2C = flgmem;
  if (game.flagfield_14EA4A == 2)
  {
    init_players_local_game();
  } else
  {
    init_players_network_game();
  }
  if (fe_computer_players)
    setup_computer_players();
  post_init_level();
  post_init_players();
  post_init_packets();
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

int setup_old_network_service(void)
{
    return setup_network_service(_DK_net_service_index_selected);
}

void wait_at_frontend(void)
{
  static const char *func_name="wait_at_frontend";
  //_DK_wait_at_frontend(); return;
#if (BFDEBUG_LEVEL > 0)
  LbSyncLog("Falling into frontend menu.\n");
#endif

  //Moon phase calculation
  calculate_moon_phase(true,false);
  if ( game.flags_cd & 0x01 )
    game.field_149E81 = 0;
  game.numfield_15 = -1;

  if (!load_default_campaign())
  {
    error(func_name, 731, "Unable to load default campaign");
    exit_keeper = 1;
    return;
  }

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
  if ( !frontend_load_data() )
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
#if (BFDEBUG_LEVEL > 0)
      LbSyncLog("%s: Windows Control exit condition invoked\n",func_name);
#endif
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
#if (BFDEBUG_LEVEL > 0)
      LbSyncLog("%s: Frontend Input exit condition invoked\n",func_name);
#endif
      break; // end while
    }

//LbSyncLog("frontend_update\n");
    frontend_update(&finish_menu);
    if ( exit_keeper )
    {
#if (BFDEBUG_LEVEL > 0)
      LbSyncLog("%s: Frontend Update exit condition invoked\n",func_name);
#endif
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
  short prev_state = frontend_menu_state;
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
          load_game(game.numfield_15);
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
#if (BFDEBUG_LEVEL > 0)
  LbSyncLog("Entering gameplay loop.\n");
#endif
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
    frontstats_initialise();
    delete_all_structures();
    clear_mapwho();
    endtime = LbTimerClock();
    quit_game = 0;
    if ( game.numfield_C & 0x02 )
        exit_keeper=true;
    playtime += endtime-starttime;
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Play time is %d seconds\n",playtime>>10);
#endif
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
  if (game.numfield_A & 0x08)
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
      if ( stricmp(parstr, "vidsmooth") == 0 )
      {
          smooth_on = 1;
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
#if (BFDEBUG_LEVEL > 0)
        LbSyncLog("Warning - Unrecognized command line parameter '%s'.\n",parstr);
#endif
        bad_param=narg;
      }
      narg++;
  }

  if (level_num == -1)
    level_num = first_singleplayer_level();
  game.numfield_16 = level_num;
  game.level_number = level_num;
  if ( (game.numfield_C & 0x02) == 0 )
    game.level_number = first_singleplayer_level();
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
  short retval;
  retval=0;
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
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("%s finished properly.\n",func_name);
#endif
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
  char *text;
  _DK_hInstance = hThisInstance;
  _DK_lpDDC = NULL;

  get_cmdln_args(bf_argc, bf_argv);

//TODO: delete when won't be needed anymore
  memcpy(_DK_menu_list,menu_list,sizeof(menu_list));
#if (BFDEBUG_LEVEL > 1)
  if (sizeof(struct Game)!=SIZEOF_Game)
  {
      text = buf_sprintf("Bad compilation - struct Game has wrong size!\nThe difference is %d bytes.\n",sizeof(struct Game)-SIZEOF_Game);
      error_dialog(func_name, 1, text);
      return 0;
  }
#endif
/*  {
  struct PlayerInfo *player=&(game.players[0]);
      static char msg_text[255];
      sprintf(msg_text,"Position of the first Player is %06x, first Camera is %06x bytes.\n",((int)player) - ((int)&_DK_game),((int)&(player->camera)) - ((int)player));
      error_dialog(func_name, 1, msg_text);
      return 0;
  }*/

  LbBullfrogMain(bf_argc, bf_argv);

//  LbFileSaveAt("!tmp_file", &_DK_game, sizeof(struct Game));

  return 0;
}
