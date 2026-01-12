/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_keybrd.h
 *     Header file for bflib_keybrd.c.
 * @par Purpose:
 *     Keyboard related routines - reading keyboard.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Feb 2008 - 30 Dec 2008
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_KEYBRD_H
#define BFLIB_KEYBRD_H

#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/** PC Keyboard scan codes.
 *  These are used as internal key codes in the Bullfrog engine.
 */
enum KeyCodes {
        KC_UNASSIGNED  = 0x00,
        KC_ESCAPE      = 0x01,
        KC_1           = 0x02,
        KC_2           = 0x03,
        KC_3           = 0x04,
        KC_4           = 0x05,
        KC_5           = 0x06,
        KC_6           = 0x07,
        KC_7           = 0x08,
        KC_8           = 0x09,
        KC_9           = 0x0A,
        KC_0           = 0x0B,
        KC_MINUS       = 0x0C,    // - on main keyboard
        KC_EQUALS      = 0x0D,    // = on main keyboard
        KC_BACK        = 0x0E,    // backspace
        KC_TAB         = 0x0F,
        KC_Q           = 0x10,
        KC_W           = 0x11,
        KC_E           = 0x12,
        KC_R           = 0x13,
        KC_T           = 0x14,
        KC_Y           = 0x15,
        KC_U           = 0x16,
        KC_I           = 0x17,
        KC_O           = 0x18,
        KC_P           = 0x19,
        KC_LBRACKET    = 0x1A,    // square bracket open
        KC_RBRACKET    = 0x1B,    // square bracket close
        KC_RETURN      = 0x1C,    // Enter on main keyboard
        KC_LCONTROL    = 0x1D,
        KC_A           = 0x1E,
        KC_S           = 0x1F,
        KC_D           = 0x20,
        KC_F           = 0x21,
        KC_G           = 0x22,
        KC_H           = 0x23,
        KC_J           = 0x24,
        KC_K           = 0x25,
        KC_L           = 0x26,
        KC_SEMICOLON   = 0x27,
        KC_APOSTROPHE  = 0x28,    // quote "'"
        KC_GRAVE       = 0x29,    // accent "`"
        KC_LSHIFT      = 0x2A,
        KC_BACKSLASH   = 0x2B,
        KC_Z           = 0x2C,
        KC_X           = 0x2D,
        KC_C           = 0x2E,
        KC_V           = 0x2F,
        KC_B           = 0x30,
        KC_N           = 0x31,
        KC_M           = 0x32,
        KC_COMMA       = 0x33,
        KC_PERIOD      = 0x34,    // . on main keyboard
        KC_SLASH       = 0x35,    // / on main keyboard
        KC_RSHIFT      = 0x36,
        KC_MULTIPLY    = 0x37,    // * on numeric keypad
        KC_LALT        = 0x38,    // left Alt
        KC_SPACE       = 0x39,
        KC_CAPITAL     = 0x3A,
        KC_F1          = 0x3B,
        KC_F2          = 0x3C,
        KC_F3          = 0x3D,
        KC_F4          = 0x3E,
        KC_F5          = 0x3F,
        KC_F6          = 0x40,
        KC_F7          = 0x41,
        KC_F8          = 0x42,
        KC_F9          = 0x43,
        KC_F10         = 0x44,
        KC_NUMLOCK     = 0x45,
        KC_SCROLL      = 0x46,    // Scroll Lock
        KC_NUMPAD7     = 0x47,
        KC_NUMPAD8     = 0x48,
        KC_NUMPAD9     = 0x49,
        KC_SUBTRACT    = 0x4A,    // - on numeric keypad
        KC_NUMPAD4     = 0x4B,
        KC_NUMPAD5     = 0x4C,
        KC_NUMPAD6     = 0x4D,
        KC_ADD         = 0x4E,    // + on numeric keypad
        KC_NUMPAD1     = 0x4F,
        KC_NUMPAD2     = 0x50,
        KC_NUMPAD3     = 0x51,
        KC_NUMPAD0     = 0x52,
        KC_DECIMAL     = 0x53,    // . on numeric keypad
        KC_OEM_102     = 0x56,    // < > | on UK/Germany keyboards
        KC_F11         = 0x57,
        KC_F12         = 0x58,
        KC_F13         = 0x64,    //                     (NEC PC98)
        KC_F14         = 0x65,    //                     (NEC PC98)
        KC_F15         = 0x66,    //                     (NEC PC98)
        KC_KANA        = 0x70,    // (Japanese keyboard)
        KC_ABNT_C1     = 0x73,    // / ? on Portugese (Brazilian) keyboards
        KC_CONVERT     = 0x79,    // (Japanese keyboard)
        KC_NOCONVERT   = 0x7B,    // (Japanese keyboard)
        KC_YEN         = 0x7D,    // (Japanese keyboard)
        KC_ABNT_C2     = 0x7E,    // Numpad . on Portugese (Brazilian) keyboards
        KC_NUMPADEQUALS= 0x8D,    // = on numeric keypad (NEC PC98)
        KC_PREVTRACK   = 0x90,    // Previous Track (KC_CIRCUMFLEX on Japanese keyboard)
        KC_AT          = 0x91,    //                     (NEC PC98)
        KC_COLON       = 0x92,    //                     (NEC PC98)
        KC_UNDERLINE   = 0x93,    //                     (NEC PC98)
        KC_KANJI       = 0x94,    // (Japanese keyboard)
        KC_STOP        = 0x95,    //                     (NEC PC98)
        KC_AX          = 0x96,    //                     (Japan AX)
        KC_UNLABELED   = 0x97,    //                        (J3100)
        KC_NEXTTRACK   = 0x99,    // Next Track
        KC_NUMPADENTER = 0x9C,    // Enter on numeric keypad
        KC_RCONTROL    = 0x9D,
        KC_MUTE        = 0xA0,    // Mute
        KC_CALCULATOR  = 0xA1,    // Calculator
        KC_PLAYPAUSE   = 0xA2,    // Play / Pause
        KC_MEDIASTOP   = 0xA4,    // Media Stop
        KC_VOLUMEDOWN  = 0xAE,    // Volume -
        KC_VOLUMEUP    = 0xB0,    // Volume +
        KC_WEBHOME     = 0xB2,    // Web home
        KC_NUMPADCOMMA = 0xB3,    // , on numeric keypad (NEC PC98)
        KC_DIVIDE      = 0xB5,    // / on numeric keypad
        KC_SYSRQ       = 0xB7,
        KC_RALT        = 0xB8,    // right Alt
        KC_PAUSE       = 0xC5,    // Pause
        KC_HOME        = 0xC7,    // Home on arrow keypad
        KC_UP          = 0xC8,    // UpArrow on arrow keypad
        KC_PGUP        = 0xC9,    // PgUp on arrow keypad
        KC_LEFT        = 0xCB,    // LeftArrow on arrow keypad
        KC_RIGHT       = 0xCD,    // RightArrow on arrow keypad
        KC_END         = 0xCF,    // End on arrow keypad
        KC_DOWN        = 0xD0,    // DownArrow on arrow keypad
        KC_PGDOWN      = 0xD1,    // PgDn on arrow keypad
        KC_INSERT      = 0xD2,    // Insert on arrow keypad
        KC_DELETE      = 0xD3,    // Delete on arrow keypad
        KC_LWIN        = 0xDB,    // Left Windows key
        KC_RWIN        = 0xDC,    // Right Windows key
        KC_APPS        = 0xDD,    // AppMenu key
        KC_POWER       = 0xDE,    // System Power
        KC_SLEEP       = 0xDF,    // System Sleep
        KC_WAKE        = 0xE3,    // System Wake
    // Add mouse buttons counting backwards from 0xFE
    // This allows them to be used as a bindable "key" by adding them to key_to_string_init[] [mouse buttons as keybinds - quick fix]
        KC_MOUSE9          = 0xF4,    // Mouse button #9
        KC_MOUSE8          = 0xF5,    // Mouse button #8
        KC_MOUSE7          = 0xF6,    // Mouse button #7
        KC_MOUSE6          = 0xF7,    // Mouse button #6
        KC_MOUSE5          = 0xF8,    // Mouse button #5
        KC_MOUSE4          = 0xF9,    // Mouse button #4
        KC_MOUSE3          = 0xFA,    // Middle Mouse button
        KC_MOUSE2          = 0xFB,    // Right Mouse button (don't use for binding, there will likely be conflicts)
        KC_MOUSE1          = 0xFC,    // Left Mouse button (don't use for binding, there will likely be conflicts)
        KC_MOUSEWHEEL_DOWN = 0xFD,    // Mouse Wheel Scroll down
        KC_MOUSEWHEEL_UP   = 0xFE,    // Mouse Wheel Scroll up
        KC_GAMEPAD_A       = 0xFF,
        KC_GAMEPAD_B       = 0x100,
        KC_GAMEPAD_X       = 0x101,
        KC_GAMEPAD_Y       = 0x102,
        KC_GAMEPAD_BACK    = 0x103,
        KC_GAMEPAD_GUIDE   = 0x104,
        KC_GAMEPAD_START   = 0x105,
        KC_GAMEPAD_LEFTSTICK  = 0x106,
        KC_GAMEPAD_RIGHTSTICK = 0x107,
        KC_GAMEPAD_LEFTSHOULDER  = 0x108,
        KC_GAMEPAD_RIGHTSHOULDER = 0x109,
        KC_GAMEPAD_DPAD_UP    = 0x10A,
        KC_GAMEPAD_DPAD_DOWN  = 0x10B,
        KC_GAMEPAD_DPAD_LEFT  = 0x10C,
        KC_GAMEPAD_DPAD_RIGHT = 0x10D,
        KC_JOYSTICK_BUTTON1    = 0x10E,
        KC_JOYSTICK_BUTTON2    = 0x10F,
        KC_JOYSTICK_BUTTON3    = 0x110,
        KC_JOYSTICK_BUTTON4    = 0x111,
        KC_JOYSTICK_BUTTON5    = 0x112,
        KC_JOYSTICK_BUTTON6    = 0x113,
        KC_JOYSTICK_BUTTON7    = 0x114,
        KC_JOYSTICK_BUTTON8    = 0x115,
        KC_JOYSTICK_BUTTON9    = 0x116,
        KC_JOYSTICK_BUTTON10   = 0x117,
        KC_JOYSTICK_BUTTON11   = 0x118,
        KC_JOYSTICK_BUTTON12   = 0x119,
        KC_JOYSTICK_BUTTON13   = 0x11A,
        KC_JOYSTICK_BUTTON14   = 0x11B,
        KC_JOYSTICK_BUTTON15   = 0x11C,
        KC_JOYSTICK_BUTTON16   = 0x11D,
        KC_JOYSTICK_BUTTON17   = 0x11E,
        KC_JOYSTICK_BUTTON18   = 0x11F,
        KC_JOYSTICK_BUTTON19   = 0x120,
        KC_JOYSTICK_BUTTON20   = 0x121,



        KC_LIST_END,

};

enum KeyAction {
        KActn_NONE = 0,
        KActn_KEYDOWN,
        KActn_KEYUP,
};

enum KeyModifiers {
        KMod_NONE        = 0x00,
        KMod_SHIFT       = 0x10,
        KMod_CONTROL     = 0x20,
        KMod_ALT         = 0x40,
};
#define KMod_DONTCARE -1

/******************************************************************************/
#pragma pack(1)

typedef uint16_t TbKeyCode;
typedef short TbKeyMods;

#pragma pack()
/******************************************************************************/

extern const char AsciiToInkey[];
extern char lbInkeyToAscii[];
extern char lbInkeyToAsciiShift[];
extern unsigned char lbKeyOn[KC_LIST_END];
extern TbKeyCode lbInkey;

/******************************************************************************/
short LbIKeyboardOpen(void);
short LbIKeyboardClose(void);
void LbKeyboardSetLanguage(int lngnum);
void keyboardControl(unsigned int action, TbKeyCode code, TbKeyMods modifiers, int ScanCode);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
