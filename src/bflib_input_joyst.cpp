/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_input_joyst.cpp
 *     Input devices control and polling.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "bflib_joyst.h"
#include "bflib_inputctrl.h"
#include "bflib_keybrd.h"
#include "bflib_mouse.h"
#include "bflib_video.h"
#include "bflib_planar.h"
#include "config_keeperfx.h"
#include "config.h"
#include "config_settings.h"
#include "front_input.h"
#include "game_legacy.h"
#include "kjm_input.h"
#include "frontend.h"
#include <SDL3/SDL.h>
#include "post_inc.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

static SDL_Gamepad *controller = NULL;

static TbControllerButtons internal_button_state = 0;
TbControllerButtons controller_button_state = 0;

static void open_controller(SDL_JoystickID id)
{
    if (controller)
        return;
    controller = SDL_OpenGamepad(id);
}

static void close_controller()
{
    if (controller) {
        SDL_CloseGamepad(controller);
        controller = NULL;
    }
}

static TbControllerButtons SDL_gamepadbutton_to_controllerbutton(SDL_GamepadButton button)
{
    switch (button) {
        case SDL_GAMEPAD_BUTTON_SOUTH:          return CBtn_A;
        case SDL_GAMEPAD_BUTTON_EAST:           return CBtn_B;
        case SDL_GAMEPAD_BUTTON_WEST:           return CBtn_X;
        case SDL_GAMEPAD_BUTTON_NORTH:          return CBtn_Y;
        case SDL_GAMEPAD_BUTTON_BACK:           return CBtn_BACK;
        case SDL_GAMEPAD_BUTTON_START:          return CBtn_START;
        case SDL_GAMEPAD_BUTTON_LEFT_STICK:     return CBtn_LEFTSTICK;
        case SDL_GAMEPAD_BUTTON_RIGHT_STICK:    return CBtn_RIGHTSTICK;
        case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:  return CBtn_LEFTSHOULDER;
        case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER: return CBtn_RIGHTSHOULDER;
        case SDL_GAMEPAD_BUTTON_DPAD_UP:        return CBtn_DPAD_UP;
        case SDL_GAMEPAD_BUTTON_DPAD_DOWN:      return CBtn_DPAD_DOWN;
        case SDL_GAMEPAD_BUTTON_DPAD_LEFT:      return CBtn_DPAD_LEFT;
        case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:     return CBtn_DPAD_RIGHT;
        case SDL_GAMEPAD_BUTTON_MISC1:          return CBtn_MISC1;
        case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE1:  return CBtn_PADDLE1;
        case SDL_GAMEPAD_BUTTON_LEFT_PADDLE1:   return CBtn_PADDLE2;
        case SDL_GAMEPAD_BUTTON_RIGHT_PADDLE2:  return CBtn_PADDLE3;
        case SDL_GAMEPAD_BUTTON_LEFT_PADDLE2:   return CBtn_PADDLE4;
        case SDL_GAMEPAD_BUTTON_TOUCHPAD:       return CBtn_TOUCHPAD;
        default: break;
    }
    return CBtn_NONE;
}

static TbControllerButtons SDL_axis_to_controllerbutton(SDL_GamepadAxis axis, int8_t sign)
{
    switch (axis) {
        case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:  return CBtn_L2;
        case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: return CBtn_R2;
        case SDL_GAMEPAD_AXIS_LEFTX:  return sign < 0 ? CBtn_LS_LEFT : CBtn_LS_RIGHT;
        case SDL_GAMEPAD_AXIS_LEFTY:  return sign < 0 ? CBtn_LS_UP   : CBtn_LS_DOWN;
        case SDL_GAMEPAD_AXIS_RIGHTX: return sign < 0 ? CBtn_RS_LEFT : CBtn_RS_RIGHT;
        case SDL_GAMEPAD_AXIS_RIGHTY: return sign < 0 ? CBtn_RS_UP   : CBtn_RS_DOWN;
        default: break;
    }
    return CBtn_NONE;
}

