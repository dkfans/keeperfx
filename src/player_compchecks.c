/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_compchecks.c
 *     Computer player checks definitions and routines.
 * @par Purpose:
 *     Defines a computer player checks and related functions.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Mar 2009 - 06 Jan 2013
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
#include <string.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"

#include "config.h"
#include "config_terrain.h"
#include "player_instances.h"
#include "creature_jobs.h"
#include "creature_states.h"
#include "creature_states_mood.h"
#include "spdigger_stack.h"
#include "magic_powers.h"
#include "map_blocks.h"
#include "map_utils.h"
#include "dungeon_data.h"
#include "room_data.h"
#include "ariadne_wallhug.h"
#include "power_hand.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "cursor_tag.h"
#include "gui_msgs.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_flight(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_vision(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_place_trap(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_for_money(struct Computer2 *comp, struct ComputerCheck * check);
long computer_check_prison_tendency(struct Computer2* comp, struct ComputerCheck* check);

/******************************************************************************/
const struct NamedCommand computer_check_func_type[] = {
  {"checks_hates",            1,},
  {"check_move_to_best_room", 2,},
  {"check_move_to_room",      3,},
  {"check_no_imps",           4,},
  {"check_for_pretty",        5,},
  {"check_for_quick_attack",  6,},
  {"check_for_accelerate",    7,},
  {"check_slap_imps",         8,},
  {"check_enemy_entrances",   9,},
  {"check_for_place_door",   10,},
  {"check_neutral_places",   11,},
  {"check_for_place_trap",   12,},
  {"check_for_expand_room",  13,},
  {"check_for_money",        14,},
  {"check_prison_tendency",  15,},
  {"check_for_flight",       16,},
  {"check_for_vision",       17,},
  {"none",                   18,},
  {NULL,                      0,},
};

Comp_Check_Func computer_check_func_list[] = {
  NULL,
  computer_checks_hates,
  computer_check_move_creatures_to_best_room,
  computer_check_move_creatures_to_room,
  computer_check_no_imps,
  computer_check_for_pretty,
  computer_check_for_quick_attack,
  computer_check_for_accelerate,
  computer_check_slap_imps,
  computer_check_enemy_entrances,
  computer_check_for_place_door,
  computer_check_neutral_places,
  computer_check_for_place_trap,
  computer_check_for_expand_room,
  computer_check_for_money,
  computer_check_prison_tendency,
  computer_check_for_flight,
  computer_check_for_vision,
  NULL,
  NULL,
};

struct ExpandRooms expand_rooms[] = {
  {RoK_TREASURE,  45},
  {RoK_LAIR,      45},
  {RoK_GARDEN,    45},
  {RoK_LIBRARY,   45},
  {RoK_TRAINING,  35},
  {RoK_WORKSHOP,  45},
  {RoK_SCAVENGER, 30},
  {RoK_PRISON,    30},
  {RoK_TEMPLE,    25},
  {RoK_TORTURE,   35},
  {RoK_GRAVEYARD, 30},
  {RoK_BARRACKS,  35},
  {RoK_NONE,       0},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/

long computer_checks_hates(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* compdngn = comp->dungeon;
    // Reference values for checking hate
    int cdngn_creatrs = count_creatures_in_dungeon(compdngn);
    int cdngn_spdiggrs = count_diggers_in_dungeon(compdngn);
    int cdngn_enrancs = count_entrances(comp, compdngn->owner);
    // Now check hate for every player
    for (int i = 0; i < PLAYERS_COUNT; i++)
    {
        struct PlayerInfo* player = get_player(i);
        struct Dungeon* dungeon = get_players_dungeon(player);
        struct OpponentRelation* oprel = &comp->opponent_relations[i];
        if (!player_exists(player) || (player->id_number == compdngn->owner)
         || (player->id_number == game.neutral_player_num))
            continue;
        if (player->is_active != 1)
            continue;
        if (players_are_mutual_allies(compdngn->owner, i))
            continue;
        int hate_reasons = 0;
        int hdngn_creatrs = count_creatures_in_dungeon(dungeon);
        int hdngn_spdiggrs = count_diggers_in_dungeon(dungeon);
        // Computers hate players who have more creatures than them
        if (hdngn_creatrs >= cdngn_creatrs)
        {
            hate_reasons++;
            oprel->hate_amount++;
        }
        // Computers hate players who have more special diggers than them
        if (cdngn_spdiggrs / 6 + cdngn_spdiggrs < hdngn_spdiggrs)
        {
            hate_reasons++;
            oprel->hate_amount++;
        }
        // Computers hate players who can build more rooms than them
        if (((int)compdngn->total_rooms + (int)compdngn->total_rooms / 6) < (int)dungeon->total_rooms)
        {
            hate_reasons++;
            oprel->hate_amount++;
        }
        // Computers highly hate players who claimed more entrances than them
        int hdngn_enrancs = count_entrances(comp, i);
        if (hdngn_enrancs > cdngn_enrancs)
        {
            hate_reasons++;
            oprel->hate_amount += 5;
        }
        // If no reason to hate the player - hate him randomly for just surviving that long
        if ((hate_reasons <= 0) && (check->primary_parameter < game.play_gameturn))
        {
            if (PLAYER_RANDOM(compdngn->owner, 100) < 20) {
                oprel->hate_amount++;
            }
        }
    }
    return CTaskRet_Unk4;
}
// 100 percent_to_reassign = num_to_move is high and creatures are moved around more
// 0 percent_to_reassign = num_to_move is 0 and all creatures do their default jobs
int calculate_number_of_creatures_to_move(struct Dungeon *dungeon, int percent_to_reassign)
{
    int creatures_doing_primary_or_secondary_job = 0;
    int creatures_doing_other_jobs = 0;

    for (int i = dungeon->creatr_list_start; i != 0;)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }

        if (!creature_is_being_unconscious(thing) && !thing_is_picked_up(thing) && !creature_is_kept_in_custody_by_enemy(thing) && !creature_is_leaving_and_cannot_be_stopped(thing))
        {
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
            if ((cctrl->job_assigned == crconf->job_primary) || (cctrl->job_assigned == crconf->job_secondary) || (cctrl->job_assigned == 0))
            {
                creatures_doing_primary_or_secondary_job += 1;
            } else {
                creatures_doing_other_jobs += 1;
            }
        }

