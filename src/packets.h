/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file packets.h
 *     Header file for packets.c.
 * @par Purpose:
 *     Packet processing routines.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     30 Jan 2009 - 11 Feb 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_PACKETS_H
#define DK_PACKETS_H

#include "bflib_basics.h"
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
enum TbPacketAction {
        PckA_None = 0,
        PckA_Unknown001,
        PckA_Unknown002,
        PckA_Unknown003,
        PckA_Unknown004,
        PckA_FinishGame,// 5
        PckA_Unknown006,
        PckA_Unknown007,
        PckA_Unknown008,
        PckA_Unknown009,
        PckA_InitPlayerNum,//10
        PckA_Unknown011,
        PckA_LevelExactCheck,
        PckA_PlyrMsgBegin,
        PckA_PlyrMsgEnd,
        PckA_Unknown015,//15
        PckA_Unknown016,
        PckA_Unknown017,
        PckA_Unknown018,
        PckA_Unknown019,
        PckA_ToggleLights,//20
        PckA_SwitchScrnRes,
        PckA_TogglePause,
        PckA_Unknown023,
        PckA_SetCluedo,
        PckA_Unknown025,//25
        PckA_BookmarkLoad,
        PckA_SetGammaLevel,
        PckA_SetMinimapConf,
        PckA_SetMapRotation,
        PckA_Unknown030,//30
        PckA_Unknown031,
        PckA_PasngrCtrlExit,
        PckA_DirectCtrlExit,
        PckA_Unknown034,
        PckA_Unknown035,//35
        PckA_SetPlyrState,
        PckA_SwitchView,
        PckA_Unknown038,
        PckA_CtrlCrtrSetInstnc,
        PckA_Unknown040,//40
        PckA_HoldAudience,
        PckA_Unknown042,
        PckA_Unknown043,
        PckA_Unknown044,
        PckA_Unknown045,//45
        PckA_Unknown046,
        PckA_Unknown047,
        PckA_Unknown048,
        PckA_Unknown049,
        PckA_Unknown050,//50
        PckA_Unknown051,
        PckA_Unknown052,
        PckA_Unknown053,
        PckA_Unknown054,
        PckA_ToggleTendency,//55
        PckA_Unknown056,
        PckA_Unknown057,
        PckA_Unknown058,
        PckA_Unknown059,
        PckA_CheatEnter,//60
        PckA_CheatAllFree,
        PckA_CheatCrtSpells,
        PckA_CheatRevealMap,
        PckA_CheatCrAllSpls,
        PckA_Unknown065,//65
        PckA_CheatAllMagic,
        PckA_CheatAllRooms,
        PckA_Unknown068,
        PckA_Unknown069,
        PckA_CheatAllResrchbl,//70
        PckA_Unknown071,
        PckA_Unknown072,
        PckA_Unknown073,
        PckA_Unknown074,
        PckA_Unknown075,//75
        PckA_Unknown076,
        PckA_Unknown077,
        PckA_Unknown078,
        PckA_Unknown079,
        PckA_SetViewType,//80
        PckA_ZoomFromMap,
        PckA_UpdatePause,
        PckA_Unknown083,
        PckA_ZoomToRoom,
        PckA_ZoomToTrap,//85
        PckA_ZoomToDoor,
        PckA_ZoomToPosition,
        PckA_Unknown088,
        PckA_PwrCTADis,
        PckA_UsePwrHandPick,//90
        PckA_UsePwrHandDrop,
        PckA_Unknown092,
        PckA_UseSpecialBox,
        PckA_Unknown094,
        PckA_ResurrectCrtr,//95
        PckA_TransferCreatr,
        PckA_UsePwrObey,
        PckA_UsePwrArmageddon,
        PckA_Unknown099,
        PckA_Unknown100,//100
        PckA_Unknown101,
        PckA_Unknown102,
        PckA_Unknown103,
        PckA_Unknown104,
        PckA_Unknown105,//105
        PckA_ZoomToSpell,
        PckA_ToggleComputer,
        PckA_PlyrFastMsg,
        PckA_SetComputerKind,
        PckA_GoSpectator,//110
        PckA_DumpHeldThingToOldPos,
        PckA_Unknown112,
        PckA_Unknown113,
        PckA_PwrSOEDis,
        PckA_EventBoxActivate,//115
        PckA_EventBoxClose,
        PckA_UsePwrOnThing,
        PckA_PlyrToggleAlly,
        PckA_SaveViewType,
        PckA_LoadViewType,//120
        PckA_PlyrMsgChar    =  121,
        PckA_PlyrMsgClear,
        PckA_DirectCtrlDragDrop
};

/** Packet flags for non-action player operation. */
enum TbPacketControl {
        PCtr_None           = 0x0000,
        PCtr_ViewRotateCW   = 0x0001,
        PCtr_ViewRotateCCW  = 0x0002,
        PCtr_MoveUp         = 0x0004,
        PCtr_MoveDown       = 0x0008,
        PCtr_MoveLeft       = 0x0010,
        PCtr_MoveRight      = 0x0020,
        PCtr_ViewZoomIn     = 0x0040,
        PCtr_ViewZoomOut    = 0x0080,
        PCtr_LBtnClick      = 0x0100,
        PCtr_RBtnClick      = 0x0200,
        PCtr_LBtnHeld       = 0x0400,
        PCtr_RBtnHeld       = 0x0800,
        PCtr_LBtnRelease    = 0x1000,
        PCtr_RBtnRelease    = 0x2000,
        PCtr_Unknown4000    = 0x4000,
        PCtr_MapCoordsValid = 0x8000,
};

