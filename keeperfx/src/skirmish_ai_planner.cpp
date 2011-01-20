/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_planner.cpp
 *     Planner/state tree explorer for new AI. Highly experimental, I don't yet
 *     know how til will perform. ;-)
 *     Is .cpp although use of C++ is minimized and can be replaced later.
 * @par Purpose:
 *     Experimental computer player intended to play multiplayer maps better.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "skirmish_ai_planner.h"

#include "config_creature.h"
#include "config_terrain.h"
#include "keeperfx.hpp"

#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include <set> //explained below

#define MAX_PLAN_SIZE       10000 //sort of: more CPU time => more intelligent

#define MAX_KEEPERS         4

#define FLAG_DUNGEON_FORTIFIED  0x1 //dungeon is unbreakable to anyone else
#define FLAG_HEART_ACCESSIBLE   0x2 //from our player
#define FLAG_BATTLE_ACCESSIBLE  0x4 //we can try to provoke a battle because we are in immediate contact
#define FLAG_LIQUID_BLOCKED     0x8 //their dungeon is unreachable from us until we get bridge

//for some reason neither min or max were defined despite including headers that define them...
#ifndef min
#define min(a, b) (a < b? a : b)
#endif

#ifndef max
#define max(a, b) (a > b? a : b)
#endif

struct NodePlayerState
{
    unsigned gold; //in treasury chamber and off map
    unsigned spells_researched; //bitmask; rationale for keeping /player is that there
        //is only one instance - thus we can easily conclude effect of stealing enemy spells
    unsigned creature_level_sum; //for calculating an average combat power /non-worker
        //storing individual levels is too much
    unsigned char creatures[CREATURE_TYPES_COUNT];
    unsigned char flags;
};

struct NodeState
{
    int time;
    struct NodePlayerState players[MAX_KEEPERS];
    unsigned char pool[CREATURE_TYPES_COUNT];
    unsigned rooms_built; //bitmask; for planning player (at least one room of kind built)
    unsigned rooms_available; //bitmask; for planning player (room kind is researched)
};

struct Node
{
    int id; //required because we can't use pointer as final compare key (order must be deterministic on multiple machines)
    struct Node * parent;
    struct SAI_PlanDecision decision;
    struct NodeState state;

    bool operator< (const Node & other) const; //omg C++... no choice
};

struct PlayerEnvironment
{
    unsigned spells_researchable; //bitmask; so we can use bool ops with spells_researched
};

struct Environment
{
    struct PlayerEnvironment players[MAX_KEEPERS];
    unsigned rooms_researchable; //bitmask; so we can use bool ops with rooms_built/rooms_available bitmasks
};

struct
{
    int my_plyr_idx; //for the player that we're making a plan forplyr_idx
    int next_node_id;
    struct Node * best_leaf;
    struct Environment env; //precompiled variables not changing on search (for efficiency)

    //rewrite using some C library later if C++ dependency becomes an issue
    std::set<Node *> open;
} static planner;


static const char plan_names[][32] = {
    "Wait",
    "Take Room",
    "Build Room",
    "Hide",
    "Launch Assault",
    "Launch Armageddon"
};


static int calc_attack_power(const struct NodeState * state, int plyr_idx)
{
    //TODO AI: add some kind of way to judge creature types
    //TODO AI: add effect of power spells and access to gold to fuel those spells
    return state->players[plyr_idx].creature_level_sum;
}

static int calc_attack_balance(const struct NodeState * state)
{
    int i;
    int my_attack;
    int best_enemy_attack;

    my_attack = calc_attack_power(state, planner.my_plyr_idx);

    best_enemy_attack = 0;
    for (i = 0; i < MAX_KEEPERS; ++i) {
        if (i != planner.my_plyr_idx && player_exists(get_player(i))) {
            best_enemy_attack = max(best_enemy_attack, calc_attack_power(state, i));
        }
    }

    return my_attack - best_enemy_attack;
}

static int node_score(const struct Node * node)
{
    int score;
    score = 0;

    score += calc_attack_balance(&node->state) * 100;
    score -= node->state.time;

    if (node->decision.type == SAI_PLAN_WAIT) {
        score -= 1; //give preference to non-wait nodes
    }

    return score;
}

bool Node::operator< (const Node & other) const
{
    int diff;

    diff = node_score(this) - node_score(&other);
    if (diff != 0) {
        return diff;
    }

    return id - other.id; //for multi-machine determinism
}

