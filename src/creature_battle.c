/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_battle.c
 *     Creature battle structure and utility functions.
 * @par Purpose:
 *     Defines CreatureBattle struct and various battle-related routines.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     23 Jul 2011 - 05 Sep 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "creature_battle.h"
#include "globals.h"

#include "bflib_math.h"
#include "creature_states.h"
#include "thing_list.h"
#include "creature_control.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "creature_states_combt.h"
#include "creature_instances.h"
#include "thing_navigate.h"
#include "thing_stats.h"
#include "thing_objects.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "post_inc.h"

/******************************************************************************/

BattleIndex visible_battles[VISIBLE_BATTLES_COUNT];
unsigned short friendly_battler_list[VISIBLE_BATTLES_COUNT*MESSAGE_BATTLERS_COUNT];
unsigned short enemy_battler_list[VISIBLE_BATTLES_COUNT*MESSAGE_BATTLERS_COUNT];

/** Battle shown on the top row of the battle panel, and the turn it started on. The panel is
 * the window of the battle list of the local player which starts at it; zero means it starts
 * at the first battle. The turn is kept to tell the anchored battle apart from another one
 * which was later handed the same index, and to keep the panel about the same place once the
 * anchored battle is over. */
static BattleIndex visible_battles_anchor;
static GameTurn visible_battles_anchor_turn;

/** Bumped whenever creatures enter or leave the battles, so that the battle panel knows it
 * has to be built again. A missed bump can only leave the panel one step behind, never in an
 * invalid state, as the panel is always built from scratch out of the battles themselves. */
static unsigned long battle_lists_change_count;
/** The state the battle panel was last built from, to skip building it again for nothing. */
static unsigned long visible_battles_built_count;
static PlayerNumber visible_battles_built_plyr;
static PlayerBitFlags visible_battles_built_allies;
/** Amount of battles the panel was built out of, which is how it knows whether it leaves any
 * of them out and so whether there is anywhere to scroll to. */
static int visible_battles_total;

void battle_lists_changed(void)
{
    battle_lists_change_count++;
}

/******************************************************************************/
/**
 * Returns CreatureBattle of given index.
 */
struct CreatureBattle *creature_battle_get(BattleIndex battle_id)
{
    if ((battle_id < 1) || (battle_id >= BATTLES_COUNT))
        return INVALID_CRTR_BATTLE;
    return &game.battles[battle_id];
}

/**
 * Returns CreatureBattle assigned to given thing.
 * Thing must be a creature.
 */
struct CreatureBattle *creature_battle_get_from_thing(const struct Thing *thing)
{
    if (!thing_is_creature(thing)) {
        return INVALID_CRTR_BATTLE;
    }
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        return INVALID_CRTR_BATTLE;
    }
    if ((cctrl->battle_id < 1) || (cctrl->battle_id >= BATTLES_COUNT)) {
      return INVALID_CRTR_BATTLE;
    }
    return &game.battles[cctrl->battle_id];
}

/**
 * Returns if given CreatureBattle pointer is incorrect.
 */
TbBool creature_battle_invalid(const struct CreatureBattle *battle)
{
  return (battle <= &game.battles[0]) || (battle == NULL);
}

/**
 * Returns if CreatureBattle of given index is an existing (ongoing) battle.
 */
TbBool creature_battle_exists(BattleIndex battle_id)
{
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle))
        return false;
    return (battle->fighters_num > 0);
}

TbBool has_melee_combat_attackers(struct Thing *victim)
{
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_melee_count > 0);
}

TbBool can_add_melee_combat_attacker(struct Thing *victim)
{
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_melee_count < COMBAT_MELEE_OPPONENTS_LIMIT);
}

TbBool has_ranged_combat_attackers(const struct Thing *victim)
{
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_ranged_count > 0);
}

BattleIndex find_first_battle_of_mine(PlayerNumber plyr_idx)
{
    for (long i = 1; i < BATTLES_COUNT; i++)
    {
        struct CreatureBattle* battle = creature_battle_get(i);
        if (battle->fighters_num != 0)
        {
            if (battle_with_creature_of_player(plyr_idx, i))
               return i;
        }
    }
    return 0;
}

TbBool can_add_ranged_combat_attacker(const struct Thing *victim)
{
    struct CreatureControl* vicctrl = creature_control_get_from_thing(victim);
    return (vicctrl->opponents_ranged_count < COMBAT_RANGED_OPPONENTS_LIMIT);
}

