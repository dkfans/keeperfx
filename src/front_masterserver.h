/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file front_masterserver.h
 *     Integration with masterserver
 * @author   KeeperFX Team
 * @date     11 Mar 2010 - 19 Jun 2023
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef GIT_FRONT_MASTERSERVER_H
#define GIT_FRONT_MASTERSERVER_H

void send_json_to_masterserver(char *buf, VALUE *out);
void masterserver_send_update();
void masterserver_fetch_sessions();

#define MASTERSERVER_STATUS_OPEN "open"

#define MASTERSERVER_KEEPALIVE_TIME 5000

#define VALUE_GET_KEY(key) \
    val = value_dict_get(ret, key); \
    if (val == NULL) \
    { \
        goto unable; \
    }

#endif //GIT_FRONT_MASTERSERVER_H
