/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script.h
 *     Header file for lvl_script.c.
 * @par Purpose:
 *     Level script commands support.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     12 Feb 2009 - 24 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LVLSCRIPT_VALUE_H
#define DK_LVLSCRIPT_VALUE_H


#ifdef __cplusplus
extern "C" {
#endif

void script_process_value(uint32_t var_index, uint32_t plr_range_id, int32_t param1, int32_t param2, int32_t param3, struct ScriptValue *value);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
