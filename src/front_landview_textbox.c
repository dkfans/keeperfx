/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_landview_textbox.c
 *     Standalone text box overlay for the Land View.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "front_landview_textbox.h"

#include <string.h>

#include "bflib_sprfnt.h"
#include "bflib_planar.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "custom_sprites.h"
#include "frontend.h"
#include "gui_draw.h"
#include "kjm_input.h"
#include "kfx/renderer/RendererManager.h"
#include "sprites.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LANDVIEW_TEXTBOX_DEFAULT_X       160
#define LANDVIEW_TEXTBOX_DEFAULT_WIDTH   480
#define LANDVIEW_TEXTBOX_DEFAULT_HEIGHT  86

struct LandViewTextBoxGeometry {
    long pos_x;
    long pos_y;
    long width;
    long height;
    struct TbRect text;
    struct TbRect close;
    struct TbRect up;
    struct TbRect down;
};

static long textbox_scale(long value)
{
    return scale_value_landview(value);
}

static void textbox_set_rect(struct TbRect *rect, long left, long top, long width, long height)
{
    rect->left = left;
    rect->top = top;
    rect->right = left + width;
    rect->bottom = top + height;
}

static void landview_textbox_geometry(const struct LandViewTextBox *box, struct LandViewTextBoxGeometry *geo)
{
    geo->width = textbox_scale(box->width);
    geo->height = textbox_scale(box->height);
    geo->pos_x = textbox_scale(box->pos_x);
    geo->pos_y = (box->pos_y == LANDVIEW_TEXTBOX_POS_BOTTOM) ?
        lbDisplay.PhysicalScreenHeight - geo->height : textbox_scale(box->pos_y);

    /* These values are the text_info_buttons offsets and dimensions. */
    textbox_set_rect(&geo->text, geo->pos_x + textbox_scale(40), geo->pos_y + textbox_scale(4),
        geo->width - textbox_scale(80), geo->height - textbox_scale(8));
    textbox_set_rect(&geo->close, geo->pos_x + textbox_scale(4), geo->pos_y + geo->height - textbox_scale(30),
        textbox_scale(30), textbox_scale(24));
    textbox_set_rect(&geo->up, geo->pos_x + textbox_scale(box->width - 34), geo->pos_y + textbox_scale(4),
        textbox_scale(30), textbox_scale(24));
    textbox_set_rect(&geo->down, geo->pos_x + textbox_scale(box->width - 34), geo->pos_y + textbox_scale(box->height - 30),
        textbox_scale(30), textbox_scale(24));
}

static TbBool textbox_point_in_rect(const struct TbRect *rect, long pos_x, long pos_y)
{
    return (pos_x >= rect->left) && (pos_x < rect->right) &&
        (pos_y >= rect->top) && (pos_y < rect->bottom);
}

static TbBool textbox_can_scroll_up(const struct LandViewTextBox *box)
{
    return (box->scroll_window.start_y < 0);
}

static TbBool textbox_can_scroll_down(const struct LandViewTextBox *box)
{
    return (box->scroll_window.window_height - box->scroll_window.text_height + 2 < box->scroll_window.start_y);
}

static void textbox_draw_button(const struct LandViewTextBox *box, const struct TbRect *rect, long sprite_idx, TbBool enabled)
{
    const struct TbSprite *standard_sprite = get_panel_sprite(sprite_idx + 1);
    TbBool pressed = enabled && left_button_held && textbox_point_in_rect(rect, GetMouseX(), GetMouseY());
    const struct TbSprite *sprite = get_panel_sprite(sprite_idx + (pressed ? 0 : 1));
    /* Match gui_area_new_normal_button(): fit the standard sprite by width and
       retain its native aspect ratio instead of stretching it to the hit box. */
    int units_per_px = ((rect->right - rect->left) * 16 + standard_sprite->SWidth / 2) / standard_sprite->SWidth;
    if (box->gui_remap != NULL)
        LbSpriteDrawResizedRemap(rect->left, rect->top, units_per_px, sprite, box->gui_remap);
    else
        LbSpriteDrawResized(rect->left, rect->top, units_per_px, sprite);
}

void landview_textbox_init(struct LandViewTextBox *box)
{
    memset(box, 0, sizeof(*box));
    box->pos_x = LANDVIEW_TEXTBOX_DEFAULT_X;
    box->pos_y = LANDVIEW_TEXTBOX_POS_BOTTOM;
    box->width = LANDVIEW_TEXTBOX_DEFAULT_WIDTH;
    box->height = LANDVIEW_TEXTBOX_DEFAULT_HEIGHT;
}

