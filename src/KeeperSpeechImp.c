/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file KeeperSpeechImp.c
 *     Import functions for KeeperSpeech module.
 * @par Purpose:
 *     To for the corresponding platform load and make KeeperSpeech functions available.
 *     If KeeperSpeech is not available for platform, functions return gracefully and
 *     pretend that nothing happens.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "KeeperSpeech.h"

#ifdef _WIN32
#include <stdarg.h>
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HINSTANCE ks_lib;
#endif

static struct
{
    fpKeeperSpeechErrorMessage  error_message;
    fpKeeperSpeechExit          exit;
    fpKeeperSpeechPopEvent      pop_event;
    fpKeeperSpeechClearEvents   clear_events;
} ks_fn;

static void clean_up()
{
#ifdef _WIN32
    if (ks_lib) {
        FreeLibrary(ks_lib);
        ks_lib = NULL;
    }
#endif

    memset(&ks_fn, 0, sizeof(ks_fn));
}

KEEPERSPEECH_REASON KeeperSpeechInit(void)
{
    fpKeeperSpeechInit init = NULL;

#ifdef _WIN32
    if (ks_lib) {
        return KSR_ALREADY_INIT;
    }

    ks_lib = LoadLibrary("KeeperSpeech.dll");
    if (!ks_lib) {
        return KSR_NO_LIB_INSTALLED;
    }

    init = (fpKeeperSpeechInit) GetProcAddress(ks_lib, "KeeperSpeechInit");
    ks_fn.error_message = (fpKeeperSpeechErrorMessage)(void *) GetProcAddress(ks_lib, "KeeperSpeechErrorMessage");
    ks_fn.exit = (fpKeeperSpeechExit) GetProcAddress(ks_lib, "KeeperSpeechExit");
    ks_fn.pop_event = (fpKeeperSpeechPopEvent) GetProcAddress(ks_lib, "KeeperSpeechPopEvent");
    ks_fn.clear_events = (fpKeeperSpeechClearEvents) GetProcAddress(ks_lib, "KeeperSpeechClearEvents");

    //check for critical functions
    if (!init ||
        !ks_fn.error_message ||
        !ks_fn.exit) {

        clean_up();
        return KSR_NO_LIB_INSTALLED;
    }
#endif

    //check in case of unimplemented platform
    if (!init) {
        return KSR_NO_LIB_INSTALLED;
    }

    KEEPERSPEECH_REASON reason = init();
    if (reason != KSR_OK) {
        clean_up();
    }

    return reason;
}

const char * KeeperSpeechErrorMessage(KEEPERSPEECH_REASON reason)
{
    if (ks_fn.error_message) {
        return ks_fn.error_message(reason);
    }

    return "KeeperSpeech module not found";
}

void KeeperSpeechExit(void)
{
    if (ks_fn.exit) {
        ks_fn.exit();
    }

    clean_up();
}

KEEPERSPEECH_REASON KeeperSpeechPopEvent(KEEPERSPEECH_EVENT * ev)
{
    if (ks_fn.pop_event) {
        return ks_fn.pop_event(ev);
    }

    return KSR_NOMOREEVENTS;
}

void KeeperSpeechClearEvents(void)
{
    if (ks_fn.clear_events) {
        ks_fn.clear_events();
    }
}
