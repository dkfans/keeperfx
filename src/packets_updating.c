/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets_upating.c
 * @par Purpose:
 *     Continious updating of game state on different routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     10 Jan 2021 -
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "packets_updating.h"
#include "creature_control.h"
#include "packets.h"
#include "player_data.h"
#include "creature_states.h"
#include "thing_physics.h"
#include "gui_msgs.h"
#include "config_crtrstates.h"
#include "net_remap.h"
#include "net_sync.h"
#include "thing_navigate.h"
#include "map_columns.h"
#include "map_blocks.h"
#include "engine_render.h"

/******************************************************************************/
void send_update_job(struct Thing *thing)
{
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    // TODO: WIP
    //create_packet_action_big(get_player(thing->owner), PckA_UpdateJob, AP_PlusTwo);
    struct BigActionPacket tmp;
    struct BigActionPacket *big = &tmp;
    big->head.arg[0] = thing->index;
    big->head.arg[1] = thing->continue_state | (thing->active_state << 8);
    big->head.arg[2] = thing->mappos.x.stl.num | (thing->mappos.y.stl.num << 8);
    switch (thing->active_state)
    {
        case CrSt_MoveToPosition:
            big->head.arg[3] = cctrl->moveto_pos.x.stl.num | (cctrl->moveto_pos.y.stl.num << 8);
            break;
    }

    if (thing->active_state == CrSt_MoveToPosition)
    {
        switch (thing->continue_state)
        {
            case CrSt_CreaturePickUpUnconsciousBody:
                big->head.arg[4] = cctrl->pickup_creature_id;
                break;
            case CrSt_CreaturePicksUpSpellObject:
            case CrSt_CreaturePicksUpCorpse:
                big->head.arg[4] = cctrl->pickup_object_id;
                break;
            case CrSt_CreaturePicksUpTrapObject:
                big->head.arg[4] = cctrl->pickup_object_id;
                big->head.arg[5] = cctrl->arming_thing_id;
                break;
            case CrSt_ImpArrivesAtReinforce:
                big->head.arg[4] = cctrl->digger.working_stl;
                break;
            case CrSt_ImpArrivesAtImproveDungeon:
            case CrSt_ImpArrivesAtConvertDungeon:
                break;
            default:
                WARNLOG("Unexpected cjob imp:%d cstate:%d", thing->index, thing->continue_state);
        }
    }
}

void process_update_job(struct BigActionPacket *big)
{
    // TODO: this should be delayed a bit and reordered
    struct Thing *thing = thing_get(big->head.arg0);
    struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
    unsigned char continue_state = big->head.arg[1] & 255;
    unsigned char active_state = thing->active_state >> 8;
    if (continue_state > 0)
    {
        if (thing->continue_state != continue_state)
        {
            NETDBG(5,"different states imp:%d old:%d new:%d", thing->index, thing->continue_state, continue_state);
        }
        thing->continue_state = continue_state;
    }
    if (active_state)
    {
        thing->active_state = active_state;
        thing->mappos.x.stl.num = big->head.arg[2] & 255;
        thing->mappos.y.stl.num = big->head.arg[2] >> 8;
        thing->mappos.z.val = get_thing_height_at(thing, &thing->mappos);

        switch (active_state)
        {
            case CrSt_MoveToPosition:
                cctrl->moveto_pos.x.stl.num = big->head.arg[3] & 255;
                cctrl->moveto_pos.y.stl.num = big->head.arg[3] >> 8;
                cctrl->moveto_pos.z.val = get_thing_height_at(thing, &cctrl->moveto_pos);
                break;
        }
    }

    if (active_state == CrSt_MoveToPosition)
    {
        switch (continue_state)
        {
            case CrSt_ImpArrivesAtReinforce:
                cctrl->digger.working_stl = big->head.arg[4];
                break;
            case CrSt_ImpArrivesAtImproveDungeon:
            case CrSt_ImpArrivesAtConvertDungeon:
                break;
                // TODO process other jobs
        }
    }
}

TbBool send_update_land(struct Thing *thing, MapSlabCoord slb_x, MapSlabCoord slb_y, SlabKind nslab)
{
    if (!netremap_is_mine(thing->owner))
        return false;
    struct BigActionPacket *big = create_packet_action_big(get_player(thing->owner), PckA_UpdateLand, AP_PlusTwo);
    NETDBG(5, "imp:%d owner:%d st:%s x:%d y:%d slb:%d", thing->owner, thing->index, creature_state_code_name(thing->active_state),
           slb_x, slb_y, nslab);
    big->head.arg[0] = thing->index;
    big->head.arg[1] = nslab | (thing->active_state << 8);
    big->head.arg[2] = thing->mappos.x.val; // precise position only because of possession
    big->head.arg[3] = thing->mappos.y.val;
    big->head.arg[4] = slb_x | (slb_y << 8);
    return true;
}

void process_update_land(int client_id, struct BigActionPacket *big)
{
    struct Thing *thing = thing_get(net_remap_thingid(client_id, big->head.arg0));
    SlabKind nslab = big->head.arg[1] & 255;
    unsigned char new_state = big->head.arg[1] >> 8;
    MapSlabCoord slb_x = big->head.arg[4] & 255;
    MapSlabCoord slb_y = big->head.arg[4] >> 8;
    struct Coord3d newpos;
    if (thing_is_invalid(thing))
    {
        ERRORLOG("unexpected thing_id:%d(%d)", net_remap_thingid(client_id, big->head.arg0), big->head.arg0);
        net_resync_needed();
        return;
    }
    if (netremap_is_mine(thing->owner))
    {
        return;
    }
    NETDBG(5, "imp:%d owner:%d", thing->index, thing->owner);
    if (NETDBG_LEVEL > 5)
    {
        JUSTLOG("st:%s x:%d y:%d slb:%d", creature_state_code_name(thing->active_state),
                slb_x, slb_y, nslab);
    }

    newpos.x.val = big->head.arg[2];
    newpos.y.val = big->head.arg[3];
    newpos.z.val = get_thing_height_at(thing, &newpos);
    move_thing_in_map(thing, &newpos);
    ariadne_invalidate_creature_route(thing);

    thing->active_state = new_state;
    thing->continue_state = CrSt_Unused;

    if (nslab == SlbT_PATH)
    {
        dig_out_block(slab_subtile_center(slb_x),slab_subtile_center(slb_y), thing->owner);
    }
    else
    {
        place_slab_type_on_map(nslab, slab_subtile_center(slb_x), slab_subtile_center(slb_y), thing->owner, 1);
    }
}
