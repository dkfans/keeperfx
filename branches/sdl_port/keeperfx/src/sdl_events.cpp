/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontend.cpp
 *     SDL event handler.
 * @par Purpose:
 *     Interprets SDL Events.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     13 April 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "keeperfx.hpp"

#include <list>
#include <SDL.h>
#include <windows.h>

#include "bflib_mouse.h"

struct DelayEvent {
	unsigned long theTick;
	SDL_Event theEvent;
};

typedef std::list<DelayEvent> DelayEventList;

static DelayEventList delayEvList;

/**
 * Converts an SDL mouse button event type and the corresponding mouse button to a Win32 API message.
 * @param eventType
 * @param button
 * @return
 */
static unsigned get_mouse_message_type(int eventType, unsigned button) {
    if (eventType == SDL_MOUSEBUTTONDOWN) {
        switch (button)  {
        case SDL_BUTTON_LEFT: return WM_LBUTTONDOWN;
        case SDL_BUTTON_MIDDLE: return WM_MBUTTONDOWN;
        case SDL_BUTTON_RIGHT: return WM_RBUTTONDOWN;
        }
    }
    else if (eventType == SDL_MOUSEBUTTONUP) {
        switch (button) {
        case SDL_BUTTON_LEFT: return WM_LBUTTONUP;
        case SDL_BUTTON_MIDDLE: return WM_MBUTTONUP;
        case SDL_BUTTON_RIGHT: return WM_RBUTTONUP;
        }
    }

    ERRORMSG("Shouldn't get here");
    return 0;
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
static LPARAM make_lparam_from_sdl_key_event(const SDL_KeyboardEvent & ev)
{
	LPARAM lParam;

	//bit 31: key up or down?
	lParam = ev.type == SDL_KEYUP? 0x80000000 : 0;

	//TODO: detect if key already was down (bit 30 of LPARAM)

	//bit 29: state of alt key(s)
	if (ev.keysym.mod & KMOD_ALT) {
		lParam |= 0x20000000;
	}

	//bits 25-28 reserved, leave 0

	//bit 24: extension bit
	if (has_ext_bit(ev.keysym.sym)) {
		lParam |= 0x1000000;
	}

	//bits 16-23: scancode
	lParam |= ev.keysym.scancode << 16;

	//bits 0-15: repeat count, always let this be 1 due to SDL semantics
	lParam |= 1;

	return lParam;
}

static void fake_key_press(SDLKey sym, unsigned scancode, bool down, int newTick)
{
	DelayEvent delayEv;

	//make sure there's no key press for same symbol already in queue
	for (DelayEventList::const_iterator it = delayEvList.begin(), end = delayEvList.end(); it != end; ++it) {
		if (it->theEvent.key.keysym.sym == sym) {
			return;
		}
	}

	delayEv.theEvent.key.keysym.sym = sym;
	delayEv.theEvent.key.keysym.scancode = scancode;
	delayEv.theEvent.key.keysym.mod = SDL_GetModState();
	delayEv.theEvent.type = down? SDL_KEYDOWN : SDL_KEYUP;

	if (down) {
		SDL_PushEvent(&delayEv.theEvent);
	}
	else {
		delayEv.theTick = newTick;
		delayEvList.push_back(delayEv);
	}
}

static void process_event(const SDL_Event & ev, TbBool mouseWheelRemap) {
	static bool middleDown = false;
	static int middleX, middleY;

	struct tagPOINT mousePos;
	int x, y;

	unsigned long thisTick = SDL_GetTicks();

	SYNCDBG(10, "Starting");

	switch (ev.type) {
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		KeyboardProc(0, 0, make_lparam_from_sdl_key_event(ev.key));
		break;
	case SDL_MOUSEMOTION:
		if (middleDown) { //mouseWheelRemap == true implied
			if (ev.motion.yrel > 0) {
				fake_key_press(SDLK_END, 79, true, thisTick + 100);
				fake_key_press(SDLK_END, 79, false, thisTick + 100);
			}
			else if (ev.motion.yrel < 0) {
				fake_key_press(SDLK_HOME, 71, true, thisTick + 100);
				fake_key_press(SDLK_HOME, 71, false, thisTick + 100);
			}
		}
		else {
			SDL_GetMouseState(&x, &y);
			mousePos.x = x;
			mousePos.y = y;
			mouseControl(WM_MOUSEMOVE, &mousePos);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		SDL_GetMouseState(&x, &y);
		mousePos.x = x;
		mousePos.y = y;

		/* TODO: Instead of faking key presses for mouse wheel actions,
		 * rewrite KeyboardProc and find out how to call those actions immediately.
		 * This is important because this experimental code does not take key remappings into account nor
		 * accurate scan codes.
		 */
		if (mouseWheelRemap && ev.button.button == SDL_BUTTON_WHEELUP) {
			fake_key_press(SDLK_PAGEDOWN, 81, ev.button.type == SDL_MOUSEBUTTONDOWN, thisTick + 100);
		}
		else if (mouseWheelRemap && ev.button.button == SDL_BUTTON_WHEELDOWN) {
			fake_key_press(SDLK_DELETE, 83, ev.button.type == SDL_MOUSEBUTTONDOWN, thisTick + 100);
		}
		else if (mouseWheelRemap && ev.button.button == SDL_BUTTON_MIDDLE) {
			if (ev.button.type == SDL_MOUSEBUTTONDOWN) {
				middleDown = true;
				middleX = ev.button.x;
				middleY = ev.button.y;
			}
			else {
				middleDown = false;
			}
		}
		else {
			mouseControl(get_mouse_message_type(ev.type, ev.button.button), &mousePos);
		}
		break;
	case SDL_ACTIVEEVENT:
		SDL_ShowCursor(ev.active.gain? SDL_DISABLE : SDL_ENABLE);
		break;
	}
}

void poll_sdl_events(TbBool mouseWheelRemap) {
    SDL_Event ev;
    unsigned long thisTick;

    SYNCDBG(10, "Starting");

    //handle delay events (notice this does not take events added by this loop into account
    //because delayEvList.end() is saved)
    thisTick = SDL_GetTicks();
    for (DelayEventList::iterator it = delayEvList.begin(), end = delayEvList.end(), curr; it != end; ) {
    	curr = it++;

    	if (curr->theTick <= thisTick) {
    		process_event(curr->theEvent, mouseWheelRemap);
    		delayEvList.erase(curr);
    	}
    }

    //TODO: handle double clicks, SDL doesn't generate them

    //process events until event queue is empty
    while (SDL_PollEvent(&ev)) {
    	process_event(ev, mouseWheelRemap);
    }
}
