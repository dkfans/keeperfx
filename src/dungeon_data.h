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
#include "config_trapdoor.h"
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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define DUNGEONS_COUNT              5
#define DIGGER_TASK_MAX_COUNT       64
#define DUNGEON_RESEARCH_COUNT      34
#define MAX_THINGS_IN_HAND          8
#define KEEPER_POWERS_COUNT         20
#define TURN_TIMERS_COUNT           8
#define SCRIPT_FLAGS_COUNT          8
#define MAX_SOE_RADIUS              13
#define CREATURE_GUI_JOBS_COUNT     3
#define CUSTOM_BOX_COUNT            256
#define FX_LINES_COUNT              32

#define INVALID_DUNGEON (&bad_dungeon)
#define INVALID_DUNGEON_ADD (&bad_dungeonadd)

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
#pragma pack(1)

struct DiggerStack { // sizeof = 4
      unsigned short stl_num;
      SpDiggerTaskType task_type;
};

struct ResearchVal { // sizeof = 6
  unsigned char rtyp;
  unsigned char rkind;
  long req_amount;
};

struct TurnTimer { // sizeof = 5
  unsigned long count;
  unsigned char state;
};

#define SIZEOF_Dungeon 0x1508
struct Dungeon {
    unsigned short dnheart_idx;
    struct Coord3d mappos;
    unsigned char creature_tendencies;
    unsigned char field_9;
    unsigned char computer_enabled;
    unsigned short room_kind[ROOM_TYPES_COUNT];
    short creatr_list_start;
    short digger_list_start;
    short field_31;
    short things_in_hand[MAX_THINGS_IN_HAND];
    short field_43;
    int field_45;
    int field_49;
    int field_4D;
    short field_51;
    short field_53;
    int field_55;
    int field_59;
    int field_5D;
    short field_61;
    unsigned char num_things_in_hand;
    unsigned short field_64[CREATURE_TYPES_COUNT][15];
    unsigned short guijob_all_creatrs_count[CREATURE_TYPES_COUNT][3];
    unsigned short guijob_angry_creatrs_count[CREATURE_TYPES_COUNT][3];
    short field_5A4[9];//originally was [15], but seem unused
    unsigned char trap_amount_offmap_[TRAP_TYPES_COUNT];
    unsigned char door_amount_offmap_[DOOR_TYPES_COUNT];
    unsigned char room_slabs_count[ROOM_TYPES_COUNT+1];
    int sight_casted_gameturn;
    short sight_casted_thing_idx;
    unsigned char sight_casted_splevel;
    unsigned char sight_casted_stl_x;
    unsigned char sight_casted_stl_y;
    unsigned char soe_explored_flags[2*MAX_SOE_RADIUS][2*MAX_SOE_RADIUS];
    unsigned char cta_stl_x;
    unsigned char cta_stl_y;
    unsigned char cta_splevel;
    unsigned long cta_start_turn;
    unsigned long must_obey_turn;
    int hold_audience_cast_turn;
    int scavenge_counters_turn;
    /** Counter of creatures scavenging of each kind, zeroized and recomputed each game turn. */
    unsigned char creatures_scavenging[CREATURE_TYPES_COUNT];
    /** Counter of creatures praying. */
    unsigned char creatures_praying[CREATURE_TYPES_COUNT];
    unsigned char chickens_sacrificed;
    unsigned char gold_piles_sacrificed;
    unsigned char creature_sacrifice[CREATURE_TYPES_COUNT];
    unsigned char creature_sacrifice_exp[CREATURE_TYPES_COUNT];
    unsigned char field_916[2];
    unsigned char num_active_diggers;
    unsigned char num_active_creatrs;
    unsigned char owned_creatures_of_model[CREATURE_TYPES_COUNT];
    /** Total amount of rooms in possession of a player. Rooms which can never be built are not counted. */
    unsigned char total_rooms;
    unsigned short total_doors;
    unsigned short total_area;
    unsigned short total_creatures_left;
    int field_941;
    int doors_destroyed;
    short room_manage_area;
    short creatures_scavenged[CREATURE_TYPES_COUNT];
    short creatures_scavenge_gain;
    short creatures_scavenge_lost;
    long scavenge_turn_points[CREATURE_TYPES_COUNT];
    short scavenge_targets[CREATURE_TYPES_COUNT];
    int creature_max_level[CREATURE_TYPES_COUNT];
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
    int total_money_owned;
    int offmap_money_owned;
    short hates_player[DUNGEONS_COUNT];
    struct MapTask task_list[MAPTASKS_COUNT];
    int task_count;
    int field_E93[3];
    unsigned char owner;
    int camera_deviate_quake;
    int camera_deviate_jump;
    long score;
    struct ResearchVal research[DUNGEON_RESEARCH_COUNT];
    int current_research_idx;
    unsigned char research_num;
unsigned char field_F7D;
    unsigned char room_buildable[ROOM_TYPES_COUNT];
    unsigned char room_resrchable[ROOM_TYPES_COUNT];
    /** How many creatures are force-enabled for each kind.
     * Force-enabled creature can come from portal without additional conditions,
     * but only until dungeon has up to given amount of their kind. */
    unsigned char creature_force_enabled[CREATURE_TYPES_COUNT];
    /** Defines whether a creature of each kind is allowed to come from portal.
     * Allowed creatures can join a dungeon if whether attraction condition is met
     * or force-enabled amount isn't reached. */
    unsigned char creature_allowed[CREATURE_TYPES_COUNT];
    unsigned char magic_level[KEEPER_POWERS_COUNT];
    unsigned char magic_resrchable[KEEPER_POWERS_COUNT];
    /** Amount of traps of every kind which are stored in workshops. Only on-map trap crates which exist in workshop are mentioned here.*/
    unsigned char trap_amount_stored_[TRAP_TYPES_COUNT];
    /** Stores flag information about players manufacture of traps of specific kind. */
    unsigned char trap_build_flags_[TRAP_TYPES_COUNT];
    /** Amount of traps of every kind for which we can place blueprints. This include both off-map traps and on-map trap boxes.*/
    unsigned char trap_amount_placeable_[TRAP_TYPES_COUNT];
    /** Amount of doors of every kind which are stored in workshops. Only on-map door crates which exist in workshop are mentioned here.*/
    unsigned char door_amount_stored_[DOOR_TYPES_COUNT];
    /** Stored flag information about players manufacture of doors of specific kind. */
    unsigned char door_build_flags_[DOOR_TYPES_COUNT];
    /** Stored information whether player can place blueprints of doors of specific kind (actually, doors are placed instantly). */
    unsigned char door_amount_placeable_[DOOR_TYPES_COUNT];
    struct TurnTimer turn_timers[TURN_TIMERS_COUNT];
    unsigned char script_flags[SCRIPT_FLAGS_COUNT];
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
    unsigned char manufacture_class;
    unsigned char manufacture_kind;
long field_118B;
long manufacture_level;
    long research_progress;
    struct LevelStats lvstats;
    struct CreatureStorage dead_creatures[DEAD_CREATURES_MAX_COUNT];
    long dead_creatures_count;
    long dead_creature_idx;
    /** Contains map event index or each even button visible on screen. */
    unsigned char event_button_index[EVENT_BUTTONS_COUNT+1];
    long event_last_run_turn_UNUSED[27];
    unsigned short tortured_creatures[CREATURE_TYPES_COUNT];
    unsigned char bodies_rotten_for_vampire;
unsigned char field_1461[36];
    long portal_scavenge_boost;
    /** Stores how many creatures of each kind of has joined the dungeon during the level.
     * Values are saturated at 255. */
    unsigned char creature_models_joined[CREATURE_TYPES_COUNT];
    unsigned long fights_num;
    unsigned char research_override; // could be easily changed into flags..
    int last_creature_dropped_gameturn;
    unsigned char devastation_centr_x;
    unsigned char devastation_centr_y;
    unsigned long devastation_turn;
    long creatures_total_pay;
unsigned short field_14BC;
unsigned long field_14BE;
    /** Index of last creature picked up of given model. */
    unsigned short selected_creatures_of_model[CREATURE_TYPES_COUNT];
    /** Index of last creature picked up of given GUI Job. */
    unsigned short selected_creatures_of_gui_job[CREATURE_GUI_JOBS_COUNT];
    };

