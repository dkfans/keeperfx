/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file keeper_facts.c
 *     The keeper-facts tracker (keeper_facts.h). Moved down from kfx_script's api_seat_decision.c, which keeps its
 *     reasons, severities and pacing and turns these facts into them.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "keeper_facts.h"

#include <stdlib.h>
#include <string.h>

#include "bflib_basics.h"
#include "config_magic.h"
#include "config_rules.h"
#include "config_trapdoor.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_states_gardn.h"
#include "dungeon_data.h"
#include "keeper_assist_state.h"
#include "keeper_visibility.h"
#include "game_legacy.h"
#include "map_data.h"
#include "player_data.h"
#include "power_hand.h"
#include "room_data.h"
#include "slab_data.h"
#include "tasks_list.h"
#include "thing_list.h"
#include "thing_traps.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SLAB_OWNED 0x8000
#define SLAB_MARKED 0x4000
/** Sight of Evil has stopped widening when it has not grown for this many game turns. */
#define SIGHT_SETTLE_TURNS 8

/** Where an update writes its facts. */
struct FactOut {
    struct KeeperFact *out;
    int64_t max;
    int64_t n;
    DaiTurn now;
    struct KeeperFactsTracker *t;
};

static struct KeeperFact *put(struct FactOut *o, enum KeeperFactKind kind, int64_t sub)
{
    if (o->n >= o->max) {
        o->t->dropped++;
        return NULL;
    }
    struct KeeperFact *f = &o->out[o->n++];
    memset(f, 0, sizeof(*f));
    f->kind = kind;
    f->sub = sub;
    f->turn = o->now;
    return f;
}

static struct KeeperFact *put_at(struct FactOut *o, enum KeeperFactKind kind, int64_t sub, DaiSubtlCoord x, DaiSubtlCoord y)
{
    struct KeeperFact *f = put(o, kind, sub);
    if (f != NULL) { f->x = x; f->y = y; }
    return f;
}

static int64_t quarter_of(PlayerNumber p)
{
    const int64_t gap = (int64_t)game.conf.rules[p].gameplay.pay_day_gap;
    const int64_t progress = (int64_t)game.pay_day_progress[p];
    return (gap > 0) ? (4 * progress / gap) % 4 : 0;
}

static void owned_kinds(PlayerNumber p, TbBool *out)
{
    memset(out, 0, sizeof(TbBool) * KEEPER_FACTS_MODEL_BITS);
    const struct Dungeon *dungeon = get_players_dungeon(get_player(p));
    if (dungeon_invalid(dungeon)) return;
    for (int pass = 0; pass < 2; pass++) {
        int64_t idx = (pass == 0) ? dungeon->creatr_list_start : dungeon->digger_list_start;
        for (int guard = 0; (idx > 0) && (guard < 2000); guard++) {
            const struct Thing *thing = thing_get((DaiThingIndex)idx);
            if (thing_is_invalid(thing) || !thing_is_creature(thing)) break;
            if ((thing->model > 0) && (thing->model < KEEPER_FACTS_MODEL_BITS)) out[thing->model] = true;
            const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
            idx = cctrl->players_next_creature_idx;
        }
    }
}

static void note_events(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    for (int i = 0; i < EVENTS_COUNT; i++) {
        const struct Event *ev = &game.event[i];
        const TbBool exists = flag_is_set(ev->flags, EvF_Exists) && (ev->owner == t->plyr);
        if (!baseline && exists && (!t->event_seen[i] || (t->event_kind[i] != ev->kind))) {
            put_at(o, KF_Event, ev->kind, (DaiSubtlCoord)coord_subtile(ev->mappos_x), (DaiSubtlCoord)coord_subtile(ev->mappos_y));
        }
        t->event_seen[i] = exists;
        t->event_kind[i] = ev->kind;
    }
}

