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
#include "keeperfx.h"

#include "kjm_input.h"

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

short setup_trap_tooltips(struct Coord3d *pos)
{
    struct Thing *thing;
    struct PlayerInfo *player;
    int stridx;
    thing = get_trap_for_slab_position(map_to_slab[pos->x.stl.num],map_to_slab[pos->y.stl.num]);;
    if (thing == NULL) return false;
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    if ((thing->byte_17.h == 0) && (player->field_2B != thing->owner))
      return false;
    if (thing != tool_tip_box.target)
    {
      help_tip_time = 0;
      tool_tip_box.target = thing;
    }
    if ((help_tip_time > 20) || (player->field_453 == 12))
    {
      tool_tip_box.field_0 = 1;
      stridx = trap_data[thing->model].field_C;
      sprintf(tool_tip_box.text, "%s", gui_strings[stridx%STRINGS_MAX]);
      tool_tip_box.pos_x = GetMouseX();
      tool_tip_box.pos_y = GetMouseY()+86;
      tool_tip_box.field_809 = 4;
    } else
    {
      help_tip_time++;
    }
    return true;
}

short setup_object_tooltips(struct Coord3d *pos)
{
  char *text;
  struct Thing *thing;
  struct PlayerInfo *player;
  long i;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  thing = thing_get(player->field_35);
  if (thing_is_invalid(thing))
      thing = NULL;
  if (thing != NULL)
  {
    if (!thing_is_special(thing))
      thing = NULL;
  }
  if (thing == NULL)
    thing = _DK_get_special_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing != NULL)
  {
    if ((void *)thing != tool_tip_box.target)
    {
      help_tip_time = 0;
      tool_tip_box.target = thing;
    }
    int stridx = specials_text[object_to_special[thing->model]];
    sprintf(tool_tip_box.text, "%s", gui_strings[stridx%STRINGS_MAX]);
    tool_tip_box.field_0 = 1;
    tool_tip_box.field_809 = 5;
    tool_tip_box.pos_x = GetMouseX();
    tool_tip_box.pos_y = GetMouseY() + 86;
    return true;
  }
  thing = _DK_get_spellbook_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    if ( (void *)thing != tool_tip_box.target )
    {
      help_tip_time = 0;
      tool_tip_box.target = (void *)thing;
    }
    int stridx;
    stridx = 0;
    i = object_to_magic[thing->model];
    if ((i >= 0) && (i <= SPELL_TYPES_COUNT))
      stridx = spell_data[i].field_D;
    if (stridx > 0)
    {
      sprintf(tool_tip_box.text,"%s",gui_strings[stridx%STRINGS_MAX]);
      tool_tip_box.field_0 = 1;
      tool_tip_box.field_809 = 5;
      tool_tip_box.pos_x = GetMouseX();
      tool_tip_box.pos_y = GetMouseY() + 86;
    }
    return 1;
  }
  thing = _DK_get_crate_at_position(pos->x.stl.num, pos->y.stl.num);
  if ( thing )
  {
    if ( (void *)thing != tool_tip_box.target )
    {
      help_tip_time = 0;
      tool_tip_box.target = (void *)thing;
    }
    tool_tip_box.field_0 = 1;
    int objidx = thing->model;
    int stridx;
    if ( _DK_workshop_object_class[objidx] == 8 )
      stridx = trap_data[_DK_object_to_door_or_trap[objidx]].field_C;
    else
      stridx = door_names[_DK_object_to_door_or_trap[objidx]];
    sprintf(tool_tip_box.text, "%s", gui_strings[stridx%STRINGS_MAX]);
    tool_tip_box.pos_x = GetMouseX();
    tool_tip_box.pos_y = GetMouseY() + 86;
    tool_tip_box.field_809 = 5;
    return true;
  }
  if (!settings.tooltips_on)
    return false;
  thing = _DK_get_nearest_object_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    int objidx = thing->model;
    int crtridx;
    if (objidx == 49)
    {
      text=buf_sprintf("%s", gui_strings[545]); // Hero Gate tooltip
    } else
    if (crtridx = _DK_objects[objidx].field_13)
    {
      int stridx=creature_data[crtridx].field_3;
      text=buf_sprintf("%s %s", gui_strings[stridx%STRINGS_MAX], gui_strings[609]); // (creature) Lair
    } else
    {
      return 0;
    }

    if ( (void *)thing != tool_tip_box.target )
    {
      help_tip_time = 0;
      tool_tip_box.target = (void *)thing;
    }
    if ( (help_tip_time > 20) || (player->field_453 == 12))
    {
      tool_tip_box.field_0 = 1;
      strcpy(tool_tip_box.text, text);
      tool_tip_box.pos_x = GetMouseX();
      tool_tip_box.field_809 = 5;
      tool_tip_box.pos_y = GetMouseY() + 86;
    } else
    {
      help_tip_time++;
    }
    return true;
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
  if (!settings.tooltips_on)
    return false;
  int slab_idx = map_to_slab[pos->x.stl.num] + map_tiles_x*map_to_slab[pos->y.stl.num];
  struct Room *room;
  room = &game.rooms[game.slabmap[slab_idx].room_index];
  if (room==NULL)
    return false;
  int stridx;
  stridx=room_data[room->kind].field_13;
  if (stridx == 201)
    return false;
  if ( room != tool_tip_box.target )
  {
    help_tip_time = 0;
    tool_tip_box.target = room;
  }
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
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
  short shown;
#if (BFDEBUG_LEVEL > 7)
  LbSyncLog("%s: Starting\n", func_name);
#endif
  shown = false;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if ((gameplay_on) && (tool_tip_time == 0) && (!busy_doing_gui))
  {
    int bblock_x;
    int bblock_y;
    if (player->acamera == NULL)
      return false;
    if (screen_to_map(player->acamera,GetMouseX(),GetMouseY(),&mappos))
    {
      bblock_x = mappos.x.stl.num;
      bblock_y = mappos.y.stl.num;
      // Get the top four bits - player flags
      bitval = (game.map[bblock_x+bblock_y*(map_subtiles_x+1)].data) >> 28;
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
