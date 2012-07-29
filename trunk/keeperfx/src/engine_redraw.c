/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_redraw.c
 *     Functions to redraw the engine screen.
 * @par Purpose:
 *     High level redrawing routines.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     06 Nov 2010 - 03 Jul 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "engine_redraw.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_sprfnt.h"
#include "bflib_sound.h"

#include "player_data.h"
#include "dungeon_data.h"
#include "player_instances.h"
#include "kjm_input.h"
#include "gui_parchment.h"
#include "gui_draw.h"
#include "gui_boxmenu.h"
#include "gui_tooltips.h"
#include "power_hand.h"
#include "engine_render.h"
#include "front_easter.h"
#include "frontend.h"
#include "frontmenu_ingame_tabs.h"
#include "vidmode.h"
#include "config.h"
#include "config_terrain.h"
#include "game_merge.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_redraw_display(void);
DLLIMPORT void _DK_process_pointer_graphic(void);
DLLIMPORT void _DK_smooth_screen_area(unsigned char *a1, long a2, long a3, long a4, long a5, long a6);
DLLIMPORT void _DK_redraw_creature_view(void);
DLLIMPORT void _DK_redraw_isometric_view(void);
DLLIMPORT void _DK_redraw_frontview(void);
DLLIMPORT void _DK_draw_overlay_compass(long a1, long a2);
DLLIMPORT void _DK_update_explored_flags_for_power_sight(struct PlayerInfo *player);
DLLIMPORT long _DK_map_fade_in(long a);
DLLIMPORT long _DK_map_fade_out(long a);
DLLIMPORT void _DK_message_draw(void);
DLLIMPORT void _DK_draw_sound_stuff(void);
/******************************************************************************/
void setup_engine_window(long x, long y, long width, long height)
{
    struct PlayerInfo *player;
    SYNCDBG(6,"Starting for size (%ld,%ld) at (%ld,%ld)",width,height,x,y);
    player=get_my_player();
    if (game.numfield_C & 0x20)
    {
      if (x > MyScreenWidth)
        x = MyScreenWidth;
      if (x < status_panel_width)
        x = status_panel_width;
    } else
    {
      if (x > MyScreenWidth)
        x = MyScreenWidth;
      if (x < 0)
        x = 0;
    }
    if (y > MyScreenHeight)
      y = MyScreenHeight;
    if (y < 0)
      y = 0;
    if (x+width > MyScreenWidth)
      width = MyScreenWidth-x;
    if (width < 0)
      width = 0;
    if (y+height > MyScreenHeight)
      height = MyScreenHeight-y;
    if (height < 0)
      height = 0;
    player->engine_window_x = x;
    player->engine_window_y = y;
    player->engine_window_width = width;
    player->engine_window_height = height;
}

void store_engine_window(TbGraphicsWindow *ewnd,int divider)
{
    struct PlayerInfo *player;
    player=get_my_player();
    if (divider <= 1)
    {
        ewnd->x = player->engine_window_x;
        ewnd->y = player->engine_window_y;
        ewnd->width = player->engine_window_width;
        ewnd->height = player->engine_window_height;
    } else
    {
        ewnd->x = player->engine_window_x/divider;
        ewnd->y = player->engine_window_y/divider;
        ewnd->width = player->engine_window_width/divider;
        ewnd->height = player->engine_window_height/divider;
    }
}

void load_engine_window(TbGraphicsWindow *ewnd)
{
    struct PlayerInfo *player;
    player=get_my_player();
    player->engine_window_x = ewnd->x;
    player->engine_window_y = ewnd->y;
    player->engine_window_width = ewnd->width;
    player->engine_window_height = ewnd->height;
}

long map_fade_in(long a)
{
  SYNCDBG(6,"Starting");
  return _DK_map_fade_in(a);
}

long map_fade_out(long a)
{
  SYNCDBG(6,"Starting");
  return _DK_map_fade_out(a);
}

void draw_overlay_compass(long a1, long a2)
{
  _DK_draw_overlay_compass(a1, a2);
}

