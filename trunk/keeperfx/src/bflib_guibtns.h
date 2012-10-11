/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_guibtns.h
 *     Header file for bflib_guibtns.c.
 * @par Purpose:
 *     GUI Buttons support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 Nov 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_GUIBTNS_H
#define BFLIB_GUIBTNS_H

#include "bflib_basics.h"
#include "globals.h"

#include "bflib_string.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct GuiButton;
struct GuiMenu;
struct GuiBox;
struct GuiBoxOption;

#define STRINGS_MAX       1024
#define DK_STRINGS_MAX     941
#define INPUT_FIELD_LEN     40
#define TOOLTIP_MAX_LEN   2048

// Type definitions
enum TbButtonType {
    Lb_CYCLEBTN  =  2,
    Lb_RADIOBTN  =  3,
    Lb_SLIDER    =  4,
    Lb_EDITBTN   =  5,
};

enum TbButtonFlags {
    LbBtnF_Unknown01  =  0x01,
    LbBtnF_Unknown02  =  0x02,
    LbBtnF_Unknown04  =  0x04,
    LbBtnF_Unknown08  =  0x08,
    LbBtnF_Unknown10  =  0x10,
    LbBtnF_Unknown20  =  0x20,
    LbBtnF_Unknown40  =  0x40,
    LbBtnF_Unknown80  =  0x80,
};

union GuiVariant {
    long lval;
    long *lptr;
    char *str;
};

typedef long (*Gf_OptnBox_4Callback)(struct GuiBox *, struct GuiBoxOption *, unsigned char, long *);
typedef long (*Gf_OptnBox_3Callback)(struct GuiBox *, struct GuiBoxOption *, long *);
typedef void (*Gf_Btn_Callback)(struct GuiButton *gbtn);
typedef void (*Gf_Mnu_Callback)(struct GuiMenu *gmnu);

struct GuiBoxOption {
       const char *label;
       unsigned char numfield_4;
       Gf_OptnBox_3Callback active_cb;
       Gf_OptnBox_4Callback callback;
       long field_D;
       long field_11;
       long field_15;
       long field_19;
       long field_1D;
       long field_21;
       char active;
       char field_26;
};

struct GuiBox {
char field_0;
    short field_1;
    long pos_x;
    long pos_y;
    long width;
    long height;
    struct GuiBoxOption *optn_list;
    struct GuiBox *next_box;
    struct GuiBox *prev_box;
};

struct DraggingBox {
  struct GuiBox *gbox;
  long start_x;
  long start_y;
};

struct GuiButtonInit {
    char field_0;
    char field_1;
    char field_2;
    short field_3;
    short field_5;
    Gf_Btn_Callback push_call;
    Gf_Btn_Callback focus_call;
    Gf_Btn_Callback field_F;
    short field_13;
    short field_15;
    short field_17;
    short field_19;
    short field_1B;
    short width;
    short height;
    Gf_Btn_Callback draw_call;
    short field_25;
    short field_27;
    struct GuiMenu *field_29;
    union GuiVariant field_2D;
    char field_31;
    char field_32;
    Gf_Btn_Callback maintain_call;
};

struct GuiButton {
       unsigned char flags;
       unsigned char field_1;
       unsigned char field_2;
       char gmenu_idx;
       short id_num;
       unsigned char gbtype;
       Gf_Btn_Callback click_event;
       Gf_Btn_Callback rclick_event;
       Gf_Btn_Callback field_F;
       Gf_Btn_Callback field_13;
       Gf_Btn_Callback field_17;
       unsigned short field_1B; // definitely a word, not two bytes
       short scr_pos_x;
       short scr_pos_y;
       short pos_x;
       short pos_y;
       short width;
       short height;
       short field_29;
       short tooltip_id;
       unsigned short field_2D;
       struct GuiMenu *field_2F;
       unsigned long *content;
       unsigned short slide_val; // slider value, scaled 0..255
};

struct GuiMenu {
      char ident;
      unsigned char visible;
      short fade_time;
      struct GuiButtonInit *buttons;
      short pos_x;
      short pos_y;
      short width;
      short height;
      Gf_Mnu_Callback draw_cb;
      char number;
      struct GuiMenu *menu_init;
      Gf_Mnu_Callback create_cb;
      unsigned char flgfield_1D;
      unsigned char flgfield_1E;
      char field_1F;
};

struct ToolTipBox {
      unsigned char flags;
      char text[TOOLTIP_MAX_LEN];
      struct GuiButton *gbutton;
      void *target;
      unsigned char field_809;
      short pos_x;
      short pos_y;
};

struct FrontEndButtonData {
        unsigned short capstr_idx;
        unsigned char font_index;
};

struct EventTypeInfo { //sizeof=0x10
    int field_0;
    unsigned short tooltip_id;
    unsigned short msgstr_id;
    int field_8;
    int field_C;
};



#pragma pack()
/******************************************************************************/
extern char *gui_strings[STRINGS_MAX+1];
extern char *gui_strings_data;
extern TbCharCount input_field_pos;
/******************************************************************************/
// Exported variables
DLLIMPORT extern struct GuiButton *_DK_input_button;
#define input_button _DK_input_button
DLLIMPORT char _DK_backup_input_field[INPUT_FIELD_LEN];
#define backup_input_field _DK_backup_input_field

DLLIMPORT extern char *_DK_strings_data;
DLLIMPORT extern char *_DK_strings[DK_STRINGS_MAX+1];
/******************************************************************************/
// Exported functions
void do_button_click_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);
void do_sound_menu_click(void);
void do_sound_button_click(struct GuiButton *gbtn);
void setup_input_field(struct GuiButton *gbtn, const char * empty_text);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
