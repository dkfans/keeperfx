/******************************************************************************/
// Free implementation of the Dungeon Keeper strategy game.
/******************************************************************************/
/** @file archon_test.c
 *     Automated archon (shared keeper) co-op test bot, enabled by -archontest.
 * @par Purpose:
 *     Every client drives its local user with a scripted bot. The bot only reads
 *     game state and fills the local packet the way human input does (actions,
 *     cursor position, mouse button flags), plus the local camera movement input.
 *     It never changes the simulation directly, so everything goes through lockstep.
 *     Results go to archontest_<user>.log in the working directory.
 * @par Comment:
 *     Scenario: host is user 0, client is user 1, both drive the same keeper.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "archon_test.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "globals.h"
#include "game_legacy.h"
#include "packets.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "creature_control.h"
#include "thing_data.h"
#include "thing_creature.h"
#include "power_hand.h"
#include "bflib_planar.h"
#include "config_magic.h"
#include "config_terrain.h"
#include "room_data.h"
#include "room_list.h"
#include "slab_data.h"
#include "map_data.h"
#include "tasks_list.h"
#include "local_camera.h"
#include "net_game.h"
#include "net_main.h"
#include "game_merge.h"
#include "bflib_datetm.h"
#include "bflib_fileio.h"
#include "config.h"
#include "config_campaigns.h"
#include "front_archon.h"
#include "front_landview.h"
#include "front_lvlstats.h"
#include "frontend.h"
#include "gui_frontmenu.h"
#include "keeperfx.hpp"
#include "game_saves.h"
#include "player_utils.h"
#include "player_instances.h"
#include "frontmenu_net.h"
#include "front_network.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
int32_t archontest_seconds = 0;
int32_t archontest_flow_levels = 0;
TbBool archontest_campaign = false;

#define AT_MAX_USERS 2
#define AT_MAX_DIG 4
#define AT_MAX_CHECKS 96

enum AtStep {
    AtS_Start = 0,
    AtS_Cheats,
    AtS_WaitGrants,
    AtS_Dig,
    AtS_Room,
    AtS_HandWait,
    AtS_Hand,
    AtS_Slap,
    AtS_PossessWait,
    AtS_Possess,
    AtS_Camera,
    AtS_Idle,
    AtS_Done,
};

struct AtCheck {
    char name[32];
    int state; // 0 pending, 1 pass, 2 fail
    char detail[160];
};

/** What the bot puts into the local packet this turn (applied on every input() call of the turn). */
struct AtPacket {
    unsigned char action;
    int32_t par1, par2;
    uint16_t par3, par4;
    TbBool has_pos;
    MapCoord pos_x, pos_y;
    unsigned char context;
    uint32_t flags;
    uint8_t addvals;
};

struct AtState {
    TbBool started;
    FILE *log;
    NetUserId me;
    PlayerNumber plyr;
    uint32_t rng;
    GameTurn end_turn;
    GameTurn built_turn;
    struct AtPacket pk;
    int step;
    int sub;
    GameTurn step_turn;
    GameTurn sub_turn;
    TbBool desync;
    GameTurn desync_turn;
    struct AtCheck checks[AT_MAX_CHECKS];
    int checks_num;
    // scenario data
    MapSlabCoord heart_slb_x, heart_slb_y;
    MapSlabCoord dig_x[AT_MAX_DIG], dig_y[AT_MAX_DIG];
    int dig_num;
    int dig_idx;
    long rooms_base[2];
    RoomKind room_kind;
    MapSlabCoord room_x, room_y;
    ThingIndex hand_tng;
    MapSlabCoord drop_x, drop_y;
    unsigned long slaps_base;
    ThingIndex possess_a, possess_b;
    int possess_stage;
    MapCoord cam_start_x, cam_start_y;
    // observations of synced state
    GameTurn grants_start;
    GameTurn last_cksum_turn;
    TbBigChecksum last_cksum;
    GameTurn simul_possess_turn;
    ThingIndex simul_things[AT_MAX_USERS];
    GameTurn shared_possess_turn;
    TbBool finished;
    // screenshots of the partner's hand (the other user's cursor is brought on screen first)
    int watch_kind;
    GameTurn watch_turn;
    TbBool watch_shot_hold, watch_shot_slap;
    MapCoord watch_pos_x, watch_pos_y;
    // shared campaign flow (-archonflow): frontend screens and levels in a row
    TbClockMSec start_clock;
    TbBool in_level;
    int fe_state;
    TbClockMSec fe_enter;
    TbClockMSec fe_time;
    int fe_step;
    int map_visits;
    int rounds_done;
    int wins;
    TbBool lose_round;
    LevelNumber lost_level;
    unsigned char heart_zoom_seen;
    LevelNumber level;
    LevelNumber progress_before;
    uint32_t progress_sum_start;
    GameTurn won_turn;
    TbBool objective_seen;
    TbClockMSec exit_time;
    // campaign mode (-archontestcampaign)
    TbBool progress_existed;
    LevelNumber start_continue;
    long progress_len;
    uint32_t progress_hash;
    GameTurn victory_turn;
    GameTurn finish_sent_turn;
    int win_attempts;
    TbBool done_written;
};

static struct AtState at;

/******************************************************************************/
TbBool archontest_active(void)
{
    return (archontest_seconds > 0);
}

static uint32_t at_random(uint32_t range)
{
    at.rng = at.rng * 1103515245u + 12345u;
    if (range == 0)
        return 0;
    return (at.rng >> 16) % range;
}

static void at_log(const char *format, ...)
{
    if (at.log == NULL)
        return;
    va_list val;
    va_start(val, format);
    vfprintf(at.log, format, val);
    va_end(val);
    fputc('\n', at.log);
    fflush(at.log);
}

static struct AtCheck *at_check_get(const char *name)
{
    for (int i = 0; i < at.checks_num; i++)
    {
        if (strcmp(at.checks[i].name, name) == 0)
            return &at.checks[i];
    }
    if (at.checks_num >= AT_MAX_CHECKS)
        return &at.checks[AT_MAX_CHECKS-1];
    struct AtCheck *chk = &at.checks[at.checks_num++];
    snprintf(chk->name, sizeof(chk->name), "%s", name);
    chk->state = 0;
    chk->detail[0] = '\0';
    return chk;
}

/** Records a check result; a check that failed once stays failed. */
static void at_check(const char *name, TbBool pass, const char *format, ...)
{
    struct AtCheck *chk = at_check_get(name);
    va_list val;
    va_start(val, format);
    vsnprintf(chk->detail, sizeof(chk->detail), format, val);
    va_end(val);
    if (chk->state != 2)
        chk->state = pass ? 1 : 2;
    at_log("CHECK turn=%lu %s %s %s", (unsigned long)get_gameturn(), name, pass ? "PASS" : "FAIL", chk->detail);
}

static void at_check_named(const char *base, TbBool pass, const char *format, ...)
{
    char name[32];
    snprintf(name, sizeof(name), "%s_u%d", base, (int)at.me);
    char detail[160];
    va_list val;
    va_start(val, format);
    vsnprintf(detail, sizeof(detail), format, val);
    va_end(val);
    at_check(name, pass, "%s", detail);
}

/******************************************************************************/
static struct UserState *at_ustate(NetUserId user)
{
    return &game.user_states[user];
}

static TbBool at_user_possessing(NetUserId user)
{
    const struct UserState *ustate = at_ustate(user);
    return (ustate->view_type == PVT_CreatureContrl) && (ustate->controlled_thing_idx > 0);
}

static void at_set_step(int step)
{
    at.step = step;
    at.sub = 0;
    at.step_turn = get_gameturn();
    at.sub_turn = get_gameturn();
}

static void at_set_sub(int sub)
{
    at.sub = sub;
    at.sub_turn = get_gameturn();
}

static GameTurn at_sub_age(void)
{
    return get_gameturn() - at.sub_turn;
}

static void at_action(unsigned char action, int32_t par1, int32_t par2)
{
    at.pk.action = action;
    at.pk.par1 = par1;
    at.pk.par2 = par2;
    at.pk.par3 = 0;
    at.pk.par4 = 0;
}

static void at_screenshot(const char *label);

static void at_cursor(MapCoord x, MapCoord y, unsigned char context)
{
    at.pk.has_pos = true;
    at.pk.pos_x = x;
    at.pk.pos_y = y;
    at.pk.context = context;
}

static void at_cursor_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, unsigned char context)
{
    at_cursor(subtile_coord_center(slab_subtile_center(slb_x)), subtile_coord_center(slab_subtile_center(slb_y)), context);
}

/** Puts the cursor 5 subtiles in front of a creature, where a user in first person looking along the creature's facing sees it. */
static void at_cursor_ahead_of(ThingIndex tng_idx)
{
    const struct Thing *thing = thing_get(tng_idx);
    if (!thing_exists(thing))
        return;
    const long angle = thing->move_angle_xy;
    at_cursor(move_coord_with_angle_x(thing->mappos.x.val, 5 * COORD_PER_STL, angle), move_coord_with_angle_y(thing->mappos.y.val, 5 * COORD_PER_STL, angle), CSt_DefaultArrow);
}

/** A user in first person turns the possessed creature to look at another creature (first person look packet: yaw, pitch). */
static void at_fp_look_at(ThingIndex own_idx, ThingIndex other_idx)
{
    const struct Thing *own = thing_get(own_idx);
    const struct Thing *other = thing_get(other_idx);
    if (!thing_exists(own) || !thing_exists(other))
        return;
    at.pk.flags = (at.pk.flags & ~PCtr_CameraContext) | CamIV_FirstPerson;
    at_cursor(get_angle_xy_to(&own->mappos, &other->mappos), 0, CSt_DefaultArrow);
}

static void at_look_at_slab(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    move_local_camera_to_position(subtile_coord_center(slab_subtile_center(slb_x)), subtile_coord_center(slab_subtile_center(slb_y)));
}

static int at_side(void)
{
    return (at.me == 0) ? 1 : -1;
}

static long at_slab_dist2(MapSlabCoord ax, MapSlabCoord ay, MapSlabCoord bx, MapSlabCoord by)
{
    return (long)(ax-bx)*(ax-bx) + (long)(ay-by)*(ay-by);
}

