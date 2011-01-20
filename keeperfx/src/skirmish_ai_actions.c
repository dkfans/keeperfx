/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file skirmish_ai_actions.c
 *     Skirmish AI actions implementation.
 * @par Purpose:
 *     Actions for Skirmish AI. These encapsulate atomic actions a player can take.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     2011
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "skirmish_ai_actions.h"

#include <stdlib.h>

enum FuncReason
{
    ON_PROCESS,
    ON_DESTROY,
    QUERY_IS_FINISHED,
    QUERY_CAN_FINISH
};

enum SAI_ActionType
{
    //ACTION_MOVE_CREATURE, //pick up and drop
    ACTION_MARK_DIG,
    ACTION_BUILD_ROOM,

    ACTION_COUNT //counter
};

struct SAI_ActionHeader
{
    enum SAI_ActionType type;
    SAI_Action next;
    int time;
};

typedef int (*ActionFunc)(enum FuncReason reason, SAI_Action action);

struct MarkDigAction
{
    struct SAI_ActionHeader header;
    int x;
    int y;
};

struct BuildRoomAction
{
    struct SAI_ActionHeader header;
    int x;
    int y;
    int kind;
};


static ActionFunc action_funcs[ACTION_COUNT] =
{

};


int SAI_is_action_done(SAI_Action action)
{
    struct SAI_ActionHeader * header;

    header = (struct SAI_ActionHeader *) action;
    return action_funcs[header->type](QUERY_IS_FINISHED, action);
}

int SAI_is_action_stuck(SAI_Action action)
{
    struct SAI_ActionHeader * header;

    header = (struct SAI_ActionHeader *) action;
    return !action_funcs[header->type](QUERY_CAN_FINISH, action);
}

void SAI_process_action(SAI_Action action, int player)
{
    struct SAI_ActionHeader * header;

    header = (struct SAI_ActionHeader *) action;
    action_funcs[header->type](ON_PROCESS, action);
}

SAI_Action SAI_get_next_action(SAI_Action action)
{
    struct SAI_ActionHeader * header;

    header = (struct SAI_ActionHeader *) action;

    return header->next;
}

void SAI_append_action(SAI_Action * head, SAI_Action tail)
{
    struct SAI_ActionHeader * header;

    if (*head) {
        header = (struct SAI_ActionHeader *) *head;
        for (;;) {
            if (header->next) {
                header = (struct SAI_ActionHeader *) header->next;
            }
            else {
                header->next = tail;
                break;
            }
        }
    }
    else {
        *head = tail;
    }
}

void SAI_destroy_action(SAI_Action action)
{
    struct SAI_ActionHeader * header;

    if (action) {
        header = (struct SAI_ActionHeader *) action;
        action_funcs[header->type](ON_DESTROY, action);
        free(action);
    }
}

SAI_Action SAI_mark_dig_action(int x, int y)
{
    struct MarkDigAction * action;
    action = (struct MarkDigAction *) malloc(sizeof(*action));
    action->x = x;
    action->y = y;

    return action;
}

SAI_Action SAI_build_room_action(int x, int y, int kind)
{
    struct BuildRoomAction * action;
    action = (struct BuildRoomAction *) malloc(sizeof(*action));
    action->x = x;
    action->y = y;
    action->kind = kind;

    return action;
}
