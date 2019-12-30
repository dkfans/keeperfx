/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_groups.c
 *     Creature grouping and groups support functions.
 * @par Purpose:
 *     Functions to creature_groups.
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
#include "creature_groups.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_math.h"
#include "thing_list.h"
#include "thing_creature.h"
#include "thing_physics.h"
#include "creature_control.h"
#include "creature_states.h"
#include "config_creature.h"
#include "room_jobs.h"
#include "ariadne_wallhug.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/******************************************************************************/
struct Thing *get_highest_experience_and_score_creature_in_group(struct Thing *grptng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(grptng);
    CrtrExpLevel best_explevel;
    long best_score;
    struct Thing *best_creatng;
    best_explevel = 0;
    best_score = 0;
    best_creatng = INVALID_THING;
    long i;
    unsigned long k;
    i = cctrl->group_info & TngGroup_LeaderIndex;
    if (i == 0) {
        // One creature is not a group, but we may still get its experience
        i = grptng->index;
    }
    k = 0;
    while (i > 0)
    {
        struct Thing *ctng;
        ctng = thing_get(i);
        TRACE_THING(ctng);
        cctrl = creature_control_get_from_thing(ctng);
        if (creature_control_invalid(cctrl))
            break;
        // Per-thing code
        if (best_explevel <= cctrl->explevel) {
            long score;
            score = get_creature_thing_score(ctng);
            // If got a new best score, or best level changed - update best values
            if ((best_score < score) || (best_explevel < cctrl->explevel)) {
                best_explevel = cctrl->explevel;
                best_score = score;
                best_creatng = ctng;
            }
        }
        // Per-thing code ends
        i = cctrl->next_in_group;
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures group");
            break;
        }
    }
    return best_creatng;
}

long get_no_creatures_in_group(const struct Thing *grptng)
{
    struct CreatureControl *cctrl;
    struct Thing *ctng;
    long i;
    unsigned long k;
    cctrl = creature_control_get_from_thing(grptng);
    i = cctrl->group_info & TngGroup_LeaderIndex;
    if (i == 0) {
        // No group - just one creature
        return 1;
    }
    k = 0;
    while (i > 0)
    {
        ctng = thing_get(i);
        TRACE_THING(ctng);
        cctrl = creature_control_get_from_thing(ctng);
        if (creature_control_invalid(cctrl))
            break;
        i = cctrl->next_in_group;
        k++;
        if (k > GROUP_MEMBERS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures group");
            break;
        }
    }
    return k;
}

struct Thing *get_last_follower_creature_in_group(const struct Thing *grptng)
{
    struct CreatureControl *cctrl;
    struct Thing *ctng;
    long i;
    unsigned long k;
    cctrl = creature_control_get_from_thing(grptng);
    i = cctrl->group_info & TngGroup_LeaderIndex;
    if (i == 0) {
        // No group - just one creature
        return INVALID_THING;
    }
    k = 0;
    while (i > 0)
    {
        ctng = thing_get(i);
        TRACE_THING(ctng);
        cctrl = creature_control_get_from_thing(ctng);
        if (creature_control_invalid(cctrl))
            break;
        i = cctrl->next_in_group;
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures group");
            break;
        }
    }
    return ctng;
}

struct Thing *get_first_follower_creature_in_group(const struct Thing *grptng)
{
    struct CreatureControl *cctrl;
    struct Thing *ctng;
    long i;
    cctrl = creature_control_get_from_thing(grptng);
    i = cctrl->group_info & TngGroup_LeaderIndex;
    if (i == 0) {
        // No group - just one creature
        return INVALID_THING;
    }
    {
        ctng = thing_get(i);
        cctrl = creature_control_get_from_thing(ctng);
        i = cctrl->next_in_group;
    }
    return thing_get(i);
}

struct Thing *get_group_leader(const struct Thing *thing)
{
  struct CreatureControl *cctrl;
  struct Thing *leadtng;
  cctrl = creature_control_get_from_thing(thing);
  leadtng = thing_get(cctrl->group_info & TngGroup_LeaderIndex);
  return leadtng;
}

TbBool creature_is_group_member(const struct Thing *thing)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(thing);
    return ((cctrl->group_info & TngGroup_LeaderIndex) > 0);
}

