#include "ftest_util.h"

#ifdef FUNCTESTING

#include "../pre_inc.h"

#include "../game_legacy.h"
#include "../bflib_memory.h"
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

struct Thing* ftest_util_create_random_creature(MapCoord x, MapCoord y, PlayerNumber owner, CrtrExpLevel max_lv)
{
    ThingModel crmodel;
    while (1) {
        crmodel = GAME_RANDOM(gameadd.crtr_conf.model_count) + 1;
        // Accept any non-spectator creatures
        struct CreatureModelConfig* crconf = &gameadd.crtr_conf.model[crmodel];
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
    CrtrExpLevel lv = GAME_RANDOM(max_lv);
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


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