void state_time_simulation(struct NodeState * out, const struct NodeState * in, int time)
{
}

static int is_room_usable(struct Room * room)
{
    return 1; //TODO AI: implement. should check capacity or some such
}

static void prepare_creature_pool_state(struct NodeState * state)
{
    int i;

    for (i = 0; i < CREATURE_TYPES_COUNT; ++i) {
        state->pool[i] = (unsigned char) min(0xFF, game.pool.crtr_kind[i]);
    }
}

static void prepare_planning_player_state(struct NodeState * state)
{
    int i, j;
    struct PlayerInfo * plyr;
    struct Dungeon * dungeon;
    struct Room * room;

    plyr = get_player(planner.my_plyr_idx);
    dungeon = get_players_dungeon(plyr);

    //look up rooms
    for (i = 0; i < ROOM_TYPES_COUNT; ++i) {
        for (j = dungeon->room_kind[i]; j != 0; j = room->next_of_owner) {
            room = room_get(i);
            if (room_is_invalid(room)) {
                continue;
            }

            if (is_room_usable(room)) {
                state->rooms_built |= 1 << i;
                break;
            }
        }

        if (is_room_available(planner.my_plyr_idx, i)) {
            state->rooms_available |= 1 << i;
        }
    }
}

static void prepare_player_environment(struct PlayerEnvironment * state, int plyr_idx)
{
    int i;
    struct PlayerInfo * plyr;
    struct Dungeon * dungeon;

    plyr = get_player(plyr_idx);
    dungeon = get_players_dungeon(plyr);

    //look up spells
    for (i = 0; i < KEEPER_SPELLS_COUNT; ++i) {
        if (dungeon->magic_resrchable[i]) {
            state->spells_researchable |= 1 << i;
        }
    }
}

static void prepare_player_state(struct NodePlayerState * state, int plyr_idx)
{
    int i;
    struct PlayerInfo * plyr;
    struct Dungeon * dungeon;
    struct CreatureControl * cctrl;
    struct Thing * thing;

    plyr = get_player(plyr_idx);
    dungeon = get_players_dungeon(plyr);

    //state->gold = dungeon-> //TODO AI: reverse draw_gold_total or check_map_for_gold to find out how to calc total gold

    //look up creatures
    for (i = dungeon->creatr_list_start; i != 0; i = cctrl->next_in_group) {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl)) {
            continue;
        }

        assert(thing->model >= 0);
        assert(thing->model < CREATURE_TYPES_COUNT);

        state->creatures[thing->model] += 1;

        if (get_players_special_digger_breed(plyr_idx) != thing->model) {
            state->creature_level_sum += cctrl->explevel;
        }
    }

    //look up spells
    for (i = 0; i < KEEPER_SPELLS_COUNT; ++i) {
        if (is_power_available(plyr_idx, i)) {
            state->spells_researched |= 1 << i;
        }
    }
}

static void prepare_environment(struct Environment * state)
{
    int i;
    struct PlayerInfo * plyr;
    struct Dungeon * dungeon;

    plyr = get_player(planner.my_plyr_idx);
    dungeon = get_players_dungeon(plyr);

    //look up rooms
    for (i = 0; i < ROOM_TYPES_COUNT; ++i) {
        if (dungeon->room_resrchable[i]) {
            state->rooms_researchable |= 1 << i;
        }
    }

    //all players
    for (i = 0; i < MAX_KEEPERS; ++i) {
        if (player_exists(get_player(i))) {
            prepare_player_environment(&state->players[i], i);
        }
    }
}

static int estimate_room_build_time(int kind)
{
    //TODO AI: interface with other parts of AI instead to give an more realistic value
    return 1000; //40 s AFAIK
}

static int estimate_room_research_time(int kind, struct NodeState * state)
{
    //TODO AI: check from research values in config, our existing creatures, and research costs
    return 1000;
}

static Node * new_node(enum SAI_PlanDecisionType type, struct Node * parent)
{
    struct Node * node;

    node = (Node *) calloc(1, sizeof(*node));
    node->decision.type = type;
    node->id = planner.next_node_id++;
    node->parent = parent;

    return node;
}

/**
 * Compiles the game state to a plan state and makes a node for it.
 * @return
 */
