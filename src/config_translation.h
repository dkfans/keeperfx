/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_translation.h
 *     Header file for config_translation.c.
 * @par Purpose:
 *     Translations configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGTRANSLATION_H
#define DK_CFGTRANSLATION_H

#include "config.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
extern TextStringId get_string_id_by_alias(const char* alias);
extern const char* get_translation_file_string(TextStringId string_id);

extern const struct ConfigFileData keeper_translation_file_data;

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
