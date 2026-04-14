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
#include <math.h>
#include <chrono>
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

//static float mouse_accum_x = 0.0f;
//static float mouse_accum_y = 0.0f;
float movement_accum_x = 0.0f;
float movement_accum_y = 0.0f;

#define TimePoint std::chrono::high_resolution_clock::time_point
#define TimeNow std::chrono::high_resolution_clock::now()

static TimePoint delta_time_previous_timepoint = TimeNow;
static float input_delta_time = 0.0f;



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

/*7
#define STICK_DEADZONE      0.15f

static void poll_controller_movement(Sint16 lx, Sint16 ly)
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
static void poll_controller_mouse(Sint16 rx, Sint16 ry)
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
*/

static float get_input_delta_time()
{
    long double frame_time_in_nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(TimeNow - delta_time_previous_timepoint).count();
    delta_time_previous_timepoint = TimeNow;
    float calculated_delta_time = (frame_time_in_nanoseconds/1000000000.0) * game_num_fps;
    return min(calculated_delta_time, 1.0f);
}
void poll_controller()
{
    input_delta_time = get_input_delta_time();
    if (controller != NULL) {
        
        //analog sticks and dpad, layout based on what's available, with mouse being most important, then movement, then cam rotation/zoom
/*
        
        Sint16 leftX = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTX);
        Sint16 leftY = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_LEFTY);
        poll_controller_movement(leftX, leftY);

        Sint16 rx = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTX);
        Sint16 ry = SDL_GameControllerGetAxis(controller, SDL_CONTROLLER_AXIS_RIGHTY);
        poll_controller_mouse(rx, ry);



        TbBool has_triggers =
            SDL_GameControllerHasAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERLEFT) &&
            SDL_GameControllerHasAxis(controller, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

        if(has_triggers)
        {
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

*/
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

TbBool btn_is_axis()
{
    return true;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif
