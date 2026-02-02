/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_computer.h
 *     Header file for player_computer.cpp.
 *     Note that this file is a C header, while its code is CPP.
 * @par Purpose:
 *     Computer player definitions and activities.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Mar 2009 - 20 Mar 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PLYR_COMPUT_H
#define DK_PLYR_COMPUT_H

#include "bflib_basics.h"
#include "globals.h"

#include "config.h"
#include "config_compp.h"
#include "player_data.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define COMPUTER_TRAP_LOC_COUNT      20

#define COMPUTER_SPARK_POSITIONS_COUNT 64
#define COMPUTER_SOE_GRID_SIZE        8

/** How strong should be the preference to dig gold from treasure room and not other rooms. Originally was 22 subtiles. */
#define TREASURE_ROOM_PREFERENCE_WHILE_DIGGING_GOLD 16

/** How often to check for possible gold veins which could be digged by computer */
#define GOLD_DEMAND_CHECK_INTERVAL 5000
/** How long to wait for diggers to prepare a place for room before dropping the task and assuming it failed */
#define COMPUTER_DIG_ROOM_TIMEOUT 7500
#define COMPUTER_URGENT_BRIDGE_TIMEOUT 1200
#define COMPUTER_TOOL_DIG_LIMIT 356
#define COMPUTER_TOOL_FAILED_DIG_LIMIT 10

/** Holds the return values for the CPU "mark for digging" functions. (see enum ToolDigResults) */
typedef signed char ToolDigResult;
/** Flags to enable actions (e.g. dig gold, build bridge) for the CPU player whilst "marking for digging" (see enum ToolDigFlags). */
typedef signed char DigFlags;

#define COMPUTER_REDROP_DELAY 80

enum ComputerTaskTypes {
    CTT_None = 0,
    CTT_DigRoomPassage,
    CTT_DigRoom,
    CTT_CheckRoomDug,
    CTT_PlaceRoom,
    CTT_DigToEntrance,
    CTT_DigToGold,
    CTT_DigToAttack,
    CTT_MagicCallToArms,
    CTT_PickupForAttack,
    CTT_MoveCreatureToRoom,     // 10
    CTT_MoveCreatureToPos,
    CTT_MoveCreaturesToDefend,
    CTT_SlapDiggers,
    CTT_DigToNeutral,
    CTT_MagicSpeedUp,
    CTT_WaitForBridge,
    CTT_AttackMagic,
    CTT_SellTrapsAndDoors,
    CTT_MoveGoldToTreasury,
};

enum TrapDoorSellingCategory {
    TDSC_EndList = 0,
    TDSC_DoorCrate,
    TDSC_TrapCrate,
    TDSC_DoorPlaced,
    TDSC_TrapPlaced,
};

enum GameActionTypes {
    GA_None = 0,
    GA_Unk01,
    GA_UsePwrHandPick,
    GA_UsePwrHandDrop,
    GA_UseMkDigger,
    GA_UseSlap,
    GA_UsePwrSight,
    GA_UsePwrObey,
    GA_UsePwrHealCrtr,
    GA_UsePwrCall2Arms,
    GA_UsePwrCaveIn,
    GA_StopPwrCall2Arms,
    GA_StopPwrHoldAudnc,
    GA_Unk13,
    GA_MarkDig,
    GA_Unk15,
    GA_PlaceRoom,
    GA_SetTendencies,
    GA_PlaceTrap,
    GA_PlaceDoor,
    GA_UsePwrLightning,
    GA_UsePwrSpeedUp,
    GA_UsePwrArmour,
    GA_UsePwrRebound,
    GA_UsePwrConceal,
    GA_UsePwrFlight,
    GA_UsePwrVision,
    GA_UsePwrHoldAudnc,
    GA_UsePwrDisease,
    GA_UsePwrChicken,
    GA_UsePwrFreeze,
    GA_UsePwrSlow,
    GA_Unk27,
    GA_UsePwrSlap,
    GA_SellTrap,
    GA_SellDoor,
};

enum ToolDigFlags {
    ToolDig_BasicOnly = 0x00, /**< Allows digging through basic earth slabs (default: always applies). */
    ToolDig_AllowValuable = 0x01, /**< Allows digging through valuable slabs. */
    ToolDig_AllowLiquidWBridge = 0x02, /**< Allows bridging over liquid (bridges must be available to the player for this to have an effect). */
};

