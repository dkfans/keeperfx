/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_compevents.c
 *     Computer player events definitions and routines.
 * @par Purpose:
 *     Defines a computer player events and related functions.
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

#include "config.h"
#include "magic_powers.h"
#include "player_instances.h"
#include "config_terrain.h"
#include "creature_instances.h"
#include "creature_states_combt.h"
#include "creature_states.h"
#include "creature_states_lair.h"
#include "power_hand.h"
#include "player_computer.h"

#include "dungeon_data.h"
#include "game_legacy.h"
#include "map_utils.h"
#include "map_data.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_save_tortured(struct Computer2 *comp, struct ComputerEvent *cevent);
long computer_event_rebuild_room(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event);
long computer_event_handle_prisoner(struct Computer2 *comp, struct ComputerEvent* cevent, struct Event *event);
long computer_event_attack_door(struct Computer2* comp, struct ComputerEvent* cevent, struct Event* event);
long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);

/******************************************************************************/
struct ComputerSpells {
    PowerKind pwkind;
    char gaction;
    char require_owned_ground;
    int repeat_num;
    KeepPwrLevel power_level;
    int amount_able;
};
/******************************************************************************/
const struct NamedCommand computer_event_test_func_type[] = {
  {"event_battle_test",       1,},
  {"event_check_fighters",    2,},
  {"event_attack_magic_foe",  3,},
  {"event_check_rooms_full",  4,},
  {"event_check_imps_danger", 5,},
  {"event_save_tortured",     6,},
  {"none",                    7,},
  {NULL,                      0,},
};

Comp_EvntTest_Func computer_event_test_func_list[] = {
  NULL,
  computer_event_battle_test,
  computer_event_check_fighters,
  computer_event_attack_magic_foe,
  computer_event_check_rooms_full,
  computer_event_check_imps_in_danger,
  computer_event_save_tortured,
  NULL,
  NULL,
};

const struct NamedCommand computer_event_func_type[] = {
  {"event_battle",            1,},
  {"event_find_link",         2,},
  {"event_check_payday",      3,},
  {"event_rebuild_room",      4,},
  {"event_handle_prisoner",   5,},
  {"event_attack_door",       6,},
  {"none",                    7,},
  {NULL,                      0,},
};

Comp_Event_Func computer_event_func_list[] = {
  NULL,
  computer_event_battle,
  computer_event_find_link,
  computer_event_check_payday,
  computer_event_rebuild_room,
  computer_event_handle_prisoner,
  computer_event_attack_door,
  NULL,
};

//PowerKind pwkind; char gaction; char require_owned_ground; int repeat_num; KeepPwrLevel power_level; int amount_able;
struct ComputerSpells computer_attack_spells[] = {
  {PwrK_DISEASE,   GA_UsePwrDisease,   1,  1, 2, 4},
  {PwrK_LIGHTNING, GA_UsePwrLightning, 0,  1, 8, 2},
  {PwrK_CHICKEN,   GA_UsePwrChicken,   1,  1, 2, 1},
  {PwrK_FREEZE,    GA_UsePwrFreeze,    1,  1, 1, 1},
  {PwrK_SLOW,      GA_UsePwrSlow,      1,  1, 1, 1},
  {PwrK_LIGHTNING, GA_UsePwrLightning, 0, -1, 1, 1},
  {PwrK_None,      GA_None,            0,  0, 0, 0},
};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
TbBool get_computer_drop_position_near_subtile(struct Coord3d *pos, struct Dungeon *dungeon, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if ((stl_x <= 0) || (stl_y <= 0)) {
        return false;
    }
    struct CompoundCoordFilterParam param;
    param.plyr_idx = dungeon->owner;
    param.slab_kind = -1;
    param.primary_number = 0;
    param.secondary_number = 0;
    param.tertiary_number = 0;
    return get_position_spiral_near_map_block_with_filter(pos,
        subtile_coord_center(stl_x), subtile_coord_center(stl_y),
        81, near_coord_filter_battle_drop_point, &param);
}

