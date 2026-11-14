/** @file DaiPersonality.h
 *  A personality as a keeper uses it: every field resolved from daimon.toml's [personality.<id>] tables, through
 *  their `inherits` chain down to `default`, defaulted and clamped (02 section 4.14, 05 section 4). Decision classes
 *  read only this, never the file. Each field is listed once in Fields(), with the class that reads it: the consumer
 *  test fails on a shipped key that is in no list (lesson L5). */
#ifndef DAIMON_DAIPERSONALITY_H
#define DAIMON_DAIPERSONALITY_H

#include <array>
#include <cstdint>

#include "daimon/DaiFix.h"
#include <span>
#include <string>
#include <vector>

#include "daimon/DaiTypes.h"

struct DaimonConfig;

class DaiPersonality {
public:
    /** One personality key: [personality.<id>.<module>] <key>. A whole-number field has `scale` 1; a fixed-point one
     *  (DaiFix) has DAI_FIX_ONE and takes a decimal in the file (0.25), rounded once at load to the nearest 1/1024 --
     *  the same text gives the same value on every machine. `dflt`, `min` and `max` are in the stored units. */
    struct Field {
        const char *module;
        const char *key;
        int64_t DaiPersonality::*member;
        int64_t dflt, min, max;
        const char *reader;
        int64_t scale = 1;
        /** A name field: the file gives one of these names (nullptr-terminated), stored as its index. */
        const char *const *names = nullptr;
    };
    /** A key of a personality's table this build does not know: an unknown key (or module table), or an unknown name
     *  for a name field -- what a newer daimon.toml may hold (05 section 4.2). `what` names it in the file. */
    struct KeyIssue {
        bool value;             /**< an unknown name for a known key, not an unknown key */
        std::string what;       /**< "personality.<id>.<module>.<key>" (and "=<name>" for a value) */
    };

    // Phase 1 (03 section 6, 8.1): the wage reserve's margin over the wage bill (DaiFix); pay days the gold must
    // cover before Economy stops pressing for gold.
    int64_t economy_reserve_margin = 256;
    int64_t economy_comfortable_paydays = 3;
    // P9 (plan 13, DaiForecast): pay days' wages kept in hand (the rest is spare, to be put to work); a vein about as near
    // a rival's heart as its own within gold_contest_share % counts half its gold; extra rooms only while the income
    // lasts extras_horizon turns or more (or a gem is held).
    int64_t economy_bank_paydays = 4, economy_gold_contest_share = 20, economy_extras_horizon = 20000;
    // A work room (library, workshop, training) at grow_full % of its capacity grows with spare gold (plan 13).
    int64_t economy_grow_full = 90;
    // Phase 2 (04 section 8): sell rooms when the next pay day is short and nothing else helps (0/1); haul turns left on
    // a vein before a treasury near it is asked for.
    int64_t economy_sell_rooms_in_emergency = 1, economy_forward_treasury_turns = 6000;
    // P9: an imp dropped by a room's earth to fortify it (0 never), a room below fortify_below % efficiency, one drop
    // each fortify_interval turns at most.
    int64_t economy_fortify = 1, economy_fortify_below = 90, economy_fortify_interval = 400;
    // Phase 1 (03 section 7.1): the dig search's price of unknown ground (bold keepers explore, cautious ones keep to
    // known ground) and of gold, in turns a gold piece, DaiFix.
    int64_t excavation_unknown_cost = 30;
    int64_t excavation_gold_to_turns = 51;
    // Phase 1 (03 section 8.2): how far ahead a vein's gems are worth counting.
    int64_t excavation_horizon_turns = 20000;
    // P9 (plan 13): a vein about as near a rival's heart as its own scores contest_vein_weight times (DaiFix): its gold
    // first, before the rival takes it.
    int64_t excavation_contest_vein_weight = 1536;
    // Phase 1 (03 section 8.3): the imp count's bounds and rates; slapping (DaiFix share of working imps kept
    // hurried; 0 off, Q2) and how often.
    int64_t workforce_imps_min = 4, workforce_imps_max = 24, workforce_work_per_imp = 4, workforce_slabs_per_imp = 25;
    // Imps trained (P9): up to train_imps at a time, to imp_train_level (speed at 3), with imp_train_rich_runway pay
    // days of gold in hand to imp_train_rich_level (teleport at 10); the imps beyond imps_min only.
    int64_t workforce_train_imps = 2, workforce_imp_train_level = 3, workforce_imp_train_rich_level = 10,
            workforce_imp_train_rich_runway = 12;
    int64_t workforce_slap_share = 256, workforce_slap_interval = 200;
    // P9: tunnellers bought instead of imps when cheaper (1) or never (0), their pay counted over tunneller_paydays.
    int64_t workforce_tunnellers = 1, workforce_tunneller_paydays = 10;
    // The share (DaiFix) of what may be spent one imp above imps_min may take: the price rises with every imp (150 more
    // each), so the share is what stops the buying while the gold still covers the rest.
    int64_t workforce_imp_gold_share = 256;
    // Phase 2 (04 section 5.2): the layout planner's weights (DaiFix) on its score's terms -- predicted efficiency in
    // percent, 100 beside an anchor room, slabs from the anchor, turns of digging, enemy slabs near, earth to grow
    // into, valuable slabs walled in.
    int64_t layout_w_eff = 4 * 1024, layout_w_adj = 1024, layout_w_dist = 4 * 1024, layout_w_dig = 102, layout_w_exp = 0,
            layout_w_grow = 2 * 1024, layout_w_waste = 20 * 1024;

