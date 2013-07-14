/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_computer.c
 *     Computer player definitions and activities.
 * @par Purpose:
 *     Defines a computer player control variables and events/checks/processes
 *      functions.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "player_computer.h"

#include <limits.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_memory.h"
#include "bflib_math.h"

#include "config.h"
#include "config_compp.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "magic.h"
#include "thing_traps.h"
#include "player_complookup.h"
#include "power_hand.h"
#include "room_data.h"
#include "game_legacy.h"
#include "skirmish_ai.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

ComputerType computer_assist_types[] = { 6, 7, 8, 9 };
unsigned short computer_types[] = { 201, 201, 201, 201, 201, 201, 729, 730, 731, 732 };

/******************************************************************************/
DLLIMPORT void _DK_setup_computer_players2(void);
DLLIMPORT void _DK_process_computer_player2(unsigned long plridx);
DLLIMPORT void _DK_setup_a_computer_player(unsigned long a1, long a2);
DLLIMPORT struct ComputerTask *_DK_computer_setup_build_room(struct Computer2 *comp, unsigned short a2, long a3, long a4, long a5);
DLLIMPORT void _DK_process_computer_players2(void);
DLLIMPORT void _DK_computer_check_events(struct Computer2 *comp);
DLLIMPORT long _DK_process_checks(struct Computer2 *comp);
DLLIMPORT long _DK_process_tasks(struct Computer2 *comp);
DLLIMPORT long _DK_get_computer_money_less_cost(struct Computer2 *comp);
DLLIMPORT long _DK_count_creatures_availiable_for_fight(struct Computer2 *comp, struct Coord3d *pos);
DLLIMPORT struct ComputerTask *_DK_is_there_an_attack_task(struct Computer2 *comp);
DLLIMPORT void _DK_get_opponent(struct Computer2 *comp, struct THate *hate);
DLLIMPORT long _DK_setup_computer_attack(struct Computer2 *comp, struct ComputerProcess *process, struct Coord3d *pos, long a4);
DLLIMPORT long _DK_count_creatures_for_defend_pickup(struct Computer2 *comp);
DLLIMPORT long _DK_computer_find_non_solid_block(struct Computer2 *comp, struct Coord3d *pos);
DLLIMPORT long _DK_computer_able_to_use_magic(struct Computer2 *comp, long a2, long a3, long a4);
DLLIMPORT long _DK_check_call_to_arms(struct Computer2 *comp);
DLLIMPORT long _DK_computer_finds_nearest_room_to_gold(struct Computer2 *comp, struct Coord3d *pos, struct GoldLookup **gldlook);

/******************************************************************************/
// Function definition needed to compare pointers - remove pending
long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *process);
long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *process);

/******************************************************************************/
DLLIMPORT extern struct ComputerProcess _DK_BuildAllRooms3x3;
#define BuildAllRooms3x3 _DK_BuildAllRooms3x3
DLLIMPORT extern struct ComputerProcess _DK_BuildAllRooms4x4;
#define BuildAllRooms4x4 _DK_BuildAllRooms4x4
DLLIMPORT extern struct ComputerProcess _DK_BuildPrisonRoom;
#define BuildPrisonRoom _DK_BuildPrisonRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTortureRoom;
#define BuildTortureRoom _DK_BuildTortureRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildScavengerRoom;
#define BuildScavengerRoom _DK_BuildScavengerRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTempleRoom;
#define BuildTempleRoom _DK_BuildTempleRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildGraveyardRoom;
#define BuildGraveyardRoom _DK_BuildGraveyardRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildBarrackRoom;
#define BuildBarrackRoom _DK_BuildBarrackRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTreasureRoom;
#define BuildTreasureRoom _DK_BuildTreasureRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildResearchRoom;
#define BuildResearchRoom _DK_BuildResearchRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildHatcheryRoom;
#define BuildHatcheryRoom _DK_BuildHatcheryRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildLairRoom;
#define BuildLairRoom _DK_BuildLairRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildTrainingRoom;
#define BuildTrainingRoom _DK_BuildTrainingRoom
DLLIMPORT extern struct ComputerProcess _DK_BuildWorkshopRoom;
#define BuildWorkshopRoom _DK_BuildWorkshopRoom
DLLIMPORT extern struct ComputerProcess _DK_DigToEntrance;
#define DigToEntrance _DK_DigToEntrance
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldForMoney;
#define DigToGoldForMoney _DK_DigToGoldForMoney
DLLIMPORT extern struct ComputerProcess _DK_BuildTreasureRoom4x4;
#define BuildTreasureRoom4x4 _DK_BuildTreasureRoom4x4
DLLIMPORT extern struct ComputerProcess _DK_BuildLairRoom4x4;
#define BuildLairRoom4x4 _DK_BuildLairRoom4x4
DLLIMPORT extern struct ComputerProcess _DK_DigToCloseGoldForMoney;
#define DigToCloseGoldForMoney _DK_DigToCloseGoldForMoney
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldGreedy;
#define DigToGoldGreedy _DK_DigToGoldGreedy
DLLIMPORT extern struct ComputerProcess _DK_DigToGoldGreedy2;
#define DigToGoldGreedy2 _DK_DigToGoldGreedy2
DLLIMPORT extern struct ComputerProcess _DK_ComputerSightOfEvil;
#define ComputerSightOfEvil _DK_ComputerSightOfEvil
DLLIMPORT extern struct ComputerProcess _DK_ComputerSightOfEvilScare;
#define ComputerSightOfEvilScare _DK_ComputerSightOfEvilScare
DLLIMPORT extern struct ComputerProcess _DK_ComputerAttack1;
#define ComputerAttack1 _DK_ComputerAttack1
DLLIMPORT extern struct ComputerProcess _DK_ComputerSafeAttack;
#define ComputerSafeAttack _DK_ComputerSafeAttack
/******************************************************************************/

