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
  virtual ~IPXServiceProvider() override;
  virtual TbError Start(struct TbNetworkSessionNameEntry *, char *, void *) override;
  virtual TbError Start(char *, char *, unsigned long, void *) override;
  virtual TbError Stop(void) override;
  virtual TbError Enumerate(TbNetworkCallbackFunc, void *) override;
  virtual TbError Enumerate(struct TbNetworkSessionNameEntry *, TbNetworkCallbackFunc, void *) override;
  virtual TbError Init(struct ReceiveCallbacks *, void *) override;
  virtual TbError Release(void) override;
  virtual TbError ChangeSettings(unsigned long, void *) override;
  virtual TbError EnableNewPlayers(TbBool allow) override;
  virtual bool ReadMessage(unsigned long *, void *, unsigned long *) override;
  virtual bool PeekMessage(unsigned long *, void *, unsigned long *) override;
  virtual TbError SendMessage(unsigned long, void *, unsigned char) override;
  virtual void update() override;
};

/******************************************************************************/

/******************************************************************************/
#endif
