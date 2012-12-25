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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define COMPUTER_TASKS_COUNT        100
#define COMPUTER_PROCESSES_COUNT     20
#define COMPUTER_CHECKS_COUNT        15
#define COMPUTER_EVENTS_COUNT        12
#define COMPUTER_PROCESS_LISTS_COUNT 14
#define COMPUTER_TRAP_LOC_COUNT      20

#define COMPUTER_PROCESS_TYPES_COUNT 26
#define COMPUTER_CHECKS_TYPES_COUNT  51
#define COMPUTER_EVENTS_TYPES_COUNT  31

/** How strong should be the preference to dig glod from treasure room and not other rooms. Originally was 22 subtiles. */
#define TREASURE_ROOM_PREFERENCE_WHILE_DIGGING_GOLD 16

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
    CTT_MoveCreatureToRoom, // 10
    CTT_MoveCreatureToPos,
    CTT_MoveCreaturesToDefend,
    CTT_SlapImps,
    CTT_DigToNeutral,
    CTT_MagicSpeedUp,
    CTT_WaitForBridge,
    CTT_AttackMagic,
    CTT_SellTrapsAndDoors,
};

enum TrapDoorSellingCategory {
    TDSC_EndList = 0,
    TDSC_Door,
    TDSC_Trap,
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
    GA_Unk12,
    GA_Unk13,
    GA_Unk14,
    GA_Unk15,
    GA_Unk16,
    GA_SetTendencies,
    GA_PlaceTrap,
    GA_PlaceDoor,
    GA_UsePwrLightning,
    GA_UsePwrSpeedUp,
    GA_UsePwrArmour,
    GA_UsePwrConceal,
    GA_UsePwrHoldAudnc,
    GA_UsePwrDisease,
    GA_UsePwrChicken,
    GA_Unk27,
    GA_UsePwrSlap,
};

enum ToolDigFlags {
    ToolDig_BasicOnly = 0x00,
    ToolDig_AllowValuable = 0x01, /**< Allows to dig through valuable slabs. */
    ToolDig_AllowLiquidWBridge = 0x02, /**< Allows to dig through liquid slabs, if only player has ability to build bridges through them.
                                            Also allows to dig through valuable slabs(which should be later changed)). */
};

/******************************************************************************/
#pragma pack(1)

struct Computer2;
struct ComputerProcess;
struct ComputerCheck;
struct ComputerEvent;
struct Event;
struct Thing;
struct ComputerTask;

typedef unsigned char ComputerType;
typedef char ComputerName[LINEMSG_SIZE];

typedef long (*Comp_Process_Func)(struct Computer2 *comp, struct ComputerProcess *process);
typedef long (*Comp_Check_Func)(struct Computer2 *comp, struct ComputerCheck * check);
typedef long (*Comp_Event_Func)(struct Computer2 *comp, struct ComputerEvent *cevent,struct Event *event);
typedef long (*Comp_EvntTest_Func)(struct Computer2 *comp, struct ComputerEvent *cevent);
typedef long (*Comp_Task_Func)(struct Computer2 *comp, struct ComputerTask *ctask);

struct Comp_Check_Func_ListItem {
  const char *name;
  Comp_Check_Func func;
};

struct TaskFunctions {
  const char *name;
  Comp_Task_Func func;
};

struct ComputerProcess { // sizeof = 72
  char *name;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned long field_14;
  Comp_Process_Func func_check;
  Comp_Process_Func func_setup;
  Comp_Process_Func func_task;
  Comp_Process_Func func_complete;
  Comp_Process_Func func_pause;
  struct ComputerProcess *parent;
  unsigned long field_30;
  unsigned long field_34;
  unsigned long field_38;
  unsigned long field_3C;
  unsigned long field_40;
  unsigned long field_44;
};

struct ComputerCheck { // sizeof = 32
  char *name;
  unsigned long flags;
  long turns_interval;
  Comp_Check_Func func;
  long param1;
  long param2;
  long param3;
  long turns_last;
};

struct ComputerEvent { // sizeof = 44
  char *name;
  unsigned long cetype;
  unsigned long field_8;
  Comp_Event_Func func_event;
  Comp_EvntTest_Func func_test;
  long test_interval;
  struct ComputerProcess *process;
  long param1;
  long param2;
  long param3;
  long last_test_gameturn; // event last checked time
};

struct ComputerProcessTypes { // sizeof = 1124
  char *name;
  long field_4;
  long field_8;
  long field_C;
  long field_10;
  long field_14;
  long field_18;
  long field_1C;
  struct ComputerProcess *processes[COMPUTER_PROCESSES_COUNT];
  struct ComputerCheck checks[COMPUTER_CHECKS_COUNT];
  struct ComputerEvent events[COMPUTER_EVENTS_COUNT];
  long field_460;
};

struct ValidRooms { // sizeof = 8
  long rkind;
  struct ComputerProcess *process;
};

struct ComputerProcessMnemonic {
  char name[16];
  struct ComputerProcess *process;
};

struct ComputerCheckMnemonic {
  char name[16];
  struct ComputerCheck *check;
};

struct ComputerEventMnemonic {
  char name[16];
  struct ComputerEvent *event;
};

struct ComputerDig { // sizeof = 78
    struct Coord3d pos_E;
    struct Coord3d pos_14;
    struct Coord3d pos_gold;
    struct Coord3d pos_20;
    long distance;
    unsigned char field_2A;
    unsigned char direction_around;
    unsigned long field_2C;
    unsigned char field_30[19];
    unsigned char field_43[5];
    long field_48;
    long field_4C;
    long field_50;
    long field_54;
    long field_58;
};