struct ValidRooms valid_rooms_to_build[] = {
  {RoK_TREASURE,  &BuildTreasureRoom},
  {RoK_LAIR,      &BuildLairRoom},
  {RoK_GARDEN,    &BuildHatcheryRoom},
  {RoK_LIBRARY,   &BuildResearchRoom},
  {RoK_TRAINING,  &BuildTrainingRoom},
  {RoK_WORKSHOP,  &BuildWorkshopRoom},
  {RoK_SCAVENGER, &BuildScavengerRoom},
  {RoK_PRISON,    &BuildPrisonRoom},
  {RoK_TEMPLE,    &BuildTempleRoom},
  {RoK_TORTURE,   &BuildTortureRoom},
  {RoK_GRAVEYARD, &BuildGraveyardRoom},
  {RoK_BARRACKS,  &BuildBarrackRoom},
  {-1,            NULL},
};

struct ComputerProcessMnemonic computer_process_config_list[] = {
  {"Unused", NULL,},
  {"", &BuildAllRooms3x3,},
  {"", &BuildAllRooms4x4,},
  {"", &BuildPrisonRoom,},
  {"", &BuildTortureRoom,},
  {"", &BuildScavengerRoom,},
  {"", &BuildTempleRoom,},
  {"", &BuildGraveyardRoom,},
  {"", &BuildBarrackRoom,},
  {"", &BuildTreasureRoom,},
  {"", &BuildResearchRoom,},
  {"", &BuildHatcheryRoom,},
  {"", &BuildLairRoom,},
  {"", &BuildTrainingRoom,},
  {"", &BuildWorkshopRoom,},
  {"", &DigToEntrance,},
  {"", &DigToGoldForMoney,},
  {"", &BuildTreasureRoom4x4,},
  {"", &BuildLairRoom4x4,},
  {"", &DigToCloseGoldForMoney,},
  {"", &DigToGoldGreedy,},
  {"", &DigToGoldGreedy2,},
  {"", &ComputerSightOfEvil,},
  {"", &ComputerSightOfEvilScare,},
  {"", &ComputerAttack1,},
  {"", &ComputerSafeAttack,},
};

char const event_pay_day_text[] = "EVENT PAY DAY";
char const event_save_imps_text[] = "EVENT SAVE IMPS";
char const event_check_room_text[] = "EVENT CHECK ROOMS FULL";
char const event_magic_foe_text[] = "EVENT MAGIC FOE";
char const event_check_fighters_text[] = "EVENT CHECK FIGHTERS";
char const event_fight_test_text[] = "EVENT FIGHT TEST";
char const event_fight_text[] = "EVENT FIGHT";
char const event_living_space_full_text[] = "EVENT LIVING SPACE FULL";
char const event_treasure_room_full_text[] = "EVENT TREASURE ROOM FULL";
char const event_heart_under_attack_text[] = "EVENT HEART UNDER ATTACK";
char const event_room_attack_text[] = "EVENT ROOM ATTACK";
char const event_dungeon_breach_text[] = "EVENT DUNGEON BREACH";

char const check_money_text[] = "CHECK MONEY";
char const check_expand_room_text[] = "CHECK EXPAND ROOM";
char const check_avail_trap_text[] = "CHECK AVAILIABLE TRAP";
char const check_neutral_places_text[] = "CHECK FOR NEUTRAL PLACES";
char const check_avail_door_text[] = "CHECK AVAILIABLE DOOR";
char const check_enemy_entrances_text[] = "CHECK FOR ENEMY ENTRANCES";
char const check_for_slap_imp_text[] = "CHECK FOR SLAP IMP";
char const check_for_speed_up_text[] = "CHECK FOR SPEED UP";
char const check_for_quick_attack_text[] = "CHECK FOR QUICK ATTACK";
char const check_to_pretty_text[] = "CHECK TO PRETTY";
char const check_enough_imps_text[] = "CHECK FOR ENOUGH IMPS";
char const move_creature_to_train_text[] = "MOVE CREATURE TO TRAINING";
char const move_creature_to_best_text[] = "MOVE CREATURE TO BEST ROOM";
char const computer_check_hates_text[] = "COMPUTER CHECK HATES";

/******************************************************************************/
long get_computer_money_less_cost(struct Computer2 *comp)
{
  return _DK_get_computer_money_less_cost(comp);
}

long count_creatures_for_pickup(struct Computer2 *comp, struct Coord3d *pos, struct Room *room, long a4)
{
    //TODO COMPUTER_EVENT_BREACH needs this function; may be also used somewhere else - not sure
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    int i;
    SYNCDBG(8,"Starting");
    //return _DK_make_all_players_creatures_angry(plyr_idx);
    int stl_x, stl_y;
    stl_x = 0;
    stl_y = 0;
    if (pos != NULL)
    {
      stl_x = pos->x.stl.num;
      stl_y = pos->y.stl.num;
    }
    int count;
    count = 0;
    k = 0;
    i = comp->dungeon->creatr_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing))
        {
            if ((thing->active_state != CrSt_CreatureUnconscious) && (cctrl->combat_flags == 0))
            {
                if (!creature_is_called_to_arms(thing) && !creature_is_being_dropped(thing))
                {
                    struct StateInfo *stati;
                    int n;
                    if (thing->active_state == CrSt_MoveToPosition)
                        n = thing->continue_state;
                    else
                        n = thing->active_state;
                    stati = get_thing_state_info_num(n);
                    if ((stati->state_type != 1) || a4 )
                    {
                        if (room_is_invalid(room))
                        {
                            if (abs(thing->mappos.x.stl.num - stl_x) + abs(thing->mappos.y.stl.num - stl_y) < 2 )
                              continue;
                        } else
                        {
                            //This needs finishing
                            //if ( !person_will_do_job_for_room(thing, room) )
                              continue;
                        }
                        count++;
                    }
                }
            }
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(19,"Finished");
    return count;
}

