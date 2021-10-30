/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file gui_draw.c
 *     GUI elements drawing functions.
 * @par Purpose:
 *     On-screen drawing of GUI elements, like buttons, menus and panels.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     20 Jan 2009 - 30 Jan 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "gui_draw.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_video.h"
#include "bflib_sprite.h"
#include "bflib_planar.h"
#include "bflib_vidraw.h"
#include "bflib_sprfnt.h"
#include "bflib_guibtns.h"

#include "front_simple.h"
#include "frontend.h"
#include "custom_sprites.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
char gui_textbuf[TEXT_BUFFER_LENGTH];

/******************************************************************************/
/******************************************************************************/

/******************************************************************************/

int get_bitmap_max_scale(int img_w,int img_h,int rect_w,int rect_h)
{
    int m;
    int w = 0;
    int h = 0;
    for (m=0; m < 5; m++)
    {
        w += img_w;
        h += img_h;
        if (w > rect_w) break;
        if (h > rect_h) break;
    }
    // The image width can't be larger than video resolution
    if (m < 1)
    {
        if (w > lbDisplay.PhysicalScreenWidth)
          return 0;
        m = 1;
    }
    return m;
}

void draw_bar64k(long pos_x, long pos_y, int units_per_px, long width)
{
    if (width < 72*units_per_px/16)
    {
        ERRORLOG("Bar is too small");
        return;
    }
    // Button opening sprite
    struct TbSprite* spr = &button_sprite[1];
    long x = pos_x;
    LbSpriteDrawResized(x, pos_y, units_per_px, spr);
    x += (spr->SWidth * units_per_px + 8) / 16;
    // Button body
    long body_end = pos_x + width - 2 * ((32 * units_per_px + 8) / 16);
    while (x < body_end)
    {
        spr = &button_sprite[2];
        LbSpriteDrawResized(x/pixel_size, pos_y/pixel_size, units_per_px, spr);
        x += spr->SWidth * units_per_px / 16;
    }
    x = body_end;
    spr = &button_sprite[2];
    LbSpriteDrawResized(x/pixel_size, pos_y/pixel_size, units_per_px, spr);
    x += (spr->SWidth * units_per_px + 8) / 16;
    // Button ending sprite
    spr = &button_sprite[3];
    LbSpriteDrawResized(x/pixel_size, pos_y/pixel_size, units_per_px, spr);
}

void draw_lit_bar64k(long pos_x, long pos_y, int units_per_px, long width)
{
    if (width < 32*units_per_px/16)
    {
        ERRORLOG("Bar is too small");
        return;
    }
    // opening sprite
    long x = pos_x;
    struct TbSprite* spr = &button_sprite[7];
    LbSpriteDrawResized(x, pos_y, units_per_px, spr);
    x += (spr->SWidth * units_per_px + 8) / 16;
    // body
    long body_end = pos_x + width - 2 * ((32 * units_per_px + 8) / 16);
    while (x < body_end)
    {
        spr = &button_sprite[8];
        LbSpriteDrawResized(x, pos_y, units_per_px, spr);
        x += (spr->SWidth * units_per_px + 8) / 16;
    }
    x = body_end;
    spr = &button_sprite[8];
    LbSpriteDrawResized(x, pos_y, units_per_px, spr);
    x += (spr->SWidth * units_per_px + 8) / 16;
    // ending sprite
    spr = &button_sprite[9];
    LbSpriteDrawResized(x, pos_y, units_per_px, spr);
}

void draw_slab64k_background(long pos_x, long pos_y, long width, long height)
{
    long i;
    long scr_x = pos_x / pixel_size;
    long scr_y = pos_y / pixel_size;
    long scr_h = height / pixel_size;
    long scr_w = width / pixel_size;
    if (scr_x < 0)
    {
        i = scr_x + width / pixel_size;
        scr_x = 0;
        scr_w = i;
    }
    if (scr_y < 0)
    {
        i = scr_y + scr_h;
        scr_y = 0;
        scr_h = i;
    }
    i = lbDisplay.PhysicalScreenWidth * pixel_size;
    if (scr_x + scr_w > i)
        scr_w = i - scr_x;
    i = MyScreenHeight;
    if (scr_y + scr_h > i)
        scr_h = i - scr_y;
    TbPixel* out = &lbDisplay.WScreen[scr_x + lbDisplay.GraphicsScreenWidth * scr_y];
    for (i=0; scr_h > i; i++)
    {
        TbPixel* inp = &gui_slab[GUI_SLAB_DIMENSION * (i % GUI_SLAB_DIMENSION)];
        if (scr_w >= GUI_SLAB_DIMENSION)
        {
            memcpy(out, inp, GUI_SLAB_DIMENSION);
            int k;
            for (k = GUI_SLAB_DIMENSION; k < scr_w - GUI_SLAB_DIMENSION; k += GUI_SLAB_DIMENSION)
            {
                memcpy(out + k, inp, GUI_SLAB_DIMENSION);
            }
            if (width - k > 0) {
                memcpy(out + k, inp, scr_w - k);
            }
        } else
        {
            memcpy(out, inp, scr_w);
        }
        out += lbDisplay.GraphicsScreenWidth;
    }
}

