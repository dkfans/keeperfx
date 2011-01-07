/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_inputctrl.c
 *     Input devices control and polling.
 * @par Purpose:
 *     Routines for accepting and processing of events from mouse,
 *     keyboard and joypad.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     16 Mar 2009 - 12 Oct 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_inputctrl.h"

#include "bflib_basics.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_video.h"
#include "bflib_vidsurface.h"
#include "bflib_planar.h"
#include <SDL/SDL.h>
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern volatile TbBool lbScreenInitialised;
extern volatile TbBool lbHasSecondSurface;
extern SDL_Color lbPaletteColors[PALETTE_COLORS];

volatile TbBool lbAppActive;
volatile int lbUserQuit = 0;
TbKeyCode keymap_sdl_to_bf[SDLK_LAST-SDLK_FIRST];
/******************************************************************************/
/**
 * Converts an SDL mouse button event type and the corresponding mouse button to a Win32 API message.
 * @param eventType SDL event type.
 * @param button SDL button definition.
 * @return
 */
static unsigned int mouse_button_actions_mapping(int eventType, const SDL_MouseButtonEvent * button)
{
    if (eventType == SDL_MOUSEBUTTONDOWN) {
        switch (button->button)  {
        case SDL_BUTTON_LEFT: return MActn_LBUTTONDOWN;
        case SDL_BUTTON_MIDDLE: return MActn_MBUTTONDOWN;
        case SDL_BUTTON_RIGHT: return MActn_RBUTTONDOWN;
        case SDL_BUTTON_WHEELUP: return MActn_WHEELMOVEUP;
        case SDL_BUTTON_WHEELDOWN: return MActn_WHEELMOVEDOWN;
        }
    }
    else if (eventType == SDL_MOUSEBUTTONUP) {
        switch (button->button) {
        case SDL_BUTTON_LEFT: return MActn_LBUTTONUP;
        case SDL_BUTTON_MIDDLE: return MActn_MBUTTONUP;
        case SDL_BUTTON_RIGHT: return MActn_RBUTTONUP;
        case SDL_BUTTON_WHEELUP: return MActn_NONE;
        case SDL_BUTTON_WHEELDOWN: return MActn_NONE;
        }
    }
    WARNMSG("Unidentified event, type %d button %d",(int)eventType,(int)button->button);
    return MActn_NONE;
}

