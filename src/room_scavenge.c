/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_scavenge.c
 *     Scavenger room maintain functions.
 * @par Purpose:
 *     Functions to create and use scavenger rooms.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 19 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "room_scavenge.h"

#include "globals.h"
#include "bflib_basics.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_effects.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
long const scavenge_effect_element[] = {TngEff_BallPuffRed, TngEff_BallPuffBlue, TngEff_BallPuffGreen, TngEff_BallPuffYellow, TngEff_BallPuffWhite, TngEff_BallPuffWhite,};
/******************************************************************************/
long get_scavenge_effect_element(PlayerNumber owner)
{
  return scavenge_effect_element[player_colors_map[owner % PLAYERS_EXT_COUNT]];
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
