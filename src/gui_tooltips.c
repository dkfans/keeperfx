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
#include "bflib_guibtns.h"

#include "kjm_input.h"
#include "keeperfx.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT void _DK_draw_tooltip(void);

/******************************************************************************/

/******************************************************************************/
inline void reset_scrolling_tooltip(void)
{
    tooltip_scroll_offset = 0;
    tooltip_scroll_timer = 25;
}

inline void set_gui_tooltip_box(int bxtype,long stridx)
{
  tool_tip_box.field_0 = 1;
  if ((stridx > 0) && (stridx < STRINGS_MAX))
    sprintf(tool_tip_box.text, "%s", gui_strings[stridx]);
  else
    sprintf(tool_tip_box.text, "%s", "n/a");
  tool_tip_box.pos_x = GetMouseX();
  tool_tip_box.pos_y = GetMouseY()+86;
  tool_tip_box.field_809 = bxtype;
}

inline void set_gui_tooltip_box_fmt(int bxtype,const char *format, ...)
{
  tool_tip_box.field_0 = 1;
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
    return true;
  }
  return false;
}

TbBool setup_trap_tooltips(struct Coord3d *pos)
{
  struct Thing *thing;
  struct PlayerInfo *player;
  thing = get_trap_for_slab_position(map_to_slab[pos->x.stl.num],map_to_slab[pos->y.stl.num]);;
  if (thing_is_invalid(thing)) return false;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if ((thing->byte_17.h == 0) && (player->field_2B != thing->owner))
    return false;
  update_gui_tooltip_target(thing);
  if ((help_tip_time > 20) || (player->field_453 == 12))
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
  char *text;
  struct Thing *thing;
  struct PlayerInfo *player;
  long i;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  // Find a special to show tooltip for
  thing = thing_get(player->field_35);
  if (thing_is_invalid(thing) || !thing_is_special(thing))
    thing = get_special_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    update_gui_tooltip_target(thing);
    set_gui_tooltip_box(5,specials_text[thing_to_special(thing)]);
    return true;
  }
  // Find a spellbook to show tooltip for
  thing = get_spellbook_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    update_gui_tooltip_target(thing);
    i = object_to_magic[thing->model];
    set_gui_tooltip_box(5,spell_data[i].field_D);
    return true;
  }
  // Find a workshop crate to show tooltip for
  thing = _DK_get_crate_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    update_gui_tooltip_target(thing);
    if (workshop_object_class[thing->model%OBJECT_TYPES_COUNT] == 8)
      i = trap_data[object_to_door_or_trap[thing->model%OBJECT_TYPES_COUNT]].name_stridx;
    else
      i = door_names[object_to_door_or_trap[thing->model%OBJECT_TYPES_COUNT]];
    set_gui_tooltip_box(5,i);
    return true;
  }
  if (!settings.tooltips_on)
    return false;
  // Find a hero gate/creature lair to show tooltip for
  thing = _DK_get_nearest_object_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    if (thing->model == 49)
    {
      update_gui_tooltip_target(thing);
      if ( (help_tip_time > 20) || (player->field_453 == 12))
      {
        set_gui_tooltip_box(5,545); // Hero Gate tooltip
      } else
      {
        help_tip_time++;
      }
      return true;
    }
    if (_DK_objects[thing->model].field_13)
    {
      update_gui_tooltip_target(thing);
      if ( (help_tip_time > 20) || (player->field_453 == 12))
      {
        i = creature_data[thing->model%CREATURE_TYPES_COUNT].field_3;
        set_gui_tooltip_box_fmt(5,"%s %s", gui_strings[i%STRINGS_MAX], gui_strings[609]); // (creature) Lair
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
  if (!settings.tooltips_on)
    return false;
  int slab_idx = map_to_slab[pos->x.stl.num] + map_tiles_x*map_to_slab[pos->y.stl.num];
  int attridx = game.slabmap[slab_idx].slab;
  int stridx = slab_attrs[attridx].field_0;
  if (stridx==201)
    return false;
  if ((void *)attridx != tool_tip_box.target)
  {
    help_tip_time = 0;
    tool_tip_box.target = (void *)attridx;
  }
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ((help_tip_time > 20) || (player->field_453 == 12))
  {
    tool_tip_box.field_0 = 1;
    sprintf(tool_tip_box.text, "%s", gui_strings[stridx%STRINGS_MAX]);
    tool_tip_box.field_809 = 2;
    tool_tip_box.pos_x = GetMouseX();
    tool_tip_box.pos_y = GetMouseY() + 86;
  } else
  {
    help_tip_time++;
  }
  return true;
}

short setup_room_tooltips(struct Coord3d *pos)
{
  struct PlayerInfo *player;
  struct SlabMap *slb;
  struct Room *room;
  if (!settings.tooltips_on)
    return false;
  slb = get_slabmap_block(map_to_slab[pos->x.stl.num], map_to_slab[pos->y.stl.num]);
  room = &game.rooms[slb->room_index];
  if (room == NULL)
    return false;
  int stridx;
  stridx = room_data[room->kind].field_13;
  if (stridx == 201)
    return false;
  if (room != tool_tip_box.target)
  {
    help_tip_time = 0;
    tool_tip_box.target = room;
  }
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  int widener=0;
  if ( (help_tip_time > 20) || (player->field_453 == 12) )
  {
      if ( room->kind >= 2 )
      {
        if ( (room->kind<=14) || (room->kind==16) )
          widener = 0;
      }
      sprintf(tool_tip_box.text, "%s", gui_strings[stridx%STRINGS_MAX]);
      tool_tip_box.field_0 = 1;
      tool_tip_box.pos_x = GetMouseX();
      tool_tip_box.pos_y = GetMouseY() + 86 + 20*widener;
      tool_tip_box.field_809 = 1;
  } else
  {
    help_tip_time++;
  }
  return true;
}

short setup_scrolling_tooltips(struct Coord3d *mappos)
{
  static const char *func_name="setup_scrolling_tooltips";
  short shown;
#if (BFDEBUG_LEVEL > 17)
  LbSyncLog("%s: Starting\n", func_name);
#endif
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
    help_tip_time = 0;
    tool_tip_box.target = NULL;
  }
  return shown;
}

short input_gameplay_tooltips(short gameplay_on)
{
  static const char *func_name="input_gameplay_tooltips";
  struct Coord3d mappos;
  unsigned short bitval;
  struct PlayerInfo *player;
  struct Map *map;
  short shown;
#if (BFDEBUG_LEVEL > 7)
  LbSyncLog("%s: Starting\n", func_name);
#endif
  shown = false;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if ((gameplay_on) && (tool_tip_time == 0) && (!busy_doing_gui))
  {
    if (player->acamera == NULL)
      return false;
    if (screen_to_map(player->acamera,GetMouseX(),GetMouseY(),&mappos))
    {
      // Get the top four bits - player flags
      map = get_map_block(mappos.x.stl.num,mappos.y.stl.num);
      bitval = map->data >> 28;
      if ((1 << player->field_2B) & (bitval))
      {
        if (player->field_37 != 1)
          shown = setup_scrolling_tooltips(&mappos);
      }
    }
  }
  if (tool_tip_box.field_0 == 0)
    reset_scrolling_tooltip();
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

void draw_tooltip(void)
{
  static const char *func_name="draw_tooltip";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_draw_tooltip();
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
