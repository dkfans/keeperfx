#include "ariadne.h"
#include "ariadne_wallhug.h"
#include "config_magic.h"
#include "dungeon_data.h"
#include "magic.h"
#include "map_data.h"
#include "map_utils.h"
#include "player_computer.h"
#include "room_data.h"
#include "slab_data.h"
#include "tasks_list.h"

static short get_hug_side(struct ComputerDig * cdig, MapSubtlCoord stl1_x, MapSubtlCoord stl1_y, MapSubtlCoord stl2_x, MapSubtlCoord stl2_y, unsigned short direction, PlayerNumber plyr_idx)
{
    SYNCDBG(4,"Starting");
    MapSubtlCoord stl_b_x;
    MapSubtlCoord stl_b_y;
    MapSubtlCoord stl_a_x;
    MapSubtlCoord stl_a_y;
    int i;
    i = get_hug_side_options(stl1_x, stl1_y, stl2_x, stl2_y, direction, plyr_idx, &stl_a_x, &stl_a_y, &stl_b_x, &stl_b_y);
    if ((i == 0) || (i == 1)) {
        return i;
    }
    i = cdig->hug_side;
    if ((i == 0) || (i == 1)) {
        return i;
    }
    int dist_a;
    int dist_b;
    dist_a = abs(stl_a_y - stl2_y) + abs(stl_a_x - stl1_x);
    dist_b = abs(stl_b_y - stl2_y) + abs(stl_b_x - stl1_x);
    if (dist_b > dist_a) {
        return 1;
    }
    if (dist_b < dist_a) {
        return 0;
    }
    // Random hug side
    return ((stl2_x+stl2_y)>>1)%2;
}

short tool_dig_to_pos2_skip_slabs_which_dont_need_digging_f(const struct Computer2 * comp, struct ComputerDig * cdig, unsigned short digflags,
    MapSubtlCoord *nextstl_x, MapSubtlCoord *nextstl_y, const char *func_name)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    MapSlabCoord nextslb_x;
    MapSlabCoord nextslb_y;
    long around_index;
    long i;
    for (i = 0; ; i++)
    {
        struct SlabMap *slb;
        nextslb_x = subtile_slab(*nextstl_x);
        nextslb_y = subtile_slab(*nextstl_y);
        slb = get_slabmap_block(nextslb_x, nextslb_y);
        if (slab_good_for_computer_dig_path(slb) && (slb->kind != SlbT_WATER))
        {
            SubtlCodedCoords stl_num;
            stl_num = get_subtile_number_at_slab_center(nextslb_x,nextslb_y);
            if (find_from_task_list(dungeon->owner, stl_num) < 0) {
                // We've reached a subtile which is good for digging and not in dig tasks list
                break;
            }
        }
        if (slab_kind_is_liquid(slb->kind))
        {
            // We've reached liquid slab - act accordingly
            if ((digflags & ToolDig_AllowLiquidWBridge) != 0) {
                break;
            }
            if (computer_check_room_available(comp, RoK_BRIDGE) != IAvail_Now) {
                break;
            }
        }
        if (slab_kind_is_door(slb->kind) && (slabmap_owner(slb) != dungeon->owner)) {
            // We've reached enemy door
            break;
        }
        cdig->pos_next.x.stl.num = *nextstl_x;
        cdig->pos_next.y.stl.num = *nextstl_y;
        if ((subtile_slab(cdig->pos_dest.x.stl.num) == nextslb_x) && (subtile_slab(cdig->pos_dest.y.stl.num) == nextslb_y))
        {
            SYNCDBG(5,"%s: Reached destination slab (%d,%d)",func_name,(int)nextslb_x,(int)nextslb_y);
            return -1;
        }
        // Move position towards the target subtile, shortest way
        around_index = small_around_index_towards_destination(*nextstl_x,*nextstl_y,cdig->pos_dest.x.stl.num,cdig->pos_dest.y.stl.num);
        (*nextstl_x) += STL_PER_SLB * small_around[around_index].delta_x;
        (*nextstl_y) += STL_PER_SLB * small_around[around_index].delta_y;
        if (i > map_tiles_x+map_tiles_y)
        {
            ERRORLOG("%s: Infinite loop while finding path to dig gold",func_name);
            return -2;
        }
    }
    return i;
}

/**
 * Moves position towards destination, tagging any slabs which require digging.
 * Stops when the straight road towards destination can no longer be continued.
 * Computer player dig helper function.
 * @see tool_dig_to_pos2_f()
 * @param comp
 * @param cdig
 * @param simulation
 * @param digflags
 * @param nextstl_x
 * @param nextstl_y
 * @param func_name
 */