/** These are the possible return values for the CPU player's "mark for digging" functions */
enum ToolDigResults {
    TDR_BuildBridgeOnSlab = -5,
    TDR_ToolDigError = -2,
    TDR_ReachedDestination = -1,
    TDR_DigSlab = 0,
};

enum CompProcessFlags {
    ComProc_Unkn0001 = 0x0001,
    ComProc_ListEnd  = 0x0002, /**< Last? */
    ComProc_Unkn0004 = 0x0004, /**< Finished */
    ComProc_Unkn0008 = 0x0008, /**< Done (for subprocesses) */
    ComProc_Unkn0010 = 0x0010,
    ComProc_Unkn0020 = 0x0020, /**< Suspended (Ed: I think this flag is RoomBuildActive...) */
    ComProc_Unkn0040 = 0x0040,
    ComProc_Unkn0080 = 0x0080,
    ComProc_Unkn0100 = 0x0100,
    ComProc_Unkn0200 = 0x0200,
    ComProc_Unkn0400 = 0x0400,
    ComProc_Unkn0800 = 0x0800,
};

enum CompCheckFlags {
    ComChk_Unkn0001 = 0x0001, /**< Disabled */
    ComChk_Unkn0002 = 0x0002, /**< Last */
    ComChk_Unkn0004 = 0x0004,
    ComChk_Unkn0008 = 0x0008,
    ComChk_Unkn0010 = 0x0010,
    ComChk_Unkn0020 = 0x0020,
    ComChk_Unkn0040 = 0x0040,
    ComChk_Unkn0080 = 0x0080,
    ComChk_Unkn0100 = 0x0100,
    ComChk_Unkn0200 = 0x0200,
    ComChk_Unkn0400 = 0x0400,
    ComChk_Unkn0800 = 0x0800,
};

enum CompTaskFlags {
    ComTsk_Unkn0001 = 0x0001, /**< task is disabled */
    ComTsk_Unkn0002 = 0x0002,
    ComTsk_AddTrapLocation = 0x0004, /** if enabled, dug slabs will be added to the computer's list of potential trap positions */
    ComTsk_Unkn0008 = 0x0008,
    ComTsk_Unkn0010 = 0x0010,
    ComTsk_Unkn0020 = 0x0020,
    ComTsk_Unkn0040 = 0x0040,
    ComTsk_Urgent = 0x0080,
};

enum CompTaskStates {
    CTaskSt_None = 0,
    CTaskSt_Wait, /**< Waiting some game turns before starting a new task. */
    CTaskSt_Select, /**< Choosing a task to be performed. */
    CTaskSt_Perform, /**< Performing the task. */
};

/** Return values for computer task functions. */
enum CompTaskRet {
    CTaskRet_Unk0 = 0,
    CTaskRet_Unk1, /**< CONTINUE */
    CTaskRet_Unk2,
    CTaskRet_Unk3,
    CTaskRet_Unk4, /**< FAIL? Wait? */
};

/** Return values for computer process functions. */
enum CompProcRet {
    CProcRet_Fail = 0,
    CProcRet_Continue,
    CProcRet_Finish,
    CProcRet_Unk3,
    CProcRet_Wait,
};

enum ItemAvailabilityRet {
    IAvail_Never         = 0,
    IAvail_Now           = 1,
    IAvail_NeedResearch  = 4,
};

enum CompChatFlags {
    CChat_None          = 0x00,
    CChat_TasksScarce   = 0x01,
    CChat_TasksFrequent = 0x02,
};

enum computer_process_func_list 
{
    cpfl_computer_check_build_all_rooms = 1,
    cpfl_computer_setup_any_room_continue,
    cpfl_computer_check_any_room,
    cpfl_computer_setup_any_room,
    cpfl_computer_check_dig_to_entrance,
    cpfl_computer_setup_dig_to_entrance,
    cpfl_computer_check_dig_to_gold,
    cpfl_computer_setup_dig_to_gold,
    cpfl_computer_check_sight_of_evil,
    cpfl_computer_setup_sight_of_evil,
    cpfl_computer_process_sight_of_evil,
    cpfl_computer_check_attack1,
    cpfl_computer_setup_attack1,
    cpfl_computer_completed_attack1,
    cpfl_computer_check_safe_attack,
    cpfl_computer_process_task,
    cpfl_computer_completed_build_a_room,
    cpfl_computer_paused_task,
    cpfl_computer_completed_task
  };