/** The objective text changed (the level opening with one is history, not news). */
static void note_objective(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    const char *text = game.evntbox_text_objective[t->plyr];
    if ((text[0] == 0) || (strcmp(t->objective, text) == 0)) return;
    snprintf(t->objective, sizeof(t->objective), "%s", text);
    if (!baseline) put(o, KF_Objective, 0);
}

/** The keeper's prisoners now; returns how many (at most KEEPER_FACTS_CUSTODY). */
static int custody_now(PlayerNumber p, struct KeeperFactsPrisoner *out)
{
    int n = 0;
    int64_t idx = game.thing_lists[TngList_Creatures].index;
    for (int guard = 0; (idx > 0) && (guard < 4000) && (n < KEEPER_FACTS_CUSTODY); guard++) {
        const struct Thing *thing = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(thing)) break;
        idx = thing->next_of_class;
        if (!thing_is_creature(thing) || (thing->owner == p) || !creature_is_kept_in_custody_by_player(thing, p)) continue;
        const struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        struct KeeperFactsPrisoner *c = &out[n++];
        c->id = thing->index;
        c->creation_turn = thing->creation_turn;
        c->hungry = hunger_is_creature_hungry(thing);
        const TbBool tortured = creature_is_being_tortured(thing);
        c->hurt = tortured && (cctrl->max_health > 0) && (3 * thing->health < cctrl->max_health);
        c->on_rack = tortured;
        c->torture_points = tortured ? cctrl->tortured.accumulated_torture_points : 0;
    }
    return n;
}

static void put_custody(struct FactOut *o, enum KeeperFactCustody what, DaiThingIndex id)
{
    struct KeeperFact *f = put(o, KF_Custody, what);
    if (f != NULL) f->args[0] = id;
}

/** Arrived, went hungry, hurt on the rack, broke (points fell back), converted to the keeper, or gone. */
static void note_custody(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    struct KeeperFactsPrisoner now[KEEPER_FACTS_CUSTODY];
    const int n = custody_now(t->plyr, now);
    if (!baseline) {
        for (int i = 0; i < n; i++) {
            const struct KeeperFactsPrisoner *was = NULL;
            for (int k = 0; k < t->custody_count; k++) {
                if ((t->custody[k].id == now[i].id) && (t->custody[k].creation_turn == now[i].creation_turn)) { was = &t->custody[k]; break; }
            }
            if (was == NULL) { put_custody(o, KFC_New, now[i].id); continue; }
            if (now[i].hungry && !was->hungry) put_custody(o, KFC_Hungry, now[i].id);
            if (now[i].hurt && !was->hurt) put_custody(o, KFC_TortureHurt, now[i].id);
            // Points fall back on the rack only when the victim breaks; taken off it, they read 0 without any break.
            if (now[i].on_rack && (was->torture_points > 0) && (now[i].torture_points < was->torture_points)) put_custody(o, KFC_Interrogated, now[i].id);
        }
        for (int k = 0; k < t->custody_count; k++) {
            TbBool still = false;
            for (int i = 0; i < n; i++) if ((now[i].id == t->custody[k].id) && (now[i].creation_turn == t->custody[k].creation_turn)) still = true;
            if (still) continue;
            const struct Thing *thing = thing_get(t->custody[k].id);
            const TbBool ours = !thing_is_invalid(thing) && thing_is_creature(thing) && (thing->creation_turn == t->custody[k].creation_turn)
                && (thing->owner == t->plyr);
            put_custody(o, ours ? KFC_Converted : KFC_Gone, t->custody[k].id);
        }
    }
    memcpy(t->custody, now, sizeof(now[0]) * (size_t)n);
    t->custody_count = n;
}

/** A recipe fires, its victims' offered counts are reset (creature_sacrifice_reset): find which. The creature that
 *  completes it is counted and reset in the same turn, so its kind never shows a fall when none of it was offered
 *  before (a bug completing a bile demon and a bug reads as bile demons alone: two of them are a curse): the creatures
 *  sinking into the keeper's pool are followed, and one gone since the last look was sacrificed. */
static void note_sacrifices(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    const struct Dungeon *dungeon = get_players_dungeon(get_player(t->plyr));
    if (dungeon_invalid(dungeon)) return;
    if (!baseline) {
        TbBool sank[CREATURE_TYPES_MAX] = {false};
        TbBool reset[CREATURE_TYPES_MAX];
        TbBool any = false, any_sank = false;
        int64_t reset_kinds = 0, best_kinds = 0;
        int best = -1;
        for (int i = 0; i < t->sinking_count; i++) {
            const struct Thing *th = thing_get(t->sinking[i].id);
            if (!thing_exists(th) || !thing_is_creature(th) || (th->creation_turn != t->sinking[i].born)) {
                sank[t->sinking[i].model] = true;
                any_sank = true;
            }
        }
        for (int m = 0; m < CREATURE_TYPES_MAX; m++) {
            reset[m] = (dungeon->creature_sacrifice[m] < t->offered[m]) || (sank[m] && (dungeon->creature_sacrifice[m] == 0));
            reset_kinds += reset[m];
            any = any || reset[m];
        }
        if (any) {
            // The recipe whose kinds are just the ones reset (the first such: the engine checks in table order), with
            // one that sank among them; two fired in one look: the recipe of most kinds among those reset.
            for (int r = 0; r < MAX_SACRIFICE_RECIPES; r++) {
                const struct SacrificeRecipe *sac = &game.conf.rules[0].sacrifices.sacrifice_recipes[r];
                if (sac->action == SacA_None) continue;
                TbBool match = true, has_sank = !any_sank, seen[CREATURE_TYPES_MAX] = {false};
                int64_t kinds = 0;
                for (int i = 0; i < MAX_SACRIFICE_VICTIMS; i++) {
                    const DaiThingModel v = sac->victims[i];
                    if (v <= 0) continue;
                    if ((v >= CREATURE_TYPES_MAX) || !reset[v]) { match = false; break; }
                    if (!seen[v]) kinds++;
                    seen[v] = true;
                    has_sank = has_sank || sank[v];
                }
                if (!match || !has_sank || (kinds == 0) || (kinds <= best_kinds)) continue;
                best = r;
                best_kinds = kinds;
                if (kinds == reset_kinds) break;
            }
            if (best >= 0) {
                struct KeeperFact *f = put(o, KF_Sacrifice, 0);
                if (f != NULL) f->args[0] = best;
            }
        }
        if (dungeon->bodies_rotten_for_vampire < t->bodies_toward_vampire) put(o, KF_VampireProgress, 0);
    }
    memcpy(t->offered, dungeon->creature_sacrifice, sizeof(t->offered));
    t->bodies_toward_vampire = dungeon->bodies_rotten_for_vampire;
    t->sinking_count = 0;
    for (int64_t idx = game.thing_lists[TngList_Creatures].index, guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
        const struct Thing *th = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(th)) break;
        idx = th->next_of_class;
        if ((t->sinking_count < KEEPER_FACTS_SINKING) && (th->model > 0) && (th->model < CREATURE_TYPES_MAX) && creature_is_being_sacrificed(th)
            && (slabmap_owner(get_slabmap_for_subtile(th->mappos.x.stl.num, th->mappos.y.stl.num)) == t->plyr)) {
            struct KeeperFactsSinking *k = &t->sinking[t->sinking_count++];
            k->id = th->index;
            k->born = th->creation_turn;
            k->model = th->model;
        }
    }
}

/** Open ground behind the keeper's walls (somebody digging around the dungeon, as a keeper watching the wall would
 *  notice): a fact when a slab joins the set. */
