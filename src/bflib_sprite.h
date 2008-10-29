/******************************************************************************/
// Bullfrog Engine Emulation Library - for use to remake classic games like
// Syndicate Wars, Magic Carpet or Dungeon Keeper.
/******************************************************************************/
// Author:  Tomasz Lis
// Created: 12 Feb 2008

// Purpose:
//    Header file for bflib_sprite.c.

// Comment:
//   Just a header file - #defines, typedefs, function prototypes etc.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/
#ifndef BFLIB_SPRITE_H
#define BFLIB_SPRITE_H

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)
struct TbSprite {
        char *Data;
        unsigned char SWidth;
        unsigned char SHeight;
};
#pragma options align=reset

struct SpriteInfo {
        struct TbSprite *Pointer;
        struct TbSprite *EndPointer;
        char *PointerData;
        struct TbSprite *Font0;
        struct TbSprite *EndFont0;
        char *Font0Data;
        struct TbSprite *Font1;
        struct TbSprite *EndFont1;
        char *Font1Data;
        struct TbSprite *Font2;
        struct TbSprite *EndFont2;
        char *Font2Data;
        struct TbSprite *Font3;
        struct TbSprite *EndFont3;
        char *Font3Data;
        struct TbSprite *Font4;
        struct TbSprite *EndFont4;
        char *Font4Data;
        struct TbSprite *Font5;
        struct TbSprite *EndFont5;
        char *Font5Data;
        struct TbSprite *DeskIcons;
        struct TbSprite *EndDeskIcons;
        char *DeskIconData;
};

struct TbSetupSprite {
        struct TbSprite **Start;
        struct TbSprite **End;
        char **Data;
};

extern struct TbSetupSprite setup_sprites[];
extern struct SpriteInfo sprites;
extern char mouse_pointer_sprite;
extern char lang_selection;

#pragma pack()

/******************************************************************************/
int __fastcall LbSpriteSetup(TbSprite *start, const TbSprite *end, const char *data);
int __fastcall LbSpriteSetupAll(struct TbSetupSprite t_setup[]);
char __fastcall font_height(const unsigned char c);
unsigned long __fastcall my_string_width(const char *str);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
