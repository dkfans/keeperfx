/** @file DaiActorKeeperCtl.h
 *  IDaiActor over the keeper controller (kfx_net's kctl_*), for the packet slot of each Daimon keeper. Attach()
 *  installs the slot's KeeperCtlWorld: the controller's planning must not ask the game's navigation, whose answers
 *  refresh caches as they go -- writes that would exist on this machine only (principle 8). The keeper's world
 *  (DaiCtlWorld) answers from its knowledge; without one every map question is answered "no", so a verb that needs one
 *  fails with the controller's code instead of touching the game.
 *  It is also the one place Daimon writes a packet or drives the controller itself (14 C1): the slot's step each turn,
 *  the assistant's gate on the human's packet and its PckA_AssistDelegate, a seat's delegation verb. */
#ifndef DAIMON_DAIACTORKEEPERCTL_H
#define DAIMON_DAIACTORKEEPERCTL_H

#include <set>

#include "keeper_ctl_types.h"       // KeeperCtlWorld

#include "daimon/IDaiActor.h"

struct Packet;

class DaiActorKeeperCtl : public IDaiActor {
public:
    DaiActorKeeperCtl();
    DaiActorKeeperCtl(const DaiActorKeeperCtl &) = delete;          // the refusing world's ctx is this
    DaiActorKeeperCtl &operator=(const DaiActorKeeperCtl &) = delete;
    /** A keeper starts writing `slot` (after the slot was handed to it, which resets the controller); `world` answers
     *  the controller's map questions (nullptr: "no" to each). */
    void Attach(int64_t slot, const KeeperCtlWorld *world = nullptr);
    /** It stops: its verbs are cancelled and the slot's controller gets the default world back. */
    void Detach(int64_t slot);
    /** A seat's lieutenant (P8.2) writes `slot` beside the agent: its verbs go into the lieutenant lane, planned with
     *  `world` and group orders; the slot's own world and orders stay the agent's. Idle and the cancels are its lane's. */
    void AttachLieutenant(int64_t slot, const KeeperCtlWorld *world);
    void DetachLieutenant(int64_t slot);
    bool Idle(int64_t slot) const override;
    const char *Submit(int64_t slot, PlayerNumber p, const KeeperVerb &v, int64_t *verb_id) override;
    const char *Check(int64_t slot, PlayerNumber p, const KeeperVerb &v) const override;
    int64_t Results(int64_t slot, DaiActResult *out, int64_t max, int64_t since) const override;
    void Cancel(int64_t slot, int64_t verb_id) override;
    void CancelAll(int64_t slot) override;
    bool SaveQueue(int64_t slot, std::vector<uint8_t> *out) const override;
    bool LoadQueue(int64_t slot, const uint8_t *data, size_t len) override;
    /** Map questions the controller asked of the refusing world (each answered "no"). */
    int64_t MapQuestions() const { return m_questions; }
    void Asked() const { m_questions++; }

    // ---- The packet side the facade drives once a turn (main thread) ----
    /** A slot handed to a keeper: its controller starts clean. */
    static void ResetSlot(int64_t slot);
    /** The slot's gestures carry everything they need (slot 0, the human's, for the assistant: no tool or cursor). */
    static void SetSelfContained(int64_t slot);
    /** The slot's controller writes this turn's step of its running gesture into `pk` (nullptr: the slot's own packet). */
    static void Tick(int64_t slot, PlayerNumber p, struct Packet *pk = nullptr);
    /** A human's packet (`user`'s) is Daimon's to write this turn (09 section 5.2): its action slot free, no button, no
     *  drag or room build under way, no hand animation playing. */
    static bool HumanPacketFree(PlayerNumber p, int64_t user);
    /** The keeper's combat-assistant rules: on (`*rules_on`), and run by Daimon rather than the game (`*by_daimon`). */
    static void AssistState(PlayerNumber p, bool *rules_on, bool *by_daimon);
    /** PckA_AssistDelegate into `user`'s packet: Daimon takes the rules over (true) or hands them back. */
    static void WriteDelegate(int64_t user, bool on);
    /** `user`'s packet has an action this turn. */
    static bool PacketHasAction(int64_t user);
    /** A seat's delegation verb (ESV_AssistDelegate) into its queue, the lieutenant's lane or the routine one: NULL or
     *  the controller's refusal. */
    static const char *QueueDelegate(int64_t user, PlayerNumber p, bool on, bool lieutenant_lane);
    /** A step of the lieutenant's lane was written into `slot`'s packet this turn. */
    static bool LieutenantWroteThisTurn(int64_t slot);
private:
    bool lieutenant(int64_t slot) const { return m_lieutenant.count(slot) != 0; }
    std::set<int64_t> m_lieutenant;
    KeeperCtlWorld m_refusing;
    mutable int64_t m_questions = 0;
};

#endif
