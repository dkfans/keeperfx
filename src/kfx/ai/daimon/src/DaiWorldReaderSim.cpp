/** @file DaiWorldReaderSim.cpp
 *  IDaiWorldReader over the simulation (DaiWorldReaderSim.h).
 *
 *  Read-only on the simulation (02 principle 8). Daimon runs on one machine, so a write here -- even a cache a query
 *  refreshes -- would exist on that machine alone. Not to be called from here:
 *    - navigation_points_connected(): rebuilds region connectivity inside the saved navigation state;
 *    - creature_can_navigate_to_with_storage(): overwrites the creature's route;
 *    - affordable_price(): records a refusal;
 *    - extseat_subtiles_connected() / creature_can_navigate_to() (the keeper controller's defaults): Daimon installs
 *      its own KeeperCtlWorld instead.
 *  The dai_knowledge_readonly ftest hashes the simulation state around a full update to keep it so. */
#include "pre_inc.h"
#include "daimon/DaiWorldReaderSim.h"
#include "daimon/DaiAbilities.h"

#include <algorithm>
#include <cstring>

#include "config_creature.h"
#include "config_daimon.h"
#include "config_effects.h"
#include "config_rules.h"
#include "config_magic.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "creature_battle.h"
#include "creature_control.h"
#include "creature_jobs.h"
#include "creature_states.h"
#include "creature_states_gardn.h"
#include "creature_states_mood.h"
#include "creature_states_prisn.h"
#include "creature_states_tortr.h"
#include "dungeon_data.h"
#include "keeper_facts.h"
#include "keeper_exclusion.h"
#include "keeper_group.h"
#include "keeper_visibility.h"
#include "list_walk.h"
#include "game_legacy.h"
#include "game_legacy.h"
#include "magic_powers.h"
#include "thing_doors.h"
#include "map_columns.h"
#include "map_data.h"
#include "config_creature.h"
#include "config_magic.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "room_workshop.h"
#include "player_data.h"
#include "room_data.h"
#include "room_library.h"
#include "slab_data.h"
#include "tasks_list.h"
#include "thing_data.h"
#include "thing_effects.h"
#include "thing_list.h"
#include "thing_objects.h"
#include "thing_traps.h"
#include "thing_creature.h"
#include "thing_stats.h"
#include "keeper_assist.h"
#include "power_hand.h"
#include "power_specials.h"
#include "power_process.h"
#include "daimon/DaiGeom.h"
#include "daimon_engine.h"
#include "player_instances.h"
#include "post_inc.h"

// Daimon's names for the game's numbers (DaiTypes.h) must be the game's.
static_assert(DAI_ROLE_HEART == RoRoF_KeeperStorage && DAI_ROLE_LAIR == RoRoF_LairStorage && DAI_ROLE_GOLD == RoRoF_GoldStorage
    && DAI_ROLE_FOOD == RoRoF_FoodStorage && DAI_ROLE_CRATES == RoRoF_CratesStorage && DAI_ROLE_SPELLBOOKS == RoRoF_PowersStorage
    && DAI_ROLE_PRISON == RoRoF_Prison && DAI_ROLE_GRAVEYARD == RoRoF_DeadStorage && DAI_ROLE_PORTAL == RoRoF_CrPoolSpawn
    && DAI_ROLE_SACRIFICE == RoRoF_CrSacrifice && DAI_ROLE_FOOD_SPAWN == RoRoF_FoodSpawn
    && DAI_ROLE_MANUFACTURE == RoRoF_CratesManufctr && DAI_ROLE_RESEARCH == RoRoF_Research && DAI_ROLE_TORTURE == RoRoF_Torture
    && DAI_ROLE_PRAY == RoRoF_CrHappyPray && DAI_ROLE_HEAL_SLEEP == RoRoF_CrHealSleep && DAI_ROLE_SCAVENGE == RoRoF_CrScavenge
    && DAI_ROLE_TRAIN == RoRoF_CrTrainExp && DAI_ROLE_GROUP == RoRoF_CrMakeGroup && DAI_ROLE_GUARD == RoRoF_CrGuard
    && DAI_ROLE_PASS_WATER == RoRoF_PassWater && DAI_ROLE_PASS_LAVA == RoRoF_PassLava, "DAI_ROLE_* differ from RoRoF_*");
static_assert((int)DaiActivity::Count == CrStTyp_ListEnd && (int)DaiActivity::Fight == CrStTyp_FightCrtr
    && (int)DaiActivity::Salary == CrStTyp_GetsSalary && (int)DaiActivity::DeepWork == CrStTyp_DeepWork, "DaiActivity differs from CrStTyp_*");
static_assert((int)DaiResearchKind::Power == RsCat_Power && (int)DaiResearchKind::Room == RsCat_Room
    && (int)DaiResearchKind::Creature == RsCat_Creature, "DaiResearchKind differs from RsCat_*");

namespace {
bool player_ok(PlayerNumber p)
{
    return (p >= 0) && (p < PLAYERS_COUNT);
}

/** A creature that digs for its keeper as an imp does but is on its creature list, not its diggers' (a tunneller from
 *  POWER_TUNNELLER: DIGGING_CREATURE, not SPECIAL_DIGGER; imp_birth puts it to the imps' work). */
bool digs_as_creature(const struct Thing *thing)
{
    return thing_is_creature_digger(thing) && !creature_is_for_dungeon_diggers_list(thing);
}

bool slab_on_map(DaiSlabCoord x, DaiSlabCoord y)
{
    return (x >= 0) && (y >= 0) && (x < game.map_tiles_x) && (y < game.map_tiles_y);
}

/** What the slab is, from its kind and the terrain config (so a mod's slab kinds classify too). */
DaiTerrain classify(const struct SlabMap *slb)
{
    const SlabKind kind = slabmap_kind(slb);
    if (slb->room_index > 0) return DaiTerrain::Room;
    const struct SlabConfigStats *st = get_slab_kind_stats(kind);
    switch (st->wlb_type) {
    case WlbT_Lava: return DaiTerrain::Lava;
    case WlbT_Water: return DaiTerrain::Water;
    case WlbT_Abyss: return DaiTerrain::Abyss;
    default: break;
    }
    if (kind == SlbT_GOLD) return DaiTerrain::Gold;
    if (kind == SlbT_DENSEGOLD) return DaiTerrain::DenseGold;
    if (kind == SlbT_GEMS) return DaiTerrain::Gem;
    if (slab_kind_is_door(kind)) return DaiTerrain::Door;
    if (st->indestructible) return DaiTerrain::Rock;
    switch (st->category) {
    case SlbAtCtg_FriableDirt: return DaiTerrain::Dirt;
    case SlbAtCtg_FortifiedWall: return DaiTerrain::Wall;
    case SlbAtCtg_Unclaimed:
    case SlbAtCtg_FortifiedGround:
    case SlbAtCtg_RoomInterior: return DaiTerrain::Floor;
    default: return DaiTerrain::Impassable;
    }
}

int64_t turns_to_payday(PlayerNumber p)
{
    const int64_t gap = (int64_t)game.conf.rules[p].gameplay.pay_day_gap;
    const int64_t per_turn = (int64_t)game.conf.rules[p].gameplay.pay_day_speed / 100;
    const int64_t progress = (int64_t)game.pay_day_progress[p];
    if ((per_turn <= 0) || (gap <= progress)) return 0;
    return (gap - progress + per_turn - 1) / per_turn;
}

/** The room panel's numbers, its slabs' box and its open sides (04 section 3). */
void room_quality(const struct Room *room, DaiRoomLook *r)
{
    r->roles = (uint64_t)get_room_roles(room->kind);
    r->efficiency = room->efficiency;
    r->health = (int64_t)room->health;
    r->max_health = (int64_t)compute_room_max_health(room->slabs_count, room->efficiency);
    r->stored = (int64_t)room->capacity_used_for_storage;
    if ((r->roles & DAI_ROLE_LAIR) != 0) {
        for (int64_t m = 1; m < CREATURE_TYPES_MAX; m++) r->occupants += room->content_per_model[m];
    } else {
        for (int64_t i = room->creatures_list, guard = 0; (i > 0) && (guard < THINGS_COUNT); guard++) {
            const struct Thing *thing = thing_get((DaiThingIndex)i);
            if (thing_is_invalid(thing) || !thing_is_creature(thing)) break;
            r->occupants++;
            i = creature_control_get_from_thing(thing)->next_in_room;
        }
    }
    const int *const dx = DAI_N4_DX, *const dy = DAI_N4_DY;
    const int64_t synergy = get_room_kind_stats(room->kind)->synergy_slab;
    bool first = true;
    SlabCodedCoords sn = room->slabs_list;
    for (int64_t guard = 0; (sn != 0) && (guard < room->slabs_count); guard++) {
        const DaiSlabCoord x = slb_num_decode_x(sn), y = slb_num_decode_y(sn);
        if (first) {
            r->ax = x; r->ay = y;
            r->box = DaiRect{ x, y, x, y };
            first = false;
        }
        if (x < r->box.x0) r->box.x0 = x;
        if (y < r->box.y0) r->box.y0 = y;
        if (x > r->box.x1) r->box.x1 = x;
        if (y > r->box.y1) r->box.y1 = y;
        for (int d = 0; d < 4; d++) {
            const DaiSlabCoord nx = (DaiSlabCoord)(x + dx[d]), ny = (DaiSlabCoord)(y + dy[d]);
            if (!slab_on_map(nx, ny)) continue;
            if (room_slab_side_score(sn, get_slab_number(nx, ny), synergy, NULL) == 0) r->open_sides++;
        }
        sn = get_next_slab_number_in_room(sn);
    }
}

bool in_area(const DaiRect &a, DaiSubtlCoord x, DaiSubtlCoord y)
{
    const DaiSlabCoord sx = DaiSubtileSlab(x), sy = DaiSubtileSlab(y);
    return (sx >= a.x0) && (sx <= a.x1) && (sy >= a.y0) && (sy <= a.y1);
}
}

