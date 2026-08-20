/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_vscroll.c
 *     Custom vertical scrollbar for the in-game Load/Save Game menus.
 * @par Purpose:
 *     Draws and drives a vertical scrollbar (built entirely from cut-outs of the
 *     std_l / std_r slot-bar sprites, using the in-game palette) for the in-game
 *     GMnu_LOAD / GMnu_SAVE screens, so more than 8 save slots can be reached with
 *     the mouse and mouse wheel.
 * @par Comment:
 *     The vertical scrollbar is built from cut-outs/masks of two existing gui1 sprites -
 *     GBS_frontend_button_std_l (idx 1) and GBS_frontend_button_std_r (idx 3) - plus the
 *     slab0-0 panel tile for the thumb's interior fill. The design is inspired by the
 *     front-end vertical scrollbar drawn in the main-menu Load Game screen, but rebuilt
 *     for the in-game GUI's own style and palette (gui1/button_sprites, engine_palette
 *     from data/palette.dat) instead of the front-end's (frontbit, front.pal).
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "gui_vscroll.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_guibtns.h"
#include "custom_sprites.h"
#include "gui_draw.h"
#include "sprites.h"
#include "kjm_input.h"
#include "game_saves.h"
#include "frontmenu_saves.h"
#include "frontend.h"
#include "gui_frontmenu.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

/* Every piece of the scrollbar is one dedicated sprite in the gui1 sheet, so all the
 * geometry below is derived from those sprites rather than hardcoded: the widget adapts
 * to whichever sheet the current video mode loaded, exactly like the rest of the GUI. */
#define SCRL_SPR_CAP_TOP      GBS_vscroll_cap_top_std
#define SCRL_SPR_CAP_TOP_ACT  GBS_vscroll_cap_top_act
#define SCRL_SPR_TRACK        GBS_vscroll_track
#define SCRL_SPR_CAP_BOT      GBS_vscroll_cap_bottom_std
#define SCRL_SPR_CAP_BOT_ACT  GBS_vscroll_cap_bottom_act
#define SCRL_SPR_THUMB        GBS_vscroll_thumb

/******************************************************************************/
long gui_vscroll_offset = 0;

/** Scale a sprite-space length by units-per-px. */
static inline long scrl_sc(long v, int upp)
{
    return v * upp / 16;
}

/** Units-per-px for the scrollbar, taken from the cap sprite's width against the button's.
 *  Deriving it from the sprite (as simple_button_sprite_*_units_per_px does elsewhere) is
 *  what makes the same code work with the low-res sheet, where the art is smaller. */
static int scrl_units_per_px(const struct GuiButton *gbtn)
{
    const struct TbSprite *spr = get_button_sprite(SCRL_SPR_CAP_TOP);
    if ((spr == NULL) || (spr->SWidth < 1))
        return 16;
    int upp = (gbtn->width * 16 + spr->SWidth / 2) / spr->SWidth;
    if (upp < 1)
        upp = 1;
    return upp;
}

/** Scaled height of a sprite, 0 if it is missing. */
static long scrl_spr_h(long spr_idx, int upp)
{
    const struct TbSprite *spr = get_button_sprite(spr_idx);
    return (spr != NULL) ? scrl_sc(spr->SHeight, upp) : 0;
}

/** Draw one sprite at a screen position. */
static void scrl_draw(long x, long y, int upp, long spr_idx)
{
    const struct TbSprite *spr = get_button_sprite(spr_idx);
    if (spr != NULL)
        LbSpriteDrawResized(x, y, upp, spr);
}

/** Tile the track sprite down [y0,y1). The track art is vertically uniform, so the last
 *  tile is simply clipped to what is left and no seam can show. */
