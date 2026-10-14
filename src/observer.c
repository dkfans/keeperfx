#include "pre_inc.h"
#include "observer.h"

#include <math.h>

#include "bflib_math.h"
#include "bflib_mouse.h"
#include "bflib_planar.h"
#include "bflib_video.h"
#include "dungeon_data.h"
#include "engine_redraw.h"
#include "engine_render.h"
#include "frontend.h"
#include "frontmenu_ingame_map.h"
#include "game_legacy.h"
#include "gui_frontmenu.h"
#include "gui_frontbtns.h"
#include "kjm_input.h"
#include "local_camera.h"
#include "map_columns.h"
#include "net_game.h"
#include "net_spectator.h"
#include "packets.h"
#include "player_data.h"
#include "player_instances.h"
#include "player_utils.h"
#include "replay.h"
#include "thing_objects.h"
#include "vidmode.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#define OBSERVER_CURSOR_OFFSET_ORIGIN (UINT16_MAX + 2 - INT16_MIN)
#define OBSERVER_CURSOR_OFFSET_SCALE 256

struct ObserverCursor {
    struct Packet packet;
    struct TbPoint screen_center_offset;
    TbBool has_screen_center_offset;
};

struct PlayerInfo local_observer_player;
struct UserState local_observer_user_state;
TbBool local_observer_rendering;

static TbBool observer_initialized;
static int32_t observer_view_mode = -1;
static struct ObserverCursor observer_cursors[MAX_NET_USERS];
static struct ObserverCursor observer_previous_cursors[MAX_NET_USERS];
static int32_t observer_cursor_screen_position[2];
static TbBool observer_cursor_visible;

TbBool observer_is_active(void)
{
    if (network_is_active() && network_user_is_spectator(netstate.my_id)) {
        return true;
    }
    return replay.load_enable && observer_initialized;
}

void observer_reset(void)
{
    observer_initialized = false;
    observer_view_mode = -1;
    local_observer_rendering = false;
    observer_cursor_visible = false;
    LbMouseSetDrawPosition(NULL);
    memset(&local_observer_player, 0, sizeof(local_observer_player));
    memset(&local_observer_user_state, 0, sizeof(local_observer_user_state));
    memset(observer_cursors, 0, sizeof(observer_cursors));
    memset(observer_previous_cursors, 0, sizeof(observer_previous_cursors));
}

void observer_set_packet_cursor(struct Packet *pckt)
{
    if (pckt->action != PckA_None || get_local_view_type(get_my_player()) != PVT_DungeonTop) {
        return;
    }
    int32_t mouse_screen_x = GetMouseX();
    int32_t mouse_screen_y = GetMouseY();
    if (units_per_pixel_best == 0 || mouse_screen_x < local_state.engine_window_x || mouse_screen_y < local_state.engine_window_y
     || mouse_screen_x >= local_state.engine_window_x + local_state.engine_window_width
     || mouse_screen_y >= local_state.engine_window_y + local_state.engine_window_height) {
        return;
    }
    float screen_to_offset_scale = (float)OBSERVER_CURSOR_OFFSET_SCALE / units_per_pixel_best;
    int32_t cursor_offset_x = lroundf((mouse_screen_x - MyScreenWidth / 2) * screen_to_offset_scale);
    int32_t cursor_offset_y = lroundf((mouse_screen_y - MyScreenHeight / 2) * screen_to_offset_scale);
    if (cursor_offset_x < INT16_MIN || cursor_offset_x > INT16_MAX || cursor_offset_y < INT16_MIN || cursor_offset_y > INT16_MAX) {
        return;
    }
    pckt->actn_par1 = -(cursor_offset_x + OBSERVER_CURSOR_OFFSET_ORIGIN);
    pckt->actn_par2 = -(cursor_offset_y + OBSERVER_CURSOR_OFFSET_ORIGIN);
}

