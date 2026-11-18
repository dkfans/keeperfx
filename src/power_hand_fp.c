/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file power_hand_fp.c
 *     Being carried in a co-user's hand while possessing a creature (archon).
 * @par Purpose:
 *     Synced state for the carried first-person view.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "power_hand_fp.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_math.h"
#include "bflib_planar.h"
#include "power_hand.h"
#include "player_instances.h"
#include "config_creature.h"
#include "creature_states.h"
#include "player_data.h"
#include "creature_control.h"
#include "thing_creature.h"
#include "thing_data.h"
#include "thing_list.h"
#include "light_data.h"
#include "map_columns.h"
#include "map_data.h"
#include "net_game.h"
#include "packets.h"
#include "game_legacy.h"
#include "post_inc.h"

/******************************************************************************/
TbBool thing_possessed_by_local_user(const struct Thing *thing)
{
    const NetUserId possessor = get_user_controlling_thing(thing, UCT_Direct);
    return (possessor >= 0) && is_local_user(possessor);
}

TbBool thing_pickable_while_possessed(NetUserId user, const struct Thing *thing)
{
    if (!thing_is_creature(thing)) {
        return false;
    }
    const NetUserId possessor = get_user_controlling_thing(thing, UCT_Direct);
    if ((possessor < 0) || (possessor == user)) {
        return false;
    }
    if (get_net_user_player_number(possessor) != get_net_user_player_number(user)) {
        return false;
    }
    if (get_user_view_type(possessor) != PVT_CreatureContrl) {
        return false;
    }
    // Spectator creatures are deleted when their possession ends
    if (flag_is_set(creature_control_get_from_thing(thing)->creature_state_flags, TF2_Spectator)) {
        return false;
    }
    // Nobody else may be riding along or zooming in
    for (NetUserId other = 0; other < MAX_NET_USERS; other++) {
        if ((other != possessor) && user_present(other)
         && user_controls_thing(other, thing, UCT_Direct | UCT_Passenger | UCT_Other | UCT_PossessionEntryZoom)) {
            return false;
        }
    }
    return true;
}

// hand holding animation for first-person pickup (archon)
enum FirstPersonHandPhase get_fp_hand_phase(NetUserId possessor, const struct Thing *thing, int32_t *phase_turn)
{
    if (phase_turn != NULL) {
        *phase_turn = 0;
    }
    if ((possessor < 0) || !thing_is_creature(thing) || !user_controls_thing(possessor, thing, UCT_Direct)) {
        return FPHand_None;
    }
    const struct UserState *ustate = get_user_state(possessor);
    if (get_user_holding_thing(thing) < 0) {
        // Recently dropped from a hand?
        if ((ustate->fp_hand_drop_turn == 0) || ((GameTurnDelta)(ustate->fp_hand_drop_turn - ustate->fp_hand_pickup_turn) <= 0)) {
            return FPHand_None;
        }
        GameTurnDelta drop_turn = (GameTurnDelta)(get_gameturn() - ustate->fp_hand_drop_turn);
        if (drop_turn < 0) {
            drop_turn = 0;
        }
        if (drop_turn >= FP_HAND_DROP_TURNS) {
            return FPHand_None;
        }
        if (phase_turn != NULL) {
            *phase_turn = drop_turn;
        }
        return FPHand_Drop;
    }
    GameTurnDelta turn = (GameTurnDelta)(get_gameturn() - ustate->fp_hand_pickup_turn);
    if (turn < 0) {
        turn = 0;
    }
    enum FirstPersonHandPhase phase;
    if (turn < FP_HAND_SQUISH_TURNS) {
        phase = FPHand_Squish;
    } else if (turn < FP_HAND_SQUISH_TURNS + FP_HAND_LIFT_TURNS) {
        phase = FPHand_Lift;
        turn -= FP_HAND_SQUISH_TURNS;
    } else {
        phase = FPHand_Held;
        turn -= FP_HAND_SQUISH_TURNS + FP_HAND_LIFT_TURNS;
    }
    if (phase_turn != NULL) {
        *phase_turn = turn;
    }
    return phase;
}

