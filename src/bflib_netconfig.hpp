/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netconfig.hpp
 *     Header file.
 * @par Purpose:
 *     Contains constants that configure UDP session host/listener and TCP
 *     client/server operations.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFX Team
 * @date     10 April 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef BFLIB_IPCONFIG_HPP
#define BFLIB_IPCONFIG_HPP

// C++ constants - internal linkage by default

const int LISTENER_PORT_NUMBER = 17777; // UDP listener
const int HOST_PORT_NUMBER = 17778;     // UDP host and TCP server
const int SESSION_HOST_PERIOD = 4000;   // 4 s
const int SESSION_LISTENER_PERIOD = 2000;

const char MSG_PREFIX[] = {'K', 'F', 'X'};
const char BROADCAST_PREFIX = 'B';
const char UPDATE_PREFIX = 'U';

#endif