void observer_store_packets(const struct Packet *packets)
{
    if (!observer_is_active() || (packets != NULL && replay_playback_is_paused())) {
        return;
    }
    memcpy(observer_previous_cursors, observer_cursors, sizeof(observer_previous_cursors));
    if (packets != NULL) {
        for (NetUserId user = 0; user < MAX_NET_USERS; user++) {
            const struct Packet *pckt = &packets[user];
            struct ObserverCursor *cursor = &observer_cursors[user];
            cursor->packet = *pckt;
            cursor->has_screen_center_offset = false;
            if (pckt->action == PckA_None
             && pckt->actn_par1 <= -(OBSERVER_CURSOR_OFFSET_ORIGIN + INT16_MIN)
             && pckt->actn_par1 >= -(OBSERVER_CURSOR_OFFSET_ORIGIN + INT16_MAX)
             && pckt->actn_par2 <= -(OBSERVER_CURSOR_OFFSET_ORIGIN + INT16_MIN)
             && pckt->actn_par2 >= -(OBSERVER_CURSOR_OFFSET_ORIGIN + INT16_MAX)) {
                cursor->screen_center_offset.x = -pckt->actn_par1 - OBSERVER_CURSOR_OFFSET_ORIGIN;
                cursor->screen_center_offset.y = -pckt->actn_par2 - OBSERVER_CURSOR_OFFSET_ORIGIN;
                cursor->has_screen_center_offset = true;
            }
        }
    }
}

static PlayerBitFlags get_observer_starting_players(void)
{
    PlayerBitFlags players = 0;
    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++) {
        const struct PlayerInfo *player = get_player(i);
        if (is_active_keeper(player) && !flag_is_set(player->allocflags, PlaF_CompCtrl)) {
            set_flag(players, to_flag(i));
        }
    }
    return players;
}

void observer_update_vision(void)
{
    PlayerBitFlags players = 0;
    if (is_observer_camera_active()) {
        for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++) {
            if (player_has_heart(i)) {
                set_flag(players, to_flag(i));
            }
        }
    } else {
        players = get_player_vision_mask(my_player_number);
    }
    if (local_observer_player.allied_players != players) {
        local_observer_player.allied_players = players;
        panel_map_update(0, 0, game.map_subtiles_x + 1, game.map_subtiles_y + 1);
    }
}

static PlayerNumber position_observer_camera_at_random_heart(PlayerNumber fallback_player, PlayerBitFlags players)
{
    struct Thing *starting_heart = NULL;
    int32_t heart_count = 0;
    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++) {
        if (!flag_is_set(players, to_flag(i))) {
            continue;
        }
        struct Thing *heart = get_player_soul_container(i);
        if (!thing_exists(heart)) {
            continue;
        }
        heart_count += 1;
        if (UNSYNC_RANDOM(heart_count) == 0) {
            starting_heart = heart;
        }
    }
    if (starting_heart != NULL) {
        local_observer_user_state.dungeon_camera.x = starting_heart->mappos.x.val;
        local_observer_user_state.dungeon_camera.y = starting_heart->mappos.y.val;
        return starting_heart->owner;
    }
    return fallback_player;
}

const struct Packet *observer_get_view_packet(void)
{
    if (!observer_is_active() || is_observer_camera_active()) {
        return NULL;
    }
    const struct PlayerInfo *player = get_player(my_player_number);
    NetUserId user = get_player_primary_user(player);
    if (user < 0 || user >= MAX_NET_USERS) {
        return NULL;
    }
    return &observer_cursors[user].packet;
}

static TbBool observer_cursor_to_screen(const struct ObserverCursor *cursor, int32_t *screen_x, int32_t *screen_y)
{
    if (cursor->has_screen_center_offset) {
        // Cursor positioning is correct and accounts for whether the sidebar is hidden or visible.
        float offset_to_screen_scale = units_per_pixel_best / (float)OBSERVER_CURSOR_OFFSET_SCALE;
        *screen_x = MyScreenWidth / 2 + lroundf(cursor->screen_center_offset.x * offset_to_screen_scale);
        *screen_y = MyScreenHeight / 2 + lroundf(cursor->screen_center_offset.y * offset_to_screen_scale);
        return *screen_x >= local_state.engine_window_x && *screen_y >= local_state.engine_window_y
            && *screen_x < local_state.engine_window_x + local_state.engine_window_width
            && *screen_y < local_state.engine_window_y + local_state.engine_window_height;
    }
    if (!flag_is_set(cursor->packet.control_flags, PCtr_MapCoordsValid)) {
        return false;
    }
    struct Coord3d cursor_map_position;
    cursor_map_position.x.val = cursor->packet.pos_x;
    cursor_map_position.y.val = cursor->packet.pos_y;
    cursor_map_position.z.val = get_floor_height_at(&cursor_map_position);
    return map_to_screen(&cursor_map_position, screen_x, screen_y);
}

