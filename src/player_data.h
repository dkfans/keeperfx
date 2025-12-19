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
#include "roomspace.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PLAYERS_COUNT       9
#define COLOURS_COUNT       9

#define INVALID_PLAYER (&bad_player)

#define PLAYER_MP_MESSAGE_LEN  64

#define WANDER_POINTS_COUNT    200

enum PlayerInitFlags {
    PlaF_Allocated               = 0x01,
    PlaF_unusedparam             = 0x02,
    PlaF_NewMPMessage            = 0x04,
    PlaF_CreaturePassengerMode   = 0x08,
    PlaF_KeyboardInputDisabled   = 0x10,
    PlaF_ChosenSlabHasActiveTask = 0x20, // Enabled when there are active tasks for the current slab. Used to determine if a high slab is tagged for digging (or not).
    PlaF_CompCtrl                = 0x40,
    PlaF_MouseInputDisabled      = 0x80,
};

enum PlayerField6Flags {
    PlaF6_DisplayNeedsUpdate = 0x01,
    PlaF6_PlyrHasQuit       = 0x02,
};

enum PlayerViewModes {
    PVM_EmptyView = 0,
    PVM_CreatureView, /**< View from a creature perspective, first person. */
    PVM_IsoWibbleView, /**< Dungeon overview from isometric front perspective, simplified version - only 4 angles. */
    PVM_ParchmentView, /**< Full screen parchment map view, showing dungeon schematic from top. */
    PVM_unusedparam,
    PVM_FrontView, /**< Dungeon overview from isometric front perspective, advanced version - fluent rotation. */
    PVM_ParchFadeIn, /**< Transitional view when fading from Isometric view to Parchment map. */
    PVM_ParchFadeOut, /**< Transitional view when fading from Parchment map back to Isometric view. */
    PVM_IsoStraightView, /**< Same as PVM_IsoWibbleView, but without wibble. */
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

enum PlayerCursorStates {
    CSt_DefaultArrow  = 0, // Default - Arrow Cursor
    CSt_PickAxe       = 1, // Dig - Pickake cursor
    CSt_DoorKey       = 2, // Lock/Unlock Door - Key cursor
    CSt_PowerHand     = 3, // Power Hand cursor
};

enum PlayerAdditionalFlags {
    PlaAF_None                      = 0x00,
    PlaAF_NoThingUnderPowerHand     = 0x01, // Chosen subtile has nothing to interact with with the Power Hand (no creature to slap etc) (But the power hand is active)
    PlaAF_ChosenSubTileIsHigh       = 0x02, // Chosen subtile is at ceiling height (dirt/rock/wall etc)
    PlaAF_FreezePaletteIsActive     = 0x04, // blue_palette is being used during Freeze Spell
    PlaAF_LightningPaletteIsActive  = 0x08, // lightning_palette is being used during Lightning Spell
    PlaAF_UnlockedLordTorture       = 0x10, // if this flag is set, the player will be sent to the Lord Torture Mini-game
    // The below are unused in KFX
    PlaAF_Unkn20                    = 0x20,
    PlaAF_Unkn40                    = 0x40,
    PlaAF_Unkn80                    = 0x80,
};

enum PlayerTypes {
    PT_Keeper,
    PT_Roaming,
    PT_Neutral
};

/******************************************************************************/
#pragma pack(1)

struct SubtileXY {
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
};

struct Wander
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
  unsigned char search_limiting_enabled;
  unsigned char wandr_slot;
  PlayerNumber plyr_idx;
  PlayerBitFlags plyr_bit; // unused?
  /** Array of points where the creatures could go wander. */
  struct SubtileXY points[WANDER_POINTS_COUNT];
};

struct CheatSelection
{
    SlabKind chosen_terrain_kind;
    PlayerNumber chosen_player;
    unsigned char chosen_creature_kind;
    unsigned char chosen_hero_kind;
    unsigned char chosen_experience_level;
};

