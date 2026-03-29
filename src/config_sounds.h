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
 * Loads sounds.cfg from a campaign's levels_location folder, allowing
 * campaigns to override or add sound mappings.
 *
 * @param levels_location Campaign levels folder path (e.g. "campgns/ami2019")
 * @return true if loaded successfully
 */
TbBool load_campaign_sounds_config(const char* levels_location);

/**
 * @brief Load a mod's sounds configuration
 *
 * Loads mods/<mod_name>/sounds.cfg to allow mods to override or add
 * sound mappings after the campaign sounds are loaded.
 *
 * @param mod_name The mod folder name (e.g. "mymod")
 * @return true if loaded successfully
 */
TbBool load_mod_sounds_config(const char* mod_name);

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

/**
 * @brief Cache commonly-used sound IDs from the registry into globals.
 * Called automatically as the post_load_func for sounds.cfg.
 */
TbBool cache_common_sound_ids(void);

/******************************************************************************/

/* Cached IDs for commonly-used sounds — populated after load_sounds_config().
 * Use these instead of hardcoded numeric IDs in gameplay code. */

/* UI */
extern SoundSmplTblID snd_refusal;
extern SoundSmplTblID snd_tab_click;
extern SoundSmplTblID snd_zoom;

/* Room */
extern SoundSmplTblID snd_room_claim;

/* Gold / salary */
extern SoundSmplTblID snd_gold_pickup;
extern int            snd_gold_pickup_count;
extern SoundSmplTblID snd_salary_full;
extern SoundSmplTblID snd_salary_partial;
extern SoundSmplTblID snd_salary_none;

/* Heart */
extern SoundSmplTblID snd_heart_beat_down;
extern SoundSmplTblID snd_heart_beat_up;

/* Doors */
extern SoundSmplTblID snd_door_open;
extern SoundSmplTblID snd_door_close;
extern SoundSmplTblID snd_door_place;
extern int            snd_door_place_count;

/* Digging */
extern SoundSmplTblID snd_dig_impact;
extern int            snd_dig_impact_count;
extern SoundSmplTblID snd_dig_dirt;

/* Footstep variants */
extern SoundSmplTblID snd_foot_spur;
extern int            snd_foot_spur_count;
extern SoundSmplTblID snd_foot_wet;
extern int            snd_foot_wet_count;
extern SoundSmplTblID snd_foot_snow;
extern int            snd_foot_snow_count;

/* Creature ambient */
extern SoundSmplTblID snd_insect_fly;       /* diptera/insect flying buzz */
extern SoundSmplTblID snd_chicken_cluck;
extern int            snd_chicken_cluck_count;

/* Combat / impacts */
extern SoundSmplTblID snd_splash;
extern SoundSmplTblID snd_explode;
extern SoundSmplTblID snd_strike_wall;
extern int            snd_strike_wall_count;
extern SoundSmplTblID snd_reinforce_hit;    /* imp wall reinforcement impact */
extern int            snd_reinforce_hit_count;

/* Spells */
extern SoundSmplTblID snd_spell_wall;
extern SoundSmplTblID snd_spell_frozen;
extern SoundSmplTblID snd_spell_stars;      /* sparkle/star effect */
extern SoundSmplTblID snd_spell_armageddon;

/* Digging spells */
extern SoundSmplTblID snd_dig_spell;
extern int            snd_dig_spell_count;
extern SoundSmplTblID snd_tunnel_dig;
extern int            snd_tunnel_dig_count;

/* UI */
extern SoundSmplTblID snd_button_click;
extern SoundSmplTblID snd_button_click2;
extern SoundSmplTblID snd_buzzer;           /* error buzz */
extern SoundSmplTblID snd_tab_fall;         /* event notification tab fall */

/* Dungeon heart */
extern SoundSmplTblID snd_heart_engine;     /* heartbeat engine hum (looping) */

/* Room sounds */
extern SoundSmplTblID snd_scavenge;         /* scavenging room sound */

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // DK_CFGSOUNDS_H
