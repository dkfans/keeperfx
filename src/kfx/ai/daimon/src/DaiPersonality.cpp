#include "pre_inc.h"
#include "daimon/DaiPersonality.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <strings.h>
#include <vector>

#include "config_daimon.h"
#include "daimon/DaiFix.h"
#include "daimon/DaiLimits.h"
#include "daimon/DaiLog.h"
#include "bflib_basics.h"
#include "post_inc.h"

namespace {
/** The rooms of the programme, in the order their anchor names give them (after "heart"). */
const DaiRoom kProgrammeRooms[] = { DaiRoom::Treasury, DaiRoom::Lair, DaiRoom::Hatchery, DaiRoom::Library, DaiRoom::Training, DaiRoom::Workshop, DaiRoom::Prison, DaiRoom::Torture, DaiRoom::Temple, DaiRoom::Graveyard, DaiRoom::Scavenger, DaiRoom::Barracks, DaiRoom::GuardPost };
const char *const kDismissNames[] = { "never", "harmful", "always", nullptr };
const char *const kCtaUseNames[] = { "never", "own_ground", "anywhere", nullptr };
const char *const kFirstTargetNames[] = { "heart", "room", "portal", nullptr };
const char *const kDisciplineNames[] = { "none", "prison", "torture", "both", nullptr };
const char *const kSacrificeNames[] = { "none", "prisoners", "any", nullptr };
const char *const kObeyNames[] = { "never", "fights", "fights_and_work", nullptr };
const char *const kArmageddonNames[] = { "never", "finisher", nullptr };
const char *const kAnchorNames[] = { "heart", "treasury", "lair", "hatchery", "library", "training", "workshop", "prison", "torture", "temple", "graveyard", "scavenger", "barracks", "guard_post", nullptr };

const DaiPersonality::Field kFields[] = {
    { "economy",   "reserve_margin",        &DaiPersonality::economy_reserve_margin,          256, 0, 4 * DAI_FIX_ONE, "DaiForecast",
      DAI_FIX_ONE },
    { "economy",   "comfortable_paydays",   &DaiPersonality::economy_comfortable_paydays,       3, 1,    50,     "DaiEconomy" },
    { "economy",   "bank_paydays",          &DaiPersonality::economy_bank_paydays,              4, 0,    50,     "DaiForecast" },
    { "economy",   "gold_contest_share",    &DaiPersonality::economy_gold_contest_share,       20, 0,   100,     "DaiForecast" },
    { "economy",   "extras_horizon",        &DaiPersonality::economy_extras_horizon,        20000, 0, 1000000,   "DaiDirector" },
    { "economy",   "grow_full",             &DaiPersonality::economy_grow_full,                90, 0,   100,     "DaiDirector" },
    { "economy",   "sell_rooms_in_emergency", &DaiPersonality::economy_sell_rooms_in_emergency, 1, 0,    1,     "DaiEconomy" },
    { "economy",   "forward_treasury_turns", &DaiPersonality::economy_forward_treasury_turns, 6000, 0, 1000000, "DaiEconomy" },
    { "economy",   "fortify",               &DaiPersonality::economy_fortify,                   1, 0,       1, "DaiEconomy" },
    { "economy",   "fortify_below",         &DaiPersonality::economy_fortify_below,            90, 0,     100, "DaiEconomy" },
    { "economy",   "fortify_interval",      &DaiPersonality::economy_fortify_interval,        400, 1, 1000000, "DaiEconomy" },
    { "excavation", "unknown_cost",         &DaiPersonality::excavation_unknown_cost,          30, 1,   10000, "DaiDigSearch" },
    { "excavation", "gold_to_turns",        &DaiPersonality::excavation_gold_to_turns,         51, 0, 100 * DAI_FIX_ONE,
      "DaiDigSearch", DAI_FIX_ONE },
    { "excavation", "horizon_turns",        &DaiPersonality::excavation_horizon_turns,      20000, 0, 1000000, "DaiExcavation" },
    { "excavation", "contest_vein_weight",  &DaiPersonality::excavation_contest_vein_weight, 1536, 0, 16 * DAI_FIX_ONE,
      "DaiExcavation", DAI_FIX_ONE },
    { "workforce", "imps_min",              &DaiPersonality::workforce_imps_min,                4, 0,   100,     "DaiWorkforce" },
    { "workforce", "train_imps", &DaiPersonality::workforce_train_imps, 2, 0, 100, "DaiCreatures" },
    { "workforce", "imp_train_level", &DaiPersonality::workforce_imp_train_level, 3, 1, 10, "DaiCreatures" },
    { "workforce", "imp_train_rich_level", &DaiPersonality::workforce_imp_train_rich_level, 10, 1, 10, "DaiCreatures" },
    { "workforce", "imp_train_rich_runway", &DaiPersonality::workforce_imp_train_rich_runway, 12, 1, 99, "DaiCreatures" },
    { "workforce", "imps_max",              &DaiPersonality::workforce_imps_max,               24, 1,   100,     "DaiWorkforce" },
    { "workforce", "work_per_imp",          &DaiPersonality::workforce_work_per_imp,            4, 1,  1000,     "DaiWorkforce" },
    { "workforce", "slabs_per_imp",         &DaiPersonality::workforce_slabs_per_imp,          25, 1, 10000,     "DaiWorkforce" },
    { "workforce", "slap_share",            &DaiPersonality::workforce_slap_share,            256, 0, DAI_FIX_ONE, "DaiWorkforce",
      DAI_FIX_ONE },
    { "workforce", "slap_interval",         &DaiPersonality::workforce_slap_interval,         200, 10, 100000,   "DaiWorkforce" },
    { "workforce", "tunnellers",            &DaiPersonality::workforce_tunnellers,              1, 0,     1,     "DaiWorkforce" },
    { "workforce", "tunneller_paydays",     &DaiPersonality::workforce_tunneller_paydays,      10, 0,  1000,     "DaiWorkforce" },
    { "workforce", "imp_gold_share",        &DaiPersonality::workforce_imp_gold_share,        256, 0, DAI_FIX_ONE, "DaiWorkforce",
      DAI_FIX_ONE },
    { "layout",    "w_eff",                 &DaiPersonality::layout_w_eff,               4 * 1024, 0, 100 * DAI_FIX_ONE, "DaiLayoutSearch",
      DAI_FIX_ONE },
    { "layout",    "w_adj",                 &DaiPersonality::layout_w_adj,                   1024, 0, 100 * DAI_FIX_ONE, "DaiLayoutSearch",
      DAI_FIX_ONE },
    { "layout",    "w_dist",                &DaiPersonality::layout_w_dist,              4 * 1024, 0, 100 * DAI_FIX_ONE, "DaiLayoutSearch",
      DAI_FIX_ONE },
    { "layout",    "w_dig",                 &DaiPersonality::layout_w_dig,                    102, 0, 100 * DAI_FIX_ONE, "DaiLayoutSearch",
      DAI_FIX_ONE },
    { "layout",    "w_exp",                 &DaiPersonality::layout_w_exp,                      0, 0, 100 * DAI_FIX_ONE, "DaiLayoutSearch",
      DAI_FIX_ONE },
    { "layout",    "w_grow",                &DaiPersonality::layout_w_grow,              2 * 1024, 0, 100 * DAI_FIX_ONE, "DaiLayoutSearch",
      DAI_FIX_ONE },
    { "layout",    "w_waste",               &DaiPersonality::layout_w_waste,            20 * 1024, 0, 100 * DAI_FIX_ONE, "DaiLayoutSearch",
      DAI_FIX_ONE },
    { "programme", "treasury_rank", &DaiPersonality::programme_treasury_rank, 1, 0, 100, "DaiDirector" },
    { "programme", "treasury_slabs", &DaiPersonality::programme_treasury_slabs, 9, 1, 400, "DaiDirector" },
    { "programme", "lair_rank", &DaiPersonality::programme_lair_rank, 2, 0, 100, "DaiDirector" },
    { "programme", "lair_slabs", &DaiPersonality::programme_lair_slabs, 16, 1, 400, "DaiDirector" },
    { "programme", "hatchery_rank", &DaiPersonality::programme_hatchery_rank, 3, 0, 100, "DaiDirector" },
    { "programme", "hatchery_slabs", &DaiPersonality::programme_hatchery_slabs, 12, 1, 400, "DaiDirector" },
    { "programme", "library_rank", &DaiPersonality::programme_library_rank, 4, 0, 100, "DaiDirector" },
    { "programme", "library_slabs", &DaiPersonality::programme_library_slabs, 20, 1, 400, "DaiDirector" },
    { "programme", "training_rank", &DaiPersonality::programme_training_rank, 5, 0, 100, "DaiDirector" },
    { "programme", "training_slabs", &DaiPersonality::programme_training_slabs, 16, 1, 400, "DaiDirector" },
    { "programme", "workshop_rank", &DaiPersonality::programme_workshop_rank, 6, 0, 100, "DaiDirector" },
    { "programme", "workshop_slabs", &DaiPersonality::programme_workshop_slabs, 17, 1, 400, "DaiDirector" },
    { "programme", "prison_rank", &DaiPersonality::programme_prison_rank, 0, 0, 100, "DaiDirector" },
    { "programme", "prison_slabs", &DaiPersonality::programme_prison_slabs, 9, 1, 400, "DaiDirector" },
    { "programme", "torture_rank", &DaiPersonality::programme_torture_rank, 0, 0, 100, "DaiDirector" },
    { "programme", "torture_slabs", &DaiPersonality::programme_torture_slabs, 9, 1, 400, "DaiDirector" },
    { "programme", "temple_rank", &DaiPersonality::programme_temple_rank, 0, 0, 100, "DaiDirector" },
    { "programme", "temple_slabs", &DaiPersonality::programme_temple_slabs, 9, 1, 400, "DaiDirector" },
    { "programme", "graveyard_rank", &DaiPersonality::programme_graveyard_rank, 0, 0, 100, "DaiDirector" },
    { "programme", "graveyard_slabs", &DaiPersonality::programme_graveyard_slabs, 9, 1, 400, "DaiDirector" },
    { "programme", "scavenger_rank", &DaiPersonality::programme_scavenger_rank, 0, 0, 100, "DaiDirector" },
    { "programme", "scavenger_slabs", &DaiPersonality::programme_scavenger_slabs, 9, 1, 400, "DaiDirector" },
    { "programme", "barracks_rank", &DaiPersonality::programme_barracks_rank, 0, 0, 100, "DaiDirector" },
    { "programme", "barracks_slabs", &DaiPersonality::programme_barracks_slabs, 9, 1, 400, "DaiDirector" },
    { "programme", "guard_post_rank", &DaiPersonality::programme_guard_post_rank, 0, 0, 100, "DaiDirector" },
    { "programme", "guard_post_slabs", &DaiPersonality::programme_guard_post_slabs, 9, 1, 400, "DaiDirector" },
    { "creatures", "train_target_level", &DaiPersonality::creatures_train_target_level, 4, 1, 10, "DaiCreatures" },
    { "creatures", "train_rich_level", &DaiPersonality::creatures_train_rich_level, 10, 0, 10, "DaiDirector" },
    { "creatures", "train_rich_runway", &DaiPersonality::creatures_train_rich_runway, 6, 1, 99, "DaiDirector" },
    { "creatures", "train_spare_paydays", &DaiPersonality::creatures_train_spare_paydays, 2, 0, 50, "DaiDirector" },
    { "creatures", "train_weight", &DaiPersonality::creatures_train_weight, 1536, 0, 4 * DAI_FIX_ONE, "DaiCreatures", DAI_FIX_ONE },
    { "creatures", "attract_upgrade", &DaiPersonality::creatures_attract_upgrade, 1536, 1024, 8 * DAI_FIX_ONE, "DaiDirector", DAI_FIX_ONE },
    { "creatures", "dismiss_weak", &DaiPersonality::creatures_dismiss_weak, 1, 0, 1, "DaiCreatures" },
    { "creatures", "work_worth", &DaiPersonality::creatures_work_worth, 100, 0, 100000, "DaiDirector" },
    { "creatures", "dismiss_interval", &DaiPersonality::creatures_dismiss_interval, 2000, 0, 1000000, "DaiCreatures" },
    { "creatures", "dismiss_upgrade", &DaiPersonality::creatures_dismiss_upgrade, 2048, 1024, 8 * DAI_FIX_ONE, "DaiCreatures", DAI_FIX_ONE },
    { "creatures", "research_rooms_weight", &DaiPersonality::creatures_research_rooms_weight, 2048, 0, 8 * DAI_FIX_ONE, "DaiCreatures", DAI_FIX_ONE },
    { "creatures", "train_ability_weight", &DaiPersonality::creatures_train_ability_weight, 1024, 0, 4 * DAI_FIX_ONE, "DaiCreatures", DAI_FIX_ONE },
    { "creatures", "dismiss_livid", &DaiPersonality::creatures_dismiss_livid, 1, 0, 2, "DaiCreatures", 1, kDismissNames },
    { "creatures", "scavenge_appetite", &DaiPersonality::creatures_scavenge_appetite, 307, 0, 4 * DAI_FIX_ONE, "DaiCreatures", DAI_FIX_ONE },
    { "director", "expand_appetite", &DaiPersonality::director_expand_appetite, 1024, 0, 4 * DAI_FIX_ONE, "DaiDirector",
      DAI_FIX_ONE },
    { "director", "contest_threshold", &DaiPersonality::director_contest_threshold, 819, 0, 4 * DAI_FIX_ONE, "DaiDirector",
      DAI_FIX_ONE },
    { "director", "recover_threshold", &DaiPersonality::director_recover_threshold, 512, 0, DAI_FIX_ONE, "DaiDirector",
      DAI_FIX_ONE },
    { "director", "dwell_turns", &DaiPersonality::director_dwell_turns, 1500, 0, 100000, "DaiDirector" },
    { "tactics", "cast_heal", &DaiPersonality::tactics_cast_heal, 1, 0, 1, "DaiFightRules" },
    { "tactics", "cast_lightning", &DaiPersonality::tactics_cast_lightning, 1, 0, 1, "DaiFightRules" },
    { "tactics", "cast_slow", &DaiPersonality::tactics_cast_slow, 1, 0, 1, "DaiFightRules" },
    { "tactics", "cast_freeze", &DaiPersonality::tactics_cast_freeze, 0, 0, 1, "DaiFightRules" },
    { "tactics", "cast_chicken", &DaiPersonality::tactics_cast_chicken, 0, 0, 1, "DaiFightRules" },
    { "tactics", "cast_speed", &DaiPersonality::tactics_cast_speed, 0, 0, 1, "DaiFightRules" },
    { "tactics", "cast_protect", &DaiPersonality::tactics_cast_protect, 0, 0, 1, "DaiFightRules" },
    { "tactics", "cast_rebound", &DaiPersonality::tactics_cast_rebound, 0, 0, 1, "DaiFightRules" },
    { "tactics", "cast_flight", &DaiPersonality::tactics_cast_flight, 0, 0, 1, "DaiFightRules" },
    { "tactics", "cast_vision", &DaiPersonality::tactics_cast_vision, 0, 0, 1, "DaiFightRules" },
    { "tactics", "cast_conceal", &DaiPersonality::tactics_cast_conceal, 1, 0, 1, "DaiFightRules" },
    { "tactics", "conceal_imps_below", &DaiPersonality::tactics_conceal_imps_below, 60, 1, 100, "DaiFightRules" },
    { "tactics", "cast_disease", &DaiPersonality::tactics_cast_disease, 0, 0, 1, "DaiFightRules" },
    { "tactics", "disease_battle", &DaiPersonality::tactics_disease_battle, 4, 1, 64, "DaiFightRules" },
    { "tactics", "disease_reserve", &DaiPersonality::tactics_disease_reserve, 12000, 0, 1000000, "DaiFightRules" },
    { "tactics", "heal_below", &DaiPersonality::tactics_heal_below, 35, 1, 99, "DaiFightRules" },
    { "tactics", "heal_level", &DaiPersonality::tactics_heal_level, 2, 0, 8, "DaiFightRules" },
    { "tactics", "hinder_ratio", &DaiPersonality::tactics_hinder_ratio, 110, 0, 1000, "DaiFightRules" },
    { "tactics", "finish_below", &DaiPersonality::tactics_finish_below, 40, 1, 100, "DaiFightRules" },
    { "tactics", "spell_level", &DaiPersonality::tactics_spell_level, 1, 0, 8, "DaiFightRules" },
    { "tactics", "cast_reserve", &DaiPersonality::tactics_cast_reserve, 1000, 0, 1000000, "DaiFightRules" },
    { "tactics", "cast_max_spend", &DaiPersonality::tactics_cast_max_spend, 4000, 0, 1000000, "DaiFightRules" },
    { "tactics", "cast_spare_share", &DaiPersonality::tactics_cast_spare_share, 50, 0, 100, "DaiFightRules" },
    { "tactics", "superiority_to_defend", &DaiPersonality::tactics_superiority_to_defend, 120, 0, 1000, "DaiTactics" },
    { "tactics", "retreat_health", &DaiPersonality::tactics_retreat_health, 25, 1, 99, "DaiTactics" },
    { "tactics", "hopeless_ratio", &DaiPersonality::tactics_hopeless_ratio, 300, 100, 10000, "DaiTactics" },
    { "tactics", "hold_audience_below", &DaiPersonality::tactics_hold_audience_below, 30, 0, 100, "DaiTactics" },
    { "tactics", "cta_use", &DaiPersonality::tactics_cta_use, 2, 0, 2, "DaiTactics", 1, kCtaUseNames },
    { "tactics", "group_size", &DaiPersonality::tactics_group_size, 1, 1, 30, "DaiDirector" },
    { "tactics", "flee_tendency", &DaiPersonality::tactics_flee_tendency, 1, 0, 1, "DaiTactics" },
    { "defence", "heart_priority", &DaiPersonality::defence_heart_priority, 4 * DAI_FIX_ONE, 0, 100 * DAI_FIX_ONE, "DaiDefence", DAI_FIX_ONE },
    { "defence", "rooms_priority", &DaiPersonality::defence_rooms_priority, 2 * DAI_FIX_ONE, 0, 100 * DAI_FIX_ONE, "DaiDefence", DAI_FIX_ONE },
    { "defence", "imps_priority", &DaiPersonality::defence_imps_priority, 1536, 0, 100 * DAI_FIX_ONE, "DaiDefence", DAI_FIX_ONE },
    { "defence", "trap_usage", &DaiPersonality::defence_trap_usage, 512, 0, DAI_FIX_ONE, "DaiFortify", DAI_FIX_ONE },
    { "defence", "heart_traps", &DaiPersonality::defence_heart_traps, 2, 0, 20, "DaiFortify" },
    { "defence", "door_preference", &DaiPersonality::defence_door_preference, 512, 0, DAI_FIX_ONE, "DaiFortify", DAI_FIX_ONE },
    { "defence", "boulder_use", &DaiPersonality::defence_boulder_use, 1, 0, 1, "DaiFortify" },
    { "defence", "guard_posts", &DaiPersonality::defence_guard_posts, 0, 0, 1, "DaiDefence" },
    { "defence", "secret_doors", &DaiPersonality::defence_secret_doors, 0, 0, 1, "DaiFortify" },
    { "defence", "spend_traps", &DaiPersonality::defence_spend_traps, 1, 0, 1, "DaiDefence" },
    { "defence", "hold_share", &DaiPersonality::defence_hold_share, 30, 0, 100, "DaiDefence" },
    { "defence", "perimeter_max_cut", &DaiPersonality::defence_perimeter_max_cut, 12, 0, 64, "DaiPerimeter" },
    { "defence", "perimeter_min_area", &DaiPersonality::defence_perimeter_min_area, 150, 1, 100000, "DaiPerimeter" },
    { "offence", "never_attack", &DaiPersonality::offence_never_attack, 0, 0, 1, "DaiRivals" },
    { "offence", "only_attack_attackers", &DaiPersonality::offence_only_attack_attackers, 0, 0, 1, "DaiRivals" },
    { "offence", "superiority_to_attack", &DaiPersonality::offence_superiority_to_attack, 150, 1, 1000, "DaiRivals" },
    { "offence", "horizon_attack_bonus", &DaiPersonality::offence_horizon_attack_bonus, 20, 0, 1000, "DaiRivals" },
    { "offence", "response_share", &DaiPersonality::offence_response_share, 50, 0, 100, "DaiRivals" },
    { "offence", "attack_level_target", &DaiPersonality::offence_attack_level_target, 4, 1, 10, "DaiRivals" },
    { "offence", "overwhelm_share", &DaiPersonality::offence_overwhelm_share, 150, 0, 10000, "DaiRivals" },
    { "offence", "urgency_turns", &DaiPersonality::offence_urgency_turns, 20000, 0, 1000000, "DaiRivals" },
    { "offence", "urgency_grace_paydays", &DaiPersonality::offence_urgency_grace_paydays, 1, 0, 100, "DaiRivals" },
    { "offence", "first_target", &DaiPersonality::offence_first_target, 0, 0, 2, "DaiRivals", 1, kFirstTargetNames },
    { "offence", "home_guard_share", &DaiPersonality::offence_home_guard_share, 307, 0, DAI_FIX_ONE, "DaiRivals", DAI_FIX_ONE },
    { "offence", "attack_threshold", &DaiPersonality::offence_attack_threshold, 1024, 0, 100 * DAI_FIX_ONE, "DaiRivals", DAI_FIX_ONE },
    { "offence", "cta_attack_budget", &DaiPersonality::offence_cta_attack_budget, 3000, 0, 1000000, "DaiOffence" },
    { "offence", "withdraw_ratio", &DaiPersonality::offence_withdraw_ratio, 200, 100, 10000, "DaiOffence" },
    { "offence", "withdraw_patience", &DaiPersonality::offence_withdraw_patience, 600, 0, 100000, "DaiOffence" },
    { "offence", "raid_appetite", &DaiPersonality::offence_raid_appetite, 512, 0, 4 * DAI_FIX_ONE, "DaiOffence", DAI_FIX_ONE },
    { "offence", "loyalty", &DaiPersonality::offence_loyalty, 717, 0, DAI_FIX_ONE, "DaiOffence", DAI_FIX_ONE },
    { "offence", "treachery", &DaiPersonality::offence_treachery, 102, 0, DAI_FIX_ONE, "DaiOffence", DAI_FIX_ONE },
    { "offence", "ally_min_value", &DaiPersonality::offence_ally_min_value, 1024, 0, 16 * DAI_FIX_ONE, "DaiOffence", DAI_FIX_ONE },
    { "tactics", "cta_switch_margin", &DaiPersonality::tactics_cta_switch_margin, 1536, DAI_FIX_ONE, 10 * DAI_FIX_ONE, "DaiTactics",
      DAI_FIX_ONE },
    { "programme", "treasury_anchor", &DaiPersonality::programme_treasury_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "lair_anchor", &DaiPersonality::programme_lair_anchor, 3, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "hatchery_anchor", &DaiPersonality::programme_hatchery_anchor, 2, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "library_anchor", &DaiPersonality::programme_library_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "training_anchor", &DaiPersonality::programme_training_anchor, 2, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "workshop_anchor", &DaiPersonality::programme_workshop_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "prison_anchor", &DaiPersonality::programme_prison_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "torture_anchor", &DaiPersonality::programme_torture_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "temple_anchor", &DaiPersonality::programme_temple_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "graveyard_anchor", &DaiPersonality::programme_graveyard_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "scavenger_anchor", &DaiPersonality::programme_scavenger_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "barracks_anchor", &DaiPersonality::programme_barracks_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "programme", "guard_post_anchor", &DaiPersonality::programme_guard_post_anchor, 0, 0, 13, "DaiConstruction", 1,
      kAnchorNames },
    { "custody",   "take_prisoners",        &DaiPersonality::custody_take_prisoners,            1, 0,     1,     "DaiCustody" },
    { "custody",   "w_convert",             &DaiPersonality::custody_w_convert,              1024, 0, 4 * DAI_FIX_ONE, "DaiCustody", DAI_FIX_ONE },
    { "custody",   "w_interrogate",         &DaiPersonality::custody_w_interrogate,          1024, 0, 4 * DAI_FIX_ONE, "DaiCustody", DAI_FIX_ONE },
    { "custody",   "w_skeleton",            &DaiPersonality::custody_w_skeleton,             1024, 0, 4 * DAI_FIX_ONE, "DaiCustody", DAI_FIX_ONE },
    { "custody",   "w_upkeep",              &DaiPersonality::custody_w_upkeep,               1024, 0, 4 * DAI_FIX_ONE, "DaiCustody", DAI_FIX_ONE },
    { "custody",   "w_ghost",               &DaiPersonality::custody_w_ghost,                1024, 0, 4 * DAI_FIX_ONE, "DaiCustody", DAI_FIX_ONE },
    { "custody",   "w_hold",                &DaiPersonality::custody_w_hold,                 1024, 0, 4 * DAI_FIX_ONE, "DaiCustody", DAI_FIX_ONE },
    { "custody",   "pay_discount_torture",  &DaiPersonality::custody_pay_discount_torture,      1, 0,     1,     "DaiCustody" },
    { "custody",   "discipline",            &DaiPersonality::custody_discipline,                0, 0,     3,     "DaiCreatures", 1, kDisciplineNames },
    { "custody",   "discipline_hold",       &DaiPersonality::custody_discipline_hold,        1500, 0, 100000, "DaiCreatures" },
    { "custody",   "sacrifices",            &DaiPersonality::custody_sacrifices,                2, 0,     2,     "DaiCustody", 1, kSacrificeNames },
    { "custody",   "vampires",              &DaiPersonality::custody_vampires,                  1, 0,     1,     "DaiCustody" },
    { "custody",   "w_body",                &DaiPersonality::custody_w_body,                 1024, 0, 4 * DAI_FIX_ONE, "DaiCustody", DAI_FIX_ONE },
    { "custody",   "rescue_value",          &DaiPersonality::custody_rescue_value,           1024, 0, 4 * DAI_FIX_ONE, "DaiCustody", DAI_FIX_ONE },
    { "magic",     "obey",                  &DaiPersonality::magic_obey,                        1, 0,     2,     "DaiMagic", 1, kObeyNames },
    { "magic",     "obey_reserve",          &DaiPersonality::magic_obey_reserve,             4000, 0, 1000000, "DaiMagic" },
    { "magic",     "obey_floor",            &DaiPersonality::magic_obey_floor,               1500, 0, 1000000, "DaiMagic" },
    { "magic",     "obey_fighters",         &DaiPersonality::magic_obey_fighters,               6, 1,    1000, "DaiMagic" },
    { "magic",     "obey_workers",          &DaiPersonality::magic_obey_workers,                8, 1,    1000, "DaiMagic" },
    { "magic",     "obey_hold",             &DaiPersonality::magic_obey_hold,                 300, 0, 1000000, "DaiMagic" },
    { "magic",     "specials",              &DaiPersonality::magic_specials,                    1, 0,       1, "DaiMagic" },
    { "magic",     "work_speed",            &DaiPersonality::magic_work_speed,                  1, 0,       1, "DaiMagic" },
    { "magic",     "work_speed_paydays",    &DaiPersonality::magic_work_speed_paydays,          2, 0,      50, "DaiMagic" },
    { "magic",     "heal_all_below",        &DaiPersonality::magic_heal_all_below,             60, 1,     100, "DaiMagic" },
    { "magic",     "heal_all_hurt",         &DaiPersonality::magic_heal_all_hurt,               3, 1,    1000, "DaiMagic" },
    { "magic",     "armageddon",            &DaiPersonality::magic_armageddon,                  0, 0,       1, "DaiMagic", 1, kArmageddonNames },
    { "magic",     "armageddon_superiority", &DaiPersonality::magic_armageddon_superiority,   133, 100, 10000, "DaiMagic" },
    { "magic",     "armageddon_heart",      &DaiPersonality::magic_armageddon_heart,           80, 0,     100, "DaiMagic" },
    { "magic",     "armageddon_traps",      &DaiPersonality::magic_armageddon_traps,            2, 0,     100, "DaiMagic" },
    { "magic",     "armageddon_reserve",    &DaiPersonality::magic_armageddon_reserve,      15000, 0, 1000000, "DaiMagic" },
    { "magic",     "armageddon_finish_turns", &DaiPersonality::magic_armageddon_finish_turns, 3000, 0, 100000, "DaiDirector" },
    { "magic",     "armageddon_gold_share", &DaiPersonality::magic_armageddon_gold_share,    15, 0,     100, "DaiMagic" },
};

/** The value a personality chain gives a field: the first table (most derived first) that has it. */
const VALUE *find_in_chain(const std::vector<const VALUE *> &chain, const char *module, const char *key)
{
    for (const VALUE *p : chain) {
        const VALUE *m = value_dict_get(p, module);
        const VALUE *v = ((m != nullptr) && (value_type(m) == VALUE_DICT)) ? value_dict_get(m, key) : nullptr;
        if (v != nullptr) return v;
    }
    return nullptr;
}

uint64_t mix(uint64_t h, int64_t v)
{
    h ^= (uint64_t)v + 0x9E3779B97F4A7C15ULL + (h << 6) + (h >> 2);
    return h;
}
}