/**
 * Additional packet flags
 */
enum TbPacketAddValues {
    PCAdV_None              = 0x00, //!< Dummy flag
    PCAdV_SpeedupPressed    = 0x01, //!< The keyboard modified used for speeding up camera movement is pressed.
    PCAdV_ContextMask       = 0x1E, //!< Instead of a single bit, this value stores is 4-byte integer; stores context of map coordinates. The context is used to set the Cursor State.
    PCAdV_CrtrContrlPressed = 0x20, //!< The keyboard modified used for creature control is pressed.
    PCAdV_CrtrQueryPressed  = 0x40, //!< The keyboard modified used for querying creatures is pressed.
    PCAdV_Unknown80         = 0x80, //!< Seem unused
};

enum ChecksumKind {
    CKS_Action = 0,
    CKS_Players,
    CKS_Creatures_1,
    CKS_Creatures_2,
    CKS_Creatures_3,
    CKS_Creatures_4,
    CKS_Creatures_5,  // Heroes
    CKS_Creatures_6,  // Neutral
    CKS_Things, //Objects, Traps, Shots etc
    CKS_Effects,
    CKS_Rooms,
    CKS_MAX
};

#define PCtr_LBtnAnyAction (PCtr_LBtnClick | PCtr_LBtnHeld | PCtr_LBtnRelease)
#define PCtr_RBtnAnyAction (PCtr_RBtnClick | PCtr_RBtnHeld | PCtr_RBtnRelease)
#define PCtr_HeldAnyButton (PCtr_LBtnHeld | PCtr_RBtnHeld)

#define INVALID_PACKET (&bad_packet)

/******************************************************************************/
#pragma pack(1)

struct PlayerInfo;
struct CatalogueEntry;

/**
 * Stores data exchanged between players each turn and used to re-create their input.
 */
struct Packet { // sizeof = 0x11 (17)
    int field_0;
    TbChecksum chksum; //! Checksum of all things within the game and synchronized random seed
    unsigned char action; //! Action kind performed by the player which owns this packet
    unsigned short actn_par1; //! Players action parameter #1
    unsigned short actn_par2; //! Players action parameter #2
    short pos_x; //! Mouse Cursor Position X
    short pos_y; //! Mouse Cursor Position Y
    unsigned short control_flags;
    unsigned char additional_packet_values; // uses the flags and values from TbPacketAddValues
};

struct PacketSaveHead { // sizeof=0xF (15)
    unsigned short game_ver_major;
    unsigned short game_ver_minor;
    unsigned short game_ver_release;
    unsigned short game_ver_build;
    unsigned long level_num;
    unsigned char players_exist;
    unsigned char players_comp;
    TbBool chksum_available; // if needed, this can be replaced with flags
};

struct PacketEx
{
    struct Packet packet;
    TbBigChecksum sums[CKS_MAX];
};

#pragma pack()
/******************************************************************************/
/******************************************************************************/
struct Packet *get_packet_direct(long pckt_idx);
struct Packet *get_packet(long plyr_idx);
void set_packet_action(struct Packet *pckt, unsigned char pcktype, unsigned short par1, unsigned short par2, unsigned short par3, unsigned short par4);
void set_players_packet_action(struct PlayerInfo *player, unsigned char pcktype, unsigned short par1, unsigned short par2, unsigned short par3, unsigned short par4);
void set_packet_control(struct Packet *pckt, unsigned long flag);
void set_players_packet_control(struct PlayerInfo *player, unsigned long flag);
unsigned char get_players_packet_action(struct PlayerInfo *player);
void unset_packet_control(struct Packet *pckt, unsigned long flag);
void unset_players_packet_control(struct PlayerInfo *player, unsigned long flag);
void set_players_packet_position(struct Packet *pckt, long x, long y, unsigned char context);
void set_packet_pause_toggle(void);
TbBool process_dungeon_control_packet_clicks(long idx);
TbBool process_players_dungeon_control_packet_action(long idx);
void process_players_creature_control_packet_control(long idx);
void process_players_creature_passenger_packet_action(long idx);
void process_players_creature_control_packet_action(long idx);
void process_frontend_packets(void);
void process_map_packet_clicks(long idx);
void process_pause_packet(long a1, long a2);
void process_quit_packet(struct PlayerInfo *player, short complete_quit);
void process_packets(void);
void clear_packets(void);
TbBigChecksum compute_players_checksum(void);
void player_packet_checksum_add(PlayerNumber plyr_idx, TbBigChecksum sum, const char *area_name);
short checksums_different(void);
void post_init_packets(void);

TbBool open_new_packet_file_for_save(void);
void load_packets_for_turn(GameTurn nturn);
TbBool open_packet_file_for_load(char *fname, struct CatalogueEntry *centry);
short save_packets(void);
void close_packet_file(void);
TbBool reinit_packets_after_load(void);
struct Room *keeper_build_room(long stl_x,long stl_y,long plyr_idx,long rkind);
TbBool player_sell_room_at_subtile(long plyr_idx, long stl_x, long stl_y);
void set_tag_untag_mode(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
