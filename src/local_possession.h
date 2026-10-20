/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file local_possession.h
 *     Header file for local_possession.c.
 * @par Purpose:
 *     Client-side prediction of the local player's possessed creature.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     20 Oct 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LOCAL_POSSESSION_H
#define DK_LOCAL_POSSESSION_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct Thing;
struct Packet;
struct CreatureControl;
struct Coord3d;

/******************************************************************************/
void update_local_possession(void);
struct Thing *get_local_possession_view(struct Thing *thing);
const struct CreatureControl *get_local_creature_control(const struct Thing *thing);
GameTurn get_local_creature_turn(const struct Thing *thing);
TbBool local_possession_look_frozen(struct Thing *thing);
TbBool local_possession_sounded_shot(const struct Thing *firing);
void offset_local_possession_shot(const struct Thing *shotng, struct Coord3d *pos);
void set_packet_possession_target(struct Packet *pckt);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