static TbBool at_slab_valid(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    return (slb_x > 0) && (slb_y > 0) && (slb_x < game.map_tiles_x-1) && (slb_y < game.map_tiles_y-1);
}

static TbBool at_slab_is_open(MapSlabCoord slb_x, MapSlabCoord slb_y);

/** A slab imps could dig out for this keeper (earth, gold, or the keeper's own wall). */
static TbBool at_slab_is_diggable(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    const struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    if (at_slab_is_open(slb_x, slb_y) || (slb->kind == SlbT_ROCK) || (slb->kind == SlbT_GEMS))
        return false;
    if (slab_kind_is_room(slb->kind) || slab_kind_is_door(slb->kind) || slab_kind_is_liquid(slb->kind))
        return false;
    return subtile_is_diggable_for_player(at.plyr, slab_subtile_center(slb_x), slab_subtile_center(slb_y), false);
}

static TbBool at_slab_is_my_floor(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    const struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    return (slb->kind == SlbT_CLAIMED) && (slabmap_owner(slb) == at.plyr);
}

static TbBool at_slab_is_open(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    const struct SlabMap *slb = get_slabmap_block(slb_x, slb_y);
    return (slb->kind == SlbT_CLAIMED) || (slb->kind == SlbT_PATH);
}

static TbBool at_slab_task_tagged(MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    return find_from_task_list(at.plyr, get_subtile_number(slab_subtile_center(slb_x), slab_subtile_center(slb_y))) >= 0;
}

/** Earth slabs next to open floor, on this user's side of the heart, nearest first. */
static void at_pick_dig_targets(void)
{
    at.dig_num = 0;
    for (int radius = 1; (radius < 12) && (at.dig_num < AT_MAX_DIG); radius++)
    {
        for (MapSlabCoord dy = -radius; (dy <= radius) && (at.dig_num < AT_MAX_DIG); dy++)
        {
            for (MapSlabCoord dx = -radius; (dx <= radius) && (at.dig_num < AT_MAX_DIG); dx++)
            {
                if ((abs(dx) != radius) && (abs(dy) != radius))
                    continue;
                if (dx * at_side() <= 0)
                    continue;
                MapSlabCoord sx = at.heart_slb_x + dx;
                MapSlabCoord sy = at.heart_slb_y + dy;
                if (!at_slab_valid(sx, sy) || !at_slab_is_diggable(sx, sy))
                    continue;
                if (!subtile_revealed(slab_subtile_center(sx), slab_subtile_center(sy), at.plyr))
                    continue;
                if (!at_slab_is_open(sx-1, sy) && !at_slab_is_open(sx+1, sy) && !at_slab_is_open(sx, sy-1) && !at_slab_is_open(sx, sy+1))
                    continue;
                at.dig_x[at.dig_num] = sx;
                at.dig_y[at.dig_num] = sy;
                at.dig_num++;
            }
        }
    }
}

/** Claimed floor of the keeper on this user's side (or any side), nearest to the heart. */
static TbBool at_pick_floor(RoomKind rkind, TbBool any_side, MapSlabCoord *out_x, MapSlabCoord *out_y)
{
    long best = -1;
    for (MapSlabCoord sy = 1; sy < game.map_tiles_y-1; sy++)
    {
        for (MapSlabCoord sx = 1; sx < game.map_tiles_x-1; sx++)
        {
            if (!any_side && ((sx - at.heart_slb_x) * at_side() <= 0))
                continue;
            if (!at_slab_is_my_floor(sx, sy))
                continue;
            if (rkind > 0) {
                if (!can_build_room_at_slab(at.plyr, rkind, sx, sy))
                    continue;
            } else {
                if (!can_drop_thing_here(slab_subtile_center(sx), slab_subtile_center(sy), at.plyr, 1))
                    continue;
            }
            long dist = at_slab_dist2(sx, sy, at.heart_slb_x, at.heart_slb_y);
            if ((dist < 4) && (rkind > 0))
                continue;
            if ((best < 0) || (dist < best)) {
                best = dist;
                *out_x = sx;
                *out_y = sy;
            }
        }
    }
    return (best >= 0);
}

/** An own digger free to be picked up or possessed, not the excluded one. */
static struct Thing *at_pick_imp(ThingIndex exclude, TbBool for_hand)
{
    struct Dungeon *dungeon = get_players_num_dungeon(at.plyr);
    struct Thing *found = INVALID_THING;
    int candidates = 0;
    ThingIndex i = dungeon->digger_list_start;
    int guard = 0;
    while ((i > 0) && (guard++ < 1000))
    {
        struct Thing *thing = thing_get(i);
        struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
        i = cctrl->players_next_creature_idx;
        if (!thing_exists(thing) || (thing->index == exclude))
            continue;
        if (get_thing_controlling_user(thing) >= 0)
            continue;
        if (thing_is_in_power_hand_list(thing, at.plyr))
            continue;
        if (for_hand && !can_thing_be_picked_up_by_player(thing, at.plyr))
            continue;
        candidates++;
        // pick a random candidate with the user's own generator
        if (thing_is_invalid(found) || (at_random(candidates) == 0))
            found = thing;
    }
    return found;
}

static int at_count_creatures(void)
{
    struct Dungeon *dungeon = get_players_num_dungeon(at.plyr);
    return dungeon->num_active_creatrs + dungeon->num_active_diggers;
}

/** Logs the slabs around the heart: # rock, E earth, $ gold, W wall, . path, c own claimed, x other claimed, R room. */
static void at_log_map(int radius)
{
    for (MapSlabCoord dy = -radius; dy <= radius; dy++)
    {
        char line[64];
        int n = 0;
        for (MapSlabCoord dx = -radius; (dx <= radius) && (n < 60); dx++)
        {
            MapSlabCoord sx = at.heart_slb_x + dx;
            MapSlabCoord sy = at.heart_slb_y + dy;
            char c = ' ';
            if (at_slab_valid(sx, sy))
            {
                const struct SlabMap *slb = get_slabmap_block(sx, sy);
                if (slb->kind == SlbT_ROCK) c = '#';
                else if ((slb->kind == SlbT_EARTH) || (slb->kind == SlbT_TORCHDIRT)) c = 'E';
                else if ((slb->kind == SlbT_GOLD) || (slb->kind == SlbT_GEMS)) c = '$';
                else if (slb->kind == SlbT_PATH) c = '.';
                else if (slb->kind == SlbT_CLAIMED) c = (slabmap_owner(slb) == at.plyr) ? 'c' : 'x';
                else if (slab_kind_is_room(slb->kind)) c = 'R';
                else c = 'W';
                if (at_slab_task_tagged(sx, sy)) c = 't';
            }
            line[n++] = c;
        }
        line[n] = 0;
        at_log("MAP %+3d %s", (int)dy, line);
    }
}

/** Reads the campaign progress file: whether it exists, its length and a hash of its content. */
static TbBool at_progress_signature(long *len, uint32_t *hash)
{
    *len = 0;
    *hash = 2166136261u;
    if (!is_campaign_loaded())
        return false;
    FILE *fp = fopen(get_campaign_progress_path(), "rb");
    if (fp == NULL)
        return false;
    int c;
    while ((c = fgetc(fp)) != EOF)
    {
        *hash = (*hash ^ (uint32_t)c) * 16777619u;
        (*len)++;
    }
    fclose(fp);
    return true;
}

/******************************************************************************/
static void at_log_state(void)
{
    struct Dungeon *dungeon = get_players_num_dungeon(at.plyr);
    struct UserState *u0 = at_ustate(0);
    struct UserState *u1 = at_ustate(1);
    at_log("STATE turn=%lu me=%d plyr=%d u0plyr=%d u1plyr=%d gold=%ld creatures=%d treasury=%ld lair=%ld u0ctrl=%d u1ctrl=%d u0view=%d u1view=%d u0work=%d u1work=%d u0cam=%ld,%ld u1cam=%ld,%ld hand=%d checksum=%08lx@%lu",
        (unsigned long)get_gameturn(), (int)at.me, (int)at.plyr, (int)u0->player_id, (int)u1->player_id,
        (long)dungeon->total_money_owned, at_count_creatures(),
        count_player_rooms_of_type(at.plyr, RoK_TREASURE), count_player_rooms_of_type(at.plyr, RoK_LAIR),
        (int)u0->controlled_thing_idx, (int)u1->controlled_thing_idx, (int)u0->view_type, (int)u1->view_type,
        (int)u0->work_state, (int)u1->work_state,
        (long)u0->dungeon_camera.x, (long)u0->dungeon_camera.y, (long)u1->dungeon_camera.x, (long)u1->dungeon_camera.y,
        (int)dungeon->num_things_in_hand, (unsigned long)at.last_cksum, (unsigned long)at.last_cksum_turn);
}

/** Watches synced state each turn for possession facts both clients can see. */
static void at_observe(void)
{
    if (at.step == AtS_Start)
        return;
    if (at_user_possessing(0) && at_user_possessing(1))
    {
        ThingIndex t0 = at_ustate(0)->controlled_thing_idx;
        ThingIndex t1 = at_ustate(1)->controlled_thing_idx;
        if (t0 == t1) {
            if (at.shared_possess_turn == 0) {
                at.shared_possess_turn = get_gameturn();
                at_check("possess_never_shared", false, "both users control thing %d", (int)t0);
            }
        } else if (at.simul_possess_turn == 0) {
            at.simul_possess_turn = get_gameturn();
            at.simul_things[0] = t0;
            at.simul_things[1] = t1;
            at_check("possess_simultaneous", true, "u0 controls %d, u1 controls %d", (int)t0, (int)t1);
        }
    }
}