//TODO COMPUTER This returns NULL, which is unsafe
#define INVALID_COMPUTER_PLAYER NULL
#define INVALID_COMPUTER_PROCESS NULL
#define INVALID_COMPUTER_TASK &game.computer_task[0]
/******************************************************************************/
#pragma pack(1)

struct Computer2;
struct ComputerProcess;
struct ComputerCheck;
struct ComputerEvent;
struct Event;
struct Thing;
struct Room;
struct ComputerTask;
struct GoldLookup;
struct THate;

typedef unsigned short ComputerTaskType;

typedef long (*Comp_Process_Func)(struct Computer2 *, struct ComputerProcess *);
typedef long (*Comp_Check_Func)(struct Computer2 *, struct ComputerCheck *);
typedef long (*Comp_Event_Func)(struct Computer2 *, struct ComputerEvent *,struct Event *);
typedef long (*Comp_EvntTest_Func)(struct Computer2 *, struct ComputerEvent *);
typedef long (*Comp_Task_Func)(struct Computer2 *, struct ComputerTask *);
typedef TbBool (*Comp_HateTest_Func)(const struct Computer2 *, const struct ComputerProcess *, const struct THate *);

struct TaskFunctions {
    const char *name;
    Comp_Task_Func func;
};

struct ValidRooms { // sizeof = 8
    int32_t rkind;
    unsigned char process_idx;
};

struct ComputerDig {
    struct Coord3d pos_E; /**< used by dig to position - set to pos_begin when a dig action fails ?? */
    struct Coord3d pos_dest; /**< used by dig to position - the destination */
    struct Coord3d pos_begin; /**< used by dig to position (the start of the path) and for room dig/place (the centre of the room) */
    struct Coord3d pos_next; /**< used by dig to position - the next position in the path to check */
    long distance; /**< used by dig to position - the distance between a given position and the destination */
    unsigned char hug_side; /**< used by dig to position - the rule to follow when hugging the wall (left-hand rule/side or right-hand rule/side) */
    SmallAroundIndex direction_around; /**< used by dig to position - the forwards direction of the path */
    unsigned long action_success_flag; /**< this is always set to 1... but it's value is used to create a bool test: did action fail */
    long number_of_failed_actions; /**< used by dig to position (incremented when gold is found but digflags is 0, or a mark for digging action failed) */
    MapSubtlCoord last_backwards_step_stl_x; /**< used by dig to position - ?? when a dig action fails, we step backwards, this is this the X coordinate of the slab we stepped back in to */
    MapSubtlCoord last_backwards_step_stl_y; /**< used by dig to position - ?? when a dig action fails, we step backwards, this is this the Y coordinate of the slab we stepped back in to */
    long calls_count; /**< used by dig to position */
    long valuable_slabs_tagged; /**< used by dig to position - Amount of valuable slabs tagged for digging during this dig process. */
    /** Variables for digging (or placing) a room. */
    struct { 
        long area; /**< The number of slabs in the room. */
        long slabs_processed; /**< The number of slabs marked for digging or converted in to a room. */
        /** Variables for the spiral used to dig slabs/place rooms. */
        struct {
            SmallAroundIndex forward_direction; /**< The current direction we are moving through the spiral. */
            long turns_made; /**< The number of turns made in the spiral. */
            long steps_to_take_before_turning; /**< The number of steps to take before the next turn in the spiral. */
            long steps_remaining_before_turn; /**< The number of steps we have left to take before we need to turn in the spiral. */
        } spiral;
    } room;
};

