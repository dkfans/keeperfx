#include "ftest.h"

#ifdef FUNCTESTING

#include "../pre_inc.h"

#include "../game_legacy.h"
#include "../keeperfx.hpp"
#include "../lvl_filesdk1.h"
#include "../slab_data.h"
#include "../room_util.h"
#include "../player_instances.h"
#includw "../gui_msgs.h"

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

const char* FTestFrameworkState_Strings[] = {
    "FTSt_InvalidState",
    "FTSt_PendingInitialSetup",
    "FTSt_InitialSetupCompleted",
    "FTSt_TestIsProcessingActions",
    "FTSt_TestHasCompletedActions_LoadNextTest",
    "FTSt_TestsCompletedSuccessfully",
};

const char* ftest_get_frameworkstate_name(FTestFrameworkState state)
{
    switch(state)
    {
        case FTSt_InvalidState:
            return FTestFrameworkState_Strings[0];
        case FTSt_PendingInitialSetup:
            return FTestFrameworkState_Strings[1];
        case FTSt_InitialSetupCompleted:
            return FTestFrameworkState_Strings[2];
        case FTSt_TestIsProcessingActions:
            return FTestFrameworkState_Strings[3];
        case FTSt_TestHasCompletedActions_LoadNextTest:
            return FTestFrameworkState_Strings[4];
        case FTSt_TestsCompletedSuccessfully:
            return FTestFrameworkState_Strings[5];

        default:
            return NULL;
    }
}

FTestFrameworkState ftest_change_state(FTestFrameworkState next)
{
    if(ftest_donottouch__vars.current_state == next)
    {
        FTESTLOG("Changing to state we are already on!");
    }

    const char* current_state_name = ftest_get_frameworkstate_name(ftest_donottouch__vars.current_state);
    const char* next_state_name = ftest_get_frameworkstate_name(next);

    if(current_state_name == NULL || next_state_name == NULL)
    {
        FTEST_FRAMEWORK_ABORT("Unknown state name!");
        return FTSt_InvalidState;
    }

    FTESTLOG("Changing from current state %s(%d) to %s(%d)", current_state_name, ftest_donottouch__vars.current_state, next_state_name, next);
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
        FTEST_FRAMEWORK_ABORT("Current action function arguments should never be null, something has gone wrong!");
        return false;
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

    TbBool too_many_tests = false;
    for(unsigned short test_list_id = 0; test_list_id < 2; ++test_list_id)
    {
        struct FTestConfig* current_test_list = NULL;

        switch(test_list_id)
        {
            case 0:
            {
                current_test_list = conf->tests_list;
                break;
            }

            case 1:
            {
                if(flag_is_set(start_params.functest_flags, FTF_IncludeLongTests))
                {
                    current_test_list = conf->long_running_tests_list;
                }
                break;
            }

            default:
                break;
        }

        if(current_test_list == NULL)
        {
            continue;
        }

        for(unsigned long i = 0; i < FTEST_MAX_TESTS; ++i)
        {
            test_config = &current_test_list[i];
            if(test_config == NULL || strnlen(test_config->test_name, FTEST_MAX_NAME_LENGTH) <= 0)
            {
                continue;
            }

            if(vars->total_tests >= FTEST_MAX_TESTS)
            {
                vars->total_tests++;
                too_many_tests = true;
                FTESTLOG("Skipped adding test '%s', too many tests!", test_config->test_name);
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
    }

    if(too_many_tests)
    {
        FTEST_FRAMEWORK_ABORT("Too many tests %lu, increase the value of FTEST_MAX_TESTS(%d)", vars->total_tests, FTEST_MAX_TESTS);
        return false;
    }

    return vars->total_tests > 0;
}

struct FTestConfig* ftest_get_next_test(TbBool restart)
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
        FTEST_FRAMEWORK_ABORT("Missing test... this shouldn't happen.");
    }

    return test_config;
}

TbBool ftest_setup_test(struct FTestConfig* const test_config)
{
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    if(test_config == NULL)
    {
        FTEST_FAIL_TEST("'test' cannot be NULL");
        return false;
    }

    // set frame skip
    game.frame_skip = test_config->frame_skip;

    // set seed
    start_params.functest_seed = test_config->seed;

    // change campaign / level
    strcpy(start_params.selected_campaign, test_config->level_file);
    LevelNumber selected_level = test_config->level;

    str_append(start_params.selected_campaign, sizeof(start_params.selected_campaign), ".cfg");
    TbBool result = change_campaign(start_params.selected_campaign);
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
    struct PlayerInfo *player = get_my_player();
    set_players_packet_action(player, PckA_QuitToMainMenu, 0, 0, 0, 0);           
}

