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
#define EVENT_KIND_COUNT       36
#define EVENTS_COUNT          200
#define INVALID_EVENT &game.event[0]

enum EventKinds {
    EvKind_Nothing = 0,
    EvKind_HeartAttacked,                  // ComputerKeeper: AttkHrt1 [event4] --> sends 99% of creatures to defend heart
    EvKind_EnemyFight,                     // ComputerKeeper: Fight1/Fight2 [event7/8] --> engages in battle
    EvKind_Objective,
    EvKind_Breach,                         // ComputerKeeper: DnBreach [event1] --> sends 75% of creatures to defend against breach
    EvKind_NewRoomResrch,
    EvKind_NewCreature,
    EvKind_NewSpellResrch,
    EvKind_NewTrap,
    EvKind_NewDoor,
    EvKind_CreatrScavenged,
    EvKind_TreasureRoomFull,               // ComputerKeeper: RomFTrsr [event5] --> builds treasury room
    EvKind_CreaturePayday,                 // ComputerKeeper: PayDay1 [event14] --> prepares gold for payday
    EvKind_AreaDiscovered,
    EvKind_SpellPickedUp,
    EvKind_RoomTakenOver,
    EvKind_CreatrIsAnnoyed,
    EvKind_NoMoreLivingSet,                // ComputerKeeper: RomFLair [event6] --> builds lair room
    EvKind_AlarmTriggered,
    EvKind_RoomUnderAttack,                // ComputerKeeper: AttkRom1/2 [event2/3] --> sends 75% of creatures to defend room
    EvKind_NeedTreasureRoom,
    EvKind_Information,
    EvKind_RoomLost,                       // ComputerKeeper: RoomLost [event15] --> rebuilds lost room
    EvKind_CreatrHungry,
    EvKind_TrapCrateFound,
    EvKind_DoorCrateFound,
    EvKind_DnSpecialFound,
    EvKind_QuickInformation,
    EvKind_FriendlyFight,
    EvKind_WorkRoomUnreachable,
    EvKind_StorageRoomUnreachable,
    EvKind_PrisonerStarving,               // ComputerKeeper: MoanPris1/2/3 [event16/17/18] --> heals/tortures prisoners
    EvKind_TorturedHurt,                   // ComputerKeeper: SaveTort1 [event19] --> saves low-health tortured creatures
    EvKind_EnemyDoor,                      // ComputerKeeper: DoorAtck1 [event20] --> attacks enemy doors
    EvKind_SecretDoorDiscovered,
    EvKind_SecretDoorSpotted,
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

struct Event {
    unsigned char flags;
    EventIndex index;
    long mappos_x;
    long mappos_y;
    unsigned char owner;
    unsigned char kind;
    long target;
    /** Button lifespan, decreased over time. When reaches 0, the button disappears. */
    unsigned long lifespan_turns;
};

struct Bookmark {
  MapSubtlCoord x;
  MapSubtlCoord y;
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
TbBool event_exists(const struct Event* event);
EventIndex event_create_event_or_update_nearby_existing_event(MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long target);
EventIndex event_create_event_or_update_same_target_existing_event(MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long target);
EventIndex event_create_event_or_update_old_event(MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long target);
void event_initialise_all(void);
long event_move_player_towards_event(struct PlayerInfo *player, long event_idx);
struct Event *event_create_event(MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long msg_id);
struct Event *event_allocate_free_event_structure(void);
void event_initialise_event(struct Event *event, MapCoord map_x, MapCoord map_y, EventKind evkind, unsigned char dngn_id, long msg_id);
void event_add_to_event_buttons_list_or_replace_button(struct Event *event, struct Dungeon *dungeon);
void event_update_on_battle_removal(BattleIndex battle_idx);
void event_delete_event(long plridx, EventIndex evidx);
void event_update_last_use(struct Event *event);
void go_on_then_activate_the_event_box(PlayerNumber plyr_idx, EventIndex evidx);
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
