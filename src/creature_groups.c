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
#include "pre_inc.h"
#include "creature_groups.h"

#include "globals.h"
#include "bflib_basics.h"

#include "bflib_math.h"
#include "thing_navigate.h"
#include "thing_list.h"
#include "thing_creature.h"
#include "thing_physics.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_states_hero.h"
#include "config_creature.h"
#include "room_jobs.h"
#include "ariadne_wallhug.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

long get_no_creatures_in_group(const struct Thing *grptng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(grptng);
    long i = cctrl->group_leader_idx;
    if (i == 0) {
        // No group - just one creature
        return 1;
    }
    unsigned long k = 0;
    while (i > 0)
    {
        struct Thing* ctng = thing_get(i);
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
    struct Thing* ctng = NULL;
    struct CreatureControl* cctrl = creature_control_get_from_thing(grptng);
    long i = cctrl->group_leader_idx;
    if (i == 0) {
        // No group - just one creature
        return INVALID_THING;
    }
    unsigned long k = 0;
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(grptng);
    long i = cctrl->group_leader_idx;
    if (i == 0) {
        // No group - just one creature
        return INVALID_THING;
    }
    {
        struct Thing* ctng = thing_get(i);
        cctrl = creature_control_get_from_thing(ctng);
        i = cctrl->next_in_group;
    }
    return thing_get(i);
}

struct Thing *get_group_leader(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    struct Thing* leadtng = thing_get(cctrl->group_leader_idx);
    return leadtng;
}

TbBool creature_is_group_member(const struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    return (cctrl->group_leader_idx > 0);
}

TbBool creature_is_group_leader(const struct Thing *thing)
{
    struct Thing* leadtng = get_group_leader(thing);
    if (thing_is_invalid(leadtng))
        return false;
    return (leadtng->index == thing->index);
}

void internal_update_leader_index_in_group(struct Thing *leadtng)
{
    long i = leadtng->index;
    unsigned long k = 0;
    while (i > 0)
    {
        struct Thing* ctng = thing_get(i);
        TRACE_THING(ctng);
        struct CreatureControl* cctrl = creature_control_get_from_thing(ctng);
        if (creature_control_invalid(cctrl))
            break;
        i = cctrl->next_in_group;
        // Per-thing code
        cctrl->group_leader_idx = leadtng->index;
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
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
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
    struct CreatureControl* crctrl = creature_control_get_from_thing(creatng);
    struct CreatureControl* ldctrl = creature_control_get_from_thing(leadtng);
    crctrl->next_in_group = leadtng->index;
    ldctrl->prev_in_group = creatng->index;
    // Remove member count from old leader; new leader will have it computed somewhere else
    ldctrl->group_member_count = 0;
}

/**
 * Removes a creature from group. If the group had size of 2, it is disbanded; otherwise, next
 *   creature becomes a leader without checking whether it's best for it.
 * @param creatng The creature to be removed.
 * @return True if the group still exists after removal, false otherwise.
 */
TbBool remove_creature_from_group_without_leader_consideration(struct Thing *creatng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
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
    cctrl->group_leader_idx = 0;
    cctrl->group_member_count = 0;
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
        cctrl->group_leader_idx = 0;
        cctrl->group_member_count = 0;
        if (creature_control_invalid(cctrl)) {
            WARNLOG("Group had only one member, %s index %d",thing_model_name(creatng),(int)creatng->index);
        }
        leadtng->alloc_flags &= ~TAlF_IsFollowingLeader;
        CrtrStateId i = get_creature_state_besides_interruptions(leadtng);
        if (i == CrSt_CreatureFollowLeader) 
        {
            set_start_state(leadtng);
        }
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

/**
 * Determines if the creature is a Tunneler or Imp to consider for leadership.
  * @return 0 if it's no digger, 1 if it's a digger who does not want to be a leader, and 2 if the digger is a preferred leader
 */
static short creature_could_be_lead_digger(struct Thing* creatng, struct CreatureControl* cctrl)
{
    short potential_leader = 0;
    if (thing_is_creature_digger(creatng))
    {
        if (cctrl->party.objective != CHeroTsk_DefendParty)
        {
            potential_leader = 2;
        }
        else
        {
            potential_leader = 1;
        }
    }
    return potential_leader;
}

/**
 * Determines if a party has a Tunneler or Imp to consider for leadership.
 * @param grptng is the creature whos party is considerd
 * @return 0 if there's no digger, 1 if there's a digger who does not want to be a leader, and 2 if the digger is a preferred leader
 */
static short creatures_group_has_special_digger_to_lead(struct Thing* grptng)
{
    struct Thing* ctng = INVALID_THING;
    short potential_leader = 0;
    struct CreatureControl* cctrl;
    cctrl = creature_control_get_from_thing(grptng);
    potential_leader = creature_could_be_lead_digger(grptng, cctrl);
    if (potential_leader == 2)
    {
        return potential_leader;
    }
    long i = cctrl->group_leader_idx;
    unsigned long k = 0;
    if (i == 0)
    {
        i = grptng->index;
    }
    while (i > 0)
    {
        ctng = thing_get(i);
        if (!thing_is_creature(ctng))
        {
            ERRORLOG("Invalid creature in group %s index %d", thing_model_name(grptng), (int)grptng->index);
            return potential_leader;
        }

        cctrl = creature_control_get_from_thing(ctng);
        potential_leader = creature_could_be_lead_digger(ctng, cctrl);
        if (potential_leader == 2)
        {
            return potential_leader;
        }
        i = cctrl->next_in_group;
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures group");
            break;
        }
    }
    return potential_leader;
}

/**
 * Finds a creature to become group leader. Considers objectives, diggers and score.
 * @param grptng is the party member which needs a leader
 * @return The Creature that should lead the group.
 */
struct Thing* get_best_creature_to_lead_group(struct Thing* grptng)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(grptng);
    CrtrExpLevel best_exp_level = 0;
    long best_score = 0;
    short has_digger = 0;
    TbBool is_digger = 0;
    struct Thing* best_creatng = INVALID_THING;
    long i = cctrl->group_leader_idx;
    if (i == 0) {
        // One creature is not a group, but we may still get its experience
        i = grptng->index;
    }
    has_digger = creatures_group_has_special_digger_to_lead(grptng);
    unsigned long k = 0;
    while (i > 0)
    {
        struct Thing* ctng = thing_get(i);
        TRACE_THING(ctng);
        if (has_digger > 0)
        {
            is_digger = thing_is_creature_digger(ctng);
        }
        cctrl = creature_control_get_from_thing(ctng);
        struct CreatureControl* bcctrl = creature_control_get_from_thing(best_creatng);
        if (creature_control_invalid(cctrl))
        {
            break;
        }
        // Per-thing code
        long score = get_creature_thing_score(ctng);
        // Units who are supposed to defend the party, are considered for party leadership last.
        if (cctrl->party.objective != CHeroTsk_DefendParty)
        {
            if (has_digger < 2 || is_digger) // if we want a digger, do not consider non-diggers
            {
                // If the current unit does not defend party, overwrite any unit that does.
                if (bcctrl->party.objective == CHeroTsk_DefendParty)
                {
                    best_exp_level = cctrl->exp_level;
                    best_score = score;
                    best_creatng = ctng;
                }
                else
                {
                    // Otherwise the level needs to be at least as high
                    if (best_exp_level <= cctrl->exp_level)
                    {
                        // For equal levels, the score is most important
                        if ((score > best_score) || (cctrl->exp_level > best_exp_level))
                        {
                            best_exp_level = cctrl->exp_level;
                            best_score = score;
                            best_creatng = ctng;
                        }
                    }
                }
            }
        }
        else // so party_objective == CHeroTsk_DefendParty)
        {
            // Only look to overwrite other defending unit, or noexisting unit, with this defending unit
            if ((bcctrl->party.objective == CHeroTsk_DefendParty) || (best_creatng == INVALID_THING))
            {
                if (has_digger < 1 || is_digger) // if we want a digger, do not consider non-diggers
                {
                    if (best_exp_level <= cctrl->exp_level)
                    {
                        // For equal levels, the score is most important
                        if ((score > best_score) || (cctrl->exp_level > best_exp_level))
                        {
                            best_exp_level = cctrl->exp_level;
                            best_score = score;
                            best_creatng = ctng;
                        }
                    }
                }
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

TbBool remove_creature_from_group(struct Thing* creatng)
{
    struct Thing* grptng = get_first_follower_creature_in_group(creatng);
    if (!remove_creature_from_group_without_leader_consideration(creatng)) {
        SYNCDBG(5, "Removing %s index %d and disbanding the party", thing_model_name(creatng), (int)creatng->index);
        // Last creature removed - party disbanded
        return false;
    }
    SYNCDBG(5, "Removing %s index %d", thing_model_name(creatng), (int)creatng->index);
    struct Thing* leadtng = get_best_creature_to_lead_group(grptng);
    make_group_member_leader(leadtng);
    return true;
}

TbBool add_creature_to_group(struct Thing *creatng, struct Thing *grptng)
{
    SYNCDBG(5,"Adding %s index %d",thing_model_name(creatng),(int)creatng->index);
    struct Thing* pvthing = get_last_follower_creature_in_group(grptng);
    if ((grptng->index == creatng->index) || (grptng->owner != creatng->owner)) {
        return false;
    }
    struct CreatureControl* crctrl = creature_control_get_from_thing(creatng);
    if (creature_is_group_member(creatng)) {
        remove_creature_from_group(creatng);
    }
    if (thing_exists(pvthing))
    {
        // If we already have a group, place the new creature at end
        crctrl = creature_control_get_from_thing(creatng);
        crctrl->prev_in_group = pvthing->index;
        struct CreatureControl* pvctrl = creature_control_get_from_thing(pvthing);
        pvctrl->next_in_group = creatng->index;
        crctrl->group_leader_idx = pvctrl->group_leader_idx;
        crctrl->next_in_group = 0;
        crctrl->group_member_count = 0;
    } else
    {
        // If there's no group, create new one made of both creatures
        crctrl = creature_control_get_from_thing(creatng);
        struct CreatureControl* grctrl = creature_control_get_from_thing(grptng);
        crctrl->prev_in_group = grptng->index;
        grctrl->next_in_group = creatng->index;
        crctrl->group_leader_idx = grptng->index;
        grctrl->group_leader_idx = grptng->index;
        crctrl->next_in_group = 0;
        grctrl->prev_in_group = 0;
        // Remove member count from non-leader creature; the leader will have it computed somewhere else
        crctrl->group_member_count = 0;
    }
    creatng->alloc_flags |= TAlF_IsFollowingLeader;
    return true;
}

long add_creature_to_group_as_leader(struct Thing *creatng, struct Thing *grptng)
{
    SYNCDBG(5,"Adding %s index %d",thing_model_name(creatng),(int)creatng->index);
    if ((grptng->index == creatng->index) || (grptng->owner != creatng->owner)) {
        return 0;
    }
    if (creature_is_group_member(creatng)) {
        remove_creature_from_group(creatng);
    }
    struct Thing* leadtng = get_group_leader(grptng);
    if (!thing_is_creature(leadtng))
        leadtng = grptng;
    // Change old leader to normal group member, and add new one to chain as its head
    internal_add_member_to_group_chain_head(creatng, leadtng);
    leadtng->alloc_flags |= TAlF_IsFollowingLeader;
    // Now go through all group members and set leader index
    internal_update_leader_index_in_group(creatng);
    return 1;
}

int get_party_index_of_name(const char *prtname)
{
    for (int i = 0; i < game.script.creature_partys_num; i++)
    {
        struct Party* party = &game.script.creature_partys[i];
        if (strcasecmp(party->prtname, prtname) == 0)
            return i;
    }
    return -1;
}

TbBool create_party(const char *prtname)
{
    if (game.script.creature_partys_num >= CREATURE_PARTYS_COUNT)
    {
        SCRPTERRLOG("Too many partys in script");
        return false;
    }
    struct Party* party = (&game.script.creature_partys[game.script.creature_partys_num]);
    snprintf(party->prtname, sizeof(party->prtname), "%s", prtname);
    party->members_num = 0;
    game.script.creature_partys_num++;
    return true;
}

TbBool add_member_to_party(int party_id, long crtr_model, CrtrExpLevel exp_level, long carried_gold, long objctv_id, long countdown, PlayerNumber target)
{
    if ((party_id < 0) && (party_id >= CREATURE_PARTYS_COUNT))
    {
        ERRORLOG("Party:%d is not defined", party_id);
        return false;
    }
    struct Party* party = &game.script.creature_partys[party_id];
    if (party->members_num >= GROUP_MEMBERS_COUNT)
    {
      ERRORLOG("Too many creatures in party '%s' (limit is %d members)",
          party->prtname, GROUP_MEMBERS_COUNT);
      return false;
    }
    struct PartyMember* member = &(party->members[party->members_num]);
    member->flags &= ~TrgF_DISABLED;
    member->crtr_kind = crtr_model;
    member->carried_gold = carried_gold;
    member->exp_level = exp_level-1;
    member->is_active = 1;
    member->objectv = objctv_id;
    member->countdown = countdown;
    party->members_num++;
    member->target = target;
    return true;
}

TbBool delete_member_from_party(int party_id, long crtr_model, CrtrExpLevel exp_level)
{
    if ((party_id < 0) && (party_id >= CREATURE_PARTYS_COUNT))
    {
        ERRORLOG("Party:%d is not defined", party_id);
        return false;
    }
    struct Party* party = &game.script.creature_partys[party_id];

    for (int i = 0; i < party->members_num; i++)
    {
        struct PartyMember* member = &(party->members[i]);
        if ((member->crtr_kind == crtr_model) && (member->exp_level == (exp_level-1)))
        {
            memmove(member, member + 1, sizeof(*member) * (party->members_num - i - 1));
            party->members_num--;
            return true;
        }
    }
    return false;
}

TbBool make_group_member_leader(struct Thing *leadtng)
{
    struct Thing* prvtng = get_group_leader(leadtng);
    if (!thing_is_creature(prvtng))
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
    struct CreatureControl* leadctrl = creature_control_get_from_thing(leadtng);
    int group_len = leadctrl->group_member_count;
    for (int i = 0; i < group_len; i++)
    {
        struct MemberPos* avail_pos = &leadctrl->followers_pos[i];
        if (((avail_pos->flags & MpF_AVAIL) != 0) && ((avail_pos->flags & MpF_OCCUPIED) == 0))
        {
            pos->x.val = subtile_coord_center(stl_num_decode_x(avail_pos->stl_num));
            pos->y.val = subtile_coord_center(stl_num_decode_y(avail_pos->stl_num));
            pos->z.val = 0;
            avail_pos->flags |= MpF_OCCUPIED;
            return true;
        }
    }
    WARNDBG(3,"Group led by %s index %d owned by player %d had all %d follower positions taken",
        thing_model_name(leadtng),(int)leadtng->index,(int)leadtng->owner,group_len);
    return false;
}

long process_obey_leader(struct Thing *thing)
{
    struct Thing* leadtng = get_group_leader(thing);
    if (!thing_is_creature(leadtng)) {
        WARNDBG(3,"Leader invalid, resetting %s index %d owned by player %d",
            thing_model_name(thing),(int)thing->index,(int)thing->owner);
        set_start_state(thing);
        return 1;
    }
    if (creature_is_being_dropped(thing))
    {
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
    struct CreatureStateConfig* stati = get_creature_state_with_task_completion(leadtng);

    switch (stati->follow_behavior)
    {
    case FlwB_FollowLeader:
        if (thing->active_state != CrSt_CreatureFollowLeader) {
            external_set_thing_state(thing, CrSt_CreatureFollowLeader);
        }
        break;
    case FlwB_MatchWorkRoom:
        cctrl = creature_control_get_from_thing(thing);
        leadctrl = creature_control_get_from_thing(leadtng);
        if ((cctrl->work_room_id != leadctrl->work_room_id) && (cctrl->target_room_id != leadctrl->work_room_id))
        {
            struct Room *room;
            room = get_room_creature_works_in(leadtng);
            struct CreatureModelConfig *crconf;
            crconf = creature_stats_get_from_thing(thing);
            CreatureJob jobpref;
            jobpref = get_job_for_room(room->kind, JoKF_None, crconf->job_primary|crconf->job_secondary);
            cleanup_current_thing_state(thing);
            send_creature_to_room(thing, room, jobpref);
        }
        break;
    case FlwB_JoinCombatOrFollow:
        cctrl = creature_control_get_from_thing(thing);
        leadctrl = creature_control_get_from_thing(leadtng);

        struct Thing* obthing;
        if ((leadctrl->combat.battle_enemy_idx > 0) && (cctrl->combat.battle_enemy_idx == 0))
        {
            obthing = thing_get(leadctrl->combat.battle_enemy_idx);
            if (thing_is_deployed_door(obthing))
            {
                set_creature_door_combat(thing, obthing);
            }
        } else
        if (thing->active_state != CrSt_CreatureFollowLeader) {
            external_set_thing_state(thing, CrSt_CreatureFollowLeader);
        }
        break;
    default:
        break;
    }
    return 1;
}

void creature_follower_pos_add(struct Thing *creatng, int ifollow, const struct Coord3d *pos)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct MemberPos* avail_pos = &cctrl->followers_pos[ifollow];
    avail_pos->stl_num = get_subtile_number(pos->x.stl.num, pos->y.stl.num);
    avail_pos->flags |= MpF_AVAIL;
}

void leader_find_positions_for_followers(struct Thing *leadtng)
{
    int group_len = get_no_creatures_in_group(leadtng);
    struct CreatureControl* cctrl = creature_control_get_from_thing(leadtng);
    // Base the position update frequency on leader move speed; speed of 48 requires refresh per 32 turns
    int recompute_interval = 32 * 48 / (get_creature_speed(leadtng) + 1);
    if (recompute_interval > 256) {
        recompute_interval = 256;
    } else if (recompute_interval < 4) {
        recompute_interval = 4;
    }
    if ((cctrl->group_member_count == group_len) && (((game.play_gameturn + leadtng->index) % recompute_interval) != 0))
    {
        SYNCDBG(7,"Reusing positions for %d followers of %s index %d owned by player %d",
            group_len,thing_model_name(leadtng),(int)leadtng->index,(int)leadtng->owner);
        for (int i = 0; i < GROUP_MEMBERS_COUNT; i++)
        {
          cctrl->followers_pos[i].flags &= ~0x01;
        }
        return;
    }
    SYNCDBG(7,"Finding positions for %d followers of %s index %d owned by player %d",
        group_len,thing_model_name(leadtng),(int)leadtng->index,(int)leadtng->owner);
    cctrl->group_member_count = group_len;
    memset(cctrl->followers_pos, 0, sizeof(cctrl->followers_pos));

    int len_xv = LbSinL(leadtng->move_angle_xy + DEGREES_180) << 8 >> 16;
    int len_yv = -((LbCosL(leadtng->move_angle_xy + DEGREES_180) << 8) >> 8) >> 8;
    int len_xh = LbSinL(leadtng->move_angle_xy - DEGREES_90) << 8 >> 16;
    int len_yh = -((LbCosL(leadtng->move_angle_xy - DEGREES_90) << 8) >> 8) >> 8;

    int ih;
    int iv;
    int ivmax = 2 * group_len;
    int ifollow = 0;

    int shift_xh;
    int shift_yh;

    int delta_yh = 2 * len_yh;
    int delta_xh = 2 * len_xh;
    int shift_yv = 2 * len_yv;
    int delta_yv = 2 * len_yv;
    int shift_yh_beg = -2 * len_yh;
    int shift_xh_beg = -2 * len_xh;
    int shift_xv = 2 * len_xv;
    int delta_xv = 2 * len_xv;
    for (iv = 2; iv <= ivmax; iv += 2)
    {
        shift_yh = shift_yh_beg;
        shift_xh = shift_xh_beg;
        for (ih = -2; ih <= 2; ih += 2)
        {
            int mcor_x = leadtng->mappos.x.val + shift_xh + shift_xv;
            int mcor_y = leadtng->mappos.y.val + shift_yv + shift_yh;
            if ((coord_slab(mcor_x) > 0) && (coord_slab(mcor_x) < game.map_tiles_x))
            {
                if ((coord_slab(mcor_y) > 0) && (coord_slab(mcor_y) < game.map_tiles_y))
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
            int mcor_x = leadtng->mappos.x.val + shift_xh + shift_xv;
            int mcor_y = leadtng->mappos.y.val + shift_yv + shift_yh;
            if ((coord_slab(mcor_x) > 0) && (coord_slab(mcor_x) < game.map_tiles_x))
            {
                if ((coord_slab(mcor_y) > 0) && (coord_slab(mcor_y) < game.map_tiles_y))
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
    for (;ifollow < group_len;ifollow++)
    {
        // Just go to my slab
        struct Coord3d pos;
        pos.x.val = leadtng->mappos.x.val;
        pos.y.val = leadtng->mappos.y.val;

        pos.x.stl.pos = THING_RANDOM(leadtng, 127);
        pos.y.stl.pos = THING_RANDOM(leadtng, 127);

        pos.z.val = get_floor_height_at(&pos);
        creature_follower_pos_add(leadtng, ifollow, &pos);
    }
}

/**
 * Spawns new creature parties. Makes given amount of the parties.
 * @param party The party to be spawned.
 * @param plyr_idx Player to own the creatures within group.
 * @param location Where the party will be spawned.
 * @param copies_num Amount of copies to be spawned.
 * @return Gives leader of last party spawned.
 */
struct Thing *script_process_new_party(struct Party *party, PlayerNumber plyr_idx, TbMapLocation location, long copies_num)
{
    struct Thing* leadtng = INVALID_THING;
    for (long i = 0; i < copies_num; i++)
    {
        struct Thing* grptng = INVALID_THING;
        for (long k = 0; k < party->members_num; k++)
        {
          if (k >= GROUP_MEMBERS_COUNT)
          {
              ERRORLOG("Party too big, %d is the limit",GROUP_MEMBERS_COUNT);
              break;
          }
          struct PartyMember* member = &(party->members[k]);
          struct Thing* thing = script_create_new_creature(plyr_idx, member->crtr_kind, location, member->carried_gold, member->exp_level, SpwnT_Default);
          if (!thing_is_invalid(thing))
          {
              struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
              cctrl->party.objective = member->objectv;
              cctrl->party.target_plyr_idx = member->target;
              cctrl->party.original_objective = cctrl->party.objective;
              cctrl->wait_to_turn = game.play_gameturn + member->countdown;
              cctrl->hero.wait_time = game.play_gameturn + member->countdown;
              if (thing_is_invalid(grptng))
              {
                  // If it is the first creature - set it as only group member and leader
                  // Inside the thing, we don't need to mark it in any way (two creatures are needed to form a real group)
                  SYNCDBG(5,"First member %s index %d",thing_model_name(thing),(int)thing->index);
                  leadtng = thing;
                  grptng = thing;
              } else
              {
                  struct Thing* bestng = get_best_creature_to_lead_group(grptng);
                  struct CreatureControl* bestctrl = creature_control_get_from_thing(bestng);
                  // If current leader wants to defend, and current unit has an objective, new unit will be group leader.
                  if ((cctrl->party.objective != CHeroTsk_DefendParty) && (bestctrl->party.objective == CHeroTsk_DefendParty))
                  {
                      add_creature_to_group_as_leader(thing, grptng);
                      leadtng = thing;
                  } else
                  // if best and current unit want to defend party, or neither do, the strongest will be leader
                  if (((cctrl->party.objective == CHeroTsk_DefendParty) && (bestctrl->party.objective == CHeroTsk_DefendParty)) || ((cctrl->party.objective != CHeroTsk_DefendParty) && (bestctrl->party.objective != CHeroTsk_DefendParty)))
                  {
                      if ((cctrl->exp_level > bestctrl->exp_level) || ((cctrl->exp_level == bestctrl->exp_level) && (get_creature_thing_score(thing) > get_creature_thing_score(bestng))))
                      {
                          add_creature_to_group_as_leader(thing, grptng);
                          leadtng = thing;
                      }
                      else
                      // If it's weaker than the current leader, joind as a group
                      {
                          add_creature_to_group(thing, grptng);
                      }
                  }
                  else
                  // If it wants to defend, but the group leader has an objective, just add it to group
                  {
                      add_creature_to_group(thing, grptng);
                  }
              }
          }
        }
    }
    return leadtng;
}

struct Thing* script_process_new_tunneller_party(PlayerNumber plyr_idx, long prty_id, TbMapLocation location, TbMapLocation heading, CrtrExpLevel exp_level, unsigned long carried_gold)
{
    struct Thing* ldthing = script_process_new_tunneler(plyr_idx, location, heading, exp_level, carried_gold);
    if (thing_is_invalid(ldthing))
    {
        ERRORLOG("Couldn't create tunneling group leader");
        return INVALID_THING;
    }
    struct Thing* gpthing = script_process_new_party(&game.script.creature_partys[prty_id], plyr_idx, location, 1);
    if (thing_is_invalid(gpthing))
    {
        ERRORLOG("Couldn't create creature group");
        return ldthing;
    }
    add_creature_to_group_as_leader(ldthing, gpthing);

    return ldthing;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
