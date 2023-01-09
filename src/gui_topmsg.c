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
#include "pre_inc.h"
#include "gui_topmsg.h"

#include <stdarg.h>
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_video.h"
#include "bflib_sprfnt.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char onscreen_msg_text[255]="";
float render_onscreen_msg_time;

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
    render_onscreen_msg_time = (float)nturns;
    return true;
}

TbBool is_onscreen_msg_visible(void)
{
    return (render_onscreen_msg_time > 0.0);
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
    JUSTMSG("testlog: crash at erstat_check position 1");
    // Don't check more often than every 7 turns
    if ((game.play_gameturn & 0x07) != 0)
        return false;
    int stat_num = last_checked_stat_num;
    JUSTMSG("testlog stat_num = %d", stat_num);
    long a = erstat[stat_num].n;
    long b = erstat[stat_num].nprv;

    JUSTMSG("Testlog a = %d, b = %d, stat_num = %d", a, b, stat_num);
    JUSTMSG("testlog a-b = %d", a - b);
    int sdiff = erstat[stat_num].n - erstat[stat_num].nprv;
    // Display an error if any things were not created in this game turn
    JUSTMSG("testlog: crash at position 2");
    if (sdiff != 0)
    {
#if (BFDEBUG_LEVEL > 0)
        JUSTMSG("testlog: crash at position 3");
        show_onscreen_msg(game.num_fps,"%s, %ld occurrences",erstat[stat_num].msg,sdiff);
        JUSTMSG("testlog: crash at position 4");
#else
        JUSTMSG("testlog: crash at position 5");
        WARNLOG("%s, %ld occurrences",erstat[stat_num].msg,sdiff);
#endif
        JUSTMSG("testlog: crash at position 6");
        erstat[stat_num].nprv = erstat[stat_num].n;
    }
    JUSTMSG("testlog: crash at position 7");
    last_checked_stat_num = (last_checked_stat_num+1) % (sizeof(erstat)/sizeof(erstat[0]));
    JUSTMSG("testlog: return from erstat_check");
    return (sdiff != 0);
}

/**
 * Draws the crucial warning messages on screen.
 * Requires the screen to be locked before.
 */
TbBool draw_onscreen_direct_messages(void)
{
    SYNCDBG(5,"Starting");
    int tx_units_per_px;
    if (dbc_language > 0)
    {
        SYNCDBG(5, " place 1");
        tx_units_per_px = scale_value_by_horizontal_resolution((MyScreenWidth >= 640) ? 16 : 32);
    }
    else
    {
        SYNCDBG(5, " place 2");
        tx_units_per_px = scale_ui_value_lofi(16);
    }
    // Display in-game message for debug purposes
    SYNCDBG(5, " place 3");

    if (erstat_check())
    {
        SYNCDBG(5, " place 4");
        if (LbScreenIsLocked())
        {
            SYNCDBG(5, " place 5");
            LbTextDrawResized(scale_value_by_horizontal_resolution(160), 0, tx_units_per_px, onscreen_msg_text);
        }
        SYNCDBG(5, " place 6");
        render_onscreen_msg_time -= gameadd.delta_time;
    }
    SYNCDBG(5, " it's not erstat check!");
    if ((render_onscreen_msg_time > 0.0)) 
    {
        SYNCDBG(5, " place 4b");
        if (LbScreenIsLocked())
        {
            SYNCDBG(5, " place 5b");
            LbTextDrawResized(scale_value_by_horizontal_resolution(160), 0, tx_units_per_px, onscreen_msg_text);
        }
        SYNCDBG(5, " place 6b");
        render_onscreen_msg_time -= gameadd.delta_time;
    }

    SYNCDBG(5, " place 7");
    unsigned int msg_pos = scale_value_by_vertical_resolution(200);
    SYNCDBG(5, " place 8");
    if ((game.system_flags & GSF_NetGameNoSync) != 0)
    {
        SYNCDBG(5, " place 9");
        ERRORLOG("OUT OF SYNC (GameTurn %7d)", game.play_gameturn);
        if (LbScreenIsLocked())
        {
            SYNCDBG(5, " place 10");
            LbTextDrawResized(scale_value_by_horizontal_resolution(260), scale_value_by_vertical_resolution(msg_pos), tx_units_per_px, "OUT OF SYNC");
        }
        SYNCDBG(5, " place 11");
        msg_pos += scale_value_by_horizontal_resolution(20);
    }
    SYNCDBG(5, " place 12");
    if ((game.system_flags & GSF_NetSeedNoSync) != 0)
    {
        SYNCDBG(5, " place 13");
        ERRORLOG("SEED OUT OF SYNC (GameTurn %7d)", game.play_gameturn);
        if (LbScreenIsLocked())
        {
            SYNCDBG(5, " place 14");
            LbTextDrawResized(scale_value_by_horizontal_resolution(260), scale_value_by_vertical_resolution(msg_pos), tx_units_per_px, "SEED OUT OF SYNC");
        }
        SYNCDBG(5, " place 15");
        msg_pos += scale_value_by_vertical_resolution(20);
        SYNCDBG(5, " place 16");
    }
    SYNCDBG(5, " place 17");
    SYNCDBG(18,"Finished");
    return true;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
