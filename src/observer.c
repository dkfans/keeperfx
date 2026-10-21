#include "pre_inc.h"
#include "kfx/renderer/RendererManager.h"
#include "observer.h"

#include <math.h>

#include "bflib_datetm.h"
#include "bflib_math.h"
#include "bflib_mouse.h"
#include "bflib_planar.h"
#include "bflib_sprite.h"
#include "bflib_vidraw.h"
#include "bflib_video.h"
#include "config_spritecolors.h"
#include "dungeon_data.h"
#include "engine_redraw.h"
#include "engine_render.h"
#include "frontend.h"
#include "frontmenu_ingame_map.h"
#include "game_legacy.h"
#include "gui_frontmenu.h"
#include "gui_frontbtns.h"
#include "gui_parchment.h"
#include "gui_soundmsgs.h"
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
#include "thing_creature.h"
#include "thing_list.h"
#include "vidmode.h"
#include "keeperfx.hpp"
#include "post_inc.h"

#define OBSERVER_CURSOR_OFFSET_ORIGIN (UINT16_MAX + 2 - INT16_MIN)
#define OBSERVER_CURSOR_SIDEBAR_SHIFT (UINT16_MAX + 1)
#define OBSERVER_CURSOR_OFFSET_SCALE 256
#define OBSERVER_VIEW_ZOOM_SCALE 65536
#define OBSERVER_FRONT_VIEW_ZOOM_SCALE 8
#define OBSERVER_OWN_CURSOR_SOLID_TIME 100
#define OBSERVER_OWN_CURSOR_HIDE_TIME 2500
#define OBSERVER_OWN_CURSOR_HOTSPOT_X 12
#define OBSERVER_OWN_CURSOR_HOTSPOT_Y 15

struct ObserverCursor {
    struct Packet packet;
    int32_t viewport_center_offset[2];
    TbBool has_viewport_center_offset;
    TbBool sidebar_reserved;
};

struct PlayerInfo local_observer_player;
struct UserState local_observer_user_state;
TbBool local_observer_rendering;

static TbBool observer_initialized;
static NetUserId observed_user = -1;
static int32_t observer_view_mode = -1;
static struct ObserverCursor observer_cursors[MAX_NET_USERS];
static struct ObserverCursor observer_previous_cursors[MAX_NET_USERS];
static int32_t observer_cursor_screen_position[2];
static TbBool observer_cursor_visible;
static int32_t observer_mouse_position[2];
static TbClockMSec observer_mouse_move_time;
static TbBool observer_own_cursor_solid;
static const struct TbSprite *observer_faded_cursor_sprite;
static int32_t observer_faded_cursor_position[2];

TbBool observer_is_active(void)
{
    if (network_is_active() && network_user_is_spectator(netstate.my_id)) {
        return true;
    }
    return replay.load_enable && observer_initialized;
}

TbBool observer_is_following_user(void)
{
    return observer_is_active() && !is_observer_camera_active();
}

void observer_reset(void)
{
    observer_initialized = false;
    observed_user = -1;
    observer_view_mode = -1;
    local_observer_rendering = false;
    observer_cursor_visible = false;
    observer_own_cursor_solid = false;
    observer_faded_cursor_sprite = NULL;
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
    int32_t cursor_offset_x = lroundf((mouse_screen_x - local_state.engine_window_x - local_state.engine_window_width / 2) * screen_to_offset_scale);
    int32_t cursor_offset_y = lroundf((mouse_screen_y - local_state.engine_window_y - local_state.engine_window_height / 2) * screen_to_offset_scale);
    if (cursor_offset_x < INT16_MIN || cursor_offset_x > INT16_MAX || cursor_offset_y < INT16_MIN || cursor_offset_y > INT16_MAX) {
        return;
    }
    int32_t origin_x = OBSERVER_CURSOR_OFFSET_ORIGIN;
    if (engine_window_reserved_left() > 0) {
        origin_x += OBSERVER_CURSOR_SIDEBAR_SHIFT;
    }
    pckt->actn_par1 = -(cursor_offset_x + origin_x);
    pckt->actn_par2 = -(cursor_offset_y + OBSERVER_CURSOR_OFFSET_ORIGIN);
}

