/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_traps.h
 *     Header file for thing_traps.c.
 * @par Purpose:
 *     Traps support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THING_TRAPS_H
#define DK_THING_TRAPS_H

#include "bflib_basics.h"
#include "globals.h"
#include "room_workshop.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;

struct TrapData {
      long field_0;
      long field_4;
      short field_8;
      short field_A;
      unsigned short name_stridx;
      short field_E;
};

struct TrapStats {  // sizeof=54
unsigned long field_0;
unsigned long field_4;
unsigned long field_8;
unsigned char field_C[6];
  unsigned char field_12;
unsigned char field_13[7];
unsigned char field_1A[6];
unsigned char field_20[8];
unsigned char field_28[8];
unsigned char field_30[6];
};


#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT extern struct TrapData _DK_trap_data[MANUFCTR_TYPES_COUNT];
#define trap_data _DK_trap_data
DLLIMPORT extern unsigned char _DK_trap_to_object[8];
#define trap_to_object _DK_trap_to_object
DLLIMPORT struct TrapStats _DK_trap_stats[7]; //not sure - maybe it's 8?
#define trap_stats _DK_trap_stats
/******************************************************************************/
TbBool destroy_trap(struct Thing *thing);
struct Thing *create_trap(struct Coord3d *pos, unsigned short a1, unsigned short a2);
struct Thing *get_trap_for_position(long pos_x, long pos_y);
struct Thing *get_trap_for_slab_position(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool trap_is_active(const struct Thing *thing);
TbBool trap_is_slappable(const struct Thing *thing, long plyr_idx);
long update_trap(struct Thing *thing);
void init_traps(void);


/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
