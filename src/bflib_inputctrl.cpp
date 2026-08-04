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
#include "pre_inc.h"
#include <math.h>
#include <map>
#include "bflib_inputctrl.h"
#include "bflib_basics.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_joyst.h"
#include "bflib_video.h"
#include "bflib_planar.h"
#include "bflib_sndlib.h"
#include "bflib_mshandler.hpp"
#include "frontmenu_ingame_tabs.h"
#include "config_keeperfx.h"
#include "config_settings.h"
#include "front_input.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include <SDL3/SDL.h>
#include "post_inc.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
volatile TbBool lbAppActive;
volatile int lbUserQuit = 0;

unsigned char last_used_input_device = 0;

static TbBool isMouseActive = true;
static TbBool isMouseActivated = false;
static TbBool firstTimeMouseInit = true;

static char lbTextInputBuffer[256];
static int lbTextInputLength = 0;

std::map<int, TbKeyCode> keymap_sdl_to_bf;

//defined here instead of bflib_joyst.h to avoid making header depend on SDL
void JEvent(const SDL_Event *ev);
/******************************************************************************/

/**
 * Converts an SDL mouse button event type and the corresponding mouse button to a Win32 API message.
 * @param eventType SDL event type.
 * @param button SDL button definition.
 * @return
 */
static unsigned int mouse_button_actions_mapping(int eventType, const SDL_MouseButtonEvent * button)
{
    if (eventType == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        switch (button->button)  {
        case SDL_BUTTON_LEFT: return MActn_LBUTTONDOWN;
        case SDL_BUTTON_MIDDLE: return MActn_MBUTTONDOWN;
        case SDL_BUTTON_RIGHT: return MActn_RBUTTONDOWN;
        }
    }
    else if (eventType == SDL_EVENT_MOUSE_BUTTON_UP) {
        switch (button->button) {
        case SDL_BUTTON_LEFT: return MActn_LBUTTONUP;
        case SDL_BUTTON_MIDDLE: return MActn_MBUTTONUP;
        case SDL_BUTTON_RIGHT: return MActn_RBUTTONUP;
        }
    }
    WARNMSG("Unidentified event, type %d button %d",(int)eventType,(int)button->button);
    return MActn_NONE;
}

