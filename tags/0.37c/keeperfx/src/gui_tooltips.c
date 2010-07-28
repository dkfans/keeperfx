/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_tooltips.c
 *     Tooltips support functions.
 * @par Purpose:
 *     Functions to show, draw and update the in-game tooltips.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     26 Feb 2009 - 14 May 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_tooltips.h"
#include "globals.h"
#include <stdarg.h>
#include "bflib_memory.h"
#include "bflib_guibtns.h"
#include "bflib_sprfnt.h"

#include "gui_draw.h"
#include "kjm_input.h"
#include "gui_topmsg.h"
#include "gui_frontmenu.h"
#include "frontend.h"
#include "thing_objects.h"
#include "thing_traps.h"
#include "dungeon_data.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "room_workshop.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_draw_tooltip(void);
DLLIMPORT void _DK_setup_gui_tooltip(struct GuiButton *gbtn);

/******************************************************************************/
const char jtytext[] = "JONTY HERE   : ...I AM WRITING THIS AT 4AM ON KEEPERS LAST DAY. I LOOK AROUND THE OFFICE AND ALL I SEE ARE THE TIRED PALE FACES OF THE KEEPER TEAM. THIS PROJECT HAS DESTROYED THE HEALTH AND SOCIAL LIVES OF EACH MEMBER, SO I HOPE YOU LIKE THE GAME. "
    "AMAZINGLY AFTER SIXTEEN HOURS A DAY, 7 DAYS A WEEK, FOR NEARLY 5 MONTHS WE STILL DO. THIS GAME HAS BEEN WRITTEN WITH A PASSION I AM PROUD TO BE PART OF.... I DO NOT JUST HOPE YOU LIKE IT, I ALSO HOPE YOU ARE AWARE OF THE HUGE AMOUNT OF WORK WE HAVE ALL DONE. "
    "ENOUGH WAFFLE AND ON TO THE REASON FOR THIS TEXT... THE ENDLESS GREETINGS... GREETINGS GO OUT TO IN RANDOM ORDER... RAB C, JACOON, BUCK, SI, BARRIE, KIK, CHRIS, PROC, RUSS, RIK, ALEX NEEDS, LARRY MOOR, EMMA TEUTON - AN ALE DUE TO EACH ONE OF YOU - THE LIST GOES ON.... "
    "LOUISE GEE, MY GOOD SIS NICOLA, GEMMA, JENNY, HALEY, JO EVANS...  SARAH TELFORD.. PETER AND STEVEN, AMELIA, SARAH, ISY, SALLY, MARK, MATT WOODCOCK, PAUL NETTLETON, PETE BANFORD, TOM, DAVE BANHAM, DAVE KEENS, ALISON BOSSON, MIKE LINCOLN, KIRSTY, DARREN SAWYER... AND ALL UEA NORWICH PAST AND PRESENT. "
    "NICK ARNOTT, BAVERSTOCK, SARAH BANKS, SEANY, MARK STACEY, GILEY MILEY COOKSON [WHERE ARE YOU?], STEVE CLARIDGE, DEUBERT, JAMES GREENGRASS, SIMON ONG, KEVIN RUSSELL, CLARE WRIGHTON, ELTON [GIB IS JUST A DAY AWAY], NICOLA GOULD, STEVE LAST, KEN MALCOLN, RICO, ANDY CAKEBREAD, ROBBO, CARR, "
    "AND THE LITTLE ONE, CROFTY, SCOOPER, JASON STANTON [A CUP OF COFFEE], AARON SENNA, MIKE DORELL, IAN HOWIE, HELEN THAIN, ALEX FOREST-HAY, LEE HAZELWOOD, VICKY ARNOLD, GUY SIMMONS, SHIN, VAL TAYLOR.... IF I FORGOT YOU I AM SORRY... BUT SLEEP IS DUE TO ME... AND I HAVE A DREAM TO LIVE...";

/******************************************************************************/

