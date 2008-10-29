/******************************************************************************/
// keeperfx.h - Dungeon Keeper fan extension.
/******************************************************************************/
// Author:   Tomasz Lis
// Created:  27 May 2008

// Purpose:
//   Header file. Defines exported routines from keeperfx.dll

// Comment:
//   None.

//Copying and copyrights:
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation; either version 2 of the License, or
//   (at your option) any later version.
/******************************************************************************/

#ifndef DK_KEEPERFX_H
#define DK_KEEPERFX_H

#include <windows.h>

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_video.h"

#define LEGAL_WIDTH  640
#define LEGAL_HEIGHT 480
#define PALETTE_SIZE 768
#define STRINGS_MAX  941

#define SIZEOF_TDDrawSdk 408

// Temporary
typedef struct SSurface {
       char a;
       } TSurface;

enum TbErrorCode {
        Lb_OK      =  0,
        Lb_FAIL    = -1,
};


#pragma pack(1)

//This seems to be array of functions in __thiscall convention
// they are all methods for class TDDrawBase and its derivants
// Note: May be incorrectly named - the Beta version was used to get them!
typedef struct {
    void __fastcall (*dt)(void *ths); // +0 TDDrawSdk::TDDrawSdk(void)
    long __fastcall (*setup_window)(void *ths); // +4 int setup_window(void);
    long CALLBACK (*WindowProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); // +8 unsigned int TDDrawSdk::WindowProc(void *,unsigned int,unsigned int,long);
    void __fastcall (*find_video_modes)(void *ths); // +12 void TDDrawSdk::find_video_modes(void);
    int (*get_palette)(void *,unsigned long,unsigned long); // +16 int TDDrawSdk::get_palette(void *,unsigned long,unsigned long);
    int (*set_palette)(void *,unsigned long,unsigned long); // +20 int TDDrawSdk::set_palette(void *,unsigned long,unsigned long);
    int (*setup_screen)(TbScreenMode); // +24 int TDDrawSdk::setup_screen(TbScreenMode);
    long __fastcall (*lock_screen)(void *ths); // +28 long TDDrawSdk::lock_screen(void);
    long __fastcall (*unlock_screen)(void *ths); // +32 long TDDrawSdk::unlock_screen(void);
    long (*clear_screen)(unsigned long); // +36 long TDDrawSdk::clear_screen(unsigned long);
    long (*clear_window)(long,long,unsigned long,unsigned long,unsigned long); // +40 long TDDrawSdk::clear_window(long,long,unsigned long,unsigned long,unsigned long);
    int __fastcall (*swap_screen)(void *ths); //+44 int TDDrawSdk::swap_screen(void);
    int __fastcall (*reset_screen)(void *ths); // +48 int TDDrawSdk::reset_screen(void);
    long __fastcall (*restore_surfaces)(void *ths); // +52 long TDDrawSdk::restore_surfaces(void);
    void __fastcall (*wait_vbi)(void *ths); // +56 void TDDrawSdk::wait_vbi(void);
    long (*swap_box)(tagPOINT,tagRECT &); // +60 long TDDrawSdk::swap_box(tagPOINT,tagRECT &);
    long (*create_surface)(TSurface *,unsigned long,unsigned long); // +64 long TDDrawSdk::create_surface(_TSurface *,unsigned long,unsigned long);
    long (*release_surface)(TSurface *); // +68 long TDDrawSdk::release_surface(_TSurface *);
    long (*blt_surface)(TSurface *,unsigned long,unsigned long,tagRECT *,unsigned long); // +72 long TDDrawSdk::blt_surface(_TSurface *,unsigned long,unsigned long,tagRECT *,unsigned long);
    long (*lock_surface)(TSurface *); // +76 long TDDrawSdk::lock_surface(_TSurface *);
    long (*unlock_surface)(TSurface *); // +80 long TDDrawSdk::unlock_surface(_TSurface *);
    void (*LoresEmulation)(int); // +84 void TDDrawSdk::LoresEmulation(int);
    void *unkn[4];
       } TDDrawBaseVTable;