void init_inputcontrol(void)
{
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_A, KC_A));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_B, KC_B));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_C, KC_C));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_D, KC_D));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_E, KC_E));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F, KC_F));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_G, KC_G));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_H, KC_H));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_I, KC_I));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_J, KC_J));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_K, KC_K));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_L, KC_L));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_M, KC_M));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_N, KC_N));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_O, KC_O));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_P, KC_P));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_Q, KC_Q));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_R, KC_R));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_S, KC_S));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_T, KC_T));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_U, KC_U));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_V, KC_V));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_W, KC_W));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_X, KC_X));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_Y, KC_Y));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_Z, KC_Z));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F1, KC_F1));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F2, KC_F2));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F3, KC_F3));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F4, KC_F4));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F5, KC_F5));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F6, KC_F6));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F7, KC_F7));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F8, KC_F8));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F9, KC_F9));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F10, KC_F10));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F11, KC_F11));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F12, KC_F12));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F13, KC_F13));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F14, KC_F14));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_F15, KC_F15));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_BACKSPACE, KC_BACK));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_TAB, KC_TAB));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_CLEAR, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_RETURN, KC_RETURN));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_PAUSE, KC_PAUSE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_ESCAPE, KC_ESCAPE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_SPACE, KC_SPACE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_EXCLAIM, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_DBLAPOSTROPHE, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_HASH, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_DOLLAR, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_AMPERSAND, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_APOSTROPHE, KC_APOSTROPHE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_LEFTPAREN, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_RIGHTPAREN, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_ASTERISK, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_PLUS, KC_ADD));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_COMMA, KC_COMMA));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_MINUS, KC_MINUS));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_PERIOD, KC_PERIOD));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_SLASH, KC_SLASH));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_0, KC_0));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_1, KC_1));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_2, KC_2));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_3, KC_3));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_4, KC_4));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_5, KC_5));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_6, KC_6));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_7, KC_7));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_8, KC_8));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_9, KC_9));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_COLON, KC_COLON));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_SEMICOLON, KC_SEMICOLON));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_LESS, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_EQUALS, KC_EQUALS));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_GREATER, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_QUESTION, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_AT, KC_AT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_LEFTBRACKET, KC_LBRACKET));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_BACKSLASH, KC_BACKSLASH));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_RIGHTBRACKET, KC_RBRACKET));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_CARET, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_UNDERSCORE, KC_UNDERLINE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_GRAVE, KC_GRAVE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(178, KC_GRAVE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_DELETE, KC_DELETE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_0, KC_NUMPAD0));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_1, KC_NUMPAD1));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_2, KC_NUMPAD2));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_3, KC_NUMPAD3));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_4, KC_NUMPAD4));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_5, KC_NUMPAD5));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_6, KC_NUMPAD6));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_7, KC_NUMPAD7));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_8, KC_NUMPAD8));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_9, KC_NUMPAD9));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_PERIOD, KC_DECIMAL));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_DIVIDE, KC_DIVIDE));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_MULTIPLY, KC_MULTIPLY));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_MINUS, KC_SUBTRACT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_PLUS, KC_ADD));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_ENTER, KC_NUMPADENTER));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_KP_EQUALS, KC_NUMPADEQUALS));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_UP, KC_UP));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_DOWN, KC_DOWN));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_RIGHT, KC_RIGHT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_LEFT, KC_LEFT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_INSERT, KC_INSERT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_HOME, KC_HOME));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_END, KC_END));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_PAGEUP, KC_PGUP));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_PAGEDOWN, KC_PGDOWN));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_NUMLOCKCLEAR, KC_NUMLOCK));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_CAPSLOCK, KC_CAPITAL));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_SCROLLLOCK, KC_SCROLL));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_RSHIFT, KC_RSHIFT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_LSHIFT, KC_LSHIFT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_RCTRL, KC_RCONTROL));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_LCTRL, KC_LCONTROL));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_RALT, KC_RALT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_LALT, KC_LALT));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_LGUI, KC_LWIN));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_RGUI, KC_RWIN));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_MODE, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_HELP, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_PRINTSCREEN, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_SYSREQ, KC_SYSRQ));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_MENU, KC_APPS));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_POWER, KC_POWER));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_UNDO, KC_UNASSIGNED));

    init_controller_input();
    LbStopTextInput();
}

static unsigned int keyboard_keys_mapping(const SDL_KeyboardEvent * key)
{
    /*
    key->keysym.scancode;         < hardware specific scancode
    key->key;         < SDL virtual keysym
    key->keysym.unicode;         < translated character
    */
    int keycode = key->key;
    std::map<int, TbKeyCode>::iterator iter;

    iter = keymap_sdl_to_bf.find(keycode);
    if (iter != keymap_sdl_to_bf.end())
    {
        return iter->second;
    }

    return KC_UNASSIGNED;
}

static TbKeyMods keyboard_mods_mapping(const SDL_KeyboardEvent * key)
{
    TbKeyMods keymod = KMod_NONE;
    switch (key->key)
    {
    // Pressing only a modifier will not treat the key as modifier.
    // If that happens, don't care, so that keyboard control won't try to fix anything.
    case SDLK_RSHIFT:
    case SDLK_LSHIFT:
    case SDLK_RCTRL:
    case SDLK_LCTRL:
    case SDLK_RALT:
    case SDLK_LALT:
    case SDLK_LGUI:
    case SDLK_RGUI:
        keymod = KMod_DONTCARE;
        break;
    // If pressed any other key, mind the modifiers, to allow keyboard control fixes.
    default:
        if ((key->mod & SDL_KMOD_CTRL) != 0)
            keymod |= KMod_CONTROL;
        if ((key->mod & SDL_KMOD_SHIFT) != 0)
            keymod |= KMod_SHIFT;
        if ((key->mod & SDL_KMOD_ALT) != 0)
            keymod |= KMod_ALT;
        break;
    }
    return keymod;
}

