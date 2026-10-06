/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file user_prefs.h
 *     Per-user preferences which affect game state.
 * @par Purpose:
 *     Preferences each user brings into a game; stored per user in the game
 *     state, exchanged at net game start and recorded in replays.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_USER_PREFS_H
#define DK_USER_PREFS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum UserPreference {
    UPref_FrontView,
    UPref_Wibble,
    UPref_Cheats,
    UPref_SkipHeartZoom,
    UPref_StartingHighlightMode,
    UPref_StartingIsometricTilt,
    UPref_MaxZoomIso,
    UPref_MaxZoomFrontview,
    UPref_Censorship,
    UPref_WallHeight, // cluedo mode
    UPref_MapFade, // parchment map fade length in turns; 0=disabled
    UPref_StartingIsometricZoom,
    UPref_StartingFrontviewZoom,
    UPref_StartingTendencies, // CrTend_Imprison|CrTend_Flee; follows the user's in-game changes
    UPref_Count
};

/** Which situations replace an existing value with the user's local preference.
 *  Every preference is taken from the user when a game starts. */
enum UserPreferenceFlags {
    UPF_ApplyOnLoad     = 0x01,
    UPF_ApplyOnTakeover = 0x02,
    UPF_NewGame         = 0xFF, // every preference
};

typedef uint32_t UserPreferences[UPref_Count];

#ifdef __cplusplus
}
#endif
#endif