inline void reset_scrolling_tooltip(void)
{
    tooltip_scroll_offset = 0;
    tooltip_scroll_timer = 25;
    set_flag_byte(&tool_tip_box.flags,TTip_NeedReset,false);
}

inline void set_gui_tooltip_box(int bxtype,long stridx)
{
  set_flag_byte(&tool_tip_box.flags,TTip_Visible,true);
  if ((stridx > 0) && (stridx < STRINGS_MAX))
    strncpy(tool_tip_box.text, gui_strings[stridx], TOOLTIP_MAX_LEN);
  else
    strcpy(tool_tip_box.text, "n/a");
  tool_tip_box.pos_x = GetMouseX();
  tool_tip_box.pos_y = GetMouseY()+86;
  tool_tip_box.field_809 = bxtype;
}

inline void set_gui_tooltip_box_fmt(int bxtype,const char *format, ...)
{
  set_flag_byte(&tool_tip_box.flags,TTip_Visible,true);
  va_list val;
  va_start(val, format);
  vsprintf(tool_tip_box.text, format, val);
  va_end(val);
  tool_tip_box.pos_x = GetMouseX();
  tool_tip_box.pos_y = GetMouseY()+86;
  tool_tip_box.field_809 = bxtype;
}

inline TbBool update_gui_tooltip_target(void *target)
{
  if (target != tool_tip_box.target)
  {
    help_tip_time = 0;
    tool_tip_box.target = target;
    set_flag_byte(&tool_tip_box.flags,TTip_NeedReset,true);
    return true;
  }
  return false;
}

inline void clear_gui_tooltip_target(void)
{
  help_tip_time = 0;
  tool_tip_box.target = NULL;
  set_flag_byte(&tool_tip_box.flags,TTip_NeedReset,true);
}

inline void clear_gui_tooltip_button(void)
{
  tool_tip_time = 0;
  tool_tip_box.gbutton = NULL;
}

TbBool setup_trap_tooltips(struct Coord3d *pos)
{
  struct Thing *thing;
  struct PlayerInfo *player;
  SYNCDBG(18,"Starting");
  thing = get_trap_for_slab_position(map_to_slab[pos->x.stl.num],map_to_slab[pos->y.stl.num]);;
  if (thing_is_invalid(thing)) return false;
  player = get_my_player();
  if ((thing->byte_18 == 0) && (player->id_number != thing->owner))
    return false;
  update_gui_tooltip_target(thing);
  if ((help_tip_time > 20) || (player->work_state == 12))
  {
    set_gui_tooltip_box(4,trap_data[thing->model%MANUFCTR_TYPES_COUNT].name_stridx);
  } else
  {
    help_tip_time++;
  }
  return true;
}

TbBool setup_object_tooltips(struct Coord3d *pos)
{
  struct Thing *thing;
  struct Objects *objdat;
  struct CreatureData *crdata;
  struct PlayerInfo *player;
  long i;
  SYNCDBG(18,"Starting");
  player = get_my_player();
  // Find a special to show tooltip for
  thing = thing_get(player->thing_under_hand);
  if (thing_is_invalid(thing) || !thing_is_special(thing))
    thing = get_special_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    update_gui_tooltip_target(thing);
    set_gui_tooltip_box(5,specials_text[box_thing_to_special(thing)]);
    return true;
  }
  // Find a spellbook to show tooltip for
  thing = get_spellbook_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    update_gui_tooltip_target(thing);
    i = book_thing_to_magic(thing);
    set_gui_tooltip_box(5,get_power_description_strindex(i));
    return true;
  }
  // Find a workshop crate to show tooltip for
  thing = get_crate_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    update_gui_tooltip_target(thing);
    if (get_workshop_object_class_for_thing(thing) == 8)
      i = trap_data[box_thing_to_door_or_trap(thing)].name_stridx;
    else
      i = door_names[box_thing_to_door_or_trap(thing)];
    set_gui_tooltip_box(5,i);
    return true;
  }
  if (!settings.tooltips_on)
    return false;
  // Find a hero gate/creature lair to show tooltip for
  thing = get_nearest_object_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    if (thing->model == 49)
    {
      update_gui_tooltip_target(thing);
      if ( (help_tip_time > 20) || (player->work_state == 12))
      {
        set_gui_tooltip_box(5,545); // Hero Gate tooltip
      } else
      {
        help_tip_time++;
      }
      return true;
    }
    objdat = get_objects_data_for_thing(thing);
    if (objdat->field_13)
    {
      update_gui_tooltip_target(thing);
      if ( (help_tip_time > 20) || (player->work_state == 12))
      {
        crdata = creature_data_get(objdat->field_13);
        set_gui_tooltip_box_fmt(5,"%s %s", gui_strings[crdata->namestr_idx%STRINGS_MAX], gui_strings[609]); // (creature) Lair
      } else
      {
        help_tip_time++;
      }
      return true;
    }
  }
  return false;
}

