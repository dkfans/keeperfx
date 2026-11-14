/** @file keeper_touch_types.h
 *  What a keeper's human just did (docs/plans/ai-plans/v2/09 section 5.3, Daimon P7.11): the touch log the game records
 *  around the human's own packet (keeper_touch_log.h, kfx_game) and Daimon's assistant turns into leases (DaiLeases) --
 *  so the assistant never undoes the human: a creature they handled is theirs a while, a slab they marked or unmarked,
 *  a room, trap or door they built, placed or sold stays as they left it. Types only.
 */
#ifndef KFX_KEEPER_TOUCH_TYPES_H
#define KFX_KEEPER_TOUCH_TYPES_H

#include <stdint.h>

enum KeeperTouchKind {
    KTch_None = 0,
    KTch_DigTag,        /**< a slab marked for digging (x, y) */
    KTch_DigUntag,      /**< a slab's mark taken off (x, y) */
    KTch_RoomBuilt,     /**< a slab of a room built, or a build started over it (x, y; detail: the room kind) */
    KTch_RoomSold,      /**< a room's slab sold, or a sale started over it (x, y) */
    KTch_TrapPlaced,    /**< a trap placed (x, y; detail: its kind) */
    KTch_TrapSold,      /**< a trap sold (x, y) */
    KTch_DoorPlaced,    /**< a door placed (x, y; detail: its kind) */
    KTch_DoorSold,      /**< a door sold (x, y) */
    KTch_Creature,      /**< a creature picked up, dropped, slapped, cast on or possessed (thing) */
    KTch_Count
};

struct KeeperTouch {
    uint8_t kind;           /**< KeeperTouchKind */
    int32_t x, y;           /**< the slab, for a slab's touch */
    int32_t thing;          /**< the thing, for a creature's */
    uint64_t thing_created; /**< its creation turn: a reused index is another creature (14 F5) */
    int32_t detail;         /**< the room, trap or door kind */
};

#endif