long get_flee_position(struct Thing *creatng, struct Coord3d *pos)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
    // Heroes should flee to their gate
    if (is_hero_thing(creatng))
    {
        struct Thing* gatetng = find_best_hero_gate_to_navigate_to(creatng);
        if ( !thing_is_invalid(gatetng) )
        {
            pos->x.val = gatetng->mappos.x.val;
            pos->y.val = gatetng->mappos.y.val;
            pos->z.val = gatetng->mappos.z.val;
            return 1;
        }
    }
    else
    // Neutral creatures don't have flee place
    if (is_neutral_thing(creatng))
    {
        if ( (pos->x.val != 0) || (pos->y.val != 0) )
        {
            return 1;
        }
        return 0;
    }
    // Same with creatures without dungeon - try using last place
    struct Dungeon* dungeon = get_dungeon(creatng->owner);
    if (dungeon_invalid(dungeon))
    {
        if ( (pos->x.val != 0) || (pos->y.val != 0) )
        {
            return 1;
        }
        return 0;
    }
    // Other creatures can flee to heart or their lair
    struct Thing* lairtng = thing_get(cctrl->lairtng_idx);
    if ( (!thing_is_invalid(lairtng)) && (creature_can_navigate_to_with_storage(creatng, &lairtng->mappos, NavRtF_Default)) )
    {
        TRACE_THING(lairtng);
        pos->x.val = lairtng->mappos.x.val;
        pos->y.val = lairtng->mappos.y.val;
        pos->z.val = lairtng->mappos.z.val;
    }
    else
    if (creature_can_get_to_dungeon_heart(creatng, creatng->owner))
    {
        struct Thing* heartng = get_player_soul_container(creatng->owner);
        TRACE_THING(heartng);
        pos->x.val = heartng->mappos.x.val;
        pos->y.val = heartng->mappos.y.val;
        pos->z.val = heartng->mappos.z.val;
    } else
    {
        struct PlayerInfo* player = get_player(creatng->owner);
        if ( ((player->allocflags & PlaF_Allocated) != 0) && (player->is_active == 1) && (player->victory_state != VicS_LostLevel) )
        {
            if (!is_hero_thing(creatng))
            {
                SYNCDBG(8,"The %s index %d has no dungeon heart or lair to flee to", thing_model_name(creatng), (int)creatng->index);
            }
            return 0;
        }
        pos->x.stl.pos = creatng->mappos.x.stl.pos;
        pos->y.stl.pos = creatng->mappos.y.stl.pos;
        pos->z.stl.pos = creatng->mappos.z.stl.pos;
    }
    return 1;
}

TbBool setup_combat_flee_position(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Invalid creature control");
        return false;
    }
    if (!get_flee_position(thing, &cctrl->flee_pos))
    {
        if (!is_hero_thing(thing))
        {
            SYNCDBG(8,"Couldn't get a flee position for %s index %d", thing_model_name(thing), (int)thing->index);
        }
        cctrl->flee_pos.x.stl.pos = thing->mappos.x.stl.pos;
        cctrl->flee_pos.y.stl.pos = thing->mappos.y.stl.pos;
        cctrl->flee_pos.z.stl.pos = thing->mappos.z.stl.pos;
        return false;
    }
    return true;
}

long get_combat_state_for_combat(struct Thing *fightng, struct Thing *enmtng, CrAttackType attack_pref)
{
    if (attack_pref == AttckT_Ranged)
    {
        if (can_add_ranged_combat_attacker(enmtng)) {
            return CmbtSt_Ranged;
        }
        return CmbtSt_Waiting;
    }
    struct CreatureModelConfig* crconf = creature_stats_get_from_thing(fightng);
    if (crconf->attack_preference == AttckT_Ranged)
    {
        if (creature_has_ranged_weapon(fightng) && can_add_ranged_combat_attacker(enmtng)) {
            return CmbtSt_Ranged;
        }
    }
    if (can_add_melee_combat_attacker(enmtng))
    {
        if (creature_has_melee_attack(fightng))
        {
            return CmbtSt_Melee;
        }
    }
    if (can_add_ranged_combat_attacker(enmtng)) 
    {
        if (creature_has_ranged_weapon(fightng))
        {
            return CmbtSt_Ranged;
        }
    }
    return CmbtSt_Waiting;
}

