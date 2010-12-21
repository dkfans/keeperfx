/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_events.c
 *     Map events support functions.
 * @par Purpose:
 *     Functions to create and maintain events placed on map.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "map_events.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"

#include "thing_objects.h"
#include "thing_doors.h"
#include "thing_traps.h"
#include "config_campaigns.h"
#include "config_creature.h"
#include "config_trapdoor.h"
#include "gui_frontmenu.h"
#include "frontend.h"
#include "room_workshop.h"

#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT long _DK_event_create_event_or_update_nearby_existing_event(long map_x, long map_y, unsigned char a3, unsigned char dngn_id, long msg_id);
DLLIMPORT void _DK_event_initialise_all(void);
DLLIMPORT long _DK_event_move_player_towards_event(struct PlayerInfo *player, long var);
DLLIMPORT struct Event *_DK_event_create_event(long map_x, long map_y, unsigned char a3, unsigned char dngn_id, long msg_id);
DLLIMPORT void _DK_go_on_then_activate_the_event_box(long plridx, long val);

/******************************************************************************/
long event_create_event_or_update_nearby_existing_event(MapCoord map_x, MapCoord map_y, unsigned char a3, unsigned char dngn_id, long msg_id)
{
    return _DK_event_create_event_or_update_nearby_existing_event(map_x, map_y, a3, dngn_id, msg_id);
}

void event_initialise_all(void)
{
  _DK_event_initialise_all();
}

long event_move_player_towards_event(struct PlayerInfo *player, long var)
{
  return _DK_event_move_player_towards_event(player,var);
}

struct Event *event_create_event(long map_x, long map_y, unsigned char evkind, unsigned char dngn_id, long msg_id)
{
  struct Dungeon *dungeon;
  struct Event *event;
  long i,k;
//  return _DK_event_create_event(map_x, map_y, evkind, dngn_id, msg_id);
  if (dngn_id >= game.neutral_player_num)
    return NULL;
  if (evkind >= 28)
  {
    ERRORLOG("Illegal Event kind to be created");
    return NULL;
  }
  dungeon = get_dungeon(dngn_id);
  i = dungeon->field_13B4[evkind%EVENT_KIND_COUNT];
  if (i != 0)
  {
    k = event_button_info[evkind].field_C;
    if ((k != 0) && (k+i >= game.play_gameturn))
    {
      return NULL;
    }
  }
  event = event_allocate_free_event_structure();
  if (event == NULL)
    return NULL;
  event_initialise_event(event, map_x, map_y, evkind, dngn_id, msg_id);
  event_add_to_event_list(event, dungeon);
  return event;
}

struct Event *event_allocate_free_event_structure(void)
{
  struct Event *event;
  long i;
  for (i=1; i < EVENTS_COUNT; i++)
  {
    event = &game.event[i];
    if ((event->field_0 & 0x01) == 0)
    {
      event->field_0 |= 0x01;
      event->field_1 = i;
      return event;
    }
  }
  return NULL;
}

void event_initialise_event(struct Event *event, long map_x, long map_y, unsigned char evkind, unsigned char dngn_id, long msg_id)
{
  event->mappos_x = map_x;
  event->mappos_y = map_y;
  event->kind = evkind;
  event->owner = dngn_id;
  event->birth_turn = event_button_info[evkind].field_8;
  event->target = msg_id;
  event->field_14 = 1;
}

void event_delete_event_structure(long ev_idx)
{
  LbMemorySet(&game.event[ev_idx], 0, sizeof(struct Event));
}

void event_delete_event(long plyr_idx, long ev_idx)
{
  struct Dungeon *dungeon;
  struct Event *event;
  long i,k;
//  _DK_event_delete_event(plridx, num);
  event = &game.event[ev_idx];
  dungeon = get_dungeon(plyr_idx);
  dungeon->field_13B4[event->kind%EVENT_KIND_COUNT] = game.play_gameturn;
  for (i=0; i <= EVENT_BUTTONS_COUNT; i++)
  {
    k = dungeon->field_13A7[i];
    if (k == ev_idx)
    {
      turn_off_event_box_if_necessary(plyr_idx, k);
      dungeon->field_13A7[i] = 0;
      break;
    }
  }
  event_delete_event_structure(ev_idx);
}

void event_add_to_event_list(struct Event *event, struct Dungeon *dungeon)
{
  long i,k;
  for (i=EVENT_BUTTONS_COUNT; i > 0; i--)
  {
    k = dungeon->field_13A7[i];
    if (k == 0)
    {
      if (dungeon->field_E9F != event->owner)
      {
        ERRORLOG("Illegal my_event player allocation");
      }
      dungeon->field_13A7[i] = event->field_1;
      break;
    }
  }
  if (i == 0)
  {
    kill_oldest_my_event(dungeon);
    dungeon->field_13A7[EVENT_BUTTONS_COUNT] = event->field_1;
  }
}

void event_reset_scroll_window(void)
{
  game.evntbox_scroll_window.start_y = 0;
  game.evntbox_scroll_window.action = 0;
  game.evntbox_scroll_window.text_height = 0;
  game.evntbox_scroll_window.window_height = 0;
}

