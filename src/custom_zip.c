/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file custom_zip.c
 *     Shared helpers for reading named entries out of a level's mapNNNNN.zip
 *     bundle (custom sprites/icons/lenses, and custom sounds/speech).
 * @par Purpose:
 *     Factored out of custom_sprites.c so non-sprite consumers (sound_manager,
 *     gui_soundmsgs) don't need to depend on the sprite/icon/lens loading code
 *     just to read a single zip entry.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "custom_zip.h"
#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "config.h"
#include <json.h>
#include <json-dom.h>
#include "post_inc.h"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

/*
 * Speedup zip stuff
 * We postulate only one zip file loaded at once
 */
static VALUE zip_cache_v;
static VALUE *zip_cache = &zip_cache_v;

int fastUnzLocateFile(unzFile zip, const char *szFileName, int iCaseSensitivity)
{
    //return unzLocateFile(file, szFileName, iCaseSensitivity);
    char seek_for[PATH_MAX];
    strncpy(seek_for, szFileName, PATH_MAX - 1);
    seek_for[PATH_MAX - 1] = '\0';
    make_lowercase(seek_for);
    VALUE *rec = value_dict_get(zip_cache, seek_for);
    if (rec == NULL)
        return UNZ_END_OF_LIST_OF_FILE;
    unz64_file_pos file_pos = {
            .pos_in_zip_directory = value_int64(value_array_get(rec, 0)),
            .num_of_file = value_int64(value_array_get(rec, 1))
    };
    return unzGoToFilePos64(zip, &file_pos);
}

/*
 * Construct a cache for files.
 * Also if there is no indexFile just return instead
 * */
int fastUnzConstructCache(unzFile zip)
{
    char szCurrentFileName[PATH_MAX];
    if (value_type(zip_cache) != VALUE_NULL)
    {
        ERRORLOG("Zip cache is not clear!");
    }
    value_init_dict(zip_cache);

    for (int err = unzGoToFirstFile(zip);
         err == UNZ_OK;
         err = unzGoToNextFile(zip))
    {
        if (UNZ_OK != unzGetCurrentFileInfo64(zip, NULL,
                                              szCurrentFileName, sizeof(szCurrentFileName) - 1,
                                              NULL, 0, NULL, 0)
                )
        {
            continue;
        }
        make_lowercase(szCurrentFileName);

        unz64_file_pos file_pos;
        unzGetFilePos64(zip, &file_pos);

        VALUE *rec = value_dict_add(zip_cache, szCurrentFileName);
        value_init_array(rec);
        value_init_int64(value_array_append(rec), file_pos.pos_in_zip_directory);
        value_init_int64(value_array_append(rec), file_pos.num_of_file);
    }
    return UNZ_OK;
}

int fastUnzClearCache()
{
    value_fini(zip_cache);
    return 0;
}

/* end of zip stuff */

TbBool read_map_zip_entry(LevelNumber lvnum, const char *entry_name, unsigned char **out_data, size_t *out_size)
{
    if ((out_data == NULL) || (out_size == NULL) || (entry_name == NULL))
    {
        return false;
    }
    *out_data = NULL;
    *out_size = 0;

    char *fname = prepare_file_fmtpath(get_level_fgroup(lvnum), "map%05lu.zip", lvnum);
    if (!LbFileExists(fname))
    {
        return false;
    }

    unzFile zip = unzOpen(fname);
    if (zip == NULL)
    {
        return false;
    }

    TbBool ok = false;
    if (UNZ_OK == fastUnzConstructCache(zip))
    {
        if (UNZ_OK == fastUnzLocateFile(zip, entry_name, 0))
        {
            unz_file_info64 zip_info = {0};
            if (UNZ_OK == unzGetCurrentFileInfo64(zip, &zip_info, NULL, 0, NULL, 0, NULL, 0))
            {
                if (zip_info.uncompressed_size > 32 * 1024 * 1024)
                {
                    WARNLOG("Zip entry too large: '%s' in '%s'", entry_name, fname);
                }
                else
                {
                    unsigned char *data = malloc((size_t)zip_info.uncompressed_size);
                    if (data != NULL)
                    {
                        if (UNZ_OK == unzOpenCurrentFile(zip))
                        {
                            if (unzReadCurrentFile(zip, data, zip_info.uncompressed_size) == (int)zip_info.uncompressed_size)
                            {
                                *out_data = data;
                                *out_size = (size_t)zip_info.uncompressed_size;
                                ok = true;
                            }
                            else
                            {
                                WARNLOG("Failed to read '%s' from '%s'", entry_name, fname);
                                free(data);
                            }
                            unzCloseCurrentFile(zip);
                        }
                        else
                        {
                            free(data);
                        }
                    }
                }
            }
        }
        fastUnzClearCache();
    }
    unzClose(zip);
    return ok;
}