typedef struct DDrawBaseClass {
       TDDrawBaseVTable *vtable;
       unsigned int unkvar4;
       char *textn2;
       char *textname;
       unsigned int flags;
       unsigned int numfield_14;
       unsigned int numfield_18;
       unsigned int numfield_1C;
       } TDDrawBaseClass;

typedef struct DDrawSdk {
       TDDrawBaseVTable *vtable;
       // From base class
       unsigned int unkvar4;
       char *textn2;
       char *textname;
       unsigned int flags;
       unsigned int numfield_14;
       unsigned int numfield_18;
       unsigned int numfield_1C;
       // The rest
       char unkn[376];
       } TDDrawSdk;


/*
class TDDrawBaseClass {
    TDDrawBaseClass(void);
    ~TDDrawBaseClass(void);
    long CALLBACK WndProc(HWND hWnd, unsigned int message, WPARAM wParam, LPARAM lParam);
    int set_double_buffering_video(int);
    int set_wscreen_in_video(int);
    int IsActive(void);
    void SetIcon(void);
    void LoresEmulation(int);

  Name:  W?$Wvf0jos:TDDrawBaseClass$$nx[]pn()v
  void (*const operator`__vftbl'[])(void)
    address      = 0002:00011874
    kind:          (data)

       };
*/

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

struct GameSettings {
    unsigned char field_0;
    unsigned char video_shadows;
    unsigned char view_distance;
    unsigned char field_3;
    unsigned char video_textures;
    unsigned char video_cluedo_mode;
    unsigned char sound_volume;
    unsigned char redbook_volume;
    unsigned char field_8;
    unsigned short gamma_correction;
    int field_B;
    unsigned char field_F[8];
    unsigned char field_17[8];
    unsigned char field_1F[8];
    unsigned char field_27[8];
    unsigned char field_2F[8];
    unsigned char field_37[8];
    unsigned char field_3F[8];
    unsigned char field_47[8];
    unsigned char field_4F;
    unsigned char field_50;
    unsigned char field_51;
    };

struct PlayerInfo {
    unsigned char field_0[3];
    unsigned char field_3;
    unsigned char field_4;
    unsigned char field_5;
    unsigned char field_6;
    unsigned char field_7[34];
    unsigned char field_29;
    unsigned char field_2A;
    unsigned char field_2B;
    unsigned char field_2C;
    unsigned char field_2D[1047];
    short field_444;
    short field_446;
    short field_448;
    short field_44A;
    unsigned char field_44C[159];
    long field_4EB;
    };

struct Dungeon {
    int field_0;
    int field_4;
    unsigned char field_8;
    unsigned char field_9;
    unsigned char field_A;
    int field_B;
    short field_F;
    short field_11;
    short field_13;
    int field_15;
    short field_19;
    short field_1B;
    short field_1D;
    short field_1F;
    short field_21;
    short field_23;
    short field_25;
    short field_27;
    int field_29;
    short field_2D;
    short field_2F;
    short field_31;
    int field_33;
    int field_37;
    int field_3B;
    int field_3F;
    short field_43;
    int field_45;
    int field_49;
    int field_4D;
    short field_51;
    short field_53;
    int field_55;
    int field_59;
    int field_5D;
    short field_61;
    unsigned char field_63;
    short field_64[480];
    short field_424[96];
    short field_4E4[111];
    int field_5C2[4];
    short field_5D2;
    int field_5D4;
    short field_5D8;
    unsigned char field_5DA[679];
    unsigned char field_881;
    unsigned char field_882;
    unsigned char field_883;
    int field_884;
    int field_888;
    int field_88C[10];
    int field_8B4[8];
    unsigned char field_8D4;
    unsigned char field_8D5;
    unsigned char field_8D6[23];
    unsigned char field_8ED[43];
    unsigned char field_918;
    unsigned char field_919;
    unsigned char field_91A[32];
    unsigned char field_93A;
    short field_93B;
    short field_93D;
    short field_93F;
    int field_941;
    int field_945;
    short field_949;
    short field_94B[32];
    short field_98B;
    short field_98D[97];
    int field_A4F;
    unsigned char field_A53[124];
    short field_ACF;
    short field_AD1;
    short field_AD3;
    short field_AD5;
    short field_AD7;
    short field_AD9;
    short field_ADB;
    int field_ADD;
    int field_AE1;
    int field_AE5[4];
    short field_AF5;
    short field_AF7;
    int field_AF9;
    int field_AFD;
    short field_B01[5];
    int field_B0B[225];
    int field_E8F[4];
    unsigned char field_E9F;
    int field_EA0;
    int field_EA4;
    int field_EA8[52];
    int field_F78;
    unsigned char field_F7C;
    unsigned char field_F7D;
    unsigned char field_F7E;
    unsigned char field_F7F;
    unsigned char field_F80;
    unsigned char field_F81;
    unsigned char field_F82[256];
    unsigned char field_1082[256];
    unsigned char field_1182[128];
    unsigned char field_1202[53];
    long time1;
    long time2;
    int field_123F;
    int field_1243;
    int field_1247;
    int field_124B;
    int field_124F;
    int field_1253;
    int field_1257;
    int field_125B;
    int field_125F;
    unsigned char field_1263[31];
    unsigned char field_1282[128];
    unsigned char field_1302[5];
    int allow_save_score;
    unsigned long player_score;
    unsigned char field_130F[19];
    unsigned char field_1322[36];
    unsigned char field_1346[60];
    unsigned char field_1382[256];
    unsigned char field_1482[64];
    unsigned char field_14C2[64];
    short field_1502;
    int field_1504;
    };