short tool_dig_to_pos2_do_action_on_slab_which_needs_it_f(struct Computer2 * comp, struct ComputerDig * cdig, TbBool simulation, unsigned short digflags,
    MapSubtlCoord *nextstl_x, MapSubtlCoord *nextstl_y, const char *func_name)
{
    struct Dungeon *dungeon;
    dungeon = comp->dungeon;
    MapSlabCoord nextslb_x;
    MapSlabCoord nextslb_y;
    long around_index;
    long i;
    for (i = 0; i < cdig->subfield_2C; i++)
    {
        struct SlabAttr *slbattr;
        struct SlabMap *slb;
        struct Map *mapblk;
        nextslb_x = subtile_slab(*nextstl_x);
        nextslb_y = subtile_slab(*nextstl_y);
        slb = get_slabmap_block(nextslb_x, nextslb_y);
        mapblk = get_map_block_at(*nextstl_x, *nextstl_y);
        slbattr = get_slab_attrs(slb);
        if ( (slbattr->is_diggable == 0) || (slb->kind == SlbT_GEMS)
          || (((mapblk->flags & SlbAtFlg_Filled) != 0) && (slabmap_owner(slb) != dungeon->owner)) )
        {
            if ( ((slbattr->block_flags & SlbAtFlg_Valuable) == 0) || (digflags == 0) ) {
                break;
            }
        }
        if ( !simulation )
        {
            if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, *nextstl_x, *nextstl_y, 1, 1) <= Lb_OK) {
                ERRORLOG("%s: Could not do game action at subtile (%d,%d)",func_name,(int)*nextstl_x,(int)*nextstl_y);
                break;
            }
            if (digflags != 0)
            {
                if ((slbattr->block_flags & SlbAtFlg_Valuable) != 0) {
                    cdig->valuable_slabs_tagged++;
                }
            }
        }
        cdig->pos_next.x.stl.num = *nextstl_x;
        cdig->pos_next.y.stl.num = *nextstl_y;
        if ((subtile_slab(cdig->pos_dest.x.stl.num) == nextslb_x) && (subtile_slab(cdig->pos_dest.y.stl.num) == nextslb_y))
        {
            SYNCDBG(5,"%s: Reached destination slab (%d,%d)",func_name,(int)nextslb_x,(int)nextslb_y);
            return -1;
        }
        // Move position towards the target subtile, shortest way
        around_index = small_around_index_towards_destination(*nextstl_x,*nextstl_y,cdig->pos_dest.x.stl.num,cdig->pos_dest.y.stl.num);
        (*nextstl_x) += STL_PER_SLB * small_around[around_index].delta_x;
        (*nextstl_y) += STL_PER_SLB * small_around[around_index].delta_y;
        if (i > map_tiles_x*map_tiles_y)
        {
            ERRORLOG("%s: Infinite loop while finding path to dig gold",func_name);
            return -2;
        }
    }
    nextslb_x = subtile_slab(*nextstl_x);
    nextslb_y = subtile_slab(*nextstl_y);
    if ((subtile_slab(cdig->pos_dest.x.stl.num) == nextslb_x) && (subtile_slab(cdig->pos_dest.y.stl.num) == nextslb_y))
    {
        cdig->pos_next.x.stl.num = *nextstl_x;
        cdig->pos_next.y.stl.num = *nextstl_y;
        SYNCDBG(5,"%s: Reached destination slab (%d,%d)",func_name,(int)nextslb_x,(int)nextslb_y);
        return -1;
    }
    return i;
}

/**
 * Tool function to do (or simulate) computer player digging.
 * @param comp Computer player which is doing the task.
 * @param cdig The ComputerDig structure to be changed. Should be dummy if simulating.
 * @param simulation Indicates if we're simulating or doing the real thing.
 * @param digflags These are not really flags, but should be changed into flags when all calls to this func are rewritten. Use values from ToolDigFlags enum.
 * @return
 */
