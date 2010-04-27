/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
/** @file bflib_netsp_ipx.hpp
 *     Header file for bflib_netsp_ipx.cpp.
 * @par Purpose:
 *     IPX Network ServiceProvider subclass declaration.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     08 Mar 2009 - 09 Aug 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef BFLIB_NETSPIPX_H
#define BFLIB_NETSPIPX_H

#include <basetyps.h>
#include "bflib_basics.h"

#include "globals.h"
#include "bflib_netsp.hpp"
/******************************************************************************/

class IPXServiceProvider : public ServiceProvider {
public:
  IPXServiceProvider();
  virtual ~IPXServiceProvider();
  virtual TbError Start(struct TbNetworkSessionNameEntry *, char *, void *);
  virtual TbError Start(char *, char *, unsigned long, void *);
  virtual TbError Stop(void);
  virtual TbError Enumerate(TbNetworkCallbackFunc, void *);
  virtual TbError Enumerate(struct TbNetworkSessionNameEntry *, TbNetworkCallbackFunc, void *);
  virtual TbError Init(struct _GUID, struct _GUID *, struct ReceiveCallbacks *, void *);
  virtual TbError Release(void);
  virtual TbError ChangeSettings(unsigned long, void *);
  virtual TbError EnableNewPlayers(TbBool allow);
  virtual bool ReadMessage(unsigned long *, void *, unsigned long *);
  virtual bool PeekMessage(unsigned long *, void *, unsigned long *);
  virtual TbError SendMessage(unsigned long, void *, unsigned char);
  virtual void update();
};

/******************************************************************************/

/******************************************************************************/
#endif
