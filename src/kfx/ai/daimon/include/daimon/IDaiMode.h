/** @file IDaiMode.h
 *  What a keeper's mode allows (02 section 4.21, as built 02 section A.5): which domains run, what it refuses, how it
 *  fights, which jobs wait; and its own state, saved in the keeper's MODE section (14 F2). Opponent (Phase 0),
 *  assistant (Phase 7) and lieutenant (Phase 8). */
#ifndef DAIMON_IDAIMODE_H
#define DAIMON_IDAIMODE_H

#include "keeper_ctl_types.h"
#include "daimon/DaiBlob.h"
#include "daimon/DaiTypes.h"

struct DaiTurnCtx;
struct DaiJob;

/** Whose fight rules Tactics casts by (02 section 4.8a, 09 section 5.4): the personality's, the human's combat-assistant
 *  rules (the assistant, delegated), or none (the human's runner casts). */
enum class DaiFightCasting : uint8_t { Personality, Human, None };

class IDaiMode {
public:
    virtual ~IDaiMode() = default;
    virtual DaimonMode GetMode() const = 0;
    virtual const char *GetName() const = 0;
    virtual bool DomainActive(DaiDomain d) const = 0;
    /** Why the mode refuses a gesture before it is submitted (the arbiter asks; nullptr: allowed). The assistant's:
     *  what the human is doing now, what they touched lately, the slab decisions they made (09 sections 5.2a, 5.3). */
    virtual const char *Refuses(const DaiTurnCtx &c, const KeeperVerb &v) const { (void)c; (void)v; return nullptr; }
    virtual DaiFightCasting FightCasting(const DaiTurnCtx &c) const { (void)c; return DaiFightCasting::Personality; }
    /** A job the mode holds back (a lieutenant's advised domain, until the agent accepts it: 10 section 3). */
    virtual bool HoldsJob(const DaiJob &j) const { (void)j; return false; }
    /** The mode's own state (the keeper's MODE section): false when the data is not the mode's. */
    virtual void Save(DaiBlobWriter &w) const { (void)w; }
    virtual bool Load(DaiBlobReader &r) { (void)r; return true; }
};

#endif
