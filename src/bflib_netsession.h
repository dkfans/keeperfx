/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsession.h
 *     Header file for bflib_netsession.c.
 * @par Purpose:
 *     Algorithms and data structures for network sessions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     09 Oct 2010 - 12 Oct 2014
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_NETSESSION_H
#define BFLIB_NETSESSION_H
#include "bflib_basics.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define NETSP_PLAYERS_COUNT 32
#define SESSION_ENTRIES_COUNT 32
#define SESSION_NAME_MAX_LEN  32
#define NETSP_PLAYER_NAME_MAX_LEN 32

enum NetMsgType
{
    NETMSGTYPE_MULTIPLAYER      = 0,
    NETMSGTYPE_ADD              = 1,
    NETMSGTYPE_DELETE           = 2,
    NETMSGTYPE_PROBABLYHOST     = 3, //might be incorrect
    NETMSGTYPE_SYSUSER          = 4,
    NETMSGTYPE_MPREQEXDATA      = 5,
    NETMSGTYPE_MPREQCOMPEXDATA  = 6,
    NETMSGTYPE_UNIDIRECTIONAL   = 7,
    NETMSGTYPE_UNKNOWN          = 8
};

struct TbNetworkSessionNameEntry {
    unsigned char joinable; //possibly active or selected is better name
    unsigned long id;
    unsigned char in_use;
    unsigned char valid_ping;
    unsigned char is_message;
    unsigned char is_invalid_version; // TODO pack them into flag
    char text[SESSION_NAME_MAX_LEN];
    char ip_port[20];
    TbClockMSec latency_time;
};

struct TbNetworkPlayerEntry {
  unsigned char field_0;
  unsigned long id;
  unsigned long field_5;
  unsigned long field_9;
  char name[32];
};

struct ReceiveCallbacks {
  void (*addMsg)(unsigned long, char *, void *);
  void (*deleteMsg)(unsigned long, void *);
  void (*hostMsg)(unsigned long, void *);
  void (*sysMsg)(void *);
  void *(*multiPlayer)(unsigned long, unsigned long, unsigned long, void *);
  void (*mpReqExDataMsg)(unsigned long, unsigned long, void *);
  void (*mpReqCompsExDataMsg)(unsigned long, unsigned long, void *);
  void *(*unidirectionalMsg)(unsigned long, unsigned long, void *);
  void (*systemUserMsg)(unsigned long, void *, unsigned long, void *);
  void *(*field_24)(unsigned long, void *);
};
/******************************************************************************/
void net_copy_name_string(char *dst,const char *src,long max_len);
/******************************************************************************/
#ifdef __cplusplus
};
#endif

#endif //BFLIB_NETSESSION_H