int32_t clamp_fp_hand_pitch(int32_t pitch)
{
    pitch &= ANGLE_MASK;
    if (pitch > DEGREES_180) {
        pitch -= DEGREES_360;
    }
    return clamp(pitch, FP_HAND_PITCH_MIN, FP_HAND_PITCH_MAX) & ANGLE_MASK;
}

static MapCoord subtile_top_height(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    const struct Column *col = get_column_at(stl_x, stl_y);
    if (column_invalid(col)) {
        return 0;
    }
    int filled = 0;
    for (unsigned int mask = col->solidmask; mask != 0; mask >>= 1) {
        filled++;
    }
    return filled * COORD_PER_STL;
}

/** distance to nearest column which exceeds wall height (max 6 slabs)
 */
static int32_t fp_hand_tall_column_distance(PlayerNumber plyr_idx, MapCoord x, MapCoord y)
{
    const int32_t range = FP_HAND_TALL_RANGE_SLABS * COORD_PER_SLB;
    const MapSubtlDelta range_stl = FP_HAND_TALL_RANGE_SLABS * STL_PER_SLB;
    const MapSubtlCoord cx = coord_subtile(x);
    const MapSubtlCoord cy = coord_subtile(y);
    int32_t best = range;
    for (MapSubtlCoord stl_y = max(cy - range_stl, 0); stl_y <= min(cy + range_stl, game.map_subtiles_y - 1); stl_y++)
    {
        for (MapSubtlCoord stl_x = max(cx - range_stl, 0); stl_x <= min(cx + range_stl, game.map_subtiles_x - 1); stl_x++)
        {
            if (subtile_top_height(stl_x, stl_y) <= FILLED_COLUMN_HEIGHT) {
                continue;
            }
            if (!subtile_revealed(stl_x, stl_y, plyr_idx)) {
                continue;
            }
            const int32_t dx = subtile_coord_center(stl_x) - x;
            const int32_t dy = subtile_coord_center(stl_y) - y;
            best = min(best, (int32_t)LbSqrL(dx * dx + dy * dy));
        }
    }
    return best;
}

/** Height the carried view settles at over (x,y): raised near high columns e.g. dungeon heart  */
MapCoord get_fp_hand_base_height(PlayerNumber plyr_idx, MapCoord x, MapCoord y)
{
    const int32_t range = FP_HAND_TALL_RANGE_SLABS * COORD_PER_SLB;
    const int32_t d = fp_hand_tall_column_distance(plyr_idx, x, y);
    return FP_HAND_HEIGHT + (range - d) * (2 * FILLED_COLUMN_HEIGHT - FP_HAND_HEIGHT) / range;
}

