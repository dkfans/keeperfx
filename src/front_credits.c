/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_credits.c
 *     Credits and story screen displaying routines.
 * @par Purpose:
 *     Functions to show and maintain credits screen and story screen.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     11 May 2009 - 20 Jun 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "front_credits.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "bflib_filelst.h"
#include "bflib_datetm.h"

#include "gui_frontbtns.h"
#include "front_simple.h"
#include "frontend.h"
#include "vidfade.h"
#include "config_strings.h"
#include "config_campaigns.h"

#if FUNCTESTING
#include "ftests/ftest.h"
#endif

#include "post_inc.h"

/******************************************************************************/
static long frontstory_text_no;
static long credits_scroll_speed;
struct TbSpriteSheet * frontstory_font = NULL;
long credits_offset;
int credits_end;
/******************************************************************************/
void frontstory_load(void)
{
    frontend_load_data_from_cd();
#ifdef SPRITE_FORMAT_V2
    frontstory_font = load_font("ldata/frontft1-64.dat", "ldata/frontft1-64.tab");
#else
    frontstory_font = load_font("ldata/frontft1.dat", "ldata/frontft1.tab");
#endif
    if (!frontstory_font)
    {
        ERRORLOG("Unable to load front story font");
    } else
    {
        LbDataLoadSetModifyFilenameFunction(defaultModifyDataLoadFilename);
        LbPaletteSet(frontend_palette);
        srand(LbTimerClock());
#if FUNCTESTING
        ftest_srand();
#endif // FUNCTESTING
        frontstory_text_no = GUIStr_EasterPoems + rand() % 26;
    }
}

void frontstory_unload(void)
{
    free_font(&frontstory_font);
}

void frontstory_draw(void)
{
    frontend_copy_background();
    LbTextSetWindow(70*units_per_pixel/16, 70*units_per_pixel/16, (640-2*70)*units_per_pixel/16, (480-2*70)*units_per_pixel/16);
    LbTextSetFont(frontstory_font);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    int tx_units_per_px = (26 * units_per_pixel) / LbTextLineHeight();
    LbTextDrawResized(0, 0, tx_units_per_px, get_string(frontstory_text_no));
}

short frontstory_input(void)
{
  return false;
}

void frontcredits_draw(void)
{
    credits_offset -= credits_scroll_speed;
    frontend_copy_background();

    lbDisplay.DrawFlags = Lb_SPRITE_OUTLINE | Lb_TEXT_HALIGN_CENTER;
    LbTextSetWindow(0, 0, lbDisplay.PhysicalScreenWidth, lbDisplay.PhysicalScreenHeight);
    int fontid = 1;
    LbTextSetFont(frontend_font[fontid]);
    long h = credits_offset;
    TbBool did_draw = h > 0;
    for (long i = 0; campaign.credits[i].kind != CIK_None; i++)
    {
        if (h >= lbDisplay.PhysicalScreenHeight)
          break;
        struct CreditsItem* credit = &campaign.credits[i];
        if (credit->font != fontid)
        {
          fontid = credit->font;
          LbTextSetFont(frontend_font[fontid]);
        }
        int ln_height = LbTextLineHeight() * units_per_pixel / 16;
        if (h > -ln_height)
        {
            const char* text;
            switch (credit->kind)
            {
            case CIK_StringId:
                text = get_string(credit->num);
                break;
            case CIK_DirectText:
                text = credit->str;
                break;
            default:
                text = "";
                break;
            }
            LbTextDrawResized(0, h, units_per_pixel, text);
            did_draw = 1;
        }
        h += ln_height + 2 * units_per_pixel / 16;
    }
    if (!did_draw)
    {
        credits_end = 1;
        credits_offset = lbDisplay.PhysicalScreenHeight;
    }
}

TbBool frontcredits_input(void)
{
    credits_scroll_speed = 1 * units_per_pixel / 16;
    int fontid = 1;
    int speed;
    if ( lbKeyOn[KC_DOWN] )
    {
        LbTextSetFont(frontend_font[fontid]);
        speed = LbTextLineHeight() * units_per_pixel / 16;
        credits_scroll_speed = speed;
    } else
    if ((lbKeyOn[KC_UP]) && (credits_offset <= 0))
    {
        LbTextSetFont(frontend_font[fontid]);
        speed = -LbTextLineHeight() * units_per_pixel / 16;
        if (speed <= credits_offset)
          speed = credits_offset;
        credits_scroll_speed = speed;
    }
    return false;
}

/******************************************************************************/