void set_creature_in_combat(struct Thing *fightng, struct Thing *enmtng, CrAttackType attack_type)
{
    SYNCDBG(8,"Starting for %s index %d and %s index %d",thing_model_name(fightng),(int)fightng->index,thing_model_name(enmtng),(int)enmtng->index);
    struct CreatureControl* cctrl = creature_control_get_from_thing(fightng);
    if (creature_control_invalid(cctrl)) {
        ERRORLOG("Invalid creature control");
        return;
    }
    if ( (cctrl->combat_flags != 0) && ((cctrl->combat_flags & (CmbtF_ObjctFight|CmbtF_DoorFight)) == 0) )
    {
        CrtrStateId crstate = get_creature_state_besides_move(fightng);
        ERRORLOG("Creature in combat already - state %s", creature_state_code_name(crstate));
        return;
    }
    if ( !external_set_thing_state(fightng, CrSt_CreatureInCombat) ) {
        ERRORLOG("Failed to enter combat state for %s index %d",thing_model_name(fightng),(int)fightng->index);
        return;
    }
    cctrl->fighting_at_same_position = 0;
    cctrl->fight_til_death = 0;
    if ( !set_creature_combat_state(fightng, enmtng, attack_type) ) {
        WARNLOG("Couldn't setup combat state for %s index %d and %s index %d",thing_model_name(fightng),(int)fightng->index,thing_model_name(enmtng),(int)enmtng->index);
        set_start_state(fightng);
        return;
    }
    setup_combat_flee_position(fightng);
}

TbBool active_battle_exists(void)
{
    return (visible_battles[0] != 0);
}

long battle_move_player_towards_battle(struct PlayerInfo *player, BattleIndex battle_idx)
{
    struct CreatureBattle* battle = creature_battle_get(battle_idx);
    struct Thing* thing = thing_get(battle->first_creatr);
    TRACE_THING(thing);
    if (!thing_exists(thing))
    {
        ERRORLOG("Jump to invalid thing detected");
        player->zoom_to_pos_x = subtile_coord_center(game.map_subtiles_x/2);
        player->zoom_to_pos_y = subtile_coord_center(game.map_subtiles_y/2);
        return 0;
    }
    player->zoom_to_pos_x = thing->mappos.x.val;
    player->zoom_to_pos_y = thing->mappos.y.val;
    set_player_instance(player, PI_ZoomToPos, 0);
    return 1;
}

void battle_initialise(void)
{
    for (int battle_idx = 0; battle_idx < BATTLES_COUNT; battle_idx++)
    {
        memset(&game.battles[battle_idx], 0, sizeof(struct CreatureBattle));
    }
    reset_visible_battles();
}

TbBool clear_battlers(unsigned short *friendly_battlers, unsigned short *enemy_battlers)
{
    for (long i = 0; i < MESSAGE_BATTLERS_COUNT; i++)
    {
        friendly_battlers[i] = 0;
        enemy_battlers[i] = 0;
    }
    return true;
}

long setup_player_battlers(struct PlayerInfo *player, struct CreatureBattle *battle, unsigned short *friendly_battlers, unsigned short *enemy_battlers)
{
    short friendly_pos = 0;
    short enemy_pos = 0;
    long i = battle->first_creatr;
    unsigned long k = 0;
    while (i > 0)
    {
        struct Thing* thing = thing_get(i);
        if (thing_is_invalid(thing)) {
            ERRORLOG("Invalid thing index %d in friend/enemy battle-list",(int)i);
            break;
        }
        TRACE_THING(thing);
        if (!thing_is_creature(thing)) {
            ERRORLOG("Dead thing index %d in friend/enemy battle-list",(int)i);
            break;
        }
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        i = cctrl->battle_prev_creatr;
        // Per creature code
        long n;
        if (players_are_mutual_allies(player->id_number, thing->owner))
        {
            n = friendly_pos;
            if (n < MESSAGE_BATTLERS_COUNT) {
                friendly_pos++;
                friendly_battlers[n] = thing->index;
            }
        } else
        {
            n = enemy_pos;
            if (n < MESSAGE_BATTLERS_COUNT) {
                enemy_pos++;
                enemy_battlers[n] = thing->index;
            }
        }
        // Per creature code ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping battle creatures list");
            break;
        }
    }
    return friendly_pos+enemy_pos;
}