int64_t DaiPersonality::ProgrammeRank(DaiRoom r) const
{
    switch (r) {
    case DaiRoom::Treasury: return programme_treasury_rank;
    case DaiRoom::Lair: return programme_lair_rank;
    case DaiRoom::Hatchery: return programme_hatchery_rank;
    case DaiRoom::Library: return programme_library_rank;
    case DaiRoom::Training: return programme_training_rank;
    case DaiRoom::Workshop: return programme_workshop_rank;
    case DaiRoom::Prison: return programme_prison_rank;
    case DaiRoom::Torture: return programme_torture_rank;
    case DaiRoom::Temple: return programme_temple_rank;
    case DaiRoom::Graveyard: return programme_graveyard_rank;
    case DaiRoom::Scavenger: return programme_scavenger_rank;
    case DaiRoom::Barracks: return programme_barracks_rank;
    case DaiRoom::GuardPost: return programme_guard_post_rank;
    default: return 0;
    }
}

int64_t DaiPersonality::ProgrammeSlabs(DaiRoom r) const
{
    switch (r) {
    case DaiRoom::Treasury: return programme_treasury_slabs;
    case DaiRoom::Lair: return programme_lair_slabs;
    case DaiRoom::Hatchery: return programme_hatchery_slabs;
    case DaiRoom::Library: return programme_library_slabs;
    case DaiRoom::Training: return programme_training_slabs;
    case DaiRoom::Workshop: return programme_workshop_slabs;
    case DaiRoom::Prison: return programme_prison_slabs;
    case DaiRoom::Torture: return programme_torture_slabs;
    case DaiRoom::Temple: return programme_temple_slabs;
    case DaiRoom::Graveyard: return programme_graveyard_slabs;
    case DaiRoom::Scavenger: return programme_scavenger_slabs;
    case DaiRoom::Barracks: return programme_barracks_slabs;
    case DaiRoom::GuardPost: return programme_guard_post_slabs;
    default: return 0;
    }
}