DaiWorldReaderSim::DaiWorldReaderSim() = default;

bool DaiWorldReaderSim::creature_seen(const struct Thing *th, PlayerNumber p) const
{
    if (keeper_creature_visible_to(th, p)) return true;
    return power_sight_explored(th->mappos.x.stl.num, th->mappos.y.stl.num, p)
        && !(players_are_enemies(p, th->owner) && creature_is_invisible(th));
}

bool DaiWorldReaderSim::place_seen(const struct Thing *th, PlayerNumber p) const
{
    return thing_revealed(th, p) || power_sight_explored(th->mappos.x.stl.num, th->mappos.y.stl.num, p);
}

DaiWorldReaderSim::~DaiWorldReaderSim()
{
    Reset();
}

void DaiWorldReaderSim::Reset()
{
    for (auto &t : m_trackers) {
        if (t) keeper_facts_free(t.get());
        t.reset();
    }
    for (auto &m : m_marks) m.reset();
    m_marks_size.fill(0);
    m_sees_all.fill(false);
    m_knows_pool.fill(false);
}

void DaiWorldReaderSim::audit(bool visible) const
{
    if (!m_audit_on) return;
    m_audit.checked++;
    if (!visible) m_audit.hidden++;
}

void DaiWorldReaderSim::MapSize(DaiSlabCoord *w, DaiSlabCoord *h) const
{
    *w = game.map_tiles_x;
    *h = game.map_tiles_y;
}

bool DaiWorldReaderSim::marked(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y) const
{
    const int64_t w = game.map_tiles_x, h = game.map_tiles_y;
    const DaiTurn now = get_gameturn();
    if (!m_marks[p] || (m_marks_size[p] != w * h) || (m_marks_turn[p] != now + 1)) {
        if (m_marks_size[p] != w * h) {
            m_marks[p] = std::make_unique<uint8_t[]>((size_t)(w * h));
            m_marks_size[p] = w * h;
        }
        std::memset(m_marks[p].get(), 0, (size_t)(w * h));
        const struct Dungeon *dungeon = get_players_num_dungeon(p);
        if (!dungeon_invalid(dungeon)) {
            const int64_t imax = (dungeon->highest_task_number < MAPTASKS_COUNT) ? dungeon->highest_task_number : MAPTASKS_COUNT;
            for (int64_t i = 0; i < imax; i++) {
                const struct MapTask *mt = &dungeon->task_list[i];
                if (mt->kind == 0) continue;
                const int64_t sx = DaiSubtileSlab(stl_num_decode_x(mt->coords)), sy = DaiSubtileSlab(stl_num_decode_y(mt->coords));
                if ((sx >= 0) && (sx < w) && (sy >= 0) && (sy < h)) m_marks[p][(size_t)(sy * w + sx)] = 1;
            }
        }
        m_marks_turn[p] = now + 1;
    }
    return m_marks[p][(size_t)(y * w + x)] != 0;
}

void DaiWorldReaderSim::Slab(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y, DaiSlabLook *out) const
{
    *out = DaiSlabLook();
    if (!player_ok(p) || !slab_on_map(x, y)) return;
    // Its own dig marks, seen or not: a player sees where they marked (never what the mark found: the task's kind).
    out->marked = marked(p, x, y);
    const bool revealed = sees_all(p) || keeper_slab_revealed(p, x, y);
    if (!revealed) return;                            // Unknown: no kind, owner or room of a slab it has not seen
    const struct SlabMap *slb = get_slabmap_block(x, y);
    out->revealed = true;
    out->terrain = classify(slb);
    out->owner = (PlayerNumber)slabmap_owner(slb);
    if (out->owner == game.neutral_player_num) out->owner = -1;
    if (slb->room_index > 0) {
        const struct Room *room = room_get(slb->room_index);
        if (!room_is_invalid(room)) out->room = room->kind;
    }
    if ((out->terrain == DaiTerrain::Gold) || (out->terrain == DaiTerrain::DenseGold)) {
        // As digging pays it out (calculate_gold_digged_out_of_slab_with_single_hit): the slab's health in hits.
        const struct SlabConfigStats *st = get_slab_kind_stats(slabmap_kind(slb));
        const int64_t full = game.block_health[st->block_health_index];
        out->gold_left = (full > 0) ? ((int64_t)slb->health * st->gold_held) / full : st->gold_held;
    }
    if ((out->owner != p) && !sees_all(p)) audit(keeper_slab_revealed(p, x, y));
}

void DaiWorldReaderSim::Own(PlayerNumber p, DaiOwnLook *out) const
{
    *out = DaiOwnLook();
    if (!player_ok(p)) return;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return;
    out->gold = dungeon->total_money_owned;
    out->imps = dungeon->num_active_diggers;
    for (int64_t i = dungeon->creatr_list_start, guard = 0; (i > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *thing = thing_get((DaiThingIndex)i);
        if (thing_is_invalid(thing) || !thing_is_creature(thing)) break;
        const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl)) break;
        i = cctrl->players_next_creature_idx;
        if (digs_as_creature(thing)) out->tunnellers++;
    }
    out->imps += out->tunnellers;
    out->creatures = dungeon->num_active_creatrs;
    out->wage_bill = dungeon->creatures_total_pay;
    out->turns_to_payday = turns_to_payday(p);
    out->payday_progress = (int64_t)game.pay_day_progress[p];
    out->payday_gap = (int64_t)game.conf.rules[p].gameplay.pay_day_gap;
    out->payday_speed = (int64_t)game.conf.rules[p].gameplay.pay_day_speed;
    out->offmap_gold = dungeon->offmap_money_owned;
    out->gold_mined = (int64_t)dungeon->lvstats.gold_mined;
    out->imp_gold_hold = creature_stats_get(get_players_special_digger_model(p))->gold_hold;
    out->food_generation_speed = game.conf.rules[p].rooms.food_generation_speed;
    // In gold: a treasury's capacities count hoard sizes, each holding gold_per_hoard / the sizes (count_gold_hoardes_in_room).
    const int64_t sizes = get_wealth_size_types_count();
    const int64_t gold_a_size = (sizes > 0) ? (int64_t)game.conf.rules[p].gameplay.gold_per_hoard / sizes : 0;
    for (int64_t ridx = dungeon->room_list_start[RoK_TREASURE], guard = 0; (ridx > 0) && (guard < ROOMS_COUNT); guard++) {
        const struct Room *room = room_get((RoomIndex)ridx);
        if (room_is_invalid(room)) break;
        out->treasury_capacity += gold_a_size * (int64_t)room->total_capacity;
        out->treasury_used += (int64_t)room->capacity_used_for_storage;
        ridx = room->next_of_owner;
    }
    // Gold in its imps' pockets, and loose piles on its own ground (both counted only once a treasury takes them).
    for (int64_t i = dungeon->digger_list_start, guard = 0; (i > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *imp = thing_get((DaiThingIndex)i);
        if (thing_is_invalid(imp)) break;
        out->gold_carried += imp->creature.gold_carried;
        const struct CreatureControl *cctrl = creature_control_get_from_thing(imp);
        if (creature_control_invalid(cctrl)) break;
        i = cctrl->players_next_creature_idx;
    }
    for (int64_t i = game.thing_lists[TngList_Objects].index, guard = 0; (i > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *obj = thing_get((DaiThingIndex)i);
        if (thing_is_invalid(obj)) break;
        if (object_is_gold_pile(obj)) {
            const struct SlabMap *slb = get_slabmap_for_subtile(obj->mappos.x.stl.num, obj->mappos.y.stl.num);
            if (slabmap_owner(slb) == p) out->loose_gold += obj->valuable.gold_stored;
        }
        i = obj->next_of_class;
    }
    const struct Thing *heart = find_players_dungeon_heart(p);
    if (!thing_is_invalid(heart)) {
        out->has_heart = true;
        out->heart_x = heart->mappos.x.stl.num;
        out->heart_y = heart->mappos.y.stl.num;
        out->heart_health = heart->health;
        out->heart_max_health = get_thing_max_health(heart);
    }
    out->cta_active = player_uses_power_call_to_arms(p);
    out->cta_paying = player_call_to_arms_is_charged(p);
    out->obey_active = player_uses_power_obey(p);
    if (game.armageddon_cast_turn != 0) {
        const PlayerNumber caster = game.armageddon_caster_idx;
        out->armageddon = true;
        out->armageddon_caster = caster;
        out->armageddon_teleport = game.armageddon_cast_turn
            + (player_ok(caster) ? game.conf.rules[caster].magic.armageddon_count_down : 0);
        out->armageddon_over = game.armageddon_over_turn;
    }
    {
        const struct PowerConfigStats *obey = get_power_model_stats(PwrK_OBEY);
        out->obey_upkeep_per_kturn = (obey->duration > 0) ? obey->cost[0] * 1000 / obey->duration : 0;
    }
    out->flee_tendency = (dungeon->creature_tendencies & CrTend_Flee) != 0;
    out->imprison_tendency = (dungeon->creature_tendencies & CrTend_Imprison) != 0;
    const struct KeeperGroupState *g = keeper_group_get(p);
    if (g != NULL) {
        out->group_count = g->count;
        out->group_order = g->order;
        out->group_leader = (g->count > 0) ? g->member[0] : 0;
    }
    out->group_max = std::min<int64_t>(game.conf.rules[p].rooms.barrack_max_party_size, KEEPER_GROUP_MAX);
}

int64_t DaiWorldReaderSim::OwnRooms(PlayerNumber p, DaiRoomLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return 0;
    int64_t n = 0;
    for (int64_t kind = 1; kind < TERRAIN_ITEMS_MAX; kind++) {
        int64_t ridx = dungeon->room_list_start[kind];
        for (int64_t guard = 0; (ridx > 0) && (guard < ROOMS_COUNT); guard++) {
            const struct Room *room = room_get((RoomIndex)ridx);
            if (room_is_invalid(room)) break;
            ridx = room->next_of_owner;
            if (n < max) {
                DaiRoomLook &r = out[n];
                r = DaiRoomLook();
                r.index = room->index;
                r.kind = room->kind;
                r.slabs = room->slabs_count;
                r.capacity = room->total_capacity;
                r.used = room->used_capacity;
                r.cx = DaiSubtileSlab(room->central_stl_x);
                r.cy = DaiSubtileSlab(room->central_stl_y);
                room_quality(room, &r);
            }
            n++;
        }
    }
    return (n < max) ? n : max;
}

int64_t DaiWorldReaderSim::OwnRoomAt(PlayerNumber p, DaiSlabCoord x, DaiSlabCoord y) const
{
    if (!player_ok(p) || !slab_on_map(x, y)) return 0;
    const struct Room *room = room_get(get_slabmap_block(x, y)->room_index);
    return (!room_is_invalid(room) && (room->owner == p)) ? room->index : 0;
}

int64_t DaiWorldReaderSim::OwnCreatures(PlayerNumber p, DaiCreatureLook *out, int64_t max, int64_t from) const
{
    if (!player_ok(p)) return 0;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return 0;
    int64_t n = 0, at = 0;
    for (int64_t i = dungeon->creatr_list_start, guard = 0; (i > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *thing = thing_get((DaiThingIndex)i);
        if (thing_is_invalid(thing) || !thing_is_creature(thing)) break;
        const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl)) break;
        i = cctrl->players_next_creature_idx;
        if (digs_as_creature(thing)) continue;      // a tunneller works as an imp: OwnImps lists it (P9)
        if (at++ < from) continue;
        DaiCreatureLook &o = out[n++];
        o = DaiCreatureLook();
        o.ref.idx = thing->index;
        o.ref.creation_turn = thing->creation_turn;
        o.model = thing->model;
        o.level = cctrl->exp_level + 1;
        o.homeless = (cctrl->lair_room_id == 0);
        o.health = thing->health;
        o.health_pct = (cctrl->max_health > 0) ? (100 * thing->health / cctrl->max_health) : 100;
        o.force = creature_force_score(thing);
        o.battle = cctrl->battle_id;
        o.x = thing->mappos.x.stl.num;
        o.y = thing->mappos.y.stl.num;
        o.state = get_creature_state_besides_interruptions(thing);
        const int64_t type = get_creature_state_type(thing);
        o.activity = ((type >= 0) && (type < (int64_t)DaiActivity::Count)) ? (DaiActivity)type : DaiActivity::Idle;
        const struct Room *room = get_room_thing_is_on(thing);
        if (!room_is_invalid(room) && (room->owner == p)) {
            o.in_room = room->kind;
            const struct RoomRulesConfig *rr = &game.conf.rules[p].rooms;
            if ((o.activity == DaiActivity::Work) && room_role_matches(room->kind, RoRoF_CrTrainExp) && (rr->train_cost_frequency > 0))
                o.upkeep_per_kturn = calculate_correct_creature_training_cost(thing) * 1000 / rr->train_cost_frequency;
            if ((o.activity == DaiActivity::Work) && room_role_matches(room->kind, RoRoF_CrScavenge))
                o.upkeep_per_kturn = calculate_correct_creature_scavenging_cost(thing) * 1000 / (rr->scavenge_cost_frequency + 1);
        }
        if (anger_is_creature_livid(thing)) o.anger = DaiAnger::Livid;
        else if (anger_is_creature_angry(thing)) o.anger = DaiAnger::Angry;
        if (o.anger != DaiAnger::Calm) {
            switch (anger_get_creature_anger_type(thing)) {
            case AngR_NotPaid: o.anger_reason = DaiAngerReason::NotPaid; break;
            case AngR_Hungry: o.anger_reason = DaiAngerReason::Hungry; break;
            case AngR_NoLair: o.anger_reason = DaiAngerReason::NoLair; break;
            case AngR_Other: o.anger_reason = DaiAngerReason::Other; break;
            default: break;
            }
        }
        o.fleeing = creature_is_fleeing_combat(thing);
        if (creature_is_kept_in_custody_by_player(thing, p)) o.held_by = p;
        else if (creature_is_kept_in_custody_by_enemy(thing) && !room_is_invalid(room)) o.held_by = room->owner;
        if (o.held_by >= 0) o.orderable = false;    // jailed or racked: no order takes it
        o.scavenged = creature_is_being_scavenged(thing);
        o.scavenge_work = compute_creature_work_value_for_room_role(thing, RoRoF_CrScavenge, ROOM_EFFICIENCY_MAX);
        o.scavenge_required = calculate_correct_creature_scavenge_required(thing, p);
        o.diseased = creature_under_spell_effect(thing, CSAfF_Disease);
        o.sacrificing = creature_is_being_sacrificed(thing);
        o.chickened = creature_under_spell_effect(thing, CSAfF_Chicken);
        o.sped = creature_under_spell_effect(thing, CSAfF_Speed);
        o.orderable = !keeper_order_near_hunger_limit(thing) && !keeper_order_owed_pay(thing);
        const struct CreatureModelConfig *crconf = creature_stats_get_from_thing(thing);
        if (creature_can_gain_experience(thing) && (crconf->to_level[cctrl->exp_level] > 0)) {
            const int64_t pct = (int64_t)(cctrl->exp_points >> 8) * 100 / (int64_t)crconf->to_level[cctrl->exp_level];
            o.exp_pct = (pct < 0) ? 0 : (pct > 99) ? 99 : pct;
        }
    }
    return n;
}

int64_t DaiWorldReaderSim::KindCount() const
{
    return game.conf.crtr_conf.model_count;
}

void DaiWorldReaderSim::Kind(PlayerNumber p, DaiThingModel m, DaiKindLook *out) const
{
    *out = DaiKindLook();
    out->model = m;
    if ((m <= 0) || (m > KindCount()) || (m >= CREATURE_TYPES_MAX)) return;
    const struct CreatureModelConfig *c = creature_stats_get(m);
    if (creature_stats_invalid(c)) return;
    out->known = true;
    for (int64_t lv = 0; (lv < DaiKindLook::LEVELS) && (lv < CREATURE_MAX_LEVEL); lv++) out->score[lv] = game.creature_scores[m].value[lv];
    {
        DaiAbilitySpec specs[DAI_ABILITY_MAX];
        const int64_t n = DaiAbilitySpecsOfKind(m, specs, DAI_ABILITY_MAX);
        static_assert(DaiKindLook::LEVELS == DAI_ABILITY_LEVELS, "DaiKindLook::LEVELS differs from DAI_ABILITY_LEVELS");
        DaiRebirth rb;
        rb.level = c->rebirth;
        rb.share = daimon_config.global.abilities_rebirth_value;
        DaiAbilityTable(specs, n, out->output, out->ability_gain, rb);
        out->magic_share = DaiAbilityMagicShare(specs, n, 5);
        out->rebirth = c->rebirth;
        out->alluring = c->entrance_force;
    }
    out->digger = (c->model_flags & CMF_IsSpecDigger) != 0;
    snprintf(out->code, sizeof(out->code), "%s", creature_code_name(m));
    out->tunneller = !out->digger && ((c->model_flags & CMF_IsDiggingCreature) != 0);
    out->speed = c->base_speed;
    out->spectator = (c->model_flags & CMF_IsSpectator) != 0;
    out->immune_gas = flag_is_set(c->immunity_flags, CSAfF_PoisonCloud);
    out->immune_boulder = (c->model_flags & CMF_ImmuneToBoulder) != 0;
    if (player_ok(p)) {
        const struct Dungeon *dungeon = get_players_num_dungeon(p);
        if (!dungeon_invalid(dungeon)) {
            out->allowed = dungeon->creature_allowed[m] != 0;
            out->forced = dungeon->creature_force_enabled[m] > dungeon->creature_models_joined[m];
        }
    }
    if ((p >= 0) && (p < DAI_MAX_PLAYERS) && m_knows_pool[(size_t)p]) out->in_pool = game.pool.crtr_kind[m];
    out->pay = c->pay;
    out->lair_size = c->lair_size;
    out->hunger_rate = c->hunger_rate;
    out->hunger_fill = c->hunger_fill;
    out->annoy_level = c->annoy_level;
    out->heal_requirement = c->heal_requirement;
    out->training_cost = c->training_cost;
    out->scavenger_cost = c->scavenger_cost;
    out->research_value = c->research_value;
    out->manufacture_value = c->manufacture_value;
    out->training_value = c->training_value;
    out->scavenge_value = c->scavenge_value;
    out->jobs_primary = (uint64_t)c->job_primary;
    out->jobs_secondary = (uint64_t)c->job_secondary;
    out->jobs_not_do = (uint64_t)c->jobs_not_do;
    out->jobs_anger = (uint64_t)c->jobs_anger;
    out->jobs_stress = (uint64_t)c->job_stress;
    static const uint64_t work_job[(size_t)DaiWork::Count] = { Job_RESEARCH, Job_MANUFACTURE, Job_TRAIN };
    for (size_t w = 0; w < (size_t)DaiWork::Count; w++) {
        out->refuses[w] = ((uint64_t)c->jobs_not_do & work_job[w]) != 0;
        out->stressful[w] = ((uint64_t)c->job_stress & work_job[w]) != 0;
    }
    // AngerJobs is a mask over creature.cfg's [angerjobN] table (bit 1 << (N - 1)), not the job flags: harmful is any
    // but leaving (P6.8 found the job-flag test never matched a real kind's anger jobs).
    out->anger_harmful = false;
    for (int64_t n = 1; n < game.conf.crtr_conf.angerjobs_count; n++) {
        if (((uint64_t)c->jobs_anger & (1ull << (n - 1))) == 0) continue;
        const char *name = game.conf.crtr_conf.angerjobs[n].name;
        out->anger_harmful = out->anger_harmful || ((strcmp(name, "LEAVE_DUNGEON") != 0) && (name[0] != '\0'));
    }
    for (int i = 0; i < ENTRANCE_ROOMS_COUNT; i++) {
        out->entrance_rooms[i] = c->entrance_rooms[i];
        out->entrance_slabs[i] = c->entrance_slabs_req[i];
    }
    out->entrance_score = c->entrance_score;
}

