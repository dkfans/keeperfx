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
#include "bflib_video.h"
#include "bflib_planar.h"
#include "bflib_sndlib.h"
#include "bflib_mshandler.hpp"
#include "frontmenu_ingame_tabs.h"
#include "config_keeperfx.h"
#include "config_settings.h"
#include "front_input.h"
#include "sounds.h"
#include "game_legacy.h" // needed for paused and possession_mode below - maybe there is a neater way than this...
#include "keeperfx.hpp" // for start_params
#include <SDL2/SDL.h>
#include "post_inc.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
volatile TbBool lbAppActive;
volatile int lbUserQuit = 0;

static int prevMouseX = 0, prevMouseY = 0;
static TbBool isMouseActive = true;
static TbBool isMouseActivated = false;
static TbBool firstTimeMouseInit = true;

std::map<int, TbKeyCode> keymap_sdl_to_bf;

static SDL_GameController *controller = NULL;
static SDL_Joystick *joystick = NULL;

static TbBool lt_pressed = false;
static TbBool rt_pressed = false;

static uint16_t num_keys_down = 0;

static Uint8 prev_back = 0;
static Uint8 prev_leftshoulder = 0;
static Uint8 prev_rightshoulder = 0;
static Uint8 prev_joy_left = 0;
//static Uint8 prev_joy_right = 0;

static float mouse_accum_x = 0.0f;
static float mouse_accum_y = 0.0f;

float movement_accum_x = 0.0f;
float movement_accum_y = 0.0f;

#define TimePoint std::chrono::high_resolution_clock::time_point
#define TimeNow std::chrono::high_resolution_clock::now()

static TimePoint delta_time_previous_timepoint = TimeNow;
static float input_delta_time = 0.0f;


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
        }
    }
    else if (eventType == SDL_MOUSEBUTTONUP) {
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
    SDL_GetMouseState(&prevMouseX, &prevMouseY);

    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_a, KC_A));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_b, KC_B));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_c, KC_C));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_d, KC_D));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_e, KC_E));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_f, KC_F));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_g, KC_G));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_h, KC_H));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_i, KC_I));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_j, KC_J));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_k, KC_K));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_l, KC_L));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_m, KC_M));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_n, KC_N));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_o, KC_O));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_p, KC_P));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_q, KC_Q));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_r, KC_R));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_s, KC_S));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_t, KC_T));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_u, KC_U));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_v, KC_V));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_w, KC_W));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_x, KC_X));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_y, KC_Y));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_z, KC_Z));
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
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_QUOTEDBL, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_HASH, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_DOLLAR, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_AMPERSAND, KC_UNASSIGNED));
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_QUOTE, KC_APOSTROPHE));
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
    keymap_sdl_to_bf.insert(pair<int, TbKeyCode>(SDLK_BACKQUOTE, KC_GRAVE));
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

    SDL_GameControllerAddMappingsFromFile(prepare_file_path(FGrp_FxData, "gamecontrollerdb.txt"));

    // Initialize controller
    if (SDL_NumJoysticks() > 0) {
        if (SDL_IsGameController(0)) {
            
            controller = SDL_GameControllerOpen(0);
            if (controller == NULL) {
                WARNLOG("Could not open gamecontroller 0: %s", SDL_GetError());
            }
            else {
                const char* controller_name = SDL_GameControllerName(controller);                
                SYNCLOG("GameController connected: %s", 
                        controller_name ? controller_name : "Unknown");
            }
        } else {
            joystick = SDL_JoystickOpen(0);
            if (joystick == NULL) {
                WARNLOG("Could not open joystick 0: %s", SDL_GetError());
            }
            else {             
                SYNCLOG("Generic Joystick connected");
            }
        }
    }
}

static unsigned int keyboard_keys_mapping(const SDL_KeyboardEvent * key)
{
    /*
    key->keysym.scancode;         < hardware specific scancode
    key->keysym.sym;         < SDL virtual keysym
    key->keysym.unicode;         < translated character
    */
    int keycode = key->keysym.sym;
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
    switch (key->keysym.sym)
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
        if ((key->keysym.mod & KMOD_CTRL) != 0)
            keymod |= KMod_CONTROL;
        if ((key->keysym.mod & KMOD_SHIFT) != 0)
            keymod |= KMod_SHIFT;
        if ((key->keysym.mod & KMOD_ALT) != 0)
            keymod |= KMod_ALT;
        break;
    }
    return keymod;
}