TbBool creature_is_group_leader(const struct Thing *thing)
{
    struct Thing *leadtng;
    //return _DK_creature_is_group_leader(thing);
    leadtng = get_group_leader(thing);
    if (thing_is_invalid(leadtng))
        return false;
    return (leadtng->index == thing->index);
}

void internal_update_leader_index_in_group(struct Thing *leadtng)
{
    long i;
    unsigned long k;
    i = leadtng->index;
    k = 0;
    while (i > 0)
    {
        struct CreatureControl *cctrl;
        struct Thing *ctng;
        ctng = thing_get(i);
        TRACE_THING(ctng);
        cctrl = creature_control_get_from_thing(ctng);
        if (creature_control_invalid(cctrl))
            break;
        i = cctrl->next_in_group;
        // Per-thing code
        cctrl->group_info ^= (leadtng->index ^ cctrl->group_info) & TngGroup_LeaderIndex;
        // Per-thing code ends
        k++;
        if (k > GROUP_MEMBERS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures group");
            break;
        }
    }
    SYNCDBG(7,"Group led by %s index %d has %d members",thing_model_name(leadtng),(int)leadtng->index,(int)k);
}

void internal_remove_member_from_group_chain(struct Thing *creatng)
{
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    struct Thing *prevtng;
    struct CreatureControl *prevctrl;
    if (cctrl->prev_in_group > 0) {
        prevtng = thing_get(cctrl->prev_in_group);
        prevctrl = creature_control_get_from_thing(prevtng);
    } else {
        prevtng = INVALID_THING;
        prevctrl = INVALID_CRTR_CONTROL;
    }
    struct Thing *nexttng;
    struct CreatureControl *nextctrl;
    if (cctrl->next_in_group > 0) {
        nexttng = thing_get(cctrl->next_in_group);
        nextctrl = creature_control_get_from_thing(nexttng);
    } else {
        nexttng = INVALID_THING;
        nextctrl = INVALID_CRTR_CONTROL;
    }
    // Remove current thing from group chain
    if (!creature_control_invalid(prevctrl))
    {
        if (!thing_is_invalid(nexttng)) {
            prevctrl->next_in_group = nexttng->index;
        } else {
            prevctrl->next_in_group = 0;
        }
    }
    if (!creature_control_invalid(nextctrl))
    {
        if (!thing_is_invalid(prevtng)) {
            nextctrl->prev_in_group = prevtng->index;
        } else {
            nextctrl->prev_in_group = 0;
        }
    }
    cctrl->next_in_group = 0;
    cctrl->prev_in_group = 0;
}

/**
 * Adds member to a group chain at its head.
 * @param creatng New group chain head.
 * @param leadtng Previous group chain head.
 */
void internal_add_member_to_group_chain_head(struct Thing *creatng, struct Thing *leadtng)
{
    // Change old leader to normal group member, and add new one to chain as its head
    struct CreatureControl *crctrl;
    crctrl = creature_control_get_from_thing(creatng);
    struct CreatureControl *ldctrl;
    ldctrl = creature_control_get_from_thing(leadtng);
    crctrl->next_in_group = leadtng->index;
    ldctrl->prev_in_group = creatng->index;
    // Remove member count from old leader; new leader will have it computed somewhere else
    ldctrl->group_info &= ~TngGroup_MemberCount;
}

/**
 * Removes a creature from group. If the group had size of 2, it is disbanded; otherwise, next
 *   creature becomes a leader without checking whether it's best for it.
 * @param creatng The creatuire to be removed.
 * @return True if the group still exists after removal, false otherwise.
 */
