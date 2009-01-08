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

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)
// Type definitions
enum TbButtonType {
        Lb_SLIDER  =  4,
};

union GuiVariant {
      long lval;
      long *lptr;
      char *str;
};


typedef long (*Gf_OptnBox_Callback)(struct GuiBox *, struct GuiBoxOption *, char, long *);
typedef void (*Gf_Btn_Callback)(struct GuiButton *gbtn);
typedef void (*Gf_Mnu_Callback)(struct GuiMenu *gmnu);

struct GuiBoxOption {
       const char *label;
       unsigned char numfield_4;
       void *ofsfield_5;
       Gf_OptnBox_Callback callback;
       long field_D;
       long field_11;
       long field_15;
       long field_19;
       long field_1D;
       long field_21;
       short field_25;
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
    short field_1D;
    short field_1F;
    Gf_Btn_Callback draw_call;
    short field_25;
    short field_27;
    GuiMenu *field_29;
    GuiVariant field_2D;
    char field_31;
    char field_32;
    Gf_Btn_Callback maintain_call;
};

struct GuiButton {
       unsigned char field_0;
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
       char field_1B;
       unsigned char field_1C;
       short field_1D;
       short field_1F;
       short pos_x;
       short pos_y;
       short width;
       short height;
       short field_29;
       short field_2B;
       unsigned short field_2D;
       long field_2F;
       unsigned long *field_33;
       unsigned short slide_val; // slider value, scaled 0..255
};

struct GuiMenu {
      char field_0;
      unsigned char field_1;
      short numfield_2;
      void *ptrfield_4;
      short pos_x;
      short pos_y;
      short width;
      short height;
      Gf_Mnu_Callback ptrfield_10;
      char field_14;
      short unkfield_15;
      char field_17;
      char field_18;
      Gf_Mnu_Callback ptrfield_19;
      unsigned char flgfield_1D;
      unsigned char flgfield_1E;
      char field_1F;
};

struct ToolTipBox {
      unsigned char field_0;
      char text[2048];
      struct GuiButton *gbutton;
      void *target;
      unsigned char field_809;
      short pos_x;
      short pos_y;
};

struct FrontEndButtonData {
        unsigned char field_0;
        unsigned char field_1;
        unsigned char field_2;
};

#pragma pack()

/******************************************************************************/
// Exported variables

/******************************************************************************/
// Exported functions
void do_button_click_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);
void do_button_release_actions(struct GuiButton *gbtn, unsigned char *, Gf_Btn_Callback callback);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
