/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_newcompeval.c
 *     New computer player evaluation functions.
 * @par Purpose:
 *     Defines functions that look at the game state to determine something, but
 *     that do not take any action themselves.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 Mar 2009 - 02 May 2015
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "creature_states.h"
#include "game_legacy.h"
#include "magic.h"
#include "player_computer.h"
#include "power_hand.h"
#include "spdigger_stack.h"
#include "thing_data.h"

/************************************************************************/
/* Any imps thinking to dig gems right now?                             */
/************************************************************************/
TbBool is_digging_any_gems(struct Dungeon *dungeon)
{
	SYNCDBG(19,"Starting");

	long i;
	unsigned long k;
	k = 0;
	i = dungeon->digger_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		if (cctrl->combat_flags == 0)
		{
			long state_type;
			state_type = get_creature_state_type(thing);

			if (state_type == CrStTyp_Work
				&& cctrl->digger.last_did_job == SDLstJob_DigOrMine
				&& is_digging_indestructible_place(thing))
			{
				SYNCDBG(18, "Gems being dug by player %d", (int)dungeon->owner);
				return 1;
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return 0;
		}
	}

	SYNCDBG(18, "Gems NOT being dug by player %d", (int)dungeon->owner);

	return 0;
}

/************************************************************************/
/* Good imp for sacrifice is level 1 and has low health and money.      */
/************************************************************************/
struct Thing * find_imp_for_sacrifice(struct Dungeon* dungeon)
{
	SYNCDBG(19,"Starting");

	long best_priority;
	struct Thing *best_tng;
	long digger_price;
	TbBool digging_gems;

	digger_price = compute_lowest_digger_price(dungeon->owner);
	best_priority = INT_MAX;
	best_tng = INVALID_THING;
	digging_gems = is_digging_any_gems(dungeon);

	long i;
	unsigned long k;
	k = 0;
	i = dungeon->digger_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		if (cctrl->combat_flags == 0
			&& cctrl->explevel == 0 //at lowest experience level
			&& (digging_gems || thing->creature.gold_carried == 0)) //no gold carried if no gem access
		{
			if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
			{
				if (!creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
				{
					struct CreatureStats* crtrstats;
					crtrstats = creature_stats_get_from_thing(thing);

					long priority;
					long state_type;
					int max_health;
					state_type = get_creature_state_type(thing);

					priority = thing->creature.gold_carried; //base value
					if (state_type == CrStTyp_Work)
						priority += 500; //aborted work valued at this many gold

					max_health = compute_creature_max_health(crtrstats->health, cctrl->explevel);
					priority += digger_price * thing->health / max_health; //full health valued at this many gold

					if (priority < best_priority)
					{
						best_priority = priority;
						best_tng = thing;
					}
				}
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return INVALID_THING;
		}
	}

	return best_tng;
}

/*************************************************************************/
/* Find good imp for claim and ensure not too many are claiming already. */
/*************************************************************************/
struct Thing * find_imp_for_claim(struct Dungeon* dungeon)
{
	SYNCDBG(19,"Starting");

	long max_claimers;
	max_claimers = (dungeon->num_active_diggers - count_player_diggers_not_counting_to_total(dungeon->owner) - 1) / 3; //4th imp can claim
	if (max_claimers <= 0)
		return INVALID_THING;

	long best_priority;
	struct Thing *best_tng;
	TbBool digging_gems;
	best_priority = INT_MIN;
	best_tng = INVALID_THING;
	digging_gems = is_digging_any_gems(dungeon);

