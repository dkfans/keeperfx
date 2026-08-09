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
 *     Every piece of the scrollbar is a rectangular cut-out (mask) of the 
 *     std_l / std_r slot-bar sprites (button_sprites, gui1-64, in-game palette), 
 *     no new sprites are added. A cut-out is drawn by clipping the graphics window
 *     to the wanted rectangle and drawing the whole sprite shifted so that rectangle
 *     lands under the window. All offsets are in native (640x480 / units_per_px==16)
 *     pixels and scaled up by units_per_px.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "gui_vscroll.h"
#include <math.h>

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
/* Source sprites every scroll piece is cut from (gui1-64 button_sprites). */
#define SCRL_SPR_L      GBS_frontend_button_std_l   /* 32 x 30 */
#define SCRL_SPR_R      GBS_frontend_button_std_r   /* 30 x 30 */

/* --- frame: a rounded-rectangle border, native px, relative to the
 * scrollbar button's top-left (== the frame's top-left). --- */
#define SCRL_FRAME_W        33
#define SCRL_FRAME_H        254
#define SCRL_EDGE           4       /* border-line thickness */
/* four rounded corners: cut rectangle (x,y,w,h) in the source sprite, placed at (px,py) */
#define SCRL_TL_CX  0
#define SCRL_TL_CY  0
#define SCRL_TL_W   14
#define SCRL_TL_H   14
#define SCRL_TR_CX  12          /* std_r is 30 wide -> 30-18 */
#define SCRL_TR_CY  0
#define SCRL_TR_W   18
#define SCRL_TR_H   11
#define SCRL_BL_CX  0
#define SCRL_BL_CY  17          /* std_l is 30 tall -> 30-13 */
#define SCRL_BL_W   11
#define SCRL_BL_H   13
#define SCRL_BR_CX  19          /* 30-11 */
#define SCRL_BR_CY  19          /* 30-11 */
#define SCRL_BR_W   11
#define SCRL_BR_H   11
/* the four 1px edge lines (tiled between the corners). left/right double as the rails. */
#define SCRL_TOPLINE_CX 31      /* std_l rightmost column, top 4px */
#define SCRL_TOPLINE_CY 0
#define SCRL_BOTLINE_CX 31      /* std_l rightmost column, bottom 4px */
#define SCRL_BOTLINE_CY 26
#define SCRL_LEFTLINE_CX 0      /* std_l row 14, 4px wide (light bevel) */
#define SCRL_LEFTLINE_CY 14
#define SCRL_RIGHTLINE_CX 26    /* std_r row 11, 4px wide (dark bevel) */
#define SCRL_RIGHTLINE_CY 11

/* --- track (rails + fill), native px relative to the frame --- */
#define SCRL_TRACK_X    10
#define SCRL_TRACK_Y    21
#define SCRL_TRACK_W    13
#define SCRL_TRACK_H    211
#define SCRL_LRAIL_X    6
#define SCRL_RRAIL_X    23
#define SCRL_RAIL_W     4
#define SCRL_FILL_COLOUR 0      /* in-game palette index of black (0,0,0) */

/* --- thumb: the same frame construction at a small fixed size, filled with slab --- */
#define SCRL_THUMB_W    32
#define SCRL_THUMB_H    27
#define SCRL_THUMB_TOP  21      /* frame-relative y at scroll offset 0 (== track top) */
#define SCRL_THUMB_BOT  205     /* frame-relative y at max scroll offset (track_h - thumb_h) */

/* --- arrows: two halves each, cut from the std_l/std_r corners --- */
/* up arrow (points up, sits above the track): std_l top-left + std_r top-right */
/* arrow_ew = -1 (each half 1px narrower) and the two halves are cut to equal height (the
 * taller right half is trimmed at its outer end), so the arrow's outer edge is even. */
#define SCRL_UP_X       6       /* frame-relative x of the arrow */
#define SCRL_UP_Y       6
#define SCRL_UPL_CX 0
#define SCRL_UPL_CY 0
#define SCRL_UPL_W  10
#define SCRL_UPL_H  11
#define SCRL_UPR_CX 19          /* 30-11 */
#define SCRL_UPR_CY 0
#define SCRL_UPR_W  11
#define SCRL_UPR_H  11          /* trimmed from 12 to match the left half */
/* down arrow (points down, sits below the track): std_l bottom-left + std_r bottom-right */
#define SCRL_DN_X       6
#define SCRL_DN_Y       236
#define SCRL_DNL_CX 0
#define SCRL_DNL_CY 18          /* 30-12 */
#define SCRL_DNL_W  10
#define SCRL_DNL_H  12
#define SCRL_DNR_CX 19          /* 30-11 */
#define SCRL_DNR_CY 18          /* 30-12 (trimmed from 13 to match the left half) */
#define SCRL_DNR_W  11
#define SCRL_DNR_H  12
#define SCRL_ARROW_W    21      /* 10 + 11 */

