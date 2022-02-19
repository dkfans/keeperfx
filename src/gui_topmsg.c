/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_topmsg.c
 *     GUI Messages at screen top functions.
 * @par Purpose:
 *     Functions to create and display messages drawn directly on screen top.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     14 May 2010 - 21 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_topmsg.h"

#include <stdarg.h>
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_video.h"
#include "bflib_sprfnt.h"
#include "game_merge.h"
#include "game_legacy.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char onscreen_msg_text[255]="";
int onscreen_msg_turns = 0;

struct ErrorStatistics erstat[] = {
    {0, 0, "Out of thing slots"},
    {0, 0, "Out of creatures"},
    {0, 0, "Out of path triangles"},
    {0, 0, "Out of room slots"},
    {0, 0, "Wrong creature state"},
    {0, 0, "Out of path points"},
    {0, 0, "Path heap failure"},
    {0, 0, "Route tree failure"},
    {0, 0, "Cannot read packet from file"},
};
int last_checked_stat_num = 0;
/******************************************************************************/
void erstats_clear(void)
{
    for (int stat_num = 0; stat_num < sizeof(erstat) / sizeof(erstat[0]); stat_num++)
    {
        erstat[stat_num].n = 0;
        erstat[stat_num].nprv = 0;
    }
    last_checked_stat_num = 0;
}

long erstat_inc(int stat_num)
{
    if ((stat_num >= 0) && (stat_num < sizeof(erstat)/sizeof(erstat[0])))
        erstat[stat_num].n++;
    return (erstat[stat_num].n-erstat[stat_num].nprv);
}

TbBool show_onscreen_msg_va(int nturns, const char *fmt_str, va_list arg)
{
    vsprintf(onscreen_msg_text, fmt_str, arg);
    SYNCMSG("Onscreen message: %s",onscreen_msg_text);
    onscreen_msg_turns = nturns;
    return true;
}

TbBool is_onscreen_msg_visible(void)
{
    return (onscreen_msg_turns > 0);
}

TbBool show_onscreen_msg(int nturns, const char *fmt_str, ...)
{
    va_list val;
    va_start(val, fmt_str);
    short result = show_onscreen_msg_va(nturns, fmt_str, val);
    va_end(val);
    return result;
}

TbBool erstat_check(void)
{
    // Don't check more often than every 7 turns
    if ((game.play_gameturn & 0x07) != 0)
        return false;
    int stat_num = last_checked_stat_num;
    int sdiff = erstat[stat_num].n - erstat[stat_num].nprv;
    // Display an error if any things were not created in this game turn
    if (sdiff != 0)
    {
#if (BFDEBUG_LEVEL > 0)
        show_onscreen_msg(game.num_fps,"%s, %ld occurrences",erstat[stat_num].msg,sdiff);
#else
        WARNLOG("%s, %ld occurrences",erstat[stat_num].msg,sdiff);
#endif
        erstat[stat_num].nprv = erstat[stat_num].n;
    }
    last_checked_stat_num = (last_checked_stat_num+1) % (sizeof(erstat)/sizeof(erstat[0]));
    return (sdiff != 0);
}

/**
 * Draws the crucial warning messages on screen.
 * Requires the screen to be locked before.
 */
TbBool draw_onscreen_direct_messages(void)
{
    SYNCDBG(5,"Starting");
    int tx_units_per_px = (22 * units_per_pixel) / LbTextLineHeight();
    // Display in-game message for debug purposes
    if ((onscreen_msg_turns > 0) || erstat_check())
    {
        if ( LbScreenIsLocked() )
      LbTextDrawResized(260*units_per_pixel/16, 0*units_per_pixel/16, tx_units_per_px, onscreen_msg_text);
        onscreen_msg_turns--;
    }
    unsigned int msg_pos = 200;
    if ((game.system_flags & GSF_NetGameNoSync) != 0)
    {
        ERRORLOG("OUT OF SYNC (GameTurn %7d)", game.play_gameturn);
        if ( LbScreenIsLocked() )
          LbTextDrawResized(260*units_per_pixel/16, msg_pos*units_per_pixel/16, tx_units_per_px, "OUT OF SYNC");
        msg_pos += 20;
    }
    if ((game.system_flags & GSF_NetSeedNoSync) != 0)
    {
        ERRORLOG("SEED OUT OF SYNC (GameTurn %7d)", game.play_gameturn);
        if ( LbScreenIsLocked() )
          LbTextDrawResized(260*units_per_pixel/16, msg_pos*units_per_pixel/16, tx_units_per_px, "SEED OUT OF SYNC");
        msg_pos += 20;
    }
    SYNCDBG(18,"Finished");
    return true;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