/******************************************************************************/
static void at_step_start(void)
{
    if (get_gameturn() < 60)
        return;
    struct UserState *u0 = at_ustate(0);
    struct UserState *u1 = at_ustate(1);
    at_check("level_loaded", at.level > 0, "level %d", (int)at.level);
    if (archontest_campaign && (at.me == 0))
    {
        // the host started without -level: the level must be the one its saved progress continues at
        LevelNumber expected = at.progress_existed ? at.start_continue : first_singleplayer_level();
        at_check("level_from_host_progress", at.level == expected, "level %d, host progress continues at %d (progress file exists=%d)",
            (int)at.level, (int)expected, (int)at.progress_existed);
    }
    at_check("same_player", (u0->player_id == u1->player_id) && (u0->player_id == at.plyr) && (at.plyr >= 0),
        "u0plyr=%d u1plyr=%d my_player=%d", (int)u0->player_id, (int)u1->player_id, (int)at.plyr);
    struct Thing *heart = get_player_soul_container(at.plyr);
    at.heart_slb_x = subtile_slab(heart->mappos.x.stl.num);
    at.heart_slb_y = subtile_slab(heart->mappos.y.stl.num);
    at.rooms_base[0] = count_player_rooms_of_type(at.plyr, RoK_TREASURE);
    at.rooms_base[1] = count_player_rooms_of_type(at.plyr, RoK_LAIR);
    at_log("INFO heart slab=%d,%d rooms treasury=%ld lair=%ld creatures=%d", (int)at.heart_slb_x, (int)at.heart_slb_y,
        at.rooms_base[0], at.rooms_base[1], at_count_creatures());
    struct Dungeon *dungeon = get_players_num_dungeon(at.plyr);
    at_log("INFO before cheats: possess=%d lair_buildable=%d lair_researchable=%d cheats_allowed=%d",
        (int)is_power_available(at.plyr, PwrK_POSSESS), (int)dungeon->room_buildable[RoK_LAIR], (int)dungeon->room_resrchable[RoK_LAIR],
        (int)user_cheats_allowed(at.me));
    at_log_map(7);
    at_look_at_slab(at.heart_slb_x, at.heart_slb_y);
    at_set_step((at.me == 0) ? AtS_Cheats : AtS_WaitGrants);
}

static void at_step_cheats(void)
{
    // Grant Possess and the Lair through the regular cheat packets (requires -alex on every client)
    switch (at.sub)
    {
    case 0: at_action(PckA_CheatAllResrchbl, 0, 0); at_set_sub(1); break;
    case 1: at_action(PckA_CheatAllMagic, 0, 0); at_set_sub(2); break;
    case 2: at_action(PckA_CheatAllRooms, 0, 0); at_set_step(AtS_WaitGrants); break;
    }
}

static void at_step_wait_grants(void)
{
    TbBool ok = is_power_available(at.plyr, PwrK_POSSESS) && is_room_available(at.plyr, RoK_LAIR);
    if (at.grants_start == 0)
        at.grants_start = get_gameturn();
    if (!ok && (at.me == 0) && (get_gameturn() - at.step_turn >= 100) && (get_gameturn() - at.grants_start < 300))
    {
        struct Dungeon *dungeon = get_players_num_dungeon(at.plyr);
        at_log("INFO grants missing: possess=%d lair_buildable=%d lair_researchable=%d; resending cheats",
            (int)is_power_available(at.plyr, PwrK_POSSESS), (int)dungeon->room_buildable[RoK_LAIR], (int)dungeon->room_resrchable[RoK_LAIR]);
        at_set_step(AtS_Cheats);
        return;
    }
    if (ok || (get_gameturn() - at.grants_start > 300))
    {
        at_check_named("cheat_grant", ok, "possess=%d lair=%d", (int)is_power_available(at.plyr, PwrK_POSSESS), (int)is_room_available(at.plyr, RoK_LAIR));
        at_pick_dig_targets();
        at_log("INFO dig targets %d", at.dig_num);
        for (int i = 0; i < at.dig_num; i++)
            at_log("INFO dig target %d slab=%d,%d", i, (int)at.dig_x[i], (int)at.dig_y[i]);
        at.dig_idx = 0;
        at_set_step(AtS_Dig);
    }
}

static void at_step_dig(void)
{
    if (at.dig_idx >= at.dig_num)
    {
        if (at_sub_age() < 10)
            return;
        int tagged = 0;
        for (int i = 0; i < at.dig_num; i++)
        {
            if (at_slab_task_tagged(at.dig_x[i], at.dig_y[i]) || at_slab_is_open(at.dig_x[i], at.dig_y[i]))
                tagged++;
        }
        at_check_named("dig_tag", (tagged > 0) && (tagged == at.dig_num), "tagged %d of %d slabs", tagged, at.dig_num);
        at_set_step(AtS_Room);
        return;
    }
    MapSlabCoord sx = at.dig_x[at.dig_idx];
    MapSlabCoord sy = at.dig_y[at.dig_idx];
    // left click on the slab with the pick-axe cursor, hold one turn, release
    switch (at.sub)
    {
    case 0:
        if (at.dig_idx == 0)
            at_look_at_slab(sx, sy);
        at_cursor_slab(sx, sy, CSt_PickAxe);
        at_set_sub(1);
        break;
    case 1:
        at_cursor_slab(sx, sy, CSt_PickAxe);
        at.pk.flags |= PCtr_LBtnClick | PCtr_LBtnHeld;
        at_set_sub(2);
        break;
    case 2:
        at_cursor_slab(sx, sy, CSt_PickAxe);
        at.pk.flags |= PCtr_LBtnHeld;
        at_set_sub(3);
        break;
    case 3:
        at_cursor_slab(sx, sy, CSt_PickAxe);
        at.pk.flags |= PCtr_LBtnRelease;
        at_log("INFO tag slab=%d,%d", (int)sx, (int)sy);
        at.dig_idx++;
        at_set_sub(0);
        break;
    }
}

static void at_step_room(void)
{
    struct UserState *ustate = at_ustate(at.me);
    long *base = (at.me == 0) ? &at.rooms_base[0] : &at.rooms_base[1];
    switch (at.sub)
    {
    case 0:
        at.room_kind = (at.me == 0) ? RoK_TREASURE : RoK_LAIR;
        at_action(PckA_SetPlyrState, PSt_BuildRoom, at.room_kind);
        at_set_sub(1);
        break;
    case 1:
        if ((ustate->work_state == PSt_BuildRoom) && (ustate->chosen_room_kind == at.room_kind))
        {
            if (!at_pick_floor(at.room_kind, false, &at.room_x, &at.room_y) && !at_pick_floor(at.room_kind, true, &at.room_x, &at.room_y))
            {
                at_check_named("room_build", false, "no slab to build room kind %d", (int)at.room_kind);
                at_set_sub(5);
                break;
            }
            at_log("INFO build room kind=%d slab=%d,%d", (int)at.room_kind, (int)at.room_x, (int)at.room_y);
            at_look_at_slab(at.room_x, at.room_y);
            at_set_sub(2);
        } else if (at_sub_age() > 100) {
            at_check_named("room_build", false, "work state not set: work=%d room=%d", (int)ustate->work_state, (int)ustate->chosen_room_kind);
            at_set_sub(5);
        }
        break;
    case 2:
        at_cursor_slab(at.room_x, at.room_y, CSt_DefaultArrow);
        if (at_sub_age() >= 3) {
            at.pk.flags |= PCtr_LBtnClick | PCtr_LBtnHeld;
            at_set_sub(3);
        }
        break;
    case 3:
        at_cursor_slab(at.room_x, at.room_y, CSt_DefaultArrow);
        at.pk.flags |= PCtr_LBtnRelease;
        at_set_sub(4);
        break;
    case 4:
        at_cursor_slab(at.room_x, at.room_y, CSt_DefaultArrow);
        if (count_player_rooms_of_type(at.plyr, at.room_kind) > *base)
        {
            at_check_named("room_build", true, "room kind %d count %ld at slab %d,%d", (int)at.room_kind,
                count_player_rooms_of_type(at.plyr, at.room_kind), (int)at.room_x, (int)at.room_y);
            at_set_sub(5);
        } else if (at_sub_age() > 100) {
            at_check_named("room_build", false, "room kind %d not built at slab %d,%d", (int)at.room_kind, (int)at.room_x, (int)at.room_y);
            at_set_sub(5);
        }
        break;
    case 5:
        at_action(PckA_SetPlyrState, PSt_CtrlDungeon, 0);
        at_set_sub(6);
        break;
    case 6:
        if ((ustate->work_state == PSt_CtrlDungeon) || (at_sub_age() > 60))
            at_set_step(AtS_HandWait);
        break;
    }
}

static void at_step_hand_wait(void)
{
    // the keeper's hand contents are shared, so the users take turns
    GameTurn start = (at.me == 0) ? 400 : 560;
    struct Dungeon *dungeon = get_players_num_dungeon(at.plyr);
    if ((get_gameturn() >= start) && (dungeon->num_things_in_hand == 0))
        at_set_step(AtS_Hand);
}

