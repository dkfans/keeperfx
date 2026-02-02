/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file actionpt.c
 *     Action points support functions.
 * @par Purpose:
 *     Functions to maintain list of action points on map.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     21 May 2010 - 07 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "actionpt.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_planar.h"
#include "power_hand.h"
#include "player_instances.h"

#include "game_legacy.h"
#include "value_util.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
struct ActionPoint *action_point_get_free(void)
{
    for (ActionPointId apt_idx = 1; apt_idx < ACTN_POINTS_COUNT; apt_idx++)
    {
        struct ActionPoint* apt = &game.action_points[apt_idx];
        if (apt->exists == false)
            return apt;
    }
    return INVALID_ACTION_POINT;
}

struct ActionPoint *allocate_free_action_point_structure_with_number(ActionPointNumber apt_num)
{
    struct ActionPoint* apt = action_point_get_by_number(apt_num);
    if (action_point_exists(apt)) {
        ERRORLOG("Attempt to allocate action point over old one");
        return INVALID_ACTION_POINT;
    }
    if (action_point_is_invalid(apt)) {
        apt = action_point_get_free();
    }
    if (action_point_is_invalid(apt)) {
        ERRORLOG("No free action points to allocate");
        return INVALID_ACTION_POINT;
    }
    apt->exists = true;
    apt->num = apt_num;
    apt->activated = 0;
    return apt;
}

struct ActionPoint *actnpoint_create_actnpoint(struct InitActionPoint *iapt)
{
    struct ActionPoint* apt = allocate_free_action_point_structure_with_number(iapt->num);
    if (action_point_is_invalid(apt))
        return INVALID_ACTION_POINT;
    apt->mappos.x.val = iapt->mappos.x.val;
    apt->mappos.y.val = iapt->mappos.y.val;
    apt->range = iapt->range;
    return apt;
}

TbBool actnpoint_create_actnpoint_adv(VALUE *init_data)
{
    int point_number = value_int32(value_dict_get(init_data, "PointNumber"));
    if (point_number < 0)
        return false;

    struct ActionPoint* apt = allocate_free_action_point_structure_with_number(point_number);
    if (action_point_is_invalid(apt))
        return false;
    apt->mappos.x.val = value_read_stl_coord(value_dict_get(init_data, "SubtileX"));
    apt->mappos.y.val = value_read_stl_coord(value_dict_get(init_data, "SubtileY"));
    apt->range = value_read_stl_coord(value_dict_get(init_data, "PointRange"));
    return true;
}

struct ActionPoint *action_point_get(ActionPointId apt_idx)
{
    if ((apt_idx < 1) || (apt_idx >= ACTN_POINTS_COUNT))
        return INVALID_ACTION_POINT;
    return &game.action_points[apt_idx];
}

struct ActionPoint *action_point_get_by_number(ActionPointNumber apt_num)
{
    for (ActionPointId apt_idx = 0; apt_idx < ACTN_POINTS_COUNT; apt_idx++)
    {
        struct ActionPoint* apt = &game.action_points[apt_idx];
        if (apt->num == apt_num)
            return apt;
    }
    return INVALID_ACTION_POINT;
}

ActionPointId action_point_number_to_index(ActionPointNumber apt_num)
{
    for (ActionPointId apt_idx = 0; apt_idx < ACTN_POINTS_COUNT; apt_idx++)
    {
        struct ActionPoint* apt = &game.action_points[apt_idx];
        if (apt->num == apt_num)
            return apt_idx;
    }
    return -1;
}

TbBool action_point_is_invalid(const struct ActionPoint *apt)
{
    return (apt == INVALID_ACTION_POINT) || (apt == NULL);
}

TbBool action_point_exists(const struct ActionPoint *apt)
{
    if (action_point_is_invalid(apt))
        return false;
    return apt->exists;
}

TbBool action_point_exists_idx(ActionPointId apt_idx)
{
    struct ActionPoint* apt = action_point_get(apt_idx);
    if (action_point_is_invalid(apt))
        return false;
    return apt->exists;
}

