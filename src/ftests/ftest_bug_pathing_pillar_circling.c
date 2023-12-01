#include "ftest_bug_pathing_pillar_circling.h"

#ifdef FUNCTESTING

#include "../pre_inc.h"

#include "ftest.h"
#include "ftest_util.h"

#include "../game_legacy.h"
#include "../keeperfx.hpp"
#include "../player_instances.h"

#include "../post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif


// forward declarations - tests
TbBool ftest_bug_pathing_pillar_circling_action001__tunneler_dig_towards_pillar_test();

TbBool ftest_bug_pathing_pillar_circling_init()
{
    // this test will showcase multiple sub-tests, one sub-test per action
    // use of static variables allows some flexibility here

    ftest_append_action(ftest_bug_pathing_pillar_circling_action001__tunneler_dig_towards_pillar_test, 20);

    return true;
}

/**
 * @brief This action will be a sub-test, it will setup the situation of a single tunneler digging towards you, and replicate getting stuck on a pillar (portal?)
 */
TbBool ftest_bug_pathing_pillar_circling_action001__tunneler_dig_towards_pillar_test()
{
    ftest_util_reveal_map(PLAYER0);

    const MapSlabCoord slb_x_tunneler_start = 25;
    const MapSlabCoord slb_y_tunneler_start = 3;

    const MapSlabCoord slb_x_pillar = 15;
    const MapSlabCoord slb_y_pillar = 3;
    const MapSubtlCoord stl_x_pillar = slab_subtile_center(slb_x_pillar);
    const MapSubtlCoord stl_y_pillar = slab_subtile_center(slb_y_pillar);

    struct Thing* tunneler = INVALID_THING;

    static TbBool isTunnelerTestSetup = false;
    if(!isTunnelerTestSetup)
    {
        // clear starting position for tunneler
        ftest_util_replace_slabs(slb_x_tunneler_start, slb_y_tunneler_start, slb_x_tunneler_start, slb_y_tunneler_start, SlbT_PATH, PLAYER_NEUTRAL);

        // place pillar/column in the way
        {
            ftest_util_replace_slab_columns(slb_x_pillar, slb_y_pillar, PLAYER_NEUTRAL, SlbT_EARTH, 30, 30, 30
                                                                                                  , 30, 02, 30
                                                                                                  , 30, 30, 30); // 01 - impenetrable rock, 02 <-> 10 - dirt, 30 - path
        }

        struct Coord3d tunneler_pos;
        //tunneler_pos.x.val = 0;
        //tunneler_pos.y.val = 0;
        set_coords_to_slab_center(&tunneler_pos, slb_x_tunneler_start, slb_y_tunneler_start);

        // create tunneler
        tunneler = create_owned_special_digger(tunneler_pos.x.val, tunneler_pos.y.val, PLAYER_GOOD);
        if(thing_is_invalid(tunneler))
        {
            FTEST_FAIL_TEST("Failed to create tunneler");
            return true;
        }



        
        //todo
        // finish test with tunneler...

        

        // for(int i = 1; i < COLUMNS_COUNT && i < 85; ++i)
        // {
        //     ftest_util_replace_slabs(8+i, 1, 8+i, 1, SlbT_PATH, PLAYER_NEUTRAL);

            
        // }
        
        
        //    return game.columns.lookup[get_mapblk_column_index(mapblk)];
        
        // struct Column* column = get_column_at(slab_subtile(3, 1), slab_subtile(1, 1));

        // column->use = 1;
        // column->bitfields = 81;
        // column->solidmask = 31;
        // column->floor_texture = 22;
        // column->orient = 0;
        // column->cubes[0] = 10;
        // column->cubes[1] = 1;
        // column->cubes[2] = 1;
        // column->cubes[3] = 1;
        // column->cubes[4] = 5;
        // column->cubes[5] = 0;
        // column->cubes[6] = 0;

        //init_columns();

        //reinitialise_map_rooms();
        //ceiling_init(0, 1);

        isTunnelerTestSetup = true;
        return false;
    }

    // delay for a while so we can watch what's going on
    //if(game.play_gameturn < 1000)
    {
        return false;
    }

    return true; //proceed to next test action
}



#endif