TbBool LbIsFrozenOrPaused(void)
{
    return ((freeze_game_on_focus_lost() && !LbIsActive()) || ((game.operation_flags & GOF_Paused) != 0));
}

static void open_controller(int device_index)
{
    if (controller || joystick) return;

    if (SDL_IsGameController(device_index)) {
        controller = SDL_GameControllerOpen(device_index);
        if (controller) {
            joystick = NULL;
            return;
        }
    }

    joystick = SDL_JoystickOpen(device_index);
}

static void close_controller(SDL_JoystickID instance_id)
{
    if (controller) {
        SDL_Joystick *joy = SDL_GameControllerGetJoystick(controller);
        if (SDL_JoystickInstanceID(joy) == instance_id) {
            SDL_GameControllerClose(controller);
            controller = NULL;
        }
    }

    if (joystick) {
        if (SDL_JoystickInstanceID(joystick) == instance_id) {
            SDL_JoystickClose(joystick);
            joystick = NULL;
        }
    }
    lt_pressed = false;
    rt_pressed = false;
}

static TbKeyCode mousebutton_to_keycode(const Uint8 *button)
{
    if (button == NULL || *button < 1 || *button > 9)
        return KC_UNASSIGNED;
    return (KC_MOUSE1 + 1 - *button);
}

static TbKeyCode gamecontrollerbutton_to_keycode(const Uint8 button)
{
    switch (button) {
        case SDL_CONTROLLER_BUTTON_A: return KC_GAMEPAD_A;
        case SDL_CONTROLLER_BUTTON_B: return KC_GAMEPAD_B;
        case SDL_CONTROLLER_BUTTON_X: return KC_GAMEPAD_X;
        case SDL_CONTROLLER_BUTTON_Y: return KC_GAMEPAD_Y;
        case SDL_CONTROLLER_BUTTON_BACK: return KC_GAMEPAD_BACK;
        case SDL_CONTROLLER_BUTTON_GUIDE: return KC_GAMEPAD_GUIDE;
        case SDL_CONTROLLER_BUTTON_START: return KC_GAMEPAD_START;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: return KC_GAMEPAD_LEFTSTICK;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return KC_GAMEPAD_RIGHTSTICK;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return KC_GAMEPAD_LEFTSHOULDER;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return KC_GAMEPAD_RIGHTSHOULDER;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: return KC_GAMEPAD_DPAD_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return KC_GAMEPAD_DPAD_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return KC_GAMEPAD_DPAD_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return KC_GAMEPAD_DPAD_RIGHT;
        default: break;
    }
    return KC_UNASSIGNED;
}

static TbKeyCode joystickbutton_to_keycode(const Uint8 button)
{
    if (controller)
    {
        // Find which controller button this joystick button corresponds to
        SDL_GameControllerButton ctrl_button = SDL_CONTROLLER_BUTTON_INVALID;
        for (int i = 0; i < SDL_CONTROLLER_BUTTON_MAX; i++)
        {
            SDL_GameControllerButtonBind bind = SDL_GameControllerGetBindForButton(controller, (SDL_GameControllerButton)i);
            if (bind.bindType == SDL_CONTROLLER_BINDTYPE_BUTTON && bind.value.button == button)
            {
                ctrl_button = (SDL_GameControllerButton)i;
                break;
            }
        }

        if (ctrl_button != SDL_CONTROLLER_BUTTON_INVALID)
        {
            return gamecontrollerbutton_to_keycode(ctrl_button);
        }
    }

    if (button < 1 || button > 20)
        return KC_UNASSIGNED;
    return KC_JOYSTICK_BUTTON1 + button - 1;
}


