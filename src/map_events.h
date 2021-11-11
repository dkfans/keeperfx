/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_events.h
 *     Header file for map_events.c.
 * @par Purpose:
 *     Map events support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_MAP_EVENTS_H
#define DK_MAP_EVENTS_H

#include "globals.h"
#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define EVENT_BUTTONS_COUNT    12
#define EVENT_KIND_COUNT       34
#define EVENTS_COUNT          100
#define INVALID_EVENT &game.event[0]

enum EventKinds {
    EvKind_Nothing = 0,
    EvKind_HeartAttacked,
    EvKind_EnemyFight,
    EvKind_Objective,
    EvKind_Breach,
    EvKind_NewRoomResrch, //5
    EvKind_NewCreature,
    EvKind_NewSpellResrch,
    EvKind_NewTrap,
    EvKind_NewDoor,
    EvKind_CreatrScavenged, //10
    EvKind_TreasureRoomFull,
    EvKind_CreaturePayday,
    EvKind_AreaDiscovered,
    EvKind_SpellPickedUp,
    EvKind_RoomTakenOver, //15
    EvKind_CreatrIsAnnoyed,
    EvKind_NoMoreLivingSet,
    EvKind_AlarmTriggered,
    EvKind_RoomUnderAttack,
    EvKind_NeedTreasureRoom,//20
    EvKind_Information,
    EvKind_RoomLost,
    EvKind_CreatrHungry,
    EvKind_TrapCrateFound,
    EvKind_DoorCrateFound, //25
    EvKind_DnSpecialFound,
    EvKind_QuickInformation,
    EvKind_FriendlyFight,
    EvKind_WorkRoomUnreachable,
    EvKind_StorageRoomUnreachable, //30
    EvKind_PrisonerStarving,
    EvKind_TorturedHurt,
    EvKind_EnemyDoor,
};

enum EventFlags {
    EvF_Exists       = 0x0001,
    EvF_BtnFirstFall = 0x0002, /*< Informs whether the button is falling for a first time. */
};

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct Dungeon;
struct PlayerInfo;

struct Event { // sizeof=0x15
    unsigned char flags;
    unsigned char index;
    long mappos_x;
    long mappos_y;
    unsigned char owner;
    unsigned char kind;
    long target;
    /** Button lifespan, decreased over time. When reaches 0, the button disappears. */
    unsigned long lifespan_turns;
    unsigned char falling_button; // Old way - make it unused when only EvF_BtnFirstFall is used
};

struct Bookmark { // sizeof = 3
  unsigned char x;
  unsigned char y;
  unsigned char flags;
};

#pragma pack()
/******************************************************************************/
extern struct EventTypeInfo event_button_info[EVENT_KIND_COUNT];
/******************************************************************************/
struct Event *get_event_of_type_for_player(EventKind evkind, PlayerNumber plyr_idx);
struct Event *get_event_of_target_and_type_for_player(long target, EventKind evkind, PlayerNumber plyr_idx);
struct Event *get_event_nearby_of_type_for_player(MapCoord map_x, MapCoord map_y, long max_dist, EventKind evkind, PlayerNumber plyr_idx);

TbBool event_is_invalid(const struct Event *event);
EventIndex event_create_event_or_update_nearby_existing_event(MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long target);
EventIndex event_create_event_or_update_same_target_existing_event(MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long target);
EventIndex event_create_event_or_update_old_event(MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long target);
void event_initialise_all(void);
long event_move_player_towards_event(struct PlayerInfo *player, long var);
struct Event *event_create_event(MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long msg_id);
struct Event *event_allocate_free_event_structure(void);
void event_initialise_event(struct Event *event, MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long msg_id);
void event_add_to_event_buttons_list(struct Event *event, struct Dungeon *dungeon);
void event_add_to_event_buttons_list_or_replace_button(struct Event *event, struct Dungeon *dungeon);
void event_update_on_battle_removal(void);
void event_delete_event(long plridx, EventIndex evidx);
void event_update_last_use(struct Event *event);
void go_on_then_activate_the_event_box(PlayerNumber plyr_idx, EventIndex evidx);
int event_get_button_index(const struct Dungeon *dungeon, EventIndex evidx);
void clear_events(void);
void remove_events_thing_is_attached_to(struct Thing *thing);
struct Thing *event_is_attached_to_thing(EventIndex evidx);
void maintain_my_event_list(struct Dungeon *dungeon);
void kill_oldest_my_event(struct Dungeon *dungeon);
void event_kill_all_players_events(long plyr_idx);
void event_process_events(void);
void update_all_events(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