short setup_land_tooltips(struct Coord3d *pos)
{
  struct PlayerInfo *player;
  struct SlabMap *slb;
  struct SlabAttr *slbattr;
  long skind;
  SYNCDBG(18,"Starting");
  if (!settings.tooltips_on)
    return false;
  slb = get_slabmap_for_subtile(pos->x.stl.num, pos->y.stl.num);
  skind = slb->slab;
  slbattr = get_slab_kind_attrs(skind);
  if (slbattr->tooltip_idx == 201)
    return false;
  update_gui_tooltip_target((void *)skind);
  player = get_my_player();
  if ((help_tip_time > 20) || (player->work_state == 12))
  {
    set_gui_tooltip_box(2,slbattr->tooltip_idx);
  } else
  {
    help_tip_time++;
  }
  return true;
}

short setup_room_tooltips(struct Coord3d *pos)
{
  struct PlayerInfo *player;
  struct Room *room;
  int stridx;
  SYNCDBG(18,"Starting");
  if (!settings.tooltips_on)
    return false;
  room = subtile_room_get(pos->x.stl.num, pos->y.stl.num);
  if (room_is_invalid(room))
    return false;
  stridx = room_data[room->kind].msg1str_idx;
  if (stridx == 201)
    return false;
  update_gui_tooltip_target(room);
  player = get_my_player();
  if ( (help_tip_time > 20) || (player->work_state == 12) )
  {
    set_gui_tooltip_box(1,stridx);
  } else
  {
    help_tip_time++;
  }
  return true;
}

short setup_scrolling_tooltips(struct Coord3d *mappos)
{
  short shown;
  SYNCDBG(18,"Starting");
  shown = false;
  if (!shown)
    shown = setup_trap_tooltips(mappos);
  if (!shown)
    shown = setup_object_tooltips(mappos);
  if (!shown)
    shown = setup_land_tooltips(mappos);
  if (!shown)
    shown = setup_room_tooltips(mappos);
  if (!shown)
  {
    clear_gui_tooltip_target();
  }
  return shown;
}

void setup_gui_tooltip(struct GuiButton *gbtn)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct CreatureData *crdata;
  char *text;
  long i,k;
  if (gbtn->tooltip_id == 201)
    return;
  if (!settings.tooltips_on)
    return;
  dungeon = get_my_dungeon();
  set_flag_byte(&tool_tip_box.flags,TTip_Visible,true);
  i = gbtn->tooltip_id;
  if ((i >= 0) && (i < STRINGS_MAX))
    text = gui_strings[i];
  else
    text = lbEmptyString;
  if ((i == 456) || (i == 455))
  {
    if (tool_tip_box.gbutton != NULL)
        k = (long)tool_tip_box.gbutton->field_33;
    else
        k = -1;
    player = get_player(k);
    if (player->field_15[0] != '\0')
      sprintf(tool_tip_box.text, "%s: %s", text, player->field_15);
    else
      sprintf(tool_tip_box.text, "%s", text);
  } else
  if ((i == 658) && (dungeon->chickens_sacrificed > 16))
  {
    strncpy(tool_tip_box.text, jtytext, TOOLTIP_MAX_LEN);
  } else
  if (i == 733)
  {
    if (gbtn->field_1B > 0)
      k = breed_activities[(top_of_breed_list+gbtn->field_1B)%CREATURE_TYPES_COUNT];
    else
      k = 23;
    crdata = creature_data_get(k);
    sprintf(tool_tip_box.text, "%-6s: %s", gui_strings[crdata->namestr_idx], text);
  } else
  {
    strncpy(tool_tip_box.text, text, TOOLTIP_MAX_LEN);
  }
  if (gbtn != tool_tip_box.gbutton)
  {
    tool_tip_box.gbutton = gbtn;
    tool_tip_box.pos_x = GetMouseX();
    tool_tip_box.pos_y = GetMouseY()+86;
  }
}