static void note_open_behind_walls(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    int64_t xy[2 * KEEPER_FACTS_OPEN];
    int64_t n = keeper_open_behind_walls(t->plyr, xy, KEEPER_FACTS_OPEN);
    if (n > KEEPER_FACTS_OPEN) n = KEEPER_FACTS_OPEN;
    if (!baseline) {
        for (int64_t i = 0; i < n; i++) {
            TbBool known = false;
            for (int64_t k = 0; (k < t->open_count) && !known; k++) {
                known = (t->open_xy[2 * k] == xy[2 * i]) && (t->open_xy[2 * k + 1] == xy[2 * i + 1]);
            }
            if (!known) { put(o, KF_WallCue, 0); break; }
        }
    }
    memcpy(t->open_xy, xy, sizeof(xy[0]) * 2 * n);
    t->open_count = n;
}

/** The combat assistant hit one of its limits since the last look: a rule's budget or the set's cap, the reserve, or the
 *  gold guard switching a lasting power off. */
static void note_assist(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    (void)t; (void)o; (void)baseline; // no combat assistant in this build
}

/** Sight of Evil shows what it uncovers only while it is open: once it has stopped widening, a fact, once a cast. */
static void note_sight(struct KeeperFactsTracker *t, struct FactOut *o)
{
    const struct Dungeon *dungeon = get_players_num_dungeon(t->plyr);
    const struct Thing *eye = (dungeon->sight_casted_thing_idx > 0) ? thing_get(dungeon->sight_casted_thing_idx) : NULL;
    if ((eye == NULL) || thing_is_invalid(eye)) {
        t->sight_eye = 0;
        return;
    }
    if ((t->sight_eye != eye->index) || (t->sight_born != eye->creation_turn)) {
        t->sight_eye = eye->index; t->sight_born = eye->creation_turn;
        t->sight_open = 0; t->sight_still = 0; t->sight_told = false;
    }
    if (t->sight_told) return;
    const int64_t now = (int64_t)o->now;
    if (now == t->sight_turn) return; // settling is counted in game turns, not in looks (a paused game is still looked at)
    t->sight_turn = now;
    int64_t open = 0;
    for (int64_t y = 0; y < 2 * MAX_SOE_RADIUS; y++)
        for (int64_t x = 0; x < 2 * MAX_SOE_RADIUS; x++)
            if (dungeon->soe_explored_flags[y][x]) open++;
    if (open > t->sight_open) {
        t->sight_open = open;
        t->sight_still = 0;
        return;
    }
    if ((open > 0) && (++t->sight_still >= SIGHT_SETTLE_TURNS)) {
        put(o, KF_SightOpen, 0);
        t->sight_told = true;
    }
}

/** The map snapshot fits the level's map (a new map size starts it again, empty). */
static TbBool slabs_ready(struct KeeperFactsTracker *t)
{
    const int64_t w = game.map_tiles_x, h = game.map_tiles_y;
    if ((t->slabs != NULL) && (t->slabs_w == w) && (t->slabs_h == h)) return true;
    free(t->slabs);
    free(t->marked);
    t->slabs = NULL;
    t->marked = NULL;
    t->slabs_w = t->slabs_h = 0;
    if ((w <= 0) || (h <= 0)) return false;
    t->slabs = (uint16_t *)calloc((size_t)(w * h), sizeof(uint16_t));
    t->marked = (unsigned char *)calloc((size_t)(w * h), 1);
    if ((t->slabs == NULL) || (t->marked == NULL)) {
        free(t->slabs);
        free(t->marked);
        t->slabs = NULL;
        t->marked = NULL;
        return false;
    }
    t->slabs_w = w;
    t->slabs_h = h;
    return true;
}

/** Losses of one scan, one entry per kind of loss, room kind and taker, in the order they were first met. */
struct LossAgg {
    enum KeeperFactLoss what;
    RoomKind room_kind;
    PlayerNumber by;
    int64_t count;
    DaiSlabCoord x, y;
};
#define LOSS_KINDS 16