struct ComputerTask * able_to_build_room_at_task(struct Computer2 *comp, RoomKind rkind, long width_slabs, long height_slabs, long area, long a6)
{
    struct ComputerTask *ctask;
    long i;
    unsigned long k;
    i = comp->task_idx;
    k = 0;
    while (i != 0)
    {
        ctask = get_computer_task(i);
        if (computer_task_invalid(ctask))
        {
            ERRORLOG("Jump to invalid task detected");
            break;
        }
        i = ctask->next_task;
        // Per-task code
        if (((ctask->flags & ComTsk_Unkn0001) != 0) && ((ctask->flags & ComTsk_Unkn0002) != 0))
        {
            unsigned short max_f7c;
            struct ComputerTask *roomtask;
            max_f7c = ((ctask->field_7C)&0xffff) / 2;
            if ( max_f7c <= ((ctask->field_7C >> 16)&0xffff) / 2 )
              max_f7c = ((ctask->field_7C >> 16)&0xffff) / 2;
            roomtask = able_to_build_room(comp, &ctask->pos_64, rkind, width_slabs, height_slabs, area + max_f7c + 1, a6);
            if (roomtask != NULL) {
                return roomtask;
            }
        }
        // Per-task code ends
        k++;
        if (k > COMPUTER_TASKS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping tasks list");
            break;
        }
    }
    return NULL;
}

/**
 * Checks if we are able to build a room starting out from already build room of given kind.
 * @param comp
 * @param rkind The room kind to be built.
 * @param look_kind The room kind which we'd like to search as starting point.
 * @param width_slabs
 * @param height_slabs
 * @param area
 * @param a6
 * @return
 */