void draw_slab64k(long pos_x, long pos_y, int units_per_px, long width, long height)
{
    // Draw one pixel more, to make sure we won't get empty area after scaling
    draw_slab64k_background(pos_x, pos_y, width+scale_value_for_resolution_with_upp(1,units_per_px), height+scale_value_for_resolution_with_upp(1,units_per_px));
    struct TbSprite* spr = &button_sprite[206];
    int bs_units_per_spr = calculate_relative_upp(16, units_per_px, spr->SWidth);
    int border_shift = scale_value_for_resolution_with_upp(6,units_per_px);
    int i;
    int i_increment = units_per_px;
    for (i = i_increment - border_shift; i < width-2*border_shift; i += i_increment)
    {
        spr = &button_sprite[210];
        LbSpriteDrawResized(pos_x + i, pos_y - border_shift, bs_units_per_spr, spr);
        spr = &button_sprite[211];
        LbSpriteDrawResized(pos_x + i, pos_y + height, bs_units_per_spr, spr);
    }
    for (i = i_increment - border_shift; i < height-2*border_shift; i += i_increment)
    {
        spr = &button_sprite[212];
        LbSpriteDrawResized(pos_x - border_shift, pos_y + i, bs_units_per_spr, spr);
        spr = &button_sprite[213];
        LbSpriteDrawResized(pos_x + width, pos_y + i, bs_units_per_spr, spr);
    }
    spr = &button_sprite[206];
    LbSpriteDrawResized(pos_x - border_shift, pos_y - border_shift, bs_units_per_spr, spr);
    spr = &button_sprite[207];
    LbSpriteDrawResized(pos_x + width - 2*border_shift, pos_y - border_shift, bs_units_per_spr, spr);
    spr = &button_sprite[208];
    LbSpriteDrawResized(pos_x - border_shift, pos_y + height - 2*border_shift, bs_units_per_spr, spr);
    spr = &button_sprite[209];
    LbSpriteDrawResized(pos_x + width - 2*border_shift, pos_y + height - 2*border_shift, bs_units_per_spr, spr);
}

void draw_ornate_slab64k(long pos_x, long pos_y, int units_per_px, long width, long height)
{
    draw_slab64k_background(pos_x, pos_y, width, height);
    struct TbSprite* spr = &button_sprite[10];
    int bs_units_per_spr = 128*units_per_px/spr->SWidth;
    int i;
    for (i=10*units_per_px/16; i < width-12*units_per_px/16; i+=32*units_per_px/16)
    {
        spr = &button_sprite[13];
        LbSpriteDrawResized(pos_x + i, pos_y - 4*units_per_px/16, bs_units_per_spr, spr);
        spr = &button_sprite[18];
        LbSpriteDrawResized(pos_x + i, pos_y + height, bs_units_per_spr, spr);
    }
    for (i=10*units_per_px/16; i < height-16*units_per_px/16; i+=32*units_per_px/16)
    {
        spr = &button_sprite[15];
        LbSpriteDrawResized(pos_x - 4*units_per_px/16, pos_y + i, bs_units_per_spr, spr);
        spr = &button_sprite[16];
        LbSpriteDrawResized(pos_x + width, pos_y + i, bs_units_per_spr, spr);
    }
    spr = &button_sprite[12];
    LbSpriteDrawResized(pos_x - 4*units_per_px/16, pos_y - 4*units_per_px/16, bs_units_per_spr, spr);
    spr = &button_sprite[14];
    LbSpriteDrawResized(pos_x + width - 28*units_per_px/16, pos_y - 4*units_per_px/16, bs_units_per_spr, spr);
    spr = &button_sprite[17];
    LbSpriteDrawResized(pos_x - 4*units_per_px/16, pos_y + height - 28*units_per_px/16, bs_units_per_spr, spr);
    spr = &button_sprite[19];
    LbSpriteDrawResized(pos_x + width - 28*units_per_px/16, pos_y + height - 28*units_per_px/16, bs_units_per_spr, spr);
    spr = &button_sprite[10];
    LbSpriteDrawResized(pos_x - 32*units_per_px/16, pos_y - 14*units_per_px/16, bs_units_per_spr, spr);
    spr = &button_sprite[11];
    LbSpriteDrawResized(pos_x - 34*units_per_px/16, pos_y + height - 78*units_per_px/16, bs_units_per_spr, spr);
    lbDisplay.DrawFlags |= Lb_SPRITE_FLIP_HORIZ;
    spr = &button_sprite[10];
    LbSpriteDrawResized(pos_x + width - 96*units_per_px/16, pos_y - 14*units_per_px/16, bs_units_per_spr, spr);
    spr = &button_sprite[11];
    LbSpriteDrawResized(pos_x + width - 92*units_per_px/16, pos_y + height - 78*units_per_px/16, bs_units_per_spr, spr);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
}