static Node * insert_root_node(void)
{
    struct Node * root;
    int i;

    root = new_node(SAI_PLAN_WAIT, NULL);

    prepare_creature_pool_state(&root->state);
    prepare_planning_player_state(&root->state);

    for (i = 0; i < MAX_KEEPERS; ++i) {
        if (player_exists(get_player(i))) {
            prepare_player_state(&root->state.players[i], i);
        }
    }

    planner.open.insert(root);

    return root;
}

static void insert_wait_node(struct Node * parent)
{
    struct Node * wait;

    wait = new_node(SAI_PLAN_WAIT, parent);
    memcpy(&wait->state, &parent->state, sizeof(wait->state)); //same as children do the waiting

    planner.open.insert(wait);
}

static void insert_build_room_node(struct Node * parent, int kind)
{
    struct Node * build;
    int time;

    build = new_node(SAI_PLAN_BUILD_ROOM, parent);
    build->decision.param.kind = kind;
    time = estimate_room_build_time(kind);
    if (parent->decision.type == SAI_PLAN_WAIT) {
        time += estimate_room_research_time(kind, &parent->state);
    }

    state_time_simulation(&build->state, &parent->state, time);
    build->state.rooms_built |= (1 << kind);

    planner.open.insert(build);
}

static int visit_node(struct Node * node)
{
    int i;
    int can_build;
    int can_wait_to_research;

    //AIDBG(14, "Visiting node of type %i", node->decision.type);

    if (node_score(node) > node_score(planner.best_leaf)) {
        planner.best_leaf = node;
    }

    //Generate children:

    //1) we can always wait if we haven't waited (a wait node following a wait node has no purpose)
    if (node->decision.type != SAI_PLAN_WAIT) {
        insert_wait_node(node);
    }

    //2) room build nodes
    can_wait_to_research = node->decision.type == SAI_PLAN_WAIT &&
        node->state.rooms_built & (1 << RoK_LIBRARY)?
            planner.env.rooms_researchable : 0;
    can_build = ~node->state.rooms_built & (node->state.rooms_available | can_wait_to_research);
    for (i = 0; i < ROOM_TYPES_COUNT; ++i) {
        if (can_build & (1 << i)) {
            insert_build_room_node(node, i);
        }
    }

    return 0;
}

static const char * decision_param_string(struct SAI_PlanDecision * decision)
{
    switch (decision->type) {
    case SAI_PLAN_TAKE_ROOM:
    case SAI_PLAN_BUILD_ROOM:
        return room_code_name(decision->param.kind);
    default:
        return "(N/A)";
    }
}

void SAI_make_plan(int plyr_idx, struct SAI_PlanDecision ** decisions, int * num_decisions)
{
    struct Node * node;
    struct Node * root_node;
    std::set<Node *> closed;
    std::set<Node *>::iterator it;
    int i;

    AIDBG(3, "Starting");

    planner.open.clear();

    planner.my_plyr_idx = plyr_idx;
    planner.next_node_id = 0;
    memset(&planner.env, 0, sizeof(planner.env));
    prepare_environment(&planner.env);

    root_node = insert_root_node();
    planner.best_leaf = root_node;
    *num_decisions = 0;

    AIDBG(4, "Beginning planning loop");

    do {
        node = *planner.open.begin();
        planner.open.erase(planner.open.begin());
        closed.insert(node); //for now only used for deallocation, because nodes are never equal for this planner

        if (visit_node(node)) {
            break;
        }
    } while (!planner.open.empty() && planner.next_node_id < MAX_PLAN_SIZE);

    AIDBG(4, "Extracting results");

    //find number of decisions for best decision path
    for (node = planner.best_leaf; node != root_node; node = node->parent) {
        *num_decisions += 1;
    }

    *decisions = (struct SAI_PlanDecision *) malloc(sizeof(**decisions) * *num_decisions);

    //extract best decision path
    for (i = *num_decisions - 1, node = planner.best_leaf; i >= 0; --i) {
        AIDBG(5, "Plan decision: %s %s", plan_names[node->decision.type],
            decision_param_string(&node->decision));

        memcpy(*decisions + i, &node->decision, sizeof(node->decision));
        node = node->parent;
    }

    AIDBG(4, "Cleaning up");

    //clean up
    for (it = closed.begin(); it != closed.end(); ++it) {
        free(*it);
    }
    for (it = planner.open.begin(); it != planner.open.end(); ++it) {
        free(*it);
    }

    planner.open.clear();

    AIDBG(3, "Ending");
}