TbBool remove_creature_from_group_without_leader_consideration(struct Thing *creatng)
{
    //return _DK_remove_creature_from_group(creatng);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    // Remember any other group member, and whether we're removing a leader
    TbBool was_leader;
    struct Thing *grptng;
    if (creature_is_group_leader(creatng)) {
        was_leader = true;
        // If we're removing leader, then all members are next
        grptng = thing_get(cctrl->next_in_group);
    } else {
        was_leader = false;
        // If we're removing follower, then at least a leader has to be previous
        grptng = thing_get(cctrl->prev_in_group);
    }
    // Set new next and previous things
    internal_remove_member_from_group_chain(creatng);
    // Finish removing the thing from group
    cctrl->group_info &= ~TngGroup_LeaderIndex;
    cctrl->group_info &= ~TngGroup_MemberCount;
    creatng->alloc_flags &= ~TAlF_IsFollowingLeader;
    // Find leader of the party
    struct Thing *leadtng;
    if (was_leader) {
        // Leader was at start of the chain, and we've removed him
        leadtng = grptng;
    } else {
        // Leader hasn't changed, we can use group_info to get him
        leadtng = get_group_leader(grptng);
    }
    // If there are no creatures besides leader, disband the group
    cctrl = creature_control_get_from_thing(leadtng);
    if (creature_control_invalid(cctrl) || (cctrl->next_in_group <= 0))
    {
        cctrl->next_in_group = 0;
        cctrl->prev_in_group = 0;
        cctrl->group_info &= ~TngGroup_LeaderIndex;
        cctrl->group_info &= ~TngGroup_MemberCount;
        if (creature_control_invalid(cctrl)) {
            WARNLOG("Group had only one member, %s index %d",thing_model_name(creatng),(int)creatng->index);
        }
        leadtng->alloc_flags &= ~TAlF_IsFollowingLeader;
        return false;
    }
    // If there is still more than one creature
    if (was_leader) {
        internal_update_leader_index_in_group(leadtng);
        leadtng->alloc_flags &= ~TAlF_IsFollowingLeader;
        leader_find_positions_for_followers(leadtng);
    }
    return true;
}

TbBool remove_creature_from_group(struct Thing *creatng)
{
    struct Thing *grptng;
    grptng = get_first_follower_creature_in_group(creatng);
    if (!remove_creature_from_group_without_leader_consideration(creatng)) {
        SYNCDBG(5,"Removing %s index %d and disbanding the party",thing_model_name(creatng),(int)creatng->index);
        // Last creature removed - party disbanded
        return false;
    }
    SYNCDBG(5,"Removing %s index %d",thing_model_name(creatng),(int)creatng->index);
    struct Thing *leadtng;
    leadtng = get_highest_experience_and_score_creature_in_group(grptng);
    make_group_member_leader(leadtng);
    return true;
}

TbBool add_creature_to_group(struct Thing *creatng, struct Thing *grptng)
{
    SYNCDBG(5,"Adding %s index %d",thing_model_name(creatng),(int)creatng->index);
    struct Thing *pvthing;
    pvthing = get_last_follower_creature_in_group(grptng);
    if ((grptng->index == creatng->index) || (grptng->owner != creatng->owner)) {
        return false;
    }
    struct CreatureControl *crctrl;
    crctrl = creature_control_get_from_thing(creatng);
    if (creature_is_group_member(creatng)) {
        remove_creature_from_group(creatng);
    }
    if (thing_exists(pvthing))
    {
        // If we already have a group, place the new creature at end
        struct CreatureControl *pvctrl;
        crctrl = creature_control_get_from_thing(creatng);
        crctrl->prev_in_group = pvthing->index;
        pvctrl = creature_control_get_from_thing(pvthing);
        pvctrl->next_in_group = creatng->index;
        crctrl->group_info ^= (crctrl->group_info ^ pvctrl->group_info) & TngGroup_LeaderIndex;
        crctrl->next_in_group = 0;
        crctrl->group_info &= ~TngGroup_MemberCount;
    } else
    {
        // If there's no group, create new one made of both creatures
        struct CreatureControl *grctrl;
        crctrl = creature_control_get_from_thing(creatng);
        grctrl = creature_control_get_from_thing(grptng);
        crctrl->prev_in_group = grptng->index;
        grctrl->next_in_group = creatng->index;
        crctrl->group_info ^= (crctrl->group_info ^ grptng->index) & TngGroup_LeaderIndex;
        grctrl->group_info ^= (grctrl->group_info ^ grptng->index) & TngGroup_LeaderIndex;
        crctrl->next_in_group = 0;
        grctrl->prev_in_group = 0;
        // Remove member count from non-leader creature; the leader will have it computed somewhere else
        crctrl->group_info &= ~TngGroup_MemberCount;
    }
    creatng->alloc_flags |= TAlF_IsFollowingLeader;
    return true;
}

