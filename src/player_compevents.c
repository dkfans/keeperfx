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
#include "player_computer.h"

#include <limits.h>
#include <string.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"

#include "config.h"
#include "magic.h"
#include "player_instances.h"
#include "config_terrain.h"
#include "creature_states_combt.h"
#include "creature_states.h"
#include "power_hand.h"

#include "dungeon_data.h"
#include "game_legacy.h"
#include "map_utils.h"

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
long computer_event_check_payday(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
long computer_event_breach(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event);

/******************************************************************************/
struct ComputerSpells {
    PowerKind pwkind;
    char gaction;
    char require_owned_ground;
    int repeat_num;
    int pwlevel;
    int amount_able;
};
/******************************************************************************/
const struct NamedCommand computer_event_test_func_type[] = {
  {"event_battle_test",       1,},
  {"event_check_fighters",    2,},
  {"event_attack_magic_foe",  3,},
  {"event_check_rooms_full",  4,},
  {"event_check_imps_danger", 5,},
  {"none",                    6,},
  {NULL,                      0,},
};

Comp_EvntTest_Func computer_event_test_func_list[] = {
  NULL,
  computer_event_battle_test,
  computer_event_check_fighters,
  computer_event_attack_magic_foe,
  computer_event_check_rooms_full,
  computer_event_check_imps_in_danger,
  NULL,
  NULL,
};

const struct NamedCommand computer_event_func_type[] = {
  {"event_battle",            1,},
  {"event_find_link",         2,},
  {"event_check_payday",      3,},
  {"none",                    4,},
  {NULL,                      0,},
};

Comp_Event_Func computer_event_func_list[] = {
  NULL,
  computer_event_battle,
  computer_event_find_link,
  computer_event_check_payday,
  NULL,
  NULL,
};

struct ComputerSpells computer_attack_spells[] = {
  {PwrK_DISEASE,   GA_UsePwrDisease,   1,  1, 2, 4},
  {PwrK_LIGHTNING, GA_UsePwrLightning, 0,  1, 8, 2},
  {PwrK_CHICKEN,   GA_UsePwrChicken,   1,  1, 2, 1},
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
    param.num1 = 0;
    param.num2 = 0;
    param.num3 = 0;
    return get_position_spiral_near_map_block_with_filter(pos,
        subtile_coord_center(stl_x), subtile_coord_center(stl_y),
        81, near_coord_filter_battle_drop_point, &param);
}

long computer_event_battle(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event)
{
    SYNCDBG(18,"Starting for %s",cevent->name);
    struct Coord3d pos;
    if (!get_computer_drop_position_near_subtile(&pos, comp->dungeon, coord_subtile(event->mappos_x), coord_subtile(event->mappos_y))) {
        SYNCDBG(8,"No drop position near (%d,%d) for %s",(int)coord_subtile(event->mappos_x),(int)coord_subtile(event->mappos_y),cevent->name);
        return 0;
    }
    // Check if there are any enemies in the vicinity - no enemies, don't drop creatures
    struct Thing* enmtng = get_creature_in_range_who_is_enemy_of_able_to_attack_and_not_specdigger(pos.x.val, pos.y.val, 21, comp->dungeon->owner);
    if (thing_is_invalid(enmtng)) {
        SYNCDBG(8,"No enemies near %s",cevent->name);
        return 0;
    }
    long creatrs_def = count_creatures_for_defend_pickup(comp);
    long creatrs_num = creatrs_def * (long)cevent->param1 / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0)) {
        creatrs_num = 1;
    }
    if (creatrs_num <= 0) {
        SYNCDBG(8,"No creatures to drop for %s",cevent->name);
        return 0;
    }
    if (!computer_find_non_solid_block(comp, &pos)) {
        SYNCDBG(8,"Drop position is solid for %s",cevent->name);
        return 0;
    }
    if (computer_able_to_use_power(comp, PwrK_HAND, 1, creatrs_num))
    {
        if (!is_task_in_progress(comp, CTT_MoveCreaturesToDefend) || ((cevent->param2 & 0x02) != 0))
        {
            if (!create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->param2)) {
                SYNCDBG(18,"Cannot move to defend for %s",cevent->name);
                return 0;
            }
            return 1;
        }
    }
    if (computer_able_to_use_power(comp, PwrK_CALL2ARMS, 1, 1))
    {
        if (!is_task_in_progress(comp, CTT_MagicCallToArms) || ((cevent->param2 & 0x02) != 0))
        {
            if (check_call_to_arms(comp))
            {
                if (!create_task_magic_battle_call_to_arms(comp, &pos, 2500, creatrs_num)) {
                    SYNCDBG(18,"Cannot call to arms for %s",cevent->name);
                    return 0;
                }
                return 1;
            }
        }
    }
    SYNCDBG(18,"No hand nor CTA, giving up with %s",cevent->name);
    return 0;
}

