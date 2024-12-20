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
#define CREATURE_FRAMELIST_LENGTH    982
#define CREATURE_GRAPHICS_INSTANCES   25

enum CreatureGraphicsInstances {
    CGI_Stand        =  0,
    CGI_Ambulate     =  1,
    CGI_Drag         =  2,
    CGI_Attack       =  3,
    CGI_Dig          =  4,
    CGI_Smoke        =  5,
    CGI_Relax        =  6,
    CGI_PrettyDance  =  7,
    CGI_GotHit       =  8,
    CGI_PowerGrab    =  9,
    CGI_GotSlapped   = 10,
    CGI_Celebrate    = 11,
    CGI_Sleep        = 12,
    CGI_EatChicken   = 13,
    CGI_Torture      = 14,
    CGI_Scream       = 15,
    CGI_DropDead     = 16,
    CGI_DeadSplat    = 17,
    CGI_Roar         = 18, // Was previously GFX18.
    CGI_QuerySymbol  = 19, // Icon, not a sprite
    CGI_HandSymbol   = 20, // Icon, not a sprite
    CGI_Piss         = 21, // Was previously GFX21.
    CGI_CastSpell    = 22,
    CGI_RangedAttack = 23,
    CGI_Custom       = 24,
};
/******************************************************************************/
#pragma pack(1)

struct Thing;

struct CreaturePickedUpOffset
{
  short delta_x;
  short delta_y;
};

/**
 * Enhanced TbSprite structure, with additional fields for thing animation sprites.
 */
enum FrameFlags {
    FFL_NoShadows = 1,
};

struct KeeperSprite { // sizeof = 16
  unsigned long DataOffset;

  unsigned short SWidth;
  unsigned short SHeight;
  unsigned short FrameWidth;
  unsigned short FrameHeight;
  unsigned char Rotable;
  unsigned char FramesCount;
  short FrameOffsW;
  short FrameOffsH;

  short offset_x;
  short offset_y;

  short shadow_offset;
  short frame_flags;
};

struct KeeperSpriteDisk {
    unsigned long DataOffset;
    unsigned char SWidth;
    unsigned char SHeight;
    unsigned char FrameWidth;
    unsigned char FrameHeight;
    unsigned char Rotable;
    unsigned char FramesCount;
    unsigned char FrameOffsW;
    unsigned char FrameOffsH;
    short offset_x;
    short offset_y;
};

/******************************************************************************/
//extern unsigned short creature_graphics[][22];
extern struct KeeperSprite *creature_table;
extern struct KeeperSprite creature_table_add[];
/******************************************************************************/

#pragma pack()
/******************************************************************************/
struct CreaturePickedUpOffset *get_creature_picked_up_offset(struct Thing *thing);

unsigned long keepersprite_index(unsigned short n);
struct KeeperSprite * keepersprite_array(unsigned short n);
unsigned char keepersprite_frames(unsigned short n); // This returns number of frames in animation
unsigned char keepersprite_rotable(unsigned short n);
void get_keepsprite_unscaled_dimensions(long kspr_anim, long angle, long frame, short *orig_w, short *orig_h, short *unsc_w, short *unsc_h);
long get_lifespan_of_animation(long ani, long speed);
short get_creature_anim(struct Thing *thing, unsigned short frame);
short get_creature_model_graphics(long crmodel, unsigned short frame);
void set_creature_model_graphics(long crmodel, unsigned short frame, unsigned long val);
void set_creature_graphic(struct Thing *thing);
void update_creature_rendering_flags(struct Thing *thing);

size_t creature_table_load_get_size(size_t disk_size);
void creature_table_load_unpack(unsigned char *src, size_t disk_size);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