static void at_step_hand(void)
{
    struct Thing *thing;
    switch (at.sub)
    {
    case 0:
        thing = at_pick_imp(0, true);
        if (thing_is_invalid(thing)) {
            if (at_sub_age() > 100) {
                at_check_named("hand_pick_drop", false, "no imp to pick up");
                at_set_step((at.me == 1) ? AtS_Slap : AtS_PossessWait);
            }
            break;
        }
        if (!at_pick_floor(0, false, &at.drop_x, &at.drop_y) && !at_pick_floor(0, true, &at.drop_x, &at.drop_y)) {
            at_check_named("hand_pick_drop", false, "no drop slab");
            at_set_step((at.me == 1) ? AtS_Slap : AtS_PossessWait);
            break;
        }
        at.hand_tng = thing->index;
        at_log("INFO pick imp %d at %d,%d drop slab %d,%d", (int)thing->index, (int)thing->mappos.x.stl.num, (int)thing->mappos.y.stl.num, (int)at.drop_x, (int)at.drop_y);
        move_local_camera_to_position(thing->mappos.x.val, thing->mappos.y.val);
        at_cursor(thing->mappos.x.val, thing->mappos.y.val, CSt_PowerHand);
        at_set_sub(1);
        break;
    case 1:
        thing = thing_get(at.hand_tng);
        at_cursor(thing->mappos.x.val, thing->mappos.y.val, CSt_PowerHand);
        if (at_sub_age() >= 3) {
            // what a left click release over a creature with the hand cursor sends
            at_action(PckA_UsePwrHandPick, at.hand_tng, 0);
            at.pk.flags |= PCtr_LBtnRelease;
            at_set_sub(2);
        }
        break;
    case 2:
        thing = thing_get(at.hand_tng);
        if (thing_is_in_power_hand_list(thing, at.plyr)) {
            at_log("INFO imp %d in hand", (int)at.hand_tng);
            at_look_at_slab(at.drop_x, at.drop_y);
            at_set_sub(3);
        } else if (at_sub_age() > 60) {
            at_check_named("hand_pick_drop", false, "imp %d was not picked up", (int)at.hand_tng);
            at_set_step((at.me == 1) ? AtS_Slap : AtS_PossessWait);
        } else {
            at_cursor(thing->mappos.x.val, thing->mappos.y.val, CSt_PowerHand);
        }
        break;
    case 3:
        at_cursor_slab(at.drop_x, at.drop_y, CSt_PowerHand);
        if (at_sub_age() >= 10) {
            at.pk.flags |= PCtr_RBtnClick;
            at_set_sub(4);
        }
        break;
    case 4:
        at_cursor_slab(at.drop_x, at.drop_y, CSt_PowerHand);
        at.pk.flags |= PCtr_RBtnRelease;
        at_set_sub(5);
        break;
    case 5:
        thing = thing_get(at.hand_tng);
        at_cursor_slab(at.drop_x, at.drop_y, CSt_DefaultArrow);
        if (!thing_is_in_power_hand_list(thing, at.plyr) && (at_sub_age() > 5)) {
            long dist = at_slab_dist2(subtile_slab(thing->mappos.x.stl.num), subtile_slab(thing->mappos.y.stl.num), at.drop_x, at.drop_y);
            at_check_named("hand_pick_drop", dist <= 2, "imp %d dropped at slab %d,%d (target %d,%d)", (int)at.hand_tng,
                (int)subtile_slab(thing->mappos.x.stl.num), (int)subtile_slab(thing->mappos.y.stl.num), (int)at.drop_x, (int)at.drop_y);
            at_set_step((at.me == 1) ? AtS_Slap : AtS_PossessWait);
        } else if (at_sub_age() > 60) {
            at_check_named("hand_pick_drop", false, "imp %d still in hand", (int)at.hand_tng);
            at_set_step((at.me == 1) ? AtS_Slap : AtS_PossessWait);
        }
        break;
    }
}

static void at_step_slap(void)
{
    struct Dungeon *dungeon = get_players_num_dungeon(at.plyr);
    struct Thing *thing;
    switch (at.sub)
    {
    case 0:
        thing = at_pick_imp(0, false);
        if (thing_is_invalid(thing)) {
            if (at_sub_age() > 100) {
                at_check_named("slap", false, "no creature to slap");
                at_set_step(AtS_PossessWait);
            }
            break;
        }
        at.hand_tng = thing->index;
        at.slaps_base = dungeon->lvstats.num_slaps;
        move_local_camera_to_position(thing->mappos.x.val, thing->mappos.y.val);
        at_cursor(thing->mappos.x.val, thing->mappos.y.val, CSt_PowerHand);
        at_set_sub(1);
        break;
    case 1:
        thing = thing_get(at.hand_tng);
        at_cursor(thing->mappos.x.val, thing->mappos.y.val, CSt_PowerHand);
        if (at_sub_age() >= 15) {
            // what a right click release over a creature sends
            at_action(PckA_UsePwrOnThing, PwrK_SLAP, at.hand_tng);
            at.pk.flags |= PCtr_RBtnRelease;
            at_set_sub(2);
        }
        break;
    case 2:
        if (dungeon->lvstats.num_slaps > at.slaps_base) {
            at_check_named("slap", true, "slapped imp %d, slaps %lu", (int)at.hand_tng, dungeon->lvstats.num_slaps);
            at_set_step(AtS_PossessWait);
        } else if (at_sub_age() > 30) {
            // the slap is refused right after a drop; try again
            at_set_sub((get_gameturn() - at.step_turn > 200) ? 3 : 1);
        }
        break;
    case 3:
        at_check_named("slap", false, "slap did not happen on imp %d", (int)at.hand_tng);
        at_set_step(AtS_PossessWait);
        break;
    }
}

static void at_step_possess_wait(void)
{
    if (get_gameturn() >= 800)
        at_set_step(AtS_Possess);
}

/** Sends what a ctrl+left click release on a creature sends (possess it). */
static void at_send_possess(ThingIndex tng_idx)
{
    struct Thing *thing = thing_get(tng_idx);
    at_cursor(thing->mappos.x.val, thing->mappos.y.val, CSt_PowerHand);
    at_action(PckA_UsePwrOnThing, PwrK_POSSESS, tng_idx);
    at.pk.flags |= PCtr_LBtnRelease;
    at.pk.addvals |= PCAdV_CrtrContrlPressed;
}

static void at_send_release(void)
{
    at_action(PckA_DirectCtrlExit, at_ustate(at.me)->controlled_thing_idx, 0);
}

static TbBool at_released(NetUserId user)
{
    return (at_ustate(user)->view_type == PVT_DungeonTop) && (at_ustate(user)->controlled_thing_idx == 0);
}

static void at_step_possess_user0(void)
{
    struct Thing *thing;
    switch (at.sub)
    {
    case 0:
        thing = at_pick_imp(0, false);
        if (thing_is_invalid(thing)) {
            if (at_sub_age() > 100) {
                at_check_named("possess", false, "no creature to possess");
                at_set_step(AtS_Camera);
            }
            break;
        }
        at.possess_a = thing->index;
        at_log("INFO possess thing %d", (int)at.possess_a);
        at_send_possess(at.possess_a);
        at_set_sub(1);
        break;
    case 1:
        if (at_user_possessing(0) && (at_ustate(0)->controlled_thing_idx == at.possess_a)) {
            at_check_named("possess", true, "controlling thing %d", (int)at.possess_a);
            at_set_sub(2);
        } else if (at_sub_age() > 100) {
            at_check_named("possess", false, "not controlling thing %d: view=%d ctrl=%d", (int)at.possess_a, (int)at_ustate(0)->view_type, (int)at_ustate(0)->controlled_thing_idx);
            at_set_step(AtS_Camera);
        }
        break;
    case 2:
        // keep possessing until the other user also possesses a creature
        if (at.simul_possess_turn != 0) {
            // both in first person: look at the other one's creature, which carries the partner marker
            if (get_gameturn() - at.simul_possess_turn >= 5)
                at_fp_look_at(at_ustate(0)->controlled_thing_idx, at_ustate(1)->controlled_thing_idx);
            if (get_gameturn() - at.simul_possess_turn > 60)
                at_set_sub(3);
        } else if (at_sub_age() > 600) {
            at_set_sub(3);
        }
        if ((at.simul_possess_turn == 0) && (at_sub_age() >= 5) && (at_sub_age() <= 35)) {
            // turn to the partner's cursor, slightly downwards, so its hand on the floor is in view
            MapCoord px;
            MapCoord py;
            const struct Thing *own = thing_get(at_ustate(0)->controlled_thing_idx);
            if (thing_exists(own) && partner_hand_user_cursor(1, &px, &py)) {
                struct Coord3d target;
                target.x.val = px;
                target.y.val = py;
                target.z.val = 0;
                at.pk.flags = (at.pk.flags & ~PCtr_CameraContext) | CamIV_FirstPerson;
                at_cursor(get_angle_xy_to(&own->mappos, &target), ANGLE_MASK - 40, CSt_DefaultArrow);
            }
        }
        if ((at.simul_possess_turn == 0) && (at_sub_age() == 35))
            at_screenshot("first person, partner hand in keeper view");
        break;
    case 3:
        at_send_release();
        at_set_sub(4);
        break;
    case 4:
        if (at_released(0)) {
            at_check_named("release", true, "back in dungeon view");
            at_set_step(AtS_Camera);
        } else if (at_sub_age() > 100) {
            at_check_named("release", false, "view=%d ctrl=%d", (int)at_ustate(0)->view_type, (int)at_ustate(0)->controlled_thing_idx);
            at_set_step(AtS_Camera);
        }
        break;
    }
}