void message_draw(void)
{
  int i,h;
  long x,y;
  SYNCDBG(7,"Starting");
  LbTextSetFont(winfont);
  h = LbTextLineHeight();
  x = 148;
  y = 28;
  for (i=0; i < game.active_messages_count; i++)
  {
      LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
      set_flag_word(&lbDisplay.DrawFlags,0x0040,false);
      LbTextDraw((x+32)/pixel_size, y/pixel_size, game.messages[i].text);
      draw_gui_panel_sprite_left(x, y, 488+game.messages[i].field_40);
      y += pixel_size * h;
  }
}

void store_backup_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon *dungeon;
    long stl_x,stl_y;
    long soe_x,soe_y;
    dungeon = get_players_dungeon(player);
    stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
    for (soe_y=0; soe_y < 2*MAX_SOE_RADIUS; soe_y++,stl_y++)
    {
        stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
        for (soe_x=0; soe_x < 2*MAX_SOE_RADIUS; soe_x++,stl_x++)
        {
            if (dungeon->soe_explored_flags[soe_y][soe_x])
            {
                struct Map *mapblk;
                mapblk = get_map_block_at(stl_x, stl_y);
                if (!map_block_invalid(mapblk))
                {
                    if (map_block_revealed(mapblk, player->id_number))
                        backup_explored[soe_y][soe_x] |= 0x01;
                    if ((mapblk->flags & MapFlg_Unkn04) != 0)
                        backup_explored[soe_y][soe_x] |= 0x02;
                    if ((mapblk->flags & MapFlg_Unkn80) != 0)
                        backup_explored[soe_y][soe_x] |= 0x04;
                }
            }
        }
    }
}

void update_vertical_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon *dungeon;
    long stl_x,stl_y;
    long soe_x,soe_y;
    long slb_x,slb_y;
    long boundstl_x;
    long delta;
    long i;
    dungeon = get_players_dungeon(player);
    stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
    for (soe_y=0; soe_y < 2*MAX_SOE_RADIUS; soe_y++,stl_y++)
    {
        if ( (stl_y >= 0) && (stl_y <= 255) )
        {
            stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
            for (soe_x=0; soe_x <= MAX_SOE_RADIUS; soe_x++,stl_x++)
            {
                if (dungeon->soe_explored_flags[soe_y][soe_x])
                {
                    soe_x++;
                    // Find max value for delta
                    delta = 0;
                    for (i=1; soe_x < 2*MAX_SOE_RADIUS; soe_x++,i++)
                    {
                        if ( dungeon->soe_explored_flags[soe_y][soe_x] )
                            delta = i;
                    }
                    boundstl_x = stl_x + delta;
                    if (stl_x < 0)
                    {
                        stl_x = 0;
                    } else
                    if (stl_x > map_subtiles_x-1)
                    {
                        stl_x = map_subtiles_x-1;
                    }
                    if (boundstl_x < 0)
                    {
                        boundstl_x = 0;
                    } else
                    if (boundstl_x > map_subtiles_x-1)
                    {
                        boundstl_x = map_subtiles_x-1;
                    }
                    if (boundstl_x >= stl_x)
                    {
                        delta = boundstl_x - stl_x + 1;
                        slb_y = map_to_slab[stl_y];
                        for (i=0; i < delta; i++)
                        {
                            struct Map *mapblk;
                            struct SlabMap *slb;
                            struct SlabAttr *slbattr;
                            mapblk = get_map_block_at(stl_x+i, stl_y);
                            reveal_map_block(mapblk, player->id_number);
                            slb_x = map_to_slab[stl_x+i];
                            slb = get_slabmap_block(slb_x, slb_y);
                            slbattr = get_slab_attrs(slb);
                            if ( !slbattr->field_14 )
                                mapblk->flags &= ~(MapFlg_Unkn80|MapFlg_Unkn04);
                            mapblk++;
                        }
                        stl_x += delta;
                    }
                }
            }
        }
    }
}

