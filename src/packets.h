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
#include "bflib_keybrd.h"
#include "globals.h"
#include "player_data.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Camera;
struct Packet;
struct PlayerInfo;
struct Thing;
/******************************************************************************/

enum TbPacketAction {
        PckA_None = 0,
        PckA_QuitToMainMenu, // Quit
        PckA_ForceApplicationClose,
        PckA_SaveGameAndQuit,
        PckA_NoOperation,
        PckA_FinishGame, // 5
        PckA_Login,      // From `enum NetMessageType`
        PckA_UserUpdate,
        PckA_Frame,
        PckA_Resync,
        PckA_InitPlayerNum,//10
        PckA_UnusedSlot011,
        PckA_LevelExactCheck,
        PckA_PlyrMsgBegin,
        PckA_PlyrMsgEnd,
        PckA_UnusedSlot015,//15
        PckA_UnusedSlot016,
        PckA_UnusedSlot017,
        PckA_UnusedSlot018,
        PckA_UnusedSlot019,
        PckA_ToggleLights,//20
        PckA_SwitchScrnRes,
        PckA_TogglePause,
        PckA_UnusedSlot023,
        PckA_SetCluedo,
        PckA_ChangeWindowSize,//25
        PckA_BookmarkLoad,
        PckA_SetGammaLevel,
        PckA_SetMinimapConf,
        PckA_SetMapRotation,
        PckA_UnusedSlot030,//30
        PckA_UnusedSlot031,
        PckA_PasngrCtrlExit,
        PckA_DirectCtrlExit,
        PckA_UnusedSlot034,
        PckA_UnusedSlot035,//35
        PckA_SetPlyrState,
        PckA_SwitchView,
        PckA_UnusedSlot038,
        PckA_CtrlCrtrSetInstnc,
        PckA_GenericLevelPower,//40
        PckA_HoldAudience,
        PckA_UnusedSlot042,
        PckA_UnusedSlot043,
        PckA_UnusedSlot044,
        PckA_UnusedSlot045,//45
        PckA_UnusedSlot046,
        PckA_UnusedSlot047,
        PckA_UnusedSlot048,
        PckA_UnusedSlot049,
        PckA_UnusedSlot050,//50
        PckA_UnusedSlot051,
        PckA_UnusedSlot052,
        PckA_UnusedSlot053,
        PckA_UnusedSlot054,
        PckA_ToggleTendency,//55
        PckA_UnusedSlot056,
        PckA_UnusedSlot057,
        PckA_UnusedSlot058,
        PckA_UnusedSlot059,
        PckA_CheatEnter,//60
        PckA_CheatAllFree,
        PckA_CheatCrtSpells,
        PckA_CheatRevealMap,
        PckA_CheatCrAllSpls,
        PckA_CheatUnusedPlaceholder065,//65
        PckA_CheatAllMagic,
        PckA_CheatAllRooms,
        PckA_CheatUnusedPlaceholder068,
        PckA_CheatUnusedPlaceholder069,
        PckA_CheatAllResrchbl,//70
        PckA_UnusedSlot071,
        PckA_UnusedSlot072,
        PckA_UnusedSlot073,
        PckA_UnusedSlot074,
        PckA_UnusedSlot075,//75
        PckA_UnusedSlot076,
        PckA_UnusedSlot077,
        PckA_UnusedSlot078,
        PckA_UnusedSlot079,
        PckA_SetViewType,//80
        PckA_ZoomFromMap,
        PckA_UpdatePause,
        PckA_ZoomToEvent,
        PckA_ZoomToRoom,
        PckA_ZoomToTrap,//85
        PckA_ZoomToDoor,
        PckA_ZoomToPosition,
        PckA_ToggleComputerProcessing,
        PckA_PwrCTADis,
        PckA_UsePwrHandPick,//90
        PckA_UsePwrHandDrop,
        PckA_EventBoxTurnOff,
        PckA_UseSpecialBox,
        PckA_UnusedSlot094,
        PckA_ResurrectCrtr,//95
        PckA_TransferCreatr,
        PckA_UsePwrObey,
        PckA_UsePwrArmageddon,
        PckA_TurnOffQuery,
        PckA_UnusedSlot100,//100
        PckA_UnusedSlot101,
        PckA_UnusedSlot102,
        PckA_UnusedSlot103,
        PckA_ZoomToBattle,
        PckA_UnusedSlot105,//105
        PckA_ZoomToSpell,
        PckA_ToggleComputer,
        PckA_PlyrFastMsg,
        PckA_SetComputerKind,
        PckA_GoSpectator,//110
        PckA_DumpHeldThingToOldPos,
        PckA_UnusedSlot112,
        PckA_UnusedSlot113,
        PckA_PwrSOEDis,
        PckA_EventBoxActivate,//115
        PckA_EventBoxClose,
        PckA_UsePwrOnThing,
        PckA_PlyrToggleAlly,
        PckA_SaveViewType,
        PckA_LoadViewType,//120
        PckA_PlyrMsgChar    =  121,
        PckA_PlyrMsgClear,
        PckA_PlyrMsgLast,
        PckA_PlyrMsgCmdAutoCompletion,
        PckA_DirectCtrlDragDrop,
        PckA_CheatPlaceTerrain,
        PckA_CheatMakeCreature,
        PckA_CheatMakeDigger,
        PckA_CheatStealSlab,
        PckA_CheatStealRoom,
        PckA_CheatHeartHealth,
        PckA_CheatKillPlayer,
        PckA_CheatConvertCreature,
        PckA_CheatSwitchTerrain,
        PckA_CheatSwitchPlayer,
        PckA_CheatSwitchCreature,
        PckA_CheatSwitchHero,
        PckA_CheatSwitchExperience,
        PckA_CheatCtrlCrtrSetInstnc,
        PckA_SetFirstPersonDigMode,
        PckA_SwitchTeleportDest,
        PckA_SelectFPPickup,
        PckA_CheatAllDoors,
        PckA_CheatAllTraps,
        PckA_SetRoomspaceAuto,
        PckA_SetRoomspaceMan,
        PckA_SetRoomspaceDrag,
        PckA_SetRoomspaceDefault,
        PckA_SetRoomspaceWholeRoom,
        PckA_SetRoomspaceSubtile,
        PckA_SetRoomspaceHighlight,
        PckA_SetNearestTeleport,
        PckA_SetRoomspaceDragPaint,
        PckA_PlyrQueryCreature,
        PckA_CheatGiveDoorTrap,
        PckA_RoomspaceHighlightToggle,
        PckA_SpriteZipCountSync,
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
        PCtr_Gui            = 0x4000,
        PCtr_MapCoordsValid = 0x8000,
        PCtr_ViewTiltUp     = 0x10000,
        PCtr_ViewTiltDown   = 0x20000,
        PCtr_ViewTiltReset  = 0x40000,
        PCtr_Ascend         = 0x80000,
        PCtr_Descend        = 0x100000,
};

