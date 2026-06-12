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
#include "config.h"
#include "gui_soundmsgs.h"

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
 * @brief Reset the entire sound system back to the fxdata baseline.
 *
 * Clears all custom audio buffers, id redirects, and the name registry,
 * then reloads fxdata/sounds.cfg. Call when returning to the main menu so
 * that GUI sounds are not affected by a previous campaign or level.
 */
void sound_reset_to_fxdata_baseline(void);

/**
 * @brief Save a snapshot of all sound state after campaign + mod sounds have loaded.
 *
 * Call once at the end of campaign loading. The snapshot is restored at the
 * start of each level load so that per-level sounds do not bleed across levels.
 */
void sound_save_campaign_snapshot(void);

/**
 * @brief Restore sound state to the campaign snapshot.
 *
 * Call at the start of each level load (before load_level_sounds_config).
 * Discards any sounds registered or loaded for the previous level.
 */
void sound_restore_to_campaign_snapshot(void);

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

/* Room */
extern SoundSmplTblID snd_room_claim;

/* Gold / salary */
extern SoundSmplTblID snd_gold_pickup;
extern int            snd_gold_pickup_count;
extern SoundSmplTblID snd_salary_full;
extern SoundSmplTblID snd_salary_partial;
extern SoundSmplTblID snd_salary_tiny;

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
extern SoundSmplTblID snd_buzzer;           /* error buzz */
extern SoundSmplTblID snd_tab_fall;         /* event notification tab fall */

/* Dungeon heart */
extern SoundSmplTblID snd_heart_engine;     /* heartbeat engine hum (looping) */

/* Room sounds */
extern SoundSmplTblID snd_scavenge;         /* scavenging room sound */

/* Gameplay actions */
extern SoundSmplTblID snd_tile_place;       /* single room tile placed */
extern SoundSmplTblID snd_tile_sell;        /* room/trap/door sold */
extern SoundSmplTblID snd_tile_dig;         /* slab tagged for digging */
extern SoundSmplTblID snd_coin_drop;        /* computer player drops gold coins */
extern SoundSmplTblID snd_alarm;            /* alarm trap triggered */
extern SoundSmplTblID snd_trap_place;       /* PlaceSound default for all traps and doors */
extern SoundSmplTblID snd_trap_trigger;     /* TriggerSound for BOULDER/ALARM/GAS/LIGHTNING/WOP/LAVA traps */
extern SoundSmplTblID snd_trap_trigger_tnt; /* TriggerSound for TNT trap */
extern SoundSmplTblID snd_cheat_activated;  /* easter-egg cheat code entered */
extern SoundSmplTblID snd_tab_hit;          /* event notification tab settles */
extern SoundSmplTblID snd_larg_tile_up;     /* large room placed — rising note */
extern SoundSmplTblID snd_larg_tile_down;   /* large room placed — falling note */

/* Object sounds */
extern SoundSmplTblID snd_torch_ambience;       /* AmbienceSound for TORCH, TEMPLE_STATUE, TORCHUN, CANDLESTCK */
extern SoundSmplTblID snd_dungeon_heart_beam;   /* EffectSound for SOUL_CONTAINER destruction beam */
extern SoundSmplTblID snd_hero_gate_ambience;   /* AmbienceSound for HERO_GATE */