void go_on_then_activate_the_event_box(long plridx, long evidx)
{
  struct Dungeon *dungeon;
  struct CreatureData *crdata;
  struct DoorConfigStats *doorst;
  struct TrapConfigStats *trapst;
  struct RoomData *rdata;
  struct Event *event;
  struct Thing *thing;
  char *text;
  int i,k;
  short other_off;
  dungeon = get_players_num_dungeon(plridx);
  event = &game.event[evidx];
  SYNCDBG(6,"Starting for event kind %d",event->kind);
  dungeon->field_1173 = evidx;
  if (plridx == my_player_number)
  {
    i = event_button_info[event->kind].field_6;
    if (i != 201)
      strcpy(game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
  }
  if (event->kind == 2)
    dungeon->field_1174 = find_first_battle_of_mine(plridx);
  if (plridx == my_player_number)
  {
    other_off = 0;
    switch (event->kind)
    {
    case 1:
    case 4:
        other_off = 1;
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 2:
        turn_off_menu(GMnu_TEXT_INFO);
        turn_on_menu(GMnu_BATTLE);
        break;
    case 3:
        strcpy(game.evntbox_scroll_window.text, game.evntbox_text_objective);
        for (i=EVENT_BUTTONS_COUNT; i >= 0; i--)
        {
          k = dungeon->field_13A7[i];
          if (game.event[k%EVENTS_COUNT].kind == 3)
          {
            other_off = 1;
            turn_on_menu(GMnu_TEXT_INFO);
            new_objective = 0;
            break;
          }
        }
        break;
    case 5:
        other_off = 1;
        rdata = room_data_get_for_kind(event->target);
        i = rdata->msg1str_idx;
        text = buf_sprintf("%s:\n%s",game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 6:
        other_off = 1;
        thing = thing_get(event->target);
        // If thing is invalid - leave the message without it.
        // Otherwise, put creature type in it.
        if (!thing_is_invalid(thing))
        {
          crdata = creature_data_get_from_thing(thing);
          i = crdata->namestr_idx;
          text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
          strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        }
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 7:
        other_off = 1;
        i = get_power_description_strindex(event->target);
        text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 8:
        other_off = 1;
        trapst = get_trap_stats(event->target);
        i = trapst->name_stridx;
        text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 9:
        other_off = 1;
        doorst = get_door_stats(event->target);
        i = doorst->name_stridx;
        text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 10: // Scavenge detected
        other_off = 1;
        thing = thing_get(event->target);
        // If thing is invalid - leave the message without it.
        // Otherwise, put creature type in it.
        if (!thing_is_invalid(thing))
        {
          crdata = creature_data_get_from_thing(thing);
          i = crdata->namestr_idx;
          text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
          strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        }
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 11:
    case 13:
        other_off = 1;
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 12:
        other_off = 1;
        text = buf_sprintf("%s:\n %d", game.evntbox_scroll_window.text, event->target);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 14:
        other_off = 1;
        thing = thing_get(event->target);
        if (thing_is_invalid(thing))
          break;
        i = get_power_description_strindex(book_thing_to_magic(thing));
        text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 15:
        other_off = 1;
        rdata = room_data_get_for_kind(event->target);
        i = rdata->msg1str_idx;
        text = buf_sprintf("%s:\n%s",game.evntbox_scroll_window.text,gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 16:
        other_off = 1;
        thing = thing_get(event->target);
        // If thing is invalid - leave the message without it.
        // Otherwise, put creature type in it.
        if (!thing_is_invalid(thing))
        {
          crdata = creature_data_get_from_thing(thing);
          i = crdata->namestr_idx;
          text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
          strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        }
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 17:
    case 18:
    case 19:
    case 20:
    case 22:
    case 23:
        other_off = 1;
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 21:
        i = (long)event->target;
        if (i < 0)
        {
          i = -i;
          event->target = i;
        }
        strncpy(game.evntbox_text_buffer, campaign.strings[i%STRINGS_MAX], MESSAGE_TEXT_LEN-1);
        strncpy(game.evntbox_scroll_window.text, game.evntbox_text_buffer, MESSAGE_TEXT_LEN-1);
        other_off = 1;
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    case 24:
        other_off = 1;
        thing = thing_get(event->target);
        if (thing_is_invalid(thing))
          break;
        trapst = get_trap_stats(box_thing_to_door_or_trap(thing));
        i = trapst->name_stridx;
        text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
      case 25:
        other_off = 1;
        thing = thing_get(event->target);
        if (thing_is_invalid(thing))
          break;
        doorst = get_door_stats(box_thing_to_door_or_trap(thing));
        i = doorst->name_stridx;
        text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
      case 26:
        other_off = 1;
        thing = thing_get(event->target);
        if (thing_is_invalid(thing))
          break;
        i = specials_text[box_thing_to_special(thing)];
        text = buf_sprintf("%s:\n%s", game.evntbox_scroll_window.text, gui_strings[i%STRINGS_MAX]);
        strncpy(game.evntbox_scroll_window.text,text,MESSAGE_TEXT_LEN-1);
        turn_on_menu(GMnu_TEXT_INFO);
        break;
      case 27:
        i = (long)event->target;
        if (i < 0)
        {
          i = -i;
          event->target = i;
        }
        strncpy(game.evntbox_text_buffer, gameadd.quick_messages[i%QUICK_MESSAGES_COUNT], MESSAGE_TEXT_LEN-1);
        strncpy(game.evntbox_scroll_window.text, game.evntbox_text_buffer, MESSAGE_TEXT_LEN-1);
        other_off = 1;
        turn_on_menu(GMnu_TEXT_INFO);
        break;
    default:
        ERRORLOG("Undefined event kind: %d", (int)event->kind);
        break;
    }
    event_reset_scroll_window();
    if (other_off)
    {
      turn_off_menu(34);
      turn_off_menu(27);
      turn_off_menu(28);
      turn_off_menu(29);
    }
  }
  SYNCDBG(8,"Finished");
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