void update_horizonal_explored_flags_for_power_sight(struct PlayerInfo *player, struct Coord3d *soe_pos)
{
    struct Dungeon *dungeon;
    long stl_x,stl_y;
    long soe_x,soe_y;
    long boundstl_y;
    long slb_x,slb_y;
    long delta;
    long i;
    dungeon = get_players_dungeon(player);
    stl_x = (long)soe_pos->x.stl.num - MAX_SOE_RADIUS;
    for (soe_x=0; soe_x < 2*MAX_SOE_RADIUS; soe_x++,stl_x++)
    {
        if ( (stl_x >= 0) && (stl_x <= 255) )
        {
            stl_y = (long)soe_pos->y.stl.num - MAX_SOE_RADIUS;
            for (soe_y=0; soe_y <= MAX_SOE_RADIUS; soe_y++,stl_y++)
            {
                if (dungeon->soe_explored_flags[soe_y][soe_x])
                {
                    soe_y++;
                    // Find max value for delta
                    delta = 0;
                    for (i=1; soe_y < 2*MAX_SOE_RADIUS; soe_y++,i++)
                    {
                        if (dungeon->soe_explored_flags[soe_y][soe_x])
                            delta = i;
                    }
                    boundstl_y = stl_y + delta;
                    if (boundstl_y < 0)
                    {
                        boundstl_y = 0;
                    } else
                    if (boundstl_y > map_subtiles_y-1)
                    {
                        boundstl_y = map_subtiles_y-1;
                    }
                    if (stl_y < 0)
                    {
                        stl_y = 0;
                    } else
                    if (stl_y > map_subtiles_y-1)
                    {
                        stl_y = map_subtiles_y-1;
                    }
                    if (stl_y <= boundstl_y)
                    {
                      delta = boundstl_y - stl_y + 1;
                      slb_x = map_to_slab[stl_x];
                      for (i=0; i < delta; i++)
                      {
                          struct Map *mapblk;
                          slb_y = map_to_slab[stl_y+i];
                          mapblk = get_map_block_at(stl_x, stl_y+i);
                          reveal_map_block(mapblk, player->id_number);
                          struct SlabMap *slb;
                          struct SlabAttr *slbattr;
                          slb = get_slabmap_block(slb_x, slb_y);
                          slbattr = get_slab_attrs(slb);
                          if ( !slbattr->field_14 )
                              mapblk->flags &= ~(MapFlg_Unkn80|MapFlg_Unkn04);
                      }
                      stl_y += delta;
                    }
                }
            }
        }
    }
}

void update_explored_flags_for_power_sight(struct PlayerInfo *player)
{
    struct Dungeon *dungeon;
    struct Thing *thing;
    SYNCDBG(9,"Starting");
    //_DK_update_explored_flags_for_power_sight(player);
    dungeon = get_players_dungeon(player);
    memset(backup_explored, 0, sizeof(backup_explored));
    if (dungeon->keeper_sight_thing_idx == 0)
        return;
    thing = thing_get(dungeon->keeper_sight_thing_idx);
    // Fill the backup_explored array
    store_backup_explored_flags_for_power_sight(player, &thing->mappos);
    update_vertical_explored_flags_for_power_sight(player, &thing->mappos);
    update_horizonal_explored_flags_for_power_sight(player, &thing->mappos);

}

