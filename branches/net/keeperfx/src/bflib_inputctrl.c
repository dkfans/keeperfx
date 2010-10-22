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

/******************************************************************************/
extern volatile TbBool lbScreenInitialised;
extern volatile TbBool lbHasSecondSurface;
extern SDL_Color lbPaletteColors[PALETTE_COLORS];

volatile TbBool lbAppActive;
volatile int lbUserQuit = 0;
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

/**
 * Temporary function that emulates the value of the extension bit in WM_KEYUP/WM_KEYDOWN
 * until KeyboardProc can be written.
 * @param sym
 * @return
 */
static bool has_ext_bit(SDLKey sym)
{
    switch (sym) {
    case SDLK_RALT:
    case SDLK_RCTRL:
    case SDLK_INSERT:
    case SDLK_HOME:
    case SDLK_END:
    case SDLK_DELETE:
    case SDLK_PAGEUP:
    case SDLK_PAGEDOWN:
    case SDLK_UP:
    case SDLK_DOWN:
    case SDLK_LEFT:
    case SDLK_RIGHT:
    case SDLK_KP_DIVIDE:
    case SDLK_KP_ENTER:
        return true;
    default:
        return false;
    }
}

/**
 * Converts an SDL_KeyboardEvent to an LPARAM parameter as per Win32 API, because that's
 * what DK's KeyboardProc expects.
 * @param ev
 * @return
 */
static LPARAM make_lparam_from_sdl_key_event(const SDL_KeyboardEvent *ev)
{
    LPARAM lParam;

    //bit 31: key up or down?
    lParam = ev->type == SDL_KEYUP? 0x80000000 : 0;

    //TODO: detect if key already was down (bit 30 of LPARAM)

    //bit 29: state of alt key(s)
    if (ev->keysym.mod & KMOD_ALT) {
        lParam |= 0x20000000;
    }

    //bits 25-28 reserved, leave 0

    //bit 24: extension bit
    if (has_ext_bit(ev->keysym.sym)) {
        lParam |= 0x1000000;
    }

    //bits 16-23: scancode
    lParam |= ev->keysym.scancode << 16;

    //bits 0-15: repeat count, always let this be 1 due to SDL semantics
    lParam |= 1;

    return lParam;
}

static void process_event(const SDL_Event *ev)
{
    struct TbPoint mousePos;
    int x, y;
    SYNCDBG(10, "Starting");

    switch (ev->type)
    {
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        KeyboardProc(0, 0, make_lparam_from_sdl_key_event(&ev->key));
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
        }
        if ((lbAppActive) && (!lbHasSecondSurface) && (lbDisplay.Palette != NULL)) {
            // Below is the faster version of LbPaletteSet(lbDisplay.Palette);
            SDL_SetColors(lbDrawSurface,lbPaletteColors, 0, PALETTE_COLORS);
        }
        break;

    case SDL_JOYAXISMOTION:
    case SDL_JOYBALLMOTION:
    case SDL_JOYHATMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
        //TODO [input] make joypad support
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