        i = cctrl->players_next_creature_idx;
    }

    int work_capable_creatures = creatures_doing_primary_or_secondary_job + creatures_doing_other_jobs;
    if (work_capable_creatures == 0) {
        return 0;
    }

    int percent_doing_other_jobs = (creatures_doing_other_jobs * 100) / work_capable_creatures;
    int num_to_move = work_capable_creatures * (percent_to_reassign - percent_doing_other_jobs) / 100;
    if (num_to_move <= 0) {return 0;}

    //JUSTLOG("-----", 0);
    //JUSTLOG("total creatures = %d", dungeon->num_active_creatrs);
    //JUSTLOG("work_capable_creatures = %d", work_capable_creatures);
    //JUSTLOG("cfg percent to reassign = %d percent should do other jobs", percent_to_reassign);
    //JUSTLOG("creatures_doing_primary_or_secondary_job = %d", creatures_doing_primary_or_secondary_job);
    //JUSTLOG("creatures_doing_other_jobs = %d", creatures_doing_other_jobs);
    return num_to_move;
}


long computer_check_move_creatures_to_best_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon* dungeon = comp->dungeon;
    SYNCDBG(8,"Starting for player %d",(int)dungeon->owner);

    int num_to_move = calculate_number_of_creatures_to_move(dungeon, check->primary_parameter);

    if (num_to_move <= 0) {
        SYNCDBG(8,"No player %d creatures to move, active %d percentage %d",
            (int)dungeon->owner,(int)dungeon->num_active_creatrs,(int)check->primary_parameter);
        return CTaskRet_Unk4;
    }

    if (!computer_able_to_use_power(comp, PwrK_HAND, 1, num_to_move)) {
        return CTaskRet_Unk4;
    }
    // If there's already task in progress which uses hand, then don't add more
    // content of the hand could be used by wrong task by mistake
    if (is_task_in_progress_using_hand(comp)) {
        return CTaskRet_Unk4;
    }
    if (!create_task_move_creatures_to_room(comp, 0, num_to_move)) {
        return CTaskRet_Unk4;
    }
    SYNCDBG(8,"Added player %d task to move %d creatures to best room",(int)dungeon->owner,(int)num_to_move);
    return CTaskRet_Unk1;
}

long computer_check_move_creatures_to_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    struct Dungeon* dungeon = comp->dungeon;
    SYNCDBG(8,"Checking player %d for move to %s", (int)dungeon->owner, room_code_name(check->secondary_parameter));
    int num_to_move = calculate_number_of_creatures_to_move(dungeon, check->primary_parameter);
    if (num_to_move <= 0) {
        SYNCDBG(8,"No creatures to move, active %d percentage %d", (int)dungeon->num_active_creatrs, (int)check->primary_parameter);
        return CTaskRet_Unk4;
    }
    if (!computer_able_to_use_power(comp, PwrK_HAND, 1, num_to_move)) {
        return CTaskRet_Unk4;
    }
    // If there's already task in progress which uses hand, then don't add more
    // content of the hand could be used by wrong task by mistake
    if (is_task_in_progress_using_hand(comp)) {
        return CTaskRet_Unk4;
    }
    unsigned long k = 0;
    long i = dungeon->room_list_start[check->secondary_parameter];
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
        if (room->total_capacity > room->used_capacity)
        {
            int num_to_move_fit = min(num_to_move, room->total_capacity - room->used_capacity);
            if (create_task_move_creatures_to_room(comp, room->index, num_to_move_fit)) {
                SYNCDBG(8,"Added task to move %d creatures to %s index %d", (int)num_to_move_fit,room_code_name(room->kind),(int)room->index);
                return CTaskRet_Unk1;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping rooms list");
            break;
        }
    }
    return CTaskRet_Unk4;
}

/**
 * Returns whether special diggers in given dungeon are actually digging indestructible valuables.
 * In standard configuration, indestructible valuables are simply slabs with gems.
 * @param dungeon
 * @return
 */
static TbBool any_digger_is_digging_indestructible_valuables(struct Dungeon *dungeon)
{
    unsigned long k = 0;
    long i = dungeon->digger_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
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
            long state_type = get_creature_state_type(thing);

            if ((state_type == CrStTyp_Work)
                && (cctrl->digger.last_did_job == SDLstJob_DigOrMine)
                && is_digging_indestructible_place(thing))
            {
                SYNCDBG(18, "Indestructible valuables being dug by player %d", (int)dungeon->owner);
                return true;
            }
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            return false;
        }
    }
    SYNCDBG(18, "Indestructible valuables NOT being dug by player %d", (int)dungeon->owner);
    return false;
}

/**
 * Returns number of diggable faces of indestructible valuables marked for digging.
 * In standard configuration, indestructible valuables are simply slabs with gems.
 * @param dungeon
 * @return
 */
static int count_faces_of_indestructible_valuables_marked_for_dig(struct Dungeon *dungeon)
{
    int num_faces = 0;
    SYNCDBG(18,"Starting");
    long i = -1;
    while (1)
    {
        i = find_next_dig_in_dungeon_task_list(dungeon, i);
        if (i < 0)
            break;
        struct MapTask* mtask = get_dungeon_task_list_entry(dungeon, i);
        MapSubtlCoord stl_x = stl_num_decode_x(mtask->coords);
        MapSubtlCoord stl_y = stl_num_decode_y(mtask->coords);
        if (subtile_revealed(stl_x, stl_y, dungeon->owner))
        {
            struct SlabMap* slb = get_slabmap_for_subtile(stl_x, stl_y);
            const struct SlabConfigStats* slabst = get_slab_stats(slb);
            if (((slabst->block_flags & SlbAtFlg_Valuable) != 0) && slab_kind_is_indestructible(slb->kind))
            {
                num_faces += block_count_diggable_sides(subtile_slab(stl_x), subtile_slab(stl_y));
            }
        }
    }
    return num_faces;
}

/**
 * Filter function for selecting creature which is best candidate for being sacrificed.
 * A specific thing can be selected either by class, model and owner.
 *
 * @param thing Creature thing to be filtered.
 * @param param Struct with specific thing which is dragged.
 * @param maximizer Previous max value.
 * @return If returned value is greater than maximizer, then the filtering result should be updated.
 */
long player_list_creature_filter_best_for_sacrifice(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);

    if ((cctrl->combat_flags == 0) && (param->secondary_number || thing->creature.gold_carried == 0)) //no gold carried if no gem access
    {
        if (creature_is_being_unconscious(thing) || creature_under_spell_effect(thing, CSAfF_Chicken))
            return -1;
        if (creature_is_being_dropped(thing) || !can_thing_be_picked_up_by_player(thing, param->plyr_idx))
            return -1;
        if ((param->plyr_idx >= 0) && (thing->owner != param->plyr_idx))
            return -1;
        if (!thing_matches_model(thing, param->model_id))
            return -1;
        if ((param->class_id > 0) && (thing->class_id != param->class_id))
            return -1;
        struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
        // Let us estimate value of the creature in gold
        long priority = thing->creature.gold_carried;             // base value
        priority += param->primary_number * thing->health / crconf->health; // full health valued at this many gold
        priority += 10000 * cctrl->exp_level; // experience earned by the creature has a big value
        if (get_creature_state_type(thing) == CrStTyp_Work)
            priority += 500; // aborted work valued at this many gold
        if (anger_is_creature_angry(thing))
            priority /= 2; // angry creatures have lower value
        if (anger_is_creature_livid(thing))
            priority /= 3; // livid creatures have minimal value
         // Return maximizer based on our evaluated gold value
        return INT32_MAX - priority;
    }
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

