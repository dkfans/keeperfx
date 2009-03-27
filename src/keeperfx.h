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
#include "bflib_video.h"
#include "bflib_keybrd.h"
#include "packets.h"
#include "thing_list.h"
#include "player_computer.h"
#include "game_saves.h"

#define LEGAL_WIDTH  640
#define LEGAL_HEIGHT 480

#define DUNGEONS_COUNT         5
#define PLAYERS_COUNT          5
#define PLAYERS_EXT_COUNT      6
#define NET_PLAYERS_COUNT      4
#define PACKETS_COUNT          5
#define THINGS_COUNT        2048
#define COLUMNS_COUNT       2048
#define EVENTS_COUNT         100
#define ROOMS_COUNT          150
#define CREATURES_COUNT      256
#define MESSAGE_QUEUE_COUNT    4
#define GUI_MESSAGES_COUNT     3
#define CREATURE_PARTYS_COUNT 16
#define WIN_CONDITIONS_COUNT   4
#define CREATURE_MAX_LEVEL    10
#define THING_CLASSES_COUNT   10
#define GAME_KEYS_COUNT       32
#define GAMMA_LEVELS_COUNT     5
#define LENSES_COUNT          15
#define PERS_ROUTINES_COUNT    4
#define MINMAXS_COUNT         64
#define LIGHTS_COUNT         400
#define SHADOW_LIMITS_COUNT 2048
#define COLUMN_STACK_HEIGHT    8
#define GOLD_LOOKUP_COUNT     40
#define TURN_TIMERS_COUNT      8
#define SLABSET_COUNT       1304
#define SLABOBJS_COUNT       512
#define MESSAGE_TEXT_LEN    1024
// Amount of instances; it's 17, 18 or 19
#define PLAYER_INSTANCES_COUNT 19
#define ZOOM_KEY_ROOMS_COUNT  14
#define SMALL_MAP_RADIUS      58
// Static textures
#define TEXTURE_BLOCKS_STAT_COUNT   544
// Animated textures
#define TEXTURE_BLOCKS_ANIM_COUNT   48
#define TEXTURE_BLOCKS_COUNT TEXTURE_BLOCKS_STAT_COUNT+TEXTURE_BLOCKS_ANIM_COUNT
// Sprites
// note - this is temporary value; not correct
#define GUI_PANEL_SPRITES_COUNT      9000
#define CREATURE_FRAMELIST_LENGTH     982
// Types of objects
#define CREATURE_TYPES_COUNT  32
#define SPELL_TYPES_COUNT     20
#define TRAP_TYPES_COUNT      11
#define DOOR_TYPES_COUNT       8
#define ROOM_TYPES_COUNT      17
#define OBJECT_TYPES_COUNT   136
#define ACTN_POINTS_COUNT     32
#define SLAB_TYPES_COUNT      58
#define MOON_BONUS_INDEX       5
// Camera constants; max zoom is when everything is large
#define CAMERA_ZOOM_MIN 4100
#define CAMERA_ZOOM_MAX 12000
#define TD_ISO_POINTS   982

#define SIZEOF_TDDrawSdk 408

#ifdef __cplusplus
extern "C" {
#endif

// Temporary
typedef struct SSurface {
       char a;
       } TSurface;

enum TbErrorCode {
    Lb_OK                   =  0,
    Lb_FAIL                 = -1,
};

enum PlayerViewType {
    PVT_DungeonTop          =  1,
    PVT_CreatureContrl      =  2,
    PVT_CreaturePasngr      =  3,
    PVT_MapScreen           =  4,
};

enum ModeFlags {
    MFlg_IsDemoMode         =  0x01,
    MFlg_EyeLensReady       =  0x02,
    MFlg_NoMusic            =  0x10,
};

enum SlabTypes {
    SlbT_ROCK               =   0,
    SlbT_GOLD               =   1,
    SlbT_EARTH              =   2,
    SlbT_TORCHDIRT          =   3,
    SlbT_WALLDRAPE          =   4,
    SlbT_WALLTORCH          =   5,
    SlbT_WALLWTWINS         =   6,
    SlbT_WALLWWOMAN         =   7,
    SlbT_WALLPAIRSHR        =   8,
    SlbT_PATH               =  10,
    SlbT_CLAIMED            =  11,
    SlbT_LAVA               =  12,
    SlbT_WATER              =  13,
    SlbT_ENTRANCE           =  14,
    SlbT_TREASURE           =  16,
    SlbT_LIBRARY            =  18,
    SlbT_PRISON             =  20,
    SlbT_TORTURE            =  22,
    SlbT_TRAINING           =  24,
    SlbT_DUNGHEART          =  26,
    SlbT_WORKSHOP           =  28,
    SlbT_SCAVENGER          =  30,
    SlbT_TEMPLE             =  32,
    SlbT_GRAVEYARD          =  34,
    SlbT_GARDEN             =  36,
    SlbT_LAIR               =  38,
    SlbT_BARRACKS           =  40,
    SlbT_DOORWOOD1          =  42,
    SlbT_DOORWOOD2          =  43,
    SlbT_DOORBRACE1         =  44,
    SlbT_DOORBRACE2         =  45,
    SlbT_DOORIRON1          =  46,
    SlbT_DOORIRON2          =  47,
    SlbT_DOORMAGIC1         =  48,
    SlbT_DOORMAGIC2         =  49,
    SlbT_BRIDGE             =  51,
    SlbT_GEMS               =  52,
    SlbT_GUARDPOST          =  53,
};

enum RoomKinds {
    RoK_NONE                =   0,
    RoK_ENTRANCE            =   1,
    RoK_TREASURE            =   2,
    RoK_LIBRARY             =   3,
    RoK_PRISON              =   4,
    RoK_TORTURE             =   5,
    RoK_TRAINING            =   6,
    RoK_DUNGHEART           =   7,
    RoK_WORKSHOP            =   8,
    RoK_SCAVENGER           =   9,
    RoK_TEMPLE              =  10,
    RoK_GRAVEYARD           =  11,
    RoK_GARDEN              =  13,
    RoK_LAIR                =  14,
    RoK_BARRACKS            =  12,
    RoK_BRIDGE              =  15,
    RoK_GUARDPOST           =  16,
    RoK_UNKN17              =  17,
};

typedef unsigned char SlabType;
typedef unsigned char RoomKind;
typedef int TbError;

#pragma pack(1)

//This seems to be array of functions in __thiscall convention
// they are all methods for class TDDrawBase and its derivants
// Note: May be incorrectly named - the Beta version was used to get them!
// The inplementation as class is made in "bflib_drawcls.h"
typedef struct {
    void __fastcall (*dt)(void *ths); // +0 virtual ~TDDrawSdk(void);
    long __fastcall (*setup_window)(void *ths); // +4 virtual int setup_window(void);
    long CALLBACK (*WindowProc)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam); // +8 virtual unsigned int WindowProc(void *,unsigned int,unsigned int,long);
    void __fastcall (*find_video_modes)(void *ths); // +12 virtual void find_video_modes(void);
    int (*get_palette)(void *,unsigned long,unsigned long); // +16 virtual int get_palette(void *,unsigned long,unsigned long);
    int (*set_palette)(void *,unsigned long,unsigned long); // +20 virtual int set_palette(void *,unsigned long,unsigned long);
    int (*setup_screen)(TbScreenMode); // +24 virtual int setup_screen(TbScreenMode);
    long __fastcall (*lock_screen)(void *ths); // +28 virtual long lock_screen(void);
    long __fastcall (*unlock_screen)(void *ths); // +32 virtual long unlock_screen(void);
    long (*clear_screen)(unsigned long); // +36 virtual long clear_screen(unsigned long);
    long (*clear_window)(long,long,unsigned long,unsigned long,unsigned long); // +40 virtual long clear_window(long,long,unsigned long,unsigned long,unsigned long);
    int __fastcall (*swap_screen)(void *ths); //+44 virtual int swap_screen(void);
    int __fastcall (*reset_screen)(void *ths); // +48 virtual int reset_screen(void);
    long __fastcall (*restore_surfaces)(void *ths); // +52 virtual long restore_surfaces(void);
    void __fastcall (*wait_vbi)(void *ths); // +56 virtual void wait_vbi(void);
    long (*swap_box)(tagPOINT,tagRECT &); // +60 virtual long swap_box(tagPOINT,tagRECT &);
    long (*create_surface)(TSurface *,unsigned long,unsigned long); // +64 virtual long create_surface(_TSurface *,unsigned long,unsigned long);
    long (*release_surface)(TSurface *); // +68 virtual long release_surface(_TSurface *);
    long (*blt_surface)(TSurface *,unsigned long,unsigned long,tagRECT *,unsigned long); // +72 virtual long blt_surface(_TSurface *,unsigned long,unsigned long,tagRECT *,unsigned long);
    long (*lock_surface)(TSurface *); // +76 virtual long lock_surface(_TSurface *);
    long (*unlock_surface)(TSurface *); // +80 virtual long unlock_surface(_TSurface *);
    void (*LoresEmulation)(int); // +84 virtual void LoresEmulation(int);
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

// Windows-standard structure
/*struct _GUID {
     unsigned long Data1;
     unsigned short Data2;
     unsigned short Data3;
     unsigned char Data4[8];
};*/

struct KeycodeString {
    TbKeyCode keys[LINEMSG_SIZE];
    long length;
};

struct StartupParameters {
    unsigned long selected_lev_file_num;
    unsigned char no_intro;
    unsigned char one_player;
    unsigned char numfield_C;
    unsigned char flags_font;
    unsigned char flags_cd;
    long num_fps;
    unsigned char packet_save_enable;
    unsigned char packet_load_enable;
    char packet_fname[150];
    unsigned char packet_checksum;
};

struct NetMessage {
unsigned char field_0;
char field_1[64];
};

struct GuiMessage { // sizeof = 0x45 (69)
    char text[64];
unsigned char field_40;
unsigned long field_41;
};

struct MessageQueueEntry { // sizeof = 9
     unsigned char field_0;
     unsigned long field_1;
     unsigned long field_5;
};

struct CreatureParty { // sizeof = 208
  char prtname[100];
  unsigned char field_64;
  unsigned char field_65;
  unsigned char field_66;
  unsigned char field_67;
  unsigned long field_68;
  unsigned char field_6C;
  unsigned short field_6D;
  unsigned short field_6F;
  unsigned char field_71[91];
  unsigned long field_CC;
};

//I'm not sure about size of this structure
struct Camera {
    struct Coord3d mappos;
    unsigned char field_6;
    int orient_a;
    int orient_b;
    int orient_c;
    int field_13;
    int field_17;
    int field_1B;
    unsigned char field_1F[9];
    short field_28;
    short field_2A;
    short field_2C;
    short field_2E;
    short field_30;
    short field_32;
    unsigned char field_34;
    int field_35;
    int field_39;
    int field_3D;
    int field_41;
    unsigned char field_45[19];
    short field_58;
    short field_5A;
    short field_5C;
    unsigned char field_5E;
    unsigned char field_5F[12];
    int field_6B;
    unsigned char field_6F[19];
    short field_82;
    short field_84;
    short field_86;
    unsigned char field_88;
    unsigned char field_89[12];
    int field_95;
    int field_99;
};

struct TurnTimer { // sizeof = 5
  unsigned long count;
  unsigned char state;
};

struct InitLight { // sizeof=0x14
short field_0;
unsigned char field_2;
unsigned char field_3;
unsigned char field_4[6];
    struct Coord3d mappos;
unsigned char field_10;
unsigned char field_11;
unsigned char field_12[2];
};

struct EngineCoord { // sizeof = 28
  unsigned long field_0;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long x;
  unsigned long y;
  unsigned long z;
};


struct M33 { // sizeof = 48
  unsigned long r0[4];
  unsigned long r1[4];
  unsigned long r2[4];
};

struct EngineCol {
//TODO!!
};

struct BasicQ
{
//TODO!!
};

struct XYZ { // sizeof = 12
    long x;
    long y;
    long z;
};

struct PolyPoint { // sizeof = 8
  long field_0;
  long field_4;
//TODO!!!
};

struct MapOffset {
  unsigned char field_0;
  unsigned char field_1;
  unsigned short field_2;
};

struct MinMax { // sizeof = 8
    long min;
    long max;
};

struct MapVolumeBox { // sizeof = 24
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2;
  unsigned long field_3;
  unsigned long field_7;
  unsigned long field_B;
  unsigned long field_F;
  unsigned long field_13;
  unsigned char field_17;
};

struct KeeperSprite { // sizeof = 16
  unsigned char field_0[9];
  unsigned char field_9;
  unsigned char field_A[4];
  unsigned char field_E[2];
};

struct GoldLookup { // sizeof = 28
unsigned char field_0;
unsigned char field_1;
unsigned long field_2;
unsigned short field_6;
unsigned short field_8;
unsigned short field_A;
unsigned short field_C;
unsigned short field_E;
unsigned long field_10;
unsigned char field_14[6];
unsigned short field_1A;
};

struct InitActionPoint { // sizeof = 8
    struct Coord2d mappos;
    unsigned short range;
    unsigned short num;
};

struct ActionPoint { // sizeof = 0xA
    unsigned char flags;
    struct Coord2d mappos;
    unsigned short range;
    unsigned short num;
char field_9;
};

struct Column { // sizeof=0x18
    short use;
    unsigned char bitfileds;
    unsigned short solidmask;
    unsigned short baseblock;
    unsigned char orient;
    unsigned short cubes[COLUMN_STACK_HEIGHT];
};

struct SpecialDesc {
long field_0;
long field_4;
long field_8;
};

struct UnkStruc5 { // sizeof=0x12
int field_0;
char field_4[4];
short field_8[5];
};

struct UnkStruc6 { // sizeof = 8
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2;
  unsigned char field_3;
  unsigned long field_4;
};

struct UnkStruc7 { // sizeof = 17
  unsigned long field_0;
  unsigned char field_4[9];
  unsigned short field_D;
  unsigned short field_F;
};

struct Light { // sizeof = 46
  unsigned char field_0;
  unsigned char field_1[6];
  unsigned short field_7;
  unsigned char field_9[5];
  unsigned short field_E;
  unsigned char field_10[28];
  unsigned short field_2C;
};

struct ShadowCache { // sizeof = 129
  unsigned char field_0[128];
  unsigned char field_80;
};

struct ObjectConfig { // sizeof=0x1D
    long health;
char field_4;
char field_5;
char field_6;
char field_7;
char field_8;
short light;
char field_B;
char field_C[14];
char field_1A[3];
};

struct SpellConfig { // sizeof=0x8
int field_0;
int field_4;
};

struct Armageddon { // sizeof = 14
  unsigned long count_down;
  unsigned long duration;
  Coord3d mappos;
};

struct ManfctrConfig { // sizeof=0x14
int field_0;
int field_4;
int field_8;
int field_C;
int field_10;
};

struct Room {
    unsigned char field_0;
    unsigned short field_1;
    unsigned char owner;
    short field_4;
    unsigned short field_6;
    unsigned char field_8;
    unsigned char field_9;
    unsigned short kind;
    unsigned char field_C[6];
    unsigned char field_12;
    unsigned char field_13[4];
    short field_17;
    unsigned short field_19;
    unsigned char field_1B[28];
    unsigned short field_37;
    unsigned short field_39;
    unsigned short field_3B;
    unsigned char field_3D[4];
    unsigned char field_41[2];
    unsigned char field_43[2];
};

struct Objects {
    unsigned char field_0;
    unsigned char field_1;
    unsigned char field_2;
    unsigned char field_3;
    unsigned char field_4;
    short field_5;
    short field_7;
    short field_9;
    short field_B;
    short field_D;
    unsigned char field_F;
    unsigned char field_10;
    unsigned char field_11;
    unsigned char field_12;
    unsigned char field_13;
    unsigned char field_14;
    unsigned char field_15;
};

struct TrapData {
      long field_0;
      long field_4;
      short field_8;
      short field_A;
      short field_C;
      short field_E;
};

struct RoomData {
      unsigned char numfield_0;
      short numfield_1;
      long ofsfield_3;
      long ofsfield_7;
      long offfield_B;
      unsigned char field_F;
      unsigned char field_10;
      short field_11;
      short field_13;
      short field_15;
};

struct SpellData {
      long field_0;
      long field_4;
      unsigned char field_8;
      short field_9;
      short field_B;
      short field_D;
      unsigned char field_F[2];
      short field_11;
      short field_13;
      long field_15;
      unsigned char field_19;
      unsigned char field_1A;
};

struct CreatureData {
      unsigned char field_0;
      short field_1;
      short field_3;
};

struct SlabMap {
      SlabType slab;
      short field_1;
      unsigned char field_3;
      unsigned char field_4;
      unsigned char field_5;
};

struct SlabAttr {
    short field_0;
    short field_2;
    short field_4;
    long field_6;
    long field_A;
    unsigned char field_E;
    unsigned char field_F;
    unsigned char field_10;
    unsigned char field_11;
    unsigned char field_12;
    unsigned char field_13;
    unsigned char field_14;
    unsigned char field_15;
};

struct SlabSet { // sizeof = 18
  short col_idx[9];
};

struct SlabObj { // sizeof = 13
  unsigned char field_0[11];
  unsigned short field_B;
};

struct Event { // sizeof=0x15
unsigned char field_0;
unsigned char field_1;
unsigned long field_2;
unsigned long field_6;
unsigned char field_A;
unsigned char field_B;
long field_C; // signed
unsigned long field_10;
unsigned char field_14;
};

struct Creatures { // sizeof = 16
  unsigned short numfield_0;
  unsigned short numfield_2;
  unsigned char field_4[2];
  unsigned char field_6;
  unsigned char field_7;
  unsigned char field_8;
  unsigned char field_9[3];
  unsigned char field_C[4];
};

struct CreatureControl {
    unsigned short field_0;
    unsigned char field_2;
char field_3[28];
    unsigned short thing_idx;
unsigned char field_21[77];
    short field_6E;
unsigned char field_70[10];
    unsigned short field_7A;
unsigned char field_7C[15];
    unsigned char field_8B;
unsigned char field_8C[31];
    unsigned char field_AB;
unsigned char field_AC;
    unsigned char field_AD;
unsigned char field_AE[3];
    unsigned char field_B1;
unsigned char field_B2[7];
unsigned short field_B9;
    CoordDelta3d pos_BB;
    unsigned char bloody_footsteps_turns;
unsigned char field_C2[6];
    short field_C8;
    short field_CA;
unsigned char field_CC[2];
    unsigned long field_CE;
unsigned char field_D2;
unsigned char field_D3[203];
    char instances[256];
unsigned char field_29E[18];
    unsigned char field_2B0;
unsigned char field_2B1[62];
    unsigned long field_2EF;
unsigned char field_2F3[11];
    unsigned long field_2FE;
    unsigned char field_302;
unsigned char field_303[3];
unsigned char field_306[2];
};

struct CreatureStats { // sizeof = 230
  unsigned short job_primary;
  unsigned short job_secondary;
  unsigned short jobs_not_do;
  unsigned char eye_effect;
  unsigned short health;
  unsigned char heal_requirement;
  unsigned char heal_threshold;
  unsigned char strength;
  unsigned char armour;
  unsigned char dexterity;
  unsigned char fear;
  unsigned char defence;
  unsigned char luck;
  unsigned char recovery;
  unsigned short hunger_rate;
  unsigned char hunger_fill;
  unsigned short annoy_level;
  unsigned char lair_size;
  unsigned char hurt_by_lava;
  unsigned char sleep_exp_slab;
  unsigned char sleep_experience;
short exp_for_hitting;
short gold_hold;
short training_cost;
short scavenger_cost;
short scavenge_require;
  unsigned char scavenge_value;
  unsigned long to_level[10];
  unsigned char base_speed;
short grow_up;
  unsigned char grow_up_level;
  unsigned char entrance_force;
short max_angle_change;
short eye_height;
short field_57[14];
short field_73;
  unsigned short size_xy;
  unsigned short size_yz;
  unsigned short walking_anim_speed;
  unsigned char flying;
  unsigned char immune_to_gas;
  unsigned char attack_preference;
short field_of_view;
  unsigned char instance_spell[10];
  unsigned char instance_level[10];
  unsigned char research_value;
  unsigned char humanoid_creature;
  unsigned char piss_on_dead;
  unsigned char training_value;
short pay;
  unsigned char manufacture_value;
  unsigned char hearing;
  unsigned char entrance_rooms[3];
  unsigned char entrance_slabs_req[3];
  unsigned char visual_range;
  unsigned char real_training;
short torture_time;
short annoy_no_lair;
short annoy_no_hatchery;
short annoy_woken_up;
short annoy_on_dead_friend;
short annoy_sulking;
short annoy_no_salary;
short annoy_slapped;
short annoy_on_dead_enemy;
short annoy_in_temple;
short annoy_sleeping;
short annoy_got_wage;
short annoy_in_torture;
short annoy_win_battle;
short annoy_untrained_time;
short annoy_untrained;
short field_C4;
short annoy_queue;
short annoy_will_not_do_job;
short job_stress;
short annoy_job_stress;
short jobs_anger;
short annoy_others_leaving;
    unsigned char slaps_to_kill;
short lair_enemy;
short hero_vs_keeper_cost;
    unsigned char rebirth;
    unsigned char can_see_invisible;
    unsigned char can_go_locked_doors;
    unsigned char bleeds;
    unsigned char affected_by_wind;
    unsigned short thing_size_xy;
    unsigned short thing_size_yz;
short annoy_eat_food;
short annoy_in_hand;
short damage_to_boulder;
};

struct Persons {
    struct CreatureControl *cctrl_lookup[CREATURES_COUNT];
    struct CreatureControl *cctrl_end;
};

struct MagicStats {  // sizeof=0x4C
long field_0;
long field_4[8];
long field_24;
long field_28[8];
long field_48;
};

struct TrapStats {  // sizeof=54
unsigned long field_0;
unsigned long field_4;
unsigned long field_8;
unsigned char field_C[6];
  unsigned char field_12;
unsigned char field_13[7];
unsigned char field_1A[6];
unsigned char field_20[8];
unsigned char field_28[8];
unsigned char field_30[6];
};

struct LevelStats { // sizeof = 392
  unsigned long things_researched;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
  unsigned long field_10;
  unsigned long field_14;
  unsigned long field_18;
  unsigned long field_1C;
  unsigned long field_20;
  unsigned long field_24;
  unsigned long field_28;
  unsigned long field_2C;
  unsigned long field_30;
  unsigned long field_34;
  unsigned long field_38;
  unsigned long field_3C;
  unsigned long field_40;
  unsigned long field_44;
  unsigned long field_48;
  unsigned long field_4C;
  unsigned long field_50;
  unsigned long field_54;
  unsigned long field_58;
  unsigned long field_5C;
  unsigned long field_60;
  unsigned long field_64;
  unsigned long field_68;
  unsigned long field_6C;
  unsigned long field_70;
  unsigned long field_74;
  unsigned long field_78;
  unsigned long field_7C;
  unsigned long field_80;
  unsigned long field_84;
  unsigned long field_88;
  unsigned long gold_mined;
  unsigned long field_90;
  unsigned long manufactured_doors;
  unsigned long manufactured_traps;
  unsigned long manufactured_items;
  unsigned long time1;
  unsigned long time2;
  unsigned long creatures_trained;
  unsigned long creatures_tortured;
  unsigned long creatures_sacrificed;
  unsigned long creatures_converted;
  unsigned long creatures_left_UNSURE;
  unsigned long num_slaps;
  unsigned long num_caveins;
  unsigned long bridges_built;
  unsigned long rock_dug_out;
  unsigned long salary_cost;
  unsigned long flies_killed_by_spiders;
  unsigned long territory_destroyed;
  unsigned long territory_lost;
  unsigned long rooms_constructed;
  unsigned long traps_used;
  unsigned long traps_armed;
  unsigned long doors_used;
  unsigned long keepers_destroyed;
  unsigned long area_claimed;
  unsigned long backs_stabbed;
  unsigned long chickens_hatched;
  unsigned long chickens_eaten;
  unsigned long chickens_wasted;
  unsigned long promises_broken;
  unsigned long ghosts_raised;
  unsigned long skeletons_raised;
  unsigned long friendly_kills;
  unsigned long lies_told;
  unsigned long creatures_annoyed;
  unsigned long graveyard_bodys;
  unsigned long vamps_created;
  unsigned long num_creatures;
  unsigned long imps_deployed;
  unsigned long battles_won;
  unsigned long battles_lost;
  unsigned long money;
  unsigned long dngn_breached_count;
  unsigned long doors_destroyed;
  unsigned long rooms_destroyed;
  unsigned long dungeon_area;
  unsigned long ideas_researched;
  unsigned long creatures_scavenged;
  unsigned long creatures_summoned;
  unsigned long spells_stolen;
  unsigned long gold_pots_stolen;
  unsigned long field_15C;
  unsigned long field_160;
  unsigned long field_164;
  unsigned long doors_unused;
  unsigned long traps_unused;
  unsigned long num_rooms;
  unsigned long field_174;
  unsigned long num_entrances;
  unsigned long hopes_dashed;
  unsigned long allow_save_score;
  unsigned long player_score;
};

struct Map {
      unsigned char flags;
      unsigned long data;
};

struct CreaturePool { // sizeof = 129
  unsigned long crtr_kind[CREATURE_TYPES_COUNT];
  unsigned char is_empty;
};

struct Bookmark { // sizeof = 3
  unsigned char x;
  unsigned char y;
  unsigned char flags;
};

struct GameKey { // sizeof = 2
  unsigned char code;
  unsigned char mods;
};

struct GameSettings { // sizeof = 0x52 (82)
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
    struct GameKey kbkeys[GAME_KEYS_COUNT];
    unsigned char tooltips_on;
    unsigned char field_50;
    unsigned char field_51;
    };

struct SoundSettings {
  char *sound_data_path;
  char *music_data_path;
  char *dir3;
  unsigned short sound_type;
  unsigned short field_E;
  unsigned char max_number_of_samples;
  unsigned char stereo;
  unsigned char field_12;
  unsigned char danger_music;
  unsigned char no_load_sounds;
  unsigned char no_load_music;
  unsigned char field_16;
  unsigned char sound_system;
  unsigned char field_18;
  unsigned char redbook_enable;
};

struct InstanceInfo { // sizeof = 42
  unsigned char field_0;
  unsigned char field_1[3];
  unsigned char field_4[4];
  unsigned char field_8[4];
  unsigned char field_C[5];
  unsigned long field_11;
  unsigned long field_15;
  unsigned char field_19;
  unsigned char field_1A;
  unsigned char field_1B[2];
  unsigned char field_1D;
  void *field_1E;
  unsigned long field_22;
  unsigned char field_26[4];
};

struct Wander {
unsigned char field_0[424];
    };

struct UnkScrptStruc1 { // sizeof = 18
  unsigned char field_0;
  unsigned char field_1[3];
  unsigned char field_4[3];
  unsigned char field_7;
  unsigned long field_8;
  unsigned long field_C;
  unsigned char field_10;
  unsigned char field_11;
};

struct UnkScrptStruc2 { // sizeof = 13
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2[2];
  unsigned long field_4;
  unsigned char field_8;
  unsigned short field_9;
  unsigned short field_B;
};

struct UnkScrptStruc3 { // sizeof = 16
  unsigned char field_0;
  unsigned char field_1;
  unsigned char field_2;
  unsigned char field_3;
  unsigned long field_4;
  unsigned long field_8;
  unsigned long field_C;
};

struct UnkScrptStruc4 { // sizeof = 12
  unsigned short field_0;
  unsigned char field_2;
  unsigned char field_3;
  unsigned char field_4;
  unsigned short field_5;
  unsigned char field_7;
  unsigned long field_8;
};

struct LevelScript { // sizeof = 5884
    struct UnkScrptStruc1 script_field_0[16];
unsigned long script_field_120;
    struct UnkScrptStruc2 script_field_124[48];
unsigned long script_field_394;
    struct UnkScrptStruc3 script_field_398[64];
unsigned long script_field_798;
    struct UnkScrptStruc4 script_field_79C[48];
unsigned long script_field_9DC;
    struct CreatureParty creature_partys[CREATURE_PARTYS_COUNT];
    unsigned long creature_partys_num;
    unsigned short win_conditions[WIN_CONDITIONS_COUNT];
    unsigned long win_conditions_num;
    unsigned short lose_conditions[WIN_CONDITIONS_COUNT];
    unsigned long lose_conditions_num;
};

#define SIZEOF_PlayerInfo 0x4EF
struct PlayerInfo {
    unsigned char field_0;
    unsigned char field_1;
    unsigned char field_2; //seems to be never used
    unsigned char field_3;
    unsigned char field_4;
    unsigned char field_5;
    unsigned char field_6;
    unsigned char *field_7;
    unsigned char packet_num; // index of packet slot associated with this player
unsigned char field_C[4];
unsigned int field_10;
unsigned char field_14;
    char field_15[20]; //size may be shorter
    unsigned char field_29;
    unsigned char field_2A;
    unsigned char field_2B;
    unsigned char field_2C;
    unsigned char field_2D[2];
    short field_2F;
    long field_31;
    short field_35;
    unsigned char field_37;
    struct Camera *camera;  // Pointer to the currently active camera
    struct Coord3d cam_mappos;
//    struct Camera camera2;
    unsigned char field_42;
    long field_43;
    unsigned char field_47[31];
//    struct Camera camera3;
    unsigned char field_66[42];
    unsigned short field_90;
    unsigned short field_92;
    unsigned char field_94[3];
    long field_97;
    unsigned char field_9B[31];
    unsigned short field_BA;
    unsigned short field_BC;
    unsigned char field_BE[3];
    long field_C1;
    unsigned char field_C5[16];
    unsigned char field_D5[4];
    unsigned char field_D9[11];
    unsigned short field_E4;
    unsigned short field_E6;
char field_E8[2];
    struct Wander wandr1;
    struct Wander wandr2;
    short field_43A;
char field_43C[2];
    short field_43E;
    long field_440;
    short engine_window_width;
    short engine_window_height;
    short engine_window_x;
    short engine_window_y;
    short mouse_x;
    short mouse_y;
    unsigned short minimap_zoom;
    unsigned char view_type;
    unsigned char field_453;
    unsigned char field_454;
char field_455;
    unsigned char field_456;
char field_457[8];
char field_45F;
short field_460;
char field_462;
    char strfield_463[64];
    char field_4A3[2];
    char field_4A5;
    char field_4A6;
    char field_4A7[9];
    unsigned char instance_num;
    unsigned long field_4B1;
    char field_4B5;
    long field_4B6;
    char field_4BA[3];
    long field_4BD;
    long field_4C1;
    long field_4C5;
    unsigned char *field_4C9;
    long field_4CD;
    char field_4D1;
    long field_4D2;
    char field_4D6[4];
    char field_4DA;
    char field_4DB[8];
    long field_4E3;
    long field_4E7;
    long field_4EB;
    };

#define SIZEOF_Dungeon 0x1508
struct Dungeon {
    unsigned short field_0;
    struct Coord3d mappos;
    unsigned char field_8;
    unsigned char field_9;
    unsigned char computer_enabled;
    unsigned short field_B[2];
    unsigned short field_F;
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
    unsigned int field_33;  // originally short, but with 2b padding
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
    unsigned short field_424[CREATURE_TYPES_COUNT][3];
    unsigned short field_4E4[CREATURE_TYPES_COUNT][3];
    short field_5A4[15];
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
    int field_A4F[32];
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
    short field_B01[DUNGEONS_COUNT];
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
    unsigned char field_F82[112];
unsigned char field_FF2;
unsigned char field_FF3[19];
unsigned char field_1006;
unsigned char field_1007[28];
unsigned char field_1023[9];
    struct TurnTimer turn_timers[TURN_TIMERS_COUNT];
unsigned long field_1054;
unsigned long field_1058;
    long field_105C;
    unsigned char field_1060[34];
    unsigned char field_1082[241];
    unsigned char field_1173;
    unsigned char field_1174;
    unsigned char field_1175;
    unsigned char field_1176;
    short field_1177;
    long field_1179;
    long field_117D;
long field_1181;
long field_1185;
    unsigned char field_1189;
unsigned char field_118A;
long field_118B;
long field_118F;
long field_1193;
    struct LevelStats lvstats;
unsigned char field_131F[3];
unsigned char field_1322[36];
unsigned char field_1346[60];
unsigned char field_1382[29];
int field_139F;
int field_13A3;
unsigned char field_13A7[121];
unsigned short field_1420[32];
unsigned char field_1460[41];
unsigned char field_1489[32];
int field_14A9;
unsigned char field_14AD;
int field_14AE;
unsigned char field_14B2[2];
int field_14B4;
int field_14B8;
unsigned char field_14BC[6];
unsigned short field_14C2[32];
    short field_1502;
    int field_1504;
    };

#define SIZEOF_Game 1382437

// only one such struct exists at .data:005F0310
// it ends at 00741B35
struct Game { // (sizeof=0x151825)
char numfield_0;
int version_major;
int version_minor;
unsigned char continue_level;
unsigned char numfield_A;
char align_B;
    unsigned char numfield_C;
char numfield_D;
    unsigned char flags_font;
char one_player;
    unsigned char eastegg01_cntr;
unsigned char flags_cd;
    unsigned char eastegg02_cntr;
char audiotrack;
char numfield_14;
char numfield_15;
    unsigned long selected_lev_file_num;
char numfield_1A;
    unsigned char numfield_1B;
    struct PlayerInfo players[PLAYERS_COUNT];
    struct Column columns[COLUMNS_COUNT];
struct UnkStruc5 struc_D8C7[512];
struct ObjectConfig objects_config[239];
char field_117DA[14];
struct ManfctrConfig traps_config[7];
struct ManfctrConfig doors_config[5];
struct SpellConfig spells_config[15];
    struct Thing *things_lookup[THINGS_COUNT];
    struct Thing *things_end;
    struct Persons persons;
    struct Column *columns_lookup[COLUMNS_COUNT];
    struct Column *columns_end;
    unsigned short slabset_num;
    struct SlabSet slabset[SLABSET_COUNT];
    unsigned short slabobjs_num;
    unsigned short slabobjs_idx[SLABSET_COUNT];
    struct SlabObj slabobjs[SLABOBJS_COUNT];
unsigned char field_1DD40;
    struct UnkStruc6 field_1DD41[1024];
    unsigned char shadow_limits[SHADOW_LIMITS_COUNT];
    struct Light lights[LIGHTS_COUNT];
    struct ShadowCache shadow_cache[40];
    unsigned short stat_light_map[256*256];
long field_46149;
char field_4614D;
char field_4614E;
int field_4614F;
int field_46153;
    unsigned short field_46157[256*256];
    struct CreatureControl cctrl_data[CREATURES_COUNT];
    struct Thing things_data[THINGS_COUNT];
    unsigned char mapflags[256*256];
    struct Map map[256*256];
    struct ComputerTask computer_task[COMPUTER_TASKS_COUNT];
    struct Computer2 computer[PLAYERS_COUNT];
    struct SlabMap slabmap[85*85];
    struct Room rooms[ROOMS_COUNT];
    struct Dungeon dungeon[DUNGEONS_COUNT];
char field_149E05;
    struct StructureList thing_lists[11];
unsigned int field_149E5E;
unsigned int field_149E62;
unsigned int field_149E66;
unsigned int field_149E6A;
unsigned int field_149E6E;
char field_149E72[5];
unsigned int field_149E77;
unsigned char field_149E7B;
unsigned int field_149E7C;
    unsigned char packet_save_enable;
    unsigned char packet_load_enable;
    char packet_fname[150];
    char packet_fopened;
    TbFileHandle packet_save_fp;
unsigned int packet_file_pos;
    struct PacketSaveHead packet_save_head;
unsigned int field_149F30;
unsigned int turns_fastforward;
unsigned char numfield_149F38;
    unsigned char packet_checksum;
unsigned int numfield_149F3A;
unsigned int numfield_149F3E;
    int numfield_149F42;
unsigned char numfield_149F46;
unsigned char numfield_149F47;
    struct CatalogueEntry save_catalogue[SAVE_SLOTS_COUNT];
    struct Event event[EVENTS_COUNT];
unsigned long field_14A804;
unsigned long field_14A808;
unsigned long field_14A80C;
unsigned long field_14A810;
unsigned long field_14A814;
short field_14A818[18];
char field_14A83C;
    unsigned char level_file_number;
    unsigned char level_number; // change it to long ASAP
short texture_animation[8*TEXTURE_BLOCKS_ANIM_COUNT];
unsigned short field_14AB3F;
    unsigned char texture_id;
    unsigned short free_things[THINGS_COUNT];
    unsigned long seedchk_random_used;
    unsigned long pckt_gameturn;
    unsigned long field_14BB4A;
    unsigned long rand_14BB4E; // primary random seed; shouldn't affect game actions
short field_14BB52;
unsigned char field_14BB54;
int field_14BB55;
int field_14BB59;
int field_14BB5D;
    unsigned long time_delta;
short field_14BB65[592];
    unsigned char small_map_state;
char field_14C006[4];
short field_14C00A;
    struct Packet packets[PACKETS_COUNT];
    struct CreatureStats creature_stats[CREATURE_TYPES_COUNT];
short field_14DD21[34];
struct MagicStats magic_stats[SPELL_TYPES_COUNT];
    struct ActionPoint action_points[ACTN_POINTS_COUNT];
char field_14E495;
    unsigned char field_14E496;
    unsigned char field_14E497;
int field_14E498;
short field_14E49C;
short field_14E49E;
int field_14E4A0;
short field_14E4A4;
    struct GoldLookup gold_lookup[GOLD_LOOKUP_COUNT];
unsigned short field_14E906;
    unsigned short block_health[9];
    unsigned short minimum_gold;
    unsigned short max_gold_lookup;
    unsigned short min_gold_to_record;
    unsigned short wait_for_room_time;
    unsigned short check_expand_time;
    unsigned short max_distance_to_dig;
    unsigned short wait_after_room_area;
    unsigned short per_imp_gold_dig_size;
    unsigned short default_generate_speed;
unsigned short field_14E92C;
unsigned short field_14E92E;
unsigned char field_14E930[4];
long field_14E934;
unsigned short field_14E938;
unsigned short field_14E93A;
    unsigned short gold_per_gold_block;
    unsigned short pot_of_gold_holds;
    unsigned short chest_gold_hold;
    unsigned short gold_pile_value;
    unsigned short gold_pile_maximum;
    unsigned short fight_max_hate;
    unsigned short fight_borderline;
    unsigned short fight_max_love;
    unsigned short food_life_out_of_hatchery;
    unsigned short fight_hate_kill_value;
    unsigned short body_remains_for;
    unsigned short graveyard_convert_time;
    unsigned short tile_strength;
    unsigned short gold_tile_strength;
unsigned char field_14E958[14];
char field_14E966[128];
char field_14E9E6[64];
unsigned char field_14EA26[28];
    unsigned char min_distance_for_teleport;
    unsigned char recovery_frequency;
unsigned short field_14EA44;
unsigned short field_14EA46;
    unsigned short food_generation_speed;
char flagfield_14EA4A;
char field_14EA4B;
char field_14EA4C[320];
    unsigned long default_max_crtrs_gen_entrance;
    unsigned long default_imp_dig_damage;
    unsigned long default_imp_dig_own_damage;
    unsigned short game_turns_in_flee;
    unsigned short hunger_health_loss;
    unsigned short turns_per_hunger_health_loss;
    unsigned short food_health_gain;
    unsigned char prison_skeleton_chance;
    unsigned short torture_health_loss;
    unsigned short turns_per_torture_health_loss;
    unsigned char ghost_convert_chance;
    struct LevelScript script;
    struct Bookmark bookmark[5];
    struct CreaturePool pool;
    long frame_skip;
    unsigned long pay_day_gap;
unsigned long field_15033A;
    unsigned long power_hand_gold_grab_amount;
    unsigned char no_intro;
    unsigned long game_hero_door_wait_time;
    unsigned long dungeon_heart_heal_time;
    long dungeon_heart_heal_health;
    unsigned long dungeon_heart_health;
    unsigned char disease_transfer_percentage;
    unsigned char disease_lose_percentage_health;
    unsigned char disease_lose_health_time;
unsigned long field_150356;
unsigned long field_15035A;
unsigned char field_15035E;
    unsigned long hold_audience_time;
    unsigned long armagedon_teleport_your_time_gap;
    unsigned long armagedon_teleport_enemy_time_gap;
    unsigned char hits_per_slab;
    long collapse_dungeon_damage;
    unsigned long turns_per_collapse_dngn_dmg;
    struct SoundSettings sound_settings;
long field_15038E;
    long num_fps;
    unsigned long train_cost_frequency;
    unsigned long scavenge_cost_frequency;
    unsigned long temple_scavenge_protection_time;
char numfield_1503A2;
    unsigned char bodies_for_vampire;
    struct UnkStruc7 field_1503A4[48];
unsigned char field_1506D4;
long field_1506D5;
    char evntbox_text_objective[MESSAGE_TEXT_LEN];
    char evntbox_text_buffer[MESSAGE_TEXT_LEN];
    char evntbox_text_shown[MESSAGE_TEXT_LEN];
int field_1512D9;
char field_1512DD;
int field_1512DE;
int field_1512E2;
char field_1512E6[1037];
int field_1516F3;
int field_1516F7; // signed
int field_1516FB;
char field_1516FF;
    long boulder_reduce_health_wall;
    long boulder_reduce_health_slap;
    long boulder_reduce_health_room;
    struct GuiMessage messages[GUI_MESSAGES_COUNT];
    unsigned char active_messages_count;
    unsigned char bonus_levels[6];
int field_1517E2;
    unsigned char transfered_creature_kind;
    unsigned char transfered_creature_level;
    struct Armageddon armageddon;
char field_1517F6;
char field_1517F7;
char field_1517F8;
char field_1517F9;
char field_1517FA;
char field_1517FB;
char field_1517FC;
short field_1517FD;
short field_1517FF;
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

struct TbNetworkPlayerInfo {
char field_0[32];
long field_20;
};

struct TbNetworkCallbackData {
char field_0[20];
};

struct SerialInitData {
long field_0;
    long numfield_4;
    long field_8;
long field_C;
    char *str_phone;
    char *str_dial;
    char *str_hang;
    char *str_answr;
};

struct ClientDataEntry {
long field_0;
long field_4;
long field_8;
long field_C;
char field_10[28];
};

struct ReceiveCallbacks {
       void (*add_msg)(unsigned long, char *, void *);
       void (*delete_msg)(unsigned long, void *);
       void (*host_msg)(unsigned long, void *);
       void *unkn1;
       void * __stdcall (*multi_player)(unsigned long, unsigned long, unsigned long, void *);
       void __stdcall (*mp_req_exdata_msg)(unsigned long, unsigned long, void *);
       void (*req_compos_exchngdat_msg)(unsigned long, unsigned long, void *);
       void * (*unidirectional_msg)(unsigned long, unsigned long, void *);
       void (*sys_user_msg)(unsigned long, void *, unsigned long, void *);
};

typedef long (*Thing_Class_Func)(struct Thing *);
typedef void (*RotPers_Func)(struct EngineCoord *epos, struct M33 *matx);
typedef void (*Perspect_Func)(struct XYZ *cor, struct PolyPoint *ppt);


// Global variables migration between DLL and the program

DLLIMPORT extern HINSTANCE _DK_hInstance;
DLLIMPORT extern TDDrawBaseClass *_DK_lpDDC;
#define lpDDC _DK_lpDDC
DLLIMPORT extern struct Game _DK_game;
#define game _DK_game
DLLIMPORT extern unsigned char _DK_my_player_number;
#define my_player_number _DK_my_player_number
DLLIMPORT extern unsigned char *_DK_blue_palette;
#define blue_palette _DK_blue_palette
DLLIMPORT extern struct TbLoadFiles _DK_game_load_files[];
#define game_load_files _DK_game_load_files
DLLIMPORT extern struct GameSettings _DK_settings;
#define settings _DK_settings
DLLIMPORT extern unsigned char _DK_exit_keeper;
#define exit_keeper _DK_exit_keeper
DLLIMPORT extern unsigned char _DK_quit_game;
#define quit_game _DK_quit_game
DLLIMPORT extern int _DK_number_of_saved_games;
#define number_of_saved_games _DK_number_of_saved_games
DLLIMPORT extern int _DK_lbUseSdk;
#define lbUseSdk _DK_lbUseSdk
DLLIMPORT extern unsigned char _DK_frontend_palette[768];
#define frontend_palette _DK_frontend_palette
DLLIMPORT extern int _DK_continue_game_option_available;
#define continue_game_option_available _DK_continue_game_option_available
DLLIMPORT extern long _DK_last_mouse_x;
#define last_mouse_x _DK_last_mouse_x
DLLIMPORT extern long _DK_last_mouse_y;
#define last_mouse_y _DK_last_mouse_y
DLLIMPORT extern int _DK_credits_end;
#define credits_end _DK_credits_end
DLLIMPORT extern long _DK_frontstory_text_no;
#define frontstory_text_no _DK_frontstory_text_no
DLLIMPORT extern int _DK_FatalError;
#define FatalError _DK_FatalError
DLLIMPORT extern int _DK_net_service_index_selected;
#define net_service_index_selected _DK_net_service_index_selected
DLLIMPORT extern unsigned char _DK_fade_palette_in;
#define fade_palette_in _DK_fade_palette_in
DLLIMPORT extern long _DK_old_mouse_over_button;
#define old_mouse_over_button _DK_old_mouse_over_button
DLLIMPORT extern long _DK_frontend_mouse_over_button;
#define frontend_mouse_over_button _DK_frontend_mouse_over_button
DLLIMPORT extern struct TbLoadFiles _DK_frontstory_load_files[4];
#define frontstory_load_files _DK_frontstory_load_files
DLLIMPORT extern struct TbLoadFiles _DK_netmap_flag_load_files[7];
#define netmap_flag_load_files _DK_netmap_flag_load_files
DLLIMPORT extern int _DK_fe_high_score_table_from_main_menu;
#define fe_high_score_table_from_main_menu _DK_fe_high_score_table_from_main_menu
DLLIMPORT extern long _DK_define_key_scroll_offset;
#define define_key_scroll_offset _DK_define_key_scroll_offset
DLLIMPORT extern struct TbSetupSprite _DK_frontstory_setup_sprites[2];
#define frontstory_setup_sprites _DK_frontstory_setup_sprites
DLLIMPORT extern unsigned long _DK_time_last_played_demo;
#define time_last_played_demo _DK_time_last_played_demo
DLLIMPORT extern unsigned short _DK_battle_creature_over;
#define battle_creature_over _DK_battle_creature_over
DLLIMPORT extern char _DK_gui_room_type_highlighted;
#define gui_room_type_highlighted _DK_gui_room_type_highlighted
DLLIMPORT extern char _DK_gui_door_type_highlighted;
#define gui_door_type_highlighted _DK_gui_door_type_highlighted
DLLIMPORT extern char _DK_gui_trap_type_highlighted;
#define gui_trap_type_highlighted _DK_gui_trap_type_highlighted
DLLIMPORT extern char _DK_gui_creature_type_highlighted;
#define gui_creature_type_highlighted _DK_gui_creature_type_highlighted
DLLIMPORT extern short _DK_drag_menu_x;
#define drag_menu_x _DK_drag_menu_x
DLLIMPORT extern short _DK_drag_menu_y;
#define drag_menu_y _DK_drag_menu_y
DLLIMPORT extern char _DK_busy_doing_gui;
#define busy_doing_gui _DK_busy_doing_gui
DLLIMPORT extern unsigned short _DK_tool_tip_time;
#define tool_tip_time _DK_tool_tip_time
DLLIMPORT extern unsigned short _DK_help_tip_time;
#define help_tip_time _DK_help_tip_time
DLLIMPORT extern long _DK_gui_last_left_button_pressed_id;
#define gui_last_left_button_pressed_id _DK_gui_last_left_button_pressed_id
DLLIMPORT extern long _DK_gui_last_right_button_pressed_id;
#define gui_last_right_button_pressed_id _DK_gui_last_right_button_pressed_id
DLLIMPORT extern long _DK_tooltip_scroll_offset;
#define tooltip_scroll_offset _DK_tooltip_scroll_offset
DLLIMPORT extern long _DK_tooltip_scroll_timer;
#define tooltip_scroll_timer _DK_tooltip_scroll_timer
DLLIMPORT extern long _DK_map_to_slab[256];
#define map_to_slab _DK_map_to_slab
DLLIMPORT extern struct TrapData _DK_trap_data[TRAP_TYPES_COUNT];
#define trap_data _DK_trap_data
DLLIMPORT extern struct RoomData _DK_room_data[ROOM_TYPES_COUNT];
#define room_data _DK_room_data
DLLIMPORT extern struct SpellData _DK_spell_data[SPELL_TYPES_COUNT+1];
#define spell_data _DK_spell_data
DLLIMPORT extern struct SlabAttr _DK_slab_attrs[SLAB_TYPES_COUNT];
#define slab_attrs _DK_slab_attrs
DLLIMPORT extern unsigned char _DK_object_to_special[OBJECT_TYPES_COUNT];
#define object_to_special _DK_object_to_special
DLLIMPORT extern unsigned char _DK_object_to_magic[OBJECT_TYPES_COUNT];
#define object_to_magic _DK_object_to_magic
DLLIMPORT extern unsigned char _DK_workshop_object_class[OBJECT_TYPES_COUNT];
#define workshop_object_class _DK_workshop_object_class
DLLIMPORT extern unsigned char _DK_object_to_door_or_trap[OBJECT_TYPES_COUNT];
#define object_to_door_or_trap _DK_object_to_door_or_trap
DLLIMPORT extern unsigned char _DK_magic_to_object[24];
#define magic_to_object _DK_magic_to_object
DLLIMPORT extern unsigned char _DK_trap_to_object[8];
#define trap_to_object _DK_trap_to_object
DLLIMPORT extern unsigned char _DK_door_to_object[DOOR_TYPES_COUNT];
#define door_to_object _DK_door_to_object
DLLIMPORT extern unsigned short _DK_specials_text[10];
#define specials_text _DK_specials_text
DLLIMPORT extern unsigned short _DK_door_names[DOOR_TYPES_COUNT];
#define door_names _DK_door_names
DLLIMPORT extern struct CreatureData _DK_creature_data[CREATURE_TYPES_COUNT];
#define creature_data _DK_creature_data
DLLIMPORT extern struct Objects _DK_objects[135];
DLLIMPORT extern unsigned char _DK_eastegg03_cntr;
#define eastegg03_cntr _DK_eastegg03_cntr
DLLIMPORT extern long _DK_pointer_x;
#define pointer_x _DK_pointer_x
DLLIMPORT extern long _DK_pointer_y;
#define pointer_y _DK_pointer_y
DLLIMPORT extern long _DK_block_pointed_at_x;
#define block_pointed_at_x _DK_block_pointed_at_x
DLLIMPORT extern long _DK_block_pointed_at_y;
#define block_pointed_at_y _DK_block_pointed_at_y
DLLIMPORT extern long _DK_pointed_at_frac_x;
#define pointed_at_frac_x _DK_pointed_at_frac_x
DLLIMPORT extern long _DK_pointed_at_frac_y;
#define pointed_at_frac_y _DK_pointed_at_frac_y
DLLIMPORT extern long _DK_top_pointed_at_x;
#define top_pointed_at_x _DK_top_pointed_at_x
DLLIMPORT extern long _DK_top_pointed_at_y;
#define top_pointed_at_y _DK_top_pointed_at_y
DLLIMPORT extern long _DK_top_pointed_at_frac_x;
#define top_pointed_at_frac_x _DK_top_pointed_at_frac_x
DLLIMPORT extern long _DK_top_pointed_at_frac_y;
#define top_pointed_at_frac_y _DK_top_pointed_at_frac_y
DLLIMPORT struct ScreenPacket _DK_net_screen_packet;
#define net_screen_packet _DK_net_screen_packet
DLLIMPORT struct _GUID _DK_net_guid;
#define net_guid _DK_net_guid
//DLLIMPORT struct PlayerInstanceInfo _DK_player_instance_info[PLAYER_INSTANCES_COUNT];
//#define player_instance_info _DK_player_instance_info
DLLIMPORT struct TbNetworkPlayerInfo _DK_net_player_info[NET_PLAYERS_COUNT];
#define net_player_info _DK_net_player_info
DLLIMPORT struct TbNetworkCallbackData _DK_enum_players_callback[NET_PLAYERS_COUNT];
#define enum_players_callback _DK_enum_players_callback
DLLIMPORT struct SerialInitData _DK_net_serial_data;
#define net_serial_data _DK_net_serial_data
DLLIMPORT struct SerialInitData _DK_net_modem_data;
#define net_modem_data _DK_net_modem_data
DLLIMPORT char _DK_tmp_net_player_name[24];
#define tmp_net_player_name _DK_tmp_net_player_name
DLLIMPORT char _DK_tmp_net_phone_number[24];
#define tmp_net_phone_number _DK_tmp_net_phone_number
DLLIMPORT char _DK_tmp_net_modem_init[20];
#define tmp_net_modem_init _DK_tmp_net_modem_init
DLLIMPORT char _DK_tmp_net_modem_dial[20];
#define tmp_net_modem_dial _DK_tmp_net_modem_dial
DLLIMPORT char _DK_tmp_net_modem_hangup[20];
#define tmp_net_modem_hangup _DK_tmp_net_modem_hangup
DLLIMPORT char _DK_tmp_net_modem_answer[20];
#define tmp_net_modem_answer _DK_tmp_net_modem_answer
DLLIMPORT int _DK_fe_network_active;
#define fe_network_active _DK_fe_network_active
DLLIMPORT char _DK_video_shadows;
#define video_shadows _DK_video_shadows
DLLIMPORT char _DK_video_view_distance_level;
#define video_view_distance_level _DK_video_view_distance_level
DLLIMPORT char _DK_video_cluedo_mode;
DLLIMPORT long _DK_sound_level;
#define sound_level _DK_sound_level
DLLIMPORT long _DK_music_level;
#define music_level _DK_music_level
DLLIMPORT long _DK_fe_mouse_sensitivity;
#define fe_mouse_sensitivity _DK_fe_mouse_sensitivity
DLLIMPORT struct InstanceInfo _DK_instance_info[48];
#define instance_info _DK_instance_info
DLLIMPORT long _DK_activity_list[24];
#define activity_list _DK_activity_list
DLLIMPORT char _DK_net_service[16][64];
#define net_service _DK_net_service
DLLIMPORT void *_DK_exchangeBuffer;
#define exchangeBuffer _DK_exchangeBuffer
DLLIMPORT long _DK_exchangeSize;
#define exchangeSize _DK_exchangeSize
DLLIMPORT int _DK_maximumPlayers;
#define maximumPlayers _DK_maximumPlayers
DLLIMPORT struct TbNetworkPlayerInfo *_DK_localPlayerInfoPtr;
#define localPlayerInfoPtr _DK_localPlayerInfoPtr
DLLIMPORT void *_DK_localDataPtr;
#define localDataPtr _DK_localDataPtr
DLLIMPORT void *_DK_compositeBuffer;
#define compositeBuffer _DK_compositeBuffer
DLLIMPORT long _DK_sequenceNumber;
#define sequenceNumber _DK_sequenceNumber
DLLIMPORT long _DK_timeCount;
#define timeCount _DK_timeCount
DLLIMPORT long _DK_maxTime;
#define maxTime _DK_maxTime
DLLIMPORT int _DK_runningTwoPlayerModel;
#define runningTwoPlayerModel _DK_runningTwoPlayerModel
DLLIMPORT int _DK_waitingForPlayerMapResponse;
#define waitingForPlayerMapResponse _DK_waitingForPlayerMapResponse
DLLIMPORT long _DK_compositeBufferSize;
#define compositeBufferSize _DK_compositeBufferSize
DLLIMPORT long _DK_basicTimeout;
#define basicTimeout _DK_basicTimeout
DLLIMPORT int _DK_noOfEnumeratedDPlayServices;
#define noOfEnumeratedDPlayServices _DK_noOfEnumeratedDPlayServices
DLLIMPORT struct ClientDataEntry _DK_clientDataTable[32];
#define clientDataTable _DK_clientDataTable
DLLIMPORT struct ReceiveCallbacks _DK_receiveCallbacks;
#define receiveCallbacks _DK_receiveCallbacks
DLLIMPORT unsigned long _DK_hostId;
#define hostId _DK_hostId
DLLIMPORT unsigned long _DK_localPlayerId;
#define localPlayerId _DK_localPlayerId
DLLIMPORT int _DK_gotCompositeData;
#define gotCompositeData _DK_gotCompositeData
DLLIMPORT struct _GUID _DK_clientGuidTable[];
#define clientGuidTable _DK_clientGuidTable
DLLIMPORT long _DK_anim_counter;
#define anim_counter _DK_anim_counter
DLLIMPORT long _DK_block_ptrs[592];
#define block_ptrs _DK_block_ptrs
DLLIMPORT unsigned char _DK_colours[16][16][16];
#define colours _DK_colours
DLLIMPORT long _DK_frame_number;
#define frame_number _DK_frame_number
DLLIMPORT unsigned char _DK_grabbed_small_map;
#define grabbed_small_map _DK_grabbed_small_map
DLLIMPORT long _DK_draw_spell_cost;
#define draw_spell_cost _DK_draw_spell_cost
DLLIMPORT int _DK_parchment_loaded;
#define parchment_loaded _DK_parchment_loaded
DLLIMPORT char _DK_level_name[88];
#define level_name _DK_level_name
DLLIMPORT unsigned char _DK_poly_pool[0x40000];
#define poly_pool _DK_poly_pool
DLLIMPORT unsigned char *_DK_hires_parchment;
#define hires_parchment _DK_hires_parchment
DLLIMPORT struct NetMessage _DK_net_message[8];
#define net_message _DK_net_message
DLLIMPORT int _DK_fe_computer_players;
#define fe_computer_players _DK_fe_computer_players
DLLIMPORT long _DK_resurrect_creature_scroll_offset;
#define resurrect_creature_scroll_offset _DK_resurrect_creature_scroll_offset
DLLIMPORT unsigned short _DK_dungeon_special_selected;
#define dungeon_special_selected _DK_dungeon_special_selected
DLLIMPORT long _DK_transfer_creature_scroll_offset;
#define transfer_creature_scroll_offset _DK_transfer_creature_scroll_offset
DLLIMPORT struct SpecialDesc _DK_special_desc[8];
#define special_desc _DK_special_desc
DLLIMPORT unsigned char *_DK_block_mem;
#define block_mem _DK_block_mem
DLLIMPORT struct Room *_DK_start_rooms;
#define start_rooms _DK_start_rooms
DLLIMPORT struct Room *_DK_end_rooms;
#define end_rooms _DK_end_rooms
DLLIMPORT unsigned char _DK_smooth_on;
#define smooth_on _DK_smooth_on
DLLIMPORT struct BigSprite _DK_status_panel;
#define status_panel _DK_status_panel
DLLIMPORT unsigned char _DK_ghost[256*16];
#define ghost _DK_ghost
DLLIMPORT unsigned char _DK_fade_tables[256*64];
#define fade_tables _DK_fade_tables
DLLIMPORT unsigned short _DK_breed_activities[CREATURE_TYPES_COUNT];
#define breed_activities _DK_breed_activities
DLLIMPORT char _DK_top_of_breed_list;
#define top_of_breed_list _DK_top_of_breed_list
DLLIMPORT char _DK_no_of_breeds_owned;
#define no_of_breeds_owned _DK_no_of_breeds_owned
DLLIMPORT long _DK_optimised_lights;
#define optimised_lights _DK_optimised_lights
DLLIMPORT long _DK_total_lights;
#define total_lights _DK_total_lights
DLLIMPORT unsigned char _DK_do_lights;
#define do_lights _DK_do_lights
DLLIMPORT struct MessageQueueEntry _DK_message_queue[MESSAGE_QUEUE_COUNT];
#define message_queue _DK_message_queue
DLLIMPORT struct TrapStats _DK_trap_stats[7]; //not sure - maybe it's 8?
#define trap_stats _DK_trap_stats
//DLLIMPORT Thing_Class_Func _DK_class_functions[14];
//#define class_functions _DK_class_functions
DLLIMPORT long _DK_imp_spangle_effects[];
#define imp_spangle_effects _DK_imp_spangle_effects
DLLIMPORT struct Creatures _DK_creatures[CREATURE_TYPES_COUNT];
#define creatures _DK_creatures
DLLIMPORT unsigned char _DK_engine_player_number;
#define engine_player_number _DK_engine_player_number
DLLIMPORT unsigned char _DK_player_bit;
#define player_bit _DK_player_bit
DLLIMPORT unsigned char _DK_temp_cluedo_mode;
#define temp_cluedo_mode _DK_temp_cluedo_mode
DLLIMPORT unsigned char _DK_lens_mode;
#define lens_mode _DK_lens_mode
DLLIMPORT long _DK_split1at;
#define split1at _DK_split1at
DLLIMPORT long _DK_split2at;
#define split2at _DK_split2at
DLLIMPORT long _DK_max_i_can_see;
#define max_i_can_see _DK_max_i_can_see
DLLIMPORT long _DK_view_height_over_2;
#define view_height_over_2 _DK_view_height_over_2
DLLIMPORT long _DK_view_width_over_2;
#define view_width_over_2 _DK_view_width_over_2
DLLIMPORT long _DK_me_distance;
#define me_distance _DK_me_distance
DLLIMPORT long _DK_lens;
#define lens _DK_lens
DLLIMPORT short _DK_mx;
#define mx _DK_mx
DLLIMPORT short _DK_my;
#define my _DK_my
DLLIMPORT short _DK_mz;
#define mz _DK_mz
DLLIMPORT struct Thing *_DK_thing_being_displayed;
#define thing_being_displayed _DK_thing_being_displayed
DLLIMPORT struct Thing *_DK_thing_pointed_at;
#define thing_pointed_at _DK_thing_pointed_at
DLLIMPORT struct Map *_DK_me_pointed_at;
#define me_pointed_at _DK_me_pointed_at
DLLIMPORT Offset _DK_vert_offset[3];
#define vert_offset _DK_vert_offset
DLLIMPORT Offset _DK_hori_offset[3];
#define hori_offset _DK_hori_offset
DLLIMPORT Offset _DK_high_offset[3];
#define high_offset _DK_high_offset
DLLIMPORT long _DK_x_init_off;
#define x_init_off _DK_x_init_off
DLLIMPORT long _DK_y_init_off;
#define y_init_off _DK_y_init_off
DLLIMPORT long _DK_floor_pointed_at_x;
#define floor_pointed_at_x _DK_floor_pointed_at_x
DLLIMPORT long _DK_floor_pointed_at_y;
#define floor_pointed_at_y _DK_floor_pointed_at_y
DLLIMPORT unsigned char *_DK_getpoly;
#define getpoly _DK_getpoly
DLLIMPORT long _DK_cells_away;
#define cells_away _DK_cells_away
DLLIMPORT long _DK_fade_max;
#define fade_max _DK_fade_max
DLLIMPORT long _DK_fade_scaler;
#define fade_scaler _DK_fade_scaler
DLLIMPORT long _DK_fade_way_out;
#define fade_way_out _DK_fade_way_out
DLLIMPORT int _DK_normal_shade_front;
#define normal_shade_front _DK_normal_shade_front
DLLIMPORT int _DK_normal_shade_back;
#define normal_shade_back _DK_normal_shade_back
DLLIMPORT long _DK_map_angle;
#define map_angle _DK_map_angle
DLLIMPORT long _DK_map_roll;
#define map_roll _DK_map_roll
DLLIMPORT long _DK_map_tilt;
#define map_tilt _DK_map_tilt
DLLIMPORT long _DK_view_alt;
#define view_alt _DK_view_alt
DLLIMPORT long _DK_fade_min;
#define fade_min _DK_fade_min
DLLIMPORT long _DK_split_1;
#define split_1 _DK_split_1
DLLIMPORT long _DK_split_2;
#define split_2 _DK_split_2
DLLIMPORT long _DK_fade_range;
#define fade_range _DK_fade_range
DLLIMPORT long _DK_depth_init_off;
#define depth_init_off _DK_depth_init_off
DLLIMPORT int _DK_normal_shade_left;
#define normal_shade_left _DK_normal_shade_left
DLLIMPORT int _DK_normal_shade_right;
#define normal_shade_right _DK_normal_shade_right
DLLIMPORT long _DK_apos;
#define apos _DK_apos
DLLIMPORT long _DK_bpos;
#define bpos _DK_bpos
//DLLIMPORT RotPers_Func _DK_rotpers_routines[];
//#define rotpers_routines _DK_rotpers_routines
//DLLIMPORT Perspect_Func _DK_perspective_routines[];
//#define perspective_routines _DK_perspective_routines
DLLIMPORT Perspect_Func _DK_perspective;
#define perspective _DK_perspective
DLLIMPORT RotPers_Func _DK_rotpers;
#define rotpers _DK_rotpers
DLLIMPORT struct BasicQ *_DK_buckets[];
#define buckets _DK_buckets
DLLIMPORT struct M33 _DK_camera_matrix;
#define camera_matrix _DK_camera_matrix
DLLIMPORT struct MapVolumeBox _DK_map_volume_box;
#define map_volume_box _DK_map_volume_box
DLLIMPORT struct EngineCoord _DK_object_origin;
#define object_origin _DK_object_origin
DLLIMPORT struct EngineCol _DK_ecs1[];
#define ecs1 _DK_ecs1
DLLIMPORT struct EngineCol _DK_ecs2[];
#define ecs2 _DK_ecs2
DLLIMPORT struct EngineCol *_DK_front_ec;
#define front_ec _DK_front_ec
DLLIMPORT struct EngineCol *_DK_back_ec;
#define back_ec _DK_back_ec
DLLIMPORT struct MinMax _DK_minmaxs[];
#define minmaxs _DK_minmaxs
DLLIMPORT unsigned long *_DK_eye_lens_memory;
#define eye_lens_memory _DK_eye_lens_memory
DLLIMPORT TbPixel *_DK_eye_lens_spare_screen_memory;
#define eye_lens_spare_screen_memory _DK_eye_lens_spare_screen_memory
DLLIMPORT unsigned char *_DK_dog_palette;
#define dog_palette _DK_dog_palette
DLLIMPORT unsigned char *_DK_vampire_palette;
#define vampire_palette _DK_vampire_palette
DLLIMPORT long _DK_ScrCenterX;
#define ScrCenterX _DK_ScrCenterX
DLLIMPORT long _DK_ScrWidth;
#define ScrWidth _DK_ScrWidth
DLLIMPORT long _DK_ScrHeight;
#define ScrHeight _DK_ScrHeight
DLLIMPORT long _DK_ScrCenterY;
#define ScrCenterY _DK_ScrCenterY
DLLIMPORT struct CScan *_DK_ScanBuffer;
#define ScanBuffer _DK_ScanBuffer
DLLIMPORT unsigned char _DK_vec_mode;
#define vec_mode _DK_vec_mode
DLLIMPORT unsigned char _DK_vec_colour;
#define vec_colour _DK_vec_colour
DLLIMPORT long _DK_my_mouse_x;
#define my_mouse_x _DK_my_mouse_x
DLLIMPORT long _DK_my_mouse_y;
#define my_mouse_y _DK_my_mouse_y
DLLIMPORT unsigned char _DK_gui_slab;
#define gui_slab _DK_gui_slab
DLLIMPORT char *_DK_level_names_data;
#define level_names_data _DK_level_names_data
DLLIMPORT char *_DK_end_level_names_data;
#define end_level_names_data _DK_end_level_names_data
DLLIMPORT unsigned char _DK_frontend_backup_palette[768];
#define frontend_backup_palette _DK_frontend_backup_palette
DLLIMPORT long _DK_dummy_x;
#define dummy_x _DK_dummy_x
DLLIMPORT long _DK_dummy_y;
#define dummy_y _DK_dummy_y
DLLIMPORT long _DK_dummy;
#define dummy _DK_dummy
DLLIMPORT long _DK_clicked_on_small_map;
#define clicked_on_small_map _DK_clicked_on_small_map
DLLIMPORT long _DK_old_mx;
#define old_mx _DK_old_mx
DLLIMPORT long _DK_old_my;
#define old_my _DK_old_my
DLLIMPORT unsigned char _DK_zoom_to_heart_palette[768];
#define zoom_to_heart_palette _DK_zoom_to_heart_palette
DLLIMPORT struct MapOffset _DK_spiral_step[2500];
#define spiral_step _DK_spiral_step
DLLIMPORT short _DK_td_iso[TD_ISO_POINTS];
#define td_iso _DK_td_iso
DLLIMPORT short _DK_iso_td[TD_ISO_POINTS];
#define iso_td _DK_iso_td
DLLIMPORT unsigned short _DK_creature_list[CREATURE_FRAMELIST_LENGTH];
#define creature_list _DK_creature_list
DLLIMPORT struct KeeperSprite *_DK_creature_table;
#define creature_table _DK_creature_table
DLLIMPORT long _DK_stat_light_needs_updating;
#define stat_light_needs_updating _DK_stat_light_needs_updating
DLLIMPORT long _DK_light_bitmask[32];
#define light_bitmask _DK_light_bitmask

#pragma pack()

// Variables inside the main module
extern TbClockMSec last_loop_time;
extern int map_subtiles_x;
extern int map_subtiles_y;
extern int map_tiles_x;
extern int map_tiles_y;
extern short default_loc_player;
extern struct GuiBox *gui_box;
extern struct GuiBox *gui_cheat_box;
extern struct PlayerInstanceInfo player_instance_info[];

extern Perspect_Func perspective_routines[];
extern RotPers_Func rotpers_routines[];
extern int test_variable;
extern unsigned short const player_cubes[];

//Functions - exported by the DLL

DLLIMPORT int __stdcall _DK_WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd);
DLLIMPORT int __stdcall _DK_LbBullfrogMain(unsigned short argc,char **argv);
DLLIMPORT int __stdcall _DK_game_loop(void);
DLLIMPORT int __stdcall _DK_LbErrorLogSetup(char *directory, char *filename, unsigned char flag);
DLLIMPORT void __fastcall _DK_get_cpu_info(struct CPU_INFO *cpu_info);
DLLIMPORT int __stdcall _DK_SyncLog(char *Format, ...);
DLLIMPORT void __cdecl _DK_set_cpu_mode(int mode);
DLLIMPORT int __cdecl _DK_setup_game_sound_heap(void);
DLLIMPORT void __cdecl _DK_input_eastegg(void);
DLLIMPORT void __cdecl _DK_update(void);
DLLIMPORT void __cdecl _DK_wait_at_frontend(void);
DLLIMPORT void __cdecl _DK_delete_all_structures(void);
DLLIMPORT void __cdecl _DK_PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *palette);
DLLIMPORT void __cdecl _DK_initialise_eye_lenses(void);
DLLIMPORT void __cdecl _DK_setup_eye_lens(long);