static TbBool observer_decode_cursor_offset(int32_t par, int32_t origin, int32_t *offset)
{
    int64_t value = -((int64_t)par + origin);
    if (value < INT16_MIN || value > INT16_MAX) {
        return false;
    }
    *offset = value;
    return true;
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
            int32_t *offset = cursor->viewport_center_offset;
            TbBool sidebar_reserved = observer_decode_cursor_offset(pckt->actn_par1, OBSERVER_CURSOR_OFFSET_ORIGIN + OBSERVER_CURSOR_SIDEBAR_SHIFT, &offset[0]);
            cursor->has_viewport_center_offset = pckt->action == PckA_None
                && (sidebar_reserved || observer_decode_cursor_offset(pckt->actn_par1, OBSERVER_CURSOR_OFFSET_ORIGIN, &offset[0]))
                && observer_decode_cursor_offset(pckt->actn_par2, OBSERVER_CURSOR_OFFSET_ORIGIN, &offset[1]);
            if (cursor->has_viewport_center_offset) {
                cursor->sidebar_reserved = sidebar_reserved;
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
            const struct PlayerInfo *player = get_player(i);
            if (player_exists(player) && player_is_seat(player)) {
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

static PlayerNumber position_observer_camera_at_starting_dungeon(PlayerNumber fallback_player, PlayerBitFlags players)
{
    PlayerNumber candidates[PLAYERS_COUNT];
    uint32_t candidate_count = 0;
    for (PlayerNumber i = 0; i < PLAYERS_COUNT; i++) {
        if (flag_is_set(players, to_flag(i)) && !dungeon_invalid(get_dungeon(i))) {
            candidates[candidate_count++] = i;
        }
    }
    if (candidate_count == 0) {
        return fallback_player;
    }
    const uint32_t pick = game.player_random_seed + netstate.my_id;
    const PlayerNumber plyr_idx = candidates[pick % candidate_count];
    const struct Thing *heart = get_player_soul_container(plyr_idx);
    const struct Coord3d *pos = thing_exists(heart) ? &heart->mappos : &get_dungeon(plyr_idx)->mappos;
    local_observer_user_state.dungeon_camera.x = pos->x.val;
    local_observer_user_state.dungeon_camera.y = pos->y.val;
    return plyr_idx;
}

NetUserId observer_get_watched_user(void)
{
    if (get_net_user_player_number(observed_user) == my_player_number) {
        return observed_user;
    }
    return -1;
}

void detach_observer_camera(void)
{
    if (is_observer_camera_active()) {
        return;
    }
    const unsigned char view_type = get_local_view_type(get_my_player());
    enter_observer_camera();
    if (view_type == PVT_MapScreen) {
        set_observer_camera_view(PVT_MapScreen);
    }
}

static const struct ObserverCursor *observer_get_view_cursor(void)
{
    if (!observer_is_active() || is_observer_camera_active()) {
        return NULL;
    }
    NetUserId user = observer_get_watched_user();
    if (user < 0 || user >= MAX_NET_USERS) {
        return NULL;
    }
    return &observer_cursors[user];
}

const struct Packet *observer_get_view_packet(void)
{
    const struct ObserverCursor *cursor = observer_get_view_cursor();
    if (cursor == NULL) {
        return NULL;
    }
    return &cursor->packet;
}

static int32_t observer_view_shift_x(void)
{
    const struct ObserverCursor *cursor = observer_get_view_cursor();
    if (cursor == NULL) {
        return 0;
    }
    int32_t player_reserved_left = 0;
    if (cursor->sidebar_reserved) {
        player_reserved_left = status_panel_width;
    }
    return (player_reserved_left - engine_window_reserved_left()) / 2;
}

static void observer_shift_view_camera(void)
{
    int32_t shift_x = observer_view_shift_x();
    if (shift_x == 0) {
        return;
    }
    struct DungeonCamera *dcam = &local_observer_user_state.dungeon_camera;
    TbBool front_view = local_observer_user_state.prefs[UPref_FrontView] != 0;
    int32_t world_per_pixel_scale = OBSERVER_VIEW_ZOOM_SCALE;
    if (front_view) {
        world_per_pixel_scale *= OBSERVER_FRONT_VIEW_ZOOM_SCALE;
    }
    int32_t distance = -shift_x * world_per_pixel_scale / (int32_t)scale_camera_zoom_to_screen(dcam->zoom[front_view]);
    int32_t angle = dcam->yaw[front_view] + DEGREES_90;
    set_view_position(&dcam->x, &dcam->y, move_coord_with_angle_x(dcam->x, distance, angle), move_coord_with_angle_y(dcam->y, distance, angle));
}

void observer_snap_view_camera(void)
{
    struct PlayerInfo *player = get_my_player();
    if ((observer_get_view_cursor() == NULL) || (get_local_view_type(player) != PVT_DungeonTop)) {
        return;
    }
    observer_update_view();
    set_local_camera_destination_pose(player, &local_observer_user_state.dungeon_camera);
    snap_local_camera_to_destination();
}

static TbBool observer_cursor_to_screen(const struct ObserverCursor *cursor, int32_t *screen_x, int32_t *screen_y)
{
    if (cursor->has_viewport_center_offset) {
        // Cursor offsets are relative to the viewport center and use a resolution-independent scale.
        float offset_to_screen_scale = units_per_pixel_best / (float)OBSERVER_CURSOR_OFFSET_SCALE;
        *screen_x = local_state.engine_window_x + local_state.engine_window_width / 2 + observer_view_shift_x() + lroundf(cursor->viewport_center_offset[0] * offset_to_screen_scale);
        *screen_y = local_state.engine_window_y + local_state.engine_window_height / 2 + lroundf(cursor->viewport_center_offset[1] * offset_to_screen_scale);
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

static void observer_set_faded_cursor(const struct TbSprite *spr, int32_t screen_x, int32_t screen_y, int32_t hot_x, int32_t hot_y)
{
    observer_faded_cursor_sprite = spr;
    observer_faded_cursor_position[0] = screen_x - scale_ui_value_lofi(hot_x);
    observer_faded_cursor_position[1] = screen_y - scale_ui_value_lofi(hot_y);
}

void observer_draw_faded_cursor(void)
{
    const struct TbSprite *spr = observer_faded_cursor_sprite;
    if (spr == NULL) {
        return;
    }
    observer_faded_cursor_sprite = NULL;
    unsigned short draw_flags = RendererGetDrawFlags();
    RendererSetDrawFlags(Lb_SPRITE_TRANSPAR4);
    LbSpriteDrawScaled(observer_faded_cursor_position[0], observer_faded_cursor_position[1], spr, scale_ui_value_lofi(spr->SWidth), scale_ui_value_lofi(spr->SHeight));
    RendererSetDrawFlags(draw_flags);
}

unsigned short observer_watched_cursor_draw_flags(void)
{
    if (observer_own_cursor_solid) {
        return Lb_SPRITE_TRANSPAR4;
    }
    return 0;
}

void observer_update_cursor(void)
{
    observer_cursor_visible = false;
    observer_own_cursor_solid = false;
    observer_faded_cursor_sprite = NULL;
    LbMouseSetDrawPosition(NULL);
    const struct ObserverCursor *cursor = observer_get_view_cursor();
    if (cursor == NULL) {
        return;
    }
    if (GetMouseX() != observer_mouse_position[0] || GetMouseY() != observer_mouse_position[1]) {
        observer_mouse_position[0] = GetMouseX();
        observer_mouse_position[1] = GetMouseY();
        observer_mouse_move_time = LbTimerClock();
    }
    TbClockMSec idle_time = LbTimerClock() - observer_mouse_move_time;
    observer_own_cursor_solid = idle_time <= OBSERVER_OWN_CURSOR_SOLID_TIME;
    const struct TbSprite *watched_sprite = lbDisplay.MouseSprite;
    int32_t hot_x = 0;
    int32_t hot_y = 0;
    GetPointerHotspot(&hot_x, &hot_y);
    if (observer_own_cursor_solid) {
        set_pointer_graphic(MousePG_Arrow);
    } else if (idle_time <= OBSERVER_OWN_CURSOR_HIDE_TIME) {
        const struct TbSprite *spr = get_sprite(pointer_sprites, get_player_colored_pointer_icon_idx(MousePG_Arrow, my_player_number));
        observer_set_faded_cursor(spr, GetMouseX(), GetMouseY(), OBSERVER_OWN_CURSOR_HOTSPOT_X, OBSERVER_OWN_CURSOR_HOTSPOT_Y);
    }
    if (gameplay_cursor_is_over_gui()) {
        return;
    }
    const struct Packet *pckt = &cursor->packet;
    const struct ObserverCursor *previous = &observer_previous_cursors[cursor - observer_cursors];
    observer_cursor_visible = observer_cursor_to_screen(cursor, &observer_cursor_screen_position[0], &observer_cursor_screen_position[1]);
    int32_t previous_screen_x;
    int32_t previous_screen_y;
    if (observer_cursor_visible && !flag_is_set(previous->packet.control_flags, PCtr_Gui)
     && packet_camera_context(&previous->packet) == packet_camera_context(pckt)
     && pckt->action != PckA_ZoomFromMap && pckt->action != PckA_BookmarkLoad
     && observer_cursor_to_screen(previous, &previous_screen_x, &previous_screen_y)) {
        observer_cursor_screen_position[0] = lroundf(interpolate(previous_screen_x, observer_cursor_screen_position[0]));
        observer_cursor_screen_position[1] = lroundf(interpolate(previous_screen_y, observer_cursor_screen_position[1]));
    }
    if (observer_own_cursor_solid) {
        if (observer_cursor_visible) {
            observer_set_faded_cursor(watched_sprite, observer_cursor_screen_position[0], observer_cursor_screen_position[1], hot_x, hot_y);
        }
    } else if (observer_cursor_visible) {
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
    if (!observer_is_following_user()) {
        return game_is_busy_doing_gui();
    }
    const struct Packet *pckt = observer_get_view_packet();
    return pckt == NULL || flag_is_set(pckt->control_flags, PCtr_Gui)
        || get_local_view_type(get_my_player()) != PVT_DungeonTop
        || game.small_map_state == 2 || a_menu_window_is_active();
}

TbBool get_gameplay_cursor_position(int32_t *screen_x, int32_t *screen_y)
{
    if (!observer_is_following_user()) {
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
    if (!is_observer_camera_active() && observer_get_watched_user() < 0) {
        detach_observer_camera();
    }
    unsigned char view_type = local_state.observer_camera_view_type;
    if (!is_observer_camera_active()) {
        const struct UserState *ustate = get_user_state(observer_get_watched_user());
        if (user_state_invalid(ustate)) {
            return;
        }
        local_observer_user_state.dungeon_camera = ustate->dungeon_camera;
        local_observer_user_state.prefs[UPref_MinimapZoom] = ustate->prefs[UPref_MinimapZoom];
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
    if (view_type == PVT_DungeonTop) {
        observer_shift_view_camera();
    }
    if (local_observer_user_state.view_type != view_type) {
        const TbBool map_toggled = (local_observer_user_state.view_type == PVT_MapScreen && view_type == PVT_DungeonTop)
            || (local_observer_user_state.view_type == PVT_DungeonTop && view_type == PVT_MapScreen);
        if (map_toggled && !is_observer_camera_active()) {
            start_local_map_fade(view_type);
        }
        local_observer_user_state.view_type = view_type;
        local_state.view_type = view_type;
        turn_off_query_menus();
        toggle_status_menu(view_type == PVT_DungeonTop && local_state.show_gui);
        if (view_type == PVT_CreatureContrl || view_type == PVT_CreaturePasngr) {
            turn_on_menu(GMnu_CREATURE_QUERY1);
            toggle_first_person_menu(false);
            toggle_first_person_menu(local_state.show_gui);
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

static void observer_clear_query(unsigned char state)
{
    turn_off_query_menus();
    if (local_state.show_gui) {
        turn_on_main_panel_menu();
    }
    local_observer_user_state.instance_num = PI_Unset;
    set_user_work_state(OBSERVER_USER, state, 0);
    clear_selected_thing(&local_observer_player);
}

static void observer_select_user(NetUserId user, PlayerNumber plyr_idx)
{
    clear_messages();
    observer_clear_query(PSt_CtrlDungeon);
    local_state.local_thing_under_hand = 0;
    observed_user = user;
    my_player_number = plyr_idx;
    local_observer_player.id_number = plyr_idx;
    update_gui_button_player_colors();
    update_creatr_model_activities_list(true);
    return_to_player_camera();
    reinit_tagged_blocks_for_player(plyr_idx);
    panel_map_update(0, 0, game.map_subtiles_x + 1, game.map_subtiles_y + 1);
}

static void observer_query_creature(ThingIndex index, TbBool reset, TbBool zoom)
{
    enter_observer_camera();
    query_creature(OBSERVER_USER, index, reset, zoom);
    process_user_instance(OBSERVER_USER);
}

TbBool observer_process_packet_action(const struct Packet *pckt)
{
    switch (pckt->action) {
    case PckA_SetPlyrState:
        if (pckt->actn_par1 == PSt_CreatrQuery || pckt->actn_par1 == PSt_CtrlDungeon) {
            enter_observer_camera();
            observer_clear_query(pckt->actn_par1);
        }
        return true;
    case PckA_PlyrQueryCreature: {
        struct Thing *thing = thing_get(pckt->actn_par1);
        if (thing_is_creature(thing)) {
            observer_query_creature(thing->index, (pckt->actn_par2 & 0x01) != 0, (pckt->actn_par2 & 0x02) != 0);
        }
        return true;
    }
    case PckA_TurnOffQuery:
        observer_clear_query(PSt_CreatrQuery);
        return true;
    default:
        return false;
    }
}

void observer_validate_query(void)
{
    if (is_observer_camera_active() && local_observer_user_state.work_state == PSt_CreatrInfo) {
        const struct Thing *selected = thing_get(local_observer_player.influenced_thing_idx);
        if (!thing_is_creature(selected) || selected->creation_turn != local_observer_player.influenced_thing_creation) {
            observer_clear_query(PSt_CreatrQuery);
        }
    }
}

void observer_update_query(void)
{
    local_state.local_thing_under_hand = 0;
    observer_validate_query();
    if (!is_observer_camera_active()) {
        local_state.local_thing_under_hand = get_user_state(get_displayed_user())->thing_under_hand;
        if (LbTimerClock() - observer_mouse_move_time > OBSERVER_OWN_CURSOR_HIDE_TIME) {
            return;
        }
    }
    struct PlayerInfo *player = get_my_player();
    if (get_local_view_type(player) != PVT_DungeonTop || game_is_busy_doing_gui()) {
        return;
    }
    struct Coord3d pos = {0};
    if (screen_to_map(get_local_active_camera(player), GetMouseX(), GetMouseY(), &pos)) {
        struct Thing *thing = get_creature_near(pos.x.val, pos.y.val);
        if (thing_is_creature(thing)) {
            local_state.local_thing_under_hand = thing->index;
        }
    }
}

TbBool observer_query_input(void)
{
    if (!is_observer_camera_active() || get_local_view_type(&local_observer_player) != PVT_DungeonTop) {
        return false;
    }
    struct Thing *thing = thing_get(local_state.local_thing_under_hand);
    if (left_button_released && thing_is_creature(thing) && local_state.show_gui) {
        observer_query_creature(thing->index, true, false);
        return true;
    }
    PlayerState work_state = local_observer_user_state.work_state;
    if (work_state != PSt_CreatrQuery && work_state != PSt_CreatrInfo) {
        return false;
    }
    if (right_button_released) {
        observer_clear_query(PSt_CtrlDungeon);
    }
    return true;
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
        watched_player = position_observer_camera_at_starting_dungeon(watched_player, starting_players);
    }
    observer_initialized = true;
    observer_select_user(get_player_primary_user(get_player(watched_player)), watched_player);
    set_gui_visible(false);
    local_state.show_panel = false;
}

/**
 * Switches the user displayed while watching a game and returns to the player camera.
 */
void observer_cycle_user(int step)
{
    NetUserId current = observer_get_watched_user();
    int tries = MAX_NET_USERS - 1;
    if (current < 0) {
        current = step > 0 ? MAX_NET_USERS - 1 : 0;
        tries = MAX_NET_USERS;
    }
    for (int i = 1; i <= tries; i++) {
        const NetUserId user = (current + step * i + MAX_NET_USERS) % MAX_NET_USERS;
        const PlayerNumber plyr_idx = get_net_user_player_number(user);
        if (plyr_idx < 0 || plyr_idx >= PLAYERS_COUNT) {
            continue;
        }
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
        observer_select_user(user, plyr_idx);
        return;
    }
}
