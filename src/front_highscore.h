/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_highscore.h
 *     Header file for front_highscore.c.
 * @par Purpose:
 *     High Score screen displaying routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     01 Jan 2012 - 23 Jun 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_FRONT_HIGHSCORE_H
#define DK_FRONT_HIGHSCORE_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_guibtns.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
DLLIMPORT struct GuiButtonInit _DK_frontend_high_score_score_buttons[];
DLLIMPORT extern long _DK_high_score_entry_input_active;
#define high_score_entry_input_active _DK_high_score_entry_input_active
DLLIMPORT extern long _DK_high_score_entry_index;
#define high_score_entry_index _DK_high_score_entry_index
DLLIMPORT extern char _DK_high_score_entry[64];
#define high_score_entry _DK_high_score_entry
DLLIMPORT extern int _DK_fe_high_score_table_from_main_menu;
#define fe_high_score_table_from_main_menu _DK_fe_high_score_table_from_main_menu
/******************************************************************************/
void frontend_draw_high_score_table(struct GuiButton *gbtn);
void frontend_quit_high_score_table(struct GuiButton *gbtn);
void frontend_maintain_high_score_ok_button(struct GuiButton *gbtn);
TbBool frontend_high_score_table_input(void);
void frontend_maintain_high_score_ok_button(struct GuiButton *gbtn);
void frontstats_save_high_score(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
