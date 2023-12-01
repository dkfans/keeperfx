#include "ftest.h"

#ifdef FUNCTESTING

#include "../pre_inc.h"

#include "../game_legacy.h"
#include "../bflib_memory.h"
#include "../keeperfx.hpp"
#include "../lvl_filesdk1.h"
#include "../slab_data.h"
#include "../room_util.h"

#include "../post_inc.h"


#ifdef __cplusplus
extern "C" {
#endif


unsigned long ftest_total_actions = 0;
unsigned long ftest_current_action = 0;
unsigned long ftest_previous_action = ULONG_MAX;

GameTurn ftest_current_turn_counter = 0;
GameTurn ftest_current_action_start_turn = 0;

FTest_Action_Func ftest_actions_func_list[FTEST_MAX_ACTIONS_PER_TEST];
GameTurn ftest_actions_func_turn_list[FTEST_MAX_ACTIONS_PER_TEST];

TbBool ftest_append_action(FTest_Action_Func func, GameTurn turn_delay)
{
    if(!func)
    {
        FTEST_FAIL_TEST("Invalid FTest_Action_Func function");
        return false;
    }

    if(ftest_total_actions + 1 >= FTEST_MAX_ACTIONS_PER_TEST)
    {
        FTEST_FAIL_TEST("Too many actions, increase FTEST_MAX_ACTIONS_PER_TEST(%d)", FTEST_MAX_ACTIONS_PER_TEST);
        return false;
    }

    ftest_current_turn_counter += turn_delay;

    ftest_actions_func_list[ftest_total_actions] = func;
    ftest_actions_func_turn_list[ftest_total_actions] = ftest_current_turn_counter;
    ++ftest_total_actions;
    return true;
}

TbBool ftest_init()
{
    struct FTestConfig* pTestConfig = NULL;
    if(strnlen(start_params.functest_name, FTEST_MAX_NAME_LENGTH) > 0)
    {
        //find matching test by name
        for(unsigned short i = 0; i < FTEST_MAX_TESTS; ++i)
        {
            if(strcmpi(start_params.functest_name, ftest_tests_list[i].name) == 0)
            {
                pTestConfig = &ftest_tests_list[i];
                break;
            }
        }
    }
    else
    {
        //fallback to first available test
        for(unsigned short i = 0; i < FTEST_MAX_TESTS; ++i)
        {
            if(strnlen(ftest_tests_list[i].name, FTEST_MAX_NAME_LENGTH) > 0)
            {
                pTestConfig = &ftest_tests_list[i];
                break;
            }
        }
        if(pTestConfig == NULL)
        {
            FTEST_FAIL_TEST("Could not find any tests..., make sure init name/function are added to ftest_tests_list (see ftest_list.c)", start_params.functest_name);
            return false;
        }
    }

    if(pTestConfig == NULL)
    {
        FTEST_FAIL_TEST("Could not find test '%s', make sure init name/function are added to ftest_tests_list (see ftest_list.c)", start_params.functest_name);
        return false;
    }
    else
    {
        JUSTLOG("Found test '%s', calling init function for test.", pTestConfig->name);

        //run init for corresponding test level
        if(pTestConfig->init_func)
        {
            pTestConfig->init_func();
        }
        else
        {
            FTEST_FAIL_TEST("Missing init function for test '%s'! (see ftest_list.c)", pTestConfig->name);
            return false;
        }
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
                if(ftest_current_action != ftest_previous_action)
                {
                    ftest_previous_action = ftest_current_action;
                    ftest_current_action_start_turn = game_turn;
                }

                if(testAction(ftest_current_action_start_turn))
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


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
