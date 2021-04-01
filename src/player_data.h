/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file player_data.h
 *     Header file for player_data.c.
 * @par Purpose:
 *     Player data structures definitions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     10 Nov 2009 - 20 Jan 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PLYR_DATA_H
#define DK_PLYR_DATA_H

#include "bflib_basics.h"
#include "globals.h"
#include "engine_camera.h"
#include "bflib_video.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PLAYERS_COUNT           5
#define PLAYERS_EXT_COUNT       6
/** This acts as default value for neutral_player_number */
#define NEUTRAL_PLAYER          5
/** This acts as default value for hero_player_number */
#define HERO_PLAYER             4

#define INVALID_PLAYER (&bad_player)

#define PLAYER_MP_MESSAGE_LEN  64

#define WANDER_POINTS_COUNT    200

enum PlayerInitFlags {
    PlaF_Allocated          = 0x01,
    PlaF_Unknown2           = 0x02,
    PlaF_NewMPMessage       = 0x04,
    PlaF_Unknown8           = 0x08,
    PlaF_Unknown10          = 0x10,
    PlaF_Unknown20          = 0x20,
    PlaF_CompCtrl           = 0x40,
    PlaF_Unknown80          = 0x80,
};

enum PlayerField6Flags {
    PlaF6_Unknown01         = 0x01,
    PlaF6_PlyrHasQuit       = 0x02,
    // The below are unused
    PlaF6_Unknown04         = 0x04,
    PlaF6_Unknown08         = 0x08,
    PlaF6_Unknown10         = 0x10,
    PlaF6_Unknown20         = 0x20,
    PlaF6_Unknown40         = 0x40,
    PlaF6_Unknown80         = 0x80,
};

enum PlayerViewModes {
    PVM_EmptyView = 0,
    PVM_CreatureView, /**< View from a creature perspective, first person. */
    PVM_IsometricView, /**< Dungeon overview from isometric front perspective, simplified version - only 4 angles. */
    PVM_ParchmentView, /**< Full screen parchment map view, showing dungeon schematic from top. */
    PVM_Unknown4,
    PVM_FrontView, /**< Dungeon overview from isometric front perspective, advanced version - fluent rotation. */
    PVM_ParchFadeIn, /**< Transitional view when fading from Isometric view to Parchment map. */
    PVM_ParchFadeOut, /**< Transitional view when fading from Parchment map back to Isometric view. */
};

enum PlayerViewType {
    PVT_None = 0,
    PVT_DungeonTop, /**< Normal map view. */
    PVT_CreatureContrl, /**< First person creature control mode. */
    PVT_CreaturePasngr, /**< First person creature view mode without controlling it. */
    PVT_MapScreen,      /**< Parchment map screen. */
    PVT_MapFadeIn, // 5
    PVT_MapFadeOut,
};

enum PlayerVictoryState {
    VicS_Undecided = 0,
    VicS_WonLevel,
    VicS_LostLevel,
    VicS_State3,
};

enum PlayerField454Val { // These could be Cursor states...?
    P454_Unkn0 = 0,
    P454_Unkn1,
    P454_Unkn2,
    P454_Unkn3,
};

enum PlayerField3Flags {
    Pf3F_None   = 0x00,
    Pf3F_Unkn01 = 0x01,
    Pf3F_Unkn02 = 0x02,
    Pf3F_Unkn04 = 0x04,
    Pf3F_Unkn08 = 0x08,
    Pf3F_Unkn10 = 0x10,
    Pf3F_Unkn20 = 0x20,
    Pf3F_Unkn40 = 0x40,
    Pf3F_Unkn80 = 0x80,
};

/******************************************************************************/
#pragma pack(1)

struct SubtileXY {
    unsigned char stl_x;
    unsigned char stl_y;
};

struct Wander // sizeof = 424
{
  unsigned long points_count;
  /** Index at which the search function inserts (or replaces) points. */
  unsigned long point_insert_idx;
  /** Slab last checked by the search function. */
  unsigned long last_checked_slb_num;
  /** Amount of slabs to be checked in one run of the search function. */
  unsigned long num_check_per_run;
  /** Max amount of points added in one run of the search function. */
  unsigned long max_found_per_check;
  unsigned char wdrfield_14;
  unsigned char wandr_slot;
  unsigned char plyr_idx;
  unsigned char plyr_bit;
  /** Array of points where the creatures could go wander. */
  struct SubtileXY points[WANDER_POINTS_COUNT];
};