long setup_my_battlers(unsigned char battle_idx, unsigned short *friendly_battlers, unsigned short *enemy_battlers)
{
    // Clear the battlers
    clear_battlers(friendly_battlers, enemy_battlers);
    // And fill them with new data
    struct PlayerInfo* player = get_my_player();
    struct CreatureBattle* battle = creature_battle_get(battle_idx);
    if (creature_battle_invalid(battle)) {
        ERRORLOG("Invalid battle %d",(int)battle_idx);
        return 0;
    }
    return setup_player_battlers(player, battle, friendly_battlers, enemy_battlers);
}

/**
 * Orders battles by the turn they started on, newest first. Battles started on the same turn
 * keep a stable order by index, so that the list does not shuffle from one frame to the next.
 */
static int compare_battles_by_recency(const void *ptr_a, const void *ptr_b)
{
    BattleIndex battle_id_a = *(const BattleIndex *)ptr_a;
    BattleIndex battle_id_b = *(const BattleIndex *)ptr_b;
    GameTurn turn_a = creature_battle_get(battle_id_a)->start_turn;
    GameTurn turn_b = creature_battle_get(battle_id_b)->start_turn;
    if (turn_a != turn_b) {
        // Newest battle first. Swapping the two returned values orders the panel the other
        // way around, oldest battle first; nothing else has to change for that.
        return (turn_a < turn_b) ? 1 : -1;
    }
    return (int)battle_id_a - (int)battle_id_b;
}

/**
 * Stores the indexes of all the ongoing battles of given player, newest battle first.
 * Returns the amount of battles stored.
 */
static int collect_battles_of_player(PlayerNumber plyr_idx, BattleIndex *battles, int max_battles)
{
    int nbattles = 0;
    for (BattleIndex battle_id = 1; (battle_id < BATTLES_COUNT) && (nbattles < max_battles); battle_id++)
    {
        if (creature_battle_exists(battle_id) && battle_with_creature_of_player(plyr_idx, battle_id)) {
            battles[nbattles] = battle_id;
            nbattles++;
        }
    }
    if (nbattles > 1) {
        // The order the panel shows is decided here alone, so another comparison function
        // sorts it by another key with no other change. The fields of struct CreatureBattle,
        // start_turn and fighters_num, are free to read; a key which belongs to the fighters
        // instead, such as their health or their exp_level, needs the battle walked from
        // first_creatr through battle_prev_creatr; work such a key out once per battle
        // before sorting rather than inside the comparison, which qsort calls far more often.
        qsort(battles, nbattles, sizeof(battles[0]), compare_battles_by_recency);
    }
    return nbattles;
}

/**
 * Returns the position which the battle panel anchor has on given battle list. When the anchored
 * battle is no longer on it, either because it is over or because its index has since been handed
 * to another battle, the panel stays on the first battle which is not newer than the anchored one
 * was, so that it keeps about the place the player had scrolled to.
 */
static int visible_battles_anchor_position(const BattleIndex *battles, int nbattles)
{
    if (visible_battles_anchor == 0) {
        return 0;
    }
    for (int i = 0; i < nbattles; i++)
    {
        // The turn has to match as well: battle indexes are handed out again as soon as a
        // battle is over, so the index alone could be a different battle by now
        if ((battles[i] == visible_battles_anchor)
         && (creature_battle_get(battles[i])->start_turn == visible_battles_anchor_turn))
            return i;
    }
    for (int i = 0; i < nbattles; i++)
    {
        if (creature_battle_get(battles[i])->start_turn <= visible_battles_anchor_turn)
            return i;
    }
    return 0;
}

/**
 * Returns the players which given player is allied with. The battle panel splits every battle
 * into allies and enemies, so it has to be built again whenever an alliance changes.
 */
static PlayerBitFlags allies_of_player(PlayerNumber plyr_idx)
{
    PlayerBitFlags allies = 0;
    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++)
    {
        if (players_are_mutual_allies(plyr_idx, i)) {
            set_flag(allies, to_flag(i));
        }
    }
    return allies;
}

/**
 * Fills the battle panel with the window of given battle list which starts at the anchor; as
 * the list has no repeated entries, neither has the panel. When there are less battles than
 * rows, the remaining rows are left empty.
 */
