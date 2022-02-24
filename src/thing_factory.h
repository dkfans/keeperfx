/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_factory.h
 *     Header file for thing_factory.c.
 * @par Purpose:
 *     Things creation unified functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Mar 2009 - 22 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_TNGFACTORY_H
#define DK_TNGFACTORY_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
#pragma pack(1)

struct InitThing { // sizeof=0x15
    struct Coord3d mappos;
    unsigned char oclass;
    unsigned char model;
    unsigned char owner;
    unsigned short range;
    unsigned short index;
    unsigned char params[8];
};

#pragma pack()
/******************************************************************************/
short thing_create_thing(struct InitThing *itng);
struct Thing *create_thing(struct Coord3d *pos, unsigned short tngclass, unsigned short model, unsigned short owner, long a4);
struct Thing *create_thing_at_position_then_move_to_valid_and_add_light(struct Coord3d *pos, unsigned char tngclass, unsigned char tngmodel, unsigned char tngowner);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