static void process_event(const SDL_Event *ev)
{
    struct TbPoint mouseDelta;
    int x;
    SYNCDBG(10, "Starting");

    switch (ev->type)
    {
    case SDL_KEYDOWN:
        x = keyboard_keys_mapping(&ev->key);
        if (x != KC_UNASSIGNED)
        {
            if (ev->key.repeat == 0)
                num_keys_down++;
            
            keyboardControl(KActn_KEYDOWN,x,keyboard_mods_mapping(&ev->key), ev->key.keysym.sym);
        }
        break;

    case SDL_KEYUP:
        x = keyboard_keys_mapping(&ev->key);
        if (x != KC_UNASSIGNED)
        {
            if (num_keys_down > 0)
                num_keys_down--;
            keyboardControl(KActn_KEYUP,x,keyboard_mods_mapping(&ev->key), ev->key.keysym.sym);
        }
        break;

    case SDL_MOUSEMOTION:
        if (!isMouseActive)
        {
          SDL_GetMouseState(&prevMouseX, &prevMouseY);
          return;
        }
        if (lbMouseGrabbed && lbDisplay.MouseMoveRatio > 0)
        {
            mouseDelta.x = ev->motion.xrel * lbDisplay.MouseMoveRatio / 256;
            mouseDelta.y = ev->motion.yrel * lbDisplay.MouseMoveRatio / 256;
        }
        else
        {
            mouseDelta.x = ev->motion.xrel;
            mouseDelta.y = ev->motion.yrel;
            if (isMouseActivated)
            {
                isMouseActivated = 0;
                pointerHandler.SetPosition(ev->motion.x + lbDisplay.MouseWindowY, ev->motion.y + lbDisplay.MouseWindowY);
                mouseDelta.x = 0;
                mouseDelta.y = 0;
            }
        }
        mouseControl(MActn_MOUSEMOVE, &mouseDelta);
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:

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
                if (ev->type == SDL_MOUSEBUTTONDOWN)
                {
                    lbKeyOn[x] = 1;
                    lbInkey = x;
                }
                else
                    lbKeyOn[x] = 0;
            }
        }
        break;

    case SDL_MOUSEWHEEL:
        mouseDelta.x = 0;
        mouseDelta.y = 0;
        mouseControl(ev->wheel.y > 0 ? MActn_WHEELMOVEUP : MActn_WHEELMOVEDOWN, &mouseDelta);
        break;

    case SDL_WINDOWEVENT:
        switch (ev->window.event)
        {
            case SDL_WINDOWEVENT_FOCUS_GAINED:
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
            case SDL_WINDOWEVENT_FOCUS_LOST:
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
            case SDL_WINDOWEVENT_ENTER:
            {
                if (lbAppActive)
                {
                    isMouseActive = true;
                    isMouseActivated = true;
                }
                break;
            }
            case SDL_WINDOWEVENT_LEAVE:
            {
                isMouseActive = false;
                break;
            }
            case SDL_WINDOWEVENT_MOVED:
            {
                redetect_screen_refresh_rate_for_draw();
                break;
            }
            default: break;
        }
        /* else if (ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
             // todo (allow window to be freely scaled): add window resize function that does what is needed, and call this new function from window init function too
        } */
        break;
    case SDL_JOYAXISMOTION:
    case SDL_JOYBALLMOTION:
    case SDL_JOYHATMOTION:
        break;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
    {   
        TbKeyCode keycode = joystickbutton_to_keycode(ev->jbutton.button);
        if (keycode != KC_UNASSIGNED)
        {
            if (ev->type == SDL_JOYBUTTONDOWN)
            {
                lbKeyOn[keycode] = 1;
                lbInkey = keycode;
            }
            else
            {
                lbKeyOn[keycode] = 0;
            }
        }
    }
        break;
    case SDL_CONTROLLERAXISMOTION:
        break;
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    {
        // Some controllers with proper mappings send these events instead of joystick events
        Uint8 button_val = ev->cbutton.button;
        TbKeyCode keycode = gamecontrollerbutton_to_keycode(button_val);
        if (keycode != KC_UNASSIGNED)
        {
            if (ev->type == SDL_CONTROLLERBUTTONDOWN)
            {
                lbKeyOn[keycode] = 1;
                lbInkey = keycode;
            }
            else
            {
                lbKeyOn[keycode] = 0;
            }
        }
    }
        break;
    case SDL_SYSWMEVENT:
    case SDL_WINDOWEVENT_RESIZED:
        break;

    case SDL_JOYDEVICEADDED:
        open_controller(ev->jdevice.which);
        break;
    case SDL_JOYDEVICEREMOVED:
        close_controller(ev->jdevice.which);
        break;

    case SDL_QUIT:
        lbUserQuit = 1;
        break;
    }
}