DLLIMPORT void __stdcall _DK_IsRunningMark(void);
DLLIMPORT void __stdcall _DK_IsRunningUnmark(void);
DLLIMPORT int __stdcall _DK_LbMouseSuspend(void);
DLLIMPORT int __stdcall _DK_LbMemoryFree(void *buffer);
DLLIMPORT int __stdcall _DK_LbMemoryReset(void);
DLLIMPORT int __stdcall _DK_play_smk_(char *fname, int smkflags, int plyflags);
DLLIMPORT int __fastcall _DK_LbFileClose(TbFileHandle handle);
DLLIMPORT void __cdecl _DK_setup_engine_window(long, long, long, long);
DLLIMPORT void __cdecl _DK_redraw_display(void);
DLLIMPORT void __cdecl _DK_cumulative_screen_shot(void);
DLLIMPORT long __cdecl _DK_anim_record_frame(unsigned char *screenbuf, unsigned char *palette);
DLLIMPORT void __cdecl _DK_frontend_set_state(long);
DLLIMPORT void __cdecl _DK_frontnet_service_update(void);
DLLIMPORT void __cdecl _DK_frontnet_session_update(void);
DLLIMPORT void __cdecl _DK_frontnet_start_update(void);
DLLIMPORT void __cdecl _DK_frontnet_modem_update(void);
DLLIMPORT void __cdecl _DK_frontnet_serial_update(void);
//DLLIMPORT void * __cdecl _DK_frontnet_session_join(GuiButton); (may be incorrect)
DLLIMPORT void __cdecl _DK_demo(void);
DLLIMPORT void __cdecl _DK_draw_gui(void);
DLLIMPORT void __cdecl _DK_save_settings(void);
DLLIMPORT int __cdecl _DK_setup_network_service(int srvidx);
DLLIMPORT int __cdecl _DK_process_3d_sounds(void);
DLLIMPORT char *_DK_mdlf_for_cd(struct TbLoadFiles *);
DLLIMPORT char *_DK_mdlf_default(struct TbLoadFiles *);
DLLIMPORT int _DK_LbSpriteSetupAll(struct TbSetupSprite t_setup[]);
DLLIMPORT void _DK_update_breed_activities(void);
DLLIMPORT void _DK_maintain_my_battle_list(void);
DLLIMPORT struct Thing *_DK_get_special_at_position(long x, long y);
DLLIMPORT struct Thing *_DK_get_spellbook_at_position(long x, long y);
DLLIMPORT struct Thing *_DK_get_crate_at_position(long x, long y);
DLLIMPORT struct Thing *_DK_get_nearest_object_at_position(long x, long y);
DLLIMPORT TbError _DK_LbNetwork_Stop(void);
DLLIMPORT TbError _DK_LbNetwork_ChangeExchangeBuffer(void *, unsigned long);
DLLIMPORT void _DK_turn_off_menu(char mnu_idx);
DLLIMPORT void _DK_turn_off_all_panel_menus(void);
DLLIMPORT void _DK_process_network_error(long);
DLLIMPORT int _DK_play_smk_via_buffer(char *fname, int smkflags, int plyflags);
DLLIMPORT int _DK_play_smk_direct(char *fname, int smkflags, int plyflags);
DLLIMPORT TbError _DK_LbNetwork_Init(unsigned long,struct _GUID guid, unsigned long, void *, unsigned long, struct TbNetworkPlayerInfo *netplayr, void *);
DLLIMPORT void _DK_process_rooms(void);
DLLIMPORT void _DK_process_dungeons(void);
DLLIMPORT void _DK_process_messages(void);
DLLIMPORT void _DK_find_nearest_rooms_for_ambient_sound(void);
DLLIMPORT void __cdecl _DK_light_render_area(int startx, int starty, int endx, int endy);
DLLIMPORT void _DK_process_player_research(int plr_idx);
DLLIMPORT long _DK_process_player_manufacturing(int plr_idx);
DLLIMPORT void _DK_event_process_events(void);
DLLIMPORT void _DK_update_all_events(void);
DLLIMPORT void _DK_process_level_script(void);
DLLIMPORT long _DK_process_action_points(void);
DLLIMPORT long _DK_PaletteFadePlayer(struct PlayerInfo *player);
DLLIMPORT void _DK_message_update(void);
DLLIMPORT long _DK_wander_point_update(struct Wander *wandr);
DLLIMPORT void __cdecl _DK_update_player_camera(struct PlayerInfo *player);
DLLIMPORT void _DK_set_level_objective(char *msg_text);
DLLIMPORT void _DK_update_flames_nearest_camera(struct Camera *camera);
DLLIMPORT void _DK_update_footsteps_nearest_camera(struct Camera *camera);
DLLIMPORT void _DK_process_player_states(void);
DLLIMPORT void _DK_process_armageddon(void);  //not sure about the name!
DLLIMPORT void _DK_set_mouse_light(struct PlayerInfo *player);
DLLIMPORT void _DK_process_pointer_graphic(void);
DLLIMPORT void _DK_message_draw(void);
DLLIMPORT void _DK_draw_slab64k(long pos_x, long pos_y, long width, long height);
DLLIMPORT void _DK_redraw_creature_view(void);
DLLIMPORT void _DK_redraw_isometric_view(void);
DLLIMPORT void _DK_redraw_frontview(void);
DLLIMPORT void _DK_draw_tooltip(void);
DLLIMPORT long _DK_map_fade_in(long a);
DLLIMPORT long _DK_map_fade_out(long a);
DLLIMPORT void _DK_draw_map_parchment(void);
DLLIMPORT void _DK_draw_2d_map(void);
DLLIMPORT void _DK_draw_gui(void);
DLLIMPORT void _DK_draw_zoom_box(void);
DLLIMPORT void _DK_light_stat_light_map_clear_area(long x1, long y1, long x2, long y2);
DLLIMPORT void _DK_light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2);
DLLIMPORT void _DK_turn_off_query(char a);
DLLIMPORT void _DK_post_init_level(void);
DLLIMPORT void _DK_post_init_players(void);
DLLIMPORT void _DK_init_level(void);
DLLIMPORT void _DK_init_player(struct PlayerInfo *player, int a2);
DLLIMPORT int _DK_frontend_is_player_allied(long plyr1, long plyr2);
#ifdef __cplusplus
}
#endif