// preferred fp hand height (before bob animation)
MapCoord get_fp_hand_view_height(NetUserId possessor, const struct Thing *thing, int32_t *lift_ease)
{
    int32_t turn;
    const enum FirstPersonHandPhase phase = get_fp_hand_phase(possessor, thing, &turn);
    if (lift_ease != NULL) {
        *lift_ease = (phase == FPHand_Held) ? (1 << LbFPMath_TrigmBits) : 0;
    }
    const MapCoord eye_z = thing->mappos.z.val + get_creature_eye_height(thing);
    if (phase == FPHand_None) {
        return eye_z;
    }
    const int32_t fp_one = (1 << LbFPMath_TrigmBits);
    const MapCoord floor_z = thing->floor_height;
    const MapCoord bottom_z = floor_z + (eye_z - floor_z) / 2;
    if (phase == FPHand_Drop) {
        if (turn < FP_HAND_DROP_DIP_TURNS) {
            const MapCoord drop_z = get_user_state(possessor)->fp_hand_drop_z;
            const int32_t den = FP_HAND_DROP_DIP_TURNS + 1;
            const int32_t rest = den - (turn + 1);
            return drop_z + (bottom_z - drop_z) * (den * den - rest * rest) / (den * den);
        }
        if (turn >= FP_HAND_DROP_SETTLE_TURNS) {
            return eye_z;
        }
        // cos ease up to eye level
        const int32_t ease = (fp_one - LbCosL(DEGREES_180 * (turn - FP_HAND_DROP_DIP_TURNS) / FP_HAND_DROP_RISE_TURNS)) / 2;
        return bottom_z + (MapCoord)(((int64_t)(eye_z - bottom_z) * ease) >> LbFPMath_TrigmBits);
    }
    if (phase == FPHand_Squish) {
        // ease out to half z
        const int32_t den = FP_HAND_SQUISH_TURNS + 1;
        const int32_t rest = den - (turn + 1);
        return eye_z + (bottom_z - eye_z) * (den * den - rest * rest) / (den * den);
    }
    const struct UserState *ustate = get_user_state(possessor);
    // bobbing
    const int32_t bob = (int32_t)(((int64_t)FP_HAND_OSC_AMPLITUDE * LbSinL(DEGREES_360 * (int32_t)(get_gameturn() % FP_HAND_OSC_PERIOD) / FP_HAND_OSC_PERIOD)) >> LbFPMath_TrigmBits);
    const MapCoord base_z = get_fp_hand_base_height(ustate->player_id, ustate->fp_hand_x, ustate->fp_hand_y) + bob;
    if (phase == FPHand_Held) {
        return base_z;
    }
    // cos ease up to hand level
    const int32_t ease = (fp_one - LbCosL(DEGREES_180 * turn / FP_HAND_LIFT_TURNS)) / 2;
    if (lift_ease != NULL) {
        *lift_ease = ease;
    }
    return bottom_z + (MapCoord)(((int64_t)(base_z - bottom_z) * ease) >> LbFPMath_TrigmBits);
}

static TbBool get_fp_hand_carrier_target(NetUserId carrier, const struct Packet *pckt, MapCoord *x, MapCoord *y)
{
    const struct UserState *carrier_state = get_user_state(carrier);
    switch (carrier_state->view_type)
    {
    case PVT_DungeonTop:
    case PVT_MapScreen:
    {
        // Cursor on dungeon/parchment
        const unsigned char context = packet_camera_context(pckt);
        const TbBool map_context = (carrier_state->view_type == PVT_MapScreen) ? (context == CamIV_Parchment)
            : ((context == CamIV_Isometric) || (context == CamIV_FrontView));
        if (!map_context || ((pckt->control_flags & (PCtr_Gui | PCtr_MapCoordsValid)) != PCtr_MapCoordsValid)) {
            return false;
        }
        *x = pckt->pos_x;
        *y = pckt->pos_y;
        return true;
    }
    case PVT_CreatureContrl:
    case PVT_CreaturePasngr:
    {
        // Above possessed creature
        const struct Thing *ctrltng = thing_get(carrier_state->controlled_thing_idx);
        if (!thing_exists(ctrltng)) {
            return false;
        }
        *x = ctrltng->mappos.x.val;
        *y = ctrltng->mappos.y.val;
        return true;
    }
    default:
        return false;
    }
}

void process_fp_hand_carrier_packet(NetUserId carrier, const struct Packet *pckt)
{
    if (flag_is_set(game.operation_flags, GOF_Paused)) {
        return;
    }
    MapCoord target_x;
    MapCoord target_y;
    if (!get_fp_hand_carrier_target(carrier, pckt, &target_x, &target_y)) {
        return;
    }
    const struct UserState *carrier_state = get_user_state(carrier);
    for (int i = 0; i < carrier_state->num_things_in_hand; i++)
    {
        const struct Thing *thing = thing_get(carrier_state->things_in_hand[i]);
        if (!thing_is_creature(thing)) {
            continue;
        }
        const NetUserId possessor = get_user_controlling_thing(thing, UCT_Direct);
        if (get_fp_hand_phase(possessor, thing, NULL) == FPHand_None) {
            continue;
        }
        struct UserState *ustate = get_user_state(possessor);

        const GameTurnDelta since_pickup = (GameTurnDelta)(get_gameturn() + 1 - ustate->fp_hand_pickup_turn);
        const int32_t ramp = clamp(since_pickup - FP_HAND_LERP_DELAY_TURNS, 0, FP_HAND_LERP_RAMP_TURNS);
        const int32_t max_percent = (carrier_state->view_type == PVT_MapScreen) ? FP_HAND_PARCHMENT_LERP_PERCENT : FP_HAND_MAX_LERP_PERCENT;
        const int32_t rate = ramp * ramp * max_percent * 100 / (FP_HAND_LERP_RAMP_TURNS * FP_HAND_LERP_RAMP_TURNS); // out of 10000
        ustate->fp_hand_x += (MapCoord)((int64_t)(target_x - ustate->fp_hand_x) * rate / 10000);
        ustate->fp_hand_y += (MapCoord)((int64_t)(target_y - ustate->fp_hand_y) * rate / 10000);
    }
}

