/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_daimon.c
 *     Daimon's configuration loader.
 * @par Purpose:
 *     See config_daimon.h.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_daimon.h"
#include "config.h"

#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <toml.h>

#include "bflib_basics.h"
#include "config_strings.h"
#include "value_util.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Daimon's checks of the personalities' and difficulties' keys, once daimon.toml is loaded (DaimonGlue.cpp). */
void daimon_on_config_loaded(void);
/******************************************************************************/
static TbBool load_daimon_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData daimon_file_data = {
    .filename = "daimon.toml",
    .load_func = load_daimon_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

struct DaimonConfig daimon_config;

const char *const daimon_personality_header_keys[] = { "name", "name_text_id", "description", "skirmish_pool", "inherits", NULL };

#define GKEY(sec, k, field, d, lo, hi, rd) { sec, k, offsetof(struct DaimonGlobalConfig, field), d, lo, hi, rd, 1 }
#define GKEYF(sec, k, field, d, lo, hi, rd) { sec, k, offsetof(struct DaimonGlobalConfig, field), d, lo, hi, rd, 1024 }
const struct DaimonGlobalKey daimon_global_keys[] = {
    GKEY("budgets", "slab_scan",           budgets_slab_scan,          256,   1, 1000000, "DaiKnowledge"),
    GKEY("budgets", "reach_visits",        budgets_reach_visits,      4096,   1, 1000000, "DaiKnowledge"),
    GKEY("economy", "reserve_window",      economy_reserve_window,    2500,   0, 1000000, "DaiForecast"),
    GKEY("budgets", "search_nodes",        budgets_search_nodes,      8000,   1, 1000000, "DaiDigSearch"),
    GKEY("dig_costs", "floor",             dig_floor,                    1,   1,   10000, "DaiDigSearch"),
    GKEY("dig_costs", "earth",             dig_earth,                   20,   1,   10000, "DaiDigSearch"),
    GKEY("dig_costs", "gold_route",        dig_gold_route,             200,   1,   10000, "DaiDigSearch"),
    GKEY("dig_costs", "dense_route",       dig_dense_route,            500,   1,   10000, "DaiDigSearch"),
    GKEY("dig_costs", "own_wall",          dig_own_wall,               200,   1,   10000, "DaiDigSearch"),
    GKEY("dig_costs", "enemy_floor",       dig_enemy_floor,             80,   1,   10000, "DaiDigSearch"),
    GKEY("dig_costs", "bridge_base",       dig_bridge_base,             10,   1,   10000, "DaiDigSearch"),
    GKEY("dig_costs", "abyss_edge",        dig_abyss_edge,               5,   0,   10000, "DaiDigSearch"),
    GKEYF("dig_costs", "heuristic_weight", dig_heuristic_weight,      1024, 1024, 10240, "DaiDigSearch"),
    GKEY("dig_costs", "creep_cost",        dig_creep_cost,              60,   1,   10000, "DaiDigSearch"),
    GKEYF("dig_costs", "door_cost_per_hp", dig_door_cost_per_hp,        51,    0, 102400, "DaiDigSearch"),
    GKEY("dig_costs", "destroy_walls_cost", dig_destroy_walls_cost,      0,   0, 1000000, "DaiDigSearch"),
    GKEY("budgets", "max_searches",        budgets_max_searches,         2,   1,     4, "DaiSearchPool"),
    GKEY("excavation", "marks_ahead",      excavation_marks_ahead,      12,   1,   256, "JobDigRoute"),
    GKEY("excavation", "mine_marks",       excavation_mine_marks,       10,   1,   256, "JobMineVein"),
    GKEYF("excavation", "replan_ratio",    excavation_replan_ratio,   1536, 1024, 10240, "JobDigRoute"),
    GKEY("excavation", "rescore_interval", excavation_rescore_interval, 250,  10, 100000, "DaiExcavation"),
    GKEY("excavation", "claim_window",     excavation_claim_window,     2000, 100, 100000, "DaiExcavation"),
    GKEY("excavation", "unreachable_retry", excavation_unreachable_retry, 3000, 100, 100000, "DaiExcavation"),
    GKEY("excavation", "max_veins",        excavation_max_veins,          3,   1,      8, "DaiExcavation"),
    GKEYF("excavation", "marks_share",     excavation_marks_share,     614,   0,   1024, "DaiExcavation"),
    GKEY("workforce", "interval",          workforce_interval,          100,   1,  10000, "DaiWorkforce"),
    GKEY("budgets", "layout_candidates",   budgets_layout_candidates,   256,   1, 100000, "DaiLayoutSearch"),
    GKEY("layout",  "radius",              layout_radius,                12,   2,     64, "DaiLayoutSearch"),
    GKEY("layout",  "rim",                 layout_rim,                    1,   1,      2, "DaiLayoutSearch"),
    GKEY("programme", "interval",          programme_interval,          500,  10, 100000, "DaiDirector"),
    GKEY("programme", "lair_spare",        programme_lair_spare,          4,   0,    100, "DaiDirector"),
    GKEY("programme", "train_runway",      programme_train_runway,        2,   0,     50, "DaiDirector"),
    GKEY("layout",  "cache_turns",         layout_cache_turns,         2000,   0, 100000, "DaiConstruction"),
    GKEYF("layout", "grow_at",             layout_grow_at,              870,   1,   1024, "DaiConstruction"),
    GKEY("construction", "room_build_concurrency", construction_room_build_concurrency, 2, 1, 4, "DaiConstruction"),
    GKEY("creatures", "moves_per_kturn",   creatures_moves_per_kturn,     6,   0,    100, "DaiCreatures"),
    GKEY("creatures", "livid_grace",       creatures_livid_grace,       600,   0, 100000, "DaiCreatures"),
    GKEY("creatures", "scavengers_max",    creatures_scavengers_max,      4,   0,     50, "DaiCreatures"),
    GKEY("creatures", "scavenge_interval", creatures_scavenge_interval, 500,  10, 100000, "DaiCreatures"),
    GKEY("creatures", "pool_try_turns",    creatures_pool_try_turns,   2000,  10, 1000000, "DaiCreatures"),
    GKEY("creatures", "pool_dry_turns",    creatures_pool_dry_turns,  10000,  10, 1000000, "DaiCreatures"),
    GKEY("director", "director_interval",  director_interval,           250,  10, 100000, "DaiDirector"),
    GKEY("director", "switch_margin",      director_switch_margin,       64,   0,   1024, "DaiDirector"),
    GKEY("director", "build_up_max_turns", director_build_up_max_turns, 12000, 0, 1000000, "DaiDirector"),
    GKEY("director", "contest_radius",     director_contest_radius,      12,   1,     64, "DaiDirector"),
    GKEY("director", "contest_forget",     director_contest_forget,    3000,  10, 1000000, "DaiDirector"),
    GKEY("director", "recover_window",     director_recover_window,    3000,  10, 1000000, "DaiDirector"),
    GKEY("director", "attack_rest",        director_attack_rest,       2000,   0, 1000000, "DaiDirector"),
    GKEY("pace",    "relaxed",             pace_relaxed,                150,   1,   1000, "DaiArbiter"),
    GKEY("pace",    "human",               pace_human,                  400,   1,   1000, "DaiArbiter"),
    GKEY("pace",    "unlimited",           pace_unlimited,             1000,   1,   1000, "DaiArbiter"),
    GKEY("cheats",  "rich_start_gold",     cheats_rich_start_gold,    10000,   0, 1000000, "Daimon"),
    GKEY("cheats",  "creature_bonus_pct",  cheats_creature_bonus_pct,    25,   0,    400, "Daimon"),
    GKEY("cheats",  "pay_discount_pct",    cheats_pay_discount_pct,      25,   0,    100, "Daimon"),
    GKEY("threat",  "interval",            threat_interval,              25,   1,  10000, "DaiThreatModel"),
    GKEY("threat",  "halflife",            threat_halflife,            1500,  10, 1000000, "DaiThreatModel"),
    GKEY("threat",  "speed_per_slab",      threat_speed_per_slab,        16,   1,   1000, "DaiThreatModel"),
    GKEY("threat",  "check_interval",      threat_check_interval,       100,  10, 100000, "DaiThreatModel"),
    GKEY("threat",  "route_interval",      threat_route_interval,      1000, 100, 1000000, "DaiThreatModel"),
    GKEY("threat",  "search_nodes",        threat_search_nodes,        4000,   1, 1000000, "DaiThreatModel"),
    GKEY("threat",  "destroy_walls",       threat_destroy_walls,          0,   0,  10000, "DaiDigSearch"),
    GKEYF("threat", "own_door_per_hp",     threat_own_door_per_hp,      256,   0, 100 * 1024, "DaiDigSearch"),
    GKEY("threat",  "frontier_weight",     threat_frontier_weight,       50,   0, 1000000, "DaiThreatModel"),
    GKEY("threat",  "hero_weight",         threat_hero_weight,          200,   0, 1000000, "DaiThreatModel"),
    GKEY("threat",  "max_chokepoints",     threat_max_chokepoints,       16,   1,     64, "DaiThreatModel"),
    GKEY("tactics", "fight_tick",          tactics_fight_tick,            2,   1,   1000, "DaiTactics"),
    GKEY("tactics", "idle_tick",           tactics_idle_tick,            50,   1,  10000, "DaiTactics"),
    GKEY("tactics", "burst_turns",         tactics_burst_turns,          60,   0,   1000, "DaiArbiter"),
    GKEY("tactics", "contact_radius",      tactics_contact_radius,        6,   0,     64, "DaiTactics"),
    GKEY("tactics", "match_margin",        tactics_match_margin,          2,   0,     64, "DaiTactics"),
    GKEY("defence", "interval",            defence_interval,            100,   1, 100000, "DaiDefence"),
    GKEY("defence", "area_threat",         defence_area_threat,          50,   1, 1000000, "DaiDefence"),
    GKEY("defence", "imps_threat",         defence_imps_threat,          50,   1, 1000000, "DaiDefence"),
    GKEY("defence", "lock_radius",         defence_lock_radius,          10,   0,     64, "DaiDefence"),
    GKEY("defence", "max_traps",           defence_max_traps,             8,   0,     64, "DaiFortify"),
    GKEY("defence", "max_doors",           defence_max_doors,             4,   0,     64, "DaiFortify"),
    GKEY("defence", "intercept_turns",     defence_intercept_turns,     600,  10, 100000, "DaiDefence"),
    GKEY("defence", "defend_cooldown",     defence_defend_cooldown,    1500,  10, 100000, "DaiDirector"),
    GKEY("defence", "threat_force",        defence_threat_force,          4,   1,   1000, "DaiDirector"),
    GKEY("defence", "max_guard_posts",     defence_max_guard_posts,       2,   0,      8, "DaiDefence"),
    GKEY("defence", "hazard_imps",         defence_hazard_imps,           2,   1,    100, "DaiThreatModel"),
    GKEY("defence", "hazard_window",       defence_hazard_window,      3000, 100, 100000, "DaiThreatModel"),
    GKEY("defence", "hazard_reach",        defence_hazard_reach,          4,   0,     16, "DaiThreatModel"),
    GKEY("defence", "hazard_rebuy_turns",  defence_hazard_rebuy_turns,  600,   0, 100000, "DaiWorkforce"),
    GKEY("defence", "hazard_imps_floor",   defence_hazard_imps_floor,     3,   0,     64, "DaiWorkforce"),
    GKEYF("defence", "ward_share",         defence_ward_share,          256,   0,   1024, "DaiDefence"),
    GKEY("defence", "spend_gold_imps",     defence_spend_gold_imps,       4,   1,    100, "DaiDefence"),
    GKEY("defence", "hold_turns",          defence_hold_turns,         1500, 100, 100000, "DaiDefence"),
    GKEY("custody", "plan_interval",       custody_plan_interval,       200,  10, 100000, "DaiCustody"),
    GKEY("custody", "prison_spare",        custody_prison_spare,          2,   0,    100, "DaiCustody"),
    GKEY("custody", "heal_victim_below",   custody_heal_victim_below,    40,   0,    100, "DaiCustody"),
    GKEY("custody", "interrogate_value",   custody_interrogate_value,    10,   0, 100000, "DaiCustody"),
    GKEYF("custody", "hold_share",         custody_hold_share,          256,   0,   1024, "DaiCustody"),
    GKEY("custody", "research_value",      custody_research_value,       20,   0, 100000, "DaiCustody"),
    GKEY("custody", "manufacture_value",   custody_manufacture_value,    10,   0, 100000, "DaiCustody"),
    GKEY("custody", "happy_value",         custody_happy_value,           2,   0, 100000, "DaiCustody"),
    GKEY("custody", "bless_value",         custody_bless_value,           1,   0, 100000, "DaiCustody"),
    GKEY("custody", "imp_price_step",      custody_imp_price_step,      150,   0, 100000, "DaiCustody"),
    GKEY("custody", "imp_horizon",         custody_imp_horizon,       20000,   0, 1000000, "DaiCustody"),
    GKEYF("abilities", "area_density",     abilities_area_density,      256,   0,  10240, "DaiAbilities"),
    GKEY("abilities", "area_max",          abilities_area_max,            4,   1,    100, "DaiAbilities"),
    GKEYF("abilities", "area_falloff",     abilities_area_falloff,      614,   0,   1024, "DaiAbilities"),
    GKEYF("abilities", "beam_targets",     abilities_beam_targets,     2560, 1024,  10240, "DaiAbilities"),
    GKEY("abilities", "enemy_hp",          abilities_enemy_hp,          800,   1, 100000, "DaiAbilities"),
    GKEY("abilities", "trap_fight_turns",  abilities_trap_fight_turns, 1000,   1, 100000, "DaiAbilities"),
    GKEY("assistant", "possession_radius", assistant_possession_radius,  8,   0,    255, "keeper_exclusion"),
    GKEY("assistant", "lease_turns",       assistant_lease_turns,      600,   0, 100000, "DaiLeases"),
    GKEY("lieutenant", "agent_lease_turns", lieutenant_agent_lease_turns, 600, 0, 100000, "DaiLeases"),
    GKEYF("abilities", "hail_hit",         abilities_hail_hit,          358,   0,   1024, "DaiAbilities"),
    GKEY("abilities", "knock_turns",       abilities_knock_turns,        40,   0,   1000, "DaiAbilities"),
    GKEY("abilities", "enemy_damage",      abilities_enemy_damage,      400,   0, 100000, "DaiAbilities"),
    GKEYF("abilities", "ranged_share",     abilities_ranged_share,      410,   0,   1024, "DaiAbilities"),
    GKEY("abilities", "teleport_value",    abilities_teleport_value,    150,   0, 100000, "DaiAbilities"),
    GKEY("abilities", "sight_value",       abilities_sight_value,        35,   0, 100000, "DaiAbilities"),
    GKEYF("abilities", "rebirth_value",    abilities_rebirth_value,     512,   0,  10240, "DaiAbilities"),
    GKEYF("abilities", "allure_value",     abilities_allure_value,      512,   0,  10240, "DaiCreatures"),
    GKEY("custody", "graveyard_bodies",    custody_graveyard_bodies,      3,   1,   1000, "DaiCustody"),
    GKEY("custody", "jailbreak_min_force", custody_jailbreak_min_force,   5,   0, 100000, "DaiCustody"),
    GKEY("custody", "jailbreak_timeout",   custody_jailbreak_timeout, 10000, 100, 1000000, "DaiCustody"),
    GKEY("custody", "jailbreak_retry",     custody_jailbreak_retry,    5000,  10, 1000000, "DaiCustody"),
    GKEY("offence", "interval",            offence_interval,            250,   1, 100000, "DaiRivals"),
    GKEY("offence", "route_interval",      offence_route_interval,     1000,   1, 1000000, "DaiRivals"),
    GKEY("offence", "search_nodes",        offence_search_nodes,       4000,   1, 1000000, "DaiRivals"),
    GKEY("offence", "hate_halflife",       offence_hate_halflife,      3000,   1, 1000000, "DaiRivals"),
    GKEY("offence", "attack_runway",       offence_attack_runway,         2,   0,    100, "DaiRivals"),
    GKEY("offence", "defence_radius",      offence_defence_radius,        5,   0,     64, "DaiRivals"),
    GKEY("offence", "op_interval",         offence_op_interval,          50,   1,  10000, "DaiOffence"),
    GKEY("offence", "recon_timeout",       offence_recon_timeout,      4000,   1, 1000000, "DaiOffence"),
    GKEY("offence", "stage_timeout",       offence_stage_timeout,     12000,   1, 1000000, "DaiOffence"),
    GKEY("offence", "breach_timeout",      offence_breach_timeout,     6000,   1, 1000000, "DaiOffence"),
    GKEY("offence", "assault_timeout",     offence_assault_timeout,    6000,   1, 1000000, "DaiOffence"),
    GKEY("offence", "withdraw_turns",      offence_withdraw_turns,     1500,   1, 1000000, "DaiOffence"),
    GKEY("offence", "retry_after",         offence_retry_after,        6000,   0, 1000000, "DaiOffence"),
    GKEY("offence", "sight_level",         offence_sight_level,           2,   0,      8, "DaiOffence"),
    GKEY("offence", "max_refusals",        offence_max_refusals,          3,   1,    100, "DaiOffence"),
    GKEY("offence", "gather_turns",        offence_gather_turns,        600,   0, 100000, "DaiTactics"),
    GKEY("offence", "gather_share",        offence_gather_share,         60,   0,    100, "DaiTactics"),
    GKEY("offence", "waves_per_kturn",     offence_waves_per_kturn,       8,   0,    100, "DaiTactics"),
    GKEY("offence", "raid_interval",       offence_raid_interval,      3000,   1, 1000000, "DaiOffence"),
    GKEY("offence", "raid_timeout",        offence_raid_timeout,       3000,   1, 1000000, "DaiOffence"),
    GKEY("offence", "harass_turns",        offence_harass_turns,        600,   1, 1000000, "DaiOffence"),
    GKEY("offence", "raid_border",         offence_raid_border,           6,   1,     64, "DaiOffence"),
    GKEY("offence", "raid_reach",          offence_raid_reach,           30,   1,    128, "DaiOffence"),
    GKEY("offence", "creep_imps",          offence_creep_imps,            4,   1,     16, "DaiOffence"),
    GKEY("offence", "exploit_turns",       offence_exploit_turns,      4000,   0, 1000000, "DaiOffence"),
    GKEY("diplomacy", "interval",          diplomacy_interval,          250,   1, 100000, "DaiOffence"),
    GKEY("diplomacy", "ally_dwell",        diplomacy_ally_dwell,       2000,   0, 1000000, "DaiOffence"),
    GKEY("diplomacy", "peace_after_war",   diplomacy_peace_after_war, 10000,   0, 10000000, "DaiOffence"),
    GKEY("inbox",   "expiry_payday_quarter",   inbox_expiry[1],      500,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_event",            inbox_expiry[2],     1000,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_objective",        inbox_expiry[3],     2000,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_custody",          inbox_expiry[4],     1000,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_sacrifice",        inbox_expiry[5],     1000,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_vampire_progress", inbox_expiry[6],     1000,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_wall_cue",         inbox_expiry[7],      600,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_assist_refusal",   inbox_expiry[8],      600,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_sight_open",       inbox_expiry[9],      200,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_loss",             inbox_expiry[10],    1000,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_loss_scan",        inbox_expiry[11],     300,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_intruder",         inbox_expiry[12],     300,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_trap_fired",       inbox_expiry[13],     300,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_enemy_trap",       inbox_expiry[14],     600,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_victory",          inbox_expiry[15],  100000,   1, 1000000, "DaiInbox"),
    GKEY("inbox",   "expiry_new_kind",         inbox_expiry[16],    2000,   1, 1000000, "DaiInbox"),
    GKEY("arbiter", "quota_window",        arbiter_quota_window,         8,    1,    64, "DaiArbiter"),
    GKEY("arbiter", "age_bonus_per_kturn", arbiter_age_bonus_per_kturn, 256,   0, 65536, "DaiArbiter"),
    GKEY("arbiter", "weight_economy",          arbiter_weight[0],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_workforce",        arbiter_weight[1],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_excavation",       arbiter_weight[2],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_construction",     arbiter_weight[3],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_creatures",        arbiter_weight[4],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_custody",          arbiter_weight[5],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_defence",          arbiter_weight[6],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_offence",          arbiter_weight[7],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_magic",            arbiter_weight[8],     1,   1,    64, "DaiArbiter"),
    GKEY("arbiter", "weight_tactics",          arbiter_weight[9],     1,   1,    64, "DaiArbiter"),
    GKEY("jobs",    "stall_window",        jobs_stall_window,          600,  20, 100000, "DaiJobPool"),
    GKEY("net",     "replicate_interval",  net_replicate_interval,     200,  20, 10000, "DaiReplicator"),
    GKEY("threads", "workers",             threads_workers,             -1,  -1, 64, "DaiWorkers"),
    GKEY("threads", "reserved_cores",      threads_reserved_cores,       2,   0, 64, "DaiWorkers"),
    GKEY("threads", "jitter_us",           threads_jitter_us,            0,   0, 100000, "DaiWorkers"),
    GKEY("threads", "base_turns_per_second", threads_base_turns_per_second, 20, 1, 1000, "DaiKeeper"),
    { NULL, NULL, 0, 0, 0, 0, NULL, 0 },
};
#undef GKEY
#undef GKEYF

/** A copy of `src` into `dst` (initialised here), however deep. */
static void value_copy_deep(VALUE *dst, const VALUE *src)
{
    switch (value_type(src)) {
    case VALUE_BOOL: value_init_bool(dst, value_bool(src)); break;
    case VALUE_INT32: case VALUE_UINT32: case VALUE_INT64: case VALUE_UINT64:
        value_init_int64(dst, value_int64(src)); break;
    case VALUE_FLOAT: case VALUE_DOUBLE: value_init_double(dst, value_double(src)); break;
    case VALUE_STRING: value_init_string_(dst, value_string(src), value_string_length(src)); break;
    case VALUE_ARRAY:
        value_init_array(dst);
        for (size_t i = 0; i < value_array_size(src); i++)
            value_copy_deep(value_array_append(dst), value_array_get(src, i));
        break;
    case VALUE_DICT: {
        value_init_dict(dst);
        const size_t n = value_dict_size(src);
        const VALUE *keys[256];
        const size_t got = value_dict_keys_sorted(src, keys, (n < 256) ? n : 256);
        for (size_t i = 0; i < got; i++) {
            const char *k = value_string(keys[i]);
            value_copy_deep(value_dict_add(dst, k), value_dict_get(src, k));
        }
        break;
    }
    default: value_init_null(dst); break;
    }
}

/** Puts `src`'s keys into `dst`: tables merge key by key, anything else replaces. */
static void value_merge(VALUE *dst, const VALUE *src)
{
    const size_t n = value_dict_size(src);
    const VALUE *keys[256];
    const size_t got = value_dict_keys_sorted(src, keys, (n < 256) ? n : 256);
    for (size_t i = 0; i < got; i++) {
        const char *k = value_string(keys[i]);
        const VALUE *sv = value_dict_get(src, k);
        VALUE *dv = value_dict_get(dst, k);
        if ((dv != NULL) && (value_type(dv) == VALUE_DICT) && (value_type(sv) == VALUE_DICT)) {
            value_merge(dv, sv);
            continue;
        }
        if (dv != NULL) value_dict_remove(dst, k);
        value_copy_deep(value_dict_add(dst, k), sv);
    }
}

/** The global sections from the merged tree: defaults, then the file's values, clamped (out of range is logged). */
static void resolve_globals(void)
{
    for (const struct DaimonGlobalKey *gk = daimon_global_keys; gk->section != NULL; gk++) {
        int64_t *field = (int64_t *)((char *)&daimon_config.global + gk->offset);
        *field = gk->dflt;
        const VALUE *sec = daimon_config.loaded ? value_dict_get(&daimon_config.root, gk->section) : NULL;
        const VALUE *v = (sec != NULL) ? value_dict_get(sec, gk->key) : NULL;
        if (v == NULL) continue;
        int64_t x;
        if ((gk->scale > 1) && value_is_compatible(v, VALUE_DOUBLE)) {
            const double d = value_double(v) * (double)gk->scale;
            x = (d >= 9.0e18) ? INT64_MAX : (d <= -9.0e18) ? INT64_MIN : (int64_t)llround(d);
        } else if (!value_is_compatible(v, VALUE_INT64)) {
            WARNLOG("daimon.toml: [%s] %s is not a %s; using the default", gk->section, gk->key,
                (gk->scale > 1) ? "number" : "whole number");
            continue;
        } else {
            x = value_int64(v);
        }
        if ((x < gk->min) || (x > gk->max)) {
            WARNLOG("daimon.toml: [%s] %s = %" PRId64 " is outside %" PRId64 "..%" PRId64 "; clamped", gk->section, gk->key, x, gk->min, gk->max);
            x = (x < gk->min) ? gk->min : gk->max;
        }
        *field = x;
    }
}

void daimon_config_clear(void)
{
    if (daimon_config.loaded)
        value_fini(&daimon_config.root);
    memset(&daimon_config, 0, sizeof(daimon_config));
    resolve_globals();
}

TbBool daimon_config_load_file(const char *fname, TbBool partial)
{
    VALUE file_root;
    if (!load_toml_file(fname, &file_root, partial ? CnfLd_IgnoreErrors : 0))
        return false;
    if (value_type(&file_root) != VALUE_DICT) {
        value_fini(&file_root);
        WARNLOG("daimon.toml: \"%s\" is not a table of sections", fname);
        return false;
    }
    if (!partial || !daimon_config.loaded) {
        if (daimon_config.loaded)
            value_fini(&daimon_config.root);
        value_copy_deep(&daimon_config.root, &file_root);
        daimon_config.loaded = true;
    } else {
        // A campaign's, map pack's or level's own file (Phase 7): its personalities added; one that changes a shipped
        // personality's keys says so, so an author sees it is a change, not a new one.
        const VALUE *mine = value_dict_get(&file_root, "personality");
        const VALUE *have = value_dict_get(&daimon_config.root, "personality");
        if ((mine != NULL) && (value_type(mine) == VALUE_DICT) && (have != NULL) && (value_type(have) == VALUE_DICT)) {
            const VALUE *keys[256];
            const size_t n = value_dict_keys_sorted(mine, keys, (value_dict_size(mine) < 256) ? value_dict_size(mine) : 256);
            for (size_t i = 0; i < n; i++)
                if (value_dict_get(have, value_string(keys[i])) != NULL)
                    JUSTMSG("daimon.toml: \"%s\" changes the personality \"%s\" (its keys over the earlier file's)", fname,
                            value_string(keys[i]));
        }
        value_merge(&daimon_config.root, &file_root);
    }
    value_fini(&file_root);
    resolve_globals();
    return true;
}

static TbBool load_daimon_config_file(const char *fname, unsigned short flags)
{
    return daimon_config_load_file(fname, (flags & CnfLd_AcceptPartial) != 0);
}

TbBool daimon_config_load(void)
{
    daimon_config_clear();
    const TbBool ok = load_config(&daimon_file_data, CnfLd_Standard);
    daimon_config_check();
    return ok;
}

static TbBool global_key_known(const char *section, const char *key)
{
    for (const struct DaimonGlobalKey *gk = daimon_global_keys; gk->section != NULL; gk++)
        if ((strcmp(gk->section, section) == 0) && (strcmp(gk->key, key) == 0)) return true;
    return false;
}

void daimon_config_check(void)
{
    if (daimon_config.loaded) {
        const VALUE *keys[256];
        const size_t n = value_dict_size(&daimon_config.root);
        const size_t got = value_dict_keys_sorted(&daimon_config.root, keys, (n < 256) ? n : 256);
        for (size_t i = 0; i < got; i++) {
            const char *section = value_string(keys[i]);
            if ((strcmp(section, "personality") == 0) || (strcmp(section, "difficulty") == 0)
                || (strcmp(section, "content") == 0) || (strcmp(section, "model_map") == 0))
                continue;   // kfx_daimon's (DaiPersonality, DaiDifficulty, DaiContent) and the level key's model map
            const VALUE *sec = value_dict_get(&daimon_config.root, section);
            const VALUE *skeys[256];
            const size_t sn = (value_type(sec) == VALUE_DICT) ? value_dict_size(sec) : 0;
            const size_t sgot = (sn > 0) ? value_dict_keys_sorted(sec, skeys, (sn < 256) ? sn : 256) : 0;
            for (size_t k = 0; k < sgot; k++) {
                const char *key = value_string(skeys[k]);
                if (global_key_known(section, key)) continue;
                char what[128];
                snprintf(what, sizeof(what), "[%s] %s", section, key);
                WARNLOG("daimon.toml: %s is not a key this build reads; ignored", what);
            }
        }
    }
    if (daimon_config_personality("default") == NULL)
        WARNLOG("daimon.toml: no [personality.default]; Daimon's keepers use the built-in default values");
    daimon_on_config_loaded();
}

const VALUE *daimon_config_personality(const char *name)
{
    if (!daimon_config.loaded || (name == NULL)) return NULL;
    const VALUE *all = value_dict_get(&daimon_config.root, "personality");
    const VALUE *p = (all != NULL) ? value_dict_get(all, name) : NULL;
    return ((p != NULL) && (value_type(p) == VALUE_DICT)) ? p : NULL;
}

/** A header string of a personality's table, or `dflt` when it is missing or not a string. */
static const char *header_string(const VALUE *p, const char *key, const char *dflt)
{
    const VALUE *v = value_dict_get(p, key);
    return ((v != NULL) && (value_type(v) == VALUE_STRING)) ? value_string(v) : dflt;
}

size_t daimon_config_personality_list(struct DaimonPersonalityInfo *out, size_t max)
{
    const char *ids[256];
    const size_t n = daimon_config_personality_names(ids, 256);
    size_t written = 0;
    // "default" first, then the others in their sorted order.
    for (int pass = 0; pass < 2; pass++) {
        for (size_t i = 0; (i < n) && (written < max); i++) {
            if ((strcmp(ids[i], "default") == 0) != (pass == 0)) continue;
            const VALUE *p = daimon_config_personality(ids[i]);
            struct DaimonPersonalityInfo *info = &out[written++];
            info->id = ids[i];
            info->name = header_string(p, "name", ids[i]);
            info->description = header_string(p, "description", "");
            const VALUE *tid = value_dict_get(p, "name_text_id");
            info->name_text_id = ((tid != NULL) && value_is_compatible(tid, VALUE_INT64) && (value_int64(tid) > 0)) ? value_int64(tid) : 0;
            const VALUE *pool = value_dict_get(p, "skirmish_pool");
            info->skirmish_pool = (pool != NULL) && (value_type(pool) == VALUE_BOOL) && value_bool(pool);
        }
    }
    return written;
}

int32_t daimon_config_personality_number(const char *id)
{
    if ((id == NULL) || (id[0] == '\0')) return 0;
    struct DaimonPersonalityInfo list[DAIMON_MAX_PERSONALITIES];
    const size_t n = daimon_config_personality_list(list, DAIMON_MAX_PERSONALITIES);
    for (size_t i = 0; i < n; i++)
        if (strcmp(list[i].id, id) == 0) return (int32_t)i + 1;
    return 0;
}

const char *daimon_config_personality_by_number(int64_t number)
{
    if (number <= 0) return NULL;
    struct DaimonPersonalityInfo list[DAIMON_MAX_PERSONALITIES];
    const size_t n = daimon_config_personality_list(list, DAIMON_MAX_PERSONALITIES);
    return ((size_t)number <= n) ? list[number - 1].id : NULL;
}

const char *daimon_personality_display_name(const struct DaimonPersonalityInfo *info)
{
    return (info->name_text_id > 0) ? get_string((TextStringId)info->name_text_id) : info->name;
}

size_t daimon_config_difficulty_names(const char **out, size_t max)
{
    static const char *const builtin[] = { "easy", "normal", "hard" };
    size_t written = 0;
    for (size_t i = 0; (i < 3) && (written < max); i++) out[written++] = builtin[i];
    const VALUE *all = daimon_config.loaded ? value_dict_get(&daimon_config.root, "difficulty") : NULL;
    if ((all == NULL) || (value_type(all) != VALUE_DICT)) return written;
    const VALUE *keys[64];
    const size_t n = value_dict_size(all);
    const size_t got = value_dict_keys_sorted(all, keys, (n < 64) ? n : 64);
    for (size_t i = 0; (i < got) && (written < max); i++) {
        const char *name = value_string(keys[i]);
        if ((strcmp(name, "easy") == 0) || (strcmp(name, "normal") == 0) || (strcmp(name, "hard") == 0)) continue;
        if (value_type(value_dict_get(all, name)) == VALUE_DICT) out[written++] = name;
    }
    return written;
}

size_t daimon_config_personality_names(const char **out, size_t max)
{
    if (!daimon_config.loaded || (out == NULL)) return 0;
    const VALUE *all = value_dict_get(&daimon_config.root, "personality");
    if ((all == NULL) || (value_type(all) != VALUE_DICT)) return 0;
    const VALUE *keys[256];
    const size_t n = value_dict_size(all);
    const size_t got = value_dict_keys_sorted(all, keys, (n < 256) ? n : 256);
    size_t written = 0;
    for (size_t i = 0; (i < got) && (written < max); i++) {
        const char *name = value_string(keys[i]);
        if (daimon_config_personality(name) != NULL) out[written++] = name;
    }
    return written;
}
/******************************************************************************/
#ifdef __cplusplus
}
#endif

const char *daimon_config_model_personality(int64_t model)
{
    if (!daimon_config.loaded) return NULL;
    const VALUE *map = value_dict_get(&daimon_config.root, "model_map");
    if ((map == NULL) || (value_type(map) != VALUE_DICT)) return NULL;
    char key[24];
    snprintf(key, sizeof(key), "%" PRId64, model);
    const VALUE *v = value_dict_get(map, key);
    return ((v != NULL) && (value_type(v) == VALUE_STRING)) ? value_string(v) : NULL;
}