#define SIZEOF_Game 0x151825

// only one such struct exists at .data:005F0310
// it ends at 00741B35
struct Game { // (sizeof=0x151825)
char numfield_0;
int numfield_1;
int numfield_5;
char numfield_9;
unsigned char numfield_A;
char align_B;
char numfield_C;
char numfield_D;
char flags_font;
char one_player;
char eastegg01_cntr;
char flags_cd;
char eastegg02_cntr;
char audiotrack;
char numfield_14;
char numfield_15;
int numfield_16;
char numfield_1A;
char numfield_1B;
struct PlayerInfo players[5];
short numfield_18c7;
char field_18C9;
short field_18CA;
char field_18CC[2868];
char field_2400[4096];
char field_3400[4096];
char field_4400[4096];
char field_5400[4096];
char field_6400[4096];
char field_7400[4096];
char field_8400[4096];
char field_9400[4096];
char field_A400[4096];
char field_B400[4096];
char field_C400[4096];
char field_D400[4096];
char field_E400[4096];
char field_F400[2993];
char field_FFB1[96];
char field_10011;
char field_10012;
char field_10013;
char field_10014;
char field_10015;
char field_10016;
char field_10017;
char field_10018;
int field_10019;
char field_1001D;
int field_1001E;
int field_10022;
char field_10026[1000000];
char field_104266[32768];
char field_10C266[32768];
char field_114266[32768];
char field_11C266[32768];
char field_124266[32768];
char field_12C266[32768];
char field_134266[32768];
char field_13C266[4096];
char field_13D266[4096];
char field_13E266[4096];
char field_13F266[4096];
char field_140266[4096];
char field_141266[4096];
char field_142266[4096];
char field_143266[631];
struct Dungeon dungeon[5];
char field_149E05[97];
int field_149E66;
int field_149E6A;
int field_149E6E;
char field_149E72[5];
int field_149E77;
char field_149E7B;
int field_149E7C;
char field_149E80;
char field_149E81;
char fname_unk1[150];
    char packet_fopened;
    TbFileHandle packet_fhandle;
int field_149F1D;
char field_149F21[5];
char field_149F26[14];
int numfield_149F34;
char numfield_149F38;
    char packet_checksum;
int numfield_149F3A;
int numfield_149F3E;
int numfield_149F42;
char numfield_149F46;
char numfield_149F47;
char numfield_149F48;
char align_149F49;
char field_149F4A[134];
char field_149FD0[22];
char field_149FE6[128];
char field_14A066[512];
char field_14A266[512];
char field_14A466[512];
char field_14A666[256];
char field_14A766[128];
char field_14A7E6[64];
char field_14A826[16];
char field_14A836[7];
unsigned char numfield_14A83D;
    unsigned char level_number; // change it to long ASAP
char field_14A83F[39];
char field_14A866[512];
char field_14AA66[2048];
char field_14B266[2048];
char field_14BA66[128];
char field_14BAE6[64];
char field_14BB26[28];
    unsigned long seedchk_random_used;
int field_14BB46;
int field_14BB4A;
int field_14BB4E;
short field_14BB52;
char field_14BB54;
int field_14BB55;
int field_14BB59;
int field_14BB5D;
    unsigned long time_delta;
short field_14BB65[592];
char field_14C005[609];
char field_14C266[4096];
char field_14D266[4096];
char field_14E266[1024];
char field_14E666[512];
char field_14E866[256];
char field_14E966[128];
char field_14E9E6[64];
char field_14EA26[32];
short field_14EA46;
short field_14EA48;
char flagfield_14EA4A;
char field_14EA4B;
char field_14EA4C[320];
char field_14EB8C[730];
char field_14EE66[1024];
char field_14F266[2800];
char field_14FD56[128];
char field_14FDD6[128];
char field_14FE56[128];
char field_14FED6[128];
char field_14FF56[128];
char field_14FFD6[128];
char field_150056[256];
char field_150156[256];
char field_150256[128];
char field_1502D6[64];
char field_150316[27];
char field_150331;
long timingvar1;
int field_150336;
int field_15033A;
int field_15033E;
    char no_intro;
int numfield_150343;
int numfield_150347;
char field_15034B[11];
char field_150356[32];
char field_150376[28];
    int num_fps;
int numfield_150396;
int numfield_15039A;
int numfield_15039E;
char numfield_1503A2;
char numfield_1503A3;
char field_1503A4[50];
char field_1503D6[64];
char field_150416[320];
char field_150556[512];
char field_150756[512];
char field_150956[512];
char field_150B56[512];
char field_150D56[1024];
char field_151156[1024];
char field_151556[128];
char field_1515D6[256];
char field_1516D6[256];
char field_1517D6[5];
char boolfield_1517DB;
char boolfield_1517DC;
char boolfield_1517DD;
char boolfield_1517DE;
char boolfield_1517DF;
char boolfield_1517E0;
  char is_full_moon;
int field_1517E2;
char field_1517E6;
char field_1517E7;
int field_1517E8;
int field_1517EC;
char field_1517F0[13];
int field_1517FD;
int field_151801;
int field_151805;
int field_151809;
    int chosen_spell_type;
    int chosen_spell_look;
    int chosen_spell_tooltip;
int numfield_151819;
int numfield_15181D;
int numfield_151821;
};

