/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file event_monitoring.h
 *     Header file for event_monitoring.c
 * @par Purpose:
 *     Implement remote logging of game events for advanced testing
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Sim
 * @date     16 Jul 2020 - 16 Jul 2020
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_EVENT_MON_H
#define DK_EVENT_MON_H

#ifdef __cplusplus
extern "C" {
#endif

extern void evm_init(char *hostport, int client_no);
extern void evm_done();
extern void evm_stat(int force_new, const char *event_fmt, ...);

#ifdef __cplusplus
}
#endif

#endif