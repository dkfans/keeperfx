/******************************************************************************/
// Dungeon Keeper fan extension.
/******************************************************************************/
/** @file bflib_coroutine.c
 *     "Improvised coroutine-like functions"
 * @par Purpose:
 *     Implementation
 * @author   KeeperFF Team
 * @date     01 Nov 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include <assert.h>

#include "bflib_basics.h"
#include "bflib_coroutine.h"

// add a new coroutine to the list
void coroutine_add(CoroutineLoop *context, CoroutineFn fn)
{
    assert(context->write_idx < COROUTINE_MAX_NUM);
    context->fns[context->write_idx++] = fn;
}

void coroutine_add_args(CoroutineLoop *context, CoroutineFn fn, int args[COROUTINE_ARGS])
{
    assert(context->write_idx < COROUTINE_MAX_NUM);
    context->fns[context->write_idx] = fn;
    memcpy(&context->args[context->write_idx * COROUTINE_ARGS], args, COROUTINE_ARGS * sizeof(int));
    context->write_idx++;
}

// exec all coroutines from the list
void coroutine_process(CoroutineLoop *context)
{
    context->read_idx = 0;
    CoroutineFn fn = context->fns[context->read_idx];
    while (fn)
    {
        CoroutineLoopState ret = fn(context);
        if (ret == CLS_CONTINUE)
        {
            context->fns[context->read_idx] = 0;
            context->read_idx++;
            fn = context->fns[context->read_idx];
        }
        else if (ret == CLS_ABORT)
        {
            context->write_idx = 0;
            context->read_idx = 0;
            return;
        }
        else if (ret == CLS_RETURN)
        {
            return;
        }
    }
    context->write_idx = 0;
    context->read_idx = 0;
}

int *coroutine_args(CoroutineLoop *context)
{
    return &context->args[context->read_idx * COROUTINE_ARGS];
}

void coroutine_clear(CoroutineLoop *context, TbBool error)
{
    for (int i = 0; i < context->write_idx; i++)
    {
        context->fns[i] = 0;
    }
    context->write_idx = 0;
    context->error |= error;
}