#pragma pack()

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
    uint8_t               activated[CUSTOM_BOX_COUNT];
};

struct ComputerInfo
{
    struct ComputerEvent events[COMPUTER_EVENTS_COUNT];
    struct ComputerCheck checks[COMPUTER_CHECKS_COUNT];
};

struct DungeonAdd
{
    struct TrapInfo       mnfct_info;
    struct BoxInfo        box_info;
    struct Coord3d        last_combat_location;
    int                   creature_awarded[CREATURE_TYPES_COUNT];
    struct RoomSpace      roomspace;
    unsigned char         creature_entrance_level;
    unsigned long         evil_creatures_converted;
    unsigned long         good_creatures_converted;
    unsigned long         traps_sold;
    unsigned long         doors_sold;
    unsigned long         manufacture_gold;
    long                  cheaper_diggers;
    TbBool                one_click_lock_cursor;
    TbBool                ignore_next_PCtr_RBtnRelease;
    TbBool                ignore_next_PCtr_LBtnRelease;
    long                  swap_to_untag_mode; // 0 = no, 1 = maybe, 2= yes, -1 = disable
    struct ComputerInfo   computer_info;
    long event_last_run_turn[EVENT_KIND_COUNT];
};
/******************************************************************************/
extern struct Dungeon bad_dungeon;
extern struct DungeonAdd bad_dungeonadd;
/******************************************************************************/
struct Dungeon *get_players_num_dungeon_f(long plyr_idx,const char *func_name);
struct Dungeon *get_players_dungeon_f(const struct PlayerInfo *player,const char *func_name);
struct Dungeon *get_dungeon_f(PlayerNumber plyr_num,const char *func_name);
struct DungeonAdd *get_dungeonadd_f(PlayerNumber plyr_num,const char *func_name);
#define get_players_num_dungeon(plyr_idx) get_players_num_dungeon_f(plyr_idx,__func__)
#define get_players_dungeon(player) get_players_dungeon_f(player,__func__)
#define get_dungeon(plyr_idx) get_dungeon_f(plyr_idx,__func__)
#define get_dungeonadd(plyr_idx) get_dungeonadd_f(plyr_idx,__func__)
#define get_my_dungeon() get_players_num_dungeon_f(my_player_number,__func__)

TbBool dungeon_invalid(const struct Dungeon *dungeon);
TbBool dungeonadd_invalid(const struct DungeonAdd *dungeonadd);

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

TbBool player_has_room(PlayerNumber plyr_idx, RoomKind rkind);
TbBool player_has_room_of_role(PlayerNumber plyr_idx, RoomRole rrole);
TbBool dungeon_has_room(const struct Dungeon *dungeon, RoomKind rkind);

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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