long add_creature_to_group_as_leader(struct Thing *creatng, struct Thing *grptng)
{
    //return _DK_add_creature_to_group_as_leader(creatng, grptng);
    SYNCDBG(5,"Adding %s index %d",thing_model_name(creatng),(int)creatng->index);
    if ((grptng->index == creatng->index) || (grptng->owner != creatng->owner)) {
        return 0;
    }
    if (creature_is_group_member(creatng)) {
        remove_creature_from_group(creatng);
    }
    struct Thing *leadtng;
    leadtng = get_group_leader(grptng);
    if (thing_is_invalid(leadtng))
        leadtng = grptng;
    // Change old leader to normal group member, and add new one to chain as its head
    internal_add_member_to_group_chain_head(creatng, leadtng);
    leadtng->alloc_flags |= TAlF_IsFollowingLeader;
    // Now go through all group members and set leader index
    internal_update_leader_index_in_group(creatng);
    return 1;
}

struct Party *get_party_of_name(const char *prtname)
{
    struct Party *party;
    int i;
    for (i = 0; i < game.script.creature_partys_num; i++)
    {
        party = &game.script.creature_partys[i];
        if (strcasecmp(party->prtname, prtname) == 0)
            return party;
    }
    return NULL;
}

int get_party_index_of_name(const char *prtname)
{
    struct Party *party;
    int i;
    for (i = 0; i < game.script.creature_partys_num; i++)
    {
        party = &game.script.creature_partys[i];
        if (strcasecmp(party->prtname, prtname) == 0)
            return i;
    }
    return -1;
}

TbBool create_party(const char *prtname)
{
    struct Party *party;
    if (game.script.creature_partys_num >= CREATURE_PARTYS_COUNT)
    {
        SCRPTERRLOG("Too many partys in script");
        return false;
    }
    party = (&game.script.creature_partys[game.script.creature_partys_num]);
    strncpy(party->prtname, prtname, sizeof(party->prtname));
    party->members_num = 0;
    game.script.creature_partys_num++;
    return true;
}

TbBool add_member_to_party_name(const char *prtname, long crtr_model, long crtr_level, long carried_gold, long objctv_id, long countdown)
{
    struct Party *party;
    struct PartyMember *member;
    party = get_party_of_name(prtname);
    if (party == NULL)
    {
      SCRPTERRLOG("Party of requested name, '%s', is not defined", prtname);
      return false;
    }
    if (party->members_num >= GROUP_MEMBERS_COUNT)
    {
      SCRPTERRLOG("Too many creatures in party '%s' (limit is %d members)",
          prtname, GROUP_MEMBERS_COUNT);
      return false;
    }
    member = &(party->members[party->members_num]);
    member->flags &= ~TrgF_DISABLED;
    member->crtr_kind = crtr_model;
    member->carried_gold = carried_gold;
    member->crtr_level = crtr_level-1;
    member->field_6F = 1;
    member->objectv = objctv_id;
    member->countdown = countdown;
    party->members_num++;
    return true;
}

TbBool make_group_member_leader(struct Thing *leadtng)
{
    struct Thing *prvtng;
    prvtng = get_group_leader(leadtng);
    if (thing_is_invalid(prvtng))
        return false;
    SYNCDBG(3,"Group owned by player %d leader change to %s index %d",
        (int)leadtng->owner,thing_model_name(leadtng),(int)leadtng->index);
    if (prvtng->index != leadtng->index)
    {
        remove_creature_from_group_without_leader_consideration(leadtng);
        add_creature_to_group_as_leader(leadtng, prvtng);
        return true;
    }
    return false;
}

TbBool get_free_position_behind_leader(struct Thing *leadtng, struct Coord3d *pos)
{
    struct CreatureControl *leadctrl;
    leadctrl = creature_control_get_from_thing(leadtng);
    int i, group_len;
    group_len = (leadctrl->group_info >> 12);
    for (i = 0; i < group_len; i++)
    {
        struct MemberPos *avail_pos;
        avail_pos = &leadctrl->followers_pos[i];
        if (((avail_pos->flags & 0x02) != 0) && ((avail_pos->flags & 0x01) == 0))
        {
            pos->x.val = subtile_coord_center(stl_num_decode_x(avail_pos->stl_num));
            pos->y.val = subtile_coord_center(stl_num_decode_y(avail_pos->stl_num));
            pos->z.val = 0;
            avail_pos->flags |= 0x01;
            return true;
        }
    }
    WARNDBG(3,"Group led by %s index %d owned by player %d had all %d follower positions taken",
        thing_model_name(leadtng),(int)leadtng->index,(int)leadtng->owner,group_len);
    return false;
}

