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
#include "pre_inc.h"
#include "gui_tooltips.h"
#include "globals.h"
#include <stdarg.h>
#include "bflib_guibtns.h"
#include "bflib_sprfnt.h"

#include "gui_draw.h"
#include "kjm_input.h"
#include "gui_topmsg.h"
#include "gui_frontmenu.h"
#include "frontend.h"
#include "front_input.h"
#include "thing_objects.h"
#include "thing_traps.h"
#include "dungeon_data.h"
#include "config_strings.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "room_workshop.h"
#include "player_instances.h"
#include "config_players.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "local_camera.h"
#include "keeperfx.hpp"
#include "post_inc.h"
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char jtytext[] = "Jonty here   : ...I am writing this at 4am on Keepers last day. I look around the office and all I see are the tired pale faces of the Keeper team. This project has destroyed the health and social lives of each member, so I hope you like the game. "
    "Amazingly after sixteen hours a day, 7 days a week, for nearly 5 months we still do. This game has been written with a passion I am proud to be part of.... I do not just hope you like it, I also hope you are aware of the huge amount of work we have all done. "
    "Enough waffle and on to the reason for this text... The endless greetings... Greetings go out to in random order... Rab C, Jacoon, Buck, Si, Barrie, Kik, Chris, Proc, Russ, Rik, Alex Needs, Larry Moor, Emma Teuton - An ale due to each one of you - the list goes on.... "
    "Louise Gee, my good sis Nicola, Gemma, Jenny, Haley, Jo Evans...  Sarah Telford.. Peter and Steven, Amelia, Sarah, Isy, Sally, Mark, Matt Woodcock, Paul Nettleton, Pete Banford, Tom, Dave Banham, Dave Keens, Alison Bosson, Mike Lincoln, Kirsty, Darren Sawyer... and all UEA Norwich past and present. "
    "Nick Arnott, Baverstock, Sarah Banks, Seany, Mark Stacey, Giley Miley Cookson [where are you?], Steve Claridge, Deubert, James Greengrass, Simon Ong, Kevin Russell, Clare Wrighton, Elton [Gib is just a day away], Nicola Gould, Steve Last, Ken Malcoln, Rico, Andy Cakebread, Robbo, Carr, "
    "and the little one, Crofty, Scooper, Jason Stanton [a cup of coffee], Aaron Senna, Mike Dorell, Ian Howie, Helen Thain, Alex Forest-Hay, Lee Hazelwood, Vicky Arnold, Guy Simmons, Shin, Val Taylor.... If I forgot you I am sorry... but sleep is due to me... and I have a dream to live...";

/******************************************************************************/

float render_tooltip_scroll_offset; // Rendering float
float render_tooltip_scroll_timer; // Rendering float
struct ToolTipBox tool_tip_box;

/******************************************************************************/
static inline void reset_scrolling_tooltip(void)
{
    render_tooltip_scroll_offset = 0;
    render_tooltip_scroll_timer = 25.0;
    clear_flag(tool_tip_box.flags, TTip_NeedReset);
}

void set_gui_tooltip_box_fmt(int bxtype,const char *format, ...)
{
  set_flag(tool_tip_box.flags, TTip_Visible);
  va_list val;
  va_start(val, format);
  vsnprintf(tool_tip_box.text, TOOLTIP_MAX_LEN, format, val);
  va_end(val);
  if (bxtype != 0) {
      tool_tip_box.pos_x = GetMouseX();
      long y_offset = scale_ui_value(86);
      tool_tip_box.pos_y = GetMouseY() + y_offset;
  }
  tool_tip_box.box_type = bxtype;
}

static inline TbBool update_gui_tooltip_target(void *target)
{
    if (target != tool_tip_box.target)
    {
        help_tip_time = 0;
        tool_tip_box.target = target;
        set_flag(tool_tip_box.flags, TTip_NeedReset);
        return true;
    }
    return false;
}

static inline void clear_gui_tooltip_target(void)
{
    help_tip_time = 0;
    tool_tip_box.target = NULL;
    set_flag(tool_tip_box.flags, TTip_NeedReset);
}

static inline TbBool update_gui_tooltip_button(struct GuiButton *gbtn)
{
    if (gbtn != tool_tip_box.gbutton)
    {
        tool_tip_box.gbutton = gbtn;
        tool_tip_box.pos_x = GetMouseX();
        long y_offset = scale_ui_value(86);
        tool_tip_box.pos_y = GetMouseY() + y_offset;
        tool_tip_box.box_type = 0;
        return true;
    }
    return false;
}

