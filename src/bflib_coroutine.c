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
#include "pre_inc.h"
#include <assert.h>

#include "bflib_basics.h"
#include "bflib_coroutine.h"
#include <string.h>
#include "post_inc.h"

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
    memcpy(&context->args[context->write_idx * COROUTINE_ARGS], args, COROUTINE_ARGS * sizeof(intptr_t));
    context->write_idx++;
}

// exec all coroutines from the list starting from first
void coroutine_process(CoroutineLoop *context)
{
    context->read_idx = 0;
    memset(context->vars, 0, sizeof(context->vars));

    CoroutineFn fn = context->fns[context->read_idx];
    if ((context->guard_2 != &context->guard_2) || (context->guard_1 != &context->guard_1))
    {
        LbSyncLog("Coroutine damaged");
        context->guard_1 = &context->guard_1;
        context->guard_2 = &context->guard_2;
    }
    while (fn)
    {
        CoroutineLoopState ret = fn(context);
        if ((context->guard_2 != &context->guard_2) || (context->guard_1 != &context->guard_1))
        {
            LbSyncLog("Coroutine damaged 2");
            context->guard_1 = &context->guard_1;
            context->guard_2 = &context->guard_2;
        }

        if (ret == CLS_CONTINUE)
        {
            context->fns[context->read_idx] = 0;
            context->read_idx++;
            fn = context->fns[context->read_idx];
            memset(context->vars, 0, sizeof(context->vars));
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
        else if (ret == CLS_ERROR)
        {
            context->error = 1;
            context->write_idx = 0;
            context->read_idx = 0;
            return;
        }
    }
    context->write_idx = 0;
    context->read_idx = 0;
}

intptr_t *coroutine_args(CoroutineLoop *context)
{
    return &context->args[context->read_idx * COROUTINE_ARGS];
}

intptr_t *coroutine_vars(CoroutineLoop *context)
{
    return context->vars;
}

void coroutine_reset(CoroutineLoop *context)
{
    memset(context, 0, sizeof(CoroutineLoop));
    context->guard_1 = &context->guard_1;
    context->guard_2 = &context->guard_2;
}