static struct Thing *find_creature_for_sacrifice(struct Computer2 *comp, ThingModel crmodel)
{
    struct Dungeon* dungeon = comp->dungeon;

    struct CompoundTngFilterParam param;
    param.plyr_idx = dungeon->owner;
    param.class_id = TCls_Creature;
    param.model_id = crmodel;
    param.primary_number = compute_lowest_power_price(dungeon->owner, PwrK_MKDIGGER, 0);
    param.secondary_number = any_digger_is_digging_indestructible_valuables(dungeon);
    Thing_Maximizer_Filter filter = player_list_creature_filter_best_for_sacrifice;
    TbBool is_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(dungeon->owner, crmodel);
    struct Thing* thing = INVALID_THING;
    if (is_spec_digger)
    {
        thing = get_player_list_creature_with_filter(dungeon->digger_list_start, filter, &param);
    }
    if ((!is_spec_digger) && thing_is_invalid(thing))
    {
        thing = get_player_list_creature_with_filter(dungeon->creatr_list_start, filter, &param);
    }
    return thing;
}

/**
 * Checks if a computer player can sacrifice imps to reduce price.
 * @param comp
 * @param check
 */
long computer_check_sacrifice_for_cheap_diggers(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    if (game.conf.rules[dungeon->owner].sacrifices.cheaper_diggers_sacrifice_model == 0) {
        return CTaskRet_Unk0;
    }

    GoldAmount power_price = compute_power_price(dungeon->owner, PwrK_MKDIGGER, 0);
    GoldAmount lowest_price = compute_lowest_power_price(dungeon->owner, PwrK_MKDIGGER, 0);
    SYNCDBG(18, "Digger creation power price: %d, lowest: %d", power_price, lowest_price);

    if ((power_price > lowest_price) && !is_task_in_progress_using_hand(comp)
        && computer_able_to_use_power(comp, PwrK_MKDIGGER, 0, 2)) //TODO COMPUTER_PLAYER add amount of imps to afford to the checks config params
    {
        struct Thing* creatng = find_creature_for_sacrifice(comp, game.conf.rules[dungeon->owner].sacrifices.cheaper_diggers_sacrifice_model);
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        if (!thing_is_invalid(creatng) && (cctrl->exp_level < 2))
        {
            SYNCDBG(18, "Got digger to sacrifice, %s index %d owner %d",thing_model_name(creatng),(int)creatng->index,(int)creatng->owner);
            if (creature_can_do_job_for_player(creatng, dungeon->owner, Job_TEMPLE_SACRIFICE, JobChk_None))
            {
                struct Coord3d pos;
                // Let's pretend a human does the drop here; computers normally should not be allowed to sacrifice
                if (get_drop_position_for_creature_job_in_dungeon(&pos, dungeon, creatng, Job_TEMPLE_SACRIFICE, JoKF_AssignHumanDrop))
                {
                    if (create_task_move_creature_to_pos(comp, creatng, pos, get_initial_state_for_job(Job_TEMPLE_SACRIFICE))) {
                        return CTaskRet_Unk1;
                    }
                }
            }
        }
    }
    return CTaskRet_Unk4;
}

/**
 * Checks if a computer player has not enough imps.
 * @param comp
 * @param check The check structure; param1 is preferred amount of imps, param2 is minimal amount,
 *     param3 is the increase in both amounts caused by face of indestructible slab marked for digging.
 */
