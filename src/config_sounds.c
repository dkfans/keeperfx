/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_sounds.c
 *     Sound name configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for sound name→ID mappings.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     14 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_sounds.h"
#include "config.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "sound_manager.h"
#include "globals.h"
#include "game_legacy.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define MIN_CONFIG_FILE_SIZE 4

static TbBool load_sounds_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_sounds_file_data = {
    .filename = "sounds.cfg",
    .load_func = load_sounds_config_file,
    .pre_load_func = NULL,
    .post_load_func = cache_common_sound_ids,
};

/******************************************************************************/
// Cached common sound IDs
/******************************************************************************/

SoundSmplTblID snd_refusal         = 119;
SoundSmplTblID snd_tab_click       = 62;
SoundSmplTblID snd_zoom            = 177;
SoundSmplTblID snd_room_claim      = 116;
SoundSmplTblID snd_gold_pickup     = 32;
int            snd_gold_pickup_count = 3;
SoundSmplTblID snd_salary_full     = 34;
SoundSmplTblID snd_salary_partial  = 33;
SoundSmplTblID snd_salary_none     = 32;
SoundSmplTblID snd_heart_beat_down = 150;  // rooms/beat1.wav
SoundSmplTblID snd_heart_beat_up   = 151;  // rooms/beat2a.wav
SoundSmplTblID snd_door_open       = 92;   // terrain/doorup2.wav
SoundSmplTblID snd_door_close      = 91;   // traps/alarm.wav
SoundSmplTblID snd_door_place      = 72;   // terrain/rocks1.wav, rocks2.wav, rocks3.wav
int            snd_door_place_count = 3;
SoundSmplTblID snd_dig_impact      = 72;   // terrain/rocks1.wav, rocks2.wav, rocks3.wav
int            snd_dig_impact_count = 3;
SoundSmplTblID snd_dig_dirt        = 73;   // terrain/rocks2.wav

/* Footstep variants */
SoundSmplTblID snd_foot_spur       = 5;    // footsteps/spur1.wav
int            snd_foot_spur_count = 4;
SoundSmplTblID snd_foot_wet        = 21;   // footsteps/footwet1.wav
int            snd_foot_wet_count  = 4;
SoundSmplTblID snd_foot_snow       = 182;  // footsteps/snowft1.wav, snowft2.wav, snowft3.wav
int            snd_foot_snow_count = 3;

/* Creature ambient */
SoundSmplTblID snd_insect_fly      = 26;   // creature_insect/fly.wav
SoundSmplTblID snd_chicken_cluck   = 112;  // chicken/chick4a.wav
int            snd_chicken_cluck_count = 3;

/* Combat / impacts */
SoundSmplTblID snd_splash          = 37;   // splash.wav
SoundSmplTblID snd_explode         = 47;   // creature_spells/firepuff.wav
SoundSmplTblID snd_strike_wall     = 128;  // strikes/swonarm3.wav, swonarm4.wav, swonarm5.wav
int            snd_strike_wall_count = 3;
SoundSmplTblID snd_reinforce_hit   = 1005; // impacts/slap2.wav, tap.wav, toasterstepa1-3.wav, toasterstepb1-2.wav
int            snd_reinforce_hit_count = 7;

/* Spells */
SoundSmplTblID snd_spell_wall      = 41;   // creature_spells/wind3.wav
SoundSmplTblID snd_spell_frozen    = 50;   // creature_spells/freeze.wav
SoundSmplTblID snd_spell_stars     = 76;   // keeper_spells/slap.wav
SoundSmplTblID snd_spell_armageddon = 180; // null.wav

/* Digging */
SoundSmplTblID snd_dig_spell       = 63;   // gui/button3.wav
int            snd_dig_spell_count = 6;
SoundSmplTblID snd_tunnel_dig      = 69;   // creature_spells/dig6.wav
int            snd_tunnel_dig_count = 3;

