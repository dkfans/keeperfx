/******************************************************************************/
// Dungeon Keeper fan extension.
/******************************************************************************/
/** @file bflib_coroutine.h
 *     "Improvised coroutine-like functions"
 * @par Purpose:
 *     Header file.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFF Team
 * @date     01 Nov 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_COROUTINE_H
#define BFLIB_COROUTINE_H

#ifdef __cplusplus
extern "C" {
#endif

#define COROUTINE_MAX_NUM 8
#define COROUTINE_ARGS 2
#define COROUTINE_VARS 2

/*
 * It is a list of functions with some common state (args)
 *
 * Maybe it is not a coroutine but whole idea is to split functions into continuable parts within working main loop
 */
struct CoroutineLoopS;

typedef enum CoroutineLoopStateS
{
    CLS_ABORT,
    CLS_REPEAT,
    CLS_CONTINUE,
    CLS_RETURN,
    CLS_ERROR,
} CoroutineLoopState;

typedef CoroutineLoopState (*CoroutineFn)(struct CoroutineLoopS *loop_context);

typedef struct CoroutineLoopS
{
    int         read_idx;
    int         write_idx;
    CoroutineFn fns[COROUTINE_MAX_NUM];
    intptr_t    args[COROUTINE_MAX_NUM * COROUTINE_ARGS];
    void*       guard_1;
    intptr_t    vars[COROUTINE_VARS];
    void*       guard_2;
    TbBool      error;
} CoroutineLoop;

// add a new coroutine to the list
extern void coroutine_add(CoroutineLoop *context, CoroutineFn fn);
// add a new coroutine to the list with args
extern void coroutine_add_args(CoroutineLoop *context, CoroutineFn fn, int args[COROUTINE_ARGS]);
// Init coroutine state
extern void coroutine_reset(CoroutineLoop *context);
// exec all coroutines from the list
extern void coroutine_process(CoroutineLoop *context);

extern intptr_t *coroutine_args(CoroutineLoop *context);
extern intptr_t *coroutine_vars(CoroutineLoop *context);

#ifdef __cplusplus
}
#endif

#endif