long computer_check_no_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    // TODO COMPUTER_PLAYER create a separate check for imps sacrificing diggers
    if (computer_check_sacrifice_for_cheap_diggers(comp, check) == CTaskRet_Unk1) {
        return CTaskRet_Unk1;
    }
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    if (!creature_count_below_map_limit(0))
    {
        SYNCDBG(7, "Computer player %d can't create imps due to map limit", (int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    long controlled_diggers = dungeon->num_active_diggers - count_player_diggers_not_counting_to_total(dungeon->owner);
    int preferred_imps;
    int minimal_imps;
    // Compute additional imps from gem faces
    preferred_imps = minimal_imps = check->tertiary_parameter * count_faces_of_indestructible_valuables_marked_for_dig(dungeon);
    // The additional imps can double the limits, but no more
    if (preferred_imps > check->primary_parameter)
        preferred_imps = check->primary_parameter;
    if (minimal_imps > check->secondary_parameter)
        minimal_imps = check->secondary_parameter;
    // Add the base limits
    preferred_imps += check->primary_parameter;
    minimal_imps += check->secondary_parameter;
    SYNCDBG(8,"Starting for player %d, digger amounts minimal=%d preferred=%d controlled=%d",(int)dungeon->owner,(int)minimal_imps,(int)preferred_imps,(int)controlled_diggers);
    if (controlled_diggers >= preferred_imps) {
        return CTaskRet_Unk4;
    }
    TbBool able_to_use_power;
    if (controlled_diggers < minimal_imps/2) {
        // We have less than half of the minimal imps amount; build one no matter what, ignoring money projections
        able_to_use_power = (is_power_available(dungeon->owner, PwrK_MKDIGGER) && (dungeon->total_money_owned >= compute_power_price(dungeon->owner, PwrK_MKDIGGER, 0)));
    } else
    if (controlled_diggers < minimal_imps) {
        // We have less than minimal imps; build one if only there are money
        able_to_use_power = computer_able_to_use_power(comp, PwrK_MKDIGGER, 0, 1);
    } else {
        // We have less than preferred amount, but higher than minimal; allow building if we've got spare money
        able_to_use_power = computer_able_to_use_power(comp, PwrK_MKDIGGER, 0, 3 + (controlled_diggers - minimal_imps)/4);
    }
    if (able_to_use_power)
    {
        struct Thing* heartng = get_player_soul_container(dungeon->owner);
        MapSubtlCoord stl_x = heartng->mappos.x.stl.num;
        MapSubtlCoord stl_y = heartng->mappos.y.stl.num;
        if (xy_walkable(stl_x, stl_y, dungeon->owner))
        {
            if ((game.computer_chat_flags & CChat_TasksScarce) != 0) {
                struct PowerConfigStats* powerst = get_power_model_stats(PwrK_MKDIGGER);
                struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[get_players_special_digger_model(dungeon->owner)];
                message_add_fmt(MsgType_Player, comp->dungeon->owner, "My %s count is only %d, casting %s!",get_string(crconf->namestr_idx),(int)controlled_diggers,get_string(powerst->name_stridx));
            }
            if (try_game_action(comp, dungeon->owner, GA_UseMkDigger, 0, stl_x, stl_y, 1, 1) > Lb_OK) {
                return CTaskRet_Unk1;
            }
        }
        return CTaskRet_Unk1;
    }
    return CTaskRet_Unk0;
}

struct Thing * find_imp_for_pickup(struct Computer2 *comp, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Dungeon* dungeon = comp->dungeon;
    int pick1_dist = INT_MAX;
    int pick2_dist = INT_MAX;
    struct Thing* pick2_tng = INVALID_THING;
    struct Thing* pick1_tng = INVALID_THING;
    unsigned long k = 0;
    long i = dungeon->digger_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (!thing_is_creature(thing) || creature_control_invalid(cctrl))
        {
          ERRORLOG("Jump to invalid creature detected");
          break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->combat_flags == 0)
        {
            if (!creature_is_being_unconscious(thing) && !creature_under_spell_effect(thing, CSAfF_Chicken))
            {
                if (!creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
                {
                    MapSubtlDelta dist = grid_distance(stl_x, stl_y, thing->mappos.x.stl.num, thing->mappos.y.stl.num);
                    long state_type = get_creature_state_type(thing);
                    if (state_type == CrStTyp_Work)
                    {
                        if (dist < pick1_dist)
                        {
                            pick1_dist = dist;
                            pick1_tng = thing;
                        }
                    }
                    else
                    {
                        if (dist < pick2_dist)
                        {
                            pick2_dist = dist;
                            pick2_tng = thing;
                        }
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
    if (!thing_is_invalid(pick2_tng)) {
        return pick2_tng;
    } else {
        return pick1_tng;
    }
}

long computer_check_for_pretty(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    if (!computer_able_to_use_power(comp, PwrK_HAND, 1, 1)) {
        return CTaskRet_Unk4;
    }
    if (is_task_in_progress_using_hand(comp)) {
        return CTaskRet_Unk4;
    }
    {
        long stack_len = dungeon->digger_stack_length;
        if (stack_len <= check->primary_parameter * dungeon->total_area / 100) {
            return CTaskRet_Unk4;
        }
        long n = find_in_imp_stack_starting_at(DigTsk_ImproveDungeon, PLAYER_RANDOM(compdngn->owner, stack_len), dungeon);
        if (n < 0) {
            return CTaskRet_Unk4;
        }
        const struct DiggerStack* dstack = &dungeon->digger_stack[n];
        stl_x = stl_num_decode_x(dstack->stl_num);
        stl_y = stl_num_decode_y(dstack->stl_num);
    }
    struct Thing* creatng = find_imp_for_pickup(comp, stl_x, stl_y);
    if (thing_is_invalid(creatng)) {
        return CTaskRet_Unk4;
    }
    if (!create_task_move_creature_to_subtile(comp, creatng, stl_x, stl_y, CrSt_ImpImprovesDungeon)) {
        return CTaskRet_Unk4;
    }
    return CTaskRet_Unk1;
}

struct Room *get_opponent_room(struct Computer2 *comp, PlayerNumber plyr_idx)
{
    static const RoomKind opponent_room_kinds[] = {RoK_DUNGHEART, RoK_PRISON, RoK_LIBRARY, RoK_TREASURE};
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon) || (game.conf.slab_conf.room_types_count < 1)) {
        return INVALID_ROOM;
    }
    int n = opponent_room_kinds[PLAYER_RANDOM(comp->dungeon->owner, sizeof(opponent_room_kinds) / sizeof(opponent_room_kinds[0]))];
    for (int i = 0; i < game.conf.slab_conf.room_types_count; i++)
    {
        struct Room* room = room_get(dungeon->room_list_start[n]);
        if (room_exists(room)) {
            return room;
        }
        n = (n + 1) % game.conf.slab_conf.room_types_count;
    }
    return INVALID_ROOM;
}

struct Room *get_hated_room_for_quick_attack(struct Computer2 *comp)
{
    SYNCDBG(8,"Starting for player %d",(int)comp->dungeon->owner);
    struct THate hates[PLAYERS_COUNT];
    get_opponent(comp, hates);
    // note that 'i' is not player index, player index is inside THate struct
    for (long i = 0; i < PLAYERS_COUNT; i++)
    {
        struct THate* hate = &hates[i];
        if (players_are_enemies(comp->dungeon->owner, hate->plyr_idx))
        {
            if (hate->pos_near != NULL)
            {
                struct Room* room = get_opponent_room(comp, hate->plyr_idx);
                if (!room_is_invalid(room)) {
                    return room;
                }
            }
        }
    }
    return INVALID_ROOM;
}

/**
 * Quick attack is just putting CTA spell on enemy room.

 * @param comp
 * @param check
 */
long computer_check_for_quick_attack(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    long attack_percentage = check->primary_parameter;
    long cta_duration = check->secondary_parameter;
    long min_creatures_to_attack = check->tertiary_parameter;
    int max_attack_amount = attack_percentage * dungeon->num_active_creatrs / 100;
    unsigned long creatures_to_fight_amount;

    if (min_creatures_to_attack >= max_attack_amount) {
        return CTaskRet_Unk4;
    }
    if (!computer_able_to_use_power(comp, PwrK_CALL2ARMS, 1, 3)) {
        return CTaskRet_Unk4;
    }
    if ((check_call_to_arms(comp) != 1) || is_there_an_attack_task(comp)) {
        return CTaskRet_Unk4;
    }
    struct Room* room = get_hated_room_for_quick_attack(comp);
    if (room_is_invalid(room)) {
        return CTaskRet_Unk4;
    }
    struct Coord3d pos;
    // TODO COMPUTER_PLAYER We should make sure the place of cast is accessible for creatures
    pos.x.val = subtile_coord_center(room->central_stl_x);
    pos.y.val = subtile_coord_center(room->central_stl_y);
    pos.z.val = subtile_coord(1,0);
    creatures_to_fight_amount = count_creatures_availiable_for_fight(comp, &pos);
    if (creatures_to_fight_amount <= min_creatures_to_attack) {
        return CTaskRet_Unk4;
    }
    if (creatures_to_fight_amount > max_attack_amount)
        creatures_to_fight_amount = max_attack_amount;
    if (!create_task_magic_support_call_to_arms(comp, &pos, cta_duration, creatures_to_fight_amount)) {
        return CTaskRet_Unk4;
    }
    SYNCLOG("Player %d decided to attack %s owned by player %d",(int)dungeon->owner,room_code_name(room->kind),(int)room->owner);
    output_message(SMsg_EnemyHarassments + SOUND_RANDOM(8), MESSAGE_DURATION_KEEPR_TAUNT);
    return CTaskRet_Unk1;
}

struct Thing *computer_check_creatures_in_room_for_accelerate(struct Computer2 *comp, struct Room *room)
{
    struct Dungeon* dungeon = comp->dungeon;
    long i = room->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature %ld detected",i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        if (!creature_under_spell_effect(thing, CSAfF_Speed)
        && !creature_is_immune_to_spell_effect(thing, CSAfF_Speed))
        {
            long n = get_creature_state_besides_move(thing);
            struct CreatureStateConfig* stati = get_thing_state_info_num(n);
            if (stati->state_type == CrStTyp_Work)
            {
                if (try_game_action(comp, dungeon->owner, GA_UsePwrSpeedUp, POWER_MAX_LEVEL, 0, 0, thing->index, 0) > Lb_OK)
                {
                    return thing;
                }
            }
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

struct Thing *computer_check_creatures_in_room_for_flight(struct Computer2 *comp, struct Room *room)
{
    struct Dungeon* dungeon = comp->dungeon;
    long i = room->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature %ld detected",i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        if (!creature_under_spell_effect(thing, CSAfF_Flying)
        && !creature_is_immune_to_spell_effect(thing, CSAfF_Flying))
        {
            long n = get_creature_state_besides_move(thing);
            struct CreatureStateConfig* stati = get_thing_state_info_num(n);
            if (stati->state_type == CrStTyp_Work)
            {
                if (try_game_action(comp, dungeon->owner, GA_UsePwrFlight, POWER_MAX_LEVEL, 0, 0, thing->index, 0) > Lb_OK)
                {
                    return thing;
                }
            }
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

struct Thing *computer_check_creatures_in_room_for_vision(struct Computer2 *comp, struct Room *room)
{
    struct Dungeon* dungeon = comp->dungeon;
    long i = room->creatures_list;
    unsigned long k = 0;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature %ld detected",i);
            break;
        }
        i = cctrl->next_in_room;
        // Per creature code
        if (!creature_under_spell_effect(thing, CSAfF_Sight)
        && !creature_is_immune_to_spell_effect(thing, CSAfF_Sight))
        {
            long n = get_creature_state_besides_move(thing);
            struct CreatureStateConfig* stati = get_thing_state_info_num(n);
            if (stati->state_type == CrStTyp_Work)
            {
                if (try_game_action(comp, dungeon->owner, GA_UsePwrVision, POWER_MAX_LEVEL, 0, 0, thing->index, 0) > Lb_OK)
                {
                    return thing;
                }
            }
        }
        // Per creature code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
    return INVALID_THING;
}

struct Thing *computer_check_creatures_in_dungeon_rooms_of_kind_for_accelerate(struct Computer2 *comp, RoomKind rkind)
{
    if ((rkind < 1) || (rkind > game.conf.slab_conf.room_types_count))
    {
        ERRORLOG("Invalid room kind %d",(int)rkind);
        return INVALID_THING;
    }
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid computer players dungeon");
        return INVALID_THING;
    }
    long i = dungeon->room_list_start[rkind];
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
        struct Thing* thing = computer_check_creatures_in_room_for_accelerate(comp, room);
        if (!thing_is_invalid(thing))
            return thing;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return INVALID_THING;
}

struct Thing *computer_check_creatures_in_dungeon_rooms_of_kind_for_flight(struct Computer2 *comp, RoomKind rkind)
{
    if ((rkind < 1) || (rkind > game.conf.slab_conf.room_types_count))
    {
        ERRORLOG("Invalid room kind %d",(int)rkind);
        return INVALID_THING;
    }
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid computer players dungeon");
        return INVALID_THING;
    }
    long i = dungeon->room_list_start[rkind];
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
        struct Thing* thing = computer_check_creatures_in_room_for_flight(comp, room);
        if (!thing_is_invalid(thing))
            return thing;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return INVALID_THING;
}

struct Thing *computer_check_creatures_in_dungeon_rooms_of_kind_for_vision(struct Computer2 *comp, RoomKind rkind)
{
    if ((rkind < 1) || (rkind > game.conf.slab_conf.room_types_count))
    {
        ERRORLOG("Invalid room kind %d",(int)rkind);
        return INVALID_THING;
    }
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Invalid computer players dungeon");
        return INVALID_THING;
    }
    long i = dungeon->room_list_start[rkind];
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
        struct Thing* thing = computer_check_creatures_in_room_for_vision(comp, room);
        if (!thing_is_invalid(thing))
            return thing;
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return INVALID_THING;
}

long computer_check_for_accelerate(struct Computer2 *comp, struct ComputerCheck * check)
{
    static RoomKind workers_in_rooms[] = {RoK_LIBRARY,RoK_LIBRARY,RoK_WORKSHOP,RoK_TRAINING,RoK_SCAVENGER};
    SYNCDBG(8,"Starting");
    int power_level = check->secondary_parameter;
    int amount = check->tertiary_parameter;
    if (!computer_able_to_use_power(comp, PwrK_SPEEDCRTR, power_level, amount))
    {
        return CTaskRet_Unk4;
    }
    long n = check->primary_parameter % (sizeof(workers_in_rooms) / sizeof(workers_in_rooms[0]));
    if (n <= 0)
        n = PLAYER_RANDOM(comp->dungeon->owner, sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    for (long i = 0; i < sizeof(workers_in_rooms) / sizeof(workers_in_rooms[0]); i++)
    {
        struct Thing* thing = computer_check_creatures_in_dungeon_rooms_of_kind_for_accelerate(comp, workers_in_rooms[n]);
        if (!thing_is_invalid(thing))
        {
            SYNCDBG(8,"Cast on thing %d",(int)thing->index);
            return CTaskRet_Unk1;
        }
        n = (n+1) % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    }
    return CTaskRet_Unk4;
}

long computer_check_for_flight(struct Computer2 *comp, struct ComputerCheck * check)
{
    static RoomKind workers_in_rooms[] = {RoK_LIBRARY,RoK_LIBRARY,RoK_WORKSHOP,RoK_TRAINING,RoK_SCAVENGER};
    SYNCDBG(8,"Starting");
    int power_level = check->secondary_parameter;
    int amount = check->tertiary_parameter;
    if (!computer_able_to_use_power(comp, PwrK_FLIGHT, power_level, amount))
    {
        return CTaskRet_Unk4;
    }
    long n = check->primary_parameter % (sizeof(workers_in_rooms) / sizeof(workers_in_rooms[0]));
    if (n <= 0)
        n = PLAYER_RANDOM(comp->dungeon->owner, sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    for (long i = 0; i < sizeof(workers_in_rooms) / sizeof(workers_in_rooms[0]); i++)
    {
        struct Thing* thing = computer_check_creatures_in_dungeon_rooms_of_kind_for_flight(comp, workers_in_rooms[n]);
        if (!thing_is_invalid(thing))
        {
            SYNCDBG(8,"Cast on thing %d",(int)thing->index);
            return CTaskRet_Unk1;
        }
        n = (n+1) % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    }
    return CTaskRet_Unk4;
}

long computer_check_for_vision(struct Computer2 *comp, struct ComputerCheck * check)
{
    static RoomKind workers_in_rooms[] = {RoK_LIBRARY,RoK_LIBRARY,RoK_WORKSHOP,RoK_TRAINING,RoK_SCAVENGER};
    SYNCDBG(8,"Starting");
    int power_level = check->secondary_parameter;
    int amount = check->tertiary_parameter;
    if (!computer_able_to_use_power(comp, PwrK_VISION, power_level, amount))
    {
        return CTaskRet_Unk4;
    }
    long n = check->primary_parameter % (sizeof(workers_in_rooms) / sizeof(workers_in_rooms[0]));
    if (n <= 0)
        n = PLAYER_RANDOM(comp->dungeon->owner, sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    for (long i = 0; i < sizeof(workers_in_rooms) / sizeof(workers_in_rooms[0]); i++)
    {
        struct Thing* thing = computer_check_creatures_in_dungeon_rooms_of_kind_for_vision(comp, workers_in_rooms[n]);
        if (!thing_is_invalid(thing))
        {
            SYNCDBG(8,"Cast on thing %d",(int)thing->index);
            return CTaskRet_Unk1;
        }
        n = (n+1) % (sizeof(workers_in_rooms)/sizeof(workers_in_rooms[0]));
    }
    return CTaskRet_Unk4;
}

long computer_check_slap_imps(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    long slap_percentage = check->primary_parameter;
    TbBool skip_imps_with_speed = check->secondary_parameter;
    struct Dungeon* dungeon = comp->dungeon;
    if (!is_power_available(dungeon->owner, PwrK_SLAP)) {
        return CTaskRet_Unk4;
    }
    long creatrs_num = slap_percentage * dungeon->num_active_diggers / 100;
    if (!is_task_in_progress(comp, CTT_SlapDiggers))
    {
        if (create_task_slap_imps(comp, creatrs_num, skip_imps_with_speed)) {
            return CTaskRet_Unk1;
        }
    }
    return CTaskRet_Unk4;
}

long computer_check_enemy_entrances(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    long result = CTaskRet_Unk4;
    for (PlayerNumber plyr_idx = 0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if (comp->dungeon->owner == plyr_idx) {
            continue;
        }
        if (players_are_mutual_allies(comp->dungeon->owner, plyr_idx)) {
            continue;
        }
        struct Dungeon* dungeon = get_dungeon(plyr_idx);
        long i = dungeon->room_list_start[RoK_ENTRANCE];
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
            struct OpponentRelation* oprel = &comp->opponent_relations[(int)plyr_idx];
            long n;
            for (n = 0; n < COMPUTER_SPARK_POSITIONS_COUNT; n++)
            {
                struct Coord3d* pos = &oprel->pos_A[n];
                if ((pos->x.val == subtile_coord(room->central_stl_x,0)) && (pos->y.val == subtile_coord(room->central_stl_y,0))) {
                    break;
                }
            }
            if (n == COMPUTER_SPARK_POSITIONS_COUNT)
            {
                n = oprel->next_idx;
                oprel->next_idx = (n + 1) % COMPUTER_SPARK_POSITIONS_COUNT;
                oprel->last_interaction_turn = game.play_gameturn;
                struct Coord3d* pos = &oprel->pos_A[n];
                pos->x.val = subtile_coord(room->central_stl_x,0);
                pos->y.val = subtile_coord(room->central_stl_y,0);
                pos->z.val = subtile_coord(1,0);
                result = CTaskRet_Unk2;
            }
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }
    return result;
}

static TbBool find_place_to_put_door_around_room(const struct Room *room, struct Coord3d *pos, PlayerNumber plyr_idx)
{
    long m = PLAYER_RANDOM(plyr_idx, SMALL_AROUND_SLAB_LENGTH);
    for (long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
    {
        // Get position containing room center
        MapSlabCoord slb_x = subtile_slab(room->central_stl_x);
        MapSlabCoord slb_y = subtile_slab(room->central_stl_y);
        // Move the position to edge of the room
        struct Room* sibroom = slab_room_get(slb_x, slb_y);
        while (!room_is_invalid(sibroom) && (sibroom->index == room->index))
        {
            slb_x += small_around[m].delta_x;
            slb_y += small_around[m].delta_y;
            sibroom = slab_room_get(slb_x, slb_y);
        }
        // Move the position a few tiles further in that direction searching for a place to put door
        //TODO COMPUTER_PLAYER Why we can only have doors if corridor is at center of the room? This should be fixed to allow doors everywhere around room.
        int i;
        for (i = 4; i > 0; i--)
        {
            struct SlabMap* slb = get_slabmap_block(slb_x, slb_y);
            if ((slabmap_owner(slb) != room->owner) || (slb->kind != SlbT_CLAIMED)) {
                i = 0;
                break;
            }
            if (tag_cursor_blocks_place_door(room->owner, slab_subtile_center(slb_x), slab_subtile_center(slb_y))) {
                break;
            }
            if (!subtile_has_door_thing_on(slab_subtile_center(slb_x), slab_subtile_center(slb_y))) {
                // No door - the position looks ok
                break;
            }
            slb_x += small_around[m].delta_x;
            slb_y += small_around[m].delta_y;
        }
        // Now if we were able to move, then the position seem ok. One last check - make sure the corridor is not dead end and doesn't already have a door
        if (i > 0)
        {
            MapSlabCoord nxslb_x = slb_x + small_around[m].delta_x;
            MapSlabCoord nxslb_y = slb_y + small_around[m].delta_y;
            struct SlabMap* nxslb = get_slabmap_block(nxslb_x, nxslb_y);
            struct SlabConfigStats* slabst = get_slab_stats(nxslb);
            if ((slabmap_owner(nxslb) == room->owner) && ((slabst->block_flags & SlbAtFlg_Filled) == 0))
            {
                if (!subtile_has_door_thing_on(slab_subtile_center(nxslb_x), slab_subtile_center(nxslb_y))) {
                    pos->x.val = subtile_coord_center(slab_subtile_center(slb_x));
                    pos->y.val = subtile_coord_center(slab_subtile_center(slb_y));
                    pos->z.val = subtile_coord(1,0);
                    return true;
                }
            }
        }
        m = (m + 1) % SMALL_AROUND_SLAB_LENGTH;
    }
    return false;
}

long computer_check_for_place_door(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    for (ThingModel doorkind = game.conf.trapdoor_conf.door_types_count; doorkind > 1; doorkind--)
    {
        if (dungeon->mnfct_info.door_amount_stored[doorkind] <= 0) {
            continue;
        }
        long rkind = check->primary_parameter;
        if (rkind == 0)
        {
            rkind = (check->secondary_parameter + 1) % game.conf.slab_conf.room_types_count;
            check->secondary_parameter = rkind;
        }
        unsigned long k = 0;
        long i = dungeon->room_list_start[rkind];
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
            pos.x.val = 0;
            pos.y.val = 0;
            pos.z.val = 0;
            if (find_place_to_put_door_around_room(room, &pos, dungeon->owner))
            {
                if (try_game_action(comp, dungeon->owner, GA_PlaceDoor, 0, pos.x.stl.num, pos.y.stl.num, doorkind, 0) > Lb_OK) {
                    return CTaskRet_Unk1;
                }
            }
            // Per-room code ends
            k++;
            if (k > ROOMS_COUNT)
            {
                ERRORLOG("Infinite loop detected when sweeping rooms list");
                break;
            }
        }
    }
    return CTaskRet_Unk4;
}

long computer_check_neutral_places(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    struct OpponentRelation* oprel = &comp->opponent_relations[game.neutral_player_num];
    struct Room* near_room = INVALID_ROOM;
    int near_dist = INT_MAX;
    struct Coord3d* near_pos = &oprel->pos_A[0];
    for (int i = 0; i < COMPUTER_SPARK_POSITIONS_COUNT; i++)
    {
        struct Coord3d* place = &oprel->pos_A[i];
        if ((place->x.val == 0) || (place->y.val == 0)) {
            continue;
        }
        struct Room* room = INVALID_ROOM;
        if (computer_finds_nearest_room_to_pos(comp, &room, place))
        {
            MapSubtlDelta dx = abs((int)room->central_stl_x - (MapSubtlDelta)place->x.stl.num);
            MapSubtlDelta dy = abs((int)room->central_stl_y - (MapSubtlDelta)place->y.stl.num);
            if (near_dist > dx+dy)
            {
                near_room = room;
                near_pos = place;
                near_dist = dx+dy;
            }
        }
    }
    if (room_is_invalid(near_room)) {
        return CTaskRet_Unk4;
    }
    struct Coord3d endpos;
    endpos.x.val = near_pos->x.val;
    endpos.y.val = near_pos->y.val;
    endpos.z.val = near_pos->z.val;
    struct Coord3d startpos;
    startpos.x.val = subtile_coord_center(stl_slab_center_subtile(near_room->central_stl_x));
    startpos.y.val = subtile_coord_center(stl_slab_center_subtile(near_room->central_stl_y));
    startpos.z.val = subtile_coord(1,0);
    if (!create_task_dig_to_neutral(comp, startpos, endpos)) {
        return CTaskRet_Unk4;
    }
    near_pos->x.val = 0;
    near_pos->y.val = 0;
    near_pos->z.val = 0;
    return CTaskRet_Unk1;
}

/**
 * Counts amount of slabs around given slab which have given kind and owner.
 * @param slb_x Target slab position, X coordinate.
 * @param slb_y Target slab position, Y coordinate.
 * @param slbkind Kind of the slabs to count.
 * @param owner Owner of the slabs to count.
 * @return Amount of matched slabs around given coordinates, 0..8.
 */
int count_slabs_around_of_kind(MapSlabCoord slb_x, MapSlabCoord slb_y, SlabKind slbkind, PlayerNumber owner)
{
    int matched_slabs = 0;
    for (unsigned long n = 1; n < MID_AROUND_LENGTH; n++)
    {
        MapSlabCoord arslb_x = slb_x + mid_around[n].delta_x;
        MapSlabCoord arslb_y = slb_y + mid_around[n].delta_y;
        struct SlabMap* slb = get_slabmap_block(arslb_x, arslb_y);
        if ((slb->kind == slbkind) && (slabmap_owner(slb) == owner)) {
            matched_slabs++;
        }
    }
    return matched_slabs;
}

/**
 * This function generates "expand room" action on a tile which is claimed ground and could have a room placed on.
 * It is used to fix vandalized or not fully built rooms, so that they will cover the whole area digged for them.
 *
 * @param comp
 * @param check Computer check data.
 * @param room The room to be checked for expand.
 * @param max_radius The max distance of the slab being put to the center of the room, in subtiles.
 * @param around_start Random value used for setting starting point of the check process.
 * @return
 */
TbBool computer_check_for_expand_specific_room(struct Computer2 *comp, struct ComputerCheck * check, struct Room *room, MapSubtlCoord max_radius, long around_start)
{
    struct Dungeon* dungeon = comp->dungeon;
    if (!is_room_available(dungeon->owner, room->kind)) {
        return false;
    }
    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        struct SlabMap* slb = get_slabmap_direct(i);
        MapSlabCoord slb_x = slb_num_decode_x(i);
        MapSlabCoord slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per-slab code
        int room_around = count_slabs_around_of_kind(slb_x, slb_y, slb->kind, dungeon->owner);
        int claimed_around = 0;
        if (room_around < 8) {
            claimed_around = count_slabs_around_of_kind(slb_x, slb_y, SlbT_CLAIMED, dungeon->owner);
        }
        if (((room_around >= 3) && (claimed_around >= 2) && (room_around+claimed_around < 8)) // If there's something besides room and claimed, then grow more aggressively
         || ((room_around >= 4) && (claimed_around >= 2) && (room_around+claimed_around >= 8)) // If we're in open space, don't expand that much
         || ((room_around >= 6) && (claimed_around >= 1))) // Allow fixing one-slab holes inside rooms
        {
            unsigned long m = around_start % SMALL_AROUND_SLAB_LENGTH;
            for (unsigned long n = 0; n < SMALL_AROUND_SLAB_LENGTH; n++)
            {
                MapSlabCoord arslb_x = slb_x + small_around[m].delta_x;
                MapSlabCoord arslb_y = slb_y + small_around[m].delta_y;
                MapSubtlCoord arstl_x = slab_subtile_center(arslb_x);
                MapSubtlCoord arstl_y = slab_subtile_center(arslb_y);
                long dist = grid_distance(room->central_stl_x, room->central_stl_y, arstl_x, arstl_y);
                if (dist <= max_radius)
                {
                    if (can_build_room_at_slab(dungeon->owner, room->kind, arslb_x, arslb_y))
                    {
                        if (try_game_action(comp, dungeon->owner, GA_PlaceRoom, 0, arstl_x, arstl_y, 1, room->kind) > Lb_OK) {
                            return true;
                        }
                    }
                }
                m = (m+1) % SMALL_AROUND_SLAB_LENGTH;
            }
        }
        // Per-slab code ends
        k++;
        if (k > room->slabs_count)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
    return false;
}

TbBool computer_check_for_expand_room_kind(struct Computer2 *comp, struct ComputerCheck * check, RoomKind rkind, long max_slabs, long around_start)
{
    struct Dungeon* dungeon = comp->dungeon;
    {
        struct RoomConfigStats* roomst = get_room_kind_stats(rkind);
        // If we don't have money for the room - don't even try
        // Check price for two slabs - after all, we don't want to end up having nothing
        if (2*roomst->cost >= dungeon->total_money_owned) {
            return false;
        }
    }
    // Don't allow the room to be made into long, narrow shape
    MapSubtlCoord max_radius = 3 * slab_subtile(LbSqrL(max_slabs), 2) / 4;
    long i = dungeon->room_list_start[rkind];
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
        if ((room->slabs_count > 0) && (room->slabs_count < max_slabs)) {
            if (computer_check_for_expand_specific_room(comp, check, room, max_radius, around_start)) {
                SYNCDBG(6,"The %s index %d will be expanded",room_code_name(room->kind),(int)room->index);
                return true;
            }
        }
        // Per-room code ends
        k++;
        if (k > ROOMS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping rooms list");
          break;
        }
    }
    return false;
}

long computer_check_for_expand_room(struct Computer2 *comp, struct ComputerCheck * check)
{
    SYNCDBG(8,"Starting");
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon_invalid(dungeon) || !player_has_heart(dungeon->owner)) {
        SYNCDBG(7,"Computer players %d dungeon in invalid or has no heart",(int)dungeon->owner);
        return CTaskRet_Unk4;
    }
    long around_start = PLAYER_RANDOM(dungeon->owner, 119);
    // Don't work when placing rooms; we could place in an area for room by mistake
    if (is_task_in_progress(comp, CTT_PlaceRoom) || is_task_in_progress(comp, CTT_CheckRoomDug)) {
        SYNCDBG(8,"No rooms expansion - colliding task already in progress");
        return CTaskRet_Unk0;
    }
    if (computer_player_in_emergency_state(comp)) {
        SYNCDBG(8,"No rooms expansion - emergency state");
        return CTaskRet_Unk0;
    }
    if (get_computer_money_less_cost(comp) < dungeon->creatures_total_pay / 3) {
        SYNCDBG(8,"No rooms expansion - not enough money buffer");
        return CTaskRet_Unk0;
    }
    for (const struct ExpandRooms* expndroom = &expand_rooms[0]; expndroom->rkind != RoK_NONE; expndroom++)
    {
        if (computer_check_for_expand_room_kind(comp, check, expndroom->rkind, expndroom->max_slabs, around_start)) {
            return CTaskRet_Unk1;
        }
    }
    SYNCDBG(8, "No rooms found for expansion");
    return CTaskRet_Unk0;
}

long computer_check_prison_tendency(struct Computer2* comp, struct ComputerCheck* check)
{
    SYNCDBG(8, "Starting");
    struct Dungeon* dungeon = comp->dungeon;
    struct PlayerInfo* player = get_player(comp->dungeon->owner);
    RoomRole rrole = get_room_role_for_job(Job_CAPTIVITY);

    int status = check->primary_parameter;
    int min_capacity = check->secondary_parameter;
    int max_units = check->tertiary_parameter;

    if (status == 0)
    {
        SYNCDBG(8, "Prison tendency handled manually by script, aborting.");
        return CTaskRet_Unk1;
    }
    int total_capacity = computer_get_room_role_total_capacity(comp, rrole);
    // Enough prison capacity to enable imprisonment
    if ((total_capacity >= min_capacity) && (dungeon->num_active_creatrs < max_units))
    {
        if ((dungeon->creature_tendencies & 0x01) != 0)
        {
            SYNCDBG(8, "Prison tendency already enabled");
            return CTaskRet_Unk1;
        }
        if (status != 3)
        {
            if (set_creature_tendencies(player, CrTend_Imprison, true))
            {
                if (is_my_player(player)) {
                    dungeon = get_players_dungeon(player);
                    game.creatures_tend_imprison = ((dungeon->creature_tendencies & 0x01) != 0);
                    game.creatures_tend_flee = ((dungeon->creature_tendencies & 0x02) != 0);
                }
                SYNCDBG(18, "Player %d has enabled imprisonment with %d total prison capacity", player->id_number, total_capacity);
                return CTaskRet_Unk1;
            }
            else
            {
                ERRORLOG("Failed to enable prison tendency");
                return CTaskRet_Unk4;
            }
        }
        else
        {
            SYNCDBG(8, "Enabling prison tendency handled manually by script, aborting.");
            return CTaskRet_Unk1;
        }
    }
    // Not enough prison capacity to keep imprisonment enabled, or too many units
    else
    {
        if ((dungeon->creature_tendencies & 0x01) == 0)
        {
            SYNCDBG(8, "Prison tendency already disabled");
            return CTaskRet_Unk1;
        }
        if (status != 2)
        {
            if (set_creature_tendencies(player, CrTend_Imprison, false))
            {
                if (is_my_player(player)) {
                    dungeon = get_players_dungeon(player);
                    game.creatures_tend_imprison = ((dungeon->creature_tendencies & 0x01) != 0);
                    game.creatures_tend_flee = ((dungeon->creature_tendencies & 0x02) != 0);
                }
                SYNCDBG(18, "Player %d has disabled imprisonment with %d total prison capacity", player->id_number, total_capacity);
                return CTaskRet_Unk1;
            }
            else
            {
                ERRORLOG("Failed to disable prison tendency");
                return CTaskRet_Unk4;
            }
        }
        else
        {
            SYNCDBG(8, "Disabling prison tendency handled manually by script, aborting.");
            return CTaskRet_Unk1;
        }
    }

 return CTaskRet_Unk0;
}
/******************************************************************************/