/* UI */
SoundSmplTblID snd_button_click    = 60;   // spit.wav
SoundSmplTblID snd_button_click2   = 61;   // gui/button1.wav
SoundSmplTblID snd_buzzer          = 89;   // coindrop.wav
SoundSmplTblID snd_tab_fall        = 947;  // null.wav

/* Dungeon heart */
SoundSmplTblID snd_heart_engine    = 93;   // terrain/doordown.wav

/* Scavenging */
SoundSmplTblID snd_scavenge        = 156;  // rooms/prayers.wav

/******************************************************************************/

/**
 * @brief Parse a single line in the format: NAME = ID [count]
 * 
 * Examples:
 *   REFUSAL = 119
 *   GOLD_PICKUP = 32 3    ; IDs 32, 33, 34
 *   CUSTOM_SOUND = sounds/explosion.wav
 */
static TbBool parse_sound_line(const char* buf, int32_t* pos, long len, const char* config_textname)
{
    char name_buf[COMMAND_WORD_LEN];
    char value_buf[COMMAND_WORD_LEN];
    char count_buf[COMMAND_WORD_LEN];
    
    // Get the sound name
    if (get_conf_parameter_single(buf, pos, len, name_buf, sizeof(name_buf)) <= 0)
    {
        return false;  // Empty line or comment
    }
    
    // Skip if it's a section header or comment
    if (name_buf[0] == '[' || name_buf[0] == ';' || name_buf[0] == '#')
    {
        return false;
    }
    
    // Expect '=' separator - check if next non-whitespace is '='
    // The get_conf_parameter_single function should handle this, but let's verify
    // Actually, the format is "NAME = VALUE" so we need to handle the '=' specially
    
    // Get the value (ID or filepath)
    if (get_conf_parameter_single(buf, pos, len, value_buf, sizeof(value_buf)) <= 0)
    {
        // Skip the '=' if present
        while (*pos < len && (buf[*pos] == '=' || buf[*pos] == ' ' || buf[*pos] == '\t'))
        {
            (*pos)++;
        }
        
        if (get_conf_parameter_single(buf, pos, len, value_buf, sizeof(value_buf)) <= 0)
        {
            WARNLOG("Sound '%s' has no value in %s", name_buf, config_textname);
            return false;
        }
    }
    
    // Handle '=' that might be captured as part of value
    if (value_buf[0] == '=')
    {
        // The '=' was captured, get the actual value
        if (get_conf_parameter_single(buf, pos, len, value_buf, sizeof(value_buf)) <= 0)
        {
            WARNLOG("Sound '%s' has no value after '=' in %s", name_buf, config_textname);
            return false;
        }
    }
    
    // Try to get optional count parameter
    int count = 1;
    if (get_conf_parameter_single(buf, pos, len, count_buf, sizeof(count_buf)) > 0)
    {
        int parsed_count = atoi(count_buf);
        if (parsed_count > 0)
        {
            count = parsed_count;
        }
    }
    
    // Determine if value is numeric ID or filepath
    SoundSmplTblID sample_id;
    char* endptr;
    long id_value = strtol(value_buf, &endptr, 10);
    
    if (*endptr == '\0' && id_value > 0)
    {
        // Numeric ID - register directly
        sample_id = (SoundSmplTblID)id_value;
        
        if (!sound_manager_register(name_buf, sample_id, count))
        {
            WARNLOG("Failed to register sound '%s' with ID %d in %s", 
                    name_buf, sample_id, config_textname);
            return false;
        }
        
        SYNCDBG(8, "Registered sound '%s' -> ID %d (count %d)", name_buf, sample_id, count);
    }
    else
    {
        // Filepath - load as custom sound
        // TODO: Implement custom sound loading from path
        // For now, just log it
        SYNCDBG(8, "Sound '%s' -> filepath '%s' (custom loading not yet implemented)", 
                name_buf, value_buf);
    }
    
    return true;
}

/**
 * @brief Parse a [section] block (like [common], [ui], [creatures])
 */