static void scrl_draw_track(long ox, long y0, long y1, int upp)
{
    const struct TbSprite *spr = get_button_sprite(SCRL_SPR_TRACK);
    if ((spr == NULL) || (spr->SHeight < 1))
        return;
    long seg = scrl_sc(spr->SHeight, upp);
    if (seg < 1)
        seg = 1;
    long w = scrl_sc(spr->SWidth, upp);
    TbGraphicsWindow grwnd;
    LbScreenStoreGraphicsWindow(&grwnd);
    for (long y = y0; y < y1; y += seg)
    {
        long h = (y + seg <= y1) ? seg : (y1 - y);
        LbScreenSetGraphicsWindow(ox, y, w, h);
        LbSpriteDrawResized(0, 0, upp, spr);
    }
    LbScreenLoadGraphicsWindow(&grwnd);
}
long gui_vscroll_total(void)
{
    int last_used = -1;
    for (int i = 0; i < save_game_catalogue_count; i++)
    {
        if ((save_game_catalogue[i].flags & CEF_InUse) != 0)
            last_used = i;
    }
    /* The Save menu reveals one free slot past the used ones (so a new game can be
     * saved there); the Load menu must not. Both keep at least the 8 on-screen rows and
     * never exceed the number of slots the catalogue currently holds. */
    long total = last_used + (menu_is_active(GMnu_SAVE) ? 2 : 1);
    if (total < GUI_VSCROLL_VISIBLE)
        total = GUI_VSCROLL_VISIBLE;
    if (total > save_game_catalogue_count)
        total = save_game_catalogue_count;
    return total;
}

long gui_vscroll_max_offset(void)
{
    long m = gui_vscroll_total() - GUI_VSCROLL_VISIBLE;
    return (m > 0) ? m : 0;
}

/* --- thumb dragging.  The thumb always sits at the stepped position of the current scroll
 * offset (one stop per slot), whether it got there from the wheel, the arrows or a drag: it
 * only ever appears on a valid stop, never between two of them.  This matches the existing
 * front-end scrollbars (campaign / mappack selection), which snap the same way. --- */
static TbBool scrl_dragging = false;
static long scrl_drag_grab = 0;     /* where inside the thumb it was grabbed, screen px */

static long scrl_thumb_dy(long off, int upp, long widget_h)
{
    long top = scrl_spr_h(SCRL_SPR_CAP_TOP, upp);
    long max_off = gui_vscroll_max_offset();
    if (max_off <= 0)
        return top;
    /* The thumb runs from just under the top cap to where its bottom meets the bottom cap. */
    long bot = widget_h - scrl_spr_h(SCRL_SPR_CAP_BOT, upp) - scrl_spr_h(SCRL_SPR_THUMB, upp);
    long travel = bot - top;
    if (travel < 0)
        travel = 0;
    return top + off * travel / max_off;
}

/** Is the mouse inside the screen rect (x,y,w,h)? */
static TbBool scrl_mouse_over(long x, long y, long w, long h)
{
    long mx = GetMouseX();
    long my = GetMouseY();
    return (mx >= x) && (mx < x + w) && (my >= y) && (my < y + h);
}