void prepare_keys_mapping(void)
{
    int i;
    for (i = 0; i < sizeof(keymap_sdl_to_bf)/sizeof(TbKeyCode); i++)
        keymap_sdl_to_bf[i] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_a-SDLK_FIRST] = KC_A;
    keymap_sdl_to_bf[SDLK_b-SDLK_FIRST] = KC_B;
    keymap_sdl_to_bf[SDLK_c-SDLK_FIRST] = KC_C;
    keymap_sdl_to_bf[SDLK_d-SDLK_FIRST] = KC_D;
    keymap_sdl_to_bf[SDLK_e-SDLK_FIRST] = KC_E;
    keymap_sdl_to_bf[SDLK_f-SDLK_FIRST] = KC_F;
    keymap_sdl_to_bf[SDLK_g-SDLK_FIRST] = KC_G;
    keymap_sdl_to_bf[SDLK_h-SDLK_FIRST] = KC_H;
    keymap_sdl_to_bf[SDLK_i-SDLK_FIRST] = KC_I;
    keymap_sdl_to_bf[SDLK_j-SDLK_FIRST] = KC_J;
    keymap_sdl_to_bf[SDLK_k-SDLK_FIRST] = KC_K;
    keymap_sdl_to_bf[SDLK_l-SDLK_FIRST] = KC_L;
    keymap_sdl_to_bf[SDLK_m-SDLK_FIRST] = KC_M;
    keymap_sdl_to_bf[SDLK_n-SDLK_FIRST] = KC_N;
    keymap_sdl_to_bf[SDLK_o-SDLK_FIRST] = KC_O;
    keymap_sdl_to_bf[SDLK_p-SDLK_FIRST] = KC_P;
    keymap_sdl_to_bf[SDLK_q-SDLK_FIRST] = KC_Q;
    keymap_sdl_to_bf[SDLK_r-SDLK_FIRST] = KC_R;
    keymap_sdl_to_bf[SDLK_s-SDLK_FIRST] = KC_S;
    keymap_sdl_to_bf[SDLK_t-SDLK_FIRST] = KC_T;
    keymap_sdl_to_bf[SDLK_u-SDLK_FIRST] = KC_U;
    keymap_sdl_to_bf[SDLK_v-SDLK_FIRST] = KC_V;
    keymap_sdl_to_bf[SDLK_w-SDLK_FIRST] = KC_W;
    keymap_sdl_to_bf[SDLK_x-SDLK_FIRST] = KC_X;
    keymap_sdl_to_bf[SDLK_y-SDLK_FIRST] = KC_Y;
    keymap_sdl_to_bf[SDLK_z-SDLK_FIRST] = KC_Z;
    keymap_sdl_to_bf[SDLK_F1-SDLK_FIRST] = KC_F1;
    keymap_sdl_to_bf[SDLK_F2-SDLK_FIRST] = KC_F2;
    keymap_sdl_to_bf[SDLK_F3-SDLK_FIRST] = KC_F3;
    keymap_sdl_to_bf[SDLK_F4-SDLK_FIRST] = KC_F4;
    keymap_sdl_to_bf[SDLK_F5-SDLK_FIRST] = KC_F5;
    keymap_sdl_to_bf[SDLK_F6-SDLK_FIRST] = KC_F6;
    keymap_sdl_to_bf[SDLK_F7-SDLK_FIRST] = KC_F7;
    keymap_sdl_to_bf[SDLK_F8-SDLK_FIRST] = KC_F8;
    keymap_sdl_to_bf[SDLK_F9-SDLK_FIRST] = KC_F9;
    keymap_sdl_to_bf[SDLK_F10-SDLK_FIRST] = KC_F10;
    keymap_sdl_to_bf[SDLK_F11-SDLK_FIRST] = KC_F11;
    keymap_sdl_to_bf[SDLK_F12-SDLK_FIRST] = KC_F12;
    keymap_sdl_to_bf[SDLK_F13-SDLK_FIRST] = KC_F13;
    keymap_sdl_to_bf[SDLK_F14-SDLK_FIRST] = KC_F14;
    keymap_sdl_to_bf[SDLK_F15-SDLK_FIRST] = KC_F15;
    keymap_sdl_to_bf[SDLK_BACKSPACE-SDLK_FIRST] = KC_BACK;
    keymap_sdl_to_bf[SDLK_TAB-SDLK_FIRST] = KC_TAB;
    keymap_sdl_to_bf[SDLK_CLEAR-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_RETURN-SDLK_FIRST] = KC_RETURN;
    keymap_sdl_to_bf[SDLK_PAUSE-SDLK_FIRST] = KC_PAUSE;
    keymap_sdl_to_bf[SDLK_ESCAPE-SDLK_FIRST] = KC_ESCAPE;
    keymap_sdl_to_bf[SDLK_SPACE-SDLK_FIRST] = KC_SPACE;
    keymap_sdl_to_bf[SDLK_EXCLAIM-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_QUOTEDBL-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_HASH-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_DOLLAR-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_AMPERSAND-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_QUOTE-SDLK_FIRST] = KC_APOSTROPHE;
    keymap_sdl_to_bf[SDLK_LEFTPAREN-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_RIGHTPAREN-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_ASTERISK-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_PLUS-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_COMMA-SDLK_FIRST] = KC_COMMA;
    keymap_sdl_to_bf[SDLK_MINUS-SDLK_FIRST] = KC_MINUS;
    keymap_sdl_to_bf[SDLK_PERIOD-SDLK_FIRST] = KC_PERIOD;
    keymap_sdl_to_bf[SDLK_SLASH-SDLK_FIRST] = KC_SLASH;
    keymap_sdl_to_bf[SDLK_0-SDLK_FIRST] = KC_0;
    keymap_sdl_to_bf[SDLK_1-SDLK_FIRST] = KC_1;
    keymap_sdl_to_bf[SDLK_2-SDLK_FIRST] = KC_2;
    keymap_sdl_to_bf[SDLK_3-SDLK_FIRST] = KC_3;
    keymap_sdl_to_bf[SDLK_4-SDLK_FIRST] = KC_4;
    keymap_sdl_to_bf[SDLK_5-SDLK_FIRST] = KC_5;
    keymap_sdl_to_bf[SDLK_6-SDLK_FIRST] = KC_6;
    keymap_sdl_to_bf[SDLK_7-SDLK_FIRST] = KC_7;
    keymap_sdl_to_bf[SDLK_8-SDLK_FIRST] = KC_8;
    keymap_sdl_to_bf[SDLK_9-SDLK_FIRST] = KC_9;
    keymap_sdl_to_bf[SDLK_COLON-SDLK_FIRST] = KC_COLON;
    keymap_sdl_to_bf[SDLK_SEMICOLON-SDLK_FIRST] = KC_SEMICOLON;
    keymap_sdl_to_bf[SDLK_LESS-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_EQUALS-SDLK_FIRST] = KC_EQUALS;
    keymap_sdl_to_bf[SDLK_GREATER-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_QUESTION-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_AT-SDLK_FIRST] = KC_AT;
    keymap_sdl_to_bf[SDLK_LEFTBRACKET-SDLK_FIRST] = KC_LBRACKET;
    keymap_sdl_to_bf[SDLK_BACKSLASH-SDLK_FIRST] = KC_BACKSLASH;
    keymap_sdl_to_bf[SDLK_RIGHTBRACKET-SDLK_FIRST] = KC_RBRACKET;
    keymap_sdl_to_bf[SDLK_CARET-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_UNDERSCORE-SDLK_FIRST] = KC_UNDERLINE;
    keymap_sdl_to_bf[SDLK_BACKQUOTE-SDLK_FIRST] = KC_GRAVE;
    keymap_sdl_to_bf[SDLK_DELETE-SDLK_FIRST] = KC_DELETE;
    keymap_sdl_to_bf[SDLK_KP0-SDLK_FIRST] = KC_NUMPAD0;
    keymap_sdl_to_bf[SDLK_KP1-SDLK_FIRST] = KC_NUMPAD1;
    keymap_sdl_to_bf[SDLK_KP2-SDLK_FIRST] = KC_NUMPAD2;
    keymap_sdl_to_bf[SDLK_KP3-SDLK_FIRST] = KC_NUMPAD3;
    keymap_sdl_to_bf[SDLK_KP4-SDLK_FIRST] = KC_NUMPAD4;
    keymap_sdl_to_bf[SDLK_KP5-SDLK_FIRST] = KC_NUMPAD5;
    keymap_sdl_to_bf[SDLK_KP6-SDLK_FIRST] = KC_NUMPAD6;
    keymap_sdl_to_bf[SDLK_KP7-SDLK_FIRST] = KC_NUMPAD7;
    keymap_sdl_to_bf[SDLK_KP8-SDLK_FIRST] = KC_NUMPAD8;
    keymap_sdl_to_bf[SDLK_KP9-SDLK_FIRST] = KC_NUMPAD9;
    keymap_sdl_to_bf[SDLK_KP_PERIOD-SDLK_FIRST] = KC_DECIMAL;
    keymap_sdl_to_bf[SDLK_KP_DIVIDE-SDLK_FIRST] = KC_DIVIDE;
    keymap_sdl_to_bf[SDLK_KP_MULTIPLY-SDLK_FIRST] = KC_MULTIPLY;
    keymap_sdl_to_bf[SDLK_KP_MINUS-SDLK_FIRST] = KC_SUBTRACT;
    keymap_sdl_to_bf[SDLK_KP_PLUS-SDLK_FIRST] = KC_ADD;
    keymap_sdl_to_bf[SDLK_KP_ENTER-SDLK_FIRST] = KC_NUMPADENTER;
    keymap_sdl_to_bf[SDLK_KP_EQUALS-SDLK_FIRST] = KC_NUMPADEQUALS;
    keymap_sdl_to_bf[SDLK_UP-SDLK_FIRST] = KC_UP;
    keymap_sdl_to_bf[SDLK_DOWN-SDLK_FIRST] = KC_DOWN;
    keymap_sdl_to_bf[SDLK_RIGHT-SDLK_FIRST] = KC_RIGHT;
    keymap_sdl_to_bf[SDLK_LEFT-SDLK_FIRST] = KC_LEFT;
    keymap_sdl_to_bf[SDLK_INSERT-SDLK_FIRST] = KC_INSERT;
    keymap_sdl_to_bf[SDLK_HOME-SDLK_FIRST] = KC_HOME;
    keymap_sdl_to_bf[SDLK_END-SDLK_FIRST] = KC_END;
    keymap_sdl_to_bf[SDLK_PAGEUP-SDLK_FIRST] = KC_PGUP;
    keymap_sdl_to_bf[SDLK_PAGEDOWN-SDLK_FIRST] = KC_PGDOWN;
    keymap_sdl_to_bf[SDLK_NUMLOCK-SDLK_FIRST] = KC_NUMLOCK;
    keymap_sdl_to_bf[SDLK_CAPSLOCK-SDLK_FIRST] = KC_CAPITAL;
    keymap_sdl_to_bf[SDLK_SCROLLOCK-SDLK_FIRST] = KC_SCROLL;
    keymap_sdl_to_bf[SDLK_RSHIFT-SDLK_FIRST] = KC_RSHIFT;
    keymap_sdl_to_bf[SDLK_LSHIFT-SDLK_FIRST] = KC_LSHIFT;
    keymap_sdl_to_bf[SDLK_RCTRL-SDLK_FIRST] = KC_RCONTROL;
    keymap_sdl_to_bf[SDLK_LCTRL-SDLK_FIRST] = KC_LCONTROL;
    keymap_sdl_to_bf[SDLK_RALT-SDLK_FIRST] = KC_RALT;
    keymap_sdl_to_bf[SDLK_LALT-SDLK_FIRST] = KC_LALT;
    keymap_sdl_to_bf[SDLK_RMETA-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_LMETA-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_LSUPER-SDLK_FIRST] = KC_LWIN;
    keymap_sdl_to_bf[SDLK_RSUPER-SDLK_FIRST] = KC_RWIN;
    keymap_sdl_to_bf[SDLK_MODE-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_COMPOSE-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_HELP-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_PRINT-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_SYSREQ-SDLK_FIRST] = KC_SYSRQ;
    keymap_sdl_to_bf[SDLK_BREAK-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_MENU-SDLK_FIRST] = KC_APPS;
    keymap_sdl_to_bf[SDLK_POWER-SDLK_FIRST] = KC_POWER;
    keymap_sdl_to_bf[SDLK_EURO-SDLK_FIRST] = KC_UNASSIGNED;
    keymap_sdl_to_bf[SDLK_UNDO-SDLK_FIRST] = KC_UNASSIGNED;
}