void redraw_creature_view(void)
{
    SYNCDBG(6,"Starting");
    TbGraphicsWindow ewnd;
    struct PlayerInfo *player;
    struct Thing *thing;
    //_DK_redraw_creature_view(); return;
    player = get_my_player();
    if (player->field_45F != 2)
      player->field_45F = 2;
    update_explored_flags_for_power_sight(player);
    thing = thing_get(player->controlled_thing_idx);
    if (!thing_is_invalid(thing))
      draw_creature_view(thing);
    if (smooth_on)
    {
      store_engine_window(&ewnd,pixel_size);
      smooth_screen_area(lbDisplay.WScreen, ewnd.x, ewnd.y,
          ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
    }
    remove_explored_flags_for_power_sight(player);
    draw_swipe();
    if ((game.numfield_C & 0x20) != 0)
      draw_whole_status_panel();
    draw_gui();
    if ((game.numfield_C & 0x20) != 0)
      draw_overlay_compass(player->mouse_x, player->mouse_y);
    message_draw();
    gui_draw_all_boxes();
    draw_tooltip();
}

void smooth_screen_area(unsigned char *scrbuf, long x, long y, long w, long h, long scanln)
{
    SYNCDBG(7,"Starting");
    long i,k;
    unsigned char *buf;
    unsigned char *lnbuf;
    unsigned int ghpos;
    lnbuf = scrbuf + scanln*y + x;
    for (i = h-y-1; i>0; i--)
    {
      buf = lnbuf;
      for (k = w-x-1; k>0; k--)
      {
          ghpos = (buf[0] << 8) + buf[1];
          ghpos = (buf[scanln] << 8) + pixmap.ghost[ghpos];
          buf[0] = ghpos;
          buf++;
      }
      lnbuf += scanln;
    }
}

void make_camera_deviations(struct PlayerInfo *player,struct Dungeon *dungeon)
{
    long x,y;
    x = player->acamera->mappos.x.val;
    y = player->acamera->mappos.y.val;
    if (dungeon->camera_deviate_quake != 0)
    {
      x += UNSYNC_RANDOM(80) - 40;
      y += UNSYNC_RANDOM(80) - 40;
    }
    if (dungeon->camera_deviate_jump != 0)
    {
      x += ( (dungeon->camera_deviate_jump * LbSinL(player->acamera->orient_a) >> 8) >> 8);
      y += (-(dungeon->camera_deviate_jump * LbCosL(player->acamera->orient_a) >> 8) >> 8);
    }
    if ((dungeon->camera_deviate_quake != 0) || (dungeon->camera_deviate_jump != 0))
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
      player->acamera->mappos.x.val = x;
      player->acamera->mappos.y.val = y;
    }
}

void redraw_isometric_view(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    TbGraphicsWindow ewnd;
    struct Coord3d pos;
    SYNCDBG(6,"Starting");
    //_DK_redraw_isometric_view(); return;

    player = get_my_player();
    if (player->acamera == NULL)
        return;
    memcpy(&pos,&player->acamera->mappos,sizeof(struct Coord3d));
    memset(&ewnd,0,sizeof(TbGraphicsWindow));
    if (player->field_45F != 1)
      player->field_45F = 1;
    dungeon = get_players_num_dungeon(my_player_number);
    // Camera position modifications
    make_camera_deviations(player,dungeon);
    update_explored_flags_for_power_sight(player);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
        store_engine_window(&ewnd,1);
        setup_engine_window(ewnd.x, ewnd.y, ewnd.width >> 1, ewnd.height >> 1);
    }
    engine(player,&player->cameras[0]);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
        load_engine_window(&ewnd);
    }
    if (smooth_on)
    {
        store_engine_window(&ewnd,pixel_size);
        smooth_screen_area(lbDisplay.WScreen, ewnd.x, ewnd.y,
            ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
    }
    remove_explored_flags_for_power_sight(player);
    if ((game.numfield_C & 0x20) != 0)
        draw_whole_status_panel();
    draw_gui();
    if ((game.numfield_C & 0x20) != 0)
        draw_overlay_compass(player->mouse_x, player->mouse_y);
    message_draw();
    gui_draw_all_boxes();
    draw_power_hand();
    draw_tooltip();
    memcpy(&player->acamera->mappos,&pos,sizeof(struct Coord3d));
    SYNCDBG(8,"Finished");
}