void draw_ornate_slab_outline64k(long pos_x, long pos_y, int units_per_px, long width, long height)
{
    struct TbSprite* spr = &button_sprite[10];
    int bs_units_per_spr = 128*units_per_px/spr->SWidth;
    long x = pos_x;
    long y = pos_y;
    int i;
    for (i=10*units_per_px/16; i < width - 12*units_per_px/16; i+=32*units_per_px/16)
    {
        spr = &button_sprite[13];
        LbSpriteDrawResized(pos_x + i, pos_y - 4*units_per_px/16, bs_units_per_spr, spr);
        spr = &button_sprite[18];
        LbSpriteDrawResized(pos_x + i, pos_y + height, bs_units_per_spr, spr);
    }
    spr = button_sprite;
    for (i=10*units_per_px/16; i < height - 16*units_per_px/16; i+=32*units_per_px/16)
    {
        spr = &button_sprite[15];
        LbSpriteDrawResized(x - 4*units_per_px/16, y + i, bs_units_per_spr, spr);
        spr = &button_sprite[16];
        LbSpriteDrawResized(x + width, y + i, bs_units_per_spr, spr);
    }
    spr = &button_sprite[12];
    LbSpriteDrawResized(x - 4*units_per_px/16,          y - 4*units_per_px/16,           bs_units_per_spr, spr);
    spr = &button_sprite[14];
    LbSpriteDrawResized(x + width - 28*units_per_px/16, y - 4*units_per_px/16,           bs_units_per_spr, spr);
    spr = &button_sprite[17];
    LbSpriteDrawResized(x - 4*units_per_px/16,          y + height - 28*units_per_px/16, bs_units_per_spr, spr);
    spr = &button_sprite[19];
    LbSpriteDrawResized(x + width - 28*units_per_px/16, y + height - 28*units_per_px/16, bs_units_per_spr, spr);
    spr = &button_sprite[10];
    LbSpriteDrawResized(x - 32*units_per_px/16,         y - 14*units_per_px/16,          bs_units_per_spr, spr);
    spr = &button_sprite[11];
    LbSpriteDrawResized(x - 34*units_per_px/16,         y + height - 78*units_per_px/16, bs_units_per_spr, spr);
    lbDisplay.DrawFlags |= Lb_SPRITE_FLIP_HORIZ;
    spr = &button_sprite[10];
    LbSpriteDrawResized(x + width - 96*units_per_px/16, y - 14*units_per_px/16,          bs_units_per_spr, spr);
    spr = &button_sprite[11];
    LbSpriteDrawResized(x + width - 92*units_per_px/16, y + height - 78*units_per_px/16, bs_units_per_spr, spr);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_FLIP_HORIZ;
}

