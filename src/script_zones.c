/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_objects.h
 *     Header file for thing_objects.c.
 * @par Purpose:
 *     Script zones
 * @par Comment:
 * @author   KeeperFx Team
 * @date     27 Mar 2021
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "script_zones.h"

#include "bflib_math.h"
#include "config_creature.h"
#include "creature_states.h"
#include "frontmenu_ingame_map.h"
#include "game_legacy.h"
#include "game_merge.h"
#include "slab_data.h"
#include "map_data.h"
#include "map_columns.h"
#include "room_data.h"
#include "room_jobs.h"
#include "room_util.h"
#include "thing_objects.h"

struct ScriptZoneRecord *add_script_zone()
{
    return &gameadd.script_zone_records[++gameadd.max_valid_script_zone_recods];
}

int script_zone_id(struct ScriptZoneRecord *zone)
{
    return zone - gameadd.script_zone_records;
}

struct ScriptZoneRecord *get_script_zone(int zone_id)
{
    if ((zone_id < 0) || (zone_id >= MAX_ZONE_RECORDS))
    {
        return INVALID_SCRIPT_ZONE;
    }
    return &gameadd.script_zone_records[zone_id];
}

struct ScriptZoneRecord *find_script_zone(int zone_id)
{
    if (zone_id <= 0)
    {
        return INVALID_SCRIPT_ZONE;
    }
    for (int i = 1; i <= gameadd.max_valid_script_zone_recods; i++)
    {
        struct ScriptZoneRecord *ptr = &gameadd.script_zone_records[i];
        if (ptr->zone_id == zone_id)
        {
            return ptr;
        }
    }
    return INVALID_SCRIPT_ZONE;
}