TbBool get_computer_drop_position_next_to_subtile(struct Coord3d* pos, struct Dungeon* dungeon, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    if ((stl_x <= 0) || (stl_y <= 0)) {
        return false;
    }
    struct CompoundCoordFilterParam param;
    param.plyr_idx = dungeon->owner;
    param.slab_kind = -1;
    param.primary_number = 0;
    param.secondary_number = 0;
    param.tertiary_number = 0;
    return get_position_next_to_map_block_with_filter(pos,
        subtile_coord_center(stl_x), subtile_coord_center(stl_y),
        near_coord_filter_battle_drop_point, &param);
}

long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event)
{
    SYNCDBG(18,"Starting for %s",cevent->name);
    struct Coord3d pos;
    if (!get_computer_drop_position_near_subtile(&pos, comp->dungeon, coord_subtile(event->mappos_x), coord_subtile(event->mappos_y))) {
        SYNCDBG(8,"No drop position near (%d,%d) for %s",(int)coord_subtile(event->mappos_x),(int)coord_subtile(event->mappos_y),cevent->name);
        return CTaskRet_Unk0;
    }
    // Check if there are any enemies in the vicinity - no enemies, don't drop creatures
    struct Thing* enmtng = get_creature_in_range_who_is_enemy_of_able_to_attack_and_not_specdigger(pos.x.val, pos.y.val, 21, comp->dungeon->owner);
    if (thing_is_invalid(enmtng))
    {
        SYNCDBG(8,"No enemies near %s",cevent->name);
        return CTaskRet_Unk0;
    }
    long creatrs_def = count_creatures_for_defend_pickup(comp);
    long creatrs_num = creatrs_def * (long)cevent->primary_parameter / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0)) {
        creatrs_num = 1;
    }
    if (creatrs_num <= 0) {
        SYNCDBG(8,"No creatures to drop for %s",cevent->name);
        return CTaskRet_Unk0;
    }
    if (!computer_find_safe_non_solid_block(comp, &pos)) {
        SYNCDBG(8,"Drop position is solid for %s",cevent->name);
        return CTaskRet_Unk0;
    }
    if (computer_able_to_use_power(comp, PwrK_HAND, 1, creatrs_num))
    {
        if (!is_task_in_progress(comp, CTT_MoveCreaturesToDefend) || ((cevent->secondary_parameter & 0x02) != 0))
        {
            if (!create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->secondary_parameter)) {
                SYNCDBG(18,"Cannot move to defend for %s",cevent->name);
                return CTaskRet_Unk0;
            }
            return CTaskRet_Unk1;
        }
        return CTaskRet_Unk4;
    }
    if (computer_able_to_use_power(comp, PwrK_CALL2ARMS, 1, 1))
    {
        if (!is_task_in_progress(comp, CTT_MagicCallToArms) || ((cevent->secondary_parameter & 0x02) != 0))
        {
            if (check_call_to_arms(comp))
            {
                if (!create_task_magic_battle_call_to_arms(comp, &pos, 2500, creatrs_num)) {
                    SYNCDBG(18,"Cannot call to arms for %s",cevent->name);
                    return CTaskRet_Unk0;
                }
                return CTaskRet_Unk1;
            }
        }
    }
    SYNCDBG(18,"No hand nor CTA, giving up with %s",cevent->name);
    return CTaskRet_Unk0;
}

long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
    long cproc_idx = 0;
    for (int i = 0; i < COMPUTER_PROCESSES_COUNT + 1; i++)
    {
        struct ComputerProcess* cproc = &comp->processes[i];
        if (flag_is_set(cproc->flags, ComProc_ListEnd))
            break;
        if (cproc->parent == cevent->process)
        {
            clear_flag(cproc->flags, (ComProc_Unkn0008|ComProc_Unkn0001|ComProc_Unkn0004));
            cproc->last_run_turn = 0;
            cproc_idx = 1;
        }
    }
    return cproc_idx;
}

/**
 * Finds computer player creature which is currently in a fight.
 * @param comp
 * @return
 */
