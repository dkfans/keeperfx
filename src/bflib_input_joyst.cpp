/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_input_joyst.cpp
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
#include "bflib_joyst.h"
#include "bflib_inputctrl.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_video.h"
#include "bflib_planar.h"
#include "button_snapping.h"
#include "config_keeperfx.h"
#include "config.h"
#include "config_settings.h"
#include "front_input.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "frontend.h"
#include <SDL2/SDL.h>
#include "post_inc.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

static SDL_GameController *controller = NULL;

static TbControllerButtons internal_button_state = 0;
TbControllerButtons controller_button_state = 0;

static void open_controller(int device_index)
{
    if (controller)
        return;
    controller = SDL_GameControllerOpen(device_index);
}

static void close_controller()
{
    if (controller) {
        SDL_GameControllerClose(controller);
        controller = NULL;
    }
}

static TbControllerButtons SDL_gamecontrollerbutton_to_controllerbutton(const Uint8 button)
{
    switch (button) {
        case SDL_CONTROLLER_BUTTON_A: return CBtn_A;
        case SDL_CONTROLLER_BUTTON_B: return CBtn_B;
        case SDL_CONTROLLER_BUTTON_X: return CBtn_X;
        case SDL_CONTROLLER_BUTTON_Y: return CBtn_Y;
        case SDL_CONTROLLER_BUTTON_BACK: return CBtn_BACK;
        case SDL_CONTROLLER_BUTTON_START: return CBtn_START;
        case SDL_CONTROLLER_BUTTON_LEFTSTICK: return CBtn_LEFTSTICK;
        case SDL_CONTROLLER_BUTTON_RIGHTSTICK: return CBtn_RIGHTSTICK;
        case SDL_CONTROLLER_BUTTON_LEFTSHOULDER: return CBtn_LEFTSHOULDER;
        case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return CBtn_RIGHTSHOULDER;
        case SDL_CONTROLLER_BUTTON_DPAD_UP: return CBtn_DPAD_UP;
        case SDL_CONTROLLER_BUTTON_DPAD_DOWN: return CBtn_DPAD_DOWN;
        case SDL_CONTROLLER_BUTTON_DPAD_LEFT: return CBtn_DPAD_LEFT;
        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT: return CBtn_DPAD_RIGHT;
        case SDL_CONTROLLER_BUTTON_MISC1: return CBtn_MISC1;
        case SDL_CONTROLLER_BUTTON_PADDLE1: return CBtn_PADDLE1;
        case SDL_CONTROLLER_BUTTON_PADDLE2: return CBtn_PADDLE2;
        case SDL_CONTROLLER_BUTTON_PADDLE3: return CBtn_PADDLE3;
        case SDL_CONTROLLER_BUTTON_PADDLE4: return CBtn_PADDLE4;
        case SDL_CONTROLLER_BUTTON_TOUCHPAD: return CBtn_TOUCHPAD;
        
        default: break;
    }
    return CBtn_NONE;
}

static TbControllerButtons SDL_axis_to_controllerbutton(const Uint8 button, int8_t sign)
{
    switch (button) {
        case SDL_CONTROLLER_AXIS_TRIGGERLEFT: return CBtn_L2;
        case SDL_CONTROLLER_AXIS_TRIGGERRIGHT: return CBtn_R2;
        case SDL_CONTROLLER_AXIS_LEFTX: return  sign < 0 ? CBtn_LS_LEFT : CBtn_LS_RIGHT;
        case SDL_CONTROLLER_AXIS_LEFTY: return  sign < 0 ? CBtn_LS_UP : CBtn_LS_DOWN;
        case SDL_CONTROLLER_AXIS_RIGHTX: return sign < 0 ? CBtn_RS_LEFT : CBtn_RS_RIGHT;
        case SDL_CONTROLLER_AXIS_RIGHTY: return sign < 0 ? CBtn_RS_UP : CBtn_RS_DOWN;

        
        default: break;
    }
    return CBtn_NONE;
}

void JEvent(const SDL_Event *ev)
{
    SYNCDBG(10, "Starting");

    switch (ev->type)
    {
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    {
        Uint8 button_val = ev->cbutton.button;
        TbControllerButtons controller_btn = SDL_gamecontrollerbutton_to_controllerbutton(button_val);

        if (controller_btn != CBtn_NONE)
        {
            if (ev->type == SDL_CONTROLLERBUTTONDOWN)
            {
                if (!(internal_button_state & controller_btn)) {
                    controller_button_state |= controller_btn;
                    internal_button_state |= controller_btn;
                }
            }
            else
            {
                controller_button_state &= ~controller_btn;
                internal_button_state &= ~controller_btn;
            }
        }
    }
    break;

    case SDL_CONTROLLERDEVICEADDED:
        open_controller(ev->cdevice.which);
        break;
    case SDL_CONTROLLERDEVICEREMOVED:
        close_controller();
        break;
    case SDL_CONTROLLERAXISMOTION:
        {
            TbControllerButtons btn_pos = SDL_axis_to_controllerbutton(ev->caxis.axis, 1);
            TbControllerButtons btn_neg = SDL_axis_to_controllerbutton(ev->caxis.axis, -1);

            if (ev->caxis.value > 10000)
            {
                if (btn_neg != CBtn_NONE) {
                    controller_button_state &= ~btn_neg;
                    internal_button_state &= ~btn_neg;
                }
                if (btn_pos != CBtn_NONE && !(internal_button_state & btn_pos)) {
                    controller_button_state |= btn_pos;
                    internal_button_state |= btn_pos;
                }
            }
            else if (ev->caxis.value < -10000)
            {
                if (btn_pos != CBtn_NONE) {
                    controller_button_state &= ~btn_pos;
                    internal_button_state &= ~btn_pos;
                }
                if (btn_neg != CBtn_NONE && !(internal_button_state & btn_neg)) {
                    controller_button_state |= btn_neg;
                    internal_button_state |= btn_neg;
                }
            }
            else
            {
                if (btn_pos != CBtn_NONE) {
                    controller_button_state &= ~btn_pos;
                    internal_button_state &= ~btn_pos;
                }
                if (btn_neg != CBtn_NONE) {
                    controller_button_state &= ~btn_neg;
                    internal_button_state &= ~btn_neg;
                }
            }
        }
        break;
    case SDL_JOYAXISMOTION:
    case SDL_JOYBALLMOTION:
    case SDL_JOYHATMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
    default:
        break;
    }
}

void controller_rumble(long ms)
{
    if (controller != NULL) {
        SDL_GameControllerRumble(controller, 0xFFFF, 0xFFFF, ms);
    }
}

void init_controller_input()
{
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) < 0) {
        ERRORLOG("SDL init: %s",SDL_GetError());
        return;
    }
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
        }
    }
}

float cbtn_axis_value(TbControllerButtons btn)
{
    if (btn & (CBtn_LS_LEFT|CBtn_LS_RIGHT|CBtn_LS_UP|CBtn_LS_DOWN|CBtn_RS_LEFT|CBtn_RS_RIGHT|CBtn_RS_UP|CBtn_RS_DOWN))
    {
        return .5f;
    }
    else if (btn == controller_button_state)
        return 1.0;
    else
        return 0.0;
}

TbBool controller_connected()
{
    return controller != NULL;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
