/** @file DaiOutput.h
 *  What one planning turn hands to the main thread (12 sections 3 and 13.5), applied D turns after the gather it was
 *  planned from: the gestures and cancels in the order the arbiter and the job tracking made them, the log lines, the
 *  veins Excavation edited and the slabs the live jobs watch. DaiOutput is the actor a planning turn sees: the
 *  controller as the gather found it, every gesture recorded under a provisional id from the keeper's own counter. */
#ifndef DAIMON_DAIOUTPUT_H
#define DAIMON_DAIOUTPUT_H

#include <cstdint>
#include <string>
#include <vector>

#include "globals.h"
#include "keeper_ctl_types.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiLog.h"
#include "daimon/DaiPlanWorld.h"
#include "daimon/DaiTypes.h"
#include "daimon/IDaiActor.h"

enum class DaiOpKind : uint8_t { Submit, Cancel, CancelAll };

struct DaiOutputOp {
    DaiOpKind kind = DaiOpKind::Submit;
    int64_t verb_id = 0;        /**< provisional (Submit, Cancel) */
    KeeperVerb verb{};          /**< Submit */
};

struct DaiPlanOutput {
    DaiTurn planned = 0;       /**< the gather's turn */
    DaiTurn lands = 0;         /**< the turn it is applied */
    std::vector<DaiOutputOp> ops;
    std::vector<DaiLogLine> lines;
    std::vector<DaiVeinEdit> veins;
    std::vector<DaiSlabXY> watch;
    std::vector<DaiQuestion> questions;     /**< the live checks it asked, for the gather after its apply */
    std::string status;         /**< an assistant's line for the Options screen (DaimonAssistStatus), written by plan */
};

class DaiOutput : public IDaiActor {
public:
    /** `idle`: the controller had nothing running or waiting at the gather. Ids come from *next_id (the last given). */
    DaiOutput(DaiPlanOutput &out, bool idle, int64_t *next_id) : m_out(out), m_idle(idle), m_next_id(next_id) {}
    bool Idle(int64_t slot) const override { (void)slot; return m_idle && !m_submitted; }
    bool Submitted() const { return m_submitted; }
    const char *Submit(int64_t slot, PlayerNumber p, const KeeperVerb &v, int64_t *verb_id) override;
    /** The controller's checks are made at apply: nothing is refused here. */
    const char *Check(int64_t slot, PlayerNumber p, const KeeperVerb &v) const override;
    /** Results reach a planning turn from the gather, not from here. */
    int64_t Results(int64_t slot, DaiActResult *out, int64_t max, int64_t since) const override;
    void Cancel(int64_t slot, int64_t verb_id) override;
    void CancelAll(int64_t slot) override;
    bool SaveQueue(int64_t slot, std::vector<uint8_t> *out) const override;
    bool LoadQueue(int64_t slot, const uint8_t *data, size_t len) override;
private:
    DaiPlanOutput &m_out;
    bool m_idle;
    bool m_submitted = false;
    int64_t *m_next_id;
};

#endif