static TbBool parse_sounds_section(char* buf, long len, const char* config_textname, 
                                   unsigned short flags, const char* section_name)
{
    int32_t pos = 0;
    const char* blockname = NULL;
    int blocknamelen = 0;
    TbBool found_section = false;
    
    // Find the requested section
    while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
    {
        if (blocknamelen > 0 && strncasecmp(blockname, section_name, blocknamelen) == 0)
        {
            found_section = true;
            break;
        }
    }
    
    if (!found_section)
    {
        // Section not found is not an error - it's optional
        return true;
    }
    
    // Parse lines until next section
    while (pos < len)
    {
        // Check if we've hit another section
        if (buf[pos] == '[')
        {
            break;
        }
        
        // Skip empty lines and comments
        if (buf[pos] == '\n' || buf[pos] == '\r')
        {
            pos++;
            continue;
        }
        
        if (buf[pos] == ';' || buf[pos] == '#')
        {
            // Skip to end of line
            while (pos < len && buf[pos] != '\n' && buf[pos] != '\r')
            {
                pos++;
            }
            continue;
        }
        
        // Try to parse as sound definition
        parse_sound_line(buf, &pos, len, config_textname);
        
        // Move to next line
        while (pos < len && buf[pos] != '\n' && buf[pos] != '\r')
        {
            pos++;
        }
        while (pos < len && (buf[pos] == '\n' || buf[pos] == '\r'))
        {
            pos++;
        }
    }
    
    return true;
}

/**
 * @brief Main loader for sounds.cfg
 */
static TbBool load_sounds_config_file(const char *fname, unsigned short flags)
{
    SYNCDBG(0, "%s file \"%s\".", ((flags & CnfLd_ListOnly) == 0) ? "Reading" : "Parsing", fname);
    
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("File \"%s\" doesn't exist or is too small.", fname);
        return false;
    }
    
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
    {
        ERRORLOG("Cannot allocate memory for %s", fname);
        return false;
    }
    
    // Load file data
    len = LbFileLoadAt(fname, buf);
    if (len <= 0)
    {
        free(buf);
        ERRORLOG("Cannot load file %s", fname);
        return false;
    }
    
    TbBool result = true;
    
    // Parse known sections
    const char* sections[] = {"common", "ui", "creatures", "powers", "effects", "doors", NULL};
    
    for (int i = 0; sections[i] != NULL; i++)
    {
        if (!parse_sounds_section(buf, len, fname, flags, sections[i]))
        {
            result = false;
        }
    }
    
    free(buf);
    return result;
}

/******************************************************************************/
// Public API
/******************************************************************************/

TbBool load_sounds_config(void)
{
    return load_config(&keeper_sounds_file_data, CnfLd_Standard);
}

TbBool load_campaign_sounds_config(const char* levels_location)
{
    if (levels_location == NULL || levels_location[0] == '\0')
    {
        return false;
    }

    // levels_location is like "campgns/ami2019" — look for sounds.cfg in it
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/sounds.cfg", levels_location);

    const char* fullpath = prepare_file_path(FGrp_Main, filepath);
    if (fullpath == NULL)
    {
        return false;
    }

    // Optional — campaigns don't need to supply sounds
    return load_sounds_config_file(fullpath, CnfLd_Standard | CnfLd_IgnoreErrors);
}

TbBool load_mod_sounds_config(const char* mod_name)
{
    if (mod_name == NULL || mod_name[0] == '\0')
    {
        return false;
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "mods/%s/sounds.cfg", mod_name);

    const char* fullpath = prepare_file_path(FGrp_Main, filepath);
    if (fullpath == NULL)
    {
        return false;
    }

    return load_sounds_config_file(fullpath, CnfLd_Standard | CnfLd_IgnoreErrors);
}

TbBool load_level_sounds_config(const char* level_name)
{
    if (level_name == NULL || level_name[0] == '\0')
    {
        return false;
    }
    
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "levels/%s/sounds.cfg", level_name);
    
    const char* fullpath = prepare_file_path(FGrp_Main, filepath);
    if (fullpath == NULL)
    {
        return false;
    }
    
    // Load with IgnoreErrors since level sounds are optional
    return load_sounds_config_file(fullpath, CnfLd_Standard | CnfLd_IgnoreErrors);
}

