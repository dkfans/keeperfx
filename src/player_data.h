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
#include "net_main.h"
#include "user_prefs.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#define PLAYERS_COUNT       9
#define COLOURS_COUNT       9

#define INVALID_PLAYER (&bad_player)
#define INVALID_USER_STATE (&bad_user_state)

#define PLAYER_MP_MESSAGE_LEN  64

#define MINIMAP_ZOOM_MIN       128
#define MINIMAP_ZOOM_DEFAULT   256
#define MINIMAP_ZOOM_MAX       2048

#define WANDER_POINTS_COUNT    200

enum PlayerInitFlags {
    PlaF_Allocated               = 0x01,
    PlaF_Placeholder             = 0x02, /**< Human user disconnected, replaced by a computer > */
    PlaF_Seat                    = 0x04, /**< Playable by a human: includes humans (and placeholders), and lobby-assigned computers > */
    PlaF_CompCtrl                = 0x40,
};

enum PlayerField6Flags {
    PlaF6_PlyrHasQuit       = 0x02,
};

enum LocalViewMode {
    PVM_IsoWibbleView = 2, /**< Isometric dungeon view, freely rotating, with wibble. */
    PVM_FrontView = 5, /**< Front dungeon view, four orientations. */
    PVM_IsoStraightView = 8, /**< Same as PVM_IsoWibbleView, but without wibble. */
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

enum UserInitFlags {
    UsrIF_NewMPMessage            = 0x04,
    UsrIF_CreaturePassengerMode   = 0x08,
    UsrIF_KeyboardInputDisabled   = 0x10,
    UsrIF_MouseInputDisabled      = 0x80,
};

enum UserAdditionalFlags {
    UsrAF_None                      = 0x00,
    UsrAF_NoThingUnderPowerHand     = 0x01, // Chosen subtile has nothing to interact with with the Power Hand (no creature to slap etc) (But the power hand is active)
    UsrAF_ChosenSubTileIsHigh       = 0x02, // Chosen subtile is at ceiling height (dirt/rock/wall etc)
    UsrAF_FreezePaletteIsActive     = 0x04, // blue_palette is being used during Freeze Spell. TODO: move to LocalState
    UsrAF_LightningPaletteIsActive  = 0x08, // lightning_palette is being used during Lightning Spell. TODO: move to LocalState
    UsrAF_UnlockedLordTorture       = 0x10, // if this flag is set, the user will be sent to the Lord Torture Mini-game
    UsrAF_Unkn20                    = 0x20,
    UsrAF_Unkn40                    = 0x40,
    UsrAF_Unkn80                    = 0x80,
};

enum PlayerTypes {
    // a proper player with room flames, payday, captures neutrals
    PT_Keeper,
    
    // hero-style player: lacks room flames, sees whole map, can't capture neutral, no economy.
    PT_Roaming,
    
