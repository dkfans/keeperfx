/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_daimon.h
 *     Daimon's configuration (fxdata/daimon.toml): the global sections parsed, the personalities kept as parsed
 *     tables for kfx_daimon's DaiPersonality::Resolve (docs/plans/ai-plans/v2/02-library-architecture.md 4.14).
 * @par Purpose:
 *     Loaded like any fxdata config (mods, the campaign and the level may override keys); held outside the saved
 *     config state -- a Daimon keeper saves its resolved values, not the file's.
 */
/******************************************************************************/
#ifndef DK_CONFIG_DAIMON_H
#define DK_CONFIG_DAIMON_H

#include "globals.h"
#include <value.h>

#ifdef __cplusplus
extern "C" {
#endif

/* (config.h's; declared here so Daimon's core, which reads these types, needs no more of the config layer: the
   port surface, scripts/check_daimon_modules.py) */
struct ConfigFileData;

/** The keeper-facts kinds (kfx_ai's KF_Count, checked in DaiInbox.cpp): [inbox] keeps an expiry for each. */
#define DAIMON_INBOX_KINDS 17
/** Daimon's arbiter domains (DaiDomain::Count, checked in DaiArbiter.cpp): [arbiter] keeps a weight for each. */
#define DAIMON_DOMAINS 10

/** The global sections, as resolved (defaulted, clamped). */
struct DaimonGlobalConfig {
    int64_t budgets_slab_scan;
    int64_t budgets_reach_visits;
    int64_t economy_reserve_window;
    int64_t budgets_search_nodes;
    int64_t dig_floor, dig_earth, dig_gold_route, dig_dense_route, dig_own_wall, dig_enemy_floor, dig_bridge_base,
            dig_abyss_edge;
    int64_t dig_heuristic_weight;               /**< x1024 */
    int64_t dig_creep_cost, dig_door_cost_per_hp, dig_destroy_walls_cost;  /**< the Attack model (07 4.2); per hp x1024 */
    int64_t budgets_max_searches;
    int64_t excavation_marks_ahead, excavation_mine_marks;
    int64_t excavation_replan_ratio;            /**< x1024 */
    int64_t excavation_rescore_interval, excavation_claim_window,
            excavation_unreachable_retry, excavation_max_veins;
    int64_t excavation_marks_share;             /**< x1024 */
    int64_t workforce_interval;
    int64_t budgets_layout_candidates;
    int64_t layout_radius, layout_rim;
    int64_t programme_interval, programme_lair_spare, programme_train_runway;
    int64_t layout_cache_turns, layout_grow_at;  /**< grow_at x1024 */
    int64_t construction_room_build_concurrency;
    int64_t creatures_moves_per_kturn, creatures_livid_grace;
    /** Scavenging (08 section 7a): scavengers at most; how often the kinds are weighed; the pool draw tried this long
     *  with nothing come, then the kind marked dry this long (a human finds out the same way, Q8). */
    int64_t creatures_scavengers_max, creatures_scavenge_interval, creatures_pool_try_turns, creatures_pool_dry_turns;
    int64_t director_interval, director_switch_margin, director_build_up_max_turns, director_contest_radius,
            director_contest_forget, director_recover_window, director_attack_rest;
    int64_t pace_relaxed, pace_human, pace_unlimited;   /**< world gestures a 1000 turns at each pace (05 section 7.2) */
    int64_t cheats_rich_start_gold, cheats_creature_bonus_pct, cheats_pay_discount_pct;
    /** The threat model (06 section 3, Phase 4). */
    int64_t threat_interval, threat_halflife, threat_speed_per_slab, threat_check_interval, threat_route_interval,
            threat_search_nodes, threat_destroy_walls, threat_frontier_weight, threat_hero_weight, threat_max_chokepoints;
    int64_t threat_own_door_per_hp;             /**< x1024 */
    /** Tactics (06 section 5.6, 10). */
    int64_t tactics_fight_tick, tactics_idle_tick, tactics_burst_turns, tactics_contact_radius, tactics_match_margin;
    /** Defence (06 sections 4, 5.2, 6). */
    int64_t defence_interval, defence_area_threat, defence_imps_threat, defence_lock_radius, defence_max_traps, defence_max_doors,
            defence_intercept_turns, defence_defend_cooldown, defence_max_guard_posts, defence_threat_force;
    /** P5.13 (07): hazards -- a neutral's or the heroes' trap by which hazard_imps of the keeper's imps were lost
     *  (within hazard_reach slabs) in hazard_window turns; the imp rebuy brake; ward casts below ward_share of an imp; a
     *  creature spends one if its force is no more than the trap's rating (the imps it took, counted more while gold is
     *  short of spend_gold_imps imps' price). Below hazard_imps_floor imps no brake holds: an imp is bought with any gold. */
    int64_t defence_hazard_imps, defence_hazard_window, defence_hazard_reach, defence_hazard_rebuy_turns, defence_ward_share;
    int64_t defence_hazard_imps_floor;
    int64_t defence_spend_gold_imps;
    /** P9 (plan 13 L3): a guard at the bridge landing stands hold_turns turns before it goes back to work. */
    int64_t defence_hold_turns;
    /** Custody (08 sections 4-6, 17): prisoners re-planned every plan_interval turns (and on a custody fact); the
     *  imprison tendency on while a prison has prison_spare places free; a victim kept alive healed below
     *  heal_victim_below % of its health; an interrogation worth interrogate_value (force score) against a keeper whose
     *  heart is unknown; holding a prisoner worth hold_share (DaiFix) of its force. */
    int64_t custody_plan_interval, custody_prison_spare, custody_heal_victim_below, custody_interrogate_value, custody_hold_share;
    /** The temple's outcomes on the same scale (08 section 7.2): research or manufacture completed, all made happy (an
     *  angry creature each), a blessing (a creature each); the imp price's step a sacrificed imp takes off, and the
     *  turns ahead (imp_horizon) the imps it would make cheaper are counted over, at the rate imps were made lately. */
    int64_t custody_research_value, custody_manufacture_value, custody_happy_value, custody_bless_value, custody_imp_price_step;
    int64_t custody_imp_horizon;
    /** A creature's abilities' worth (DaiAbilities, P9), as damage-equivalents: an area of radius r subtiles hits
     *  1 + area_density (DaiFix) x pi (r / 3)^2 enemies, area_max at most, area_falloff (DaiFix) of its damage each; a beam or
     *  a boulder beam_targets (DaiFix); no hit worth more than enemy_hp; a hailstorm's pellets hail_hit (DaiFix) of them;
     *  knocked back at full strength, knock_turns out of the fight; an enemy deals enemy_damage in 100 turns (what a
     *  disable or a buff keeps from it), ranged_share (DaiFix) of it in shots a rebound turns back; travel over the
     *  keeper's ground (teleport) and sight (the counter to conceal and invisibility) teleport_value and sight_value in
     *  100 turns. */
    int64_t abilities_area_density, abilities_area_max, abilities_area_falloff, abilities_beam_targets, abilities_enemy_hp,
            abilities_hail_hit, abilities_knock_turns, abilities_enemy_damage, abilities_ranged_share,
            abilities_teleport_value, abilities_sight_value;
    /** A trap's worth while triggered holds only while its charges last within a fight of trap_fight_turns turns
     *  (Shots x TimeBetweenShots; 255 shots never run out; a one-shot trap, boulder or TNT, is its shot): an empty trap
     *  waits for a new crate. */
    int64_t abilities_trap_fight_turns;
    /** The human's assistant (09 section 5.2a): no cast or drop of the assistant's within possession_radius subtiles of
     *  the creature the human possesses (the gesture packets refuse it whoever planned it). */
    int64_t assistant_possession_radius;
    /** A creature the human touched is theirs for lease_turns (09 section 5.3); their slab decisions stand for good. */
    int64_t assistant_lease_turns;
    int64_t lieutenant_agent_lease_turns;   /**< [lieutenant] agent_lease_turns: a creature the agent touched is its own */
    /** A kind reborn at its lair when killed (the config's Rebirth): its output worth rebirth_value (DaiFix) more from
     *  that level; scavenging by an alluring kind (ALLURING_SCVNGR) with nothing of its kind to lure, worth
     *  allure_value (DaiFix) of a level-1 creature from the pool a lure. */
    int64_t abilities_rebirth_value, abilities_allure_value;
    /** A graveyard once this many bodies have lain on its ground at once (08 section 9). */
    int64_t custody_graveyard_bodies;
    /** A jailbreak (08 section 10): for a creature of its own held by another keeper worth jailbreak_min_force (force x the
     *  personality's rescue_value); given up after jailbreak_timeout turns, not tried again for jailbreak_retry. */
    int64_t custody_jailbreak_min_force, custody_jailbreak_timeout, custody_jailbreak_retry;
    int64_t offence_interval, offence_route_interval, offence_search_nodes, offence_hate_halflife, offence_attack_runway,
            offence_defence_radius;
    int64_t offence_op_interval, offence_recon_timeout, offence_stage_timeout, offence_breach_timeout, offence_assault_timeout,
            offence_withdraw_turns, offence_retry_after, offence_sight_level, offence_max_refusals;
    int64_t offence_gather_turns, offence_gather_share, offence_waves_per_kturn;
    int64_t offence_raid_interval, offence_raid_timeout, offence_harass_turns, offence_raid_border, offence_creep_imps, offence_exploit_turns;
    /** P9 (plan 13 L2): with the force estimate favourable, a raid looks this far from the keeper's land. */
    int64_t offence_raid_reach;
    int64_t diplomacy_interval, diplomacy_ally_dwell, diplomacy_peace_after_war;
    int64_t inbox_expiry[DAIMON_INBOX_KINDS];   /**< by enum KeeperFactKind; [0] (KF_None) unused */
    int64_t arbiter_quota_window;
    int64_t arbiter_age_bonus_per_kturn;
    int64_t arbiter_weight[DAIMON_DOMAINS];     /**< by DaiDomain */
    int64_t jobs_stall_window;
    int64_t net_replicate_interval;
    int64_t threads_workers;
    int64_t threads_reserved_cores;
    int64_t threads_jitter_us;
    int64_t threads_base_turns_per_second;
};

/** A global key: where it is in the file, its field, range and default, and the class that reads it. A key with a
 *  `scale` above 1 is fixed point: it takes a decimal in the file, stored times the scale (rounded once at load), and
 *  its default and range are in the stored units. */
struct DaimonGlobalKey {
    const char *section;
    const char *key;
    size_t offset;           /**< into struct DaimonGlobalConfig */
    int64_t dflt, min, max;
    const char *reader;
    int64_t scale;           /**< 0 or 1: a whole number */
};

struct DaimonConfig {
    struct DaimonGlobalConfig global;
    /** The whole file as loaded, overrides merged in: what the personalities resolve from. */
    VALUE root;
    TbBool loaded;
};

/** A personality as the frontend lists it (05 section 4.1's header keys; none of them inherited). Strings point into
 *  the loaded file, valid until it is reloaded. */
struct DaimonPersonalityInfo {
    const char *id;                 /**< the [personality.<id>] name: what saves and scripts use */
    const char *name;               /**< `name`, or the id when it has none */
    int64_t name_text_id;           /**< `name_text_id`: a translated display name (0: none, use `name`) */
    const char *description;        /**< `description`, or "" */
    TbBool skirmish_pool;           /**< `skirmish_pool`: Skirmish's "random" may pick it (false when absent) */
};

/** The keys of a personality's own table that are not module tables: its header and `inherits`. NULL-terminated. */
extern const char *const daimon_personality_header_keys[];

extern struct DaimonConfig daimon_config;
extern const struct ConfigFileData daimon_file_data;
/** Every global key the loader knows, ending with a NULL section. */
extern const struct DaimonGlobalKey daimon_global_keys[];

/** Loads fxdata/daimon.toml and its overrides into daimon_config (defaults for what is missing). */
TbBool daimon_config_load(void);
/** Loads one file the way load_config() does each: `partial` merges into what is loaded; otherwise replaces it.
 *  Then resolves the global sections. For tests and daimon_file_data. */
TbBool daimon_config_load_file(const char *fname, TbBool partial);
/** Forgets everything loaded (the defaults stay). */
void daimon_config_clear(void);
/** The [personality.<name>] table, or NULL. */
const VALUE *daimon_config_personality(const char *name);
/** daimon.toml's [model_map]: the personality a built-in AI model (COMPUTER_PLAYER's 0-16) plays as under a level's
 *  DAIMON = MODELS key; NULL for a model it does not name (Daimon's default then). */
const char *daimon_config_model_personality(int64_t model);
/** The personalities, "default" first and the rest sorted by id: how many were written to `out` (at most `max`). */
size_t daimon_config_personality_list(struct DaimonPersonalityInfo *out, size_t max);
/** The most personalities a list holds (the screens, PckA_AssistDomains' number). */
#define DAIMON_MAX_PERSONALITIES 64
/** A personality's number in daimon_config_personality_list, 1-based (0: none of that id), and the id of a number
 *  (NULL: none) -- PckA_AssistDomains' par4, both ends (14 S5). */
int32_t daimon_config_personality_number(const char *id);
const char *daimon_config_personality_by_number(int64_t number);
/** What a player sees for a personality: its translated name when it has a name_text_id, else its name. */
const char *daimon_personality_display_name(const struct DaimonPersonalityInfo *info);
/** After every layer has loaded: the keys of the global sections nothing reads, logged and put in the compat report
 *  (unknown keys: what a newer daimon.toml may hold); a missing [personality.default] logged (the built-in values
 *  stand in for it); then kfx_daimon checks the personalities' and difficulties' keys (daimon_on_config_loaded). */
void daimon_config_check(void);
/** The difficulty levels the setup screens offer: easy, normal and hard (built in), then the file's other
 *  [difficulty.<id>] tables, sorted. Pointers into static strings or the loaded file. */
size_t daimon_config_difficulty_names(const char **out, size_t max);
/** The personalities' names, sorted (pointers into the loaded file, valid until it is reloaded): how many were
 *  written to `out` (at most `max`). */
size_t daimon_config_personality_names(const char **out, size_t max);

#ifdef __cplusplus
}
#endif

#endif
