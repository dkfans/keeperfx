#include "ftest.h"

#ifdef FUNCTESTING

#include "../pre_inc.h"

#include "../game_legacy.h"
#include "../bflib_memory.h"
#include "../keeperfx.hpp"
#include "../lvl_filesdk1.h"
#include "../slab_data.h"
#include "../room_util.h"
#include "../player_instances.h"

#include "../post_inc.h"


#ifdef __cplusplus
extern "C" {
#endif


// example of test variables wraped in a struct, this prevents variable name collisions with other tests, allowing you to name your variables how you like!
struct ftest_donottouch__variables ftest_donottouch__vars = {
    .current_state = FTSt_PendingInitialSetup,
    .previous_state = FTSt_PendingInitialSetup,
    .total_tests = 0,
    .current_test = 0,
    .previous_test = 0,
    .pending_init = NULL,
    .total_actions = 0,
    .current_action = 0,
    .previous_action = ULONG_MAX,
    .is_restarting_actions_queue = false,
    .current_turn_counter = 0
};

FTestFrameworkState ftest_change_state(FTestFrameworkState next)
{
    if(ftest_donottouch__vars.current_state == next)
    {
        FTESTLOG("Changing to state we are already on!");
    }

    FTESTLOG("Changing from current state %d to %d", ftest_donottouch__vars.current_state, next);
    ftest_donottouch__vars.previous_state = ftest_donottouch__vars.current_state;
    ftest_donottouch__vars.current_state = next;

    return ftest_donottouch__vars.current_state;
}

void ftest_reset_action_args(struct FTestActionArgs* const args)
{
    if(args == NULL)
    {
        return;
    }

    args->intended_start_at_game_turn = 0;
    args->actual_started_at_game_turn = ULONG_MAX;
    args->action_index = 0;
    args->times_executed = 0;
    args->data = NULL;
}

void ftest_clear_actions()
{
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    vars->total_actions = 0;
    vars->current_action = 0;
    vars->previous_action = ULONG_MAX;
    vars->is_restarting_actions_queue = false;
    vars->current_turn_counter = 0;

    for(unsigned long i = 0; i < FTEST_MAX_ACTIONS_PER_TEST; ++i)
    {
        vars->actions_func_list[i] = NULL;
        vars->actions_func_turn_list[i] = 0;

        struct FTestActionArgs* action_args = &vars->actions_func_arguments[i];
        ftest_reset_action_args(action_args);
    }
}

TbBool ftest_append_action(FTest_Action_Func func, GameTurn turn_delay, void* data)
{
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
        ftest_reset_action_args(action_args);
        action_args->intended_start_at_game_turn = vars->current_turn_counter;
        action_args->action_index = vars->total_actions;
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

TbBool ftest_parse_arg(char * const arg)
{
    if(strlen(arg) > 0 && arg[0] != '-')
    {
        FTESTLOG("Argument '%s' provided by user", arg);
        snprintf(start_params.functest_name, sizeof(start_params.functest_name), "%s", arg);        
        return true;
    }

    return false;
}

TbBool ftest_fill_teststorun_by_name(char* const name)
{
    struct ftest_onlyappendtests__config* const conf = &ftest_onlyappendtests__conf;
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    struct FTestConfig* test_config = NULL;
    vars->total_tests = 0;
    for(unsigned long i = 0; i < FTEST_MAX_TESTS; ++i) { vars->tests_to_run[i] = NULL; }
    for(unsigned long i = 0; i < FTEST_MAX_TESTS; ++i)
    {
        test_config = &conf->tests_list[i];
        if(test_config == NULL || strnlen(test_config->test_name, FTEST_MAX_NAME_LENGTH) <= 0)
        {
            continue;
        }

        if(name[0] == '*' || strnlen(name, FTEST_MAX_NAME_LENGTH) <= 0)
        {
            FTESTLOG("Added test '%s' via wildcard match", test_config->test_name);
            vars->tests_to_run[vars->total_tests++] = test_config;
        }
        else if(strcmp(name, test_config->test_name) == 0)
        {
            FTESTLOG("Added test '%s' via exact match", test_config->test_name);
            vars->tests_to_run[vars->total_tests++] = test_config;
        }
    }

    return vars->total_tests > 0;
}

struct FTestConfig* const ftest_get_next_test(TbBool restart)
{
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    if(restart)
    {
        vars->current_test = 0;
    }
    else if(++vars->current_test >= vars->total_tests)
    {
        FTESTLOG("No more tests available, pass restart=true if you want to restart tests");
        vars->current_test = vars->total_tests;
        return NULL;
    }

    struct FTestConfig* test_config = vars->tests_to_run[vars->current_test];
    if(test_config == NULL)
    {
        FTEST_FAIL_TEST("Missing test... this shouldn't happen.");
    }

    return test_config;
}

TbBool ftest_setup_test(struct FTestConfig* const test_config)
{
    struct ftest_onlyappendtests__config* const conf = &ftest_onlyappendtests__conf;
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    if(test_config == NULL)
    {
        FTEST_FAIL_TEST("'test' cannot be NULL");
        return false;
    }

    // set frame skip
    game.frame_skip = test_config->frame_skip;

    // change campaign / level
    strcpy(start_params.selected_campaign, test_config->level_file);
    LevelNumber selected_level = test_config->level;

    TbBool result = change_campaign(strcat(start_params.selected_campaign,".cfg"));
    if(!result)
    {
        FTEST_FAIL_TEST("Failed to load campaign '%d'", start_params.selected_campaign)
        return false;
    }
    else
    {
        set_selected_level_number(selected_level);
    }
    
    ftest_clear_actions();

    //queue init for corresponding test level
    vars->pending_init = test_config;

    return true;
}

void ftest_quit_game()
{
    FTESTLOG("Quitting/exiting map");
    struct PlayerInfo *player;
    player = get_my_player();
    set_players_packet_action(player, PckA_Unknown001, 0, 0, 0, 0);           
}

void ftest_srand()
{
    if(flag_is_set(start_params.functest_flags, FTF_Enabled))
    {
        game.action_rand_seed = 1;
        game.unsync_rand_seed = 1;
        srand(1);
    }
}

TbBool ftest_init()
{
    struct ftest_onlyappendtests__config* const conf = &ftest_onlyappendtests__conf;
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    if(vars->current_state != FTSt_PendingInitialSetup)
    {
        FTEST_FAIL_TEST("Init should only be called once at startup!")
        return false;
    }

    start_params.no_intro = 1;

    set_flag_byte(&start_params.functest_flags, FTF_Enabled, true);
    set_flag_byte(&start_params.functest_flags, FTF_Failed, false);
    set_flag_byte(&start_params.functest_flags, FTF_LevelLoaded, false);

    if(flag_is_set(start_params.operation_flags, GOF_SingleLevel))
    {
        FTESTLOG("Unsetting GOF_SingleLevel, -level arg is ignored for functional tests!");
        set_flag_byte(&start_params.operation_flags,GOF_SingleLevel,false);
    }

    set_flag_byte(&start_params.operation_flags,GOF_SingleLevel,true);

    if(!ftest_fill_teststorun_by_name(start_params.functest_name))
    {
        FTEST_FAIL_TEST("Failed to find any tests. (user provided: '%d') make sure init name/function are added to tests_list (see ftest_list.c)", start_params.functest_name)
        return false;
    }

    ftest_change_state(FTSt_InitialSetupCompleted);

    return true;
}

FTestFrameworkState ftest_update(FTestFrameworkState* const out_prev_state)
{
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;
    
    // enforce init call first
    if(!flag_is_set(start_params.functest_flags, FTF_Enabled) || vars->current_state == FTSt_PendingInitialSetup)
    {
        FTEST_FAIL_TEST("Tests not initialized! This shouldn't happen! ftest_init should be called once before ftest_update!");
        return ftest_change_state(FTSt_InvalidState);
    }

    // override seed
    game.action_rand_seed = game.play_gameturn;
    game.unsync_rand_seed = game.play_gameturn;
    srand(game.play_gameturn);

    // track previous state
    if(out_prev_state != NULL)
    {
        (*out_prev_state) = vars->current_state;
    }

    // if there was a test error, exit
    if (flag_is_set(start_params.functest_flags, FTF_Failed))
    {
        return ftest_change_state(FTSt_InvalidState);
    }

    // tests already completed!
    if(vars->current_state == FTSt_TestsCompletedSuccessfully)
    {
        return FTSt_TestsCompletedSuccessfully; 
    }

    // setup tests (change to campaign/level)
    if(vars->current_state == FTSt_InitialSetupCompleted || vars->current_state == FTSt_TestHasCompletedActions_LoadNextTest)
    {
        TbBool restart_tests = vars->current_state == FTSt_InitialSetupCompleted;
        struct FTestConfig* test_config = ftest_get_next_test(restart_tests);

        if(test_config == NULL)
        {
            FTESTLOG("Tests completed!");
            return ftest_change_state(FTSt_TestsCompletedSuccessfully);
        }
        else
        {
            FTESTLOG("Found next test '%s', setting up...", test_config->test_name);
            if(!ftest_setup_test(test_config))
            {
                return ftest_change_state(FTSt_InvalidState);
            }

            ftest_change_state(FTSt_TestIsProcessingActions);
        }
    }

    // process test init/actions
    if(vars->current_state == FTSt_TestIsProcessingActions)
    {
        if(!flag_is_set(start_params.functest_flags, FTF_LevelLoaded))
        {
            FTESTLOG("Waiting for level load...");
            return FTSt_TestIsProcessingActions;
        }

        if(vars->pending_init != NULL)
        {
            message_add_fmt(PLAYER0, "Initializing Functional Test %s", vars->pending_init->test_name);
            FTESTLOG("Initializing Functional Test %s", vars->pending_init->test_name);
            if(vars->pending_init->init_func)
            {
                vars->pending_init->init_func();
                vars->pending_init = NULL;
            }
            else
            {
                FTEST_FAIL_TEST("Missing init function for test '%s'! (see ftest_list.c)", vars->pending_init->test_name);
                return ftest_change_state(FTSt_InvalidState);
            }
        }

        const unsigned long ftest_actions_length = sizeof(vars->actions_func_list) / sizeof(vars->actions_func_list[0]);
        if(vars->current_action < ftest_actions_length)
        {
            //get next valid test action
            FTest_Action_Func test_action = NULL;
            struct FTestActionArgs* current_test_action_args = NULL;
            do
            {
                test_action = vars->actions_func_list[vars->current_action];
                current_test_action_args = &vars->actions_func_arguments[vars->current_action];
                
                if(test_action == NULL)
                {
                    //empty, skip to next
                    ++vars->current_action;
                }
                if(current_test_action_args == NULL)
                {
                    FTEST_FAIL_TEST("Current action function arguments should never be null, something has gone wrong!");
                    return FTSt_InvalidState;
                }
            } while (test_action == NULL && vars->current_action < ftest_actions_length);
            
            if(test_action)
            {
                if(game.play_gameturn >= current_test_action_args->intended_start_at_game_turn)
                {
                    if(vars->current_action != vars->previous_action)
                    {
                        FTESTLOG("executing action %d", vars->current_action);
                        vars->previous_action = vars->current_action;
                        current_test_action_args->actual_started_at_game_turn = game.play_gameturn;
                    }

                    if(test_action(current_test_action_args))
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

        TbBool isDoneActions = vars->current_action >= ftest_actions_length;
        if(isDoneActions)
        {
            set_flag_byte(&start_params.functest_flags, FTF_LevelLoaded, false);
            ftest_quit_game();
            return ftest_change_state(FTSt_TestHasCompletedActions_LoadNextTest);
        }

        return FTSt_TestIsProcessingActions;
    }

    return FTSt_InvalidState;
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