void observer_update_cursor(void)
{
    observer_cursor_visible = false;
    LbMouseSetDrawPosition(NULL);
    const struct Packet *pckt = observer_get_view_packet();
    if (pckt == NULL || gameplay_cursor_is_over_gui()) {
        return;
    }
    NetUserId user = get_player_primary_user(get_player(my_player_number));
    const struct ObserverCursor *previous = &observer_previous_cursors[user];
    observer_cursor_visible = observer_cursor_to_screen(&observer_cursors[user], &observer_cursor_screen_position[0], &observer_cursor_screen_position[1]);
    int32_t previous_screen_x;
    int32_t previous_screen_y;
    if (observer_cursor_visible && !flag_is_set(previous->packet.control_flags, PCtr_Gui)
     && packet_camera_context(&previous->packet) == packet_camera_context(pckt)
     && pckt->action != PckA_ZoomFromMap && pckt->action != PckA_BookmarkLoad
     && observer_cursor_to_screen(previous, &previous_screen_x, &previous_screen_y)) {
        observer_cursor_screen_position[0] = lroundf(interpolate(previous_screen_x, observer_cursor_screen_position[0]));
        observer_cursor_screen_position[1] = lroundf(interpolate(previous_screen_y, observer_cursor_screen_position[1]));
    }
    if (observer_cursor_visible) {
        struct TbPoint cursor_draw_position;
        cursor_draw_position.x = observer_cursor_screen_position[0] / pixel_size;
        cursor_draw_position.y = observer_cursor_screen_position[1] / pixel_size;
        LbMouseSetDrawPosition(&cursor_draw_position);
    } else {
        set_pointer_graphic(MousePG_Invisible);
    }
}

TbBool gameplay_cursor_is_over_gui(void)
{
    if (get_displayed_player() == get_my_player()) {
        return game_is_busy_doing_gui();
    }
    const struct Packet *pckt = observer_get_view_packet();
    return pckt == NULL || flag_is_set(pckt->control_flags, PCtr_Gui)
        || get_local_view_type(get_my_player()) != PVT_DungeonTop
        || game.small_map_state == 2 || a_menu_window_is_active();
}

TbBool get_gameplay_cursor_position(int32_t *screen_x, int32_t *screen_y)
{
    if (get_displayed_player() == get_my_player()) {
        *screen_x = GetMouseX();
        *screen_y = GetMouseY();
        return true;
    }
    if (!observer_cursor_visible || gameplay_cursor_is_over_gui()) {
        return false;
    }
    *screen_x = observer_cursor_screen_position[0];
    *screen_y = observer_cursor_screen_position[1];
    return true;
}

void observer_update_view(void)
{
    observer_update_vision();
    const struct PlayerInfo *player = get_player(my_player_number);
    const struct Dungeon *dungeon = get_players_dungeon(player);
    if (dungeon_invalid(dungeon)) {
        return;
    }
    game.creatures_tend_imprison = (dungeon->creature_tendencies & CrTend_Imprison) != 0;
    game.creatures_tend_flee = (dungeon->creature_tendencies & CrTend_Flee) != 0;
    unsigned char view_type = local_state.observer_camera_view_type;
    if (!is_observer_camera_active()) {
        const struct UserState *ustate = get_player_user_state(player);
        if (user_state_invalid(ustate)) {
            return;
        }
        local_observer_user_state.dungeon_camera = ustate->dungeon_camera;
        local_observer_user_state.prefs[UPref_FrontView] = ustate->prefs[UPref_FrontView];
        if (observer_view_mode < 0) {
            local_observer_user_state.prefs[UPref_Wibble] = ustate->prefs[UPref_Wibble];
        } else {
            TbBool front_view;
            TbBool wibble = local_observer_user_state.prefs[UPref_Wibble] != 0;
            rotate_mode_to_dungeon_view(observer_view_mode, &front_view, &wibble);
            local_observer_user_state.prefs[UPref_FrontView] = front_view;
            local_observer_user_state.prefs[UPref_Wibble] = wibble;
        }
        view_type = ustate->view_type;
    }
    if (view_type == PVT_MapFadeIn) {
        view_type = PVT_MapScreen;
    } else if (view_type == PVT_MapFadeOut) {
        view_type = PVT_DungeonTop;
    }
    if (local_observer_user_state.view_type != view_type) {
        local_observer_user_state.view_type = view_type;
        local_state.view_type = view_type;
        turn_off_query_menus();
        toggle_status_menu(view_type == PVT_DungeonTop && (game.operation_flags & GOF_ShowGui) != 0);
        if (view_type == PVT_CreatureContrl || view_type == PVT_CreaturePasngr) {
            turn_on_menu(GMnu_CREATURE_QUERY1);
            toggle_first_person_menu(false);
            toggle_first_person_menu((game.operation_flags & GOF_ShowGui) != 0);
        }
        setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
    }
}

