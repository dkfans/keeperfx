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
        PckA_None           =    0,
        PckA_FinishGame     =    5,
        PckA_PlyrMsgBegin   =   13,
        PckA_PlyrMsgEnd     =   14,
        PckA_ToggleLights   =   20,
        PckA_SwitchScrnRes  =   21,
        PckA_TogglePause    =   22,
        PckA_BookmarkLoad   =   26,
        PckA_SetGammaLevel  =   27,
        PckA_SetMinimapConf =   28,
        PckA_PasngrCtrlExit =   32,
        PckA_SetPlyrState   =   36,
        PckA_HoldAudience   =   41,
        PckA_ToggleTendency =   55,
        PckA_CheatEnter     =   60,
        PckA_CheatAllFree   =   61,
        PckA_CheatCrtSpells =   62,
        PckA_CheatRevealMap =   63,
        PckA_CheatCrAllSpls =   64,
        PckA_CheatAllMagic  =   66,
        PckA_CheatAllRooms  =   67,
        PckA_CheatAllResrchbl=  70,
        PckA_SpellCTADis    =   89,
        PckA_DumpHeldThings =   91,
        PckA_UseSpecialBox  =   93,
        PckA_ResurrectCrtr  =   95,
        PckA_TransferCreatr =   96,
        PckA_ToggleComputer =  107,
        PckA_PlyrFastMsg    =  108,
        PckA_SetComputerKind=  109,
        PckA_SpellSOEDis    =  114,
        PckA_PlyrToggleAlly =  118,
        PckA_PlyrMsgChar    =  121,
};

enum TbPacketControl {
        PCtr_None           = 0x0000,
        PCtr_LBtnClick      = 0x0100,
        PCtr_RBtnClick      = 0x0200,
        PCtr_LBtnHeld       = 0x0400,
        PCtr_RBtnHeld       = 0x0800,
        PCtr_LBtnRelease    = 0x1000,
        PCtr_RBtnRelease    = 0x2000,
        PCtr_MapCoordsValid = 0x8000,
};

#define PCtr_LBtnAnyAction (PCtr_LBtnClick | PCtr_LBtnHeld | PCtr_LBtnRelease)
#define PCtr_RBtnAnyAction (PCtr_RBtnClick | PCtr_RBtnHeld | PCtr_RBtnRelease)
#define PCtr_HeldAnyButton (PCtr_LBtnHeld | PCtr_RBtnHeld)

#define INVALID_PACKET (&bad_packet)

#ifdef __cplusplus
#pragma pack(1)
#endif

struct PlayerInfo;

struct Packet { // sizeof = 0x11 (17)
    int field_0;
    TbChecksum chksum;
    unsigned char action;
    unsigned short field_6;
    unsigned short field_8;
    short pos_x;
    short pos_y;
    unsigned short control_flags;
    unsigned char field_10;
    };

struct PacketSaveHead { // sizeof=0xF (15)
unsigned int field_0;
    unsigned long level_num;
unsigned int field_8;
    unsigned char field_C;
    unsigned char field_D;
    TbChecksum chksum;
    };

#ifdef __cplusplus
#pragma pack()
#endif

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
void set_players_packet_position(struct PlayerInfo *player, long x, long y);
short set_packet_pause_toggle(void);
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
void set_player_packet_checksum(long plyr_idx,TbBigChecksum sum);
short checksums_different(void);
void post_init_packets(void);

TbBool open_new_packet_file_for_save(void);
void load_packets_for_turn(long nturn);
TbBool open_packet_file_for_load(char *fname);
short save_packets(void);
void close_packet_file(void);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
