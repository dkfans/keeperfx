/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_actions.h
 *     Header file for skirmish_ai_actions.c
 * @par Purpose:
 *     Actions for Skirmish AI. These encapsulate atomic actions a player can take.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef SKIRMISH_AI_ACTIONS_H
#define SKIRMISH_AI_ACTIONS_H

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _SAI_Action * SAI_Action;


/**
 * Is the action finished (or aborted)?
 * @param action
 * @return
 */
int SAI_is_action_done(SAI_Action action);

/**
 * If this action should remain but cannot be completed right now.
 * @param action
 * @return
 */
int SAI_is_action_stuck(SAI_Action action);

/**
 * Executes the action for this frame.
 * @param action
 * @param plyr Index of game player that will execute this action.
 */
void SAI_process_action(SAI_Action action, int plyr);

/**
 * Gets the action that follows this one.
 * @param action
 * @return
 */
SAI_Action SAI_get_next_action(SAI_Action action);

/**
 * Appends one or more actions to the end of an action list.
 * @param head If location pointed at is NULL, the location pointed at is set to tail.
 * @param tail Will follow the last element of the head list.
 */
void SAI_append_action(SAI_Action * head, SAI_Action tail);

/**
 * Destroys an action and deallocates any allocated memory.
 * @param action Can be NULL, by convention for destruction functions.
 */
void SAI_destroy_action(SAI_Action action);


SAI_Action SAI_mark_dig_action(int plyr, int x, int y);
SAI_Action SAI_build_room_action(int plyr, int x, int y, int kind);


#ifdef __cplusplus
}
#endif

#endif //SKIRMISH_AI_ACTIONS_H
