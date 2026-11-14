#include "pre_inc.h"
#include "daimon/DaiActorKeeperCtl.h"

#include <cstring>

#include "keeper_ctl.h"
#include "packets.h"
#include "player_data.h"
#include "player_instances.h"
#include "game_legacy.h"
#include "post_inc.h"

namespace {
const DaiActorKeeperCtl &actor_of(void *ctx) { return *static_cast<const DaiActorKeeperCtl *>(ctx); }

TbBool no_connection(void *ctx, DaiSubtlCoord ax, DaiSubtlCoord ay, DaiSubtlCoord bx, DaiSubtlCoord by)
{
    (void)ax; (void)ay; (void)bx; (void)by;
    actor_of(ctx).Asked();
    return false;
}

TbBool no_reach(void *ctx, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y)
{
    (void)creature; (void)x; (void)y;
    actor_of(ctx).Asked();
    return false;
}

TbBool no_target(void *ctx, DaiThingIndex target)
{
    (void)target;
    actor_of(ctx).Asked();
    return false;
}

const char *no_drop(void *ctx, PlayerNumber plyr, DaiThingIndex creature, DaiSubtlCoord x, DaiSubtlCoord y, int64_t radius,
    DaiSubtlCoord reach_x, DaiSubtlCoord reach_y, DaiSubtlCoord *out_x, DaiSubtlCoord *out_y)
{
    (void)plyr; (void)creature; (void)x; (void)y; (void)radius; (void)reach_x; (void)reach_y; (void)out_x; (void)out_y;
    actor_of(ctx).Asked();
    return "NO_DROP_SPOT";
}

}

DaiActorKeeperCtl::DaiActorKeeperCtl() : m_refusing{ this, no_connection, no_reach, no_target, no_drop } {}

void DaiActorKeeperCtl::Attach(int64_t slot, const KeeperCtlWorld *world)
{
    kctl_set_world(slot, (world != nullptr) ? world : &m_refusing);
    // Order creature mode is a cheat state the game refuses in a network game: Daimon's creatures move as a group.
    kctl_set_group_orders(slot, true);
}

void DaiActorKeeperCtl::Detach(int64_t slot)
{
    kctl_cancel(slot);
    kctl_set_world(slot, nullptr);
    kctl_set_group_orders(slot, false);
}

void DaiActorKeeperCtl::AttachLieutenant(int64_t slot, const KeeperCtlWorld *world)
{
    kctl_set_lieutenant_planning(slot, (world != nullptr) ? world : &m_refusing, true);
    m_lieutenant.insert(slot);
}

void DaiActorKeeperCtl::DetachLieutenant(int64_t slot)
{
    kctl_cancel_lane_and_running(slot, EXTSEAT_LANE_LIEUTENANT);
    kctl_set_lieutenant_planning(slot, nullptr, false);
    m_lieutenant.erase(slot);
}

bool DaiActorKeeperCtl::Idle(int64_t slot) const
{
    return lieutenant(slot) ? !kctl_lane_busy(slot, EXTSEAT_LANE_LIEUTENANT) : kctl_idle(slot);
}

const char *DaiActorKeeperCtl::Submit(int64_t slot, PlayerNumber p, const KeeperVerb &v, int64_t *verb_id)
{
    ExtSeatSubmitInfo info;
    memset(&info, 0, sizeof(info));
    // Queued behind the running gesture if there is one (a fast-forwarded keeper's, 12 section 13.13; at normal speed a
    // keeper submits only to an idle controller).
    const char *err = lieutenant(slot) ? kctl_submit_lane(slot, p, &v, true, EXTSEAT_LANE_LIEUTENANT, &info)
                                       : kctl_submit(slot, p, &v, true, &info);
    *verb_id = (err == nullptr) ? info.id : 0;
    return err;
}

const char *DaiActorKeeperCtl::Check(int64_t slot, PlayerNumber p, const KeeperVerb &v) const
{
    int64_t steps = 0;
    return lieutenant(slot) ? kctl_check_lane(slot, p, &v, EXTSEAT_LANE_LIEUTENANT, &steps) : kctl_check(slot, p, &v, &steps);
}