DaiRoom DaiPersonality::ProgrammeAnchor(DaiRoom r) const
{
    int64_t a = 0;
    switch (r) {
    case DaiRoom::Treasury: a = programme_treasury_anchor; break;
    case DaiRoom::Lair: a = programme_lair_anchor; break;
    case DaiRoom::Hatchery: a = programme_hatchery_anchor; break;
    case DaiRoom::Library: a = programme_library_anchor; break;
    case DaiRoom::Training: a = programme_training_anchor; break;
    case DaiRoom::Workshop: a = programme_workshop_anchor; break;
    case DaiRoom::Prison: a = programme_prison_anchor; break;
    case DaiRoom::Torture: a = programme_torture_anchor; break;
    case DaiRoom::Temple: a = programme_temple_anchor; break;
    case DaiRoom::Graveyard: a = programme_graveyard_anchor; break;
    case DaiRoom::Scavenger: a = programme_scavenger_anchor; break;
    case DaiRoom::Barracks: a = programme_barracks_anchor; break;
    case DaiRoom::GuardPost: a = programme_guard_post_anchor; break;
    default: break;
    }
    return ((a >= 1) && (a <= (int64_t)(sizeof(kProgrammeRooms) / sizeof(kProgrammeRooms[0])))) ? kProgrammeRooms[a - 1] : DaiRoom::Count;
}

std::vector<DaiPersonality::KeyIssue> DaiPersonality::CheckKeys(const DaimonConfig &cfg)
{
    std::vector<KeyIssue> out;
    const char *ids[256];
    const size_t n = cfg.loaded ? daimon_config_personality_names(ids, 256) : 0;
    for (size_t i = 0; i < n; i++) {
        const VALUE *p = daimon_config_personality(ids[i]);
        const VALUE *keys[256];
        const size_t got = value_dict_keys_sorted(p, keys, std::min<size_t>(value_dict_size(p), 256));
        for (size_t k = 0; k < got; k++) {
            const char *module = value_string(keys[k]);
            bool header = false;
            for (const char *const *h = daimon_personality_header_keys; *h != nullptr; h++) header |= (strcmp(*h, module) == 0);
            if (header) continue;
            const std::string at = std::string("personality.") + ids[i] + "." + module;
            const VALUE *mod = value_dict_get(p, module);
            if (strcmp(module, "favour") == 0) {       // kinds by their code: any key, a number each (P9)
                if (value_type(mod) != VALUE_DICT) { out.push_back({ false, at }); continue; }
                const VALUE *fkeys[256];
                const size_t fgot = value_dict_keys_sorted(mod, fkeys, std::min<size_t>(value_dict_size(mod), 256));
                for (size_t m = 0; m < fgot; m++)
                    if (!value_is_compatible(value_dict_get(mod, value_string(fkeys[m])), VALUE_DOUBLE))
                        out.push_back({ true, at + "." + value_string(fkeys[m]) });
                continue;
            }
            bool known_module = false;
            for (const Field &f : kFields) known_module |= (strcmp(f.module, module) == 0);
            if (!known_module || (value_type(mod) != VALUE_DICT)) {
                out.push_back({ false, at });
                continue;
            }
            const VALUE *mkeys[256];
            const size_t mgot = value_dict_keys_sorted(mod, mkeys, std::min<size_t>(value_dict_size(mod), 256));
            for (size_t m = 0; m < mgot; m++) {
                const char *key = value_string(mkeys[m]);
                const Field *field = nullptr;
                for (const Field &f : kFields) if ((strcmp(f.module, module) == 0) && (strcmp(f.key, key) == 0)) field = &f;
                if (field == nullptr) {
                    out.push_back({ false, at + "." + key });
                    continue;
                }
                const VALUE *v = value_dict_get(mod, key);
                if ((field->names == nullptr) || (value_type(v) != VALUE_STRING)) continue;
                bool named = false;
                for (int64_t j = 0; field->names[j] != nullptr; j++) named |= (strcmp(field->names[j], value_string(v)) == 0);
                if (!named) out.push_back({ true, at + "." + key + "=" + value_string(v) });
            }
        }
    }
    return out;
}

