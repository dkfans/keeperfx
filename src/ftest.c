#include "ftest.h"

#ifdef FUNCTESTING

#include "pre_inc.h"

#include "game_legacy.h"
#include "bflib_memory.h"
#include "keeperfx.hpp"
#include "lvl_filesdk1.h"
#include "slab_data.h"
#include "room_util.h"

#include "post_inc.h"


#ifdef __cplusplus
extern "C" {
#endif


unsigned long ftest_total_actions = 0;
unsigned long ftest_current_action = 0;
GameTurn ftest_current_turn_counter = 0;
FTest_Action_Func ftest_actions_func_list[FTEST_ACTIONS_MAX];
GameTurn ftest_actions_func_turn_list[FTEST_ACTIONS_MAX];

TbBool ftest_append_action(FTest_Action_Func func, GameTurn turn_delay)
{
    if(!func)
    {
        FTEST_FAIL_TEST("Invalid FTest_Action_Func function");
        return false;
    }

    if(ftest_total_actions + 1 >= FTEST_ACTIONS_MAX)
    {
        FTEST_FAIL_TEST("Too many actions, increase FTEST_ACTIONS_MAX(%d)", FTEST_ACTIONS_MAX);
        return false;
    }

    ++ftest_total_actions;
    ftest_current_turn_counter += turn_delay;

    ftest_actions_func_list[ftest_total_actions] = func;
    ftest_actions_func_turn_list[ftest_total_actions] = ftest_current_turn_counter;
    return true;
}

TbBool ftest_init(unsigned short test_index)
{
    unsigned long tests_available = sizeof(ftest_init_func_list) / sizeof(ftest_init_func_list[0]);
    if(test_index >= tests_available)
    {
        FTEST_FAIL_TEST("Test index out of range! (see FTEST_TESTS_MAX in ftest.h if more tests are needed)");
        return false;
    }

    //run init for corresponding test level
    FTest_Init_Func init_func = ftest_init_func_list[test_index];
    if(init_func)
    {
        init_func();
    }
    else
    {
        FTEST_FAIL_TEST("Missing init function for test '%d'! (see ftest_list.c)", test_index + 1);
        return false;
    }

    return true;
}

TbBool ftest_update(const GameTurn game_turn)
{
    //if there was a test error, exit
    if (start_params.functest_flags & FTF_Failed)
    {
        return true;
    }

    const unsigned long ftest_actions_length = sizeof(ftest_actions_func_list) / sizeof(ftest_actions_func_list[0]);

    if(ftest_current_action < ftest_actions_length)
    {
        //get next valid test action
        FTest_Action_Func testAction = NULL;
        GameTurn testActionTurn = 0;
        do
        {
            testAction = ftest_actions_func_list[ftest_current_action];
            testActionTurn = ftest_actions_func_turn_list[ftest_current_action];
            if(testAction == NULL)
            {
                //empty, skip to next
                ++ftest_current_action;
            }
        } while (testAction == NULL && ftest_current_action < ftest_actions_length);
        
         
        if(testAction)
        {
            if(game_turn >= testActionTurn)
            {
                if(testAction(game_turn))
                {
                    //action completed, skip to next
                    ++ftest_current_action;
                }
            }
        }
    }

    TbBool isDoneTests = ftest_current_action >= ftest_actions_length;
    return isDoneTests;
}

TbBool ftest_replace_slabs(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, SlabKind slab_kind, PlayerNumber owner)
{
    TbBool valid_player_number = player_exists(get_player(owner));

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
                result = false;
            }
        }
    }

    return result;
}

TbBool ftest_does_player_own_any_slabs(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, PlayerNumber owner)
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

TbBool ftest_reveal_map(PlayerNumber plyr_idx)
{
    struct PlayerInfo* player = get_player(plyr_idx);
    if(player_invalid(player))
    {
        return false;
    }

    reveal_whole_map(player);
    
    return true;
}


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