/* Keeper power sounds — voice (SoundSamples) paired with cast effect (SoundPlayed) */
extern SoundSmplTblID snd_power_imp;              /* voice; POWER_IMP (no cast sound) */
extern SoundSmplTblID snd_power_possess;          /* voice; POWER_POSSESS */
extern SoundSmplTblID snd_cast_possess;           /* cast;  POWER_POSSESS */
extern SoundSmplTblID snd_power_sight;            /* voice; POWER_SIGHT */
extern SoundSmplTblID snd_cast_sight;             /* cast;  POWER_SIGHT */
extern SoundSmplTblID snd_power_obey;             /* voice; POWER_OBEY */
extern SoundSmplTblID snd_power_hold_audience;    /* voice; POWER_HOLD_AUDIENCE */
extern SoundSmplTblID snd_cast_obey;              /* cast;  POWER_OBEY and POWER_HOLD_AUDIENCE (shared) */
extern SoundSmplTblID snd_power_call_to_arms;     /* voice; POWER_CALL_TO_ARMS */
extern SoundSmplTblID snd_cast_call_to_arms;      /* cast;  POWER_CALL_TO_ARMS (battle fanfare) */
extern SoundSmplTblID snd_power_cave_in;          /* voice; POWER_CAVE_IN */
extern SoundSmplTblID snd_cast_cave_in;           /* cast;  POWER_CAVE_IN */
extern SoundSmplTblID snd_power_heal;             /* voice; POWER_HEAL_CREATURE */
extern SoundSmplTblID snd_cast_heal;              /* cast;  POWER_HEAL_CREATURE; also FiringSound for SHOT_RANGED_HEAL */
extern SoundSmplTblID snd_power_lightning;        /* voice; POWER_LIGHTNING */
extern SoundSmplTblID snd_cast_lightning;         /* cast;  POWER_LIGHTNING; also FiringSound for SHOT_LIGHTNING family */
extern SoundSmplTblID snd_power_speed;            /* voice; POWER_SPEED */
extern SoundSmplTblID snd_cast_speed;             /* cast;  POWER_SPEED; also FiringSound for SHOT_RANGED_SPEED */
extern SoundSmplTblID snd_power_protect;          /* voice; POWER_PROTECT */
extern SoundSmplTblID snd_cast_armour;            /* cast;  POWER_PROTECT; also FiringSound for SHOT_RANGED_ARMOUR */
extern SoundSmplTblID snd_power_conceal;          /* voice; POWER_CONCEAL */
extern SoundSmplTblID snd_cast_conceal;           /* cast;  POWER_CONCEAL */
extern SoundSmplTblID snd_power_disease;          /* voice; POWER_DISEASE */
extern SoundSmplTblID snd_cast_disease;           /* cast;  POWER_DISEASE (disease spit on target) */
extern SoundSmplTblID snd_power_chicken;          /* voice; POWER_CHICKEN */
extern SoundSmplTblID snd_cast_pickup;            /* cast;  POWER_CHICKEN, POWER_PICKUP_FOOD, POWER_PICKUP_OBJECT (shared) */
extern SoundSmplTblID snd_power_destroy_walls;    /* voice; POWER_DESTROY_WALLS (cast = snd_dig_dirt) */
extern SoundSmplTblID snd_power_armageddon;       /* voice; POWER_ARMAGEDDON    (cast = snd_spell_armageddon) */
/* Powers with no voice — cast effect only */
extern SoundSmplTblID snd_cast_slap;              /* cast;  POWER_SLAP */
extern SoundSmplTblID snd_cast_rebound;           /* cast;  POWER_REBOUND; also FiringSound for SHOT_RANGED_REBOUND */
extern SoundSmplTblID snd_cast_flight;            /* cast;  POWER_FLIGHT */
extern SoundSmplTblID snd_cast_vision;            /* cast;  POWER_VISION */

/* Shot projectile travel sounds (ShotSound in magic.cfg) */
extern SoundSmplTblID snd_shot_freeze;            /* travel; ShotSound for SHOT_FREEZE / SHOT_SLOW */
extern SoundSmplTblID snd_shot_homing_missile;    /* travel; ShotSound for SHOT_NAVI_MISSILE / SHOT_MISSILE */
extern SoundSmplTblID snd_shot_bouncing_grenade;  /* travel; ShotSound for SHOT_GRENADE / SHOT_LIZARD */
/* Shot impact sounds */
extern SoundSmplTblID snd_shot_freeze_impact;     /* impact; creature frozen effect (same ID as snd_spell_frozen) */
extern SoundSmplTblID snd_shot_splat;             /* impact; generic splat on hit */
/* Shot firing sounds (FiringSound in magic.cfg — played on the creature at moment of release) */
extern SoundSmplTblID snd_shot_fire;              /* firing; FiringSound for SHOT_FIREBALL/FIREBOMB/POISON_CLOUD/DRAIN/GROUP/CHICKEN/TIME_BOMB/HAILSTORM */
extern SoundSmplTblID snd_shot_bow;               /* firing; FiringSound for SHOT_ARROW and SHOT_BALLISTA */
extern SoundSmplTblID snd_shot_wind;              /* firing; FiringSound for SHOT_WIND */
extern SoundSmplTblID snd_shot_breath;            /* firing; FiringSound for SHOT_FLAME_BREATH */
extern SoundSmplTblID snd_shot_freeze_fire;       /* firing; FiringSound for SHOT_FREEZE and SHOT_SLOW */
/* Melee swing / misc shot sounds */
extern SoundSmplTblID snd_melee_swing;            /* firing; FiringSound for melee shots (SWING_CLAW, SWING_FIST, DIG, CRIPPLE) */
extern int            snd_melee_swing_count;      /* variants; 6 consecutive IDs (26-31) */
extern SoundSmplTblID snd_boulder_roll;           /* travel; ShotSound for SHOT_BOULDER */
extern SoundSmplTblID snd_shot_magic_travel;      /* travel; ShotSound for many generic magic projectiles */
extern SoundSmplTblID snd_trap_tnt_fire;          /* firing; FiringSound for SHOT_TRAP_TNT */
extern SoundSmplTblID snd_fear_shriek;            /* firing; FiringSound for SHOT_FEAR */
extern SoundSmplTblID snd_cast_cleanse;           /* firing; FiringSound for SHOT_RANGED_CLEANSE */
/* Hit impact sounds (HitXxxSound fields in magic.cfg) */
/* Note: HitWallSound=128 3 reuses snd_strike_wall; HitWater=36 reuses snd_splash; HitWater=21 reuses snd_foot_wet */
extern SoundSmplTblID snd_hit_creature_sword;     /* HitCreatureSound for sword/claw shots (light slice) */
extern SoundSmplTblID snd_hit_creature;           /* HitCreatureSound for fist/ranged shots (heavy hit) */
extern SoundSmplTblID snd_hit_wall;               /* HitWallSound for fist/ballista/sentry */
extern int            snd_hit_wall_count;         /* variants; IDs 138-140 */
extern SoundSmplTblID snd_hit_door_sword;         /* HitDoorSound for sword/claw shots */
extern int            snd_hit_door_sword_count;   /* variants; IDs 131-133 */
extern SoundSmplTblID snd_hit_door;               /* HitDoorSound for fist/heavy shots */
extern int            snd_hit_door_count;         /* variants; IDs 141-143 */
extern SoundSmplTblID snd_hit_heart;              /* HitHeartSound for sword/ballista/ranged shots */
extern SoundSmplTblID snd_hit_heart_fist;         /* HitHeartSound for fist/heavy shots */
extern int            snd_hit_heart_fist_count;   /* variants; IDs 144-146 */
extern SoundSmplTblID snd_hit_wall_boulder;       /* HitWallSound for SHOT_BOULDER */