static void add_loss(struct LossAgg *agg, int *n, enum KeeperFactLoss what, RoomKind rkind, PlayerNumber by, DaiSlabCoord x, DaiSlabCoord y)
{
    for (int i = 0; i < *n; i++) {
        struct LossAgg *l = &agg[i];
        if ((l->what == what) && (l->room_kind == rkind) && (l->by == by)) { l->count++; return; }
    }
    if (*n >= LOSS_KINDS) return;
    struct LossAgg *l = &agg[(*n)++];
    l->what = what; l->room_kind = rkind; l->by = by; l->count = 1; l->x = x; l->y = y;
}

/** Ground and rooms the keeper lost since the last scan, from its own map (the engine's events cover only part of it):
 *  a slab it owned and does not now -- another keeper's (room or ground captured), nobody's (a room slab destroyed,
 *  floor or wall neutralised or dug through). Not a loss: what it still owns, a slab its own imps were marked to dig,
 *  a bridge sold back to water or lava. */
static void scan_losses(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    if (!slabs_ready(t)) return;
    const PlayerNumber p = t->plyr;
    const int64_t w = t->slabs_w, h = t->slabs_h;
    memset(t->marked, 0, (size_t)(w * h));
    const struct Dungeon *dungeon = get_players_num_dungeon(p);
    if (!dungeon_invalid(dungeon)) {
        const int64_t imax = (dungeon->highest_task_number < MAPTASKS_COUNT) ? dungeon->highest_task_number : MAPTASKS_COUNT;
        for (int64_t i = 0; i < imax; i++) {
            const struct MapTask *mt = &dungeon->task_list[i];
            if (mt->kind == 0) continue;
            const int64_t sx = DaiSubtileSlab(stl_num_decode_x(mt->coords)), sy = DaiSubtileSlab(stl_num_decode_y(mt->coords));
            if ((sx >= 0) && (sx < w) && (sy >= 0) && (sy < h)) t->marked[sy * w + sx] = 1;
        }
    }
    struct LossAgg agg[LOSS_KINDS];
    int nagg = 0;
    int64_t lost = 0; // bits 1 << KFL_*
    for (int64_t y = 0; y < h; y++) {
        const struct SlabMap *row = &game.slabmap[y * w];   // slabs_ready: w is the map's width
        for (int64_t x = 0; x < w; x++) {
            const struct SlabMap *slb = &row[x];
            const PlayerNumber owner = slabmap_owner(slb);
            uint16_t code = 0;
            if (owner == p) {
                code = SLAB_OWNED | (t->marked[y * w + x] ? SLAB_MARKED : 0);
                const struct Room *room = (slb->room_index > 0) ? room_get(slb->room_index) : NULL;
                if ((room != NULL) && !room_is_invalid(room)) code |= (uint16_t)(room->kind & 0xFF);
            }
            const int64_t i = y * w + x;
            const uint16_t was = t->slabs[i];
            t->slabs[i] = code;
            if (baseline || !(was & SLAB_OWNED) || (code & SLAB_OWNED)) continue;
            const RoomKind was_room = (RoomKind)(was & 0xFF);
            if ((owner != game.neutral_player_num) && (owner >= 0) && (owner < PLAYERS_COUNT)) {
                add_loss(agg, &nagg, was_room ? KFL_RoomCaptured : KFL_GroundCaptured, was_room, owner, (DaiSlabCoord)x, (DaiSlabCoord)y);
                lost |= was_room ? (1 << KFL_RoomCaptured) : (1 << KFL_GroundLost);
            } else if (was_room) {
                if (room_role_matches(was_room, RoRoF_PassWater | RoRoF_PassLava | RoRoF_PassAbyss)) continue; // a bridge sold
                add_loss(agg, &nagg, KFL_RoomDestroyed, was_room, -1, (DaiSlabCoord)x, (DaiSlabCoord)y);
                lost |= (1 << KFL_RoomDestroyed);
            } else if (!(was & SLAB_MARKED)) {
                add_loss(agg, &nagg, KFL_GroundLost, 0, -1, (DaiSlabCoord)x, (DaiSlabCoord)y);
                lost |= (1 << KFL_GroundLost);
            }
        }
    }
    for (int i = 0; i < nagg; i++) {
        struct KeeperFact *f = put_at(o, KF_Loss, agg[i].what, DaiSlabSubtileCenter(agg[i].x), DaiSlabSubtileCenter(agg[i].y));
        if (f != NULL) { f->args[0] = agg[i].room_kind; f->args[1] = agg[i].by; f->args[2] = agg[i].count; }
    }
    if (lost != 0) put(o, KF_LossScan, lost);
}