struct ComputerTask { // sizeof = 148
    unsigned char flags;
    unsigned char field_1;
    unsigned char ttype;
    unsigned char ottype;
    unsigned char field_4[6];
    long field_A;
    union {
        struct ComputerDig dig;
        struct {
            unsigned char field_E[21];
            unsigned char field_23[32];
            unsigned char field_43[6];
            unsigned char field_49[19];
        };
    };
    long field_5C;
    long field_60;
    struct Coord3d pos_64;
    unsigned char field_6A[6];
    union {
    struct Coord3d pos_70;
    struct {
      long field_70;
      unsigned char field_74[2];
    };
    };
    union {
    struct Coord3d pos_76;
    long long_76;
    struct {
      short word_76;
      short word_78;
    };
    };
    long field_7C;
    union {
    long field_80;
    struct {
        union {
        short gold_lookup_idx;
        short word_80;
        };
      short word_82;
    };
    };
    unsigned char field_84[2];
    union {
    long long_86;
    struct {
      short word_86;
      short word_88;
    };
    };
    unsigned char field_8A[2];
    unsigned short field_8C;
    long field_8E;
    unsigned short next_task;
};

struct Comp2_UnkStr1 { // sizeof = 394
  unsigned char field_0[6];
  unsigned long field_6;
  unsigned char field_A[380];
  unsigned long field_186;
};

struct Computer2 { // sizeof = 5322
  long field_0;
  unsigned long gameturn_delay;
  unsigned long gameturn_wait;
  unsigned long field_C;
  unsigned long tasks_did;
  unsigned long field_14;
  unsigned long field_18;
  unsigned long field_1C; // seems to be signed long
  unsigned long field_20;
  struct Dungeon *dungeon;
  unsigned long model;
  unsigned long field_2C;
  unsigned long field_30;
  unsigned long field_34;
  struct ComputerProcess processes[COMPUTER_PROCESSES_COUNT+1];
  struct ComputerCheck checks[COMPUTER_CHECKS_COUNT];
  struct ComputerEvent events[COMPUTER_EVENTS_COUNT];
  struct Comp2_UnkStr1 unkarr_A10[5];
  unsigned char field_11C2[394];
  struct Coord3d trap_locations[COMPUTER_TRAP_LOC_COUNT];
  unsigned char field_13C4[60];
  unsigned char field_1400[196];
  short ongoing_process;
  short task_idx;
  short field_14C8;
};

struct ComputerPlayerConfig {
    int processes_count;
    int checks_count;
    int events_count;
    int computers_count;
};

struct THate {
    char field_0;
};

#pragma pack()
/******************************************************************************/
extern unsigned short computer_types[];
/******************************************************************************/
DLLIMPORT struct ComputerProcessTypes _DK_ComputerProcessLists[14];
//#define ComputerProcessLists _DK_ComputerProcessLists
/******************************************************************************/
struct ComputerProcessTypes *get_computer_process_type_template(long cpt_idx);
void shut_down_process(struct Computer2 *comp, struct ComputerProcess *process);
void reset_process(struct Computer2 *comp, struct ComputerProcess *process);
void suspend_process(struct Computer2 *comp, struct ComputerProcess *process);
/******************************************************************************/
long set_next_process(struct Computer2 *comp);
void computer_check_events(struct Computer2 *comp);
TbBool process_checks(struct Computer2 *comp);
long get_computer_money_less_cost(struct Computer2 *comp);
struct Room *get_room_to_place_creature(const struct Computer2 *comp, const struct Thing *thing);
/******************************************************************************/
struct ComputerTask *get_computer_task(long idx);
struct ComputerTask *get_task_in_progress(struct Computer2 *comp, long a2);
struct ComputerTask *get_free_task(struct Computer2 *comp, long a2);
TbBool computer_task_invalid(struct ComputerTask *ctask);
TbBool remove_task(struct Computer2 *comp, struct ComputerTask *ctask);
TbBool create_task_move_creatures_to_defend(struct Computer2 *comp, struct Coord3d *pos, long creatrs_num, unsigned long evflags);
TbBool create_task_magic_call_to_arms(struct Computer2 *comp, struct Coord3d *pos, long creatrs_num);
TbBool create_task_sell_traps_and_doors(struct Computer2 *comp, long value);
TbBool create_task_move_creature_to_pos(struct Computer2 *comp, struct Thing *thing, long a2, long a3);
long computer_able_to_use_magic(struct Computer2 *comp, long a2, long a3, long a4);
long process_tasks(struct Computer2 *comp);
TbResult game_action(PlayerNumber plyr_idx, unsigned short gaction, unsigned short alevel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2);
TbResult try_game_action(struct Computer2 *comp, PlayerNumber plyr_idx, unsigned short gaction, unsigned short alevel,
    MapSubtlCoord stl_x, MapSubtlCoord stl_y, unsigned short param1, unsigned short param2);
short tool_dig_to_pos2(struct Computer2 * comp, struct ComputerDig * cdig, TbBool simulation, unsigned short digflags);
/******************************************************************************/
void setup_a_computer_player(unsigned short plyridx, long comp_model);
void process_computer_players2(void);
TbBool load_computer_player_config(unsigned short flags);
void setup_computer_players2(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
