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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

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

#if defined(BUILD_DLL)
# define DLLIMPORT __declspec (dllexport)
#else // Not defined BUILD_DLL
# define DLLIMPORT __declspec (dllimport)
#endif

#ifdef __cplusplus
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
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
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
#define ERRORMSG(format,args...) LbErrorLog(format "\n", ## args)
#define WARNMSG(format,args...) LbWarnLog(format "\n", ## args)
#define SYNCMSG(format,args...) LbSyncLog(format "\n", ## args)
#define JUSTMSG(format,args...) LbJustLog(format "\n", ## args)
#define SCRPTMSG(format,args...) LbScriptLog(format "\n", ## args)
#define NETMSG(format,args...) LbNetLog(format "\n", ## args)
#define NOMSG(format,args...)

// Debug function-like macros - for code logging (with function name)
#define ERRORLOG(format,args...) LbErrorLog("%s: " format "\n", __func__ , ## args)
#define WARNLOG(format,args...) LbWarnLog("%s: " format "\n", __func__ , ## args)
#define SYNCLOG(format,args...) LbSyncLog("%s: " format "\n", __func__ , ## args)
#define JUSTLOG(format,args...) LbJustLog("%s: " format "\n", __func__ , ## args)
#define SCRPTLOG(format,args...) LbScriptLog(text_line_number,"%s: " format "\n", __func__ , ## args)
#define SCRPTERRLOG(format,args...) LbErrorLog("%s(line %lu): " format "\n", __func__ , text_line_number, ## args)
#define SCRPTWRNLOG(format,args...) LbWarnLog("%s(line %lu): " format "\n", __func__ , text_line_number, ## args)
#define CONFLOG(format,args...) LbConfigLog(text_line_number,"%s: " format "\n", __func__ , ## args)
#define CONFERRLOG(format,args...) LbErrorLog("%s(line %lu): " format "\n", __func__ , text_line_number, ## args)
#define CONFWRNLOG(format,args...) LbWarnLog("%s(line %lu): " format "\n", __func__ , text_line_number, ## args)
#define NETLOG(format,args...) LbNetLog("%s: " format "\n", __func__ , ## args)
#define NOLOG(format,args...)

// Debug function-like macros - for debug code logging
#if (BFDEBUG_LEVEL > 0)
  #define SYNCDBG(dblv,format,args...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbSyncLog("%s: " format "\n", __func__ , ## args); }
  #define WARNDBG(dblv,format,args...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbWarnLog("%s: " format "\n", __func__ , ## args); }
  #define ERRORDBG(dblv,format,args...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbErrorLog("%s: " format "\n", __func__ , ## args); }
  #define SCRIPTDBG(dblv,format,args...) {\
    if (BFDEBUG_LEVEL > dblv)\
      LbScriptLog(text_line_number,"%s: " format "\n", __func__ , ## args); }
  #define NETDBG(dblv,format,args...) {\
	if (BFDEBUG_LEVEL > dblv)\
	  LbNetLog("%s: " format "\n", __func__ , ## args); }
#else
  #define SYNCDBG(dblv,format,args...)
  #define WARNDBG(dblv,format,args...)
  #define ERRORDBG(dblv,format,args...)
  #define SCRIPTDBG(dblv,format,args...)
  #define NETDBG(dblv,format,args...)
#endif

#ifdef __cplusplus
#pragma pack(1)
#endif

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

struct ARect {
  long left;
  long top;
  long right;
  long bottom;
};

struct Around { // sizeof = 2
  signed char delta_x;
  signed char delta_y;
};

struct AroundLByte {
  signed short delta_x;
  signed short delta_y;
};

typedef long LevelNumber;
typedef long MapCoord;
typedef long MapSubtlCoord;
typedef short MapSlabCoord;
typedef unsigned char PlayerFlags;

#ifdef __cplusplus
#pragma pack()
#endif

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
