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


// example of test variables wraped in a struct, this prevents variable name collisions with other tests, allowing you to name your variables how you like!
struct ftest_donottouch__variables ftest_donottouch__vars = {
    .total_actions = 0,
    .current_action = 0,
    .previous_action = ULONG_MAX,
    .is_restarting_actions_queue = false,
    .current_turn_counter = 0
};

TbBool ftest_append_action(FTest_Action_Func func, GameTurn turn_delay, void* data)
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    if(!func)
    {
        FTEST_FAIL_TEST("Invalid FTest_Action_Func function");
        return false;
    }

    if(vars->total_actions + 1 >= FTEST_MAX_ACTIONS_PER_TEST)
    {
        FTEST_FAIL_TEST("Too many actions, increase FTEST_MAX_ACTIONS_PER_TEST(%d)", FTEST_MAX_ACTIONS_PER_TEST);
        return false;
    }

    vars->current_turn_counter += turn_delay;

    vars->actions_func_list[vars->total_actions] = func;
    vars->actions_func_turn_list[vars->total_actions] = vars->current_turn_counter;

    struct FTestActionArgs* action_args = &vars->actions_func_arguments[vars->total_actions];
    if(action_args != NULL)
    {
        action_args->intended_start_at_game_turn = vars->current_turn_counter;
        action_args->actual_started_at_game_turn = ULONG_MAX;
        action_args->action_index = vars->total_actions;
        action_args->times_executed = 0;
        action_args->data = data;
    }
    else
    {
        FTEST_FAIL_TEST("Current action function arguments should never be null, something has gone wrong!");
        return true;
    }

    ++vars->total_actions;
    return true;
}

TbBool ftest_init()
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_onlyappendtests__config* const conf = &ftest_onlyappendtests__conf;

    struct FTestConfig* pTestConfig = NULL;
    if(strnlen(start_params.functest_name, FTEST_MAX_NAME_LENGTH) > 0)
    {
        //find matching test by name
        for(unsigned short i = 0; i < FTEST_MAX_TESTS; ++i)
        {
            if(strcmpi(start_params.functest_name, conf->tests_list[i].name) == 0)
            {
                pTestConfig = &conf->tests_list[i];
                break;
            }
        }
    }
    else
    {
        //fallback to first available test
        for(unsigned short i = 0; i < FTEST_MAX_TESTS; ++i)
        {
            if(strnlen(conf->tests_list[i].name, FTEST_MAX_NAME_LENGTH) > 0)
            {
                pTestConfig = &conf->tests_list[i];
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

TbBool ftest_update()
{
    // to make the test variable names shorter, use a pointer!
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    //if there was a test error, exit
    if (start_params.functest_flags & FTF_Failed)
    {
        return true;
    }

    const unsigned long ftest_actions_length = sizeof(vars->actions_func_list) / sizeof(vars->actions_func_list[0]);

    if(vars->current_action < ftest_actions_length)
    {
        //get next valid test action
        FTest_Action_Func testAction = NULL;
        //GameTurn current_test_action_activation_turn = 0;
        struct FTestActionArgs* current_test_action_args = NULL;
        do
        {
            testAction = vars->actions_func_list[vars->current_action];
            current_test_action_args = &vars->actions_func_arguments[vars->current_action];
            
            if(testAction == NULL)
            {
                //empty, skip to next
                ++vars->current_action;
            }
            if(current_test_action_args == NULL)
            {
                FTEST_FAIL_TEST("Current action function arguments should never be null, something has gone wrong!");
                return true;
            }
        } while (testAction == NULL && vars->current_action < ftest_actions_length);
         
        if(testAction)
        {
            if(game.play_gameturn >= current_test_action_args->intended_start_at_game_turn)
            {
                if(vars->current_action != vars->previous_action)
                {
                    vars->previous_action = vars->current_action;
                    current_test_action_args->actual_started_at_game_turn = game.play_gameturn;
                }

                if(testAction(current_test_action_args))
                {
                    if(!vars->is_restarting_actions_queue)
                    {
                        //action completed, skip to next
                        ++vars->current_action;
                    }
                    else
                    {
                        vars->is_restarting_actions_queue = false;
                    }
                }
                ++current_test_action_args->times_executed;
            }
        }
    }

    TbBool isDoneTests = vars->current_action >= ftest_actions_length;
    return isDoneTests;
}

void ftest_restart_actions()
{
    const unsigned long ftest_actions_length = sizeof(ftest_donottouch__vars.actions_func_list) / sizeof(ftest_donottouch__vars.actions_func_list[0]);

    ftest_donottouch__vars.is_restarting_actions_queue = true;
    
    for(unsigned long i = 0; i < ftest_actions_length; ++i)
    {
        if(ftest_donottouch__vars.actions_func_list[i] == NULL)
        {
            continue;
        }

        //struct FTestActionArgs* prev_action_args = i == 0 ? NULL : &ftest_donottouch__vars.actions_func_arguments[i-1];

        struct FTestActionArgs* current_action_args = &ftest_donottouch__vars.actions_func_arguments[i];
        if(current_action_args != NULL)
        {
            current_action_args->intended_start_at_game_turn = game.play_gameturn + ftest_donottouch__vars.actions_func_turn_list[i];
            current_action_args->actual_started_at_game_turn = ULONG_MAX;
        }
    }

    ftest_donottouch__vars.current_action = 0;
}


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