void DaiWorldReaderSim::Research(PlayerNumber p, DaiResearchLook *out) const
{
    *out = DaiResearchLook();
    if (!player_ok(p)) return;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return;
    out->max_creatures = dungeon->max_creatures_attracted;
    // The game researches the first item still needed (get_next_research_item); the order is the level's.
    for (int64_t i = 0; (i < dungeon->research_num) && (i < DUNGEON_RESEARCH_COUNT); i++) {
        const struct ResearchVal *r = &dungeon->research[i];
        if (!research_needed(r, dungeon)) continue;
        if (out->count < DaiResearchLook::MAX) {
            DaiResearchItem &it = out->items[out->count];
            it.kind = (r->rtyp < (int)DaiResearchKind::Count) ? (DaiResearchKind)r->rtyp : DaiResearchKind::None;
            it.which = r->rkind;
            it.amount = r->req_amount;
        }
        out->count++;
    }
    if (out->count > 0) out->progress = dungeon->research_progress >> 8;
}

int64_t DaiWorldReaderSim::OwnFood(PlayerNumber p, DaiFoodLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    for (int64_t i = game.thing_lists[TngList_Objects].index, guard = 0; (i > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *obj = thing_get((DaiThingIndex)i);
        if (thing_is_invalid(obj)) break;
        i = obj->next_of_class;
        if (!object_is_mature_food(obj) || (obj->owner != p)) continue;
        const struct Room *room = get_room_thing_is_on(obj);
        if (room_is_invalid(room) || (room->owner != p) || !room_role_matches(room->kind, RoRoF_FoodStorage)) continue;
        DaiFoodLook &f = out[n++];
        f.ref.idx = obj->index;
        f.ref.creation_turn = obj->creation_turn;
        f.x = obj->mappos.x.stl.num;
        f.y = obj->mappos.y.stl.num;
    }
    return n;
}

int64_t DaiWorldReaderSim::OwnBookSlabs(PlayerNumber p, DaiSlabXY *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    for (int64_t i = game.thing_lists[TngList_Objects].index, guard = 0; (i > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *obj = thing_get((DaiThingIndex)i);
        if (thing_is_invalid(obj)) break;
        i = obj->next_of_class;
        if (!thing_is_spellbook(obj) || (obj->owner != p)) continue;
        const struct Room *room = get_room_thing_is_on(obj);
        if (room_is_invalid(room) || (room->owner != p) || !room_role_matches(room->kind, RoRoF_PowersStorage)) continue;
        const DaiSlabXY at{ DaiSubtileSlab(obj->mappos.x.stl.num), DaiSubtileSlab(obj->mappos.y.stl.num) };
        bool seen = false;
        for (int64_t k = 0; k < n; k++) seen = seen || ((out[k].x == at.x) && (out[k].y == at.y));
        if (!seen) out[n++] = at;
    }
    return n;
}

void DaiWorldReaderSim::Stock(PlayerNumber p, DaiStockLook *out) const
{
    *out = DaiStockLook();
    if (!player_ok(p)) return;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return;
    out->manufacture_level = dungeon->manufacture_level;
    out->making_door = (dungeon->manufacture_class == TCls_Door);
    out->making_kind = ((dungeon->manufacture_class == TCls_Door) || (dungeon->manufacture_class == TCls_Trap)) ? dungeon->manufacture_kind : 0;
    out->progress = dungeon->manufacture_progress >> 8;
    const struct TrapDoorConfig *td = &game.conf.trapdoor_conf;
    out->trap_kinds = (td->trap_types_count < DaiStockLook::MAX_KINDS) ? td->trap_types_count : DaiStockLook::MAX_KINDS;
    out->door_kinds = (td->door_types_count < DaiStockLook::MAX_KINDS) ? td->door_types_count : DaiStockLook::MAX_KINDS;
    const struct TrapInfo *mi = &dungeon->mnfct_info;
    auto trap = [&](int64_t s, int64_t k) {
        out->traps_placeable[s] = mi->trap_amount_placeable[k];
        out->traps_stored[s] = mi->trap_amount_stored[k];
        out->traps_buildable[s] = (mi->trap_build_flags[k] & MnfBldF_Manufacturable) != 0;
        out->trap_levels[s] = get_trap_model_stats(k)->manufct_level;
        out->trap_ratings[s] = DaiTrapRating((DaiThingModel)k);
        snprintf(out->trap_names[s], sizeof(out->trap_names[s]), "%s", trap_code_name(k));
    };
    auto door = [&](int64_t s, int64_t k) {
        out->doors_placeable[s] = mi->door_amount_placeable[k];
        out->doors_stored[s] = mi->door_amount_stored[k];
        out->doors_buildable[s] = (mi->door_build_flags[k] & MnfBldF_Manufacturable) != 0;
        out->door_levels[s] = get_door_model_stats(k)->manufct_level;
        snprintf(out->door_names[s], sizeof(out->door_names[s]), "%s", door_code_name(k));
    };
    for (int64_t k = 0; k < out->trap_kinds; k++) trap(k, k);
    for (int64_t k = 0; k < out->door_kinds; k++) door(k, k);
    // A map pack's own kinds numbered past the dense ones (Phase 7: dk2maps' from 1001), those the config names.
    for (int64_t k = DaiStockLook::MAX_KINDS; (k < td->trap_types_count) && (out->trap_extra < DaiStockLook::MAX_EXTRA); k++) {
        if (get_trap_model_stats(k)->code_name[0] == '\0') continue;
        out->trap_extra_kind[out->trap_extra] = k;
        trap(DaiStockLook::MAX_KINDS + out->trap_extra++, k);
    }
    for (int64_t k = DaiStockLook::MAX_KINDS; (k < td->door_types_count) && (out->door_extra < DaiStockLook::MAX_EXTRA); k++) {
        if (get_door_model_stats(k)->code_name[0] == '\0') continue;
        out->door_extra_kind[out->door_extra] = k;
        door(DaiStockLook::MAX_KINDS + out->door_extra++, k);
    }
}

uint64_t DaiWorldReaderSim::PredictHandDropJob(PlayerNumber p, const DaiThingRef &c, DaiSlabCoord x, DaiSlabCoord y) const
{
    if (!player_ok(p) || !slab_on_map(x, y)) return 0;
    struct Thing *thing = thing_get(c.idx);
    if (thing_is_invalid(thing) || !thing_is_creature(thing) || (thing->owner != p) || (thing->creation_turn != c.creation_turn))
        return 0;
    // Read-only with these arguments (no JobChk_* flags: no anger, message or state on a refusal); the
    // dai_knowledge_readonly ftest hashes the simulation around it.
    return (uint64_t)predict_hand_drop_job(thing, p, DaiSlabSubtileCenter(x), DaiSlabSubtileCenter(y));
}

int64_t DaiWorldReaderSim::VisibleThings(PlayerNumber p, const DaiRect &area, DaiThingLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    int64_t idx = game.thing_lists[TngList_Creatures].index;
    for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *th = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(th)) break;
        idx = th->next_of_class;
        if ((th->owner == p) || !in_area(area, th->mappos.x.stl.num, th->mappos.y.stl.num)) continue;
        if (!sees_all(p) && !creature_seen(th, p)) continue;
        DaiThingLook &l = out[n++];
        l = DaiThingLook();
        l.ref.idx = th->index;
        l.ref.creation_turn = th->creation_turn;
        l.model = th->model;
        l.owner = th->owner;
        l.x = th->mappos.x.stl.num;
        l.y = th->mappos.y.stl.num;
        const struct CreatureControl *cctrl = creature_control_get_from_thing(th);
        l.level = creature_control_invalid(cctrl) ? 1 : (int64_t)cctrl->exp_level + 1;
        l.force = creature_force_score(th);
        l.health_pct = (creature_control_invalid(cctrl) || (cctrl->max_health <= 0)) ? 100 : (100 * th->health / cctrl->max_health);
        l.digger = thing_is_creature_digger(th);
        l.battle = creature_control_invalid(cctrl) ? 0 : cctrl->battle_id;
        // A keeper's creature (no hero, no neutral: the rules' ScavengeGoodAllowed/NeutralAllowed are off by default),
        // free -- the engine's scavenger target test leaves out those held, in a hand and leaving.
        const TbBool free = !creature_is_kept_in_custody(th) && !thing_is_picked_up(th) && !l.digger && !is_hero_thing(th) && !is_neutral_thing(th);
        if (free) l.scavenge_required = calculate_correct_creature_scavenge_required(th, p);
        if (!sees_all(p)) audit(creature_seen(th, p));
    }
    // Each keeper's heart as find_players_dungeon_heart() finds it (the first of its hearts on the objects list), in one
    // walk of the list rather than one a keeper: the knowledge asks for every scanned row.
    const struct Thing *hearts[PLAYERS_COUNT];
    for (const struct Thing *&h : hearts) h = INVALID_THING;
    FOR_EACH_THING(th, thing_walk_list(game.thing_lists[TngList_Objects].index, THINGS_COUNT)) {
        if (!thing_is_dungeon_heart(th) || (th->owner >= PLAYERS_COUNT)) continue;
        if (thing_is_invalid(hearts[th->owner])) hearts[th->owner] = th;
    }
    for (PlayerNumber r = 0; (r < PLAYERS_COUNT) && (n < max); r++) {
        if (r == p) continue;
        const struct Thing *heart = hearts[r];
        if (thing_is_invalid(heart) || !in_area(area, heart->mappos.x.stl.num, heart->mappos.y.stl.num)) continue;
        if (!sees_all(p) && !place_seen(heart, p)) continue;
        DaiThingLook &l = out[n++];
        l = DaiThingLook();
        l.ref.idx = heart->index;
        l.ref.creation_turn = heart->creation_turn;
        l.heart = true;
        l.model = heart->model;
        l.owner = r;
        l.x = heart->mappos.x.stl.num;
        l.y = heart->mappos.y.stl.num;
        if (!sees_all(p)) audit(place_seen(heart, p));
    }
    return n;
}