TbBool gui_button_tooltip_update(int gbtn_idx)
{
  static TbBool doing_tooltip = false;
  struct PlayerInfo *player;
  struct GuiButton *gbtn;
  if ((gbtn_idx < 0) || (gbtn_idx >= ACTIVE_BUTTONS_COUNT))
  {
    clear_gui_tooltip_button();
    return false;
  }
  doing_tooltip = false;
  player = get_my_player();
  gbtn = &active_buttons[gbtn_idx];
  if ((get_active_menu(gbtn->gmenu_idx)->field_1 == 2) && ((gbtn->field_1B & 0x8000u) == 0))
  {
    if (tool_tip_box.gbutton == gbtn)
    {
        if ((tool_tip_time > 10) || (player->work_state == 12))
        {
          busy_doing_gui = 1;
          if (gbtn->field_13 != gui_area_text)
            setup_gui_tooltip(gbtn);
        } else
        {
          tool_tip_time++;
          doing_tooltip = true;
          busy_doing_gui = 1;
        }
    } else
    {
        tool_tip_time = 0;
        tool_tip_box.gbutton = gbtn;
        tool_tip_box.pos_x = GetMouseX();
        tool_tip_box.pos_y = GetMouseY()+86;
        tool_tip_box.field_809 = 0;
    }
    return true;
  }
  clear_gui_tooltip_button();
  return false;
}

TbBool input_gameplay_tooltips(TbBool gameplay_on)
{
  struct Coord3d mappos;
  struct PlayerInfo *player;
  TbBool shown;
  SYNCDBG(7,"Starting");
  shown = false;
  player = get_my_player();
  if ((gameplay_on) && (tool_tip_time == 0) && (!busy_doing_gui))
  {
    if (player->acamera == NULL)
    {
      ERRORLOG("No active camera");
      return false;
    }
    if (screen_to_map(player->acamera,GetMouseX(),GetMouseY(),&mappos))
    {
      if (subtile_revealed(mappos.x.stl.num,mappos.y.stl.num, player->id_number))
      {
        if (player->field_37 != 1)
          shown = setup_scrolling_tooltips(&mappos);
      }
    }
  }
  if (((tool_tip_box.flags & TTip_Visible) == 0) || ((tool_tip_box.flags & TTip_NeedReset) != 0))
    reset_scrolling_tooltip();
  SYNCDBG(19,"Finished");
  return shown;
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

void draw_tooltip_slab64k(char *tttext, long pos_x, long pos_y, long ttwidth, long ttheight, long viswidth)
{
  unsigned int flg_mem;
  long x,y;
  flg_mem = lbDisplay.DrawFlags;
  if (ttwidth > viswidth)
  {
    if (tooltip_scroll_timer <= 0)
    {
      if (-ttwidth >= tooltip_scroll_offset)
        tooltip_scroll_offset = viswidth;
      else
        tooltip_scroll_offset -= 4;
    } else
    {
      tooltip_scroll_timer--;
      if (tooltip_scroll_timer < 0)
        tooltip_scroll_offset = 0;
    }
  }
  if (tttext != NULL)
  {
    x = pos_x+26;
    lbDisplay.DrawFlags &= 0xFFBFu;
    y = pos_y - (ttheight+28);
    if (x > MyScreenWidth)
      x = MyScreenWidth;
    if (x < 6)
      x = 6;
    if (y > MyScreenHeight)
      y = MyScreenHeight;
    if (y < 4)
      y = 4;
    if (x+viswidth >= MyScreenWidth)
      x = MyScreenWidth-viswidth;
    if (y+ttheight >= MyScreenHeight)
      y = MyScreenHeight-ttheight;
    if (*tttext)
    {
      LbTextSetWindow(x/pixel_size, y/pixel_size, viswidth/pixel_size, ttheight/pixel_size);
      draw_slab64k(x, y, viswidth, ttheight);
      lbDisplay.DrawFlags = 0;
      LbTextDraw(tooltip_scroll_offset/pixel_size, -2/pixel_size, tttext);
    }
  }
  LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenHeight/pixel_size, MyScreenWidth/pixel_size);
  lbDisplay.DrawFlags = flg_mem;
}

