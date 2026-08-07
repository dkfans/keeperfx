/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file custom_zip.h
 *     Header file for custom_zip.c.
 * @par Purpose:
 *     Shared helpers for reading named entries out of a level's mapNNNNN.zip
 *     bundle (custom sprites/icons/lenses, and custom sounds/speech).
 *     If more things are going to use the zip loader, best this is in a separate file so the sprite/icon/lens code doesn't have to drag in the sound/speech code.
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

#ifndef GIT_CUSTOM_ZIP_H
#define GIT_CUSTOM_ZIP_H

#include "globals.h"
#include <minizip/unzip.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Locate a file inside an open zip by name, using the fast case-insensitive
 * cache built by fastUnzConstructCache(). Call fastUnzConstructCache() first.
 */
int fastUnzLocateFile(unzFile zip, const char *szFileName, int iCaseSensitivity);

/**
 * @brief Build a case-insensitive name -> position cache for the given open zip,
 * used by fastUnzLocateFile(). Only one zip's cache is held at a time; pair every
 * call with a matching fastUnzClearCache() once done with that zip.
 */
int fastUnzConstructCache(unzFile zip);

/**
 * @brief Clear the cache built by fastUnzConstructCache().
 */
int fastUnzClearCache(void);

/**
 * @brief Read a single named entry out of a map's mapNNNNN.zip bundle into a freshly
 * malloc'd buffer.
 *
 * Only ever reads the exact entry_name requested — never scans/lists the zip's
 * contents for the caller, so nothing is picked up unless a config explicitly names it.
 *
 * @param lvnum       Level number whose mapNNNNN.zip should be searched.
 * @param entry_name  Path of the entry within the zip, e.g. "sound/boom.wav".
 * @param out_data    On success, set to a malloc'd buffer (caller must free()).
 * @param out_size    On success, set to the entry's uncompressed size.
 * @return true if the zip exists and the entry was found and read successfully.
 */
TbBool read_map_zip_entry(LevelNumber lvnum, const char *entry_name, unsigned char **out_data, size_t *out_size);

#ifdef __cplusplus
}
#endif

#endif //GIT_CUSTOM_ZIP_H