struct CPU_INFO {
int field_0;
int field_4;
int field_8;
int field_C;
int field_10;
int field_14;
char *field_18;
};

struct TbLoadFiles {
        char FName[28];
        unsigned char **Start;
        unsigned char **SEnd;
        unsigned long SLength;
        unsigned short Flags;
        unsigned short Spare;
};

struct CatalogueEntry {
        char used;
        char  numfield_1;
        char textname[15];
};

struct FrontEndButtonData {
        char field_0;
        char field_1;
        char field_2;
};

struct HighScore {
        long score;
        char name[64];
        long level;
};

typedef char * (*ModDL_Fname_Func)(struct TbLoadFiles *);
typedef long (*Gf_OptnBox_Callback)(struct GuiBox *, struct GuiBoxOption *, char, long *);

struct GuiBoxOption {
       char *label;
       unsigned char numfield_4;
       void *ofsfield_5;
       Gf_OptnBox_Callback callback;
       long field_D;
       long field_11;
       long field_15;
       long field_19;
       long field_1D;
       long field_21;
       short field_25;
};

#pragma pack()
 
#ifdef __cplusplus
extern "C" {
#endif

//Functions - exported by the DLL

//DLLIMPORT int __stdcall _DK_WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
//DLLIMPORT int __stdcall _DK_LbBullfrogMain(unsigned short argc,char **argv);
DLLIMPORT int __stdcall _DK_setup_game(void);
DLLIMPORT int __stdcall _DK_game_loop(void);
DLLIMPORT int __stdcall _DK_init_sound(void);
DLLIMPORT int __stdcall _DK_LbErrorLogSetup(char *directory, char *filename, unsigned char flag);
DLLIMPORT void __fastcall _DK_get_cpu_info(struct CPU_INFO *cpu_info);
DLLIMPORT int __stdcall _DK_SyncLog(char *Format, ...);
DLLIMPORT int __stdcall _DK_load_configuration(void);
DLLIMPORT int __cdecl _DK_LbDataLoadAll(struct TbLoadFiles *load_files);
DLLIMPORT int __cdecl _DK_LbScreenSetup(TbScreenMode mode, unsigned int width,
               unsigned int height, unsigned char *palette, int flag1, int flag2);
DLLIMPORT int __cdecl _DK_LbScreenClear(TbPixel colour);
//DLLIMPORT int __cdecl _DK_LbScreenSwap();
DLLIMPORT int __stdcall _DK_LbWindowsControl(void);
DLLIMPORT int __stdcall _DK_LbScreenLock(void);
DLLIMPORT int __stdcall _DK_LbScreenUnlock(void);
DLLIMPORT int __stdcall _DK_LbScreenSwap(void);
DLLIMPORT int __cdecl _DK_LbPaletteSet(unsigned char *palette);
DLLIMPORT int __cdecl _DK_LbPaletteGet(unsigned char *palette);
DLLIMPORT void __cdecl _DK_set_cpu_mode(int mode);
DLLIMPORT void __cdecl _DK_check_cd_in_drive(void);
DLLIMPORT void __cdecl _DK_display_loading_screen(void);
DLLIMPORT int __cdecl _DK_initial_setup(void);
DLLIMPORT int __cdecl _DK_setup_game_sound_heap(void);
DLLIMPORT int __cdecl _DK_load_settings(void);
DLLIMPORT void __cdecl _DK_init_creature_scores(void);
DLLIMPORT void __cdecl _DK_setup_3d(void);
DLLIMPORT void __cdecl _DK_setup_stuff(void);
DLLIMPORT void __cdecl _DK_init_lookups(void);
DLLIMPORT void __cdecl _DK_set_gamma(char, int);
DLLIMPORT void __cdecl _DK_update_mouse(void);
DLLIMPORT void __cdecl _DK_input_eastegg(void);
DLLIMPORT void __cdecl _DK_update(void);
DLLIMPORT void __cdecl _DK_wait_at_frontend(void);
DLLIMPORT void __cdecl _DK_set_player_instance(struct PlayerInfo *playerinf, long, int);
DLLIMPORT void __cdecl _DK_clear_mapwho(void);
DLLIMPORT void __cdecl _DK_delete_all_structures(void);
DLLIMPORT void __cdecl _DK_frontstats_initialise(void);
DLLIMPORT int __cdecl _DK_LbMouseChangeSpriteAndHotspot(TbSprite *spr, int, int);
DLLIMPORT int __cdecl _DK_PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *palette);
DLLIMPORT void __cdecl _DK_initialise_eye_lenses(void);
DLLIMPORT void __cdecl _DK_reset_eye_lenses(void);
DLLIMPORT void __cdecl _DK_setup_eye_lens(long);
DLLIMPORT int __cdecl _DK_LbPaletteFade(unsigned char *pal, long n, TbPaletteFadeFlag flg);
DLLIMPORT int __cdecl _DK_LbMouseChangeSprite(long);