std::span<const DaiPersonality::Field> DaiPersonality::Fields()
{
    return std::span<const Field>(kFields);
}

const DaiPersonality::Field *DaiPersonality::FindField(const char *dotted)
{
    if (dotted == nullptr) return nullptr;
    const char *dot = strchr(dotted, '.');
    if (dot == nullptr) return nullptr;
    const std::string module(dotted, (size_t)(dot - dotted));
    for (const Field &f : kFields) if ((module == f.module) && (strcmp(dot + 1, f.key) == 0)) return &f;
    return nullptr;
}

bool DaiPersonality::ParseValue(const Field &f, const char *text, int64_t *out, std::string *error)
{
    if ((text == nullptr) || (text[0] == '\0')) { if (error) *error = "no value"; return false; }
    if (f.names != nullptr) {
        for (int64_t i = 0; f.names[i] != nullptr; i++)
            if (strcasecmp(f.names[i], text) == 0) { *out = i; return true; }
        if (error) *error = std::string("'") + text + "' is not one of its names";
        return false;
    }
    if (strcasecmp(text, "TRUE") == 0) { *out = 1 * f.scale; return true; }
    if (strcasecmp(text, "FALSE") == 0) { *out = 0; return true; }
    char *end = nullptr;
    if (f.scale > 1) {
        const double v = strtod(text, &end);
        if ((end == text) || (*end != '\0')) { if (error) *error = std::string("'") + text + "' is not a number"; return false; }
        *out = (int64_t)llround(v * (double)f.scale);
    } else {
        const long long v = strtoll(text, &end, 10);
        if ((end == text) || (*end != '\0')) { if (error) *error = std::string("'") + text + "' is not a whole number"; return false; }
        *out = (int64_t)v;
    }
    return true;
}

