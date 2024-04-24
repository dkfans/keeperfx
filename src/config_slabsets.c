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
const char keeper_slabset_file[]="slabset.toml";
const char keeper_columns_file[]="columnset.toml";
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

    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        clear_slabsets();
    }
    
    if (!load_toml_file(textname, fname,&file_root,flags))
        return false;
    
    char key[64];
    VALUE *slb_section;
    // Create sections
    for (int slab_kind = 0; slab_kind < game.conf.slab_conf.slab_types_count; slab_kind++)
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

                VALUE *col_arr = value_dict_get(section, "Columns");
                if (value_type(col_arr) == VALUE_ARRAY)
                {
                    for (size_t col_no = 0; col_no < 9; col_no++)
                    {
                        ColumnIndex col_idx = value_int32(value_array_get(col_arr, col_no));
                        game.slabset[slabset_no].col_idx[col_no] = -col_idx;
                    }
                }

                sprintf(key, "%s_objects", slab_styles_commands[slabstyle_no].name);
                VALUE * objects_arr = value_dict_get(slb_section, key);
                for (size_t i = 0; i < value_array_size(objects_arr); i++)
                {
                    if (game.slabobjs_num >= SLABOBJS_COUNT)
                    {
                        ERRORLOG("Exceeding max of %d slabobjects",SLABOBJS_COUNT);
                        break;
                    }
                    struct SlabObj* slabobj = &game.slabobjs[game.slabobjs_num];
                    VALUE * object = value_array_get(objects_arr, i);
                    CONDITIONAL_ASSIGN_CLASS(object,"ThingType",slabobj->class_id);
                    CONDITIONAL_ASSIGN_BOOL(object, "IsLight", slabobj->isLight);
                    CONDITIONAL_ASSIGN_MODEL(object,"Subtype",slabobj->model,slabobj->class_id);
                    CONDITIONAL_ASSIGN_ARR3_INT(object,"RelativePosition",slabobj->offset_x,slabobj->offset_y,slabobj->offset_z)
                    CONDITIONAL_ASSIGN_INT(object, "EffectRange", slabobj->range);
                    CONDITIONAL_ASSIGN_INT(object, "Subtile",     slabobj->stl_id);

                    slabobj->slabset_id = slabset_no;
                    if (i == 0)
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
    
    if (!load_toml_file(textname, fname,&file_root,flags))
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
            TbBool permanent = true;
            bitfields |= permanent;

            
            VALUE *lintel_val = value_dict_get(section, "Lintel");
            if (value_type(lintel_val) == VALUE_INT32)
            {
                char Lintel = value_int32(lintel_val);
                if (Lintel > 7 || Lintel < 0)
                {
                    ERRORLOG("invalid Lintel (%d) for column %d",Lintel,col_no);
                    continue;
                }
                Lintel <<= 1;
                bitfields |= Lintel;
            }
            
            VALUE *height_val = value_dict_get(section, "Height");
            if (value_type(height_val) == VALUE_INT32)
            {
                char floorHeight = value_int32(height_val);
                if (floorHeight > COLUMN_STACK_HEIGHT || floorHeight < 0)
                {
                    ERRORLOG("invalid floorHeight (%d) for column %d",floorHeight,col_no);
                    continue;
                }
                floorHeight <<= 4;
                bitfields |= floorHeight;
            }

            cols[col_no].bitfields = bitfields;
            CONDITIONAL_ASSIGN_INT(section, "SolidMask",    cols[col_no].solidmask    );
            CONDITIONAL_ASSIGN_INT(section, "FloorTexture", cols[col_no].floor_texture);
            CONDITIONAL_ASSIGN_INT(section, "Orientation",  cols[col_no].orient       );

            VALUE *Cubes_arr = value_dict_get(section, "Cubes");
            if(value_type(Cubes_arr) == VALUE_ARRAY)
            {
                for (size_t cube_no = 0; cube_no < COLUMN_STACK_HEIGHT; cube_no++)
                {
                    cols[col_no].cubes[cube_no] = value_int32(value_array_get(Cubes_arr, cube_no));
                }
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
    static const char config_level_textname[] = "level slabset config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_slabset_config_file(config_global_textname, fname, flags);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_slabset_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), conf_fname);
    if (strlen(fname) > 0)
    {
        load_slabset_config_file(config_level_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors);
    }
    //Freeing and exiting
    return result;
}

TbBool load_columns_config(const char *conf_fname,unsigned short flags,struct Column *cols,long *ccount)
{
    static const char config_global_textname[] = "global columns config";
    static const char config_campgn_textname[] = "campaign columns config";
    static const char config_level_textname[] = "level columns config";
    char* fname = prepare_file_path(FGrp_FxData, conf_fname);
    TbBool result = load_columns_config_file(config_global_textname, fname, flags,cols,ccount);
    fname = prepare_file_path(FGrp_CmpgConfig,conf_fname);
    if (strlen(fname) > 0)
    {
        load_columns_config_file(config_campgn_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors,cols,ccount);
    }
    fname = prepare_file_fmtpath(FGrp_CmpgLvls, "map%05lu.%s", get_selected_level_number(), conf_fname);
    if (strlen(fname) > 0)
    {
        load_columns_config_file(config_level_textname,fname,flags|CnfLd_AcceptPartial|CnfLd_IgnoreErrors,cols,ccount);
    }
    //Freeing and exiting

    return result;
}

void clear_slabsets(void)
{
    struct SlabSet *sset;
    struct SlabObj *sobj;
    int i;
    for (i=0; i < SLABSET_COUNT; i++)
    {
        sset = &game.slabset[i];
        memset(sset, 0, sizeof(struct SlabSet));
        game.slabobjs_idx[i] = -1;
    }
    game.slabset_num = SLABSET_COUNT;
    game.slabobjs_num = 0;
    for (i=0; i < SLABOBJS_COUNT; i++)
    {
        sobj = &game.slabobjs[i];
        memset(sobj, 0, sizeof(struct SlabObj));
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