TbBool LbIsFrozenOrPaused(void)
{
    return ((freeze_game_on_focus_lost() && !LbIsActive()) || ((game.operation_flags & GOF_Paused) != 0));
}

static TbKeyCode mousebutton_to_keycode(const Uint8 *button)
{
    if (button == NULL || *button < 1 || *button > 9)
        return KC_UNASSIGNED;
    return (KC_MOUSE1 + 1 - *button);
}

static void process_event(const SDL_Event *ev)
{
    struct TbPoint mouseDelta;
    int x;
    SYNCDBG(10, "Starting");

    switch (ev->type)
    {
    case SDL_EVENT_KEY_DOWN:
        x = keyboard_keys_mapping(&ev->key);
        if (x != KC_UNASSIGNED)
        {
            keyboardControl(KActn_KEYDOWN,x,keyboard_mods_mapping(&ev->key), ev->key.key);
        }
        last_used_input_device = ID_Keyboard_Mouse;
        break;

    case SDL_EVENT_KEY_UP:
        x = keyboard_keys_mapping(&ev->key);
        if (x != KC_UNASSIGNED)
        {
            keyboardControl(KActn_KEYUP,x,keyboard_mods_mapping(&ev->key), ev->key.key);
        }
        last_used_input_device = ID_Keyboard_Mouse;
        break;

    case SDL_EVENT_MOUSE_MOTION:
        if (!isMouseActive)
        {
          return;
        }
        static int frac_x = 0, frac_y = 0;
        if (lbMouseGrabbed && lbDisplay.MouseMoveRatio > 0)
        {
            int dx = ev->motion.xrel * lbDisplay.MouseMoveRatio + frac_x;
            int dy = ev->motion.yrel * lbDisplay.MouseMoveRatio + frac_y;

            mouseDelta.x = (dx + 128) >> 8;
            mouseDelta.y = (dy + 128) >> 8;

            frac_x = dx - (mouseDelta.x * 256);
            frac_y = dy - (mouseDelta.y * 256);
        }
        else
        {
            mouseDelta.x = ev->motion.xrel;
            mouseDelta.y = ev->motion.yrel;
            if (isMouseActivated)
            {
                isMouseActivated = 0;
                pointerHandler.SetMousePosition(ev->motion.x + lbDisplay.MouseWindowY, ev->motion.y + lbDisplay.MouseWindowY);
                mouseDelta.x = 0;
                mouseDelta.y = 0;
                frac_x = 0;
                frac_y = 0;
            }
        }
        mouseControl(MActn_MOUSEMOVE, &mouseDelta);
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        last_used_input_device = ID_Keyboard_Mouse;

        if(ev->button.button == SDL_BUTTON_LEFT || ev->button.button == SDL_BUTTON_RIGHT || ev->button.button == SDL_BUTTON_MIDDLE)
        {
            if (!isMouseActive)
            {
            return;
            }
            mouseDelta.x = 0;
            mouseDelta.y = 0;
            mouseControl(mouse_button_actions_mapping(ev->type, &ev->button), &mouseDelta);
        }
        else
        {
            x = mousebutton_to_keycode(&ev->button.button);
            if (x != KC_UNASSIGNED)
            {
                if (ev->type == SDL_EVENT_MOUSE_BUTTON_DOWN)
                {
                    lbKeyOn[x] = 1;
                    lbInkey = x;
                }
                else
                    lbKeyOn[x] = 0;
            }
        }
        break;

    case SDL_EVENT_MOUSE_WHEEL:
        last_used_input_device = ID_Keyboard_Mouse;
        mouseDelta.x = 0;
        mouseDelta.y = 0;
        mouseControl(ev->wheel.y > 0 ? MActn_WHEELMOVEUP : MActn_WHEELMOVEDOWN, &mouseDelta);
        break;

    case SDL_EVENT_TEXT_INPUT:
        if (SDL_TextInputActive(lbWindow))
        {
            int len = strlen(ev->text.text);
            int freeSpace = sizeof(lbTextInputBuffer) - lbTextInputLength - 1;
            if (freeSpace > 0)
            {
                if (len > freeSpace)
                    len = freeSpace;
                memcpy(lbTextInputBuffer + lbTextInputLength, ev->text.text, len);
                lbTextInputLength += len;
                lbTextInputBuffer[lbTextInputLength] = '\0';
            }
        }
        break;

    // top-level event type (SDL_EVENT_WINDOW_*).
    case SDL_EVENT_WINDOW_FOCUS_GAINED:
    {
        lbAppActive = true;
        isMouseActive = true;
        isMouseActivated = true;
        LbGrabMouseCheck(MG_OnFocusGained);
        if (freeze_game_on_focus_lost() && !LbIsFrozenOrPaused())
        {
            resume_music();
        }
        if (mute_audio_on_focus_lost() && !LbIsFrozenOrPaused())
        {
            mute_audio(false);
        }
        redetect_screen_refresh_rate_for_draw();
        break;
    }
    case SDL_EVENT_WINDOW_FOCUS_LOST:
    {
        lbAppActive = false;
        isMouseActive = false;
        isMouseActivated = false;
        LbGrabMouseCheck(MG_OnFocusLost);
        if (freeze_game_on_focus_lost())
        {
            pause_music();
        }
        if (mute_audio_on_focus_lost())
        {
            mute_audio(true);
        }
        break;
    }
    case SDL_EVENT_WINDOW_MOUSE_ENTER:
    {
        if (lbAppActive)
        {
            isMouseActive = true;
            isMouseActivated = true;
        }
        break;
    }
    case SDL_EVENT_WINDOW_MOUSE_LEAVE:
    {
        isMouseActive = false;
        break;
    }
    case SDL_EVENT_WINDOW_MOVED:
    {
        redetect_screen_refresh_rate_for_draw();
        break;
    }
    /* case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
         // todo (allow window to be freely scaled): add window resize function that does what is needed, and call this new function from window init function too
    } */
    case SDL_EVENT_WINDOW_RESIZED:
        break;

    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
    case SDL_EVENT_JOYSTICK_ADDED:
    case SDL_EVENT_JOYSTICK_REMOVED:    
    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
    case SDL_EVENT_JOYSTICK_BALL_MOTION:
    case SDL_EVENT_JOYSTICK_HAT_MOTION:
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
        last_used_input_device = ID_Controller;
        JEvent(ev);
        break;

    case SDL_EVENT_QUIT:
        lbUserQuit = 1;
        break;
    }
}