void landview_textbox_set_geometry(struct LandViewTextBox *box, long pos_x, long pos_y, long width, long height)
{
    TbBool size_changed = (box->width != width) || (box->height != height);
    box->pos_x = pos_x;
    box->pos_y = pos_y;
    box->width = width;
    box->height = height;
    /* Geometry may be supplied every frame. Do not discard a pending scroll action. */
    if (size_changed)
        box->scroll_window.text_height = 0;
}

void landview_textbox_set_text_rendering(struct LandViewTextBox *box, const struct TbSpriteSheet *font, const unsigned char *text_remap)
{
    box->font = font;
    box->text_remap = text_remap;
}

void landview_textbox_set_gui_remap(struct LandViewTextBox *box, const unsigned char *gui_remap)
{
    box->gui_remap = gui_remap;
}

void landview_textbox_set_glass_map(struct LandViewTextBox *box, const unsigned char *glass_map)
{
    box->glass_map = glass_map;
}

void landview_textbox_show(struct LandViewTextBox *box, const char *text)
{
    snprintf(box->scroll_window.text, sizeof(box->scroll_window.text), "%s", (text != NULL) ? text : "");
    box->scroll_window.start_y = 0;
    box->scroll_window.action = 0;
    box->scroll_window.text_height = 0;
    box->scroll_window.window_height = 0;
    box->active = true;
}

void landview_textbox_hide(struct LandViewTextBox *box)
{
    box->active = false;
}

void landview_textbox_draw(struct LandViewTextBox *box)
{
    if (!box->active)
        return;

    struct LandViewTextBoxGeometry geo;
    landview_textbox_geometry(box, &geo);
    unsigned short flags = RendererGetDrawFlags();
    unsigned char *glass_map = lbDisplay.GlassMap;
    if (box->glass_map != NULL)
        lbDisplay.GlassMap = (unsigned char *)box->glass_map;
    draw_round_slab64k_remap(geo.pos_x, geo.pos_y, units_per_pixel_landview, geo.width, geo.height, ROUNDSLAB64K_LIGHT, box->gui_remap);
    lbDisplay.GlassMap = glass_map;
    textbox_draw_button(box, &geo.close, GPS_message_message_btn_accept_act, true);
    textbox_draw_button(box, &geo.up, GPS_message_message_btn_up_act, textbox_can_scroll_up(box));
    textbox_draw_button(box, &geo.down, GPS_message_message_btn_down_act, textbox_can_scroll_down(box));

    if (box->font != NULL)
        LbTextSetFont(box->font);
    LbTextSetRemap(box->text_remap);
    draw_scrolling_text_at(geo.text.left, geo.text.top, geo.text.right - geo.text.left,
        geo.text.bottom - geo.text.top, &box->scroll_window, box->scroll_window.text);
    LbTextSetRemap(NULL);
    RendererSetDrawFlags(flags);
}

TbBool landview_textbox_input(struct LandViewTextBox *box)
{
    if (!box->active)
        return false;

    struct LandViewTextBoxGeometry geo;
    struct TbRect panel;
    landview_textbox_geometry(box, &geo);
    textbox_set_rect(&panel, geo.pos_x, geo.pos_y, geo.width, geo.height);
    long mouse_x = GetMouseX();
    long mouse_y = GetMouseY();
    TbBool over_panel = textbox_point_in_rect(&panel, mouse_x, mouse_y);
    TbBool over_up = textbox_point_in_rect(&geo.up, mouse_x, mouse_y);
    TbBool over_down = textbox_point_in_rect(&geo.down, mouse_x, mouse_y);
    TbBool over_close = textbox_point_in_rect(&geo.close, mouse_x, mouse_y);
    TbBool wheel_consumed = false;

    if (wheel_scrolled_up)
    {
        box->scroll_window.action = 1;
        wheel_consumed = true;
    }
    else if (wheel_scrolled_down)
    {
        box->scroll_window.action = 2;
        wheel_consumed = true;
    }

    if (left_button_clicked && (over_close || over_up || over_down || over_panel))
    {
        left_button_clicked = 0;
        if (over_close)
            landview_textbox_hide(box);
        else if (over_up && textbox_can_scroll_up(box))
            box->scroll_window.action = 1;
        else if (over_down && textbox_can_scroll_down(box))
            box->scroll_window.action = 2;
        return true;
    }
    return over_panel || wheel_consumed;
}

#ifdef __cplusplus
}
#endif
