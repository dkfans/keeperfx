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
#include "front_landview.h"
#include "frontmenu_select.h"
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
static SDL_Joystick *joystick = NULL;

static TbBool lt_pressed = false;
static TbBool rt_pressed = false;


static Uint8 prev_back = 0;
static Uint8 prev_leftshoulder = 0;
static Uint8 prev_rightshoulder = 0;
static Uint8 prev_joy_left = 0;
//static Uint8 prev_joy_right = 0;

static Uint8 prev_dpad_up    = 0;
static Uint8 prev_dpad_down  = 0;
static Uint8 prev_dpad_left  = 0;
static Uint8 prev_dpad_right = 0;

static float mouse_accum_x = 0.0f;
static float mouse_accum_y = 0.0f;

float movement_accum_x = 0.0f;
float movement_accum_y = 0.0f;

#define TimePoint std::chrono::high_resolution_clock::time_point
#define TimeNow std::chrono::high_resolution_clock::now()

static TimePoint delta_time_previous_timepoint = TimeNow;
static float input_delta_time = 0.0f;

// Forward declarations
static void snap_cursor_to_button(long *snap_to_x, long *snap_to_y);
extern void gui_get_creature_in_battle(struct GuiButton *gbtn);
extern void gui_setup_friend_over(struct GuiButton *gbtn);
/******************************************************************************/

static TbBool try_scroll_select_list_edge(long mouse_x, long mouse_y, float dx, float dy)
{
    if (fabsf(dy) < fabsf(dx)) {
        return false;
    }

    const TbBool is_down = (dy > 0.0f);
    const TbBool is_up = (dy < 0.0f);

    for (int i = 0; i < ACTIVE_BUTTONS_COUNT; i++) {
        struct GuiButton* gbtn = &active_buttons[i];

        if (!(gbtn->flags & LbBtnF_Active)) continue;
        if (!(gbtn->flags & LbBtnF_Visible)) continue;
        if (!(gbtn->flags & LbBtnF_Enabled)) continue;
        if (gbtn->click_event == NULL) continue;

        if (mouse_x < gbtn->scr_pos_x || mouse_x >= (gbtn->scr_pos_x + gbtn->width) ||
            mouse_y < gbtn->scr_pos_y || mouse_y >= (gbtn->scr_pos_y + gbtn->height)) {
            continue;
        }

        if (is_down && gbtn->content.lval != 51) {
            return false;
        }
        if (is_up && gbtn->content.lval != 45) {
            return false;
        }

        if (gbtn->click_event == frontend_level_select) {
            if (is_down) {
                frontend_level_select_down(NULL);
            } else {
                frontend_level_select_up(NULL);
            }
            return true;
        }

        if (gbtn->click_event == frontend_campaign_select) {
            if (is_down) {
                frontend_campaign_select_down(NULL);
            } else {
                frontend_campaign_select_up(NULL);
            }
            return true;
        }

        if (gbtn->click_event == frontend_mappack_select) {
            if (is_down) {
                frontend_mappack_select_down(NULL);
            } else {
                frontend_mappack_select_up(NULL);
            }
            return true;
        }

        return false;
    }

    return false;
}

static float get_button_score(long mouse_x, long mouse_y, long btn_center_x, long btn_center_y, float dx, float dy, float MIN_DOT)
{
    // Vector from mouse to button
    float to_btn_x = (float)(btn_center_x - mouse_x);
    float to_btn_y = (float)(btn_center_y - mouse_y);
    float dist = sqrtf(to_btn_x * to_btn_x + to_btn_y * to_btn_y);
    
    if (dist < 5.0f) return -1.0f; // Skip if already on button
    
    // Normalize
    to_btn_x /= dist;
    to_btn_y /= dist;
    
    // Check alignment with desired direction
    float dot = dx * to_btn_x + dy * to_btn_y;
    if (dot < MIN_DOT) return -1.0f;
    
    // Score based on alignment and distance (prefer close + aligned)
    float score = dot / (1.0f + dist / 200.0f);
    return score;
}

