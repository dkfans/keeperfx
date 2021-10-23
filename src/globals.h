/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file globals.h
 *     KeeperFX global compile config file.
 * @par Purpose:
 *     Header file for global definitions.
 * @par Comment:
 *     Defines basic includes and definitions, used in whole program.
 * @author   Tomasz Lis
 * @date     08 Aug 2008 - 03 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KEEPFX_GLOBALS_H
#define KEEPFX_GLOBALS_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <assert.h>

#if defined(unix) && !defined(GO32)
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <termios.h>
#if !defined(stricmp)
#define stricmp strcasecmp
#endif
#if !defined(strnicmp)
#define strnicmp strncasecmp
#endif

#elif defined(MSDOS)
#include <dos.h>
#include <process.h>
#endif

#include "version.h"

#ifndef BFDEBUG_LEVEL
#error "BFDEBUG_LEVEL should be defined in version.h"
#define BFDEBUG_LEVEL 0
#endif

#if defined(BUILD_DLL)
# define DLLIMPORT __declspec (dllexport)
#else // Not defined BUILD_DLL
# define DLLIMPORT __declspec (dllimport)
#endif

#ifdef __cplusplus
#include <algorithm>
using std::min;
using std::max;
extern "C" {
#endif

// Basic Definitions

#if defined(unix) && !defined (GO32)
#define SEPARATOR "/"
#else
#define SEPARATOR "\\"
#endif

#ifndef false
#define false 0
#endif
#ifndef true
#define true 1
#endif
#ifndef NULL
#define NULL 0
#endif

#ifndef __cplusplus
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#endif

// Return values for verification functions
#define VERIF_ERROR   0
#define VERIF_OK      1
#define VERIF_WARN    2

// Return values for all other functions
#define ERR_NONE           0
// Note: error codes -1..-79 are reserved standard C library errors with sign reverted.
//    these are defined in errno.h
#define ERR_BASE_RNC      -90

// Debug fuction-like macros - for free messages
#define ERRORMSG(format, ...) LbErrorLog(format "\n", ##__VA_ARGS__)
#define WARNMSG(format, ...) LbWarnLog(format "\n", ##__VA_ARGS__)
#define SYNCMSG(format, ...) LbSyncLog(format "\n", ##__VA_ARGS__)
#define JUSTMSG(format, ...) LbJustLog(format "\n", ##__VA_ARGS__)
#define SCRPTMSG(format, ...) LbScriptLog(format "\n", ##__VA_ARGS__)
#define NETMSG(format, ...) LbNetLog(format "\n", ##__VA_ARGS__)
#define NOMSG(format, ...)

// Debug function-like macros - for code logging (with function name)
#define ERRORLOG(format, ...) LbErrorLog("%s: " format "\n", __func__ , ##__VA_ARGS__)
#define WARNLOG(format, ...) LbWarnLog("%s: " format "\n", __func__ , ##__VA_ARGS__)
#define SYNCLOG(format, ...) LbSyncLog("%s: " format "\n", __func__ , ##__VA_ARGS__)
#define JUSTLOG(format, ...) LbJustLog("%s: " format "\n", __func__ , ##__VA_ARGS__)
#define SCRPTLOG(format, ...) LbScriptLog(text_line_number,"%s: " format "\n", __func__ , ##__VA_ARGS__)
#define SCRPTERRLOG(format, ...) LbErrorLog("%s(line %lu): " format "\n", __func__ , text_line_number, ##__VA_ARGS__)
#define SCRPTWRNLOG(format, ...) LbWarnLog("%s(line %lu): " format "\n", __func__ , text_line_number, ##__VA_ARGS__)
#define CONFLOG(format, ...) LbConfigLog(text_line_number,"%s: " format "\n", __func__ , ##__VA_ARGS__)
#define CONFERRLOG(format, ...) LbErrorLog("%s(line %lu): " format "\n", __func__ , text_line_number, ##__VA_ARGS__)
#define CONFWRNLOG(format, ...) LbWarnLog("%s(line %lu): " format "\n", __func__ , text_line_number, ##__VA_ARGS__)
#define NETLOG(format, ...) LbNetLog("%s: " format "\n", __func__ , ##__VA_ARGS__)
#define NOLOG(format, ...)

// Debug function-like macros - for debug code logging
#if (BFDEBUG_LEVEL > 0)
  #define SYNCDBG(dblv,format, ...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbSyncLog("%s: " format "\n", __func__ , ##__VA_ARGS__); }
  #define WARNDBG(dblv,format, ...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbWarnLog("%s: " format "\n", __func__ , ##__VA_ARGS__); }
  #define ERRORDBG(dblv,format, ...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbErrorLog("%s: " format "\n", __func__ , ##__VA_ARGS__); }
  #define NAVIDBG(dblv,format, ...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbNaviLog("%s: " format "\n", __func__ , ##__VA_ARGS__); }
  #define NETDBG(dblv,format, ...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbNetLog("%s: " format "\n", __func__ , ##__VA_ARGS__); }
  #define SCRIPTDBG(dblv,format, ...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbScriptLog(text_line_number,"%s: " format "\n", __func__ , ##__VA_ARGS__); }
  #define AIDBG(dblv,format, ...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbAiLog("%s: " format "\n", __func__ , ##__VA_ARGS__); }
#else
  #define SYNCDBG(dblv,format, ...)
  #define WARNDBG(dblv,format, ...)
  #define ERRORDBG(dblv,format, ...)
  #define NAVIDBG(dblv,format, ...)
  #define NETDBG(dblv,format, ...)
  #define SCRIPTDBG(dblv,format, ...)
  #define AIDBG(dblv,format, ...)
#endif

#if AUTOTESTING
  #include "event_monitoring.h"
  #define EVM_CREATURE_EVENT(event_name, plyr_id, thing) \
    evm_stat(0, "ev.%s,cr=%s,thing=%d,plyr=%d cnt=1", event_name, get_string(creature_data_get(thing->model)->namestr_idx), thing->index, plyr_id)
  #define EVM_CREATURE_EVENT_WITH_TARGET(event_name, plyr_id, thing, targ_val) \
    evm_stat(0, "ev.%s,cr=%s,thing=%d,plyr=%d cnt=1,targ=%d", event_name, get_string(creature_data_get(thing->model)->namestr_idx), thing->index, plyr_id, targ_val)
  #define EVM_MAP_EVENT(event_name, plyr_idx, x, y, opt) \
    evm_stat(0, "map.%s,x=%d,y=%d,plyr=%d,opt=%s cnt=1,x=%d,y=%d", event_name, x, y, plyr_idx, opt, x,y)
  #define EVM_CREATURE_STAT(event_name, plyr_id, thing, stat_name, stat_val) \
    evm_stat(0, "ev.%s,cr=%s,thing=%d,plyr=%d %s=%d", event_name, get_string(creature_data_get(thing->model)->namestr_idx), thing->index, plyr_id, stat_name, stat_val)
#else
  #define EVM_CREATURE_EVENT(event_name, plyr_id, thing)
  #define EVM_CREATURE_EVENT_WITH_TARGET(event_name, plyr_id, thing, targ_val)
  #define EVM_CREATURE_STAT(event_name, plyr_id, thing, stat_name, stat_val)
  #define EVM_MAP_EVENT(event_name, plyr_idx, x, y, opt)
#endif

void replaceFn(void* oldFn, void* newFn);
#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)
#define HOOK_DK_FUNC(name) \
	DLLIMPORT void _DK_##name(); \
	__attribute__((constructor)) static void CONCAT(hookFn, __COUNTER__)(void) { replaceFn(&_DK_##name, &name); }

#pragma pack(1)

/** Screen coordinate in scale of the game (resolution independent). */
typedef int ScreenCoord;
/** Screen coordinate in scale of the real screen. */
typedef int RealScreenCoord;
/** Player identification number, or owner of in-game thing/room/slab. */
typedef signed char PlayerNumber;
/** Type which stores thing class. */
typedef unsigned char ThingClass;
/** Type which stores thing model. */
typedef unsigned char ThingModel;
/** Type which stores thing index. */
typedef unsigned short ThingIndex;
/** Type which stores creature state index. */
typedef unsigned short CrtrStateId;
/** Type which stores creature experience level. */
typedef unsigned char CrtrExpLevel;
/** Type which stores creature annoyance reason, from CreatureAngerReasons enumeration. */
typedef unsigned char AnnoyMotive;
/** Type which stores room kind index. */
typedef unsigned char RoomKind;
/** Type which stores room role flags. */
typedef unsigned long RoomRole;
/** Type which stores room index. */
typedef unsigned short RoomIndex;
/** Type which stores slab kind index. */
typedef unsigned char SlabKind;
/** Type which stores SplK_* values. */
typedef unsigned short SpellKind;
/** Type which stores PwrK_* values. */
typedef unsigned short PowerKind;
/** Type which stores EvKind_* values. */
typedef unsigned char EventKind;
/** Type which stores dungeon special kind. */
typedef unsigned short SpecialKind;
/** Type which stores index of the new event, or negative index of updated event, in map events array. */
typedef short EventIndex;
typedef short BattleIndex;
typedef long HitPoints;
/** Type which stores TUFRet_* values. */
typedef short TngUpdateRet;
/** Type which stores CrStRet_* values. */
typedef short CrStateRet;
/** Type which stores CrCkRet_* values. */
typedef short CrCheckRet;
/** Type which stores Job_* values. */
typedef unsigned long long CreatureJob;
/** Creature instance index, stores CrInst_* values. */
typedef short CrInstance;
/** Creature attack type, stores AttckT_* values. */
typedef short CrAttackType;
/** Creature death flags, stores CrDed_* values. */
typedef unsigned short CrDeathFlags;
/** Level number within a campaign. */
typedef long LevelNumber;
/** Game turn number, used for in-game time computations. */
typedef unsigned long GameTurn;
/** Game turns difference, used for in-game time computations. */
typedef long GameTurnDelta;
/** Identifier of a national text string. */
typedef int TextStringId;
/** Map coordinate in full resolution. Position within subtile is scaled 0..255. */
typedef long MapCoord;
/** Distance between map coordinates in full resolution. */
typedef long MapCoordDelta;
/** Map subtile coordinate. Every slab consists of 3x3 subtiles. */
typedef long MapSubtlCoord;
/** Distance between map subtiles. */
typedef long MapSubtlDelta;
/** Map slab coordinate. Slab is a cubic part of map with specific content. */
typedef short MapSlabCoord;
/** Map subtile 2D coordinates, coded into one number. */
typedef unsigned long SubtlCodedCoords;
/** Map slab 2D coordinates, coded into one number. */
typedef unsigned long SlabCodedCoords;
/** Index in the columns array. */
typedef short ColumnIndex;
/** A variable which bits store bool value for each player. */
typedef unsigned char PerPlayerFlags;
/** Movement speed on objects in the game. */
typedef short MoveSpeed;
/** Parameter for storing gold sum or price. */
typedef long GoldAmount;
/** Type for storing Action Point index.
 * Note that it stores index in array, not Action Point number.
 * Action Point number doesn't need type, it will probably be replaced by a string. */
typedef long ActionPointId;
/** Parameter for filtering functions which return an item with max filter parameter. */
typedef long FilterParam;
/** Type which stores IAvail_* values. */
typedef char ItemAvailability;
/** Type which stores types of damage as DmgT_* values. */
typedef unsigned char DamageType;
/** Type which stores hit filters for things as THit_* values. */
typedef unsigned char ThingHitType;
/** Type which stores hit filters for things as HitTF_* flags. */
typedef unsigned long HitTargetFlags;
/** Index within active_buttons[] array. */
typedef char ActiveButtonID;
/** Type which stores FeST_* values from FrontendMenuStates enumeration. */
typedef short FrontendMenuState;
/** Type which stores digger task type as DigTsk_* values. */
typedef unsigned short SpDiggerTaskType;
/** Flags for tracing route for creature movement. */
typedef unsigned char NaviRouteFlags;

struct Coord2d {
    union {
      unsigned short val;
      struct {
        unsigned char pos;
        unsigned char num;
        } stl;
    } x;
    union {
      unsigned short val;
      struct {
        unsigned char pos;
        unsigned char num;
        } stl;
    } y;
};


struct Coord3d {
    union {
      unsigned short val;
      struct {
        unsigned char pos;
        unsigned char num;
        } stl;
    } x;
    union {
      unsigned short val;
      struct {
        unsigned char pos;
        unsigned char num;
        } stl;
    } y;
    union {
      unsigned short val;
      struct {
        unsigned char pos;
        unsigned char num;
        } stl;
    } z;
};

struct CoordDelta3d {
    union {
      short val;
      struct {
        unsigned char pos;
        char num;
        } stl;
    } x;
    union {
      short val;
      struct {
        unsigned char pos;
        char num;
        } stl;
    } y;
    union {
      short val;
      struct {
        unsigned char pos;
        char num;
        } stl;
    } z;
};

struct Around { // sizeof = 2
  signed char delta_x;
  signed char delta_y;
};

struct AroundLByte {
  signed short delta_x;
  signed short delta_y;
};

#pragma pack()

struct IPOINT_2D {
    int x;
    int y;
};

struct IPOINT_3D {
    int x;
    int y;
    int z;
};

struct UPOINT_2D {
    unsigned int x;
    unsigned int y;
};

struct UPOINT_3D {
    unsigned int x;
    unsigned int y;
    unsigned int z;
};

struct USPOINT_2D {
    unsigned short x;
    unsigned short y;
};

struct IRECT_2D {
    int l;
    int r;
    int t;
    int b;
};

#ifdef __cplusplus
}
#endif
#endif // KEEPFX_GLOBALS_H