    // capturable by nearby keepers
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
  uint32_t points_count;
  /** Index at which the search function inserts (or replaces) points. */
  uint32_t point_insert_idx;
  /** Slab last checked by the search function. */
  uint32_t last_checked_slb_num;
  /** Amount of slabs to be checked in one run of the search function. */
  uint32_t num_check_per_run;
  /** Max amount of points added in one run of the search function. */
  uint32_t max_found_per_check;
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

/*
 * Per-player game data.
 *
 * Players present in a game can be human-controlled keepers, Computer-controlled keepers,
 * "Placeholder" keepers (humans who have dropped and are replaced by a computer),
 * "Roaming" (the hero team), or neutral (capturable by keepers).
 * 
 * Note: for legacy reasons, this struct currently contains some fields that
 * should eventually be ported to UserState or LocalState.
*/
struct PlayerInfo {
    unsigned char allocflags;
    unsigned char display_flags;
    char player_name[20];
    unsigned char victory_state;
    PlayerBitFlags allied_players;
    PlayerBitFlags players_with_locked_ally_status;
    unsigned char id_number;
    TbBool unused;
    struct Wander wandr_within;
    struct Wander wandr_outside;
    short cta_flag_idx;
    /** Hand style of this keeper; copied to users when they take control of the keeper. */
    unsigned char default_hand_idx;
    GameTurn power_of_cooldown_turn;
    int32_t game_version;
    GameTurn display_objective_turn;
    unsigned char player_type; //enum PlayerTypes
    ThingModel special_digger;
    unsigned short generate_speed;
};

#define PLAYER_NONE (-1)

/* Game state that exists per human user. Computer-controlled
 * players do not have users.
 *
 * Local games only have a single user. Networked games have one
 * user per client including the host.
 */
struct UserState {
    PlayerNumber player_id;
    unsigned char init_flags; // Uses UserInitFlags
    unsigned char additional_flags; // Uses UserAdditionalFlags
    unsigned char input_crtr_control;
    unsigned char input_crtr_query;
    short cursor_light_idx;
    /** Cursor position, and the subtile it last clicked on. */
    MapSubtlCoord cursor_subtile_x;
    MapSubtlCoord cursor_subtile_y;
    MapSubtlCoord previous_cursor_subtile_x;
    MapSubtlCoord previous_cursor_subtile_y;
    MapSubtlCoord cursor_clicked_subtile_x;
    MapSubtlCoord cursor_clicked_subtile_y;
    unsigned char cursor_button_down; // left or right button down (whilst using the bounding box cursor)
    TbBool mouse_on_map;
    unsigned char primary_cursor_state;
    unsigned char secondary_cursor_state;
    TbBool one_click_mode_exclusive;
    TbBool one_click_lock_cursor;
    TbBool ignore_next_PCtr_RBtnRelease;
    TbBool ignore_next_PCtr_LBtnRelease;
    char swap_to_untag_mode;
    TbBool interpolated_tagging;
    /** First person (possession) controls. */
    TbBool first_person_dig_claim_mode;
    unsigned short selected_fp_thing_pickup;
    unsigned char teleport_destination;
    TbBool nearest_teleport;
    BattleIndex battleid;
    struct CheatSelection cheatselection;
    unsigned char boxsize;
    unsigned char chosen_room_kind;
    unsigned char full_slab_cursor; // 0 for subtile sized cursor, 1 for slab sized cursor
    ThingModel chosen_trap_kind;
    ThingModel chosen_door_kind;
    PowerKind chosen_power_kind;
    TbBool pickup_all_gold;
    unsigned char view_type;
    struct DungeonCamera dungeon_camera;
    /** Target of the PI_ZoomToPos camera move. */
    MapCoord zoom_to_pos_x;
    MapCoord zoom_to_pos_y;
    /** Chat message being typed, the last sent one (for recall), and a pending one. */
    char mp_message_text[PLAYER_MP_MESSAGE_LEN];
    char mp_pending_message[PLAYER_MP_MESSAGE_LEN];
    char mp_message_text_last[PLAYER_MP_MESSAGE_LEN];
    UserPreferences prefs;
    PlayerState work_state;
    PlayerState continue_work_state;
    short thing_under_hand;
    short hand_thing_idx;
    /** Game turn until which this user's hand is busy (slap/pickup cooldown). */
    unsigned int hand_busy_until_turn;
    /** Hand style (index into power hand config). */
    unsigned char hand_idx;
    /** Target of the user's current instance (creature being possessed, thing being grabbed, etc), or room for console commands; with its creation turn for validation. */
    short influenced_thing_idx;
    GameTurn influenced_thing_creation;
    /** Overcharge level while casting keeper powers. */
    int32_t cast_expand_level;
    int32_t hand_animationId;
    struct RoomSpace render_roomspace;
    /** In-progress room build/sell selection being applied over several turns. */
    struct RoomSpace roomspace;
    unsigned char roomspace_mode;
    int roomspace_detection_looseness;
    int roomspace_width;
    int roomspace_height;
    unsigned char roomspace_highlight_mode;
    TbBool roomspace_drag_paint_mode;
    unsigned char roomspace_l_shape;
    TbBool roomspace_horizontal_first;
    /** Possession: the thing this user is currently controlling (direct control or passenger). */
    short controlled_thing_idx;
    GameTurn controlled_thing_creatrn;
    TbBool possession_lock;
    /** An "instance" is a short, scripted animation. See: enum PlayerInstanceNum */
    unsigned char instance_num;
    unsigned long instance_remain_turns;
};

/******************************************************************************/

extern unsigned char my_player_number;

#pragma pack()
/******************************************************************************/

struct LocalCameraState {
    struct Camera current[CamIV_EndList];
    struct Camera previous[CamIV_EndList];
    struct Camera destination[CamIV_EndList];
    float previous_deviation_x;
    float previous_deviation_y;
    float destination_deviation_x;
    float destination_deviation_y;
    TbBool ready;
    MapCoord move_target[2];
    MapCoordDelta move_delta[2];
    struct Camera *move_cam;
    TbBool rotation_pending;
    int32_t rotation_angle;
    TbBool has_rotation_pivot;
    MapCoord rotation_pivot_x;
    MapCoord rotation_pivot_y;
    TbBool first_person_look_pending;
    int32_t first_person_look_yaw;
    int32_t first_person_look_pitch;
    int32_t first_person_look_roll;
    int32_t possession_start_zoom;
};

/* Miscellaneous state relating to this device and
 * the local human player.
 *
 * Not sync'd over the network.
 */
extern struct LocalState {
    unsigned char view_type;
    TbBool tooltips_restore; /**< Used to store/restore the value of settings.tooltips_on when transitioning to/from the map. */
    TbBool status_menu_restore; /**< Used to store/restore the current status menu visibility when the map is shown/hidden. */
    TbBool status_menu_hidden_for_map; /**< The status menu is hidden for the map and status_menu_restore holds its visibility. */
    TbBool tooltips_hidden_for_map; /**< Tooltips are off for a map fade and tooltips_restore holds the setting. */
    TbBool paused_state_restore; /**< Used to restore pause state after saving */
    TbBool display_needs_update;
    short local_thing_under_hand;
    TbBool swipe_sprite_drawLR; /**< Used to decide whether to draw the swipe sprite left to right (TRUE), or [default] right to left (FALSE). */
    unsigned char *lens_palette;
    unsigned char *main_palette;
    TbBool map_fade_prepared;
    int32_t palette_fade_step_pain;
    int32_t palette_fade_step_possession;
    short engine_window_width;
    short engine_window_height;
    short engine_window_x;
    short engine_window_y;
    short minimap_pos_x;
    short minimap_pos_y;
    int roomspace_size;
    // FIXME: use fixed-point precision instead
    float camera_movement_x;
    float camera_movement_y;
    TbBool camera_speedup_pressed;
    TbBool camera_rotate_cw;
    TbBool camera_rotate_ccw;
    TbBool camera_rotate_around_cursor;
    struct LocalCameraState camera;
    TbBool observer_camera_active;
    unsigned char observer_camera_view_type;
    unsigned char observer_camera_idx;
    int32_t fast_forward;
    long double process_turn_time;
    float delta_time;
    uint32_t time_delta;
    TbBool show_gui;
    TbBool show_panel;
} local_state;

extern unsigned short player_colors_map[];
extern TbPixel player_path_colours[];
extern TbPixel player_room_colours[];
extern TbPixel player_flash_colours[];
extern TbPixel player_highlight_colours[];
extern TbPixel possession_hit_colours[];
extern unsigned short const player_cubes[];
extern struct PlayerInfo bad_player;
extern struct UserState bad_user_state;
/******************************************************************************/
struct PlayerInfo *get_player_f(PlayerNumber plyr_idx,const char *func_name);
#define get_player(plyr_idx) get_player_f(plyr_idx,__func__)
struct PlayerInfo *get_my_player(void);
const struct PlayerInfo *get_displayed_player(void);
TbBool player_invalid(const struct PlayerInfo *player);
TbBool player_exists(const struct PlayerInfo *player);
TbBool is_active_keeper(const struct PlayerInfo *player);
TbBool is_my_player(const struct PlayerInfo *player);
struct UserState *get_user_state(NetUserId user);
struct UserState *get_player_user_state(const struct PlayerInfo *player);
struct UserState *get_local_user_state(void);
uint32_t get_local_minimap_zoom(void);
TbBool user_state_invalid(const struct UserState *ustate);
int32_t user_zoom_min(const struct UserState *ustate, TbBool front_view);
TbBool is_my_player_number(PlayerNumber plyr_num);
TbBool player_allied_with(const struct PlayerInfo *player, PlayerNumber ally_idx);
TbBool players_are_enemies(PlayerNumber plyr1_idx, PlayerNumber plyr2_idx);
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

void set_user_work_state(NetUserId user, short nwrk_state, int32_t chosen_kind);
void set_user_view_type(NetUserId user, unsigned short nview);
void reset_user_view_type(NetUserId user, unsigned short nview);

void clear_players(void);

TbBool player_has_user_in_dungeon_view(const struct PlayerInfo *player);
unsigned char get_player_script_view_type(const struct PlayerInfo *player);
ThingIndex get_player_script_controlled_thing(const struct PlayerInfo *player);
unsigned char get_player_view_type(const struct PlayerInfo *player);
/** User state to use for LOCAL-ONLY presentation (camera, cursor, hand drawing): the local user's own state when the player is the one it drives. Never use in simulation. */
struct UserState *get_player_local_user_state(const struct PlayerInfo *player);
unsigned char get_player_local_view_type(const struct PlayerInfo *player);
unsigned char get_player_active_camera_index(const struct PlayerInfo *player);
unsigned char get_user_active_camera_index(const struct UserState *ustate);
int32_t get_player_dungeon_yaw(const struct PlayerInfo *player);
enum LocalViewMode get_dungeon_view_mode(const struct UserState *ustate);
void rotate_mode_to_dungeon_view(unsigned char mode, TbBool *front_view, TbBool *wibble);
NetUserId get_player_primary_user(const struct PlayerInfo *player);
void set_player_users_hand_idx(PlayerNumber plyr_idx, unsigned char hand_idx);
// Bitmask (1 << user) of every user driving this keeper; 0 if none. Use for effects that apply to all of a keeper's users.
uint32_t get_player_user_mask(const struct PlayerInfo *player);
// True if 'user' is one of the users driving 'player' (false for the observer and for invalid players).
TbBool user_drives_player(NetUserId user, const struct PlayerInfo *player);
// Runs the following statement for every user driving 'player'. Variable 'user' is declared by the macro.
// The empty-if form keeps a following 'else' from binding to the macro's hidden condition.
#define FOR_EACH_PLAYER_USER(user, player)     for (NetUserId user = 0; user < MAX_NET_USERS; ++user)         if (!user_drives_player(user, player)) {} else
/** User whose packet is being processed right now, or -1. Set only by process_packets(), never from local state. */
void set_acting_user(NetUserId user);
/** Returns the acting user if it belongs to the given player, otherwise the player's primary user (script/AI callers). */
NetUserId get_player_acting_user(const struct PlayerInfo *player);
NetUserId get_thing_controlling_user(const struct Thing *thing);
/** True if a user other than the given one possesses (or is about to possess) the thing. */
TbBool is_thing_possessed_by_other_user(const struct Thing *thing, NetUserId user);
/** Returns the user possessing the thing, or the player's first user if nobody is possessing it. */
struct UserState *get_thing_user_state_or_primary(const struct Thing *thing, const struct PlayerInfo *player);
struct PlayerInfo *get_user_player(NetUserId user);

unsigned char get_player_color_idx(PlayerNumber plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