void ftest_srand()
{
    if(flag_is_set(start_params.functest_flags, FTF_Enabled))
    {
        if(start_params.functest_seed == 0)
        {
            game.action_random_seed = game.play_gameturn;
            game.ai_random_seed = game.play_gameturn * 9377 + 9439 + game.play_gameturn;
            game.player_random_seed = game.play_gameturn * 9439 + 9377 + game.play_gameturn;
            unsync_random_seed = game.play_gameturn;
            sound_random_seed = game.play_gameturn * 7919 + 7927;
            srand(game.play_gameturn);
        }
        else
        {
            game.action_random_seed = start_params.functest_seed;
            game.ai_random_seed = start_params.functest_seed * 9377 + 9439 + game.play_gameturn;
            game.player_random_seed = start_params.functest_seed * 9439 + 9377 + game.play_gameturn;
            unsync_random_seed = start_params.functest_seed;
            sound_random_seed = start_params.functest_seed * 7919 + 7927;
            srand(start_params.functest_seed);
        }
    }
}

TbBool ftest_init()
{
    struct ftest_donottouch__variables* const vars = &ftest_donottouch__vars;

    if(!flag_is_set(start_params.functest_flags, FTF_Enabled))
    {
        FTESTLOG("Functional Tests are not enabled, skipping Init.");
        return false;
    }

    if(vars->current_state != FTSt_PendingInitialSetup)
    {
        FTEST_FRAMEWORK_ABORT("Init should only be called once at startup!")
        return false;
    }

    start_params.no_intro = 1;

    clear_flag(start_params.functest_flags, FTF_TestFailed);
    clear_flag(start_params.functest_flags, FTF_LevelLoaded);

    if(flag_is_set(start_params.operation_flags, GOF_SingleLevel))
    {
        FTESTLOG("Unsetting GOF_SingleLevel, -level arg is ignored for functional tests!");
        clear_flag(start_params.operation_flags,GOF_SingleLevel);
    }

    if(!ftest_fill_teststorun_by_name(start_params.functest_name))
    {
        FTEST_FRAMEWORK_ABORT("Failed to find any tests. (user provided: '%s') make sure init name/function are added to tests_list or long_running_tests_list (see ftest_list.c)", start_params.functest_name)
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
        FTEST_FRAMEWORK_ABORT("Tests not initialized! This shouldn't happen! ftest_init should be called once before ftest_update!");
        return ftest_change_state(FTSt_InvalidState);
    }

    // if there was a framework error, exit
    if (flag_is_set(start_params.functest_flags, FTF_Abort))
    {
        return ftest_change_state(FTSt_InvalidState);
    }

    // override seed
    ftest_srand();

    // track previous state
    if(out_prev_state != NULL)
    {
        (*out_prev_state) = vars->current_state;
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
            message_add_fmt(MsgType_Player, PLAYER0, "Initializing Functional Test %s", vars->pending_init->test_name);
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

        struct FTestConfig* current_test_config = vars->tests_to_run[vars->current_test];
        if(current_test_config == NULL)
        {
            FTEST_FRAMEWORK_ABORT("Missing test config... this shouldn't happen.");
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
                    FTEST_FRAMEWORK_ABORT("Current action function arguments should never be null, something has gone wrong!");
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

        TbBool test_failed = flag_is_set(start_params.functest_flags, FTF_TestFailed);
        TbBool is_done_actions = vars->current_action >= ftest_actions_length || test_failed; // actions completed, OR test failed
        if(is_done_actions)
        {
            if(!test_failed)
            {
                FTESTLOG("Test %s passed!", current_test_config->test_name);
            }
            else
            {
                if(flag_is_set(start_params.functest_flags, FTF_ExitOnTestFailure))
                {
                    FTESTLOG("Test %s failed and FTF_ExitOnTestFailure is set, aborting...", current_test_config->test_name);
                    return ftest_change_state(FTSt_InvalidState);
                }
                else
                {
                    FTESTLOG("Test %s failed...", current_test_config->test_name);
                }
            }

            if(current_test_config->repeat_n_times-1 > 0)
            {
                FTESTLOG("Test %s is marked to repeat %d times, restarting...", current_test_config->test_name, current_test_config->repeat_n_times);
                --vars->current_test;
                --current_test_config->repeat_n_times;
            }

            clear_flag(start_params.functest_flags, FTF_TestFailed);
            clear_flag(start_params.functest_flags, FTF_LevelLoaded);
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

/**
 * @brief Returns the current test config (some tests may want to modify it... meta!)
 * 
 */
struct FTestConfig* ftest_get_current_test_config()
{
    return ftest_donottouch__vars.tests_to_run[ftest_donottouch__vars.current_test];
}

#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