static inline void clear_gui_tooltip_button(void)
{
    tool_tip_time = 0;
    tool_tip_box.gbutton = NULL;
}

TbBool cursor_moved_to_new_subtile(struct PlayerInfo *player)
{
    return ((player->cursor_subtile_x != player->previous_cursor_subtile_x) || (player->cursor_subtile_y != player->previous_cursor_subtile_y));
}

TbBool setup_trap_tooltips(struct Coord3d *pos)
{
    SYNCDBG(18,"Starting");
    // Traps searching is restricted to one subtile - otherwise we could lose tooltips for other objects.
    struct Thing* thing = get_trap_at_subtile_of_model_and_owned_by(pos->x.stl.num, pos->y.stl.num, -1, -1);
    //thing = get_trap_for_slab_position(subtile_slab(pos->x.stl.num),subtile_slab(pos->y.stl.num));
    if (thing_is_invalid(thing)) return false;
    struct PlayerInfo* player = get_my_player();
    if ((thing->trap.revealed == 0) && (player->id_number != thing->owner))
        return false;
    update_gui_tooltip_target(thing);
    if ((help_tip_time > 20) || (player->work_state == PSt_CreatrQuery))
    {
        struct TrapConfigStats* trapst = get_trap_model_stats(thing->model);
        set_gui_tooltip_box_fmt(4,"%s",get_string(trapst->name_stridx));
    } else
    {
        help_tip_time++;
    }
    return true;
}

TbBool setup_object_tooltips(struct Coord3d *pos)
{
    long i;
    SYNCDBG(18,"Starting");
    struct PlayerInfo* player = get_my_player();
    struct Thing* thing = thing_get(player->thing_under_hand);
    if (!thing_is_object(thing))
    {
        thing = get_nearest_object_with_tooltip_at_position(pos->x.stl.num, pos->y.stl.num,0);
    }
    struct ObjectConfigStats* objst;
    if (thing_is_invalid(thing))
    {
        if (!settings.tooltips_on)
        {
            return false;
        }
        thing = get_nearest_object_with_tooltip_at_position(pos->x.stl.num, pos->y.stl.num, 1);
    }
    if (!thing_is_invalid(thing))
    {
        update_gui_tooltip_target(thing);
        objst = get_object_model_stats(thing->model);
        if ((objst->tooltip_stridx >= 0) && (objst->tooltip_stridx != GUIStr_Empty))
        {
            if ((help_tip_time > 20) || (player->work_state == PSt_CreatrQuery))
            {
                set_gui_tooltip_box_fmt(5, "%s", get_string(objst->tooltip_stridx));
            }
            else
            {
                help_tip_time++;
            }
            return true;
        }
        if (objst->tooltip_stridx == -1)
        {
            if (thing_is_spellbook(thing))
            {
                i = book_thing_to_power_kind(thing);
                set_gui_tooltip_box_fmt(5, "%s", get_string(get_power_name_strindex(i)));
                return true;
            }
            else
            if (thing_is_special_box(thing))
            {
                if (thing_is_custom_special_box(thing))
                {
                    // TODO: get it from Map script
                    if (game.box_tooltip[thing->custom_box.box_kind][0] == 0)
                    {
                        i = box_thing_to_special(thing);
                        long strngindex = get_special_description_strindex(i);
                        if (strngindex != GUIStr_Empty)
                        {
                            set_gui_tooltip_box_fmt(5, "%s", get_string(strngindex));
                        }
                    }
                    else
                    {
                        set_gui_tooltip_box_fmt(5, "%s", game.box_tooltip[thing->custom_box.box_kind]);
                    }
                }
                else
                {
                    i = box_thing_to_special(thing);
                    set_gui_tooltip_box_fmt(5, "%s", get_string(get_special_description_strindex(i)));
                }
                return true;
            }
            else
            if (thing_is_workshop_crate(thing))
            {
                update_gui_tooltip_target(thing);
                if (crate_thing_to_workshop_item_class(thing) == TCls_Trap)
                {
                    struct TrapConfigStats* trapst = get_trap_model_stats(crate_thing_to_workshop_item_model(thing));
                    i = trapst->name_stridx;
                }
                else
                {
                    struct DoorConfigStats* doorst = get_door_model_stats(crate_thing_to_workshop_item_model(thing));
                    i = doorst->name_stridx;
                }
                set_gui_tooltip_box_fmt(5, "%s", get_string(i));
                return true;
            }
            else
            if (objst->related_creatr_model)
            {
                update_gui_tooltip_target(thing);
                if ((help_tip_time > 20) || (player->work_state == PSt_CreatrQuery))
                {
                    struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[objst->related_creatr_model];
                    const struct RoomConfigStats* roomst = get_room_kind_stats(RoK_LAIR);     //TODO use a separate string for creature lair object than for lair room
                    set_gui_tooltip_box_fmt(5, "%s %s", get_string(crconf->namestr_idx), get_string(roomst->name_stridx)); // (creature) Lair
                }
                else
                {
                    help_tip_time++;
                }
                return true;
            }
        }
    }
    return false;
}