long process_obey_leader(struct Thing *thing)
{
    struct Thing *leadtng;
    leadtng = get_group_leader(thing);
    if (thing_is_invalid(leadtng)) {
        WARNDBG(3,"Leader invalid, resetting %s index %d owned by player %d",
            thing_model_name(thing),(int)thing->index,(int)thing->owner);
        set_start_state(thing);
        return 1;
    }
    if ((leadtng->alloc_flags & TAlF_IsControlled) != 0)
    {
        // If leader is controlled, always force followers to stay
        if (thing->active_state != CrSt_CreatureFollowLeader) {
            external_set_thing_state(thing, CrSt_CreatureFollowLeader);
        }
        return 1;
    }
    struct CreatureControl *cctrl;
    struct CreatureControl *leadctrl;
    struct StateInfo *stati;
    stati = get_creature_state_with_task_completion(leadtng);
    switch (stati->field_21)
    {
    case 1:
        if (thing->active_state != CrSt_CreatureFollowLeader) {
            external_set_thing_state(thing, CrSt_CreatureFollowLeader);
        }
        break;
    case 2:
        cctrl = creature_control_get_from_thing(thing);
        leadctrl = creature_control_get_from_thing(leadtng);
        if ((cctrl->work_room_id != leadctrl->work_room_id) && (cctrl->target_room_id != leadctrl->work_room_id))
        {
            struct Room *room;
            room = get_room_creature_works_in(leadtng);
            struct CreatureStats *crstat;
            crstat = creature_stats_get_from_thing(thing);
            CreatureJob jobpref;
            jobpref = get_job_for_room(room->kind, JoKF_None, crstat->job_primary|crstat->job_secondary);
            cleanup_current_thing_state(thing);
            send_creature_to_room(thing, room, jobpref);
        }
        break;
    default:
        break;
    }
    return 1;
}

void creature_follower_pos_add(struct Thing *creatng, int ifollow, const struct Coord3d *pos)
{
    struct MemberPos *avail_pos;
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(creatng);
    avail_pos = &cctrl->followers_pos[ifollow];
    avail_pos->stl_num = get_subtile_number(pos->x.stl.num, pos->y.stl.num);
    avail_pos->flags |= 0x02;
}

