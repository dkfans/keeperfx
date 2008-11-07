/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 16 Nov 2008

// Purpose:
//    Header file for bflib_drawcls.c.

// Comment:
//   Just a header file - #defines, typedefs, function prototypes etc.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#ifndef BFLIB_DRAWCLS_H
#define BFLIB_DRAWCLS_H

#include "bflib_basics.h"

#pragma pack(1)

/******************************************************************************/
// Exported classes

//TODO: mark the pure virtual methods
class TDDrawBaseClass {
    TDDrawBaseClass(void);
    virtual ~TDDrawBaseClass(void);
    // Methods
    virtual int setup_window(void);
    virtual long CALLBACK WndProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam);
    virtual void find_video_modes(void);
    virtual int get_palette(void *,unsigned long,unsigned long);
    virtual int set_palette(void *,unsigned long,unsigned long);
    virtual int setup_screen(TbScreenMode);
    virtual long lock_screen(void);
    virtual long unlock_screen(void);
    virtual long clear_screen(unsigned long);
    virtual long clear_window(long,long,unsigned long,unsigned long,unsigned long);
    virtual int swap_screen(void);
    virtual int reset_screen(void);
    virtual long restore_surfaces(void);
    virtual void wait_vbi(void);
    virtual long swap_box(tagPOINT,tagRECT &);
    virtual long create_surface(_TSurface *,unsigned long,unsigned long);
    virtual long release_surface(_TSurface *);
    virtual long blt_surface(_TSurface *,unsigned long,unsigned long,tagRECT *,unsigned long);
    virtual long lock_surface(_TSurface *);
    virtual long unlock_surface(_TSurface *);
    virtual void LoresEmulation(int);
    virtual int unkn01();
    virtual int unkn02();
    virtual int unkn03();
    virtual int unkn04();
    int set_double_buffering_video(int);
    int set_wscreen_in_video(int);
    int IsActive(void);
    void SetIcon(void);
    // Properties
    unsigned int unkvar4;
    char *textn2;
    char *textname;
    unsigned int flags;
    unsigned int numfield_14;
    unsigned int numfield_18;
    unsigned int numfield_1C;
    };

class TDDrawSdk {
    TDDrawSdk(void);
    virtual ~TDDrawSdk(void);
    // Methods
    int setup_window(void);
    long CALLBACK WndProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam);
    void find_video_modes(void);
    int get_palette(void *,unsigned long,unsigned long);
    int set_palette(void *,unsigned long,unsigned long);
    int setup_screen(TbScreenMode);
    long lock_screen(void);
    long unlock_screen(void);
    long clear_screen(unsigned long);
    long clear_window(long,long,unsigned long,unsigned long,unsigned long);
    int swap_screen(void);
    int reset_screen(void);
    long restore_surfaces(void);
    void wait_vbi(void);
    long swap_box(tagPOINT,tagRECT &);
    long create_surface(_TSurface *,unsigned long,unsigned long);
    long release_surface(_TSurface *);
    long blt_surface(_TSurface *,unsigned long,unsigned long,tagRECT *,unsigned long);
    long lock_surface(_TSurface *);
    long unlock_surface(_TSurface *);
    void LoresEmulation(int);
    int unkn01();
    int unkn02();
    int unkn03();
    int unkn04();
    // Properties
    char unkn[376];
    };