    // Phase 2 (04 section 4): the room programme -- each room's rank (1 first; 0: only when a manager asks for it) and
    // the slabs it starts at.
    int64_t programme_treasury_rank = 1, programme_treasury_slabs = 9;
    int64_t programme_lair_rank = 2, programme_lair_slabs = 16;
    int64_t programme_hatchery_rank = 3, programme_hatchery_slabs = 12;
    int64_t programme_library_rank = 4, programme_library_slabs = 20;
    int64_t programme_training_rank = 5, programme_training_slabs = 16;
    int64_t programme_workshop_rank = 6, programme_workshop_slabs = 17;
    int64_t programme_prison_rank = 0, programme_prison_slabs = 9;
    int64_t programme_torture_rank = 0, programme_torture_slabs = 9;
    int64_t programme_temple_rank = 0, programme_temple_slabs = 9;
    int64_t programme_graveyard_rank = 0, programme_graveyard_slabs = 9;
    int64_t programme_scavenger_rank = 0, programme_scavenger_slabs = 9;
    int64_t programme_barracks_rank = 0, programme_barracks_slabs = 9;
    int64_t programme_guard_post_rank = 0, programme_guard_post_slabs = 9;
    // Phase 2 (04 section 5.2, 6): where each room is placed -- beside the heart (0) or a room of the programme's
    // (1 + its place in DaiProgrammeRooms).
    int64_t programme_treasury_anchor = 0;
    int64_t programme_lair_anchor = 3;
    int64_t programme_hatchery_anchor = 2;
    int64_t programme_library_anchor = 0;
    int64_t programme_training_anchor = 2;
    int64_t programme_workshop_anchor = 0;
    int64_t programme_prison_anchor = 0;
    int64_t programme_torture_anchor = 0;
    int64_t programme_temple_anchor = 0;
    int64_t programme_graveyard_anchor = 0;
    int64_t programme_scavenger_anchor = 0;
    int64_t programme_barracks_anchor = 0;
    int64_t programme_guard_post_anchor = 0;
    // Phase 2 (04 section 7.4-7.5): the level creatures are trained to; which livid creatures are dismissed (a name
    // field: 0 never, 1 harmful, 2 always).
    int64_t creatures_train_target_level = 4, creatures_dismiss_livid = 1;
    // Training to train_rich_level (0: never past train_target_level) with train_rich_runway pay days of gold in hand;
    // training's worth to a creature against its other work, times train_weight (DaiFix): each level adds a third of
    // its kind's job values and most of its fight (P9).
    int64_t creatures_train_rich_level = 10, creatures_train_rich_runway = 6, creatures_train_weight = 1536;
    // P9 (plan 13): spare gold for train_spare_paydays pay days' wages (at least 1,000 a pay day) puts training first in
    // any posture but defend -- creatures taken from their work to train (DaiDirector, DaiCreatures).
    int64_t creatures_train_spare_paydays = 2;
    // And more by the share of its fight its next two levels' abilities add (DaiAbilities: a warlock's at level 1
    // doubles it), times train_ability_weight (DaiFix, P9).
    int64_t creatures_train_ability_weight = 1024;
    // Research worth research_rooms_weight (DaiFix) times its skill while a room is still to be researched (P9).
    int64_t creatures_research_rooms_weight = 2048;
    // Kinds drawn and kept (P9): a kind worth attract_upgrade (DaiFix) more than the best that can come already gets its
    // rooms built; at its creature limit, its weakest free creature dismissed (dismiss_weak 1; 0 never) for a kind that
    // can come worth dismiss_upgrade (DaiFix) more.
    int64_t creatures_attract_upgrade = 1536, creatures_dismiss_weak = 1, creatures_dismiss_upgrade = 2048;
    // A kind's work counted as work_worth a point of its best skill (research or making); at most one dismissal each
    // dismiss_interval turns (P9: some 50 a game, the portal sending the same kinds back).
    int64_t creatures_work_worth = 100, creatures_dismiss_interval = 2000;
    // Phase 6 (08 section 7a): how keen it is on scavenging (DaiFix: 0.5 weighs a lured creature at its worth; 0 never).
    int64_t creatures_scavenge_appetite = 307;
    // Phase 3 (05 section 6.2): the director's postures -- appetite for expanding (DaiFix: scales expand's score and
    // the ExpandTo weights), the share of its own strength a rival needs for contest and of its creatures lost for
    // recover (DaiFix), and the turns a posture is held before another may replace it.
    int64_t director_expand_appetite = 1024, director_contest_threshold = 819, director_recover_threshold = 512;
    int64_t director_dwell_turns = 1500;
    // Phase 4 (06 sections 7, 11): the cast rules Tactics generates for its fights, in the combat assistant's language
    // (DaiFightRules) -- which powers (1 yes, 0 no; heal, lightning and slow by default); heal below this health % at
    // heal_level; the hindering spells (slow, freeze, chicken on the strongest enemy on its own ground) and the helping
    // ones (speed, protect) when the enemy's force is above this % of its own; lightning to finish an enemy below this
    // health %, with none of its allies' creatures in reach; their overcharge level; and the gold: never spent under
    // the reserve (nor under the next wages plus it), at most max_spend a pay day for all of them together.
    int64_t tactics_cast_heal = 1, tactics_cast_lightning = 1, tactics_cast_slow = 1, tactics_cast_freeze = 0,
            tactics_cast_chicken = 0, tactics_cast_speed = 0, tactics_cast_protect = 0;
    int64_t tactics_heal_below = 35, tactics_heal_level = 2, tactics_hinder_ratio = 110, tactics_finish_below = 40;
    int64_t tactics_spell_level = 1, tactics_cast_reserve = 1000, tactics_cast_max_spend = 4000;
    // P9 (plan 13): with spare gold, the most the spells may spend in a pay day rises to cast_spare_share % of it.
    int64_t tactics_cast_spare_share = 50;
    // P9: Rebound, Flight and Vision on its strongest fighter as Speed and Protect are; Conceal on an imp caught in a
    // fight under conceal_imps_below health %.
    int64_t tactics_cast_rebound = 0, tactics_cast_flight = 0, tactics_cast_vision = 0, tactics_cast_conceal = 1;
    int64_t tactics_conceal_imps_below = 60;
    // P9: Disease on the strongest enemy in a fight on its own ground of disease_battle creatures at least, with
    // disease_reserve gold after the next wages (it costs thousands); never beside an ally's creatures.
    int64_t tactics_cast_disease = 0, tactics_disease_battle = 4, tactics_disease_reserve = 12000;
    // Phase 4 (06 sections 5.4, 11): the force wanted in a fight, as % of the enemy's, before no more is sent (DK2's
    // BAThreatSuperiority); how much more an engagement's stakes must be than the banner's before it moves (DaiFix).
    int64_t tactics_superiority_to_defend = 120, tactics_cta_switch_margin = 1536;
    // Phase 4 (06 sections 5.5, 11, Q3): fighters under this health % are lifted out of a fight (when no heal can
    // come); a fight whose enemy/own force % is above hopeless_ratio and that no help can turn is given up (its
    // fighters lifted home); Hold Audience when the heart's health % is below hold_audience_below (0: never).
    int64_t tactics_retreat_health = 25, tactics_hopeless_ratio = 300, tactics_hold_audience_below = 30;
    // Phase 4 (06 sections 5.4, 5a, 11): where the Call to Arms banner may go (a name field: 0 never, 1 own_ground, 2
    // anywhere an objective pays for it); the group it wants (more than 1: a barracks in the room programme); the flee
    // tendency kept on (1) or off (0).
    int64_t tactics_cta_use = 2, tactics_group_size = 1, tactics_flee_tendency = 1;
    // Phase 4 (06 section 11, defence): the weights Defence gives its objectives (DaiFix); the share of [defence]
    // max_traps and max_doors it plans (DaiFix 0..1); boulders allowed (1) or not.
    int64_t defence_heart_priority = 4 * 1024, defence_rooms_priority = 2 * 1024, defence_imps_priority = 1536;
    int64_t defence_trap_usage = 512, defence_door_preference = 512, defence_boulder_use = 1;
    // Phase 4 (06 sections 4.1, 11): guard posts beside the busiest chokepoints, and secret doors on them (1 yes, 0 no).
    int64_t defence_guard_posts = 0, defence_secret_doors = 0;
    int64_t defence_heart_traps = 2;    /**< last-ditch traps by its heart (P9; 0 none): DaiFortify's heart role */
    // P5.13 (07): a hazard's shots spent by a creature walking onto it (1), or never (0: sealed or left).
    int64_t defence_spend_traps = 1;
    // P9 (plan 13 L3): the share (%) of its free fighters kept at a guard post by the busiest chokepoint while the force
    // estimate is against it and the way in is under threat; 0 never.
    int64_t defence_hold_share = 30;
    // Plan 13 section 32: the perimeter -- the fewest slabs (at most perimeter_max_cut; 0: none) that, held, close off
    // perimeter_min_area slabs or more round its heart from the rest of the map (level 61's ring of rock).
    int64_t defence_perimeter_max_cut = 12, defence_perimeter_min_area = 150;
    // Phase 5 (07 sections 3.2-3.3, 5): never attack (1), or only a rival that has attacked it (1); the force wanted, as %
    // of a target's defence (DK2 IAThreatSuperiority), and the creatures' mean level, before attacking; the first target
    // (a name field: 0 heart, 1 room, 2 portal); the share of force kept home (DaiFix); the score a target needs (DaiFix).
    int64_t offence_never_attack = 0, offence_only_attack_attackers = 0, offence_superiority_to_attack = 150,
            offence_attack_level_target = 4, offence_first_target = 0, offence_home_guard_share = 307,
            offence_attack_threshold = 1024;
    // P9 (plan 13): with the income ending within [economy] extras_horizon turns (no gem), superiority_to_attack less
    // horizon_attack_bonus (at least 100): the rival's treasury is the next income.
    int64_t offence_horizon_attack_bonus = 20;
    // P9: the share (%) of a rival's estimated force (KnownForce, the panel's count included, and its growth) that comes
    // to defend whatever is attacked -- its heart, a portal, a room -- on top of the defence seen there.
    int64_t offence_response_share = 50;
    // Plan 13: the force (% of what an attack needs) at which it goes without the level and runway asks (0: always waits
    // for them); and the turns before its income's end (its side's gold mined) over which the asks relax -- the level
    // to 1, the runway to none, overwhelm_share to 100 % -- so that the attack goes with gold left for its spells (0: no
    // urgency), none in the game's first urgency_grace_paydays pay days (the sweep of 2026-11-09: on level 78, its gold
    // forecast to end within 4 pay days from turn 4,000, level-1 creatures went at a portal and the game was lost).
    int64_t offence_overwhelm_share = 150, offence_urgency_turns = 20000, offence_urgency_grace_paydays = 1;
    // Phase 5 (07 sections 4, 5; DaiOffence): the gold a 1000 turns an assault may spend on the banner's upkeep off the
    // keeper's ground; the enemy force, as % of its own there, past which an assault withdraws after withdraw_patience turns.
    int64_t offence_cta_attack_budget = 3000, offence_withdraw_ratio = 200, offence_withdraw_patience = 600;
    int64_t offence_loyalty = 717, offence_treachery = 102, offence_ally_min_value = 1024;     /**< fixed point (07 section 5a) */
    int64_t offence_raid_appetite = 512;    /**< fixed point: how often raids run, raid_interval / it turns between two (0: never) */
    // Phase 6 (08 sections 4-6, 8, 17): prisoners taken (the imprison tendency, a prison asked for) or not; each prisoner
    // plan's value scaled (DaiFix) -- convert, interrogate, skeleton, ghost, hold; a victim of its most-paid kind kept on
    // a rack for the pay discount (1) or not; discipline of a livid creature instead of dismissal (a name field: 0 none,
    // 1 prison, 2 torture, 3 both), held while its cause is fixed within discipline_hold turns.
    // P9 (plan 13): each plan pays its creature's upkeep (pay, as force at the keeper's force per gold) times how short
    // the gold is (the income's horizon; none with a gem held) times w_upkeep (DaiFix).
    int64_t custody_w_upkeep = 1024;
    int64_t custody_take_prisoners = 1, custody_w_convert = 1024, custody_w_interrogate = 1024, custody_w_skeleton = 1024,
            custody_w_ghost = 1024, custody_w_hold = 1024, custody_pay_discount_torture = 1, custody_discipline = 0,
            custody_discipline_hold = 1500;
    int64_t custody_sacrifices = 2;     /**< a name field: 0 none, 1 prisoners, 2 any (08 section 7.2, Q3) */
    int64_t custody_vampires = 1;       /**< a graveyard for the bodies its fights leave (P6.6) */
    int64_t custody_w_body = 1024;      /**< DaiFix: a body's worth to its graveyard (a share of a vampire), against a
                                         *   prisoner's, when it chooses which its fights leave (P9) */
    int64_t custody_rescue_value = 1024;    /**< DaiFix: its held creatures' force counted for a jailbreak (P6.9; 0 never) */
    // P9 (DaiMagic): Must Obey (a name field: 0 never, 1 for its fights, 2 for its fights and its work); on only with
    // obey_reserve gold after the next wages, off below obey_floor; a fight is obey_fighters of its creatures fighting,
    // work obey_workers of them researching, manufacturing or training; switched at most once every obey_hold turns
    // (the floor aside).
    int64_t magic_obey = 1, magic_obey_reserve = 4000, magic_obey_floor = 1500, magic_obey_fighters = 6,
            magic_obey_workers = 8, magic_obey_hold = 300;
    // P9 (DaiMagic): special boxes used (1) or left (0); heal all once heal_all_hurt creatures are under heal_all_below %.
    int64_t magic_specials = 1, magic_heal_all_below = 60, magic_heal_all_hurt = 3;
    // P9 (plan 13; the built-in computer's SplSpdu): Speed Up on its creatures at work, training first, then research while
    // a room is left to research, then manufacture -- one not sped up already, the lowest level first -- while it has
    // spare gold for work_speed_paydays pay days' wages (0 never).
    int64_t magic_work_speed = 1, magic_work_speed_paydays = 2;
    // P9 (DaiMagic): Armageddon (a name field: 0 never, 1 finisher) -- cast once its creatures' levels are
    // armageddon_superiority % of every rival keeper's estimated together (DaiMagic::RivalsForce), its heart's health at
    // armageddon_heart % or more, armageddon_traps of its traps within ARMAGEDDON_TRAP_REACH of the heart, and the gold
    // for the price and armageddon_reserve more (the final battle's spells; an end-game move, the wages not counted).
    // After anyone's Armageddon, until armageddon_finish_turns after it is over, the director assaults the best
    // target's keeper once its creatures' levels are armageddon_superiority % of that keeper's, seen whole (DaiDirector).
    int64_t magic_armageddon = 0, magic_armageddon_superiority = 133, magic_armageddon_heart = 80, magic_armageddon_traps = 2,
            magic_armageddon_reserve = 15000, magic_armageddon_finish_turns = 3000;
    // Armageddon only with gold plentiful (owner, plan 13): its price at most armageddon_gold_share % of the gold it can
    // still have, its bank and its side's gold left (no limit with a gem held).
    int64_t magic_armageddon_gold_share = 15;
    /** A room's anchor: the room it should be beside, or DaiRoom::Count for the heart. */
    DaiRoom ProgrammeAnchor(DaiRoom r) const;
    /** A room's rank and starting slabs (0, 0 for a room the programme has no entry for: bridges, portals). */
    int64_t ProgrammeRank(DaiRoom r) const;
    int64_t ProgrammeSlabs(DaiRoom r) const;

