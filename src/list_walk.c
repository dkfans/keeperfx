/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file list_walk.c
 *     Iterators for the sim's linked lists.
 * @par Purpose:
 *     The out-of-line parts of list_walk.h: starting a walk, and the error
 *     paths (an invalid element, a list longer than its guard).
 * @par Comment:
 *     Refactor pass 3, S01.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "list_walk.h"

#include "bflib_basics.h"
#include "creature_control.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static const char *list_walk_what(const struct ListWalk *walk)
{
    switch (walk->kind)
    {
    case LWalk_Creatures:
        return "creatures";
    case LWalk_RoomsOwner:
    case LWalk_RoomsKind:
        return "rooms";
    case LWalk_RoomSlabs:
        return "room slabs";
    default:
        return "things";
    }
}

struct ListWalk list_walk_start(int64_t first, unsigned char kind, uint64_t limit, const char *func)
{
    struct ListWalk walk;
    memset(&walk, 0, sizeof(walk));
    walk.next = first;
    walk.limit = limit;
    walk.func = func;
    walk.kind = kind;
    walk.live = 1;
    return walk;
}

DaiThingIndex players_creature_list_for_model(const struct Dungeon *dungeon, PlayerNumber plyr_idx, DaiThingModel crmodel)
{
    TbBool need_spec_digger = (crmodel > 0) && creature_kind_is_for_dungeon_diggers_list(plyr_idx, crmodel);
    if ((!need_spec_digger) || (crmodel == CREATURE_ANY) || (crmodel == CREATURE_NOT_A_DIGGER))
        return dungeon->creatr_list_start;
    return dungeon->digger_list_start;
}

void list_walk_invalid(struct ListWalk *walk)
{
    ERRORLOG("%s: Jump to invalid item when sweeping %s list", walk->func, list_walk_what(walk));
    walk->next = 0;
}

void list_walk_overflow(struct ListWalk *walk)
{
    ERRORLOG("%s: Infinite loop detected when sweeping %s list", walk->func, list_walk_what(walk));
    if (walk->mapblk != NULL)
        break_mapwho_infinite_chain(walk->mapblk);
    walk->next = 0;
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
