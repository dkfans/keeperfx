/**
 * @file ftest.h
 * @author demonds1
 * @brief To add new functional tests you must add your init function inside ftest_list.c
 * See ftest_bug_imp_tp_job_attack_door.h for an example.
 * DO NOT FORGET TO ADD YOUR object file to the makefile, eg: obj/ftest_<test_name>.o
 * @version 0.1
 * @date 2023-11-24
 * 
 * @copyright Copyright (c) 2023
 */

#ifndef KEEPFX_FTEST_H
#define KEEPFX_FTEST_H

#include "globals.h"

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
 * NOTE: DO NOT FORGET TO ADD YOUR object file to the makefile, eg: obj/ftest_<test_name>.o
 */
typedef TbBool (*FTest_Init_Func)(void);

/**
 * @brief Function Test Action Func -
 * Your test will be comprised of these actions, append them inside your init func using ftest_append_action
 */
typedef TbBool (*FTest_Action_Func)(void);

struct FTestConfig {
    char name[FTEST_MAX_NAME_LENGTH];
    FTest_Init_Func init_func;
};

extern struct FTestConfig ftest_tests_list[FTEST_MAX_TESTS];

extern unsigned long ftest_total_actions;
extern unsigned long ftest_current_action;
extern GameTurn ftest_current_turn_counter;
extern FTest_Action_Func ftest_actions_func_list[];
extern GameTurn ftest_actions_func_turn_list[];

/**
 * @brief Add the given FTest_Action_Func to the test action list, it will be called at turn_delay, the turns are accumulated for each action
 * 
 * @param func 
 * @param turn_delay 
 * @return TbBool 
 */
TbBool ftest_append_action(FTest_Action_Func func, GameTurn turn_delay);

TbBool ftest_init();
TbBool ftest_update(const GameTurn game_turn);

// helpers

/**
 * @brief Replaces slabs in the given area.
 * If the owner exists, it will claim the slab first.
 * 
 * @param slb_x_from 
 * @param slb_y_from 
 * @param slb_x_to 
 * @param slb_y_to 
 * @param slab_kind 
 * @param owner
 * @return true, false if any slab replacements failed
 */
TbBool ftest_replace_slabs(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, SlabKind slab_kind, PlayerNumber owner);

/**
 * @brief Checks if the player owns any slabs in the given area.
 * 
 * @param slb_x_from 
 * @param slb_y_from 
 * @param slb_x_to 
 * @param slb_y_to 
 * @param owner 
 * @return true if any slabs owned, false otherwise
 */
TbBool ftest_does_player_own_any_slabs(MapSlabCoord slb_x_from, MapSlabCoord slb_y_from, MapSlabCoord slb_x_to, MapSlabCoord slb_y_to, PlayerNumber owner);

/**
 * @brief Reveals the map for the given player
 * 
 * @param plyr_idx 
 * @return
 */
TbBool ftest_reveal_map(PlayerNumber plyr_idx);


#ifdef __cplusplus
}
#endif

#endif // FUNCTESTING

#endif // KEEPFX_FTEST_H