int64_t DaiWorldReaderSim::Battles(PlayerNumber p, DaiBattleLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return 0;
    int64_t n = 0;
    // The battles its creatures (imps too) are in, each once; then every fighter in each.
    for (const int64_t start : { dungeon->creatr_list_start, dungeon->digger_list_start }) {
        for (int64_t i = start, guard = 0; (i > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
            const struct Thing *thing = thing_get((DaiThingIndex)i);
            if (thing_is_invalid(thing)) break;
            const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
            if (creature_control_invalid(cctrl)) break;
            i = cctrl->players_next_creature_idx;
            const BattleIndex b = cctrl->battle_id;
            if (b <= 0) continue;
            bool seen = false;
            for (int64_t k = 0; k < n; k++) seen = seen || (out[k].index == b);
            if (seen) continue;
            DaiBattleLook &l = out[n++];
            l = DaiBattleLook();
            l.index = b;
            int64_t sx = 0, sy = 0;
            const struct CreatureBattle *battle = creature_battle_get(b);
            // The game's battle list runs from first_creatr along battle_prev_creatr (insert_thing_in_battle_list; as
            // count_creatures_really_in_combat walks it): battle_next_creatr from the first is always 0.
            for (DaiThingIndex f = (DaiThingIndex)battle->first_creatr, fg = 0; (f > 0) && (fg < THINGS_COUNT); fg++) {
                const struct Thing *ft = thing_get(f);
                if (thing_is_invalid(ft)) break;
                const struct CreatureControl *fc = creature_control_get_from_thing(ft);
                if (creature_control_invalid(fc)) break;
                f = fc->battle_prev_creatr;
                if (ft->owner == p) {
                    l.own_count++;
                    l.own_force += creature_force_score(ft);
                    sx += ft->mappos.x.stl.num;
                    sy += ft->mappos.y.stl.num;
                } else {
                    const bool visible = sees_all(p) || creature_seen(ft, p);
                    if (!sees_all(p)) audit(visible);
                    if (!visible) continue;
                    l.enemy_count++;
                    l.enemy_force += creature_force_score(ft);
                }
            }
            if (l.own_count > 0) {
                l.x = (DaiSubtlCoord)(sx / l.own_count);
                l.y = (DaiSubtlCoord)(sy / l.own_count);
            }
        }
    }
    return n;
}

int64_t DaiWorldReaderSim::OwnTraps(PlayerNumber p, DaiTrapLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    int64_t idx = game.thing_lists[TngList_Traps].index;
    for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *th = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(th)) break;
        idx = th->next_of_class;
        if (th->owner != p) continue;
        DaiTrapLook &l = out[n++];
        l = DaiTrapLook();
        l.ref.idx = th->index;
        l.ref.creation_turn = th->creation_turn;
        l.model = th->model;
        l.x = th->mappos.x.stl.num;
        l.y = th->mappos.y.stl.num;
        l.shots = th->trap.num_shots;
        l.armed = trap_is_active(th);
    }
    return n;
}

int64_t DaiWorldReaderSim::OwnDoors(PlayerNumber p, DaiDoorLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    int64_t idx = game.thing_lists[TngList_Doors].index;
    for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *th = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(th)) break;
        idx = th->next_of_class;
        if (th->owner != p) continue;
        DaiDoorLook &l = out[n++];
        l = DaiDoorLook();
        l.ref.idx = th->index;
        l.ref.creation_turn = th->creation_turn;
        l.model = th->model;
        l.x = th->mappos.x.stl.num;
        l.y = th->mappos.y.stl.num;
        l.health = th->health;
        l.max_health = get_door_model_stats(th->model)->health;
        l.locked = th->door.is_locked;
    }
    return n;
}

bool DaiWorldReaderSim::PowerCastable(PlayerNumber p, DaiPowerKind k, DaiSubtlCoord x, DaiSubtlCoord y, const DaiThingRef *target) const
{
    if (!player_ok(p) || (k <= 0)) return false;
    if (target == nullptr) return can_cast_power_at_xy(p, k, x, y, 0);
    const struct Thing *th = thing_get(target->idx);
    if (thing_is_invalid(th) || (th->creation_turn != target->creation_turn)) return false;
    return can_cast_power_on_thing(p, th, k);
}

bool DaiWorldReaderSim::SightOfEvil(PlayerNumber p, DaiSightLook *out) const
{
    *out = DaiSightLook();
    if (!player_ok(p)) return false;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon) || (dungeon->sight_casted_thing_idx <= 0)) return false;
    const struct Thing *eye = thing_get(dungeon->sight_casted_thing_idx);
    if (!thing_exists(eye)) return false;
    out->active = true;
    out->x = eye->mappos.x.stl.num;
    out->y = eye->mappos.y.stl.num;
    for (int64_t y = 0; y < 2 * MAX_SOE_RADIUS; y++)
        for (int64_t x = 0; x < 2 * MAX_SOE_RADIUS; x++) out->open += dungeon->soe_explored_flags[y][x] ? 1 : 0;
    return true;
}

bool DaiWorldReaderSim::SightShows(PlayerNumber p, DaiSubtlCoord x, DaiSubtlCoord y) const
{
    return player_ok(p) && power_sight_explored(x, y, p);
}

int64_t DaiWorldReaderSim::EnemyDefences(PlayerNumber p, DaiEnemyDefenceLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    for (const int list : { (int)TngList_Traps, (int)TngList_Doors }) {
        int64_t idx = game.thing_lists[list].index;
        for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
            struct Thing *th = thing_get((DaiThingIndex)idx);
            if (thing_is_invalid(th)) break;
            idx = th->next_of_class;
            const bool door = (list == TngList_Doors);
            // Neutral traps too (P5.13: a level's traps are often neutral), not neutral doors.
            if ((th->owner == p) || (door && (th->owner == game.neutral_player_num))) continue;
            // As a human sees it: on revealed ground, or in its Sight of Evil; a hidden trap once revealed, a secret
            // door once found.
            const bool shown = door ? !door_is_hidden_to_player(th, p)
                                    : ((th->trap.revealed != 0) || !get_trap_model_stats(th->model)->hidden);
            const bool visible = sees_all(p) || (place_seen(th, p) && shown);
            if (!visible) continue;
            if (!sees_all(p)) audit(place_seen(th, p) && shown);
            DaiEnemyDefenceLook &l = out[n++];
            l = DaiEnemyDefenceLook();
            l.ref.idx = th->index;
            l.ref.creation_turn = th->creation_turn;
            l.owner = th->owner;
            l.door = door;
            l.model = th->model;
            l.x = th->mappos.x.stl.num;
            l.y = th->mappos.y.stl.num;
            l.health = th->health;
            l.locked = door && th->door.is_locked;
            l.spent = !door && (th->trap.num_shots == 0);
        }
    }
    return n;
}

int64_t DaiWorldReaderSim::RivalRooms(PlayerNumber p, DaiRivalRoomLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    for (int64_t i = 1; (i < ROOMS_COUNT) && (n < max); i++) {
        const struct Room *room = room_get(i);
        if (room_is_invalid(room) || !room_exists(room)) continue;
        if ((room->owner == p) || (room->owner == game.neutral_player_num)) continue;
        DaiRivalRoomLook l;
        int64_t guard = 0;
        for (SlabCodedCoords s = room->slabs_list; (s > 0) && (guard++ < 4096); s = get_next_slab_number_in_room(s)) {
            const DaiSlabCoord sx = slb_num_decode_x(s), sy = slb_num_decode_y(s);
            const bool seen = sees_all(p) || keeper_slab_revealed(p, sx, sy)
                || power_sight_explored(DaiSlabSubtileCenter(sx), DaiSlabSubtileCenter(sy), p);
            if (!seen) continue;
            if (!sees_all(p)) audit(keeper_slab_revealed(p, sx, sy) || power_sight_explored(DaiSlabSubtileCenter(sx), DaiSlabSubtileCenter(sy), p));
            if (l.slabs_seen == 0) {
                l.x = sx;
                l.y = sy;
                l.box = DaiRect{ sx, sy, sx, sy };
            }
            l.slabs_seen++;
            l.box.x0 = std::min(l.box.x0, sx); l.box.y0 = std::min(l.box.y0, sy);
            l.box.x1 = std::max(l.box.x1, sx); l.box.y1 = std::max(l.box.y1, sy);
        }
        if (l.slabs_seen == 0) continue;
        l.index = i;
        l.owner = room->owner;
        l.kind = room->kind;
        out[n++] = l;
    }
    return n;
}

