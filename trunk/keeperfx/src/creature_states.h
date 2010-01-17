/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file creature_states.h
 *     Header file for creature_states.c.
 * @par Purpose:
 *     Creature states structure and function definitions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     23 Sep 2009 - 11 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CRTRSTATE_H
#define DK_CRTRSTATE_H

#include "bflib_basics.h"
#include "globals.h"

#define CREATURE_STATES_COUNT 145

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
#pragma pack(1)
#endif

struct Thing;

struct StateInfo { // sizeof = 41
  short (*ofsfield_0)(struct Thing *);
  short (*ofsfield_4)(struct Thing *);
  char (*ofsfield_8)(struct Thing *);
  long (*ofsfield_C)(struct Thing *);
  unsigned char field_10;
  unsigned char field_11;
  unsigned char field_12;
  unsigned char field_13;
  unsigned char field_14;
  unsigned char field_15;
  unsigned char field_16;
  unsigned char field_17;
  unsigned char field_18;
  unsigned char field_19;
  unsigned char field_1A;
  unsigned char field_1B;
  unsigned char field_1C;
  unsigned char field_1D;
  unsigned char field_1E;
  unsigned char field_1F;
  unsigned char field_20;
  unsigned short field_21;
  unsigned char field_23;
  unsigned short field_24;
  unsigned char field_26;
  unsigned char field_27;
  unsigned char field_28;
};

#ifdef __cplusplus
#pragma pack()
#endif
/******************************************************************************/
DLLIMPORT struct StateInfo _DK_states[];
//#define states _DK_states
extern struct StateInfo states[];
DLLIMPORT long _DK_r_stackpos;
#define r_stackpos _DK_r_stackpos
DLLIMPORT struct ImpStack _DK_reinforce_stack[];
#define reinforce_stack _DK_reinforce_stack
/******************************************************************************/
long check_out_imp_last_did(struct Thing *thing);
/******************************************************************************/
TbBool creature_model_bleeds(unsigned long model);
TbBool internal_set_thing_state(struct Thing *thing, long nState);
struct StateInfo *get_thing_state_info(struct Thing *thing);
struct StateInfo *get_thing_state_info_num(long state_id);
TbBool state_info_invalid(struct StateInfo *stati);
void create_effect_around_thing(struct Thing *thing, long eff_kind);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