	long claimers;
	long i;
	unsigned long k;
	claimers = 0;
	k = 0;
	i = dungeon->digger_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		if (cctrl->combat_flags == 0
			&& (digging_gems || thing->creature.gold_carried == 0)) //no gold carried if no gem access
		{
			if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
			{
				CrtrStateId state;
				state = get_creature_state_besides_move(thing);
				if (state == CrSt_ImpConvertsDungeon || state == CrSt_ImpArrivesAtConvertDungeon)
					claimers += 1;
				else if (!creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
				{
					long priority;
					long state_type;
					state_type = get_creature_state_type(thing);

					priority = 1000 * thing->health / (1000 + thing->creature.gold_carried); //base value
					if (state_type == CrStTyp_Work)
						priority /= 2;

					if (priority > best_priority)
					{
						best_priority = priority;
						best_tng = thing;
					}
				}
			}
		}
		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			return INVALID_THING;
		}
	}

	if (claimers >= max_claimers)
		best_tng = INVALID_THING;
	return best_tng;
}

/*******************************************************************************************/
/* Find creature that can be considered good for non-combat attacks, e.g. opening doors.   */
/*******************************************************************************************/
struct Thing * find_creature_for_low_priority_attack(struct Dungeon* dungeon, TbBool strong)
{
	SYNCDBG(19,"Starting");

	long max_attackers;
	max_attackers = (dungeon->num_active_creatrs - count_player_creatures_not_counting_to_total(dungeon->owner) + 2) / 3; //1st creature can attack, one third can attack
	if (max_attackers <= 0)
		return INVALID_THING;

	long best_priority;
	struct Thing *best_tng;
	best_priority = INT_MIN;
	best_tng = INVALID_THING;

	long attackers;
	long i;
	unsigned long k;
	attackers = 0;
	k = 0;
	int test;
	test = 0;
	i = dungeon->creatr_list_start;
	while (i != 0)
	{
		struct Thing *thing;
		struct CreatureControl *cctrl;
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body
		//if (cctrl->combat_flags != 0)
		{
			if (!creature_is_being_unconscious(thing) && !creature_affected_by_spell(thing, SplK_Chicken))
			{
				CrtrStateId state;
				state = get_creature_state_besides_move(thing);
				if (state == CrSt_CreatureDoorCombat)
					attackers += 1;
				else if (state != CrSt_CreatureInCombat && state != CrSt_CreatureObjectCombat && !creature_is_being_dropped(thing) && can_thing_be_picked_up_by_player(thing, dungeon->owner))
				{
					test += 1;
					struct CreatureStats* crtrstats;
					int max_health;
					crtrstats = creature_stats_get_from_thing(thing);
					max_health = compute_creature_max_health(crtrstats->health, cctrl->explevel);

					if (thing->health * 2 >= max_health)
					{
						long priority;
						long state_type;
						state_type = get_creature_state_type(thing);

						priority = thing->health * (crtrstats->armour + crtrstats->strength);

						if (strong && state_type == CrStTyp_Work)
							priority /= 2;

						if (!strong)
							priority = INT_MAX / 2 - priority;

						if (priority > best_priority)
						{
							best_priority = priority;
							best_tng = thing;
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
			return INVALID_THING;
		}
	}

	if (attackers >= max_attackers)
		best_tng = INVALID_THING;
	return best_tng;
}

/************************************************************************/
/* Higher strength => good chance of player winning a battle.           */
/************************************************************************/
float calc_players_strength(struct Dungeon* dungeon)
{
	SYNCDBG(19,"Starting");

	struct CreatureControl *cctrl;
	struct Thing *thing;
	unsigned long k;
	int i;
	float strength;
	k = 0;
	i = dungeon->creatr_list_start;
	strength = 0.0f;
	while (i != 0)
	{
		thing = thing_get(i);
		cctrl = creature_control_get_from_thing(thing);
		if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
		{
			ERRORLOG("Jump to invalid creature detected");
			break;
		}
		i = cctrl->players_next_creature_idx;
		// Thing list loop body

		strength += get_creature_thing_score(thing); //TODO: replace with better estimate

		// Thing list loop body ends
		k++;
		if (k > CREATURES_COUNT)
		{
			ERRORLOG("Infinite loop detected when sweeping creatures list");
			break;
		}
	}
	SYNCDBG(19,"Finished");

	return strength;
}
