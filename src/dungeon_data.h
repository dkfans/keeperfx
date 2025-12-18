/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file dungeon_data.h
 *     Header file for dungeon_data.c.
 * @par Purpose:
 *     Dungeon data structures definitions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_DNGN_DATA_H
#define DK_DNGN_DATA_H

#include "bflib_basics.h"
#include "config_magic.h"
#include "config_trapdoor.h"
#include "config_terrain.h"
#include "player_computer.h"
#include "globals.h"
#include "dungeon_stats.h"
#include "engine_camera.h"
#include "thing_creature.h"
#include "thing_doors.h"
#include "room_data.h"
#include "player_data.h"
#include "map_events.h"
#include "tasks_list.h"
#include "thing_traps.h"
#include "roomspace.h"
#include "config_creature.h"
#include "creature_states.h"

#include "power_hand.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define DUNGEONS_COUNT              9
#define DIGGER_TASK_MAX_COUNT       64
#define DUNGEON_RESEARCH_COUNT      2000
#define MAX_THINGS_IN_HAND          64
#define TURN_TIMERS_COUNT           8
#define SCRIPT_FLAGS_COUNT          8
#define MAX_SOE_RADIUS              13
#define CREATURE_GUI_JOBS_COUNT     3
#define CUSTOM_BOX_COUNT            256
#define FX_LINES_COUNT              32
#define MAX_SUMMONS                 255

#define INVALID_DUNGEON (&bad_dungeon)

enum CreatureGUIJob {
    CrGUIJob_Any        =-1,
    CrGUIJob_Wandering  = 0,
    CrGUIJob_Working    = 1,
    CrGUIJob_Fighting   = 2,
};

enum DungeonCreatureTendencies {
    CrTend_None       = 0,
    CrTend_Imprison   = 1,
    CrTend_Flee       = 2,
};

enum DungeonResearchCategory {
    RsCat_None        = 0,
    RsCat_Power       = 1,
    RsCat_Room        = 2,
    RsCat_Creature    = 3,
};

enum DungeonManufactureBuildFlags {
    MnfBldF_Manufacturable = 0x01,
    MnfBldF_Built          = 0x02,
    MnfBldF_Used           = 0x04,
};

/******************************************************************************/
struct DiggerStack {
      SubtlCodedCoords stl_num;
      SpDiggerTaskType task_type;
};

struct ResearchVal {
  unsigned char rtyp;
  unsigned short rkind;
  long req_amount;
};

struct TurnTimer {
  unsigned long count;
  unsigned char state;
};

struct TrapInfo
{
    unsigned char trap_amount_offmap[TRAPDOOR_TYPES_MAX];
    unsigned char trap_amount_stored[TRAPDOOR_TYPES_MAX];
    /** Stores flag information about players manufacture of traps of specific kind. */
    unsigned char trap_build_flags[TRAPDOOR_TYPES_MAX];
    /** Amount of traps of every kind for which we can place blueprints. This include both off-map traps and on-map trap boxes.*/
    unsigned char trap_amount_placeable[TRAPDOOR_TYPES_MAX];
    /** Amount of doors of every kind which are stored in workshops. Only on-map door crates which exist in workshop are mentioned here.*/

    unsigned char door_amount_offmap[TRAPDOOR_TYPES_MAX];
    unsigned char door_amount_stored[TRAPDOOR_TYPES_MAX];
    /** Stored flag information about players manufacture of doors of specific kind. */
    unsigned char door_build_flags[TRAPDOOR_TYPES_MAX];
    /** Stored information whether player can place blueprints of doors of specific kind (actually, doors are placed instantly). */
    unsigned char door_amount_placeable[TRAPDOOR_TYPES_MAX];

};

struct BoxInfo
{
    uint16_t              activated[CUSTOM_BOX_COUNT];
};


/** Used to set player modifier with script command. */
struct Modifiers
{
    unsigned short health;
    unsigned short strength;
    unsigned short armour;
    unsigned short spell_damage;
    unsigned short speed;
    unsigned short pay;
    unsigned short training_cost;
    unsigned short scavenging_cost;
    unsigned short loyalty;
};