int64_t DaiWorldReaderSim::HeroGates(PlayerNumber p, DaiSlabXY *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    int64_t idx = game.thing_lists[TngList_Objects].index;
    for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *t = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(t)) break;
        idx = t->next_of_class;
        if (!object_is_hero_gate(t) || thing_is_in_limbo(t)) continue;
        if (!sees_all(p)) {
            const bool visible = place_seen(t, p);
            audit(visible);
            if (!visible) continue;
        }
        out[n++] = DaiSlabXY{ DaiSubtileSlab(t->mappos.x.stl.num), DaiSubtileSlab(t->mappos.y.stl.num) };
    }
    return n;
}

bool DaiWorldReaderSim::PowerTakes(PlayerNumber p, DaiPowerKind k, const DaiThingRef &target) const
{
    if (!player_ok(p) || (k <= 0)) return false;
    const struct Thing *th = thing_get(target.idx);
    if (thing_is_invalid(th) || (th->creation_turn != target.creation_turn) || !thing_is_creature(th)) return false;
    if ((th->owner != p) && !sees_all(p)) {
        const bool visible = creature_seen(th, p);
        audit(visible);
        if (!visible) return false;
    }
    return keeper_assist_power_takes(p, k, th);
}

int64_t DaiWorldReaderSim::PowerAreaRange(DaiPowerKind k, int64_t level, bool *own_too) const
{
    TbBool own = false, skip_armoured = false;
    const int64_t range = (k > 0) ? keeper_assist_area_range(k, level, &own, &skip_armoured) : 0;
    *own_too = own;
    return range;
}

DaiPowerKind DaiWorldReaderSim::PowerOf(DaiPower d) const
{
    const int64_t k = power_model_id(DaiPowerCode(d));
    return (k > 0) ? (DaiPowerKind)k : 0;
}

void DaiWorldReaderSim::HumanFightRules(PlayerNumber p, DaiHumanFightRules *out) const
{
    *out = DaiHumanFightRules();
    (void)p; // no combat assistant in this build: no rules of the human's
}

void DaiWorldReaderSim::LiveExclusion(PlayerNumber p, DaiLiveExclusion *out) const
{
    *out = DaiLiveExclusion();
    if (!player_ok(p)) return;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return;
    auto add = [&](DaiThingIndex t) { if ((t != 0) && !out->Excludes(t) && (out->n < DaiLiveExclusion::MAX)) out->things[out->n++] = t; };
    out->radius = daimon_config.global.assistant_possession_radius;
    out->possessed = keeper_possessed_thing(p);
    if (out->possessed != 0) {
        const struct Thing *t = thing_get(out->possessed);
        out->px = t->mappos.x.stl.num;
        out->py = t->mappos.y.stl.num;
        add(out->possessed);
        // The group it leads: every creature of the keeper's the exclusion names.
        int64_t idx = game.thing_lists[TngList_Creatures].index;
        for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
            const struct Thing *c = thing_get((DaiThingIndex)idx);
            if (thing_is_invalid(c)) break;
            idx = c->next_of_class;
            if ((c->owner == p) && keeper_thing_excluded(p, c)) add(c->index);
        }
    }
    // Each of the keeper's users: what each holds, points at or looks at (Daimon's own slot is not one of them)
    for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
        const struct UserState *seat = get_user_state(user);
        if (user_state_invalid(seat) || (seat->player_id != p)) continue;
        for (int64_t i = 0; i < seat->num_things_in_hand; i++) add(seat->things_in_hand[i]);
        if (seat->thing_under_hand > 0) add((DaiThingIndex)seat->thing_under_hand);
        if (seat->work_state == PSt_CreatrInfo) add(seat->controlled_thing_idx);     // the info panel's creature
    }
    for (int64_t i = 0; i < dungeon->num_things_in_hand; i++) add(dungeon->things_in_hand[i]);   // the drop queue
}

void DaiWorldReaderSim::ContentSpecs(std::vector<DaiPowerSpec> *powers, std::vector<DaiTrapSpec> *traps,
                                     std::vector<DaiDoorSpec> *doors) const
{
    const struct MagicConfig *mc = &game.conf.magic_conf;
    for (int64_t k = 1; k < mc->power_types_count; k++) {
        const struct PowerConfigStats *pw = get_power_model_stats((DaiPowerKind)k);
        if ((pw == nullptr) || (pw->code_name[0] == '\0') || (strcmp(pw->code_name, "NOPOWER") == 0)) continue;
        DaiPowerSpec s;
        s.kind = k;
        snprintf(s.name, sizeof(s.name), "%s", pw->code_name);
        s.on_panel = (pw->panel_tab_idx > 0);
        const int64_t f = pw->magic_use_func_idx;
        s.func = ((f >= 0) && (f <= 14)) ? (DaiPowerFunc)f : DaiPowerFunc::Other;
        s.enemy_creatures = (pw->can_cast_flags & PwCast_EnemyCrtrs) != 0;
        s.own_creatures = (pw->can_cast_flags & PwCast_OwnedCrtrs) != 0;
        s.ground = (pw->can_cast_flags & PwCast_AllGround) != 0;
        s.cost = pw->cost[0];
        s.strength = pw->strength[0];
        if ((s.func == DaiPowerFunc::Tunneller) && (pw->creature_model > 0)) {
            s.creature = pw->creature_model;
            s.creature_level = pw->strength[0];
        }
        if ((s.func == DaiPowerFunc::ApplySpell) && (pw->spell_idx > 0)) {
            const struct SpellConfig *sp = get_spell_config(pw->spell_idx);
            if (sp != nullptr) {
                const uint64_t fl = sp->spell_flags;
                s.spell_hostile = (fl & (CSAfF_Slow | CSAfF_Disease | CSAfF_Chicken | CSAfF_Freeze | CSAfF_Fear | CSAfF_MadKilling)) != 0;
                s.spell_friendly = (fl & (CSAfF_Speed | CSAfF_Armour | CSAfF_Rebound | CSAfF_Flying | CSAfF_Invisibility
                                          | CSAfF_Sight | CSAfF_Light)) != 0;
                s.spell_heals = (fl & CSAfF_Heal) != 0;
                s.spell_damage = sp->damage;
                s.spell_heal = sp->healing_recovery;
                s.spell_shot = (sp->shot_model > 0);
                if (sp->crtr_summon_model > 0) {
                    s.creature = sp->crtr_summon_model;
                    s.creature_level = sp->crtr_summon_level;
                }
            }
        }
        powers->push_back(s);
    }
    const struct TrapDoorConfig *td = &game.conf.trapdoor_conf;
    for (int64_t k = 1; k < td->trap_types_count; k++) {
        const struct TrapConfigStats *t = get_trap_model_stats(k);
        if ((t == nullptr) || (t->code_name[0] == '\0') || (strcmp(t->code_name, "NOTRAP") == 0)) continue;
        DaiTrapSpec s;
        s.kind = k;
        snprintf(s.name, sizeof(s.name), "%s", t->code_name);
        s.on_panel = (t->panel_tab_idx > 0);
        s.activation = t->activation_type;
        s.shots = t->shots;
        s.rating = DaiTrapRating((DaiThingModel)k);
        s.health = t->health;
        s.destructible = (t->destructible != 0);
        if ((t->activation_type == TrpAcT_EffectonTrap) && (t->created_itm_model > 0)) {
            const struct EffectConfigStats *e = get_effect_model_stats(t->created_itm_model);
            s.gas = (e != nullptr) && (e->area_affect_type == AAffT_GasDamage);
        }
        traps->push_back(s);
    }
    for (int64_t k = 1; k < td->door_types_count; k++) {
        const struct DoorConfigStats *d = get_door_model_stats(k);
        if ((d == nullptr) || (d->code_name[0] == '\0') || (strcmp(d->code_name, "NODOOR") == 0)) continue;
        DaiDoorSpec s;
        s.kind = k;
        snprintf(s.name, sizeof(s.name), "%s", d->code_name);
        s.on_panel = (d->panel_tab_idx > 0);
        s.health = d->health;
        s.secret = (d->model_flags & DoMF_Secret) != 0;
        s.midas = (d->model_flags & DoMF_Midas) != 0;
        s.resist_non_magic = (d->model_flags & DoMF_ResistNonMagic) != 0;
        doors->push_back(s);
    }
}

int64_t DaiWorldReaderSim::PowerKindCount() const
{
    return game.conf.magic_conf.power_types_count;
}

bool DaiWorldReaderSim::PowerAvailable(PlayerNumber p, DaiPowerKind k) const
{
    return player_ok(p) && is_power_available(p, k);
}

int64_t DaiWorldReaderSim::PowerPrice(PlayerNumber p, DaiPowerKind k, int64_t level) const
{
    return player_ok(p) ? compute_power_price(p, k, (KeepPwrLevel)level) : 0;
}