struct ComputerTask * able_to_build_room_from_room(struct Computer2 *comp, RoomKind rkind, RoomKind look_kind, long width_slabs, long height_slabs, long area, long a6)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    long i;
    unsigned long k;
    i = dungeon->room_kind[look_kind];
    k = 0;
    while (i != 0)
    {
        struct Room *room;
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        struct Coord3d pos;
        struct ComputerTask *roomtask;
        pos.x.val = subtile_coord_center(room->central_stl_x);
        pos.y.val = subtile_coord_center(room->central_stl_y);
        pos.z.val = 256;
        roomtask = able_to_build_room(comp, &pos, rkind, width_slabs, height_slabs, area, a6);
        if (roomtask != NULL) {
            return roomtask;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return NULL;
}

struct ComputerTask *computer_setup_build_room(struct Computer2 *comp, RoomKind rkind, long width_slabs, long height_slabs, long look_randstart)
{
    struct Dungeon *dungeon;
    //return _DK_computer_setup_build_room(comp, rkind, a3, a4, a5);
    dungeon = comp->dungeon;
    long max_slabs;
    long area_min,area_max;
    long i;
    max_slabs = height_slabs;
    if (max_slabs < width_slabs)
        max_slabs = width_slabs;
    area_min = (max_slabs + 1) / 2 + 1;
    area_max = area_min / 3 + 2 * area_min;
    if (rkind == RoK_LAIR)
    {
        if (width_slabs*height_slabs < dungeon->max_creatures_attracted)
        {
            i = LbSqrL(dungeon->max_creatures_attracted);
            width_slabs = i + 1;
            height_slabs = i + 1;
        }
    }
    const long arr_length = sizeof(look_through_rooms)/sizeof(look_through_rooms[0]);
    long area;
    for (area=area_min; area < area_max; area++)
    {
        long aparam;
        for (aparam=1; aparam >= 0; aparam--)
        {
            unsigned int lookidx;
            lookidx = look_randstart;
            if (look_randstart < 0)
            {
                lookidx = ACTION_RANDOM(arr_length);
            }
            for (i=0; i < arr_length; i++)
            {
                struct ComputerTask *roomtask;
                int look_kind;
                look_kind = look_through_rooms[lookidx];
                if (look_kind == RoK_UNKN17)
                {
                    roomtask = able_to_build_room_at_task(comp, rkind, width_slabs, height_slabs, area, aparam);
                } else
                {
                    roomtask = able_to_build_room_from_room(comp, rkind, look_kind, width_slabs, height_slabs, area, aparam);
                }
                if (roomtask != NULL) {
                    return roomtask;
                }
                lookidx = (lookidx + 1) % arr_length;
            }
        }
    }
    SYNCLOG("Unable to find a place for %s sized %dx%d",(int)width_slabs, (int)height_slabs, room_code_name(rkind));
    return NULL;
}

void setup_dig_to(struct ComputerDig *cdig, const struct Coord3d startpos, const struct Coord3d endpos)
{
    memset(cdig,0,sizeof(struct ComputerDig));
    cdig->pos_gold.x.val = startpos.x.val;
    cdig->pos_gold.y.val = startpos.y.val;
    cdig->pos_gold.z.val = startpos.z.val;
    cdig->pos_E.x.val = startpos.x.val;
    cdig->pos_E.y.val = startpos.y.val;
    cdig->pos_E.z.val = startpos.z.val;
    cdig->pos_14.x.val = endpos.x.val;
    cdig->pos_14.y.val = endpos.y.val;
    cdig->pos_14.z.val = endpos.z.val;
    cdig->distance = LONG_MAX;
    cdig->subfield_2C = 1;
    cdig->pos_20.x.val = 0;
    cdig->pos_20.y.val = 0;
    cdig->pos_20.z.val = 0;
    cdig->subfield_54 = 0;
}

long computer_finds_nearest_room_to_gold_lookup(const struct Dungeon *dungeon, const struct GoldLookup *gldlook, struct Room **nearroom)
{
    struct Room *room;
    long rkind;
    long distance,min_distance;
    struct Coord3d gold_pos;
    *nearroom = INVALID_ROOM;
    gold_pos.x.val = 0;
    gold_pos.y.val = 0;
    gold_pos.z.val = 0;
    gold_pos.x.stl.num = gldlook->x_stl_num;
    gold_pos.y.stl.num = gldlook->y_stl_num;
    min_distance = LONG_MAX;
    distance = LONG_MAX;
    for (rkind=1; rkind < ROOM_TYPES_COUNT; rkind++)
    {
        room = find_room_nearest_to_position(dungeon->owner, rkind, &gold_pos, &distance);
        if (!room_is_invalid(room))
        {
            distance >>= 8; // Convert to subtiles
            // Decrease the value by gold area radius
            distance -= (gldlook->field_E >> 3);
            // We can accept longer distances if digging directly to treasure room
            if (room->kind == RoK_TREASURE)
                distance -= TREASURE_ROOM_PREFERENCE_WHILE_DIGGING_GOLD;
            if (min_distance > distance)
            {
                *nearroom = room;
                min_distance = distance;
            }
        }
    }
    return min_distance;
}

long computer_finds_nearest_task_to_gold(const struct Computer2 *comp, const struct GoldLookup *gldlook, struct ComputerTask ** near_task)
{
    struct Coord3d task_pos;
    long i;
    unsigned long k;
    struct ComputerTask *ctask;
    long distance,min_distance;
    long delta_x,delta_y;
    task_pos.x.val = 0;
    task_pos.y.val = 0;
    task_pos.z.val = 0;
    task_pos.x.stl.num = gldlook->x_stl_num;
    task_pos.y.stl.num = gldlook->y_stl_num;
    min_distance = LONG_MAX;
    i = comp->task_idx;
    k = 0;
    while (i != 0)
    {
        ctask = get_computer_task(i);
        if (computer_task_invalid(ctask))
        {
            ERRORLOG("Jump to invalid task detected");
            break;
        }
        i = ctask->next_task;
        // Per-task code
        if ( ((ctask->flags & ComTsk_Unkn0001) != 0) && ((ctask->flags & ComTsk_Unkn0002) != 0) )
        {
            delta_x = (long)ctask->pos_64.x.val - (long)task_pos.x.val;
            delta_y = (long)ctask->pos_64.y.val - (long)task_pos.y.val;
            distance = LbDiagonalLength(abs(delta_x), abs(delta_y));
            distance >>= 8; // Convert to subtiles
            distance -= (gldlook->field_E >> 3);
            if (min_distance > distance)
            {
                *near_task = ctask;
                min_distance = distance;
            }
        }
        // Per-task code ends
        k++;
        if (k > COMPUTER_TASKS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping tasks list");
            break;
        }
    }
    return min_distance;
}

/**
 * Finds nearest place to start digging gold from, and the target GoldLookup to be digged.
 *
 * @param comp Computer player which considers starting the digging.
 * @param pos Resurns position to start digging from.
 * @param gldlookref Returns reference to GoldLookup containing coords of the place to dig to.
 * @return Lower or equal 0 on failure, positive if gold digging is ready to go.
 */
long computer_finds_nearest_room_to_gold(struct Computer2 *comp, struct Coord3d *pos, struct GoldLookup **gldlookref)
{
    struct Dungeon *dungeon;
    struct GoldLookup *gldlook;
    struct GoldLookup *gldlooksel;
    struct Coord3d locpos;
    struct Coord3d *spos;
    long dig_distance;
    long lookups_checked;
    long i;
    //return _DK_computer_finds_nearest_room_to_gold(comp, pos, gldlookref);
    dungeon = comp->dungeon;
    gldlooksel = NULL;
    *gldlookref = gldlooksel;
    locpos.x.val = 0;
    locpos.y.val = 0;
    locpos.z.val = 0;
    spos = &locpos;
    lookups_checked = 0;
    dig_distance = LONG_MAX;
    for (i=0; i < GOLD_LOOKUP_COUNT; i++)
    {
        gldlook = &game.gold_lookup[i];
        if ((gldlook->field_0 & 0x01) == 0)
            continue;
        if ((gldlook->plyrfield_1[dungeon->owner] & 0x03) != 0)
            continue;
        SYNCDBG(8,"Searching for place to reach (%d,%d)",(int)gldlook->x_stl_num,(int)gldlook->y_stl_num);
        lookups_checked++;
        struct Room *room = INVALID_ROOM;
        long new_dist;
        new_dist = computer_finds_nearest_room_to_gold_lookup(dungeon, gldlook, &room);
        if (dig_distance > new_dist)
        {
            locpos.x.val = (room->central_stl_x << 8);
            locpos.y.val = (room->central_stl_y << 8);
            locpos.z.val = (1 << 8);
            spos = &locpos;
            dig_distance = new_dist;
            gldlooksel = gldlook;
            SYNCDBG(8,"Distance from room at (%d,%d) is %d",(int)spos->x.stl.num,(int)spos->y.stl.num,(int)dig_distance);
        }
        struct ComputerTask *ctask = NULL;
        new_dist = computer_finds_nearest_task_to_gold(comp, gldlook, &ctask);
        if (dig_distance > new_dist)
        {
            spos = &ctask->pos_64;
            dig_distance = new_dist;
            gldlooksel = gldlook;
            SYNCDBG(8,"Distance from task at (%d,%d) is %d",(int)spos->x.stl.num,(int)spos->y.stl.num,(int)dig_distance);
        }
    }
    if (gldlooksel == NULL)
    {
        SYNCDBG(8,"Checked %d lookups, but no gold to dig found",(int)lookups_checked);
        if (lookups_checked == 0)
        {
            return -1;
        } else
        {
            return 0;
        }
    }
    SYNCDBG(8,"Best digging start to reach (%d,%d) is on subtile (%d,%d); distance is %d",(int)gldlooksel->x_stl_num,(int)gldlooksel->y_stl_num,(int)spos->x.stl.num,(int)spos->y.stl.num,(int)dig_distance);
    *gldlookref = gldlooksel;
    pos->x.val = spos->x.val;
    pos->y.val = spos->y.val;
    pos->z.val = spos->z.val;
    if (dig_distance < 1)
        dig_distance = 1;
    if (dig_distance > LONG_MAX)
        dig_distance = LONG_MAX;
    return dig_distance;
}

long count_creatures_availiable_for_fight(struct Computer2 *comp, struct Coord3d *pos)
{
    return _DK_count_creatures_availiable_for_fight(comp, pos);
}

struct ComputerTask *is_there_an_attack_task(struct Computer2 *comp)
{
    return _DK_is_there_an_attack_task(comp);
}

void get_opponent(struct Computer2 *comp, struct THate *hate)
{
    _DK_get_opponent(comp, hate);
}

long setup_computer_attack(struct Computer2 *comp, struct ComputerProcess *process, struct Coord3d *pos, long a4)
{
    return _DK_setup_computer_attack(comp, process, pos, a4);
}

long count_entrances(const struct Computer2 *comp, PlayerNumber plyr_idx)
{
    const struct Room *room;
    const struct Dungeon *dungeon;
    long i;
    unsigned long k;
    long count;
    dungeon = comp->dungeon;
    count = 0;
    i = game.entrance_room_id;
    k = 0;
    while (i != 0)
    {
        room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_kind;
        // Per-room code
        if ((room->field_12[dungeon->owner] & 0x01) == 0)
        {
            if ((plyr_idx < 0) || (room->owner == plyr_idx))
                count++;
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return count;
}

long count_creatures_in_dungeon(const struct Dungeon *dungeon)
{
    return count_player_list_creatures_of_model(dungeon->creatr_list_start, 0);
}

long count_diggers_in_dungeon(const struct Dungeon *dungeon)
{
    return count_player_list_creatures_of_model(dungeon->digger_list_start, 0);
}

long buildable_traps_amount(struct Dungeon *dungeon, ThingModel trmodel)
{
    if ((trmodel < 1) || (trmodel >= TRAP_TYPES_COUNT))
        return 0;
    if (dungeon->trap_buildable[trmodel] > 0)
    {
        return dungeon->trap_amount[trmodel];
    }
    return 0;
}

/**
 * Retrieves count of different trap kinds of which given dungeon has at least given number.
 * @param dungeon
 * @param base_amount
 */
long get_number_of_trap_kinds_with_amount_at_least(struct Dungeon *dungeon, long base_amount)
{
    long i,kinds;
    kinds = 0;
    for (i=1; i < TRAP_TYPES_COUNT; i++)
    {
        if (buildable_traps_amount(dungeon, i) >= base_amount)
        {
            kinds++;
        }
    }
    return kinds;
}

/**
 * Retrieves count of different trap kinds of which given dungeon has at least given number.
 * @param dungeon
 * @param base_amount
 */
long get_nth_of_trap_kinds_with_amount_at_least(struct Dungeon *dungeon, long base_amount, long n)
{
    long i;
    for (i=1; i < TRAP_TYPES_COUNT; i++)
    {
        if (buildable_traps_amount(dungeon, i) >= base_amount)
        {
            if (n <= 0)
                return i;
            n--;
        }
    }
    return 0;
}

/**
 * Retrieves best kind of trap to place in dungeon.
 * @param dungeon The dungeon which owns the spare traps.
 * @param allow_last Accept kind of which only one box exists.
 * @param kind_preselect If possible, try to use given kind; only if it's not accessible, check others.
 */
long computer_choose_best_trap_kind_to_place(struct Dungeon *dungeon, long allow_last, ThingModel kind_preselect)
{
    long kinds_multiple,kinds_single;
    // If there are multiple buildable traps of preselected kind
    if ((kind_preselect > 0) && (buildable_traps_amount(dungeon, kind_preselect) >= 2))
        return kind_preselect;
    // No pre-selection - check if there are other multiple traps
    kinds_multiple = get_number_of_trap_kinds_with_amount_at_least(dungeon, 2);
    if (kinds_multiple > 0) {
        SYNCDBG(18,"Returning one of %d plentiful traps",(int)kinds_multiple);
        return get_nth_of_trap_kinds_with_amount_at_least(dungeon, 2, ACTION_RANDOM(kinds_multiple));
    }
    // If there are no multiple traps, and we're not allowing to spend last one
    if (!allow_last)
        return 0;
    // If there are buildable traps of preselected kind
    if ((kind_preselect > 0) && (buildable_traps_amount(dungeon, kind_preselect) >= 1))
        return kind_preselect;
    kinds_single = get_number_of_trap_kinds_with_amount_at_least(dungeon, 1);
    if (kinds_single > 0) {
        SYNCDBG(18,"Returning one of %d single traps",(int)kinds_single);
        return get_nth_of_trap_kinds_with_amount_at_least(dungeon, 1, ACTION_RANDOM(kinds_single));
    }
    return 0;
}

long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon *dungeon;
    long i;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_for_place_trap(comp, check);
    dungeon = comp->dungeon;
    long kind_chosen;
    kind_chosen = computer_choose_best_trap_kind_to_place(dungeon, check->param2, check->param3);
    if (kind_chosen <= 0)
        return 4;
    //TODO COMPUTER_AI Maybe we should prefer corridors when placing traps?
    for (i=0; i < COMPUTER_TRAP_LOC_COUNT; i++)
    {
        struct Coord3d *location;
        location = &comp->trap_locations[i];
        // Check if the entry has coords stored
        if ((location->x.val <= 0) && (location->y.val <= 0))
            continue;
        MapSlabCoord slb_x,slb_y;
        slb_x = subtile_slab(location->x.stl.num);
        slb_y = subtile_slab(location->y.stl.num);
        struct SlabMap *slb;
        slb = get_slabmap_block(slb_x,slb_y);
        if (slabmap_block_invalid(slb)) {
            ERRORLOG("Trap location contained off-map point (%d,%d)",(int)location->x.stl.num,(int)location->y.stl.num);
            location->x.val = 0;
            location->y.val = 0;
            continue;
        }
        if ((slabmap_owner(slb) == dungeon->owner) && (slb->kind == SlbT_CLAIMED))
        { // If it's our owned claimed ground, give it a try
            long ret;
            struct Thing *thing;
            thing = get_trap_for_slab_position(slb_x, slb_y);
            // Only allow to place trap at position where there's no traps already
            if (thing_is_invalid(thing)) {
                SYNCDBG(8,"Trying to place %s trap at (%d,%d)",trap_code_name(kind_chosen),(int)location->x.stl.num,(int)location->y.stl.num);
                ret = try_game_action(comp, dungeon->owner, GA_PlaceTrap, 0, location->x.stl.num, location->y.stl.num, kind_chosen, 0);
            } else {
                ret = -1;
            }
            location->x.val = 0;
            location->y.val = 0;
            if (ret > 0)
              return 1;
        } else
        if (slb->kind != SlbT_PATH)
        { // If it would be a path, we could wait for someone to claim it; but if it's not..
            if (find_from_task_list(dungeon->owner, get_slab_number(slb_x,slb_y)) < 0)
            { // If we have no intention of doing a task there - remove it from list
                ERRORLOG("Removing outdated trap location (%d,%d)",(int)location->x.stl.num,(int)location->y.stl.num);
                location->x.val = 0;
                location->y.val = 0;
            }
        }
    }
    return 4;
}

long computer_pick_trainig_or_scavenging_creatures_and_place_on_room(struct Computer2 *comp, struct Room *room, long thing_idx, long tasks_limit)
{
    struct CreatureControl *cctrl;
    struct Thing *thing;
    long new_tasks;
    unsigned long k;
    long i;
    new_tasks = 0;
    // Sweep through creatures list
    i = thing_idx;
    k = 0;
    while (i != 0)
    {
      thing = thing_get(i);
      if (thing_is_invalid(thing))
      {
        ERRORLOG("Jump to invalid thing detected");
        break;
      }
      cctrl = creature_control_get_from_thing(thing);
      i = cctrl->players_next_creature_idx;
      // Per creature code
      if (creature_is_training(thing) || creature_is_scavengering(thing)) // originally, only CrSt_Training and CrSt_Scavengering were accepted
      {
        if (!create_task_move_creature_to_pos(comp, thing, room->central_stl_x, room->central_stl_y))
          break;
        new_tasks++;
        if (new_tasks >= tasks_limit)
          break;
      }
      // Per creature code ends
      k++;
      if (k > THINGS_COUNT)
      {
        ERRORLOG("Infinite loop detected when sweeping things list");
        break;
      }
    }
    return new_tasks;
}

/** Picks creatures and workers from given dungeon who are doing expensive jobs, then places them on lair.
 *
 * @param comp Computer player who controls the target dungeon.
 * @param tasks_limit Max amount of computer tasks to create.
 * @return Amount of new computer tasks created.
 */
long computer_pick_expensive_job_creatures_and_place_on_lair(struct Computer2 *comp, long tasks_limit)
{
    struct Dungeon *dungeon;
    struct Room *room;
    long new_tasks;
    dungeon = comp->dungeon;
    room = room_get(dungeon->room_kind[RoK_LAIR]);
    new_tasks = 0;
    // If we don't have lair, then don't even bother
    if (room_is_invalid(room))
      return new_tasks;
    // Sweep through creatures list
    new_tasks += computer_pick_trainig_or_scavenging_creatures_and_place_on_room(comp, room, dungeon->creatr_list_start, tasks_limit);
    if (new_tasks >= tasks_limit)
        return new_tasks;
    // Sweep through workers list
    new_tasks += computer_pick_trainig_or_scavenging_creatures_and_place_on_room(comp, room, dungeon->digger_list_start, tasks_limit-new_tasks);
    return new_tasks;
}

long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check)
{
    long money;
    struct ComputerProcess *cproc;
    struct Dungeon *dungeon;
    long ret;
    long i;
    SYNCDBG(8,"Starting");
    //return _DK_computer_check_for_money(comp, check);
    ret = 4;
    // Try creating digging for gold process
    money = get_computer_money_less_cost(comp);
    if ((check->param3 > money) || (check->param2 > money))
    {
      for (i=0; i <= COMPUTER_PROCESSES_COUNT; i++)
      {
          cproc = &comp->processes[i];
          if ((cproc->flags & ComProc_Unkn0002) != 0)
              break;
          //TODO COMPUTER_PLAYER comparing function pointers is a bad practice
          if (cproc->func_check == computer_check_dig_to_gold)
          {
            cproc->field_4++;
            if (game.play_gameturn - cproc->field_3C > 20)
              cproc->field_3C = 0;
          }
      }
    }

    // Try selling traps and doors
    dungeon = comp->dungeon;
    if (dungeon->field_14B8 > dungeon->total_money_owned)
    {
      if (dungeon_has_room(dungeon, RoK_WORKSHOP))
      {
        if (get_task_in_progress(comp, CTT_SellTrapsAndDoors) == NULL)
        {
          if (create_task_sell_traps_and_doors(comp, 3*dungeon->field_14B8/2))
          {
            ret = 1;
          }
        }
      }
    }
    if (3*dungeon->field_14B8/2 <= dungeon->total_money_owned)
      return ret;

    // Move creatures away from rooms which costs a lot
    if (computer_pick_expensive_job_creatures_and_place_on_lair(comp, 3) > 0)
        ret = 1;
    return ret;
}

long count_creatures_for_defend_pickup(struct Computer2 *comp)
{
    return _DK_count_creatures_for_defend_pickup(comp);
}

long computer_find_non_solid_block(struct Computer2 *comp, struct Coord3d *pos)
{
    return _DK_computer_find_non_solid_block(comp, pos);
}

long computer_able_to_use_magic(struct Computer2 *comp, PowerKind pwkind, long a3, long a4)
{
    return _DK_computer_able_to_use_magic(comp, pwkind, a3, a4);
}

long check_call_to_arms(struct Computer2 *comp)
{
    return _DK_check_call_to_arms(comp);
}

TbBool setup_a_computer_player(PlayerNumber plyr_idx, long comp_model)
{
  struct ComputerProcessTypes *cpt;
  struct ComputerProcess *process;
  struct ComputerProcess *newproc;
  struct ComputerCheck *check;
  struct ComputerCheck *newchk;
  struct ComputerEvent *event;
  struct ComputerEvent *newevnt;
  struct Comp2_UnkStr1 *unkptr;
  struct Computer2 *comp;
  long i;
  //_DK_setup_a_computer_player(plyridx, comp_model); return;
  if ((plyr_idx >= PLAYERS_COUNT) || (plyr_idx == game.hero_player_num)
      || (plyr_idx == game.neutral_player_num)) {
      WARNLOG("Tried to setup player %d which can't be used this way",(int)plyr_idx);
      return false;
  }
  comp = &game.computer[plyr_idx];
  LbMemorySet(comp, 0, sizeof(struct Computer2));
  cpt = get_computer_process_type_template(comp_model);
  comp->dungeon = get_players_num_dungeon(plyr_idx);
  comp->model = comp_model;
  if (dungeon_invalid(comp->dungeon)) {
      WARNLOG("Tried to setup player %d which has no dungeon",(int)plyr_idx);
      comp->dungeon = INVALID_DUNGEON;
      comp->model = 0;
      return false;
  }
  comp->field_18 = cpt->field_C;
  comp->field_14 = cpt->field_8;
  comp->max_room_build_tasks = cpt->max_room_build_tasks;
  comp->field_2C = cpt->field_14;
  comp->field_20 = cpt->field_18;
  comp->field_C = 1;
  comp->task_state = CTaskSt_Select;

  for (i=0; i < PLAYERS_COUNT; i++)
  {
    unkptr = &comp->unkarr_A10[i];
    if (i == plyr_idx)
      unkptr->field_6 = 0x80000000;
    else
      unkptr->field_6 = 0;
  }
  comp->field_1C = cpt->field_4;

  for (i=0; i < COMPUTER_PROCESSES_COUNT; i++)
  {
    process = cpt->processes[i];
    newproc = &comp->processes[i];
    if ((process == NULL) || (process->name == NULL))
    {
      newproc->name = NULL;
      break;
    }
    // Modifying original ComputerProcessTypes structure - I don't like it!
    //TODO COMPUTER_PLAYER comparing function pointers is a bad practice
    if (process->func_setup == computer_setup_any_room)
    {
      if (process->field_14 >= 0)
        process->field_14 = get_room_look_through(process->field_14);
    }
    LbMemoryCopy(newproc, process, sizeof(struct ComputerProcess));
    newproc->parent = process;
  }
  newproc = &comp->processes[i];
  newproc->flags |= ComProc_Unkn0002;

  for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
  {
    check = &cpt->checks[i];
    newchk = &comp->checks[i];
    if ((check == NULL) || (check->name == NULL))
    {
      newchk->name = NULL;
      break;
    }
    LbMemoryCopy(newchk, check, sizeof(struct ComputerCheck));
  }
  // Note that we don't have special, empty check at end of array
  // The check with 0x02 flag identifies end of active checks
  // (the check with 0x02 flag is invalid - only previous checks are in use)
  //newchk = &comp->checks[i];
  newchk->flags |= 0x02;

  for (i=0; i < COMPUTER_EVENTS_COUNT; i++)
  {
    event = &cpt->events[i];
    newevnt = &comp->events[i];
    if ((event == NULL) || (event->name == NULL))
    {
      newevnt->name = NULL;
      break;
    }
    LbMemoryCopy(newevnt, event, sizeof(struct ComputerEvent));
  }
  return true;
}

void computer_check_events(struct Computer2 *comp)
{
    struct Dungeon * dungeon;
    struct ComputerEvent * cevent;
    struct Event * event;
    long i,n;
    SYNCDBG(17,"Starting");
    //_DK_computer_check_events(comp);
    dungeon = comp->dungeon;
    for (i=0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        cevent = &comp->events[i];
        if (cevent->name == NULL)
            break;
        switch (cevent->cetype)
        {
        case 0:
            for (n=0; n < EVENTS_COUNT; n++)
            {
                event = &game.event[n];
                if ( ((event->field_0 & 0x01) != 0) &&
                      (event->owner == dungeon->owner) &&
                      (event->kind == cevent->field_8) )
                {
                    if (cevent->func_event(comp, cevent, event) == 1)
                        cevent->last_test_gameturn = game.play_gameturn;
                }
            }
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            if ((cevent->last_test_gameturn + cevent->test_interval) <= (long)game.play_gameturn)
            {
                if (cevent->func_test(comp,cevent) == 1)
                    ; // nothing done with this "if" - hmm... could be intentional, or not.
                cevent->last_test_gameturn = game.play_gameturn;
            }
            break;
        default:
            ERRORLOG("Unhandled Computer Event Type %d",(int)cevent->cetype);
            break;
        }
    }
}

TbBool process_checks(struct Computer2 *comp)
{
    struct ComputerCheck *ccheck;
    long delta;
    long i;
    SYNCDBG(17,"Starting");
    //return _DK_process_checks(comp);
    for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        ccheck = &comp->checks[i];
        if (comp->tasks_did <= 0)
            break;
        if ((ccheck->flags & 0x02) != 0)
            break;
        if ((ccheck->flags & 0x01) == 0)
        {
            delta = (game.play_gameturn - ccheck->param4);
            if ((delta > ccheck->turns_interval) && (ccheck->func != NULL))
            {
                SYNCDBG(8,"Executing check %ld, \"%s\"",i,ccheck->name);
                ccheck->func(comp, ccheck);
                ccheck->param4 = game.play_gameturn;
            }
        }
    }
    return true;
}

