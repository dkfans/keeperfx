/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_instances.h
 *     Header file for creature_instances.c.
 * @par Purpose:
 *     creature_instances functions.
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
#ifndef DK_CRTRINSTANCE_H
#define DK_CRTRINSTANCE_H

#include "globals.h"
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
typedef long (*Creature_Instf_Func)(struct Thing *, long *);

#ifdef __cplusplus
#pragma pack(1)
#endif

struct InstanceInfo { // sizeof = 42
unsigned char field_0;
  long time;
  long fp_time;
  long action_time;
  long fp_action_time;
  long reset_time;
  long fp_reset_time;
  unsigned char graphics_idx;
unsigned char field_1A;
  short force_visibility;
unsigned char field_1D;
    Creature_Instf_Func func_cb;
  long field_22;
unsigned char field_26[4];
};

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
extern const struct NamedCommand creature_instances_func_type[];
extern Creature_Instf_Func creature_instances_func_list[];
/******************************************************************************/
/** Returns creature instance info structure for given instance index. */
#define creature_instance_info_get(inst_idx) creature_instance_info_get_ptr(inst_idx,__func__)
struct InstanceInfo *creature_instance_info_get_ptr(long inst_idx,const char *func_name);
void process_creature_instance(struct Thing *thing);
TbBool creature_instance_info_invalid(const struct InstanceInfo *inst_inf);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
