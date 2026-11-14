/** @file DaiFightRules.h
 *  Which fight rules Tactics runs, and the rules themselves (02 section 4.8a, 06 section 7): rule sets in the combat
 *  assistant's language (cast rules), one bank per kind of objective, handed to Tactics as a KeeperFightRules view.
 *  The personality's tactics traits are the one source until Phases 7-8 add the human's and the agent's rules. Two banks:
 *  the defensive one (Phase 4) and the offensive one (07 section 4.3, P5.6) for fights matched to an Assault or Raid. */
#ifndef DAIMON_DAIFIGHTRULES_H
#define DAIMON_DAIFIGHTRULES_H

#include <array>
#include <cstdint>

#include "keeper_assist.h"
#include "daimon/DaiContent.h"
#include "daimon/DaiPersonality.h"
#include "daimon/IDaiPlanWorld.h"
#include "daimon/IDaiWorldReader.h"

enum class DaiFightRulesSource : uint8_t { Personality, Human };   // an agent's rules are the seat's: Human (10 P8.7)
/** The banks: a rule set per kind of objective. */
enum class DaiFightBank : uint8_t { Defensive, Offensive, Count };

class DaiFightRules {
public:
    DaiFightRules();
    DaiFightRulesSource Source() const { return m_source; }
    /** The active bank's rules. */
    const KeeperFightRules &View() const { return m_view; }
    void SetBank(DaiFightBank b);
    DaiFightBank Bank() const { return m_bank; }
    /** Generates every bank from the personality's tactics traits, for the game's powers (a power the game has not
     *  got, or a rule its rule check refuses, is left out and logged). */
    /** final: Armageddon's last fight (P9) -- no gold kept back, no spending cap. */
    /** `spare`: the forecast's spare gold, cast_spare_share % of which the spells may spend in a pay day (plan 13). */
    /** `content` (Phase 7, 09 section 13.3): a map pack's own strike, disable, boost and heal powers each get the rule of
     *  KeeperFX's power of that role (lightning, freeze at home and slow abroad, protect, heal), on when that power's
     *  trait is, after KeeperFX's rules, the best worth for its price first. */
    void UsePersonality(const DaiPersonality &p, const IDaiPlanWorld &world, PlayerNumber player, bool final = false, int64_t spare = 0,
                        const DaiContent *content = nullptr);
    /** The human's combat-assistant rules (09 section 5.4, P7.12): their live cast and power-off rules, in their order,
     *  in both banks, with their globals (hand rules are an agent's, never a human's: left out). */
    void UseHuman(const DaiHumanFightRules &h);
    /** A bank's rules and globals (tests, logs). */
    const KeeperAssistRule *Rules(DaiFightBank b) const { return m_rules[(size_t)b].data(); }
    int64_t Count(DaiFightBank b) const { return m_count[(size_t)b]; }
    const KeeperAssistGlobals &Globals() const { return m_globals; }
private:
    void refresh_view();
    DaiFightRulesSource m_source = DaiFightRulesSource::Personality;
    DaiFightBank m_bank = DaiFightBank::Defensive;
    std::array<std::array<KeeperAssistRule, KEEPER_ASSIST_RULES>, (size_t)DaiFightBank::Count> m_rules{};
    std::array<int64_t, (size_t)DaiFightBank::Count> m_count{};
    KeeperAssistGlobals m_globals{};
    KeeperFightRules m_view{};
};

#endif