TbBool process_processes_and_task(struct Computer2 *comp)
{
  struct ComputerProcess *process;
  Comp_Process_Func callback;
  int i;
  SYNCDBG(17,"Starting");
  for (i=comp->tasks_did; i > 0; i--)
  {
    if (comp->tasks_did <= 0)
        return false;
    if ((game.play_gameturn % comp->field_18) == 0)
        process_tasks(comp);
    switch (comp->task_state)
    {
    case CTaskSt_Wait:
        comp->gameturn_wait--;
        if (comp->gameturn_wait <= 0)
        {
            comp->gameturn_wait = comp->gameturn_delay;
            set_next_process(comp);
        }
        break;
    case CTaskSt_Select:
        set_next_process(comp);
        break;
    case CTaskSt_Perform:
        if ((comp->ongoing_process > 0) && (comp->ongoing_process <= COMPUTER_PROCESSES_COUNT))
        {
            process = &comp->processes[comp->ongoing_process];
            callback = process->func_task;
            if (callback != NULL) {
                callback(comp,process);
            }
        } else
        {
            ERRORLOG("No Process %d for a computer player",(int)comp->ongoing_process);
            comp->task_state = CTaskSt_Wait;
        }
        break;
    default:
        ERRORLOG("Invalid task state %d",(int)comp->task_state);
        break;
    }
  }
  return true;
}