//Functions - reworked
short setup_game(void);
void game_loop(void);
short reset_game(void);
void update(void);

short setup_network_service(int srvidx);
TbError LbNetwork_Init(unsigned long,struct _GUID guid, unsigned long, void *, unsigned long, struct TbNetworkPlayerInfo *netplayr, void *);
void ProperFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg);
void ProperForcedFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg);
short get_gui_inputs(short gameplay_on);
void intro(void);
void outro(void);

void create_room_flag(struct Room *room);
struct Room *allocate_free_room_structure(void);
unsigned short i_can_allocate_free_room_structure(void);
struct Room *link_adjacent_rooms_of_type(unsigned char owner, long x, long y, unsigned char rkind);
struct Room *create_room(unsigned char owner, unsigned char rkind, unsigned short x, unsigned short y);
void delete_room_structure(struct Room *room);
void set_room_efficiency(struct Room *room);
void set_room_capacity(struct Room *room, long capac);
void init_top_texture_to_cube_table(void);
void make_solidmask(struct Column *col);
void clear_columns(void);
void init_columns(void);
void init_whole_blocks(void);

void reveal_whole_map(struct PlayerInfo *player);
void increase_level(struct PlayerInfo *player);
void multiply_creatures(struct PlayerInfo *player);
struct Thing *create_creature(struct Coord3d *pos, unsigned short a1, unsigned short a2);
struct Thing *create_object(struct Coord3d *pos, unsigned short model, unsigned short owner, long a4);
void set_creature_level(struct Thing *thing, long nlvl);
short thing_is_special(struct Thing *thing);
#define is_dungeon_special thing_is_special
void activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player);
void resurrect_creature(struct Thing *thing, unsigned char a2, unsigned char a3, unsigned char a4);
void transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char a3);
void creature_increase_level(struct Thing *thing);
void remove_events_thing_is_attached_to(struct Thing *thing);
unsigned long steal_hero(struct PlayerInfo *player, struct Coord3d *pos);
void make_safe(struct PlayerInfo *player);
short activate_bonus_level(struct PlayerInfo *player);
void delete_thing_structure(struct Thing *thing, long a2);
long creature_instance_is_available(struct Thing *thing, long inum);
long add_gold_to_hoarde(struct Thing *thing, struct Room *room, long amount);
void check_map_for_gold(void);
short init_animating_texture_maps(void);
void reset_gui_based_on_player_mode(void);
void reinit_tagged_blocks_for_player(unsigned char idx);
void restore_computer_player_after_load(void);
void sound_reinit_after_load(void);
void draw_power_hand(void);
void draw_swipe(void);
void draw_bonus_timer(void);
void draw_view(struct Camera *cam, unsigned char a2);
void zoom_from_map(void);
void zoom_to_map(void);
void toggle_tooltips(void);
void toggle_hero_health_flowers(void);
void check_players_won(void);
void check_players_lost(void);
void process_dungeon_power_magic(void);
void process_dungeon_devastation_effects(void);
void process_entrance_generation(void);
void process_things_in_dungeon_hand(void);
void process_payday(void);
short bonus_timer_enabled(void);
void load_parchment_file(void);
void reload_parchment_file(short hires);
void process_sound_heap(void);