long computer_event_find_link(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event)
{
    long cproc_idx = 0;
    for (int i = 0; i < COMPUTER_PROCESSES_COUNT + 1; i++)
    {
        struct ComputerProcess* cproc = &comp->processes[i];
        if ((cproc->flags & ComProc_Unkn0002) != 0)
            break;
        if (cproc->parent == cevent->process)
        {
            cproc->flags &= ~ComProc_Unkn0008;
            cproc->flags &= ~ComProc_Unkn0001;
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
        if (thing_is_invalid(creatng) || creature_control_invalid(cctrl))
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
        if (thing_is_invalid(creatng) || creature_control_invalid(cctrl))
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
        return 4;
    }
    struct Thing* creatng = find_creature_in_fight_with_enemy(comp);
    if (thing_is_invalid(creatng)) {
        return 4;
    }
    struct Coord3d pos;
    pos.x.val = creatng->mappos.x.val;
    pos.y.val = creatng->mappos.y.val;
    pos.z.val = creatng->mappos.z.val;
    long creatrs_def = count_creatures_for_defend_pickup(comp);
    long creatrs_num = creatrs_def * (long)cevent->param1 / 100;
    if ((creatrs_num < 1) && (creatrs_def > 0)) {
        creatrs_num = 1;
    }
    if (creatrs_num <= 0) {
        return 4;
    }
    if (!computer_find_non_solid_block(comp, &pos)) {
        return 4;
    }
    if (computer_able_to_use_power(comp, PwrK_HAND, 1, creatrs_num))
    {
        if (!is_task_in_progress(comp, CTT_MoveCreaturesToDefend))
        {
            if (!create_task_move_creatures_to_defend(comp, &pos, creatrs_num, cevent->param2)) {
                return 4;
            }
            return 1;
        }
    }
    if (computer_able_to_use_power(comp, PwrK_CALL2ARMS, 8, 1))
    {
        if (!is_task_in_progress(comp, CTT_MagicCallToArms))
        {
            if (check_call_to_arms(comp))
            {
                if (!create_task_magic_battle_call_to_arms(comp, &pos, 2500, creatrs_num)) {
                    return 4;
                }
                return 1;
            }
        }
    }
    return 4;
}

/**
 * Returns a creature in fight which gives highest score value.
 * @return The thing in fight, or invalid thing if not found.
 */
struct Thing *computer_get_creature_in_fight(struct Computer2 *comp, PowerKind pwkind)
{
    return find_players_highest_score_creature_in_fight_not_affected_by_spell(comp->dungeon->owner, pwkind);
}

long computer_event_check_fighters(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    if (comp->dungeon->fights_num <= 0) {
        return 4;
    }
    if (!(computer_able_to_use_power(comp, PwrK_SPEEDCRTR, cevent->param1, 1) ||  computer_able_to_use_power(comp, PwrK_PROTECT, cevent->param1, 1))) {
        return 4;
    }
    struct Thing* fightng = computer_get_creature_in_fight(comp, PwrK_SPEEDCRTR);
    if (thing_is_invalid(fightng)) 
    {
        fightng = computer_get_creature_in_fight(comp, PwrK_PROTECT);
        if (thing_is_invalid(fightng))
        {
            return 4;
        }
    }
    if (!create_task_magic_speed_up(comp, fightng, cevent->param1)) {
        return 4;
    }
    return 1;
}

PowerKind computer_choose_attack_spell(struct Computer2 *comp, struct ComputerEvent *cevent, struct Thing *creatng)
{
    struct Dungeon* dungeon = comp->dungeon;
    int i = (cevent->param3 + 1) % (sizeof(computer_attack_spells) / sizeof(computer_attack_spells[0]));
    // Do the loop if we've reached starting value
    while (i != cevent->param3)
    {
        struct ComputerSpells* caspl = &computer_attack_spells[i];
        // If we've reached end of array, loop it
        if (caspl->pwkind == PwrK_None) {
            i = 0;
            continue;
        }
        if (can_cast_spell(dungeon->owner, caspl->pwkind, creatng->mappos.x.stl.num, creatng->mappos.y.stl.num, creatng, CastChk_Default))
        {
            if (!thing_affected_by_spell(creatng, caspl->pwkind))
            {
                if (computer_able_to_use_power(comp, caspl->pwkind, cevent->param1, caspl->amount_able)) {
                    cevent->param3 = i;
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
        return 4;
    }
    // TODO COMPUTER_PLAYER Do we really only want to help creature with the highest score?
    struct Thing* fightng = computer_get_creature_in_fight(comp, PwrK_None);
    if (thing_is_invalid(fightng)) {
        return 4;
    }
    struct CreatureControl* figctrl = creature_control_get_from_thing(fightng);
    struct Thing* creatng = thing_get(figctrl->combat.battle_enemy_idx);
    if (!thing_is_creature(creatng) || creature_is_dying(creatng)) {
        return 4;
    }
    PowerKind pwkind = computer_choose_attack_spell(comp, cevent, creatng);
    if (pwkind == PwrK_None) {
        return 4;
    }
    struct ComputerSpells* caspl = &computer_attack_spells[cevent->param3];
    int repeat_num = caspl->repeat_num;
    if (repeat_num < 0)
      repeat_num = cevent->param2;
    int splevel = caspl->pwlevel;
    if (splevel < 0)
      repeat_num = cevent->param1;
    int gaction = caspl->gaction;
    // Create the new task
    if (!create_task_attack_magic(comp, creatng, pwkind, repeat_num, splevel, gaction)) {
        return 4;
    }
    return 1;
}

long computer_event_check_rooms_full(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    SYNCDBG(18,"Starting");
    long ret = 4;
    TbBool emergency_state = computer_player_in_emergency_state(comp);
    for (struct ValidRooms* bldroom = valid_rooms_to_build; bldroom->rkind > 0; bldroom++)
    {
        if (computer_get_room_kind_free_capacity(comp, bldroom->rkind) > 0) {
            continue;
        }
        struct RoomConfigStats* roomst = &slab_conf.room_cfgstats[bldroom->rkind];
        int tiles = get_room_slabs_count(comp->dungeon->owner,bldroom->rkind);
        if ((tiles >= cevent->param3) && !(cevent->param3 == 0)) // Room has reached the preconfigured maximum size
        {
            SYNCDBG(8,"Player %d reached maximum size %d for %s",(int)comp->dungeon->owner,tiles,room_code_name(bldroom->rkind));
            if (bldroom->rkind == RoK_WORKSHOP)
            {
                struct Dungeon* dungeon = comp->dungeon;
                long used_capacity;
                long total_capacity;
                long storaged_capacity;
                get_room_kind_total_used_and_storage_capacity(dungeon, RoK_WORKSHOP, &total_capacity, &used_capacity, &storaged_capacity);
                if (storaged_capacity > (used_capacity / 2))
                {
                create_task_sell_traps_and_doors(comp, storaged_capacity/3*2 ,100000,false);
                SYNCDBG(8,"Player %d to sell crates to free up space in %s",(int)comp->dungeon->owner,room_code_name(bldroom->rkind));
                }
            }
            continue;
        } else 
        {
            if (emergency_state && ((roomst->flags & RoCFlg_BuildToBroke) == 0)) {
                continue;
            }
            SYNCDBG(8,"Player %d needs %s",(int)comp->dungeon->owner,room_code_name(bldroom->rkind));
            // Find the corresponding build process and mark it as needed
            for (long i = 0; i <= COMPUTER_PROCESSES_COUNT; i++)
            {
                struct ComputerProcess* cproc = &comp->processes[i];
                if ((cproc->flags & ComProc_Unkn0002) != 0)
                    break;
                if (cproc->parent == bldroom->process)
                {
                    SYNCDBG(8,"Player %d will allow process \"%s\"",(int)comp->dungeon->owner,cproc->name);
                    ret = 1;
                    cproc->flags &= ~ComProc_Unkn0008;
                    cproc->flags &= ~ComProc_Unkn0001;
                    cproc->last_run_turn = 0;
                    cproc->param_3 = 0;
                }
            }
        }
    }
    return ret;
}

long computer_event_check_imps_in_danger(struct Computer2 *comp, struct ComputerEvent *cevent)
{
    struct Dungeon* dungeon = comp->dungeon;
    if (dungeon->fights_num <= 0) {
        return 4;
    }
    // If we don't have the power to pick up creatures, fail now
    if (!computer_able_to_use_power(comp, PwrK_HAND, 1, 1)) {
        return 4;
    }
    long result = 4;
    // Search through special diggers
    unsigned long k = 0;
    int i = dungeon->digger_list_start;
    while (i != 0)
    {
        struct Thing* creatng = thing_get(i);
        struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
        if (thing_is_invalid(creatng) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if ((cctrl->combat_flags & (CmbtF_Melee|CmbtF_Ranged)) != 0)
        {
            if (!creature_is_being_unconscious(creatng) && !creature_affected_by_spell(creatng, SplK_Chicken))
            {
                // Small chance to casting invisibility,on imp in battle.
                if((ACTION_RANDOM(150) == 1) && computer_able_to_use_power(comp, PwrK_CONCEAL, 8, 1) && !thing_affected_by_spell(creatng, PwrK_CONCEAL))
                {
                    magic_use_available_power_on_thing(creatng->owner, PwrK_CONCEAL, 8, 0, 0, creatng, PwMod_Default);
                }
                else if (!creature_is_being_dropped(creatng) && can_thing_be_picked_up_by_player(creatng, dungeon->owner))
                {
                    TbBool needs_help;
                    if (get_creature_health_permil(creatng) < 500)
                    {
                        needs_help = 1;
                    } else
                    {
                        needs_help = creature_is_being_attacked_by_enemy_creature_not_digger(creatng);
                    }
                    if (needs_help)
                    {
                        // Move creature to heart, unless it already is near the heart
                        struct Thing* heartng = get_player_soul_container(dungeon->owner);
                        if (get_2d_distance(&creatng->mappos, &heartng->mappos) > subtile_coord(16,0))
                        {
                            if (!create_task_move_creature_to_subtile(comp, creatng,
                                heartng->mappos.x.stl.num, heartng->mappos.y.stl.num, CrSt_ImpDoingNothing)) {
                                break;
                            }
                            result = 1;
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
            if (create_task_sell_traps_and_doors(comp, cevent->param2, 3*(dungeon->creatures_total_pay-dungeon->total_money_owned)/2,true)) {
                return CTaskRet_Unk1;
            }
        }
    }
    // Move any gold laying around to treasure room
    if (dungeon_has_room(dungeon, RoK_TREASURE))
    {
        // If there's already task in progress which uses hand, then don't add more
        // content; the hand could be used by wrong task by mistake
        if (!is_task_in_progress_using_hand(comp) && computer_able_to_use_power(comp, PwrK_HAND, 1, cevent->param2))
        {
            SYNCDBG(8,"Creating task to move neutral gold to treasury");
            if (create_task_move_gold_to_treasury(comp, cevent->param2, 3*dungeon->creatures_total_pay/2)) {
                return CTaskRet_Unk1;
            }
        }
    }

    return CTaskRet_Unk4;
}

long computer_event_breach(struct Computer2 *comp, struct ComputerEvent *cevent, struct Event *event)
{
    //TODO COMPUTER_EVENT_BREACH is remade from beta; make it work (if it's really needed)
    struct Coord3d pos;

    //TODO COMPUTER_EVENT_BREACH check why mappos_x and mappos_y isn't used normally
    pos.x.val = event->mappos_x;
    pos.y.val = event->mappos_y;
    pos.z.val = 0;
    if ((pos.x.val <= 0) || (pos.y.val <= 0)) {
        return 0;
    }
    long count = count_creatures_for_pickup(comp, &pos, 0, cevent->param2);
    long i = count * cevent->param1 / 100;
    if ((i <= 0) && (count > 0)) {
        i = 1;
    }
    if (i <= 0) {
        return 4;
    }
    if (!computer_find_non_solid_block(comp, &pos)) {
        return 4;
    }
    if (!create_task_move_creatures_to_defend(comp, &pos, i, cevent->param2)) {
        return 4;
    }
    return 1;
}

/******************************************************************************/
