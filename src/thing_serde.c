/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_serde.c
 *
 * @par Purpose:
 *     Serialize and deserialize things
 * @par Comment:
 *     None
 * @author   Sim
 * @date     08 Sep 2020 - 08 Sep 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "console_cmd.h"
#include "globals.h"

#include "game_legacy.h"
#include "thing_data.h"

void serde_pre__things()
{
    struct CreatureControl cctrl_data[CREATURES_COUNT];
    for (int i = 0; i < CREATURES_COUNT; i++)
    {
    }

    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing *thing = &game.things_data[i];
        if ((thing->alloc_flags & TAlF_Exists) == 0)
        {
            // TODO: memset to zero
            NETDBG(6, "free i:%04d idx:%04d -> %04d", i, thing->index, game.free_things[i-1]);
            continue;
        }
        else
        {
            NETDBG(6, "item i:%04d idx:%04d", i, thing->index);
        }
    }
}

static int compare_shorts(const void *a_, const void *b_)
{
    const short *a = (const short*)a_;
    const short *b = (const short*)b_;
    return (*a < *b)? -1 : ((*a > *b)? 1 : 0 );
}

void serde_post_things()
{
    struct StructureList* slist;

#ifdef DUMP_THINGS
    mkdir("dump");
#endif

    NETDBG(5, "game.free_things_start_index:%d", game.free_things_start_index);
    game.free_things_start_index = 0;

    for (int i = 0; i < ((int)TngList_DynamLights)+1; i++) // What about caveins? where are they?
    {
        game.thing_lists[i].index = 0;
        game.thing_lists[i].count = 0;
    }

#if NETDBG_LEVEL > 6
    // Here we want to find unexpectedly allocated things
    for (int i = 1; i < THINGS_COUNT; i++)
    {
        int idx = game.free_things[i-1];
        if (idx != 0)
        {
            struct Thing *thing = thing_get(idx);
            if (thing->alloc_flags & TAlF_Exists)
            {
                NETDBG(5, "newly allocated thing idx:%d", idx);
            }
        }
    }
    // Here we want to find unexpectedly deleted things
    short sorted[THINGS_COUNT];
    memcpy(sorted, game.free_things, sizeof(sorted));
    qsort(sorted, THINGS_COUNT, sizeof(short), &compare_shorts);

    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing *thing = &game.things_data[i];
        if ((thing->alloc_flags & TAlF_Exists) == 0)
        {
            short *del_idx = bsearch(&thing->index, sorted, THINGS_COUNT, sizeof(short), &compare_shorts);
            if (del_idx == NULL)
            {
                NETDBG(5, "newly deleted thing idx:%d", thing->index);
            }
        }
    }
#endif

    // moving free_things_start_index upwards for each allocated item
    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing *thing = thing_get(i);
#ifdef DUMP_THINGS
        char buf[64];
        sprintf(buf, "dump/%d", i);
        FILE *F = fopen(buf, "w");
        if (F)
        {
            fwrite(thing, sizeof(struct Thing), 1, F);
            fclose(F);
        }
#endif
        if (thing->alloc_flags & TAlF_Exists)
        {
            game.free_things[game.free_things_start_index] = 0;
            game.free_things_start_index++;
        }
    }

    // TODO: clear mapwho data for whole map?

    // accounting free_things
    int free_idx = game.free_things_start_index;
    for (int i = 1; i < THINGS_COUNT; i++)
    {
        struct Thing *thing = thing_get(i);
        if ((thing->alloc_flags & TAlF_Exists) == 0)
        {
            NETDBG(6, "free i:%04d idx:%04d -> %04d %p", i, thing->index, game.free_things[i-1], thing);
            game.free_things[free_idx] = i;
            free_idx++;
        }
        else
        {
            NETDBG(6, "item i:%04d idx:%04d alloc_flags:%04x %p", i, thing->index, thing->alloc_flags, thing);
            if (i != thing->index)
            {
                JUSTLOG("Invalid thing index delta:%d class:%d model:%d", 
                    (thing - thing_get(0)), thing->class_id, thing->model);
            }
   
            slist = get_list_for_thing_class(thing->class_id);
            thing->alloc_flags &= ~TAlF_IsInStrucList;
            thing->prev_of_class = 0;
            thing->next_of_class = 0;
            add_thing_to_list(thing, slist);
            // thing <-> mapwho
            if ((thing->alloc_flags & TAlF_IsInMapWho) && (thing->prev_on_mapblk == 0))
            {
                struct Map* mapblk = get_map_block_at(thing->mappos.x.stl.num, thing->mappos.y.stl.num);
                if (map_block_invalid(mapblk))
                {
                    JUSTLOG("Invalid mapblk for thing_idx:%04d", thing->index);
                }
                else if (get_mapwho_thing_index(mapblk) != thing->index)
                {
                    //TODO are where are interested where it moved from?
                    NETDBG(5, "moving to mapblk thing_idx:%04d", thing->index);
                    set_mapwho_thing_index(mapblk, thing->index);
                }
            }
            // TODO: control_structure
            if (thing_is_creature(thing))
            {
                struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
                if (thing->alloc_flags & TAlF_InDungeonList)
                {
                    if (cctrl->flgfield_2 & TF2_Spectator)
                    {
//                dungeon->num_active_creatrs++;
//                dungeon->owned_creatures_of_model[thing->model]++;
                    }
                }
            }
            // TODO: remove_events_thing_is_attached_to
            // TODO: tasks
        }
    }
    NETDBG(6, "done");
}

void serde_dump_thing(long thing)
{
/*
#ifdef DUMP_THINGS
    char buf[64];
    sprintf(buf, "dump/%d", i);
    FILE *F = fopen(buf, "w");
    if (F)
    {
        fwrite(tng, sizeof(struct Thing), 1, F);
        fclose(F);
    }
#endif
*/
}