bool DaiPersonality::Set(const char *dotted, const char *text, int64_t *was, int64_t *now, std::string *error)
{
    const Field *f = FindField(dotted);
    if (f == nullptr) { if (error) *error = std::string("no personality key '") + (dotted ? dotted : "") + "'"; return false; }
    int64_t v = 0;
    if (!ParseValue(*f, text, &v, error)) return false;
    v = std::clamp(v, f->min, f->max);
    if (was != nullptr) *was = this->*(f->member);
    this->*(f->member) = v;
    if (now != nullptr) *now = v;
    return true;
}

DaiFix DaiPersonality::Favour(const char *code) const
{
    for (int64_t i = 0; i < m_nfavours; i++) if (strcmp(m_favours[(size_t)i].code, code) == 0) return m_favours[(size_t)i].weight;
    return DAI_FIX_ONE;
}

void DaiPersonality::SetFavour(const char *code, DaiFix weight)
{
    for (int64_t i = 0; i < m_nfavours; i++) {
        if (strcmp(m_favours[(size_t)i].code, code) != 0) continue;
        m_favours[(size_t)i].weight = weight;
        return;
    }
    if (m_nfavours >= MAX_FAVOURS) return;
    FavourEntry &e = m_favours[(size_t)m_nfavours++];
    snprintf(e.code, sizeof(e.code), "%s", code);
    e.weight = weight;
}

