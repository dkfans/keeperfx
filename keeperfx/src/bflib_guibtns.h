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
#define SLIDER_MAXVALUE    256

    // Type definitions
    enum TbButtonType {
        LbBtnType_NormalButton = 0,

        // Have continous effect when mouse is held on it. 
        // Used for scrolling creature list.
        LbBtnType_HoldableButton = 1,

        // Toggleing between states. Used at toggle imprisonment or fleeing.
        LbBtnType_ToggleButton = 2,

        // Selecting one from candidates. Used at tab title.
        LbBtnType_RadioButton = 3,
        LbBtnType_HorizontalSlider = 4,
        LbBtnType_EditBox = 5,

        // Never used.
        LbBtnType_Unknown = 6,    
    };

    enum TbButtonFlags 
    {
        // Created, slot occupied
        LbBtnFlag_Created = 0x01,  
        // Dismiss current menu on click.
        LbBtnFlag_CloseCurrentMenu = 0x02,  
        // Enabled.
        LbBtnFlag_Enabled = 0x04,  
        // TODO what is this, also related with enable. maybe Enable-able?
        LbBtnFlag_Unknown08 = 0x08,
        // Mouse over.
        LbBtnFlag_MouseOver = 0x10,  
        LbBtnFlag_Unknown20 = 0x20,

        LbBtnFlag_Unused40 = 0x40,
        LbBtnFlag_Unused80 = 0x80,
    };

    enum GuiVisibility
    {
        // Killed, or not created.
        Visibility_NoExist = 0,
        // Created and shown.
        Visibility_Shown = 1,
        // Is fading.
        Visibility_Fading = 2,
        // Turned off but not killed.
        Visibility_Hidden = 3,
    };

    union GuiVariant {
        long lval;
        long *lptr;
        char *str;
    };

    typedef long(*Gf_OptnBox_4Callback)(struct GuiBox *, struct GuiBoxOption *, unsigned char, long *);
    typedef long(*Gf_OptnBox_3Callback)(struct GuiBox *, struct GuiBoxOption *, long *);
    typedef void(*Gf_Btn_Callback)(struct GuiButton *gbtn);
    typedef void(*Gf_Mnu_Callback)(struct GuiMenu *gmnu);

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
        char flag;
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

    // Initial Gui button(can be other controls) template.
    struct GuiButtonTemplate {
        char button_type;
        short index; // originally was long.
        short field_3; // unused.
        short field_5; // Unknown.

        // Click event callback.
        Gf_Btn_Callback callback_click;
        // Right click event callback.
        Gf_Btn_Callback callback_rightclick;
        // Mouse hover event callback.
        Gf_Btn_Callback callback_mousehover;

        short field_13;

        // Position before scaling.
        short scr_pos_x;
        short scr_pos_y;
        short pos_x;
        short pos_y;

        // Size before scaling.
        short width, height;

        // Callback on button draw.
        Gf_Btn_Callback callback_draw;
        short sprite_idx;
        short tooltip_str_idx;
        struct GuiMenu *parent_menu;
        union GuiVariant content;

        // Acts as text length upper bound or max value of cycle/slider button, -1 when not applicable.
        char max_value;
        // max_value should be short and this is the other half, not merging now because may cause Endian problem.
        char field_32;

        // Callback on button maintain.
        Gf_Btn_Callback callback_maintain;
    };

    // Button(can be other controls) structure.
    struct GuiButton {
        unsigned char flags;
        unsigned char leftclick_flag;
        unsigned char rightclick_flag;

        // Index of the menu this button is located.
        // not necessary equal to parent_menu(show more info button).
        char menu_idx;
        short index;
        unsigned char button_type;

        // Click event callback.
        Gf_Btn_Callback callback_click;
        // Right click event callback.
        Gf_Btn_Callback callback_rightclick;
        // Pointer over event callback.
        Gf_Btn_Callback callback_mousehover;
        // Callback on button draw.
        Gf_Btn_Callback callback_draw;
        // Callback on button maintain.
        Gf_Btn_Callback callback_maintain;

        unsigned short field_1B; // definitely a word, not two bytes

        // Position after scaling.
        short scr_pos_x;
        short scr_pos_y;
        short pos_x;
        short pos_y;

        // Size after scaling.
        short width, height;
        short sprite_idx;

        /** Tooltip string ID. Positive for GUI string, negative for campaign string. */
        short tooltip_str_idx;

        // Acts as text length upper bound or max value of toggle/slider button, -1 when not applicable.
        unsigned short max_value;
        struct GuiMenu *parent_menu;
        //TODO FRONTEND change it to GuiVariant
        unsigned long *content; 
        // slider value, scaled 0..255
        unsigned short slider_value;
    };

    struct GuiMenu {
        // Ident in current menu stack.
        char ident;

        // Current visible state of menu.
        unsigned char visibility;

        // Decrease to 0 and menu is hide.
        short fade_time;

        struct GuiButtonTemplate *buttons;

        short pos_x;
        short pos_y;
        short width;
        short height;

        // Callback on menu draw.
        Gf_Mnu_Callback callback_draw;

        char index;

        // Pointer to initial menu template of this menu.
        struct GuiMenu *menu_template;

        // Callback on menu create.
        Gf_Mnu_Callback callback_create;

        // Whether the menu is turned on or is current active tab.
        unsigned char isTurnedOn;
        unsigned char isMonopolyMenu;

        // Whether the menu is a sub tab on main panel.
        char isTab;
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
        unsigned short tooltip_str_idx;
        unsigned short msg_stridx;
        int lifespan_turns;
        /** Indicates how many turns must pass before another event of the kind is created. */
        int turns_between_events;
        /** Indicates the event kind which is to be replaced by new event. */
        unsigned char replace_event_kind_button;
    };



#pragma pack()
    /******************************************************************************/
    extern TbCharCount input_field_pos;
    /******************************************************************************/
    // Exported variables
    DLLIMPORT extern struct GuiButton *_DK_input_button;
#define input_button _DK_input_button
    DLLIMPORT char _DK_backup_input_field[INPUT_FIELD_LEN];
#define backup_input_field _DK_backup_input_field
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