void process_computer_player2(PlayerNumber plyr_idx)
{
    struct Computer2 *comp;
    SYNCDBG(7,"Starting for player %d",(int)plyr_idx);
    //_DK_process_computer_player2(plyr_idx);
    if (plyr_idx >= PLAYERS_COUNT) {
        return;
    }
    comp = &game.computer[plyr_idx];
    if ((comp->field_14 != 0) && (comp->field_2C <= game.play_gameturn))
      comp->tasks_did = 1;
    else
      comp->tasks_did = 0;
    if (comp->tasks_did <= 0) {
        return;
    }
    computer_check_events(comp);
    process_checks(comp);
    process_processes_and_task(comp);
    if ((comp->tasks_did < 0) || (comp->tasks_did > 1)) {
        ERRORLOG("Computer player %d performed %d tasks instead of up to one",(int)plyr_idx,(int)comp->tasks_did);
    }
}

struct ComputerProcess *computer_player_find_process_by_func_setup(long plyr_idx,Comp_Process_Func func_setup)
{
  struct ComputerProcess *process;
  struct Computer2 *comp;
  comp = &(game.computer[plyr_idx]);
  process = &comp->processes[0];
  while ((process->flags & ComProc_Unkn0002) == 0)
  {
    if (process->func_setup == func_setup)
    {
        return process;
    }
    process++;
  }
  return NULL;
}