void gui_vscroll_draw(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    int upp = scrl_units_per_px(gbtn);
    long ox = gbtn->scr_pos_x;
    long oy = gbtn->scr_pos_y;
    long w = gbtn->width;
    unsigned short flg = RendererGetDrawFlags();
    RendererSetDrawFlags(0);

    long top_h = scrl_spr_h(SCRL_SPR_CAP_TOP, upp);
    long bot_h = scrl_spr_h(SCRL_SPR_CAP_BOT, upp);
    long track_y0 = oy + top_h;
    long track_y1 = oy + gbtn->height - bot_h;
    long max_off = gui_vscroll_max_offset();

    /* Caps light up while the mouse is over them, but only when that arrow can actually
     * scroll - the lit art is the hover area, so the two can never drift apart. */
    TbBool up_lit = (gui_vscroll_offset > 0) && scrl_mouse_over(ox, oy, w, top_h);
    TbBool dn_lit = (gui_vscroll_offset < max_off) && scrl_mouse_over(ox, track_y1, w, bot_h);

    /* WORKAROUND, not a clean fix. The engine's sprite scaler starts its row grid half a
     * source pixel in (LbSpriteSetScalingHeight*Array uses `factor >> 1`), so a scaled
     * sprite gets its first row one destination pixel too tall and everything below slides
     * down by the same amount. On the caps that surfaces as a hairline of the base's colour
     * sitting where the track should still be - visible from 2x upwards, not at 640x480
     * where the grid is 1:1. We cover it with the track: its art is uniform and it tiles to
     * any length, so running it that half-step into the bottom cap hides the stray pixels
     * and costs nothing, and it is the one piece here that can absorb the slack. Only the
     * BOTTOM cap needs it: the same downward slide leaves no gap under the top cap, and
     * running the track up into it would eat the last row of that base instead. The real
     * fix belongs in the scaler, but that changes how every sprite in the game is drawn,
     * which is far beyond this widget. Drawn after the caps so it paints over them. */
    long bias = upp / 32;               /* the scaler's half-step, in screen pixels */
    scrl_draw(ox, oy, upp, up_lit ? SCRL_SPR_CAP_TOP_ACT : SCRL_SPR_CAP_TOP);
    scrl_draw(ox, track_y1, upp, dn_lit ? SCRL_SPR_CAP_BOT_ACT : SCRL_SPR_CAP_BOT);
    scrl_draw_track(ox, track_y0, track_y1 + bias, upp);

    {
        const struct TbSprite *th = get_button_sprite(SCRL_SPR_THUMB);
        long tw = (th != NULL) ? scrl_sc(th->SWidth, upp) : 0;
        scrl_draw(ox + (w - tw) / 2, oy + scrl_thumb_dy(gui_vscroll_offset, upp, gbtn->height),
                  upp, SCRL_SPR_THUMB);
    }

    RendererSetDrawFlags(flg);
}

static int vscroll_selected_row(void)
{
    if ((input_button == NULL) || !menu_is_active(GMnu_SAVE))
        return -1;
    for (int row = 0; row < GUI_VSCROLL_VISIBLE; row++)
    {
        if (input_button->content.str == input_string[row])
            return row;
    }
    return -1;
}

static struct GuiButton *vscroll_find_slot_button(int row, char gmenu_idx)
{
    for (int i = 0; i < ACTIVE_BUTTONS_COUNT; i++)
    {
        struct GuiButton *b = &active_buttons[i];
        if ((b->flags & LbBtnF_Active) == 0)
            continue;
        if ((b->gmenu_idx == gmenu_idx) && (b->content.str == input_string[row]))
            return b;
    }
    return NULL;
}

static void vscroll_apply_offset(long new_off)
{
    long max_off = gui_vscroll_max_offset();
    if (new_off < 0)
        new_off = 0;
    if (new_off > max_off)
        new_off = max_off;

    int sel_row = vscroll_selected_row();
    long sel_slot = (sel_row >= 0) ? (gui_vscroll_offset + sel_row) : -1;
    if (sel_row >= 0)
    {
        /* keep the selected slot within rows [0 .. GUI_VSCROLL_VISIBLE-1] */
        if (new_off > sel_slot)
            new_off = sel_slot;
        if (new_off < sel_slot - (GUI_VSCROLL_VISIBLE - 1))
            new_off = sel_slot - (GUI_VSCROLL_VISIBLE - 1);
        if (new_off < 0)
            new_off = 0;
        if (new_off > max_off)
            new_off = max_off;
    }
    if (new_off == gui_vscroll_offset)
        return;   /* no movement: leave input_string alone (preserves a name being typed) */

    char typed[SAVE_TEXTNAME_LEN + 1];
    char sel_gmenu = 0;
    struct GuiButton *sel_btn = NULL;
    if (sel_row >= 0)
    {
        snprintf(typed, sizeof(typed), "%s", input_string[sel_row]);
        sel_gmenu = input_button->gmenu_idx;
        sel_btn = input_button;
    }
    gui_vscroll_offset = new_off;
    update_loadsave_input_strings(save_game_catalogue);   /* refill the visible rows */
    if (sel_row >= 0)
    {
        int new_row = (int)(sel_slot - new_off);
        snprintf(input_string[new_row], SAVE_TEXTNAME_LEN, "%s", typed);   /* carry the typed name */
        struct GuiButton *nb = vscroll_find_slot_button(new_row, sel_gmenu);
        if (nb != NULL)
        {
            sel_btn->button_state_left_pressed = 0;   /* old row: drop the red highlight */
            nb->button_state_left_pressed = 1;        /* new row: show it red */
            input_button = nb;                        /* move edit + caret onto the new row */
        }
    }
}