DLLIMPORT void __stdcall _DK_IsRunningMark(void);
DLLIMPORT void __stdcall _DK_IsRunningUnmark(void);
DLLIMPORT int __stdcall _DK_LbMouseSuspend(void);
DLLIMPORT int __stdcall _DK_LbScreenReset(void);
DLLIMPORT int __stdcall _DK_LbDataFreeAll(struct TbLoadFiles *load_files);
DLLIMPORT int __stdcall _DK_LbMemoryFree(void *buffer);
DLLIMPORT int __stdcall _DK_LbMemoryReset(void);
DLLIMPORT int __stdcall _DK_play_smk_(char *fname, int smkflags, int plyflags);
DLLIMPORT int __fastcall _DK_LbFileClose(TbFileHandle handle);
DLLIMPORT int __cdecl _DK_LbTextDraw(int posx, int posy, const char *text);
DLLIMPORT int __cdecl _DK_process_sound_heap(void);
DLLIMPORT void __cdecl _DK_setup_engine_window(long, long, long, long);
DLLIMPORT void __cdecl _DK_redraw_display(void);
DLLIMPORT void __cdecl _DK_cumulative_screen_shot(void);
DLLIMPORT long __cdecl _DK_anim_record_frame(unsigned char *screenbuf, unsigned char *palette);
DLLIMPORT int __stdcall _DK_LbIsActive(void);
DLLIMPORT void __cdecl _DK_input(void);
DLLIMPORT void __cdecl _DK_load_game_save_catalogue(CatalogueEntry *catalg);
DLLIMPORT long __cdecl _DK_frontmap_update(void);
DLLIMPORT void __cdecl _DK_frontstats_initialise(void);
DLLIMPORT long __cdecl _DK_frontnetmap_update(void);
DLLIMPORT void __cdecl _DK_frontend_set_state(long);
DLLIMPORT void __cdecl _DK_frontnet_service_update(void);
DLLIMPORT void __cdecl _DK_frontnet_session_update(void);
DLLIMPORT void __cdecl _DK_frontnet_session_update(void);
DLLIMPORT void __cdecl _DK_frontnet_start_update(void);
DLLIMPORT void __cdecl _DK_frontnet_modem_update(void);
DLLIMPORT void __cdecl _DK_frontnet_serial_update(void);
DLLIMPORT void __cdecl _DK_frontstats_update(void);
DLLIMPORT void __cdecl _DK_fronttorture_update(void);
//DLLIMPORT void * __cdecl _DK_frontnet_session_join(GuiButton); (may be incorrect)
DLLIMPORT  int __cdecl _DK_set_game_key(long key_id, unsigned char key, int shift_state, int ctrl_state);
DLLIMPORT void __cdecl _DK_frontmap_input(void);
DLLIMPORT void __cdecl _DK_frontnetmap_input(void);
DLLIMPORT void __cdecl _DK_fronttorture_input(void);
DLLIMPORT long __cdecl _DK_GetMouseY(void);
DLLIMPORT int __cdecl _DK_get_gui_inputs(int);
DLLIMPORT void __cdecl _DK_frontnet_start_input(void);
DLLIMPORT void __cdecl _DK_frontend_high_score_table_input(void);
DLLIMPORT int __cdecl _DK_play_smacker_file(char *fname, int);
DLLIMPORT int __cdecl _DK_LbTextSetWindow(int, int, int, int);
DLLIMPORT int __cdecl _DK_draw_text_box(char *text);
DLLIMPORT void __cdecl _DK_demo(void);
DLLIMPORT void __cdecl _DK_draw_gui(void);
DLLIMPORT void __cdecl _DK_frontmap_draw(void);
DLLIMPORT void __cdecl _DK_frontcredits_draw(void);
DLLIMPORT void __cdecl _DK_fronttorture_draw(void);
DLLIMPORT void __cdecl _DK_frontnetmap_draw(void);
DLLIMPORT int __cdecl _DK_setup_screen_mode(short nmode);
DLLIMPORT int __cdecl _DK_setup_screen_mode_minimal(short nmode);
DLLIMPORT void __cdecl _DK_save_settings(void);
DLLIMPORT void __cdecl _DK_startup_saved_packet_game(void);
DLLIMPORT void __cdecl _DK_startup_network_game(void);
DLLIMPORT int __cdecl _DK_frontend_load_data(void);
DLLIMPORT int __cdecl _DK_setup_network_service(int srvidx);
DLLIMPORT int __cdecl _DK_load_game(long);
DLLIMPORT int __cdecl _DK_process_3d_sounds(void);
DLLIMPORT int __cdecl _DK_LbMouseSetPosition(int, int);
DLLIMPORT void _DK_frontmap_unload(void);
DLLIMPORT void _DK_turn_off_menu(char);
DLLIMPORT void _DK_turn_on_menu(char);
DLLIMPORT void _DK_init_gui(void);
DLLIMPORT void _DK_frontnet_serial_reset(void);
DLLIMPORT void _DK_frontnet_modem_reset(void);
DLLIMPORT void _DK_fronttorture_unload(void);
DLLIMPORT void _DK_fronttorture_load(void);
DLLIMPORT void _DK_frontnetmap_unload(void);
DLLIMPORT void _DK_frontnetmap_load(void);
DLLIMPORT int _DK_frontmap_load(void);
DLLIMPORT void _DK_frontnet_service_setup(void);
DLLIMPORT void _DK_frontnet_session_setup(void);
DLLIMPORT void _DK_frontnet_start_setup(void);
DLLIMPORT void _DK_frontnet_modem_setup(void);
DLLIMPORT void _DK_frontnet_serial_setup(void);
DLLIMPORT char *_DK_mdlf_for_cd(struct TbLoadFiles *);
DLLIMPORT char *_DK_mdlf_default(TbLoadFiles *);
DLLIMPORT int _DK_LbSpriteSetupAll(struct TbSetupSprite t_setup[]);
DLLIMPORT void _DK_frontstats_set_timer(void);

