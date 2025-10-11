/**
 * @file ftest.h
 * @author demonds1
 * @brief Functional Test Framework
 * To add new functional tests you must add your init function inside ftest_list.c
 * See ftest_template.h for an example.
 * @version 0.1
 * @date 2023-11-24
 * 
 * @copyright Copyright (c) 2023
 */

#pragma once

#include "../globals.h"

#ifdef FUNCTESTING

#ifdef __cplusplus
extern "C" {
#endif

#define FTESTLOG(format, ...) LbFTestLog("[%d] %s: " format "\n", get_gameturn(), __func__ , ##__VA_ARGS__)

#define FTEST_FAIL_TEST(format, ...) { \
    set_flag(start_params.functest_flags, FTF_TestFailed); \
    FTESTLOG("Failing test"); \
    FTESTLOG(format, ##__VA_ARGS__); \
}

#define FTEST_FRAMEWORK_ABORT(format, ...) { \
    set_flag(start_params.functest_flags, FTF_Abort); \
    FTESTLOG("Framework error, aborting..."); \
    FTESTLOG(format, ##__VA_ARGS__); \
}

#define FTEST_MAX_NAME_LENGTH 128
#define FTEST_MAX_TESTS 128
#define FTEST_MAX_ACTIONS_PER_TEST 100

typedef unsigned char TbBool; //redefine rather than include extraneus header info

/**
 * @brief Functional Test Init Func -
 * Your test should implement this function and add it to the list inside ftest_list.c
 * The init func for your test is used to a) setup your initial test environment b) add your tests actions to the list of actions for execution.
 */
typedef TbBool (*FTest_Init_Func)();

struct FTestActionArgs
{
    GameTurn intended_start_at_game_turn;
    GameTurn actual_started_at_game_turn;
    unsigned long action_index;
    unsigned long times_executed;
    void* data;
};

typedef enum FTestActionResult
{
    FTRs_Repeat_Current_Action = 0,
    FTRs_Go_To_Next_Action = 1
} FTestActionResult;

/**
 * @brief Function Test Action Func -
 * Your test will be comprised of these actions, append them inside your init func using ftest_append_action
 */
typedef FTestActionResult (*FTest_Action_Func)(struct FTestActionArgs* const args);

typedef enum FTestFrameworkState
{
    FTSt_InvalidState = -1,
    FTSt_PendingInitialSetup = 0,
    FTSt_InitialSetupCompleted = 1,
    FTSt_TestIsProcessingActions = 2,
    FTSt_TestHasCompletedActions_LoadNextTest = 3,
    FTSt_TestsCompletedSuccessfully = 4
} FTestFrameworkState;

extern const char* FTestFrameworkState_Strings[];

const char* ftest_get_frameworkstate_name(FTestFrameworkState state);

/**
 * @brief Configuration object for a functional test
 * 
 */
struct FTestConfig {
    /**
     * @brief The name of the test, it is used to run a specific test via args -ftest <test_name>
     * 
     */
    char test_name[FTEST_MAX_NAME_LENGTH];

    /**
     * @brief The init function of your test, that will append actions to perform
     * 
     */
    FTest_Init_Func init_func;

    /**
     * @brief The campaign or mappack this test uses
     * 
     */
    char level_file[FTEST_MAX_NAME_LENGTH];

    /**
     * @brief The level number for the test to load (mappack or campaign)
     * 
     */
    LevelNumber level;

    /**
     * @brief Override frameskip for your test (optional)
     * 
     */
    int frame_skip;

    /**
     * @brief Override seed for your test (optional)
     * This is generally not needed unless you want to fudge the seed value for a specific test.
     * (0): default behaviour, overrides seed each game turn with the current GameTurn. (non 0): overrides seed each game turn to that value. NOTE: Even cheats like spawning random creatures will always return the same type of creature for that seed.
     */
    unsigned int seed;

    /**
     * @brief Repeat the test N times (optional)
     * 
     */
    unsigned short repeat_n_times;
};

struct ftest_onlyappendtests__config
{
    struct FTestConfig tests_list[FTEST_MAX_TESTS];
    struct FTestConfig long_running_tests_list[FTEST_MAX_TESTS];
};
extern struct ftest_onlyappendtests__config ftest_onlyappendtests__conf;

struct ftest_donottouch__variables
{
    FTestFrameworkState current_state;
    FTestFrameworkState previous_state;

    unsigned long total_tests;
    unsigned long current_test;
    unsigned long previous_test;
    struct FTestConfig* tests_to_run[FTEST_MAX_TESTS];
    struct FTestConfig* pending_init;

    unsigned long total_actions;
    unsigned long current_action;
    unsigned long previous_action;
    TbBool is_restarting_actions_queue;

    GameTurn current_turn_counter;

    FTest_Action_Func           actions_func_list[FTEST_MAX_ACTIONS_PER_TEST];
    GameTurn                    actions_func_turn_list[FTEST_MAX_ACTIONS_PER_TEST];

    struct FTestActionArgs      actions_func_arguments[FTEST_MAX_ACTIONS_PER_TEST];
};
extern struct ftest_donottouch__variables ftest_donottouch__vars;

/**
 * @brief 
 * 
 * @param func 
 * @param turn_delay 
 * @return TbBool 
 */

/**
 * @brief Add the given FTest_Action_Func to the test action list, it will be called at turn_delay, the turns are accumulated for each action
 * 
 * @param func Your action function that will be called
 * @param turn_delay How many GameTurns before this action should be executed (after the previous action has completed)
 * @param data Pointer to any extra data you want passed to your action (this allows re-using actions, even between different tests if you want)
 * @return TbBool 
 */
TbBool ftest_append_action(FTest_Action_Func func, GameTurn turn_delay, void* data);

TbBool ftest_parse_arg(char* const arg);

void ftest_srand();

TbBool ftest_init();

FTestFrameworkState ftest_update(FTestFrameworkState* const out_prev_state);

/**
 * @brief Resets the current action to action 0 of the actions list (allowing repeat test logic)
 * 
 */
void ftest_restart_actions();

/**
 * @brief Returns the current test config (some tests may want to modify it... meta!)
 * 
 */
struct FTestConfig* ftest_get_current_test_config();

#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING
