/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_doors.h
 *     Header file for thing_doors.c.
 * @par Purpose:
 *     XXXX functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 12 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_TNGDOORS_H
#define DK_TNGDOORS_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct DoorStats { // sizeof = 8
  unsigned short field_0;
  unsigned char field_2[2];
  unsigned char field_4[2];
  unsigned short field_6;
};

#pragma pack()
/******************************************************************************/
void lock_door(struct Thing *thing);
void unlock_door(struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
