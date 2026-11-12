#include "pre_inc.h"
#include "daimon/DaiContent.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "bflib_basics.h"
#include "config_daimon.h"
#include "daimon/DaiLog.h"
#include "post_inc.h"

namespace {
constexpr const char *ROLE_NAMES[] = { "ignore", "summon", "strike", "disable", "boost", "heal", "utility", "pressure",
                                       "alarm", "boulder", "terrain", "blocker", "door", "secret", "midas" };
static_assert(sizeof(ROLE_NAMES) / sizeof(ROLE_NAMES[0]) == (size_t)DaiContentRole::Count, "a name for every role");
constexpr const char *CLASS_NAMES[] = { "power", "trap", "door" };

// KeeperFX 1.4's own kinds (core fxdata magic.cfg, trapdoor.cfg): the managers' named paths handle these as before.
constexpr const char *BASE_POWERS[] = { "POWER_HAND", "POWER_IMP", "POWER_OBEY", "POWER_SLAP", "POWER_SIGHT",
    "POWER_CALL_TO_ARMS", "POWER_CAVE_IN", "POWER_HEAL_CREATURE", "POWER_HOLD_AUDIENCE", "POWER_LIGHTNING",
    "POWER_SPEED", "POWER_PROTECT", "POWER_CONCEAL", "POWER_DISEASE", "POWER_CHICKEN", "POWER_DESTROY_WALLS",
    "POWER_TIME_BOMB", "POWER_POSSESS", "POWER_ARMAGEDDON", "POWER_PICKUP_CREATURE", "POWER_PICKUP_GOLD",
    "POWER_PICKUP_FOOD", "POWER_REBOUND", "POWER_FREEZE", "POWER_SLOW", "POWER_FLIGHT", "POWER_VISION",
    "POWER_TUNNELLER", "POWER_PICKUP_OBJECT", nullptr };
constexpr const char *BASE_TRAPS[] = { "BOULDER", "ALARM", "POISON_GAS", "LIGHTNING", "WORD_OF_POWER", "LAVA", "TNT",
    "SENTRY", "BALLISTA", nullptr };
constexpr const char *BASE_DOORS[] = { "WOOD", "BRACED", "STEEL", "MAGIC", "SECRET", "MIDAS", nullptr };

void copy_name(char (&dst)[32], const char *src)
{
    snprintf(dst, sizeof(dst), "%s", (src != nullptr) ? src : "");
}

const char *class_key(DaiContentClass c) { return CLASS_NAMES[(size_t)c]; }
}

const char *DaiContentRoleName(DaiContentRole r)
{
    return (r < DaiContentRole::Count) ? ROLE_NAMES[(size_t)r] : "?";
}

DaiContentRole DaiContentRoleNamed(const char *name)
{
    if (name == nullptr) return DaiContentRole::Count;
    for (size_t i = 0; i < (size_t)DaiContentRole::Count; i++) if (strcmp(ROLE_NAMES[i], name) == 0) return (DaiContentRole)i;
    return DaiContentRole::Count;
}

const char *DaiContentClassName(DaiContentClass c)
{
    return (c < DaiContentClass::Count) ? CLASS_NAMES[(size_t)c] : "?";
}

bool DaiContent::IsBaseName(DaiContentClass c, const char *name)
{
    const char *const *list = (c == DaiContentClass::Power) ? BASE_POWERS : (c == DaiContentClass::Trap) ? BASE_TRAPS : BASE_DOORS;
    for (; *list != nullptr; list++) if (strcmp(*list, name) == 0) return true;
    return false;
}

DaiContentRole DaiContent::InferPower(const DaiPowerSpec &s)
{
    if (!s.on_panel) return DaiContentRole::Ignore;
    switch (s.func) {
    case DaiPowerFunc::Lightning: return DaiContentRole::Strike;
    case DaiPowerFunc::Tunneller: return (s.creature > 0) ? DaiContentRole::Summon : DaiContentRole::Ignore;
    case DaiPowerFunc::ApplySpell:
        if (s.creature > 0) return DaiContentRole::Summon;
        if (s.spell_heals || (s.spell_heal > 0)) return DaiContentRole::Heal;
        if (s.spell_hostile && s.enemy_creatures) return DaiContentRole::Disable;
        if (s.spell_friendly && s.own_creatures) return DaiContentRole::Boost;
        if ((s.spell_damage > 0) || s.spell_shot) return DaiContentRole::Strike;
        return DaiContentRole::Utility;
    case DaiPowerFunc::None: case DaiPowerFunc::Other: return DaiContentRole::Ignore;
    default: return DaiContentRole::Utility;
    }
}

DaiContentRole DaiContent::InferTrap(const DaiTrapSpec &s)
{
    if (!s.on_panel) return DaiContentRole::Ignore;
    if (s.activation == DaiTrapAct_SlabChange) return DaiContentRole::Terrain;
    if (s.activation == DaiTrapAct_HeadforTarget90) return DaiContentRole::Boulder;
    if (s.shots <= 0) return ((s.health > 0) && s.destructible) ? DaiContentRole::Blocker : DaiContentRole::Ignore;
    if (s.rating > 0) return DaiContentRole::Pressure;
    if (s.activation == DaiTrapAct_None) return DaiContentRole::Alarm;
    return DaiContentRole::Ignore;
}

DaiContentRole DaiContent::InferDoor(const DaiDoorSpec &s)
{
    if (!s.on_panel) return DaiContentRole::Ignore;
    if (s.secret) return DaiContentRole::Secret;
    if (s.midas) return DaiContentRole::Midas;
    return DaiContentRole::Door;
}