long find_string_length_to_first_character(char *str, char fch)
{
  long i;
  for (i=0; str[i] != '\0'; i++)
  {
    if (str[i] == fch)
      break;
  }
  return i;
}

long find_string_width_to_first_character(char *str, char fch)
{
  long len;
  char text[TOOLTIP_MAX_LEN];
  len = find_string_length_to_first_character(str, fch);
  if (len >= sizeof(text))
  {
    WARNLOG("This bloody tooltip is too long");
    len = sizeof(text)-1;
  }
  strncpy(text, str, len);
  text[len] = '\0';
  return pixel_size * LbTextStringWidth(text);
}

void move_characters_forward_and_fill_empty_space(char *str,long move_pos,long shift,long clear_pos,long dst_pos,char fill_ch)
{
  long i = dst_pos;
  while (i >= move_pos)
  {
    str[i] = str[i-shift];
    i--;
  }
  while (i >= clear_pos)
  {
    str[i] = fill_ch;
    i--;
  }
}

long find_and_pad_string_width_to_first_character(char *str, char fch)
{
  long len,fill_len;
  len = find_string_length_to_first_character(str, fch);
  fill_len = 10-len;
  if (fill_len > 0)
  {
    // Moving characters after fch beyond the tooltip box size
    move_characters_forward_and_fill_empty_space(str,10,fill_len,len,strlen(str)+9,' ');
    move_characters_forward_and_fill_empty_space(str,fill_len/2,fill_len/2,0,9,' ');
  }
  return find_string_width_to_first_character(str, fch);
}

void draw_tooltip_at(long ttpos_x,long ttpos_y,char *tttext)
{
  struct PlayerInfo *player;
  unsigned int flg_mem;
  long hdwidth,ttwidth,ttheight;
  long pos_x,pos_y;
  if (tttext == NULL)
    return;
  flg_mem = lbDisplay.DrawFlags;
  lbDisplay.DrawFlags &= 0xFFBFu;
  hdwidth = find_and_pad_string_width_to_first_character(tttext, ':');
  ttwidth = pixel_size * LbTextStringWidth(tttext);
  ttheight = pixel_size * LbTextStringHeight(tttext);
  lbDisplay.DrawFlags = flg_mem;
  player = get_my_player();
  pos_x = ttpos_x;
  pos_y = ttpos_y;
  if (player->view_type == 4)
  {
    pos_y = GetMouseY() + 24;
    if (pos_y > MyScreenHeight-104)
      pos_y = MyScreenHeight - 104;
    if (pos_y < 0)
      pos_y = 0;
  }
  draw_tooltip_slab64k(tttext, pos_x, pos_y, ttwidth, ttheight, hdwidth);
}

void draw_tooltip(void)
{
  SYNCDBG(7,"Starting");
  lbFontPtr = winfont;
  if ((tool_tip_box.flags & TTip_Visible) != 0)
  {
    draw_tooltip_at(tool_tip_box.pos_x,tool_tip_box.pos_y,tool_tip_box.text);
  }
  LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  set_flag_byte(&tool_tip_box.flags,TTip_Visible,false);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
