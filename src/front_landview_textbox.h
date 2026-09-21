/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_landview_textbox.h
 *     Standalone text box overlay for the Land View.
 */
/******************************************************************************/
#ifndef DK_FRONT_LANDVIEW_TEXTBOX_H
#define DK_FRONT_LANDVIEW_TEXTBOX_H

#include "bflib_basics.h"
#include "game_merge.h"

struct TbSpriteSheet;

#define LANDVIEW_TEXTBOX_POS_BOTTOM (-1)

struct LandViewTextBox {
    TbBool active;
    struct TextScrollWindow scroll_window;
    long pos_x;
    long pos_y;
    long width;
    long height;
    const struct TbSpriteSheet *font;
    const unsigned char *text_remap;
    const unsigned char *gui_remap;
    const unsigned char *glass_map;
};

void landview_textbox_init(struct LandViewTextBox *box);
void landview_textbox_set_geometry(struct LandViewTextBox *box, long pos_x, long pos_y, long width, long height);
void landview_textbox_set_text_rendering(struct LandViewTextBox *box, const struct TbSpriteSheet *font, const unsigned char *text_remap);
void landview_textbox_set_gui_remap(struct LandViewTextBox *box, const unsigned char *gui_remap);
void landview_textbox_set_glass_map(struct LandViewTextBox *box, const unsigned char *glass_map);
void landview_textbox_show(struct LandViewTextBox *box, const char *text);
void landview_textbox_hide(struct LandViewTextBox *box);
void landview_textbox_draw(struct LandViewTextBox *box);
TbBool landview_textbox_input(struct LandViewTextBox *box);

#endif
