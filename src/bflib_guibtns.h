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

#define INPUT_FIELD_LEN     40
#define TOOLTIP_MAX_LEN   2048

/** Default button designation ID. Other designation IDs should be defined relatively to it. */
#define BID_DEFAULT 0

// Type definitions
enum TbButtonType {
    LbBtnT_NormalBtn = 0,
    /** Have continuous effect when mouse is held on it. Used for scrollable lists. */
    LbBtnT_HoldableBtn,
    /** Toggles between two or more states. Used for on/off switches. */
    LbBtnT_ToggleBtn,
    /** Allows selecting one from grouped buttons. */
    LbBtnT_RadioBtn,
    LbBtnT_HorizSlider,
    LbBtnT_EditBox,
    LbBtnT_Hotspot,
};

enum TbButtonFlags {
    LbBtnF_Active     =  0x01,  // Created, slot occupied
    LbBtnF_Clickable  =  0x02,
    LbBtnF_Visible    =  0x04,  /**< Informs if the button is visible and uses its drawing callback. If not set, the button is not being displayed. */
    LbBtnF_Enabled    =  0x08,  /**< Informs if the button is enabled and can be clicked, or disabled and grayed out with no reaction to input. */
    LbBtnF_MouseOver  =  0x10,
    LbBtnF_Toggle     =  0x20,
};

enum GBoxFlags {
    GBoxF_Allocated = 0x01,
    GBoxF_InList = 0x02,
};

union GuiVariant {
    long lval;
    int32_t *lptr;
    void *ptr;
    char *str;
};

typedef long (*Gf_OptnBox_4Callback)(struct GuiBox *, struct GuiBoxOption *, unsigned char, int32_t *);
typedef long (*Gf_OptnBox_3Callback)(struct GuiBox *, struct GuiBoxOption *, int32_t *);
typedef void (*Gf_Btn_Callback)(struct GuiButton *gbtn);
typedef void (*Gf_Mnu_Callback)(struct GuiMenu *gmnu);

struct GuiBoxOption {
       const char *label;
       unsigned char is_enabled;
       Gf_OptnBox_3Callback active_cb;
       Gf_OptnBox_4Callback callback;
       int32_t acb_param1;
       long max_count;
       long unused_param1;
       int32_t cb_param1;
       long option_index;
       long unused_param2;
       TbBool active;
       TbBool enabled;
};

struct GuiBox {
    char flags;
    short box_index;
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
    char gbtype; /**< GUI Button Type, directly copied to button instance. */
    short id_num; /**< GUI Button ID, directly copied to button instance. If there is no need of identifying the button within game code, it should be set to BID_DEFAULT.*/
    short unused_field; // unused
    unsigned short button_flags; // two bool values; maybe convert it to flags?
    Gf_Btn_Callback click_event;
    Gf_Btn_Callback rclick_event;
    Gf_Btn_Callback ptover_event;
    unsigned short btype_value; /**< Value specific to button type, directly copied to button instance. */
    short scr_pos_x;
    short scr_pos_y;
    short pos_x;
    short pos_y;
    short width;
    short height;
    Gf_Btn_Callback draw_call;
    short sprite_idx;
    short tooltip_stridx;
    struct GuiMenu *parent_menu;
    union GuiVariant content;
    short maxval;
    Gf_Btn_Callback maintain_call;
};

struct GuiButton {
       unsigned char flags;
       unsigned char button_state_left_pressed;
       unsigned char button_state_right_pressed;
       char gmenu_idx;
       short id_num; /**< GUI Button ID, identifying the button designation within game code.*/
       unsigned char gbtype; /**< GUI Button Type, from LbBtnF_* enumeration. */
       Gf_Btn_Callback click_event;
       Gf_Btn_Callback rclick_event;
       Gf_Btn_Callback ptover_event;
       Gf_Btn_Callback draw_call;
       Gf_Btn_Callback maintain_call;
       unsigned short btype_value; /**< Value specific to button type. Contains index in group for grouped buttons, bool state for toggle button, menu index for tab button etc. */
       short scr_pos_x;
       short scr_pos_y;
       short pos_x;
       short pos_y;
       short width;
       short height;
       short sprite_idx;
       /** Tooltip string ID. Positive for GUI string, negative for campaign string. */
       short tooltip_stridx;
       /** Max value. For cycle button - max value before returning to 0; for area input - max string length. */
       unsigned short maxval;
       struct GuiMenu *parent_menu;
       union GuiVariant content;
       unsigned short slide_val; // slider value, scaled 0..255
};

struct GuiMenu {
      char ident;
      unsigned char visual_state;
      float fade_time;
      struct GuiButtonInit *buttons;
      short pos_x;
      short pos_y;
      short width;
      short height;
      Gf_Mnu_Callback draw_cb;
      char number;
      struct GuiMenu *menu_init;
      Gf_Mnu_Callback create_cb;
      unsigned char is_turned_on; /**< Whether the menu is turned on or is current active tab. */
      unsigned char is_monopoly_menu;
      char is_active_panel;
};

struct ToolTipBox {
      unsigned char flags;
      char text[TOOLTIP_MAX_LEN];
      struct GuiButton *gbutton;
      void *target;
      unsigned char box_type; // 0 = doesn't move with cursor
      short pos_x;
      short pos_y;
};

struct FrontEndButtonData {
        unsigned short capstr_idx;
        unsigned char font_index;
};

struct EventTypeInfo {
    int bttn_sprite;
    unsigned short tooltip_stridx;
    unsigned short msg_stridx;
    int lifespan_turns;
    /** Indicates how many turns must pass before another event of the kind is created. */
    int turns_between_events;
    /** Indicates the event kind which is to be replaced by new event. */
    unsigned char replace_event_kind_button;
};

/******************************************************************************/
// Exported variables
extern char backup_input_field[INPUT_FIELD_LEN];
extern struct GuiButton *input_button;
#pragma pack()
/******************************************************************************/
extern TbCharCount input_field_pos;
/******************************************************************************/
// Exported functions
void do_sound_menu_click(void);
void do_sound_button_click(struct GuiButton *gbtn);
void setup_input_field(struct GuiButton *gbtn, const char * empty_text);

TbBool check_if_pos_is_over_button(const struct GuiButton *gbtn, TbScreenPos pos_x, TbScreenPos pos_y);
// Defined in the interface but external
extern void do_button_click_actions(struct GuiButton *, unsigned char *, Gf_Btn_Callback);
void do_button_press_actions(struct GuiButton *, unsigned char *, Gf_Btn_Callback);
extern void do_button_release_actions(struct GuiButton *, unsigned char *, Gf_Btn_Callback);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