std::vector<DaiContentOverride> DaiContent::ReadOverrides()
{
    std::vector<DaiContentOverride> out;
    if (!daimon_config.loaded) return out;
    const VALUE *content = value_dict_get(&daimon_config.root, "content");
    if ((content == nullptr) || (value_type(content) != VALUE_DICT)) return out;
    for (size_t c = 0; c < (size_t)DaiContentClass::Count; c++) {
        const VALUE *cls = value_dict_get(content, class_key((DaiContentClass)c));
        if ((cls == nullptr) || (value_type(cls) != VALUE_DICT)) continue;
        const VALUE *names[256];
        const size_t n = value_dict_keys_sorted(cls, names, std::min<size_t>(value_dict_size(cls), 256));
        for (size_t i = 0; i < n; i++) {
            const char *name = value_string(names[i]);
            const VALUE *e = value_dict_get(cls, name);
            if ((e == nullptr) || (value_type(e) != VALUE_DICT)) continue;
            DaiContentOverride o;
            o.cls = (DaiContentClass)c;
            copy_name(o.name, name);
            const VALUE *role = value_dict_get(e, "role");
            if ((role != nullptr) && (value_type(role) == VALUE_STRING)) {
                o.role = DaiContentRoleNamed(value_string(role));
                if (o.role == DaiContentRole::Count)
                    DaiLog::Warn(-1, "daimon.toml: [content.%s.%s] role \"%s\" is not a role; the inferred one kept",
                                 class_key(o.cls), name, value_string(role));   // (-1: no keeper, the file)
            }
            const VALUE *worth = value_dict_get(e, "worth");
            if ((worth != nullptr) && value_is_compatible(worth, VALUE_INT64)) o.worth = std::max<int64_t>(value_int64(worth), 0);
            out.push_back(o);
        }
    }
    return out;
}

void DaiContent::Build(const std::vector<DaiPowerSpec> &powers, const std::vector<DaiTrapSpec> &traps,
                       const std::vector<DaiDoorSpec> &doors, const std::vector<DaiContentOverride> &overrides)
{
    for (auto &v : m_entries) v.clear();
    auto add = [&](DaiContentClass c, int64_t kind, const char *name, DaiContentRole role, int64_t worth) -> DaiContentEntry & {
        DaiContentEntry e;
        e.cls = c;
        e.kind = kind;
        copy_name(e.name, name);
        e.base = IsBaseName(c, e.name);
        e.role = role;
        e.worth = worth;
        m_entries[(size_t)c].push_back(e);
        return m_entries[(size_t)c].back();
    };
    for (const DaiPowerSpec &s : powers) {
        DaiContentEntry &e = add(DaiContentClass::Power, s.kind, s.name, InferPower(s), s.strength);
        e.cost = s.cost;
        e.creature = s.creature;
        e.creature_level = s.creature_level;
    }
    for (const DaiTrapSpec &s : traps) {
        // A blocker does no harm: its worth is the health it stands with.
        const DaiContentRole role = InferTrap(s);
        DaiContentEntry &e = add(DaiContentClass::Trap, s.kind, s.name, role, (role == DaiContentRole::Blocker) ? s.health : s.rating);
        e.gas = s.gas;
        e.health = s.health;
    }
    for (const DaiDoorSpec &s : doors) {
        DaiContentEntry &e = add(DaiContentClass::Door, s.kind, s.name, InferDoor(s), s.health);
        e.health = s.health;
        e.secret = s.secret;
        e.midas = s.midas;
        e.resist_non_magic = s.resist_non_magic;
    }
    for (const DaiContentOverride &o : overrides) {
        DaiContentEntry *e = nullptr;
        for (DaiContentEntry &x : m_entries[(size_t)o.cls]) if (strcmp(x.name, o.name) == 0) e = &x;
        if (e == nullptr) continue;             // a kind this level does not have: nothing to set
        if (o.role != DaiContentRole::Count) { e->role = o.role; e->overridden = true; }
        if (o.worth >= 0) { e->worth = o.worth; e->overridden = true; }
    }
    m_built = true;
}

const DaiContentEntry *DaiContent::Find(DaiContentClass c, int64_t kind) const
{
    for (const DaiContentEntry &e : m_entries[(size_t)c]) if (e.kind == kind) return &e;
    return nullptr;
}

const DaiContentEntry *DaiContent::FindNamed(DaiContentClass c, const char *name) const
{
    for (const DaiContentEntry &e : m_entries[(size_t)c]) if (strcmp(e.name, name) == 0) return &e;
    return nullptr;
}

std::vector<const DaiContentEntry *> DaiContent::OfRole(DaiContentClass c, DaiContentRole r) const
{
    std::vector<const DaiContentEntry *> out;
    for (const DaiContentEntry &e : m_entries[(size_t)c]) if (e.role == r) out.push_back(&e);
    return out;
}

void DaiContent::Log(PlayerNumber plyr) const
{
    for (size_t c = 0; c < (size_t)DaiContentClass::Count; c++) {
        for (const DaiContentEntry &e : m_entries[c]) {
            if (e.base && !e.overridden) continue;
            char extra[64] = "";
            if (e.creature > 0) snprintf(extra, sizeof(extra), ", creature %d level %d", (int)e.creature, (int)e.creature_level);
            DaiLog::Info(plyr, "content: %s %s (kind %d) %s, worth %d%s (%s)", class_key((DaiContentClass)c), e.name,
                         (int)e.kind, DaiContentRoleName(e.role), (int)e.worth, extra, e.overridden ? "daimon.toml" : "inferred");
        }
    }
}
