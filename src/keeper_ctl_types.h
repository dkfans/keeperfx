/**
 * @file keeper_ctl_types.h
 * @brief The keeper controller's types (keeper_ctl.h): verbs, the steps a verb becomes, results. Types and limits only,
 *        no functions, so code that plans gestures without driving the controller (Daimon's decision classes,
 *        docs/plans/ai-plans/v2/02-library-architecture.md section 3.2) may include it.
 *
 * The types keep their External seat names (ExtSeat*, EXTSEAT_*): the seat was their first user and api.c and the
 * ftests use them throughout. KeeperVerb, KeeperStep and KeeperCtlResult are the controller's names for them.
 */
#ifndef DK_KEEPER_CTL_TYPES_H
#define DK_KEEPER_CTL_TYPES_H

#include "globals.h"
#include "daimon_base_types.h"
#include "keeper_assist_rules.h"
#include "keeper_ctl_world.h"
#include "keeper_group.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Longest gesture, in steps: a serpentine dig over the largest allowed area, or a sell of the largest area. */
#define EXTSEAT_MAX_STEPS 160
/** Area limits (slabs) for the area verbs, so a request cannot occupy the seat for minutes. */
#define EXTSEAT_MAX_DIG_ROWS 32
/** The engine keeps at most MAPTASKS_COUNT (300) dig marks per player, so more than this cannot all be tagged. */
#define EXTSEAT_MAX_DIG_SLABS 256
#define EXTSEAT_MAX_BUILD_SLABS 256
#define EXTSEAT_MAX_SELL_SLABS 64
#define EXTSEAT_MAX_OVERCHARGE_TURNS 32
/** Creatures sent somewhere with move_creature are tracked and handed back automatically, so an order the agent forgets never
 *  costs a creature its pay or its food (see extseat_auto_releases). */
#define EXTSEAT_MAX_ORDERED 32
#define EXTSEAT_AUTO_RELEASE_RING 8
/** Release when an enemy attacks it, pay day is this close (turns), the creature is owed pay, or its hunger reaches 3/4
 *  of its limit. */
#define EXTSEAT_ORDER_PAYDAY_MARGIN_TURNS KEEPER_ORDER_PAYDAY_MARGIN_TURNS
/** The shortest hold an order may ask for. The default hold is a quarter of the pay day gap, the longest half of it. */
#define EXTSEAT_ORDER_MIN_HOLD_TURNS KEEPER_ORDER_MIN_HOLD_TURNS
/** How long a hand step waits for the hand to be free, or for the picked creature to reach it, before giving up. */
#define EXTSEAT_HAND_WAIT_TURNS 30
/** send_message: at most this many characters (the engine's PLAYER_MP_MESSAGE_LEN, less its terminator), and at most
 *  one message per this many turns -- short messages, not a stream. */
#define EXTSEAT_CHAT_MAX_LEN 63
#define EXTSEAT_CHAT_MIN_TURNS 100
/** How long a build_room waits for the seat's previous room drag to finish building before giving up (BUILD_BUSY). */
#define EXTSEAT_BUILD_WAIT_TURNS 600
/** Most things one pick_up_and_drop may carry (the hand's own limit, rules MaxThingsInHand, is checked too). */
#define EXTSEAT_MAX_HAND_SET 16
/** Verbs that may wait behind the running gesture (submitted with queue=true), per lane, and results remembered per seat. */
#define EXTSEAT_MAX_QUEUED_VERBS 16
/** The seat's queue has two lanes (docs/plans/llm-bridge-realtime.md, Phase 3): urgent verbs (priority "now") run before
 *  routine ones, and a long routine verb that can be safely cut (a room build, a dig or unmark, a sell, a rule change) is
 *  cut for one: it goes back to the front of its lane and is planned again when it resumes. */
#define EXTSEAT_LANE_NOW 0
#define EXTSEAT_LANE_ROUTINE 1
/** The seat's Daimon lieutenant (docs/plans/ai-plans/v2/10 section 7.1, P8.2): below routine, so it never cuts an
 *  agent's verb; any agent verb waiting cuts a running lieutenant verb that can be cut, which goes back to the front of
 *  its lane. */
#define EXTSEAT_LANE_LIEUTENANT 2
#define EXTSEAT_LANES 3
#define EXTSEAT_RESULT_RING 16