int setup_old_network_service(void);
short toggle_computer_player(int idx);
short save_settings(void);
void setup_engine_window(long x1, long y1, long x2, long y2);
void store_engine_window(struct TbGraphicsWindow *ewnd);
void load_engine_window(struct TbGraphicsWindow *ewnd);
short show_onscreen_msg(int nturns, const char *fmt_str, ...);
short is_onscreen_msg_visible(void);
void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal);
void set_engine_view(struct PlayerInfo *player, long val);
unsigned char find_first_battle_of_mine(unsigned char idx);
unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl);
void init_good_player_as(long plr_idx);
void init_keepers_map_exploration(void);
void init_dungeons_research(void);
void clear_creature_pool(void);
long load_stats_files(void);
void check_and_auto_fix_stats(void);
long update_dungeon_scores(void);
long update_dungeon_generation_speeds(void);
void calculate_dungeon_area_scores(void);
void setup_computer_players2(void);
long get_next_research_item(struct Dungeon *dungeon);
void init_creature_level(struct Thing *thing, long nlev);
short send_creature_to_room(struct Thing *thing, struct Room *room);
struct Room *get_room_thing_is_on(struct Thing *thing);
short set_start_state(struct Thing *thing);
void init_creature_state(struct Thing *thing);
void gui_set_button_flashing(long a1, long a2);
struct Thing *get_trap_for_position(long x, long y);
void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7);
void draw_status_sprites(long a1, long a2, struct Thing *thing, long a4);
long element_top_face_texture(struct Map *map);
long thing_is_spellbook(struct Thing *thing);
int LbSpriteDrawOneColour(long x, long y, struct TbSprite *spr, TbPixel colour);
long object_is_gold(struct Thing *thing);
unsigned short find_next_annoyed_creature(unsigned char a1, unsigned short a2);
unsigned char active_battle_exists(unsigned char a1);
unsigned char step_battles_forward(unsigned char a1);
short zoom_to_next_annoyed_creature(void);
short zoom_to_fight(unsigned char a1);
void go_to_my_next_room_of_type(unsigned long rkind);