void controller_rumble(long ms)
{
    if (controller != NULL) {
        SDL_GameControllerRumble(controller, 0xFFFF, 0xFFFF, ms);
    }
}


#define STICK_DEADZONE      0.15f

void poll_controller_movement(Sint16 lx, Sint16 ly)
{
    float nx = lx / 32768.0f;
    float ny = ly / 32768.0f;
    
    // Handle horizontal movement - just accumulate for local camera
    float move_mag_x = fabsf(nx);
    if (move_mag_x > STICK_DEADZONE) {
        float norm_mag = (move_mag_x - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
        float curved = norm_mag * norm_mag;
        float presses_this_frame = curved * input_delta_time;
        
        movement_accum_x += (nx > 0 ? presses_this_frame : -presses_this_frame);

        struct PlayerInfo* player = get_my_player();
        if(player->work_state == PSt_FreeCtrlDirect || player->work_state == PSt_CtrlDirect) {
            struct Packet* packet = get_packet(my_player_number);
            while (movement_accum_x >= 1.0f) {
                set_packet_control(packet, PCtr_MoveRight);
                movement_accum_x -= 1.0f;
            }
            while (movement_accum_x <= -1.0f) {
                set_packet_control(packet, PCtr_MoveLeft);
                movement_accum_x += 1.0f;
            }
        }
    }
    
    // Handle vertical movement - just accumulate for local camera
    float move_mag_y = fabsf(ny);
    if (move_mag_y > STICK_DEADZONE) {
        float norm_mag = (move_mag_y - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
        float curved = norm_mag * norm_mag;
        float presses_this_frame = curved * input_delta_time;
        
        movement_accum_y += (ny > 0 ? presses_this_frame : -presses_this_frame);
        
        struct PlayerInfo* player = get_my_player();
        if(player->work_state == PSt_FreeCtrlDirect || player->work_state == PSt_CtrlDirect) {
            struct Packet* packet = get_packet(my_player_number);
            while (movement_accum_y >= 1.0f) {
                set_packet_control(packet, PCtr_MoveDown);
                movement_accum_y -= 1.0f;
            }
            while (movement_accum_y <= -1.0f) {
                set_packet_control(packet, PCtr_MoveUp);
                movement_accum_y += 1.0f;
            }
        }
    }
    // Packets will be sent by send_camera_catchup_packets() based on position difference
}

#define SECONDS_TO_CROSS   20.0f
void poll_controller_mouse(Sint16 rx, Sint16 ry)
{
    float nx = rx / 32768.0f;
    float ny = ry / 32768.0f;
    float mag = sqrtf(nx * nx + ny * ny);

    if (mag < STICK_DEADZONE)
        return;

    nx /= mag;
    ny /= mag;

    float norm_mag = (mag - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
    float curved = norm_mag * norm_mag;
    float pixels_per_second = lbDisplay.GraphicsWindowWidth / SECONDS_TO_CROSS;
    float pixels_this_frame = pixels_per_second * input_delta_time;

    mouse_accum_x += nx * curved * pixels_this_frame;
    mouse_accum_y += ny * curved * pixels_this_frame;

    int dx = (int)mouse_accum_x;
    int dy = (int)mouse_accum_y;

    mouse_accum_x -= dx;
    mouse_accum_y -= dy;

    if (dx != 0 || dy != 0) {
        struct TbPoint mouseDelta = { dx, dy };
        mouseControl(MActn_MOUSEMOVE, &mouseDelta);
    }
}

static float get_input_delta_time()
{
    long double frame_time_in_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(TimeNow - delta_time_previous_timepoint).count();
    delta_time_previous_timepoint = TimeNow;
    float calculated_delta_time = (frame_time_in_nanoseconds/1000000000.0) * game_num_fps;
    return min(calculated_delta_time, 1.0f);
}

static void poll_controller()
{
    input_delta_time = get_input_delta_time();
    if (controller != NULL) {
        
        TbBool has_right_stick =
            SDL_GameControllerHasAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX) &&
            SDL_GameControllerHasAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        TbBool has_left_stick =
            SDL_GameControllerHasAxis(controller, SDL_CONTROLLER_AXIS_LEFTX) &&
            SDL_GameControllerHasAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);

        //analog sticks and dpad, layout based on what's available, with mouse being most important, then movement, then cam rotation/zoom

        if (has_right_stick && has_left_stick) {
            lbKeyOn[KC_HOME] =   SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
            lbKeyOn[KC_END] =    SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            lbKeyOn[KC_PGDOWN] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            lbKeyOn[KC_DELETE] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

            
            Sint16 leftX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            Sint16 leftY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
            poll_controller_movement(leftX, leftY);

            Sint16 rx = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
            Sint16 ry = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
            poll_controller_mouse(rx, ry);

        } else if (has_left_stick) {

            lbKeyOn[KC_UP]    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
            lbKeyOn[KC_DOWN]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            lbKeyOn[KC_LEFT]  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            lbKeyOn[KC_RIGHT] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            
            Sint16 rx = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
            Sint16 ry = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
            poll_controller_mouse(rx, ry);
        } else {
            TbBool up    = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
            TbBool down  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            TbBool left  = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            TbBool right = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);

            struct TbPoint mouseDelta;
            mouseDelta.x = (right - left)  * lbDisplay.GraphicsWindowHeight * input_delta_time / 0.02f;
            mouseDelta.y = (down - up)     * lbDisplay.GraphicsWindowHeight * input_delta_time / 0.02f;
            mouseControl(MActn_MOUSEMOVE, &mouseDelta);
        }

        TbBool has_triggers =
            SDL_GameControllerHasAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) &&
            SDL_GameControllerHasAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

        if(has_triggers)
        {
            Uint8 current_leftshoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            if (current_leftshoulder && !prev_leftshoulder) {
                go_to_adjacent_menu_tab(-1);
            }
            prev_leftshoulder = current_leftshoulder;

            Uint8 current_rightshoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            if (current_rightshoulder && !prev_rightshoulder) {
                go_to_adjacent_menu_tab(1);
            }
            prev_rightshoulder = current_rightshoulder;


            // Handle triggers for mouse buttons
            Sint16 lt = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
            Sint16 rt = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
            struct TbPoint delta = {0, 0};
            if (lt > 10000 && !lt_pressed) {
                lt_pressed = true;
                mouseControl(MActn_RBUTTONDOWN, &delta);
            } else if (lt <= 10000 && lt_pressed) {
                lt_pressed = false;
                mouseControl(MActn_RBUTTONUP, &delta);
            }
            if (rt > 10000 && !rt_pressed) {
                rt_pressed = true;
                mouseControl(MActn_LBUTTONDOWN, &delta);
            } else if (rt <= 10000 && rt_pressed) {
                rt_pressed = false;
                mouseControl(MActn_LBUTTONUP, &delta);
            }
        }
        else {
            struct TbPoint delta = {0, 0};

            Uint8 current_leftshoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            if (current_leftshoulder && !prev_leftshoulder) {
                mouseControl(MActn_RBUTTONDOWN, &delta);
            }
            else if (!current_leftshoulder && prev_leftshoulder) {
                mouseControl(MActn_RBUTTONUP, &delta);
            }
            prev_leftshoulder = current_leftshoulder;

            Uint8 current_rightshoulder = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            if (current_rightshoulder && !prev_rightshoulder) {
                mouseControl(MActn_LBUTTONDOWN, &delta);
            }
            else if (!current_rightshoulder && prev_rightshoulder) {
                mouseControl(MActn_LBUTTONUP, &delta);
            }
            prev_rightshoulder = current_rightshoulder;
        }

        lbKeyOn[KC_LSHIFT] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_A);
        lbKeyOn[KC_LCONTROL] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
        lbKeyOn[settings.kbkeys[Gkey_ZoomToFight].code]   = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_X);
        lbKeyOn[settings.kbkeys[Gkey_ZoomRoomHeart].code] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_Y);

        // Handle Start and Back buttons with edge detection to simulate key presses
        lbKeyOn[KC_SPACE] = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_BACK);


        Uint8 current_back = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_START);
        if (current_back && !prev_back) {
            keyboardControl(KActn_KEYDOWN, KC_ESCAPE, KMod_NONE, 0);
            keyboardControl(KActn_KEYDOWN, KC_P, KMod_NONE, 0);
        } else if (!current_back && prev_back) {
            keyboardControl(KActn_KEYUP, KC_ESCAPE, KMod_NONE, 0);
            keyboardControl(KActn_KEYUP, KC_P, KMod_NONE, 0);
        }
        prev_back = current_back;

        Uint8 current_joy_left = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_LEFTSTICK);
        if (current_joy_left && !prev_joy_left) {
            keyboardControl(KActn_KEYDOWN, settings.kbkeys[Gkey_SwitchToMap].code, KMod_NONE, 0);
        } else if (!current_joy_left && prev_joy_left) {
            keyboardControl(KActn_KEYUP, settings.kbkeys[Gkey_SwitchToMap].code, KMod_NONE, 0);
        }
        prev_joy_left = current_joy_left;

        //Uint8 current_joy_right = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
        //if (current_joy_right && !prev_joy_right) {
        //    keyboardControl(KActn_KEYDOWN, KC_P, KMod_NONE, 0);
        //} else if (!current_joy_right && prev_joy_right) {
        //    keyboardControl(KActn_KEYUP, KC_P, KMod_NONE, 0);
        //}
        //prev_joy_right = current_joy_right;


    } else if (joystick != NULL) {
        // Map joystick buttons to keyboard keys (assuming standard layout)
        lbKeyOn[KC_HOME] = SDL_JoystickGetButton(joystick, 10); // D-pad up
        lbKeyOn[KC_END] = SDL_JoystickGetButton(joystick, 12); // D-pad down
        lbKeyOn[KC_PGDOWN] = SDL_JoystickGetButton(joystick, 13); // D-pad left
        lbKeyOn[KC_DELETE] = SDL_JoystickGetButton(joystick, 11); // D-pad right

        //lbKeyOn[KC_SPACE] = SDL_JoystickGetButton(joystick, 0); // Button 0 (A)
        //lbKeyOn[KC_LCONTROL] = SDL_JoystickGetButton(joystick, 1); // Button 1 (B)
        lbKeyOn[KC_LSHIFT] = SDL_JoystickGetButton(joystick, 2); // Button 2 (X)
        lbKeyOn[KC_LCONTROL] = SDL_JoystickGetButton(joystick, 3); // Button 3 (Y)
        lbKeyOn[settings.kbkeys[Gkey_ZoomToFight].code] = SDL_JoystickGetButton(joystick, 4); 
        lbKeyOn[settings.kbkeys[Gkey_ZoomRoomHeart].code] = SDL_JoystickGetButton(joystick, 5);
        lbKeyOn[settings.kbkeys[Gkey_SwitchToMap].code] = SDL_JoystickGetButton(joystick, 6);
        lbKeyOn[settings.kbkeys[Gkey_ToggleMessage].code] = SDL_JoystickGetButton(joystick, 7);
        
        // Handle analog sticks for movement (axes 0 and 1)
        Sint16 leftX = SDL_JoystickGetAxis(joystick, 0);
        Sint16 leftY = SDL_JoystickGetAxis(joystick, 1);
        poll_controller_movement(leftX, leftY);

        // Handle right stick for mouse movement (axes 2 and 3)
        Sint16 rightX = SDL_JoystickGetAxis(joystick, 3);
        Sint16 rightY = SDL_JoystickGetAxis(joystick, 4);
        poll_controller_mouse(rightX, rightY);

        // Handle triggers for mouse buttons (axes 4 and 5)
        struct TbPoint delta = { 0, 0 };
        if (SDL_JoystickGetButton(joystick, 0))
        {
            lt_pressed = true;
            mouseControl(MActn_LBUTTONDOWN, &delta);
        }
        else
        {
            if (lt_pressed)
            {
                mouseControl(MActn_LBUTTONUP, &delta);
            }
            lt_pressed = false;
        }
        if (SDL_JoystickGetButton(joystick, 1))
        {
            rt_pressed = true;
            mouseControl(MActn_RBUTTONDOWN, &delta);
        }
        else
        {
            if (rt_pressed)
            {
                mouseControl(MActn_RBUTTONUP, &delta);
            }
            rt_pressed = false;
        }
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
    
    if (num_keys_down == 0)
        poll_controller();

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
            SDL_SetRelativeMouseMode(SDL_TRUE);
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
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
        LbMouseCheckPosition((previousGrabState != lbMouseGrabbed));
    }
    SDL_ShowCursor((lbAppActive ? SDL_DISABLE : SDL_ENABLE)); // show host OS cursor when window has lost focus
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
