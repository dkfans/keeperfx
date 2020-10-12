/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file version.h
 *     Project name, version, copyrights and debug level definitions.
 * @par Purpose:
 *     Header file for global names and defines used by resource compiler.
 * @par Comment:
 *     Can only contain commands which resource compiler can understand.
 * @author   Tomasz Lis
 * @date     08 Jan 2010 - 23 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef KEEPERFX_VERSION_H
#define KEEPERFX_VERSION_H

#ifndef BFDEBUG_LEVEL
/* Debug level is scaled 0..10, usually defined in Makefile. */
#define BFDEBUG_LEVEL 10
#endif
#ifndef DEBUG_NETWORK_PACKETS
/* Network packets debugging. */
#define DEBUG_NETWORK_PACKETS 0
#endif
/* Version definitions */
#include "../obj/ver_defs.h"
//#define VER_MAJOR         1
//#define VER_MINOR         2
//#define VER_RELEASE       3
//#define VER_BUILD         4
//#define VER_STRING        "1.2.3.4"
/* Program name, copyrights and file names */
#define PROGRAM_NAME      "Dungeon Keeper FX"
#define PROGRAM_FULL_NAME "KeeperFX"
#define COMPANY_NAME      "Fan community"
#define INTERNAL_NAME     "keeperfx"
#define LEGAL_COPYRIGHT   "Open Source"
#define LEGAL_TRADEMARKS  "DK is a trademark of Electronic Arts"
#define FILE_VERSION VER_STRING
#define FILE_DESCRIPTION PROGRAM_NAME
#define ORIGINAL_FILENAME INTERNAL_NAME".exe"
#define PRODUCT_NAME PROGRAM_FULL_NAME
#define PRODUCT_VERSION    VER_STRING
#define DEFAULT_LOG_FILENAME INTERNAL_NAME".log"

#endif /*KEEPERFX_VERSION_H*/
/******************************************************************************/
