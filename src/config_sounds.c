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
    .post_load_func = NULL,
};

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

TbBool load_campaign_sounds_config(const char* campaign_name)
{
    if (campaign_name == NULL || campaign_name[0] == '\0')
    {
        return false;
    }
    
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "campgns/%s/sounds.cfg", campaign_name);
    
    const char* fullpath = prepare_file_path(FGrp_Main, filepath);
    if (fullpath == NULL)
    {
        return false;
    }
    
    // Load with IgnoreErrors since campaign sounds are optional
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

/******************************************************************************/
#ifdef __cplusplus
}
#endif
