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
#include <math.h>
#include <chrono>
#include "bflib_inputctrl.h"
#include "bflib_basics.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_video.h"
#include "bflib_planar.h"
#include "frontmenu_ingame_tabs.h"
#include "config_keeperfx.h"
#include "config_settings.h"
#include "front_input.h"
#include "game_legacy.h"
#include <SDL2/SDL.h>
#include "post_inc.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

static SDL_GameController *controller = NULL;
static SDL_Joystick *joystick = NULL;

static TbBool lt_pressed = false;
static TbBool rt_pressed = false;


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

void JEvent(const SDL_Event *ev)
{
    SYNCDBG(10, "Starting");

    switch (ev->type)
    {
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

    case SDL_JOYDEVICEADDED:
        open_controller(ev->jdevice.which);
        break;
    case SDL_JOYDEVICEREMOVED:
        close_controller(ev->jdevice.which);
        break;
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


void init_controller_input()
{
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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
