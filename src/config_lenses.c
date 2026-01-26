/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_lenses.c
 *     Eye lense algorithms config functions.
 * @par Purpose:
 *     Support of configuration files for eye lenses.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 07 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_lenses.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include "config.h"
#include "thing_doors.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "keeperfx.hpp"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct LensesConfig lenses_conf;
struct NamedCommand lenses_desc[LENS_ITEMS_MAX];
/******************************************************************************/
static TbBool load_lenses_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_lenses_file_data = {
    .filename = "lenses.cfg",
    .load_func = load_lenses_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

static int64_t value_mist(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);
static int64_t value_pallete(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);
static int64_t value_displace(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);
static int64_t value_overlay(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags);

const struct NamedField lenses_data_named_fields[] = {
    //name           //pos    //field                                           //default //min     //max    //NamedCommand
    {"NAME",              0, field(lenses_conf.lenses[0].code_name),                0,        0,        0, lenses_desc,  value_name,      assign_null},
    {"MIST",              0, field(lenses_conf.lenses[0].mist_file),                0,        0,        0, NULL,         value_mist,      assign_null},
    {"MIST",              1, field(lenses_conf.lenses[0].mist_lightness),           0,        0,       63, NULL,         value_default,   assign_default},
    {"MIST",              2, field(lenses_conf.lenses[0].mist_ghost),               0,        0,      255, NULL,         value_default,   assign_default},
    {"DISPLACEMENT",      0, field(lenses_conf.lenses[0].displace_kind),            0,        0,      255, NULL,         value_default,   assign_default},
    {"DISPLACEMENT",      1, field(lenses_conf.lenses[0].displace_magnitude),       0,        0,      511, NULL,         value_default,   assign_default},
    {"DISPLACEMENT",      2, field(lenses_conf.lenses[0].displace_period),          1,        0,      511, NULL,         value_displace,  assign_default},
    {"PALETTE",           0, field(lenses_conf.lenses[0].palette),                  0,        0,        0, NULL,         value_pallete,   assign_null},
    {"OVERLAY",           0, field(lenses_conf.lenses[0].overlay_file),             0,        0,        0, NULL,         value_overlay,   assign_null},
    {"OVERLAY",           1, field(lenses_conf.lenses[0].overlay_alpha),          128,        0,      255, NULL,         value_default,   assign_default},
    {NULL},
};

const struct NamedFieldSet lenses_data_named_fields_set = {
    &lenses_conf.lenses_count,
    "lens",
    lenses_data_named_fields,
    lenses_desc,
    LENS_ITEMS_MAX,
    sizeof(lenses_conf.lenses[0]),
    lenses_conf.lenses,
};

/******************************************************************************/

struct LensConfig *get_lens_config(long lens_idx)
{
    if ((lens_idx < 1) || (lens_idx > lenses_conf.lenses_count))
        return &lenses_conf.lenses[0];
    return &lenses_conf.lenses[lens_idx];
}

static int64_t value_mist(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    lenses_conf.lenses[idx].flags |= LCF_HasMist;
    return value_name(named_field, value_text, named_fields_set, idx, src_str, flags);
}

static int64_t value_displace(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    lenses_conf.lenses[idx].flags |= LCF_HasDisplace;
    return value_default(named_field, value_text, named_fields_set, idx, src_str, flags);
}


static int64_t value_pallete(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    lenses_conf.lenses[idx].flags |= LCF_HasPalette;
    char* fname = prepare_file_path(FGrp_StdData, value_text);
    if (LbFileLoadAt(fname, (char*)(named_field->field) + named_fields_set->struct_size * idx) != PALETTE_SIZE)
    {
        CONFWRNLOG("Couldn't load \"%s\" file for \"%s\" parameter in [%s%d] block of lens.cfg file.",
            value_text, named_field->name, named_fields_set->block_basename, idx);
    }
    return 0;
}

static int64_t value_overlay(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if (value_text == NULL || value_text[0] == '\0') {
        CONFWRNLOG("Empty filename for \"%s\" parameter in [%s%d] block of lens.cfg file.",
            named_field->name, named_fields_set->block_basename, idx);
        return 0;
    }
    
    lenses_conf.lenses[idx].flags |= LCF_HasOverlay;
    struct LensConfig* lenscfg = &lenses_conf.lenses[idx];
    
    // Store the filename
    strncpy(lenscfg->overlay_file, value_text, DISKPATH_SIZE - 1);
    lenscfg->overlay_file[DISKPATH_SIZE - 1] = '\0';
    
    // Try to extract mod directory from src_str (full config file path)
    // src_str format for mods: "<keeper_dir>/mods/<mod_name>/fxdata/lenses.cfg"
    // We want to extract "mods/<mod_name>"
    lenscfg->overlay_mod_dir[0] = '\0';  // Default: no mod directory
    if (src_str != NULL) {
        const char* mods_pos = strstr(src_str, "mods/");
        if (mods_pos != NULL) {
            // Find the end of the mod name (before next '/')
            const char* mod_start = mods_pos;
            const char* next_slash = strchr(mods_pos + 5, '/');  // +5 to skip "mods/"
            if (next_slash != NULL) {
                size_t len = next_slash - mod_start;
                if (len < DISKPATH_SIZE) {
                    strncpy(lenscfg->overlay_mod_dir, mod_start, len);
                    lenscfg->overlay_mod_dir[len] = '\0';
                    SYNCDBG(9, "Detected mod directory for overlay: %s", lenscfg->overlay_mod_dir);
                }
            }
        }
    }
    
    // Initialize overlay data pointer to NULL - will be loaded during setup
    lenscfg->overlay_data = NULL;
    lenscfg->overlay_width = 0;
    lenscfg->overlay_height = 0;
    
    return 0;
}

static TbBool load_lenses_config_file(const char *fname, unsigned short flags)
{
    SYNCLOG("%s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("file \"%s\" doesn't exist or is too small.",fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
        return false;
    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file

    parse_named_field_blocks(buf, len, fname, flags, &lenses_data_named_fields_set);

    //Freeing and exiting
    free(buf);
    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