void leader_find_positions_for_followers(struct Thing *leadtng)
{
    //_DK_leader_find_positions_for_followers(thing);
    int group_len;
    group_len = get_no_creatures_in_group(leadtng);
    struct CreatureControl *cctrl;
    cctrl = creature_control_get_from_thing(leadtng);
    // Base the position update frequency on leader move speed; speed of 48 requires refresh per 32 turns
    int recompute_interval;
    recompute_interval = 32*48 / (get_creature_speed(leadtng)+1);
    if (recompute_interval > 256) {
        recompute_interval = 256;
    } else if (recompute_interval < 4) {
        recompute_interval = 4;
    }
    if (((cctrl->group_info >> 12) == group_len) && (((game.play_gameturn + leadtng->index) % recompute_interval) != 0))
    {
        SYNCDBG(7,"Reusing positions for %d followers of %s index %d owned by player %d",
            group_len,thing_model_name(leadtng),(int)leadtng->index,(int)leadtng->owner);
        int i;
        for (i=0; i < GROUP_MEMBERS_COUNT; i++)
        {
          cctrl->followers_pos[i].flags &= ~0x01;
        }
        return;
    }
    SYNCDBG(7,"Finding positions for %d followers of %s index %d owned by player %d",
        group_len,thing_model_name(leadtng),(int)leadtng->index,(int)leadtng->owner);
    cctrl->group_info = (group_len << 12) | (cctrl->group_info & ~TngGroup_MemberCount);
    memset(cctrl->followers_pos, 0, sizeof(cctrl->followers_pos));

    int len_xv, len_yv;
    int len_xh, len_yh;
    len_xv = LbSinL(leadtng->move_angle_xy + LbFPMath_PI) << 8 >> 16;
    len_yv = -((LbCosL(leadtng->move_angle_xy + LbFPMath_PI) << 8) >> 8) >> 8;
    len_xh = LbSinL(leadtng->move_angle_xy - LbFPMath_PI/2) << 8 >> 16;
    len_yh = -((LbCosL(leadtng->move_angle_xy - LbFPMath_PI/2) << 8) >> 8) >> 8;

    int ih, iv, ivmax;
    ivmax = 2 * group_len;
    int ifollow;
    ifollow = 0;

    int shift_xh, shift_yh;
    int shift_xv, shift_yv;
    int shift_xh_beg, shift_yh_beg;
    int delta_xh, delta_yh;
    int delta_xv, delta_yv;

    delta_yh = 2 * len_yh;
    delta_xh = 2 * len_xh;
    shift_yv = 2 * len_yv;
    delta_yv = 2 * len_yv;
    shift_yh_beg = -2 * len_yh;
    shift_xh_beg = -2 * len_xh;
    shift_xv = 2 * len_xv;
    delta_xv = 2 * len_xv;
    for (iv = 2; iv <= ivmax; iv += 2)
    {
        shift_yh = shift_yh_beg;
        shift_xh = shift_xh_beg;
        for (ih = -2; ih <= 2; ih += 2)
        {
            int mcor_x, mcor_y;
            mcor_x = leadtng->mappos.x.val + shift_xh + shift_xv;
            mcor_y = leadtng->mappos.y.val + shift_yv + shift_yh;
            if ((coord_slab(mcor_x) > 0) && (coord_slab(mcor_x) < map_tiles_x))
            {
                if ((coord_slab(mcor_y) > 0) && (coord_slab(mcor_y) < map_tiles_y))
                {
                    struct Coord3d pos;
                    pos.x.val = mcor_x;
                    pos.y.val = mcor_y;
                    pos.z.val = get_floor_height_at(&pos);
                    // if position is ok for leader instead of for followers - close enough
                    if (!thing_in_wall_at(leadtng, &pos) && !terrain_toxic_for_creature_at_position(leadtng, pos.x.stl.num, pos.y.stl.num))
                    {
                        creature_follower_pos_add(leadtng, ifollow, &pos);
                        ifollow++;
                        // If we're not able to store more coordinates, quit now
                        if (ifollow >= group_len) {
                          return;
                        }
                    }
                }
            }
            shift_yh += delta_yh;
            shift_xh += delta_xh;
        }
        shift_yv += delta_yv;
        shift_xv += delta_xv;
    }

    shift_yv = len_yv;
    delta_yh = 2 * len_yh;
    delta_yv = 2 * len_yv;
    delta_xh = 2 * len_xh;
    shift_xv = len_xv;
    delta_xv = 2 * len_xv;
    shift_yh_beg = -len_yh;
    shift_xh_beg = -len_xh;
    for (iv = 1; iv <= ivmax; iv += 2)
    {
        shift_yh = shift_yh_beg;
        shift_xh = shift_xh_beg;
        for (ih = -1; ih <= 2; ih += 2)
        {
            int mcor_x, mcor_y;
            mcor_x = leadtng->mappos.x.val + shift_xh + shift_xv;
            mcor_y = leadtng->mappos.y.val + shift_yv + shift_yh;
            if ((coord_slab(mcor_x) > 0) && (coord_slab(mcor_x) < map_tiles_x))
            {
                if ((coord_slab(mcor_y) > 0) && (coord_slab(mcor_y) < map_tiles_y))
                {
                    struct Coord3d pos;
                    pos.x.val = mcor_x;
                    pos.y.val = mcor_y;
                    pos.z.val = get_floor_height_at(&pos);
                    if (!thing_in_wall_at(leadtng, &pos))
                    {
                        creature_follower_pos_add(leadtng, ifollow, &pos);
                        ifollow++;
                        // If we're not able to store more coordinates, quit now
                        if (ifollow >= group_len) {
                          return;
                        }
                    }
                }
            }
            shift_yh += delta_yh;
            shift_xh += delta_xh;
        }
        shift_yv += delta_yv;
        shift_xv += delta_xv;
    }
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