    /** Every field, in file order. */
    static std::span<const Field> Fields();
    /** Every personality's keys checked against Fields() and the header keys (daimon_personality_header_keys). */
    static std::vector<KeyIssue> CheckKeys(const DaimonConfig &cfg);
    /** The personality `name` (NULL or empty: "default"), resolved; false with a reason in *error (unknown name,
     *  a cycle or too deep an inheritance chain). Out-of-range values are clamped and logged, not refused. */
    static bool Resolve(const DaimonConfig &cfg, const char *name, DaiPersonality *out, std::string *error);

    const std::string &GetId() const { return m_id; }
    /** A saved keeper's personality: the id and hash it was saved with (its values are set field by field). */
    void SetIdentity(const std::string &id, uint64_t hash) { m_id = id; m_hash = hash; }
    /** DAIMON_SET / DaimonSet (Phase 7, 09 section 4): a key by its "module.key" name (lower case) and a value as the
     *  script writes it -- a number, a decimal for a decimal key, TRUE/FALSE, or one of a name key's names --
     *  clamped to the key's range. False (and *error) for an unknown key or a value it cannot read. */
    static const Field *FindField(const char *dotted);
    static bool ParseValue(const Field &f, const char *text, int64_t *out, std::string *error);
    bool Set(const char *dotted, const char *text, int64_t *was, int64_t *now, std::string *error);
    /** A hash of the resolved values, saved with the keeper (05 section 4.3): a load can tell the file changed. */
    uint64_t SourceHash() const { return m_hash; }

    /** Kinds it favours (P9, [personality.<id>.favour]: a kind's code = its weight), DaiFix: how much more (or less) a
     *  creature of the kind is worth to it -- the rooms it builds for one, the creatures it keeps. 1.0 unless named. */
    static constexpr int64_t MAX_FAVOURS = 16;
    struct FavourEntry {
        char code[32] = {};
        DaiFix weight = DAI_FIX_ONE;
    };
    DaiFix Favour(const char *code) const;
    void SetFavour(const char *code, DaiFix weight);
    int64_t Favours() const { return m_nfavours; }
    const FavourEntry &FavourAt(int64_t i) const { return m_favours[(size_t)i]; }
private:
    std::array<FavourEntry, MAX_FAVOURS> m_favours{};
    int64_t m_nfavours = 0;
    std::string m_id = "default";
    uint64_t m_hash = 0;
};

#endif
