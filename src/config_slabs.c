/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_slabs.c
 *     Slabs, rooms, traps and doors configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for trap and door elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_slabs.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"

#include <toml.h>
#include "config_strings.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_slabset_file[]="slabset.cfg";
/******************************************************************************/
typedef struct VALUE VALUE;
const struct NamedCommand slab_styles_commands[] = {
    {"S",         0},
    {"W",         1},
    {"N",         2},
    {"E",         3},
    {"SW",        4},
    {"NW",        5},
    {"NE",        6},
    {"SE",        7},
    {"ALL",       8},
    {"S_LAVA",    9},
    {"W_LAVA",   10},
    {"N_LAVA",   11},
    {"E_LAVA",   12},
    {"SW_LAVA",  13},
    {"NW_LAVA",  14},
    {"NE_LAVA",  15},
    {"SE_LAVA",  16},
    {"ALL_LAVA", 17},
    {"S_WATER",  18},
    {"W_WATER",  19},
    {"N_WATER",  20},
    {"E_WATER",  21},
    {"SW_WATER", 22},
    {"NW_WATER", 23},
    {"NE_WATER", 24},
    {"SE_WATER", 25},
    {"ALL_WATER",26},
    {"CENTER",   27}
};

TbBool load_slabset_config_file(const char *textname, const char *fname, unsigned short flags)
{
    SYNCDBG(5,"Starting");
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        WARNMSG("The %s file \"%s\" doesn't exist or is too small.",textname,fname);
        return false;
    }
    char* buf = (char*)LbMemoryAlloc(len + 256);
    if (buf == NULL)
        return false;
    // Loading file data
    long fsize = LbFileLoadAt(fname, buf);

    if (fsize < len)
    {
        WARNMSG("failed to read the %s file \"%s\".",textname,fname);
        LbMemoryFree(buf);
        return false;
    }
    
    if (buf == NULL)
        return false;
    VALUE file_root, *root_ptr = &file_root;
    char err[255];
    char key[64];

    if (toml_parse((char*)buf, err, sizeof(err), root_ptr))
    {
        WARNMSG("Unable to load %s file\n %s", fname, err);
        LbMemoryFree(buf);
        return false;
    }

    VALUE *slb_section;
    // Create sections
    for (int slab_kind = 0; slab_kind < game.slab_conf.slab_types_count; slab_kind++)
    {
       
        {
            sprintf(key, "slab%d", slab_kind);
            slb_section = value_dict_get(root_ptr, key);
        }
        if (value_type(slb_section) != VALUE_DICT)
        {
            WARNMSG("Invalid section %d", slab_kind);
        }
        else
        {
            for (int slabset_no = 0; slabset_no < SLABSETS_PER_SLAB; slabset_no++)
            {
                VALUE * section = value_dict_get(slb_section, slab_styles_commands[slabset_no].name);
                
                for (size_t col_no = 0; col_no < 9; col_no++)
                {
                    VALUE *col_arr = value_dict_get(section, "columns");
                    ColumnIndex col_idx = value_int32(value_array_get(col_arr, col_no));
                    game.slabset[slab_kind * SLABSETS_PER_SLAB + slabset_no].col_idx[col_no] = -col_idx;
                }
            }
        }
    }
    value_fini(root_ptr);
    LbMemoryFree(buf);
    return true;
}




TbBool load_slabset_config(const char *conf_fname,unsigned short flags)
{
    static const char config_global_textname[] = "global slabset config";
    static const char config_campgn_textname[] = "campaign slabset config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_slabset_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_slabset_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
