#include "ftest_util.h"

#ifdef FUNCTESTING

#include "../pre_inc.h"

#include "../game_legacy.h"
#include "../bflib_math.h"
#include "../keeperfx.hpp"
#include "../lvl_filesdk1.h"
#include "../slab_data.h"
#include "../room_util.h"
#include "../thing_physics.h"
#include "../creature_states.h"
#include "../frontend.h"
#include "../bflib_mouse.h"
#include "../bflib_planar.h"

#include "../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif


TbBool ftest_util_replace_slabs(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, SlabKind slab_kind, PlayerNumber owner)
{
    TbBool valid_player_number = owner >= 0 && owner <= PLAYERS_COUNT;
    if(owner < PLAYERS_COUNT) // check players that can exist on map
    {
        valid_player_number = player_exists(get_player(owner));
    }

    TbBool result = true;
    unsigned long x;
    unsigned long y;
    for (y = slb_y_from; y <= slb_y_to; y++)
    {
        for (x = slb_x_from; x <= slb_x_to; x++)
        {
            if(valid_player_number)
            {
                set_slab_owner(x, y, owner);
            }
            
            if(!replace_slab_from_script(x, y, slab_kind))
            {
                ERRORLOG("Failed to replace slab at (%d,%d)", x, y);
                result = false;
            }
        }
    }

    return result;
}

TbBool ftest_util_does_player_own_any_slabs(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, PlayerNumber owner)
{
    TbBool valid_player_number = player_exists(get_player(owner));
    if(!valid_player_number)
    {
        return false;
    }

    struct SlabMap *slb;
    unsigned long x;
    unsigned long y;
    for (y = slb_y_from; y <= slb_y_to; y++)
    {
        for (x = slb_x_from; x <= slb_x_to; x++)
        {
            slb = get_slabmap_block(x, y);
            if(slabmap_block_invalid(slb))
            {
                return false;
            }

            if(slabmap_owner(slb) != owner)
            {
                return false;
            }
        }
    }

    return true;
}

TbBool ftest_util_do_any_slabs_match(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, SlabKind slab)
{
    struct SlabMap *slb;
    unsigned long x;
    unsigned long y;
    for (y = slb_y_from; y <= slb_y_to; y++)
    {
        for (x = slb_x_from; x <= slb_x_to; x++)
        {
            slb = get_slabmap_block(x, y);
            if(slabmap_block_invalid(slb))
            {
                return false;
            }

            if(slb->kind == slab)
            {
                return true;
            }
        }
    }

    return false;
}

TbBool ftest_util_reveal_map(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if(player_invalid(player))
    {
        return false;
    }

    reveal_whole_map(player);
    
    return true;
}

TbBool ftest_util_replace_slab_columns(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber owner, SlabKind slab_base_type, ColumnIndex column0_type, ColumnIndex column1_type, ColumnIndex column2_type, ColumnIndex column3_type, ColumnIndex column4_type, ColumnIndex column5_type, ColumnIndex column6_type, ColumnIndex column7_type, ColumnIndex column8_type)
{
    if(!ftest_util_replace_slabs(slb_x, slb_y, slb_x, slb_y, slab_base_type, owner))
    {
        return false;
    }

    unsigned char i = 0;
    ColumnIndex column_type = 0;
    struct Map* mapblk = INVALID_MAP_BLOCK;
    for(unsigned char y = 0; y < 3; ++y)
    {
        for(unsigned char x = 0; x < 3; ++x)
        {
            i = x + 3 * y;
            switch(i)
            {
                case 0:
                    column_type = column0_type;
                    break;
                case 1:
                    column_type = column1_type;
                    break;
                case 2:
                    column_type = column2_type;
                    break;
                case 3:
                    column_type = column3_type;
                    break;
                case 4:
                    column_type = column4_type;
                    break;
                case 5:
                    column_type = column5_type;
                    break;
                case 6:
                    column_type = column6_type;
                    break;
                case 7:
                    column_type = column7_type;
                    break;
                case 8:
                    column_type = column8_type;
                    break;

                default:
                    break;
            }

            mapblk = get_map_block_at(slab_subtile(slb_x, x), slab_subtile(slb_y, y));
            if(map_block_invalid(mapblk))
            {
                ERRORLOG("Map block at slab (%d,%d), subtile (%d,%d) is invalid.", slb_x, slb_y, x, y);
                return false;
            }
            set_mapblk_column_index(mapblk, column_type);
        }
    }

    return true;
}