enum ExtSeatVerbKind {
    ESV_None = 0,
    ESV_PlaceTrap,
    ESV_PlaceDoor,
    ESV_Slap,
    ESV_PickUp,
    ESV_Drop,
    ESV_CastPower,
    ESV_BuildRoom,
    ESV_MarkDig,
    ESV_Sell,
    ESV_Cancel,
    ESV_MoveCreature,    /**< send one of our creatures to a subtile (the cheat menu's Order creature mode); it holds there until released */
    ESV_SetTendency,     /**< imprison / flee tendency of all the seat's creatures: name "imprison" or "flee", `enabled` */
    ESV_ReleaseCreature, /**< hand an ordered creature back to its normal behaviour */
    ESV_SetAlliance,     /**< declare (or withdraw) alliance with `target_player`: `enabled` */
    ESV_PickUpAndDrop,   /**< pick things up with the hand (our creatures, our prisoners, food) and drop them at one spot; one the hand may not take now is skipped (PARTIAL) */
    ESV_UnmarkDig,       /**< take the dig marks off the marked slabs of a rectangle */
    ESV_SetDoorLock,     /**< lock (`enabled`) or unlock one of our doors, at `pos` */
    ESV_PowerOff,        /**< end a lasting power early: POWER_CALL_TO_ARMS, POWER_SIGHT, POWER_OBEY */
    ESV_UseSpecial,      /**< use a dungeon special box (thing_id); resurrect needs `name` + `level`, transfer `target_thing` */
    ESV_SendMessage,     /**< a short chat message to every player (`message`) */
    ESV_SetAssist,       /**< replace the combat assistant's rules and globals (`assist_rules`, `assist_globals`):
                          *   only what changed is sent, as packets that go live together (keeper_assist_packets.h) */
    ESV_AssistEnable,    /**< switch the combat assistant on or off (`enabled`) without touching its rules */
    ESV_Attack,          /**< send one of our creatures (thing_id) to fight what is at `pos`: an enemy door, heart, trap or room, or
                          *   the enemies there. It walks there held (as move_creature), then fights as at a Call to Arms banner */
    ESV_AssistUseBank,   /**< make one of the combat assistant's banks (assist_bank) the live set: one packet */
    ESV_GiveGold,        /**< take gold with the hand -- from a treasury hoard (target_thing, or else the seat's largest), or a
                          *   loose pile -- and drop it on one of our creatures (thing_id), as a human pays one by hand: it
                          *   counts as pay, cheers the creature up, and settles a pay day it is owed. A grab from a hoard
                          *   takes what the hand takes (a quarter of it, at least 100) */
    ESV_AssistDelegate,  /**< the seat's Daimon lieutenant takes its combat-assistant rules over, or hands them back (`enabled`):
                          *   one PckA_AssistDelegate (Daimon P8.7; not an agent's verb) */
    ESV_COUNT,           /**< not a verb: one past the last (a saved verb's range check, keeper_ctl.c ki_verb) */
};

/** A verb as the agent expressed it, already parsed out of JSON. Positions are subtile coordinates. */
struct ExtSeatVerb {
    enum ExtSeatVerbKind kind;
    char name[40];          /**< trap / door / power code name, when the verb has one */
    TbBool has_pos;
    int64_t stl_x, stl_y;
    TbBool has_thing;
    int64_t thing_id;
    TbBool has_rect;        /**< area verbs: an inclusive rectangle of SLABS, corners in any order */
    int64_t slab_x0, slab_y0, slab_x1, slab_y1;
    int64_t overcharge_turns; /**< cast_power on a subtile: turns to hold before releasing (level = turns / 4) */
    TbBool has_enabled;     /**< set_tendency: `enabled` was given */
    TbBool enabled;
    int64_t hold_turns;     /**< move_creature: automatic release after this many turns (0 = a quarter of the pay day gap) */
    int64_t expires_turn;   /**< 0 = never; else the verb is refused/dropped if it has not started by this game turn */
    TbBool has_target_player; /**< set_alliance: `target_player` was given */
    int64_t target_player;
    int64_t thing_ids[EXTSEAT_MAX_HAND_SET]; /**< pick_up_and_drop: several things in one trip (in place of thing_id) */
    int64_t thing_count;
    TbBool has_room;        /**< pick_up_and_drop / drop: `room_id` in place of `pos`, resolved to a spot inside it */
    int64_t room_id;
    TbBool has_direction;   /**< slap on a trap that shoots (a boulder): the way it goes, as a map angle */
    int64_t direction_angle;
    int64_t level;          /**< use_special (resurrect): the dead creature's level, 1-based */
    TbBool has_target_thing; /**< use_special (transfer): the creature to transfer */
    int64_t target_thing;
    char message[64];       /**< send_message: the text (PLAYER_MP_MESSAGE_LEN) */
    TbBool release;         /**< hand drops: dropping a prisoner outside your prison/torture chamber frees it; say so */
    TbBool sacrifice;       /**< hand drops: onto a temple pool (sacrificing the creature); refused otherwise */
    TbBool imprison;        /**< hand drops: yes, put this creature of the seat's own in its prison (it is jailed) */
    TbBool torture;         /**< hand drops: yes, put this creature of the seat's own on its torture chamber's rack */
    TbBool dismiss;         /**< hand drops: yes, drop this creature on the seat's portal (it leaves the dungeon for good) */
    TbBool lose_spells;     /**< sell: yes, sell library slabs whose spellbooks have nowhere else to go (the spells are lost) */
    struct KeeperAssistRule assist_rules[KEEPER_ASSIST_RULES]; /**< set_assist: the whole wanted set, slot = index */
    struct KeeperAssistGlobals assist_globals;                 /**< set_assist */
    uint8_t assist_given;   /**< set_assist: what the request gave (enum KeeperAssistGiven); the rest is taken from the set
                             *   as it is when the verb runs (keeper_assist_overlay). 0: assist_rules / assist_globals whole */
    int64_t assist_bank;    /**< set_assist: 0 = the live set; 1..KEEPER_ASSIST_BANKS = store it in that bank instead.
                             *   use_assist_bank: the bank to make live */
    TbBool priority;        /**< priority "now": the urgent lane (EXTSEAT_LANE_NOW), ahead of every routine verb */
    int64_t cancel_id;      /**< cancel: just this verb (0 = all, unless cancel_lane) */
    int64_t cancel_lane;    /**< cancel: every verb waiting in lane cancel_lane - 1 (0 = none); the running one is not touched */
};