void redraw_frontview(void)
{
    SYNCDBG(6,"Starting");
    //_DK_redraw_frontview();
    struct PlayerInfo *player;
    long w,h;
    player = get_my_player();
    update_explored_flags_for_power_sight(player);
    if ((game.flags_font & FFlg_unk08) != 0)
    {
      w = player->engine_window_width;
      h = player->engine_window_height;
      setup_engine_window(player->engine_window_x, player->engine_window_y, w, h >> 1);
    } else
    {
      w = 0;
      h = 0;
    }
    draw_frontview_engine(&player->cameras[3]);
    if ((game.flags_font & FFlg_unk08) != 0)
      setup_engine_window(player->engine_window_x, player->engine_window_y, w, h);
    remove_explored_flags_for_power_sight(player);
    if ((game.numfield_C & 0x20) != 0)
      draw_whole_status_panel();
    draw_gui();
    if ((game.numfield_C & 0x20) != 0)
      draw_overlay_compass(player->mouse_x, player->mouse_y);
    message_draw();
    draw_power_hand();
    draw_tooltip();
    gui_draw_all_boxes();
}

void process_pointer_graphic(void)
{
  struct PlayerInfo *player;
  struct SpellData *pwrdata;
  struct Dungeon *dungeon;
  struct Thing *thing;
  long i;
  //_DK_process_pointer_graphic(); return;
  player = get_my_player();
  dungeon = get_dungeon(player->id_number);
  if (dungeon_invalid(dungeon))
  {
      set_pointer_graphic(0);
      return;
  }
  SYNCDBG(6,"Starting for view %d, player state %d, instance %d",(int)player->view_type,(int)player->work_state,(int)player->instance_num);
  switch (player->view_type)
  {

  case 1:
      if (player->instance_num == PI_MapFadeFrom)
      {
        set_pointer_graphic(0);
      } else
      if (((game.numfield_C & 0x20) != 0) && mouse_is_over_small_map(player->mouse_x, player->mouse_y))
      {
          if (game.small_map_state == 2)
            set_pointer_graphic(0);
          else
            set_pointer_graphic(1);
      } else
      if (battle_creature_over > 0)
      {
          i = -1;
          if (player->work_state < PLAYER_STATES_COUNT)
            i = player_state_to_spell[player->work_state];
          pwrdata = get_power_data(i);
          if ((i > 0) && (pwrdata->flag_19))
          {
            thing = thing_get(battle_creature_over);
            draw_spell_cursor(player->work_state, battle_creature_over,
                thing->mappos.x.stl.num, thing->mappos.y.stl.num);
          } else
          {
            set_pointer_graphic(1);
          }
      } else
      if (game_is_busy_doing_gui())
      {
        set_pointer_graphic(1);
      } else
      switch (player->work_state)
      {
      case PSt_CtrlDungeon:
          if (player->field_455)
            i = player->field_455;
          else
            i = player->field_454;
          switch (i)
          {
          case 1:
              set_pointer_graphic(2);
              break;
          case 2:
              set_pointer_graphic(39);
              break;
          case 3:
              thing = thing_get(player->thing_under_hand);
              if ((!thing_is_invalid(thing)) && (player->field_4) && (dungeon->things_in_hand[0] != player->thing_under_hand)
                  && can_thing_be_possessed(thing, player->id_number))
              {
                if (is_feature_on(Ft_BigPointer))
                {
                  set_pointer_graphic(96+(game.play_gameturn%i));
                } else
                {
                  set_pointer_graphic(47);
                }
                player->field_6 |= 0x01;
              } else
              if ((!thing_is_invalid(thing)) && (player->field_5) && (dungeon->things_in_hand[0] != player->thing_under_hand)
                  && can_thing_be_queried(thing, player->id_number))
              {
                set_pointer_graphic(4);
                player->field_6 |= 0x01;
              } else
              {
                if ((player->field_3 & 0x02) != 0)
                  set_pointer_graphic(2);
                else
                  set_pointer_graphic(0);
              }
              break;
          default:
              if (player->field_10 <= game.play_gameturn)
                set_pointer_graphic(1);
              else
                set_pointer_graphic(0);
              break;
          }
          break;
      case PSt_BuildRoom:
          switch (player->chosen_room_kind)
          {
          case 2:
              set_pointer_graphic(25);
              break;
          case 3:
              set_pointer_graphic(27);
              break;
          case 4:
              set_pointer_graphic(29);
              break;
          case 5:
              set_pointer_graphic(28);
              break;
          case 6:
              set_pointer_graphic(30);
              break;
          case 8:
              set_pointer_graphic(34);
              break;
          case 9:
              set_pointer_graphic(35);
              break;
          case 10:
              set_pointer_graphic(33);
              break;
          case 11:
              set_pointer_graphic(32);
              break;
          case 12:
              set_pointer_graphic(31);
              break;
          case 13:
              set_pointer_graphic(26);
              break;
          case 14:
              set_pointer_graphic(36);
              break;
          case 15:
              set_pointer_graphic(37);
              break;
          case 16:
              set_pointer_graphic(38);
              break;
          }
          return;
      case PSt_Unknown5:
      case PSt_Slap:
          set_pointer_graphic(0);
          break;
      case PSt_CallToArms:
      case PSt_CaveIn:
      case PSt_SightOfEvil:
      case PSt_CtrlPassngr:
      case PSt_CtrlDirect:
      case PSt_Lightning:
      case PSt_SpeedUp:
      case PSt_Armour:
      case PSt_Conceal:
      case PSt_Heal:
      case PSt_CreateDigger:
      case PSt_DestroyWalls:
      case PSt_CastDisease:
      case PSt_TurnChicken:
          draw_spell_cursor(player->work_state, 0, game.pos_14C006.x.stl.num, game.pos_14C006.y.stl.num);
          break;
      case PSt_Unknown12:
      case PSt_Unknown15:
          set_pointer_graphic(4);
          break;
      case PSt_PlaceTrap:
          switch (player->chosen_trap_kind)
          {
          case 1:
              set_pointer_graphic(5);
              break;
          case 2:
              set_pointer_graphic(9);
              break;
          case 3:
              set_pointer_graphic(7);
              break;
          case 4:
              set_pointer_graphic(8);
              break;
          case 5:
              set_pointer_graphic(6);
              break;
          case 6:
              set_pointer_graphic(10);
              break;
          }
          return;
      case PSt_PlaceDoor:
          switch (player->chosen_door_kind)
          {
          case 1:
              set_pointer_graphic(11);
              break;
          case 2:
              set_pointer_graphic(12);
              break;
          case 3:
              set_pointer_graphic(13);
              break;
          case 4:
              set_pointer_graphic(14);
              break;
          }
          return;
      case PSt_Sell:
          set_pointer_graphic(3);
          break;
      default:
          set_pointer_graphic(1);
          break;
      }
      break;
  case 2:
  case 3:
      if ((game.numfield_D & 0x08) != 0)
        set_pointer_graphic(1);
      else
        set_pointer_graphic(0);
      break;
  case 4:
  case 5:
  case 6:
      set_pointer_graphic(1);
      break;
  case 0:
      set_pointer_graphic_none();
      return;
  default:
      WARNLOG("Unsupported view type");
      set_pointer_graphic_none();
      return;
  }
}

