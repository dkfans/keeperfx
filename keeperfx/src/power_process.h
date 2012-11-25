/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_process.h
 *     Header file for power_process.c.
 * @par Purpose:
 *     Keeper powers process functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 21 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_POWERPROCESS_H
#define DK_POWERPROCESS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;
struct PlayerInfo;

struct Armageddon { // sizeof = 14
  unsigned long count_down;
  unsigned long duration;
  struct Coord3d mappos;
};

#pragma pack()
/******************************************************************************/
void process_disease(struct Thing *thing);
void process_armageddon(void);

void update_god_lightning_ball(struct Thing *thing);
void god_lightning_choose_next_creature(struct Thing *thing);
void draw_god_lightning(struct Thing *thing);

void update_explored_flags_for_power_sight(struct PlayerInfo *player);
void remove_explored_flags_for_power_sight(struct PlayerInfo *player);

unsigned char general_expand_check(void);
unsigned char sight_of_evil_expand_check(void);
unsigned char call_to_arms_expand_check(void);

void turn_off_call_to_arms(long a);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
