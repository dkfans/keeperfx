
#include <windows.h>
#include <winbase.h>
#include <math.h>
#include <string>
#include "keeperfx.hpp"

#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_heapmgr.h"
#include "bflib_keybrd.h"
#include "bflib_inputctrl.h"
#include "bflib_datetm.h"
#include "bflib_bufrw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_sndlib.h"
#include "bflib_fmvids.h"
#include "bflib_cpu.h"
#include "bflib_crash.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"
#include "bflib_mouse.h"
#include "bflib_filelst.h"
#include "bflib_network.h"

#include "version.h"
#include "front_simple.h"
#include "frontend.h"
#include "front_input.h"
#include "gui_draw.h"
#include "gui_tooltips.h"
#include "gui_parchment.h"
#include "gui_frontmenu.h"
#include "gui_msgs.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "kjm_input.h"
#include "packets.h"
#include "config.h"
#include "config_strings.h"
#include "config_campaigns.h"
#include "config_terrain.h"
#include "config_trapdoor.h"
#include "config_objects.h"
#include "config_rules.h"
#include "config_lenses.h"
#include "config_magic.h"
#include "config_creature.h"
#include "config_crtrstates.h"
#include "config_crtrmodel.h"
#include "config_compp.h"
#include "config_effects.h"
#include "lvl_script.h"
#include "lvl_filesdk1.h"
#include "thing_list.h"
#include "player_instances.h"
#include "player_utils.h"
#include "player_states.h"
#include "player_computer.h"
#include "game_heap.h"
#include "game_saves.h"
#include "engine_render.h"
#include "engine_lenses.h"
#include "engine_camera.h"
#include "engine_arrays.h"
#include "engine_textures.h"
#include "engine_redraw.h"
#include "front_landview.h"
#include "front_lvlstats.h"
#include "front_easter.h"
#include "front_fmvids.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_creature.h"
#include "thing_corpses.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_doors.h"
#include "thing_traps.h"
#include "thing_shots.h"
#include "thing_navigate.h"
#include "thing_factory.h"
#include "slab_data.h"
#include "room_data.h"
#include "room_entrance.h"
#include "room_jobs.h"
#include "room_util.h"
#include "room_library.h"
#include "map_columns.h"
#include "map_events.h"
#include "map_utils.h"
#include "map_blocks.h"
#include "ariadne_wallhug.h"
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "creature_states_rsrch.h"
#include "creature_states_lair.h"
#include "creature_states_mood.h"
#include "lens_api.h"
#include "light_data.h"
#include "magic.h"
#include "power_process.h"
#include "power_hand.h"
#include "power_specials.h"
#include "game_merge.h"
#include "gui_topmsg.h"
#include "gui_boxmenu.h"
#include "gui_soundmsgs.h"
#include "gui_frontbtns.h"
#include "frontmenu_ingame_tabs.h"
#include "ariadne.h"
#include "net_game.h"
#include "sounds.h"
#include "vidfade.h"
#include "KeeperSpeech.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "room_list.h"

#include "music_player.h"

#ifdef AUTOTESTING
#include "event_monitoring.h"
#endif

#ifdef _MSC_VER
#define strcasecmp _stricmp
#endif

int test_variable;

char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];

short default_loc_player = 0;
TbBool force_player_num = false;
struct StartupParameters start_params;

struct Room *droom = &_DK_game.rooms[25];

//static
TbClockMSec last_loop_time=0;

#ifdef __cplusplus
extern "C" {
#endif

DLLIMPORT int _DK_can_thing_be_queried(struct Thing *thing, long a2);
DLLIMPORT void _DK_tag_cursor_blocks_sell_area(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT unsigned char _DK_tag_cursor_blocks_place_door(unsigned char a1, long a2, long a3);
DLLIMPORT void _DK_tag_cursor_blocks_dig(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT void _DK_tag_cursor_blocks_thing_in_hand(unsigned char a1, long a2, long a3, int a4, long a5);
DLLIMPORT long _DK_ceiling_init(unsigned long a1, unsigned long a2);
DLLIMPORT long _DK_apply_wallhug_force_to_boulder(struct Thing *thing);
DLLIMPORT void __stdcall _DK_IsRunningMark(void);
DLLIMPORT void __stdcall _DK_IsRunningUnmark(void);
DLLIMPORT void _DK_update_flames_nearest_camera(struct Camera *camera);
DLLIMPORT void _DK_update_player_camera_fp(struct Camera *cam, struct Thing *thing);
DLLIMPORT void _DK_view_move_camera_left(struct Camera *cam, long a2);
DLLIMPORT void _DK_view_move_camera_right(struct Camera *cam, long a2);
DLLIMPORT void _DK_view_move_camera_up(struct Camera *cam, long a2);
DLLIMPORT void _DK_view_move_camera_down(struct Camera *cam, long a2);
DLLIMPORT long _DK_ceiling_block_is_solid_including_corners_return_height(long a1, long a2, long a3);
DLLIMPORT unsigned long _DK_setup_move_out_of_cave_in(struct Thing *thing);
// Now variables
DLLIMPORT extern HINSTANCE _DK_hInstance;

TbPixel get_player_path_colour(unsigned short owner)
{
  return player_path_colours[player_colors_map[owner % PLAYERS_EXT_COUNT]];
}

void setup_stuff(void)
{
    setup_texture_block_mem();
    init_fades_table();
    init_alpha_table();
}

void powerful_magic_breaking_sparks(struct Thing *breaktng)
{
    struct Coord3d pos;
    pos.x.val = subtile_coord_center(breaktng->mappos.x.stl.num + ACTION_RANDOM(11) - 5);
    pos.y.val = subtile_coord_center(breaktng->mappos.y.stl.num + ACTION_RANDOM(11) - 5);
    pos.z.val = get_floor_height_at(&pos);
    draw_lightning(&breaktng->mappos, &pos, 96, 60);
    if ( !S3DEmitterIsPlayingSample(breaktng->snd_emitter_id, 157, 0) ) {
        thing_play_sample(breaktng, 157, NORMAL_PITCH, -1, 3, 1, 6, FULL_LOUDNESS);
    }
}

void initialise_devastate_dungeon_from_heart(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon->devastation_turn == 0)
    {
        struct Thing *heartng;
        heartng = get_player_soul_container(plyr_idx);
        if (thing_exists(heartng)) {
            dungeon->devastation_turn = 1;
            dungeon->devastation_centr_x = heartng->mappos.x.stl.num;
            dungeon->devastation_centr_y = heartng->mappos.y.stl.num;
        } else {
            dungeon->devastation_turn = 1;
            dungeon->devastation_centr_x = map_subtiles_x/2;
            dungeon->devastation_centr_y = map_subtiles_y/2;
        }
    }
}

void process_dungeon_destroy(struct Thing *heartng)
{
    struct Dungeon *dungeon;
    long plyr_idx;
    plyr_idx = heartng->owner;
    //_DK_process_dungeon_destroy(heartng); return;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon->heart_destroy_state == 0) {
        return;
    }
    powerful_magic_breaking_sparks(heartng);
    const struct Coord3d *central_pos;
    central_pos = &heartng->mappos;
    switch ( dungeon->heart_destroy_state )
    {
    case 1:
        initialise_devastate_dungeon_from_heart(plyr_idx);
        dungeon->heart_destroy_turn++;
        if (dungeon->heart_destroy_turn < 32)
        {
            if ( ACTION_RANDOM(96) < (dungeon->heart_destroy_turn << 6) / 32 + 32 ) {
                create_effect(central_pos, TngEff_Unknown44, plyr_idx);
            }
        } else
        { // Got to next phase
            dungeon->heart_destroy_state = 2;
            dungeon->heart_destroy_turn = 0;
        }
        break;
    case 2:
        dungeon->heart_destroy_turn++;
        if (dungeon->heart_destroy_turn < 32)
        {
            create_effect(central_pos, TngEff_Unknown44, plyr_idx);
        } else
        { // Got to next phase
            dungeon->heart_destroy_state = 3;
            dungeon->heart_destroy_turn = 0;
        }
        break;
    case 3:
        // Drop all held things, by keeper
        if ((dungeon->num_things_in_hand > 0) && ((gameadd.classic_bugs_flags & ClscBug_NoHandPurgeOnDefeat) == 0))
        {
            dump_all_held_things_on_map(plyr_idx, central_pos->x.stl.num, central_pos->y.stl.num);
        }
        // Drop all held things, by computer player
        struct Computer2 *comp;
        comp = get_computer_player(plyr_idx);
        computer_force_dump_held_things_on_map(comp, central_pos);
        // Now if players things are still in hand - they must be kept by enemies
        // Got to next phase
        dungeon->heart_destroy_state = 4;
        dungeon->heart_destroy_turn = 0;
        break;
    case 4:
        // Final phase - destroy the heart, both pedestal room and container thing
        {
            struct Thing *efftng;
            efftng = create_effect(central_pos, TngEff_Unknown04, plyr_idx);
            if (!thing_is_invalid(efftng))
              efftng->byte_16 = 8;
            efftng = create_effect(central_pos, TngEff_WoPExplosion, plyr_idx);
            if (!thing_is_invalid(efftng))
                efftng->byte_16 = 8;
            destroy_dungeon_heart_room(plyr_idx, heartng);
            delete_thing_structure(heartng, 0);
        }
        { // If there is another heart owned by this player, set it to "working" heart
            struct PlayerInfo *player;
            player = get_player(plyr_idx);
            init_player_start(player, true);
            if (player_has_heart(plyr_idx))
            {
                // If another heart was found, stop the process
                dungeon->devastation_turn = 0;
            } else
            {
                // If this is the last heart the player had, finish him
                setup_all_player_creatures_and_diggers_leave_or_die(plyr_idx);
                player->allied_players = (1 << player->id_number);
            }
        }
        dungeon->heart_destroy_state = 0;
        dungeon->heart_destroy_turn = 0;
        break;
    }
}

TbBool all_dungeons_destroyed(const struct PlayerInfo *win_player)
{
    long win_plyr_idx;
    long i;
    win_plyr_idx = win_player->id_number;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      if (i == win_plyr_idx)
        continue;
      if (!player_is_friendly_or_defeated(i,win_plyr_idx))
        return false;
    }
    SYNCDBG(1,"Returning true for player %ld",win_plyr_idx);
    return true;
}

void clear_creature_pool(void)
{
    memset(&game.pool,0,sizeof(struct CreaturePool));
    game.pool.is_empty = true;
}

void give_shooter_drained_health(struct Thing *shooter, long health_delta)
{
    struct CreatureControl *cctrl;
    HitPoints max_health;
    HitPoints health;
    if ( !thing_exists(shooter) )
        return;
    cctrl = creature_control_get_from_thing(shooter);
    max_health = cctrl->max_health;
    health = shooter->health + health_delta;
    if (health < max_health) {
        shooter->health = health;
    } else {
        shooter->health = max_health;
    }
}

long get_foot_creature_has_down(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    unsigned short val;
    long i;
    int n;
    //return _DK_get_foot_creature_has_down(thing);
    cctrl = creature_control_get_from_thing(thing);
    val = thing->field_48;
    if (val == (cctrl->field_CE >> 8))
        return 0;
    n = get_creature_model_graphics(thing->model, CGI_Ambulate);
    i = convert_td_iso(n);
    if (i != thing->anim_sprite)
        return 0;
    if (val == 1)
        return 1;
    if (val == 4)
        return 2;
    return 0;
}

void process_keeper_spell_effect(struct Thing *thing)
{
    struct CreatureControl *cctrl;
    //_DK_process_keeper_spell_effect(thing);
    TRACE_THING(thing);
    cctrl = creature_control_get_from_thing(thing);
    cctrl->field_2AE--;
    if (cctrl->field_2AE <= 0)
    {
        cctrl->field_2B0 = 0;
        return;
    }
    if (cctrl->field_2B0 == 7)
    {
        struct Coord3d pos;
        long amp;
        long direction;
        long delta_x;
        long delta_y;
        amp = 5 * thing->clipbox_size_xy / 8;
        direction = ACTION_RANDOM(2*LbFPMath_PI);
        delta_x = (amp * LbSinL(direction) >> 8);
        delta_y = (amp * LbCosL(direction) >> 8);
        pos.x.val = thing->mappos.x.val + (delta_x >> 8);
        pos.y.val = thing->mappos.y.val - (delta_y >> 8);
        pos.z.val = thing->mappos.z.val;
        create_effect_element(&pos, TngEff_Unknown45, thing->owner);
    }
}

unsigned long lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos)
{
    //return _DK_lightning_is_close_to_player(player, pos);
    return get_2d_box_distance(&player->acamera->mappos, pos) < subtile_coord(45,0);
}

TngUpdateRet affect_thing_by_wind(struct Thing *thing, ModTngFilterParam param)
{
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    if (thing->index == param->num2) {
        return TUFRet_Unchanged;
    }
    struct Thing *shotng;
    shotng = (struct Thing *)param->ptr3;
    if ((thing->index == shotng->index) || (thing->index == shotng->parent_idx)) {
        return TUFRet_Unchanged;
    }
    MapCoordDelta dist;
    dist = LONG_MAX;
    TbBool apply_velocity;
    apply_velocity = false;
    if (thing->class_id == TCls_Creature)
    {
        if (!thing_is_picked_up(thing) && !creature_is_being_unconscious(thing))
        {
            struct CreatureStats *crstat;
            crstat = creature_stats_get_from_thing(thing);
            dist = get_2d_box_distance(&shotng->mappos, &thing->mappos) + 1;
            if ((dist < param->num1) && crstat->affected_by_wind)
            {
                set_start_state(thing);
                struct CreatureControl *cctrl;
                cctrl = creature_control_get_from_thing(thing);
                cctrl->idle.start_gameturn = game.play_gameturn;
                apply_velocity = true;
            }
        }
    } else
    if (thing->class_id == TCls_EffectElem)
    {
        if (!thing_is_picked_up(thing))
        {
            struct EffectElementStats *eestat;
            eestat = get_effect_element_model_stats(thing->model);
            dist = get_2d_box_distance(&shotng->mappos, &thing->mappos) + 1;
            if ((dist < param->num1) && eestat->affected_by_wind)
            {
                apply_velocity = true;
            }
        }
    } else
    if (thing->class_id == TCls_Shot)
    {
        if (!thing_is_picked_up(thing))
        {
            struct ShotConfigStats *shotst;
            shotst = get_shot_model_stats(thing->model);
            dist = get_2d_box_distance(&shotng->mappos, &thing->mappos) + 1;
            if ((dist < param->num1) && shotst->old->affected_by_wind)
            {
                apply_velocity = true;
            }
        }
    } else
    if (thing->class_id == TCls_Effect)
    {
        if (!thing_is_picked_up(thing))
        {

            struct EffectConfigStats *effcst;
            effcst = get_effect_model_stats(thing->model);
            dist = get_2d_box_distance(&shotng->mappos, &thing->mappos) + 1;
            if ((dist < param->num1) && effcst->old->affected_by_wind)
            {
                apply_velocity = true;
            }
        }
    }
    if (apply_velocity)
    {
        struct ComponentVector wind_push;
        wind_push.x = (shotng->veloc_base.x.val * param->num1) / dist;
        wind_push.y = (shotng->veloc_base.y.val * param->num1) / dist;
        wind_push.z = (shotng->veloc_base.z.val * param->num1) / dist;
        SYNCDBG(8,"Applying (%d,%d,%d) to %s index %d",(int)wind_push.x,(int)wind_push.y,(int)wind_push.z,thing_model_name(thing),(int)thing->index);
        apply_transitive_velocity_to_thing(thing, &wind_push);
        return TUFRet_Modified;
    }
    return TUFRet_Unchanged;
}

void affect_nearby_enemy_creatures_with_wind(struct Thing *shotng)
{
    //_DK_affect_nearby_enemy_creatures_with_wind(shotng); return;
    Thing_Modifier_Func do_cb;
    struct CompoundTngFilterParam param;
    param.plyr_idx = -1;
    param.class_id = 0;
    param.model_id = 0;
    param.num1 = 2048;
    param.num2 = shotng->parent_idx;
    param.ptr3 = shotng;
    do_cb = affect_thing_by_wind;
    do_to_things_with_param_spiral_near_map_block(&shotng->mappos, param.num1-COORD_PER_STL, do_cb, &param);
}

void affect_nearby_stuff_with_vortex(struct Thing *thing)
{
    //TODO implement vortex; it's not implemented in original DK
    WARNLOG("Not implemented");
}

void affect_nearby_friends_with_alarm(struct Thing *traptng)
{
    //_DK_affect_nearby_friends_with_alarm(thing);
    SYNCDBG(8,"Starting");
    if (is_neutral_thing(traptng)) {
        return;
    }
    struct Dungeon *dungeon;
    unsigned long k;
    int i;
    dungeon = get_players_num_dungeon(traptng->owner);
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct CreatureControl *cctrl;
        struct Thing *thing;
        thing = thing_get(i);
        TRACE_THING(thing);
        cctrl = creature_control_get_from_thing(thing);
        if (creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!thing_is_picked_up(thing) && !is_thing_directly_controlled(thing) &&
            !creature_is_being_unconscious(thing) && !creature_is_kept_in_custody(thing) &&
            (cctrl->combat_flags == 0) && !creature_is_dragging_something(thing) && !creature_is_dying(thing))
        {
            struct StateInfo *stati;
            stati = get_thing_state_info_num(get_creature_state_besides_interruptions(thing));
            if (stati->react_to_cta && (get_2d_box_distance(&traptng->mappos, &thing->mappos) < 4096))
            {
                creature_mark_if_woken_up(thing);
                if (external_set_thing_state(thing, CrSt_ArriveAtAlarm))
                {
                    if (setup_person_move_to_position(thing, traptng->mappos.x.stl.num, traptng->mappos.y.stl.num, 0))
                    {
                        thing->continue_state = CrSt_ArriveAtAlarm;
                        cctrl->field_2FA = game.play_gameturn + 800;
                        cctrl->alarm_stl_x = traptng->mappos.x.stl.num;
                        cctrl->alarm_stl_y = traptng->mappos.y.stl.num;
                    }
                }
            }
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
}

long apply_wallhug_force_to_boulder(struct Thing *thing)
{
  return _DK_apply_wallhug_force_to_boulder(thing);
}

void draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long delta_step, long num_per_step)
{
  //_DK_draw_flame_breath(pos1, pos2, a3, a4);
  MapCoordDelta dist_x;
  MapCoordDelta dist_y;
  MapCoordDelta dist_z;
  dist_x = pos2->x.val - (MapCoordDelta)pos1->x.val;
  dist_y = pos2->y.val - (MapCoordDelta)pos1->y.val;
  dist_z = pos2->z.val - (MapCoordDelta)pos1->z.val;
  int delta_x;
  int delta_y;
  int delta_z;
  if (dist_x >= 0)
  {
      delta_x = delta_step;
    } else {
        dist_x = -dist_x;
        delta_x = -delta_step;
    }
    if (dist_y >= 0) {
        delta_y = delta_step;
    } else {
        dist_y = -dist_y;
        delta_y = -delta_step;
    }
    if (dist_z >= 0) {
        delta_z = delta_step;
    } else {
        dist_z = -dist_z;
        delta_z = -delta_step;
    }
    // Now our dist_x,dist_y,dist_z is always non-negative,
    // and sign is stored in delta_x,delta_y,delta_z.
    if ((dist_x != 0) || (dist_y != 0) || (dist_z != 0))
    {
        int nsteps;
        // Find max dimension, and scale deltas to it
        if ((dist_z > dist_x) && (dist_z > dist_y))
        {
            nsteps = dist_z / delta_step;
            delta_y = dist_y * delta_y / dist_z;
            delta_x = dist_x * delta_x / dist_z;
        } else
        if ((dist_x > dist_y) && (dist_x > dist_z))
        {
            nsteps = dist_x / delta_step;
            delta_y = dist_y * delta_y / dist_x;
            delta_z = dist_z * delta_z / dist_x;
        } else
        if ((dist_y > dist_x) && (dist_y > dist_z))
        {
            nsteps = dist_y / delta_step;
            delta_x = dist_x * delta_x / dist_y;
            delta_z = dist_z * delta_z / dist_y;
        } else
        { // No dominate direction
            nsteps = (dist_x + dist_y + dist_z) / delta_step;
            delta_x = dist_x * delta_x / (dist_x + dist_y + dist_z);
            delta_y = dist_y * delta_y / (dist_x + dist_y + dist_z);
            delta_z = dist_z * delta_z / (dist_x + dist_y + dist_z);
        }
        struct EffectElementStats *eestat;
        eestat = get_effect_element_model_stats(9);
        int sprsize;
        int delta_size;
        delta_size = ((eestat->sprite_size_max - eestat->sprite_size_min) << 8) / (nsteps+1);
        sprsize = (eestat->sprite_size_min << 8);
        int deviat;
        deviat = 1;
        struct Coord3d curpos;
        curpos.x.val = pos1->x.val;
        curpos.y.val = pos1->y.val;
        curpos.z.val = pos1->z.val;
        int i;
        for (i=nsteps+1; i > 0; i--)
        {
            int devrange;
            devrange = 2 * deviat;
            int k;
            for (k = num_per_step; k > 0; k--)
            {
                struct Coord3d tngpos;
                tngpos.x.val = curpos.x.val + deviat - ACTION_RANDOM(devrange);
                tngpos.y.val = curpos.y.val + deviat - ACTION_RANDOM(devrange);
                tngpos.z.val = curpos.z.val + deviat - ACTION_RANDOM(devrange);
                if ((tngpos.x.val < subtile_coord(map_subtiles_x,0)) && (tngpos.y.val < subtile_coord(map_subtiles_y,0)))
                {
                    struct Thing *eelemtng;
                    eelemtng = create_thing(&tngpos, TCls_EffectElem, 9, game.neutral_player_num, -1);
                    if (!thing_is_invalid(eelemtng)) {
                        eelemtng->sprite_size = sprsize >> 8;
                    }
                }
            }
            curpos.x.val += delta_x;
            curpos.y.val += delta_y;
            curpos.z.val += delta_z;
            deviat += 16;
            sprsize += delta_size;
        }
    }
}