/******************************************************************************/
TbBool LbPollInputs(void)
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

TbBool LbIsMouseActive(void)
{
    return isMouseActive;
}

void LbMouseCheckPosition(TbBool grab_state_changed)
{
    if (!lbAppActive)
    {
        if (IsMouseInsideWindow())
        {
            LbMoveHostCursorToGameCursor(); // release host mouse
        }
    }
    else // app has focus
    {
        if (lbMouseGrabbed)
        {
            if (grab_state_changed || firstTimeMouseInit) // if start grab, move cursor appropriately
            {
                firstTimeMouseInit = false;
                if (IsMouseInsideWindow())
                {
                    LbMoveGameCursorToHostCursor();
                }
                else
                {
                    LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
                }
            }
        }
        else
        {
            if (firstTimeMouseInit) // if start no-grab, move cursor appropriately
            {
                firstTimeMouseInit = false;
                if (IsMouseInsideWindow() && lbAppActive)
                {
                    LbMoveGameCursorToHostCursor();
                }
            }
            else if (grab_state_changed) // if release grab, move cursor appropriately
            {
                if (IsMouseInsideWindow() && lbAppActive)
                {
                    LbMoveHostCursorToGameCursor();
                }
            }
        }
    }
}

void LbSetMouseGrab(TbBool grab_mouse)
{
    TbBool previousGrabState = lbMouseGrabbed;
    lbMouseGrabbed = grab_mouse;
    if (lbMouseGrabbed)
    {
        LbMouseCheckPosition((previousGrabState != lbMouseGrabbed));
        if (SDL_getenv("NO_RELATIVE_MOUSE"))
        {
            JUSTLOG("NO_RELATIVE_MOUSE is set");
        }
        else
        {
            SDL_SetWindowRelativeMouseMode(lbWindow, true);
        }
    }
    else
    {
        if (SDL_getenv("NO_RELATIVE_MOUSE"))
        {
            JUSTLOG("NO_RELATIVE_MOUSE is set");
        }
        else
        {
            SDL_SetWindowRelativeMouseMode(lbWindow, false);
        }
        LbMouseCheckPosition((previousGrabState != lbMouseGrabbed));
    }
    if (lbAppActive) SDL_HideCursor(); else SDL_ShowCursor(); // show host OS cursor when window has lost focus
}