/* --- bases: gradient line (2 lines x 2px = 4px) as wide as the arrow (21px),
 * glued to the arrow. Each is a horizontal gradient sampled from the arrow's edge colours
 * (bottom edge for the up base, top edge for the down one), with a darker 2nd line. */
#define SCRL_BASE_H     4
#define SCRL_BASE_Y_UP  17      /* frame-relative y of the up base (just under up arrow) */
#define SCRL_BASE_Y_DN  232     /* frame-relative y of the down base (just over down arrow) */
/* up base: top 2px = main gradient (glued to arrow's bottom), bottom 2px = darker */
static const unsigned char scrl_base_up_top[SCRL_ARROW_W] =
    { 12, 11, 11, 10, 10,  9,  9,  8,  8,  7,  7,241, 83, 51,  5, 82, 82, 81, 81, 49,  2 };
static const unsigned char scrl_base_up_bot[SCRL_ARROW_W] =
    { 84, 83, 83, 83, 83, 83, 83, 82, 82, 82, 82, 82, 81, 81, 81,  2,  2, 48,145,  1,253 };
/* down base: bottom 2px = main gradient (glued to arrow's top), top 2px = darker */
static const unsigned char scrl_base_dn_top[SCRL_ARROW_W] =
    {  5,  5, 82, 82, 82, 82, 82, 82, 81, 81, 81, 81, 81, 49,  2,  2, 48,145,  1,  1,253 };
static const unsigned char scrl_base_dn_bot[SCRL_ARROW_W] =
    {  9,  9,  8,  8,  8,  7,  7,241,  6,  6, 51,  5,  5, 82, 82,  4, 81, 81, 49,  2,  2 };

/******************************************************************************/
long gui_vscroll_offset = 0;

static inline long scrl_sc(long v, int upp)
{
    return v * upp / 16;
}

/* All pieces are placed by native-pixel boundaries relative to a base origin
 * (ox,oy): a piece occupying native columns [nx, nx+cw) lands on screen columns
 * [ox+scrl_sc(nx), ox+scrl_sc(nx+cw)). Because the right edge of one piece is the
 * left edge of the next (both == ox+scrl_sc(shared_boundary)), adjacent pieces
 * never leave a rounding gap nor overlap, at any units_per_px. Cropped-sprite
 * pieces are drawn by clipping the graphics window to that screen rectangle and
 * drawing the whole sprite shifted so the wanted crop lands under the window. */

static void scrl_blit(long ox, long oy, int upp, long spr_idx, int nx, int ny, int cx, int cy, int cw, int ch)
{
    const struct TbSprite *spr = get_button_sprite(spr_idx);
    if (spr == NULL)
        return;
    long x0 = ox + scrl_sc(nx, upp);
    long x1 = ox + scrl_sc(nx + cw, upp);
    long y0 = oy + scrl_sc(ny, upp);
    long y1 = oy + scrl_sc(ny + ch, upp);
    if ((x1 <= x0) || (y1 <= y0))
        return;
    TbGraphicsWindow grwnd;
    LbScreenStoreGraphicsWindow(&grwnd);
    LbScreenSetGraphicsWindow(x0, y0, x1 - x0, y1 - y0);
    LbSpriteDrawResized(-scrl_sc(cx, upp), -scrl_sc(cy, upp), upp, spr);
    LbScreenLoadGraphicsWindow(&grwnd);
}


static void scrl_vline(long ox, long oy, int upp, long spr_idx, int nx, int ny0, int ny1, int cw, int cx, int cy)
{
    const struct TbSprite *spr = get_button_sprite(spr_idx);
    if (spr == NULL)
        return;
    long x0 = ox + scrl_sc(nx, upp);
    long w = scrl_sc(cw, upp);
    long yA = oy + scrl_sc(ny0, upp);
    long yB = oy + scrl_sc(ny1, upp);
    if ((w < 1) || (yB <= yA))
        return;
    TbGraphicsWindow grwnd;
    LbScreenStoreGraphicsWindow(&grwnd);
    for (long y = yA; y < yB; y++)
    {
        LbScreenSetGraphicsWindow(x0, y, w, 1);
        LbSpriteDrawResized(-scrl_sc(cx, upp), -scrl_sc(cy, upp), upp, spr);
    }
    LbScreenLoadGraphicsWindow(&grwnd);
}