short tool_dig_to_pos2_f(struct Computer2 * comp, struct ComputerDig * cdig, TbBool simulation, unsigned short digflags, const char *func_name)
{
    struct Dungeon *dungeon;
    struct SlabMap *slb;
    struct SlabMap *slbw;
    struct Map *mapblk;
    struct Map *mapblkw;
    MapSubtlCoord gldstl_x;
    MapSubtlCoord gldstl_y;
    MapSubtlCoord digstl_x;
    MapSubtlCoord digstl_y;
    MapSlabCoord digslb_x;
    MapSlabCoord digslb_y;
    long counter1;
    long i;
    SYNCDBG(14,"%s: Starting",func_name);
    dungeon = comp->dungeon;
    // Limit amount of calls
    cdig->calls_count++;
    if (cdig->calls_count >= COMPUTER_TOOL_DIG_LIMIT) {
        WARNLOG("%s: Player %d ComputerDig calls count (%d) exceeds limit for path from (%d,%d) to (%d,%d)",func_name,
            (int)dungeon->owner,(int)cdig->calls_count,(int)coord_slab(cdig->pos_begin.x.val),(int)coord_slab(cdig->pos_begin.y.val),
            (int)coord_slab(cdig->pos_dest.x.val),(int)coord_slab(cdig->pos_dest.y.val));
        return -2;
    }
    gldstl_x = stl_slab_center_subtile(cdig->pos_begin.x.stl.num);
    gldstl_y = stl_slab_center_subtile(cdig->pos_begin.y.stl.num);
    SYNCDBG(4,"%s: Dig slabs from (%d,%d) to (%d,%d)",func_name,subtile_slab(gldstl_x),subtile_slab(gldstl_y),subtile_slab(cdig->pos_dest.x.stl.num),subtile_slab(cdig->pos_dest.y.stl.num));
    if (get_2d_distance(&cdig->pos_begin, &cdig->pos_dest) <= cdig->distance)
    {
        SYNCDBG(4,"%s: Player %d does small distance digging",func_name,(int)dungeon->owner);
        counter1 = tool_dig_to_pos2_skip_slabs_which_dont_need_digging_f(comp, cdig, digflags, &gldstl_x, &gldstl_y, func_name);
        if (counter1 < 0) {
            return counter1;
        }
        // Being here means we didn't reached the destination - we must do some kind of action
        if (slab_is_liquid(subtile_slab(gldstl_x), subtile_slab(gldstl_y)))
        {
            if (computer_check_room_available(comp, RoK_BRIDGE) == IAvail_Now) {
                cdig->pos_next.x.stl.num = gldstl_x;
                cdig->pos_next.y.stl.num = gldstl_y;
                SYNCDBG(5,"%s: Player %d has bridge, so is going through liquid slab (%d,%d)",func_name,
                    (int)dungeon->owner,(int)subtile_slab(gldstl_x),(int)subtile_slab(gldstl_y));
                return -5;
            }
        }
        counter1 = tool_dig_to_pos2_do_action_on_slab_which_needs_it_f(comp, cdig, simulation, digflags, &gldstl_x, &gldstl_y, func_name);
        if (counter1 < 0) {
            return counter1;
        }
        // If the straight road stopped and we were not able to find anything to dig, check other directions
        long around_index;
        around_index = small_around_index_towards_destination(gldstl_x,gldstl_y,cdig->pos_dest.x.stl.num,cdig->pos_dest.y.stl.num);
        if (counter1 > 0)
        {
            cdig->pos_begin.x.stl.num = gldstl_x;
            cdig->pos_begin.y.stl.num = gldstl_y;
            cdig->distance = get_2d_distance(&cdig->pos_next, &cdig->pos_dest);
            // In case we're finishing the easy road, prepare vars for long distance digging
            cdig->hug_side = get_hug_side(cdig, cdig->pos_next.x.stl.num, cdig->pos_next.y.stl.num,
                cdig->pos_dest.x.stl.num, cdig->pos_dest.y.stl.num, around_index, dungeon->owner);
            cdig->direction_around = (around_index + (cdig->hug_side < 1 ? 3 : 1)) & 3;
            SYNCDBG(5,"%s: Going through slab (%d,%d)",func_name,(int)subtile_slab(gldstl_x),(int)subtile_slab(gldstl_y));
            return 0;
        }
        if (cdig->subfield_2C == comp->field_C)
        {
            gldstl_x -= STL_PER_SLB * small_around[around_index].delta_x;
            gldstl_y -= STL_PER_SLB * small_around[around_index].delta_y;
            cdig->pos_begin.x.val = subtile_coord(gldstl_x,0);
            cdig->pos_begin.y.val = subtile_coord(gldstl_y,0);
            cdig->pos_begin.z.val = 0;
            cdig->pos_E.x.val = cdig->pos_begin.x.val;
            cdig->pos_E.y.val = cdig->pos_begin.y.val;
            cdig->pos_E.z.val = cdig->pos_begin.z.val;
        }
        if ((cdig->pos_next.x.val == 0) && (cdig->pos_next.y.val == 0) && (cdig->pos_next.z.val == 0))
        {
            cdig->pos_next.x.val = cdig->pos_E.x.val;
            cdig->pos_next.y.val = cdig->pos_E.y.val;
            cdig->pos_next.z.val = cdig->pos_E.z.val;
        }
        cdig->subfield_48++;
        if ((cdig->subfield_48 > 10) && (cdig->sub4C_stl_x == gldstl_x) && (cdig->sub4C_stl_y == gldstl_y)) {
            SYNCDBG(5,"%s: Positions are equal at subtile (%d,%d)",func_name,(int)gldstl_x,(int)gldstl_y);
            return -2;
        }
        cdig->sub4C_stl_x = gldstl_x;
        cdig->sub4C_stl_y = gldstl_y;
        cdig->distance = get_2d_distance(&cdig->pos_next, &cdig->pos_dest);
        cdig->hug_side = get_hug_side(cdig, cdig->pos_next.x.stl.num, cdig->pos_next.y.stl.num,
                           cdig->pos_dest.x.stl.num, cdig->pos_dest.y.stl.num, around_index, dungeon->owner);
        cdig->direction_around = (around_index + (cdig->hug_side < 1 ? 3 : 1)) & 3;
        i = dig_to_position(dungeon->owner, cdig->pos_next.x.stl.num, cdig->pos_next.y.stl.num,
            cdig->direction_around, cdig->hug_side);
        if (i == -1) {
            SYNCDBG(5,"%s: Player %d short digging to subtile (%d,%d) preparations failed",func_name,(int)dungeon->owner,(int)cdig->pos_next.x.stl.num,(int)cdig->pos_next.y.stl.num);
            return -2;
        }
        digstl_x = stl_num_decode_x(i);
        digstl_y = stl_num_decode_y(i);
        digslb_x = subtile_slab(digstl_x);
        digslb_y = subtile_slab(digstl_y);
        slb = get_slabmap_block(digslb_x, digslb_y);
        if (slab_kind_is_liquid(slb->kind) && (computer_check_room_available(comp, RoK_BRIDGE) == IAvail_Now))
        {
            cdig->pos_next.y.stl.num = digstl_y;
            cdig->pos_next.x.stl.num = digstl_x;
            SYNCDBG(5,"%s: Player %d has bridge, so is going through liquid subtile (%d,%d)",func_name,(int)dungeon->owner,(int)gldstl_x,(int)gldstl_y);
            return -5;
        }
    } else
    {
        SYNCDBG(4,"%s: Player %d does long distance digging",func_name,(int)dungeon->owner);
        i = dig_to_position(dungeon->owner, gldstl_x, gldstl_y, cdig->direction_around, cdig->hug_side);
        if (i == -1) {
            SYNCDBG(5,"%s: Player %d long digging to subtile (%d,%d) preparations failed",func_name,(int)dungeon->owner,(int)gldstl_x,(int)gldstl_y);
            return -2;
        }
        digstl_x = stl_num_decode_x(i);
        digstl_y = stl_num_decode_y(i);
        digslb_x = subtile_slab(digstl_x);
        digslb_y = subtile_slab(digstl_y);
    }
    slb = get_slabmap_block(digslb_x, digslb_y);
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    if ((slbattr->is_diggable) && (slb->kind != SlbT_GEMS))
    {
        mapblk = get_map_block_at(digstl_x, digstl_y);
        if (((mapblk->flags & SlbAtFlg_Filled) == 0) || (slabmap_owner(slb) == dungeon->owner))
        {
            i = get_subtile_number_at_slab_center(digslb_x,digslb_y);
            if ((find_from_task_list(dungeon->owner, i) < 0) && (!simulation))
            {
                // Only when the computer has enough gold to cast lvl8, will he consider casting lvl3 power, so he has some gold left.
                if( computer_able_to_use_power(comp, PwrK_DESTRWALLS, 8, 1))
                {
                    mapblkw = get_map_block_at(digstl_x, digstl_y-3);
                    slbw = get_slabmap_block(digslb_x, digslb_y-1);
                    if(((mapblkw->flags & SlbAtFlg_Filled) >= 1) && (slabmap_owner(slbw) != dungeon->owner))
                    {
                        magic_use_available_power_on_subtile(dungeon->owner, PwrK_DESTRWALLS, 3, digstl_x, digstl_y-3, PwCast_Unrevealed);
                        return -5;
                    }
                    else
                    {
                        mapblkw = get_map_block_at(digstl_x, digstl_y+3);
                        slbw = get_slabmap_block(digslb_x, digslb_y+1);
                        if(((mapblkw->flags & SlbAtFlg_Filled) >= 1) && (slabmap_owner(slbw) != dungeon->owner))
                        {
                            magic_use_available_power_on_subtile(dungeon->owner, PwrK_DESTRWALLS, 3, digstl_x, digstl_y+3, PwCast_Unrevealed);
                            return -5;
                        }
                        else
                        {
                            mapblkw = get_map_block_at(digstl_x-3, digstl_y);
                            slbw = get_slabmap_block(digslb_x-1, digslb_y);
                            if(((mapblkw->flags & SlbAtFlg_Filled) >= 1) && (slabmap_owner(slbw) != dungeon->owner))
                            {
                                magic_use_available_power_on_subtile(dungeon->owner, PwrK_DESTRWALLS, 3, digstl_x-3, digstl_y, PwCast_Unrevealed);
                                return -5;
                            }
                            else
                            {
                                mapblkw = get_map_block_at(digstl_x+3, digstl_y);
                                slbw = get_slabmap_block(digslb_x+1, digslb_y);
                                if(((mapblkw->flags & SlbAtFlg_Filled) >= 1) && (slabmap_owner(slbw) != dungeon->owner))
                                {
                                    magic_use_available_power_on_subtile(dungeon->owner, PwrK_DESTRWALLS, 3, digstl_x+3, digstl_y, PwCast_Unrevealed);
                                    return -5;
                                }
                            }
                        }
                    }
                }
                if (try_game_action(comp, dungeon->owner, GA_MarkDig, 0, digstl_x, digstl_y, 1, 1) <= Lb_OK) 
                {
                    ERRORLOG("%s: Couldn't do game action - cannot dig",func_name);
                    return -2;
                }
            }
        }
    }
    cdig->direction_around = small_around_index_towards_destination(cdig->pos_next.x.stl.num,cdig->pos_next.y.stl.num,digstl_x,digstl_y);
    cdig->pos_next.x.stl.num = digstl_x;
    cdig->pos_next.y.stl.num = digstl_y;
    if ((subtile_slab(cdig->pos_dest.x.stl.num) == digslb_x) && (subtile_slab(cdig->pos_dest.y.stl.num) == digslb_y))
    {
        SYNCDBG(5,"%s: Reached destination slab (%d,%d)",func_name,(int)digslb_x,(int)digslb_y);
        return -1;
    }
    cdig->pos_begin.x.stl.num = digstl_x;
    cdig->pos_begin.y.stl.num = digstl_y;
    SYNCDBG(5,"%s: Going through slab (%d,%d)",func_name,(int)digslb_x,(int)digslb_y);
    return 0;
}