static void LbClearTextInput(void)
{
    lbTextInputLength = 0;
    lbTextInputBuffer[0] = '\0';
}

int LbGetTextInput(char *dst, int maxChars)
{
    if ((dst == NULL) || (maxChars <= 0) || (lbTextInputLength <= 0))
        return 0;
    int count = lbTextInputLength;
    if (count >= maxChars)
        count = maxChars - 1;
    memcpy(dst, lbTextInputBuffer, count);
    dst[count] = '\0';
    LbClearTextInput();
    return count;
}

TbBool LbIsTextInputActive(void)
{
    return SDL_TextInputActive(lbWindow);
}

void LbStartTextInput(void)
{
    LbClearTextInput();
    if (!SDL_TextInputActive(lbWindow))
        SDL_StartTextInput(lbWindow);
}

void LbStopTextInput(void)
{
    if (SDL_TextInputActive(lbWindow))
        SDL_StopTextInput(lbWindow);
    LbClearTextInput();
}

void LbGrabMouseInit(void)
{
    LbGrabMouseCheck(MG_InitMouse);
}

void LbGrabMouseCheck(long grab_event)
{
    TbBool window_has_focus = lbAppActive;
    TbBool paused = ((game.operation_flags & GOF_Paused) != 0);
    TbBool possession_mode = (get_my_player()->view_type == PVT_CreatureContrl) && ((game.view_mode_flags & GNFldD_CreaturePasngr) == 0);
    TbBool grab_cursor = lbMouseGrabbed;
    if (!window_has_focus)
    {
        grab_cursor = false;
    }
    else
    {
        if (!game.packet_load_enable)
        {
            switch (grab_event)
            {
            case MG_OnPauseEnter:
                if (unlock_cursor_when_game_paused() && lbMouseGrabbed)
                {
                    grab_cursor = false;
                }
                break;
            case MG_OnPauseLeave:
                if ((unlock_cursor_when_game_paused() && lbMouseGrab) || (!lbMouseGrab && lock_cursor_in_possession() && possession_mode && unlock_cursor_when_game_paused()))
                {
                    grab_cursor = true;
                }
                break;
            case MG_OnPossessionEnter:
                if (lock_cursor_in_possession() && !lbMouseGrabbed)
                {
                    grab_cursor = true;
                }
                break;
            case MG_OnPossessionLeave:
                if (lock_cursor_in_possession() && !lbMouseGrab)
                {
                    grab_cursor = false;
                }
                break;
            case MG_InitMouse:
                    grab_cursor = true;
                break;
            case MG_OnFocusGained:
                grab_cursor = lbMouseGrab;
                if (paused && unlock_cursor_when_game_paused())
                {
                    grab_cursor = false;
                }
                if (!paused && possession_mode && lock_cursor_in_possession() && !lbMouseGrab)
                {
                    grab_cursor = true;
                }
                break;
            default:
                break;
            }
        }
        else
        {
            grab_cursor = lbMouseGrab; // keep the default grab state if the player is viewing a saved packet
        }
    }
    LbSetMouseGrab(grab_cursor);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