//Double exported - functions which are imports in DLL

DLLIMPORT int __stdcall _DK_FreeAudio(void);
DLLIMPORT int __stdcall _DK_SetRedbookVolume(int volume);
DLLIMPORT int __stdcall _DK_SetSoundMasterVolume(int volume);
DLLIMPORT int __stdcall _DK_SetMusicMasterVolume(int volume);
DLLIMPORT int __stdcall _DK_GetSoundInstalled(void);
int __stdcall _DK_PlayRedbookTrack(int);
int __stdcall _DK_MonitorStreamedSoundTrack(void);
int __stdcall _DK_StopRedbookTrack(void);
DLLIMPORT int __stdcall _DK_GetSoundDriver(void);
DLLIMPORT int __stdcall _DK_StopAllSamples(void);
DLLIMPORT int __stdcall _DK_GetFirstSampleInfoStructure(void);
DLLIMPORT int __stdcall _DK_LoadMusic(int);
DLLIMPORT int __stdcall _DK_InitAudio(int);
DLLIMPORT int __stdcall _DK_SetupAudioOptionDefaults(int);
int __stdcall _DK_StopStreamedSample(void);
DLLIMPORT int __stdcall _DK_StreamedSampleFinished(void);
DLLIMPORT int __stdcall _DK_SetStreamedSampleVolume(int);
DLLIMPORT int __stdcall _DK_GetLastSampleInfoStructure(void);
DLLIMPORT int __stdcall _DK_GetCurrentSoundMasterVolume(void);
int __stdcall _DK_StopMusic(void);
DLLIMPORT int __stdcall _DK_LoadAwe32Soundfont(int);

