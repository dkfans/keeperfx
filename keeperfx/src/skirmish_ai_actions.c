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

#include "player_computer.h"

#include <assert.h>
#include <stdlib.h>

#define TO_CENTER_STL(x)    (x * 3 + 1)

struct _SAI_Action
{
    SAI_Action next; //list
    unsigned char done; //non-zero if action is finished
    unsigned char failed; //tracks number of failures, up to 255

    //the following correspond the arguments of game_action()
    unsigned short gaction; //2nd
    unsigned short a3; //3rd etc.
    unsigned short stl_x;
    unsigned short stl_y;
    unsigned short param1;
    unsigned short param2;
    char plyr_idx; //1st
};


int SAI_is_action_done(SAI_Action action)
{
    return action->done;
}

int SAI_is_action_stuck(SAI_Action action)
{
    return !action->done && action->failed;
}

void SAI_process_action(SAI_Action action, int player)
{
    short ret;

    assert(!action->done);

    ret = game_action(action->plyr_idx, action->gaction, action->a3,
        action->stl_x, action->stl_y, action->param1, action->param2);

    if (ret) {
        action->done = 1;
    }
    else if (action->failed < 0xff) {
        action->failed += 1;
    }
}

SAI_Action SAI_get_next_action(SAI_Action action)
{
    return action->next;
}

void SAI_append_action(SAI_Action * head, SAI_Action tail)
{
    if (*head) {
        for (;;) {
            if ((*head)->next) {
                *head = (*head)->next;
            }
            else {
                (*head)->next = tail;
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
    free(action);
}

SAI_Action SAI_mark_dig_action(int plyr, int x, int y)
{
    SAI_Action action;
    action = (SAI_Action) calloc(1, sizeof(*action));
    action->gaction = 14;
    action->plyr_idx = plyr;
    action->stl_x = TO_CENTER_STL(x);
    action->stl_y = TO_CENTER_STL(y);

    return action;
}

SAI_Action SAI_build_room_action(int plyr, int x, int y, int kind)
{
    SAI_Action action;
    action = (SAI_Action) calloc(1, sizeof(*action));
    action->gaction = 15;
    action->plyr_idx = plyr;
    action->stl_x = TO_CENTER_STL(x);
    action->stl_y = TO_CENTER_STL(y);
    action->param2 = kind;

    return action;
}
