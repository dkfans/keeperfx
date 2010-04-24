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

#define COMPUTER_CHECKS_TYPES_COUNT  51
#define COMPUTER_EVENTS_TYPES_COUNT  31

#ifdef __cplusplus
#pragma pack(1)
#endif

struct Computer2;
struct ComputerProcess;
struct ComputerCheck;
struct ComputerEvent;
struct Event;

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
  unsigned long field_4;
  unsigned long field_8;
  Comp_Event_Func func_event;
  Comp_EvntTest_Func func_test;
  long field_14;
  struct ComputerProcess *process;
  long param1;
  long param2;
  long param3;
  long param4;
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

struct ComputerTask { // sizeof = 148
    unsigned char field_0;
    unsigned char field_1;
    unsigned char ttype;
    unsigned char field_3[7];
    long field_A;
    unsigned char field_E[21];
    unsigned char field_23[32];
    unsigned char field_43[6];
    unsigned char field_49[19];
    long field_5C;
    long field_60;
    unsigned char field_64[12];
    long field_70;
    unsigned char field_74[2];
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
      short word_80;
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
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned long field_14;
  unsigned long field_18;
  unsigned long field_1C;
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
  unsigned char field_11C2[446];
  unsigned char field_1380[128];
  unsigned char field_1400[196];
  short field_14C4;
  short field_14C6;
  short field_14C8;
};

#ifdef __cplusplus
#pragma pack()
#endif

/******************************************************************************/
extern unsigned short computer_types[];
/******************************************************************************/
DLLIMPORT struct ComputerProcessTypes _DK_ComputerProcessLists[14];
//#define ComputerProcessLists _DK_ComputerProcessLists
/******************************************************************************/
void shut_down_process(struct Computer2 *comp, struct ComputerProcess *process);
void reset_process(struct Computer2 *comp, struct ComputerProcess *process);
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
long process_tasks(struct Computer2 *comp);
/******************************************************************************/
void setup_a_computer_player(unsigned short plyridx, long comp_model);
void process_computer_players2(void);
short load_computer_player_config(void);
void setup_computer_players2(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