static void scrl_hline(long ox, long oy, int upp, long spr_idx, int nx0, int nx1, int ny, int ch, int cx, int cy)
{
    const struct TbSprite *spr = get_button_sprite(spr_idx);
    if (spr == NULL)
        return;
    long y0 = oy + scrl_sc(ny, upp);
    long h = scrl_sc(ch, upp);
    long xA = ox + scrl_sc(nx0, upp);
    long xB = ox + scrl_sc(nx1, upp);
    if ((h < 1) || (xB <= xA))
        return;
    TbGraphicsWindow grwnd;
    LbScreenStoreGraphicsWindow(&grwnd);
    for (long x = xA; x < xB; x++)
    {
        LbScreenSetGraphicsWindow(x, y0, 1, h);
        LbSpriteDrawResized(-scrl_sc(cx, upp), -scrl_sc(cy, upp), upp, spr);
    }
    LbScreenLoadGraphicsWindow(&grwnd);
}

static void scrl_draw_frame(long ox, long oy, int upp, int fw, int fh)
{
    /* edges tiled only between the corners (so the corners keep their rounded shape) */
    scrl_hline(ox, oy, upp, SCRL_SPR_L, SCRL_TL_W, fw - SCRL_TR_W, 0, SCRL_EDGE, SCRL_TOPLINE_CX, SCRL_TOPLINE_CY);
    scrl_hline(ox, oy, upp, SCRL_SPR_L, SCRL_BL_W, fw - SCRL_BR_W, fh - SCRL_EDGE, SCRL_EDGE, SCRL_BOTLINE_CX, SCRL_BOTLINE_CY);
    scrl_vline(ox, oy, upp, SCRL_SPR_L, 0, SCRL_TL_H, fh - SCRL_BL_H, SCRL_EDGE, SCRL_LEFTLINE_CX, SCRL_LEFTLINE_CY);
    scrl_vline(ox, oy, upp, SCRL_SPR_R, fw - SCRL_EDGE, SCRL_TR_H, fh - SCRL_BR_H, SCRL_EDGE, SCRL_RIGHTLINE_CX, SCRL_RIGHTLINE_CY);
    /* corners on top */
    scrl_blit(ox, oy, upp, SCRL_SPR_L, 0, 0, SCRL_TL_CX, SCRL_TL_CY, SCRL_TL_W, SCRL_TL_H);
    scrl_blit(ox, oy, upp, SCRL_SPR_R, fw - SCRL_TR_W, 0, SCRL_TR_CX, SCRL_TR_CY, SCRL_TR_W, SCRL_TR_H);
    scrl_blit(ox, oy, upp, SCRL_SPR_L, 0, fh - SCRL_BL_H, SCRL_BL_CX, SCRL_BL_CY, SCRL_BL_W, SCRL_BL_H);
    scrl_blit(ox, oy, upp, SCRL_SPR_R, fw - SCRL_BR_W, fh - SCRL_BR_H, SCRL_BR_CX, SCRL_BR_CY, SCRL_BR_W, SCRL_BR_H);
}

static void scrl_draw_base_gradient(long ox, long oy, int upp, int by,
    const unsigned char *top, const unsigned char *bot)
{
    long y0 = oy + scrl_sc(by, upp);
    long ym = oy + scrl_sc(by + 2, upp);
    long y1 = oy + scrl_sc(by + 4, upp);
    for (int c = 0; c < SCRL_ARROW_W; c++)
    {
        long x0 = ox + scrl_sc(SCRL_UP_X + c, upp);
        long x1 = ox + scrl_sc(SCRL_UP_X + c + 1, upp);
        LbDrawBox(x0, y0, x1 - x0, ym - y0, top[c]);
        LbDrawBox(x0, ym, x1 - x0, y1 - ym, bot[c]);
    }
}

