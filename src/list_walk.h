/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file list_walk.h
 *     Iterators for the sim's linked lists.
 * @par Purpose:
 *     One implementation of the list walk the sim, AI, scripts and UI repeat:
 *     fetch the element, stop on an invalid one, step to the next index,
 *     and guard against a cycle. Refactor pass 3, S01.
 * @par Comment:
 *     A walk is a `for` header and its body:
 *
 *         FOR_EACH_THING(thing, thing_walk_map_block(mapblk))
 *         {
 *             if (thing_is_object(thing))
 *                 return thing;
 *         }
 *
 *     `break`, `continue` and `return` work as in a plain loop; the number of
 *     elements handed out is `thing_walk.count` (the walk variable is named
 *     after the element).
 *
 *     Thing and room walks read the next index before the body runs, so the
 *     body may delete the current element. Room-slab walks step after the
 *     body, as most of their hand-written loops did; the _ahead starters read
 *     the next slab before the body, for loops that stepped first.
 *
 *     Each walk stops when the guard is exceeded: the count of elements
 *     handed out goes above the walk's limit. Walks over a class list whose
 *     length changes while it is walked (a body that deletes things) keep
 *     reading the list's own count, as the hand-written loops did.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_LISTWALK_H
#define DK_LISTWALK_H

#include "globals.h"
#include "daimon_base_types.h"
#include "thing_data.h"
#include "thing_list.h"
#include "creature_control.h"
#include "room_data.h"
#include "slab_data.h"
#include "map_data.h"
#include "dungeon_data.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
enum ListWalkKind {
    LWalk_MapBlock = 1, /**< things on a map block, `next_on_mapblk` */
    LWalk_Class,        /**< things of a class list, `next_of_class` */
    LWalk_Creatures,    /**< a player's creatures, `CreatureControl.players_next_creature_idx` */
    LWalk_RoomsOwner,   /**< rooms of a player and kind, `next_of_owner` */
    LWalk_RoomsKind,    /**< rooms linked by kind, `next_of_kind` */
    LWalk_RoomSlabs,    /**< slabs of a room, get_next_slab_number_in_room() */
};

/** State of one walk. Made by a thing_walk_*() / room_walk_*() starter. */
struct ListWalk {
    int64_t next;                        /**< index handed out by the next step; 0 ends the walk */
    uint64_t count;                      /**< elements handed out so far */
    uint64_t limit;                      /**< the guard: stop once count goes above it */
    const struct StructureList *slist;   /**< class walks: read slist->count instead of limit */
    const struct Room *room;             /**< slab walks: read room->slabs_count instead of limit */
    const struct Map *mapblk;            /**< map-block walks: the chain broken on overflow, or NULL */
    const char *func;                    /**< the caller, for thing_get()'s log */
    unsigned char kind;                  /**< enum ListWalkKind */
    unsigned char started;               /**< slab walks: the first slab was handed out */
    unsigned char step_first;            /**< slab walks: read the next slab before the body, not after */
    unsigned char live;                  /**< used by the FOR_EACH macros */
};

/* Out of line: the rare error paths. */
void list_walk_invalid(struct ListWalk *walk);
void list_walk_overflow(struct ListWalk *walk);
struct ListWalk list_walk_start(int64_t first, unsigned char kind, uint64_t limit, const char *func);
DaiThingIndex players_creature_list_for_model(const struct Dungeon *dungeon, PlayerNumber plyr_idx, DaiThingModel crmodel);

/******************************************************************************/
/** Things on a map block (THINGS_COUNT guard; a cycle is broken on overflow). */
#define thing_walk_map_block(mapblk) thing_walk_map_block_f(mapblk, __func__)
/** Things on a map-block chain starting at an index, with no block to repair. */
#define thing_walk_mapwho_from(first) list_walk_start(first, LWalk_MapBlock, THINGS_COUNT, __func__)
/** Things of a class list, with a fixed guard. */
#define thing_walk_list(first, limit) list_walk_start(first, LWalk_Class, limit, __func__)
/** Things of a class list, guarded by the list's live count. */
#define thing_walk_structure_list(slist) thing_walk_structure_list_f(slist, __func__)
/** A player's creature (or digger) list, with a fixed guard. */
#define thing_walk_creatures(first, limit) list_walk_start(first, LWalk_Creatures, limit, __func__)
/**
 * The creature list to walk for a creature model: a player's digger list when the
 * model is one of their special diggers, otherwise their creature list (CREATURES_COUNT
 * guard). plyr_idx is the player whose digger kinds decide it.
 */
#define thing_walk_players_creatures_of_model(dungeon, plyr_idx, crmodel) \
    list_walk_start(players_creature_list_for_model(dungeon, plyr_idx, crmodel), LWalk_Creatures, CREATURES_COUNT, __func__)
/** Rooms of a player and kind (ROOMS_COUNT guard). */
#define room_walk_owner(first) list_walk_start(first, LWalk_RoomsOwner, ROOMS_COUNT, __func__)
/** Rooms linked by kind (ROOMS_COUNT guard). */
#define room_walk_kind(first) list_walk_start(first, LWalk_RoomsKind, ROOMS_COUNT, __func__)
/** Slabs of a room, guarded by the room's live slab count; steps after the body. */
#define room_slab_walk(room) room_slab_walk_f(room, 0, __func__)
/** Slabs of a room from a first slab, with a fixed guard (UINT64_MAX: none); steps after the body. */
#define room_slab_walk_from(first, limit) room_slab_walk_from_f((int64_t)(first), limit, 0, __func__)
/**
 * As room_slab_walk() / room_slab_walk_from(), but the next slab is read before the body
 * runs, as in a loop that steps first: use it when the body may unlink or relink the
 * current slab (kill_room_slab_and_contents(), create_room(), ...).
 */
