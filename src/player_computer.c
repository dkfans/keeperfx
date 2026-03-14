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
#include "pre_inc.h"
#include "player_computer.h"

#include <limits.h>

#include "ariadne_wallhug.h"
#include "bflib_basics.h"
#include "bflib_dernc.h"
#include "bflib_fileio.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "config.h"
#include "config_compp.h"
#include "config_creature.h"
#include "config_terrain.h"
#include "creature_states.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "globals.h"
#include "gui_msgs.h"
#include "keeperfx.hpp"
#include "magic_powers.h"
#include "map_utils.h"
#include "player_complookup.h"
#include "player_utils.h"
#include "power_hand.h"
#include "room_data.h"
#include "spdigger_stack.h"
#include "thing_navigate.h"
#include "thing_traps.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
// Function definition needed to compare pointers - remove pending
long computer_setup_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_check_dig_to_gold(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_setup_any_room(struct Computer2 *comp, struct ComputerProcess *cproc);

/******************************************************************************/
struct Computer2 *get_computer_player_f(long plyr_idx,const char *func_name)
{
    if ((plyr_idx >= 0) && (plyr_idx < PLAYERS_COUNT))
        return &game.computer[plyr_idx];
    ERRORMSG("%s: Tried to get non-existing computer player %d!",func_name,(int)plyr_idx);
    return INVALID_COMPUTER_PLAYER;
}

TbBool computer_player_invalid(const struct Computer2 *comp)
{
    if (comp == INVALID_COMPUTER_PLAYER)
        return true;
    return (comp < &game.computer[0]);
}

TbBool computer_player_in_emergency_state(const struct Computer2 *comp)
{
    struct Dungeon* dungeon = comp->dungeon;
    if (get_computer_money_less_cost(comp) < -1000)
        return true;
    if (dungeon->num_active_diggers < 3)
        return true;
    return false;
}

GoldAmount get_dungeon_money_less_cost(const struct Dungeon *dungeon)
{
    // As payday need, take amount planned for next payday
    GoldAmount money_payday = dungeon->creatures_total_pay;
    // In case payday expenses are low, require enough money to make special digger
    GoldAmount money_mkdigger = compute_power_price(dungeon->owner, PwrK_MKDIGGER, 0);
    if (money_payday < money_mkdigger)
        money_payday = money_mkdigger;
    return dungeon->total_money_owned - money_payday;
}

GoldAmount get_computer_money_less_cost(const struct Computer2 *comp)
{
    return get_dungeon_money_less_cost(comp->dungeon);
}

long set_autopilot_type(PlayerNumber plyr_idx, long aptype)
{
    setup_a_computer_player(plyr_idx, comp_player_conf.computer_assist_types[aptype-1]);
    return 1;
}

struct ComputerTask * able_to_build_room_at_task(struct Computer2 *comp, RoomKind rkind, long width_slabs, long height_slabs, long max_distance, long perfect)
{
    long i = comp->task_idx;
    unsigned long k = 0;
    while (i != 0)
    {
        struct ComputerTask* ctask = get_computer_task(i);
        if (computer_task_invalid(ctask))
        {
            ERRORLOG("Jump to invalid task detected");
            break;
        }
        i = ctask->next_task;
        // Per-task code
        if (flag_is_set(ctask->flags, (ComTsk_Unkn0001|ComTsk_Unkn0002)))
        {
            unsigned short max_radius = ctask->create_room.width / 2;
            if (max_radius <= ctask->create_room.height / 2)
              max_radius = ctask->create_room.height / 2;
            struct ComputerTask* roomtask = able_to_build_room(comp, &ctask->new_room_pos, rkind, width_slabs, height_slabs, max_distance + max_radius + 1, perfect);
            if (!computer_task_invalid(roomtask)) {
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
 * @param comp Computer player being checked.
 * @param rkind The room kind to be built.
 * @param look_kind The room kind which we'd like to search as starting point for new corridor.
 * @param width_slabs
 * @param height_slabs
 * @param area
 * @param a6
 * @return
 */
struct ComputerTask * able_to_build_room_from_room(struct Computer2 *comp, RoomKind rkind, RoomKind look_kind, long width_slabs, long height_slabs, long max_slabs_dist, long perfect)
{
    struct Dungeon* dungeon = comp->dungeon;
    long i = dungeon->room_list_start[look_kind];
    unsigned long k = 0;
    while (i != 0)
    {
        struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_owner;
        // Per-room code
        struct Coord3d pos;
        pos.x.val = subtile_coord_center(room->central_stl_x);
        pos.y.val = subtile_coord_center(room->central_stl_y);
        pos.z.val = subtile_coord(1,0);
        struct ComputerTask* roomtask = able_to_build_room(comp, &pos, rkind, width_slabs, height_slabs, max_slabs_dist, perfect);
        if (!computer_task_invalid(roomtask)) {
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
    return INVALID_COMPUTER_TASK;
}

struct ComputerTask *computer_setup_build_room(struct Computer2 *comp, RoomKind rkind, long width_slabs, long height_slabs, long look_randstart)
{
    struct Dungeon* dungeon = comp->dungeon;
    long i;
    if (room_role_matches(rkind,RoRoF_LairStorage))
    {
        //the first lair might be bigger if the portal limit is high
        if (!dungeon_has_room(dungeon, rkind))
        {
            if (width_slabs * height_slabs < dungeon->max_creatures_attracted)
            {
                width_slabs++;
                if (width_slabs * height_slabs < dungeon->max_creatures_attracted)
                {
                    height_slabs++;
                }
            }
        }
    }
    long max_slabs = height_slabs;
    if (max_slabs < width_slabs)
        max_slabs = width_slabs;
    long dist_min = (max_slabs + 1) / 2 + 1;
    long dist_max = dist_min / 3 + 2 * dist_min;
    const long arr_length = game.conf.slab_conf.room_types_count;
    for (long distance_in_slabs = dist_min; distance_in_slabs < dist_max; distance_in_slabs++)
    {
        for (long perfect = 1; perfect >= 0; perfect--)
        {
            unsigned int look_kind = look_randstart;
            if (look_randstart < 0)
            {
                look_kind = AI_RANDOM(arr_length);
            }
            for (i=0; i < arr_length; i++)
            {
                struct ComputerTask *roomtask;
                if (look_kind == RoK_TYPES_COUNT)
                {
                    roomtask = able_to_build_room_at_task(comp, rkind, width_slabs, height_slabs, distance_in_slabs, perfect);
                } else
                {
                    roomtask = able_to_build_room_from_room(comp, rkind, look_kind, width_slabs, height_slabs, distance_in_slabs, perfect);
                }
                if (!computer_task_invalid(roomtask)) {
                    return roomtask;
                }
                look_kind = (look_kind + 1) % arr_length;
            }
        }
    }
    SYNCLOG("Player %d dungeon has no place for %s sized %dx%d", (int)dungeon->owner, room_code_name(rkind), (int)width_slabs, (int)height_slabs);
    return INVALID_COMPUTER_TASK;
}

/**
 * Finds a room which would be a good place to start digging for a gold vein.
 *
 * @param dungeon
 * @param gldlook
 * @param nearroom
 * @return Distance to the selected room in subtiles, or INT32_MAX if no room was found.
 */
long computer_finds_nearest_room_to_gold_lookup(const struct Dungeon *dungeon, const struct GoldLookup *gldlook, struct Room **nearroom)
{
    *nearroom = INVALID_ROOM;
    struct Coord3d gold_pos;
    gold_pos.x.val = 0;
    gold_pos.y.val = 0;
    gold_pos.z.val = 0;
    gold_pos.x.stl.num = gldlook->stl_x;
    gold_pos.y.stl.num = gldlook->stl_y;
    long min_distance = INT32_MAX;
    int32_t distance = INT32_MAX;
    for (long rkind = 1; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        struct Room* room = find_room_nearest_to_position(dungeon->owner, rkind, &gold_pos, &distance);
        if (!room_is_invalid(room))
        {
            // Convert to subtiles
            distance = coord_subtile(distance);
            // Decrease the value by gold area radius
            distance -= LbSqrL(gldlook->num_gold_slabs * STL_PER_SLB);
            distance -= LbSqrL(gldlook->num_gem_slabs * STL_PER_SLB * 4);
            // We can accept longer distances if digging directly to treasure room

            if (room_role_matches(room->kind,RoRoF_GoldStorage))
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
    task_pos.x.val = 0;
    task_pos.y.val = 0;
    task_pos.z.val = 0;
    task_pos.x.stl.num = gldlook->stl_x;
    task_pos.y.stl.num = gldlook->stl_y;
    long min_distance = INT32_MAX;
    long i = comp->task_idx;
    unsigned long k = 0;
    while (i != 0)
    {
        struct ComputerTask* ctask = get_computer_task(i);
        if (computer_task_invalid(ctask))
        {
            ERRORLOG("Jump to invalid task detected");
            break;
        }
        i = ctask->next_task;
        // Per-task code
        if (flag_is_set(ctask->flags, (ComTsk_Unkn0001|ComTsk_Unkn0002)))
        {
            MapCoordDelta delta_x = ctask->new_room_pos.x.val - (MapCoordDelta)task_pos.x.val;
            MapCoordDelta delta_y = ctask->new_room_pos.y.val - (MapCoordDelta)task_pos.y.val;
            long distance = LbDiagonalLength(abs(delta_x), abs(delta_y));
            // Convert to subtiles
            distance = coord_subtile(distance);
            // Decrease the value by gold area radius
            distance -= LbSqrL(gldlook->num_gold_slabs * STL_PER_SLB);
            distance -= LbSqrL(gldlook->num_gem_slabs * STL_PER_SLB * 4);
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
 * @return Lower or equal 0 on failure, positive amount of subtiles if gold digging is ready to go.
 */
long computer_finds_nearest_room_to_gold(struct Computer2 *comp, struct Coord3d *pos, struct GoldLookup **gldlookref)
{
    SYNCDBG(5,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    struct GoldLookup* gldlooksel = NULL;
    *gldlookref = gldlooksel;
    struct Coord3d locpos;
    locpos.x.val = 0;
    locpos.y.val = 0;
    locpos.z.val = 0;
    struct Coord3d* spos = &locpos;
    long lookups_checked = 0;
    long dig_distance = INT32_MAX;
    for (long i = 0; i < GOLD_LOOKUP_COUNT; i++)
    {
        struct GoldLookup* gldlook = get_gold_lookup(i);
        if ((gldlook->flags & 0x01) == 0)
            continue;
        SYNCDBG(18,"Valid vein at (%d,%d)",(int)gldlook->stl_x,(int)gldlook->stl_y);
        if ((gldlook->player_interested[dungeon->owner] & 0x03) != 0)
            continue;
        SYNCDBG(8,"Searching for place to reach (%d,%d)",(int)gldlook->stl_x,(int)gldlook->stl_y);
        lookups_checked++;
        struct Room *room = INVALID_ROOM;
        long new_dist = computer_finds_nearest_room_to_gold_lookup(dungeon, gldlook, &room);
        if (dig_distance > new_dist)
        {
            locpos.x.val = subtile_coord_center(room->central_stl_x);
            locpos.y.val = subtile_coord_center(room->central_stl_y);
            locpos.z.val = subtile_coord(1,0);
            spos = &locpos;
            dig_distance = new_dist;
            gldlooksel = gldlook;
            SYNCDBG(8,"Distance from room at (%d,%d) is %d",(int)spos->x.stl.num,(int)spos->y.stl.num,(int)dig_distance);
        }
        struct ComputerTask *ctask = NULL;
        new_dist = computer_finds_nearest_task_to_gold(comp, gldlook, &ctask);
        if (dig_distance > new_dist)
        {
            spos = &ctask->new_room_pos;
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
    SYNCDBG(8,"Best digging start to reach (%d,%d) is on subtile (%d,%d); distance is %d",(int)gldlooksel->stl_x,(int)gldlooksel->stl_y,(int)spos->x.stl.num,(int)spos->y.stl.num,(int)dig_distance);
    *gldlookref = gldlooksel;
    pos->x.val = spos->x.val;
    pos->y.val = spos->y.val;
    pos->z.val = spos->z.val;
    if (dig_distance < 1)
        dig_distance = 1;
    if (dig_distance > INT32_MAX)
        dig_distance = INT32_MAX;
    return dig_distance;
}

unsigned long count_creatures_availiable_for_fight(struct Computer2 *comp, struct Coord3d *pos)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    unsigned long count = 0;
    unsigned long k = 0;
    int i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->combat_flags == 0)
        {
            if ((pos == NULL) || creature_can_navigate_to(thing, pos, NavRtF_NoOwner)) {
                count++;
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

TbBool is_there_an_attack_task(const struct Computer2 *comp)
{
    static const ComputerTaskType attack_tasks[] = {
        CTT_DigToAttack,
        CTT_MagicCallToArms,
        CTT_PickupForAttack,
        CTT_MoveCreatureToRoom,
        CTT_MoveCreatureToPos,
        CTT_MoveCreaturesToDefend,
        CTT_None,
    };
    const struct ComputerTask* ctask = get_task_in_progress_in_list(comp, attack_tasks);
    return !computer_task_invalid(ctask);
}

/**
 * Gathers information about players hatred to other players.
 *
 * @param comp Computer player to be considered.
 * @param hates The output hatred array. The array size should be PLAYERS_COUNT.
 *   The function will fill the array with hatred information, and sort it so that
 *   first entry informs of the player toward whom the hatred is highest.
 */
void get_opponent(struct Computer2 *comp, struct THate hates[])
{
    SYNCDBG(7,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    long i;
    // Initialize hate struct
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        struct THate* hate = &hates[i];
        struct OpponentRelation* oprel = &comp->opponent_relations[i];
        hate->amount = oprel->hate_amount;
        hate->plyr_idx = i;
        hate->pos_near = NULL;
        hate->distance_near = INT32_MAX;
    }
    // Sort the hates, using basic sorting algorithm
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        for (long n = 0; n < PLAYERS_COUNT - 1; n++)
        {
            struct THate* hat1 = &hates[n];
            struct THate* hat2 = &hates[n + 1];
            if (hat2->amount > hat1->amount)
            {
                // Switch hates so larger one is first
                struct THate tmp;
                memcpy(&tmp,hat2,sizeof(struct THate));
                memcpy(hat2,hat1,sizeof(struct THate));
                memcpy(hat1,&tmp,sizeof(struct THate));
            }
        }
    }
    // Get position of a possible attack - select one of minimum distance to our heart
    struct Thing* heartng = get_player_soul_container(dungeon->owner);
    MapSubtlCoord dnstl_x = heartng->mappos.x.stl.num;
    MapSubtlCoord dnstl_y = heartng->mappos.y.stl.num;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        struct THate* hate = &hates[i];
        struct OpponentRelation* oprel = &comp->opponent_relations[hate->plyr_idx];
        int ptidx = oprel->next_idx;
        if (ptidx > 0)
          ptidx--;
        for (long n = 0; n < COMPUTER_SPARK_POSITIONS_COUNT; n++)
        {
            struct Coord3d* pos = &oprel->pos_A[ptidx];
            if ((pos->x.val > 0) && (pos->y.val > 0))
            {
                if (search_spiral(pos, hate->plyr_idx, 25, xy_walkable) == 25)
                {
                    pos->x.val = 0;
                } else
                if (find_from_task_list_by_subtile(dungeon->owner, pos->x.stl.num, pos->y.stl.num) >= 0)
                {
                    pos->x.val = 0;
                } else
                {
                    long dist = grid_distance(pos->x.stl.num, pos->y.stl.num, dnstl_x, dnstl_y);
                    if (hate->distance_near >= dist)
                    {
                        hate->distance_near = dist;
                        hate->pos_near = pos;
                    }
                }
            }
            ptidx = (ptidx + 1) % COMPUTER_SPARK_POSITIONS_COUNT;
        }
    }
}

TbBool computer_finds_nearest_room_to_pos(struct Computer2 *comp, struct Room **retroom, struct Coord3d *nearpos){
    long nearest_distance = INT32_MAX;
    struct Dungeon* dungeon = comp->dungeon;
    *retroom = NULL;

    for (RoomKind i = 0; i < game.conf.slab_conf.room_types_count; i++)
    {
        struct Room* room = room_get(dungeon->room_list_start[i]);

        while (!room_is_invalid(room))
        {
            struct Coord3d room_center_pos;
            room_center_pos.x.val = subtile_coord_center(room->central_stl_x);
            room_center_pos.y.val = subtile_coord_center(room->central_stl_y);
            room_center_pos.z.val = get_floor_height_at(&room_center_pos);

            long distance = get_2d_distance_squared(&room_center_pos, nearpos);
            if (distance < nearest_distance)
            {
                nearest_distance = distance;
                *retroom = room;
            }
            room = room_get(room->next_of_owner);
        }

    }
    if (nearest_distance == INT32_MAX)
        return false;
    return true;
}

long setup_computer_attack(struct Computer2 *comp, struct ComputerProcess *cproc, struct Coord3d *pos, long victim_plyr_idx)
{
    struct Room *room;
    SYNCDBG(8,"Starting player %d attack on %d",(int)comp->dungeon->owner,(int)victim_plyr_idx);
    if (!computer_finds_nearest_room_to_pos(comp, &room, pos)) {
        SYNCDBG(7,"Cannot find owned room near (%d,%d), giving up",(int)pos->x.stl.num,(int)pos->y.stl.num);
        return 0;
    }
    struct Coord3d startpos;
    startpos.x.val = subtile_coord_center(stl_slab_center_subtile(room->central_stl_x));
    startpos.y.val = subtile_coord_center(stl_slab_center_subtile(room->central_stl_y));
    startpos.z.val = subtile_coord(1,0);
    struct Coord3d endpos;
    endpos.x.val = pos->x.val;
    endpos.y.val = pos->y.val;
    endpos.z.val = pos->z.val;
    long parent_cproc_idx = computer_process_index(comp, cproc);
    if (!create_task_dig_to_attack(comp, startpos, endpos, victim_plyr_idx, parent_cproc_idx)) {
        SYNCDBG(7,"Cannot create task to dig to (%d,%d), giving up",(int)pos->x.stl.num,(int)pos->y.stl.num);
        return 0;
    }
    SYNCDBG(7,"Attack setup complete for destination (%d,%d)",(int)pos->x.stl.num,(int)pos->y.stl.num);
    return 1;
}

long count_entrances(const struct Computer2 *comp, PlayerNumber plyr_idx)
{
    const struct Dungeon* dungeon = comp->dungeon;
    long count = 0;
    long i = game.entrance_room_id;
    unsigned long k = 0;
    while (i != 0)
    {
        const struct Room* room = room_get(i);
        if (room_is_invalid(room))
        {
            ERRORLOG("Jump to invalid room detected");
            break;
        }
        i = room->next_of_kind;
        // Per-room code
        if ((room->player_interested[dungeon->owner] & 0x01) == 0)
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
    return count_player_list_creatures_of_model(dungeon->creatr_list_start, CREATURE_ANY);
}

long count_diggers_in_dungeon(const struct Dungeon *dungeon)
{
    return count_player_list_creatures_of_model(dungeon->digger_list_start, CREATURE_ANY);
}

/**
 * Returns amount of traps in workshop of given manufacturable model.
 * @param dungeon
 * @param trmodel
 */
long buildable_traps_amount(struct Dungeon *dungeon, ThingModel trmodel)
{
    if ((trmodel < 1) || (trmodel >= game.conf.trapdoor_conf.trap_types_count))
        return 0;

    if ((dungeon->mnfct_info.trap_build_flags[trmodel] & MnfBldF_Manufacturable) != 0)
    {
        return dungeon->mnfct_info.trap_amount_stored[trmodel];
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
    long kinds = 0;
    for (long i = 1; i < game.conf.trapdoor_conf.trap_types_count; i++)
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
    for (long i = 1; i < game.conf.trapdoor_conf.trap_types_count; i++)
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
    // If there are multiple buildable traps of preselected kind
    if ((kind_preselect > 0) && (buildable_traps_amount(dungeon, kind_preselect) >= 2))
        return kind_preselect;
    // No pre-selection - check if there are other multiple traps
    long kinds_multiple = get_number_of_trap_kinds_with_amount_at_least(dungeon, 2);
    if (kinds_multiple > 0) {
        SYNCDBG(18,"Returning one of %d plentiful traps",(int)kinds_multiple);
        return get_nth_of_trap_kinds_with_amount_at_least(dungeon, 2, AI_RANDOM(kinds_multiple));
    }
    // If there are no multiple traps, and we're not allowing to spend last one
    if (!allow_last)
        return 0;
    // If there are buildable traps of preselected kind
    if ((kind_preselect > 0) && (buildable_traps_amount(dungeon, kind_preselect) >= 1))
        return kind_preselect;
    long kinds_single = get_number_of_trap_kinds_with_amount_at_least(dungeon, 1);
    if (kinds_single > 0) {
        SYNCDBG(18,"Returning one of %d single traps",(int)kinds_single);
        return get_nth_of_trap_kinds_with_amount_at_least(dungeon, 1, AI_RANDOM(kinds_single));
    }
    return 0;
}

int computer_find_more_trap_place_locations_around_room(struct Computer2 *comp, const struct Room *room)
{
    //TODO implement finding trap locations
    return 0;
}

int computer_find_more_trap_place_locations(struct Computer2 *comp)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    int num_added = 0;
    RoomKind rkind = AI_RANDOM(game.conf.slab_conf.room_types_count);
    for (int m = 0; m < game.conf.slab_conf.room_types_count; m++, rkind = (rkind + 1) % game.conf.slab_conf.room_types_count)
    {
        unsigned long k = 0;
        int i = dungeon->room_list_start[rkind];
        while (i != 0)
        {
            struct Room* room = room_get(i);
            if (room_is_invalid(room))
            {
                ERRORLOG("Jump to invalid room detected");
                break;
            }
            i = room->next_of_owner;
            // Per-room code
            int nadded = computer_find_more_trap_place_locations_around_room(comp, room);
            if (nadded < 0)
                break;
            num_added += nadded;
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
              ERRORLOG("Infinite loop detected when sweeping rooms list");
              break;
            }
        }
    }
    return num_added;
}

TbBool computer_get_trap_place_location_and_update_locations(struct Computer2 *comp, ThingModel trapmodel, struct Coord3d *retloc)
{
    struct Dungeon* dungeon = comp->dungeon;
    for (long i = 0; i < COMPUTER_TRAP_LOC_COUNT; i++)
    {
        struct Coord3d* location = &comp->trap_locations[i];
        // Check if the entry has coords stored
        if ((location->x.val <= 0) && (location->y.val <= 0))
            continue;
        MapSlabCoord slb_x = subtile_slab(location->x.stl.num);
        MapSlabCoord slb_y = subtile_slab(location->y.stl.num);
        struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
        if (slabmap_block_invalid(slb)) {
            ERRORLOG("Trap location contained off-map point (%d,%d)",(int)location->x.stl.num,(int)location->y.stl.num);
            location->x.val = 0;
            location->y.val = 0;
            continue;
        }
        if (can_place_trap_on(dungeon->owner, location->x.stl.num, location->y.stl.num, trapmodel))
        { // If it's our owned claimed ground, give it a try
            retloc->x.val = location->x.val;
            retloc->y.val = location->y.val;
            retloc->z.val = 0;
            location->x.val = 0;
            location->y.val = 0;
            return true;
        } else
        if (slb->kind != SlbT_PATH)
        { // If it would be a path, we could wait for someone to claim it; but if it's not..
            if (find_from_task_list_by_slab(dungeon->owner, slb_x, slb_y) < 0)
            { // If we have no intention of doing a task there - remove it from list
                WARNLOG("Removing player %d trap location (%d,%d) because of %s there",(int)dungeon->owner,(int)location->x.stl.num,(int)location->y.stl.num,slab_code_name(slb->kind));
                location->x.val = 0;
                location->y.val = 0;
                continue;
            }
        }
    }
    return false;
}

long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    long kind_chosen = computer_choose_best_trap_kind_to_place(dungeon, check->primary_parameter, check->secondary_parameter);
    if (kind_chosen <= 0)
        return CTaskRet_Unk4;
    struct Coord3d pos;
    if (!computer_get_trap_place_location_and_update_locations(comp, kind_chosen, &pos))
    {
        // update list of locations and try to get location again
        if (computer_find_more_trap_place_locations(comp) <= 0) {
            SYNCDBG(7,"Computer players %d could not find any new locations for traps",(int)dungeon->owner);
            return CTaskRet_Unk4;
        }
        if (!computer_get_trap_place_location_and_update_locations(comp, kind_chosen, &pos)) {
            SYNCDBG(7,"Computer players %d could not find place for trap",(int)dungeon->owner);
            return CTaskRet_Unk4;
        }
    }
    // Only allow to place trap at position where there's no traps already
    SYNCDBG(8,"Trying to place %s trap at (%d,%d)",trap_code_name(kind_chosen),(int)pos.x.stl.num,(int)pos.y.stl.num);
    TbResult ret = try_game_action(comp, dungeon->owner, GA_PlaceTrap, 0, pos.x.stl.num, pos.y.stl.num, kind_chosen, 0);
    if (ret > Lb_OK)
      return CTaskRet_Unk1;
    return CTaskRet_Unk4;
}

/**
 * Picks creatures which are currently training or scavenging and places them into lair.
 * @param comp
 * @param room
 * @param thing_idx
 * @param tasks_limit
 * @return Gives the amount of creatures moved.
 */
long computer_pick_training_or_scavenging_creatures_and_place_on_room(struct Computer2 *comp, struct Room *room, long thing_idx, long tasks_limit)
{
    long new_tasks = 0;
    // Sweep through creatures list
    long i = thing_idx;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
      }
      struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
      i = cctrl->players_next_creature_idx;
      // Per creature code
      if (creature_is_training(thing) || creature_is_scavengering(thing)) // originally, only CrSt_Training and CrSt_Scavengering were accepted
      {
        if (!create_task_move_creature_to_subtile(comp, thing, room->central_stl_x, room->central_stl_y, CrSt_CreatureDoingNothing))
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
    struct Dungeon* dungeon = comp->dungeon;
    struct Room* room = room_get(dungeon->room_list_start[RoK_LAIR]);
    long new_tasks = 0;
    // If we don't have lair, then don't even bother
    if (room_is_invalid(room)) {
        return new_tasks;
    }
    // If we can't pick up creatures, admit the failure now
    if (!computer_able_to_use_power(comp, PwrK_HAND, 1, 1)) {
        return new_tasks;
    }
    // Sweep through creatures list
    new_tasks += computer_pick_training_or_scavenging_creatures_and_place_on_room(comp, room, dungeon->creatr_list_start, tasks_limit);
    if (new_tasks >= tasks_limit)
        return new_tasks;
    // Sweep through workers list
    new_tasks += computer_pick_training_or_scavenging_creatures_and_place_on_room(comp, room, dungeon->digger_list_start, tasks_limit-new_tasks);
    return new_tasks;
}

/**
 * Checks how much money the player lacks for next payday.
 * @param comp Computer player who controls the target dungeon.
 * @param check The check being executed; param1 is low gold value, param2 is critical gold value.
 * @note check->primary_parameter is the gold surplus minimum below which we will take a standard action.
 * @note check->secondary_parameter is the gold surplus critical value below which we will take an aggressive action.
 */

long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(18,"Starting");
    long ret = CTaskRet_Unk4;
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    // Check how much money we will have left after payday (or other expenses)
    GoldAmount money_left = get_computer_money_less_cost(comp);
    // Try increasing priority of digging for gold process
    if ((money_left < check->secondary_parameter) || (money_left < check->primary_parameter))
    {
        SYNCDBG(8,"Increasing player %d gold dig process priority",(int)dungeon->owner);
        for (long i = 0; i <= COMPUTER_PROCESSES_COUNT; i++)
        {
            struct ComputerProcess* cproc = &comp->processes[i];
            if (flag_is_set(cproc->flags, ComProc_ListEnd))
                break;
            //TODO COMPUTER_PLAYER comparing function pointers is a bad practice
            if (cproc->func_check == cpfl_computer_check_dig_to_gold)
            {
                cproc->priority++;
                if (game.play_gameturn - cproc->last_run_turn > 20) {
                    cproc->last_run_turn = 0;
                }
            }
        }
    }
    // Try selling traps and doors - aggressive way
    if ((money_left < check->secondary_parameter) && dungeon_has_room_of_role(dungeon, RoRoF_CratesManufctr))
    {
        if (dungeon_has_any_buildable_traps(dungeon) || dungeon_has_any_buildable_doors(dungeon) ||
            player_has_deployed_trap_of_model(dungeon->owner, -1) || player_has_deployed_door_of_model(dungeon->owner, -1, 0))
        {
            if (!is_task_in_progress(comp, CTT_SellTrapsAndDoors))
            {
                SYNCDBG(8,"Creating task to sell any player %d traps and doors",(int)dungeon->owner);
                if (create_task_sell_traps_and_doors(comp, 6, max(check->secondary_parameter-money_left,1),true)) {
                    ret = CTaskRet_Unk1;
                }
            }
        }
    }
    // Try selling traps and doors - cautious way
    if ((money_left < check->primary_parameter) && dungeon_has_room_of_role(dungeon, RoRoF_CratesManufctr))
    {
        if (dungeon_has_any_buildable_traps(dungeon) || dungeon_has_any_buildable_doors(dungeon))
        {
            if (!is_task_in_progress(comp, CTT_SellTrapsAndDoors))
            {
                SYNCDBG(8,"Creating task to sell player %d trap and door boxes",(int)dungeon->owner);
                if (create_task_sell_traps_and_doors(comp, 6, max(check->primary_parameter-money_left,1),false)) {
                    ret = CTaskRet_Unk1;
                }
            }
        }
    }
    // Power hand tasks are exclusive, so select randomly
    int pwhand_task_choose = AI_RANDOM(100);
    // Cautious selling of traps can be used as base for stable economy.
    // If we were able to use it, do not try to move creatures from their jobs.
    if ((ret == CTaskRet_Unk1) && (pwhand_task_choose < 33)) {
        pwhand_task_choose += 33;
    }
    // Move creatures away from rooms which cost a lot to use
    if ((money_left < check->primary_parameter) && (pwhand_task_choose < 33))
    {
        int num_to_move = 3;
        if (!is_task_in_progress_using_hand(comp) && computer_able_to_use_power(comp, PwrK_HAND, 1, num_to_move))
        {
            SYNCDBG(8,"Creating task to pick player %d creatures from expensive jobs",(int)dungeon->owner);
            if (computer_pick_expensive_job_creatures_and_place_on_lair(comp, num_to_move) > 0) {
                ret = CTaskRet_Unk1;
            }
        }
    }
    // Drop imps on gold/gems mining sites
    if ((money_left < check->primary_parameter) && (pwhand_task_choose < 66) && dungeon_has_room_of_role(dungeon, RoRoF_GoldStorage))
    {
        int num_to_move = 3;
        // If there's already task in progress which uses hand, then don't add more
        // content of the hand could be used by wrong task by mistake
        if (!is_task_in_progress_using_hand(comp) && computer_able_to_use_power(comp, PwrK_HAND, 1, num_to_move))
        {
            MapSubtlCoord stl_x = -1;
            MapSubtlCoord stl_y = -1;
            // Find a gold digging site which could use a worker
            int tsk_id = get_random_mining_undug_area_position_for_digger_drop(dungeon->owner, &stl_x, &stl_y);
            if (tsk_id >= 0)
            {
                struct Coord3d pos;
                pos.x.val = subtile_coord_center(stl_x);
                pos.y.val = subtile_coord_center(stl_y);
                pos.z.val = subtile_coord(1,0);
                SYNCDBG(8,"Creating task to move player %d diggers near gold to mine",(int)dungeon->owner);
                if (move_imp_to_mine_here(comp, &pos, num_to_move) > 0) {
                    ret = CTaskRet_Unk1;
                }
            }
        }
    }
    // Move any gold laying around to treasure room
    if ((money_left < check->primary_parameter) && dungeon_has_room_of_role(dungeon, RoRoF_GoldStorage))
    {
        int num_to_move = 10;
        // If there's already task in progress which uses hand, then don't add more
        // content of the hand could be used by wrong task by mistake
        if (!is_task_in_progress_using_hand(comp) && computer_able_to_use_power(comp, PwrK_HAND, 1, num_to_move))
        {
            SYNCDBG(8,"Creating task to move neutral gold to treasury");
            if (create_task_move_gold_to_treasury(comp, num_to_move, 2*dungeon->creatures_total_pay)) {
                ret = CTaskRet_Unk1;
            }
        }
    }
    return ret;
}

long count_creatures_for_defend_pickup(struct Computer2 *comp)
{
  struct Thing *i;
  struct Dungeon *dungeon = comp->dungeon;
  int count = 0;
  int k = 0;

  for ( i = thing_get(dungeon->creatr_list_start);
        !thing_is_invalid(i);
        i = thing_get(creature_control_get_from_thing(i)->players_next_creature_idx) )
    {
        if ( can_thing_be_picked_up_by_player(i, dungeon->owner) )
        {
            struct CreatureControl* cctrl = creature_control_get_from_thing(i);
            if ( !cctrl->combat_flags )
            {
                int crtr_state = get_creature_state_besides_move(i);
                if (( crtr_state != CrSt_CreatureCombatFlee ) &&
                    ( crtr_state != CrSt_ArriveAtAlarm ) &&
                    (!cctrl->called_to_arms ) &&
                    ( crtr_state != CrSt_CreatureGoingHomeToSleep ) &&
                    ( crtr_state != CrSt_CreatureSleep ) &&
                    ( crtr_state != CrSt_AtLairToSleep ) &&
                    ( crtr_state != CrSt_CreatureChooseRoomForLairSite ) &&
                    ( crtr_state != CrSt_CreatureAtNewLair ) &&
                    ( crtr_state != CrSt_CreatureWantsAHome ) &&
                    ( crtr_state != CrSt_CreatureChangeLair ) &&
                    ( crtr_state != CrSt_CreatureAtChangedLair ) &&
                    ( crtr_state != CrSt_CreatureBeingDropped ))
                {
                    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(i);
                    if (crconf->health > 0)
                    {
                        if (255 * i->health / (compute_creature_max_health(crconf->health,cctrl->exp_level)) > crconf->heal_requirement) //before it was 20%, but heal_requirement is 255 based.
                        {
                            ++count;
                        }
                    }
                }
            }
        }

        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    return count;
}

/**
 * Modifies given position into nearest one where thing can be dropped.
 * @param comp
 * @param pos
 */
TbBool computer_find_non_solid_block(const struct Computer2 *comp, struct Coord3d *pos)
{
    for (unsigned long n = 0; n < MID_AROUND_LENGTH; n++)
    {
        MapSubtlCoord arstl_x = pos->x.stl.num + STL_PER_SLB * start_at_around[n].delta_x;
        MapSubtlCoord arstl_y = pos->y.stl.num + STL_PER_SLB * start_at_around[n].delta_y;
        for (unsigned long k = 0; k < MID_AROUND_LENGTH; k++)
        {
            MapSubtlCoord sstl_x = arstl_x + start_at_around[k].delta_x;
            MapSubtlCoord sstl_y = arstl_y + start_at_around[k].delta_y;
            if (can_drop_thing_here(sstl_x, sstl_y, comp->dungeon->owner, 0))
            {
              pos->x.val = subtile_coord_center(sstl_x);
              pos->y.val = subtile_coord_center(sstl_y);
              return true;
            }
        }
    }
    return false;
}

/**
 * Modifies given position into nearest one where creature can be dropped.
 * Excludes dangerous tiles.
 * @param comp
 * @param pos
 */
TbBool computer_find_safe_non_solid_block(const struct Computer2* comp, struct Coord3d* pos)
{
    for (unsigned long n = 0; n < LARGE_AROUND_LIMITED; n++)
    {
        MapSubtlCoord arstl_x = pos->x.stl.num + STL_PER_SLB * large_around[n].delta_x;
        MapSubtlCoord arstl_y = pos->y.stl.num + STL_PER_SLB * large_around[n].delta_y;
        for (unsigned long k = 0; k < MID_AROUND_LENGTH; k++)
        {
            MapSubtlCoord sstl_x = arstl_x + start_at_around[k].delta_x;
            MapSubtlCoord sstl_y = arstl_y + start_at_around[k].delta_y;
            if (can_drop_thing_here(sstl_x, sstl_y, comp->dungeon->owner, 0) && !is_dangerous_drop_subtile(sstl_x, sstl_y))
            {
                pos->x.val = subtile_coord_center(sstl_x);
                pos->y.val = subtile_coord_center(sstl_y);
                return true;
            }
        }
    }
    return false;
}

/**
 * Returns whether computer player is able to use given keeper power.
 * Originally was computer_able_to_use_magic(), returning 0..4.
 * @param comp
 * @param pwkind
 * @param power_level
 * @param amount
 * @return
 */
TbBool computer_able_to_use_power(struct Computer2 *comp, PowerKind pwkind, KeepPwrLevel power_level, long amount)
{
    struct Dungeon* dungeon = comp->dungeon;
    if (!is_power_available(dungeon->owner, pwkind)) {
        return false;
    }
    if (power_level >= MAGIC_OVERCHARGE_LEVELS)
        power_level = MAGIC_OVERCHARGE_LEVELS;
    GoldAmount money = get_computer_money_less_cost(comp);
    GoldAmount price = compute_power_price(dungeon->owner, pwkind, power_level);
    if ((price > 0) && (amount * price > money)) {
        return false;
    }
    return true;
}

long check_call_to_arms(struct Computer2 *comp)
{
    SYNCDBG(8,"Starting for player %d",(int)comp->dungeon->owner);
    long ret = 1;
    if (comp->dungeon->cta_start_turn != 0)
    {
        long i = comp->task_idx;
        unsigned long k = 0;
        while (i != 0)
        {
            const struct ComputerTask* ctask = get_computer_task(i);
            if (computer_task_invalid(ctask))
            {
                ERRORLOG("Jump to invalid task detected");
                break;
            }
            i = ctask->next_task;
            // Per-task code
            if (flag_is_set(ctask->flags, ComTsk_Unkn0001))
            {
                if ((ctask->ttype == CTT_MagicCallToArms) && (ctask->task_state == CTaskSt_Select))
                {
                    if (ret == 1) {
                        SYNCDBG(8,"Found existing CTA task");
                        ret = 0;
                    }
                    if (ctask->delay + ctask->lastrun_turn - (long)game.play_gameturn < ctask->delay - ctask->delay/10) {
                        SYNCDBG(8,"Less than 90 turns");
                        ret = -1;
                        break;
                    }
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
    }
    return ret;
}

TbBool setup_a_computer_player(PlayerNumber plyr_idx, long comp_model)
{
    struct ComputerProcess *newproc;
    struct ComputerCheck *newchk;
    long i;
    if ((plyr_idx >= PLAYERS_COUNT)) {
        WARNLOG("Tried to setup player %d which can't be used this way",(int)plyr_idx);
        return false;
    }
    if(!player_is_keeper(plyr_idx))
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        player->player_type = PT_Keeper;
    }
    struct Computer2* comp = get_computer_player(plyr_idx);
    if (computer_player_invalid(comp)) {
        ERRORLOG("Tried to setup player %d which has no computer capability",(int)plyr_idx);
        return false;
    }
    memset(comp, 0, sizeof(struct Computer2));

    struct ComputerType* cpt = get_computer_type_template(comp_model);
    comp->dungeon = get_players_num_dungeon(plyr_idx);
    comp->model = comp_model;
    if (dungeon_invalid(comp->dungeon)) {
        WARNLOG("Tried to setup player %d which has no dungeon",(int)plyr_idx);
        comp->dungeon = INVALID_DUNGEON;
        comp->model = 0;
        return false;
    }
    comp->click_rate = cpt->click_rate;
    comp->processes_time = cpt->processes_time;
    comp->max_room_build_tasks = cpt->max_room_build_tasks;
    comp->turn_begin = cpt->turn_begin;
    comp->sim_before_dig = cpt->sim_before_dig;
    comp->action_status_flag = 1;
    comp->task_delay = cpt->drop_delay;
    comp->task_state = CTaskSt_Select;

    for (i=0; i < PLAYERS_COUNT; i++)
    {
        struct OpponentRelation* oprel = &comp->opponent_relations[i];
        oprel->last_interaction_turn = 0;
        oprel->next_idx = 0;
        if (i == plyr_idx) {
            oprel->hate_amount = INT32_MIN;
        } else {
            oprel->hate_amount = 0;
        }
    }
    comp->dig_stack_size = cpt->dig_stack_size;

    for (i=0; i < COMPUTER_PROCESSES_COUNT; i++)
    {
        struct ComputerProcess* cproc = &comp_player_conf.process_types[cpt->processes[i]];
        newproc = &comp->processes[i];
        if ((cproc == NULL) || (cproc->name[0] == '\0'))
        {
          newproc->name[0] = '\0';
          break;
        }
        memcpy(newproc, cproc, sizeof(struct ComputerProcess));
        newproc->parent = cpt->processes[i];
    }
    newproc = &comp->processes[i];
    newproc->flags |= ComProc_ListEnd;

    for (i=0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        struct ComputerCheck* ccheck = &comp_player_conf.check_types[cpt->checks[i]];
        newchk = &comp->checks[i];
        if ((ccheck == NULL) || (ccheck->name[0] == '\0'))
        {
            newchk->name[0] = '\0';
            break;
        }
        memcpy(newchk, ccheck, sizeof(struct ComputerCheck));
    }
    // Note that we don't have special, empty check at end of array
    // The check with 0x02 flag identifies end of active checks
    // (the check with 0x02 flag is invalid - only previous checks are in use)
    //newchk = &comp->checks[i];
    set_flag(newchk->flags, ComChk_Unkn0002);

    for (i=0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        struct ComputerEvent* event = &comp_player_conf.event_types[cpt->events[i]];
        struct ComputerEvent* newevnt = &comp->events[i];
        if ((event == NULL) || (event->name[0] == '\0'))
        {
            newevnt->name[0] = '\0';
            break;
        }
        memcpy(newevnt, event, sizeof(struct ComputerEvent));
    }
    return true;
}


TbBool script_support_setup_player_as_computer_keeper(PlayerNumber plyr_idx, long comp_model)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if (player_invalid(player) || player_is_neutral(plyr_idx)) {
        SCRPTWRNLOG("Tried to set up invalid player %d",(int)plyr_idx);
        return false;
    }
    // It uses >= because the count will be one higher than
    // the actual highest possible computer model number.
    if ((comp_model < 0) || (comp_model >= COMPUTER_MODELS_COUNT)) {
        SCRPTWRNLOG("Tried to set up player %d as outranged computer model %d",(int)plyr_idx,(int)comp_model);
        comp_model = 0;
    }
    player->allocflags |= PlaF_Allocated;
    player->id_number = plyr_idx;
    player->is_active = 1;
    player->allocflags |= PlaF_CompCtrl;
    init_player_start(player, false);
    if (!setup_a_computer_player(plyr_idx, comp_model)) {
        player->allocflags &= ~PlaF_CompCtrl;
        player->allocflags &= ~PlaF_Allocated;
        return false;
    }
    init_keeper_map_exploration_by_terrain(player);
    init_keeper_map_exploration_by_creatures(player);
    if (game.play_gameturn > 0)
    {
        check_map_for_gold();
    }
    return true;
}

void computer_check_events(struct Computer2 *comp)
{
    SYNCDBG(17,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    for (long i = 0; i < COMPUTER_EVENTS_COUNT; i++)
    {
        struct ComputerEvent* cevent = &comp->events[i];
        if (cevent->name[0] == '\0')
            break;
        switch (cevent->cetype)
        {
        case 0:
            if ((long)game.play_gameturn < (cevent->last_test_gameturn + cevent->test_interval))
            {
                break;
            }
            for (long n = 0; n < EVENTS_COUNT; n++)
            {
                struct Event* event = &game.event[n];
                if ( ((event->flags & EvF_Exists) != 0) &&
                      (event->owner == dungeon->owner) &&
                      (event->kind == cevent->mevent_kind) )
                {
                    if (computer_event_func_list[cevent->func_event](comp, cevent, event) == 1) {
                        SYNCDBG(5,"Player %d reacted on %s",(int)dungeon->owner,cevent->name);
                        cevent->last_test_gameturn = game.play_gameturn;
                    }
                }
            }
            break;
        case 1:
        case 2:
        case 3:
        case 4:
            if ((long)game.play_gameturn < (cevent->last_test_gameturn + cevent->test_interval)) {
                break;
            }
            {
                if (computer_event_test_func_list[cevent->func_test](comp,cevent) == 1) {
                    SYNCDBG(5,"Player %d reacted on %s",(int)dungeon->owner,cevent->name);
                }
                // Update test turn no matter if event triggered something
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
    SYNCDBG(17,"Starting");
    for (long i = 0; i < COMPUTER_CHECKS_COUNT; i++)
    {
        struct ComputerCheck* ccheck = &comp->checks[i];
        if (comp->tasks_did <= 0)
            break;
        if ((ccheck->flags & ComChk_Unkn0002) != 0)
            break;
        if ((ccheck->flags & ComChk_Unkn0001) == 0)
        {
            long delta = (game.play_gameturn - ccheck->last_run_turn);
            if ((delta > ccheck->turns_interval) && (computer_check_func_list[ccheck->func] != NULL))
            {
                SYNCDBG(8,"Executing check %ld, \"%s\"",i,ccheck->name);
                computer_check_func_list[ccheck->func](comp, ccheck);
                ccheck->last_run_turn = game.play_gameturn;
            }
        }
    }
    return true;
}

TbBool process_processes_and_task(struct Computer2 *comp)
{
  SYNCDBG(17,"Starting");
  for (int i = comp->tasks_did; i > 0; i--)
  {
    if (comp->tasks_did <= 0)
        return false;
    if ((game.play_gameturn % comp->click_rate) == 0)
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
    {
        if ((comp->ongoing_process > 0) && (comp->ongoing_process <= COMPUTER_PROCESSES_COUNT))
        {
            Comp_Process_Func callback = NULL;
            struct ComputerProcess* cproc = get_computer_process(comp, comp->ongoing_process);
            if (cproc != NULL) {
                callback = computer_process_func_list[cproc->func_task];
                SYNCDBG(7,"Performing process \"%s\"",cproc->name);
            } else {
                ERRORLOG("Invalid computer process %d referenced",(int)comp->ongoing_process);
            }
            if (callback != NULL) {
                callback(comp,cproc);
            }
        } else
        {
            ERRORLOG("No Process %d for a computer player",(int)comp->ongoing_process);
            comp->task_state = CTaskSt_Wait;
        }
        break;
    }
    default:
        ERRORLOG("Invalid task state %d",(int)comp->task_state);
        break;
    }
  }
  return true;
}

void process_computer_player2(PlayerNumber plyr_idx)
{
    SYNCDBG(7,"Starting for player %d",(int)plyr_idx);
    if (plyr_idx >= PLAYERS_COUNT) {
        return;
    }
    struct Computer2* comp = get_computer_player(plyr_idx);
    if (computer_player_invalid(comp)) {
        ERRORLOG("Player %d has no computer capability",(int)plyr_idx);
        return;
    }
    if (dungeon_invalid(comp->dungeon)) {
        ERRORLOG("Computer player %d has invalid dungeon",(int)plyr_idx);
        return;
    }
    if ((comp->processes_time != 0) && (comp->turn_begin <= game.play_gameturn))
      comp->tasks_did = 1;
    else
      comp->tasks_did = 0;
    if (comp->tasks_did <= 0) {
        return;
    }
    computer_check_events(comp);
    process_checks(comp);
    process_processes_and_task(comp);
    if (comp->tasks_did > 1) {
        ERRORLOG("Computer player %d performed %d tasks instead of up to one",(int)plyr_idx,(int)comp->tasks_did);
    }
}

struct ComputerProcess *computer_player_find_process_by_func_setup(PlayerNumber plyr_idx,Comp_Process_Func func_setup)
{
    struct Computer2* comp = get_computer_player(plyr_idx);
    if (computer_player_invalid(comp))
    {
        ERRORLOG("Player %d has no computer capability", (int)plyr_idx);
        return NULL;
  }
  struct ComputerProcess* cproc = &comp->processes[0];
  while (!flag_is_set(cproc->flags, ComProc_ListEnd))
  {
      if (computer_process_func_list[cproc->func_setup] == func_setup)
      {
          return cproc;
      }
      cproc++;
  }
  return NULL;
}

TbBool computer_player_demands_gold_check(PlayerNumber plyr_idx)
{
  //TODO COMPUTER_PLAYER comparing function pointers is a bad practice
  struct ComputerProcess* dig_process = computer_player_find_process_by_func_setup(plyr_idx, computer_setup_dig_to_gold);
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
  if (game.turn_last_checked_for_gold+GOLD_DEMAND_CHECK_INTERVAL < game.play_gameturn)
  {
      dig_process->flags &= ~ComProc_Unkn0004;
      return true;
  }
  return false;
}

void process_computer_players2(void)
{
    TbBool needs_gold_check = false;
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        struct Dungeon* dungeon = get_players_dungeon(player);
        if (!player_exists(player) || dungeon_invalid(dungeon))
            continue;
        if (((player->allocflags & PlaF_CompCtrl) != 0) || ((dungeon->computer_enabled & 0x01) != 0))
        {
          if (player->is_active == 1)
          {
            process_computer_player2(i);
            if (computer_player_demands_gold_check(i))
            {
                needs_gold_check = true;
            }
          }
        }
    }
    if (needs_gold_check)
    {
        SYNCDBG(0,"Computer players demand gold check.");
        check_map_for_gold();
    }
}

void setup_computer_players2(void)
{
  int i;
  game.turn_last_checked_for_gold = game.play_gameturn;
  check_map_for_gold();
  for (i=0; i < COMPUTER_TASKS_COUNT; i++)
  {
    memset(&game.computer_task[i], 0, sizeof(struct ComputerTask));
  }

  // Using a seed for rand() based on the current time, so that the same
  // random results aren't used in the same order every time.
  srand((unsigned) time(NULL));

#ifdef FUNCTESTING
  ftest_srand();
#endif
  struct PlayerInfo* player = INVALID_PLAYER;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->is_active == 1))
      {
        // The range from which the computer model is selected
        // is between minSkirmishAI and maxSkirmishAI, inclusive of both. User defined in keepcompp.cfg
        int minSkirmishAI = comp_player_conf.skirmish_first;
        int maxSkirmishAI = comp_player_conf.skirmish_last;

        int skirmish_AI_type = GAME_RANDOM(maxSkirmishAI + 1 - minSkirmishAI) + minSkirmishAI;
        if (i == game.local_plyr_idx)
        {
            skirmish_AI_type = comp_player_conf.player_assist_default;
        }
        setup_a_computer_player(i, skirmish_AI_type);
        if ((game.computer_chat_flags & CChat_TasksScarce) != 0)
        {
            message_add_fmt(MsgType_Player, i, "Ai model %d", skirmish_AI_type);
        }
        if (i != game.local_plyr_idx)
        {
            JUSTMSG("No model defined for Player %d, assigned computer model %d", i, skirmish_AI_type);
        }
      }
  }
}

void restore_computer_player_after_load(void)
{
    SYNCDBG(7,"Starting");
    for (long plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        struct Computer2* comp = get_computer_player(plyr_idx);
        if (computer_player_invalid(comp)) {
            ERRORLOG("Player %d has no computer capability",(int)plyr_idx);
            continue;
        }
        if (!player_exists(player)) {
            memset(comp, 0, sizeof(struct Computer2));
            comp->dungeon = INVALID_DUNGEON;
            continue;
        }
        if (player->is_active != 1)
        {
            memset(comp, 0, sizeof(struct Computer2));
            comp->dungeon = get_players_dungeon(player);
            continue;
        }
        comp->dungeon = get_players_dungeon(player);
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