void JEvent(const SDL_Event *ev)
{
    SYNCDBG(10, "Starting");

    switch (ev->type)
    {
    case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
    case SDL_EVENT_GAMEPAD_BUTTON_UP:
    {
        SDL_GamepadButton button_val = (SDL_GamepadButton)ev->gbutton.button;
        TbControllerButtons controller_btn = SDL_gamepadbutton_to_controllerbutton(button_val);

        if (controller_btn != CBtn_NONE)
        {
            if (ev->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
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

    case SDL_EVENT_GAMEPAD_ADDED:
        open_controller(ev->gdevice.which);
        break;
    case SDL_EVENT_GAMEPAD_REMOVED:
        close_controller();
        break;
    case SDL_EVENT_GAMEPAD_AXIS_MOTION:
        {
            SDL_GamepadAxis axis = (SDL_GamepadAxis)ev->gaxis.axis;
            TbControllerButtons btn_pos = SDL_axis_to_controllerbutton(axis, 1);
            TbControllerButtons btn_neg = SDL_axis_to_controllerbutton(axis, -1);

            if (ev->gaxis.value > 10000)
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
            else if (ev->gaxis.value < -10000)
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
    case SDL_EVENT_JOYSTICK_AXIS_MOTION:
    case SDL_EVENT_JOYSTICK_HAT_MOTION:
    case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
    case SDL_EVENT_JOYSTICK_BUTTON_UP:
    default:
        break;
    }
}

void controller_rumble(long ms)
{
    if (controller != NULL && last_used_input_device == ID_Controller) {
        SDL_RumbleGamepad(controller, 0xFFFF, 0xFFFF, (Uint32)ms);
    }
}

void init_controller_input()
{
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        ERRORLOG("SDL gamepad init: %s", SDL_GetError());
        return;
    }
    SDL_AddGamepadMappingsFromFile(prepare_file_path(FGrp_FxData, "gamecontrollerdb.txt"));

    // Open the first available gamepad, if any.
    int count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);
    if (gamepads && count > 0)
    {
        controller = SDL_OpenGamepad(gamepads[0]);
        if (controller == NULL) {
            WARNLOG("Could not open gamepad 0: %s", SDL_GetError());
        } else {
            const char* name = SDL_GetGamepadName(controller);
            SYNCLOG("Gamepad connected: %s", name ? name : "Unknown");
        }
    }
    if (gamepads)
        SDL_free(gamepads);
}

float cbtn_axis_value(TbControllerButtons btn)
{
    if (controller == NULL) {
        return 0.0f;
    }

    const float deadzone = 10000.0f;
    const float max_axis = 32767.0f;

    float value = 0.0f;

    auto sample_direction = [&](Sint16 axis_value, float sign) {
        float directional = ((float)axis_value) * sign;
        if (directional <= deadzone) {
            return 0.0f;
        }
        float normalized = directional / max_axis;
        if (normalized > 1.0f) {
            normalized = 1.0f;
        }
        return normalized;
    };

    if (btn & (CBtn_LS_LEFT|CBtn_LS_RIGHT|CBtn_LS_UP|CBtn_LS_DOWN|CBtn_RS_LEFT|CBtn_RS_RIGHT|CBtn_RS_UP|CBtn_RS_DOWN))
    {
        const Sint16 left_x = SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFTX);
        const Sint16 left_y = SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_LEFTY);
        const Sint16 right_x = SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHTX);
        const Sint16 right_y = SDL_GetGamepadAxis(controller, SDL_GAMEPAD_AXIS_RIGHTY);

        if (btn & CBtn_LS_LEFT)  value = std::max(value, sample_direction(left_x,  -1.0f));
        if (btn & CBtn_LS_RIGHT) value = std::max(value, sample_direction(left_x,   1.0f));
        if (btn & CBtn_LS_UP)    value = std::max(value, sample_direction(left_y,  -1.0f));
        if (btn & CBtn_LS_DOWN)  value = std::max(value, sample_direction(left_y,   1.0f));
        if (btn & CBtn_RS_LEFT)  value = std::max(value, sample_direction(right_x, -1.0f));
        if (btn & CBtn_RS_RIGHT) value = std::max(value, sample_direction(right_x,  1.0f));
        if (btn & CBtn_RS_UP)    value = std::max(value, sample_direction(right_y, -1.0f));
        if (btn & CBtn_RS_DOWN)  value = std::max(value, sample_direction(right_y,  1.0f));

        if (value > 0.0f) {
            return value;
        }
    }

    if ((controller_button_state & btn) != 0)
        return 1.0;

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