bool DaiPersonality::Resolve(const DaimonConfig &cfg, const char *name, DaiPersonality *out, std::string *error)
{
    const char *id = ((name == nullptr) || (name[0] == '\0')) ? "default" : name;
    // The chain from the one named down to "default" (which inherits nothing).
    std::vector<const VALUE *> chain;
    std::vector<std::string> seen;
    std::string at = id;
    while (true) {
        for (const std::string &s : seen) {
            if (s == at) {
                if (error) *error = "personality '" + std::string(id) + "' inherits in a circle through '" + at + "'";
                return false;
            }
        }
        if ((int64_t)seen.size() >= DAI_MAX_INHERIT_DEPTH) {
            if (error) *error = "personality '" + std::string(id) + "' inherits too deep";
            return false;
        }
        const VALUE *p = cfg.loaded ? daimon_config_personality(at.c_str()) : nullptr;
        if (p == nullptr) {
            if (at == "default") break; // no [personality.default]: the built-in defaults below the chain
            if (error) *error = "no personality '" + at + "'";
            return false;
        }
        chain.push_back(p);
        seen.push_back(at);
        if (at == "default") break;
        const VALUE *inh = value_dict_get(p, "inherits");
        at = ((inh != nullptr) && (value_type(inh) == VALUE_STRING)) ? value_string(inh) : "default";
    }
    DaiPersonality r;
    r.m_id = id;
    uint64_t h = 0xD41D0Fu;
    for (const Field &f : kFields) {
        int64_t v = f.dflt;
        const VALUE *val = find_in_chain(chain, f.module, f.key);
        if ((val != nullptr) && (f.names != nullptr)) {
            int64_t found = -1;
            if (value_type(val) == VALUE_STRING)
                for (int64_t i = 0; (found < 0) && (f.names[i] != nullptr); i++) if (strcmp(f.names[i], value_string(val)) == 0) found = i;
            if (found < 0) DaiLog::Warn(-1, "personality %s: %s.%s is not one of its names; using the default", id, f.module, f.key);
            else v = found;
        } else if (val != nullptr) {
            if ((f.scale != 1) && value_is_compatible(val, VALUE_DOUBLE)) {
                const double d = value_double(val) * (double)f.scale;
                v = (d >= 9.0e18) ? INT64_MAX : (d <= -9.0e18) ? INT64_MIN : (int64_t)llround(d);
                if ((v < f.min) || (v > f.max)) {
                    DaiLog::Warn(-1, "personality %s: %s.%s = %g is outside %g..%g; clamped", id, f.module, f.key,
                        value_double(val), (double)f.min / f.scale, (double)f.max / f.scale);
                    v = (v < f.min) ? f.min : f.max;
                }
            } else if (!value_is_compatible(val, VALUE_INT64)) {
                DaiLog::Warn(-1, "personality %s: %s.%s is not a %s; using the default", id, f.module, f.key,
                    (f.scale != 1) ? "number" : "whole number");
            } else {
                v = value_int64(val);
                if ((v < f.min) || (v > f.max)) {
                    DaiLog::Warn(-1, "personality %s: %s.%s = %" PRId64 " is outside %" PRId64 "..%" PRId64 "; clamped", id,
                        f.module, f.key, v, f.min, f.max);
                    v = (v < f.min) ? f.min : f.max;
                }
            }
        }
        r.*(f.member) = v;
        h = mix(h, v);
    }
    // The kinds favoured: down the chain from "default", a later one's weight for a kind over an earlier's (P9).
    for (auto it = chain.rbegin(); it != chain.rend(); ++it) {
        const VALUE *fav = value_dict_get(*it, "favour");
        if ((fav == nullptr) || (value_type(fav) != VALUE_DICT)) continue;
        const VALUE *keys[MAX_FAVOURS * 2];
        const size_t got = value_dict_keys_sorted(fav, keys, std::min<size_t>(value_dict_size(fav), MAX_FAVOURS * 2));
        for (size_t i = 0; i < got; i++) {
            const char *code = value_string(keys[i]);
            const VALUE *v = value_dict_get(fav, code);
            if ((v == nullptr) || !value_is_compatible(v, VALUE_DOUBLE)) {
                DaiLog::Warn(-1, "personality %s: favour.%s is not a number; left out", id, code);
                continue;
            }
            const int64_t w = std::clamp<int64_t>((int64_t)llround(value_double(v) * (double)DAI_FIX_ONE), 0, 4 * DAI_FIX_ONE);
            r.SetFavour(code, w);
        }
    }
    for (int64_t i = 0; i < r.m_nfavours; i++) {
        for (const char *ch = r.m_favours[(size_t)i].code; *ch != '\0'; ch++) h = mix(h, *ch);
        h = mix(h, r.m_favours[(size_t)i].weight);
    }
    r.m_hash = h;
    *out = r;
    return true;
}
