/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file actionpt.h
 *     Header file for actionpt.c.
 * @par Purpose:
 *     actionpt functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     21 May 2010 - 07 Jun 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_ACTIONPT_H
#define DK_ACTIONPT_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ACTN_POINTS_COUNT     32

/******************************************************************************/
#pragma pack(1)

enum ActionPointFlags {
    AptF_Exists            = 0x01,
};

struct InitActionPoint { // sizeof = 8
    struct Coord2d mappos;
    unsigned short range;
    unsigned short num;
};

struct ActionPoint { // sizeof = 0xA
    unsigned char flags;
    struct Coord2d mappos;
    unsigned short range;
    unsigned short num;
    unsigned char activated;
};

#pragma pack()

#define INVALID_ACTION_POINT (&game.action_points[0])
/******************************************************************************/
struct ActionPoint *allocate_free_action_point_structure_with_number(long apt_num);
struct ActionPoint *actnpoint_create_actnpoint(struct InitActionPoint *iapt);
struct ActionPoint *action_point_get(ActionPointId apt_idx);
struct ActionPoint *action_point_get_by_number(long apt_num);
TbBool action_point_exists(const struct ActionPoint *apt);
TbBool action_point_exists_idx(ActionPointId apt_idx);
TbBool action_point_exists_number(long apt_num);
ActionPointId action_point_number_to_index(long apt_num);
TbBool action_point_is_invalid(const struct ActionPoint *apt);

TbBool action_point_reset_idx(ActionPointId apt_idx);
unsigned long get_action_point_activated_by_players_mask(ActionPointId apt_idx);
TbBool action_point_activated_by_player(ActionPointId apt_idx, PlayerNumber plyr_idx);

void clear_action_points(void);
void delete_all_action_point_structures(void);
TbBool process_action_points(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