void draw_round_slab64k(long pos_x, long pos_y, int units_per_px, long width, long height)
{
    unsigned short drwflags_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_SPRITE_OUTLINE;
    lbDisplay.DrawFlags |= Lb_SPRITE_TRANSPAR4;
    LbDrawBox(pos_x + 4*units_per_px/16, pos_y + 4*units_per_px/16, width - 8*units_per_px/16, height - 8*units_per_px/16, 1);
    lbDisplay.DrawFlags &= ~Lb_SPRITE_TRANSPAR4;
    int x;
    int y;
    struct TbSprite* spr = &gui_panel_sprites[242];
    int ps_units_per_spr = 26*units_per_px/spr->SWidth;
    long i;
    for (i = 0; i < width - 68*units_per_px/16; i += 26*units_per_px/16)
    {
        x = pos_x + i + 34*units_per_px/16;
        y = pos_y;
        spr = &gui_panel_sprites[242];
        LbSpriteDrawResized(x, y, ps_units_per_spr, spr);
        y += height - 4*units_per_px/16;
        spr = &gui_panel_sprites[248];
        LbSpriteDrawResized(x, y, ps_units_per_spr, spr);
    }
    for (i = 0; i < height - 56*units_per_px/16; i += 20*units_per_px/16)
    {
        x = pos_x;
        y = pos_y + i + 28*units_per_px/16;
        spr = &gui_panel_sprites[244];
        LbSpriteDrawResized(x, y, ps_units_per_spr, spr);
        x += width - 4*units_per_px/16;
        spr = &gui_panel_sprites[246];
        LbSpriteDrawResized(x, y, ps_units_per_spr, spr);
    }
    x = pos_x + width - 34*units_per_px/16;
    y = pos_y + height - 28*units_per_px/16;
    spr = &gui_panel_sprites[241];
    LbSpriteDrawResized(pos_x, pos_y, ps_units_per_spr, spr);
    spr = &gui_panel_sprites[243];
    LbSpriteDrawResized(x,     pos_y, ps_units_per_spr, spr);
    spr = &gui_panel_sprites[247];
    LbSpriteDrawResized(pos_x, y,     ps_units_per_spr, spr);
    spr = &gui_panel_sprites[249];
    LbSpriteDrawResized(x,     y,     ps_units_per_spr, spr);
    lbDisplay.DrawFlags = drwflags_mem;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one panel sprite.
 * Uses sprite height as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_gui_panel_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    struct TbSprite* spr = &gui_panel_sprites[spridx];
    if (spr->SHeight < 1)
        return 16;
    int units_per_px = ((gbtn->height * fraction / 100) * 16 + spr->SHeight / 2) / spr->SHeight;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one panel sprite.
 * Uses sprite width as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_gui_panel_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    struct TbSprite* spr = &gui_panel_sprites[spridx];
    if (spr->SWidth < 1)
        return 16;
    int units_per_px = ((gbtn->width * fraction / 100) * 16 + spr->SWidth / 2) / spr->SWidth;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one button sprite.
 * Uses sprite height as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_button_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    const struct TbSprite* spr = get_button_sprite(spridx);
    if (spr->SHeight < 1)
        return 16;
    int units_per_px = ((gbtn->height * fraction / 100) * 16 + spr->SHeight / 2) / spr->SHeight;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one button sprite.
 * Uses sprite width as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_button_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    const struct TbSprite* spr = get_button_sprite(spridx);
    if (spr->SWidth < 1)
        return 16;
    int units_per_px = ((gbtn->width * fraction / 100) * 16 + spr->SWidth / 2) / spr->SWidth;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one sprite.
 * Uses sprite height as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_frontend_sprite_height_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    const struct TbSprite* spr = get_frontend_sprite(spridx);
    if (spr->SHeight < 1)
        return 16;
    int units_per_px = ((gbtn->height * fraction / 100) * 16 + spr->SHeight / 2) / spr->SHeight;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/**
 * Returns units-per-pixel to be used for drawing given GUI button, assuming it consists of one sprite.
 * Uses sprite width as constant factor.
 * @param gbtn
 * @param spridx
 * @return
 */
int simple_frontend_sprite_width_units_per_px(const struct GuiButton *gbtn, long spridx, int fraction)
{
    const struct TbSprite* spr = get_frontend_sprite(spridx);
    if (spr->SWidth < 1)
        return 16;
    int units_per_px = ((gbtn->width * fraction / 100) * 16 + spr->SWidth / 2) / spr->SWidth;
    if (units_per_px < 1)
        units_per_px = 1;
    return units_per_px;
}

/** Draws a string on GUI button.
 *  Note that the source text buffer may be damaged by this function.
 * @param gbtn Button to draw text on.
 * @param base_width Width of the button before scaling.
 * @param text Text to be displayed.
 */
void draw_button_string(struct GuiButton *gbtn, int base_width, const char *text)
{
    static unsigned char cursor_type = 0;
    unsigned long flgmem = lbDisplay.DrawFlags;
    long cursor_pos = -1;
    static char dtext[TEXT_BUFFER_LENGTH];
    LbStringCopy(dtext, text, TEXT_BUFFER_LENGTH);
    if ((gbtn->gbtype == LbBtnT_EditBox) && (gbtn == input_button))
    {
        cursor_type++;
        if ((cursor_type & 0x02) == 0)
          cursor_pos = input_field_pos;
        LbLocTextStringConcat(dtext, " ", TEXT_BUFFER_LENGTH);
        lbDisplay.DrawColour = LbTextGetFontFaceColor();
        lbDisplayEx.ShadowColour = LbTextGetFontBackColor();
    }
    LbTextSetJustifyWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width);
    LbTextSetClipWindow(gbtn->scr_pos_x, gbtn->scr_pos_y, gbtn->width, gbtn->height);
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;// | Lb_TEXT_UNDERLNSHADOW;
    if (cursor_pos >= 0) {
        // Mind the order, 'cause inserting makes positions shift
        LbLocTextStringInsert(dtext, "\x0B", cursor_pos+1, TEXT_BUFFER_LENGTH);
        LbLocTextStringInsert(dtext, "\x0B", cursor_pos, TEXT_BUFFER_LENGTH);
    }
    int units_per_px = (gbtn->width * 16 + base_width / 2) / base_width;
    int tx_units_per_px = units_per_px * 22 / LbTextLineHeight();
    unsigned long w = 4 * units_per_px / 16;
    unsigned long h = (gbtn->height - text_string_height(tx_units_per_px, dtext)) / 2 - 3 * units_per_px / 16;
    LbTextDrawResized(w, h, tx_units_per_px, dtext);
    LbTextSetJustifyWindow(0, 0, LbGraphicsScreenWidth());
    LbTextSetClipWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
    lbDisplay.DrawFlags = flgmem;
}