static unsigned int keyboard_keys_mapping(const SDL_KeyboardEvent * key)
{
    /*
    key->keysym.scancode;         < hardware specific scancode
    key->keysym.sym;         < SDL virtual keysym
    key->keysym.unicode;         < translated character
    */
    int keycode = key->keysym.sym - SDLK_FIRST;
    if ((keycode >= 0) && (keycode < sizeof(keymap_sdl_to_bf)))
        return keymap_sdl_to_bf[keycode];
    return KC_UNASSIGNED;
}

static int keyboard_mods_mapping(const SDL_KeyboardEvent * key)
{
    /*
    key->keysym.mod
        (KMOD_LCTRL|KMOD_RCTRL)
        (KMOD_LSHIFT|KMOD_RSHIFT)
        (KMOD_LALT|KMOD_RALT)
        (KMOD_LMETA|KMOD_RMETA)
    */
    return KMod_DONTCARE;
}

static void process_event(const SDL_Event *ev)
{
    struct TbPoint mousePos;
    int x, y;
    SYNCDBG(10, "Starting");

    switch (ev->type)
    {
    case SDL_KEYDOWN:
        x = keyboard_keys_mapping(&ev->key);
        if (x != KC_UNASSIGNED)
            keyboardControl(KActn_KEYDOWN,x,keyboard_mods_mapping(&ev->key));
        break;

    case SDL_KEYUP:
        x = keyboard_keys_mapping(&ev->key);
        if (x != KC_UNASSIGNED)
            keyboardControl(KActn_KEYUP,x,keyboard_mods_mapping(&ev->key));
        break;

    case SDL_MOUSEMOTION:
        SDL_GetMouseState(&x, &y);
        mousePos.x = x;
        mousePos.y = y;
        mouseControl(MActn_MOUSEMOVE, &mousePos);
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
        SDL_GetMouseState(&x, &y);
        mousePos.x = x;
        mousePos.y = y;
        mouseControl(mouse_button_actions_mapping(ev->type, &ev->button), &mousePos);
        break;

    case SDL_ACTIVEEVENT:
        if (ev->active.state & SDL_APPACTIVE) {
            lbAppActive = (ev->active.gain != 0);
            SDL_ShowCursor(lbAppActive ? SDL_DISABLE : SDL_ENABLE);
            SDL_WM_GrabInput(lbAppActive ? SDL_GRAB_ON : SDL_GRAB_OFF);
        }
        if ((lbAppActive) && (lbDisplay.Palette != NULL)) {
            // Below is the faster version of LbPaletteSet(lbDisplay.Palette);
            SDL_SetColors(lbDrawSurface,lbPaletteColors, 0, PALETTE_COLORS);
        }
        break;

    case SDL_JOYAXISMOTION:
    case SDL_JOYBALLMOTION:
    case SDL_JOYHATMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        //TODO INPUT make joypad support
        break;

    case SDL_SYSWMEVENT:
    case SDL_VIDEORESIZE:
        break;

    case SDL_VIDEOEXPOSE:
        break;

    case SDL_QUIT:
        lbUserQuit = 1;
        break;
    }
}
/******************************************************************************/
TbBool LbWindowsControl(void)
{
    SDL_Event ev;
    //process events until event queue is empty
    while (SDL_PollEvent(&ev)) {
        process_event(&ev);
    }
    return (lbUserQuit < 1);
}

TbBool LbIsActive(void)
{
  // On error, let's assume the window is active.
    if (!lbScreenInitialised)
        return true;
    return lbAppActive;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
