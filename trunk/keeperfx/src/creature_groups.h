/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_groups.h
 *     Header file for creature_groups.c.
 * @par Purpose:
 *     Creature grouping and groups support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     11 Mar 2010 - 12 May 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRGROUPS_H
#define DK_CRTRGROUPS_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

enum TriggerFlags {
    TrgF_NONE                          =  0x00,
    TrgF_REUSABLE                      =  0x01,
    TrgF_DISABLED                      =  0x02,
};

/******************************************************************************/
#pragma pack(1)

struct Thing;


#pragma pack()
/******************************************************************************/
/******************************************************************************/
int get_party_index_of_name(const char *prtname);
long get_highest_experience_level_in_group(struct Thing *thing);
long add_creature_to_group(struct Thing *crthing, struct Thing *grthing);
TbBool create_party(char *prtname);
TbBool add_member_to_party_name(const char *prtname, long crtr_model, long crtr_level, long carried_gold, long objctv_id, long countdown);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