static void at_step_possess_user1(void)
{
    struct Thing *thing;
    switch (at.sub)
    {
    case 0:
        // wait for the other user to possess, then try to possess the same creature
        if (at_user_possessing(0)) {
            at.possess_a = at_ustate(0)->controlled_thing_idx;
            thing = thing_get(at.possess_a);
            move_local_camera_to_position(thing->mappos.x.val, thing->mappos.y.val);
            at_cursor_ahead_of(at.possess_a);
            at_set_sub(1);
        } else if (at_sub_age() > 400) {
            at_check_named("possess_denied", false, "user 0 never possessed");
            at_set_step(AtS_Camera);
        }
        break;
    case 1:
        // the cursor rests in front of the creature user 0 possesses, to be seen in its first person view
        at_cursor_ahead_of(at.possess_a);
        if (at_sub_age() < 10)
            break;
        at_log("INFO try possess thing %d already possessed by user 0", (int)at.possess_a);
        at_send_possess(at.possess_a);
        at_set_sub(2);
        break;
    case 2:
        at_cursor_ahead_of(at.possess_a);
        if (at_sub_age() == 20)
            at_screenshot("keeper view, partner possessing a creature");
        if (at_sub_age() < 60)
            break;
        at_check_named("possess_denied", !at_user_possessing(1) && (at_ustate(0)->controlled_thing_idx == at.possess_a) && at_user_possessing(0),
            "u1 view=%d ctrl=%d; u0 view=%d ctrl=%d", (int)at_ustate(1)->view_type, (int)at_ustate(1)->controlled_thing_idx,
            (int)at_ustate(0)->view_type, (int)at_ustate(0)->controlled_thing_idx);
        if (at_user_possessing(1)) {
            at_set_sub(6);
            break;
        }
        at_set_sub(3);
        break;
    case 3:
        thing = at_pick_imp(at.possess_a, false);
        if (thing_is_invalid(thing)) {
            if (at_sub_age() > 100) {
                at_check_named("possess", false, "no second creature");
                at_set_step(AtS_Camera);
            }
            break;
        }
        at.possess_b = thing->index;
        at_log("INFO possess thing %d", (int)at.possess_b);
        at_send_possess(at.possess_b);
        at_set_sub(4);
        break;
    case 4:
        if (at_user_possessing(1) && (at_ustate(1)->controlled_thing_idx == at.possess_b)) {
            at_check_named("possess", true, "controlling thing %d", (int)at.possess_b);
            at_set_sub(5);
        } else if (at_sub_age() > 100) {
            at_check_named("possess", false, "not controlling thing %d: view=%d ctrl=%d", (int)at.possess_b, (int)at_ustate(1)->view_type, (int)at_ustate(1)->controlled_thing_idx);
            at_set_step(AtS_Camera);
        }
        break;
    case 5:
        // hold until the other user released, then release too; looking at the other one's creature meanwhile
        if (at.simul_possess_turn != 0 && get_gameturn() - at.simul_possess_turn >= 5 && !at_released(0))
            at_fp_look_at(at_ustate(1)->controlled_thing_idx, at_ustate(0)->controlled_thing_idx);
        if (at_released(0) || (at_sub_age() > 600))
            at_set_sub(6);
        break;
    case 6:
        at_send_release();
        at_set_sub(7);
        break;
    case 7:
        if (at_released(1)) {
            at_check_named("release", true, "back in dungeon view");
            at_set_sub(8);
        } else if (at_sub_age() > 100) {
            at_check_named("release", false, "view=%d ctrl=%d", (int)at_ustate(1)->view_type, (int)at_ustate(1)->controlled_thing_idx);
            at_set_step(AtS_Camera);
        }
        break;
    case 8:
        // the creature released by the other user can now be possessed by this one
        if (at_sub_age() < 20)
            break;
        if (!at_released(0)) {
            at_check_named("possess_after_release", false, "user 0 still possessing");
            at_set_step(AtS_Camera);
            break;
        }
        at_send_possess(at.possess_a);
        at_set_sub(9);
        break;
    case 9:
        if (at_user_possessing(1) && (at_ustate(1)->controlled_thing_idx == at.possess_a)) {
            at_check_named("possess_after_release", true, "controlling thing %d released by user 0", (int)at.possess_a);
            at_set_sub(10);
        } else if (at_sub_age() > 100) {
            at_check_named("possess_after_release", false, "not controlling thing %d: view=%d ctrl=%d", (int)at.possess_a, (int)at_ustate(1)->view_type, (int)at_ustate(1)->controlled_thing_idx);
            at_set_step(AtS_Camera);
        }
        break;
    case 10:
        if (at_sub_age() > 40) {
            at_send_release();
            at_set_sub(11);
        }
        break;
    case 11:
        if (at_released(1) || (at_sub_age() > 100)) {
            at_check_named("release_again", at_released(1), "view=%d ctrl=%d", (int)at_ustate(1)->view_type, (int)at_ustate(1)->controlled_thing_idx);
            at_set_step(AtS_Camera);
        }
        break;
    }
}

static void at_step_camera(void)
{
    struct UserState *ustate = at_ustate(at.me);
    switch (at.sub)
    {
    case 0:
        if (get_gameturn() < at.end_turn - 400)
            break;
        at_look_at_slab(at.heart_slb_x, at.heart_slb_y);
        at_set_sub(1);
        break;
    case 1:
        if (at_sub_age() < 40)
            break;
        at.cam_start_x = ustate->dungeon_camera.x;
        at.cam_start_y = ustate->dungeon_camera.y;
        at_log("INFO camera start %ld,%ld", (long)at.cam_start_x, (long)at.cam_start_y);
        at_set_sub(2);
        break;
    case 2:
        // hold a movement key: user 0 scrolls right, user 1 scrolls down
        if (at.me == 0)
            local_state.camera_movement_x = 1.0f;
        else
            local_state.camera_movement_y = 1.0f;
        if (at_sub_age() >= 12)
            at_set_sub(3);
        break;
    case 3:
        if (at_sub_age() < 20)
            break;
        {
            long dx = (long)ustate->dungeon_camera.x - at.cam_start_x;
            long dy = (long)ustate->dungeon_camera.y - at.cam_start_y;
            at_check_named("camera_move", (labs(dx) + labs(dy)) > COORD_PER_STL * STL_PER_SLB,
                "moved by %ld,%ld to %ld,%ld", dx, dy, (long)ustate->dungeon_camera.x, (long)ustate->dungeon_camera.y);
        }
        at_set_step(AtS_Idle);
        break;
    }
}

static void at_write_done(void);

static void at_final_checks(void)
{
    struct Dungeon *dungeon = get_players_num_dungeon(at.plyr);
    long treasury = count_player_rooms_of_type(at.plyr, RoK_TREASURE);
    long lair = count_player_rooms_of_type(at.plyr, RoK_LAIR);
    at_check("rooms_both_users", (treasury > at.rooms_base[0]) && (lair > at.rooms_base[1]), "treasury %ld (was %ld), lair %ld (was %ld)",
        treasury, at.rooms_base[0], lair, at.rooms_base[1]);
    int dug = 0;
    for (int i = 0; i < at.dig_num; i++)
    {
        if (at_slab_is_open(at.dig_x[i], at.dig_y[i]))
            dug++;
    }
    at_check_named("imps_dug", dug > 0, "%d of %d tagged slabs dug", dug, at.dig_num);
    at_check("slap_done", dungeon->lvstats.num_slaps > 0, "slaps %lu", dungeon->lvstats.num_slaps);
    if (at.shared_possess_turn == 0)
        at_check("possess_never_shared", true, "no creature was controlled by two users");
    if (at.simul_possess_turn == 0)
        at_check("possess_simultaneous", false, "never saw two users possessing at once");
    struct UserState *u0 = at_ustate(0);
    struct UserState *u1 = at_ustate(1);
    at_check("camera_independent", (u0->dungeon_camera.x != u1->dungeon_camera.x) || (u0->dungeon_camera.y != u1->dungeon_camera.y),
        "u0 %ld,%ld u1 %ld,%ld", (long)u0->dungeon_camera.x, (long)u0->dungeon_camera.y, (long)u1->dungeon_camera.x, (long)u1->dungeon_camera.y);
    at_check("no_desync", !at.desync, at.desync ? "desync at turn %lu" : "checksums matched (turn %lu)", (unsigned long)(at.desync ? at.desync_turn : get_gameturn()));
    // every scenario step of this user must have produced a result
    const char *own[] = {"cheat_grant", "dig_tag", "room_build", "hand_pick_drop", "possess", "release", "camera_move"};
    for (int i = 0; i < (int)(sizeof(own)/sizeof(own[0])); i++)
    {
        char name[32];
        snprintf(name, sizeof(name), "%s_u%d", own[i], (int)at.me);
        struct AtCheck *chk = at_check_get(name);
        if (chk->state == 0)
            at_check(name, false, "step never completed (bot step %d.%d)", at.step, at.sub);
    }
    if (at.me == 1)
    {
        const char *own1[] = {"slap_u1", "possess_denied_u1", "possess_after_release_u1"};
        for (int i = 0; i < 3; i++)
        {
            struct AtCheck *chk = at_check_get(own1[i]);
            if (chk->state == 0)
                at_check(own1[i], false, "step never completed (bot step %d.%d)", at.step, at.sub);
        }
    }
}

static void at_write_done(void)
{
    if (at.done_written)
        return;
    at.done_written = true;
    char failed[2048];
    failed[0] = '\0';
    for (int i = 0; i < at.checks_num; i++)
    {
        if (at.checks[i].state != 1) {
            size_t len = strlen(failed);
            snprintf(failed + len, sizeof(failed) - len, "%s%s", (len > 0) ? "," : "", at.checks[i].name);
        }
    }
    for (int i = 0; i < at.checks_num; i++)
        at_log("SUMMARY %s %s %s", at.checks[i].name, (at.checks[i].state == 1) ? "PASS" : "FAIL", at.checks[i].detail);
    if (failed[0] == '\0')
        at_log("ARCHONTEST DONE result=PASS level=%d turn=%lu", (int)at.level, (unsigned long)get_gameturn());
    else
        at_log("ARCHONTEST DONE result=FAIL failed=%s level=%d turn=%lu", failed, (int)at.level, (unsigned long)get_gameturn());
}

/******************************************************************************/
static uint32_t at_progress_file_sum(void);

static void at_start(void)
{
    at.started = true;
    at.me = netstate.my_id;
    at.plyr = my_player_number;
    at.rng = 0x9E3779B9u * (uint32_t)(at.me + 1);
    at.end_turn = (GameTurn)archontest_seconds * turns_per_second;
    at.start_clock = LbTimerClock();
    char fname[64];
    snprintf(fname, sizeof(fname), "archontest_%d.log", (int)at.me);
    at.log = fopen(fname, "w");
    at.level = get_loaded_level_number();
    at.progress_existed = at_progress_signature(&at.progress_len, &at.progress_hash);
    at.start_continue = is_campaign_loaded() ? peek_campaign_continue_level() : 0;
    at_log("ARCHONTEST START user=%d player=%d level=%d campaign=%s end_turn=%lu turns_per_second=%d flow_levels=%d", (int)at.me, (int)at.plyr,
        (int)at.level, is_campaign_loaded() ? campaign.fname : "-", (unsigned long)at.end_turn, (int)turns_per_second, (int)archontest_flow_levels);
    at_log("INFO progress file %s exists=%d len=%ld hash=%08lx continue_level=%d", is_campaign_loaded() ? get_campaign_progress_path() : "-",
        (int)at.progress_existed, at.progress_len, (unsigned long)at.progress_hash, is_campaign_loaded() ? (int)peek_campaign_continue_level() : 0);
    if (archontest_flow_levels > 0)
        at.level = SINGLEPLAYER_NOTSTARTED; // set from the shared map pick
    at_set_step(AtS_Start);
    if (archontest_flow_levels > 0)
    {
        at.fe_state = -1;
        at.progress_sum_start = at_progress_file_sum();
        at_log("INFO own campaign progress file sum=%08lx", (unsigned long)at.progress_sum_start);
    }
}

