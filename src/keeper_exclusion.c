#include "pre_inc.h"
#include "keeper_exclusion.h"

#include <stdlib.h>

#include "config_players.h"
#include "creature_groups.h"
#include "player_data.h"
#include "power_hand.h"
#include "thing_data.h"
#include "post_inc.h"

DaiThingIndex keeper_possessed_thing(PlayerNumber plyr_idx)
{
    // Any of the keeper's users: Daimon's own slot is not one of them.
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        const struct UserState *ustate = get_user_state(user);
        if (user_state_invalid(ustate) || (ustate->player_id != plyr_idx))
            continue;
        if ((ustate->view_type != PVT_CreatureContrl) && (ustate->view_type != PVT_CreaturePasngr))
            continue;
        const struct Thing *thing = thing_get(ustate->controlled_thing_idx);
        if (!thing_is_invalid(thing))
            return thing->index;
    }
    return 0;
}

TbBool keeper_thing_excluded(PlayerNumber plyr_idx, const struct Thing *thing)
{
    if (thing_is_invalid(thing))
        return false;
    if (!player_exists(get_player(plyr_idx)))
        return false;
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        const struct UserState *ustate = get_user_state(user);
        if (user_state_invalid(ustate) || (ustate->player_id != plyr_idx))
            continue;
        if ((ustate->view_type == PVT_CreatureContrl) || (ustate->view_type == PVT_CreaturePasngr))
        {
            const struct Thing *possessed = thing_get(ustate->controlled_thing_idx);
            if (!thing_is_invalid(possessed))
            {
                if (thing->index == possessed->index)
                    return true;
                if (creature_is_group_member(thing)) {
                    const struct Thing *leader = get_group_leader(thing);
                    if (!thing_is_invalid(leader) && (leader->index == possessed->index))
                        return true;
                }
            }
        }
        // The creature open in the info panel (PSt_CreatrInfo keeps it as the controlled thing).
        if ((ustate->work_state == PSt_CreatrInfo) && (ustate->controlled_thing_idx == thing->index))
            return true;
        if ((ustate->thing_under_hand != 0) && (ustate->thing_under_hand == thing->index))
            return true;
    }
    return thing_is_in_power_hand_list(thing, plyr_idx);
}

TbBool keeper_area_excluded(PlayerNumber plyr_idx, DaiSubtlCoord stl_x, DaiSubtlCoord stl_y, int64_t radius)
{
    // Around what any of the keeper's users possesses
    for (NetUserId user = 0; user < MAX_NET_USERS; user++)
    {
        const struct UserState *ustate = get_user_state(user);
        if (user_state_invalid(ustate) || (ustate->player_id != plyr_idx))
            continue;
        if ((ustate->view_type != PVT_CreatureContrl) && (ustate->view_type != PVT_CreaturePasngr))
            continue;
        const struct Thing *possessed = thing_get(ustate->controlled_thing_idx);
        if (thing_is_invalid(possessed))
            continue;
        const int64_t dx = llabs((int64_t)possessed->mappos.x.stl.num - stl_x), dy = llabs((int64_t)possessed->mappos.y.stl.num - stl_y);
        if (((dx > dy) ? dx : dy) <= radius)
            return true;
    }
    return false;
}