short setup_land_tooltips(struct Coord3d *pos)
{
  SYNCDBG(18,"Starting");
  if (!settings.tooltips_on)
    return false;
  struct SlabMap* slb = get_slabmap_for_subtile(pos->x.stl.num, pos->y.stl.num);
  long skind = slb->kind;
  struct SlabConfigStats* slabst = get_slab_kind_stats(skind);
  if (slabst->tooltip_stridx == GUIStr_Empty)
    return false;
  update_gui_tooltip_target((void *)(uintptr_t)skind);
  struct PlayerInfo* player = get_my_player();
  struct Thing *handthing = thing_get(player->thing_under_hand);
  TbBool in_query_mode = (player->work_state == PSt_CreatrQuery || player->work_state == PSt_QueryAll);
  if (in_query_mode == false) {
      if (cursor_moved_to_new_subtile(player) || thing_exists(handthing)) {
          return false;
      }
      if (help_tip_time <= 50) {
          help_tip_time++;
          return true;
      }
  }
  set_gui_tooltip_box_fmt(2, "%s", get_string(slabst->tooltip_stridx));
  return true;
}

short setup_room_tooltips(struct Coord3d *pos)
{
  SYNCDBG(18,"Starting");
  if (!settings.tooltips_on)
    return false;
  struct Room* room = subtile_room_get(pos->x.stl.num, pos->y.stl.num);
  const struct RoomConfigStats* roomst = get_room_kind_stats(room->kind);
  if (room_is_invalid(room))
    return false;
  int stridx = roomst->name_stridx;
  if (stridx == GUIStr_Empty)
    return false;
  update_gui_tooltip_target(room);
  struct PlayerInfo* player = get_my_player();
  struct Thing *handthing = thing_get(player->thing_under_hand);

  TbBool in_query_mode = (player->work_state == PSt_CreatrQuery || player->work_state == PSt_QueryAll);
  if (in_query_mode == false) {
      if (cursor_moved_to_new_subtile(player) || thing_exists(handthing)) {
          return false;
      }
      if (help_tip_time <= 50) {
          help_tip_time++;
          return true;
      }
  }
  set_gui_tooltip_box_fmt(1,"%s",get_string(stridx));
  return true;
}