short ceiling_set_info(long height_max, long height_min, long step);
void initialise_eye_lenses(void);
void setup_eye_lens(long nlens);
void reinitialise_eye_lens(long nlens);
void reset_eye_lenses(void);
void view_set_camera_y_inertia(struct Camera *cam, long a2, long a3);
void view_set_camera_x_inertia(struct Camera *cam, long a2, long a3);
void view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3);
void view_zoom_camera_in(struct Camera *cam, long a2, long a3);
void set_camera_zoom(struct Camera *cam, long val);
void view_zoom_camera_out(struct Camera *cam, long a2, long a3);
long get_camera_zoom(struct Camera *camera);
void update_camera_zoom_bounds(struct Camera *cam,unsigned long zoom_max,unsigned long zoom_min);
void keep_local_camera_zoom_level(unsigned long prev_units_per_pixel_size);
void set_mouse_light(struct PlayerInfo *player);
void start_resurrect_creature(struct PlayerInfo *player, struct Thing *thing);
void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing);
void magic_use_power_hold_audience(unsigned char idx);
void clear_slab_dig(long a1, long a2, char a3);
void light_delete_light(long idx);
void light_initialise_lighting_tables(void);
void light_initialise(void);
void delete_all_structures(void);
void clear_mapwho(void);
void clear_map(void);
void clear_game(void);
void clear_game_for_save(void);
void clear_complete_game(void);
long init_navigation(void);
void draw_creature_view(struct Thing *thing);
void update_explored_flags_for_power_sight(struct PlayerInfo *player);
void engine(struct Camera *cam);
void smooth_screen_area(unsigned char *a1, long a2, long a3, long a4, long a5, long a6);
void remove_explored_flags_for_power_sight(struct PlayerInfo *player);
void DrawBigSprite(long x, long y, struct BigSprite *bigspr, struct TbSprite *sprite);
void draw_gold_total(unsigned char a1, long a2, long a3, long a4);
void pannel_map_draw(long x, long y, long zoom);
void draw_overlay_things(long zoom);
void draw_overlay_compass(long a1, long a2);
void draw_lens(unsigned char *dstbuf, unsigned char *srcbuf, unsigned long *lens_mem, int width, int height, int scanln);
void flyeye_blitsec(unsigned char *srcbuf, unsigned char *dstbuf, long srcwidth, long dstwidth, long n, long height);
void display_drawlist(void);
void frame_wibble_generate(void);
void find_gamut(void);
void fiddle_gamut(long a1, long a2);
void create_map_volume_box(long a1, long a2, long a3);
void setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8);
void do_a_plane_of_engine_columns_perspective(long a1, long a2, long a3, long a4);
void do_a_plane_of_engine_columns_cluedo(long a1, long a2, long a3, long a4);
void do_a_plane_of_engine_columns_isometric(long a1, long a2, long a3, long a4);
void rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx);
void rotate_base_axis(struct M33 *matx, short a2, unsigned char a3);
void fill_in_points_perspective(long a1, long a2, struct MinMax *mm);
void fill_in_points_cluedo(long a1, long a2, struct MinMax *mm);
void fill_in_points_isometric(long a1, long a2, struct MinMax *mm);
void init_lens(unsigned long *lens_mem, int width, int height, int scanln, int nlens);
void flyeye_setup(long width, long height);
void perspective_standard(struct XYZ *cor, struct PolyPoint *ppt);
void perspective_fisheye(struct XYZ *cor, struct PolyPoint *ppt);
void rotpers_parallel(struct EngineCoord *epos, struct M33 *matx);
void rotpers_standard(struct EngineCoord *epos, struct M33 *matx);
void rotpers_circular(struct EngineCoord *epos, struct M33 *matx);
void rotpers_fisheye(struct EngineCoord *epos, struct M33 *matx);
short screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos);
void draw_jonty_mapwho(struct JontySpr *jspr);
struct Thing *find_base_thing_on_mapwho(unsigned char oclass, unsigned short okind, unsigned short x, unsigned short y);
void draw_keepsprite_unscaled_in_buffer(unsigned short a1, short a2, unsigned char a3, unsigned char *a4);
void draw_engine_number(struct Number *num);
void draw_engine_room_flag_top(struct RoomFlag *rflg);
short mouse_is_over_small_map(long x, long y);
void do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5);
long get_room_look_through(RoomKind rkind);

