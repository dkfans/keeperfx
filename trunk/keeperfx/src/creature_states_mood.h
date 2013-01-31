/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states_mood.h
 *     Header file for creature_states_mood.c.
 * @par Purpose:
 *     Creature state machine functions related to their mood.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     23 Sep 2009 - 05 Jan 2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRSTATEMOOD_H
#define DK_CRTRSTATEMOOD_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

struct Thing;

#pragma pack()
/******************************************************************************/
TbBool anger_is_creature_livid(const struct Thing *thing);
TbBool anger_is_creature_angry(const struct Thing *thing);
AnnoyMotive anger_get_creature_anger_type(const struct Thing *creatng);
void anger_set_creature_anger(struct Thing *thing, long annoy_lv, AnnoyMotive reason);
TbBool anger_make_creature_angry(struct Thing *thing, AnnoyMotive reason);

short creature_moan(struct Thing *thing);
short creature_roar(struct Thing *thing);
short creature_be_happy(struct Thing *thing);
short creature_piss(struct Thing *thing);
short mad_killing_psycho(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