void draw_lightning(const struct Coord3d *pos1, const struct Coord3d *pos2, long eeinterspace, long eemodel)
{
    //_DK_draw_lightning(pos1, pos2, a3, a4);
    MapCoordDelta dist_x;
    MapCoordDelta dist_y;
    MapCoordDelta dist_z;
    dist_x = pos2->x.val - (MapCoordDelta)pos1->x.val;
    dist_y = pos2->y.val - (MapCoordDelta)pos1->y.val;
    dist_z = pos2->z.val - (MapCoordDelta)pos1->z.val;
    int delta_x;
    int delta_y;
    int delta_z;
    if (dist_x >= 0) {
        delta_x = eeinterspace;
    } else {
        dist_x = -dist_x;
        delta_x = -eeinterspace;
    }
    if (dist_y >= 0) {
        delta_y = eeinterspace;
    } else {
        dist_y = -dist_y;
        delta_y = -eeinterspace;
    }
    if (dist_z >= 0) {
        delta_z = eeinterspace;
    } else {
        dist_z = -dist_z;
        delta_z = -eeinterspace;
    }
    if ((dist_x != 0) || (dist_y != 0) || (dist_z != 0))
    {
        int nsteps;
        if ((dist_z >= dist_x) && (dist_z >= dist_y))
        {
            nsteps = dist_z / eeinterspace;
            delta_y = delta_y * dist_y / dist_z;
            delta_x = dist_x * delta_x / dist_z;
        } else
        if ((dist_x >= dist_y) && (dist_x >= dist_z))
        {
            nsteps = dist_x / eeinterspace;
            delta_y = delta_y * dist_y / dist_x;
            delta_z = delta_z * dist_z / dist_x;
        } else
        {
            nsteps = dist_y / eeinterspace;
            delta_x = dist_x * delta_x / dist_y;
            delta_z = delta_z * dist_z / dist_y;
        }
        int deviat_x;
        int deviat_y;
        int deviat_z;
        deviat_x = 0;
        deviat_y = 0;
        deviat_z = 0;
        struct Coord3d curpos;
        curpos.x.val = pos1->x.val + ACTION_RANDOM(eeinterspace/4);
        curpos.y.val = pos1->y.val + ACTION_RANDOM(eeinterspace/4);
        curpos.z.val = pos1->z.val + ACTION_RANDOM(eeinterspace/4);
        int i;
        for (i=nsteps+1; i > 0; i--)
        {
            struct Coord3d tngpos;
            tngpos.x.val = curpos.x.val + deviat_x;
            tngpos.y.val = curpos.y.val + deviat_y;
            tngpos.z.val = curpos.z.val + deviat_z;
            if ((tngpos.x.val < subtile_coord(map_subtiles_x,0)) && (tngpos.y.val < subtile_coord(map_subtiles_y,0)))
            {
                create_thing(&tngpos, TCls_EffectElem, eemodel, game.neutral_player_num, -1);
            }
            if (UNSYNC_RANDOM(6) >= 3) {
                deviat_x -= 32;
            } else {
                deviat_x += 32;
            }
            if (UNSYNC_RANDOM(6) >= 3) {
                deviat_y -= 32;
            } else {
                deviat_y += 32;
            }
            if (UNSYNC_RANDOM(6) >= 3) {
                deviat_z -= 32;
            } else {
                deviat_z += 32;
            }
            int deviat_limit;
            long dist;
            dist = get_3d_box_distance(&curpos, pos2);
            deviat_limit = 128;
            if (dist < 1024)
              deviat_limit = (dist * 128) / 1024;
            // Limit deviations
            if (deviat_x >= -deviat_limit) {
                deviat_x = -deviat_limit;
            } else
            if (deviat_x > deviat_limit) {
                deviat_x = deviat_limit;
            }
            if (deviat_y < -deviat_limit) {
                deviat_y = -deviat_limit;
            } else
            if (deviat_y > deviat_limit) {
                deviat_y = deviat_limit;
            }
            if (deviat_z < -deviat_limit) {
                deviat_z = -deviat_limit;
            } else
            if (deviat_z > deviat_limit) {
                deviat_z = deviat_limit;
            }
            curpos.x.val += delta_x;
            curpos.y.val += delta_y;
            curpos.z.val += delta_z;
        }
    }
}

TbBool setup_move_off_lava(struct Thing *thing)
{
    //return _DK_setup_move_off_lava(thing);
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab(thing->mappos.x.stl.num);
    slb_y = subtile_slab(thing->mappos.y.stl.num);
    long i;
    for (i=0; i < 32; i++)
    {
        struct MapOffset *sstep;
        MapSubtlCoord cx;
        MapSubtlCoord cy;
        sstep = &spiral_step[i];
        cx = slab_subtile_center(slb_x + sstep->h);
        cy = slab_subtile_center(slb_y + sstep->v);
        struct SlabMap *slb;
        slb = get_slabmap_for_subtile(cx,cy);
        if (slabmap_block_invalid(slb))
            continue;
        const struct SlabAttr *slbattr;
        slbattr = get_slab_attrs(slb);
        if (!slbattr->is_safe_land)
            continue;
        // Check all subtiles of the slab in random order
        long k;
        long n;
        n = ACTION_RANDOM(AROUND_TILES_COUNT);
        for (k=0; k < AROUND_TILES_COUNT; k++, n=(n + 1) % AROUND_TILES_COUNT)
        {
            struct Map *mapblk;
            long stl_x;
            long stl_y;
            stl_x = cx + around[k].delta_x;
            stl_y = cy + around[k].delta_y;
            mapblk = get_map_block_at(stl_x,stl_y);
            if (!map_block_invalid(mapblk))
            {
                if ((mapblk->flags & SlbAtFlg_Blocking) == 0)
                {
                    if (setup_person_move_to_position(thing, stl_x, stl_y, 0)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

TbBool any_player_close_enough_to_see(const struct Coord3d *pos)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if ( (player_exists(player)) && ((player->allocflags & PlaF_CompCtrl) == 0))
      {
        if (player->acamera == NULL)
          continue;
        if (get_2d_box_distance(&player->acamera->mappos, pos) <= (24 << 8))
          return true;
      }
    }
    return false;
}

void update_thing_animation(struct Thing *thing)
{
    SYNCDBG(18,"Starting for %s",thing_model_name(thing));
    int i;
    struct CreatureControl *cctrl;
    if (thing->class_id == TCls_Creature)
    {
      cctrl = creature_control_get_from_thing(thing);
      if (!creature_control_invalid(cctrl))
        cctrl->field_CE = thing->field_40;
    }
    if ((thing->field_3E != 0) && (thing->field_49 != 0))
    {
        thing->field_40 += thing->field_3E;
        i = (thing->field_49 << 8);
        if (i <= 0) i = 256;
        while (thing->field_40  < 0)
        {
          thing->field_40 += i;
        }
        if (thing->field_40 > i-1)
        {
          if (thing->field_4F & TF4F_Unknown40)
          {
            thing->field_3E = 0;
            thing->field_40 = i-1;
          } else
          {
            thing->field_40 %= i;
          }
        }
        thing->field_48 = (thing->field_40 >> 8) & 0xFF;
    }
    if (thing->field_4A != 0)
    {
      thing->sprite_size += thing->field_4A;
      if (thing->sprite_size > thing->field_4B)
      {
        if (thing->sprite_size >= thing->field_4D)
        {
          thing->sprite_size = thing->field_4D;
          if ((thing->field_50 & 0x02) != 0)
            thing->field_4A = -thing->field_4A;
          else
            thing->field_4A = 0;
        }
      } else
      {
        thing->sprite_size = thing->field_4B;
        if ((thing->field_50 & 0x02) != 0)
          thing->field_4A = -thing->field_4A;
        else
          thing->field_4A = 0;
      }
    }
}

void init_censorship(void)
{
  if ( censorship_enabled() )
  {
    // Modification for Dark Mistress
      set_creature_model_graphics(20, 14, 48);
  }
}

void engine_init(void)
{
    //_DK_engine_init(); return;
    fill_floor_heights_table();
    generate_wibble_table();
    load_ceiling_table();
}

void init_keeper(void)
{
    SYNCDBG(8,"Starting");
    engine_init();
    init_iso_3d_conversion_tables();
    init_objects();
    init_colours();
    init_spiral_steps();
    init_key_to_strings();
    // Load configs which may have per-campaign part, and even be modified within a level
    load_computer_player_config(CnfLd_Standard);
    load_stats_files();
    check_and_auto_fix_stats();
    init_creature_scores();
    // Load graphics structures
    load_cubes_config(CnfLd_Standard);
    //load_cube_file();
    init_top_texture_to_cube_table();
    load_texture_anim_file();
    game.neutral_player_num = neutral_player_number;
    game.field_14EA34 = 4;
    game.field_14EA38 = 200;
    game.field_14EA28 = 256;
    game.field_14EA2A = 256;
    game.field_14EA2C = 256;
    game.field_14EA2E = 256;
    if (game.generate_speed <= 0)
      game.generate_speed = game.default_generate_speed;
    poly_pool_end = &poly_pool[sizeof(poly_pool)-128];
    lbDisplay.GlassMap = pixmap.ghost;
    lbDisplay.DrawColour = colours[15][15][15];
    game.comp_player_aggressive = 0;
    game.comp_player_defensive = 1;
    game.comp_player_construct = 0;
    game.comp_player_creatrsonly = 0;
    game.creatures_tend_imprison = 0;
    game.creatures_tend_flee = 0;
    game.operation_flags |= GOF_ShowPanel;
    game.numfield_D |= (GNFldD_Unkn20 | GNFldD_Unkn40);
    init_censorship();
    SYNCDBG(9,"Finished");
}

short ceiling_set_info(long height_max, long height_min, long step)
{
    SYNCDBG(6,"Starting");
    long dist;
    if (step <= 0)
    {
      ERRORLOG("Illegal ceiling step value");
      return 0;
    }
    if (height_max > 15)
    {
      ERRORLOG("Max height is too high");
      return 0;
    }
    if (height_min > height_max)
    {
      ERRORLOG("Ceiling max height is smaller than min height");
      return 0;
    }
    dist = (height_max - height_min) / step;
    if ( dist >= 2500 )
      dist = 2500;
    game.field_14A80C = dist;
    if (dist > 20)
    {
      ERRORLOG("Ceiling search distance too big");
      return 0;
    }
    game.field_14A804 = height_max;
    game.field_14A808 = height_min;
    game.field_14A814 = step;
    game.field_14A810 = (2*game.field_14A80C+1) * (2*game.field_14A80C+1);
    return 1;
}

void IsRunningMark(void)
{
    _DK_IsRunningMark();
/*  HKEY hKey;
  if ( !RegCreateKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Bullfrog Productions Ltd\\Dungeon Keeper\\IsRunning", &hKey) )
    RegCloseKey(hKey);*/
}

void IsRunningUnmark(void)
{
    _DK_IsRunningUnmark();
    /*HKEY hKey;
    if ( !RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Bullfrog Productions Ltd\\Dungeon Keeper\\IsRunning",
            0, 0x20019u, &hKey) )
    {
        RegCloseKey(hKey);
        RegDeleteKeyA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Bullfrog Productions Ltd\\Dungeon Keeper\\IsRunning");
    }*/
}

/**
 * Initial video setup - loads only most important files to show startup screens.
 */
TbBool initial_setup(void)
{
    SYNCDBG(6,"Starting");
    // setting this will force video mode change, even if previous one is same
    MinimalResolutionSetup = true;
    // Set size of static textures buffer
    game_load_files[1].SLength = max((ulong)TEXTURE_BLOCKS_STAT_COUNT*block_dimension*block_dimension,(ulong)LANDVIEW_MAP_WIDTH*LANDVIEW_MAP_HEIGHT);
    if (LbDataLoadAll(game_load_files))
    {
        ERRORLOG("Unable to load game_load_files");
        return false;
    }
    // was LoadMcgaData, but minimal should be enough at this point.
    if (!LoadMcgaDataMinimal())
    {
        ERRORLOG("Loading MCGA files failed");
        return false;
    }
    load_pointer_file(0);
    update_screen_mode_data(320, 200);
    clear_game();
    lbDisplay.DrawFlags |= 0x4000u;
    return true;
}

/**
 * Displays 'legal' screens, intro and initializes basic game data.
 * If true is returned, then all files needed for startup were loaded,
 * and there should be the loading screen visible.
 * @return Returns true on success, false on error which makes the
 *   gameplay impossible (usually files loading failure).
 * @note The current screen resolution at end of this function may vary.
 */
short setup_game(void)
{
  struct CPU_INFO cpu_info; // CPU status variable
  short result;

  // Do only a very basic setup
  cpu_detect(&cpu_info);
  SYNCMSG("CPU %s type %d family %d model %d stepping %d features %08x",cpu_info.vendor,
      (int)cpu_get_type(&cpu_info),(int)cpu_get_family(&cpu_info),(int)cpu_get_model(&cpu_info),
      (int)cpu_get_stepping(&cpu_info),cpu_info.feature_edx);
  update_memory_constraits();
  // Enable features that require more resources
  update_features(mem_size);

  // Configuration file
  if ( !load_configuration() )
  {
      ERRORLOG("Configuration load error.");
      return 0;
  }

  LbIKeyboardOpen();

  if (LbDataLoadAll(legal_load_files) != 0)
  {
      ERRORLOG("Error on allocation/loading of legal_load_files.");
      return 0;
  }

  // View the legal screen

  if (!setup_screen_mode_zero(get_frontend_vidmode()))
  {
      ERRORLOG("Unable to set display mode for legal screen");
      return 0;
  }

  result = init_actv_bitmap_screen(RBmp_SplashLegal);
 if ( result )
 {
     result = show_actv_bitmap_screen(3000);
     free_actv_bitmap_screen();
 } else
      SYNCLOG("Legal image skipped");

  // Now do more setup
  // Prepare the Game structure
  clear_complete_game();
  // Moon phase calculation
  calculate_moon_phase(true,true);
  // Start the sound system
  if (!init_sound())
    WARNMSG("Sound system disabled.");
  // Note: for some reason, signal handlers must be installed AFTER
  // init_sound(). This will probably change when we'll move sound
  // to SDL - then we'll put that line earlier, before setup_game().
  LbErrorParachuteInstall();

  // View second splash screen
  result = init_actv_bitmap_screen(RBmp_SplashFx);
 if ( result )
 {
     result = show_actv_bitmap_screen(4000);
     free_actv_bitmap_screen();
 } else
      SYNCLOG("startup_fx image skipped");
  draw_clear_screen();

  // View Bullfrog company logo animation when new moon
  if ( is_new_moon )
    if ( !game.no_intro )
    {
        result = moon_video();
        if ( !result ) {
            ERRORLOG("Unable to play new moon movie");
        }
    }

  result = 1;
  // The 320x200 mode is required only for the intro;
  // loading and no CD screens can run in both 320x2?0 and 640x4?0.
  if ( result && (!game.no_intro) )
  {
      LbPaletteDataFillBlack(engine_palette);
      int mode_ok = LbScreenSetup(get_movies_vidmode(), 320, 200, engine_palette, 2, 0);
      if (mode_ok != 1)
      {
        ERRORLOG("Can't enter movies screen mode to play intro");
        result=0;
      }
  }

  if ( result )
  {
      draw_clear_screen();
      result = wait_for_cd_to_be_available();
  }

  game.frame_skip = start_params.frame_skip;

  if ( result && (!game.no_intro) )
  {
     result = intro_replay();
  }
  // Intro problems shouldn't force the game to quit,
  // so we're re-setting the result flag
  result = 1;

  if ( result )
  {
      display_loading_screen();
  }
  LbDataFreeAll(legal_load_files);

  if ( result )
  {
      IsRunningMark();
      if ( !initial_setup() )
        result = 0;
  }

  if ( result )
  {
    load_settings();
    if ( !setup_gui_strings_data() )
      result = 0;
  }

  if ( result )
  {
    if ( !setup_heaps() )
      result = 0;
  }

  if ( result )
  {
      init_keeper();
      switch (start_params.force_ppro_poly)
      {
      case 1:
          gpoly_enable_pentium_pro(true);
          break;
      case 2:
          gpoly_enable_pentium_pro(false);
          break;
      default:
          if (cpu_info.feature_intl == 0)
          {
              gpoly_enable_pentium_pro(false);
          } else
          if ( ((cpu_info.feature_intl>>8) & 0x0F) < 0x06 ) {
              gpoly_enable_pentium_pro(false);
          } else {
              gpoly_enable_pentium_pro(true);
          }
          break;
      }
      set_gamma(settings.gamma_correction, 0);
      SetMusicPlayerVolume(settings.redbook_volume);
      SetSoundMasterVolume(settings.sound_volume);
      SetMusicMasterVolume(settings.sound_volume);
      setup_3d();
      setup_stuff();
      init_lookups();
      result = 1;
  }

  if (result) {
      KEEPERSPEECH_REASON reason = KeeperSpeechInit();
      if (reason == KSR_NO_LIB_INSTALLED) {
          SYNCLOG("Speech recognition disabled: %s",
              KeeperSpeechErrorMessage(reason));
      } else
      if (reason != KSR_OK) {
          ERRORLOG("Failed to initialize Speech recognition module: %s",
              KeeperSpeechErrorMessage(reason));
      }
  }

  return result;
}

/** Returns if cursor for given player is at top of the dungeon in 3D view.
 *  Cursor placed at top of dungeon is marked by green/red "volume box";
 *   if there's no volume box, cursor should be of the field behind it
 *   (the exact field in a line of view through cursor). If cursor is at top
 *   of view, then pointed map field is a bit lower than the line of view
 *   through cursor.
 *
 * @param player
 * @return
 */
TbBool players_cursor_is_at_top_of_view(struct PlayerInfo *player)
{
    int i;
    i = player->work_state;
    if ( (i == PSt_BuildRoom) || (i == PSt_PlaceDoor) || (i == PSt_PlaceTrap) || (i == PSt_SightOfEvil) || (i == PSt_Sell) )
        return true;
    if ( (i == PSt_CtrlDungeon) && (player->field_454 != P454_Unkn0) && (player->thing_under_hand == 0) )
        return true;
    return false;
}

TbBool engine_point_to_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
    struct PlayerInfo *player;
    player = get_my_player();
    *map_x = 0;
    *map_y = 0;
    if ( (pointer_x >= 0) && (pointer_y >= 0)
      && (pointer_x < (player->engine_window_width/pixel_size))
      && (pointer_y < (player->engine_window_height/pixel_size)) )
    {
        if ( players_cursor_is_at_top_of_view(player) )
        {
              *map_x = subtile_coord(top_pointed_at_x,top_pointed_at_frac_x);
              *map_y = subtile_coord(top_pointed_at_y,top_pointed_at_frac_y);
        } else
        {
              *map_x = subtile_coord(block_pointed_at_x,pointed_at_frac_x);
              *map_y = subtile_coord(block_pointed_at_y,pointed_at_frac_y);
        }
        // Clipping coordinates
        if (*map_y < 0)
          *map_y = 0;
        else
        if (*map_y > subtile_coord(map_subtiles_y,-1))
          *map_y = subtile_coord(map_subtiles_y,-1);
        if (*map_x < 0)
          *map_x = 0;
        else
        if (*map_x > subtile_coord(map_subtiles_x,-1))
          *map_x = subtile_coord(map_subtiles_x,-1);
        return true;
    }
    return false;
}

TbBool screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos)
{
    TbBool result;
    long x;
    long y;
    SYNCDBG(19,"Starting");
    result = false;
    if (camera != NULL)
    {
      switch (camera->view_mode)
      {
        case PVM_CreatureView:
        case PVM_IsometricView:
        case PVM_FrontView:
          // 3D view mode
          result = engine_point_to_map(camera,screen_x,screen_y,&x,&y);
          break;
        case PVM_ParchmentView: //map mode
          result = point_to_overhead_map(camera,screen_x/pixel_size,screen_y/pixel_size,&x,&y);
          break;
        default:
          result = false;
          break;
      }
    }
    if ( result )
    {
      mappos->x.val = x;
      mappos->y.val = y;
    }
    if ( mappos->x.val > ((map_subtiles_x<<8)-1) )
      mappos->x.val = ((map_subtiles_x<<8)-1);
    if ( mappos->y.val > ((map_subtiles_y<<8)-1) )
      mappos->y.val = ((map_subtiles_y<<8)-1);
    SYNCDBG(19,"Finished");
    return result;
}

void update_creatr_model_activities_list(void)
{
    //_DK_update_breed_activities();
    struct Dungeon *dungeon;
    dungeon = get_my_dungeon();
    ThingModel crmodel;
    int num_breeds;
    num_breeds = no_of_breeds_owned;
    // Add to breed activities
    for (crmodel=1; crmodel < CREATURE_TYPES_COUNT; crmodel++)
    {
        if ((dungeon->owned_creatures_of_model[crmodel] > 0)
            && (crmodel != get_players_spectator_model(my_player_number)))
        {
            int i;
            for (i=0; i < num_breeds; i++)
            {
                if (breed_activities[i] == crmodel)
                {
                    break;
                }
            }
            if (num_breeds == i)
            {
                breed_activities[i] = crmodel;
                num_breeds++;
            }
        }
    }
    // Remove from breed activities
    for (crmodel=1; crmodel < CREATURE_TYPES_COUNT; crmodel++)
    {
        if ((dungeon->owned_creatures_of_model[crmodel] <= 0)
          && (crmodel != get_players_special_digger_model(my_player_number)))
        {
            int i;
            for (i=0; i < num_breeds; i++)
            {
                if (breed_activities[i] == crmodel)
                {
                    for (; i < num_breeds-1;  i++) {
                        breed_activities[i] = breed_activities[i+1];
                    }
                    num_breeds--;
                    breed_activities[i] = 0;
                    break;
                }
            }
        }
        no_of_breeds_owned = num_breeds;
    }
}

void toggle_hero_health_flowers(void)
{
    const char *statstr;
    toggle_flag_byte(&game.flags_cd,MFlg_NoHeroHealthFlower);
    if (game.flags_cd & MFlg_NoHeroHealthFlower)
    {
      statstr = "off";
    } else
    {
      do_sound_menu_click();
      statstr = "on";
    }
    show_onscreen_msg(2*game.num_fps, "Hero health flowers %s", statstr);
}

void reset_gui_based_on_player_mode(void)
{
    struct PlayerInfo *player;
    //_DK_reset_gui_based_on_player_mode();
    player = get_my_player();
    if ((player->view_type == PVT_CreatureContrl) || (player->view_type == PVT_CreaturePasngr))
    {
        turn_on_menu(GMnu_CREATURE_QUERY1);
    } else
    {
        turn_on_menu(GMnu_MAIN);
        if (game.active_panel_mnu_idx > 0)
        {
            initialise_tab_tags(game.active_panel_mnu_idx);
            turn_on_menu(game.active_panel_mnu_idx);
            MenuNumber mnuidx;
            mnuidx = menu_id_to_number(GMnu_MAIN);
            if (mnuidx != MENU_INVALID_ID) {
                setup_radio_buttons(&active_menus[mnuidx]);
            }
        }
        else
        {
            turn_on_menu(GMnu_ROOM);
        }
    }
    settings.video_cluedo_mode = player->video_cluedo_mode;
    set_gui_visible(true);
}

void reinit_tagged_blocks_for_player(PlayerNumber plyr_idx)
{
    //_DK_reinit_tagged_blocks_for_player(plyr_idx); return;
    // Clear tagged blocks
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    for (stl_y=0; stl_y < map_subtiles_y; stl_y++)
    {
        for (stl_x=0; stl_x < map_subtiles_x; stl_x++)
        {
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x, stl_y);
            mapblk->flags &= ~SlbAtFlg_Unexplored;
            mapblk->flags &= ~SlbAtFlg_TaggedValuable;
        }
    }
    // Reinit with data from current players dungeon
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    int task_idx;
    for (task_idx = 0; task_idx < dungeon->highest_task_number; task_idx++)
    {
        struct MapTask  *mtask;
        mtask = &dungeon->task_list[task_idx];
        MapSubtlCoord taskstl_x;
        MapSubtlCoord taskstl_y;
        taskstl_x = stl_num_decode_x(mtask->coords);
        taskstl_y = stl_num_decode_y(mtask->coords);
        switch (mtask->kind)
        {
        case 2:
            for (stl_y = taskstl_y - 1; stl_y <= taskstl_y + 1; stl_y++)
            {
                for (stl_x = taskstl_x - 1; stl_x <= taskstl_x + 1; stl_x++)
                {
                    struct Map *mapblk;
                    mapblk = get_map_block_at(stl_x, stl_y);
                    mapblk->flags |= SlbAtFlg_TaggedValuable;
                }
            }
            break;
        case 1:
        case 3:
            for (stl_y = taskstl_y - 1; stl_y <= taskstl_y + 1; stl_y++)
            {
                for (stl_x = taskstl_x - 1; stl_x <= taskstl_x + 1; stl_x++)
                {
                    struct Map *mapblk;
                    mapblk = get_map_block_at(stl_x, stl_y);
                    mapblk->flags |= SlbAtFlg_Unexplored;
                }
            }
            break;
        default:
            break;
        }
    }
}

void instant_instance_selected(CrInstance check_inst_id)
{
    //_DK_instant_instance_selected(check_inst_id);
    struct PlayerInfo *player;
    player = get_player(my_player_number);
    struct Thing *ctrltng;
    ctrltng = thing_get(player->controlled_thing_idx);
    struct CreatureStats *crstat;
    crstat = creature_stats_get_from_thing(ctrltng);
    long i;
    long k;
    int avail_pos;
    int match_avail_pos;
    avail_pos = 0;
    match_avail_pos = 0;
    for (i=0; i < CREATURE_MAX_LEVEL; i++)
    {
        k = crstat->learned_instance_id[i];
        if (creature_instance_is_available(ctrltng, k))
        {
            if (k == check_inst_id) {
                match_avail_pos = avail_pos;
                break;
            }
            avail_pos++;
        }
    }
    first_person_instance_top_half_selected = match_avail_pos < 6 && (first_person_instance_top_half_selected || match_avail_pos < 4);
}

short zoom_to_next_annoyed_creature(void)
{
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *thing;
    player = get_my_player();
    dungeon = get_players_num_dungeon(my_player_number);
    dungeon->zoom_annoyed_creature_idx = find_next_annoyed_creature(player->id_number,dungeon->zoom_annoyed_creature_idx);
    thing = thing_get(dungeon->zoom_annoyed_creature_idx);
    if (thing_is_invalid(thing))
    {
      return false;
    }
    set_players_packet_action(player, PckA_ZoomToPosition, thing->mappos.x.val, thing->mappos.y.val, 0, 0);
    return true;
}

TbBool toggle_computer_player(PlayerNumber plyr_idx)
{
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    if ((dungeon->computer_enabled & 0x01) == 0)
    {
        dungeon->computer_enabled |= 0x01;
    } else
    {
        dungeon->computer_enabled &= ~0x01;
    }
    struct Computer2 *comp;
    comp = get_computer_player(player->id_number);
    computer_force_dump_held_things_on_map(comp, &dungeon->essential_pos);
    return true;
}

void reinit_level_after_load(void)
{
    struct PlayerInfo *player;
    int i;
    SYNCDBG(6,"Starting");
    // Reinit structures from within the game
    player = get_my_player();
    player->field_7 = 0;
    init_lookups();
    init_navigation();
    reinit_packets_after_load();
    game.flags_font |= start_params.flags_font;
    parchment_loaded = 0;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if (player_exists(player))
        set_engine_view(player, player->view_mode);
    }
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];
    load_texture_map_file(game.texture_id, 2);
    init_animating_texture_maps();

    init_gui();
    reset_gui_based_on_player_mode();
    erstats_clear();
    player = get_my_player();
    reinit_tagged_blocks_for_player(player->id_number);
    restore_computer_player_after_load();
    sound_reinit_after_load();
}