/**
 * Simulates digging from and to given coords with given flags.
 * @param comp Computer player which does the simulation.
 * @param startpos Digging start point, may be updated in a berrer one is seen.
 * @param endpos Digging final point, constant.
 * @param dig_distance Value which is increased by the amount of slabs travelled.
 * @param digflags Digging flags to be used.
 */
TbBool simulate_dig_to(struct Computer2 *comp, struct Coord3d *startpos, const struct Coord3d *endpos, unsigned long *dig_distance, unsigned short digflags)
{
    struct Dungeon* dungeon = comp->dungeon;
    struct ComputerDig cdig;
    long digres;
    // Setup the digging on dummy ComputerDig, to compute distance and move start position near to wall
    setup_dig_to(&cdig, *startpos, *endpos);
    while ( 1 )
    {
        digres = tool_dig_to_pos2(comp, &cdig, true, digflags);
        if (digres != 0)
          break;
        // If the slab we've got from digging is safe to walk and connected to original room, use it as starting position
        // But don't change distance - it should be computed from our rooms (and resetting it could lead to infinite loop)
        // Note: when verifying the path traced by computer player, we might want to disable this to see the full path
        if (slab_is_safe_land(dungeon->owner, coord_slab(cdig.pos_next.x.val), coord_slab(cdig.pos_next.y.val))) {
            if (navigation_points_connected(startpos, &cdig.pos_next)) {
                *startpos = cdig.pos_next;
            }
        }
        (*dig_distance)++;
    }
    return ((digres == -1) || (digres == -5));
}
