/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_serde.h
 *     Header file for thing_serde.c.
 * @par Purpose:
 *     Serialize and deserialize things
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Sim
 * @date     08 Sep 2020 - 08 Sep 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_THING_SERDE_H
#define DK_THING_SERDE_H

#include "globals.h"

/* Process things on server BEFORE syncing */
void serde_srv_things();
/* Process things on client BEFORE syncing */
void serde_cli_things();

/* Deserialize things on client and on server AFTER syncing*/
void serde_fin_things();

void serde_dump_thing(long thing);
#endif