int64_t DaiWorldReaderSim::OwnMarks(PlayerNumber p, DaiSlabXY *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return 0;
    const int64_t imax = (dungeon->highest_task_number < MAPTASKS_COUNT) ? dungeon->highest_task_number : MAPTASKS_COUNT;
    int64_t n = 0;
    for (int64_t i = 0; (i < imax) && (n < max); i++) {
        const struct MapTask *mt = &dungeon->task_list[i];
        if (mt->kind == 0) continue;    // only that there is a mark: its kind says what an unseen slab holds
        const DaiSlabCoord sx = DaiSubtileSlab(stl_num_decode_x(mt->coords)), sy = DaiSubtileSlab(stl_num_decode_y(mt->coords));
        if (slab_on_map(sx, sy)) out[n++] = DaiSlabXY{ sx, sy };
    }
    return n;
}

static_assert(((int)DaiSpecial::RevealMap == SpcKind_RevealMap) && ((int)DaiSpecial::Resurrect == SpcKind_Resurrect)
    && ((int)DaiSpecial::Transfer == SpcKind_TrnsfrCrtr) && ((int)DaiSpecial::HiddenWorld == SpcKind_HiddnWorld)
    && ((int)DaiSpecial::Custom == SpcKind_Custom) && ((int)DaiSpecial::MakeUnsafe == SpcKind_MakeUnsafe), "DaiSpecial differs from SpcKind_*");

int64_t DaiWorldReaderSim::SpecialBoxes(PlayerNumber p, DaiSpecialLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    int64_t n = 0;
    for (int64_t i = game.thing_lists[TngList_Objects].index, guard = 0; (i > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *obj = thing_get((DaiThingIndex)i);
        if (thing_is_invalid(obj)) break;
        i = obj->next_of_class;
        if (!thing_is_special_box(obj) || !can_thing_be_picked_up_by_player(obj, p)) continue;
        const int64_t kind = box_thing_to_special(obj);
        DaiSpecialLook &o = out[n++];
        o = DaiSpecialLook();
        o.ref.idx = obj->index;
        o.ref.creation_turn = obj->creation_turn;
        o.kind = ((kind > 0) && (kind < (int64_t)DaiSpecial::Count)) ? (DaiSpecial)kind : DaiSpecial::Custom;
        o.x = obj->mappos.x.stl.num;
        o.y = obj->mappos.y.stl.num;
    }
    return n;
}

int64_t DaiWorldReaderSim::DeadCreatures(PlayerNumber p, DaiDeadLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return 0;
    int64_t n = 0;
    for (int64_t i = 0; (i < dungeon->dead_creatures_count) && (i < DEAD_CREATURES_MAX_COUNT) && (n < max); i++) {
        out[n].model = dungeon->dead_creatures[i].model;
        out[n].level = dungeon->dead_creatures[i].exp_level + 1;
        n++;
    }
    return n;
}

int64_t DaiWorldReaderSim::OwnImps(PlayerNumber p, DaiImpLook *out, int64_t max) const
{
    if (!player_ok(p)) return 0;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    int64_t n = 0;
    // The diggers' list, then the tunnellers on the creatures' (P9).
    for (const int64_t start : { dungeon->digger_list_start, dungeon->creatr_list_start })
    for (int64_t i = start, guard = 0; (i > 0) && (guard < THINGS_COUNT) && (n < max); guard++) {
        const struct Thing *imp = thing_get((DaiThingIndex)i);
        if (thing_is_invalid(imp)) break;
        const struct CreatureControl *cctrl = creature_control_get_from_thing(imp);
        if (creature_control_invalid(cctrl)) break;
        if ((start == dungeon->creatr_list_start) && !digs_as_creature(imp)) {
            i = cctrl->players_next_creature_idx;
            continue;
        }
        DaiImpLook &o = out[n++];
        o = DaiImpLook();
        o.ref.idx = imp->index;
        o.ref.creation_turn = imp->creation_turn;
        o.x = imp->mappos.x.stl.num;
        o.y = imp->mappos.y.stl.num;
        o.force = get_creature_thing_score(imp);
        o.level = cctrl->exp_level + 1;
        o.pickable = can_thing_be_picked_up_by_player(imp, p);
        // Training: given the job (a dropped digger's, creature_jobs.c) and in one of its keeper's training rooms -- its
        // state alone is not enough, a trainee walks between the posts; the room alone neither, imps cross it.
        {
            const struct Room *room = subtile_room_get(imp->mappos.x.stl.num, imp->mappos.y.stl.num);
            o.training = (cctrl->job_assigned == Job_TRAIN) && !room_is_invalid(room) && (room->owner == p)
                && room_role_matches(room->kind, RoRoF_CrTrainExp);
        }
        switch (get_creature_state_besides_interruptions(imp)) {
        case CrSt_AtTrainingRoom: case CrSt_Training:
            o.training = true;
            break;
        case CrSt_ImpArrivesAtDigDirt: case CrSt_ImpDigsDirt: case CrSt_ImpArrivesAtMineGold: case CrSt_ImpMinesGold:
        case CrSt_ImpArrivesAtImproveDungeon: case CrSt_ImpImprovesDungeon: case CrSt_ImpArrivesAtConvertDungeon:
        case CrSt_ImpConvertsDungeon: case CrSt_ImpArrivesAtReinforce: case CrSt_ImpReinforces:
            o.working = true;
            break;
        default:
            break;
        }
        o.sped_up = cctrl->slap_turns > 0;
        o.sacrificing = creature_is_being_sacrificed(imp);
        o.gold = imp->creature.gold_carried;
        o.tunneller = (start == dungeon->creatr_list_start);
        o.model = imp->model;
        i = cctrl->players_next_creature_idx;
    }
    return n;
}

RoomKind DaiWorldReaderSim::RoomOf(DaiRoom d) const
{
    // The first room kind with the role (04 section 2.1): the shipped terrain.cfg gives each role one room.
    const uint64_t role = DaiRoomRole(d);
    if (role == 0) return 0;
    const RoomKind k = find_first_roomkind_with_role((RoomRole)role);
    return room_role_matches(k, (RoomRole)role) ? k : 0;
}

DaiBridgeRoles DaiWorldReaderSim::BridgeRoles() const
{
    DaiBridgeRoles r;
    r.water = room_role_matches(RoK_BRIDGE, RoRoF_PassWater);
    r.lava = room_role_matches(RoK_BRIDGE, RoRoF_PassLava);
    r.abyss = room_role_matches(RoK_BRIDGE, RoRoF_PassAbyss);
    return r;
}

bool DaiWorldReaderSim::RoomAvailable(PlayerNumber p, RoomKind k) const
{
    return player_ok(p) && is_room_available(p, k);
}

void DaiWorldReaderSim::Custody(PlayerNumber p, DaiCustodyLook *out) const
{
    *out = DaiCustodyLook();
    if (!player_ok(p)) return;
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (dungeon_invalid(dungeon)) return;
    // Its rooms: free torture devices, the temple pool and who prays (the seat view's add_custody, add_graveyard_and_temples).
    for (int64_t rkind = 1; rkind < TERRAIN_ITEMS_MAX; rkind++) {
        const bool torture = room_role_matches((RoomKind)rkind, RoRoF_Torture), temple = room_role_matches((RoomKind)rkind, RoRoF_CrSacrifice);
        if (!torture && !temple) continue;
        for (int64_t ridx = dungeon->room_list_start[rkind], guard = 0; (ridx > 0) && (guard < ROOMS_COUNT); guard++) {
            const struct Room *room = room_get((RoomIndex)ridx);
            if (room_is_invalid(room)) break;
            ridx = room->next_of_owner;
            SlabCodedCoords sn = room->slabs_list;
            for (int64_t g2 = 0; (sn != 0) && (g2 < 1024); g2++) {
                const DaiSlabCoord sx = slb_num_decode_x(sn), sy = slb_num_decode_y(sn);
                if (torture) {
                    const struct Thing *dev = find_base_thing_on_mapwho(TCls_Object, ObjMdl_Torturer, DaiSlabSubtileCenter(sx), DaiSlabSubtileCenter(sy));
                    if (!thing_is_invalid(dev) && (dev->torturer.belongs_to == 0)) out->devices_free++;
                }
                for (int k = 0; temple && !out->pool && (k < 9); k++) {
                    const DaiSubtlCoord x = slab_subtile(sx, k % 3), y = slab_subtile(sy, k / 3);
                    if (subtile_has_sacrificial_on_top(x, y) && can_drop_thing_here(x, y, p, 0)) {
                        out->pool = true;
                        out->pool_x = x;
                        out->pool_y = y;
                    }
                }
                sn = get_next_slab_number_in_room(sn);
            }
            for (int64_t ci = room->creatures_list, g3 = 0; temple && (ci > 0) && (g3 < THINGS_COUNT); g3++) {
                const struct Thing *t = thing_get((DaiThingIndex)ci);
                if (thing_is_invalid(t) || !thing_is_creature(t)) break;
                out->praying++;
                ci = creature_control_get_from_thing(t)->next_in_room;
            }
        }
    }
    // Its prisoners.
    for (int64_t idx = game.thing_lists[TngList_Creatures].index, guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *thing = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(thing)) break;
        idx = thing->next_of_class;
        if (!thing_is_creature(thing) || (thing->owner == p) || !creature_is_kept_in_custody_by_player(thing, p)) continue;
        if (out->prisoners >= DaiCustodyLook::MAX_PRISONERS) break;
        const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        DaiPrisonerLook &o = out->prisoner[out->prisoners++];
        o.ref.idx = thing->index;
        o.ref.creation_turn = thing->creation_turn;
        o.model = thing->model;
        o.level = cctrl->exp_level + 1;
        o.owner = thing->owner;
        o.health = thing->health;
        o.max_health = cctrl->max_health;
        o.hungry = hunger_is_creature_hungry(thing);
        o.x = thing->mappos.x.stl.num;
        o.y = thing->mappos.y.stl.num;
        const struct Room *room = get_room_thing_is_on(thing);
        o.at = thing_is_picked_up_by_player(thing, p) ? DaiHeldAt::Hand : creature_is_being_tortured(thing) ? DaiHeldAt::Torture
            : creature_is_kept_in_prison(thing) ? DaiHeldAt::Prison : DaiHeldAt::Moving;
        if (!room_is_invalid(room)) {
            o.room = room->index;
            const struct RoomRulesConfig *rr = &game.conf.rules[p].rooms;
            if ((o.at == DaiHeldAt::Prison) && (rr->prison_skeleton_chance > 0)) o.on_death = prison_death_kind(thing, room);
            if ((o.at == DaiHeldAt::Torture) && (rr->ghost_convert_chance > 0)) o.on_death = torture_death_kind(thing, room);
        }
        if (o.at == DaiHeldAt::Torture) o.torture_turns = (int64_t)get_gameturn() - (int64_t)cctrl->tortured.state_start_turn;
        o.break_time = creature_stats_get_from_thing(thing)->torture_break_time;
        o.force = get_creature_thing_score(thing);
        // What it would become in a prison or on a rack of the keeper's (the kinds depend on the room's kind only).
        struct Room probe;
        memset(&probe, 0, sizeof(probe));
        for (int64_t rkind = 1; rkind < TERRAIN_ITEMS_MAX; rkind++) {
            probe.kind = (RoomKind)rkind;
            if ((o.skeleton == 0) && room_role_matches((RoomKind)rkind, RoRoF_Prison)) o.skeleton = prison_death_kind(thing, &probe);
            if ((o.ghost == 0) && room_role_matches((RoomKind)rkind, RoRoF_Torture)) o.ghost = torture_death_kind(thing, &probe);
        }
        // The score table get_creature_thing_score reads (the dungeon score's; creature_force_score's scale).
        const CrtrExpLevel exp = (cctrl->exp_level < CREATURE_MAX_LEVEL) ? cctrl->exp_level : (CrtrExpLevel)(CREATURE_MAX_LEVEL - 1);
        auto score = [&](DaiThingModel m) {
            return ((m > 0) && (m < game.conf.crtr_conf.model_count)) ? (int64_t)game.creature_scores[m].value[exp] : 0;
        };
        o.skeleton_force = score(o.skeleton);
        o.ghost_force = score(o.ghost);
    }
    const struct RoomRulesConfig *rr = &game.conf.rules[p].rooms;
    out->convert_chance = rr->torture_convert_chance;
    out->death_chance = rr->torture_death_chance;
    out->skeleton_chance = rr->prison_skeleton_chance;
    out->ghost_chance = rr->ghost_convert_chance;
    out->torture_payday = game.conf.rules[p].gameplay.torture_payday;
    // The graveyard: toward the next vampire, and bodies lying where it can see them.
    out->bodies_rotting = dungeon->bodies_rotten_for_vampire;
    out->bodies_per_vampire = game.conf.rules[p].rooms.bodies_for_vampire;
    for (RoomKind rk = 1; (rk < game.conf.slab_conf.room_types_count) && (out->vampire_model == 0); rk++)
        if (room_role_matches(rk, RoRoF_DeadStorage)) out->vampire_model = get_room_create_creature_model(rk);
    for (int64_t idx = game.thing_lists[TngList_DeadCreatrs].index, guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *t = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(t)) break;
        idx = t->next_of_class;
        if (!sees_all(p) && !subtile_revealed(t->mappos.x.stl.num, t->mappos.y.stl.num, p)) continue;
        const struct Room *room = get_room_thing_is_on(t);
        if (!room_is_invalid(room) && room_role_matches(room->kind, RoRoF_DeadStorage)) continue;
        out->bodies_lying++;
    }
    // What it has offered since a recipe last fired (the temple's messages tell a human as much).
    for (int64_t m = 1; (m < CREATURE_TYPES_MAX) && (m < DAI_MAX_KINDS) && (m <= KindCount()); m++) {
        out->offered[m] = dungeon->creature_sacrifice[m];
        out->offered_levels[m] = dungeon->creature_sacrifice_exp[m];
    }
    for (int64_t idx = game.thing_lists[TngList_Creatures].index, guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *t = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(t)) break;
        idx = t->next_of_class;
        if (!creature_is_being_sacrificed(t) || (slabmap_owner(get_slabmap_for_subtile(t->mappos.x.stl.num, t->mappos.y.stl.num)) != p)) continue;
        if ((t->model < 1) || (t->model >= DAI_MAX_KINDS)) continue;
        out->sinking[t->model]++;
        out->sinking_levels[t->model] += creature_control_get_from_thing(t)->exp_level + 1;
    }
    out->imps_sacrificed = dungeon->cheaper_diggers;
}