/** Takes an in-game screenshot (scrshots/scrNNNNN.png), as the screenshot key does. */
static void at_screenshot(const char *label)
{
    set_flag(local_system_flags, GSF_CaptureSShot);
    at_log("SHOT turn=%lu %s", (unsigned long)get_gameturn(), label);
}

/******************************************************************************/
// Shared campaign flow (-archonflow <levels>): shared map, briefing, level, statistics, back to the map.

/** Checksum of this machine's own progress file of the archon campaign (0 if there is none). */
static uint32_t at_progress_file_sum(void)
{
    const char *cmpgn = (archon_campaign_fname[0] != '\0') ? archon_campaign_fname : "keeporig";
    char name[96];
    snprintf(name, sizeof(name), "%s", cmpgn);
    char *dot = strrchr(name, '.');
    if (dot != NULL)
        *dot = '\0';
    str_append(name, sizeof(name), ".cid");
    FILE *f = fopen(prepare_file_path(FGrp_Save, name), "rb");
    if (f == NULL)
        return 0;
    uint32_t sum = 2166136261u;
    int c;
    while ((c = fgetc(f)) != EOF)
        sum = (sum ^ (uint32_t)c) * 16777619u;
    fclose(f);
    return sum;
}

static void at_check_level(const char *base, int level_idx, TbBool pass, const char *format, ...)
{
    char name[32];
    snprintf(name, sizeof(name), "%s_%d", base, level_idx);
    char detail[160];
    va_list val;
    va_start(val, format);
    vsnprintf(detail, sizeof(detail), format, val);
    va_end(val);
    at_check(name, pass, "%s", detail);
}

static TbBool at_users_selected(LevelNumber lvnum)
{
    for (NetUserId user = 0; user < AT_MAX_USERS; user++)
    {
        const struct ScreenPacket *nspck = &net_screen_packet[user];
        if (((nspck->networkstatus_flags & NetStat_PlayerConnected) == 0) || (nspck->action_par1 != lvnum))
            return false;
    }
    return true;
}

/** Rounds the flow test plays: every level won once, plus a lost first attempt at the second level. */
static int at_flow_rounds(void)
{
    return archontest_flow_levels + ((archontest_flow_levels >= 2) ? 1 : 0);
}

static TbBool at_flow_round_loses(int round)
{
    return (archontest_flow_levels >= 2) && (round == 2);
}

static void at_flow_final_checks(const char *reason)
{
    at_log("INFO flow finished: %s, rounds %d (of %d), levels won %d of %d", reason, at.rounds_done, at_flow_rounds(), at.wins, (int)archontest_flow_levels);
    // every round of the run must have produced all its checks
    const char *per_level[] = {"briefing_both", "level_loaded_same_player", "heart_zoom_both", "objective_both", "no_desync", "stats_both"};
    for (int lv = 1; lv <= at_flow_rounds(); lv++)
    {
        const TbBool lost = at_flow_round_loses(lv);
        char rname[32];
        snprintf(rname, sizeof(rname), "%s_%d", lost ? "defeat_both" : "victory_both", lv);
        if (at_check_get(rname)->state == 0)
            at_check(rname, false, "never reached (%s)", reason);
        if (lost && (at.me == 0))
        {
            snprintf(rname, sizeof(rname), "retry_after_defeat_%d", lv + 1);
            if (at_check_get(rname)->state == 0)
                at_check(rname, false, "never reached (%s)", reason);
        }
        for (int i = 0; i < (int)(sizeof(per_level)/sizeof(per_level[0])); i++)
        {
            char name[32];
            snprintf(name, sizeof(name), "%s_%d", per_level[i], lv);
            struct AtCheck *chk = at_check_get(name);
            if (chk->state == 0)
                at_check(name, false, "never reached (%s)", reason);
        }
        char name[32];
        snprintf(name, sizeof(name), "%s_%d", (at.me != 0) ? "client_progress_untouched" : (lost ? "host_progress_kept" : "host_progress_advanced"), lv);
        if (at_check_get(name)->state == 0)
            at_check(name, false, "never reached (%s)", reason);
        snprintf(name, sizeof(name), "back_to_map_both_%d", lv);
        if (at_check_get(name)->state == 0)
            at_check(name, false, "never reached (%s)", reason);
        if (at.me != 0)
        {
            snprintf(name, sizeof(name), "host_pick_seen_by_client_%d", lv);
            if (at_check_get(name)->state == 0)
                at_check(name, false, "never reached (%s)", reason);
        }
    }
    const char *once[] = {"map_shared", (at.me == 0) ? "host_picks" : "client_cannot_pick", (at.me == 0) ? "host_continues" : "client_cannot_continue"};
    for (int i = 0; i < (int)(sizeof(once)/sizeof(once[0])); i++)
    {
        if (at_check_get(once[i])->state == 0)
            at_check(once[i], false, "never reached (%s)", reason);
    }
    at_write_done();
    at.finished = true;
    at.exit_time = LbTimerClock() + ((at.me == 0) ? 4000 : 1000);
}

static void at_fe_map(void)
{
    const TbClockMSec now = LbTimerClock();
    const TbClockMSec age = now - at.fe_enter;
    switch (at.fe_step)
    {
    case 0:
        at.map_visits++;
        at_log("FE map visit=%d land_level=%d levels=%d first=%d my_player_number=%d", at.map_visits, (int)archon_map_level(0),
            archon_map_levels_count(), (int)archon_map_level(1), (int)my_player_number);
        at.fe_step = 1;
        break;
    case 1:
    {
        if (age < 1500)
            break;
        TbBool all = archon_flow_users_report(ArchonFe_OnMap);
        if (!all && (age < 20000))
            break;
        if (at.map_visits == 1)
            at_check("map_shared", all, "every user on the shared map (land view level %d)", (int)archon_map_level(0));
        else
            at_check_level("back_to_map_both", at.rounds_done, all, "every user back on the shared map after level %d", (int)at.level);
        at_screenshot((at.map_visits == 1) ? "shared map" : "back on shared map");
        at.fe_step = (at.wins >= archontest_flow_levels) ? 9 : 2;
        break;
    }
    case 2:
    {
        // a joiner cannot pick a level; the host picks the level its progress continues with
        LevelNumber lvnum = archon_map_level(0);
        if (at.me != 0)
        {
            if (age < 2500)
                break;
            LevelNumber before = fe_net_level_selected;
            TbBool accepted = frontnetmap_archon_pick(lvnum);
            at_check("client_cannot_pick", !accepted && (fe_net_level_selected == before), "pick of level %d accepted=%d selection %ld -> %ld",
                (int)lvnum, (int)accepted, before, fe_net_level_selected);
            at.fe_step = 3;
            break;
        }
        if (age < 3500)
            break;
        at.level = lvnum;
        at.progress_before = get_continue_level_number();
        if (at.lost_level > 0)
        {
            at_check_level("retry_after_defeat", at.rounds_done + 1, lvnum == at.lost_level, "after losing level %d the host's progress offers level %d", (int)at.lost_level, (int)lvnum);
            at.lost_level = 0;
        }
        TbBool accepted = frontnetmap_archon_pick(lvnum);
        at_check("host_picks", accepted && (fe_net_level_selected == lvnum), "host picked level %d accepted=%d", (int)lvnum, (int)accepted);
        at.fe_step = 3;
        at.fe_time = now;
        break;
    }
    case 3:
        if (at.me == 0)
        {
            TbBool acked = at_users_selected(at.level);
            if (!acked && (now - at.fe_time < 20000))
                break;
            at_check_level("briefing_both", at.rounds_done + 1, acked && (frontmap_briefing_level() == at.level),
                "every user has the briefing of level %d (host briefing %d)", (int)at.level, (int)frontmap_briefing_level());
            at_screenshot("briefing host");
            at.fe_step = 4;
            at.fe_time = now;
        } else
        {
            if ((fe_net_level_selected <= 0) && (age < 30000))
                break;
            at.level = fe_net_level_selected;
            at_check_level("host_pick_seen_by_client", at.rounds_done + 1, (at.level > 0) && (at.level == archon_map_level(0)),
                "client follows host selection %d (host continue level %d)", (int)at.level, (int)archon_map_level(0));
            at_check_level("briefing_both", at.rounds_done + 1, (at.level > 0) && (frontmap_briefing_level() == at.level),
                "client briefing of level %d", (int)frontmap_briefing_level());
            at_screenshot("briefing client");
            at.fe_step = 5;
        }
        break;
    case 4:
        // the host starts the level after a short look at the briefing
        if (now - at.fe_time < 4000)
            break;
        at_log("FE host starts level %d", (int)at.level);
        frontnetmap_archon_pick(at.level);
        at.fe_step = 5;
        break;
    case 9:
        at_flow_final_checks("all levels played");
        at.fe_step = 10;
        break;
    default:
        break;
    }
}

