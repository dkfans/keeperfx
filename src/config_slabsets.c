/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_slabsets.c
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
#include "config_slabsets.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_memory.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "value_util.h"

#include <toml.h>
#include "config_strings.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
const char keeper_slabset_file[]="slabset.cfg";
const char keeper_columns_file[]="columns.cfg";
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
    VALUE file_root;
    
    if (!load_toml_file(textname, fname,&file_root))
        return false;
    
    char key[64];
    VALUE *slb_section;
    // Create sections
    for (int slab_kind = 0; slab_kind < game.slab_conf.slab_types_count; slab_kind++)
    {
       
        {
            sprintf(key, "slab%d", slab_kind);
            slb_section = value_dict_get(&file_root, key);
        }
        if (value_type(slb_section) != VALUE_DICT)
        {
            if ((flags & CnfLd_IgnoreErrors) == 0)
            {
                WARNMSG("Invalid section %d", slab_kind);
            }
        }
        else
        {
            for (int slabstyle_no = 0; slabstyle_no < SLABSETS_PER_SLAB; slabstyle_no++)
            {
                VALUE * section = value_dict_get(slb_section, slab_styles_commands[slabstyle_no].name);

                int slabset_no = slab_kind * SLABSETS_PER_SLAB + slabstyle_no;
                
                for (size_t col_no = 0; col_no < 9; col_no++)
                {
                    VALUE *col_arr = value_dict_get(section, "columns");
                    ColumnIndex col_idx = value_int32(value_array_get(col_arr, col_no));
                    game.slabset[slabset_no].col_idx[col_no] = -col_idx;
                }

                sprintf(key, "%s_objects", slab_styles_commands[slabstyle_no].name);
                VALUE * objects_arr = value_dict_get(slb_section, key);
                for (size_t i = 0; i < value_array_size(objects_arr); i++)
                {
                    if(game.slabobjs_num >= SLABOBJS_COUNT)
                    {
                        ERRORLOG("exceeding max of %d slabobjects",SLABOBJS_COUNT);
                        break;
                    }
                    VALUE * object = value_array_get(objects_arr, i);
                    
                    unsigned char class_id = value_parse_class(value_dict_get(object, "ThingType"));
                    game.slabobjs[game.slabobjs_num].class_id = class_id;
                    game.slabobjs[game.slabobjs_num].isLight  = value_int32(value_dict_get(object, "IsLight"));
                    game.slabobjs[game.slabobjs_num].model    = value_parse_model(class_id,value_dict_get(object, "Subtype"));

                    VALUE *RelativePosition_arr = value_dict_get(section, "RelativePosition");
                    game.slabobjs[game.slabobjs_num].offset_x = value_int32(value_array_get(RelativePosition_arr, 0));
                    game.slabobjs[game.slabobjs_num].offset_y = value_int32(value_array_get(RelativePosition_arr, 1));
                    game.slabobjs[game.slabobjs_num].offset_z = value_int32(value_array_get(RelativePosition_arr, 2));
                    game.slabobjs[game.slabobjs_num].range    = value_int32(value_dict_get(object, "EffectRange"));
                    game.slabobjs[game.slabobjs_num].stl_id   = value_int32(value_dict_get(object, "Subtile"));
                    game.slabobjs[game.slabobjs_num].slabset_id = slabset_no;
                    if(i == 0)
                    {
                        game.slabobjs_idx[slabset_no] = game.slabobjs_num;
                    }
                    game.slabobjs_num++;
                }
            }
        }
    }
    value_fini(&file_root);
    
    return true;
}

TbBool load_columns_config_file(const char *textname, const char *fname, unsigned short flags,struct Column *cols,long *ccount)
{
    VALUE file_root;
    
    if (!load_toml_file(textname, fname,&file_root))
        return false;

    VALUE *common_section = value_dict_get(&file_root, "common");
    if (!common_section)
    {
        WARNMSG("No [common] in %s for file %d", textname, fname);
        value_fini(&file_root);
        return false;
    }

    long count = value_int32(value_dict_get(common_section, "ColumnsCount"));
    if (count > *ccount)
    {
        *ccount = count;
    }
    if (*ccount > COLUMNS_COUNT)
    {
        ERRORLOG("more columns then allowed in %s %d/%d",textname,*ccount,COLUMNS_COUNT);
        *ccount = COLUMNS_COUNT;
    }

    char key[64];
    VALUE *section;
    // Create sections
    for (int col_no = 0; col_no < *ccount; col_no++)
    {
       
        {
            sprintf(key, "column%d", col_no);
            section = value_dict_get(&file_root, key);
        }
        if (value_type(section) != VALUE_DICT)
        {
            if ((flags & CnfLd_IgnoreErrors) == 0)
            {
                WARNMSG("Invalid column section %d", col_no);
            }
        }
        else
        {
            unsigned char bitfields = 0;
            TbBool permanent = value_int32(value_dict_get(section, "Permanent"));
            if (permanent > 1)
            {
                ERRORLOG("invalid Utilized (%d) for column %d",permanent,col_no);
                continue;
            }
            bitfields |= permanent;

            char Lintel = value_int32(value_dict_get(section, "Lintel"));
            if (Lintel > 7 || Lintel < 0)
            {
                ERRORLOG("invalid Lintel (%d) for column %d",Lintel,col_no);
                continue;
            }
            Lintel <<= 1;
            bitfields |= Lintel;

            char floorHeight = value_int32(value_dict_get(section, "Height"));
            if (floorHeight > COLUMN_STACK_HEIGHT || floorHeight < 0)
            {
                ERRORLOG("invalid floorHeight (%d) for column %d",floorHeight,col_no);
                continue;
            }
            floorHeight <<= 4;
            bitfields |= floorHeight;

            cols[col_no].use = value_int32(value_dict_get(section, "Utilized"));
            cols[col_no].bitfields = bitfields;
            cols[col_no].solidmask = value_int32(value_dict_get(section, "SolidMask"));
            cols[col_no].floor_texture = value_int32(value_dict_get(section, "FloorTexture"));
            cols[col_no].orient = value_int32(value_dict_get(section, "Orientation"));

            VALUE *Cubes_arr = value_dict_get(section, "Cubes");
            for (size_t cube_no = 0; cube_no < COLUMN_STACK_HEIGHT; cube_no++)
            {
                cols[col_no].cubes[cube_no] = value_int32(value_array_get(Cubes_arr, cube_no));
            }
        }
    }
    value_fini(&file_root);
    
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

TbBool load_columns_config(const char *conf_fname,unsigned short flags,struct Column *cols,long *ccount)
{
    static const char config_global_textname[] = "global columns config";
    static const char config_campgn_textname[] = "campaign columns config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_columns_config_file(config_global_textname, fname, flags,cols,ccount);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_columns_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors,cols,ccount);
    }
    //Freeing and exiting

    return result;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