static void fill_visible_battles(PlayerNumber plyr_idx, const BattleIndex *battles, int nbattles)
{
    int anchor_pos = visible_battles_anchor_position(battles, nbattles);
    for (int i = 0; i < VISIBLE_BATTLES_COUNT; i++)
    {
        BattleIndex battle_id = (i < nbattles) ? battles[(anchor_pos + i) % nbattles] : 0;
        visible_battles[i] = battle_id;
        if (battle_id > 0) {
            setup_my_battlers(battle_id, &friendly_battler_list[MESSAGE_BATTLERS_COUNT*i], &enemy_battler_list[MESSAGE_BATTLERS_COUNT*i]);
        } else {
            clear_battlers(&friendly_battler_list[MESSAGE_BATTLERS_COUNT*i], &enemy_battler_list[MESSAGE_BATTLERS_COUNT*i]);
        }
    }
    visible_battles_anchor = visible_battles[0];
    visible_battles_anchor_turn = creature_battle_get(visible_battles_anchor)->start_turn;
    visible_battles_total = nbattles;
    visible_battles_built_count = battle_lists_change_count;
    visible_battles_built_plyr = plyr_idx;
    visible_battles_built_allies = allies_of_player(plyr_idx);
}

/**
 * Rebuilds the battle panel out of the battles of given player.
 */
static void rebuild_visible_battles(PlayerNumber plyr_idx)
{
    BattleIndex battles[BATTLES_COUNT];
    int nbattles = collect_battles_of_player(plyr_idx, battles, BATTLES_COUNT);
    fill_visible_battles(plyr_idx, battles, nbattles);
}

/**
 * Moves the battle panel by given amount of rows over the battle list of given player.
 * Scrolling the panel only leads anywhere while the list is longer than the panel, as every
 * battle is on show otherwise; moving on to another battle wraps around the list however few
 * battles are on it, so that it can be used to visit them one after another.
 */
static void move_visible_battles(PlayerNumber plyr_idx, int delta, TbBool wrap_when_all_shown)
{
    BattleIndex battles[BATTLES_COUNT];
    int nbattles = collect_battles_of_player(plyr_idx, battles, BATTLES_COUNT);
    if ((nbattles > 0) && (wrap_when_all_shown || (nbattles > VISIBLE_BATTLES_COUNT)))
    {
        int anchor_pos = visible_battles_anchor_position(battles, nbattles);
        visible_battles_anchor = battles[(anchor_pos + delta + nbattles) % nbattles];
    }
    fill_visible_battles(plyr_idx, battles, nbattles);
}

void maintain_my_battle_list(void)
{
    struct PlayerInfo* player = get_my_player();
    if ((visible_battles_built_count == battle_lists_change_count)
     && (visible_battles_built_plyr == player->id_number)
     && (visible_battles_built_allies == allies_of_player(player->id_number))) {
        // Nothing which the panel shows has changed since it was built
        return;
    }
    rebuild_visible_battles(player->id_number);
}

void reset_visible_battles(void)
{
    struct PlayerInfo* player = get_my_player();
    visible_battles_anchor = 0;
    visible_battles_anchor_turn = 0;
    rebuild_visible_battles(player->id_number);
}

TbBool step_battles_forward(PlayerNumber plyr_idx)
{
    move_visible_battles(plyr_idx, 1, false);
    return active_battle_exists();
}

TbBool step_battles_backward(PlayerNumber plyr_idx)
{
    move_visible_battles(plyr_idx, -1, false);
    return active_battle_exists();
}

/**
 * Returns whether the battle panel has anywhere to scroll to, which it only has while there
 * are more battles than it has rows to show them on.
 */
TbBool battle_panel_can_scroll(void)
{
    return (visible_battles_total > VISIBLE_BATTLES_COUNT);
}

TbBool cycle_to_next_battle(PlayerNumber plyr_idx)
{
    move_visible_battles(plyr_idx, 1, true);
    return active_battle_exists();
}

unsigned long count_active_battles(PlayerNumber plyr_idx)
{
    unsigned long result = 0;
    for (int i = 1; i < BATTLES_COUNT; i++)
    {
        struct CreatureBattle* battle = creature_battle_get(i);
        if (battle->fighters_num > 0)
        {
            if (battle_with_creature_of_player(plyr_idx, i))
            {
                result++;
            }
        }
    }
    return result;
}
/******************************************************************************/