short setup_scrolling_tooltips(struct Coord3d *mappos)
{
  SYNCDBG(18,"Starting");
  short shown = false;
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

void setup_gui_tooltip(struct GuiButton* gbtn)
{
    long k;
    if (gbtn->tooltip_stridx == GUIStr_Empty)
        return;
    if (!settings.tooltips_on)
        return;
    struct Dungeon* dungeon = get_my_dungeon();
    long i = gbtn->tooltip_stridx;
    const char* text = get_string(i);
    if ((i == GUIStr_NumberOfCreaturesDesc) || (i == GUIStr_NumberOfRoomsDesc))
    {
        if (tool_tip_box.gbutton != NULL)
            k = tool_tip_box.gbutton->content.lval;
        else
            k = -1;
        struct PlayerInfo* player = get_player(k);
        if (player->player_name[0] != '\0')
            set_gui_tooltip_box_fmt(0, "%s: %s", text, player->player_name);
        else
            set_gui_tooltip_box_fmt(0, "%s", text);
    }
    else
    if ((i == get_power_description_strindex(PwrK_CHICKEN)) && (dungeon->chickens_sacrificed > 16)) // Chicken spell tooltip easter egg
    {
        set_gui_tooltip_box_fmt(0, "%s", jtytext);
    }
    else
    if (i == GUIStr_PickCreatrMostExpDesc)
    {
        k = gbtn->btype_value & LbBFeF_IntValueMask;
        if ((k > 0) && (top_of_breed_list + k < game.conf.crtr_conf.model_count))
            k = breed_activities[top_of_breed_list + k];
        else
            k = get_players_special_digger_model(my_player_number);
        if (k > 0)
        {
            struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[k];
            set_gui_tooltip_box_fmt(0, "%-6s: %s", get_string(crconf->namestr_idx), text);
        }
    }
    else
    {
        set_gui_tooltip_box_fmt(0, "%s", text);
    }
    update_gui_tooltip_button(gbtn);
}

TbBool gui_button_tooltip_update(int gbtn_idx)
{
  if ((gbtn_idx < 0) || (gbtn_idx >= ACTIVE_BUTTONS_COUNT))
  {
    clear_gui_tooltip_button();
    return false;
  }
  int tooltip_delay;
  struct PlayerInfo* player = get_my_player();
  struct GuiButton* gbtn = &active_buttons[gbtn_idx];
  if ((get_active_menu(gbtn->gmenu_idx)->visual_state == 2) && ((gbtn->btype_value & LbBFeF_NoTooltip) == 0))
  {
    if (tool_tip_box.gbutton == gbtn)
    {
        // Increase tooltip time if the tooltip has been shown before
        if (gbtn->has_shown_before == 2) {
            tooltip_delay = 40;
        } else {
            tooltip_delay = 10;
        }

        struct GuiMenu* gmnu = get_active_menu(gbtn->gmenu_idx);
        if (gmnu) {
            long menu_id = gmnu->ident;
            if (menu_id == GMnu_OPTIONS || menu_id == GMnu_VIDEO || menu_id == GMnu_SOUND ||
                menu_id == GMnu_AUTOPILOT) {
                tooltip_delay = 0;
            }
        }

        if ( (tool_tip_time > tooltip_delay) || (player->work_state == PSt_CreatrQuery) )
        {
          if (gbtn->has_shown_before == 0) {
            gbtn->has_shown_before = 1;
          }
          busy_doing_gui = 1;
          if (gbtn->draw_call != gui_area_text)
            setup_gui_tooltip(gbtn);
        } else
        {
          tool_tip_time++;
          busy_doing_gui = 1;
        }
    } else
    {
        clear_gui_tooltip_button();
        update_gui_tooltip_button(gbtn);
        if (gbtn->has_shown_before == 1) {
          gbtn->has_shown_before = 2;
        }
    }
    return true;
  }
  clear_gui_tooltip_button();
  return false;
}

TbBool input_gameplay_tooltips(TbBool gameplay_on)
{
    SYNCDBG(17,"Starting");
    TbBool shown = false;
    struct PlayerInfo* player = get_my_player();
    if ((gameplay_on) && (tool_tip_time == 0) && (!busy_doing_gui))
    {
        if (player->acamera == NULL)
        {
            ERRORLOG("No active camera");
            return false;
        }
        struct Coord3d mappos;
        if (screen_to_map(get_local_camera(player->acamera), GetMouseX(), GetMouseY(), &mappos))
        {
            if (subtile_revealed(mappos.x.stl.num,mappos.y.stl.num, player->id_number))
            {
                if (player->view_mode != PVM_CreatureView)
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
  show_onscreen_msg(2*game_num_fps, "Tooltips %s", statstr);
  save_settings();
}

void draw_tooltip_slab64k(char *tttext, long pos_x, long pos_y, long ttwidth, long ttheight, long viswidth)
{
    unsigned int flg_mem = lbDisplay.DrawFlags;
    if (ttwidth > viswidth)
    {
        if (render_tooltip_scroll_timer <= 0)
        {
            if (-ttwidth >= render_tooltip_scroll_offset)
              render_tooltip_scroll_offset = viswidth;
            else
              render_tooltip_scroll_offset -= ((MyScreenHeight >= 400) ? 4.0 : 2.0) * game.delta_time;
        } else
        {
            render_tooltip_scroll_timer -= 1.0 * game.delta_time;
            if (render_tooltip_scroll_timer < 0)
              render_tooltip_scroll_offset = 0;
        }
    }
    if (tttext != NULL)
    {
        long x = pos_x + scale_ui_value(26);
        lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
        long y = pos_y - scale_ui_value(ttheight + 28);
        if (x > MyScreenWidth)
          x = MyScreenWidth;
        if (x < scale_ui_value(6))
          x = scale_ui_value(6);
        if (y > MyScreenHeight)
          y = MyScreenHeight;
        if (y < scale_ui_value(4))
          y = scale_ui_value(4);
        if (x + scale_ui_value(viswidth) >= MyScreenWidth)
          x = MyScreenWidth - scale_ui_value(viswidth);
        if (y + scale_ui_value(ttheight) >= MyScreenHeight)
          y = MyScreenHeight - scale_ui_value(ttheight);
        if (tttext[0] != '\0')
        {
            draw_slab64k(x, y, units_per_pixel_ui, scale_ui_value_lofi(viswidth), scale_ui_value_lofi(ttheight));
            lbDisplay.DrawFlags = 0;
            int tx_units_per_px, tx, ty;
            if ( (MyScreenHeight < 400) && (dbc_language > 0) )
            {
                LbTextSetWindow(x, y, scale_ui_value(viswidth * 2), scale_ui_value(ttheight * 2));
                tx_units_per_px = scale_value_by_horizontal_resolution(32);
                tx = scale_ui_value(render_tooltip_scroll_offset * 2);
                ty = -scale_ui_value(2);
            }
            else
            {
                LbTextSetWindow(x, y, scale_ui_value_lofi(viswidth), scale_ui_value_lofi(ttheight));
                tx_units_per_px = calculate_relative_upp(22, units_per_pixel_ui, LbTextLineHeight());
                tx = scale_ui_value_lofi(render_tooltip_scroll_offset);
                ty = -scale_ui_value_lofi(2);
            }
            LbTextDrawResized(tx, ty, tx_units_per_px, tttext);
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
  char text[TOOLTIP_MAX_LEN];
  long len = find_string_length_to_first_character(str, fch) + 1;
  if (len >= sizeof(text))
  {
    WARNLOG("This bloody tooltip is too long");
    len = sizeof(text)-1;
  }
  snprintf(text, len, "%s", str);
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
    long len = find_string_length_to_first_character(str, fch);
    long fill_len = 10 - len;
    if (fill_len > 0)
    {
        // Moving characters after fch beyond the tooltip box size
        move_characters_forward_and_fill_empty_space(str, 10, fill_len, len, strlen(str) + 9, ' ');
        move_characters_forward_and_fill_empty_space(str, fill_len / 2, fill_len / 2, 0, 9, ' ');
  }
  return find_string_width_to_first_character(str, fch);
}

void draw_tooltip_at(long ttpos_x,long ttpos_y,char *tttext)
{
  if (tttext == NULL)
    return;
  unsigned int flg_mem = lbDisplay.DrawFlags;
  lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
  long hdwidth = find_and_pad_string_width_to_first_character(tttext, ':');
  long ttwidth = LbTextStringWidth(tttext);
  long ttheight = LbTextStringHeight(tttext);
  lbDisplay.DrawFlags = flg_mem;
  struct PlayerInfo* player = get_my_player();
  long pos_x = ttpos_x;
  long pos_y = ttpos_y;
  if (player->view_type == PVT_MapScreen)
  {
      pos_y = GetMouseY() + scale_ui_value(24);
      if (pos_y > MyScreenHeight - scale_ui_value(104))
          pos_y = MyScreenHeight - scale_ui_value(104);
      if (pos_y < 0)
          pos_y = 0;
  }
  draw_tooltip_slab64k(tttext, pos_x, pos_y, ttwidth, ttheight, hdwidth);
}

void draw_tooltip(void)
{
    SYNCDBG(7,"Starting");
    LbTextSetFont(winfont);
    if ((tool_tip_box.flags & TTip_Visible) != 0)
    {
      if (tool_tip_box.box_type != 0) {
          tool_tip_box.pos_x = GetMouseX();
          long y_offset = scale_ui_value(86);
          tool_tip_box.pos_y = GetMouseY() + y_offset;
        }
        draw_tooltip_at(tool_tip_box.pos_x,tool_tip_box.pos_y,tool_tip_box.text);
    }
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