static float get_battle_buttons_top_score(const struct GuiButton* gbtn, long *btn_center_x, long *btn_center_y, long mouse_x, long mouse_y, float dx, float dy, float MIN_DOT)
{
    int visbtl_id = gbtn->btype_value & LbBFeF_IntValueMask;
    struct Dungeon* dungeon = get_players_num_dungeon(my_player_number);
    BattleIndex battle_id = dungeon->visible_battles[visbtl_id];
    struct CreatureBattle* battle = creature_battle_get(battle_id);
    if (creature_battle_invalid(battle)) {
        return -1.0f;
    }
    if (battle->fighters_num <= 0) {
        return -1.0f;
    }

    long btn_y = gbtn->scr_pos_y + gbtn->height / 2;
    float best_score = -1.0f;

    TbBool friendly = (gbtn->ptover_event == gui_setup_friend_over);

    for (int i = 0; i < MESSAGE_BATTLERS_COUNT; i++) {
        struct Thing* thing;
        if (friendly)
            thing = thing_get(friendly_battler_list[(MESSAGE_BATTLERS_COUNT * visbtl_id) + i]);
        else
            thing = thing_get(enemy_battler_list[(MESSAGE_BATTLERS_COUNT * visbtl_id) + i]);
        
        if (thing_exists(thing) && thing_revealed(thing, my_player_number))
        {
            const int slot_w = gbtn->width / 7;
            int btn_x;
            if (friendly)
                btn_x = gbtn->scr_pos_x + (6 - i) * slot_w + slot_w / 2;
            else
                btn_x = gbtn->scr_pos_x + i * slot_w + slot_w / 2;
            
            float score = get_button_score(mouse_x, mouse_y, btn_x, btn_y, dx, dy, MIN_DOT);
            if (score > best_score) {
                *btn_center_x = btn_x;
                *btn_center_y = btn_y;
                
                best_score = score;
            }
        }
    }

    return best_score;
}

static TbBool find_nearest_button_in_direction(long mouse_x, long mouse_y, float dx, float dy, long *snap_to_x, long *snap_to_y)
{
    float best_score = -1.0f;
    const float MIN_DOT = 0.3f; // Minimum alignment required
    *snap_to_x = 0;
    *snap_to_y = 0;

    TbBool btn_found = false;
    
    // Normalize direction
    float mag = sqrtf(dx * dx + dy * dy);
    if (mag < 0.01f) return false;
    dx /= mag;
    dy /= mag;
    
    for (int i = 0; i < ACTIVE_BUTTONS_COUNT; i++) {
        struct GuiButton* gbtn = &active_buttons[i];
        
        // Skip inactive, invisible, or disabled buttons
        if (!(gbtn->flags & LbBtnF_Active)) continue; 
        if (!(gbtn->flags & LbBtnF_Visible)) continue;
        if (!(gbtn->flags & LbBtnF_Enabled)) continue;
        if (gbtn->click_event == NULL) continue;

        
        // Calculate button center
        long btn_center_x = gbtn->pos_x + gbtn->width / 2;
        long btn_center_y = gbtn->pos_y + gbtn->height / 2;

        float score;

        if (gbtn->click_event == gui_get_creature_in_battle)
        {
            score = get_battle_buttons_top_score(gbtn,&btn_center_x,&btn_center_y, mouse_x, mouse_y, dx, dy, MIN_DOT);
        }
        else
        {
            score = get_button_score(mouse_x, mouse_y, btn_center_x, btn_center_y, dx, dy, MIN_DOT);
        }
        
        if (score > best_score) {
            *snap_to_x = btn_center_x;
            *snap_to_y = btn_center_y;
            
            best_score = score;
            btn_found = true;
        }
    }

    return btn_found;
}

