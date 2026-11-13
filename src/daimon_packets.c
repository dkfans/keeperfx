/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file daimon_packets.c
 *     The packet actions Daimon's keeper controller sends (daimon_packets.h): its self-contained gestures and the keeper's group, handled with the same functions and checks a human's clicks use. The fork's game_commands.c handlers, as Daimon's port tools copy them (scripts/daimon_port/sync.py in the fork).
 */
/******************************************************************************/
#include "pre_inc.h"
#include "daimon_packets.h"
#include "keeper_group.h"
#include "packets.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "game_legacy.h"
#include "daimon_engine.h"
#include "keeper_exclusion.h"
#include "config_daimon.h"
#include "magic_powers.h"
#include "player_instances.h"
#include "map_blocks.h"
#include "roomspace.h"
#include "power_hand.h"
#include "room_data.h"
#include "slab_data.h"
#include "thing_traps.h"
#include "thing_doors.h"
#include "config_trapdoor.h"
#include "config_terrain.h"
#include "net_main.h"
#include "creature_control.h"
#include "thing_creature.h"
#include "player_utils.h"
#include "daimon_slots.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** What the fork's packet dispatch hands its handlers (game_commands.c's GlobalAction): the packet, its keeper, its user and the seat it acts for. */
struct GlobalAction {
    const struct Packet *pckt;
    PlayerNumber plyr_idx;
    NetUserId user;
    struct PlayerInfo *player;
};

static TbBool global_daimon_cheats(const struct GlobalAction *ctx)
{
    const struct Packet *pckt = ctx->pckt;
    if (ctx->user != SERVER_ID) {
        WARNLOG("Ignoring Daimon's cheats from user %" PRId64 "; only the host may send them", (int64_t)ctx->user);
        return 0;
    }
    const PlayerNumber plyr_idx = (PlayerNumber)pckt->actn_par1;
    const int64_t gold = pckt->actn_par2, flags = (uint16_t)pckt->actn_par3 & 0xF, discount = (uint16_t)pckt->actn_par3 >> 4;
    const int64_t bonus = pckt->actn_par4;
    const char *why = ((plyr_idx < 0) || (plyr_idx >= PLAYERS_COUNT)) ? "no such player"
        : (daimon_slot_of(plyr_idx) < 0) ? "not a Daimon keeper"
        : (((flags & ~6) != 0) || (flags == 0)) ? "unknown flags"
        : ((gold < 0) || (gold > 1000000) || (bonus < 0) || (bonus > 400) || (discount > 100)) ? "amounts out of range" : NULL;
    if (why != NULL) {
        WARNLOG("Ignoring Daimon's cheats for player %" PRId64 ": %s", (int64_t)pckt->actn_par1, why);
        return 0;
    }
    struct PlayerInfo *player = get_player(plyr_idx);
    if (flag_is_set(player->allocflags, PlaF_DaimonCheats)) return 0;
    set_flag(player->allocflags, PlaF_DaimonCheats);
    struct Dungeon *dungeon = get_dungeon(plyr_idx);
    if ((flags & 2) != 0) player_add_offmap_gold(plyr_idx, gold);
    if ((flags & 4) != 0) {
        dungeon->modifier.health += bonus;
        dungeon->modifier.strength += bonus;
        dungeon->modifier.armour += bonus;
        dungeon->modifier.pay = (dungeon->modifier.pay > discount) ? dungeon->modifier.pay - discount : 0;
        do_to_players_all_creatures_of_model(plyr_idx, CREATURE_ANY, update_relative_creature_health);
    }
    JUSTLOG("p:%" PRId64 " Daimon's cheats applied (flags %" PRId64 ", gold %" PRId64 ", creatures +%" PRId64 "%%, pay -%" PRId64 "%%)",
        (int64_t)plyr_idx, flags, ((flags & 2) != 0) ? gold : 0, ((flags & 4) != 0) ? bonus : 0, ((flags & 4) != 0) ? discount : 0);
    return 0;
}

