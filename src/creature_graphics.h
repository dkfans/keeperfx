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
#define CREATURE_GRAPHICS_INSTANCES     22

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
    CGI_Celebrate   = 11,
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
#pragma pack(1)

struct Thing;

struct CreaturePickedUpOffset // sizeof = 8
{
  short delta_x;
  short delta_y;
  short field_4;
  short field_6;
};

/**
 * Enhanced TbSprite structure, with additional fields for thing animation sprites.
 */
struct KeeperSprite { // sizeof = 16
  unsigned long DataOffset;
#ifdef SPRITE_FORMAT_V2
  unsigned short SWidth;
  unsigned short SHeight;
  unsigned short FrameWidth;
  unsigned short FrameHeight;
  unsigned char Rotable;
  unsigned char FramesCount;
  unsigned short FrameOffsW;
  unsigned short FrameOffsH;
#else
  unsigned char SWidth;
  unsigned char SHeight;
  unsigned char FrameWidth;
  unsigned char FrameHeight;
  unsigned char Rotable;
  unsigned char FramesCount;
  unsigned char FrameOffsW;
  unsigned char FrameOffsH;
#endif
  short field_C; // Offset x
  short field_E; // Offset y
};

struct KeeperSpriteExt // More info for custom sprites
{
    unsigned char rotation; // Used to implement rotated statues from rotatable
};
/******************************************************************************/
//extern unsigned short creature_graphics[][22];
//extern struct KeeperSprite *creature_table;
extern struct KeeperSprite creature_table_add[];
extern struct KeeperSpriteExt creatures_table_ext[];
/******************************************************************************/
DLLIMPORT struct KeeperSprite *_DK_creature_table;
#define creature_table _DK_creature_table
/******************************************************************************/

#pragma pack()
/******************************************************************************/
struct CreaturePickedUpOffset *get_creature_picked_up_offset(struct Thing *thing);

unsigned long keepersprite_index(unsigned short n);
struct KeeperSprite * keepersprite_array(unsigned short n);
unsigned char keepersprite_frames(unsigned short n);
unsigned char keepersprite_rotable(unsigned short n);
void get_keepsprite_unscaled_dimensions(long kspr_frame, long a2, long a3, short *orig_w, short *orig_h, short *unsc_w, short *unsc_h);
long get_lifespan_of_animation(long ani, long frameskip);
short get_creature_anim(struct Thing *thing, unsigned short frame);
short get_creature_model_graphics(long crmodel, unsigned short frame);
void set_creature_model_graphics(long crmodel, unsigned short frame, unsigned long val);
void set_creature_graphic(struct Thing *thing);
void update_creature_graphic_field_4F(struct Thing *thing);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