static TbBool find_nearest_landview_flag_in_direction(long mouse_x, long mouse_y, float dx, float dy, long *snap_to_x, long *snap_to_y)
{
    const float MIN_DOT = 0.3f;
    float best_score = -1.0f;
    *snap_to_x = 0;
    *snap_to_y = 0;

    float mag = sqrtf(dx * dx + dy * dy);
    if (mag < 0.01f) return false;
    dx /= mag;
    dy /= mag;

    long screen_max_x = map_info.screen_shift_x + lbDisplay.PhysicalScreenWidth * 16 / units_per_pixel_landview;
    long screen_max_y = map_info.screen_shift_y + lbDisplay.PhysicalScreenHeight * 16 / units_per_pixel_landview;

    TbBool flag_found = false;
    struct LevelInformation* lvinfo = get_first_level_info();
    while (lvinfo != NULL && lvinfo->lvnum != 0)
    {
        if ((frontend_menu_state == FeSt_LAND_VIEW && lvinfo->level_type & LvKind_IsMulti)
         || (frontend_menu_state == FeSt_NETLAND_VIEW && (lvinfo->level_type & (LvKind_IsSingle|LvKind_IsExtra|LvKind_IsBonus))))
        {
            lvinfo = get_next_level_info(lvinfo);
            continue;
        }
            
        if (lvinfo->state == LvSt_Visible)
        {
            if ((lvinfo->ensign_zoom_x >= map_info.screen_shift_x) && (lvinfo->ensign_zoom_x < screen_max_x)
             && (lvinfo->ensign_zoom_y >= map_info.screen_shift_y) && (lvinfo->ensign_zoom_y < screen_max_y))
            {
                long btn_center_x = scale_value_landview(lvinfo->ensign_x - (long)map_info.screen_shift_x);
                long btn_center_y = scale_value_landview(lvinfo->ensign_y - (long)map_info.screen_shift_y);
                const struct TbSprite *spr = get_ensign_sprite_for_level(lvinfo, 0);
                if (spr != NULL)
                {
                    btn_center_y = scale_value_landview(lvinfo->ensign_y - (long)map_info.screen_shift_y - (long)((spr->SHeight * 2) / 3));
                }
                float score = get_button_score(mouse_x, mouse_y, btn_center_x, btn_center_y, dx, dy, MIN_DOT);
                if (score > best_score)
                {
                    *snap_to_x = btn_center_x;
                    *snap_to_y = btn_center_y;
                    best_score = score;
                    flag_found = true;
                }
            }
        }
        lvinfo = get_next_level_info(lvinfo);
    }

    return flag_found;

}

static void snap_cursor_to_button(long *snap_to_x, long *snap_to_y)
{
    if (snap_to_x == NULL || snap_to_y == NULL) return;
    
    long btn_center_x = *snap_to_x;
    long btn_center_y = *snap_to_y;
    
    struct TbPoint delta;
    delta.x = btn_center_x - GetMouseX();
    delta.y = btn_center_y - GetMouseY();
    
    mouseControl(MActn_MOUSEMOVE, &delta);

    mouse_accum_x = 0.0f;
    mouse_accum_y = 0.0f;
}

static void snap_to_direction(long mouse_x, long mouse_y, float dx, float dy)
{
    if (try_scroll_select_list_edge(mouse_x, mouse_y, dx, dy)) {
        return;
    }

    long snap_to_x, snap_to_y;
    TbBool found;
    if ((frontend_menu_state == FeSt_LAND_VIEW) || (frontend_menu_state == FeSt_NETLAND_VIEW)) {
        found = find_nearest_landview_flag_in_direction(mouse_x, mouse_y, dx, dy, &snap_to_x, &snap_to_y);
    }
    else {
        found = find_nearest_button_in_direction(mouse_x, mouse_y, dx, dy, &snap_to_x, &snap_to_y);
    }

    if (found)
        snap_cursor_to_button(&snap_to_x, &snap_to_y);
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
            // D-pad for button snapping
            Uint8 dpad_up =    SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
            Uint8 dpad_down =  SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            Uint8 dpad_left =  SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            Uint8 dpad_right = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
            Uint8 btn_B = SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_B);
            
            if (!btn_B) {
            
                // Check for button press (edge detection)
                if (dpad_up && !prev_dpad_up) {
                    snap_to_direction(GetMouseX(), GetMouseY(), 0.0f, -1.0f);
                }
                if (dpad_down && !prev_dpad_down) {
                    snap_to_direction(GetMouseX(), GetMouseY(), 0.0f, 1.0f);
                }
                if (dpad_left && !prev_dpad_left) {
                    snap_to_direction(GetMouseX(), GetMouseY(), -1.0f, 0.0f);
                }
                if (dpad_right && !prev_dpad_right) {
                    snap_to_direction(GetMouseX(), GetMouseY(), 1.0f, 0.0f);
                }
            }
            else {
                lbKeyOn[KC_UP] = dpad_up;
                lbKeyOn[KC_DOWN] = dpad_down;
                lbKeyOn[KC_RIGHT] = dpad_left;
                lbKeyOn[KC_LEFT] = dpad_right;
            }
            
            prev_dpad_up = dpad_up;
            prev_dpad_down = dpad_down;
            prev_dpad_left = dpad_left;
            prev_dpad_right = dpad_right;
            
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
        lbKeyOn[KC_HOME] =   SDL_JoystickGetButton(joystick, 10); // D-pad up
        lbKeyOn[KC_END] =    SDL_JoystickGetButton(joystick, 12); // D-pad down
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