/**
 * Sets to defaults some basic parameters which are
 * later copied into Game structure.
 */
TbBool set_default_startup_parameters(void)
{
    memset(&start_params, 0, sizeof(struct StartupParameters));
    start_params.packet_checksum_verify = 1;
    set_flag_byte(&start_params.flags_font,FFlg_unk01,false);
    // Set levels to 0, as we may not have the campaign loaded yet
    start_params.selected_level_number = 0;
    start_params.num_fps = 20;
    start_params.one_player = 1;
    start_params.computer_chat_flags = CChat_None;
    set_flag_byte(&start_params.flags_cd,MFlg_IsDemoMode,false);
    set_flag_byte(&start_params.flags_cd,MFlg_unk40,true);
    start_params.force_ppro_poly = 0;
    return true;
}

/**
 * Clears the Game structure completely, and copies startup parameters
 * from start_params structure.
 */
void clear_complete_game(void)
{
    memset(&game, 0, sizeof(struct Game));
    memset(&gameadd, 0, sizeof(struct GameAdd));
    memset(&intralvl, 0, sizeof(struct IntralevelData));
    game.turns_packetoff = -1;
    game.local_plyr_idx = default_loc_player;
    game.packet_checksum_verify = start_params.packet_checksum_verify;
    game.numfield_1503A2 = -1;
    game.flags_font = start_params.flags_font;
    game.numfield_149F47 = 0;
    // Set levels to 0, as we may not have the campaign loaded yet
    set_continue_level_number(first_singleplayer_level());
    if ((start_params.operation_flags & GOF_SingleLevel) != 0)
      set_selected_level_number(start_params.selected_level_number);
    else
      set_selected_level_number(first_singleplayer_level());
    game.num_fps = start_params.num_fps;
    game.flags_cd = start_params.flags_cd;
    game.no_intro = start_params.no_intro;
    set_flag_byte(&game.system_flags,GSF_AllowOnePlayer,start_params.one_player);
    gameadd.computer_chat_flags = start_params.computer_chat_flags;
    game.operation_flags = start_params.operation_flags;
    strncpy(game.packet_fname,start_params.packet_fname,150);
    game.packet_save_enable = start_params.packet_save_enable;
    game.packet_load_enable = start_params.packet_load_enable;
    my_player_number = default_loc_player;
}

void clear_slabsets(void)
{
    struct SlabSet *sset;
    struct SlabObj *sobj;
    int i;
    for (i=0; i < SLABSET_COUNT; i++)
    {
        sset = &game.slabset[i];
        memset(sset, 0, sizeof(struct SlabSet));
        game.slabobjs_idx[i] = -1;
    }
    game.slabset_num = SLABSET_COUNT;
    game.slabobjs_num = 0;
    for (i=0; i < SLABOBJS_COUNT; i++)
    {
        sobj = &game.slabobjs[i];
        memset(sobj, 0, sizeof(struct SlabObj));
    }
}

void clear_map(void)
{
    clear_mapmap();
    clear_slabs();
    clear_columns();
    clear_slabsets();
}

void clear_things_and_persons_data(void)
{
    struct Thing *thing;
    long i;
    for (i=0; i < THINGS_COUNT; i++)
    {
        thing = &game.things_data[i];
        memset(thing, 0, sizeof(struct Thing));
        thing->owner = PLAYERS_COUNT;
        thing->mappos.x.val = subtile_coord_center(map_subtiles_x/2);
        thing->mappos.y.val = subtile_coord_center(map_subtiles_y/2);
    }
    for (i=0; i < CREATURES_COUNT; i++)
    {
      memset(&game.cctrl_data[i], 0, sizeof(struct CreatureControl));
    }
}

void clear_computer(void)
{
    long i;
    SYNCDBG(8,"Starting");
    for (i=0; i < COMPUTER_TASKS_COUNT; i++)
    {
        LbMemorySet(&game.computer_task[i], 0, sizeof(struct ComputerTask));
    }
    for (i=0; i < GOLD_LOOKUP_COUNT; i++)
    {
        LbMemorySet(&game.gold_lookup[i], 0, sizeof(struct GoldLookup));
    }
    for (i=0; i < PLAYERS_COUNT; i++)
    {
        LbMemorySet(&game.computer[i], 0, sizeof(struct Computer2));
    }
}

void init_keepers_map_exploration(void)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if (player_exists(player) && (player->is_active == 1))
      {
          // Additional init - the main one is in init_player()
          if ((player->allocflags & PlaF_CompCtrl) != 0) {
              init_keeper_map_exploration_by_terrain(player);
              init_keeper_map_exploration_by_creatures(player);
          }
      }
    }
}

void clear_players_for_save(void)
{
    struct PlayerInfo *player;
    unsigned short id_mem;
    unsigned short mem2;
    unsigned short memflg;
    struct Camera cammem;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      id_mem = player->id_number;
      mem2 = player->is_active;
      memflg = player->allocflags;
      LbMemoryCopy(&cammem,&player->cameras[CamIV_FirstPerson],sizeof(struct Camera));
      memset(player, 0, sizeof(struct PlayerInfo));
      player->id_number = id_mem;
      player->is_active = mem2;
      set_flag_byte(&player->allocflags,PlaF_Allocated,((memflg & PlaF_Allocated) != 0));
      set_flag_byte(&player->allocflags,PlaF_CompCtrl,((memflg & PlaF_CompCtrl) != 0));
      LbMemoryCopy(&player->cameras[CamIV_FirstPerson],&cammem,sizeof(struct Camera));
      player->acamera = &player->cameras[CamIV_FirstPerson];
    }
}

void delete_all_thing_structures(void)
{
    long i;
    struct Thing *thing;
    for (i=1; i < THINGS_COUNT; i++)
    {
      thing = thing_get(i);
      if (thing_exists(thing)) {
          delete_thing_structure(thing, 1);
      }
    }
    for (i=0; i < THINGS_COUNT-1; i++) {
      game.free_things[i] = i+1;
    }
    game.free_things_start_index = 0;
}

void delete_all_structures(void)
{
    SYNCDBG(6,"Starting");
    delete_all_thing_structures();
    delete_all_control_structures();
    delete_all_room_structures();
    delete_all_action_point_structures();
    light_initialise();
    SYNCDBG(16,"Done");
}

/**
 * Clears game structures at end of level.
 * Also used as part of clearing before new level is loaded.
 */
void clear_game_for_summary(void)
{
    SYNCDBG(6,"Starting");
    delete_all_structures();
    clear_shadow_limits(&game.lish);
    clear_stat_light_map();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_rand_seed = 0;
    game.operation_flags &= ~GOF_Unkn04;
    game.operation_flags &= ~GOF_Paused;
    clear_columns();
    clear_action_points();
    clear_players();
    clear_dungeons();
}

void clear_game(void)
{
    SYNCDBG(6,"Starting");
    clear_game_for_summary();
    game.audiotrack = 0;
    clear_map();
    clear_computer();
    clear_script();
    clear_events();
    clear_things_and_persons_data();
    ceiling_set_info(12, 4, 1);
    init_animating_texture_maps();
}

void clear_game_for_save(void)
{
    SYNCDBG(6,"Starting");
    delete_all_structures();
    light_initialise();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_rand_seed = 0;
    set_flag_byte(&game.operation_flags,GOF_Unkn04,false);
    clear_columns();
    clear_players_for_save();
    clear_dungeons();
}

void reset_creature_max_levels(void)
{
    int i;
    int k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        struct Dungeon *dungeon;
        dungeon = get_dungeon(i);
        for (k=1; k < CREATURE_TYPES_COUNT; k++)
        {
            dungeon->creature_max_level[k] = CREATURE_MAX_LEVEL+1;
        }
    }
}

/**
 * Resets timers and flags of all players into default (zeroed) state.
 * Also enables spells which are always enabled by default.
 */
void reset_script_timers_and_flags(void)
{
    struct Dungeon *dungeon;
    int plyr_idx;
    int k;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        add_power_to_player(PwrK_HAND, plyr_idx);
        add_power_to_player(PwrK_SLAP, plyr_idx);
        add_power_to_player(PwrK_POSSESS, plyr_idx);
        dungeon = get_dungeon(plyr_idx);
        for (k=0; k<TURN_TIMERS_COUNT; k++)
        {
            memset(&dungeon->turn_timers[k], 0, sizeof(struct TurnTimer));
            dungeon->turn_timers[k].state = 0;
        }
        for (k=0; k<SCRIPT_FLAGS_COUNT; k++)
        {
            dungeon->script_flags[k] = 0;
        }
      }
}

void init_good_player_as(PlayerNumber plr_idx)
{
    struct PlayerInfo *player;
    game.hero_player_num = plr_idx;
    player = get_player(plr_idx);
    player->allocflags |= PlaF_Allocated;
    player->allocflags |= PlaF_CompCtrl;
    player->id_number = game.hero_player_num;
}

void change_engine_window_relative_size(long w_delta, long h_delta)
{
    struct PlayerInfo *myplyr;
    myplyr=get_my_player();
    setup_engine_window(myplyr->engine_window_x, myplyr->engine_window_y,
        myplyr->engine_window_width+w_delta, myplyr->engine_window_height+h_delta);
}

void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal)
{
    //Todo: Figure out statement below. Used to be blue_palette like 'pinstfe_control_creature_fade'.
    if (pal == red_palette)
    {
      if ((player->field_3 & Pf3F_Unkn04) == 0)
        return;
      player->field_3 |= Pf3F_Unkn04;
    } else
    {
      player->field_3 &= ~Pf3F_Unkn04;
    }
    if ( (player->field_7 == 0) || ((pal != player->palette) && (pal == player->field_7)) )
    {
        player->palette = pal;
        player->field_4C1 = 0;
        player->field_4C5 = 0;
        if (is_my_player(player))
        {
            LbScreenWaitVbi();
            LbPaletteSet(pal);
        }
    }
}

TbBool set_gamma(char corrlvl, TbBool do_set)
{
    char *fname;
    TbBool result = true;
    if (corrlvl < 0)
      corrlvl = 0;
    else
    if (corrlvl > 4)
      corrlvl = 4;
    settings.gamma_correction = corrlvl;
    fname=prepare_file_fmtpath(FGrp_StdData,"pal%05d.dat",settings.gamma_correction);
    if (!LbFileExists(fname))
    {
      WARNMSG("Palette file \"%s\" doesn't exist.", fname);
      result = false;
    }
    if (result)
    {
      result = (LbFileLoadAt(fname, engine_palette) != -1);
    }
    if ((result) && (do_set))
    {
      struct PlayerInfo *myplyr;
      myplyr=get_my_player();
      PaletteSetPlayerPalette(myplyr, engine_palette);
    }
    if (!result)
      ERRORLOG("Can't load palette file.");
    return result;
}