/** Enemy creatures on the keeper's ground (any slab it owns) that it is shown (not an invisible one): an enemy walking in, or tunnelling in
 *  and claiming, is the "enemy in your base" a keeper sees before anything is lost. Not an intruder: the keeper's own,
 *  neutral creatures, an ally's, a prisoner it holds, one knocked out, one in a hand. A fact for one the keeper has not
 *  seen on its ground in the last KEEPER_FACTS_INTRUDER_FORGET_TURNS turns. */
static void scan_intruders(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    const PlayerNumber p = t->plyr;
    const int64_t now = (int64_t)o->now;
    TbBool fresh = false;
    DaiSubtlCoord fresh_x = 0, fresh_y = 0;
    t->intruder_count = 0;
    t->intruder_total = 0;
    int64_t idx = game.thing_lists[TngList_Creatures].index;
    for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT); guard++) {
        struct Thing *thing = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(thing)) break;
        idx = thing->next_of_class;
        const PlayerNumber owner = thing->owner;
        if ((owner == p) || (owner == game.neutral_player_num) || (owner < 0) || (owner >= PLAYERS_COUNT)) continue;
        if (players_are_mutual_allies(p, owner) || thing_is_picked_up(thing)) continue;
        if (creature_is_kept_in_custody_by_player(thing, p) || creature_is_being_unconscious(thing)) continue;
        // One the keeper is not shown (an invisible enemy) is not one it hears about either.
        if (!keeper_creature_visible_to(thing, p)) continue;
        if (slabmap_owner(get_slabmap_for_subtile(thing->mappos.x.stl.num, thing->mappos.y.stl.num)) != p) continue;
        t->intruder_total++;
        if (t->intruder_count < KEEPER_FACTS_INTRUDERS_LISTED) {
            struct KeeperFactsIntruder *in = &t->intruders[t->intruder_count++];
            in->id = thing->index; in->model = thing->model; in->owner = owner;
            in->level = (int)creature_control_get_from_thing(thing)->exp_level + 1;
            in->x = thing->mappos.x.stl.num; in->y = thing->mappos.y.stl.num;
        }
        int k = 0;
        while ((k < t->remembered_count) && !((t->remembered[k].id == thing->index) && (t->remembered[k].born == thing->creation_turn))) k++;
        if (k == t->remembered_count) {
            if (!baseline && !fresh) { fresh_x = thing->mappos.x.stl.num; fresh_y = thing->mappos.y.stl.num; }
            if (!baseline) fresh = true;
            if (t->remembered_count < KEEPER_FACTS_INTRUDERS_REMEMBERED) {
                t->remembered_count++;
            } else {
                k = 0; // full: forget the one seen longest ago
                for (int j = 1; j < t->remembered_count; j++) if (t->remembered[j].last_seen < t->remembered[k].last_seen) k = j;
            }
            t->remembered[k].id = thing->index;
            t->remembered[k].born = thing->creation_turn;
        }
        t->remembered[k].last_seen = now;
    }
    for (int k = 0; k < t->remembered_count; ) {
        if (now - t->remembered[k].last_seen > KEEPER_FACTS_INTRUDER_FORGET_TURNS) t->remembered[k] = t->remembered[--t->remembered_count];
        else k++;
    }
    if (fresh) put_at(o, KF_Intruder, 0, fresh_x, fresh_y);
}

