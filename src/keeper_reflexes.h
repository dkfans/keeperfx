/** @file keeper_reflexes.h
 *     The combat assistant's hand rules, decided (docs/plans/ai-plans/v2/06-phase4-defence.md section 8, D8):
 *     evacuate_diggers (from enemy fighters and rolling boulders), lift_wounded and reinforce, as the External seat has
 *     carried them out (seat_reflexes.h has what each does), on candidate lists the caller builds.
 * @par Purpose:
 *     One decision for two drivers: the seat builds its candidates from the game and its world from the controller's
 *     default answers; Daimon's Tactics from its knowledge (visible enemies only) and its own world. The decision reads
 *     nothing but its input and the world's answers, and acts only through the caller's `exec`.
 */
#ifndef DK_KEEPER_REFLEXES_H
#define DK_KEEPER_REFLEXES_H

#include "globals.h"
#include "daimon_base_types.h"
#include "keeper_assist.h"
#include "keeper_assist_rules.h"
#include "keeper_ctl_world.h"

#ifdef __cplusplus
extern "C" {
#endif

/** What a candidate is to the rules. */
enum KeeperReflexRole {
    KRR_Fighter = 1,   /**< one of the keeper's creatures, not a digger, the hand could take now */
    KRR_Digger,        /**< one of its diggers the hand could take now */
    KRR_Wounded,       /**< one of its fighters in a fight (the assistant's candidate), the hand could take now */
    KRR_Enemy,         /**< an enemy fighter (not a digger) the keeper sees */
    KRR_Boulder,       /**< a rolling boulder the keeper sees, or its own */
    KRR_BattleOwn,     /**< one of its creatures in a battle (`battle`), not dying or unconscious */
    KRR_BattleEnemy,   /**< an enemy in that battle the keeper sees */
};

/** One candidate, as the caller sees it. A battle's members come together, battle by battle in the order to weigh them;
 *  the other roles in the order the rules should meet them (the seat: the game's creature lists). */
struct KeeperReflexCandidate {
    DaiThingIndex thing;
    uint8_t role;                       /**< enum KeeperReflexRole */
    TbBool ordered;                     /**< own: an order of the driver holds it (left alone) */
    TbBool digger;                      /**< own: a digger (reinforce never sends one) */
    DaiSubtlCoord x, y;
    int64_t pos_x, pos_y, size;         /**< map units, and its clipbox width (boulders and the creatures they may hit) */
    int64_t force, health_pct;
    BattleIndex battle;                 /**< own fighters: the battle they fight in (0 none); battle members: theirs */
    int dx, dy;                         /**< boulders: heading, unit steps */
    int64_t speed;                      /**< boulders: map units a turn */
    int64_t lane_clear;                 /**< boulders: subtiles ahead to its first wall (the creatures past it are safe) */
    struct KeeperAssistCandidate ac;    /**< own creatures: the assistant's facts, for the rules' conditions */
};

struct KeeperReflexInput {
    PlayerNumber plyr;
    const struct KeeperReflexCandidate *cands;
    int64_t n;
    const struct KeeperAssistRule *rules;       /**< KEEPER_ASSIST_RULES slots: the live rules */
    const int64_t *dungeon_facts;               /**< enum KeeperAssistFact */
    const int64_t *last_fire_turn;              /**< per slot: when it last fired (0 never) */
    int64_t now;
    int64_t hand_max;                           /**< the rules' max_things_in_hand */
    int64_t lair_room;                          /**< lift_wounded's lair when its rule names no spot (0: none) */
    TbBool heal_ready;                          /**< a heal rule of the assistant could heal now: lift_wounded waits */
};

/** One lift: the creatures, and where they go -- a subtile, or into room `room_id` when it is > 0 (x the room, y -1). */
struct KeeperReflexLift {
    int slot;
    uint8_t action;                     /**< enum KeeperAssistAction */
    uint8_t count;
    DaiThingIndex things[KEEPER_ASSIST_HAND_MAX];
    int64_t x, y, room_id;
    DaiThingIndex boulder;                 /**< evacuate_diggers: the boulder they are lifted out of the way of (0: a creature) */
};

/** Carries a lift out (the seat: queues the pick-up-and-drop); NULL done, else why not. */
typedef const char *(*KeeperReflexExec)(void *ctx, const struct KeeperReflexLift *lift);

/** Turns a grab takes (the hand's PI_Grab instance): the hand holds a creature this long after it is told to take it. */
#define KEEPER_REFLEX_GRAB_TURNS 3
/** Turns from a firing to the hand holding the `k`-th creature of its lift (0: the first). */
int64_t keeper_reflexes_lift_turns(int64_t k);
/** Turns until a boulder at (bx, by) rolling (dx, dy) at `speed` map units a turn reaches a creature at (cx, cy) (map
 *  units), touching when `reach` apart along its heading and no more than `reach` to its side: 0 if they touch already,
 *  -1 if out of its way or the boulder is not moving. */
int64_t keeper_reflexes_impact_turns(int64_t bx, int64_t by, int dx, int dy, int64_t speed, int64_t cx, int64_t cy, int64_t reach);

/** The hand rules in slot order, each after its cooldown (a boulder's lift before it, and before any lift for an enemy
 *  creature): the first that decides a lift has `exec` carry it out. A lift `exec` refuses becomes that slot's refusal
 *  and the next rule is tried. `refusal` (KEEPER_ASSIST_RULES entries) gets why a matching rule held back, and NULL
 *  for the one that fired. True if one fired. */
TbBool keeper_reflexes_decide(const struct KeeperReflexInput *in, const struct KeeperCtlWorld *world, KeeperReflexExec exec,
    void *exec_ctx, const char **refusal);

#ifdef __cplusplus
}
#endif
#endif