struct Dungeon {
    unsigned short dnheart_idx;
    struct Coord3d mappos;
    unsigned char creature_tendencies;
    unsigned char computer_enabled;
    short creatr_list_start;
    short digger_list_start;
    ThingIndex summon_list[MAX_SUMMONS];
    unsigned short num_summon;
    ThingIndex things_in_hand[MAX_THINGS_IN_HAND];
    unsigned char num_things_in_hand;
    unsigned short crmodel_state_type_count[CREATURE_TYPES_MAX][STATE_TYPES_COUNT];
    unsigned short guijob_all_creatrs_count[CREATURE_TYPES_MAX][3];
    unsigned short guijob_angry_creatrs_count[CREATURE_TYPES_MAX][3];
    int sight_casted_gameturn;
    short sight_casted_thing_idx;
    KeepPwrLevel sight_casted_power_level;
    MapSubtlCoord sight_casted_stl_x;
    MapSubtlCoord sight_casted_stl_y;
    unsigned char soe_explored_flags[2*MAX_SOE_RADIUS][2*MAX_SOE_RADIUS];
    MapSubtlCoord cta_stl_x;
    MapSubtlCoord cta_stl_y;
    KeepPwrLevel cta_power_level;
    unsigned long cta_start_turn;
    TbBool cta_free;
    unsigned long must_obey_turn;
    int hold_audience_cast_turn;
    int scavenge_counters_turn;
    /** Counter of creatures scavenging of each kind, zeroized and recomputed each game turn. */
    unsigned char creatures_scavenging[CREATURE_TYPES_MAX];
    /** Counter of creatures praying. */
    unsigned char creatures_praying[CREATURE_TYPES_MAX];
    unsigned char chickens_sacrificed;
    unsigned char gold_piles_sacrificed;
    unsigned char creature_sacrifice[CREATURE_TYPES_MAX];
    unsigned char creature_sacrifice_exp[CREATURE_TYPES_MAX];
    unsigned short num_active_diggers;
    unsigned short num_active_creatrs;
    unsigned short owned_creatures_of_model[CREATURE_TYPES_MAX];
    /** Total amount of rooms in possession of a player. Rooms which can never be built are not counted. */
    unsigned char total_rooms;
    unsigned short total_doors;
    unsigned short total_area;
    unsigned short total_creatures_left;
    int doors_destroyed;
    short room_manage_area;
    short creatures_scavenged[CREATURE_TYPES_MAX];
    short creatures_scavenge_gain;
    short creatures_scavenge_lost;
    long scavenge_turn_points[CREATURE_TYPES_MAX];
    short scavenge_targets[CREATURE_TYPES_MAX];
    CrtrExpLevel creature_max_level[CREATURE_TYPES_MAX];
    unsigned short creatures_annoyed;
    unsigned short battles_lost;
    unsigned short battles_won;
    /** Amount of room tiles a player had which were destroyed (vandalized or damaged by enemy). */
    unsigned short rooms_destroyed;
    unsigned short spells_stolen;
    unsigned short times_broken_into;
    unsigned short gold_pots_stolen;
    int last_entrance_generation_gameturn;
    int turns_between_entrance_generation;
    int last_research_complete_gameturn;
    int manage_score;
    int total_score;
    unsigned long max_gameplay_score;
    short times_breached_dungeon;
    short highest_task_number;
    GoldAmount total_money_owned;
    GoldAmount offmap_money_owned;
    struct MapTask task_list[MAPTASKS_COUNT];
    int task_count;
    unsigned char owner;
    int camera_deviate_quake;
    int camera_deviate_jump;
    long score;
    struct ResearchVal research[DUNGEON_RESEARCH_COUNT];
    int current_research_idx;
    unsigned short research_num;
    /** How many creatures are force-enabled for each kind.
     * Force-enabled creature can come from portal without additional conditions,
     * but only until dungeon has up to given amount of their kind. */
    unsigned char creature_force_enabled[CREATURE_TYPES_MAX];
    /** Defines whether a creature of each kind is allowed to come from portal.
     * Allowed creatures can join a dungeon if whether attraction condition is met
     * or force-enabled amount isn't reached. */
    unsigned char creature_allowed[CREATURE_TYPES_MAX];
    unsigned short magic_level[POWER_TYPES_MAX];
    unsigned short magic_resrchable[POWER_TYPES_MAX];
    struct TurnTimer turn_timers[TURN_TIMERS_COUNT];
    long max_creatures_attracted;
    unsigned char heart_destroy_state;
    long heart_destroy_turn;
    struct Coord3d essential_pos;
    struct DiggerStack digger_stack[DIGGER_TASK_MAX_COUNT];
    unsigned long digger_stack_update_turn;
    unsigned long digger_stack_length;
    unsigned char visible_event_idx;
    /** Array with battle indexes with the battles currently visible in fight event message */
    unsigned char visible_battles[3];
    short zoom_annoyed_creature_idx;
    long total_experience_creatures_gained;
    long total_research_points;
    long total_manufacture_points;
    long manufacture_progress;
    ThingClass manufacture_class;
    ThingModel manufacture_kind;
    long turn_last_manufacture;
    long manufacture_level;
    long research_progress;
    struct LevelStats lvstats;
    struct CreatureStorage dead_creatures[DEAD_CREATURES_MAX_COUNT];
    long dead_creatures_count;
    long dead_creature_idx;
    /** Contains map event index or each even button visible on screen. */
    unsigned char event_button_index[EVENT_BUTTONS_COUNT+1];
    unsigned short tortured_creatures[CREATURE_TYPES_MAX];
    unsigned char bodies_rotten_for_vampire;
    long portal_scavenge_boost;
    /** Stores how many creatures of each kind of has joined the dungeon during the level.
     * Values are saturated at 255. */
    unsigned char creature_models_joined[CREATURE_TYPES_MAX];
    unsigned long fights_num;
    unsigned char research_override; // could be easily changed into flags..
    int last_creature_dropped_gameturn;
    MapSubtlCoord devastation_centr_x;
    MapSubtlCoord devastation_centr_y;
    GameTurn devastation_turn;
    long creatures_total_pay;
    unsigned short gold_hoard_for_pickup;
    unsigned long gold_pickup_amount;
    /** Index of last creature picked up of given model. */
    unsigned short selected_creatures_of_model[CREATURE_TYPES_MAX];
    /** Index of last creature picked up of given GUI Job. */
    unsigned short selected_creatures_of_gui_job[CREATURE_GUI_JOBS_COUNT];
    unsigned char texture_pack;
    unsigned char color_idx;
    struct Modifiers      modifier;
    struct TrapInfo       mnfct_info;
    struct BoxInfo        box_info;
    struct BoxInfo        trap_info;
    struct Coord3d        last_combat_location;
    struct Coord3d        last_eventful_death_location;
    struct Coord3d        last_trap_event_location;
    int                   creature_awarded[CREATURE_TYPES_MAX];
    CrtrExpLevel          creature_entrance_level;
    unsigned long         evil_creatures_converted;
    unsigned long         good_creatures_converted;
    unsigned long         creatures_transferred;
    unsigned long         traps_sold;
    unsigned long         doors_sold;
    unsigned long         manufacture_gold;
    long                  creatures_total_backpay;
    long                  cheaper_diggers;
    long                  event_last_run_turn[EVENT_KIND_COUNT];
    long                  script_flags[SCRIPT_FLAGS_COUNT];
    unsigned short        room_list_start[TERRAIN_ITEMS_MAX];
    unsigned char         room_buildable[TERRAIN_ITEMS_MAX];
    unsigned char         room_resrchable[TERRAIN_ITEMS_MAX];
    unsigned char         room_discrete_count[TERRAIN_ITEMS_MAX+1];
    unsigned short        backup_heart_idx;
    unsigned short        free_soul_idx;
    struct HandRule       hand_rules[CREATURE_TYPES_MAX][HAND_RULE_SLOTS_COUNT];
};
/******************************************************************************/
extern struct Dungeon bad_dungeon;
/******************************************************************************/
struct Dungeon *get_players_num_dungeon_f(long plyr_idx,const char *func_name);
struct Dungeon *get_players_dungeon_f(const struct PlayerInfo *player,const char *func_name);
struct Dungeon *get_dungeon_f(PlayerNumber plyr_num,const char *func_name);
#define get_players_num_dungeon(plyr_idx) get_players_num_dungeon_f(plyr_idx,__func__)
#define get_players_dungeon(player) get_players_dungeon_f(player,__func__)
#define get_dungeon(plyr_idx) get_dungeon_f(plyr_idx,__func__)
#define get_my_dungeon() get_players_num_dungeon_f(my_player_number,__func__)

