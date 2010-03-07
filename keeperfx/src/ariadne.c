/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file ariadne.c
 *     Dungeon routing and path finding system.
 * @par Purpose:
 *     Defines functions for finding creature routes and navigating
 *     through the dungeon.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     10 Jan 2010 - 20 Feb 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "ariadne.h"

#include "globals.h"
#include "bflib_basics.h"
#include "keeperfx.hpp"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
DLLIMPORT AriadneReturn _DK_ariadne_initialise_creature_route(struct Thing *thing, struct Coord3d *pos, long a3, unsigned char a4);
DLLIMPORT AriadneReturn _DK_creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, long a4, unsigned char a5);
DLLIMPORT void _DK_path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
AriadneReturn ariadne_initialise_creature_route(struct Thing *thing, struct Coord3d *pos, long a3, unsigned char a4)
{
    return _DK_ariadne_initialise_creature_route(thing, pos, a3, a4);
}

AriadneReturn creature_follow_route_to_using_gates(struct Thing *thing, struct Coord3d *pos1, struct Coord3d *pos2, long a4, unsigned char a5)
{
    SYNCDBG(18,"Starting");
    return _DK_creature_follow_route_to_using_gates(thing, pos1, pos2, a4, a5);
}

void path_init8_wide(struct Path *path, long start_x, long start_y, long end_x, long end_y, long a6, unsigned char nav_size)
{
    _DK_path_init8_wide(path, start_x, start_y, end_x, end_y, a6, nav_size);
}

/******************************************************************************/
