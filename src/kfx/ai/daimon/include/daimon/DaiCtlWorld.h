/** @file DaiCtlWorld.h
 *  The keeper controller's map questions for a Daimon keeper's slot (02 section 4.11), answered from that keeper's
 *  knowledge instead of the game's navigation, whose queries refresh caches as they go -- writes that would exist on
 *  this machine only (principle 8). Two spots are connected, and a creature can reach a spot, when the keeper's last
 *  complete reach map stands on them (its own creatures are on its ground); a target is valid when the keeper sees it
 *  (or owns it); drop spots are DaiFindDropSpot's (P4.7). Phase 1's verbs ask none of these; move and attack do. */
#ifndef DAIMON_DAICTLWORLD_H
#define DAIMON_DAICTLWORLD_H

#include <cstdint>

#include "keeper_ctl_types.h"
#include "daimon/DaiKnowledge.h"

class DaiCtlWorld {
public:
    explicit DaiCtlWorld(const DaiKnowledge &k);
    DaiCtlWorld(const DaiCtlWorld &) = delete;
    DaiCtlWorld &operator=(const DaiCtlWorld &) = delete;
    const KeeperCtlWorld *World() const { return &m_world; }
    /** Questions this keeper's controller asked, and how many were answered yes (main thread: kctl's planning). */
    int64_t Questions() const { return m_questions; }
    int64_t Yes() const { return m_yes; }
    /** For the answers (ctx is this world). */
    const DaiKnowledge &Know() const { return m_k; }
    bool Answer(bool yes) const { m_questions++; if (yes) m_yes++; return yes; }
private:
    const DaiKnowledge &m_k;
    KeeperCtlWorld m_world;
    mutable int64_t m_questions = 0, m_yes = 0;
};

#endif