void draw_message_box_at(long startx, long starty, long box_width, long box_height, long spritesx, long spritesy)
{
    struct TbSprite *spr;
    long n;

    // Draw top line of sprites
    long x = startx;
    long y = starty;
    {
        spr = &frontend_sprite[25];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    for (n=0; n < spritesx; n++)
    {
        spr = &frontend_sprite[(n % 4) + 26];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    x = startx;
    {
        spr = &frontend_sprite[25];
        x += spr->SWidth * units_per_pixel / 16;
    }
    for (n=0; n < spritesx; n++)
    {
        spr = &frontend_sprite[(n % 4) + 26];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    {
        spr = &frontend_sprite[30];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
    }
    // Draw centered line of sprites
    spr = &frontend_sprite[25];
    x = startx;
    y += spr->SHeight * units_per_pixel / 16;
    {
        spr = &frontend_sprite[40];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    for (n=0; n < spritesx; n++)
    {
        spr = &frontend_sprite[(n % 4) + 41];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    {
        spr = &frontend_sprite[45];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
    }
    // Draw bottom line of sprites
    spr = &frontend_sprite[40];
    x = startx;
    y += spr->SHeight * units_per_pixel / 16;
    {
        spr = &frontend_sprite[47];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    for (n=0; n < spritesx; n++)
    {
        spr = &frontend_sprite[(n % 4) + 48];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
        x += spr->SWidth * units_per_pixel / 16;
    }
    {
        spr = &frontend_sprite[52];
        LbSpriteDrawResized(x, y, units_per_pixel, spr);
    }
}

TbBool draw_text_box(const char *text)
{
    long spritesy;
    long spritesx;
    LbTextSetFont(frontend_font[1]);
    long n = LbTextStringWidth(text);
    if (n < (4*108)) {
        spritesy = 1;
        spritesx = n / 108;
    } else {
        spritesx = 4;
        spritesy = n / (3*108);
    }
    if (spritesy > 4) {
      ERRORLOG("Text too long for error box");
    }
    if (spritesx < 2) {
        spritesx = 2;
    } else
    if (spritesx > 4) {
        spritesx = 4;
    }
    long box_width = (108 * spritesx + 18) * units_per_pixel / 16;
    long box_height = 92 * units_per_pixel / 16;
    long startx = (lbDisplay.PhysicalScreenWidth - box_width) / 2;
    long starty = (lbDisplay.PhysicalScreenHeight - box_height) / 2;
    draw_message_box_at(startx, starty, box_width, box_height, spritesx, spritesy);
    // Draw the text inside box
    lbDisplay.DrawFlags = Lb_TEXT_HALIGN_CENTER;
    int tx_units_per_px = ((box_height / 4) * 13 / 11) * 16 / LbTextLineHeight();
    LbTextSetWindow(startx, starty, box_width, box_height);
    n = LbTextLineHeight() * tx_units_per_px / 16;
    return LbTextDrawResized(0, (box_height - spritesy * n) / 2, tx_units_per_px, text);
}

int scroll_box_get_units_per_px(struct GuiButton *gbtn)
{
    int width = 0;
    int spridx = 40;
    const struct TbSprite* spr = get_frontend_sprite(spridx);
    for (int i = 6; i > 0; i--)
    {
        width += spr->SWidth;
        spr++;
    }
    return (gbtn->width * 16 + 8) / width;
}

void draw_scroll_box(struct GuiButton *gbtn, int units_per_px, int num_rows)
{
    const struct TbSprite *spr;
    int pos_x;
    int i;
    lbDisplay.DrawFlags = 0;
    int pos_y = gbtn->scr_pos_y;
    { // First row
        pos_x = gbtn->scr_pos_x;
        spr = &frontend_sprite[25];
        for (i = 6; i > 0; i--)
        {
            LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
            pos_x += spr->SWidth * units_per_px / 16;
            spr++;
        }
        spr = &frontend_sprite[25];
        pos_y += spr->SHeight * units_per_px / 16;
    }
    // Further rows
    while (num_rows > 0)
    {
        int spridx = 40;
        if (num_rows < 3)
          spridx = 33;
        spr = get_frontend_sprite(spridx);
        pos_x = gbtn->scr_pos_x;
        for (i = 6; i > 0; i--)
        {
            LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
            pos_x += spr->SWidth * units_per_px / 16;
            spr++;
        }
        spr = &frontend_sprite[spridx];
        pos_y += spr->SHeight * units_per_px / 16;
        int delta = 3;
        if (num_rows < 3)
            delta = 1;
        num_rows -= delta;
    }
    // Last row
    spr = &frontend_sprite[47];
    pos_x = gbtn->scr_pos_x;
    for (i = 6; i > 0; i--)
    {
        LbSpriteDrawResized(pos_x, pos_y, units_per_px, spr);
        pos_x += spr->SWidth * units_per_px / 16;
        spr++;
    }
}

void draw_gui_panel_sprite_left(long x, long y, int units_per_px, long spridx)
{
    if ((spridx <= 0) || (spridx >= num_icons_total))
      return;
    struct TbSprite* spr = &gui_panel_sprites[spridx];
    LbSpriteDrawResized(x, y, units_per_px, spr);
}

void draw_gui_panel_sprite_rmleft(long x, long y, int units_per_px, long spridx, unsigned long remap)
{
    if ((spridx <= 0) || (spridx >= num_icons_total))
      return;
    struct TbSprite* spr = &gui_panel_sprites[spridx];
    LbSpriteDrawResizedRemap(x, y, units_per_px, spr, &pixmap.fade_tables[remap*256]);
}

void draw_gui_panel_sprite_ocleft(long x, long y, int units_per_px, long spridx, TbPixel color)
{
    if ((spridx <= 0) || (spridx > num_icons_total))
      return;
    struct TbSprite* spr = &gui_panel_sprites[spridx];
    LbSpriteDrawResizedOneColour(x, y, units_per_px, spr, color);
}

void draw_gui_panel_sprite_centered(long x, long y, int units_per_px, long spridx)
{
    if ((spridx <= 0) || (spridx > num_icons_total))
      return;
    struct TbSprite* spr = &gui_panel_sprites[spridx];
    x -= ((spr->SWidth*units_per_px/16) >> 1);
    y -= ((spr->SHeight*units_per_px/16) >> 1);
    LbSpriteDrawResized(x, y, units_per_px, spr);
}

void draw_gui_panel_sprite_occentered(long x, long y, int units_per_px, long spridx, TbPixel color)
{
    if ((spridx <= 0) || (spridx > num_icons_total))
      return;
    struct TbSprite* spr = &gui_panel_sprites[spridx];
    x -= ((spr->SWidth*units_per_px/16) >> 1);
    y -= ((spr->SHeight*units_per_px/16) >> 1);
    LbSpriteDrawResizedOneColour(x, y, units_per_px, spr, color);
}

void draw_button_sprite_left(long x, long y, int units_per_px, long spridx)
{
    const struct TbSprite* spr = get_button_sprite(spridx);
    LbSpriteDrawResized(x, y, units_per_px, spr);
}

void draw_button_sprite_rmleft(long x, long y, int units_per_px, long spridx, unsigned long remap)
{
    const struct TbSprite* spr = get_button_sprite(spridx);
    LbSpriteDrawResizedRemap(x, y, units_per_px, spr, &pixmap.fade_tables[remap*256]);
}

void draw_frontend_sprite_left(long x, long y, int units_per_px, long spridx)
{
    const struct TbSprite* spr = get_frontend_sprite(spridx);
    LbSpriteDrawResized(x, y, units_per_px, spr);
}

void draw_string64k(long x, long y, int units_per_px, const char * text)
{
    unsigned short drwflags_mem = lbDisplay.DrawFlags;
    lbDisplay.DrawFlags &= ~Lb_TEXT_ONE_COLOR;
    LbTextDrawResized(x, y, units_per_px, text);
    lbDisplay.DrawFlags = drwflags_mem;
}

TbBool frontmenu_copy_background_at(const struct TbRect *bkgnd_area, int units_per_px)
{
    int img_width = 640;
    int img_height = 480;
    const unsigned char *srcbuf = frontend_background;
    // Only 8bpp supported for now
    if (LbGraphicsScreenBPP() != 8)
        return false;
    // Do the drawing
    copy_raw8_image_buffer(lbDisplay.WScreen,LbGraphicsScreenWidth(),LbGraphicsScreenHeight(),
        img_width*units_per_px/16,img_height*units_per_px/16,bkgnd_area->left,bkgnd_area->top,srcbuf,img_width,img_height);
    // Burning candle flames
    return true;
}

long get_frontmenu_background_area_rect(int rect_x, int rect_y, int rect_w, int rect_h, struct TbRect *bkgnd_area)
{
    int img_width = 640;
    int img_height = 480;
    // Parchment bitmap scaling
    int units_per_px = max(16 * rect_w / img_width, 16 * rect_h / img_height);
    int units_per_px_max = min(16 * 7 * rect_w / (6 * img_width), 16 * 4 * rect_h / (3 * img_height));
    if (units_per_px > units_per_px_max)
        units_per_px = units_per_px_max;
    // The image width can't be larger than video resolution
    if (units_per_px < 1) {
        units_per_px = 1;
    }
    // Set rectangle coords
    bkgnd_area->left = rect_x + (rect_w-units_per_px*img_width/16)/2;
    bkgnd_area->top = rect_y + (rect_h-units_per_px*img_height/16)/2;
    if (bkgnd_area->top < 0) bkgnd_area->top = 0;
    bkgnd_area->right = bkgnd_area->left + units_per_px*img_width/16;
    bkgnd_area->bottom = bkgnd_area->top + units_per_px*img_height/16;
    if (bkgnd_area->bottom > rect_y+rect_h) bkgnd_area->bottom = rect_y+rect_h;
    return units_per_px;
}

/**
 * Draws menu background.
 */
void draw_frontmenu_background(int rect_x,int rect_y,int rect_w,int rect_h)
{
    // Validate parameters with video mode
    TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(LbScreenActiveMode());
    if (rect_w == POS_AUTO)
      rect_w = mdinfo->Width-rect_x;
    if (rect_h == POS_AUTO)
      rect_h = mdinfo->Height-rect_y;
    if (rect_w<0) rect_w=0;
    if (rect_h<0) rect_h=0;
    // Get background area rectangle
    struct TbRect bkgnd_area;
    int units_per_px = get_frontmenu_background_area_rect(rect_x, rect_y, rect_w, rect_h, &bkgnd_area);
    // Draw it
    frontmenu_copy_background_at(&bkgnd_area, units_per_px);
    SYNCDBG(9,"Done");
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
