#include "pre_inc.h"
#include "daimon/DaiModeAssistant.h"
#include "daimon/DaiGeom.h"

#include <algorithm>
#include <cstring>

#include "config_daimon.h"
#include "daimon/DaiKnowledge.h"
#include "daimon/DaiLeases.h"
#include "daimon/DaiTurnCtx.h"
#include "post_inc.h"

uint32_t DaiModeAssistant::DomainBit(DaiDomain d)
{
    switch (d) {
    case DaiDomain::Economy: return DAI_ASSIST_ECONOMY;
    case DaiDomain::Excavation: return DAI_ASSIST_EXCAVATION;
    case DaiDomain::Construction: return DAI_ASSIST_CONSTRUCTION;
    case DaiDomain::Workforce: return DAI_ASSIST_WORKFORCE;
    case DaiDomain::Creatures: return DAI_ASSIST_CREATURES;
    case DaiDomain::Custody: return DAI_ASSIST_CUSTODY;
    case DaiDomain::Defence: return DAI_ASSIST_DEFENCE;
    case DaiDomain::Offence: return DAI_ASSIST_OFFENCE;
    default: return 0;
    }
}

bool DaiModeAssistant::DomainActiveFor(uint32_t domains, DaiDomain d)
{
    // Tactics fights for defence (and offence); Magic serves whichever manager asked (its jobs run for any).
    if (d == DaiDomain::Tactics) return (domains & (DAI_ASSIST_DEFENCE | DAI_ASSIST_OFFENCE)) != 0;
    if (d == DaiDomain::Magic) return domains != 0;
    return (domains & DomainBit(d)) != 0;
}

bool DaiModeAssistant::DomainActive(DaiDomain d) const
{
    return DomainActiveFor(m_domains, d);
}

bool DaiModeAssistant::Preset(const char *name, uint32_t *domains, const char **personality)
{
    if (name == nullptr) return false;
    for (const DaiAssistPresetDef &d : dai_assist_presets) {
        if (strcmp(d.name, name) != 0) continue;
        *domains = d.domains;
        *personality = d.personality;
        return true;
    }
    return false;
}

namespace {
/** Every slab of a verb's rectangle (inclusive, either corner first). */
template <typename F> bool any_slab(const KeeperVerb &v, F f)
{
    const int64_t x0 = std::min(v.slab_x0, v.slab_x1), x1 = std::max(v.slab_x0, v.slab_x1);
    const int64_t y0 = std::min(v.slab_y0, v.slab_y1), y1 = std::max(v.slab_y0, v.slab_y1);
    for (int64_t y = y0; y <= y1; y++)
        for (int64_t x = x0; x <= x1; x++)
            if (f((DaiSlabCoord)x, (DaiSlabCoord)y)) return true;
    return false;
}
}

const char *DaiModeAssistant::RefusesFor(const DaiTurnCtx &c, const KeeperVerb &v, int64_t lease, bool agent)
{
    const DaiLiveExclusion &live = c.know.LiveExclusion();
    auto thing_refused = [&](int64_t t) -> const char * {
        if (t <= 0) return nullptr;
        if (live.Excludes((DaiThingIndex)t)) return "EXCLUDED_BY_PLAYER";
        if (c.leases != nullptr) {
            // The creature's own creation turn, from the keeper's knowledge (0 when not one of its own: leased as known).
            const DaiCreatureLook *cr = c.know.CreatureByIdx((DaiThingIndex)t);
            const DaiThingRef ref = (cr != nullptr) ? cr->ref : DaiThingRef{ (DaiThingIndex)t, 0 };
            if (c.leases->CreatureLeased(ref, c.now, lease)) return agent ? "LEASED_BY_AGENT" : "LEASED_BY_PLAYER";
        }
        return nullptr;
    };
    // The things it would touch.
    if (v.has_thing) {
        if (const char *why = thing_refused(v.thing_id)) return why;
    }
    for (int64_t i = 0; i < v.thing_count; i++) {
        if (const char *why = thing_refused(v.thing_ids[i])) return why;
    }
    // A cast or a drop by the creature the human possesses.
    if (v.has_pos && ((v.kind == ESV_CastPower) || (v.kind == ESV_Drop) || (v.kind == ESV_PickUpAndDrop))
        && live.NearPossessed((DaiSubtlCoord)v.stl_x, (DaiSubtlCoord)v.stl_y)) return "NEAR_POSSESSED";
    if (c.leases == nullptr) return nullptr;
    const DaiLeases &l = *c.leases;
    // The slab decisions the human made stand.
    switch (v.kind) {
    case ESV_MarkDig:
        if (v.has_rect && any_slab(v, [&](DaiSlabCoord x, DaiSlabCoord y) { return !l.MayTag(x, y); })) return agent ? "AGENT_UNMARKED" : "HUMAN_UNMARKED";
        break;
    case ESV_UnmarkDig:
        if (v.has_rect && any_slab(v, [&](DaiSlabCoord x, DaiSlabCoord y) { return !l.MayUntag(x, y); })) return agent ? "AGENT_MARKED" : "HUMAN_MARKED";
        break;
    case ESV_BuildRoom:
        if (v.has_rect && any_slab(v, [&](DaiSlabCoord x, DaiSlabCoord y) { return !l.MayBuild(x, y); })) return agent ? "AGENT_SOLD" : "HUMAN_SOLD";
        break;
    case ESV_Sell:
        if (v.has_rect && any_slab(v, [&](DaiSlabCoord x, DaiSlabCoord y) {
                return !l.MaySellRoom(x, y) || !l.MaySellTrap(x, y) || !l.MaySellDoor(x, y);
            })) return agent ? "AGENT_BUILT" : "HUMAN_BUILT";
        if (v.has_pos) {
            const DaiSlabCoord x = DaiSubtileSlab((DaiSubtlCoord)v.stl_x), y = DaiSubtileSlab((DaiSubtlCoord)v.stl_y);
            if (!l.MaySellRoom(x, y) || !l.MaySellTrap(x, y) || !l.MaySellDoor(x, y)) return agent ? "AGENT_BUILT" : "HUMAN_BUILT";
        }
        break;
    case ESV_PlaceTrap:
        if (v.has_pos && !l.MayPlaceTrap(DaiSubtileSlab((DaiSubtlCoord)v.stl_x), DaiSubtileSlab((DaiSubtlCoord)v.stl_y))) return agent ? "AGENT_SOLD" : "HUMAN_SOLD";
        break;
    case ESV_PlaceDoor:
        if (v.has_pos && !l.MayPlaceDoor(DaiSubtileSlab((DaiSubtlCoord)v.stl_x), DaiSubtileSlab((DaiSubtlCoord)v.stl_y))) return agent ? "AGENT_SOLD" : "HUMAN_SOLD";
        break;
    default:
        break;
    }
    return nullptr;
}

const char *DaiModeAssistant::Refuses(const DaiTurnCtx &c, const KeeperVerb &v) const
{
    return RefusesFor(c, v, c.cfg.assistant_lease_turns, false);
}

DaiFightCasting DaiModeAssistant::CastingFor(uint32_t domains, bool human_on, bool delegated)
{
    if ((domains & DAI_ASSIST_DEFENCE) == 0) return DaiFightCasting::None;
    if (!human_on) return DaiFightCasting::Personality;
    return delegated ? DaiFightCasting::Human : DaiFightCasting::None;
}

DaiFightCasting DaiModeAssistant::FightCasting(const DaiTurnCtx &c) const
{
    const DaiHumanFightRules &h = c.know.HumanFightRules();
    return CastingFor(m_domains, h.enabled, h.delegated);
}