static TbBool global_keeper_group(const struct GlobalAction *ctx)
{
    const struct Packet *pckt = ctx->pckt;
    const PlayerNumber plyr_idx = ctx->plyr_idx;
    const char *why = NULL;
    switch (pckt->action) {
    case PckA_FormGroup:
        why = keeper_group_form(plyr_idx, (DaiThingIndex)pckt->actn_par1, true);
        break;
    case PckA_GroupAdd: {
        const int64_t members[4] = { pckt->actn_par1, pckt->actn_par2, pckt->actn_par3, pckt->actn_par4 };
        for (int i = 0; i < 4; i++) {
            if (members[i] <= 0) continue;
            const char *w = keeper_group_add(plyr_idx, (DaiThingIndex)members[i], true);
            if (w != NULL) {
                WARNLOG("p:%d group: member %" PRId64 " not added (%s)", (int)plyr_idx, members[i], w);
            }
        }
        break;
    }
    case PckA_OrderGroup:
        why = keeper_group_order(plyr_idx, (uint8_t)(pckt->actn_par1 & 0xFF), (DaiSubtlCoord)packet_unpack_x(pckt->actn_par2),
            (DaiSubtlCoord)packet_unpack_y(pckt->actn_par2), (int64_t)((uint32_t)pckt->actn_par1 >> 8), true);
        break;
    case PckA_ReleaseGroup:
        why = keeper_group_release(plyr_idx, "released", true);
        break;
    default:
        break;
    }
    if (why != NULL) {
        WARNLOG("p:%d group packet %d refused (%s)", (int)plyr_idx, (int)pckt->action, why);
    }
    return 0;
}

static void unpack_xy(int32_t v, int64_t *x, int64_t *y)
{
    *x = packet_unpack_x(v);
    *y = packet_unpack_y(v);
}