struct PlayerInfo {
    unsigned char allocflags;
    TbBool tooltips_restore; /**< Used to store/restore the value of settings.tooltips_on when transitioning to/from the map. */
    TbBool status_menu_restore; /**< Used to store/restore the current status menu visibility when the map is shown/hidden. */
    TbBool paused_state_restore; /**< Used to restore pause state after saving */
    TbBool swipe_sprite_drawLR; /**< Used to decide whether to draw the swipe sprite left to right (TRUE), or [default] right to left (FALSE). */
    unsigned char boxsize; //field_2 seems to be used in DK, so now renamed and used in KeeperFX
    unsigned char additional_flags; // Uses PlayerAdditionalFlags
    unsigned char input_crtr_control;
    unsigned char input_crtr_query;
    unsigned char display_flags;
    unsigned char *lens_palette;
    /** Index of packet slot associated with this player. */
    unsigned char packet_num;
    long hand_animationId;
    unsigned int hand_busy_until_turn;
    char player_name[20];
    unsigned char victory_state;
    PlayerBitFlags allied_players;
    PlayerBitFlags players_with_locked_ally_status;
    unsigned char id_number;
    TbBool is_active;
    short controlled_thing_idx;
    GameTurn controlled_thing_creatrn;
    short thing_under_hand;
    TbBool possession_lock;
    unsigned char view_mode;
    /** Pointer to the currently active camera. */
    struct Camera *acamera;
    struct Camera cameras[4];
    MapCoord zoom_to_pos_x;
    MapCoord zoom_to_pos_y;
    struct Wander wandr_within;
    struct Wander wandr_outside;
    short hand_thing_idx;
    short cta_flag_idx;
    short influenced_thing_idx;
    GameTurn influenced_thing_creation;
    short engine_window_width;
    short engine_window_height;
    short engine_window_x;
    short engine_window_y;
    short minimap_pos_x;
    short minimap_pos_y;
    unsigned short minimap_zoom;
    unsigned char view_type;
    PlayerState work_state;
    unsigned char primary_cursor_state;
    unsigned char secondary_cursor_state;
    PlayerState continue_work_state;
    short cursor_light_idx;
    char mp_message_text[PLAYER_MP_MESSAGE_LEN];
    char mp_pending_message[PLAYER_MP_MESSAGE_LEN];
    char mp_message_text_last[PLAYER_MP_MESSAGE_LEN];
    unsigned char chosen_room_kind;
    unsigned char full_slab_cursor; // 0 for subtile sized cursor, 1 for slab sized cursor
    ThingModel chosen_trap_kind;
    ThingModel chosen_door_kind;
    PowerKind chosen_power_kind;
    MapSubtlCoord cursor_clicked_subtile_x; // x coord of subtile clicked by mouse cursor
    MapSubtlCoord cursor_clicked_subtile_y; // y coord of subtile clicked by mouse cursor
    unsigned char cursor_button_down; // left or right button down (whilst using the bounding box cursor)
    /** Player instance, from PlayerInstanceNum enum. */
    unsigned char instance_num;
    unsigned long instance_remain_turns;
    /** If view mode is temporarily covered by another, the original mode which is to be restored later will be saved here.*/
    char view_mode_restore;
    long dungeon_camera_zoom;
    long palette_fade_step_map;
    long palette_fade_step_pain;
    long palette_fade_step_possession;
    unsigned char *main_palette;
    /** Overcharge level while casting keeper powers. */
    long cast_expand_level;
    char video_cluedo_mode;
    MapCoordDelta zoom_to_movement_x;
    MapCoordDelta zoom_to_movement_y;
    GameTurn power_of_cooldown_turn;
    long game_version;
    GameTurn display_objective_turn;
    unsigned long isometric_view_zoom_level;
    unsigned long frontview_zoom_level;
    unsigned char hand_idx;
    struct CheatSelection cheatselection;
    TbBool first_person_dig_claim_mode;
    unsigned char teleport_destination;
    TbBool nearest_teleport;
    BattleIndex battleid;
    unsigned short selected_fp_thing_pickup;
    struct RoomSpace render_roomspace;
    struct RoomSpace roomspace;
    unsigned char roomspace_mode;
    int user_defined_roomspace_width;
    int roomspace_detection_looseness;
    int roomspace_width;
    int roomspace_height;
    TbBool one_click_mode_exclusive;
    TbBool one_click_lock_cursor;
    TbBool ignore_next_PCtr_RBtnRelease;
    TbBool ignore_next_PCtr_LBtnRelease;
    char swap_to_untag_mode; // 0 = no, 1 = maybe, 2= yes, -1 = disable
    unsigned char roomspace_highlight_mode;
    TbBool roomspace_no_default;
    MapSubtlCoord cursor_subtile_x;
    MapSubtlCoord cursor_subtile_y;
    MapSubtlCoord previous_cursor_subtile_x;
    MapSubtlCoord previous_cursor_subtile_y;
    TbBool mouse_on_map;
    TbBool interpolated_tagging;
    TbBool roomspace_drag_paint_mode;
    unsigned char roomspace_l_shape;
    TbBool roomspace_horizontal_first;
    TbBool pickup_all_gold;
    unsigned char player_type; //enum PlayerTypes
    ThingModel special_digger;
    int isometric_tilt;
    unsigned short generate_speed;
    };

/******************************************************************************/

extern unsigned char my_player_number;

#pragma pack()
/******************************************************************************/
extern unsigned short player_colors_map[];
extern TbPixel player_path_colours[];
extern TbPixel player_room_colours[];
extern TbPixel player_flash_colours[];
extern TbPixel player_highlight_colours[];
extern TbPixel possession_hit_colours[];
extern unsigned short const player_cubes[];
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
TbBool set_ally_with_player(PlayerNumber plyr_idx, PlayerNumber ally_idx, TbBool make_ally);
void toggle_ally_with_player(PlayerNumber plyr_idx, PlayerNumber ally_idx);
TbBool is_player_ally_locked(PlayerNumber plyr_idx, PlayerNumber ally_idx);
void set_player_ally_locked(PlayerNumber plyr_idx, PlayerNumber ally_idx, TbBool lock_alliance);

TbBool player_is_roaming(PlayerNumber plyr_num);
TbBool player_is_keeper(PlayerNumber plyr_num);
TbBool player_is_neutral(PlayerNumber plyr_num);

void set_player_state(struct PlayerInfo *player, short a1, long a2);
void set_player_mode(struct PlayerInfo *player, unsigned short nview);
void reset_player_mode(struct PlayerInfo *player, unsigned short nview);

void clear_players(void);

unsigned char rotate_mode_to_view_mode(unsigned char mode);

unsigned char get_player_color_idx(PlayerNumber plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