static void at_fe_stats(void)
{
    const TbClockMSec now = LbTimerClock();
    const TbClockMSec age = now - at.fe_enter;
    switch (at.fe_step)
    {
    case 0:
    {
        at.rounds_done++;
        at_log("FE stats after level %d (%d of %d)", (int)get_loaded_level_number(), at.rounds_done, (int)archontest_flow_levels);
        at_check_level("no_desync", at.rounds_done, !at.desync, at.desync ? "desync at turn %lu" : "checksums matched", (unsigned long)at.desync_turn);
        if (at.lose_round)
            at.lost_level = at.level;
        else
            at.wins++;
        if (at.me == 0)
        {
            LevelNumber cont = get_continue_level_number();
            if (at.lose_round)
                at_check_level("host_progress_kept", at.rounds_done, cont == at.progress_before, "lost level %d: host continue level %d -> %d",
                    (int)at.level, (int)at.progress_before, (int)cont);
            else
                at_check_level("host_progress_advanced", at.rounds_done, (cont != at.progress_before) && (cont == next_singleplayer_level(at.level, false)),
                    "host continue level %d -> %d", (int)at.progress_before, (int)cont);
        } else
        {
            uint32_t sum = at_progress_file_sum();
            at_check_level("client_progress_untouched", at.rounds_done, sum == at.progress_sum_start, "own progress file sum %08lx (start %08lx)",
                (unsigned long)sum, (unsigned long)at.progress_sum_start);
        }
        at.fe_step = 1;
        break;
    }
    case 1:
    {
        if (age < 1500)
            break;
        TbBool all = archon_flow_users_report(ArchonFe_InStats);
        if (!all && (age < 20000))
            break;
        at_check_level("stats_both", at.rounds_done, all, "every user on the statistics screen");
        at_screenshot("level statistics");
        at.fe_step = 2;
        break;
    }
    case 2:
        if (at.me != 0)
        {
            // a joiner cannot take the session on alone
            if (age < 3000)
                break;
            frontstats_leave(NULL);
            at.fe_step = 3;
            at.fe_time = now;
            break;
        }
        if (age < 4500)
            break;
        at_log("FE host continues");
        at_check("host_continues", true, "host left the statistics screen");
        frontstats_leave(NULL);
        at.fe_step = 4;
        at.fe_time = now;
        break;
    case 3:
        if (now - at.fe_time < 300)
            break;
        at_check("client_cannot_continue", (frontend_menu_state == FeSt_LEVEL_STATS), "client still on statistics after continue");
        at.fe_step = 5;
        break;
    case 4:
        if (now - at.fe_time > 3000)
        {
            at_log("FE host continues again");
            frontstats_leave(NULL);
            at.fe_time = now;
        }
        break;
    default:
        break;
    }
}

/**
 * Host started as a plain network session (no -archon): set the co-op campaign through the
 * create-session dialog's "Campaign" option and press the lobby's start button.
 */
static void at_fe_lobby(void)
{
    const TbClockMSec now = LbTimerClock();
    if ((at.me != 0) || (at.map_visits > 0))
        return;
    switch (at.fe_step)
    {
    case 0:
        if ((net_number_of_enum_players < AT_MAX_USERS) || archon_requested)
            break;
        at_log("FE lobby: setting the co-op campaign from the menu");
        // the session options dialog, shown as the session menu's create button does
        turn_on_menu(GMnu_FEADD_SESSION);
        set_menu_visible_off(GMnu_FENET_START);
        set_menu_visible_on(GMnu_FEADD_SESSION);
        at.fe_step = 1;
        at.fe_time = now;
        break;
    case 1:
    {
        if (now - at.fe_time < 1000)
            break;
        // the option button cycles Off -> each campaign
        struct GuiButton gbtn;
        memset(&gbtn, 0, sizeof(gbtn));
        gbtn.btype_value = 5;
        for (unsigned long i = 0; i <= campaigns_list.items_num; i++)
        {
            frontnet_change_lobby_option(&gbtn);
            if (archon_requested && (strcasecmp(archon_campaign_fname, "keeporig.cfg") == 0))
                break;
        }
        at_check("menu_campaign_option", archon_requested && (strcasecmp(archon_campaign_fname, "keeporig.cfg") == 0),
            "campaign option set to '%s' archon=%d", archon_campaign_fname, (int)archon_requested);
        at_screenshot("create session dialog with campaign option");
        at.fe_step = 2;
        at.fe_time = now;
        break;
    }
    case 2:
        if (now - at.fe_time < 1500)
            break;
        turn_off_menu(GMnu_FEADD_SESSION);
        set_menu_visible_on(GMnu_FENET_START);
        at_log("FE lobby: start button");
        set_packet_start(NULL);
        at.fe_step = 3;
        break;
    default:
        break;
    }
}

/** Frontend part of the shared campaign flow test, called every frontend frame. */
void archontest_frontend_update(void)
{
    if (archontest_active() && archontest_campaign && (archontest_flow_levels <= 0) && network_is_host()
      && archon_flow_active() && (frontend_menu_state == FeSt_NETLAND_VIEW))
    {
        // campaign mode started without -level: on the shared map the host plays the level its progress continues at
        static TbClockMSec map_time = 0;
        if (map_time == 0)
            map_time = LbTimerClock();
        if ((fe_net_level_selected <= 0) && (LbTimerClock() - map_time > 2000))
            frontnetmap_archon_pick(archon_map_level(0));
        else if ((fe_net_level_selected > 0) && (LbTimerClock() - map_time > 4000))
            frontnetmap_archon_pick(fe_net_level_selected);
        return;
    }
    if (!archontest_active() || (archontest_flow_levels <= 0) || !network_is_active())
        return;
    if (!at.started)
        at_start();
    at.in_level = false;
    if (at.finished)
    {
        if ((at.exit_time != 0) && (LbTimerClock() >= at.exit_time))
        {
            at_log("ARCHONTEST EXIT frontend");
            at.exit_time = 0;
            exit_keeper = 1;
        }
        return;
    }
    if (LbTimerClock() - at.start_clock > (TbClockMSec)archontest_seconds * 1000)
    {
        at_flow_final_checks("time limit");
        return;
    }
    if (frontend_menu_state != at.fe_state)
    {
        at.fe_state = frontend_menu_state;
        at.fe_enter = LbTimerClock();
        at.fe_step = 0;
        at_log("FE state %d", (int)frontend_menu_state);
    }
    if (frontend_menu_state == FeSt_NET_START)
        at_fe_lobby();
    if (!archon_flow_active())
        return;
    if (frontend_menu_state == FeSt_NETLAND_VIEW)
        at_fe_map();
    else if (frontend_menu_state == FeSt_LEVEL_STATS)
        at_fe_stats();
}

static void at_flow_level_start(void)
{
    at.in_level = true;
    at.plyr = my_player_number;
    at.desync = false;
    at.desync_turn = 0;
    at.won_turn = 0;
    at.objective_seen = false;
    at.heart_zoom_seen = 0;
    at.lose_round = at_flow_round_loses(at.rounds_done + 1);
    if (at.level <= 0)
    {
        // started directly (-level), not from the shared map
        at.level = get_loaded_level_number();
        at.progress_before = get_continue_level_number();
    }
    at.built_turn = 0;
    at_log("LEVEL START level=%d picked=%d player=%d round=%d %s", (int)get_loaded_level_number(), (int)at.level, (int)at.plyr,
        at.rounds_done + 1, at.lose_round ? "(to be lost)" : "(to be won)");
}

/** In-level part of the shared campaign flow: check the level, then win it through the cheat packet. */
static void at_flow_turn(void)
{
    struct PlayerInfo *player = get_my_player();
    const GameTurn turn = get_gameturn();
    const int lv = at.rounds_done + 1;
    if (at.finished)
    {
        if ((at.exit_time != 0) && (LbTimerClock() >= at.exit_time))
            at_action(PckA_ForceApplicationClose, 0, 0);
        return;
    }
    if (LbTimerClock() - at.start_clock > (TbClockMSec)archontest_seconds * 1000)
    {
        at_flow_final_checks("time limit in level");
        return;
    }
    if ((turn % 100) == 0)
        at_log_state();
    if (turn == 20)
    {
        struct UserState *u0 = at_ustate(0);
        struct UserState *u1 = at_ustate(1);
        at_check_level("level_loaded_same_player", lv, (get_loaded_level_number() == at.level) && (u0->player_id == u1->player_id) && (u0->player_id == at.plyr),
            "level %d (picked %d) u0plyr=%d u1plyr=%d my_player=%d", (int)get_loaded_level_number(), (int)at.level, (int)u0->player_id, (int)u1->player_id, (int)at.plyr);
    }
    // the level start zooms every user's view out of the dungeon heart
    if (turn <= 40)
    {
        for (NetUserId user = 0; user < AT_MAX_USERS; user++)
        {
            if (at_ustate(user)->instance_num == PI_HeartZoom)
                at.heart_zoom_seen |= (1 << user);
        }
        if (turn == 40)
            at_check_level("heart_zoom_both", lv, at.heart_zoom_seen == 3, "heart zoom seen for users mask %d", (int)at.heart_zoom_seen);
    }
    // the level script's objective (with its speech) opens the objective text on every user's screen
    if (!at.objective_seen && (turn > 1) && menu_is_active(GMnu_TEXT_INFO) && (game.evntbox_text_objective[at.plyr][0] != '\0'))
    {
        at.objective_seen = true;
        at_check_level("objective_both", lv, true, "objective shown at turn %lu: %.60s", (unsigned long)turn, game.evntbox_text_objective[at.plyr]);
        at_screenshot("level objective");
    }
    if (!at.objective_seen && (turn == 300))
        at_check_level("objective_both", lv, false, "objective text never shown (text '%.40s')", game.evntbox_text_objective[at.plyr]);
    const unsigned char end_state = at.lose_round ? VicS_LostLevel : VicS_WonLevel;
    const char *end_check = at.lose_round ? "defeat_both" : "victory_both";
    if ((at.me == 0) && (turn == 120))
        at_action(at.lose_round ? PckA_CheatLoseLevel : PckA_CheatWinLevel, 0, 0);
    if ((player->victory_state == end_state) && (at.won_turn == 0))
    {
        at.won_turn = turn;
        at_check_level(end_check, lv, true, "level %d %s at turn %lu", (int)get_loaded_level_number(), at.lose_round ? "lost" : "won", (unsigned long)turn);
        at_screenshot(at.lose_round ? "defeat" : "victory");
    }
    if ((at.won_turn == 0) && (turn == 600))
        at_check_level(end_check, lv, false, "level not decided by turn %lu (victory state %d)", (unsigned long)turn, (int)player->victory_state);
    // the host leaves the decided level (the finish key); every user goes on to the statistics
    if ((at.me == 0) && (at.won_turn != 0) && (turn >= at.won_turn + 60) && (((turn - at.won_turn) % 100) == 60))
        at_action(PckA_FinishGame, end_state, 0);
}

