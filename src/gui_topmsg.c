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
    {0, 0, "Cannot create unsynced thing, no free slots - likely too many particles being created"},
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
    const int num_stats = sizeof(erstat) / sizeof(erstat[0]);
    if ((stat_num < 0) || (stat_num >= num_stats)) {
        return 1;
    }
    erstat[stat_num].n++;
    return (erstat[stat_num].n - erstat[stat_num].nprv);
}

TbBool is_onscreen_msg_visible(void)
{
    return (render_onscreen_msg_time > 0.0);
}

TbBool show_onscreen_msg(int nturns, const char *fmt_str, ...)
{
    va_list val;
    va_start(val, fmt_str);
    vsnprintf(onscreen_msg_text, sizeof(onscreen_msg_text), fmt_str, val);
    va_end(val);
    SYNCMSG("Onscreen message: %s",onscreen_msg_text);
    render_onscreen_msg_time = (float)nturns;
    return true;
}

TbBool erstat_check(void)
{
    // Don't check more often than every 7 turns
    if ((game.play_gameturn & 0x07) != 0)
        return false;

    if (last_checked_stat_num >= sizeof(erstat) / sizeof(erstat[0]))
    {
        ERRORLOG("Invalid last checked stat number %d, resetting to 0", last_checked_stat_num);
        last_checked_stat_num = 0;
    }

    int stat_num = last_checked_stat_num;
    int sdiff = erstat[stat_num].n - erstat[stat_num].nprv;
    // Display an error if any things were not created in this game turn
    if (sdiff != 0)
    {
#if (BFDEBUG_LEVEL > 0)
        show_onscreen_msg(game_num_fps,"%s, %ld occurrences",erstat[stat_num].msg,sdiff);
#else
        WARNLOG("%s, %d occurrences",erstat[stat_num].msg,sdiff);
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
    int tx_units_per_px;
    if (dbc_language > 0)
    {
        tx_units_per_px = scale_value_by_horizontal_resolution((MyScreenWidth >= 640) ? 16 : 32);
    }
    else
    {
        tx_units_per_px = scale_ui_value_lofi(16);
    }
    // Display in-game message for debug purposes
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_LEFT;
    if ((render_onscreen_msg_time > 0.0) || erstat_check())
    {
        if (LbScreenIsLocked())
        {
            LbTextDrawResized(scale_value_by_horizontal_resolution(160), 0, tx_units_per_px, onscreen_msg_text);
        }
        render_onscreen_msg_time -= game.delta_time;
    }
    unsigned int msg_pos = scale_value_by_vertical_resolution(200);
    if ((game.system_flags & GSF_NetGameNoSync) != 0)
    {
        ERRORLOG("OUT OF SYNC (GameTurn %7u)", game.play_gameturn);
        if (LbScreenIsLocked())
        {
            LbTextDrawResized(scale_value_by_horizontal_resolution(260), scale_value_by_vertical_resolution(msg_pos), tx_units_per_px, "OUT OF SYNC");
        }
        msg_pos += scale_value_by_horizontal_resolution(20);
    }
    if ((game.system_flags & GSF_NetSeedNoSync) != 0)
    {
        ERRORLOG("SEED OUT OF SYNC (GameTurn %7u)", game.play_gameturn);
        if (LbScreenIsLocked())
        {
            LbTextDrawResized(scale_value_by_horizontal_resolution(260), scale_value_by_vertical_resolution(msg_pos), tx_units_per_px, "SEED OUT OF SYNC");
        }
        msg_pos += scale_value_by_vertical_resolution(20);
    }
    SYNCDBG(18,"Finished");
    return true;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