TbBool computer_player_demands_gold_check(PlayerNumber plyr_idx)
{
  struct ComputerProcess *dig_process;
  //TODO COMPUTER_PLAYER comparing function pointers is a bad practice
  dig_process = computer_player_find_process_by_func_setup(plyr_idx,computer_setup_dig_to_gold);
  // If this computer player has no gold digging process
  if (dig_process == NULL)
  {
      SYNCDBG(18,"Player %d has no digging ability.",(int)plyr_idx);
      return false;
  }
  if ((dig_process->flags & ComProc_Unkn0004) == 0)
  {
      SYNCDBG(18,"Player %d isn't interested in digging.",(int)plyr_idx);
      return false;
  }
  SYNCDBG(8,"Player %d wants to start digging.",(int)plyr_idx);
  // If the computer player needs to dig for gold
  if (gameadd.turn_last_checked_for_gold+GOLD_DEMAND_CHECK_INTERVAL < game.play_gameturn)
  {
      dig_process->flags &= ~ComProc_Unkn0004;
      return true;
  }
  return false;
}

void process_computer_players2(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    TbBool needs_gold_check;
    int i;
    //_DK_process_computer_players2();
    needs_gold_check = false;
#ifdef PETTER_AI
    SAI_run_shared();
#endif
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        player = get_player(i);
        if (!player_exists(player))
          continue;
        dungeon = get_players_dungeon(player);
        if (((player->field_0 & 0x40) != 0) || ((dungeon->computer_enabled & 0x01) != 0))
        {
          if (player->field_2C == 1)
          {
#ifdef PETTER_AI
            SAI_run_for_player(i);
#else
            process_computer_player2(i);
            if (computer_player_demands_gold_check(i))
            {
                needs_gold_check = true;
            }
#endif
          }
        }
    }
    if (needs_gold_check)
    {
      SYNCDBG(0,"Computer players demand gold check.");
      gameadd.turn_last_checked_for_gold = game.play_gameturn;
      check_map_for_gold();
    } else
    if (gameadd.turn_last_checked_for_gold > game.play_gameturn)
    {
      gameadd.turn_last_checked_for_gold = 0;
    }
}

