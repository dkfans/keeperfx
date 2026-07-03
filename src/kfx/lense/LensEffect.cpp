/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file LensEffect.cpp
 *     Base class for lens effects implementation.
 * @par Purpose:
 *     Common functionality for all lens effects.
 * @par Comment:
 *     None.
 * @author   Peter Lockett, KeeperFX Team
 * @date     09 Feb 2026
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "../../pre_inc.h"
#include "LensEffect.h"

#include "../../config_mods.h"
#include "../../bflib_fileio.h"
#include "../../bflib_dernc.h"
#include "../../globals.h"

#include "../../keeperfx.hpp"
#include "../../post_inc.h"

/******************************************************************************/

LensEffect::LensEffect(LensEffectType type, const char* name)
    : m_type(type)
    , m_name(name)
    , m_enabled(true)  // Enabled by default, can be toggled via config
    , m_user_data(NULL)
{
}

LensEffect::~LensEffect()
{
}

TbBool LensEffect::LoadAssetWithFallback(const char* filename, unsigned char* buffer, 
                                         size_t buffer_size, const char** loaded_from)
{
    if (filename == NULL || filename[0] == '\0') {
        WARNLOG("Empty filename for effect '%s'", m_name);
        return false;
    }
    
    if (buffer == NULL || buffer_size == 0) {
        ERRORLOG("Invalid buffer for effect '%s'", m_name);
        return false;
    }
    
    // Try loading from all loaded mods' data directories first
    for (int i = 0; i < mods_conf.after_base_cnt; i++)
    {
        const struct ModConfigItem* mod_item = &mods_conf.after_base_item[i];
        // Only check mods that have a directory (mod_dir flag)
        if (mod_item->state.mod_dir)
        {
            char mod_dir[256];
            snprintf(mod_dir, sizeof(mod_dir), "%s/%s", MODS_DIR_NAME, mod_item->name);
            char* fname_mod = prepare_file_path_mod(mod_dir, FGrp_StdData, filename);
            
            // Check if file exists first
            if (LbFileExists(fname_mod))
            {
                long file_size = LbFileLengthRnc(fname_mod);
                
                // Only load if file size matches expected size
                if (file_size == (long)buffer_size)
                {
                    long loaded = LbFileLoadAt(fname_mod, buffer);
                    if (loaded == (long)buffer_size)
                    {
                        if (loaded_from != NULL) {
                            *loaded_from = mod_item->name;
                        }
                        SYNCDBG(7, "Effect '%s' loaded '%s' from mod '%s'", m_name, filename, mod_item->name);
                        return true;
                    }
                }
            }
        }
    }
    
    // If not found in mods, try base game directory
    char* fname_base_path = prepare_file_path(FGrp_StdData, filename);
    if (LbFileExists(fname_base_path))
    {
        long file_size = LbFileLengthRnc(fname_base_path);
        
        if (file_size == (long)buffer_size)
        {
            long loaded = LbFileLoadAt(fname_base_path, buffer);
            if (loaded == (long)buffer_size)
            {
                if (loaded_from != NULL) {
                    *loaded_from = NULL;
                }
                SYNCDBG(7, "Effect '%s' loaded '%s' from base game", m_name, filename);
                return true;
            }
        }
    }
    
    WARNLOG("Effect '%s' failed to load asset '%s'", m_name, filename);
    return false;
}

/******************************************************************************/