void redraw_display(void)
{
    //_DK_redraw_display();return;
    char *text;
    struct PlayerInfo *player;
    SYNCDBG(5,"Starting");
    player = get_my_player();
    set_flag_byte(&player->field_6,0x01,false);
    if (game.game_kind == GKind_Unknown1)
      return;
    if (game.small_map_state == 2)
      set_pointer_graphic_none();
    else
      process_pointer_graphic();
    switch (player->view_mode)
    {
    case PVM_EmptyView:
        break;
    case PVM_CreatureView:
        redraw_creature_view();
        parchment_loaded = 0;
        break;
    case PVM_IsometricView:
        redraw_isometric_view();
        parchment_loaded = 0;
        break;
    case PVM_ParchmentView:
        redraw_parchment_view();
        break;
    case PVM_FrontView:
        redraw_frontview();
        parchment_loaded = 0;
        break;
    case PVM_ParchFadeIn:
        parchment_loaded = 0;
        player->field_4BD = map_fade_in(player->field_4BD);
        break;
    case PVM_ParchFadeOut:
        parchment_loaded = 0;
        player->field_4BD = map_fade_out(player->field_4BD);
        break;
    default:
        ERRORLOG("Unsupported drawing state, %d",(int)player->view_mode);
        break;
    }
    //LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    LbTextSetFont(winfont);
    lbDisplay.DrawFlags &= 0xFFBFu;
    LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
    if ((player->field_0 & 0x04) != 0)
    {
        text = buf_sprintf( ">%s_", player->mp_message_text);
        LbTextDraw(148/pixel_size, 8/pixel_size, text);
    }
    if ( draw_spell_cost )
    {
        long pos_x,pos_y;
        unsigned short drwflags_mem;
        drwflags_mem = lbDisplay.DrawFlags;
        LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
        lbDisplay.DrawFlags = 0;
        LbTextSetFont(winfont);
        text = buf_sprintf("%d", draw_spell_cost);
        pos_y = GetMouseY() - pixel_size*LbTextStringHeight(text)/2 - 2;
        pos_x = GetMouseX() - pixel_size*LbTextStringWidth(text)/2;
        LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, text);
        lbDisplay.DrawFlags = drwflags_mem;
        draw_spell_cost = 0;
    }
    if (bonus_timer_enabled())
      draw_bonus_timer();
    if (((game.numfield_C & 0x01) != 0) && ((game.numfield_C & 0x80) == 0))
    {
          LbTextSetFont(winfont);
          text = gui_strings[320]; // "Paused"
          long pos_x,pos_y;
          long w,h;
          int i;
          i = LbTextCharWidth(' ');
          w = pixel_size * (LbTextStringWidth(text) + 2*i);
          i = player->view_mode;
          if ((i == 2) || (i == 5) || (i == 1))
            pos_x = player->engine_window_x + (MyScreenWidth-w-player->engine_window_x)/2;
          else
            pos_x = (MyScreenWidth-w)/2;
          pos_y=16;
          i = LbTextLineHeight();
          lbDisplay.DrawFlags = 0x0100;
          h = pixel_size*i + pixel_size*i/2;
          LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
          draw_slab64k(pos_x, pos_y, w, h);
          LbTextDraw(0/pixel_size, 0/pixel_size, text);
          LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    }
    if (game.field_150356 != 0)
    {
      long pos_x,pos_y;
      long w,h;
      int i;
      if (game.armageddon.count_down+game.field_150356 <= game.play_gameturn)
      {
        i = 0;
        if ( game.field_15035A - game.armageddon.duration <= game.play_gameturn )
          i = game.field_15035A - game.play_gameturn;
      } else
      {
        i = game.play_gameturn - game.field_150356 - game.armageddon.count_down;
      }
      LbTextSetFont(winfont);
      text = buf_sprintf(" %s %03d", gui_strings[646], i/2); // Armageddon message
      i = LbTextCharWidth(' ');
      w = pixel_size*LbTextStringWidth(text) + 6*i;
      pos_x = MyScreenWidth - w - 16;
      pos_y = 16;
      i = LbTextLineHeight();
      lbDisplay.DrawFlags = 0x0100;
      h = pixel_size*i + pixel_size*i/2;
      LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
      draw_slab64k(pos_x, pos_y, w, h);
      LbTextDraw(0/pixel_size, 0/pixel_size, text);
      LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    }
    draw_eastegg();
  //show_onscreen_msg(8, "Physical(%d,%d) Graphics(%d,%d) Lens(%d,%d)", (int)lbDisplay.PhysicalScreenWidth, (int)lbDisplay.PhysicalScreenHeight, (int)lbDisplay.GraphicsScreenWidth, (int)lbDisplay.GraphicsScreenHeight, (int)eye_lens_width, (int)eye_lens_height);
    SYNCDBG(7,"Finished");
}

/**
 * Redraws the game display buffer.
 */
TbBool keeper_screen_redraw(void)
{
  struct PlayerInfo *player;
  SYNCDBG(5,"Starting");
  player = get_my_player();
  LbScreenClear(0);
  if (LbScreenLock() == Lb_SUCCESS)
  {
    setup_engine_window(player->engine_window_x, player->engine_window_y,
        player->engine_window_width, player->engine_window_height);
    redraw_display();
    LbScreenUnlock();
    return true;
  }
  return false;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