void setup_computer_players2(void)
{
  struct PlayerInfo *player;
  int i;
  gameadd.turn_last_checked_for_gold = game.play_gameturn;
  check_map_for_gold();
  for (i=0; i < COMPUTER_TASKS_COUNT; i++)
  {
    LbMemorySet(&game.computer_task[i], 0, sizeof(struct ComputerTask));
  }
#ifdef PETTER_AI
  SAI_init_for_map();
#endif
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player))
    {
      if (player->field_2C == 1)
      {
#ifdef PETTER_AI
        SAI_init_for_player(i);
#else
        setup_a_computer_player(i, 7);
#endif
      }
    }
  }
}

void restore_computer_player_after_load(void)
{
    struct Computer2 *comp;
    struct PlayerInfo *player;
    struct ComputerProcessTypes *cpt;
    long plyr_idx;
    long i;
    SYNCDBG(7,"Starting");
    //_DK_restore_computer_player_after_load();
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        player = get_player(plyr_idx);
        comp = &game.computer[plyr_idx];
        if (!player_exists(player))
        {
            LbMemorySet(comp, 0, sizeof(struct Computer2));
            comp->dungeon = INVALID_DUNGEON;
            continue;
        }
        if (player->field_2C != 1)
        {
            LbMemorySet(comp, 0, sizeof(struct Computer2));
            comp->dungeon = get_players_dungeon(player);
            continue;
        }
        comp->dungeon = get_players_dungeon(player);
        cpt = get_computer_process_type_template(comp->model);

        for (i=0; i < COMPUTER_PROCESSES_COUNT; i++)
        {
            if (cpt->processes[i] == NULL)
                break;
            //if (cpt->processes[i]->name == NULL)
            //    break;
            SYNCDBG(12,"Player %ld process %ld is \"%s\"",plyr_idx,i,cpt->processes[i]->name);
            comp->processes[i].name = cpt->processes[i]->name;
            comp->processes[i].parent = cpt->processes[i];
            comp->processes[i].func_check = cpt->processes[i]->func_check;
            comp->processes[i].func_setup = cpt->processes[i]->func_setup;
            comp->processes[i].func_task = cpt->processes[i]->func_task;
            comp->processes[i].func_complete = cpt->processes[i]->func_complete;
            comp->processes[i].func_pause = cpt->processes[i]->func_pause;
        }
        for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
        {
            if (cpt->checks[i].name == NULL)
              break;
            SYNCDBG(12,"Player %ld check %ld is \"%s\"",plyr_idx,i,cpt->checks[i].name);
            comp->checks[i].name = cpt->checks[i].name;
            comp->checks[i].func = cpt->checks[i].func;
        }
        for (i=0; i < COMPUTER_EVENTS_COUNT; i++)
        {
            if (cpt->events[i].name == NULL)
              break;
            comp->events[i].name = cpt->events[i].name;
            comp->events[i].func_event = cpt->events[i].func_event;
            comp->events[i].func_test = cpt->events[i].func_test;
            comp->events[i].process = cpt->events[i].process;
        }
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