struct ComputerTask {
    unsigned char flags; /**< Values from ComTsk_* enumeration. */
    unsigned char task_state;
    unsigned char ttype;
    unsigned char ottype;
    unsigned char rkind;
    int32_t created_turn;
    struct ComputerDig dig;
    int32_t lastrun_turn;
    int32_t delay;
    struct Coord3d new_room_pos;
    struct Coord3d starting_position;
    union {
    struct {
        /** Amount of items to be sold; task is removed when it reaches zero. */
        int32_t items_amount;
        /** Sum of gold generated by selling. */
        int32_t gold_gain;
        /** Limit of gold generated by selling; task is removed when it is exceeded. */
        int32_t gold_gain_limit;
        /** Limit of total gold owned by player; task is removed when it is exceeded. */
        int32_t total_money_limit;
        /** Whether deployed traps and doors are considered while selling. */
        short allow_deployed; // can be converted to flags
        /** Index of the item currently being checked in list of sellable things. */
        int32_t sell_idx;
    } sell_traps_doors;
    struct {
        /* Amount of gold piles/pots to move */
        int32_t items_amount;
        short room_idx;
        int32_t gold_gain;
        int32_t gold_gain_limit;
        int32_t total_money_limit;
    } move_gold;
    struct {
        struct Coord3d target_pos;
        long repeat_num;
    } magic_cta;
    struct {
        KeepPwrLevel power_level;
        short target_thing_idx;
        int32_t repeat_num;
        int32_t gaction;
        int32_t pwkind;
    } attack_magic;
    struct {
        RoomIndex room_idx1;
        int32_t repeat_num;
        RoomIndex room_idx2;
    } move_to_room;
    struct {
        int32_t evflags;
        struct Coord3d target_pos;
        int32_t repeat_num;
        CrtrStateId target_state;
    } move_to_defend;
    struct {
        short target_thing_idx;
        struct Coord3d target_pos;
        int32_t repeat_num;
        CrtrStateId target_state;
    } move_to_pos;
    struct {
        struct Coord3d target_pos;
        int32_t repeat_num;
        CrtrStateId target_state;
    } pickup_for_attack;
    struct {
        struct Coord3d startpos;
        struct Coord3d endpos;
        /** Target room index. */
        short target_room_idx;
        short target_plyr_idx;
    } dig_to_room;
    struct {
        struct Coord3d startpos;
        struct Coord3d endpos;
        /** Target gold lookup index. */
        short target_lookup_idx;
        int32_t slabs_dig_count;
    } dig_to_gold;
    struct {
        struct Coord3d startpos;
        struct Coord3d endpos;
        short target_plyr_idx;
    } dig_somewhere;
    struct {
        struct Coord3d startpos;
        struct Coord3d endpos;
        short width;
        short height;
        RoomKind kind;
        long area;
    } create_room;
    struct {
        TbBool skip_speed;
    } slap_imps;
    };
    unsigned short cproc_idx; /**< CProcessId */
    GameTurnDelta cta_duration;
    unsigned short next_task;
};

struct OpponentRelation { // sizeof = 394
    GameTurn last_interaction_turn;
    short next_idx;
    int32_t hate_amount;
    struct Coord3d pos_A[COMPUTER_SPARK_POSITIONS_COUNT];
};

struct Computer2 { // sizeof = 5322
  int32_t task_state;
  uint32_t gameturn_delay;
  uint32_t gameturn_wait;
  uint32_t action_status_flag;
  uint32_t tasks_did;
  uint32_t processes_time;
  uint32_t click_rate;
  int32_t dig_stack_size; // seems to be signed long
  uint32_t sim_before_dig;
  struct Dungeon *dungeon;
  uint32_t model;
  uint32_t turn_begin;
  uint32_t max_room_build_tasks;
  uint32_t task_delay;
  struct ComputerProcess processes[COMPUTER_PROCESSES_COUNT+1];
  struct ComputerCheck checks[COMPUTER_CHECKS_COUNT];
  struct ComputerEvent events[COMPUTER_EVENTS_COUNT];
  struct OpponentRelation opponent_relations[PLAYERS_COUNT];
  // TODO we could use coord2d for trap locations
  struct Coord3d trap_locations[COMPUTER_TRAP_LOC_COUNT];
  /** Stores Sight Of Evil target points data. */
  unsigned long soe_targets[COMPUTER_SOE_GRID_SIZE];
  short ongoing_process;
  short task_idx;
  short held_thing_idx;
};

/**
 * Contains value of hate between players.
 */
struct THate {
    int32_t amount;
    int32_t plyr_idx;
    struct Coord3d *pos_near;
    int32_t distance_near;
};

struct ExpandRooms {
    RoomKind rkind;
    short max_slabs;
};

/******************************************************************************/

#pragma pack()
/******************************************************************************/

/******************************************************************************/
extern struct ValidRooms valid_rooms_to_build[];

extern const struct NamedCommand computer_process_func_type[];
extern Comp_Process_Func computer_process_func_list[];

extern const struct NamedCommand computer_event_func_type[];
extern Comp_Event_Func computer_event_func_list[];

extern const struct NamedCommand computer_event_test_func_type[];
extern Comp_EvntTest_Func computer_event_test_func_list[];

