/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_hand_fp.h
 *     Header file for power_hand_fp.c.
 * @par Purpose:
 *     Being carried in a co-user's hand while possessing a creature (archon).
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_POWERHAND_FP_H
#define DK_POWERHAND_FP_H

#include "bflib_basics.h"
#include "globals.h"
#include "net_main.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
struct Thing;
struct Packet;

/* constants relating to being picked up in first-person mode. */
#define FP_HAND_SQUISH_PITCH  (-DEGREES_45)
#define FP_HAND_PITCH_MIN     (-DEGREES_45 * 10 / 9) /* -50 */
#define FP_HAND_PITCH_MAX     (-DEGREES_45 * 4 / 9) /* -20 */

#define FP_HAND_HEIGHT (FILLED_COLUMN_HEIGHT * 3 / 2)
#define FP_HAND_MAX_LERP_PERCENT 40
#define FP_HAND_PARCHMENT_LERP_PERCENT 15
#define FP_HAND_LERP_DELAY_TURNS 10
#define FP_HAND_LERP_RAMP_TURNS  30
#define FP_HAND_TALL_RANGE_SLABS 6

#define FP_HAND_SQUISH_TURNS 4
#define FP_HAND_LIFT_TURNS 12
#define FP_HAND_OSC_AMPLITUDE (FILLED_COLUMN_HEIGHT / 5)
#define FP_HAND_OSC_PERIOD    60
#define FP_HAND_DROP_DIP_TURNS     4
#define FP_HAND_DROP_RISE_TURNS    10
#define FP_HAND_DROP_SETTLE_TURNS  (FP_HAND_DROP_DIP_TURNS + FP_HAND_DROP_RISE_TURNS)
#define FP_HAND_DROP_PITCH_TURNS   10
#define FP_HAND_DROP_CONTROL_TURNS 6
#define FP_HAND_DROP_TURNS         max(FP_HAND_DROP_SETTLE_TURNS, FP_HAND_DROP_PITCH_TURNS)
/* View distance cap (subtiles) while the ceiling is hidden */
#define FP_HAND_VIEW_DISTANCE_SUBTILES (24 * 3) /* 24 slabs */

enum FirstPersonHandPhase {
    FPHand_None = 0,
    FPHand_Squish, // pre-empt pickup
    FPHand_Lift, // rise to hand
    FPHand_Held, // held in hand
    FPHand_Drop, // dropping
};

/******************************************************************************/
TbBool thing_possessed_by_local_user(const struct Thing *thing);
TbBool thing_pickable_while_possessed(NetUserId user, const struct Thing *thing);
enum FirstPersonHandPhase get_fp_hand_phase(NetUserId possessor, const struct Thing *thing, int32_t *phase_turn);
int32_t clamp_fp_hand_pitch(int32_t pitch);
MapCoord get_fp_hand_base_height(PlayerNumber plyr_idx, MapCoord x, MapCoord y);
MapCoord get_fp_hand_view_height(NetUserId possessor, const struct Thing *thing, int32_t *lift_ease);
void process_fp_hand_carrier_packet(NetUserId carrier, const struct Packet *pckt);
void process_fp_hand_drop_views(void);
TbBool fp_hand_blocks_creature_commands(const struct Thing *thing);
void begin_fp_hand_pickup(struct Thing *thing);
void begin_fp_hand_drop(struct Thing *thing);
void restore_possessed_thing_from_hand(struct Thing *thing);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