#define SIZEOF_PlayerInfo 0x4EF
struct PlayerInfo {
    unsigned char allocflags;
    unsigned char field_1;
    unsigned char boxsize; //field_2 seems to be used in DK, so now renamed and used in KeeperFX
    unsigned char field_3;
    unsigned char input_crtr_control;
    unsigned char input_crtr_query;
    unsigned char flgfield_6;
    unsigned char *field_7;
    /** Index of packet slot associated with this player. */
    unsigned char packet_num;
    long field_C;
    unsigned int hand_busy_until_turn;
unsigned char field_14;
    char field_15[20]; //size may be shorter
    unsigned char victory_state;
    unsigned char allied_players;
    unsigned char id_number;
    unsigned char is_active;
    unsigned char field_2D[2];
    short controlled_thing_idx;
    long controlled_thing_creatrn;
    short thing_under_hand;
    unsigned char view_mode;
    /** Pointer to the currently active camera. */
    struct Camera *acamera;
    struct Camera cameras[4];
    unsigned short zoom_to_pos_x;
    unsigned short zoom_to_pos_y;
char field_E8[2];
    struct Wander wandr_within;
    struct Wander wandr_outside;
    short hand_thing_idx;
    short field_43C;
    short influenced_thing_idx;
    long influenced_thing_creation;
    short engine_window_width;
    short engine_window_height;
    short engine_window_x;
    short engine_window_y;
    short minimap_pos_x;
    short minimap_pos_y;
    unsigned short minimap_zoom;
    unsigned char view_type;
    unsigned char work_state;
    unsigned char field_454;
    unsigned char field_455;
    unsigned char continue_work_state;
char field_457[8];
char field_45F;
short field_460;
char field_462;
    char mp_message_text[PLAYER_MP_MESSAGE_LEN];
    unsigned char chosen_room_kind;
    unsigned char full_slab_cursor; // 0 for subtile sized cursor, 1 for slab sized cursor
    char chosen_trap_kind;
    char chosen_door_kind;
    char field_4A7[4];
    short cursor_stl_x; // current x coord of subtile under the mouse cursor
    short cursor_stl_y; // current y coord of subtile under the mouse cursor
    unsigned char cursor_button_down; // left or right button down (whilst using the bounding box cursor)
    /** Player instance, from PlayerInstanceNum enum. */
    unsigned char instance_num;
    unsigned long instance_remain_rurns;
    /** If view mode is temporarily covered by another, the original mode which is to be restored later will be saved here.*/
    char view_mode_restore;
    long dungeon_camera_zoom;
    char field_4BA[3];
    long field_4BD;
    long field_4C1;
    long field_4C5;
    unsigned char *palette;
    long field_4CD;
    char field_4D1;
    /** Overcharge level while casting keeper powers. */
    long cast_expand_level;
    long field_4D6;
    char video_cluedo_mode;
    long field_4DB;
    long field_4DF;
    long field_4E3;
    long field_4E7;
    long field_4EB;
    };

/******************************************************************************/
DLLIMPORT extern unsigned char _DK_my_player_number;
#define my_player_number _DK_my_player_number

#pragma pack()
/******************************************************************************/
extern unsigned short player_colors_map[];
extern TbPixel player_path_colours[];
extern TbPixel player_room_colours[];
extern TbPixel player_flash_colours[];
extern TbPixel player_highlight_colours[];
extern unsigned short const player_cubes[];
extern long neutral_player_number;
extern long hero_player_number;
extern struct PlayerInfo bad_player;
/******************************************************************************/
struct PlayerInfo *get_player_f(long plyr_idx,const char *func_name);
#define get_player(plyr_idx) get_player_f(plyr_idx,__func__)
#define get_my_player() get_player_f(my_player_number,__func__)
TbBool player_invalid(const struct PlayerInfo *player);
TbBool player_exists(const struct PlayerInfo *player);
TbBool is_my_player(const struct PlayerInfo *player);
TbBool is_my_player_number(PlayerNumber plyr_num);
TbBool player_allied_with(const struct PlayerInfo *player, PlayerNumber ally_idx);
TbBool players_are_enemies(long plyr1_idx, long plyr2_idx);
TbBool players_are_mutual_allies(PlayerNumber plyr1_idx, PlayerNumber plyr2_idx);
TbBool players_creatures_tolerate_each_other(PlayerNumber plyr1_idx, PlayerNumber plyr2_idx);
TbBool player_is_friendly_or_defeated(PlayerNumber check_plyr_idx, PlayerNumber origin_plyr_idx);
TbBool set_ally_with_player(PlayerNumber plyridx, PlayerNumber ally_idx, TbBool state);
void  toggle_ally_with_player(long plyridx, unsigned int allyidx);

void set_player_state(struct PlayerInfo *player, short a1, long a2);
void set_player_mode(struct PlayerInfo *player, unsigned short nview);
void reset_player_mode(struct PlayerInfo *player, unsigned short nview);

void clear_players(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