extern const struct NamedCommand computer_check_func_type[];
extern Comp_Check_Func computer_check_func_list[];
/******************************************************************************/
struct Computer2 *get_computer_player_f(long plyr_idx,const char *func_name);
#define get_computer_player(plyr_idx) get_computer_player_f(plyr_idx,__func__)
TbBool computer_player_invalid(const struct Computer2 *comp);
long set_autopilot_type(PlayerNumber plridx, long aptype);
/******************************************************************************/
void shut_down_process(struct Computer2 *comp, struct ComputerProcess *cproc);
void reset_process(struct Computer2 *comp, struct ComputerProcess *cproc);
void suspend_process(struct Computer2 *comp, struct ComputerProcess *cproc);
long computer_process_index(const struct Computer2 *comp, const struct ComputerProcess *cproc);
struct ComputerProcess *get_computer_process(struct Computer2 *comp, int cproc_idx);
/******************************************************************************/
TbBool computer_player_in_emergency_state(const struct Computer2 *comp);
TbBool is_there_an_attack_task(const struct Computer2 *comp);
struct ComputerTask *computer_setup_build_room(struct Computer2 *comp, RoomKind rkind, long width_slabs, long height_slabs, long look_randstart);
struct ComputerTask * able_to_build_room(struct Computer2 *comp, struct Coord3d *pos, RoomKind rkind, long width_slabs, long height_slabs, long max_slabs_dist, long perfect);
long computer_finds_nearest_room_to_gold(struct Computer2 *comp, struct Coord3d *pos, struct GoldLookup **gldlookref);
void setup_dig_to(struct ComputerDig *cdig, const struct Coord3d startpos, const struct Coord3d endpos);
long move_imp_to_dig_here(struct Computer2 *comp, struct Coord3d *pos, long max_amount);
long move_imp_to_mine_here(struct Computer2 *comp, struct Coord3d *pos, long max_amount);
void get_opponent(struct Computer2 *comp, struct THate hate[]);
long add_to_trap_locations(struct Computer2 *, struct Coord3d *);
/******************************************************************************/
long set_next_process(struct Computer2 *comp);
void computer_check_events(struct Computer2 *comp);
TbBool process_checks(struct Computer2 *comp);
GoldAmount get_computer_money_less_cost(const struct Computer2 *comp);
GoldAmount get_dungeon_money_less_cost(const struct Dungeon *dungeon);
TbBool creature_could_be_placed_in_better_room(const struct Computer2 *comp, const struct Thing *thing);
CreatureJob get_job_to_place_creature_in_room(const struct Computer2 *comp, const struct Thing *thing);
long xy_walkable(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long plyr_idx);
/******************************************************************************/
struct ComputerTask *get_computer_task(long idx);
struct ComputerTask *get_task_in_progress(struct Computer2 *comp, ComputerTaskType ttype);
struct ComputerTask *get_task_in_progress_in_list(const struct Computer2 *comp, const ComputerTaskType *ttypes);
TbBool is_task_in_progress(struct Computer2 *comp, ComputerTaskType ttype);
TbBool is_task_in_progress_using_hand(struct Computer2 *comp);
TbBool computer_task_invalid(const struct ComputerTask *ctask);
TbBool remove_task(struct Computer2 *comp, struct ComputerTask *ctask);
void shut_down_task_process(struct Computer2 *comp, struct ComputerTask *ctask);
const char *computer_task_code_name(int ctask_type);

TbBool create_task_move_creatures_to_defend(struct Computer2 *comp, struct Coord3d *pos, long creatrs_num, unsigned long evflags);
TbBool create_task_move_creatures_to_room(struct Computer2 *comp, int room_idx, long creatrs_num);
TbBool create_task_magic_battle_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long par2, long creatrs_num);
TbBool create_task_magic_support_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long cta_duration, long repeat_num);
TbBool create_task_pickup_for_attack(struct Computer2 *comp, struct Coord3d *pos, long creatrs_num);
TbBool create_task_sell_traps_and_doors(struct Computer2 *comp, long num_to_sell, GoldAmount gold_up_to, TbBool allow_deployed);
TbBool create_task_move_gold_to_treasury(struct Computer2 *comp, long num_to_move, long gold_up_to);
TbBool create_task_move_creature_to_subtile(struct Computer2 *comp, const struct Thing *thing, MapSubtlCoord stl_x, MapSubtlCoord stl_y, CrtrStateId dst_state);
TbBool create_task_move_creature_to_pos(struct Computer2 *comp, const struct Thing *thing, const struct Coord3d pos, CrtrStateId dst_state);
TbBool create_task_dig_to_attack(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, PlayerNumber victim_plyr_idx, long parent_cproc_idx);
TbBool create_task_slap_imps(struct Computer2 *comp, long creatrs_num, TbBool skip_speed);
TbBool create_task_dig_to_neutral(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos);
TbBool create_task_dig_to_gold(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, long parent_cproc_idx, long count_slabs_to_dig, long gold_lookup_idx);
TbBool create_task_dig_to_entrance(struct Computer2 *comp, const struct Coord3d startpos, const struct Coord3d endpos, long parent_cproc_idx, long entroom_idx);
TbBool create_task_magic_speed_up(struct Computer2 *comp, const struct Thing *creatng, KeepPwrLevel power_level);
TbBool create_task_attack_magic(struct Computer2 *comp, const struct Thing *creatng, PowerKind pwkind, int repeat_num, KeepPwrLevel power_level, int gaction);
TbResult script_computer_dig_to_location(long plyr_idx, TbMapLocation origin, TbMapLocation destination);