static void scrl_draw_arrow(long ox, long oy, int upp, int nx, int ny, int kind)
{
    if (kind == 0)
    {   /* up: two top corners, top-aligned */
        scrl_blit(ox, oy, upp, SCRL_SPR_L, nx, ny, SCRL_UPL_CX, SCRL_UPL_CY, SCRL_UPL_W, SCRL_UPL_H);
        scrl_blit(ox, oy, upp, SCRL_SPR_R, nx + SCRL_UPL_W, ny, SCRL_UPR_CX, SCRL_UPR_CY, SCRL_UPR_W, SCRL_UPR_H);
    } else
    {   /* down: two bottom corners, bottom-aligned (left half is 1px shorter) */
        scrl_blit(ox, oy, upp, SCRL_SPR_L, nx, ny + (SCRL_DNR_H - SCRL_DNL_H), SCRL_DNL_CX, SCRL_DNL_CY, SCRL_DNL_W, SCRL_DNL_H);
        scrl_blit(ox, oy, upp, SCRL_SPR_R, nx + SCRL_DNL_W, ny, SCRL_DNR_CX, SCRL_DNR_CY, SCRL_DNR_W, SCRL_DNR_H);
    }
}

/* Draw the thumb (the frame construction at thumb size, slab-filled) at native y=ty. Enclosed 
 * interior of the 32x27 framed thumb, as [xl,xr) per native row (0..26). The thumb is a hexagon:
 * rows 0-3 and 23-26 are the solid pointed top/bottom (no interior), rows 4-22 are its body.
   The tile is clipped to it so it can never show in the rounded-away corners. */
static const unsigned char scrl_thumb_span[SCRL_THUMB_H][2] = {
    { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},
    {10,21},{ 9,22},{ 8,23},{ 7,24},{ 6,25},{ 5,26},{ 4,27},
    { 4,28},{ 4,28},{ 4,28},{ 4,28},{ 4,28},
    { 5,27},{ 6,26},{ 7,25},{ 8,24},{ 9,23},{10,22},{11,21},
    { 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},
};

static void scrl_fill_thumb_interior(long tox, long toy, int upp)
{
    long ax = tox / pixel_size;      /* tile-pattern origin, in WScreen pixels */
    long ay = toy / pixel_size;
    long maxw = lbDisplay.GraphicsScreenWidth;
    long maxh = MyScreenHeight;
    for (int ny = 0; ny < SCRL_THUMB_H; ny++)
    {
        /* Dilate the interior span by 1 native px (a 3x3 dilation: widen by the vertical
         * neighbours' spans, then 1px each side) so the fill always runs a hair under the
         * frame border. The border (drawn on top, SCRL_EDGE px thick) hides that overfill,
         * and it guarantees no 1px gap can open between fill and border when the two are
         * scaled with different rounding (scrl_sc truncates; the sprite scaler rounds).
         * 1 native px == upp/16 screen px, so it scales with the resolution. */
        int xl = SCRL_THUMB_W, xr = 0;
        for (int k = ny - 1; k <= ny + 1; k++)
        {
            if ((k < 0) || (k >= SCRL_THUMB_H))
                continue;
            if (scrl_thumb_span[k][1] <= scrl_thumb_span[k][0])
                continue;
            if (scrl_thumb_span[k][0] < xl) xl = scrl_thumb_span[k][0];
            if (scrl_thumb_span[k][1] > xr) xr = scrl_thumb_span[k][1];
        }
        if (xr <= xl)
            continue;
        xl -= 1;
        xr += 1;
        if (xl < 0) xl = 0;
        if (xr > SCRL_THUMB_W) xr = SCRL_THUMB_W;
        long sy0 = (toy + scrl_sc(ny, upp)) / pixel_size;
        long sy1 = (toy + scrl_sc(ny + 1, upp)) / pixel_size;
        long sx0 = (tox + scrl_sc(xl, upp)) / pixel_size;
        long sx1 = (tox + scrl_sc(xr, upp)) / pixel_size;
        if (sy0 < 0) sy0 = 0;
        if (sx0 < 0) sx0 = 0;
        if (sy1 > maxh) sy1 = maxh;
        if (sx1 > maxw) sx1 = maxw;
        for (long sy = sy0; sy < sy1; sy++)
        {
            const TbPixel* trow = &gui_slab[GUI_SLAB_DIMENSION * ((sy - ay) & (GUI_SLAB_DIMENSION - 1))];
            TbPixel* out = &lbDisplay.WScreen[sy * lbDisplay.GraphicsScreenWidth];
            for (long sx = sx0; sx < sx1; sx++)
                out[sx] = trow[(sx - ax) & (GUI_SLAB_DIMENSION - 1)];
        }
    }
}