TbBool dungeon_invalid(const struct Dungeon *dungeon);

void clear_dungeons(void);
void init_dungeons(void);

void decrease_dungeon_area(PlayerNumber plyr_idx, long value);
void increase_room_area(PlayerNumber plyr_idx, long value);
void decrease_room_area(PlayerNumber plyr_idx, long value);
void increase_dungeon_area(PlayerNumber plyr_idx, long value);
TbBool mark_creature_joined_dungeon(struct Thing *creatng);

void player_add_offmap_gold(PlayerNumber plyr_idx, GoldAmount value);

void init_dungeons_essential_position(void);
const struct Coord3d *dungeon_get_essential_pos(PlayerNumber plyr_idx);
TbBool player_has_heart(PlayerNumber plyr_idx);
struct Thing *get_player_soul_container(PlayerNumber plyr_idx);

TbBool player_has_room_of_role(PlayerNumber plyr_idx, RoomRole rrole);
TbBool dungeon_has_room(const struct Dungeon *dungeon, RoomKind rkind);
TbBool dungeon_has_room_of_role(const struct Dungeon *dungeon, RoomRole rrole);
long count_player_discrete_rooms_with_role(PlayerNumber plyr_idx, RoomRole rrole);

TbBool set_creature_tendencies(struct PlayerInfo *player, unsigned short tend_type, TbBool val);
TbBool toggle_creature_tendencies(struct PlayerInfo *player, unsigned short tend_type);
TbBool player_creature_tends_to(PlayerNumber plyr_idx, unsigned short tend_type);

TbBool set_trap_buildable_and_add_to_amount(PlayerNumber plyr_idx, ThingModel trap_kind, long buildable, long amount);
TbBool set_door_buildable_and_add_to_amount(PlayerNumber plyr_idx, ThingModel door_kind, long buildable, long amount);
TbBool dungeon_has_any_buildable_traps(struct Dungeon *dungeon);
TbBool dungeon_has_any_buildable_doors(struct Dungeon *dungeon);

TbBool restart_script_timer(PlayerNumber plyr_idx, long timer_id);
TbBool set_script_flag(PlayerNumber plyr_idx, long flag_id, long value);
void add_to_script_timer(PlayerNumber plyr_idx, unsigned char timer_id, long value);

void add_heart_health(PlayerNumber plyr_idx,HitPoints healthdelta,TbBool warn_on_damage);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
