/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp_tcp.cpp
 *     TCP network ServiceProvider subclass declaration.
 * @par Purpose:
 *     Defines ServiceProvider for TCP network.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     10 April 2010 - ?
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#include "bflib_netsp_tcp.hpp"

TCPServiceProvider::TCPServiceProvider() {

}

TCPServiceProvider::~TCPServiceProvider() {

}

TbError TCPServiceProvider::Start(struct TbNetworkSessionNameEntry *, char *, void *) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Start(char *, char *, unsigned long, void *) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Stop(void) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Enumerate(TbNetworkCallbackFunc, void *) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Enumerate(struct TbNetworkSessionNameEntry *, TbNetworkCallbackFunc, void *) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Init(struct _GUID, struct _GUID *, struct ReceiveCallbacks *, void *) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::Release(void) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::ChangeSettings(unsigned long, void *) {
	return Lb_FAIL;
}

TbError TCPServiceProvider::EnableNewPlayers(TbBool allow) {
	return Lb_FAIL;
}

unsigned long TCPServiceProvider::ReadMessage(unsigned long *, void *, unsigned long *) {
	return 0;
}

unsigned long TCPServiceProvider::PeekMessage(unsigned long *, void *, unsigned long *) {
	return 0;
}

TbError TCPServiceProvider::SendMessage(unsigned long, void *, unsigned char) {
	return Lb_FAIL;
}
