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

#include "bflib_basics.h"
#include <stdbool.h> // Introduced in C99. Provides true/false.
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h> // Provides NULL.
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

#ifdef _MSC_VER
    #define strcasecmp _stricmp
    #define strncasecmp _strnicmp
#endif

#include "version.h"

#ifndef BFDEBUG_LEVEL
#error "BFDEBUG_LEVEL should be defined in version.h"
#define BFDEBUG_LEVEL 0
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

#ifndef __cplusplus
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef clamp
#define clamp(v,lo,hi) (max(lo, min(v, hi)))
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
#define ERRORLOG(format, ...) LbErrorLog("[%lu] %s: " format "\n", get_gameturn(), __func__ , ##__VA_ARGS__)
#define WARNLOG(format, ...) LbWarnLog("[%lu] %s: " format "\n", get_gameturn(), __func__ , ##__VA_ARGS__)
#define SYNCLOG(format, ...) LbSyncLog("[%lu] %s: " format "\n", get_gameturn(), __func__ , ##__VA_ARGS__)
#define JUSTLOG(format, ...) LbJustLog("[%lu] %s: " format "\n", get_gameturn(), __func__ , ##__VA_ARGS__)
#define SCRPTLOG(format, ...) LbScriptLog(text_line_number,"%s: " format "\n", __func__ , ##__VA_ARGS__)
#define SCRPTERRLOG(format, ...) LbErrorLog("%s(line %lu): " format "\n", __func__ , text_line_number, ##__VA_ARGS__)
#define SCRPTWRNLOG(format, ...) LbWarnLog("%s(line %lu): " format "\n", __func__ , text_line_number, ##__VA_ARGS__)
#define CONFLOG(format, ...) LbConfigLog(text_line_number,"%s: " format "\n", __func__ , ##__VA_ARGS__)
#define CONFERRLOG(format, ...) LbErrorLog("%s(line %lu): " format "\n", __func__ , text_line_number, ##__VA_ARGS__)
#define CONFWRNLOG(format, ...) LbWarnLog("%s(line %lu): " format "\n", __func__ , text_line_number, ##__VA_ARGS__)
#define NETLOG(format, ...) LbNetLog("[%lu] %s: " format "\n", get_gameturn(), __func__ , ##__VA_ARGS__)
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
#else
  #define SYNCDBG(dblv,format, ...)
  #define WARNDBG(dblv,format, ...)
  #define ERRORDBG(dblv,format, ...)
  #define NAVIDBG(dblv,format, ...)
  #define NETDBG(dblv,format, ...)
  #define SCRIPTDBG(dblv,format, ...)
#endif

#define MAX_TILES_X 170
#define MAX_TILES_Y 170
#define MAX_SUBTILES_X 511
#define MAX_SUBTILES_Y 511

enum AnglesAndDegrees {
    // Cardinal directions (clockwise from North)
    ANGLE_NORTH = 0,        // 0° - North direction (up)
    ANGLE_NORTHEAST = 256,  // 45° - Northeast direction (up-right)
    ANGLE_EAST = 512,       // 90° - East direction (right)
    ANGLE_SOUTHEAST = 768,  // 135° - Southeast direction (down-right)
    ANGLE_SOUTH = 1024,     // 180° - South direction (down)
    ANGLE_SOUTHWEST = 1280, // 225° - Southwest direction (down-left)
    ANGLE_WEST = 1536,      // 270° - West direction (left)
    ANGLE_NORTHWEST = 1792, // 315° - Northwest direction (up-left)
    ANGLE_MASK = 2047,      // Bitmask for angle/degrees values (0x7FF)
    // Degrees
    DEGREES_2_8125 = 16,    // 2.8125° - DEGREES_180 / 64
    DEGREES_8_18 = 46,      // 8.18° - DEGREES_180 / 22
    DEGREES_10 = 56,        // 10° - DEGREES_180 / 18
    DEGREES_11_25 = 64,     // 11.25° - DEGREES_180 / 16
    DEGREES_15 = 85,        // 15° - DEGREES_180 / 12
    DEGREES_20 = 113,       // 20° - DEGREES_180 / 9
    DEGREES_22_5 = 128,     // 22.5° - DEGREES_180 / 8
    DEGREES_30 = 170,       // 30° - DEGREES_180 / 6
    DEGREES_45 = 256,       // 45° - DEGREES_180 / 4
    DEGREES_50 = 284,       // 50°
    DEGREES_60 = 341,       // 60° - DEGREES_180 / 3
    DEGREES_90 = 512,       // 90° - DEGREES_180 / 2
    DEGREES_120 = 682,      // 120° - 2 * DEGREES_180 / 3
    DEGREES_135 = 768,      // 135°
    DEGREES_180 = 1024,     // 180° - Half a circle
    DEGREES_202_5 = 1151,   // 202.5° - Sprite flip threshold
    DEGREES_225 = 1280,     // 225°
    DEGREES_270 = 1536,     // 270°
    DEGREES_315 = 1792,     // 315°
    DEGREES_337_5 = 1919,   // 337.5° - Sprite flip threshold
    DEGREES_360 = 2048,     // 360° - Full circle
};

#pragma pack(1)