int64_t DaiWorldReaderSim::Recipes(DaiRecipeLook *out, int64_t max) const
{
    int64_t n = 0;
    for (int r = 0; (r < MAX_SACRIFICE_RECIPES) && (n < max); r++) {
        const struct SacrificeRecipe *rc = &game.conf.rules[0].sacrifices.sacrifice_recipes[r];
        // The engine's do-while (process_sacrifice_award): slot 0 always, then on until an empty slot.
        if ((r > 0) && (rc->action == SacA_None)) break;
        if (rc->action == SacA_None) continue;
        DaiRecipeLook &o = out[n++];
        o = DaiRecipeLook();
        for (int v = 0; (v < MAX_SACRIFICE_VICTIMS) && (v < DaiRecipeLook::MAX_VICTIMS); v++) o.victims[v] = rc->victims[v];
        o.index = r;
        switch (rc->action) {
        case SacA_MkCreature: o.action = DaiRecipeAction::MakeCreature; o.creature = (DaiThingModel)rc->param; break;
        case SacA_MkGoodHero: o.action = DaiRecipeAction::MakeHero; o.creature = (DaiThingModel)rc->param; break;
        case SacA_NegSpellAll: o.action = DaiRecipeAction::CurseAll; o.spell = rc->param; break;
        case SacA_PosSpellAll: o.action = DaiRecipeAction::BlessAll; o.spell = rc->param; break;
        case SacA_NegUniqFunc: o.action = DaiRecipeAction::Punish; break;
        case SacA_PosUniqFunc:
            o.action = DaiRecipeAction::Boon;
            o.boon = (rc->param == UnqF_ComplResrch) ? DaiBoon::CompleteResearch : (rc->param == UnqF_ComplManufc) ? DaiBoon::CompleteManufacture
                : (rc->param == UnqF_CheaperImp) ? DaiBoon::CheaperImps : (rc->param == UnqF_MkAllHappy) ? DaiBoon::AllHappy : DaiBoon::None;
            break;
        case SacA_CustomReward: o.action = DaiRecipeAction::CustomReward; break;
        case SacA_CustomPunish: o.action = DaiRecipeAction::CustomPunish; break;
        default: o.action = DaiRecipeAction::Punish; break;     // unknown: never worth it
        }
    }
    return n;
}

void DaiWorldReaderSim::SetSeesAll(PlayerNumber p, bool on)
{
    if ((p >= 0) && (p < DAI_MAX_PLAYERS)) m_sees_all[(size_t)p] = on;
}

void DaiWorldReaderSim::SetKnowsPool(PlayerNumber p, bool on)
{
    if ((p >= 0) && (p < DAI_MAX_PLAYERS)) m_knows_pool[(size_t)p] = on;
}

bool DaiWorldReaderSim::RivalKeeper(PlayerNumber p, PlayerNumber r) const
{
    if (!player_ok(p) || (r == p) || (r < 0) || (r >= PLAYERS_COUNT) || (r == PLAYER_GOOD) || (r == PLAYER_NEUTRAL)) return false;
    const struct PlayerInfo *player = get_player(r);
    return player_exists(player) && (player->player_type == PT_Keeper) && (player->victory_state == VicS_Undecided)
        && !players_are_mutual_allies(p, r);
}

void DaiWorldReaderSim::Alliance(PlayerNumber p, PlayerNumber r, DaiAllianceLook *out) const
{
    *out = DaiAllianceLook();
    if (!player_ok(p) || (r == p) || (r < 0) || (r >= PLAYERS_COUNT) || (r == PLAYER_GOOD) || (r == PLAYER_NEUTRAL)) return;
    const struct PlayerInfo *player = get_player(r);
    if (!player_exists(player)) return;
    out->keeper = (player->player_type == PT_Keeper) && (player->victory_state == VicS_Undecided);
    out->ours = player_allied_with(get_player(p), r);
    out->theirs = player_allied_with(player, p);
    out->locked = is_player_ally_locked(p, r);
    const struct Dungeon *d = get_players_dungeon(player);
    if (!dungeon_invalid(d)) {
        out->creatures = d->num_active_creatrs;
        out->rooms = d->total_rooms;
    }
}

int64_t DaiWorldReaderSim::RoomPrice(PlayerNumber p, RoomKind k) const
{
    (void)p;
    return get_room_kind_stats(k)->cost;
}

int64_t DaiWorldReaderSim::Facts(PlayerNumber p, DaiTurn now, KeeperFact *out, int64_t max)
{
    if (!player_ok(p)) return 0;
    if (!m_trackers[p]) {
        m_trackers[p] = std::make_unique<KeeperFactsTracker>();
        keeper_facts_init(m_trackers[p].get(), p);
    }
    return keeper_facts_update(m_trackers[p].get(), now, out, max);
}

void DaiWorldReaderSim::Rebaseline(PlayerNumber p)
{
    if (player_ok(p) && m_trackers[p]) keeper_facts_rebaseline(m_trackers[p].get());
}