/******************************************************************************/

/**
 * @brief Per-message speech override paths, indexed by SMsg_* enum value.
 * A non-empty string means output_message() will play this file instead of
 * the default speech-bank sample.  Populated by the [speech] section of
 * sounds.cfg.
 */
extern char g_speech_overrides[SMsg_MAX][512];

/**
 * @brief Resolve a sound name or file path to a sample ID.
 *
 * Used by toml-based config parsers (e.g. effects.toml) via CONDITIONAL_ASSIGN_SOUND.
 * Resolution order:
 *  1. Registered name — looked up via sound_manager_get_id().
 *  2. File path ending in .wav or .mp3 — loaded and registered on the fly.
 *
 * @param text Sound name or file path string
 * @return Sample ID, or 0 if unresolvable
 */
int sound_id_from_text(const char* text);

/**
 * @brief NamedField parse function that resolves a sound name, numeric ID, or inline file path.
 *
 * Resolution order:
 *  1. Numeric value — behaves like value_default.
 *  2. Registered name — looked up via sound_manager_get_id().
 *  3. File path ending in .wav or .mp3 — loaded and registered on the fly
 *     via sound_manager_load_named_sound(), using the path as the cache key.
 *
 * Returns the sample ID, or 0 on failure.
 */
int64_t value_sound_id(const struct NamedField* named_field, const char* value_text,
                       const struct NamedFieldSet* named_fields_set, int idx,
                       const char* src_str, unsigned char flags);

/**
 * @brief Parses a config text value into a SpeechRef.
 *
 * Accepts a numeric SMsg_* ID, a named speech message (e.g. "LevelWon"), or a file path (.wav/.mp3).
 * Used by manual config parsers (e.g. config_magic.c).
 */
void speech_ref_parse(SpeechRef* ref, const char* text);

/**
 * @brief NamedField parse function for SpeechRef fields.
 *
 * Accepts numeric IDs, named speech messages, or audio file paths.
 * When a file path is detected, stores it in an internal buffer and returns a sentinel value.
 * Must be paired with assign_speech_ref.
 */
int64_t value_speech_ref(const struct NamedField* named_field, const char* value_text,
                         const struct NamedFieldSet* named_fields_set, int idx,
                         const char* src_str, unsigned char flags);

/**
 * @brief NamedField assign function for SpeechRef fields.
 *
 * Expects named_field->field to point to a SpeechRef struct within the named field set.
 * Must be paired with value_speech_ref.
 */
void assign_speech_ref(const struct NamedField* named_field, int64_t value,
                       const struct NamedFieldSet* named_fields_set, int idx,
                       const char* src_str, unsigned char flags);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif // DK_CFGSOUNDS_H