/*
  Name:  W?backup$:.0$:?LoresEmulation$n(i)v:TDDrawSdk$n$TbDisplayStruct$$
    address      = 0002:00282440
    module index = 227
    kind:          (static pubdef) (data)
  Name:  W?w$:.0$:?LoresEmulation$n(i)v:TDDrawSdk$nul
    address      = 0002:002824B8
    module index = 227
    kind:          (static pubdef) (data)
  Name:  W?h$:.0$:?LoresEmulation$n(i)v:TDDrawSdk$nul
    address      = 0002:002824BC
    module index = 227
    kind:          (static pubdef) (data)
  Name:  W?set$:.0$:?LoresEmulation$n(i)v:TDDrawSdk$ni
    address      = 0002:002824C0
    module index = 227
    kind:          (static pubdef) (data)
  Name:  W?$Wvf0dos:TDDrawSdk$$nx[]pn()v
    address      = 0002:000118E0
    module index = 227
    kind:          (data)
  Name:  W?SendDDMsg$:TDDrawSdk$n(ipnv)v
    address      = 0001:000DA350
    module index = 227
    kind:          (code)
  Name:  W?$ct:TDDrawSdk$n()_
    address      = 0001:000DA3FC
    module index = 227
    kind:          (code)
  Name:  W?$dt:TDDrawSdk$n()_
    address      = 0001:000DA498
    module index = 227
    kind:          (code)
  Name:  W?find_video_modes$:TDDrawSdk$n()v
    address      = 0001:000DA544
    module index = 227
    kind:          (code)
  Name:  W?setup_direct_draw$:TDDrawSdk$n()i
    address      = 0001:000DA58C
    module index = 227
    kind:          (code)
  Name:  W?reset_direct_draw$:TDDrawSdk$n()i
    address      = 0001:000DA688
    module index = 227
    kind:          (code)
  Name:  W?setup_dds_double_video$:TDDrawSdk$n()i
    address      = 0001:000DA718
    module index = 227
    kind:          (code)
  Name:  W?setup_dds_single_video$:TDDrawSdk$n()i
    address      = 0001:000DA854
    module index = 227
    kind:          (code)
  Name:  W?setup_dds_system$:TDDrawSdk$n()i
    address      = 0001:000DA930
    module index = 227
    kind:          (code)
  Name:  W?setup_surfaces$:TDDrawSdk$n(sss)i
    address      = 0001:000DA9A4
    module index = 227
    kind:          (code)
  Name:  W?release_surfaces$:TDDrawSdk$n()i
    address      = 0001:000DAA98
    module index = 227
    kind:          (code)
  Name:  W?set_palette$:TDDrawSdk$n(pnvulul)i
    address      = 0001:000DAAF8
    module index = 227
    kind:          (code)
  Name:  W?get_palette$:TDDrawSdk$n(pnvulul)i
    address      = 0001:000DAC6C
    module index = 227
    kind:          (code)
  Name:  W?release_palettes$:TDDrawSdk$n()i
    address      = 0001:000DAD30
    module index = 227
    kind:          (code)
  Name:  W?setup_screen$:TDDrawSdk$n($TbScreenMode$$)i
    address      = 0001:000DAD74
    module index = 227
    kind:          (code)
  Name:  W?restore_surfaces$:TDDrawSdk$n()l
    address      = 0001:000DB080
    module index = 227
    kind:          (code)
  Name:  W?wscreen_surface$:TDDrawSdk$n()pn$IDirectDrawSurface$$
    address      = 0001:000DB0F0
    module index = 227
    kind:          (code)
  Name:  W?lock_screen$:TDDrawSdk$n()l
    address      = 0001:000DB124
    module index = 227
    kind:          (code)
  Name:  W?unlock_screen$:TDDrawSdk$n()l
    address      = 0001:000DB1F0
    module index = 227
    kind:          (code)
  Name:  W?clear_screen$:TDDrawSdk$n(ul)l
    address      = 0001:000DB30C
    module index = 227
    kind:          (code)
  Name:  W?clear_window$:TDDrawSdk$n(llululul)l
    address      = 0001:000DB3D0
    module index = 227
    kind:          (code)
  Name:  W?swap_box$:TDDrawSdk$n($tagPOINT$$rn$tagRECT$$)l
    address      = 0001:000DB490
    module index = 227
    kind:          (code)
  Name:  W?LoresEmulation$:TDDrawSdk$n(i)v
    address      = 0001:000DB514
    module index = 227
    kind:          (code)
  Name:  W?swap_screen$:TDDrawSdk$n()i
    address      = 0001:000DB698
    module index = 227
    kind:          (code)
  Name:  W?reset_screen$:TDDrawSdk$n()i
    address      = 0001:000DB864
    module index = 227
    kind:          (code)
  Name:  W?wait_vbi$:TDDrawSdk$n()v
    address      = 0001:000DB8A8
    module index = 227
    kind:          (code)
  Name:  W?initFail$:TDDrawSdk$n(pna)i
    address      = 0001:000DB8D4
    module index = 227
    kind:          (code)
  Name:  W?WindowProc$:TDDrawSdk$n(pnvuiuil)ul
    address      = 0001:000DB8E8
    module index = 227
    kind:          (code)
  Name:  W?sdk_window_thread$:TDDrawSdk$n(ul)ul
    address      = 0001:000DBB18
    module index = 227
    kind:          (code)
  Name:  W?setup_window$:TDDrawSdk$n()l
    address      = 0001:000DBB70
    module index = 227
    kind:          (code)
  Name:  W?create_sdk_window$:TDDrawSdk$n()l
    address      = 0001:000DBBCC
    module index = 227
    kind:          (code)
  Name:  W?remove_sdk_window$:TDDrawSdk$n()l
    address      = 0001:000DBC58
    module index = 227
    kind:          (code)
  Name:  W?create_surface$:TDDrawSdk$n(pn$_TSurface$$ulul)l
    address      = 0001:000DBD00
    module index = 227
    kind:          (code)
  Name:  W?release_surface$:TDDrawSdk$n(pn$_TSurface$$)l
    address      = 0001:000DBDAC
    module index = 227
    kind:          (code)
  Name:  W?blt_surface$:TDDrawSdk$n(pn$_TSurface$$ululpn$tagRECT$$ul)l
    address      = 0001:000DBDC8
    module index = 227
    kind:          (code)
  Name:  W?lock_surface$:TDDrawSdk$n(pn$_TSurface$$)l
    address      = 0001:000DBE80
    module index = 227
    kind:          (code)
  Name:  W?unlock_surface$:TDDrawSdk$n(pn$_TSurface$$)l
    address      = 0001:000DBEC4
    module index = 227
    kind:          (code)
  Name:  W?$Wts0dn$TDDrawSdk$$$nx[]uc
    address      = 0002:000118CC
    module index = 227
    kind:          (data)
  Name:  W?$Wsi0grect$n$tagRECT$$$:.7$:?swap_screen$n()i:TDDrawSdk$nx$tagRECT$$
    address      = 0002:00031AA4
    module index = 227
    kind:          (static pubdef) (data)
  Name:  W?$Wsi0grect$n$tagRECT$$$:.j$:?swap_screen$n()i:TDDrawSdk$nx$tagRECT$$
    address      = 0002:00031AB4
    module index = 227
    kind:          (static pubdef) (data)
  Name:  W?$Wsi0hdrect$n$tagRECT$$$:.j$:?swap_screen$n()i:TDDrawSdk$nx$tagRECT$$
    address      = 0002:00031AC4
    module index = 227
    kind:          (static pubdef) (data)
  Name:  W?$Wsi0dr$n$tagRECT$$$:.c$:?blt_surface$n(pn$_TSurface$$ululpn$tagRECT$$ul)l:TDDrawSdk$nx$3$
    address      = 0002:002824D4
    module index = 227
    kind:          (static pubdef) (data)

*/

#pragma pack()


/******************************************************************************/

#endif