void centre_engine_window(void)
{
    long x1;
    long y1;
    struct PlayerInfo *player=get_my_player();
    if ((game.operation_flags & GOF_ShowGui) != 0)
      x1 = (MyScreenWidth-player->engine_window_width-status_panel_width) / 2 + status_panel_width;
    else
      x1 = (MyScreenWidth-player->engine_window_width) / 2;
    y1 = (MyScreenHeight-player->engine_window_height) / 2;
    setup_engine_window(x1, y1, player->engine_window_width, player->engine_window_height);
}

void turn_off_query(PlayerNumber plyr_idx)
{
    //_DK_turn_off_query(a);
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    set_player_instance(player, PI_UnqueryCrtr, 0);
}

void level_lost_go_first_person(PlayerNumber plyr_idx)
{
    struct CreatureControl *cctrl;
    struct PlayerInfo *player;
    struct Dungeon *dungeon;
    struct Thing *thing;
    ThingModel spectator_breed;
    SYNCDBG(6,"Starting for player %d",(int)plyr_idx);
    //_DK_level_lost_go_first_person(plridx);
    player = get_player(plyr_idx);
    dungeon = get_dungeon(player->id_number);
    if (dungeon_invalid(dungeon)) {
        ERRORLOG("Unable to get player %d dungeon",(int)plyr_idx);
        return;
    }
    spectator_breed = get_players_spectator_model(plyr_idx);
    player->dungeon_camera_zoom = get_camera_zoom(player->acamera);
    thing = create_and_control_creature_as_controller(player, spectator_breed, &dungeon->mappos);
    if (thing_is_invalid(thing)) {
        ERRORLOG("Unable to create spectator creature");
        return;
    }
    cctrl = creature_control_get_from_thing(thing);
    cctrl->flgfield_1 |= CCFlg_NoCompControl;
    SYNCDBG(8,"Finished");
}

void find_map_location_coords(long location, long *x, long *y, const char *func_name)
{
    struct ActionPoint *apt;
    struct Thing *thing;
    long pos_x;
    long pos_y;
    long i;
    SYNCDBG(15,"From %s; Location %ld, pos(%ld,%ld)",func_name, location, *x, *y);
    pos_y = 0;
    pos_x = 0;
    switch (get_map_location_type(location))
    {
    case MLoc_ACTIONPOINT:
        i = get_map_location_longval(location);
        // Location stores action point index
        apt = action_point_get(i);
        if (!action_point_is_invalid(apt))
        {
          pos_y = apt->mappos.y.stl.num;
          pos_x = apt->mappos.x.stl.num;
        } else
          WARNMSG("%s: Action Point %d location not found",func_name,i);
        break;
    case MLoc_HEROGATE:
        i = get_map_location_longval(location);
        thing = find_hero_gate_of_number(i);
        if (!thing_is_invalid(thing))
        {
          pos_y = thing->mappos.y.stl.num;
          pos_x = thing->mappos.x.stl.num;
        } else
          WARNMSG("%s: Hero Gate %d location not found",func_name,i);
        break;
    case MLoc_PLAYERSHEART:
        i = get_map_location_longval(location);
        if (i < PLAYERS_COUNT)
        {
            thing = get_player_soul_container(i);
        } else
          thing = INVALID_THING;
        if (!thing_is_invalid(thing))
        {
          pos_y = thing->mappos.y.stl.num;
          pos_x = thing->mappos.x.stl.num;
        } else
          WARNMSG("%s: Dungeon Heart location for player %d not found",func_name,i);
        break;
    case MLoc_NONE:
        pos_y = *y;
        pos_x = *x;
        break;
    case MLoc_THING:
        i = get_map_location_longval(location);
        thing = thing_get(i);
        if (!thing_is_invalid(thing))
        {
          pos_y = thing->mappos.y.stl.num;
          pos_x = thing->mappos.x.stl.num;
        } else
          WARNMSG("%s: Thing %d location not found",func_name,i);
        break;
    case MLoc_CREATUREKIND:
    case MLoc_OBJECTKIND:
    case MLoc_ROOMKIND:
    case MLoc_PLAYERSDUNGEON:
    case MLoc_APPROPRTDUNGEON:
    case MLoc_DOORKIND:
    case MLoc_TRAPKIND:
    default:
          WARNMSG("%s: Unsupported location, %lu.",func_name,location);
        break;
    }
    *y = pos_y;
    *x = pos_x;
}

void set_general_information(long msg_id, long target, long x, long y)
{
    struct PlayerInfo *player;
    long pos_x;
    long pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, __func__);
    pos_x = 0;
    pos_y = 0;
    if ((x != 0) || (y != 0))
    {
        pos_y = subtile_coord_center(y);
        pos_x = subtile_coord_center(x);
    }
    event_create_event(pos_x, pos_y, EvKind_Information, player->id_number, -msg_id);
}

void set_quick_information(long msg_id, long target, long x, long y)
{
    struct PlayerInfo *player;
    long pos_x;
    long pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, __func__);
    pos_x = 0;
    pos_y = 0;
    if ((x != 0) || (y != 0))
    {
        pos_y = subtile_coord_center(y);
        pos_x = subtile_coord_center(x);
    }
    event_create_event(pos_x, pos_y, EvKind_QuickInformation, player->id_number, -msg_id);
}

void set_general_objective(long msg_id, long target, long x, long y)
{
    process_objective(get_string(msg_id), target, x, y);
}

void process_objective(const char *msg_text, long target, long x, long y)
{
    struct PlayerInfo *player;
    long pos_x;
    long pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, __func__);
    pos_y = y;
    pos_x = x;
    set_level_objective(msg_text);
    display_objectives(player->id_number, pos_x, pos_y);
}

short winning_player_quitting(struct PlayerInfo *player, long *plyr_count)
{
    struct PlayerInfo *swplyr;
    int i;
    int k;
    int n;
    if (player->victory_state == VicS_LostLevel)
    {
      return 0;
    }
    k = 0;
    n = 0;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      swplyr = get_player(i);
      if (player_exists(swplyr))
      {
        if (swplyr->is_active == 1)
        {
          k++;
          if (swplyr->victory_state == VicS_LostLevel)
            n++;
        }
      }
    }
    *plyr_count = k;
    return ((k - n) == 1);
}

short lose_level(struct PlayerInfo *player)
{
    if (!is_my_player(player))
        return false;
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        LbNetwork_Stop();
    }
    quit_game = 1;
    return true;
}

short resign_level(struct PlayerInfo *player)
{
    if (!is_my_player(player))
        return false;
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        LbNetwork_Stop();
    }
    quit_game = 1;
    return true;
}

short complete_level(struct PlayerInfo *player)
{
    SYNCDBG(6,"Starting");
    if (!is_my_player(player))
        return false;
    if ((game.system_flags & GSF_NetworkActive) != 0)
    {
        LbNetwork_Stop();
        quit_game = 1;
        return true;
    }
    LevelNumber lvnum;
    lvnum = get_continue_level_number();
    if (get_loaded_level_number() == lvnum)
    {
        SYNCDBG(7,"Progressing the campaign");
        move_campaign_to_next_level();
    }
    quit_game = 1;
    return true;
}

void init_lookups(void)
{
    long i;
    SYNCDBG(8,"Starting");
    for (i=0; i < THINGS_COUNT; i++)
    {
      game.things.lookup[i] = &game.things_data[i];
    }
    game.things.end = &game.things_data[THINGS_COUNT];

    memset(&game.persons, 0, sizeof(struct Persons));
    for (i=0; i < CREATURES_COUNT; i++)
    {
      game.persons.cctrl_lookup[i] = &game.cctrl_data[i];
    }
    game.persons.cctrl_end = &game.cctrl_data[CREATURES_COUNT];

    for (i=0; i < COLUMNS_COUNT; i++)
    {
      game.columns.lookup[i] = &game.columns_data[i];
    }
    game.columns.end = &game.columns_data[COLUMNS_COUNT];
}

void clear_lookups(void)
{
    long i;
    SYNCDBG(8,"Starting");
    for (i=0; i < THINGS_COUNT; i++)
    {
      game.things.lookup[i] = NULL;
    }
    game.things.end = NULL;

    memset(&game.persons, 0, sizeof(struct Persons));

    for (i=0; i < COLUMNS_COUNT; i++)
    {
      game.columns.lookup[i] = NULL;
    }
    game.columns.end = NULL;
}

void set_mouse_light(struct PlayerInfo *player)
{
    SYNCDBG(6,"Starting");
    //_DK_set_mouse_light(player);
    struct Packet *pckt;
    pckt = get_packet_direct(player->packet_num);
    if (player->field_460 != 0)
    {
        if ((pckt->control_flags & PCtr_MapCoordsValid) != 0)
        {
            struct Coord3d pos;
            pos.x.val = pckt->pos_x;
            pos.y.val = pckt->pos_y;
            pos.z.val = get_floor_height_at(&pos);
            if (is_my_player(player)) {
                game.pos_14C006 = pos;
            }
            light_turn_light_on(player->field_460);
            light_set_light_position(player->field_460, &pos);
        }
        else
        {
            light_turn_light_off(player->field_460);
        }
    }
}

void check_players_won(void)
{
  SYNCDBG(8,"Starting");

    if (!(game.system_flags & GSF_NetworkActive))
        return;

    unsigned int playerIdx = 0;
    for (; playerIdx < PLAYERS_COUNT; ++playerIdx)
    {
        PlayerInfo* curPlayer = get_player(playerIdx);
        if (!player_exists(curPlayer) || curPlayer->is_active != 1 || curPlayer->victory_state != VicS_Undecided)
            continue;

        // check if any other player is still alive
        for (unsigned int secondPlayerIdx = 0; secondPlayerIdx < PLAYERS_COUNT; ++secondPlayerIdx)
        {
            if (secondPlayerIdx == playerIdx)
                continue;

            PlayerInfo* otherPlayer = get_player(secondPlayerIdx);
            if (player_exists(otherPlayer) && otherPlayer->is_active == 1)
            {
                Thing* heartng = get_player_soul_container(secondPlayerIdx);
                if (heartng->active_state != ObSt_BeingDestroyed)
                    goto continueouterloop;
            }
        }
        break;
    continueouterloop:
        ;
    }
    set_player_as_won_level(&game.players[playerIdx]);
}

void check_players_lost(void)
{
  long i;
  SYNCDBG(8,"Starting");
  //_DK_check_players_lost(); return;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
      struct PlayerInfo *player;
      player = get_player(i);
      if (player_exists(player) && (player->is_active == 1))
      {
          struct Thing *heartng;
          heartng = get_player_soul_container(i);
          if ((!thing_exists(heartng) || (heartng->active_state == ObSt_BeingDestroyed)) && (player->victory_state == VicS_Undecided))
          {
            event_kill_all_players_events(i);
            set_player_as_lost_level(player);
            //this would easily prevent computer player activities on dead player, but it also makes dead player unable to use
            //floating spirit, so it can't be done this way: player->is_active = 0;
            if (is_my_player_number(i)) {
                LbPaletteSet(engine_palette);
            }
          }
      }
  }
}

void blast_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb)) {
        return;
    }
    if (slabmap_owner(slb) != plyr_idx) {
        return;
    }
    struct Thing *doortng;
    doortng = get_door_for_position(slab_subtile_center(slb_x), slab_subtile_center(slb_y));
    if (!thing_is_invalid(doortng)) {
        destroy_door(doortng);
    }
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    if (slbattr->category == SlbAtCtg_FortifiedGround)
    {
      place_slab_type_on_map(10, slab_subtile_center(slb_x), slab_subtile_center(slb_y), game.neutral_player_num, 1);
      decrease_dungeon_area(plyr_idx, 1);
      do_unprettying(game.neutral_player_num, slb_x, slb_y);
      do_slab_efficiency_alteration(slb_x, slb_y);
      remove_traps_around_subtile(slab_subtile_center(slb_x), slab_subtile_center(slb_y), NULL);
      struct Coord3d pos;
      pos.x.val = subtile_coord_center(slab_subtile_center(slb_x));
      pos.y.val = subtile_coord_center(slab_subtile_center(slb_y));
      pos.z.val = get_floor_height_at(&pos);
      create_effect_element(&pos, TngEff_Unknown10, plyr_idx);
    }
}

void process_dungeon_devastation_effects(void)
{
    SYNCDBG(8,"Starting");
    //_DK_process_dungeon_devastation_effects(); return;
    int plyr_idx;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        struct Dungeon *dungeon;
        dungeon = get_players_num_dungeon(plyr_idx);
        if (dungeon->devastation_turn == 0)
            continue;
        if ((game.play_gameturn & 1) != 0)
            continue;
        dungeon->devastation_turn++;
        if (dungeon->devastation_turn >= max(map_tiles_x,map_tiles_y))
            continue;
        MapSlabCoord slb_x;
        MapSlabCoord slb_y;
        int i;
        int range;
        slb_x = subtile_slab(dungeon->devastation_centr_x) - dungeon->devastation_turn;
        slb_y = subtile_slab(dungeon->devastation_centr_y) - dungeon->devastation_turn;
        range = 2*dungeon->devastation_turn;
        for (i = 0; i <= range; i++)
        {
            blast_slab(slb_x + i, slb_y,         dungeon->owner);
            blast_slab(slb_x + i, slb_y + range, dungeon->owner);
        }
        for (i = 0; i <= range; i++)
        {
            blast_slab(slb_x,         slb_y + i, dungeon->owner);
            blast_slab(slb_x + range, slb_y + i, dungeon->owner);
        }
    }
}

void count_players_creatures_being_paid(int *creatures_count)
{
    unsigned long k;
    long i;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
    k = 0;
    while (i != 0)
    {
        struct Thing *thing;
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if ((thing->owner != game.hero_player_num) && (thing->owner != game.neutral_player_num))
        {
            struct CreatureStats *crstat;
            crstat = creature_stats_get_from_thing(thing);
            if (crstat->pay != 0)
            {
                struct CreatureControl *cctrl;
                cctrl = creature_control_get_from_thing(thing);
                if (cctrl->prepayments_received > 0)
                {
                    cctrl->prepayments_received--;
                } else
                {
                    cctrl->paydays_owed++;
                    creatures_count[thing->owner]++;
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

void process_payday(void)
{
    //_DK_process_payday();
    game.pay_day_progress = game.pay_day_progress + (gameadd.pay_day_speed / 100);
    PlayerNumber plyr_idx;
    for (plyr_idx=0; plyr_idx < PLAYERS_COUNT; plyr_idx++)
    {
        if ((plyr_idx == game.hero_player_num) || (plyr_idx == game.neutral_player_num)) {
            continue;
        }
        struct PlayerInfo *player;
        player = get_player(plyr_idx);
        if (player_exists(player) && (player->is_active == 1))
        {
            compute_and_update_player_payday_total(plyr_idx);
        }
    }
    if (game.pay_day_gap <= game.pay_day_progress)
    {
        output_message(SMsg_Payday, 0, true);
        game.pay_day_progress = 0;
        // Prepare a list which counts how many creatures of each owner needs pay
        int player_paid_creatures_count[PLAYERS_EXT_COUNT];

        for (plyr_idx=0; plyr_idx < PLAYERS_EXT_COUNT; plyr_idx++)
        {
            player_paid_creatures_count[plyr_idx] = 0;
        }
        count_players_creatures_being_paid(player_paid_creatures_count);
        // Players which have creatures being paid, should get payday notification
        for (plyr_idx=0; plyr_idx < PLAYERS_EXT_COUNT; plyr_idx++)
        {
            if (player_paid_creatures_count[plyr_idx] > 0)
            {
                struct Dungeon *dungeon;
                dungeon = get_players_num_dungeon(plyr_idx);
                event_create_event_or_update_nearby_existing_event(0, 0,
                    EvKind_CreaturePayday, plyr_idx, dungeon->creatures_total_pay);
            }
        }
    }
}

void count_dungeon_stuff(void)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  int i;

  game.field_14E4A4 = 0;
  game.field_14E4A0 = 0;
  game.field_14E49E = 0;

  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = get_dungeon(i);
    player = get_player(i);
    if (player_exists(player))
    {
      game.field_14E4A0 += dungeon->total_money_owned;
      game.field_14E4A4 += dungeon->num_active_diggers;
      game.field_14E49E += dungeon->num_active_creatrs;
    }
  }
}

void process_dungeons(void)
{
  SYNCDBG(7,"Starting");
  check_players_won();
  check_players_lost();
  process_dungeon_power_magic();
  count_dungeon_stuff();
  process_dungeon_devastation_effects();
  process_entrance_generation();
  process_payday();
  process_things_in_dungeon_hand();
  SYNCDBG(9,"Finished");
}

void process_level_script(void)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo *player;
  player = get_my_player();
  // Do NOT stop executing scripts after winning if the RUN_AFTER_VICTORY(1) script command has been issued
  if ((player->victory_state == VicS_Undecided) || (game.system_flags & GSF_RunAfterVictory))
  {
      process_conditions();
      process_check_new_creature_partys();
    //script_process_messages(); is not here, but it is in beta - check why
      process_check_new_tunneller_partys();
      process_values();
      process_win_and_lose_conditions(my_player_number); //player->id_number may be uninitialized yet
    //  show_onscreen_msg(8, "Flags %d %d %d %d %d %d", game.dungeon[0].script_flags[0],game.dungeon[0].script_flags[1],
    //    game.dungeon[0].script_flags[2],game.dungeon[0].script_flags[3],game.dungeon[0].script_flags[4],game.dungeon[0].script_flags[5]);
  }
  SYNCDBG(19,"Finished");
}

void update_player_camera_fp(struct Camera *cam, struct Thing *thing)
{
    struct CreatureStatsOLD *creature_stats_OLD = &game.creature_stats_OLD[thing->model];
    struct CreatureStats* crstat = creature_stats_get_from_thing(thing);
    struct CreatureControl *cctrl = creature_control_get_from_thing(thing);
    creature_stats_OLD->eye_height = crstat->eye_height + (crstat->eye_height * crtr_conf.exp.size_increase_on_exp * cctrl->explevel) / 100;
    _DK_update_player_camera_fp(cam, thing);
}

void view_move_camera_left(struct Camera *cam, long a2)
{
    _DK_view_move_camera_left(cam, a2); return;
}

void view_move_camera_right(struct Camera *cam, long a2)
{
    _DK_view_move_camera_right(cam, a2); return;
}

void view_move_camera_up(struct Camera *cam, long a2)
{
    _DK_view_move_camera_up(cam, a2); return;
}

void view_move_camera_down(struct Camera *cam, long a2)
{
    _DK_view_move_camera_down(cam, a2); return;
}

void view_process_camera_inertia(struct Camera *cam)
{
    int i;
    i = cam->field_20;
    if (i > 0) {
        view_move_camera_right(cam, abs(i));
    } else
    if (i < 0) {
        view_move_camera_left(cam, abs(i));
    }
    if ( cam->field_24 ) {
        cam->field_24 = 0;
    } else {
        cam->field_20 /= 2;
    }
    i = cam->field_25;
    if (i > 0) {
        view_move_camera_down(cam, abs(i));
    } else
    if (i < 0) {
        view_move_camera_up(cam, abs(i));
    }
    if (cam->field_29) {
        cam->field_29 = 0;
    } else {
        cam->field_25 /= 2;
    }
    if (cam->field_1B) {
        cam->orient_a = (cam->field_1B + cam->orient_a) & LbFPMath_AngleMask;
    }
    if (cam->field_1F) {
        cam->field_1F = 0;
    } else {
        cam->field_1B /= 2;
    }
}

void update_player_camera(struct PlayerInfo *player)
{
    //_DK_update_player_camera(player);
    struct Dungeon *dungeon;
    dungeon = get_players_dungeon(player);
    struct Camera *cam;
    cam = player->acamera;
    view_process_camera_inertia(cam);
    switch (cam->view_mode)
    {
    case PVM_CreatureView:
        if (player->controlled_thing_idx > 0) {
            struct Thing *ctrltng;
            ctrltng = thing_get(player->controlled_thing_idx);
            update_player_camera_fp(cam, ctrltng);
        } else
        if (player->instance_num != PI_HeartZoom) {
            ERRORLOG("Cannot go first person without controlling creature");
        }
        break;
    case PVM_IsometricView:
        player->cameras[CamIV_FrontView].mappos.x.val = cam->mappos.x.val;
        player->cameras[CamIV_FrontView].mappos.y.val = cam->mappos.y.val;
        break;
    case PVM_FrontView:
        player->cameras[CamIV_Isometric].mappos.x.val = cam->mappos.x.val;
        player->cameras[CamIV_Isometric].mappos.y.val = cam->mappos.y.val;
        break;
    }
    if (dungeon->camera_deviate_quake) {
        dungeon->camera_deviate_quake--;
    }
    if (dungeon->camera_deviate_jump > 0) {
        dungeon->camera_deviate_jump -= 32;
    }
}

void update_research(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->is_active == 1))
      {
          process_player_research(i);
      }
  }
}