// Global variables migration between DLL and the program

DLLIMPORT extern HINSTANCE _DK_hInstance;
DLLIMPORT extern TDDrawBaseClass *_DK_lpDDC;
DLLIMPORT extern int _DK_icon_index;
DLLIMPORT extern int _DK_SoundDisabled;
DLLIMPORT extern char _DK_keeper_runtime_directory[152];
DLLIMPORT extern struct Game _DK_game;
DLLIMPORT extern char _DK_my_player_number;
DLLIMPORT extern unsigned long _DK_mem_size;
DLLIMPORT extern float _DK_phase_of_moon;
DLLIMPORT extern struct TbLoadFiles _DK_legal_load_files[];
DLLIMPORT extern unsigned char *_DK_palette;
DLLIMPORT extern unsigned char *_DK_scratch;
DLLIMPORT extern unsigned char _DK_lbKeyOn[256];
DLLIMPORT extern unsigned char _DK_lbInkey;
DLLIMPORT extern char _DK_datafiles_path[150];
DLLIMPORT extern char *_DK_strings_data;
DLLIMPORT extern struct TbLoadFiles _DK_game_load_files[];
DLLIMPORT extern char *_DK_strings[STRINGS_MAX+1];
DLLIMPORT extern struct GameSettings _DK_settings;
DLLIMPORT extern unsigned char _DK_exit_keeper;
DLLIMPORT extern unsigned char _DK_quit_game;
DLLIMPORT extern unsigned short _DK_pixel_size;
DLLIMPORT extern long _DK_left_button_held_x;
DLLIMPORT extern long _DK_left_button_held_y;
DLLIMPORT extern long _DK_left_button_double_clicked_y;
DLLIMPORT extern long _DK_left_button_double_clicked_x;
DLLIMPORT extern long _DK_right_button_double_clicked_y;
DLLIMPORT extern long _DK_right_button_double_clicked_x;
DLLIMPORT extern char _DK_right_button_clicked;
DLLIMPORT extern char _DK_left_button_clicked;
DLLIMPORT extern long _DK_right_button_released_x;
DLLIMPORT extern long _DK_right_button_released_y;
DLLIMPORT extern char _DK_right_button_double_clicked;
DLLIMPORT extern long _DK_left_button_released_y;
DLLIMPORT extern long _DK_left_button_released_x;
DLLIMPORT extern char _DK_left_button_double_clicked;
DLLIMPORT extern char _DK_right_button_released;
DLLIMPORT extern char _DK_right_button_held;
DLLIMPORT extern long _DK_right_button_click_space_count;
DLLIMPORT extern long _DK_right_button_held_y;
DLLIMPORT extern long _DK_left_button_clicked_y;
DLLIMPORT extern long _DK_left_button_clicked_x;
DLLIMPORT extern long _DK_left_button_click_space_count;
DLLIMPORT extern long _DK_right_button_held_x;
DLLIMPORT extern char _DK_left_button_released;
DLLIMPORT extern long _DK_right_button_clicked_y;
DLLIMPORT extern long _DK_right_button_clicked_x;
DLLIMPORT extern char _DK_left_button_held;
DLLIMPORT extern struct CatalogueEntry _DK_save_game_catalogue[8];
DLLIMPORT extern int _DK_number_of_saved_games;
DLLIMPORT extern int _DK_lbUseSdk;
DLLIMPORT extern unsigned char _DK_frontend_palette[768];
DLLIMPORT extern int _DK_load_game_scroll_offset;
DLLIMPORT extern int _DK_frontend_menu_state;
DLLIMPORT extern int _DK_continue_game_option_available;
DLLIMPORT extern struct FrontEndButtonData _DK_frontend_button_info[105];
DLLIMPORT extern int _DK_defining_a_key;
DLLIMPORT extern long _DK_defining_a_key_id;
DLLIMPORT extern long _DK_credits_scroll_speed;
DLLIMPORT extern long _DK_credits_offset;
DLLIMPORT extern struct TbSprite *_DK_frontend_font[4];
DLLIMPORT extern long _DK_last_mouse_x;
DLLIMPORT extern long _DK_last_mouse_y;
DLLIMPORT extern int _DK_credits_end;
DLLIMPORT extern unsigned char *_DK_frontend_background;
DLLIMPORT extern struct TbSprite *_DK_frontstory_font;
DLLIMPORT extern struct TbSprite *_DK_lbFontPtr;
DLLIMPORT extern long _DK_frontstory_text_no;
DLLIMPORT extern int _DK_FatalError;
DLLIMPORT extern int _DK_net_service_index_selected;
DLLIMPORT extern unsigned char _DK_fade_palette_in;
DLLIMPORT extern long _DK_old_mouse_over_button;
DLLIMPORT extern long _DK_frontend_mouse_over_button;
DLLIMPORT extern struct HighScore _DK_high_score_table[10];
DLLIMPORT extern struct TbLoadFiles _DK_frontstory_load_files[4];
DLLIMPORT extern struct TbLoadFiles _DK_netmap_flag_load_files[7];
DLLIMPORT extern int _DK_fe_high_score_table_from_main_menu;
DLLIMPORT extern struct TbSprite *_DK_frontend_sprite;
DLLIMPORT extern long _DK_define_key_scroll_offset;
DLLIMPORT extern ModDL_Fname_Func _DK_modify_data_load_filename_function;
DLLIMPORT extern struct TbSetupSprite _DK_frontstory_setup_sprites[2];
DLLIMPORT extern long _DK_high_score_entry_input_active;
DLLIMPORT extern long _DK_high_score_entry_index;
DLLIMPORT extern char _DK_high_score_entry[64];
DLLIMPORT extern unsigned long _DK_time_last_played_demo;
DLLIMPORT extern char _DK_lbfade_open;

#ifdef __cplusplus
}
#endif

//Functions - reworked
void LbSetIcon(unsigned short nicon);
short setup_game(void);
void game_loop(void);
short reset_game(void);
void error(const char *codefile,const int ecode,const char *message);

//Functions - internal


#endif // DK_KEEPERFX_H
