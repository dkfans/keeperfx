/** @file IDaiActor.h
 *  The act seam (02 section 4.11): how a keeper's gestures leave Daimon. DaiActorKeeperCtl submits them to the keeper
 *  controller of the keeper's packet slot; tests use DaiFakeActor; Phase 10's DaiOutput queues them for the main
 *  thread. One gesture runs at a time per slot, at the controller's one packet a turn; a fast-forwarded keeper's next
 *  ones wait in the controller's queue behind it. */
#ifndef DAIMON_IDAIACTOR_H
#define DAIMON_IDAIACTOR_H

#include <cstdint>
#include <vector>

#include "globals.h"
#include "keeper_ctl_types.h"

/** What became of a submitted verb. */
struct DaiActResult {
    int64_t verb_id = 0;
    bool failed = false;        /**< refused when its turn came (the gesture never ran) */
    char error[28] = {};        /**< the controller's code when failed, kept verbatim */
};

class IDaiActor {
public:
    virtual ~IDaiActor() = default;
    /** Nothing running or waiting: a new gesture would start now. */
    virtual bool Idle(int64_t slot) const = 0;
    /** nullptr and the verb's id in *verb_id, or the controller's error code (nothing submitted). */
    virtual const char *Submit(int64_t slot, PlayerNumber p, const KeeperVerb &v, int64_t *verb_id) = 0;
    /** The submit's checks without submitting: nullptr if it would be accepted. */
    virtual const char *Check(int64_t slot, PlayerNumber p, const KeeperVerb &v) const = 0;
    /** Results of verbs whose id is above `since`, oldest first. */
    virtual int64_t Results(int64_t slot, DaiActResult *out, int64_t max, int64_t since) const = 0;
    virtual void Cancel(int64_t slot, int64_t verb_id) = 0;
    virtual void CancelAll(int64_t slot) = 0;
    /** The slot's gesture in progress and its recent results, for a saved game: false if what the slot holds cannot be
     *  kept (it is then dropped at the load, and the stall rule ends the job that waited on it). */
    virtual bool SaveQueue(int64_t slot, std::vector<uint8_t> *out) const = 0;
    /** Puts a saved queue back into the slot; false (the slot is left empty) on anything malformed. */
    virtual bool LoadQueue(int64_t slot, const uint8_t *data, size_t len) = 0;
};

#endif
