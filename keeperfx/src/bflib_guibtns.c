/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_guibtns.c
 *     GUI Buttons support.
 * @par Purpose:
 *     Definition of button, and common routines to handle it.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_guibtns.h"

#include <string.h>
#include <stdio.h>

#include "bflib_basics.h"
#include "globals.h"
#include "bflib_string.h"
#include "bflib_sound.h"
#include "bflib_keybrd.h"

#ifdef __cplusplus
extern "C" {
#endif

DLLIMPORT void _DK_do_button_click_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);
DLLIMPORT void _DK_do_button_release_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);
/******************************************************************************/
// Global variables
char *gui_strings_data;
char *gui_strings[STRINGS_MAX+1];
TbCharCount input_field_pos;
/******************************************************************************/
// Functions

void do_sound_menu_click(void)
{
    play_non_3d_sample_no_overlap(61);
}

void do_sound_button_click(struct GuiButton *gbtn)
{
    if ((gbtn->id_num >= 1) && (gbtn->id_num <= 5))
        play_non_3d_sample(60);
    else
        play_non_3d_sample(61);
}

void do_button_click_actions(struct GuiButton *gbtn, unsigned char *s, Gf_Btn_Callback callback)
{
  _DK_do_button_click_actions(gbtn, s, callback);
}

void setup_input_field(struct GuiButton *gbtn, const char * empty_text)
{
    char *content;
    lbInkey = 0;
    memset(backup_input_field, 0, INPUT_FIELD_LEN);
    content = (char *)gbtn->content;
    if (content == NULL)
    {
        ERRORLOG("Button has invalid content pointer");
        return;
    }
    strncpy(backup_input_field, content, INPUT_FIELD_LEN-1);
    backup_input_field[INPUT_FIELD_LEN-1] = '\0';
    // Check if the text drawn should be treated as empty; if it is, ignore that string
    if ((empty_text != NULL) && (strncmp(empty_text, backup_input_field, INPUT_FIELD_LEN-1) == 0))
    {
        *content = '\0';
    }
    input_field_pos = LbLocTextStringLength(content);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