void observer_set_view_mode(unsigned char mode)
{
    observer_view_mode = mode;
    TbBool front_view;
    TbBool wibble = local_observer_user_state.prefs[UPref_Wibble] != 0;
    rotate_mode_to_dungeon_view(mode, &front_view, &wibble);
    set_engine_view(&local_observer_player, front_view, wibble);
    if (is_observer_camera_active()) {
        local_state.observer_camera_idx = CamIV_Isometric;
        if (front_view) {
            local_state.observer_camera_idx = CamIV_FrontView;
        }
    }
}

static void observer_select_player(PlayerNumber plyr_idx)
{
    my_player_number = plyr_idx;
    local_observer_player.id_number = plyr_idx;
    update_gui_button_player_colors();
    update_creatr_model_activities_list(true);
    return_to_player_camera();
    reinit_tagged_blocks_for_player(plyr_idx);
    panel_map_update(0, 0, game.map_subtiles_x + 1, game.map_subtiles_y + 1);
}

void observer_init(PlayerNumber camera_player_number)
{
    observer_reset();
    build_local_user_preferences(local_observer_user_state.prefs);
    const struct UserState *camera_state = get_player_user_state(get_player(camera_player_number));
    if (!user_state_invalid(camera_state)) {
        local_observer_user_state.dungeon_camera = camera_state->dungeon_camera;
        local_observer_user_state.prefs[UPref_FrontView] = camera_state->prefs[UPref_FrontView];
        local_observer_user_state.prefs[UPref_Wibble] = camera_state->prefs[UPref_Wibble];
    }
    local_observer_player.allocflags = PlaF_Allocated;
    local_observer_player.victory_state = VicS_LostLevel;
    local_observer_user_state.work_state = PSt_CtrlDungeon;
    local_observer_user_state.continue_work_state = PSt_CtrlDungeon;
    local_observer_user_state.roomspace_width = 1;
    local_observer_user_state.roomspace_height = 1;
    local_observer_user_state.roomspace_detection_looseness = DEFAULT_USER_ROOMSPACE_DETECTION_LOOSENESS;
    local_observer_user_state.view_type = PVT_DungeonTop;
    local_observer_user_state.teleport_destination = 19;
    local_observer_user_state.battleid = 1;
    const PlayerBitFlags starting_players = get_observer_starting_players();
    PlayerNumber watched_player = my_player_number;
    if (watched_player == PLAYER_NEUTRAL) {
        for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++) {
            if (flag_is_set(starting_players, to_flag(i))) {
                watched_player = i;
                break;
            }
        }
    }
    if (!replay.load_enable) {
        watched_player = position_observer_camera_at_random_heart(watched_player, starting_players);
    }
    observer_initialized = true;
    observer_select_player(watched_player);
    set_gui_visible(false);
    clear_flag(game.operation_flags, GOF_ShowPanel);
}

/**
 * Switches the player displayed while watching a game.
 */
void observer_cycle_player(int step)
{
    for (int i = 1; i < PLAYERS_COUNT; i++) {
        const PlayerNumber plyr_idx = (my_player_number + step * i + PLAYERS_COUNT) % PLAYERS_COUNT;
        if (replay.load_enable) {
            if (!flag_is_set(replay.head.players_exist, to_flag(plyr_idx))
             || flag_is_set(replay.head.players_comp, to_flag(plyr_idx))) {
                continue;
            }
        } else {
            const struct PlayerInfo *player = get_player(plyr_idx);
            if (!is_active_keeper(player) || flag_is_set(player->allocflags, PlaF_CompCtrl)) {
                continue;
            }
        }
        observer_select_player(plyr_idx);
        return;
    }
}