int64_t DaiActorKeeperCtl::Results(int64_t slot, DaiActResult *out, int64_t max, int64_t since) const
{
    KeeperCtlResult res[EXTSEAT_RESULT_RING];
    const int64_t have = kctl_results(slot, res, EXTSEAT_RESULT_RING);
    int64_t n = 0;
    for (int64_t i = 0; (i < have) && (n < max); i++) {
        if (res[i].id <= since) continue;
        if (lieutenant(slot) && (res[i].lane != EXTSEAT_LANE_LIEUTENANT)) continue;   // the agent's verbs are not its own
        out[n].verb_id = res[i].id;
        out[n].failed = res[i].rejected;
        snprintf(out[n].error, sizeof(out[n].error), "%s", res[i].error);
        n++;
    }
    return n;
}

void DaiActorKeeperCtl::Cancel(int64_t slot, int64_t verb_id)
{
    kctl_cancel_id(slot, verb_id);
}

void DaiActorKeeperCtl::CancelAll(int64_t slot)
{
    if (lieutenant(slot)) kctl_cancel_lane_and_running(slot, EXTSEAT_LANE_LIEUTENANT);   // never the agent's verbs
    else kctl_cancel(slot);
}

bool DaiActorKeeperCtl::SaveQueue(int64_t slot, std::vector<uint8_t> *out) const
{
    if (lieutenant(slot)) return false;         // the seat's queue, the agent's verbs in it: not Daimon's to save
    const size_t n = kctl_serialise(slot, nullptr, 0);
    if (n == 0) return false;
    out->resize(n);
    return kctl_serialise(slot, out->data(), n) == n;
}

bool DaiActorKeeperCtl::LoadQueue(int64_t slot, const uint8_t *data, size_t len)
{
    return kctl_deserialise(slot, data, len);
}

void DaiActorKeeperCtl::ResetSlot(int64_t slot)
{
    kctl_reset(slot);
}

void DaiActorKeeperCtl::SetSelfContained(int64_t slot)
{
    kctl_set_self_contained(slot, true);
}

void DaiActorKeeperCtl::Tick(int64_t slot, PlayerNumber p, struct Packet *pk)
{
    kctl_sweep(slot);
    kctl_tick(slot, p, (pk != nullptr) ? pk : get_packet(slot));
}

bool DaiActorKeeperCtl::HumanPacketFree(PlayerNumber p, int64_t user)
{
    const struct Packet *pk = get_packet(user);
    if (pk->action != PckA_None) return false;
    const uint64_t buttons = PCtr_LBtnClick | PCtr_RBtnClick | PCtr_LBtnHeld | PCtr_RBtnHeld | PCtr_LBtnRelease | PCtr_RBtnRelease;
    if ((pk->control_flags & buttons) != 0) return false;
    // the slot's own hand and tools
    const struct UserState *player = get_user_state(user);
    if (user_state_invalid(player)) return false;
    if (player->roomspace.is_active || (get_user_state(user)->cursor_button_down != 0)) return false;
    return (player->instance_num != PI_Grab) && (player->instance_num != PI_Drop) && (player->instance_num != PI_Whip)
           && (player->instance_num != PI_WhipEnd);
}

void DaiActorKeeperCtl::AssistState(PlayerNumber p, bool *rules_on, bool *by_daimon)
{
    (void)p; // no combat assistant in this build
    if (rules_on != nullptr) *rules_on = false;
    if (by_daimon != nullptr) *by_daimon = false;
}

void DaiActorKeeperCtl::WriteDelegate(int64_t user, bool on)
{
    (void)user; (void)on; // no combat assistant in this build
}

bool DaiActorKeeperCtl::PacketHasAction(int64_t user)
{
    return get_packet(user)->action != PckA_None;
}

const char *DaiActorKeeperCtl::QueueDelegate(int64_t user, PlayerNumber p, bool on, bool lieutenant_lane)
{
    KeeperVerb v;
    memset(&v, 0, sizeof(v));
    v.kind = ESV_AssistDelegate;
    v.has_enabled = true;
    v.enabled = on;
    ExtSeatSubmitInfo info;
    memset(&info, 0, sizeof(info));
    return kctl_submit_lane(user, p, &v, true, lieutenant_lane ? EXTSEAT_LANE_LIEUTENANT : EXTSEAT_LANE_ROUTINE, &info);
}

bool DaiActorKeeperCtl::LieutenantWroteThisTurn(int64_t slot)
{
    return kctl_lane_wrote_this_turn(slot, EXTSEAT_LANE_LIEUTENANT);
}
