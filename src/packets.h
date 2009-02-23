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
enum TbPacketType {
        PckT_None           =  0,
        PckT_PlyrMsgBegin   =  13,
        PckT_PlyrMsgEnd     =  14,
        PckT_SwitchScrnRes  =  21,
        PckT_SetMinimapConf =  28,
        PckT_PlyrFastMsg    =  108,
        PckT_SpellSOEDis    =  114,
        PckT_PlyrToggleAlly =  118,
        PckT_PlyrMsgChar    =  121,
};

#pragma pack(1)

struct Packet { // sizeof = 0x11 (17)
    int field_0;
    TbChecksum field_4;
    unsigned char field_5;
    unsigned short field_6;
    unsigned short field_8;
    short field_A;
    short field_C;
    short field_E;
    unsigned char field_10;
    };

#pragma pack()

/******************************************************************************/

/******************************************************************************/
void set_packet_action(struct Packet *pckt, unsigned char pcktype, unsigned short par1, unsigned short par2, unsigned short par3, unsigned short par4);
void set_packet_control(struct Packet *pckt, unsigned long val);
void process_dungeon_control_packet_clicks(long idx);
void process_players_dungeon_control_packet_action(long idx);
void process_players_creature_control_packet_control(long idx);
void process_players_creature_passenger_packet_action(long idx);
void process_players_creature_control_packet_action(long idx);
void process_map_packet_clicks(long idx);
void process_pause_packet(long a1, long a2);
void process_packets(void);
void clear_packets(void);
unsigned long compute_players_checksum(void);
short checksums_different(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
