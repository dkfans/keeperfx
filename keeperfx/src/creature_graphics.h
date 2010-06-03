/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_graphics.h
 *     Header file for creature_graphics.c.
 * @par Purpose:
 *     Creature graphics support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 23 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRGRAPHICS_H
#define DK_CRTRGRAPHICS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

// note - this is temporary value; not correct
#define CREATURE_FRAMELIST_LENGTH     982

enum CreatureGraphicsInstances {
    CGI_Stand       =  0,
    CGI_Ambulate    =  1,
    CGI_Drag        =  2,
    CGI_Attack      =  3,
    CGI_Dig         =  4,
    CGI_Smoke       =  5,
    CGI_Relax       =  6,
    CGI_PrettyDance =  7,
    CGI_GotHit      =  8,
    CGI_PowerGrab   =  9,
    CGI_GotSlapped  = 10,
    CGI_WaveHands   = 11,
    CGI_Sleep       = 12,
    CGI_EatChicken  = 13,
    CGI_Torture     = 14,
    CGI_Scream      = 15,
    CGI_DropDead    = 16,
    CGI_DeadSplat   = 17,
    CGI_GFX18       = 18,
    CGI_QuerySymbol = 19,
    CGI_HandSymbol  = 20,
    CGI_GFX21       = 21,
};
/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct CreaturePickedUpOffset // sizeof = 8
{
  short delta_x;
  short delta_y;
  short field_4;
  short field_6;
};

struct KeeperSprite { // sizeof = 16
  unsigned char field_0[9];
  unsigned char field_9;
  unsigned char field_A[4];
  unsigned char field_E[2];
};

/******************************************************************************/
//extern unsigned short creature_graphics[][22];
/******************************************************************************/

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
struct CreaturePickedUpOffset *get_creature_picked_up_offset(struct Thing *thing);

unsigned char keepersprite_frames(unsigned short n);
long get_lifespan_of_animation(long ani, long frameskip);
unsigned long get_creature_anim(struct Thing *thing, unsigned short frame);
unsigned long get_creature_breed_graphics(long breed, unsigned short frame);
void set_creature_breed_graphics(long breed, unsigned short frame, unsigned long val);
void set_creature_graphic(struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
