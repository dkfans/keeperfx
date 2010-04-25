/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp_ipx.cpp
 *     IPX Network ServiceProvider subclass declaration.
 * @par Purpose:
 *     Defines ServiceProvider for IPX network.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     08 Mar 2009 - 09 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "bflib_netsp_ipx.hpp"

#include "bflib_basics.h"
#include "bflib_netsp.hpp"

/******************************************************************************/
/******************************************************************************/
// methods of IPXServiceProvider

IPXServiceProvider::IPXServiceProvider()
{
  //TODO NET
}

IPXServiceProvider::~IPXServiceProvider()
{
  //TODO NET
}

TbError IPXServiceProvider::Start(struct TbNetworkSessionNameEntry *nsname, char *a2, void *a3)
{
  //TODO NET
  return Lb_FAIL;
}

TbError IPXServiceProvider::Start(char *, char *, unsigned long, void *)
{
  //TODO NET
  return Lb_FAIL;
}

TbError IPXServiceProvider::Stop(void)
{
  //TODO NET
  return Lb_FAIL;
}

TbError IPXServiceProvider::Enumerate(TbNetworkCallbackFunc callback, void *buf)
{
  //TODO NET
  return Lb_FAIL;
}

TbError IPXServiceProvider::Enumerate(struct TbNetworkSessionNameEntry *nsname, TbNetworkCallbackFunc callback, void *buf)
{
  //TODO NET
  return Lb_FAIL;
}

TbError IPXServiceProvider::Init(struct _GUID guid, struct _GUID *guidp, struct ReceiveCallbacks *recv_cb, void *buf)
{
  //TODO NET
  return Lb_FAIL;
}

TbError IPXServiceProvider::Release(void)
{
  //TODO NET
  return Lb_FAIL;
}

TbError IPXServiceProvider::ChangeSettings(unsigned long, void *)
{
  return Lb_OK;
}

TbError IPXServiceProvider::EnableNewPlayers(TbBool allow)
{
  //TODO NET
  return Lb_FAIL;
}

bool IPXServiceProvider::ReadMessage(unsigned long *, void *, unsigned long *)
{
  //TODO NET
  return Lb_FAIL;
}

bool IPXServiceProvider::PeekMessage(unsigned long *, void *, unsigned long *)
{
  //TODO NET
  return Lb_FAIL;
}

TbError IPXServiceProvider::SendMessage(unsigned long, void *, unsigned char)
{
  //TODO NET
  return Lb_FAIL;
}

void IPXServiceProvider::tick()
{
}

/******************************************************************************/
