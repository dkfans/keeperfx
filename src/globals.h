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
#elif defined(MSDOS)
#include <dos.h>
#include <process.h>
#endif

#if defined(BUILD_DLL)
# define DLLIMPORT __declspec (dllexport)
#else // Not defined BUILD_DLL
# define DLLIMPORT __declspec (dllimport)
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

// Debug level is scaled 0..10, default=1
#define BFDEBUG_LEVEL 10
#define PROGRAM_NAME "Dungeon Keeper FX"
#define PROGRAM_FULL_NAME "Dungeon Keeper Fan eXpansion"

// Return values for verification functions
#define VERIF_ERROR   0
#define VERIF_OK      1
#define VERIF_WARN    2

// Return values for all other functions
#define ERR_NONE           0
// Note: error codes -1..-79 are reserved standard C library errors with sign reverted.
//    these are defined in errno.h
#define ERR_BASE_RNC      -90

#pragma pack(1)

#ifdef __cplusplus
#pragma pack(1)
#endif

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

#pragma pack()

#endif // KEEPFX_GLOBALS_H