static void swap_zones(struct ScriptZoneRecord *first, struct ScriptZoneRecord *second)
{
    if (first == second)
        return;
    for (int dy = 0; dy < first->hheight * STL_PER_SLB; dy++)
    {
        for (int dx = 0; dx < first->hwidth * STL_PER_SLB; dx++)
        {
            struct Map *sblk = get_map_block_at(slab_subtile(first->min_x, 0) + dx, slab_subtile(first->min_y, 0) + dy);
            struct Map *dblk = get_map_block_at(slab_subtile(second->min_x, 0) + dx, slab_subtile(second->min_y, 0) + dy);
            // moving columns and visibility
            struct Map old_blk = *dblk;
            *dblk = *sblk;
            *sblk = old_blk;

            struct Thing *tng;
            // moving things on dblk
            for (int idx = get_mapwho_thing_index(dblk);
                idx != 0;
                )
            {
                tng = thing_get(idx);
                idx = tng->next_on_mapblk;
                // Remove all creatures from rooms. We will add them back later
                struct CreatureControl *cctrl = creature_control_get_from_thing(tng);
                if (!creature_control_invalid(cctrl))
                {
                    remove_creature_from_work_room(tng);
                }
                // We should adjust mappos for deleted things because deleting would query location
                tng->mappos.x.stl.num -= slab_subtile(first->min_x, 0);
                tng->mappos.x.stl.num += slab_subtile(second->min_x, 0);
                tng->mappos.y.stl.num -= slab_subtile(first->min_y, 0);
                tng->mappos.y.stl.num += slab_subtile(second->min_y, 0);
                if (tng->class_id == TCls_Object)
                {
                    switch (tng->model)
                    {
                        // Room flames
                        case 55:
                        case 56:
                        case 57:
                        case 58:
                        case OBJECT_TYPE_ROOM_FLAG:
                            delete_thing_structure(tng, 0);
                            continue;
                        default:
                            break;
                    }
                }
            }
            // moving things on sblk
            for (int idx = get_mapwho_thing_index(sblk);
                 idx != 0;
                )
            {
                tng = thing_get(idx);
                idx = tng->next_on_mapblk;
                // Remove all creatures from rooms. We will add them back later
                struct CreatureControl *cctrl = creature_control_get_from_thing(tng);
                if (!creature_control_invalid(cctrl))
                {
                    remove_creature_from_work_room(tng);
                }
                tng->mappos.x.stl.num -= slab_subtile(second->min_x, 0);
                tng->mappos.x.stl.num += slab_subtile(first->min_x, 0);
                tng->mappos.y.stl.num -= slab_subtile(second->min_y, 0);
                tng->mappos.y.stl.num += slab_subtile(first->min_y, 0);
                if (tng->class_id == TCls_Object)
                {
                    switch (tng->model)
                    {
                        case 55:
                        case 56:
                        case 57:
                        case 58:
                        case OBJECT_TYPE_ROOM_FLAG:
                            delete_thing_structure(tng, 0);
                            continue;
                        default:
                            break;
                    }
                }
            }
        }
    }
    for (int dy = 0; dy < first->hheight; dy++)
    {
        for (int dx = 0; dx < first->hwidth; dx++)
        {
            struct SlabMap *sslb = get_slabmap_block(first->min_x + dx, first->min_y + dy);
            struct SlabMap *dslb = get_slabmap_block(second->min_x + dx, second->min_y + dy);

            // Remove tiles from rooms
            if (sslb->room_index != 0)
            {
                struct Room *room = room_get(sslb->room_index);
                remove_slab_from_room_tiles_list(room, first->min_x + dx, first->min_y + dy);
                if (room->slabs_count < 1)
                {
                    // we have to move them silently
                    // reset_creatures_rooms(room);
                    free_room_structure(room);
                }
                else
                {
                    do_room_integration(room);
                }
            }
            if (dslb->room_index != 0)
            {
                struct Room *room = room_get(dslb->room_index);
                remove_slab_from_room_tiles_list(room, second->min_x + dx, second->min_y + dy);
                if (room->slabs_count < 1)
                {
                    // reset_creatures_rooms(room);
                    free_room_structure(room);
                }
                else
                {
                    do_room_integration(room);
                }
            }
            // Moving tiles and ownership
            struct SlabMap old_slb = *dslb;
            *dslb = *sslb;
            *sslb = old_slb;
        }
    }
    // Move tiles into (possible new) rooms
    for (int dy = 0; dy < first->hheight; dy++)
    {
        for (int dx = 0; dx < first->hwidth; dx++)
        {
            struct SlabMap *dslb = get_slabmap_block(first->min_x + dx, first->min_y + dy);
            struct SlabMap *sslb = get_slabmap_block(second->min_x + dx, second->min_y + dy);

            RoomKind rkind;
            rkind = slab_corresponding_room(sslb->kind);
            if (rkind > 0)
            {
                struct Room *room = create_room(slabmap_owner(sslb), rkind, slab_subtile_center(second->min_x + dx),
                            slab_subtile_center(second->min_y + dy));
                do_room_integration(room);
            }
            rkind = slab_corresponding_room(dslb->kind);
            if (rkind > 0)
            {
                struct Room *room = create_room(slabmap_owner(dslb), rkind, slab_subtile_center(first->min_x + dx),
                            slab_subtile_center(first->min_y + dy));
                do_room_integration(room);
            }
        }
    }
    for (int dy = 0; dy < first->hheight * STL_PER_SLB; dy++)
    {
        for (int dx = 0; dx < first->hwidth * STL_PER_SLB; dx++)
        {
            struct Map *sblk = get_map_block_at(slab_subtile(second->min_x, 0) + dx, slab_subtile(second->min_y, 0) + dy);
            struct Map *dblk = get_map_block_at(slab_subtile(first->min_x, 0) + dx, slab_subtile(first->min_y, 0) + dy);
            struct SlabMap* sslb = get_slabmap_for_subtile(slab_subtile(second->min_x, 0) + dx, slab_subtile(second->min_y, 0) + dy);
            struct SlabMap* dslb = get_slabmap_for_subtile(slab_subtile(first->min_x, 0) + dx, slab_subtile(first->min_y, 0) + dy);
            if (sslb->room_index != 0)
            {
                struct Room *room = room_get(sslb->room_index);
                for (int idx = get_mapwho_thing_index(sblk);
                     idx != 0;
                    )
                {
                    struct Thing *tng = thing_get(idx);
                    idx = tng->next_on_mapblk;
                    struct CreatureControl *cctrl = creature_control_get_from_thing(tng);
                    if (!creature_control_invalid(cctrl))
                    {
                        CreatureJob jobpref = get_job_for_creature_state(get_creature_state_besides_interruptions(tng));
                        add_creature_to_work_room(tng, room, jobpref);
                    }
                }
            }
            if (dslb->room_index != 0)
            {
                struct Room *room = room_get(dslb->room_index);
                for (int idx = get_mapwho_thing_index(dblk);
                     idx != 0;
                    )
                {
                    struct Thing *tng = thing_get(idx);
                    idx = tng->next_on_mapblk;
                    struct CreatureControl *cctrl = creature_control_get_from_thing(tng);
                    if (!creature_control_invalid(cctrl))
                    {
                        CreatureJob jobpref = get_job_for_creature_state(get_creature_state_besides_interruptions(tng));
                        add_creature_to_work_room(tng, room, jobpref);
                    }
                }
            }
        }
    }
    // TODO: Move things into new rooms
    pannel_map_update(first->min_x * STL_PER_SLB, first->min_y * STL_PER_SLB,first->hwidth * STL_PER_SLB, first->hheight * STL_PER_SLB);
    pannel_map_update(second->min_x * STL_PER_SLB, second->min_y * STL_PER_SLB,first->hwidth * STL_PER_SLB, first->hheight * STL_PER_SLB);
}

// Here we expect that zones are checked and valid
void swap_script_zone(int zone_id, enum ScriptZoneSwapDirection dir)
{
    struct ScriptZoneRecord *first = find_script_zone(zone_id);
    struct ScriptZoneRecord *next;
    if (dir == SZS_Backward)
    {
        next = get_script_zone(first->next_idx);
        for (struct ScriptZoneRecord *cur = first; next != first;)
        {
            swap_zones(cur, next);
            cur = next;
            next = get_script_zone(cur->next_idx);
        }
    } else if (dir == SZS_Forward)
    {
        next = get_script_zone(first->prev_idx);
        for (struct ScriptZoneRecord *cur = first; next != first;)
        {
            swap_zones(cur, next);
            cur = next;
            next = get_script_zone(cur->prev_idx);
        }
    } else if (dir == SZS_Shuffle)
    {
        int cnt = 1;
        next = get_script_zone(first->next_idx);
        for (;next != first;)
        {
            cnt++;
            next = get_script_zone(next->next_idx);
        }
        for (; cnt > 1; cnt--)
        {
            int i = ACTION_RANDOM(cnt);
            struct ScriptZoneRecord *next = get_script_zone(first->next_idx);
            for (; i > 0; i--)
            {
                next = get_script_zone(next->next_idx);
            }
            swap_zones(first, next);
            first = get_script_zone(first->next_idx);
        }
    }
}