TbBool keeper_facts_trap_fired(const struct KeeperTrapLook *was, const struct KeeperTrapLook *now)
{
    if (now->rearm_turn > was->rearm_turn) return true;
    if ((was->shots != INFINITE_CHARGES) && (now->shots < was->shots)) return true;
    return !was->revealed && now->revealed;
}

static void put_trap(struct FactOut *o, TbBool own, DaiThingIndex id, DaiThingModel model, PlayerNumber owner, DaiSubtlCoord x, DaiSubtlCoord y)
{
    struct KeeperFact *f = put_at(o, own ? KF_TrapFired : KF_EnemyTrap, 0, x, y);
    if (f != NULL) { f->args[0] = id; f->args[1] = model; f->args[2] = owner; }
}

/** Traps going off: the keeper's own (something walked into it, or its hand slapped it) and an enemy's it sees fire (a
 *  hidden one revealing itself counts). Read off the traps themselves, since only an alarm trap raises an event. An
 *  enemy trap is watched only on ground the keeper has revealed; one spent and taken off the map the turn it fires is
 *  caught by the shot it left, whose parent it is. */
static void scan_traps(struct KeeperFactsTracker *t, struct FactOut *o, TbBool baseline)
{
    const PlayerNumber p = t->plyr;
    const int64_t now = (int64_t)o->now;
    struct KeeperFactsTrap *mem = t->traps;
    const int was_count = baseline ? 0 : t->trap_count;
    struct KeeperFactsTrap *next = t->traps_next;
    TbBool *matched = t->traps_matched;
    memset(matched, 0, sizeof(t->traps_matched));
    int n = 0;
    int64_t idx = game.thing_lists[TngList_Traps].index;
    for (int64_t guard = 0; (idx > 0) && (guard < THINGS_COUNT) && (n < KEEPER_FACTS_TRAPS); guard++) {
        const struct Thing *tr = thing_get((DaiThingIndex)idx);
        if (thing_is_invalid(tr)) break;
        idx = tr->next_of_class;
        if (!thing_is_deployed_trap(tr)) continue;
        const DaiSubtlCoord x = tr->mappos.x.stl.num, y = tr->mappos.y.stl.num;
        const TbBool own = (tr->owner == p);
        if (!own && (!players_are_enemies(p, tr->owner) || !subtile_revealed(x, y, p))) continue;
        struct KeeperFactsTrap *e = &next[n++];
        e->id = tr->index;
        e->born = tr->creation_turn;
        e->model = tr->model;
        e->x = x; e->y = y;
        e->look.shots = tr->trap.num_shots;
        e->look.rearm_turn = (int64_t)tr->trap.rearm_turn;
        e->look.revealed = tr->trap.revealed;
        for (int k = 0; k < was_count; k++) {
            if ((mem[k].id != e->id) || (mem[k].born != e->born)) continue;
            matched[k] = true;
            if (!keeper_facts_trap_fired(&mem[k].look, &e->look)) break;
            // An enemy's trap fires where the keeper sees it; a hidden one is seen once it has given itself away.
            if (!own && !tr->trap.revealed && get_trap_model_stats(tr->model)->hidden) break;
            put_trap(o, own, tr->index, tr->model, tr->owner, x, y);
            break;
        }
    }
    // A trap gone since the last look with shots left: if it left a shot behind this turn, that was its last firing.
    for (int k = 0; k < was_count; k++) {
        if (matched[k] || (mem[k].look.shots <= 0)) continue;
        int64_t si = game.thing_lists[TngList_Shots].index;
        for (int64_t guard = 0; (si > 0) && (guard < THINGS_COUNT); guard++) {
            const struct Thing *s = thing_get((DaiThingIndex)si);
            if (thing_is_invalid(s)) break;
            si = s->next_of_class;
            if ((s->parent_idx != mem[k].id) || (s->creation_turn + 1 < (DaiTurn)now)) continue;
            const TbBool own = (s->owner == p);
            if (!own && !subtile_revealed(s->mappos.x.stl.num, s->mappos.y.stl.num, p)) break;
            put_trap(o, own, mem[k].id, mem[k].model, s->owner, mem[k].x, mem[k].y);
            break;
        }
    }
    memcpy(mem, next, sizeof(next[0]) * (size_t)n);
    t->trap_count = n;
}