TbBool action_point_reset_idx(ActionPointId apt_idx, PlayerNumber plyr_idx)
{
    struct ActionPoint* apt = action_point_get(apt_idx);
    if (action_point_is_invalid(apt))
        return false;
    if (plyr_idx == ALL_PLAYERS)
    {
        apt->activated = 0;
    }
    else
    {
        clear_flag(apt->activated, to_flag(plyr_idx));
    }
    return apt->exists;
}

/**
 * Returns if the action point of given index was triggered by given player.
 */
TbBool action_point_activated_by_player(ActionPointId apt_idx, PlayerNumber plyr_idx)
{
    struct ActionPoint* apt = action_point_get(apt_idx);
    return flag_is_set(apt->activated, to_flag(plyr_idx));
}

TbBool action_point_is_creature_from_list_within(const struct ActionPoint *apt, long first_thing_idx)
{
    SYNCDBG(8,"Starting");
    unsigned long k = 0;
    int i = first_thing_idx;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature (%d) detected", i);
            break;
        }
        i = cctrl->players_next_creature_idx;
        if (thing_is_picked_up(thing))
        {
            continue;
        }
        if (creature_is_being_unconscious(thing) || thing_is_dragged_or_pulled(thing) || creature_is_being_dropped(thing))
        {
            continue;
        }
        // Thing list loop body
        // Range of 0 means activate when on the same subtile
        if (apt->range <= 0)
        {
            if ((apt->mappos.x.stl.num == thing->mappos.x.stl.num)
             && (apt->mappos.y.stl.num == thing->mappos.y.stl.num)) {
                return true;
            }
        } else
        {
            long dist = get_distance_xy(thing->mappos.x.val, thing->mappos.y.val, apt->mappos.x.val, apt->mappos.y.val);
            if (apt->range > dist) {
                return true;
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
    return false;
}

PlayerBitFlags action_point_get_players_within(long apt_idx)
{
    struct ActionPoint* apt = action_point_get(apt_idx);
    PlayerBitFlags activated = apt->activated;
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct PlayerInfo* player = get_player(plyr_idx);
        if (player_exists(player))
        {
            if (!flag_is_set(activated, to_flag(plyr_idx)))
            {
                struct Dungeon* dungeon = get_players_dungeon(player);
                if (dungeon_invalid(dungeon)) {
                    continue;
                }
                SYNCDBG(16,"Checking player %d",(int)plyr_idx);
                if (action_point_is_creature_from_list_within(apt, dungeon->digger_list_start)) {
                    set_flag(activated, to_flag(plyr_idx));
                    continue;
                }
                if (action_point_is_creature_from_list_within(apt, dungeon->creatr_list_start)) {
                    set_flag(activated, to_flag(plyr_idx));
                    continue;
                }
            }
        }
    }
    return activated;
}

TbBool process_action_points(void)
{
    SYNCDBG(6,"Starting");
    for (long i = 1; i < ACTN_POINTS_COUNT; i++)
    {
        struct ActionPoint* apt = &game.action_points[i];
        if (apt->exists == true)
        {
            if (((apt->num + game.play_gameturn) & 7) == 0)
            {
                apt->activated = action_point_get_players_within(i);
                //if (i==1) show_onscreen_msg(2*game.num_fps, "APT PLYRS %d", (int)apt->activated);
            }
      }
    }
    return true;
}

void clear_action_points(void)
{
    for (long i = 0; i < ACTN_POINTS_COUNT; i++)
    {
        memset(&game.action_points[i], 0, sizeof(struct ActionPoint));
    }
}

void delete_action_point_structure(struct ActionPoint *apt)
{
    if (apt->exists == true)
    {
        memset(apt, 0, sizeof(struct ActionPoint));
    }
}

void delete_all_action_point_structures(void)
{
    for (long i = 1; i < ACTN_POINTS_COUNT; i++)
    {
        struct ActionPoint* apt = &game.action_points[i];
        if (apt != NULL)
        {
            delete_action_point_structure(apt);
      }
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
