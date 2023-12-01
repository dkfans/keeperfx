/**
 * @file ftest.h
 * @author demonds1
 * @brief To add new functional tests you must add your init function inside ftest_list.c
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
    set_flag_byte(&start_params.functest_flags, FTF_Failed, true); \
    FTESTLOG("Failing test"); \
    FTESTLOG(format, ##__VA_ARGS__); \
}

#define FTEST_MAX_NAME_LENGTH 128
#define FTEST_MAX_TESTS 128
#define FTEST_MAX_ACTIONS_PER_TEST 100

typedef unsigned char TbBool; //redefine rather than include extraneus header info

/**
 * @brief Functional Test Init Func -
 * Your test should implement this function and add it to the list inside ftest_list.c
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

/**
 * @brief Function Test Action Func -
 * Your test will be comprised of these actions, append them inside your init func using ftest_append_action
 */
typedef TbBool (*FTest_Action_Func)(struct FTestActionArgs* const args);

struct FTestConfig {
    char name[FTEST_MAX_NAME_LENGTH];
    FTest_Init_Func init_func;
};

struct ftest_onlyappendtests__config
{
    struct FTestConfig tests_list[FTEST_MAX_TESTS];
};
extern struct ftest_onlyappendtests__config ftest_onlyappendtests__conf;

struct ftest_donottouch__variables
{
    unsigned long total_actions;
    unsigned long current_action;
    unsigned long previous_action;

    GameTurn current_turn_counter;

    FTest_Action_Func           actions_func_list[FTEST_MAX_ACTIONS_PER_TEST];
    GameTurn                    actions_func_turn_list[FTEST_MAX_ACTIONS_PER_TEST];

    struct FTestActionArgs      actions_func_arguments[FTEST_MAX_ACTIONS_PER_TEST];
};
extern struct ftest_donottouch__variables ftest_donottouch__vars;

/**
 * @brief Add the given FTest_Action_Func to the test action list, it will be called at turn_delay, the turns are accumulated for each action
 * 
 * @param func 
 * @param turn_delay 
 * @return TbBool 
 */
TbBool ftest_append_action(FTest_Action_Func func, GameTurn turn_delay, void* data);

TbBool ftest_init();
TbBool ftest_update();


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING