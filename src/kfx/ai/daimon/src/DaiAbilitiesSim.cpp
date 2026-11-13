// What a kind's abilities are worth (DaiAbilities, P9), from the game's creature, instance, shot, spell, power and
// effect configs. Every number a damage-equivalent: a hit's damage (no hit worth more than [abilities] enemy_hp), an
// area's on the enemies its radius takes, a knock-back as the turns it keeps an enemy out of the fight, a disable or a
// buff as the damage it keeps from the creature (an enemy dealing enemy_damage in 100 turns), a heal as the health it
// gives back. Whole numbers only: the simulation's determinism.
#include "pre_inc.h"
#include "daimon/DaiAbilities.h"

#include <algorithm>

#include "config_creature.h"
#include "config_daimon.h"
#include "config_effects.h"
#include "config_magic.h"
#include "config_trapdoor.h"
#include "creature_instances.h"
#include "game_legacy.h"
#include "thing_effects.h"
#include "thing_shots.h"
#include "thing_stats.h"
#include "thing_traps.h"
#include "post_inc.h"

namespace {
const DaimonGlobalConfig &cfg() { return daimon_config.global; }

/** Enemies an area of radius r subtiles takes, DaiFix: 1 + area_density x pi (r / 3)^2, area_max at most. */
DaiFix area_targets(int64_t r)
{
    const DaiFix pi = 3217;                                        // 3.1416
    const DaiFix t = DAI_FIX_ONE + FixMul(FixMul(cfg().abilities_area_density, pi), FixFromInt(r * r) / 9);
    return std::min<DaiFix>(t, FixFromInt(cfg().abilities_area_max));
}

/** The damage an enemy deals in `turns` turns, kept from the creature at `share` (DaiFix). */
int64_t kept(int64_t turns, DaiFix share)
{
    return FixToInt(FixMul(FixFromInt(turns * cfg().abilities_enemy_damage / 100), share));
}

/** Knocked back at `blow` (255 full): the damage it keeps from each enemy taken. */
int64_t knock(int64_t blow)
{
    return std::min<int64_t>(blow, 255) * cfg().abilities_knock_turns * cfg().abilities_enemy_damage / 100 / 255;
}

int64_t spell_use(SpellKind sp, int64_t lv, int64_t strength, int64_t cycle);

/** A shot's worth at level lv (1-based; a creature of that strength). */
int64_t shot_use(DaiThingModel shot, int64_t lv, int64_t strength)
{
    const struct ShotConfigStats *s = get_shot_model_stats(shot);
    if (s == nullptr) return 0;
    const int64_t up = 100 + game.conf.crtr_conf.exp.spell_damage_increase_on_exp * (lv - 1);
    int64_t dmg = s->damage * up / 100;
    if ((s->model_flags & ShMF_StrengthBased) != 0) dmg = strength * ((s->damage != 0) ? s->damage : 100) / 100;
    dmg = std::min<int64_t>(dmg, cfg().abilities_enemy_hp);
    const bool beam = (s->fire_logic == ShFL_Beam), homing = (s->model_flags & ShMF_Navigable) != 0;
    const int64_t hit = (beam || homing || (s->speed == 0)) ? 100 : std::min<int64_t>(100, s->speed * 100 / 256);
    int64_t v = dmg * hit / 100;
    if (beam || ((s->model_flags & ShMF_Boulder) != 0)) v = FixToInt(FixMul(FixFromInt(dmg), cfg().abilities_beam_targets));
    if ((s->effect_amount > 1) && (s->fire_logic != ShFL_Breathe))                       // pellets (a hailstorm)
        v = FixToInt(FixMul(FixFromInt(dmg * s->effect_amount), cfg().abilities_hail_hit));
    if (s->fire_logic == ShFL_Breathe) v = dmg * std::max<int64_t>(s->health, 1) * 3 / 2;  // a stream over its life, a cone
    if (s->update_logic == ShUL_TrapLightning) {                                         // a strike every 16 turns of its life
        const struct ShotConfigStats *ball = get_shot_model_stats(ShM_GodLightBall);
        v = (s->health / 16) * ((ball != nullptr) ? ball->damage : 0);
    }
    if (s->area_range > 0) {
        const DaiFix t = area_targets(s->area_range);
        const int64_t area = FixToInt(FixMul(FixFromInt(std::min<int64_t>(s->area_damage * up / 100, cfg().abilities_enemy_hp)), cfg().abilities_area_falloff));
        v += FixToInt(FixMul(FixFromInt(area + knock(s->area_blow)), t));
    }
    if (s->hit_creature.effect_model > 0) {                                              // a gas cloud where it hits
        const struct EffectConfigStats *e = get_effect_model_stats(s->hit_creature.effect_model);
        if ((e != nullptr) && (e->area_affect_type == AAffT_GasDamage))
            v += FixToInt(FixMul(FixFromInt(std::min<int64_t>(120 * up / 100, cfg().abilities_enemy_hp)), area_targets(5)));
    }
    if (s->cast_spell_kind != 0) v += spell_use(s->cast_spell_kind, lv, strength, 0);
    return v;
}

/** A spell's worth at level lv, cast once a `cycle` turns (0: a shot's, its whole duration). */
int64_t spell_use(SpellKind sp, int64_t lv, int64_t strength, int64_t cycle)
{
    const struct SpellConfig *s = get_spell_config(sp);
    if (s == nullptr) return 0;
    const uint64_t f = s->spell_flags;
    const int64_t span = (cycle > 0) ? std::min<int64_t>(s->duration, cycle) : s->duration;
    struct Share { uint64_t flag; DaiFix share; };
    // A disable keeps the enemy's damage from the creature (a frozen enemy and a chicken all of it); a buff part of
    // what comes at it; a rebound turns the enemy's shots back onto another (kept and dealt: twice).
    static const Share kShares[] = {
        { CSAfF_Freeze, 1024 }, { CSAfF_Chicken, 1024 }, { CSAfF_Fear, 717 }, { CSAfF_Slow, 512 },
        { CSAfF_Armour, 338 }, { CSAfF_Invisibility, 512 }, { CSAfF_Speed, 307 }, { CSAfF_Flying, 102 },
    };
    if ((f & CSAfF_Rebound) != 0) return kept(span, 2 * cfg().abilities_ranged_share);
    for (const Share &sh : kShares) if ((f & sh.flag) != 0) return kept(span, sh.share);
    if ((f & CSAfF_Heal) != 0) {
        if (s->linked_power == PwrK_None) return s->healing_recovery;
        const struct PowerConfigStats *p = get_power_model_stats(s->linked_power);
        return (p != nullptr) ? p->strength[std::clamp<int64_t>(lv - 1, 0, MAGIC_OVERCHARGE_LEVELS)] : 0;
    }
    if (s->cast_effect_model > 0) {
        const struct EffectConfigStats *e = get_effect_model_stats(s->cast_effect_model);
        if ((e != nullptr) && (e->area_affect_type == AAffT_GasDamage))                  // a damaging gas (a bile demon's)
            return FixToInt(FixMul(FixFromInt(std::min<int64_t>(120, cfg().abilities_enemy_hp)), area_targets(5)));
        if ((e != nullptr) && ((e->area_affect_type == AAffT_GasDamageEffect) || (e->area_affect_type == AAffT_GasEffect)))
            return FixToInt(FixMul(FixFromInt(kept(s->duration, 512)), area_targets(5)));
    }
    if (s->shot_model > 0) {
        const struct ShotConfigStats *sh = get_shot_model_stats(s->shot_model);
        // Wind: no hit, no damage -- every enemy near blown away (and a gas with it).
        if ((sh != nullptr) && ((sh->model_flags & ShMF_NoHit) != 0) && (sh->damage == 0) && (sh->area_range == 0))
            return FixToInt(FixMul(FixFromInt(knock(255)), area_targets(9)));
        return shot_use(s->shot_model, lv, strength);
    }
    return 0;
}
}

int64_t DaiAbilitySpecsOfKind(DaiThingModel m, DaiAbilitySpec *out, int64_t max)
{
    const struct CreatureModelConfig *c = creature_stats_get(m);
    if (creature_stats_invalid(c)) return 0;
    int64_t n = 0;
    for (int64_t i = 0; (i < LEARNED_INSTANCES_COUNT) && (n < max); i++) {
        const CrInstance inst = c->learned_instance_id[i];
        const int64_t learnt = c->learned_instance_level[i];
        if ((inst <= 0) || (learnt < 1) || (learnt > DAI_ABILITY_LEVELS)) continue;
        const struct InstanceInfo *ii = creature_instance_info_get(inst);
        DaiAbilitySpec &a = out[n];
        a = DaiAbilitySpec();
        a.level = learnt;
        a.time = std::max<int64_t>(ii->time, 1);
        a.cycle = a.time + std::max<int64_t>(ii->reset_time, 0);
        const int64_t fire_shot = 3, cast_spell = 2;                // creature_instances_func_type[] (config_funcnames.c)
        bool any = false;
        if ((ii->func_idx == cast_spell) && (ii->func_params[0] > 0)) {
            const struct SpellConfig *s = get_spell_config((SpellKind)ii->func_params[0]);
            if ((s != nullptr) && ((s->spell_flags & (CSAfF_Teleport | CSAfF_Sight)) != 0)) {
                a.flat = true;
                const int64_t w = ((s->spell_flags & CSAfF_Teleport) != 0) ? cfg().abilities_teleport_value : cfg().abilities_sight_value;
                for (int64_t lv = 0; lv < DAI_ABILITY_LEVELS; lv++) a.worth[lv] = w;
                n++;
                continue;
            }
        }
        DaiThingModel shot_model = 0;
        if ((ii->func_idx == fire_shot) && (ii->func_params[0] > 0)) shot_model = (DaiThingModel)ii->func_params[0];
        else if ((ii->func_idx == cast_spell) && (ii->func_params[0] > 0)) {
            const struct SpellConfig *s = get_spell_config((SpellKind)ii->func_params[0]);
            if (s != nullptr) shot_model = s->shot_model;
        }
        if (shot_model > 0) {
            const struct ShotConfigStats *st = get_shot_model_stats(shot_model);
            a.shot = true;
            a.magical = (st != nullptr) && (st->is_magical != 0);
        }
        for (int64_t lv = 1; lv <= DAI_ABILITY_LEVELS; lv++) {
            const int64_t strength = compute_creature_max_strength(c->strength, (CrtrExpLevel)(lv - 1));
            int64_t w = 0;
            if ((ii->func_idx == fire_shot) && (ii->func_params[0] > 0)) w = shot_use((DaiThingModel)ii->func_params[0], lv, strength);
            else if ((ii->func_idx == cast_spell) && (ii->func_params[0] > 0)) w = spell_use((SpellKind)ii->func_params[0], lv, strength, a.cycle);
            a.worth[lv - 1] = std::max<int64_t>(w, 0);
            any = any || (w > 0);
        }
        if (any) n++;
    }
    return n;
}

int64_t DaiTrapRating(DaiThingModel trap)
{
    const struct TrapConfigStats *t = get_trap_model_stats(trap);
    if (t == nullptr) return 0;
    int64_t shot = 0;
    if ((t->activation_type == TrpAcT_EffectonTrap) && (t->created_itm_model > 0)) {
        const struct EffectConfigStats *e = get_effect_model_stats(t->created_itm_model);
        if ((e != nullptr) && (e->area_affect_type == AAffT_GasDamage))
            shot = FixToInt(FixMul(FixFromInt(std::min<int64_t>(120, cfg().abilities_enemy_hp)), area_targets(5)));
    } else if (((t->activation_type == TrpAcT_ShotonTrap) || (t->activation_type == DaiTrapAct_HeadforTarget90)
                || (t->activation_type == TrpAcT_CreatureShot)) && (t->created_itm_model > 0)) {
        shot = shot_use((DaiThingModel)t->created_itm_model, 1, 0);
    }
    // In 100 turns while triggered: a shot each shots_delay turns (a one-shot trap: its shot).
    if (t->shots_delay <= 1) return shot;
    int64_t rate = shot * 100 / t->shots_delay;
    // Only while its charges last within a fight (an empty trap waits for a new crate); 255 never run out.
    const int64_t fight = std::max<int64_t>(cfg().abilities_trap_fight_turns, 1);
    if ((t->shots > 0) && (t->shots != INFINITE_CHARGES) && (t->shots * t->shots_delay < fight))
        rate = rate * t->shots * t->shots_delay / fight;
    return rate;
}