SoundSmplTblID get_sound_id(const char* name)
{
    return sound_manager_get_id(name);
}

TbBool is_sound_registered(const char* name)
{
    return sound_manager_is_registered(name);
}

TbBool cache_common_sound_ids(void)
{
    SoundSmplTblID id;
    int count;

    #define CACHE_SND(var, name)           id = sound_manager_get_id(name); if (id > 0) { var = id; }
    #define CACHE_SND_COUNT(var, cvar, nm) id = sound_manager_get_id(nm);   if (id > 0) { var = id; count = sound_manager_get_count(nm); if (count > 0) cvar = count; }

    CACHE_SND(snd_refusal,         "REFUSAL")
    CACHE_SND(snd_tab_click,       "TAB_CLICK")
    CACHE_SND(snd_zoom,            "ZOOM")
    CACHE_SND(snd_room_claim,      "ROOM_CLAIM")
    CACHE_SND_COUNT(snd_gold_pickup, snd_gold_pickup_count, "GOLD_PICKUP")
    CACHE_SND(snd_salary_full,     "SALARY_FULL")
    CACHE_SND(snd_salary_partial,  "SALARY_PARTIAL")
    CACHE_SND(snd_salary_none,     "SALARY_NONE")
    CACHE_SND(snd_heart_beat_down, "HEART_BEAT_DOWN")
    CACHE_SND(snd_heart_beat_up,   "HEART_BEAT_UP")
    CACHE_SND(snd_door_open,       "DOOR_OPEN")
    CACHE_SND(snd_door_close,      "DOOR_CLOSE")
    CACHE_SND_COUNT(snd_door_place,  snd_door_place_count,  "DOOR_PLACE")
    CACHE_SND_COUNT(snd_dig_impact,  snd_dig_impact_count,  "DIG_IMPACT")
    CACHE_SND(snd_dig_dirt,        "DIG_DIRT")

    CACHE_SND_COUNT(snd_foot_spur,   snd_foot_spur_count,   "FOOT_SPUR")
    CACHE_SND_COUNT(snd_foot_wet,    snd_foot_wet_count,    "FOOT_WET")
    CACHE_SND_COUNT(snd_foot_snow,   snd_foot_snow_count,   "FOOT_SNOW")
    CACHE_SND(snd_insect_fly,      "INSECT_FLY")
    CACHE_SND_COUNT(snd_chicken_cluck, snd_chicken_cluck_count, "CHICKEN_CLUCK")
    CACHE_SND(snd_splash,          "SPLASH")
    CACHE_SND(snd_explode,         "EXPLODE")
    CACHE_SND_COUNT(snd_strike_wall, snd_strike_wall_count, "STRIKE_WALL")
    CACHE_SND_COUNT(snd_reinforce_hit, snd_reinforce_hit_count, "REINFORCE_HIT")
    CACHE_SND(snd_spell_wall,      "SPELL_WALL")
    CACHE_SND(snd_spell_frozen,    "SPELL_FROZEN")
    CACHE_SND(snd_spell_stars,     "SPELL_STARS")
    CACHE_SND(snd_spell_armageddon, "SPELL_ARMAGEDDON")
    CACHE_SND_COUNT(snd_dig_spell,   snd_dig_spell_count,   "DIG_SPELL")
    CACHE_SND_COUNT(snd_tunnel_dig,  snd_tunnel_dig_count,  "TUNNEL_DIG")
    CACHE_SND(snd_button_click,    "BUTTON_CLICK")
    CACHE_SND(snd_button_click2,   "BUTTON_CLICK2")
    CACHE_SND(snd_buzzer,          "BUZZER")
    CACHE_SND(snd_tab_fall,        "TAB_FALL")
    CACHE_SND(snd_heart_engine,    "HEART_ENGINE")
    CACHE_SND(snd_scavenge,        "SCAVENGE")

    #undef CACHE_SND
    #undef CACHE_SND_COUNT

    SYNCDBG(8, "Common sound IDs cached");
    return true;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