struct Thing *find_creature_in_fight_with_enemy(struct Computer2 *comp)
{
    struct CreatureControl *cctrl;
    struct Thing *creatng;
    struct Dungeon* dungeon = comp->dungeon;
    // Search through special diggers
    unsigned long k = 0;
    int i = dungeon->digger_list_start;
    while (i != 0)
    {
        creatng = thing_get(i);
        cctrl = creature_control_get_from_thing(creatng);
        if (!thing_is_creature(creatng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->combat_flags != 0)
        {
            if (creature_is_being_attacked_by_enemy_player(creatng)) {
                return creatng;
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
    // Search through normal creatures
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        creatng = thing_get(i);
        cctrl = creature_control_get_from_thing(creatng);
        if (!thing_is_creature(creatng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->combat_flags != 0) {
            if (creature_is_being_attacked_by_enemy_player(creatng)) {
                return creatng;
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
    return INVALID_THING;
}

long computer_event_battle_test(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    if (comp->dungeon->fights_num <= 0) {
        return CTaskRet_Unk4;
    }
    struct Thing* creatng = find_creature_in_fight_with_enemy(comp);
    if (thing_is_invalid(creatng)) {
        return CTaskRet_Unk4;
    }
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    long creatrs_def = count_creatures_for_defend_pickup(comp);
    long creatrs_num = creatrs_def * (long)cevent->primary_parameter / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0)) {
        creatrs_num = 1;
    }
    if (creatrs_num <= 0) {
        return CTaskRet_Unk4;
    }
    if (!computer_find_safe_non_solid_block(comp, &pos)) {
        return CTaskRet_Unk4;
    }
    if (computer_able_to_use_power(comp, PwrK_HAND, 1, creatrs_num))
    {
        if (!is_task_in_progress(comp, CTT_MoveCreaturesToDefend))
        {
            if (!create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->secondary_parameter)) {
                return CTaskRet_Unk4;
            }
            return CTaskRet_Unk1;
        }
        return CTaskRet_Unk4;
    }
    if (computer_able_to_use_power(comp, PwrK_CALL2ARMS, 8, 1))
    {
        if (!is_task_in_progress(comp, CTT_MagicCallToArms))
        {
            if (check_call_to_arms(comp))
            {
                if (!create_task_magic_battle_call_to_arms(comp, &pos, 2500, creatrs_num)) {
                    return CTaskRet_Unk4;
                }
                return CTaskRet_Unk1;
            }
        }
    }
    return CTaskRet_Unk4;
}

/**
 * Returns a creature in fight which gives highest score value.
 * @return The thing in fight, or invalid thing if not found.
 */
struct Thing *computer_get_creature_in_fight(struct Computer2 *comp, PowerKind pwkind)
{
    struct PowerConfigStats *powerst = get_power_model_stats(pwkind);
    return find_players_highest_score_creature_in_fight_not_affected_by_spell(comp->dungeon->owner, powerst->spell_idx);
}

long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    if (comp->dungeon->fights_num <= 0)
    {
        return CTaskRet_Unk4;
    }
    if (!(computer_able_to_use_power(comp, PwrK_SPEEDCRTR, cevent->primary_parameter, 1) || computer_able_to_use_power(comp, PwrK_PROTECT, cevent->primary_parameter, 1) ||
          computer_able_to_use_power(comp, PwrK_REBOUND, cevent->primary_parameter, 1)   || computer_able_to_use_power(comp, PwrK_FLIGHT, cevent->primary_parameter, 1) ||
          computer_able_to_use_power(comp, PwrK_VISION, cevent->primary_parameter, 1)))
    {
        return CTaskRet_Unk4;
    }
    struct Thing* fightng = computer_get_creature_in_fight(comp, PwrK_SPEEDCRTR);
    if (thing_is_invalid(fightng))
    {
        fightng = computer_get_creature_in_fight(comp, PwrK_PROTECT);
        if (thing_is_invalid(fightng))
        {
            fightng = computer_get_creature_in_fight(comp, PwrK_REBOUND);
            if (thing_is_invalid(fightng))
            {
                fightng = computer_get_creature_in_fight(comp, PwrK_FLIGHT);
                if (thing_is_invalid(fightng))
                {
                    fightng = computer_get_creature_in_fight(comp, PwrK_VISION);
                    if (thing_is_invalid(fightng))
                    {
                        return CTaskRet_Unk4;
                    }
                }
            }
        }
    }
    if (!is_task_in_progress(comp, CTT_MagicSpeedUp))
    {
        if (!create_task_magic_speed_up(comp, fightng, cevent->primary_parameter))
        {
            return CTaskRet_Unk4;
        }
    }
    return CTaskRet_Unk1;
}

PowerKind computer_choose_attack_spell(struct Computer2 *comp, struct ComputerEvent *cevent, struct Thing *creatng)
{
    struct Dungeon* dungeon = comp->dungeon;
    struct PowerConfigStats *powerst;
    struct SpellConfig *spconf;
    int i = (cevent->tertiary_parameter + 1) % (sizeof(computer_attack_spells) / sizeof(computer_attack_spells[0]));
    // Do the loop if we've reached starting value
    while (i != cevent->tertiary_parameter)
    {
        struct ComputerSpells* caspl = &computer_attack_spells[i];
        // If we've reached end of array, loop it
        if (caspl->pwkind == PwrK_None) {
            i = 0;
            continue;
        }

        // Only cast lightning on imps, don't waste expensive chicken or disease spells
        if ((thing_is_creature_digger(creatng)) && (caspl->pwkind != PwrK_LIGHTNING))
        {
            i++;
            continue;
        }

        if (can_cast_spell(dungeon->owner, caspl->pwkind, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, creatng, CastChk_Default))
        {
            powerst = get_power_model_stats(caspl->pwkind);
            spconf = get_spell_config(powerst->spell_idx);
            if (!creature_under_spell_effect(creatng, spconf->spell_flags)
            && !creature_is_immune_to_spell_effect(creatng, spconf->spell_flags))
            {
                if (computer_able_to_use_power(comp, caspl->pwkind, cevent->primary_parameter, caspl->amount_able)) {
                    cevent->tertiary_parameter = i;
                    return caspl->pwkind;
                }
            }
        }
        i++;
    }
    return PwrK_None;
}

long computer_event_attack_magic_foe(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon->fights_num <= 0) {
        return CTaskRet_Unk4;
    }
    // TODO COMPUTER_PLAYER Do we really only want to help creature with the highest score?
    struct Thing* fightng = computer_get_creature_in_fight(comp, PwrK_None);
    if (thing_is_invalid(fightng)) {
        return CTaskRet_Unk4;
    }
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    struct Thing* creatng = thing_get(figctrl->combat.battle_enemy_idx);
    if (!thing_is_creature(creatng) || creature_is_dying(creatng)) {
        return CTaskRet_Unk4;
    }
    if (creatng->owner == fightng->owner)
    {
        //TODO: Stop computer from initiating attack event on friendly fights
        return CTaskRet_Unk4;
    }
    PowerKind pwkind = computer_choose_attack_spell(comp, cevent, creatng);
    if (pwkind == PwrK_None) {
        return CTaskRet_Unk4;
    }
    struct ComputerSpells* caspl = &computer_attack_spells[cevent->tertiary_parameter];
    int repeat_num = caspl->repeat_num;
    if (repeat_num < 0)
    {
        repeat_num = cevent->secondary_parameter;
    }
    KeepPwrLevel power_level = caspl->power_level;
    int gaction = caspl->gaction;
    if (!is_task_in_progress(comp, CTT_AttackMagic))
    {
        // Create the new task
        if (!create_task_attack_magic(comp, creatng, pwkind, repeat_num, power_level, gaction)) {
            return CTaskRet_Unk4;
        }
    }
    return CTaskRet_Unk1;
}

long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    SYNCDBG(18,"Starting");
    long ret = CTaskRet_Unk4;
    TbBool emergency_state = computer_player_in_emergency_state(comp);
    for (struct ValidRooms* bldroom = valid_rooms_to_build; bldroom->rkind > 0; bldroom++)
    {
        if (computer_get_room_kind_free_capacity(comp, bldroom->rkind) > 0) {
            continue;
        }
        struct RoomConfigStats* roomst = &game.conf.slab_conf.room_cfgstats[bldroom->rkind];
        int tiles = get_room_slabs_count(comp->dungeon->owner,bldroom->rkind);
        if ((tiles >= cevent->tertiary_parameter) && !(cevent->tertiary_parameter == 0)) // Room has reached the preconfigured maximum size
        {
            SYNCDBG(8,"Player %d reached maximum size %d for %s",(int)comp->dungeon->owner,tiles,room_code_name(bldroom->rkind));
            if (room_role_matches(bldroom->rkind, RoRoF_CratesManufctr))
            {
                struct Dungeon* dungeon = comp->dungeon;
                int32_t used_capacity;
                int32_t total_capacity;
                int32_t storaged_capacity;
                get_room_kind_total_used_and_storage_capacity(dungeon, bldroom->rkind, &total_capacity, &used_capacity, &storaged_capacity);
                if ((cevent->secondary_parameter != 0) && (storaged_capacity > (used_capacity * cevent->secondary_parameter / 100)))
                {
                    if (!is_task_in_progress(comp, CTT_SellTrapsAndDoors))
                    {
                        create_task_sell_traps_and_doors(comp, storaged_capacity/3*2 ,100000,false);
                        SYNCDBG(8,"Player %d to sell crates to free up space in %s",(int)comp->dungeon->owner,room_code_name(bldroom->rkind));
                    }
                }
            }
            continue;
        } else
        {
            if (emergency_state && ((roomst->flags & RoCFlg_BuildTillBroke) == 0)) {
                continue;
            }
            SYNCDBG(8,"Player %d needs %s",(int)comp->dungeon->owner,room_code_name(bldroom->rkind));
            // Find the corresponding build process and mark it as needed
            for (long i = 0; i <= COMPUTER_PROCESSES_COUNT; i++)
            {
                struct ComputerProcess* cproc = &comp->processes[i];
                if (flag_is_set(cproc->flags, ComProc_ListEnd))
                    break;
                if (cproc->parent == bldroom->process_idx)
                {
                    SYNCDBG(8,"Player %d will allow process \"%s\"",(int)comp->dungeon->owner,cproc->name);
                    ret = CTaskRet_Unk1;
                    reactivate_build_process(comp, bldroom->rkind);
                }
            }
        }
    }
    return ret;
}

long computer_event_attack_door(struct Computer2* comp, struct ComputerEvent* cevent, struct Event* event)
{
    SYNCDBG(18, "Starting for %s", cevent->name);
    struct Thing* thing = thing_get(event->target);
    if (!thing_is_deployed_door(thing))
    {
        SYNCDBG(8, "Target %s is not a door", thing_model_name(thing));
        return CTaskRet_Unk0;
    }
    if (!players_are_enemies(comp->dungeon->owner, thing->owner))
    {
        SYNCDBG(8, "Door owner is no longer an enemy");
        return CTaskRet_Unk0;
    }

    struct Coord3d freepos;
    if (!get_computer_drop_position_next_to_subtile(&freepos, comp->dungeon, coord_subtile(event->mappos_x), coord_subtile(event->mappos_y))) {
        SYNCDBG(18, "No drop position near (%d,%d) for %s", (int)coord_subtile(event->mappos_x), (int)coord_subtile(event->mappos_y), cevent->name);
        return CTaskRet_Unk0;
    }

    int32_t creatrs_def = count_creatures_for_defend_pickup(comp);
    if (creatrs_def < cevent->primary_parameter)
    {
        SYNCDBG(18, "Not enough creatures for event %s", cevent->name);
        return CTaskRet_Unk4;
    }

    if (computer_able_to_use_power(comp, PwrK_HAND, 1, 1))
    {
        if (!is_task_in_progress_using_hand(comp))
        {
            struct Thing* creatng = find_creature_for_defend_pickup(comp);
            if (thing_is_invalid(creatng))
            {
                SYNCDBG(18, "Invalid creature selected for event %s", cevent->name);
                return CTaskRet_Unk4;
            }
            if (!create_task_move_creature_to_pos(comp, creatng, freepos, CrSt_CreatureDoorCombat))
            {
                SYNCDBG(18, "Cannot move to position for event %s", cevent->name);
                return CTaskRet_Unk0;
            }
            return CTaskRet_Unk1;
        }
    }

    if (computer_able_to_use_power(comp, PwrK_CALL2ARMS, 1, 1))
    {
        if (!is_task_in_progress(comp, CTT_MagicCallToArms))
        {
            if (check_call_to_arms(comp))
            {
                if (!create_task_magic_battle_call_to_arms(comp, &freepos, cevent->secondary_parameter, cevent->tertiary_parameter))
                {
                    if (computer_find_safe_non_solid_block(comp, &freepos))
                    {
                        if (!create_task_magic_battle_call_to_arms(comp, &freepos, cevent->secondary_parameter, cevent->tertiary_parameter))
                        {
                            SYNCDBG(18, "Cannot call to arms for %s", cevent->name);
                            return CTaskRet_Unk0;
                        }
                        return CTaskRet_Unk1;
                    }
                    else
                    {
                        SYNCDBG(8, "Drop position is solid for %s", cevent->name);
                        return CTaskRet_Unk0;
                    }

                }
                return CTaskRet_Unk1;
            }
        }
    }
    SYNCDBG(18, "No hand nor CTA, giving up with %s", cevent->name);
    return CTaskRet_Unk0;
}

long computer_event_handle_prisoner(struct Computer2* comp, struct ComputerEvent* cevent, struct Event* event)
{
    SYNCDBG(18, "Starting");
    struct Dungeon* dungeon = comp->dungeon;
    struct Thing* creatng = thing_get(event->target);
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    struct Room* destroom;

    int actions_allowed = cevent->primary_parameter;
    int power_level = cevent->secondary_parameter;
    int amount = cevent->tertiary_parameter;

    if (actions_allowed == 0)
    {
        return CTaskRet_Unk1;
    }

    if (dungeon_has_room_of_role(dungeon, RoRoF_Torture) && (!creature_is_being_tortured(creatng))) // avoid repeated action on same unit
    {
        if (!creature_would_benefit_from_healing(creatng))
        {
            if (!computer_able_to_use_power(comp, PwrK_HAND, 1, 1)) {
                    return CTaskRet_Unk4;
            }
            if (is_task_in_progress_using_hand(comp)) {
                    return CTaskRet_Unk4;
            }
            destroom = find_room_of_role_with_spare_capacity(dungeon->owner, RoRoF_Torture, 1);
            if (!room_is_invalid(destroom))
            {
                if (create_task_move_creature_to_subtile(comp, creatng, destroom->central_stl_x, destroom->central_stl_y, CrSt_Torturing))
                {
                    return CTaskRet_Unk1;
                }
            }
            else
            {
                // wait for capacity to free up.
                return CTaskRet_Unk4;
            }
        }
        else if (cctrl->instance_available[CrInst_HEAL] == 0)
        {
            if (((!crconf->humanoid_creature) && (actions_allowed >= 2)) || (actions_allowed == 2)) // 1 = move only, 2 = everybody, 3 = non_humanoids
            {
                if (computer_able_to_use_power(comp, PwrK_HEALCRTR, power_level, amount))
                {
                    magic_use_available_power_on_thing(comp->dungeon->owner, PwrK_HEALCRTR, power_level, 0, 0, creatng, PwMod_Default);
                    return CTaskRet_Unk1;
                }
                return CTaskRet_Unk4;
            }
        }
    }
    return CTaskRet_Unk1;
}

long computer_event_rebuild_room(struct Computer2* comp, struct ComputerEvent* cevent, struct Event* event)
{
    SYNCDBG(18, "Starting");
    if (count_slabs_of_room_type(comp->dungeon->owner, event->target) == 0)
    {
        for (int i = 0; i < COMPUTER_PROCESSES_COUNT + 1; i++)
        {
            struct ComputerProcess* cproc = &comp->processes[i];
            if (flag_is_set(cproc->flags, ComProc_ListEnd))
                break;
            if ((cproc->func_check == cpfl_computer_check_any_room) && (cproc->process_configuration_value_4 == event->target))
            {
                SYNCDBG(8,"Resetting process for player %d to build room %s", (int)comp->dungeon->owner, room_code_name(event->target));
                clear_flag(cproc->flags, (ComProc_Unkn0008|ComProc_Unkn0001));
                cproc->last_run_turn = 0;
            }
        }
    }
    return CTaskRet_Unk1;
}

long computer_event_save_tortured(struct Computer2* comp, struct ComputerEvent* cevent)
{
    struct Dungeon* dungeon = comp->dungeon;
    int health_permil = (cevent->primary_parameter * 10);
    // Do not check for PwrK_HAND here; this would prevent the computer from taking other actions without it!
    // Do we have a prison to put the unit back into?
    struct Room* destroom = NULL;
    TbBool can_return = false;
    if (dungeon_has_room(dungeon, RoK_PRISON))
    {
        destroom = find_room_of_role_with_spare_capacity(dungeon->owner, RoRoF_Prison, 1);
        if (!room_is_invalid(destroom))
        {
            can_return = true;
        }
    }

    unsigned long moved = 0;
    unsigned long slapped = 0;
    struct Dungeon* victdungeon;
    for (int j = 0; j < DUNGEONS_COUNT; j++)
    {
        if (j == comp->dungeon->owner)
        {
            continue;
        }
        victdungeon = get_dungeon(j);
        int i = victdungeon->creatr_list_start;
        while (i != 0)
        {
            struct Thing* creatng = thing_get(i);
            struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
            if (!thing_is_creature(creatng) || creature_control_invalid(cctrl))
            {
                ERRORLOG("Jump to invalid creature detected");
                break;
            }
            i = cctrl->players_next_creature_idx;
            if (!creature_is_being_tortured(creatng))
            {
                continue;
            }
            if (get_creature_health_permil(creatng) > health_permil)
            {
                continue;
            }
            struct Room* room = get_room_thing_is_on(creatng);
            if (room->owner == creatng->owner)
            {
                continue;
            }
            //We found a unit in our torture room that's in need of healing.
            if ((cctrl->instance_available[CrInst_HEAL] != 0) && (cctrl->slap_turns == 0))
            {
                //slap creature so he will heal himself
                if (can_cast_spell(dungeon->owner, PwrK_SLAP, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, creatng, CastChk_Default))
                {
                    struct CreatureModelConfig* crconf;
                    crconf = creature_stats_get_from_thing(creatng);
                    // Check if the slap may cause death
                    if ((crconf->slaps_to_kill < 1) || (get_creature_health_permil(creatng) >= 2 * 1000 / crconf->slaps_to_kill))
                    {
                        if (try_game_action(comp, dungeon->owner, GA_UsePwrSlap, 0, 0, 0, creatng->index, 0) > Lb_OK)
                        {
                            slapped++;
                            continue;
                        }

                    }
                }
            }

            //move back to prison
            if (can_return == true)
            {
                // If we don't have the power to pick up creatures, fail now
                if (!computer_able_to_use_power(comp, PwrK_HAND, 1, 1)) {
                    return CTaskRet_Unk4;
                }
                if (is_task_in_progress_using_hand(comp)) {
                    return CTaskRet_Unk4;
                }
                if (create_task_move_creature_to_subtile(comp, creatng, destroom->central_stl_x, destroom->central_stl_y, CrSt_CreatureInPrison))
                {
                    moved++;
                }
            }
        }
    }
    return CTaskRet_Unk1;
}

long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon->fights_num <= 0) {
        return CTaskRet_Unk4;
    }
    // Do not check for PwrK_HAND here; this would prevent the computer from taking other actions without it!
    long result = CTaskRet_Unk4;
    // Search through special diggers
    unsigned long k = 0;
    int i = dungeon->digger_list_start;
    while (i != 0)
    {
        struct Thing* creatng = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        if (!thing_is_creature(creatng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if ((cctrl->combat_flags & (CmbtF_Melee|CmbtF_Ranged)) != 0)
        {
            if (!creature_is_being_unconscious(creatng) && !creature_under_spell_effect(creatng, CSAfF_Chicken))
            {
                // Small chance to casting invisibility,on imp in battle.
                if ((THING_RANDOM(creatng, 150) == 1)
                && computer_able_to_use_power(comp, PwrK_CONCEAL, 8, 1)
                && !creature_under_spell_effect(creatng, CSAfF_Invisibility)
                && !creature_is_immune_to_spell_effect(creatng, CSAfF_Invisibility))
                { // TODO: check if PwrK_CONCEAL is still applying Invisibility, in case it changes?
                    magic_use_available_power_on_thing(creatng->owner, PwrK_CONCEAL, 8, 0, 0, creatng, PwMod_Default);
                }
                else if (!creature_is_being_dropped(creatng) && can_thing_be_picked_up_by_player(creatng, dungeon->owner))
                {
                    TbBool needs_help;
                    if (get_creature_health_permil(creatng) < 500)
                    {
                        needs_help = true;
                    } else
                    {
                        needs_help = creature_is_being_attacked_by_enemy_creature_not_digger(creatng);
                    }
                    if (needs_help)
                    {
                        // PwrK_HAND check should not be needed here, because we've already done that with can_thing_be_picked_up_by_player
                        if (is_task_in_progress_using_hand(comp)) {
                            return CTaskRet_Unk4;
                        }
                        // Move creature to heart, unless it already is near the heart
                        struct Thing* heartng = get_player_soul_container(dungeon->owner);
                        if (get_2d_distance(&creatng->mappos, &heartng->mappos) > subtile_coord(16,0))
                        {
                            if (!create_task_move_creature_to_subtile(comp, creatng,
                                heartng->mappos.x.stl.num, heartng->mappos.y.stl.num, CrSt_ImpDoingNothing)) {
                                break;
                            }
                            result = CTaskRet_Unk1;
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
    return result;
}

long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon->total_money_owned >= dungeon->creatures_total_pay) {
        return CTaskRet_Unk4;
    }
    if (dungeon_has_any_buildable_traps(dungeon) || dungeon_has_any_buildable_doors(dungeon) ||
        player_has_deployed_trap_of_model(dungeon->owner, -1) || player_has_deployed_door_of_model(dungeon->owner, -1, 0))
    {
        if (!is_task_in_progress(comp, CTT_SellTrapsAndDoors))
        {
            SYNCDBG(8,"Creating task to sell player %d traps and doors",(int)dungeon->owner);
            if (create_task_sell_traps_and_doors(comp, cevent->primary_parameter, 3*(dungeon->creatures_total_pay-dungeon->total_money_owned)/2,true)) {
                return CTaskRet_Unk1;
            }
        }
    }
    // Move any gold laying around to treasure room
    if (dungeon_has_room_of_role(dungeon, RoRoF_GoldStorage))
    {
        // If there's already task in progress which uses hand, then don't add more
        // content; the hand could be used by wrong task by mistake
        if (!is_task_in_progress_using_hand(comp) && computer_able_to_use_power(comp, PwrK_HAND, 1, cevent->secondary_parameter))
        {
            SYNCDBG(8,"Creating task to move neutral gold to treasury");
            if (create_task_move_gold_to_treasury(comp, cevent->secondary_parameter, 3*dungeon->creatures_total_pay/2)) {
                return CTaskRet_Unk1;
            }
        }
    }

    return CTaskRet_Unk4;
}

/******************************************************************************/