/** What became of a submitted verb: it finished writing its steps, or was refused when its turn came. */
struct ExtSeatResult {
    int64_t id;
    TbBool rejected;
    char error[28];         /**< the error code when rejected ("EXPIRED", "NOT_AVAILABLE", "CANCELLED", ...) */
    int64_t turn;           /**< game turn it was decided */
    int64_t submitted;      /**< game turn it was accepted */
    int64_t started;        /**< game turn it became the running gesture (-1: it never started) */
    int64_t lane;           /**< EXTSEAT_LANE_NOW, EXTSEAT_LANE_ROUTINE or EXTSEAT_LANE_LIEUTENANT */
    int64_t suspended;      /**< times it was cut for an urgent verb (and planned again after it) */
};

/** The queue as it stands, for the view (seat.queue). */
struct ExtSeatQueueInfo {
    int64_t running_id;     /**< 0 = nothing running (or only raw steps) */
    int64_t running_lane;
    int64_t running_steps_left;
    int64_t lane_ids[EXTSEAT_LANES][EXTSEAT_MAX_QUEUED_VERBS];
    TbBool lane_suspended[EXTSEAT_LANES][EXTSEAT_MAX_QUEUED_VERBS]; /**< cut for an urgent verb; resumes from here */
    int64_t lane_count[EXTSEAT_LANES];
};

/** A creature handed back to its own behaviour by the safety net rather than by the agent. */
struct ExtSeatAutoRelease {
    int64_t thing_id;
    char reason[16];        /**< "attacked", "max_hold", "payday", "owed_pay", "hungry", "agent_lost", "seat_released", "released",
                             *   "attacking" (an attack order arrived and set it fighting) */
    int64_t turn;
};

/** Outcome of a submit: how many steps it needs, its id, and where it stands. */
struct ExtSeatSubmitInfo {
    int64_t steps;
    int64_t id;
    int64_t queued_behind;  /**< verbs ahead of it (0 = starts now) */
};

struct ExtSeatStep {
    unsigned char action;
    int64_t par1, par2, par3, par4;
    TbBool has_pos;
    int64_t pos_x, pos_y;   /**< map coordinates (subtile centre), not subtiles */
    uint64_t control_flags;
    unsigned char context;  /**< cursor state carried with the position (CSt_PickAxe for digging) */
    DaiThingIndex follow_thing; /**< if set, the position is that creature's subtile at the moment the step is written */
    TbBool hand_set_drop;   /**< drops the top of the hand if it is one of the running verb's things (pick_up_and_drop);
                             *   when none of them is left in the hand the verb ends: PICK_UP_FAILED if none arrived,
                             *   PARTIAL if only some did -- whatever else might be held is never dropped */
    TbBool send_chat;       /**< the running verb's message goes into the player's pending chat message before this step */
    TbBool wait_build_free; /**< held back while the seat's previous room drag is still being built (one slab per turn):
                             *   the engine drops a new drag outright while one is in progress */
    TbBool wait_hand_free;  /**< held back while the hand is still grabbing or dropping (the engine ignores a hand action
                             *   then); gives up with HAND_BUSY after EXTSEAT_HAND_WAIT_TURNS */
};

/* struct KeeperCtlWorld, what the controller's planning asks about the map: keeper_ctl_world.h (kfx_ai, where the hand
 * reflexes' decisions take it too). */

typedef struct ExtSeatVerb KeeperVerb;
typedef struct ExtSeatStep KeeperStep;
typedef struct ExtSeatResult KeeperCtlResult;

#ifdef __cplusplus
}
#endif

#endif