void gui_vscroll_maintain(struct GuiButton *gbtn)
{
    static TbBool prev_mleft = false;
    long off = gui_vscroll_offset;
    if (wheel_scrolled_up)
        off--;
    if (wheel_scrolled_down)
        off++;
    if (gbtn != NULL)
    {
        int upp = scrl_units_per_px(gbtn);
        long rel_x = GetMouseX() - gbtn->scr_pos_x;
        long rel_y = GetMouseY() - gbtn->scr_pos_y;
        long max_off = gui_vscroll_max_offset();
        TbBool over = (rel_x >= 0) && (rel_x < gbtn->width)
                   && (rel_y >= 0) && (rel_y < gbtn->height);
        TbBool click_edge = (lbDisplay.MLeftButton != 0) && !prev_mleft;
        long th = scrl_spr_h(SCRL_SPR_THUMB, upp);
        long top = scrl_spr_h(SCRL_SPR_CAP_TOP, upp);
        long bot = gbtn->height - scrl_spr_h(SCRL_SPR_CAP_BOT, upp) - th;
        if (over && click_edge)
        {
            if (rel_y < top)
                off--; /* up arrow */
            else if (rel_y > bot + th)
                off++; /* down arrow */
            else if (max_off > 0)
            {
                /* Grab the thumb: pressing on it keeps the grabbed point under the cursor
                 * (so it does not jump), pressing the bare track drops it centred there. */
                long tdy = scrl_thumb_dy(gui_vscroll_offset, upp, gbtn->height);
                scrl_drag_grab = ((rel_y >= tdy) && (rel_y < tdy + th)) ? (rel_y - tdy) : (th / 2);
                scrl_dragging = true;
            }
        }
        if (lbDisplay.MLeftButton == 0)
            scrl_dragging = false;
        if (scrl_dragging && (max_off > 0))
        {
            /* Turn the cursor position straight into a slot offset - rounded, so it flips at
             * the midpoint between two stops. The thumb is then drawn from that offset, so it
             * snaps to valid stops as you drag rather than following the cursor freely.
             * Deliberately not gated on over: once grabbed, the drag keeps working even
             * if the cursor wanders off the bar sideways. */
            long dy = rel_y - scrl_drag_grab;
            if (dy < top)
                dy = top;
            if (dy > bot)
                dy = bot;
            long travel = bot - top;
            off = (travel > 0) ? (((dy - top) * max_off + travel / 2) / travel) : 0;
        }
    }
    prev_mleft = (lbDisplay.MLeftButton != 0);
    vscroll_apply_offset(off);
}

void gui_vscroll_input(struct GuiButton *gbtn)
{
    /* Intentionally empty: all input is handled in gui_vscroll_maintain (which runs even
     * while a Save-slot name is being edited, unlike this click_event).
     * Do NOT replace this with NULL in the button table: the menu code treats a non-NULL
     * click_event as the flag for "this button is interactive", and gates the whole
     * press/release handling on it (see gui_frontbtns.c). With NULL our button matches
     * none of the other conditions there, so it would stop consuming clicks and let them
     * fall through to whatever is underneath. */
    (void)gbtn;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