static void scrl_draw_thumb(long ox, long oy, int upp, long dy)
{
    int tnx = (SCRL_FRAME_W - SCRL_THUMB_W) / 2;    /* == 0: thumb is as wide as the frame */
    long tox = ox + scrl_sc(tnx, upp);
    long toy = oy + dy;
    scrl_fill_thumb_interior(tox, toy, upp);        /* tile, clipped to the hexagon interior */
    scrl_draw_frame(tox, toy, upp, SCRL_THUMB_W, SCRL_THUMB_H);
}

static int scrl_units_per_px(const struct GuiButton *gbtn)
{
    int upp = (gbtn->height * 16 + SCRL_FRAME_H / 2) / SCRL_FRAME_H;
    if (upp < 1)
        upp = 1;
    return upp;
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

/* --- thumb dragging.  The thumb normally sits at the stepped position of the current
 * scroll offset (one stop per slot), which is what the wheel and the arrows produce.  While
 * it is being dragged it instead follows the cursor pixel for pixel, so it never jumps
 * under the finger; the list still snaps to whole slots, and on release the thumb returns
 * to the stepped position of wherever it ended up.  The drag position is kept in screen
 * pixels (not native ones) so it stays exact at any units_per_px. --- */
static TbBool scrl_dragging = false;
static long scrl_drag_dy = 0;       /* thumb top, screen px from the frame top */
static long scrl_drag_grab = 0;     /* where inside the thumb it was grabbed, screen px */

static long scrl_thumb_dy_for(long off, int upp)
{
    long top = scrl_sc(SCRL_THUMB_TOP, upp);
    long max_off = gui_vscroll_max_offset();
    if (max_off <= 0)
        return top;
    long travel = scrl_sc(SCRL_THUMB_BOT, upp) - top;
    return top + off * travel / max_off;
}

static long scrl_thumb_dy(int upp)
{
    return scrl_dragging ? scrl_drag_dy : scrl_thumb_dy_for(gui_vscroll_offset, upp);
}

/* --- arrow "active" red glow (arrow + base union), shown while the mouse is over a
 * clickable arrow. The arrow's triangle plus its base
 * rectangle form one shape, banded by distance to the silhouette (edge -> centre) with the
 * 5 red tones below (in-game palette indices). Built once per direction and cached. --- */
#define SCRL_GLOW_BANDS 5
#define SCRL_GLOW_MAXW  24
#define SCRL_GLOW_MAXH  20
static const unsigned char scrl_glow_ramp[SCRL_GLOW_BANDS] = { 132, 72, 76, 79, 233 };

static int scrl_in_arrow_tri(int kind, int aw, int ah, int lx, int ly)
{
    float ax, ay, bx, by, cx, cy;
    if (kind == 0) { ax = aw / 2.0f; ay = 0.5f;      bx = 0.5f; by = ah - 0.5f; cx = aw - 0.5f; cy = ah - 0.5f; }
    else           { ax = aw / 2.0f; ay = ah - 0.5f; bx = 0.5f; by = 0.5f;      cx = aw - 0.5f; cy = 0.5f;      }
    float px = lx + 0.5f, py = ly + 0.5f;
    float s1 = (px - bx) * (ay - by) - (ax - bx) * (py - by);
    float s2 = (px - cx) * (by - cy) - (bx - cx) * (py - cy);
    float s3 = (px - ax) * (cy - ay) - (cx - ax) * (py - ay);
    return ((s1 <= 0.0f && s2 <= 0.0f && s3 <= 0.0f) || (s1 >= 0.0f && s2 >= 0.0f && s3 >= 0.0f));
}

struct ScrlGlow {
    unsigned char bands[SCRL_GLOW_MAXW * SCRL_GLOW_MAXH];   /* per-cell band 0..4, or SCRL_GLOW_BANDS = empty */
    int w, h;
};

static void scrl_build_glow(int kind, int ah, struct ScrlGlow *g)
{
    int aw = SCRL_ARROW_W;
    int W = aw, H = ah + SCRL_BASE_H;
    int arrow_y = (kind == 0) ? 0 : SCRL_BASE_H;
    int base_y  = (kind == 0) ? ah : 0;
    unsigned char inside[SCRL_GLOW_MAXH][SCRL_GLOW_MAXW];
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
        {
            int in = scrl_in_arrow_tri(kind, aw, ah, x, y - arrow_y);
            if (!in && (y >= base_y) && (y < base_y + SCRL_BASE_H))
                in = 1;
            inside[y][x] = (unsigned char)in;
        }
    float dist[SCRL_GLOW_MAXH][SCRL_GLOW_MAXW];
    float maxd = 0.0f;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
        {
            if (!inside[y][x]) { dist[y][x] = -1.0f; continue; }
            float best = 1.0e9f;
            for (int ny = -1; ny <= H; ny++)
                for (int nx = -1; nx <= W; nx++)
                {
                    int out = (nx < 0 || nx >= W || ny < 0 || ny >= H) ? 1 : !inside[ny][nx];
                    if (!out)
                        continue;
                    float dx = (float)(x - nx), dy = (float)(y - ny);
                    float d = sqrtf(dx * dx + dy * dy);
                    if (d < best) best = d;
                }
            dist[y][x] = best;
            if (best > maxd) maxd = best;
        }
    float bandsz = maxd / SCRL_GLOW_BANDS;
    if (bandsz < 1.0f) bandsz = 1.0f;
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
        {
            if (dist[y][x] < 0.0f) { g->bands[y * W + x] = SCRL_GLOW_BANDS; continue; }
            int b = (int)(dist[y][x] / bandsz);
            if (b >= SCRL_GLOW_BANDS) b = SCRL_GLOW_BANDS - 1;
            g->bands[y * W + x] = (unsigned char)b;
        }
    g->w = W;
    g->h = H;
}

static struct ScrlGlow scrl_glow_up, scrl_glow_dn;
static TbBool scrl_glow_ready = false;

static void scrl_ensure_glow(void)
{
    if (scrl_glow_ready)
        return;
    scrl_build_glow(0, SCRL_UPL_H, &scrl_glow_up);
    scrl_build_glow(1, SCRL_DNL_H, &scrl_glow_dn);
    scrl_glow_ready = true;
}

static TbBool scrl_mouse_over(long ox, long oy, int upp, int nx, int ny, int w, int h)
{
    long mx = GetMouseX();
    long my = GetMouseY();
    return (mx >= ox + scrl_sc(nx, upp)) && (mx < ox + scrl_sc(nx + w, upp))
        && (my >= oy + scrl_sc(ny, upp)) && (my < oy + scrl_sc(ny + h, upp));
}

static void scrl_draw_glow(long ox, long oy, int upp, int nx, int ny, const struct ScrlGlow *g)
{
    for (int r = 0; r < g->h; r++)
        for (int c = 0; c < g->w; c++)
        {
            unsigned char b = g->bands[r * g->w + c];
            if (b >= SCRL_GLOW_BANDS)
                continue;
            long x0 = ox + scrl_sc(nx + c, upp);
            long x1 = ox + scrl_sc(nx + c + 1, upp);
            long y0 = oy + scrl_sc(ny + r, upp);
            long y1 = oy + scrl_sc(ny + r + 1, upp);
            LbDrawBox(x0, y0, x1 - x0, y1 - y0, scrl_glow_ramp[b]);
        }
}

void gui_vscroll_draw(struct GuiButton *gbtn)
{
    if (gbtn == NULL)
        return;
    int upp = scrl_units_per_px(gbtn);
    long ox = gbtn->scr_pos_x;
    long oy = gbtn->scr_pos_y;
    unsigned short flg = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags = 0;

    /* 1) frame border */
    scrl_draw_frame(ox, oy, upp, SCRL_FRAME_W, SCRL_FRAME_H);
    /* 2) track: black fill between the rails (boundary coords), then the two rails */
    {
        long fx0 = ox + scrl_sc(SCRL_TRACK_X, upp);
        long fx1 = ox + scrl_sc(SCRL_TRACK_X + SCRL_TRACK_W, upp);
        long fy0 = oy + scrl_sc(SCRL_TRACK_Y, upp);
        long fy1 = oy + scrl_sc(SCRL_TRACK_Y + SCRL_TRACK_H, upp);
        LbDrawBox(fx0, fy0, fx1 - fx0, fy1 - fy0, SCRL_FILL_COLOUR);
    }
    scrl_vline(ox, oy, upp, SCRL_SPR_L, SCRL_LRAIL_X, SCRL_TRACK_Y, SCRL_TRACK_Y + SCRL_TRACK_H, SCRL_RAIL_W, SCRL_LEFTLINE_CX, SCRL_LEFTLINE_CY);
    scrl_vline(ox, oy, upp, SCRL_SPR_R, SCRL_RRAIL_X, SCRL_TRACK_Y, SCRL_TRACK_Y + SCRL_TRACK_H, SCRL_RAIL_W, SCRL_RIGHTLINE_CX, SCRL_RIGHTLINE_CY);
    /* 3) arrows */
    scrl_draw_arrow(ox, oy, upp, SCRL_UP_X, SCRL_UP_Y, 0);
    scrl_draw_arrow(ox, oy, upp, SCRL_DN_X, SCRL_DN_Y, 1);
    /* 4) bases (between each arrow and the track) */
    scrl_draw_base_gradient(ox, oy, upp, SCRL_BASE_Y_UP, scrl_base_up_top, scrl_base_up_bot);
    scrl_draw_base_gradient(ox, oy, upp, SCRL_BASE_Y_DN, scrl_base_dn_top, scrl_base_dn_bot);
    /* 5) thumb */
    scrl_draw_thumb(ox, oy, upp, scrl_thumb_dy(upp));

    /* 6) hover glow: light the arrow+base red while the mouse is over a clickable arrow.
     * The hover box is the glow bitmap's own rect, which spans the arrow together with its
     * base - so the whole shape that lights up also reacts to the mouse (hovering just the
     * base used to miss, since the test only covered the arrow's height), and the two stay
     * in step automatically if the art changes an arrow or base height. */
    scrl_ensure_glow();
    {
        long max_off = gui_vscroll_max_offset();
        if ((gui_vscroll_offset > 0)
         && scrl_mouse_over(ox, oy, upp, SCRL_UP_X, SCRL_UP_Y, scrl_glow_up.w, scrl_glow_up.h))
            scrl_draw_glow(ox, oy, upp, SCRL_UP_X, SCRL_UP_Y, &scrl_glow_up);
        else if ((gui_vscroll_offset < max_off)
         && scrl_mouse_over(ox, oy, upp, SCRL_UP_X, SCRL_BASE_Y_DN, scrl_glow_dn.w, scrl_glow_dn.h))
            scrl_draw_glow(ox, oy, upp, SCRL_UP_X, SCRL_BASE_Y_DN, &scrl_glow_dn);
    }

    lbDisplay.DrawFlags = flg;
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
        TbBool over = (rel_x >= 0) && (rel_x < scrl_sc(SCRL_FRAME_W, upp))
                   && (rel_y >= 0) && (rel_y < scrl_sc(SCRL_FRAME_H, upp));
        TbBool click_edge = (lbDisplay.MLeftButton != 0) && !prev_mleft;
        long top = scrl_sc(SCRL_THUMB_TOP, upp);
        long bot = scrl_sc(SCRL_THUMB_BOT, upp);
        long th = scrl_sc(SCRL_THUMB_H, upp);
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
                long tdy = scrl_thumb_dy(upp);
                scrl_drag_grab = ((rel_y >= tdy) && (rel_y < tdy + th)) ? (rel_y - tdy) : (th / 2);
                scrl_dragging = true;
            }
        }
        if (lbDisplay.MLeftButton == 0)
            scrl_dragging = false;
        if (scrl_dragging && (max_off > 0))
        {
            /* Follow the cursor pixel for pixel (no snapping), then derive the slot the
             * list scrolls to - rounded, so it flips at the midpoint between two stops.
             * Deliberately not gated on over: once grabbed, the drag keeps working even
             * if the cursor wanders off the bar sideways. */
            long dy = rel_y - scrl_drag_grab;
            if (dy < top)
                dy = top;
            if (dy > bot)
                dy = bot;
            scrl_drag_dy = dy;
            long travel = bot - top;
            off = (travel > 0) ? (((dy - top) * max_off + travel / 2) / travel) : 0;
        }
    }
    prev_mleft = (lbDisplay.MLeftButton != 0);
    vscroll_apply_offset(off);
    /* If the offset was refused (an edited Save slot must stay visible), pull the thumb
     * back onto the offset that actually applied, so thumb and list never disagree. */
    if (scrl_dragging && (gbtn != NULL) && (gui_vscroll_offset != off))
        scrl_drag_dy = scrl_thumb_dy_for(gui_vscroll_offset, scrl_units_per_px(gbtn));
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
