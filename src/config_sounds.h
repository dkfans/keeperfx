/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_sounds.h
 *     Header file for config_sounds.c.
 * @par Purpose:
 *     Support of configuration files for sound name mappings.
 * @par Comment:
 *     Defines functions for loading sounds.cfg that maps sound names to IDs.
 * @author   KeeperFX Team
 * @date     14 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGSOUNDS_H
#define DK_CFGSOUNDS_H

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_sound.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

/** Configuration file data for sounds.cfg */
extern const struct ConfigFileData keeper_sounds_file_data;

/**
 * @brief Load the sounds.cfg configuration file
 * 
 * Parses the sounds.cfg file and registers all sound name→ID mappings
 * with the SoundManager registry.
 * 
 * @return true if loaded successfully, false otherwise
 */
TbBool load_sounds_config(void);

/**
 * @brief Load campaign-specific sounds configuration
 * 
 * Loads sounds.cfg from a campaign folder, allowing campaigns to
 * override or add sound mappings.
 * 
 * @param campaign_name Name of the campaign folder
 * @return true if loaded successfully
 */
TbBool load_campaign_sounds_config(const char* campaign_name);

/**
 * @brief Load level-specific sounds configuration
 * 
 * Loads sounds.cfg from a level folder for level-specific audio.
 * 
 * @param level_name Level folder name (e.g., "map00001")
 * @return true if loaded successfully
 */
TbBool load_level_sounds_config(const char* level_name);

/**
 * @brief Get the sample ID for a named sound
 * 
 * Convenience function that wraps sound_manager_get_id().
 * 
 * @param name Sound name (e.g., "REFUSAL", "FIREBALL")
 * @return Sample ID, or 0 if not found
 */
SoundSmplTblID get_sound_id(const char* name);

/**
 * @brief Check if a sound name is registered
 * 
 * @param name Sound name to check
 * @return true if registered
 */
TbBool is_sound_registered(const char* name);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // DK_CFGSOUNDS_H