/** Campaign mode after the in-level checks: win through the cheat packet, then leave the level like the finish key does. */
static void at_campaign_turn(void)
{
    struct PlayerInfo *player = get_my_player();
    const GameTurn turn = get_gameturn();
    if (at.done_written)
    {
        // the level did not end: leave anyway so the run does not hang
        if (turn == at.finish_sent_turn + 20)
            at_action(PckA_ForceApplicationClose, 0, 0);
        return;
    }
    if (at.victory_turn == 0)
    {
        if (player->victory_state == VicS_WonLevel)
        {
            at.victory_turn = turn;
            at_check("victory_both", true, "keeper %d won level %d, seen by user %d", (int)at.plyr, (int)at.level, (int)at.me);
            at_screenshot("victory");
            return;
        }
        if ((at.me == 0) && (at.win_attempts < 3) && ((turn - at.end_turn) % 100 == 1))
        {
            at.win_attempts++;
            at_action(PckA_CheatWinLevel, 0, 0);
        }
        if (turn > at.end_turn + 400)
        {
            at_check("victory_both", false, "victory state %d", (int)player->victory_state);
            at_write_done();
            at.finish_sent_turn = turn;
        }
        return;
    }
    // what the finish level key sends once the level is decided; the host's packet ends the level for both
    if ((at.me == 0) && (turn >= at.victory_turn + 40) && ((turn - at.victory_turn) % 40 == 0))
    {
        at.finish_sent_turn = turn;
        at_action(PckA_FinishGame, player->victory_state, 0);
    }
    if (turn > at.victory_turn + 600)
    {
        at_check("level_end", false, "level did not end after victory");
        at_write_done();
        at.finish_sent_turn = turn;
    }
}

void archontest_level_ended(void)
{
    // the shared campaign flow test goes on through the frontend after each level
    if (!archontest_active() || !at.started || at.done_written || (archontest_flow_levels > 0))
        return;
    at_log("INFO level %d ended at turn %lu", (int)at.level, (unsigned long)get_gameturn());
    if (archontest_campaign)
    {
        at_check("level_end", at.victory_turn != 0, "level ended after victory at turn %lu", (unsigned long)at.victory_turn);
        long len;
        uint32_t hash;
        TbBool exists = at_progress_signature(&len, &hash);
        if (at.me == 0)
        {
            LevelNumber continue_level = peek_campaign_continue_level();
            LevelNumber expected = next_singleplayer_level(at.level, true);
            at_check("host_progress_updated", exists && ((len != at.progress_len) || (hash != at.progress_hash) || !at.progress_existed)
                && (continue_level == expected) && (continue_level != at.level) && (continue_level != at.start_continue),
                "progress file exists=%d len=%ld hash=%08lx (was exists=%d hash=%08lx), continue level %d, expected %d",
                (int)exists, len, (unsigned long)hash, (int)at.progress_existed, (unsigned long)at.progress_hash, (int)continue_level, (int)expected);
        } else
        {
            at_check("client_progress_untouched", (exists == at.progress_existed) && (len == at.progress_len) && (hash == at.progress_hash),
                "progress file exists=%d len=%ld hash=%08lx (was exists=%d len=%ld hash=%08lx)",
                (int)exists, len, (unsigned long)hash, (int)at.progress_existed, at.progress_len, (unsigned long)at.progress_hash);
        }
    }
    at_write_done();
    // the test run ends with the level
    exit_keeper = 1;
}

/** Brings the other user's cursor on screen and takes a screenshot of it while it is idle, holding a creature and slapping, to see the partner's hand drawn. */
static void at_partner_watch(void)
{
    const NetUserId other = (at.me == 0) ? 1 : 0;
    const struct UserState *pu = at_ustate(other);
    const struct UserState *mu = at_ustate(at.me);
    const GameTurn turn = get_gameturn();
    if ((mu->view_type != PVT_DungeonTop) || (pu->view_type != PVT_DungeonTop))
        return;
    static const char *const labels[] = { "", "partner hand idle", "partner hand holding", "partner hand slapping" };
    static const int delays[] = { 0, 6, 5, 2 };
    if (at.watch_kind == 0)
    {
        if (turn == 320)
            at.watch_kind = 1;
        else if (!at.watch_shot_hold && (pu->hand_animationId == HndA_Hold) && !power_hand_is_empty(get_my_player()) && (at.step != AtS_Hand))
            at.watch_kind = 2;
        else if (!at.watch_shot_slap && (pu->instance_num == PI_Whip) && (at.step != AtS_Slap))
            at.watch_kind = 3;
        if (at.watch_kind == 0)
            return;
        at.watch_turn = turn;
    }
    // where the partner's hand is drawn: the cursor of its last packet, remembered while it was valid
    MapCoord pos_x;
    MapCoord pos_y;
    if (partner_hand_user_cursor(other, &pos_x, &pos_y)) {
        at.watch_pos_x = pos_x;
        at.watch_pos_y = pos_y;
    }
    if (at.watch_pos_x != 0 || at.watch_pos_y != 0)
        move_local_camera_to_position(at.watch_pos_x, at.watch_pos_y);
    if (turn - at.watch_turn >= delays[at.watch_kind])
    {
        const struct Packet *pckt = get_packet(other);
        at_log("WATCH turn=%lu kind=%d partner_pos=%d,%d pckt_pos=%d,%d flags=0x%x", (unsigned long)turn, at.watch_kind,
            (int)at.watch_pos_x, (int)at.watch_pos_y, (int)pckt->pos_x, (int)pckt->pos_y, (unsigned)pckt->control_flags);
        at_screenshot(labels[at.watch_kind]);
        if (at.watch_kind == 2)
            at.watch_shot_hold = true;
        if (at.watch_kind == 3)
            at.watch_shot_slap = true;
        at.watch_kind = 0;
    }
}

/** Decides the packet content for a new turn. */
static void at_build_turn(void)
{
    memset(&at.pk, 0, sizeof(at.pk));
    struct PlayerInfo *player = get_my_player();
    struct UserState *ustate = at_ustate(at.me);
    const GameTurn turn = get_gameturn();
    if (archontest_flow_levels > 0)
    {
        at_flow_turn();
        if (at.pk.action != PckA_None)
            at_log("ACTION turn=%lu action=%d par1=%ld par2=%ld", (unsigned long)turn, (int)at.pk.action, (long)at.pk.par1, (long)at.pk.par2);
        return;
    }
    at_observe();
    if ((turn % 100) == 0)
        at_log_state();
    if (turn == 300)
        at_screenshot("rooms built, dig tags");
    if (turn == 780)
        at_screenshot("imps dug tagged slabs");
    if ((at.simul_possess_turn != 0) && (turn == at.simul_possess_turn + 30))
        at_screenshot("both users possessing");
    if (turn == at.end_turn - 250)
        at_screenshot("after camera moves");
    if (at.finished && archontest_campaign)
    {
        at_campaign_turn();
        return;
    }
    if (at.finished)
    {
        // leave: the client first, the host after the client is gone
        GameTurn leave_turn = at.end_turn + ((at.me == 0) ? 60 : 10);
        if (turn >= leave_turn) {
            if (turn == leave_turn)
                at_log("ARCHONTEST EXIT turn=%lu", (unsigned long)turn);
            at_action(PckA_ForceApplicationClose, 0, 0);
        }
        return;
    }
    if (turn >= at.end_turn)
    {
        at_log_state();
        at_screenshot("end");
        at_final_checks();
        if (!archontest_campaign)
            at_write_done();
        at.finished = true;
        return;
    }
    if (ustate->view_type == PVT_DungeonTop)
    {
        // idle cursor over the map, as when the mouse rests on it
        unsigned char context = power_hand_is_empty(player) ? CSt_DefaultArrow : CSt_PowerHand;
        // the two users rest their cursors apart, so a partner hand is not drawn over the own cursor
        at_cursor_slab(at.heart_slb_x + ((at.me == 0) ? -3 : 3), at.heart_slb_y + 2, context);
    }
    at_partner_watch();
    switch (at.step)
    {
    case AtS_Start: at_step_start(); break;
    case AtS_Cheats: at_step_cheats(); break;
    case AtS_WaitGrants: at_step_wait_grants(); break;
    case AtS_Dig: at_step_dig(); break;
    case AtS_Room: at_step_room(); break;
    case AtS_HandWait: at_step_hand_wait(); break;
    case AtS_Hand: at_step_hand(); break;
    case AtS_Slap: at_step_slap(); break;
    case AtS_PossessWait: at_step_possess_wait(); break;
    case AtS_Possess:
        if (at.me == 0)
            at_step_possess_user0();
        else
            at_step_possess_user1();
        break;
    case AtS_Camera: at_step_camera(); break;
    default: break;
    }
    if (at.pk.action != PckA_None)
        at_log("ACTION turn=%lu action=%d par1=%ld par2=%ld", (unsigned long)turn, (int)at.pk.action, (long)at.pk.par1, (long)at.pk.par2);
}

void archontest_get_inputs(struct Packet *pckt)
{
    if (!archontest_active() || !network_is_active())
        return;
    struct PlayerInfo *player = get_my_player();
    if (!player_exists(player))
        return;
    if (!at.started)
        at_start();
    if ((archontest_flow_levels > 0) && !at.in_level)
        at_flow_level_start();
    if (at.built_turn != get_gameturn() + 1)
    {
        at.built_turn = get_gameturn() + 1;
        at_build_turn();
    }
    // Same packet content however often input() runs within the turn
    set_packet_action(pckt, at.pk.action, at.pk.par1, at.pk.par2, at.pk.par3, at.pk.par4);
    pckt->control_flags = at.pk.flags;
    pckt->additional_packet_values = at.pk.addvals;
    pckt->pos_x = 0;
    pckt->pos_y = 0;
    if (at.pk.has_pos)
        set_players_packet_position(pckt, at.pk.pos_x, at.pk.pos_y, at.pk.context);
}

void archontest_note_checksum(GameTurn turn, TbBigChecksum checksum)
{
    if (!at.started || (at.log == NULL))
        return;
    at.last_cksum = checksum;
    at.last_cksum_turn = turn;
    if ((turn % 20) == 0)
        at_log("CKSUM turn=%lu sum=%08lx", (unsigned long)turn, (unsigned long)checksum);
}

void archontest_note_desync(GameTurn turn)
{
    if (!archontest_active() || at.desync)
        return;
    at.desync = true;
    at.desync_turn = turn;
    at_log("ARCHONTEST DESYNC turn %lu", (unsigned long)turn);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
