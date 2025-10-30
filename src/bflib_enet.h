/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/**
 * @author   KeeperFX Team
 * @date     18 Oct 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef GIT_BFLIB_ENET_H
#define GIT_BFLIB_ENET_H

#ifdef __cplusplus
extern "C" {
#endif

struct NetSP;
struct NetSP* InitEnetSP();
unsigned long LbEnet_GetPing(int id);

#ifdef __cplusplus
}
#endif

#endif //GIT_BFLIB_ENET_H
