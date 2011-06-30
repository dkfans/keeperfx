/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_topmsg.h
 *     GUI Messages at screen top functions.
 * @par Purpose:
 *     gui_topmsg functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     14 May 2010 - 21 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_GUI_TOPMSG_H
#define DK_GUI_TOPMSG_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#pragma pack(1)

enum ErrorStatisticEntries {
    ESE_NoFreeThings = 0,
    ESE_NoFreeCreatrs,
    ESE_NoFreeTriangls,
    ESE_NoFreeRooms,
    ESE_BadCreatrState,
    ESE_NoFreePathPts,
    ESE_BadPathHeap,
    ESE_BadRouteTree,
    ESE_CantReadPackets,
};

struct ErrorStatistics {
    unsigned long n;
    unsigned long nprv;
    const char *msg;
};

#pragma pack()
/******************************************************************************/
void erstats_clear(void);
long erstat_inc(int stat_num);

TbBool is_onscreen_msg_visible(void);
TbBool show_onscreen_msg(int nturns, const char *fmt_str, ...);
TbBool draw_onscreen_direct_messages(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