TbBool computer_able_to_use_power(struct Computer2 *comp, PowerKind pwkind, KeepPwrLevel power_level, long amount);
long computer_get_room_role_total_capacity(struct Computer2 *comp, RoomRole rrole);
long computer_get_room_kind_free_capacity(struct Computer2 *comp, RoomKind room_kind);
TbBool computer_finds_nearest_room_to_pos(struct Computer2 *comp, struct Room **retroom, struct Coord3d *nearpos);
long process_tasks(struct Computer2 *comp);
long computer_check_any_room(struct Computer2* comp, struct ComputerProcess* cproc);
TbResult game_action(PlayerNumber plyr_idx, unsigned short gaction, KeepPwrLevel power_level,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2);
TbResult try_game_action(struct Computer2 *comp, PlayerNumber plyr_idx, unsigned short gaction, KeepPwrLevel power_level,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2);
ToolDigResult tool_dig_to_pos2_f(struct Computer2 * comp, struct ComputerDig * cdig, TbBool simulation, DigFlags digflags, const char *func_name);
TbBool add_trap_location_if_requested(struct Computer2 *comp, struct ComputerTask *ctask, TbBool is_task_dig_to_attack);
#define tool_dig_to_pos2(comp,cdig,simulation,digflags) tool_dig_to_pos2_f(comp,cdig,simulation,digflags,__func__)
#define search_spiral(pos, owner, area_total, cb) search_spiral_f(pos, owner, area_total, cb, __func__)
int search_spiral_f(struct Coord3d *pos, PlayerNumber owner, int area_total, long (*cb)(MapSubtlCoord, MapSubtlCoord, long), const char *func_name);
/******************************************************************************/
ItemAvailability computer_check_room_available(const struct Computer2 * comp, RoomKind rkind);
TbBool computer_find_non_solid_block(const struct Computer2 *comp, struct Coord3d *pos);
TbBool computer_find_safe_non_solid_block(const struct Computer2* comp, struct Coord3d* pos);

long count_creatures_in_dungeon(const struct Dungeon *dungeon);
long count_entrances(const struct Computer2 *comp, PlayerNumber plyr_idx);
long count_diggers_in_dungeon(const struct Dungeon *dungeon);
long check_call_to_arms(struct Computer2 *comp);
long count_creatures_for_defend_pickup(struct Computer2 *comp);
long count_creatures_for_pickup(struct Computer2 *comp, struct Coord3d *pos, struct Room *room, long a4);
unsigned long count_creatures_availiable_for_fight(struct Computer2 *comp, struct Coord3d *pos);

long setup_computer_attack(struct Computer2 *comp, struct ComputerProcess *cproc, struct Coord3d *pos, long victim_plyr_idx);

TbBool setup_a_computer_player(PlayerNumber plyr_idx, long comp_model);
void process_computer_players2(void);
void setup_computer_players2(void);
void restore_computer_player_after_load(void);

TbBool computer_force_dump_held_things_on_map(struct Computer2 *comp, const struct Coord3d *pos);
TbBool computer_force_dump_specific_held_thing(struct Computer2 *comp, struct Thing *thing, const struct Coord3d *pos);
TbBool thing_is_in_computer_power_hand_list(const struct Thing *thing, PlayerNumber plyr_idx);
struct Thing* find_creature_for_defend_pickup(struct Computer2* comp);

TbBool script_support_setup_player_as_computer_keeper(PlayerNumber plyr_idx, long comp_model);
TbBool script_support_setup_player_as_zombie_keeper(PlayerNumber plyr_idx);
TbBool reactivate_build_process(struct Computer2* comp, RoomKind rkind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