/**
 * Additional packet flags
 */
enum TbPacketAddValues {
    PCAdV_None              = 0x00, //!< Dummy flag
    PCAdV_SpeedupPressed    = 0x01, //!< The keyboard modified used for speeding up camera movement is pressed.
    PCAdV_ContextMask       = 0x1E, //!< Instead of a single bit, this value stores is 4-bit integer; stores context of map coordinates. The context is used to set the Cursor State.
    PCAdV_CrtrContrlPressed = 0x20, //!< The keyboard modified used for creature control is pressed.
    PCAdV_CrtrQueryPressed  = 0x40, //!< The keyboard modified used for querying creatures is pressed.
    PCAdV_RotatePressed     = 0x80,
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

extern unsigned long initial_replay_seed;
extern unsigned long scheduled_unpause_time;

/**
 * Stores data exchanged between players each turn and used to re-create their input.
 */
struct Packet {
    GameTurn turn;
    TbBigChecksum checksum; //! Checksum of the entire game state of the previous turn, used solely for desync detection
    unsigned char action; //! Action kind performed by the player which owns this packet
    int32_t actn_par1; //! Players action parameter #1
    int32_t actn_par2; //! Players action parameter #2
    int32_t pos_x; //! Mouse Cursor Position X
    int32_t pos_y; //! Mouse Cursor Position Y
    uint32_t control_flags;
    unsigned char additional_packet_values; // uses the flags and values from TbPacketAddValues
    int32_t actn_par3; //! Players action parameter #3
    int32_t actn_par4; //! Players action parameter #4
};

struct PacketSaveHead {
    unsigned short game_ver_major;
    unsigned short game_ver_minor;
    unsigned short game_ver_release;
    unsigned short game_ver_build;
    uint32_t level_num;
    PlayerBitFlags players_exist;
    PlayerBitFlags players_comp;
    uint32_t isometric_view_zoom_level;
    uint32_t frontview_zoom_level;
    int isometric_tilt;
    unsigned char video_rotate_mode;
    TbBool chksum_available; // if needed, this can be replaced with flags
    uint32_t action_seed;
    TbBool default_imprison_tendency;
    TbBool default_flee_tendency;
    TbBool skip_heart_zoom;
    TbBool highlight_mode;
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
void set_packet_action(struct Packet *pckt, unsigned char pcktype, long par1, long par2, unsigned short par3, unsigned short par4);
TbBool is_packet_empty(const struct Packet *pckt);
void set_players_packet_action(struct PlayerInfo *player, unsigned char pcktype, unsigned long par1, unsigned long par2, unsigned short par3, unsigned short par4);
void set_packet_control(struct Packet *pckt, unsigned long flag);
void set_players_packet_control(struct PlayerInfo *player, unsigned long flag);
unsigned char get_players_packet_action(struct PlayerInfo *player);
void unset_packet_control(struct Packet *pckt, unsigned long flag);
void unset_players_packet_control(struct PlayerInfo *player, unsigned long flag);
void set_players_packet_position(struct Packet *pckt, long x, long y, unsigned char context);
void set_packet_pause_toggle(void);
void force_application_close(void);
void apply_default_flee_and_imprison_setting(void);
TbBool process_dungeon_control_packet_clicks(long idx);
TbBool process_players_dungeon_control_packet_action(long idx);
void process_players_creature_control_packet_control(long idx);
void process_players_creature_passenger_packet_action(long idx);
void process_players_creature_control_packet_action(long idx);
void process_frontend_packets(void);
void process_map_packet_clicks(long idx);
void process_pause_packet(long a1, long a2);
void process_quit_packet(struct PlayerInfo *player, short complete_quit);
void message_text_key_add(char *message, TbKeyCode key, TbKeyMods kmodif);
void process_chat_message_end(int player_id, const char *message);
void process_camera_controls(struct Camera* cam, struct Packet* pckt, struct PlayerInfo* player);
void process_first_person_look(struct Thing *thing, struct Packet *pckt, long current_horizontal, long current_vertical, long *out_horizontal, long *out_vertical, long *out_roll);
void process_packets(void);
void set_local_packet_turn(void);
void clear_packets(void);
TbBigChecksum compute_replay_integrity(void);
void post_init_packets(void);

TbBool open_new_packet_file_for_save(void);
void load_packets_for_turn(GameTurn nturn);
TbBool open_packet_file_for_load(char *fname, struct CatalogueEntry *centry);
short save_packets(void);
void close_packet_file(void);
TbBool reinit_packets_after_load(void);
struct Room *keeper_build_room(long stl_x,long stl_y,long plyr_idx,long rkind);
TbBool player_sell_room_at_subtile(long plyr_idx, long stl_x, long stl_y);
void set_tag_untag_mode(PlayerNumber plyr_idx);
TbBool packets_process_cheats(PlayerNumber plyr_idx, MapCoord x, MapCoord y,
    struct Packet* pckt, MapSubtlCoord stl_x, MapSubtlCoord stl_y, MapSlabCoord slb_x, MapSlabCoord slb_y);
void send_sprite_zip_count_to_other_players(void);
void process_sprite_zip_count_sync(long plyr_idx, long zip_count);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