unsigned long seed_check_random(unsigned long range, unsigned long *seed, const char *func_name, unsigned long place);
void setup_heap_manager(void);
int setup_heap_memory(void);
void reset_heap_manager(void);
void reset_heap_memory(void);
int load_settings(void);
long light_create_light(struct InitLight *ilght);
void light_set_light_never_cache(long idx);
long light_is_light_allocated(long lgt_id);
void light_set_light_position(long lgt_id, struct Coord3d *pos);
void reset_player_mode(struct PlayerInfo *player, unsigned char a2);
void init_keeper_map_exploration(struct PlayerInfo *player);
void init_player_cameras(struct PlayerInfo *player);
void init_lookups(void);
void pannel_map_update(long x, long y, long w, long h);
short play_smacker_file(char *filename, int nstate);
struct Thing *create_effect(struct Coord3d *pos, unsigned short a2, unsigned char a3);
void turn_off_query(short a);
void free_swipe_graphic(void);
void complete_level(struct PlayerInfo *player);
short set_gamma(char corrlvl, short do_set);
void resync_game(void);
void level_lost_go_first_person(long plridx);
long battle_move_player_towards_battle(struct PlayerInfo *player, long var);
void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
void  toggle_ally_with_player(long plyridx, unsigned int allyidx);
void magic_use_power_armageddon(unsigned int plridx);
void lose_level(struct PlayerInfo *player);
void directly_cast_spell_on_thing(long plridx, unsigned char a2, unsigned short a3, long a4);
void output_message(long idx, long a, long b);
void go_on_then_activate_the_event_box(long plridx, long evidx);
void turn_off_sight_of_evil(long plridx);
short dump_held_things_on_map(unsigned int plyridx, long a2, long a3, short a4);
void set_player_mode(struct PlayerInfo *player, long val);
long set_autopilot_type(unsigned int plridx, long aptype);
void event_delete_event(long plridx, long num);
void set_player_state(struct PlayerInfo *player, short a1, long a2);
short magic_use_power_obey(unsigned short plridx);
long place_thing_in_power_hand(struct Thing *thing, long var);
void turn_off_call_to_arms(long a);
long event_move_player_towards_event(struct PlayerInfo *player, long var);
void toggle_creature_tendencies(struct PlayerInfo *player, short val);
long creature_instance_has_reset(struct Thing *thing, long a2);
long get_human_controlled_creature_target(struct Thing *thing, long a2);
void set_creature_instance(struct Thing *thing, long a1, long a2, long a3, struct Coord3d *pos);
void instant_instance_selected(long a1);
void centre_engine_window(void);
void change_engine_window_relative_size(long w_delta, long h_delta);
void light_set_lights_on(char state);
void message_add(char c);
void init_messages(void);
void battle_initialise(void);
void event_initialise_all(void);
long find_column(struct Column *col);
long create_column(struct Column *col);
unsigned short find_column_height(struct Column *col);
void add_thing_to_list(struct Thing *thing, struct StructureList *list);
struct Thing *allocate_free_thing_structure(unsigned char a1);
short thing_create_thing(struct InitThing *itng);
unsigned char i_can_allocate_free_thing_structure(unsigned char a1);
struct Thing *create_ambient_sound(struct Coord3d *pos, unsigned short a2, unsigned short owner);
void update_thing_animation(struct Thing *thing);
long update_thing(struct Thing *thing);
TbBigChecksum get_thing_checksum(struct Thing *thing);
short update_thing_sound(struct Thing *thing);
long update_cave_in(struct Thing *thing);
void move_thing_in_map(struct Thing *thing, struct Coord3d *pos);
void process_creature_instance(struct Thing *thing);
void update_creature_count(struct Thing *thing);
long process_creature_state(struct Thing *thing);
long get_floor_height_under_thing_at(struct Thing *thing, struct Coord3d *pos);
long slabs_count_near(long tx,long ty,long rad,unsigned short slbtype);
short initialise_map_rooms(void);
void initialise_map_collides(void);
void initialise_map_health(void);
void init_creature_scores(void);
void setup_3d(void);
void setup_stuff(void);
void reinitialise_treaure_rooms(void);
long ceiling_init(unsigned long a1, unsigned long a2);
long move_creature(struct Thing *thing);
void process_spells_affected_by_effect_elements(struct Thing *thing);
long get_top_cube_at_pos(long mpos);
void apply_damage_to_thing_and_display_health(struct Thing *thing, long a1, char a2);
long get_foot_creature_has_down(struct Thing *thing);
void process_disease(struct Thing *thing);
void set_creature_graphic(struct Thing *thing);
void process_keeper_spell_effect(struct Thing *thing);
long creature_is_group_leader(struct Thing *thing);
void leader_find_positions_for_followers(struct Thing *thing);
long update_creature_levels(struct Thing *thing);
long process_creature_self_spell_casting(struct Thing *thing);
void process_thing_spell_effects(struct Thing *thing);
struct ActionPoint *allocate_free_action_point_structure_with_number(long apt_num);
struct ActionPoint *actnpoint_create_actnpoint(struct InitActionPoint *iapt);
short kill_creature(struct Thing *thing, struct Thing *tngrp, char a1, unsigned char a2, unsigned char a3, unsigned char a4);
struct Thing *create_thing(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4);
struct Thing *create_door(struct Coord3d *pos, unsigned short a1, unsigned char a2, unsigned short a3, unsigned char a4);
struct Thing *create_effect_generator(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4);
void process_creature_standing_on_corpses_at(struct Thing *thing, struct Coord3d *pos);
long cleanup_current_thing_state(struct Thing *thing);
unsigned long setup_move_off_lava(struct Thing *thing);
struct Room *player_has_room_of_type(long plr_idx, long roomkind);
struct Room *find_room_with_spare_room_item_capacity(unsigned char a1, signed char a2);
long create_workshop_object_in_workshop_room(long a1, long a2, long a3);
void set_thing_draw(struct Thing *thing, long a2, long a3, long a4, char a5, char a6, unsigned char a7);
void light_set_light_minimum_size_to_cache(long a1, long a2, long a3);
long get_next_manufacture(struct Dungeon *dungeon);
unsigned char keepersprite_frames(unsigned short n);
void remove_thing_from_mapwho(struct Thing *thing);
void place_thing_in_mapwho(struct Thing *thing);
long get_thing_height_at(struct Thing *thing, struct Coord3d *pos);
struct Thing *get_nearest_thing_for_hand_or_slap(unsigned char a1, long a2, long a3);
unsigned long can_drop_thing_here(long x, long y, long a3, unsigned long a4);
long thing_in_wall_at(struct Thing *thing, struct Coord3d *pos);
unsigned long control_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
unsigned long control_creature_as_passenger(struct PlayerInfo *player, struct Thing *thing);
void load_swipe_graphic_for_creature(struct Thing *thing);
short can_place_thing_here(struct Thing *thing, long x, long y, long dngn_idx);
short do_left_map_drag(long begin_x, long begin_y, long curr_x, long curr_y, long zoom);
short do_left_map_click(long begin_x, long begin_y, long curr_x, long curr_y, long zoom);
short do_right_map_click(long start_x, long start_y, long curr_x, long curr_y, long zoom);

long update_object(struct Thing *thing);
long update_shot(struct Thing *thing);
long update_effect_element(struct Thing *thing);
long update_dead_creature(struct Thing *thing);
long update_creature(struct Thing *thing);
long update_effect(struct Thing *thing);
long process_effect_generator(struct Thing *thing);
long update_trap(struct Thing *thing);
long process_door(struct Thing *thing);

int LbNetwork_Exchange(struct Packet *pckt);
void startup_network_game(void);
void startup_saved_packet_game(void);
void faststartup_saved_packet_game(void);
void reinit_level_after_load(void);

//Functions - internal

//Functions - inline

void inline fade_in(void)
{
  ProperFadePalette(_DK_frontend_palette, 8, Lb_PALETTE_FADE_OPEN);
}

void inline fade_out(void)
{
  ProperFadePalette(0, 8, Lb_PALETTE_FADE_CLOSED);
  LbScreenClear(0);
}

short inline thing_is_invalid(const struct Thing *thing)
{
  return (thing == game.things_lookup[0]) || (thing == NULL);
}
// Variables - no longer imported

#endif // DK_KEEPERFX_H