#define room_slab_walk_ahead(room) room_slab_walk_f(room, 1, __func__)
#define room_slab_walk_ahead_from(first, limit) room_slab_walk_from_f((int64_t)(first), limit, 1, __func__)

static inline struct ListWalk thing_walk_map_block_f(const struct Map *mapblk, const char *func)
{
    struct ListWalk walk = list_walk_start(get_mapwho_thing_index(mapblk), LWalk_MapBlock, THINGS_COUNT, func);
    walk.mapblk = mapblk;
    return walk;
}

static inline struct ListWalk thing_walk_structure_list_f(const struct StructureList *slist, const char *func)
{
    struct ListWalk walk = list_walk_start(slist->index, LWalk_Class, 0, func);
    walk.slist = slist;
    return walk;
}

static inline struct ListWalk room_slab_walk_f(const struct Room *room, unsigned char step_first, const char *func)
{
    struct ListWalk walk = list_walk_start((int64_t)room->slabs_list, LWalk_RoomSlabs, 0, func);
    walk.room = room;
    walk.step_first = step_first;
    return walk;
}

static inline struct ListWalk room_slab_walk_from_f(int64_t first, uint64_t limit, unsigned char step_first, const char *func)
{
    struct ListWalk walk = list_walk_start(first, LWalk_RoomSlabs, limit, func);
    walk.step_first = step_first;
    return walk;
}

static inline TbBool list_walk_over_limit(const struct ListWalk *walk)
{
    if (walk->slist != NULL)
        return walk->count > walk->slist->count;
    if (walk->room != NULL)
        return (int64_t)walk->count > walk->room->slabs_count;
    return walk->count > walk->limit;
}

/** The next thing of a thing walk, or NULL at the end, on an invalid element, or at the guard. */
static inline struct Thing *thing_walk_next(struct ListWalk *walk)
{
    if (list_walk_over_limit(walk)) {
        list_walk_overflow(walk);
        return NULL;
    }
    if (walk->next == 0)
        return NULL;
    struct Thing *thing = thing_get_f(walk->next, walk->func);
    if (thing_is_invalid(thing)) {
        list_walk_invalid(walk);
        return NULL;
    }
    switch (walk->kind)
    {
    case LWalk_MapBlock:
        walk->next = thing->next_on_mapblk;
        break;
    case LWalk_Class:
        walk->next = thing->next_of_class;
        break;
    default: {
        struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl)) {
            list_walk_invalid(walk);
            return NULL;
        }
        walk->next = cctrl->players_next_creature_idx;
        break;
    }
    }
    walk->count++;
    return thing;
}

/** The next room of a room walk, or NULL. */
static inline struct Room *room_walk_next(struct ListWalk *walk)
{
    if (list_walk_over_limit(walk)) {
        list_walk_overflow(walk);
        return NULL;
    }
    if (walk->next == 0)
        return NULL;
    struct Room *room = room_get(walk->next);
    if (room_is_invalid(room)) {
        list_walk_invalid(walk);
        return NULL;
    }
    walk->next = (walk->kind == LWalk_RoomsKind) ? room->next_of_kind : room->next_of_owner;
    walk->count++;
    return room;
}

/**
 * Steps a slab walk: sets *slb_num to the next slab of the room and returns true,
 * or returns false at the end, on an invalid slab, or at the guard. By default the
 * step to the following slab happens here, after the previous body ran; with
 * step_first it happens as the slab is handed out, before its body.
 */
static inline TbBool room_slab_walk_next(struct ListWalk *walk, SlabCodedCoords *slb_num)
{
    if (walk->started) {
        if (list_walk_over_limit(walk)) {
            list_walk_overflow(walk);
            return false;
        }
        if (!walk->step_first)
            walk->next = (int64_t)get_next_slab_number_in_room((SlabCodedCoords)walk->next);
    }
    walk->started = 1;
    if (walk->next == 0)
        return false;
    if (slabmap_block_invalid(get_slabmap_direct((SlabCodedCoords)walk->next))) {
        list_walk_invalid(walk);
        return false;
    }
    walk->count++;
    *slb_num = (SlabCodedCoords)walk->next;
    if (walk->step_first)
        walk->next = (int64_t)get_next_slab_number_in_room(*slb_num);
    return true;
}

/******************************************************************************/
#define FOR_EACH_THING(thing, start) \
    for (struct ListWalk thing##_walk = (start); thing##_walk.live; thing##_walk.live = 0) \
        for (struct Thing *thing; (thing = thing_walk_next(&thing##_walk)) != NULL; )

#define FOR_EACH_ROOM(room, start) \
    for (struct ListWalk room##_walk = (start); room##_walk.live; room##_walk.live = 0) \
        for (struct Room *room; (room = room_walk_next(&room##_walk)) != NULL; )

#define FOR_EACH_ROOM_SLAB(slb_num, start) \
    for (struct ListWalk slb_num##_walk = (start); slb_num##_walk.live; slb_num##_walk.live = 0) \
        for (SlabCodedCoords slb_num = 0; room_slab_walk_next(&slb_num##_walk, &slb_num); )

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