static const char *dai_gesture(const struct GlobalAction *ctx)
{
    const struct Packet *pckt = ctx->pckt;
    const PlayerNumber plyr_idx = ctx->plyr_idx;
    const NetUserId user = ctx->user;
    struct PlayerInfo *player = ctx->player;
    struct UserState *ustate = get_user_state(user); // the gesture's own hand and tools
    struct Dungeon *dungeon = get_players_dungeon(player);
    if (!player_exists(player) || dungeon_invalid(dungeon))
        return "NO_KEEPER";
    int64_t ax, ay, bx, by;
    unpack_xy(pckt->actn_par1, &ax, &ay);
    unpack_xy(pckt->actn_par2, &bx, &by);
    switch (pckt->action) {
    case PckA_DaiDigRect: {
        // As a pickaxe drag's walk over its slabs (apply_roomspace_dig_tag_selection), the mode given, not taken
        // from the first slab.
        if (slab_coords_invalid(ax, ay) || slab_coords_invalid(bx, by))
            return "OFF_MAP";
        const TbBool untag = (pckt->actn_par3 != 0);
        int64_t changed = 0;
        for (int64_t y = min(ay, by); y <= max(ay, by); y++)
            for (int64_t x = min(ax, bx); x <= max(ax, bx); x++) {
                const DaiSubtlCoord stl_x = slab_subtile(x, 0), stl_y = slab_subtile(y, 0);
                if (untag)
                    changed += untag_blocks_for_digging_in_area(stl_x, stl_y, plyr_idx);
                else if (dungeon->task_count < MAPTASKS_COUNT)
                    changed += tag_blocks_for_digging_in_area(stl_x, stl_y, plyr_idx);
                else
                    return "TASKS_FULL";
            }
        return NULL;
    }
    case PckA_DaiBuildRect: {
        // As a released room drag (get_dungeon_build_user_roomspace's drag branch, then the preview's check).
        const RoomKind rkind = (RoomKind)pckt->actn_par3;
        if (slab_coords_invalid(ax, ay) || slab_coords_invalid(bx, by))
            return "OFF_MAP";
        if ((rkind <= 0) || (rkind >= game.conf.slab_conf.room_types_count) || !is_room_available(plyr_idx, rkind))
            return "NOT_AVAILABLE";
        if (ustate->roomspace.is_active)
            return "BUILD_BUSY";
        struct RoomConfigStats *roomst = get_room_kind_stats(rkind);
        struct RoomSpace rs;
        memset(&rs, 0, sizeof(rs));
        rs.is_roomspace_a_box = true;
        rs.render_roomspace_as_box = true;
        rs.plyr_idx = plyr_idx;
        rs.user = user;
        rs.rkind = rkind;
        const TbBool bridge = room_role_matches(rkind, RoRoF_PassWater|RoRoF_PassLava|RoRoF_PassAbyss);
        if (bridge && !can_build_room_at_slab(plyr_idx, rkind, ax, ay) && !players_land_by_bridgeable_slab(plyr_idx, ax, ay))
            return "CANNOT_BUILD_HERE";
        rs = create_box_roomspace_from_drag(rs, ax, ay, bx, by);
        if (rs.drag_mode && bridge)
            detect_bridge_shape(plyr_idx, &rs);
        rs = check_slabs_in_roomspace(rs, roomst->cost);
        if (can_build_roomspace(plyr_idx, rkind, rs) <= 0)
            return "CANNOT_BUILD_HERE";
        keeper_start_roomspace(user, &rs, rkind);
        return NULL;
    }
    case PckA_DaiSellRect: {
        // As a released sell drag (get_dungeon_sell_user_roomspace's drag branch).
        if (slab_coords_invalid(ax, ay) || slab_coords_invalid(bx, by))
            return "OFF_MAP";
        if (ustate->roomspace.is_active)
            return "BUILD_BUSY";
        struct RoomSpace rs;
        memset(&rs, 0, sizeof(rs));
        rs.plyr_idx = plyr_idx;
        rs.user = user;
        rs = create_box_roomspace_from_drag(rs, ax, ay, bx, by);
        rs = check_roomspace_for_sellable_slabs(rs, plyr_idx);
        if (rs.slab_count <= 0)
            return "NOTHING_TO_SELL";
        keeper_start_roomspace(user, &rs, RoK_SELL);
        return NULL;
    }
    case PckA_DaiPlaceTrap: {
        // As a click with the trap tool (process_dungeon_control_packet_dungeon_place_trap).
        const DaiThingModel kind = (DaiThingModel)pckt->actn_par2;
        if (subtile_coords_invalid(ax, ay))
            return "OFF_MAP";
        if ((kind <= 0) || (kind >= game.conf.trapdoor_conf.trap_types_count))
            return "NOT_AVAILABLE";
        if (!can_place_trap_on(plyr_idx, ax, ay, kind))
            return "CANNOT_PLACE_HERE";
        return player_place_trap_at(ax, ay, plyr_idx, user, kind) ? NULL : "NONE_IN_STOCK";
    }
    case PckA_DaiPlaceDoor: {
        // As a click with the door tool (the PSt_PlaceDoor case of process_dungeon_control_packet_clicks).
        const DaiThingModel kind = (DaiThingModel)pckt->actn_par2;
        if (subtile_coords_invalid(ax, ay))
            return "OFF_MAP";
        if ((kind <= 0) || (kind >= game.conf.trapdoor_conf.door_types_count))
            return "NOT_AVAILABLE";
        if (!door_placement_allowed(plyr_idx, ax, ay))
            return "CANNOT_PLACE_HERE";
        return (packet_place_door(ax, ay, plyr_idx, user, kind, true) != 0) ? NULL : "NONE_IN_STOCK";
    }
    case PckA_DaiDropAt: {
        // As a drop from the hand (PckA_UsePwrHandDrop), the creature paid as the one under the hand.
        if (subtile_coords_invalid(ax, ay))
            return "OFF_MAP";
        if (ustate->num_things_in_hand <= 0)
            return "HAND_EMPTY";
        if ((pckt->actn_par2 > 0) && (ustate->things_in_hand[0] != (DaiThingIndex)pckt->actn_par2))
            return "HAND_CHANGED";
        // What the keeper's human is doing now stays theirs (keeper_exclusion.h): no drop by the creature they possess,
        // no pay to a creature they hold, possess, lead or point at.
        if (keeper_area_excluded(plyr_idx, ax, ay, daimon_config.global.assistant_possession_radius))
            return "NEAR_POSSESSED";
        if ((pckt->actn_par3 > 0) && keeper_thing_excluded(plyr_idx, thing_get((DaiThingIndex)pckt->actn_par3)))
            return "EXCLUDED_BY_PLAYER";
        if (pckt->actn_par3 > 0)
            ustate->thing_under_hand = (DaiThingIndex)pckt->actn_par3;
        return dump_first_held_thing_on_map(plyr_idx, user, ax, ay, 1) ? NULL : "CANNOT_DROP_HERE";
    }
    case PckA_DaiCastAt: {
        // As a click released with a subtile power chosen (the PSt_CastPowerOnSubtile family's click), at the level given.
        const DaiPowerKind pwkind = (DaiPowerKind)pckt->actn_par2;
        if (subtile_coords_invalid(ax, ay))
            return "OFF_MAP";
        if ((pwkind <= 0) || (pwkind >= game.conf.magic_conf.power_types_count) || !is_power_available(plyr_idx, pwkind))
            return "NOT_AVAILABLE";
        const uint64_t ws = get_power_model_stats(pwkind)->work_state;
        if ((ws != PSt_CastPowerOnSubtile) && (ws != PSt_CallToArms) && (ws != PSt_SightOfEvil) && (ws != PSt_CreateDigger))
            return "NOT_A_SUBTILE_POWER";
        // Nothing cast by the creature the keeper's human possesses (keeper_exclusion.h).
        if (keeper_area_excluded(plyr_idx, ax, ay, daimon_config.global.assistant_possession_radius))
            return "NEAR_POSSESSED";
        const KeepPwrLevel level = (KeepPwrLevel)max(0, min(POWER_MAX_LEVEL, pckt->actn_par3));
        return (magic_use_available_power_on_subtile(plyr_idx, user, pwkind, level, ax, ay, PwCast_None, PwMod_Default) == Lb_SUCCESS)
            ? NULL : "CANNOT_CAST_HERE";
    }
    case PckA_DaiHandPick: {
        // As PckA_UsePwrHandPick from the map; never a thing the keeper's human holds, possesses, leads, points at or
        // has open in the info panel.
        struct Thing *thing = thing_get((DaiThingIndex)pckt->actn_par1);
        if (thing_is_invalid(thing))
            return "NO_SUCH_THING";
        if (keeper_thing_excluded(plyr_idx, thing))
            return "EXCLUDED_BY_PLAYER";
        // Checked here: use_power_hand takes the thing under the cursor -- the human's -- for one it cannot take.
        if (!can_thing_be_picked_up_by_player(thing, plyr_idx))
            return "CANNOT_PICK_UP";
        return (use_power_hand(plyr_idx, user, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing->index) == Lb_SUCCESS)
            ? NULL : "HAND_REFUSED";
    }
    case PckA_DaiCastOn: {
        // As PckA_UsePwrOnThing, at the level given and with no camera angle (a slap leaves the camera alone).
        struct Thing *thing = thing_get((DaiThingIndex)pckt->actn_par2);
        if (thing_is_invalid(thing))
            return "NO_SUCH_THING";
        if (keeper_thing_excluded(plyr_idx, thing))
            return "EXCLUDED_BY_PLAYER";
        // Availability checked here, as DaiCastAt does: the cast path's own check warns and plays the refusal sound to
        // the local player -- the human, for the assistant (14 R13).
        const DaiPowerKind pwkind = (DaiPowerKind)pckt->actn_par1;
        if ((pwkind <= 0) || (pwkind >= game.conf.magic_conf.power_types_count) || !is_power_available(plyr_idx, pwkind))
            return "NOT_AVAILABLE";
        const KeepPwrLevel level = (KeepPwrLevel)max(0, min(POWER_MAX_LEVEL, pckt->actn_par3));
        return (magic_use_available_power_on_thing(plyr_idx, user, pwkind, level, thing->mappos.x.stl.num, thing->mappos.y.stl.num, thing,
                    PwMod_Default) == Lb_SUCCESS) ? NULL : "CANNOT_CAST";
    }
    default:
        return "UNKNOWN";
    }
}

static TbBool global_dai_gesture(const struct GlobalAction *ctx)
{
    const char *why = dai_gesture(ctx);
    if (why != NULL)
        WARNLOG("p:%d gesture packet %d refused (%s)", (int)ctx->plyr_idx, (int)ctx->pckt->action, why);
    return 0;
}

TbBool daimon_process_packet_action(NetUserId user)
{
    struct GlobalAction ctx;
    ctx.pckt = get_packet(user);
    ctx.plyr_idx = get_net_user_player_number(user);
    ctx.user = user;
    ctx.player = get_player(ctx.plyr_idx);
    switch (ctx.pckt->action) {
    case PckA_DaimonCheats:
        return global_daimon_cheats(&ctx);
    case PckA_FormGroup: case PckA_GroupAdd: case PckA_OrderGroup: case PckA_ReleaseGroup:
        return global_keeper_group(&ctx);
    default:
        return global_dai_gesture(&ctx);
    }
}

#ifdef __cplusplus
}
#endif
