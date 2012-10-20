/******************************************************************************/
/** @file globals.h
 * Header file for global definitions.
 * @par Purpose:
 *     Defines basic includes and definitions, used in whole program.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @author   Jon Skeet
 * @date     14 Oct 1997 - 08 Sep 1998
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef BULL_GLOBALS_H
#define BULL_GLOBALS_H

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

// Return values for all other functions
#define ERR_NONE           0
// Note: error codes -1..-79 are reserved standard C library errors with sign reverted.
//    these are defined in errno.h
#define ERR_BASE_RNC      -90

#endif // BULL_GLOBALS_H