TbBool ftest_util_move_camera(long x, long y, PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if(player_invalid(player))
    {
        LbErrorLog("Player %d not found", plyr_idx);
        return false;
    }

    struct Camera* camera = &player->cameras[CamIV_Isometric];
    if(camera == NULL)
    {
        LbErrorLog("Could not find camera %d", CamIV_Isometric);
        return false;
    }

    camera->mappos.x.val = x;
    camera->mappos.y.val = y;

    return true;
}

TbBool ftest_util_move_camera_to_thing(struct Thing* const thing, PlayerNumber plyr_idx)
{
    if(thing_is_invalid(thing))
    {
        LbErrorLog("Must provide a valid thing!");
        return false;
    }

    return ftest_util_move_camera(thing->mappos.x.val, thing->mappos.y.val, plyr_idx);
}

TbBool ftest_util_move_camera_to_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct Coord3d target;
    if(!set_coords_to_slab_center(&target, slb_x, slb_y))
    {
        LbErrorLog("Must provide a valid thing!");
        return false;
    }

    return ftest_util_move_camera(target.x.val, target.y.val, plyr_idx);
}

struct Thing* ftest_util_create_random_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_level)
{
    ThingModel crmodel;
    while (1) {
        crmodel = GAME_RANDOM(game.conf.crtr_conf.model_count) + 1;
        // Accept any non-spectator creatures
        struct CreatureModelConfig* crconf = &game.conf.crtr_conf.model[crmodel];
        if ((crconf->model_flags & CMF_IsSpectator) != 0) {
            continue;
        }
        break;
    }
    struct Coord3d pos;
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    struct Thing* thing = create_creature(&pos, crmodel, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create creature %s at (%ld,%ld)",creature_code_name(crmodel),x,y);
        return false;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Creature %s at (%ld,%ld) deleted because is in wall",creature_code_name(crmodel),x,y);
        return false;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    set_start_state(thing);
    CrtrExpLevel exp_level = GAME_RANDOM(max_level);
    set_creature_level(thing, exp_level);
    return thing;
}

struct Thing* ftest_util_create_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_level, ThingModel creature_model)
{
    struct Coord3d pos;
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    struct Thing* thing = create_creature(&pos, creature_model, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create creature %s at (%ld,%ld)",creature_code_name(creature_model),x,y);
        return false;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Creature %s at (%ld,%ld) deleted because is in wall",creature_code_name(creature_model),x,y);
        return false;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    set_start_state(thing);
    CrtrExpLevel lv = GAME_RANDOM(max_level);
    set_creature_level(thing, lv);
    return thing;
}

void ftest_util_center_cursor_over_dungeon_view()
{
    struct PlayerInfo *player = get_my_player();
    if(player_invalid(player))
    {
        FTEST_FAIL_TEST("Failed to find player, this shouldn't happen!");
        return;
    }

    struct TbPoint point;
    point.x = player->engine_window_width/2;
    point.y = player->engine_window_height/2;
    if ((game.operation_flags & GOF_ShowGui) != 0)
    {
        point.x += status_panel_width;
    }

    LbMouseSetPosition(point.x, point.y);
}

TbBool ftest_util_replace_slabs_with_dungeon_hearts(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, PlayerNumber owner)
{
    if(!ftest_util_replace_slabs(slb_x_from, slb_y_from, slb_x_to, slb_y_to, SlbT_DUNGHEART, owner))
    {
        return false;
    }

    MapSlabCoord x_offset = 0;
    MapSlabCoord y_offset = 0;
    for(MapSlabCoord y = slb_y_from; y < slb_y_to; ++y)
    {
        for(MapSlabCoord x = slb_x_from; x < slb_x_to; ++x)
        {
            x_offset = slb_x_from - x;
            y_offset = slb_y_from - y;
            if(x_offset % 4 == 0 || y_offset % 4 == 0)
            {
                if(!ftest_util_replace_slabs(x, y, x, y, SlbT_CLAIMED, owner))
                {
                    return false;
                }
            }
        }
    }

    return true;
}

TbBool ftest_util_mark_slab_for_highlight(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    long stl_x = slab_subtile(slb_x,0);
    long stl_y = slab_subtile(slb_y,0);

    long dx;
    long dy;
    struct Map* mapblk = INVALID_MAP_BLOCK;
    for (dy=0; dy < STL_PER_SLB; dy++)
    {
        for (dx=0; dx < STL_PER_SLB; dx++)
        {
            mapblk = get_map_block_at(stl_x + dx, stl_y + dy);
            if(map_block_invalid(mapblk))
            {
                ERRORLOG("Invalid map block for slab (%d,%d) at (%ld,%ld)", slb_x, slb_y, stl_x + dx, stl_y + dy);
                return false;
            }

            if((slb_x % 2 == 0 && slb_y % 2 != 0) || (slb_x % 2 != 0 && slb_y % 2 == 0))
            {
                if(((dx == 0 || dx == 2) && (dy == 0 || dy == 2)) || (dx == 1 && dy == 1))
                {
                    mapblk->flags |= SlbAtFlg_Unexplored;
                }
            }
            else
            {
                if(!(((dx == 0 || dx == 2) && (dy == 0 || dy == 2)) || (dx == 1 && dy == 1)))
                {
                    mapblk->flags |= SlbAtFlg_Unexplored;
                }
            }
        }
    }

    return true;
}

TbBool ftest_util_action__create_and_fill_torture_room(struct FTestActionArgs* const args)
{
    struct ftest_util_action__create_and_fill_torture_room__variables* const vars = args->data;

    if(vars->only_run_once && args->times_executed > 0)
    {
        return true;
    }

    struct LevelInformation* const level_info = get_level_info(1);
    if(level_info == NULL)
    {
        FTEST_FAIL_TEST("Failed to grab level info. Something is wrong.");
        return true;
    }

    const MapSlabCoord room_start_x = vars->room_slb_x_start;
    const MapSlabCoord room_start_y = vars->room_slb_y_start;
    const MapSlabCoord room_end_x = vars->room_slb_x_start + vars->room_width;
    const MapSlabCoord room_end_y = vars->room_slb_y_start + vars->room_height;

    if(room_start_x <= 0 || room_end_x >= level_info->mapsize_x || room_start_y <= 0 || room_end_y >= level_info->mapsize_y)
    {
        FTEST_FAIL_TEST("Room bounds (%d,%d, %d,%d) exceed map border (%d,%d, %d,%d)", room_start_x, room_start_x, room_end_x, room_end_y
                                                                                     , 0, 0, level_info->mapsize_x, level_info->mapsize_y);
        return true;
    }

    if(!ftest_util_replace_slabs(room_start_x, room_start_y, room_end_x, room_end_y, SlbT_TORTURE, vars->room_owner))
    {
        FTEST_FAIL_TEST("Failed to create torture chamber at (%d,%d, %d,%d)", room_start_x, room_start_x, room_end_x, room_end_y);
        return true;
    }

    struct Coord3d center_of_room_pos;
    set_coords_to_slab_center(&center_of_room_pos, room_start_x + vars->room_width/2, room_start_y + vars->room_height);

    struct Thing* torture_victim = ftest_util_create_creature(center_of_room_pos.x.val, center_of_room_pos.y.val, vars->victim_player_owner, vars->victim_max_level, vars->victim_creature_model);
    if(thing_is_invalid(torture_victim))
    {
        FTEST_FAIL_TEST("Cannot create creature %s at (%ld,%ld)",creature_code_name(vars->victim_creature_model), center_of_room_pos.x.val, center_of_room_pos.y.val);
        return true;
    }

    return true;
}

struct Thing* ftest_util_create_door_for_player_with_health(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber owner, HitPoints health)
{
    // create enemy wooden door with low health
    struct Coord3d doorPos;
    set_coords_to_slab_center(&doorPos, slb_x, slb_y);

    //ThingModel doorModel = game.conf.trapdoor_conf.door_to_object[1]; // couldn't find proper type mapping
    ThingModel doorModel = 1; // wooden door == 1 (hardcoded for now)
    unsigned char orient = find_door_angle(doorPos.x.stl.num, doorPos.y.stl.num, owner);
    struct Thing* new_door = create_door(&doorPos, doorModel, orient, owner, true); 
    if(thing_is_invalid(new_door))
    {
        FTEST_FAIL_TEST("Failed to create locked door");
        return INVALID_THING;
    }
    new_door->health = health;

    return new_door;
}


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