// x/y coord of camera for fp view
static void fp_view_position(const struct Thing *thing, MapCoord *x, MapCoord *y)
{
    *x = move_coord_with_angle_x(thing->mappos.x.val, -90, thing->move_angle_xy);
    *y = move_coord_with_angle_y(thing->mappos.y.val, -90, thing->move_angle_xy);
}

void begin_fp_hand_pickup(struct Thing *thing)
{
    const NetUserId possessor = thing_is_creature(thing) ? get_user_controlling_thing(thing, UCT_Direct) : -1;
    if (possessor < 0) {
        return;
    }
    struct UserState *ustate = get_user_state(possessor);
    ustate->fp_hand_pickup_turn = get_gameturn() + 1;
    fp_view_position(thing, &ustate->fp_hand_x, &ustate->fp_hand_y);
    thing->move_angle_z = FP_HAND_SQUISH_PITCH & ANGLE_MASK;
}

void process_fp_hand_drop_views(void)
{
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        struct UserState *ustate = get_user_state(user);
        if (user_state_invalid(ustate) || !user_present(user)) {
            continue;
        }
        const struct Thing *thing = thing_get(ustate->controlled_thing_idx);
        if (get_fp_hand_phase(user, thing, NULL) != FPHand_Drop) {
            continue;
        }
        MapCoord x;
        MapCoord y;
        fp_view_position(thing, &x, &y);
        ustate->fp_hand_x += (MapCoord)((int64_t)(x - ustate->fp_hand_x) * FP_HAND_MAX_LERP_PERCENT / 100);
        ustate->fp_hand_y += (MapCoord)((int64_t)(y - ustate->fp_hand_y) * FP_HAND_MAX_LERP_PERCENT / 100);
    }
}


TbBool fp_hand_blocks_creature_commands(const struct Thing *thing)
{
    if (thing_is_picked_up(thing)) {
        return true;
    }
    int32_t turn;
    const NetUserId possessor = get_user_controlling_thing(thing, UCT_Direct);
    return (get_fp_hand_phase(possessor, thing, &turn) == FPHand_Drop) && (turn < FP_HAND_DROP_CONTROL_TURNS);
}

void begin_fp_hand_drop(struct Thing *thing)
{
    if (!thing_is_creature(thing)) {
        return;
    }
    const NetUserId possessor = get_user_controlling_thing(thing, UCT_Direct);
    const enum FirstPersonHandPhase phase = get_fp_hand_phase(possessor, thing, NULL);
    if ((phase == FPHand_None) || (phase == FPHand_Drop)) {
        return;
    }
    struct UserState *ustate = get_user_state(possessor);
    ustate->fp_hand_drop_z = get_fp_hand_view_height(possessor, thing, NULL);
    ustate->fp_hand_drop_turn = get_gameturn() + 1;
    thing->move_angle_z = 0;
}

void restore_possessed_thing_from_hand(struct Thing *thing)
{
    if (!thing_is_creature(thing) || !flag_is_set(thing->alloc_flags, TAlF_IsControlled)) {
        return;
    }
    thing->rendering_flags |= TRF_Invisible;
    const struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
    if ((thing->light_id == 0) && !crconf->illuminated && !creature_under_spell_effect(thing, CSAfF_Light)) {
        create_light_for_possession(thing);
    }
}
/******************************************************************************/