/** Screen coordinate in scale of the game (resolution independent). */
typedef int ScreenCoord;
/** Screen coordinate in scale of the real screen. */
typedef int RealScreenCoord;
/** Player identification number, or owner of in-game thing/room/slab. */
typedef int8_t PlayerNumber;
/** bitflags where each bit represents a player (e.g. player id 0 = 0b000001, player id 1 = 0b000010, player id 2 = 0b000100). */
typedef unsigned short PlayerBitFlags;
/** Type which stores thing class. */
typedef unsigned char ThingClass;
/** Type which stores thing model. */
typedef short ThingModel;
/** Type which stores thing index. */
typedef unsigned short ThingIndex;
/** Type which stores effectModels on positive or EffectElements on Negative. Should be as big as ThingModel */
typedef short EffectOrEffElModel;
/** Type which stores creature state index. */
typedef unsigned short CrtrStateId;
/** Type which stores creature experience level. */
typedef unsigned char CrtrExpLevel;
/** Type which stores keeper power level. */
typedef unsigned char KeepPwrLevel;
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
/** Type which stores spell kind index. */
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
typedef int32_t MapSubtlCoord;
/** Distance between map subtiles. */
typedef long MapSubtlDelta;
/** Map slab coordinate. Slab is a cubic part of map with specific content. */
typedef short MapSlabCoord;
/** Distance between map coordinates in slabs.  */
typedef short MapSlabDelta;
/** Map subtile 2D coordinates, coded into one number. */
typedef long SubtlCodedCoords;
/** Map slab 2D coordinates, coded into one number. */
typedef unsigned long SlabCodedCoords;
/** Index in the columns array. */
typedef short ColumnIndex;
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
/** Type which stores hit filters for things as THit_* values. */
typedef unsigned char ThingHitType;
/** Type which stores hit filters for things as HitTF_* flags. */
typedef unsigned long long HitTargetFlags;
/** Index within active_buttons[] array. */
typedef char ActiveButtonID;
/** Type which stores FeST_* values from FrontendMenuStates enumeration. */
typedef short FrontendMenuState;
/** Type which stores digger task type as DigTsk_* values. */
typedef unsigned short SpDiggerTaskType;
/** Flags for tracing route for creature movement. */
typedef unsigned char NaviRouteFlags;
/** data used for navigating contains floor height, locked doors per player, unsafe surfaces */
typedef unsigned short NavColour;
/** Either North (0), East (1), South (2), or West (3). */
typedef int8_t SmallAroundIndex;
/** a player state as defined in config_players*/
typedef unsigned char PlayerState;
typedef unsigned short CctrlIndex;
/** index to a function, positive for C functions, negative for lua functions*/
typedef short FuncIdx;
typedef unsigned long TbMapLocation;


/**
 * Stores a 2d coordinate (x,y).
 *
 * Members:
 * .val - coord position (relative to whole map)
 * .stl.pos - coord position (relative to subtile)
 * .stl.num - subtile position (relative to whole map)
 */
struct Coord2d {
    union { // x position
      unsigned long val; /**< x.val - coord x position (relative to whole map) */
      struct { // subtile
        unsigned char pos; /**< x.stl.pos - coord x position (relative to subtile) */
        unsigned short num; /**< x.stl.num - subtile x position (relative to whole map) */
        } stl;
    } x;
    union { // y position
      unsigned long val; /**< y.val - coord y position (relative to whole map) */
      struct { // subtile
        unsigned char pos; /**< y.stl.pos - coord y position (relative to subtile) */
        unsigned short num; /**< y.stl.num - subtile y position (relative to whole map) */
        } stl;
    } y;
};

/**
 * Stores a 3d coordinate (x,y).
 *
 * Members:
 * .val - coord position (relative to whole map)
 * .stl.pos - coord position (relative to subtile)
 * .stl.num - subtile position (relative to whole map)
 */
struct Coord3d {
    union { // x position
      long val; /**< x.val - coord x position (relative to whole map) */
      struct { // subtile
        unsigned char pos; /**< x.stl.pos - coord x position (relative to subtile) */
        unsigned short num; /**< x.stl.num - subtile x position (relative to whole map) */
        } stl;
    } x;
    union { // y position
      long val; /**< y.val - coord y position (relative to whole map) */
      struct { // subtile
        unsigned char pos; // y.stl.pos - coord y position (relative to subtile) */
        unsigned short num; // y.stl.num - subtile y position (relative to whole map) */
        } stl;
    } y;
    union { // z position
      long val; /**< z.val - coord z position (relative to whole map) */
      struct { // subtile
        unsigned char pos; /**< z.stl.pos - coord z position (relative to subtile) */
        unsigned short num; /**< z.stl.num - subtile z position (relative to whole map) */
        } stl;
    } z;
};

struct CoordDelta3d {
    union {
      long val;
      struct {
        unsigned char pos;
        short num;
        } stl;
    } x;
    union {
      long val;
      struct {
        unsigned char pos;
        short num;
        } stl;
    } y;
    union {
      long val;
      struct {
        unsigned char pos;
        short num;
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

struct PickedUpOffset
{
    short delta_x;
    short delta_y;
};

extern GameTurn get_gameturn();
#ifdef __cplusplus
}
#endif
#endif // KEEPFX_GLOBALS_H