void update_manufacturing(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(16,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->is_active == 1))
      {
          process_player_manufacturing(i);
      }
  }
}

void update_all_players_cameras(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player = get_player(i);
    if (player_exists(player) && ((player->allocflags & PlaF_CompCtrl) == 0))
    {
          update_player_camera(player);
    }
  }
}

void update_flames_nearest_camera(struct Camera *camera)
{
  if (camera == NULL)
    return;
  _DK_update_flames_nearest_camera(camera);
}

void update_near_creatures_for_footsteps(long *near_creatures, const struct Coord3d *srcpos)
{
    long near_distance[3];
    // Don't allow creatures which are far by over 20 subtiles
    near_distance[0] = subtile_coord(20,0);
    near_distance[1] = subtile_coord(20,0);
    near_distance[2] = subtile_coord(20,0);
    near_creatures[0] = 0;
    near_creatures[1] = 0;
    near_creatures[2] = 0;
    // Find the closest thing for footsteps
    struct Thing *thing;
    unsigned long k;
    long i;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
    k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_of_class;
        // Per-thing code
        thing->state_flags &= ~TF1_DoFootsteps;
        if (!thing_is_picked_up(thing))
        {
            struct CreatureSound *crsound;
            crsound = get_creature_sound(thing, CrSnd_Foot);
            if (crsound->index > 0)
            {
                struct CreatureControl *cctrl;
                cctrl = creature_control_get_from_thing(thing);
                long ndist;
                ndist = get_2d_box_distance(srcpos, &thing->mappos);
                if (ndist < near_distance[0])
                {
                    if (((cctrl->field_9 != 0) && ((int)thing->field_60 >= (int)thing->mappos.z.val))
                      || ((thing->movement_flags & TMvF_Flying) != 0))
                    {
                        // Insert the new item to our list
                        int n;
                        for (n = 2; n>0; n--)
                        {
                            near_creatures[n] = near_creatures[n-1];
                            near_distance[n] = near_distance[n-1];
                        }
                        near_distance[0] = ndist;
                        near_creatures[0] = thing->index;
                    }
                }
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

long stop_playing_flight_sample_in_all_flying_creatures(void)
{
    struct Thing *thing;
    unsigned long k;
    long i;
    long naffected;
    naffected = 0;
    const struct StructureList *slist;
    slist = get_list_for_thing_class(TCls_Creature);
    i = slist->index;
    k = 0;
    while (i != 0)
    {
        thing = thing_get(i);
        if (thing_is_invalid(thing))
        {
          ERRORLOG("Jump to invalid thing detected");
          break;
        }
        i = thing->next_of_class;
        // Per-thing code
        if ((get_creature_model_flags(thing) & CMF_IsDiptera) && ((thing->state_flags & TF1_DoFootsteps) == 0))
        {
            if ( S3DEmitterIsPlayingSample(thing->snd_emitter_id, 25, 0) ) {
                S3DDeleteSampleFromEmitter(thing->snd_emitter_id, 25, 0);
            }
        }
        // Per-thing code ends
        k++;
        if (k > THINGS_COUNT)
        {
          ERRORLOG("Infinite loop detected when sweeping things list");
          break;
        }
    }
    return naffected;
}

void update_footsteps_nearest_camera(struct Camera *cam)
{
    static long timeslice = 0;
    static long near_creatures[3];
    struct Coord3d srcpos;
    SYNCDBG(6,"Starting");
    if (cam == NULL)
        return;
    //_DK_update_footsteps_nearest_camera(camera);
    srcpos.x.val = cam->mappos.x.val;
    srcpos.y.val = cam->mappos.y.val;
    srcpos.z.val = cam->mappos.z.val;
    if (timeslice == 0) {
        update_near_creatures_for_footsteps(near_creatures, &srcpos);
    }
    long i;
    for (i=0; i < 3; i++)
    {
        struct Thing *thing;
        if (near_creatures[i] == 0)
            break;
        thing = thing_get(near_creatures[i]);
        if (thing_is_creature(thing)) {
            thing->state_flags |= TF1_DoFootsteps;
            play_thing_walking(thing);
        }
    }
    if (timeslice == 0)
    {
        stop_playing_flight_sample_in_all_flying_creatures();
    }
    timeslice = (timeslice + 1) % 4;
}

void add_creature_to_pool(long kind, long amount, unsigned long a3)
{
    long prev_amount;
    kind %= CREATURE_TYPES_COUNT;
    prev_amount = game.pool.crtr_kind[kind];
    if ((a3 == 0) || (prev_amount != -1))
    {
        if ((amount != -1) && (amount != 0) && (prev_amount != -1))
            game.pool.crtr_kind[kind] = prev_amount + amount;
        else
            game.pool.crtr_kind[kind] = amount;
    }
}

short update_creature_pool_state(void)
{
  int i;
  game.pool.is_empty = true;
  for (i=1; i < CREATURE_TYPES_COUNT; i++)
  {
      if (game.pool.crtr_kind[i] > 0)
      { game.pool.is_empty = false; break; }
  }
  return true;
}

int clear_active_dungeons_stats(void)
{
  struct Dungeon *dungeon;
  int i;
  for (i=0; i <= game.hero_player_num; i++)
  {
      dungeon = get_dungeon(i);
      if (dungeon_invalid(dungeon))
          break;
      memset((char *)dungeon->field_64, 0, CREATURE_TYPES_COUNT * 15 * sizeof(unsigned short));
      memset((char *)dungeon->guijob_all_creatrs_count, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
      memset((char *)dungeon->guijob_angry_creatrs_count, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
  }
  return i;
}

TbBool setup_move_out_of_cave_in(struct Thing *thing)
{
    // return _DK_setup_move_out_of_cave_in(thing);
    MapSlabCoord bx = 0;
    MapSlabCoord by = 0;
    MapSubtlCoord cx = 0;
    MapSubtlCoord cy = 0;
    struct Thing *tng;
    MapOffset *sstep;
    struct Map* blk;
    if (setup_combat_flee_position(thing))
    {
        struct CreatureControl* cctrl;
        cctrl = creature_control_get_from_thing(thing);
        if ( setup_person_move_to_coord(thing, &cctrl->flee_pos, 0) )
        {
            return true;
        }
    }
    else
    {
        MapSlabCoord slb_x = subtile_slab(thing->mappos.x.stl.num);
        MapSlabCoord slb_y = subtile_slab(thing->mappos.y.stl.num);
        for (signed int i=0; i < 32; i++)
        {
            sstep = &spiral_step[i];
            bx = sstep->h + slb_x;
            by = sstep->v + slb_y;
            struct SlabMap *slb;
            slb = get_slabmap_block(bx, by);
            if ( slabmap_block_invalid(slb) )
            {
                continue;
            }
            blk = get_map_block_at(slab_subtile(bx, 0), slab_subtile(by, 0));
            long n = get_mapwho_thing_index(blk);
            while ( n != 0 )
            {
                tng = thing_get(n);
                TRACE_THING(tng);
                if ( tng->class_id == TCls_EffectElem && tng->model == 46 )
                {
                    break;
                }
                n = tng->next_on_mapblk;
                if (thing_is_invalid(tng))
                {
                    bx = sstep->h + slb_x;
                    break;
                }
            }
            bx = sstep->h + slb_x;
            cx = slab_subtile_center(bx);
            cy = slab_subtile_center(by);
            long j = ACTION_RANDOM(AROUND_TILES_COUNT);
            for (long k=0; k < AROUND_TILES_COUNT; k++, j=(j + 1) % AROUND_TILES_COUNT)
            {
                MapSubtlCoord stl_x = cx + around[j].delta_x;
                MapSubtlCoord stl_y = cy + around[j].delta_y;
                struct Map *mapblk = get_map_block_at(stl_x,stl_y);
                if (!map_block_invalid(mapblk))
                {
                    if (subtile_is_blocking_wall_or_lava(stl_x, stl_y, thing->owner) == 0)
                    {
                        if (setup_person_move_to_position(thing, stl_x, stl_y, 0)) 
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }
    return false;
}

TngUpdateRet damage_creatures_with_physical_force(struct Thing *thing, ModTngFilterParam param)
{
    SYNCDBG(18,"Starting for %s index %d",thing_model_name(thing),(int)thing->index);
    if (thing_is_picked_up(thing) || thing_is_dragged_or_pulled(thing))
    {
        return TUFRet_Unchanged;
    }
    if (thing_is_creature(thing))
    {
        apply_damage_to_thing_and_display_health(thing, param->num2, DmgT_Physical, param->num1);
        if (thing->health >= 0)
        {
            if ((thing->alloc_flags & TAlF_IsControlled) == 0)
            {
                if (get_creature_state_besides_interruptions(thing) != CrSt_CreatureEscapingDeath)
                {
                    if (cleanup_current_thing_state(thing) && setup_move_out_of_cave_in(thing))
                        thing->continue_state = CrSt_CreatureEscapingDeath;
                }
            }
            return TUFRet_Modified;
        } else
        {
            kill_creature(thing, INVALID_THING, param->num1, CrDed_NoEffects|CrDed_DiedInBattle);
            return TUFRet_Deleted;
        }
    }
    return TUFRet_Unchanged;
}

TbBool valid_cave_in_position(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x,stl_y);
    if ((mapblk->flags & SlbAtFlg_Blocking) != 0)
        return false;
    struct SlabMap *slb;
    slb = get_slabmap_for_subtile(stl_x,stl_y);
    return (plyr_idx == game.neutral_player_num) || (slabmap_owner(slb) == game.neutral_player_num) || (slabmap_owner(slb) == plyr_idx);
}

long update_cave_in(struct Thing *thing)
{
    //return _DK_update_cave_in(thing);
    thing->health--;
    thing->field_4F |= TF4F_Unknown01;
    if (thing->health < 1)
    {
        delete_thing_structure(thing, 0);
        return 1;
    }

    const struct MagicStats *pwrdynst;
    pwrdynst = get_power_dynamic_stats(PwrK_CAVEIN);
    struct Thing *efftng;
    struct Coord3d pos;
    PlayerNumber owner;
    owner = thing->owner;
    if ((game.play_gameturn % 3) == 0)
    {
        int n;
        n = ACTION_RANDOM(AROUND_TILES_COUNT);
        pos.x.val = thing->mappos.x.val + ACTION_RANDOM(0x2C0) * around[n].delta_x;
        pos.y.val = thing->mappos.y.val + ACTION_RANDOM(0x2C0) * around[n].delta_y;
        if (subtile_has_slab(coord_subtile(pos.x.val),coord_subtile(pos.y.val)))
        {
            pos.z.val = get_ceiling_height(&pos) - 128;
            efftng = create_effect_element(&pos, TngEff_Unknown48, owner);
            if (!thing_is_invalid(efftng)) {
                efftng->health = pwrdynst->time;
            }
        }
    }

    GameTurnDelta turns_between;
    GameTurnDelta turns_alive;
    turns_between = pwrdynst->time / 5;
    turns_alive = game.play_gameturn - thing->creation_turn;
    if ((turns_alive != 0) && ((turns_between < 1) || (3 * turns_between / 4 == turns_alive % turns_between)))
    {
        pos.x.val = thing->mappos.x.val + ACTION_RANDOM(128);
        pos.y.val = thing->mappos.y.val + ACTION_RANDOM(128);
        pos.z.val = get_floor_height_at(&pos) + 384;
        create_effect(&pos, TngEff_Unknown31, owner);
    }

    if ((turns_alive % game.turns_per_collapse_dngn_dmg) == 0)
    {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = subtile_coord(1,0);
        Thing_Modifier_Func do_cb;
        struct CompoundTngFilterParam param;
        param.plyr_idx = -1;
        param.class_id = 0;
        param.model_id = 0;
        param.num1 = thing->owner;
        param.num2 = game.collapse_dungeon_damage;
        param.ptr3 = 0;
        do_cb = damage_creatures_with_physical_force;
        do_to_things_with_param_around_map_block(&pos, do_cb, &param);
    }

    if ((8 * pwrdynst->time / 10 >= thing->health) && (2 * pwrdynst->time / 10 <= thing->health))
    {
        if ((pwrdynst->time < 10) || ((thing->health % (pwrdynst->time / 10)) == 0))
        {
            int round_idx;
            round_idx = ACTION_RANDOM(AROUND_TILES_COUNT);
            set_coords_to_slab_center(&pos, subtile_slab(thing->mappos.x.val + 3 * around[round_idx].delta_x), subtile_slab(thing->mappos.y.val + 3 * around[round_idx].delta_y));
            if (subtile_has_slab(coord_subtile(pos.x.val), coord_subtile(pos.y.val)) && valid_cave_in_position(thing->owner, coord_subtile(pos.x.val), coord_subtile(pos.y.val)))
            {
                struct Thing *ncavitng;
                ncavitng = get_cavein_at_subtile_owned_by(coord_subtile(pos.x.val), coord_subtile(pos.y.val), -1);
                if (thing_is_invalid(ncavitng))
                {
                    long dist;
                    struct Coord3d pos2;
                    pos2.x.val = subtile_coord(thing->byte_13,0);
                    pos2.y.val = subtile_coord(thing->byte_14,0);
                    pos2.z.val = subtile_coord(1,0);
                    dist = get_2d_box_distance(&pos, &pos2);
                    if (pwrdynst->strength[thing->byte_17] >= coord_subtile(dist))
                    {
                        ncavitng = create_thing(&pos, TCls_CaveIn, thing->byte_17, owner, -1);
                        if (!thing_is_invalid(ncavitng))
                        {
                            thing->health += 5;
                            if (thing->health > 0)
                            {
                                ncavitng->byte_13 = thing->byte_13;
                                ncavitng->byte_14 = thing->byte_14;
                            }
                        }
                    }
                }
            }
        }
    }
    return 1;
}

void update(void)
{
    struct PlayerInfo *player;
    SYNCDBG(4,"Starting for turn %ld",(long)game.play_gameturn);

    if ((game.operation_flags & GOF_Paused) == 0)
        update_light_render_area();
    process_packets();
    if (quit_game || exit_keeper) {
        return;
    }
    if (game.game_kind == GKind_Unknown1)
    {
        game.field_14EA4B = 0;
        return;
    }

    if ((game.operation_flags & GOF_Paused) == 0)
    {
        player = get_my_player();
        if (player->field_3 & Pf3F_Unkn08)
        {
            PaletteSetPlayerPalette(player, engine_palette);
            set_flag_byte(&player->field_3,0x08,false);
        }
        clear_active_dungeons_stats();
        update_creature_pool_state();
        if ((game.play_gameturn & 0x01) != 0)
            update_animating_texture_maps();
        update_things();
        process_rooms();
        process_dungeons();
        update_research();
        update_manufacturing();
        event_process_events();
        update_all_events();
        process_level_script();
        if ((game.numfield_D & GNFldD_Unkn04) != 0)
            process_computer_players2();
        process_players();
        process_action_points();
        player = get_my_player();
        if (player->view_mode == PVM_CreatureView)
            update_flames_nearest_camera(player->acamera);
        update_footsteps_nearest_camera(player->acamera);
        PaletteFadePlayer(player);
        process_armageddon();
#if (BFDEBUG_LEVEL > 9)
        lights_stats_debug_dump();
        things_stats_debug_dump();
        creature_stats_debug_dump();
#endif
    }

    message_update();
    update_all_players_cameras();
    update_player_sounds();
    game.field_14EA4B = 0;
    SYNCDBG(6,"Finished");
}

long near_map_block_thing_filter_queryable_object(const struct Thing *thing, MaxTngFilterParam param, long maximizer)
{
/* Currently this only makes Dungeon Heart blinking; maybe I'll find a purpose for it later
    long dist_x,dist_y;
    if ((thing->class_id == TCls_Object) && (thing->model == 5))
    {
      if (thing->owner == param->plyr_idx)
      {
          // note that abs() is not required because we're computing square of the values
          dist_x = param->num1-(MapCoord)thing->mappos.x.val;
          dist_y = param->num2-(MapCoord)thing->mappos.y.val;
          // This function should return max value when the distance is minimal, so:
          return LONG_MAX-(dist_x*dist_x + dist_y*dist_y);
      }
    }
*/
    // If conditions are not met, return -1 to be sure thing will not be returned.
    return -1;
}

struct Thing *get_queryable_object_near(MapCoord pos_x, MapCoord pos_y, long plyr_idx)
{
    Thing_Maximizer_Filter filter;
    struct CompoundTngFilterParam param;
    SYNCDBG(19,"Starting");
    //return _DK_get_queryable_object_near(pos_x, pos_y, plyr_idx);
    filter = near_map_block_thing_filter_queryable_object;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

void tag_cursor_blocks_dig(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long a4)
{
  SYNCDBG(7,"Starting for player %d at subtile (%d,%d)",(int)plyr_idx,(int)stl_x,(int)stl_y);
  _DK_tag_cursor_blocks_dig(plyr_idx, stl_x, stl_y, a4);
}

void tag_cursor_blocks_thing_in_hand(unsigned char a1, long a2, long a3, int a4, long a5)
{
  SYNCDBG(7,"Starting");
  _DK_tag_cursor_blocks_thing_in_hand(a1, a2, a3, a4, a5);
}

void set_player_cameras_position(struct PlayerInfo *player, long pos_x, long pos_y)
{
    player->cameras[CamIV_Parchment].mappos.x.val = pos_x;
    player->cameras[CamIV_FrontView].mappos.x.val = pos_x;
    player->cameras[CamIV_Isometric].mappos.x.val = pos_x;
    player->cameras[CamIV_Parchment].mappos.y.val = pos_y;
    player->cameras[CamIV_FrontView].mappos.y.val = pos_y;
    player->cameras[CamIV_Isometric].mappos.y.val = pos_y;
}

void scale_tmap2(long a1, long flags, long a3, long a4x, long a4y, long a6x, long a6y)
{
    if ((a6x == 0) || (a6y == 0)) {
        return;
    }
    long xstart;
    long ystart;
    long xend;
    long yend;
    char orient;
    switch (flags)
    {
    case 0:
        xstart = 0;
        ystart = 0;
        xend = 2097151 / a6x;
        yend = 2097151 / a6y;
        orient = 0;
        break;
    case 0x10:
        xstart = 2097151;
        ystart = 0;
        xend = -2097151 / a6x;
        yend = 2097151 / a6y;
        orient = 0;
        break;
    case 0x20:
        xstart = 0;
        ystart = 2097151;
        xend = 2097151 / a6x;
        yend = -2097151 / a6y;
        orient = 0;
        break;
    case 0x30:
        xstart = 2097151;
        ystart = 2097151;
        xend = -2097151 / a6x;
        yend = -2097151 / a6y;
        orient = 0;
        break;
    case 0x40:
        ystart = 0;
        xstart = 0;
        yend = 2097151 / a6y;
        xend = 2097151 / a6x;
        orient = 1;
        break;
    case 0x50:
        ystart = 0;
        xstart = 2097151;
        yend = 2097151 / a6y;
        xend = -2097151 / a6x;
        orient = 1;
        break;
    case 0x60:
        ystart = 2097151;
        xstart = 0;
        yend = -2097151 / a6y;
        xend = 2097151 / a6x;
        orient = 1;
        break;
    case 0x70:
        xstart = 2097151;
        ystart = 2097151;
        yend = -2097151 / a6y;
        xend = -2097151 / a6x;
        orient = 1;
        break;
    default:
          return;
    }
    long v10;
    long v12;
    v10 = a4x;
    if (v10 < 0)
    {
        a6x += a4x;
        if (a6x < 0) {
            return;
        }
        xstart -= xend * a4x;
        v10 = 0;
    }
    if (v10 + a6x > vec_window_width)
    {
        a6x = vec_window_width - v10;
        if (a6x < 0) {
            return;
        }
    }
    v12 = a4y;
    if (v12 < 0)
    {
        a6y += a4y;
        if (a6y < 0) {
            return;
        }
        ystart -= a4y * yend;
        v12 = 0;
    }
    if (v12 + a6y > vec_window_height)
    {
        a6y = vec_window_height - v12;
        if (a6y < 0) {
            return;
        }
    }
    int i;
    long hlimits[480];
    long wlimits[640];
    long *xlim;
    long *ylim;
    unsigned char *dbuf;
    unsigned char *block;
    if (!orient)
    {
        xlim = wlimits;
        for (i = a6x; i > 0; i--)
        {
            *xlim = xstart;
            xlim++;
            xstart += xend;
        }
        ylim = hlimits;
        for (i = a6y; i > 0; i--)
        {
            *ylim = ystart;
            ylim++;
            ystart += yend;
        }
        dbuf = &vec_screen[v10 + v12 * vec_screen_width];
        block = block_ptrs[a1];
        ylim = hlimits;
        long px;
        long py;
        int srcx;
        int srcy;
        unsigned char *d;
        if ( a3 >= 0 )
        {
          for (py = a6y; py > 0; py--)
          {
              xlim = wlimits;
              d = dbuf;
              srcy = (((*ylim) & 0xFF0000u) >> 16);
              for (px = a6x; px > 0; px--)
              {
                srcx = (((*xlim) & 0xFF0000u) >> 16);
                xlim++;
                *d = pixmap.fade_tables[256 * a3 + block[(srcy << 8) + srcx]];
                ++d;
              }
              dbuf += vec_screen_width;
              ylim++;
          }
        } else
        {
          for (py = a6y; py > 0; py--)
          {
            xlim = wlimits;
            d = dbuf;
            srcy = (((*ylim) & 0xFF0000u) >> 16);
            for (px = a6x; px > 0; px--)
            {
              srcx = (((*xlim) & 0xFF0000u) >> 16);
              xlim++;
              *d = block[(srcy << 8) + srcx];
              ++d;
            }
            dbuf += vec_screen_width;
            ylim++;
          }
        }
    } else
    {
        ylim = wlimits;
        for (i = a6y; i > 0; i--)
        {
          *ylim = ystart;
          ylim++;
          ystart += yend;
        }
        xlim = hlimits;
        for (i = a6x; i > 0; i--)
        {
          *xlim = xstart;
          xlim++;
          xstart += xend;
        }
        dbuf = &vec_screen[v10 + v12 * vec_screen_width];
        block = block_ptrs[a1];
        ylim = wlimits;
        long px;
        long py;
        int srcx;
        int srcy;
        unsigned char *d;
        if ( a3 >= 0 )
        {
          for (py = a6y; py > 0; py--)
          {
              xlim = hlimits;
              d = dbuf;
              srcy = (((*ylim) & 0xFF0000u) >> 16);
              for (px = a6x; px > 0; px--)
              {
                srcx = (((*xlim) & 0xFF0000u) >> 16);
                xlim++;
                *d = pixmap.fade_tables[256 * a3 + block[(srcx << 8) + srcy]];
                ++d;
              }
              dbuf += vec_screen_width;
              ylim++;
          }
        } else
        {
          for (py = a6y; py > 0; py--)
          {
            xlim = hlimits;
            d = dbuf;
            srcy = (((*ylim) & 0xFF0000u) >> 16);
            for (px = a6x; px > 0; px--)
            {
              srcx = (((*xlim) & 0xFF0000u) >> 16);
              xlim++;
              *d = block[(srcx << 8) + srcy];
              ++d;
            }
            dbuf += vec_screen_width;
            ylim++;
          }
        }
    }
}

void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7)
{
    //_DK_draw_texture(a1, a2, a3, a4, a5, a6, a7);
    scale_tmap2(a5, a6, a7, a1 / pixel_size, a2 / pixel_size, a3 / pixel_size, a4 / pixel_size);
}

void update_block_pointed(int i,long x, long x_frac, long y, long y_frac)
{
    struct Map *mapblk;
    struct Column *colmn;
    short visible;
    unsigned int smask;
    long k;

    if (i > 0)
    {
      mapblk = get_map_block_at(x,y);
      visible = map_block_revealed_bit(mapblk, player_bit);
      if ((!visible) || ((mapblk->data & 0x7FF) > 0))
      {
        if (visible)
          k = mapblk->data & 0x7FF;
        else
          k = game.unrevealed_column_idx;
        colmn = get_column(k);
        smask = colmn->solidmask;
        if ((temp_cluedo_mode) && (smask != 0))
        {
          if (visible)
            k = mapblk->data & 0x7FF;
          else
            k = game.unrevealed_column_idx;
          colmn = get_column(k);
          if (colmn->solidmask >= 8)
          {
            if ( (!visible) || (((get_navigation_map(x,y) & 0x80) == 0) && ((mapblk->flags & SlbAtFlg_IsRoom) == 0)) )
              smask &= 3;
          }
        }
        if (smask & (1 << (i-1)))
        {
          pointed_at_frac_x = x_frac;
          pointed_at_frac_y = y_frac;
          block_pointed_at_x = x;
          block_pointed_at_y = y;
          me_pointed_at = mapblk;
        }
        if (((!temp_cluedo_mode) && (i == 5)) || ((temp_cluedo_mode) && (i == 2)))
        {
          top_pointed_at_frac_x = x_frac;
          top_pointed_at_frac_y = y_frac;
          top_pointed_at_x = x;
          top_pointed_at_y = y;
        }
      }
    } else
    {
        mapblk = get_map_block_at(x,y);
        floor_pointed_at_x = x;
        floor_pointed_at_y = y;
        block_pointed_at_x = x;
        block_pointed_at_y = y;
        pointed_at_frac_x = x_frac;
        pointed_at_frac_y = y_frac;
        me_pointed_at = mapblk;
    }
}

void update_blocks_pointed(void)
{
    long x;
    long y;
    long x_frac;
    long y_frac;
    long hori_ptr_y;
    long vert_ptr_y;
    long hori_hdelta_y;
    long vert_hdelta_y;
    long hori_ptr_x;
    long vert_ptr_x;
    long hvdiv_x;
    long hvdiv_y;
    long long lltmp;
    long k;
    int i;
    SYNCDBG(19,"Starting");
    if ((!vert_offset[1]) && (!hori_offset[1]))
    {
        block_pointed_at_x = 0;
        block_pointed_at_y = 0;
        me_pointed_at = INVALID_MAP_BLOCK;//get_map_block_at(0,0);
    } else
    {
        hori_ptr_y = (long)hori_offset[0] * (pointer_y - y_init_off);
        vert_ptr_y = (long)vert_offset[0] * (pointer_y - y_init_off);
        hori_hdelta_y = (long)hori_offset[0] * ((long)high_offset[1] >> 8);
        vert_hdelta_y = (long)vert_offset[0] * ((long)high_offset[1] >> 8);
        vert_ptr_x = (long)(vert_offset[1] * (pointer_x - x_init_off)) >> 1;
        hori_ptr_x = (long)(hori_offset[1] * (pointer_x - x_init_off)) >> 1;
        lltmp = hori_offset[0] * (long long)vert_offset[1] - vert_offset[0] * (long long)hori_offset[1];
        hvdiv_x = (lltmp >> 11);
        if (hvdiv_x == 0) hvdiv_x = 1;
        lltmp = vert_offset[0] * (long long)hori_offset[1] - hori_offset[0] * (long long)vert_offset[1];
        hvdiv_y = (lltmp >> 11);
        if (hvdiv_y == 0) hvdiv_y = 1;
        for (i=0; i < 8; i++)
        {
          k = (vert_ptr_x - (vert_ptr_y >> 1)) / hvdiv_x;
          x_frac = (k & 3) << 6;
          x = k >> 2;
          k = (hori_ptr_x - (hori_ptr_y >> 1)) / hvdiv_y;
          y_frac = (k & 3) << 6;
          y = k >> 2;
          if ((x >= 0) && (x < map_subtiles_x) && (y >= 0) && (y < map_subtiles_y))
          {
              update_block_pointed(i,x,x_frac,y,y_frac);
          }
          hori_ptr_y -= hori_hdelta_y;
          vert_ptr_y -= vert_hdelta_y;
        }
    }
    SYNCDBG(19,"Finished");
}

void engine(struct PlayerInfo *player, struct Camera *cam)
{
    TbGraphicsWindow grwnd;
    TbGraphicsWindow ewnd;
    unsigned short flg_mem;

    SYNCDBG(9,"Starting");
    //_DK_engine(cam); return;

    flg_mem = lbDisplay.DrawFlags;
    update_engine_settings(player);
    mx = cam->mappos.x.val;
    my = cam->mappos.y.val;
    mz = cam->mappos.z.val;
    pointer_x = (GetMouseX() - player->engine_window_x) / pixel_size;
    pointer_y = (GetMouseY() - player->engine_window_y) / pixel_size;
    lens = (cam->field_13 * ((long)MyScreenWidth))/pixel_size / 320;
    if (lens_mode == 0)
        update_blocks_pointed();
    LbScreenStoreGraphicsWindow(&grwnd);
    store_engine_window(&ewnd,pixel_size);
    view_height_over_2 = ewnd.height/2;
    view_width_over_2 = ewnd.width/2;
    LbScreenSetGraphicsWindow(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
    setup_vecs(lbDisplay.GraphicsWindowPtr, 0, lbDisplay.GraphicsScreenWidth,
        ewnd.width, ewnd.height);
    camera_zoom = scale_camera_zoom_to_screen(cam->zoom);
    draw_view(cam, 0);
    lbDisplay.DrawFlags = flg_mem;
    thing_being_displayed = 0;
    LbScreenLoadGraphicsWindow(&grwnd);
}

void find_frame_rate(void)
{
    static TbClockMSec prev_time2=0;
    static TbClockMSec cntr_time2=0;
    unsigned long curr_time;
    curr_time = LbTimerClock();
    cntr_time2++;
    if (curr_time-prev_time2 >= 1000)
    {
        double time_fdelta = 1000.0*((double)(cntr_time2))/(curr_time-prev_time2);
        prev_time2 = curr_time;
        game.time_delta = (unsigned long)(time_fdelta*256.0);
        cntr_time2 = 0;
    }
}

void packet_load_find_frame_rate(unsigned long incr)
{
    static TbClockMSec start_time=0;
    static TbClockMSec extra_frames=0;
    TbClockMSec curr_time;
    curr_time = LbTimerClock();
    if ((curr_time-start_time) < 5000)
    {
        extra_frames += incr;
    } else
    {
        double time_fdelta = 1000.0*((double)(extra_frames+incr))/(curr_time-start_time);
        start_time = curr_time;
        game.time_delta = (unsigned long)(time_fdelta*256.0);
        extra_frames = 0;
    }
}

/**
 * Checks if the game screen needs redrawing.
 */
short display_should_be_updated_this_turn(void)
{
    if ((game.operation_flags & GOF_Paused) != 0)
      return true;
    if ( (game.turns_fastforward == 0) && (!game.numfield_149F38) )
    {
      find_frame_rate();
      if ( (game.frame_skip == 0) || ((game.play_gameturn % game.frame_skip) == 0))
        return true;
    } else
    if ( ((game.play_gameturn & 0x3F)==0) ||
         ((game.numfield_149F38) && ((game.play_gameturn & 7)==0)) )
    {
      packet_load_find_frame_rate(64);
      return true;
    }
    return false;
}

/**
 * Makes last updates to the video buffer, and swaps buffers to show
 * the new image.
 */
TbBool keeper_screen_swap(void)
{
/*  // For resolution 640x480, move the graphics data 40 lines lower
  if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
    if (LbScreenLock() == Lb_SUCCESS)
    {
      int i;
      int scrmove_x=0;
      int scrmove_y=40;
      int scanline_len=640;
      for (i=400;i>=0;i--)
        memcpy(lbDisplay.WScreen+scanline_len*(i+scrmove_y)+scrmove_x, lbDisplay.WScreen+scanline_len*i, scanline_len-scrmove_x);
      memset(lbDisplay.WScreen, 0, scanline_len*scrmove_y);
      LbScreenUnlock();
    }*/
  LbScreenSwap();
  return true;
}

/**
 * Waits until the next game turn. Delay is usually controlled by
 * num_fps variable.
 */
TbBool keeper_wait_for_next_turn(void)
{
    if ((game.numfield_D & GNFldD_Unkn10) != 0)
    {
        // No idea when such situation occurs
        TbClockMSec sleep_end = last_loop_time + 1000;
        LbSleepUntil(sleep_end);
        last_loop_time = LbTimerClock();
        return true;
    }
    if (game.frame_skip == 0)
    {
        // Standard delaying system
        TbClockMSec sleep_end = last_loop_time + 1000/game.num_fps;
        LbSleepUntil(sleep_end);
        last_loop_time = LbTimerClock();
        return true;
    }
    return false;
}

TbBool keeper_wait_for_screen_focus(void)
{
    do {
        if ( !LbWindowsControl() )
        {
          if ((game.system_flags & GSF_NetworkActive) == 0)
          {
            exit_keeper = 1;
            break;
          }
          SYNCLOG("Alex's point reached");
        }
        if (LbIsActive())
          return true;
        if ((game.system_flags & GSF_NetworkActive) != 0)
          return true;
        LbSleepFor(50);
    } while ((!exit_keeper) && (!quit_game));
    return false;
}

void keeper_gameplay_loop(void)
{
    short do_draw;
    struct PlayerInfo *player;
    SYNCDBG(5,"Starting");
    player = get_my_player();
    PaletteSetPlayerPalette(player, engine_palette);
    if ((game.operation_flags & GOF_SingleLevel) != 0)
        initialise_eye_lenses();

#ifdef AUTOTESTING
    if ((start_params.autotest_flags & ATF_AI_Player) != 0)
    {
        toggle_computer_player(player->id_number);
    }
#endif

    SYNCDBG(0,"Entering the gameplay loop for level %d",(int)get_loaded_level_number());

    KeeperSpeechClearEvents();
    LbErrorParachuteUpdate(); // For some reasone parachute keeps changing; Remove when won't be needed anymore

    //the main gameplay loop starts
    while ((!quit_game) && (!exit_keeper))
    {
        if ((game.flags_font & FFlg_unk10) != 0)
        {
          if (game.play_gameturn == 4)
              LbNetwork_ChangeExchangeTimeout(0);
        }

#ifdef AUTOTESTING
        if ((start_params.autotest_flags & ATF_ExitOnTurn) && (start_params.autotest_exit_turn == game.play_gameturn))
        {
            quit_game = true;
            exit_keeper = true;
            break;
        }
        evm_stat(1, "turn val=%ld,action_seed=%ld,unsync_seed=%ld", game.play_gameturn, game.action_rand_seed, game.unsync_rand_seed);
        if (start_params.autotest_flags & ATF_FixedSeed)
        {
            game.action_rand_seed = game.play_gameturn;
            game.unsync_rand_seed = game.play_gameturn;
            srand(game.play_gameturn);
        }
#endif
        // Check if we should redraw screen in this turn
        do_draw = display_should_be_updated_this_turn() || (!LbIsActive());

        LbWindowsControl();
        input_eastegg();
        input();
        update();

        if (quit_game || exit_keeper)
            do_draw = false;

        if ( do_draw )
            keeper_screen_redraw();
        keeper_wait_for_screen_focus();
        // Direct information/error messages
        if (LbScreenLock() == Lb_SUCCESS)
        {
            if ( do_draw )
                perform_any_screen_capturing();
            draw_onscreen_direct_messages();
            LbScreenUnlock();
        }

        // Music and sound control
        if ( !SoundDisabled )
        {
            if ( (game.turns_fastforward == 0) && (!game.numfield_149F38) )
            {
                MonitorStreamedSoundTrack();
                process_sound_heap();
            }
        }

        // Move the graphics window to center of screen buffer and swap screen
        if ( do_draw )
            keeper_screen_swap();

        // Make delay if the machine is too fast
        if ( (!game.packet_load_enable) || (game.turns_fastforward == 0) )
            keeper_wait_for_next_turn();
        if (game.turns_packetoff == game.play_gameturn)
            exit_keeper = 1;
    } // end while
    SYNCDBG(0,"Gameplay loop finished after %lu turns",(unsigned long)game.play_gameturn);
}

int can_thing_be_queried(struct Thing *thing, long a2)
{
  return _DK_can_thing_be_queried(thing, a2);
}

TbBool tag_cursor_blocks_sell_area(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long a4, TbBool Subtile)
{
    SYNCDBG(7,"Starting");
    // _DK_tag_cursor_blocks_sell_area(plyr_idx, stl_x, stl_y, a4);
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    int v6 = slab_subtile(slb_x, 0);
    int v7 = slab_subtile(slb_y, 0);
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    signed int parl;
    TbBool allowed = false;
    if (!subtile_revealed(stl_x, stl_y, plyr_idx)
        || ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0))
    {
        parl = temp_cluedo_mode < 1u ? 5 : 2;
    }
    else if (slab_kind_is_liquid(slb->kind))
    {
        parl = 0;
    }
    else
    {
        if ( ( ((subtile_is_sellable_room(plyr_idx, stl_x, stl_y)) || ( (slabmap_owner(slb) == plyr_idx) && ( (slab_is_door(slb_x, slb_y))
            || (Subtile ? (subtile_has_trap_on(stl_x, stl_y)) : (slab_has_trap_on(slb_x, slb_y))) ) ) ) )
            && ( slb->kind != SlbT_ENTRANCE && slb->kind != SlbT_DUNGHEART ) )
        {
            allowed = true;
        }
        parl = 1;
    }
    if ( is_my_player_number(plyr_idx) && !game_is_busy_doing_gui() && game.small_map_state != 2 )
    {
        map_volume_box.visible = 1;
        map_volume_box.beg_x = Subtile ? (subtile_coord(stl_x,0)) : (v6 << 8);
        map_volume_box.beg_y = Subtile ? (subtile_coord(stl_y,0)) : (v7 << 8);
        map_volume_box.field_13 = parl;
        map_volume_box.end_x = Subtile ? (subtile_coord(stl_x+1,0)) : ((v6 + 2 * a4 + 1) << 8);
        map_volume_box.color = allowed;
        map_volume_box.end_y = Subtile ? (subtile_coord(stl_y+1,0)) : ((v7 + 2 * a4 + 1) << 8);
    }
    return allowed;
}

long packet_place_door(MapSubtlCoord stl_x, MapSubtlCoord stl_y, PlayerNumber plyr_idx, ThingModel tngmodel, unsigned char a5)
{
    //return _DK_packet_place_door(a1, a2, a3, a4, a5);
    if (!a5) {
        if (is_my_player_number(plyr_idx))
            play_non_3d_sample(119);
        return 0;
    }
    if (!player_place_door_at(stl_x, stl_y, plyr_idx, tngmodel)) {
        return 0;
    }
    remove_dead_creatures_from_slab(subtile_slab(stl_x), subtile_slab(stl_y));
    return 1;
}

TbBool tag_cursor_blocks_place_door(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    SYNCDBG(7,"Starting");
    // return _DK_tag_cursor_blocks_place_door(a1, a2, a3);
    MapSlabCoord slb_x = subtile_slab_fast(stl_x);
    MapSlabCoord slb_y = subtile_slab_fast(stl_y);
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    signed int parl;
    TbBool allowed = false;
    char Orientation;
    TbBool Check = false;
    if (!subtile_revealed(stl_x, stl_y, plyr_idx) || ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0))
    {
        parl = temp_cluedo_mode < 1u ? 5 : 2;
    }
    else if (slab_kind_is_liquid(slb->kind))
    {
        parl = 0;
    }
    else
    {
        Orientation = find_door_angle(stl_x, stl_y, plyr_idx);
        if (gameadd.place_traps_on_subtiles)
        {
            switch(Orientation)
            {
                case 0:
                {
                    Check = (!slab_middle_row_has_trap_on(slb_x, slb_y) );
                    break;
                }
                case 1:
                {
                    Check = (!slab_middle_column_has_trap_on(slb_x, slb_y) );
                    break;
                }
            }
        }
        if ( ( (slabmap_owner(slb) == plyr_idx) && (slb->kind == SlbT_CLAIMED) )
            && (Orientation != -1)
            && ( ( (gameadd.place_traps_on_subtiles) ? (Check) : (!slab_has_trap_on(slb_x, slb_y) ) ) && (!slab_has_door_thing_on(stl_x, stl_y) ) )
            )
        {
            allowed = true;
        }
        parl = 1;
    }
    if ( is_my_player_number(plyr_idx) && !game_is_busy_doing_gui() && game.small_map_state != 2 )
    {
        map_volume_box.visible = 1;
        map_volume_box.beg_x = subtile_coord(slab_subtile(slb_x, 0), 0);
        map_volume_box.beg_y = subtile_coord(slab_subtile(slb_y, 0), 0);
        map_volume_box.end_x = subtile_coord(slab_subtile(slb_x, 3), 0);
        map_volume_box.end_y = subtile_coord(slab_subtile(slb_y, 3), 0);
        map_volume_box.field_13 = parl;
        map_volume_box.color = allowed;
    }
    return allowed;
}

TbBool tag_cursor_blocks_place_room(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, long a4)
{
    SYNCDBG(7,"Starting");
    //return _DK_tag_cursor_blocks_place_room(plyr_idx, a2, a3, a4);
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    slb_x = subtile_slab_fast(stl_x);
    slb_y = subtile_slab_fast(stl_y);
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    struct SlabAttr *slbattr;
    slbattr = get_slab_attrs(slb);
    struct PlayerInfo *player;
    player = get_player(plyr_idx);
    int par1;
    if (!subtile_revealed(stl_x, stl_y, plyr_idx) ||
       ((slbattr->block_flags & (SlbAtFlg_Filled|SlbAtFlg_Digable|SlbAtFlg_Valuable)) != 0))
    {
        par1 = temp_cluedo_mode < 1u ? 5 : 2;
    } else
    if (slab_kind_is_liquid(slb->kind))
    {
        par1 = 0;
    } else
    {
        par1 = 1;
    }
    TbBool allowed;
    allowed = false;
    if (can_build_room_at_slab(plyr_idx, player->chosen_room_kind, slb_x, slb_y)) {
        allowed = true;
    } else {
        SYNCDBG(7,"Cannot build %s on slab (%d,%d)",slab_code_name(slb->kind),room_code_name(player->chosen_room_kind),(int)slb_x,(int)slb_y);
    }
    if (is_my_player_number(plyr_idx) && !game_is_busy_doing_gui() && (game.small_map_state != 2))
    {
        map_volume_box.visible = 1;
        map_volume_box.beg_x = subtile_coord(slab_subtile(slb_x, 0), 0);
        map_volume_box.beg_y = subtile_coord(slab_subtile(slb_y, 0), 0);
        map_volume_box.field_13 = par1;
        map_volume_box.end_x = subtile_coord(slab_subtile(slb_x, 2*a4+1), 0);
        map_volume_box.color = allowed;
        map_volume_box.end_y = subtile_coord(slab_subtile(slb_y, 2*a4+1), 0);
    }
    return allowed;
}

void initialise_map_collides(void)
{
    SYNCDBG(7,"Starting");
    //_DK_initialise_map_collides(); return;
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    for (slb_y=0; slb_y < map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < map_tiles_x; slb_x++)
        {
            struct SlabMap *slb;
            slb = get_slabmap_block(slb_x, slb_y);
            int ssub_x;
            int ssub_y;
            for (ssub_y=0; ssub_y < STL_PER_SLB; ssub_y++)
            {
                for (ssub_x=0; ssub_x < STL_PER_SLB; ssub_x++)
                {
                    MapSubtlCoord stl_x;
                    MapSubtlCoord stl_y;
                    stl_x = slab_subtile(slb_x,ssub_x);
                    stl_y = slab_subtile(slb_y,ssub_y);
                    struct Map *mapblk;
                    mapblk = get_map_block_at(stl_x, stl_y);
                    mapblk->flags = 0;
                    update_map_collide(slb->kind, stl_x, stl_y);
                }
            }
        }
    }
}

void initialise_map_health(void)
{
    SYNCDBG(7,"Starting");
    //_DK_initialise_map_health();
    MapSlabCoord slb_x;
    MapSlabCoord slb_y;
    for (slb_y=0; slb_y < map_tiles_y; slb_y++)
    {
        for (slb_x=0; slb_x < map_tiles_x; slb_x++)
        {
            struct SlabMap *slb;
            slb = get_slabmap_block(slb_x, slb_y);
            struct SlabAttr *slbattr;
            slbattr = get_slab_attrs(slb);
            slb->health = game.block_health[slbattr->field_4];
        }
    }
}

long ceiling_block_is_solid_including_corners_return_height(long a1, long a2, long a3)
{
    return _DK_ceiling_block_is_solid_including_corners_return_height(a1, a2, a3);
}

long get_ceiling_filled_subtiles_from_cubes(const struct Column *col)
{
    if (col->solidmask == 0) {
        return 0;
    }
    int i;
    for (i = COLUMN_STACK_HEIGHT-1; i >= 0; i--)
    {
        if (col->cubes[i] != 0)
            break;
    }
    return i + 1;
}

int get_ceiling_or_floor_filled_subtiles(int stl_num)
{
    const struct Map *mapblk;
    mapblk = get_map_block_at_pos(stl_num);
    const struct Column *col;
    col = get_map_column(mapblk);
    if (get_map_ceiling_filled_subtiles(mapblk) > 0) {
        return get_ceiling_filled_subtiles_from_cubes(col);
    } else {
        return get_map_floor_filled_subtiles(mapblk);
    }
}
long ceiling_init(unsigned long a1, unsigned long a2)
{
    return _DK_ceiling_init(a1, a2);
    //TODO Fix, then enable rewritten version
    MapSubtlCoord stl_x;
    MapSubtlCoord stl_y;
    for (stl_y=0; stl_y < map_subtiles_y; stl_y++)
    {
        for (stl_x=0; stl_x < map_subtiles_x; stl_x++)
        {
            int filled_h;
            if (map_pos_solid_at_ceiling(stl_x, stl_y))
            {
                filled_h = get_ceiling_or_floor_filled_subtiles(get_subtile_number(stl_x,stl_y));
            } else
            if (stl_x > 0 && map_pos_solid_at_ceiling(stl_x-1, stl_y))
            {
                filled_h = get_ceiling_or_floor_filled_subtiles(get_subtile_number(stl_x-1,stl_y));
            } else
            if (stl_y > 0 && map_pos_solid_at_ceiling(stl_x, stl_y-1))
            {
                filled_h = get_ceiling_or_floor_filled_subtiles(get_subtile_number(stl_x,stl_y-1));
            } else
            if (stl_x > 0 && stl_y > 0 && map_pos_solid_at_ceiling(stl_x-1, stl_y-1)) {
                filled_h = get_ceiling_or_floor_filled_subtiles(get_subtile_number(stl_x-1,stl_y-1));
            } else {
                filled_h = -1;
            }

            if (filled_h <= -1)
            {
              if (game.field_14A810 <= 0)
              {
                  filled_h = game.field_14A804;
              }
              else
              {
                int i;
                i = 0;
                while ( 1 )
                {
                    struct MapOffset *sstep;
                    sstep = &spiral_step[i];
                    MapSubtlCoord cstl_x;
                    MapSubtlCoord cstl_y;
                    cstl_x = stl_x + sstep->h;
                    cstl_y = stl_y + sstep->v;
                    if ((cstl_x >= 0) && (cstl_x <= map_subtiles_x))
                    {
                        if ((cstl_y >= 0) && (cstl_y <= map_subtiles_y))
                        {
                            filled_h = ceiling_block_is_solid_including_corners_return_height(sstep->both + get_subtile_number(stl_x,stl_y), cstl_x, cstl_y);
                            if (filled_h > -1)
                            {
                                int delta_tmp;
                                int delta_max;
                                delta_tmp = abs(stl_x - cstl_x);
                                delta_max = abs(stl_y - cstl_y);
                                if (delta_max <= delta_tmp)
                                    delta_max = delta_tmp;
                                if (filled_h < game.field_14A804)
                                {
                                    filled_h += game.field_14A814 * delta_max;
                                    if (filled_h >= game.field_14A804)
                                        filled_h = game.field_14A804;
                                } else
                                if ( filled_h > game.field_14A804 )
                                {
                                    filled_h -= game.field_14A814 * delta_max;
                                    if (filled_h <= game.field_14A808)
                                        filled_h = game.field_14A808;
                                }
                                break;
                            }
                        }
                    }
                    ++i;
                    if (i >= game.field_14A810) {
                        filled_h = game.field_14A804;
                        break;
                    }
                }
              }
            }
            struct Map *mapblk;
            mapblk = get_map_block_at(stl_x,stl_y);
            set_mapblk_filled_subtiles(mapblk, filled_h);
        }
    }
    return 1;
}

void do_creature_swap(long ncrt_id, long crtr_id)
{
//TODO SCRIPT rewrite from DD
  WARNMSG("Swapping creatures is only supported in Deeper Dungeons");
}

TbBool swap_creature(long ncrt_id, long crtr_id)
{
    if ((crtr_id < 0) || (crtr_id >= CREATURE_TYPES_COUNT))
    {
        ERRORLOG("Creature index %d is invalid", crtr_id);
        return false;
    }
    if (creature_swap_idx[crtr_id] > 0)
    {
        ERRORLOG("Creature of index %d already swapped", crtr_id);
        return false;
    }
    do_creature_swap(ncrt_id, crtr_id);
    return true;
}

void init_level(void)
{
    SYNCDBG(6,"Starting");
    struct IntralevelData transfer_mem;
    //_DK_init_level(); return;
    //LbMemoryCopy(&transfer_mem,&game.intralvl.transferred_creature,sizeof(struct CreatureStorage));
    LbMemoryCopy(&transfer_mem,&intralvl,sizeof(struct IntralevelData));
    game.flags_gui = GGUI_SoloChatEnabled;
    game.action_rand_seed = 1;
    free_swipe_graphic();
    game.loaded_swipe_idx = -1;
    game.play_gameturn = 0;
    clear_game();
    reset_heap_manager();
    lens_mode = 0;
    setup_heap_manager();

    // Load configs which may have per-campaign part, and can even be modified within a level
    load_computer_player_config(CnfLd_Standard);
    load_stats_files();
    check_and_auto_fix_stats();

    // We should do this after 'load stats'
    update_trap_tab_to_config();

    init_creature_scores();

    init_good_player_as(hero_player_number);
    light_set_lights_on(1);
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];

    erstats_clear();
    init_dungeons();
    // Load the actual level files
    preload_script(get_selected_level_number());
    load_map_file(get_selected_level_number());

    init_navigation();
    clear_messages();
    LbStringCopy(game.campaign_fname,campaign.fname,sizeof(game.campaign_fname));
#ifdef AUTOTESTING
    if (start_params.autotest_flags & ATF_FixedSeed)
    {
      game.action_rand_seed = 1;
      game.unsync_rand_seed = 1;
      srand(1);
    }
    else
#else
    // Initialize unsynchronized random seed (the value may be different
    // on computers in MP, as it shouldn't affect game actions)
    game.unsync_rand_seed = (unsigned long)LbTimeSec();
#endif
    if (!SoundDisabled)
    {
        game.field_14BB54 = (UNSYNC_RANDOM(67) % 3 + 1);
        game.field_14BB55 = 0;
    }
    light_set_lights_on(1);
    {
        struct PlayerInfo *player;
        player = get_player(game.hero_player_num);
        init_player_start(player, false);
    }
    game.numfield_D |= GNFldD_Unkn04;
    //LbMemoryCopy(&game.intralvl.transferred_creature,&transfer_mem,sizeof(struct CreatureStorage));
    LbMemoryCopy(&intralvl,&transfer_mem,sizeof(struct IntralevelData));
    event_initialise_all();
    battle_initialise();
    ambient_sound_prepare();
    zero_messages();
    game.armageddon_cast_turn = 0;
    game.armageddon_field_15035A = 0;
    init_messages();
    game.creatures_tend_imprison = 0;
    game.creatures_tend_flee = 0;
    game.pay_day_progress = 0;
    game.chosen_room_kind = 0;
    game.chosen_room_spridx = 0;
    game.chosen_room_tooltip = 0;
    set_chosen_power_none();
    game.manufactr_element = 0;
    game.manufactr_spridx = 0;
    game.manufactr_tooltip = 0;
}

void post_init_level(void)
{
    SYNCDBG(8,"Starting");
    //_DK_post_init_level(); return;
    if (game.packet_save_enable)
        open_new_packet_file_for_save();
    calculate_dungeon_area_scores();
    init_animating_texture_maps();
    reset_creature_max_levels();
    clear_creature_pool();
    setup_computer_players2();
    load_script(get_loaded_level_number());
    init_dungeons_research();
    init_dungeons_essential_position();
    create_transferred_creature_on_level();
    update_dungeons_scores();
    update_dungeon_generation_speeds();
    init_traps();
    init_all_creature_states();
    init_keepers_map_exploration();
    SYNCDBG(9,"Finished");
}

void startup_saved_packet_game(void)
{
    struct CatalogueEntry centry;
    //_DK_startup_saved_packet_game(); return;
    clear_packets();
    open_packet_file_for_load(game.packet_fname,&centry);
    if (!change_campaign(centry.campaign_fname))
    {
        ERRORLOG("Unable to load campaign associated with packet file");
    }
    set_selected_level_number(game.packet_save_head.level_num);
    lbDisplay.DrawColour = colours[15][15][15];
    game.pckt_gameturn = 0;
#if (BFDEBUG_LEVEL > 0)
    SYNCDBG(0,"Initialising level %d", (int)get_selected_level_number());
    SYNCMSG("Packet Loading Active (File contains %d turns)", game.turns_stored);
    SYNCMSG("Packet Checksum Verification %s",game.packet_checksum_verify ? "Enabled" : "Disabled");
    SYNCMSG("Fast Forward through %d game turns", game.turns_fastforward);
    if (game.turns_packetoff != -1)
        SYNCMSG("Packet Quit at %d", game.turns_packetoff);
    if (game.packet_load_enable)
    {
      if (game.log_things_end_turn != game.log_things_start_turn)
        SYNCMSG("Logging things, game turns %d -> %d", game.log_things_start_turn, game.log_things_end_turn);
    }
    SYNCMSG("Packet file prepared on KeeperFX %d.%d.%d.%d",(int)game.packet_save_head.game_ver_major,(int)game.packet_save_head.game_ver_minor,
        (int)game.packet_save_head.game_ver_release,(int)game.packet_save_head.game_ver_build);
#endif
    if ((game.packet_save_head.game_ver_major != VER_MAJOR) || (game.packet_save_head.game_ver_minor != VER_MINOR)
     || (game.packet_save_head.game_ver_release != VER_RELEASE) || (game.packet_save_head.game_ver_build != VER_BUILD)) {
        WARNLOG("Packet file was created with different version of the game; this rarely works");
    }
    game.game_kind = GKind_LocalGame;
    if (!(game.packet_save_head.players_exist & (1 << game.local_plyr_idx))
      || (game.packet_save_head.players_comp & (1 << game.local_plyr_idx)))
      my_player_number = 0;
    else
      my_player_number = game.local_plyr_idx;
    init_level();
    setup_zombie_players();//TODO GUI What about packet file from network game? No zombies there..
    init_players();
    if (game.active_players_count == 1)
      game.game_kind = GKind_LocalGame;
    if (game.turns_stored < game.turns_fastforward)
      game.turns_fastforward = game.turns_stored;
    post_init_level();
    post_init_players();
    set_selected_level_number(0);
    if (is_key_pressed(KC_LALT, KMod_NONE))
    {
        struct PlayerInfo* player = get_my_player();
        set_engine_view(player, PVM_FrontView);
    }
}

void faststartup_saved_packet_game(void)
{
    reenter_video_mode();
    startup_saved_packet_game();
    {
        struct PlayerInfo *player;
        player = get_my_player();
        player->flgfield_6 &= ~PlaF6_PlyrHasQuit;
    }
    set_gui_visible(false);
    set_flag_byte(&game.operation_flags,GOF_ShowPanel,false);
}

void startup_network_game(TbBool local)
{
    SYNCDBG(0,"Starting up network game");
    //_DK_startup_network_game(); return;
    unsigned int flgmem;
    struct PlayerInfo *player;
    setup_count_players();
    player = get_my_player();
    flgmem = player->is_active;
    if (local && (campaign.human_player >= 0) && (!force_player_num))
    {
        default_loc_player = campaign.human_player;
        game.local_plyr_idx = default_loc_player;
        my_player_number = default_loc_player;
    }
    init_level();
    player = get_my_player();
    player->is_active = flgmem;
    //if (game.flagfield_14EA4A == 2) //was wrong because init_level sets this to 2. global variables are evil (though perhaps that's why they were chosen for DK? ;-))
    TbBool ShouldAssignCpuKeepers = 0;
    if (local)
    {
        game.game_kind = GKind_LocalGame;
        init_players_local_game();
        if (AssignCpuKeepers || campaign.assignCpuKeepers) {
            ShouldAssignCpuKeepers = 1;
        }
    } else
    {
        game.game_kind = GKind_MultiGame;
        init_players_network_game();
    }
    if (fe_computer_players || ShouldAssignCpuKeepers)
    {
        SYNCDBG(5,"Setting up uninitialized players as computer players");
        setup_computer_players();
    } else
    {
        SYNCDBG(5,"Setting up uninitialized players as zombie players");
        setup_zombie_players();
    }
    post_init_level();
    post_init_players();
    post_init_packets();
    set_selected_level_number(0);
    //LbNetwork_EnableLag(1);
}

void faststartup_network_game(void)
{
    struct PlayerInfo *player;
    SYNCDBG(3,"Starting");
    reenter_video_mode();
    my_player_number = default_loc_player;
    game.game_kind = GKind_LocalGame;
    if (!is_campaign_loaded())
    {
        if (!change_campaign(""))
        ERRORLOG("Unable to load campaign");
    }
    player = get_my_player();
    player->is_active = 1;
    startup_network_game(true);
    player = get_my_player();
    player->flgfield_6 &= ~PlaF6_PlyrHasQuit;
}

void wait_at_frontend(void)
{
    struct PlayerInfo *player;
    SYNCDBG(0,"Falling into frontend menu.");
    // Moon phase calculation
    calculate_moon_phase(true,false);
    update_extra_levels_visibility();
    // Returning from Demo Mode
    if (game.flags_cd & MFlg_IsDemoMode)
    {
      close_packet_file();
      game.packet_load_enable = 0;
    }
    game.numfield_15 = -1;
    // Make sure campaigns are loaded
    if (!load_campaigns_list())
    {
      ERRORLOG("No valid campaign files found");
      exit_keeper = 1;
      return;
    }
    // Make sure mappacks are loaded
    if (!load_mappacks_list())
    {
      WARNMSG("No valid mappack files found");
    }
    //Set level number and campaign (for single level mode: GOF_SingleLevel)
    if ((start_params.operation_flags & GOF_SingleLevel) != 0) {
        TbBool result = false;
        if (start_params.selected_campaign[0] != '\0') {
            result = change_campaign(strcat(start_params.selected_campaign,".cfg"));
        }
        if (!result) {
            if (!change_campaign("")) {
                WARNMSG("Unable to load default campaign for the specified level CMD Line parameter");
            }
            else if (start_params.selected_campaign[0] != '\0') { // only show this log message if the user actually specified a campaign
                WARNMSG("Unable to load campaign associated with the specified level CMD Line parameter, default loaded.");
            }
            else {
                JUSTLOG("No campaign specified. Default campaign loaded for selected level (%d).", start_params.selected_level_number);
            }
        }
        set_selected_level_number(start_params.selected_level_number);
        //game.selected_level_number = start_params.selected_level_number;
    }
    else {
        set_selected_level_number(first_singleplayer_level());
    }
    // Init load/save catalogue
    initialise_load_game_slots();
    // Prepare to enter PacketLoad game
    if ((game.packet_load_enable) && (!game.numfield_149F47))
    {
      faststartup_saved_packet_game();
      return;
    }
    // Prepare to enter network/standard game
    if ((game.operation_flags & GOF_SingleLevel) != 0)
    {
      faststartup_network_game();
      return;
    }

    if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
    {
      FatalError = 1;
      exit_keeper = 1;
      return;
    }
    LbScreenClear(0);
    LbScreenSwap();
    if (frontend_load_data() != Lb_SUCCESS)
    {
      ERRORLOG("Unable to load frontend data");
      exit_keeper = 1;
      return;
    }
    memset(scratch, 0, PALETTE_SIZE);
    LbPaletteSet(scratch);
    frontend_set_state(get_startup_menu_state());

    short finish_menu = 0;
    set_flag_byte(&game.flags_cd,MFlg_unk40,false);
    // Begin the frontend loop
    long last_loop_time = LbTimerClock();
    do
    {
      if (!LbWindowsControl())
      {
        if ((game.system_flags & GSF_NetworkActive) == 0)
        {
            exit_keeper = 1;
            SYNCDBG(0,"Windows Control exit condition invoked");
            break;
        }
      }
      update_mouse();
      update_key_modifiers();
      old_mouse_over_button = frontend_mouse_over_button;
      frontend_mouse_over_button = 0;

      frontend_input();
      if ( exit_keeper )
      {
        SYNCDBG(0,"Frontend Input exit condition invoked");
        break; // end while
      }

      frontend_update(&finish_menu);
      if ( exit_keeper )
      {
        SYNCDBG(0,"Frontend Update exit condition invoked");
        break; // end while
      }

      if ((!finish_menu) && (LbIsActive()))
      {
        frontend_draw();
        LbScreenSwap();
      }

      if (!SoundDisabled)
      {
        process_3d_sounds();
        process_sound_heap();
        MonitorStreamedSoundTrack();
      }

      if (fade_palette_in)
      {
        fade_in();
        fade_palette_in = 0;
      } else
      {
        LbSleepUntil(last_loop_time + 30);
      }
      last_loop_time = LbTimerClock();
    } while (!finish_menu);

    LbPaletteFade(0, 8, Lb_PALETTE_FADE_CLOSED);
    LbScreenClear(0);
    LbScreenSwap();
    FrontendMenuState prev_state;
    prev_state = frontend_menu_state;
    frontend_set_state(FeSt_INITIAL);
    if (exit_keeper)
    {
      player = get_my_player();
      player->flgfield_6 &= ~PlaF6_PlyrHasQuit;
      return;
    }
    reenter_video_mode();

    display_loading_screen();
    short flgmem;
    switch (prev_state)
    {
    case FeSt_START_KPRLEVEL:
          my_player_number = default_loc_player;
          game.game_kind = GKind_LocalGame;
          set_flag_byte(&game.system_flags,GSF_NetworkActive,false);
          player = get_my_player();
          player->is_active = 1;
          startup_network_game(true);
          break;
    case FeSt_START_MPLEVEL:
          set_flag_byte(&game.system_flags,GSF_NetworkActive,true);
          game.game_kind = GKind_MultiGame;
          player = get_my_player();
          player->is_active = 1;
          startup_network_game(false);
          break;
    case FeSt_LOAD_GAME:
          flgmem = game.numfield_15;
          set_flag_byte(&game.system_flags,GSF_NetworkActive,false);
          LbScreenClear(0);
          LbScreenSwap();
          if (!load_game(game.numfield_15))
          {
              ERRORLOG("Loading game %d failed; quitting.",(int)game.numfield_15);
              quit_game = 1;
          }
          game.numfield_15 = flgmem;
          break;
    case FeSt_PACKET_DEMO:
          game.flags_cd |= MFlg_IsDemoMode;
          startup_saved_packet_game();
          set_gui_visible(false);
          set_flag_byte(&game.operation_flags,GOF_ShowPanel,false);
          break;
    }
    player = get_my_player();
    player->flgfield_6 &= ~PlaF6_PlyrHasQuit;
}

void game_loop(void)
{
    //_DK_game_loop(); return;
    unsigned long total_play_turns;
    unsigned long playtime;
    playtime = 0;
    total_play_turns = 0;
    SYNCDBG(0,"Entering gameplay loop.");

    while ( !exit_keeper )
    {
      update_mouse();
      wait_at_frontend();
      if ( exit_keeper )
        break;
      struct PlayerInfo *player;
      player = get_my_player();
      if (game.game_kind == GKind_LocalGame)
      {
        if (game.numfield_15 == -1)
        {
          set_player_instance(player, PI_HeartZoom, 0);
        } else
        {
          game.numfield_15 = -1;
          set_flag_byte(&game.operation_flags,GOF_Paused,false);
        }
      }
      unsigned long starttime;
      unsigned long endtime;
      struct Dungeon *dungeon;
      // get_my_dungeon() can't be used here because players are not initialized yet
      dungeon = get_dungeon(my_player_number);
      starttime = LbTimerClock();
      dungeon->lvstats.start_time = starttime;
      dungeon->lvstats.end_time = starttime;
      LbScreenClear(0);
      LbScreenSwap();
      keeper_gameplay_loop();
      set_pointer_graphic_none();
      LbScreenClear(0);
      LbScreenSwap();
      StopMusic();
      StopMusicPlayer();
      turn_off_all_menus();
      delete_all_structures();
      clear_mapwho();
      endtime = LbTimerClock();
      quit_game = 0;
      if ((game.operation_flags & GOF_SingleLevel) != 0)
          exit_keeper=true;
      playtime += endtime-starttime;
      SYNCDBG(0,"Play time is %d seconds",playtime>>10);
      total_play_turns += game.play_gameturn;
      reset_eye_lenses();
      close_packet_file();
      game.packet_load_enable = false;
      game.packet_save_enable = false;
    } // end while

    ShutdownMusicPlayer();
    // Stop the movie recording if it's on
    if ((game.system_flags & GSF_CaptureMovie) != 0) {
        movie_record_stop();
    }
    SYNCDBG(7,"Done");
}

short reset_game(void)
{
    SYNCDBG(6,"Starting");
    IsRunningUnmark();

    KeeperSpeechExit();

    LbMouseSuspend();
    LbIKeyboardClose();
    LbScreenReset();
    LbDataFreeAll(game_load_files);
    free_gui_strings_data();
    FreeAudio();
    return LbMemoryReset();
}

short process_command_line(unsigned short argc, char *argv[])
{
  char fullpath[CMDLN_MAXLEN+1];
  strncpy(fullpath, argv[0], CMDLN_MAXLEN);

  sprintf( keeper_runtime_directory, fullpath);
  char *endpos = strrchr( keeper_runtime_directory, '\\');
  if (endpos==NULL)
      endpos=strrchr( keeper_runtime_directory, '/');
  if (endpos!=NULL)
      *endpos='\0';
  else
      strcpy(keeper_runtime_directory, ".");

  AssignCpuKeepers = 0;
  SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain
  LbErrorLogSetup(0, 0, 1);

  set_default_startup_parameters();

  short bad_param;
  LevelNumber level_num;
  bad_param = 0;
  unsigned short narg;
  level_num = LEVELNUMBER_ERROR;
  TbBool one_player_mode = 0;
  narg = 1;
  while ( narg < argc )
  {
      char *par;
      par = argv[narg];
      if ( (par == NULL) || ((par[0] != '-') && (par[0] != '/')) )
          return -1;
      char parstr[CMDLN_MAXLEN+1];
      char pr2str[CMDLN_MAXLEN+1];
      char pr3str[CMDLN_MAXLEN+1];
      strncpy(parstr, par+1, CMDLN_MAXLEN);
      if (narg + 1 < argc)
      {
          strncpy(pr2str,  argv[narg+1], CMDLN_MAXLEN);
          if (narg + 2 < argc)
              strncpy(pr3str,  argv[narg+2], CMDLN_MAXLEN);
          else
              pr3str[0]='\0';
      }
      else
      {
          pr2str[0]='\0';
          pr3str[0]='\0';
      }

      if (strcasecmp(parstr, "nointro") == 0)
      {
        start_params.no_intro = 1;
      } else
      if (strcasecmp(parstr, "nocd") == 0)
      {
          set_flag_byte(&start_params.flags_cd,MFlg_NoCdMusic,true);
      } else
      if (strcasecmp(parstr, "1player") == 0)
      {
          start_params.one_player = 1;
          one_player_mode = 1;
      } else
      if ((strcasecmp(parstr, "s") == 0) || (strcasecmp(parstr, "nosound") == 0))
      {
          SoundDisabled = 1;
      } else
      if (strcasecmp(parstr, "fps") == 0)
      {
          narg++;
          start_params.num_fps = atoi(pr2str);
      } else
      if (strcasecmp(parstr, "human") == 0)
      {
          narg++;
          default_loc_player = atoi(pr2str);
          force_player_num = true;
      } else
      if (strcasecmp(parstr, "usersfont") == 0)
      {
          set_flag_byte(&start_params.flags_font,FFlg_UsrSndFont,true);
      } else
      if (strcasecmp(parstr, "vidsmooth") == 0)
      {
          smooth_on = 1;
      } else
      if ( strcasecmp(parstr,"level") == 0 )
      {
        set_flag_byte(&start_params.operation_flags,GOF_SingleLevel,true);
        level_num = atoi(pr2str);
        narg++;
      } else
      if ( strcasecmp(parstr,"campaign") == 0 )
      {
        strcpy(start_params.selected_campaign, pr2str);
        narg++;
      } else
      if ( strcasecmp(parstr,"ppropoly") == 0 )
      {
          start_params.force_ppro_poly = atoi(pr2str);
          narg++;
      } else
      if ( strcasecmp(parstr,"altinput") == 0 )
      {
          SYNCLOG("Mouse auto reset disabled");
          lbMouseGrab = false;
      } else
      if (strcasecmp(parstr,"packetload") == 0)
      {
         if (start_params.packet_save_enable)
            WARNMSG("PacketSave disabled to enable PacketLoad.");
         start_params.packet_load_enable = true;
         start_params.packet_save_enable = false;
         strncpy(start_params.packet_fname,pr2str,sizeof(start_params.packet_fname)-1);
         narg++;
      } else
      if (strcasecmp(parstr,"packetsave") == 0)
      {
         if (start_params.packet_load_enable)
            WARNMSG("PacketLoad disabled to enable PacketSave.");
         start_params.packet_load_enable = false;
         start_params.packet_save_enable = true;
         strncpy(start_params.packet_fname,pr2str,sizeof(start_params.packet_fname)-1);
         narg++;
      } else
      if (strcasecmp(parstr,"q") == 0)
      {
         set_flag_byte(&start_params.operation_flags,GOF_SingleLevel,true);
      } else
      if (strcasecmp(parstr,"columnconvert") == 0)
      {
         set_flag_byte(&start_params.operation_flags,GOF_ColumnConvert,true);
      } else
      if (strcasecmp(parstr,"lightconvert") == 0)
      {
         set_flag_byte(&start_params.operation_flags,GOF_LightConvert,true);
      } else
      if (strcasecmp(parstr, "dbgshots") == 0)
      {
          start_params.debug_flags |= DFlg_ShotsDamage;
      } else
      if (strcasecmp(parstr, "dbgpathfind") == 0)
      {
	      start_params.debug_flags |= DFlg_CreatrPaths;
      } else
      if (strcasecmp(parstr, "compuchat") == 0)
      {
          if (strcasecmp(pr2str,"scarce") == 0) {
              start_params.computer_chat_flags = CChat_TasksScarce;
          } else
          if (strcasecmp(pr2str,"frequent") == 0) {
              start_params.computer_chat_flags = CChat_TasksScarce|CChat_TasksFrequent;
          } else {
              start_params.computer_chat_flags = CChat_None;
          }
          narg++;
      } else
      if (strcasecmp(parstr, "sessions") == 0) {
          narg++;
          LbNetwork_InitSessionsFromCmdLine(pr2str);
      } else
      if (strcasecmp(parstr,"alex") == 0)
      {
         set_flag_byte(&start_params.flags_font,FFlg_AlexCheat,true);
      } else
      if (strcasecmp(parstr,"frameskip") == 0)
      {
         start_params.frame_skip = atoi(pr2str);
         narg++;
      }
#ifdef AUTOTESTING
      else if (strcasecmp(parstr, "exit_at_turn") == 0)
      {
         set_flag_byte(&start_params.autotest_flags, ATF_ExitOnTurn, true);
         start_params.autotest_exit_turn = atol(pr2str);
         narg++;
      } else
      if (strcasecmp(parstr, "fixed_seed") == 0)
      {
         set_flag_byte(&start_params.autotest_flags, ATF_FixedSeed, true);
      } else
      if (strcasecmp(parstr, "tests") == 0)
      {
        set_flag_byte(&start_params.autotest_flags, ATF_TestsCampaign, true);

        if (!change_campaign("../tests/campaign.cfg"))
        {
          ERRORLOG("Unable to load tests campaign");
          bad_param=narg;
        }
      } else
      if (strcasecmp(parstr, "ai_player") == 0)
      {
         set_flag_byte(&start_params.autotest_flags, ATF_AI_Player, true);
         fe_computer_players = 1;
      } else
      if (strcasecmp(parstr, "monitoring") == 0)
      {
          int instance_no = atoi(pr3str);
          evm_init(pr2str, instance_no);
          narg++;
          if ((instance_no > 0) || (strcmp(pr3str, "0") == 0))
              narg++;
      }
#endif
      else
      {
        WARNMSG("Unrecognized command line parameter '%s'.",parstr);
        bad_param=narg;
      }
      narg++;
  }

  if (level_num == LEVELNUMBER_ERROR)
  {
      if (first_singleplayer_level() > 0)
      {
          level_num = first_singleplayer_level();
      }
      else
      {
          level_num = 1;
      }
  }
  else {
      if (one_player_mode) {
          AssignCpuKeepers = 1;
      }
  }
  start_params.selected_level_number = level_num;
  my_player_number = default_loc_player;
  return (bad_param==0);
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
    short retval;
    retval=0;
    LbErrorLogSetup("/", log_file_name, 5);

    retval = process_command_line(argc,argv);
    if (retval < 1)
    {
        static const char *msg_text="Command line parameters analysis failed.\n";
        error_dialog_fatal(__func__, 1, msg_text);
        LbErrorLogClose();
        return 0;
    }

    retval = true;
    retval &= (LbTimerInit() != Lb_FAIL);
    retval &= (LbScreenInitialize() != Lb_FAIL);
    LbSetTitle(PROGRAM_NAME);
    LbSetIcon(1);
    LbScreenSetDoubleBuffering(true);
#ifdef AUTOTESTING
    if (start_params.autotest_flags & ATF_FixedSeed)
    {
      srand(1);
    }
    else
#else
    srand(LbTimerClock());
#endif
    if (!retval)
    {
        static const char *msg_text="Basic engine initialization failed.\n";
        error_dialog_fatal(__func__, 1, msg_text);
        LbErrorLogClose();
        return 0;
    }

    retval = setup_game();
    if (retval)
    {
      if ((install_info.lang_id == Lang_Japanese) ||
          (install_info.lang_id == Lang_ChineseInt) ||
          (install_info.lang_id == Lang_ChineseTra) ||
          (install_info.lang_id == Lang_Korean))
      {
        switch (install_info.lang_id)
        {
        case Lang_Japanese:
            dbc_set_language(1);
            break;
        case Lang_ChineseInt:
            dbc_set_language(2);
            break;
        case Lang_ChineseTra:
            dbc_set_language(3);
            break;
        case Lang_Korean:
            dbc_set_language(4);
            break;
        }
        if (dbc_initialize("fxdata"))
        {
          ERRORLOG("DBC fonts Initialization failed.");
        }
      }
    }
    if ( retval )
    {
        game_loop();
    }
#ifdef AUTO_TESTING
    ev_done();
#endif
    reset_game();
    LbScreenReset();
    if ( !retval )
    {
        static const char *msg_text="Setting up game failed.\n";
        error_dialog_fatal(__func__, 2, msg_text);
    } else
    {
        SYNCDBG(0,"finished properly");
    }
    LbErrorLogClose();
    return 0;
}

void get_cmdln_args(unsigned short &argc, char *argv[])
{
    char *ptr;
    const char *cmndln_orig;
    cmndln_orig = GetCommandLineA();
    strncpy(cmndline, cmndln_orig, CMDLN_MAXLEN);
    ptr = cmndline;
    bf_argc = 0;
    while (*ptr != '\0')
    {
        if ((*ptr == '\t') || (*ptr == ' '))
        {
            ptr++;
            continue;
        }
        if (*ptr == '\"')
        {
            ptr++;
            bf_argv[bf_argc] = ptr;
            bf_argc++;
            while (*ptr != '\0')
            {
              if (*ptr == '\"')
              {
                  *ptr++ = '\0';
                  break;
              }
              ptr++;
            }
        } else
        {
            bf_argv[bf_argc] = ptr;
            bf_argc++;
            while (*ptr != '\0')
            {
              if ((*ptr == '\t') || (*ptr == ' '))
              {
                  *ptr++ = '\0';
                  break;
              }
              ptr++;
            }
        }
    }
}

int main(int argc, char *argv[])
{
  char *text;
  _DK_hInstance = GetModuleHandle(NULL);

  get_cmdln_args(bf_argc, bf_argv);

//TODO DLL_CLEANUP delete when won't be needed anymore
  memcpy(_DK_menu_list,menu_list,40*sizeof(struct GuiMenu *));
  memcpy(_DK_player_instance_info,player_instance_info,17*sizeof(struct PlayerInstanceInfo));
  memcpy(_DK_states,states,145*sizeof(struct StateInfo));
  memcpy(_DK_room_data,room_data,17*sizeof(struct RoomData));

#if (BFDEBUG_LEVEL > 1)
  if (sizeof(struct Game) != SIZEOF_Game)
  {
      long delta1;
      long delta2;
      long delta3;
      if (sizeof(struct PlayerInfo) != SIZEOF_PlayerInfo)
      {
          text = buf_sprintf("Bad compilation - struct PlayerInfo has wrong size!\nThe difference is %d bytes.\n",sizeof(struct PlayerInfo)-SIZEOF_PlayerInfo);
          error_dialog(__func__, 1, text);
          return 1;
      }
      if (sizeof(struct Dungeon) != SIZEOF_Dungeon)
      {
          text = buf_sprintf("Bad compilation - struct Dungeon has wrong size!\nThe difference is %d bytes.\n",sizeof(struct Dungeon)-SIZEOF_Dungeon);
          error_dialog(__func__, 1, text);
          return 1;
      }
      if (sizeof(struct CreatureControl) != SIZEOF_CreatureControl)
      {
          //delta1 =((char *)&game.cctrl_data[0].moveto_pos) - ((char *)&game.cctrl_data);
          text = buf_sprintf("Bad compilation - struct CreatureControl has wrong size!\nThe difference is %d bytes.\n",sizeof(struct CreatureControl)-SIZEOF_CreatureControl);
          error_dialog(__func__, 1, text);
          return 1;
      }
      delta1 =((char *)&game.land_map_start) - ((char *)&game) - 0x1DD40;
      delta2 =((char *)&game.cctrl_data) - ((char *)&game) - 0x66157;
      delta3 =((char *)&game.creature_scores) - ((char *)&game) - 0x14EA4C;
      text = buf_sprintf("Bad compilation - struct Game has wrong size!\nThe difference is %d bytes.\n"
          "Field \"land_map_start\" is moved by %ld bytes.\nField \"cctrl_data\" is moved by %ld bytes.\n"
          "Field \"creature_scores\" is moved by %ld bytes.\n",sizeof(struct Game)-SIZEOF_Game,delta1,delta2,delta3);
      error_dialog(__func__, 1, text);
      return 1;
  }
  if (sizeof(struct S3DSample) != 37)
  {
      text = buf_sprintf("Bad compilation - struct S3DSample has wrong size!\nThe difference is %d bytes.\n",sizeof(struct S3DSample)-37);
      error_dialog(__func__, 1, text);
      return 1;
  }
#endif

  try {
  LbBullfrogMain(bf_argc, bf_argv);
  } catch (...)
  {
      text = buf_sprintf("Exception raised!");
      error_dialog(__func__, 1, text);
      return 1;
  }

//  LbFileSaveAt("!tmp_file", &_DK_game, sizeof(struct Game));

  return 0;
}

#ifdef __cplusplus
}
#endif
