/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file local_camera_hand_fp.h
 *     Header file for local_camera_hand_fp.c.
 * @par Purpose:
 *     Local first-person camera while carried in a co-user's hand (archon).
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LOCAL_CAMERA_HAND_FP_H
#define DK_LOCAL_CAMERA_HAND_FP_H

#include "globals.h"
#include "bflib_basics.h"
#include "power_hand_fp.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct Camera;
struct Thing;

/******************************************************************************/
enum FirstPersonHandPhase get_local_fp_hand_phase(const struct Thing *ctrltng, int32_t *phase_turn);
TbBool fp_hand_positions_view(enum FirstPersonHandPhase phase, int32_t phase_turn);
enum FirstPersonHandPhase start_local_fp_hand_camera(const struct Camera *cam, const struct Thing *ctrltng, int32_t *phase_turn);
void update_local_fp_hand_camera(struct Camera *cam, const struct Thing *ctrltng, enum FirstPersonHandPhase phase, int32_t phase_turn);
void apply_local_fp_hand_look(const struct Thing *ctrltng);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
