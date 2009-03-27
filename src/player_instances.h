/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_instances.h
 *     Header file for player_instances.c.
 * @par Purpose:
 *     Player instances support and switching code.
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

#ifndef DK_PLYR_INSTNC_H
#define DK_PLYR_INSTNC_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

typedef long (*InstncInfo_Func)(struct PlayerInfo *player, long *n);

struct PlayerInstanceInfo { // sizeof = 44
  unsigned long field_0;
  unsigned long field_4;
  InstncInfo_Func start_cb;
  InstncInfo_Func maintain_cb;
  InstncInfo_Func end_cb;
  long field_14[2];
  unsigned char field_1C[8];
  long field_24;
  unsigned long field_28;
};

#pragma pack()
/******************************************************************************/
/******************************************************************************/

void set_player_instance(struct PlayerInfo *player, long ninum, short force);
void process_player_instance(struct PlayerInfo *player);
void process_player_instances(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