void keeper_facts_init(struct KeeperFactsTracker *t, PlayerNumber plyr_idx)
{
    memset(t, 0, sizeof(*t));
    t->plyr = plyr_idx;
}

void keeper_facts_free(struct KeeperFactsTracker *t)
{
    free(t->slabs);
    free(t->marked);
    t->slabs = NULL;
    t->marked = NULL;
    t->slabs_w = t->slabs_h = 0;
}

void keeper_facts_rebaseline(struct KeeperFactsTracker *t)
{
    t->baselined = false;
}

const char *keeper_facts_objective(const struct KeeperFactsTracker *t)
{
    return t->objective;
}

int64_t keeper_facts_update(struct KeeperFactsTracker *t, DaiTurn now, struct KeeperFact *out, int64_t max)
{
    struct FactOut o = { out, (out != NULL) ? max : 0, 0, now, t };
    const PlayerNumber p = t->plyr;
    if ((p < 0) || (p >= PLAYERS_COUNT)) return 0;
    TbBool kinds[KEEPER_FACTS_MODEL_BITS];
    owned_kinds(p, kinds);
    if (!t->baselined) {
        // The first look (or after a load): take the baseline, report nothing.
        uint16_t *slabs = t->slabs;
        unsigned char *marked = t->marked;
        const int64_t w = t->slabs_w, h = t->slabs_h;
        memset(t, 0, sizeof(*t));
        t->plyr = p;
        t->slabs = slabs; t->marked = marked; t->slabs_w = w; t->slabs_h = h;
        t->baselined = true;
        t->last_quarter = quarter_of(p);
        t->last_victory = get_player(p)->victory_state;
        note_objective(t, &o, true);
        note_custody(t, &o, true);
        note_sacrifices(t, &o, true);
        note_open_behind_walls(t, &o, true);
        note_assist(t, &o, true);
        scan_losses(t, &o, true);
        scan_intruders(t, &o, true);
        scan_traps(t, &o, true);
        t->scan_turn = (int64_t)now;
        memcpy(t->kind_owned, kinds, sizeof(kinds));
        note_events(t, &o, true);
        return 0;
    }
    const int64_t q = quarter_of(p);
    if (q != t->last_quarter) {
        put(&o, KF_PaydayQuarter, q);
        t->last_quarter = q;
    }
    note_events(t, &o, false);
    note_objective(t, &o, false);
    note_custody(t, &o, false);
    note_sacrifices(t, &o, false);
    note_open_behind_walls(t, &o, false);
    note_assist(t, &o, false);
    note_sight(t, &o);
    if (t->scan_turn != (int64_t)now) { // the map, intruders and traps: once per game turn
        t->scan_turn = (int64_t)now;
        scan_losses(t, &o, false);
        scan_intruders(t, &o, false);
        scan_traps(t, &o, false);
    }
    const unsigned char vic = get_player(p)->victory_state;
    if (vic != t->last_victory) {
        put(&o, KF_Victory, vic);
        t->last_victory = vic;
    }
    for (int m = 1; m < KEEPER_FACTS_MODEL_BITS; m++) {
        if (kinds[m] && !t->kind_owned[m]) put(&o, KF_NewKind, m);
        t->kind_owned[m] = kinds[m];
    }
    return o.n;
}

#ifdef __cplusplus
}
#endif
