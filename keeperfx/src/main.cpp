
#include <windows.h>
#include <winbase.h>
#include <math.h>
#include <string>
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
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

#include "front_simple.h"
#include "frontend.h"
#include "front_input.h"
#include "gui_draw.h"
#include "gui_tooltips.h"
#include "gui_parchment.h"
#include "gui_frontmenu.h"
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
#include "lvl_script.h"
#include "lvl_filesdk1.h"
#include "thing_list.h"
#include "player_instances.h"
#include "player_utils.h"
#include "game_heap.h"
#include "game_saves.h"
#include "engine_render.h"
#include "engine_lenses.h"
#include "engine_camera.h"
#include "engine_arrays.h"
#include "engine_redraw.h"
#include "front_landview.h"
#include "front_lvlstats.h"
#include "front_easter.h"
#include "thing_stats.h"
#include "thing_physics.h"
#include "thing_creature.h"
#include "thing_corpses.h"
#include "thing_objects.h"
#include "thing_effects.h"
#include "thing_doors.h"
#include "thing_traps.h"
#include "thing_shots.h"
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
#include "creature_control.h"
#include "creature_states.h"
#include "creature_instances.h"
#include "creature_graphics.h"
#include "creature_states_rsrch.h"
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
#include "frontmenu_ingame_tabs.h"
#include "ariadne.h"
#include "net_game.h"
#include "sounds.h"
#include "vidfade.h"
#include "KeeperSpeech.h"
#include "config_settings.h"
#include "game_legacy.h"
#include "room_list.h"

int test_variable;

// Max length of the command line
#define CMDLN_MAXLEN 259
char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];

short default_loc_player = 0;
struct StartupParameters start_params;

//long const imp_spangle_effects[] = {

unsigned short const player_state_to_spell[] = {
  0, 0, 0,  0,  0,  0, 6, 7, 5, 0, 18, 18, 0, 0, 0, 0,
  0,10, 0, 11, 12, 13, 8, 0, 2,16, 14, 15, 0, 3, 0, 0,
};

//static
TbClockMSec last_loop_time=0;

#ifdef __cplusplus
extern "C" {
#endif

DLLIMPORT void _DK_draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4);
DLLIMPORT void _DK_draw_lightning(const struct Coord3d *pos1, const struct Coord3d *pos2, long a3, long a4);
DLLIMPORT void _DK_init_alpha_table(void);
DLLIMPORT void _DK_engine_init(void);
DLLIMPORT long _DK_load_anim_file(void);
DLLIMPORT void _DK_init_colours(void);
DLLIMPORT void _DK_place_animating_slab_type_on_map(long a1, char a2, unsigned char a3, unsigned char a4, unsigned char a5);
DLLIMPORT void _DK_draw_spell_cursor(unsigned char a1, unsigned short a2, unsigned char stl_x, unsigned char stl_y);
DLLIMPORT void _DK_update_breed_activities(void);
DLLIMPORT struct Thing *_DK_get_queryable_object_near(unsigned short a1, unsigned short a2, long a3);
DLLIMPORT int _DK_can_thing_be_queried(struct Thing *thing, long a2);
DLLIMPORT int _DK_can_thing_be_possessed(struct Thing *thing, long a2);
DLLIMPORT long _DK_tag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3);
DLLIMPORT void _DK_untag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3);
DLLIMPORT void _DK_tag_cursor_blocks_sell_area(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT long _DK_packet_place_door(long a1, long a2, long a3, long a4, unsigned char a5);
DLLIMPORT void _DK_delete_room_slabbed_objects(long a1);
DLLIMPORT unsigned char _DK_tag_cursor_blocks_place_door(unsigned char a1, long a2, long a3);
DLLIMPORT unsigned char _DK_tag_cursor_blocks_place_room(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT void _DK_tag_cursor_blocks_dig(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT void _DK_tag_cursor_blocks_thing_in_hand(unsigned char a1, long a2, long a3, int a4, long a5);
DLLIMPORT struct Thing *_DK_get_door_for_position(long pos_x, long pos_y);
DLLIMPORT long _DK_is_thing_passenger_controlled(struct Thing *thing);
DLLIMPORT void _DK_setup_3d(void);
DLLIMPORT void _DK_setup_stuff(void);
DLLIMPORT void _DK_init_keeper(void);
DLLIMPORT void _DK_check_map_for_gold(void);
DLLIMPORT void _DK_set_thing_draw(struct Thing *thing, long a2, long a3, long a4, char a5, char a6, unsigned char a7);
DLLIMPORT unsigned long _DK_can_drop_thing_here(long x, long y, long a3, unsigned long a4);
DLLIMPORT void _DK_go_to_my_next_room_of_type(unsigned long rkind);
DLLIMPORT void _DK_instant_instance_selected(long a1);
DLLIMPORT void _DK_initialise_map_collides(void);
DLLIMPORT void _DK_initialise_map_health(void);
DLLIMPORT void _DK_initialise_extra_slab_info(unsigned long lv_num);
DLLIMPORT void _DK_clear_map(void);
DLLIMPORT long _DK_ceiling_init(unsigned long a1, unsigned long a2);
DLLIMPORT long _DK_screen_to_map(struct Camera *camera, long scrpos_x, long scrpos_y, struct Coord3d *mappos);
DLLIMPORT void _DK_draw_swipe(void);
DLLIMPORT void _DK_draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7);
DLLIMPORT void _DK_check_players_won(void);
DLLIMPORT void _DK_check_players_lost(void);
DLLIMPORT void _DK_process_dungeon_power_magic(void);
DLLIMPORT void _DK_process_dungeon_devastation_effects(void);
DLLIMPORT void _DK_process_payday(void);
DLLIMPORT struct Room *_DK_player_has_room_of_type(long plr_idx, long roomkind);
DLLIMPORT unsigned long _DK_setup_move_off_lava(struct Thing *thing);
DLLIMPORT long _DK_load_texture_map_file(unsigned long lv_num, unsigned char n);
DLLIMPORT long _DK_get_foot_creature_has_down(struct Thing *thing);
DLLIMPORT void _DK_process_keeper_spell_effect(struct Thing *thing);
DLLIMPORT unsigned long _DK_lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos);
DLLIMPORT void _DK_affect_nearby_enemy_creatures_with_wind(struct Thing *thing);
DLLIMPORT void _DK_affect_nearby_stuff_with_vortex(struct Thing *thing);
DLLIMPORT void _DK_affect_nearby_friends_with_alarm(struct Thing *thing);
DLLIMPORT long _DK_apply_wallhug_force_to_boulder(struct Thing *thing);
DLLIMPORT long _DK_process_creature_self_spell_casting(struct Thing *thing);
DLLIMPORT void _DK_check_and_auto_fix_stats(void);
DLLIMPORT void _DK_delete_all_structures(void);
DLLIMPORT void _DK_clear_game(void);
DLLIMPORT void _DK_clear_game_for_save(void);
DLLIMPORT long _DK_update_cave_in(struct Thing *thing);
DLLIMPORT void _DK_update_thing_animation(struct Thing *thing);
DLLIMPORT void _DK_init_messages(void);
DLLIMPORT void _DK_message_add(char c);
DLLIMPORT void _DK_toggle_creature_tendencies(struct PlayerInfo *player, char val);
DLLIMPORT long _DK_set_autopilot_type(long plridx, long aptype);
DLLIMPORT void _DK_turn_off_sight_of_evil(long plridx);
DLLIMPORT void _DK_directly_cast_spell_on_thing(unsigned char plridx, unsigned char a2, unsigned short a3, long a4);
DLLIMPORT void _DK_lose_level(struct PlayerInfo *player);
DLLIMPORT void _DK_level_lost_go_first_person(long plridx);
DLLIMPORT void _DK_complete_level(struct PlayerInfo *player);
DLLIMPORT void _DK_free_swipe_graphic(void);
DLLIMPORT void _DK_draw_bonus_timer(void);
DLLIMPORT void _DK_engine(struct Camera *cam);
DLLIMPORT void _DK_draw_overlay_things(long zoom);
DLLIMPORT void _DK_reinit_level_after_load(void);
DLLIMPORT void _DK_reinit_tagged_blocks_for_player(unsigned char idx);
DLLIMPORT void _DK_reset_gui_based_on_player_mode(void);
DLLIMPORT void _DK_init_animating_texture_maps(void);
DLLIMPORT void _DK_init_lookups(void);
DLLIMPORT void _DK_restore_computer_player_after_load(void);
DLLIMPORT int _DK_play_smacker_file(char *fname, int);
DLLIMPORT int _DK_LoadMcgaData(void);
DLLIMPORT long _DK_ceiling_set_info(long a1, long a2, long a3);
DLLIMPORT void _DK_startup_saved_packet_game(void);
DLLIMPORT void __stdcall _DK_IsRunningMark(void);
DLLIMPORT void __stdcall _DK_IsRunningUnmark(void);
DLLIMPORT int __stdcall _DK_play_smk_(char *fname, int smkflags, int plyflags);
DLLIMPORT void _DK_cumulative_screen_shot(void);
DLLIMPORT void _DK_frontend_set_state(long);
DLLIMPORT void _DK_demo(void);
DLLIMPORT void _DK_draw_gui(void);
DLLIMPORT void _DK_process_dungeons(void);
DLLIMPORT void _DK_process_level_script(void);
DLLIMPORT void _DK_message_update(void);
DLLIMPORT long _DK_wander_point_update(struct Wander *wandr);
DLLIMPORT void _DK_update_player_camera(struct PlayerInfo *player);
DLLIMPORT void _DK_set_level_objective(char *msg_text);
DLLIMPORT void _DK_update_flames_nearest_camera(struct Camera *camera);
DLLIMPORT void _DK_update_footsteps_nearest_camera(struct Camera *camera);
DLLIMPORT void _DK_process_player_states(void);
DLLIMPORT void _DK_set_mouse_light(struct PlayerInfo *player);
DLLIMPORT void _DK_draw_gui(void);
DLLIMPORT void _DK_turn_off_query(char a);
DLLIMPORT void _DK_post_init_level(void);
DLLIMPORT void _DK_init_level(void);
DLLIMPORT int _DK_frontend_is_player_allied(long plyr1, long plyr2);
DLLIMPORT void _DK_process_dungeon_destroy(struct Thing *thing);
DLLIMPORT long _DK_wp_check_map_pos_valid(struct Wander *wandr, long a1);
DLLIMPORT void _DK_startup_network_game(void);
DLLIMPORT void _DK_load_ceiling_table(void);
// Now variables
DLLIMPORT extern HINSTANCE _DK_hInstance;

#ifdef __cplusplus
}
#endif

TbPixel get_player_path_colour(unsigned short owner)
{
  return player_path_colours[player_colors_map[owner % PLAYERS_EXT_COUNT]];
}

void setup_block_mem(void)
{
    unsigned char **dst;
    unsigned char *src;
    unsigned long i,k,n;
    dst = block_ptrs;
    n = 0;
    for (i=0; i < 68; i++)
    {
        src = block_mem + n;
        for (k=0; k < 8; k++)
        {
            *dst = src;
            src += 32;
            dst++;
        }
        n += 8192;
    }
}

TbBool init_fades_table(void)
{
    char *fname;
    long i;
    static const char textname[] = "fade table";
    fname = prepare_file_path(FGrp_StdData,"tables.dat");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    setup_block_mem();
    if (LbFileLoadAt(fname, &pixmap) != sizeof(struct TbColorTables))
    {
        compute_fade_tables(&pixmap,_DK_palette,_DK_palette);
        LbFileSaveAt(fname, &pixmap, sizeof(struct TbColorTables));
    }
    lbDisplay.FadeTable = pixmap.fade_tables;
    TbPixel cblack = 144;
    // Update black color
    for (i=0; i < 8192; i++)
    {
        if (pixmap.fade_tables[i] == 0) {
            pixmap.fade_tables[i] = cblack;
        }
    }
    return true;
}


TbBool init_alpha_table(void)
{
    char *fname;
    static const char textname[] = "alpha color table";
    fname = prepare_file_path(FGrp_StdData,"alpha.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    //_DK_init_alpha_table(); return true;
    // Loading file data
    if (LbFileLoadAt(fname, &alpha_sprite_table) != sizeof(struct TbAlphaTables))
    {
        compute_alpha_tables(&alpha_sprite_table,_DK_palette,_DK_palette);
        LbFileSaveAt(fname, &alpha_sprite_table, sizeof(struct TbAlphaTables));
    }
    return true;
}

TbBool init_rgb2idx_table(void)
{
    char *fname;
    static const char textname[] = "rgb-to-index color table";
    fname = prepare_file_path(FGrp_StdData,"colours.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &colours) != sizeof(TbRGBColorTable))
    {
        compute_rgb2idx_table(colours,_DK_palette);
        LbFileSaveAt(fname, &colours, sizeof(TbRGBColorTable));
    }
    return true;
}

TbBool init_redpal_table(void)
{
    char *fname;
    static const char textname[] = "red-blended color table";
    fname = prepare_file_path(FGrp_StdData,"redpal.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &red_pal) != 256)
    {
        compute_shifted_palette_table(red_pal, _DK_palette, _DK_palette, 20, -10, -10);
        LbFileSaveAt(fname, &red_pal, 256);
    }
    return true;
}

TbBool init_whitepal_table(void)
{
    char *fname;
    static const char textname[] = "white-blended color table";
    fname = prepare_file_path(FGrp_StdData,"whitepal.col");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    // Loading file data
    if (LbFileLoadAt(fname, &white_pal) != 256)
    {
        compute_shifted_palette_table(white_pal, _DK_palette, _DK_palette, 48, 48, 48);
        LbFileSaveAt(fname, &white_pal, 256);
    }
    return true;
}

void init_colours(void)
{
    //_DK_init_colours(); return true;
    init_rgb2idx_table();
    init_redpal_table();
    init_whitepal_table();
}

void setup_stuff(void)
{
    init_fades_table();
    init_alpha_table();
}

void powerful_magic_breaking_sparks(struct Thing *breaktng)
{
    struct Coord3d pos;
    pos.x.stl.num = breaktng->mappos.x.stl.num + ACTION_RANDOM(11) - 5;
    pos.y.stl.num = breaktng->mappos.y.stl.num + ACTION_RANDOM(11) - 5;
    pos.x.stl.pos = 128;
    pos.y.stl.pos = 128;
    pos.z.val = get_floor_height_at(&pos);
    draw_lightning(&breaktng->mappos, &pos, 96, 60);
    if ( !S3DEmitterIsPlayingSample(breaktng->snd_emitter_id, 157, 0) ) {
        thing_play_sample(breaktng, 157, 100, -1, 3, 1, 6, 256);
    }
}

void blast_slab(MapSlabCoord slb_x, MapSlabCoord slb_y, PlayerNumber plyr_idx)
{
    struct SlabMap *slb;
    slb = get_slabmap_block(slb_x, slb_y);
    if (slabmap_block_invalid(slb))
        return;
    //TODO rewrite used by process_dungeon_devastation_effects()
}

void process_devastate_dungeon_from_heart(PlayerNumber plyr_idx)
{
    //TODO rewrite used to remove dungeon slabs when heart is destroyed
    // probably near process_dungeon_devastation_effects()
}

void initialise_devastate_dungeon_from_heart(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(plyr_idx);
    if (dungeon->field_14B4 == 0)
    {
        struct Thing *heartng;
        heartng = INVALID_THING;
        if (dungeon->dnheart_idx > 0) {
            heartng = thing_get(dungeon->dnheart_idx);
            TRACE_THING(heartng);
        }
        if (thing_exists(heartng)) {
            dungeon->field_14B4 = 1;
            dungeon->field_14B2[0] = heartng->mappos.x.stl.num;
            dungeon->field_14B2[1] = heartng->mappos.y.stl.num;
        } else {
            dungeon->field_14B4 = 1;
            dungeon->field_14B2[0] = map_subtiles_x/2;
            dungeon->field_14B2[1] = map_subtiles_y/2;
        }
    }
}

/**
 * Destroys all slabs of given room, creating gold rubble effect in the place.
 * @param room The room structure which slabs are to be destroyed.
 * @note The room structure is freed before this function end.
 */
void destroy_room_leaving_unclaimed_ground(struct Room *room)
{
    long slb_x, slb_y;
    unsigned long k;
    long i;
    k = 0;
    i = room->slabs_list;
    while (i != 0)
    {
        slb_x = slb_num_decode_x(i);
        slb_y = slb_num_decode_y(i);
        i = get_next_slab_number_in_room(i);
        // Per room tile code
        if (room->owner != game.neutral_player_num)
        {
            struct Dungeon *dungeon;
            dungeon = get_players_num_dungeon(room->owner);
            dungeon->rooms_destroyed++;
        }
        delete_room_slab(slb_x, slb_y, 1); // Note that this function might also delete the whole room
        create_dirt_rubble_for_dug_slab(slb_x, slb_y);
        // Per room tile code ends
        k++;
        if (k > map_tiles_x*map_tiles_y) // we can't use room->slabs_count as room may be deleted
        {
            ERRORLOG("Room slabs list length exceeded when sweeping");
            break;
        }
    }
}

void destroy_dungeon_heart_room(PlayerNumber plyr_idx, const struct Thing *heartng)
{
    struct Dungeon *dungeon;
    long i;
    dungeon = get_dungeon(plyr_idx);
    struct Room *room;
    room = get_room_thing_is_on(heartng);
    if (room_is_invalid(room) || (room->kind != RoK_DUNGHEART))
    {
        WARNLOG("The heart thing is not in heart room");
        i = dungeon->room_kind[RoK_DUNGHEART];
        room = room_get(i);
    }
    if (room_is_invalid(room))
    {
        ERRORLOG("Tried to destroy heart for player who doesn't have one");
        return;
    }
    if (dungeon->room_kind[RoK_DUNGHEART] == room->index) {
        dungeon->room_kind[RoK_DUNGHEART] = room->next_of_kind;
    }
    destroy_room_leaving_unclaimed_ground(room);
}

void setup_all_player_creatures_and_diggers_leave_or_die(PlayerNumber plyr_idx)
{
    struct Dungeon *dungeon;
    struct CreatureControl *cctrl;
    struct Thing *thing;
    unsigned long k;
    int i;
    if ((plyr_idx == game.hero_player_num) || (plyr_idx == game.neutral_player_num)) {
        // Don't affect heroes and neutral creatures
        return;
    }
    dungeon = get_dungeon(plyr_idx);
    // Force leave or kill normal creatures
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!creature_is_manually_controlled_by_owner(thing))
        {
            if (!creature_is_kept_in_custody_by_enemy(thing))
            {
                setup_creature_leaves_or_dies(thing);
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
    // Kill all special workers
    k = 0;
    i = dungeon->digger_list_start;
    while (i != 0)
    {
        thing = thing_get(i);
        cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (!creature_is_kept_in_custody_by_enemy(thing))
        {
            kill_creature(thing, INVALID_THING, -1, 0, 0, 0);
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

void process_dungeon_destroy(struct Thing *heartng)
{
    struct Dungeon *dungeon;
    long plyr_idx;
    plyr_idx = heartng->owner;
    //_DK_process_dungeon_destroy(thing);
    dungeon = get_dungeon(plyr_idx);
    if (!dungeon->field_1060) {
        return;
    }
    powerful_magic_breaking_sparks(heartng);
    const struct Coord3d *central_pos;
    central_pos = &heartng->mappos;
    switch ( dungeon->field_1060 )
    {
    case 1:
        initialise_devastate_dungeon_from_heart(plyr_idx);
        dungeon->field_1061++;
        if (dungeon->field_1061 < 32)
        {
            if ( ACTION_RANDOM(96) < (dungeon->field_1061 << 6) / 32 + 32 ) {
                create_effect(central_pos, 44, plyr_idx);
            }
        } else
        { // Got to next phase
            dungeon->field_1060 = 2;
            dungeon->field_1061 = 0;
        }
        break;
    case 2:
        dungeon->field_1061++;
        if (dungeon->field_1061 < 32)
        {
            create_effect(central_pos, 44, plyr_idx);
        } else
        { // Got to next phase
            dungeon->field_1060 = 3;
            dungeon->field_1061 = 0;
        }
        break;
    case 3:
        // Drop all held things
        if (dungeon->num_things_in_hand > 0)
        {
            dump_held_things_on_map(plyr_idx, central_pos->x.stl.num, central_pos->y.stl.num, 0);
        }
        // Got to next phase
        dungeon->field_1060 = 4;
        dungeon->field_1061 = 0;
        break;
    case 4:
        // Final phase - destroy the heart, both pedestal room and container thing
        setup_all_player_creatures_and_diggers_leave_or_die(plyr_idx);
        {
            struct Thing *efftng;
            efftng = create_effect(central_pos, 4, plyr_idx);
            if (!thing_is_invalid(efftng))
              efftng->byte_16 = 8;
            efftng = create_effect(central_pos, 14, plyr_idx);
            if (!thing_is_invalid(efftng))
                efftng->byte_16 = 8;
            destroy_dungeon_heart_room(plyr_idx, heartng);
            delete_thing_structure(heartng, 0);
            { // If there is another heart owned by this player, set it to "working" heart
                struct PlayerInfo *player;
                player = get_player(plyr_idx);
                init_player_start(player);
            }
        }
        dungeon->field_1060 = 0;
        dungeon->field_1061 = 0;
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
    struct CreatureStats *crstat;
    long max_health,health;
    if ( !thing_exists(shooter) )
        return;
    crstat = creature_stats_get_from_thing(shooter);
    cctrl = creature_control_get_from_thing(shooter);
    max_health = compute_creature_max_health(crstat->health, cctrl->explevel);
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
    n = get_creature_breed_graphics(thing->model, 1);
    i = convert_td_iso(n);
    if (i != thing->field_44)
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
        long amp,direction;
        long delta_x,delta_y;
        amp = 5 * thing->sizexy / 8;
        direction = ACTION_RANDOM(2*LbFPMath_PI);
        delta_x = (amp * LbSinL(direction) >> 8);
        delta_y = (amp * LbCosL(direction) >> 8);
        pos.x.val = thing->mappos.x.val + (delta_x >> 8);
        pos.y.val = thing->mappos.y.val - (delta_y >> 8);
        pos.z.val = thing->mappos.z.val;
        create_effect_element(&pos, 0x2D, thing->owner);
    }
}

long process_creature_self_spell_casting(struct Thing *thing)
{
    return _DK_process_creature_self_spell_casting(thing);
}

unsigned long lightning_is_close_to_player(struct PlayerInfo *player, struct Coord3d *pos)
{
    return _DK_lightning_is_close_to_player(player, pos);
}

void affect_nearby_enemy_creatures_with_wind(struct Thing *thing)
{
    _DK_affect_nearby_enemy_creatures_with_wind(thing);
}

void affect_nearby_stuff_with_vortex(struct Thing *thing)
{
  _DK_affect_nearby_stuff_with_vortex(thing);
}

void affect_nearby_friends_with_alarm(struct Thing *thing)
{
    _DK_affect_nearby_friends_with_alarm(thing);
}

long apply_wallhug_force_to_boulder(struct Thing *thing)
{
  return _DK_apply_wallhug_force_to_boulder(thing);
}

void draw_flame_breath(struct Coord3d *pos1, struct Coord3d *pos2, long a3, long a4)
{
  _DK_draw_flame_breath(pos1, pos2, a3, a4);
}

void draw_lightning(const struct Coord3d *pos1, const struct Coord3d *pos2, long a3, long a4)
{
  _DK_draw_lightning(pos1, pos2, a3, a4);
}

unsigned long setup_move_off_lava(struct Thing *thing)
{
  return _DK_setup_move_off_lava(thing);
}

TbBool any_player_close_enough_to_see(const struct Coord3d *pos)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if ( (player_exists(player)) && ((player->field_0 & 0x40) == 0))
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
          if (thing->field_4F & 0x40)
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
      thing->field_46 += thing->field_4A;
      if (thing->field_46 > thing->field_4B)
      {
        if (thing->field_46 >= thing->field_4D)
        {
          thing->field_46 = thing->field_4D;
          if (thing->field_50 & 0x02)
            thing->field_4A = -thing->field_4A;
          else
            thing->field_4A = 0;
        }
      } else
      {
        thing->field_46 = thing->field_4B;
        if ((thing->field_50 & 0x02) != 0)
          thing->field_4A = -thing->field_4A;
        else
          thing->field_4A = 0;
      }
    }
}

/**
 * Plays a smacker animation file and sets frontend state to nstate.
 * @param nstate Frontend state; -1 means no change, -2 means don't even
 *    change screen mode.
 * @return Returns false if fatal error occurred and program execution should end.
 */
short play_smacker_file(char *filename, int nstate)
{
  unsigned int movie_flags = 0;
  if ( SoundDisabled )
    movie_flags |= 0x01;
  short result;

  result = 1;
  if ((result)&&(nstate>-2))
  {
    if ( setup_screen_mode_minimal(get_movies_vidmode()) )
    {
      LbMouseChangeSprite(NULL);
      LbScreenClear(0);
      LbScreenSwap();
    } else
    {
      ERRORLOG("Can't enter movies video mode to play a Smacker file");
      result=0;
    }
  }
  if (result)
  {
    // Fail in playing shouldn't set result=0, because result=0 means fatal error.
    if (play_smk_(filename, 0, movie_flags | 0x100) == 0)
    {
      ERRORLOG("Smacker play error");
      result=0;
    }
  }
  if (nstate>-2)
  {
    if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
    {
      ERRORLOG("Can't re-enter frontend video mode after playing Smacker file");
      FatalError = 1;
      exit_keeper = 1;
      return 0;
    }
  } else
  {
    memset(frontend_palette, 0, PALETTE_SIZE);
  }
  LbScreenClear(0);
  LbScreenSwap();
  LbPaletteSet(frontend_palette);
  if (nstate >= 0)
    frontend_set_state(nstate);
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
  lbDisplay.MiddleButton = 0;
  if (nstate > -2)
    LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
  return result;
}

void init_censorship(void)
{
  if ( censorship_enabled() )
  {
    // Modification for Dark Mistress
      set_creature_breed_graphics(20, 14, 48);
  }
}

long load_anim_file(void)
{
    SYNCDBG(8,"Starting");
    //return _DK_load_anim_file();
    char *fname;
    static const char textname[] = "animated tmap";
    fname = prepare_file_path(FGrp_StdData,"tmapanim.dat");
    SYNCDBG(0,"Reading %s file \"%s\".",textname,fname);
    if (LbFileLoadAt(fname, game.texture_animation) != sizeof(game.texture_animation))
    {
        return false;
    }
    return true;
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
    load_stats_files();
    check_and_auto_fix_stats();
    init_creature_scores();
    load_cube_file();
    init_top_texture_to_cube_table();
    load_anim_file();
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
    game.creatures_tend_1 = 0;
    game.creatures_tend_2 = 0;
    game.numfield_C |= 0x40;
    game.numfield_D |= (0x20 | 0x40);
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
    if (LbDataLoadAll(game_load_files))
    {
      ERRORLOG("Unable to load game_load_files");
      return false;
    }
    // was LoadMcgaData, but minimal should be enough at this point.
    if ( !LoadMcgaDataMinimal() )
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
  char *fname;
  short result;

  // Do only a very basic setup
  cpu_detect(&cpu_info);
  SYNCMSG("CPU %s type %d family %d model %d stepping %d features %08x",cpu_info.vendor,
      (int)cpu_get_type(&cpu_info),(int)cpu_get_family(&cpu_info),(int)cpu_get_model(&cpu_info),
      (int)cpu_get_stepping(&cpu_info),cpu_info.feature_edx);
  update_memory_constraits();
  // Enable features thar require more resources
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
      fname = prepare_file_path(FGrp_FxData,"bullfrog.smk");
      result = play_smacker_file(fname, -2);
      if ( !result )
        ERRORLOG("Unable to play new moon movie");
    }

  result = 1;
  // The 320x200 mode is required only for the intro;
  // loading and no CD screens can run in both 320x2?0 and 640x4?0.
  if ( result && (!game.no_intro) )
  {
      LbPaletteDataFillBlack(_DK_palette);
      int mode_ok = LbScreenSetup(get_movies_vidmode(), 320, 200, _DK_palette, 2, 0);
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
  if ( result && (!game.no_intro) )
  {
      fname = prepare_file_path(FGrp_LoData,"intromix.smk");
      result = play_smacker_file(fname, -2);
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
      SetRedbookVolume(settings.redbook_volume);
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

void init_messages(void)
{
    //_DK_init_messages();
    clear_messages();
    // Set end turn
    init_messages_turns(0);
}

void zero_messages(void)
{
    int i;
    game.active_messages_count = 0;
    for (i=0; i<3; i++)
    {
      memset(&game.messages[i], 0, sizeof(struct GuiMessage));
    }
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
    if ( (i == PSt_CtrlDungeon) && (player->field_454 != 0) && (player->thing_under_hand == 0) )
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
              *map_x = ( top_pointed_at_x << 8) + top_pointed_at_frac_x;
              *map_y = ( top_pointed_at_y << 8) + top_pointed_at_frac_y;
        } else
        {
              *map_x = (block_pointed_at_x << 8) + pointed_at_frac_x;
              *map_y = (block_pointed_at_y << 8) + pointed_at_frac_y;
        }
        // Clipping coordinates
        if (*map_y < 0)
          *map_y = 0;
        else
        if (*map_y > ((map_subtiles_y<<8)-1))
          *map_y = ((map_subtiles_y<<8)-1);
        if (*map_x < 0)
          *map_x = 0;
        else
        if (*map_x > ((map_subtiles_x<<8)-1))
          *map_x = ((map_subtiles_x<<8)-1);
        return true;
    }
    return false;
}

short point_to_overhead_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
  *map_x = 0;
  *map_y = 0;
  if ((screen_x >= 150) && (screen_x < 490)
    && (screen_y >= 56) && (screen_y < 396))
  {
    *map_x = 3*256 * (screen_x-150) / 4 + 384;
    *map_y = 3*256 * (screen_y-56) / 4 + 384;
    return ((*map_x >= 0) && (*map_x < (map_subtiles_x+1)<<8) && (*map_y >= 0) && (*map_y < (map_subtiles_y+1)<<8));
  }
  return false;
}

TbBool screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos)
{
  TbBool result;
  long x,y;
  //SYNCDBG(19,"Starting");
  result = false;
  if (camera != NULL)
  {
    switch (camera->field_6)
    {
      case 1:
      case 2:
      case 5:
        // 3D view mode
        result = engine_point_to_map(camera,screen_x,screen_y,&x,&y);
        break;
      case 3: //map mode
        result = point_to_overhead_map(camera,screen_x,screen_y,&x,&y);
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
  //SYNCDBG(19,"Finished");
  return result;
}

void update_breed_activities(void)
{
  _DK_update_breed_activities();
}

void toggle_hero_health_flowers(void)
{
  const char *statstr;
  toggle_flag_byte(&game.flags_cd,0x80);
  if (game.flags_cd & 0x80)
  {
    statstr = "off";
  } else
  {
    do_sound_menu_click();
    statstr = "on";
  }
  show_onscreen_msg(2*game.num_fps, "Hero health flowers %s", statstr);
}

void zoom_to_map(void)
{
  struct PlayerInfo *player;
  turn_off_all_window_menus();
  if ((game.numfield_C & 0x20) == 0)
    set_flag_byte(&game.numfield_C,0x40,false);
  else
    set_flag_byte(&game.numfield_C,0x40,true);
  player=get_my_player();
  if (((game.system_flags & GSF_NetworkActive) != 0)
      || (lbDisplay.PhysicalScreenWidth > 320))
  {
    if (!toggle_status_menu(false))
      set_flag_byte(&game.numfield_C,0x40,false);
    set_players_packet_action(player, PckA_Unknown119, 4, 0, 0, 0);
    turn_off_roaming_menus();
  } else
  {
    set_players_packet_action(player, PckA_Unknown080, 5, 0, 0, 0);
    turn_off_roaming_menus();
  }
}

void zoom_from_map(void)
{
  struct PlayerInfo *player;
  player=get_my_player();
  if (((game.system_flags & GSF_NetworkActive) != 0)
      || (lbDisplay.PhysicalScreenWidth > 320))
  {
      if ((game.numfield_C & 0x40) != 0)
        toggle_status_menu(true);
      set_players_packet_action(player, PckA_Unknown120,1,0,0,0);
  } else
  {
      set_players_packet_action(player, PckA_Unknown080,6,0,0,0);
  }
}

void reset_gui_based_on_player_mode(void)
{
  _DK_reset_gui_based_on_player_mode();
}

void reinit_tagged_blocks_for_player(unsigned char idx)
{
  _DK_reinit_tagged_blocks_for_player(idx);
}

void instant_instance_selected(long a1)
{
    _DK_instant_instance_selected(a1);
}

TbBool create_random_evil_creature(long x, long y, PlayerNumber owner, long max_lv)
{
    struct Thing *thing;
    struct Coord3d pos;
    long i;
    i = ACTION_RANDOM(17) + 14;
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    thing = create_creature(&pos, i, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create evil creature type %ld at (%ld,%ld)",i,x,y);
        return false;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Evil creature type %ld at (%ld,%ld) deleted because is in wall",i,x,y);
        return false;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    set_start_state(thing);
    i = ACTION_RANDOM(max_lv);
    set_creature_level(thing, i);
    return true;
}

TbBool create_random_hero_creature(long x, long y, PlayerNumber owner, long max_lv)
{
  struct Thing *thing;
  struct Coord3d pos;
  long i;
  i = ACTION_RANDOM(13) + 1;
  pos.x.val = x;
  pos.y.val = y;
  pos.z.val = 0;
  thing = create_creature(&pos, i, owner);
  if (thing_is_invalid(thing))
  {
      ERRORLOG("Cannot create player %d hero model %ld at (%ld,%ld)",(int)owner,i,x,y);
      return false;
  }
  pos.z.val = get_thing_height_at(thing, &pos);
  if (thing_in_wall_at(thing, &pos))
  {
      delete_thing_structure(thing, 0);
      ERRORLOG("Hero model %ld at (%ld,%ld) deleted because is in wall",i,x,y);
      return false;
  }
  thing->mappos.x.val = pos.x.val;
  thing->mappos.y.val = pos.y.val;
  thing->mappos.z.val = pos.z.val;
  remove_first_creature(thing);
  set_first_creature(thing);
//  set_start_state(thing); - simplified to the following two commands
  game.field_14E498 = game.play_gameturn;
  game.field_14E49C++;
  i = ACTION_RANDOM(max_lv);
  set_creature_level(thing, i);
  return true;
}

TbBool create_hero_special_worker(MapCoord x, MapCoord y, PlayerNumber owner)
{
    struct Thing *thing;
    struct Coord3d pos;
    long i;
    i = 8;
    pos.x.val = x;
    pos.y.val = y;
    pos.z.val = 0;
    thing = create_creature(&pos, i, owner);
    if (thing_is_invalid(thing))
    {
        ERRORLOG("Cannot create hero creature type %ld at (%ld,%ld)",i,x,y);
        return false;
    }
    pos.z.val = get_thing_height_at(thing, &pos);
    if (thing_in_wall_at(thing, &pos))
    {
        delete_thing_structure(thing, 0);
        ERRORLOG("Hero creature type %ld at (%ld,%ld) deleted because is in wall",i,x,y);
        return false;
    }
    thing->mappos.x.val = pos.x.val;
    thing->mappos.y.val = pos.y.val;
    thing->mappos.z.val = pos.z.val;
    remove_first_creature(thing);
    set_first_creature(thing);
    return true;
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
  set_players_packet_action(player, PckA_Unknown087, thing->mappos.x.val, thing->mappos.y.val, 0, 0);
  return true;
}

void go_to_my_next_room_of_type(unsigned long rkind)
{
  _DK_go_to_my_next_room_of_type(rkind);
}

short toggle_computer_player(int idx)
{
  struct Dungeon *dungeon;
  dungeon = get_players_num_dungeon(idx);
  if (dungeon_invalid(dungeon))
    return false;
  toggle_flag_byte(&dungeon->computer_enabled,0x01);
  return true;
}

TbBool load_texture_map_file(unsigned long tmapidx, unsigned char n)
{
    char *fname;
    SYNCDBG(7,"Starting");
    fname = prepare_file_fmtpath(FGrp_StdData,"tmapa%03d.dat",tmapidx);
    if (!wait_for_cd_to_be_available())
        return false;
    if (!LbFileExists(fname))
    {
        WARNMSG("Texture file \"%s\" doesn't exits.",fname);
        return false;
    }
    // The texture file has always over 500kb
    if (LbFileLoadAt(fname, block_mem) < 65536)
    {
        WARNMSG("Texture file \"%s\" can't be loaded or is too small.",fname);
        return false;
    }
    return true;
}

void reinit_level_after_load(void)
{
    struct PlayerInfo *player;
    int i;
    SYNCDBG(6,"Starting");
    player = get_my_player();
    player->field_7 = 0;
    init_lookups();
    init_navigation();
    reinit_packets_after_load();
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
    load_computer_player_config(CnfLd_Standard);
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
    start_params.packet_checksum = 1;
    set_flag_byte(&start_params.flags_font,FFlg_unk01,false);
    // Set levels to 0, as we may not have the campaign loaded yet
    start_params.selected_level_number = 0;
    start_params.num_fps = 20;
    start_params.one_player = 1;
    set_flag_byte(&start_params.flags_cd,MFlg_IsDemoMode,false);
    set_flag_byte(&start_params.flags_cd,MFlg_unk40,true);
    start_params.force_ppro_poly = 0;
    return true;
}

/**
 * Clears the Game structure completely, and copies statrup parameters
 * from start_params structure.
 */
void clear_complete_game(void)
{
    memset(&game, 0, sizeof(struct Game));
    memset(&gameadd, 0, sizeof(struct GameAdd));
    game.turns_packetoff = -1;
    game.numfield_149F46 = 0;
    game.packet_checksum = start_params.packet_checksum;
    game.numfield_1503A2 = -1;
    game.flags_font = start_params.flags_font;
    game.numfield_149F47 = 0;
    // Set levels to 0, as we may not have the campaign loaded yet
    set_continue_level_number(first_singleplayer_level());
    if ((start_params.numfield_C & 0x02) != 0)
      set_selected_level_number(start_params.selected_level_number);
    else
      set_selected_level_number(first_singleplayer_level());
    game.num_fps = start_params.num_fps;
    game.flags_cd = start_params.flags_cd;
    game.no_intro = start_params.no_intro;
    set_flag_byte(&game.system_flags,GSF_AllowOnePlayer,start_params.one_player);
  //  game.one_player = start_params.one_player;
    game.numfield_C = start_params.numfield_C;
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
        thing->mappos.x.val = get_subtile_center_pos(map_subtiles_x/2);
        thing->mappos.y.val = get_subtile_center_pos(map_subtiles_y/2);
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
      memset(&game.computer_task[i], 0, sizeof(struct ComputerTask));
    }
    for (i=0; i < GOLD_LOOKUP_COUNT; i++)
    {
      memset(&game.gold_lookup[i], 0, sizeof(struct GoldLookup));
    }
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      memset(&game.computer[i], 0, sizeof(struct Computer2));
    }
}

void init_keepers_map_exploration(void)
{
    struct PlayerInfo *player;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          if ((player->field_0 & 0x40) != 0)
            init_keeper_map_exploration(player);
      }
    }
}

void clear_players_for_save(void)
{
    struct PlayerInfo *player;
    unsigned short id_mem,mem2,memflg;
    struct Camera cammem;
    int i;
    for (i=0; i < PLAYERS_COUNT; i++)
    {
      player = get_player(i);
      id_mem = player->id_number;
      mem2 = player->field_2C;
      memflg = player->field_0;
      LbMemoryCopy(&cammem,&player->cameras[1],sizeof(struct Camera));
      memset(player, 0, sizeof(struct PlayerInfo));
      player->id_number = id_mem;
      player->field_2C = mem2;
      set_flag_byte(&player->field_0,0x01,((memflg & 0x01) != 0));
      set_flag_byte(&player->field_0,0x40,((memflg & 0x40) != 0));
      LbMemoryCopy(&player->cameras[1],&cammem,sizeof(struct Camera));
      player->acamera = &player->cameras[1];
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

void clear_game_for_summary(void)
{
    SYNCDBG(6,"Starting");
    delete_all_structures();
    clear_shadow_limits(&game.lish);
    clear_stat_light_map();
    clear_mapwho();
    game.entrance_room_id = 0;
    game.action_rand_seed = 0;
    set_flag_byte(&game.numfield_C,0x04,false);
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
    set_flag_byte(&game.numfield_C,0x04,false);
    clear_columns();
    clear_players_for_save();
    clear_dungeons();
}

void reset_creature_max_levels(void)
{
    struct Dungeon *dungeon;
    int i,k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        for (k=1; k < CREATURE_TYPES_COUNT; k++)
        {
            dungeon = get_dungeon(i);
            dungeon->creature_max_level[k] = CREATURE_MAX_LEVEL+1;
        }
    }
}

void reset_script_timers_and_flags(void)
{
    struct Dungeon *dungeon;
    int i,k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
      dungeon = get_dungeon(i);
      dungeon->magic_resrchable[PwrK_POSSESS] = 1;
      dungeon->magic_level[PwrK_POSSESS] = 1;
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
    player->field_0 |= 0x01;
    player->field_0 |= 0x40;
    player->id_number = game.hero_player_num;
}

void message_add(char c)
{
    _DK_message_add(c);
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
    if (pal == blue_palette)
    {
      if ((player->field_3 & 0x04) == 0)
        return;
      player->field_3 |= 0x04;
    } else
    {
      player->field_3 &= 0xFB;
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
      result = (LbFileLoadAt(fname, _DK_palette) != -1);
    }
    if ((result) && (do_set))
    {
      struct PlayerInfo *myplyr;
      myplyr=get_my_player();
      PaletteSetPlayerPalette(myplyr, _DK_palette);
    }
    if (!result)
      ERRORLOG("Can't load palette file.");
    return result;
}

void centre_engine_window(void)
{
    long x1,y1;
    struct PlayerInfo *player=get_my_player();
    if ((game.numfield_C & 0x20) != 0)
      x1 = (MyScreenWidth-player->engine_window_width-status_panel_width) / 2 + status_panel_width;
    else
      x1 = (MyScreenWidth-player->engine_window_width) / 2;
    y1 = (MyScreenHeight-player->engine_window_height) / 2;
    setup_engine_window(x1, y1, player->engine_window_width, player->engine_window_height);
}

void turn_off_query(short a)
{
  _DK_turn_off_query(a);
}

long set_autopilot_type(PlayerNumber plyr_idx, long aptype)
{
  return _DK_set_autopilot_type(plyr_idx, aptype);
}

void level_lost_go_first_person(PlayerNumber plyr_idx)
{
  struct CreatureControl *cctrl;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Thing *thing;
  long spectator_breed;
  SYNCDBG(6,"Starting for player %ld",plyr_idx);
  //_DK_level_lost_go_first_person(plridx);
  player = get_player(plyr_idx);
  dungeon = get_dungeon(player->id_number);
  spectator_breed = get_players_spectator_breed(plyr_idx);
  player->dungeon_camera_zoom = get_camera_zoom(player->acamera);
  thing = create_and_control_creature_as_controller(player, spectator_breed, &dungeon->mappos);
  if (thing_is_invalid(thing))
  {
    ERRORLOG("Unable to create spectator creature");
    return;
  }
  cctrl = creature_control_get_from_thing(thing);
  cctrl->flgfield_1 |= CCFlg_NoCompControl;
  SYNCDBG(8,"Finished");
}

void find_map_location_coords(long location, long *x, long *y, const char *func_name)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct ActionPoint *apt;
  struct Thing *thing;
  long pos_x,pos_y;
  long i;
  SYNCDBG(15,"From %s; Location %ld, pos(%ld,%ld)",func_name, location, *x, *y);
  pos_y = 0;
  pos_x = 0;
  switch (get_map_location_type(location))
  {
  case MLoc_ACTIONPOINT:
      i = get_map_location_longval(location);
      apt = action_point_get_by_number(i);
      if (!action_point_is_invalid(apt))
      {
        pos_y = apt->mappos.y.stl.num;
        pos_x = apt->mappos.x.stl.num;
      } else
        WARNMSG("%s: Action Point %d location for not found",func_name,i);
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
        player = get_player(i);
        dungeon = get_dungeon(player->id_number);
        thing = thing_get(dungeon->dnheart_idx);
      } else
        thing = NULL;
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
  case MLoc_CREATUREBREED:
  case MLoc_OBJECTKIND:
  case MLoc_ROOMKIND:
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
  long pos_x,pos_y;
  player = get_my_player();
  find_map_location_coords(target, &x, &y, __func__);
  pos_x = 0;
  pos_y = 0;
  if ((x != 0) || (y != 0))
  {
    pos_y = (y << 8) + 128;
    pos_x = (x << 8) + 128;
  }
  event_create_event(pos_x, pos_y, EvKind_Information, player->id_number, -msg_id);
}

void set_quick_information(long msg_id, long target, long x, long y)
{
    struct PlayerInfo *player;
    long pos_x,pos_y;
    player = get_my_player();
    find_map_location_coords(target, &x, &y, __func__);
    pos_x = 0;
    pos_y = 0;
    if ((x != 0) || (y != 0))
    {
      pos_y = (y << 8) + 128;
      pos_x = (x << 8) + 128;
    }
    event_create_event(pos_x, pos_y, EvKind_QuickInformation, player->id_number, -msg_id);
}

void set_general_objective(long msg_id, long target, long x, long y)
{
    process_objective(cmpgn_string(msg_id), target, x, y);
}

void process_objective(const char *msg_text, long target, long x, long y)
{
    struct PlayerInfo *player;
    long pos_x,pos_y;
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
    int i,k,n;
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
        if (swplyr->field_2C == 1)
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

void place_animating_slab_type_on_map(long a1, char a2, unsigned char a3, unsigned char a4, unsigned char a5)
{
    SYNCDBG(7,"Starting");
    _DK_place_animating_slab_type_on_map(a1,a2,a3,a4,a5);
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
  _DK_set_mouse_light(player);
}

void check_players_won(void)
{
  SYNCDBG(8,"Starting");
  _DK_check_players_won();
}

void check_players_lost(void)
{
  long i;
  SYNCDBG(8,"Starting");
  //_DK_check_players_lost();
  for (i=0; i < PLAYERS_COUNT; i++)
  {
      struct PlayerInfo *player;
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          struct Dungeon *dungeon;
          struct Thing *heartng;
          dungeon = get_players_dungeon(player);
          heartng = thing_get(dungeon->dnheart_idx);
          if ((thing_is_invalid(heartng) || (heartng->active_state == ObSt_State3)) && (player->victory_state == VicS_Undecided))
          {
            event_kill_all_players_events(i);
            set_player_as_lost_level(player);
            //TODO: make sure we really want to do this; it wasn't here in oroginal code, but it will prevent computer player activities on dead player.
            player->field_2C = 0;
            if (is_my_player_number(i))
              LbPaletteSet(_DK_palette);
          }
      }
  }
}

void process_dungeon_power_magic(void)
{
    SYNCDBG(8,"Starting");
    _DK_process_dungeon_power_magic();
}

void process_dungeon_devastation_effects(void)
{
    SYNCDBG(8,"Starting");
    _DK_process_dungeon_devastation_effects();
}

void process_payday(void)
{
  _DK_process_payday();
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

/**
 * Returns if there is a bonus timer visible on the level.
 */
TbBool bonus_timer_enabled(void)
{
  return ((game.flags_gui & GGUI_CountdownTimer) != 0);
/*  LevelNumber lvnum;
  lvnum = get_loaded_level_number();
  return (is_bonus_level(lvnum) || is_extra_level(lvnum));*/
}

struct Room *player_has_room_of_type(long plyr_idx, long rkind)
{
  return _DK_player_has_room_of_type(plyr_idx, rkind);
}

void process_level_script(void)
{
  SYNCDBG(6,"Starting");
  struct PlayerInfo *player;
  player = get_my_player();
  if (((game.system_flags & GSF_NetworkActive) == 0)
      && (player->victory_state != VicS_Undecided))
    return;
  process_conditions();
  process_check_new_creature_partys();
//script_process_messages(); is not here, but it is in beta - check why
  process_check_new_tunneller_partys();
  process_values();
  process_win_and_lose_conditions(my_player_number); //player->id_number may be uninitialized yet
//  show_onscreen_msg(8, "Flags %d %d %d %d %d %d", game.dungeon[0].script_flags[0],game.dungeon[0].script_flags[1],
//    game.dungeon[0].script_flags[2],game.dungeon[0].script_flags[3],game.dungeon[0].script_flags[4],game.dungeon[0].script_flags[5]);
  SYNCDBG(19,"Finished");
}

void message_update(void)
{
  SYNCDBG(6,"Starting");
  _DK_message_update();
}

long wp_check_map_pos_valid(struct Wander *wandr, long a1)
{
    return _DK_wp_check_map_pos_valid(wandr, a1);
}

long wander_point_update(struct Wander *wandr)
{
    long array[20];
    double realidx,delta;
    long tile1,tile2;
    long slb_num,valid_num,idx;
    long i;
    //return _DK_wander_point_update(wandr);
    valid_num = 0;
    slb_num = wandr->field_8;
    tile1 = 85;
    tile2 = slb_num_decode_x(slb_num);
    idx = 0;
    for (i = 0; i < wandr->field_C; i++)
    {
        if (wp_check_map_pos_valid(wandr, tile1))
        {
          if (valid_num >= 20)
              break;
          valid_num++;
          array[idx] = tile1;
          idx++;
          if (((wandr->field_14 & 0xFF) != 0) && (wandr->field_10 == valid_num))
          {
              slb_num = (wandr->field_C + wandr->field_8) % (map_tiles_x*map_tiles_y);
              break;
          }
        }
        slb_num++;
        tile2++;
        if (tile2 < map_tiles_x)
        {
          tile1 += 3;
        } else
        {
          tile2 = 0;
          if (slb_num >= map_tiles_x*map_tiles_y)
            slb_num = 0;
          tile1 = get_subtile_number(slab_subtile_center(slb_num_decode_x(slb_num)), slab_subtile_center(slb_num_decode_y(slb_num)));
        }
    }
    wandr->field_8 = slb_num;
    if (valid_num <= 0)
        return 1;
    if (valid_num > wandr->field_10)
    {
      if (wandr->field_10 <= 0)
          return 1;
      wandr->field_4 %= 200;
      delta = (double)valid_num / (double)wandr->field_10;
      realidx = 0.0;
      for (i = 0; i < wandr->field_10; i++)
      {
          tile1 = array[(unsigned int)realidx];
          tile2 = ((tile1 >> 24) & 0xFF);
          wandr->field_18[2 * wandr->field_4] = (tile2 ^ ((tile2 ^ tile1) - tile2)) - tile2;
          wandr->field_18[2 * wandr->field_4 + 1] = (tile1 >> 8) & 0xFF;
          wandr->field_4 = (wandr->field_4 + 1) % 200;
          if (wandr->field_0 < 200)
            wandr->field_0++;
          realidx += delta;
      }
    } else
    {
        idx = 0;
        while (valid_num > 0)
        {
            tile1 = array[idx];
            wandr->field_18[2 * wandr->field_4] = abs(tile1);
            wandr->field_18[2 * wandr->field_4 + 1] = (tile1 >> 8) & 0xFF;
            wandr->field_4 = (wandr->field_4 + 1) % 200;
            if (wandr->field_0 < 200)
              wandr->field_0++;
            idx++;
            valid_num--;
        }
    }
    return 1;
}

void update_player_camera(struct PlayerInfo *player)
{
  _DK_update_player_camera(player);
}

void update_research(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          process_player_research(i);
      }
  }
}

void update_manufacturing(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting");
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
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
    if (player_exists(player) && ((player->field_0 & 0x40) == 0))
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

void update_footsteps_nearest_camera(struct Camera *camera)
{
  SYNCDBG(6,"Starting");
  if (camera == NULL)
    return;
  _DK_update_footsteps_nearest_camera(camera);
}

void process_player_states(void)
{
  SYNCDBG(6,"Starting");
  _DK_process_player_states();
}

void set_level_objective(const char *msg_text)
{
    if (msg_text == NULL)
    {
        ERRORLOG("Invalid message pointer");
        return;
    }
    strncpy(game.evntbox_text_objective, msg_text, MESSAGE_TEXT_LEN);
    new_objective = 1;
}

void update_player_objectives(int plridx)
{
  struct PlayerInfo *player;
  SYNCDBG(6,"Starting for player %d",plridx);
  player = get_player(plridx);
  if ((game.system_flags & GSF_NetworkActive) != 0)
  {
    if ((!player->field_4EB) && (player->victory_state != VicS_Undecided))
      player->field_4EB = game.play_gameturn+1;
  }
  if (player->field_4EB == game.play_gameturn)
  {
    switch (player->victory_state)
    {
    case VicS_WonLevel:
        if (plridx == my_player_number)
          set_level_objective(cmpgn_string(CpgStr_SuccessLandIsYours));
        display_objectives(player->id_number, 0, 0);
        break;
    case VicS_LostLevel:
        if (plridx == my_player_number)
          set_level_objective(cmpgn_string(CpgStr_LevelLost));
        display_objectives(player->id_number, 0, 0);
        break;
    }
  }
}

void process_players(void)
{
  int i;
  struct PlayerInfo *player;
  SYNCDBG(5,"Starting");
  process_player_instances();
  process_player_states();
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player = get_player(i);
      if (player_exists(player) && (player->field_2C == 1))
      {
          SYNCDBG(6,"Doing updates for player %d",i);
          wander_point_update(&player->wandr1);
          wander_point_update(&player->wandr2);
          update_power_sight_explored(player);
          update_player_objectives(i);
      }
  }
  SYNCDBG(17,"Finished");
}

short update_animating_texture_maps(void)
{
  int i;
  SYNCDBG(18,"Starting");
  anim_counter = (anim_counter+1) % 8;
  short result=true;
  for (i=0; i<TEXTURE_BLOCKS_ANIM_COUNT; i++)
  {
        short j = game.texture_animation[8*i+anim_counter];
        if ((j>=0) && (j<TEXTURE_BLOCKS_STAT_COUNT))
        {
          block_ptrs[TEXTURE_BLOCKS_STAT_COUNT+i] = block_ptrs[j];
        } else
        {
          result=false;
        }
  }
  return result;
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
      memset((char *)dungeon->field_64, 0, 480 * sizeof(short));
      memset((char *)dungeon->job_breeds_count, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
      memset((char *)dungeon->field_4E4, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
  }
  return i;
}

long update_cave_in(struct Thing *thing)
{
  return _DK_update_cave_in(thing);
}

void update(void)
{
    struct PlayerInfo *player;
    SYNCDBG(4,"Starting for turn %ld",(long)game.play_gameturn);

    if ((game.numfield_C & 0x01) == 0)
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

    if ((game.numfield_C & 0x01) == 0)
    {
        player = get_my_player();
        if (player->field_3 & 0x08)
        {
            PaletteSetPlayerPalette(player, _DK_palette);
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
        if ((game.numfield_D & 0x04) != 0)
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

void draw_spell_cursor(unsigned char wrkstate, unsigned short tng_idx, unsigned char stl_x, unsigned char stl_y)
{
    struct PlayerInfo *player;
    struct Thing *thing;
    struct SpellData *pwrdata;
    struct MagicStats *magstat;
    Expand_Check_Func chkfunc;
    TbBool allow_cast;
    long spl_id;
    long i;
    //_DK_draw_spell_cursor(wrkstate, tng_idx, stl_x, stl_y); return;
    spl_id = -1;
    if (wrkstate < PLAYER_STATES_COUNT)
      spl_id = player_state_to_spell[wrkstate];
    SYNCDBG(5,"Starting for spell %ld",spl_id);
    if (spl_id <= 0)
    {
      set_pointer_graphic(0);
      return;
    }
    player = get_my_player();
    thing = thing_get(tng_idx);
    allow_cast = false;
    pwrdata = get_power_data(spl_id);
    if ((tng_idx == 0) || (thing->owner == player->id_number) || (pwrdata->can_cast_on_enemy != 0))
    {
      if (can_cast_spell_at_xy(player->id_number, spl_id, stl_x, stl_y, 0))
      {
        if ((tng_idx == 0) || can_cast_spell_on_creature(player->id_number, thing, spl_id))
        {
          allow_cast = true;
        }
      }
    }
    if (!allow_cast)
    {
      set_pointer_graphic(15);
      return;
    }
    chkfunc = pwrdata->field_15;
    if (chkfunc != NULL)
    {
      if (chkfunc())
      {
        i = get_power_overcharge_level(player);
        set_pointer_graphic(16+i);
        magstat = &game.magic_stats[spl_id];
        draw_spell_cost = magstat->cost[i];
        return;
      }
    }
    i = pwrdata->field_13;
    set_pointer_graphic_spell(i, game.play_gameturn);
}

void draw_bonus_timer(void)
{
  _DK_draw_bonus_timer(); return;
}

void draw_swipe(void)
{
  _DK_draw_swipe();
}

long near_map_block_thing_filter_queryable_object(const struct Thing *thing, MaxFilterParam param, long maximizer)
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
    struct CompoundFilterParam param;
    SYNCDBG(19,"Starting");
    //return _DK_get_queryable_object_near(pos_x, pos_y, plyr_idx);
    filter = near_map_block_thing_filter_queryable_object;
    param.plyr_idx = plyr_idx;
    param.num1 = pos_x;
    param.num2 = pos_y;
    return get_thing_near_revealed_map_block_with_filter(pos_x, pos_y, filter, &param);
}

void tag_cursor_blocks_dig(unsigned char a1, long a2, long a3, long a4)
{
  SYNCDBG(7,"Starting");
  _DK_tag_cursor_blocks_dig(a1, a2, a3, a4);
}

void tag_cursor_blocks_thing_in_hand(unsigned char a1, long a2, long a3, int a4, long a5)
{
  SYNCDBG(7,"Starting");
  _DK_tag_cursor_blocks_thing_in_hand(a1, a2, a3, a4, a5);
}

unsigned long can_drop_thing_here(long x, long y, long a3, unsigned long allow_unclaimed)
{
  return _DK_can_drop_thing_here(x, y, a3, allow_unclaimed);
}

/**
 * Returns if a given player (owner) can dig the specified subtile.
 */
short can_dig_here(long stl_x, long stl_y, long plyr_idx)
{
    struct SlabMap *slb;
    struct SlabAttr *slbattr;
    slb = get_slabmap_for_subtile(stl_x,stl_y);
    if (slabmap_block_invalid(slb))
      return false;
    if (!subtile_revealed(stl_x, stl_y, plyr_idx))
      return true;
    if (slab_kind_is_nonmagic_door(slb->kind))
    {
        if (slabmap_owner(slb) == plyr_idx)
          return false;
    }
    slbattr = get_slab_attrs(slb);
    if ((slbattr->flags & 0x29) != 0)
      return true;
    return false;
}

void set_player_cameras_position(struct PlayerInfo *player, long pos_x, long pos_y)
{
    player->cameras[2].mappos.x.val = pos_x;
    player->cameras[3].mappos.x.val = pos_x;
    player->cameras[0].mappos.x.val = pos_x;
    player->cameras[2].mappos.y.val = pos_y;
    player->cameras[3].mappos.y.val = pos_y;
    player->cameras[0].mappos.y.val = pos_y;
}

void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7)
{
  _DK_draw_texture(a1, a2, a3, a4, a5, a6, a7);
}

void update_block_pointed(int i,long x, long x_frac, long y, long y_frac)
{
    struct Map *map;
    struct Column *colmn;
    short visible;
    unsigned int mask;
    long k;

    if (i > 0)
    {
      map = get_map_block_at(x,y);
      visible = map_block_revealed_bit(map, player_bit);
      if ((!visible) || ((map->data & 0x7FF) > 0))
      {
        if (visible)
          k = map->data & 0x7FF;
        else
          k = game.unrevealed_column_idx;
        colmn = get_column(k);
        mask = colmn->solidmask;
        if ((temp_cluedo_mode) && (mask != 0))
        {
          if (visible)
            k = map->data & 0x7FF;
          else
            k = game.unrevealed_column_idx;
          colmn = get_column(k);
          if (colmn->solidmask >= 8)
          {
            if ( (!visible) || (((get_navigation_map(x,y) & 0x80) == 0) && ((map->flags & MapFlg_IsRoom) == 0)) )
              mask &= 3;
          }
        }
        if (mask & (1 << (i-1)))
        {
          pointed_at_frac_x = x_frac;
          pointed_at_frac_y = y_frac;
          block_pointed_at_x = x;
          block_pointed_at_y = y;
          me_pointed_at = map;
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
        map = get_map_block_at(x,y);
        floor_pointed_at_x = x;
        floor_pointed_at_y = y;
        block_pointed_at_x = x;
        block_pointed_at_y = y;
        pointed_at_frac_x = x_frac;
        pointed_at_frac_y = y_frac;
        me_pointed_at = map;
    }
}

void update_blocks_pointed(void)
{
    long x,y;
    long x_frac,y_frac;
    long hori_ptr_y,vert_ptr_y;
    long hori_hdelta_y,vert_hdelta_y;
    long hori_ptr_x,vert_ptr_x;
    long hvdiv_x,hvdiv_y;
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

void draw_overlay_things(long zoom)
{
  SYNCDBG(7,"Starting");
  _DK_draw_overlay_things(zoom);
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
    if ((game.numfield_C & 0x01) != 0)
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
    if ((game.numfield_D & 0x10) != 0)
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
    PaletteSetPlayerPalette(player, _DK_palette);
    if ((game.numfield_C & 0x02) != 0)
        initialise_eye_lenses();
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

        // Check if we should redraw screen in this turn
        do_draw = display_should_be_updated_this_turn() || (!LbIsActive());

        LbWindowsControl();
        update_mouse();
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

void intro(void)
{
    char *fname;
    fname = prepare_file_path(FGrp_LoData, "intromix.smk");
    SYNCDBG(0,"Playing intro movie \"%s\"",fname);
    play_smacker_file(fname, 1);
}

void outro(void)
{
    char *fname;
    fname = prepare_file_path(FGrp_LoData, "outromix.smk");
    SYNCDBG(0,"Playing outro movie \"%s\"",fname);
    play_smacker_file(fname, 17);
}

void set_thing_draw(struct Thing *thing, long anim, long speed, long a4, char a5, char start_frame, unsigned char a7)
{
  unsigned long i;
  //_DK_set_thing_draw(thing, anim, speed, a4, a5, start_frame, a7); return;
  thing->field_44 = convert_td_iso(anim);
  thing->field_50 &= 0x03;
  thing->field_50 |= (a7 << 2);
  thing->field_49 = keepersprite_frames(thing->field_44);
  if (speed != -1)
  {
    thing->field_3E = speed;
  }
  if (a4 != -1)
  {
    thing->field_46 = a4;
  }
  if (a5 != -1)
  {
    set_flag_byte(&thing->field_4F, 0x40, a5);
  }
  if (start_frame == -2)
  {
    i = keepersprite_frames(thing->field_44) - 1;
    thing->field_48 = i;
    thing->field_40 = i << 8;
  } else
  if (start_frame == -1)
  {
    i = ACTION_RANDOM(thing->field_49);
    thing->field_48 = i;
    thing->field_40 = i << 8;
  } else
  {
    i = start_frame;
    thing->field_48 = i;
    thing->field_40 = i << 8;
  }
}

void init_dungeons(void)
{
  int i,k;
  struct Dungeon *dungeon;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = get_dungeon(game.hero_player_num);
    dungeon->hates_player[i] = game.fight_max_hate;
    dungeon = get_dungeon(i);
    dungeon->hates_player[game.hero_player_num%DUNGEONS_COUNT] = game.fight_max_hate;
    dungeon->num_active_diggers = 0;
    dungeon->num_active_creatrs = 0;
    dungeon->creatr_list_start = 0;
    dungeon->digger_list_start = 0;
    dungeon->owner = i;
    dungeon->max_creatures_attracted = game.default_max_crtrs_gen_entrance;
    dungeon->dead_creatures_count = 0;
    dungeon->dead_creature_idx = 0;
    for (k=0; k < DUNGEONS_COUNT; k++)
    {
      if (k == i)
        dungeon->hates_player[k] = game.fight_max_love;
      else
        dungeon->hates_player[k] = game.fight_max_hate;
    }
    LbMemorySet(dungeon->field_1489, 0, 32);
  }
}

int can_thing_be_queried(struct Thing *thing, long a2)
{
  return _DK_can_thing_be_queried(thing, a2);
}

int can_thing_be_possessed(struct Thing *thing, long a2)
{
  return _DK_can_thing_be_possessed(thing, a2);
}

long tag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3)
{
  return _DK_tag_blocks_for_digging_in_rectangle_around(a1, a2, a3);
}

void untag_blocks_for_digging_in_rectangle_around(long a1, long a2, char a3)
{
  _DK_untag_blocks_for_digging_in_rectangle_around(a1, a2, a3);
}

void tag_cursor_blocks_sell_area(unsigned char a1, long a2, long a3, long a4)
{
    SYNCDBG(7,"Starting");
    _DK_tag_cursor_blocks_sell_area(a1, a2, a3, a4);
}

long packet_place_door(long a1, long a2, long a3, long a4, unsigned char a5)
{
    return _DK_packet_place_door(a1, a2, a3, a4, a5);
}

void delete_room_slabbed_objects(long a1)
{
    SYNCDBG(17,"Starting");
    _DK_delete_room_slabbed_objects(a1);
}

unsigned char tag_cursor_blocks_place_door(unsigned char a1, long a2, long a3)
{
    SYNCDBG(7,"Starting");
    return _DK_tag_cursor_blocks_place_door(a1, a2, a3);
}

unsigned char tag_cursor_blocks_place_room(unsigned char a1, long a2, long a3, long a4)
{
    SYNCDBG(7,"Starting");
    return _DK_tag_cursor_blocks_place_room(a1, a2, a3, a4);
}

void initialise_map_collides(void)
{
  SYNCDBG(7,"Starting");
  _DK_initialise_map_collides();
}

void initialise_map_health(void)
{
  SYNCDBG(7,"Starting");
  _DK_initialise_map_health();
}

long slabs_count_near(long tx,long ty,long rad,unsigned short slbkind)
{
  long dx,dy;
  long x,y;
  long count;
  count=0;
  struct SlabMap *slb;
  for (dy=-rad; dy <= rad; dy++)
  {
    y = ty+dy;
    if ((y>=0) && (y<map_tiles_y))
      for (dx=-rad; dx <= rad; dx++)
      {
        x = tx+dx;
        if ((x>=0) && (x<map_tiles_x))
        {
          slb = get_slabmap_block(x, y);
          if (slb->kind == slbkind)
            count++;
        }
      }
  }
  return count;
}

long ceiling_init(unsigned long a1, unsigned long a2)
{
  return _DK_ceiling_init(a1, a2);
}

long process_temple_special(struct Thing *thing)
{
    struct Dungeon *dungeon;
    dungeon = get_dungeon(thing->owner);
    if (object_is_mature_food(thing))
    {
        dungeon->chickens_sacrificed++;
    } else
    {
        dungeon->field_8D5++;
    }
    return 0;
}

void do_creature_swap(long ncrt_id, long crtr_id)
{
//TODO SCRIPT rewrite from DD
  WARNMSG("Swaping creatures is only supported in Deeper Dungeons");
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
    struct CreatureStorage transfer_mem;
    //_DK_init_level(); return;
    LbMemoryCopy(&transfer_mem,&game.intralvl_transfered_creature,sizeof(struct CreatureStorage));
    game.flags_gui = 0;
    game.action_rand_seed = 1;
    free_swipe_graphic();
    game.loaded_swipe_idx = -1;
    game.play_gameturn = 0;
    clear_game();
    reset_heap_manager();
    lens_mode = 0;
    setup_heap_manager();
    load_computer_player_config(CnfLd_Standard);
    init_good_player_as(hero_player_number);

    light_set_lights_on(1);
    start_rooms = &game.rooms[1];
    end_rooms = &game.rooms[ROOMS_COUNT];

    erstats_clear();
    init_dungeons();
    preload_script(get_selected_level_number());
    load_map_file(get_selected_level_number());

    init_navigation();
    clear_messages();
    LbStringCopy(game.campaign_fname,campaign.fname,sizeof(game.campaign_fname));
    // Initialize unsynchronized random seed (the value may be different
    // on computers in MP, as it shouldn't affect game actions)
    game.unsync_rand_seed = (unsigned long)LbTimeSec();
    if (!SoundDisabled)
    {
        game.field_14BB54 = (UNSYNC_RANDOM(67) % 3 + 1);
        game.field_14BB55 = 0;
    }
    light_set_lights_on(1);
    {
        struct PlayerInfo *player;
        player = get_player(game.hero_player_num);
        init_player_start(player);
    }
    game.numfield_D |= 0x04;
    LbMemoryCopy(&game.intralvl_transfered_creature,&transfer_mem,sizeof(struct CreatureStorage));
    event_initialise_all();
    battle_initialise();
    ambient_sound_prepare();
    zero_messages();
    game.field_150356 = 0;
    game.field_15035A = 0;
    init_messages();
    game.creatures_tend_1 = 0;
    game.creatures_tend_2 = 0;
    game.field_15033A = 0;
    game.field_151801 = 0;
    game.field_151805 = 0;
    game.field_151809 = 0;
    set_chosen_spell_none();
    game.manufactr_element = 0;
    game.numfield_15181D = 0;
    game.manufactr_tooltip = 0;
}

void set_chosen_spell(long sptype, long sptooltip)
{
    struct SpellData *pwrdata;
    pwrdata = get_power_data(sptype);
    if (power_data_is_invalid(pwrdata))
      sptype = 0;
    SYNCDBG(6,"Setting to %ld",sptype);
    game.chosen_spell_type = sptype;
    game.chosen_spell_look = pwrdata->field_9;
    game.chosen_spell_tooltip = sptooltip;
}

void set_chosen_spell_none(void)
{
    SYNCDBG(6,"Setting to %d",0);
    game.chosen_spell_type = 0;
    game.chosen_spell_look = 0;
    game.chosen_spell_tooltip = 0;
}

void post_init_level(void)
{
    SYNCDBG(8,"Starting");
    struct Dungeon *dungeon;
    //_DK_post_init_level(); return;
    if (game.packet_save_enable)
        open_new_packet_file_for_save();
    calculate_dungeon_area_scores();
    init_animating_texture_maps();
    int i,k;
    for (i=0; i < DUNGEONS_COUNT; i++)
    {
        dungeon = get_dungeon(i);
        for (k=0; k < CREATURE_TYPES_COUNT; k++)
        {
          dungeon->creature_max_level[k] = 10;
        }
    }
    clear_creature_pool();
    setup_computer_players2();
    load_stats_files();
    check_and_auto_fix_stats();
    load_script(get_loaded_level_number());
    init_dungeons_research();
    create_transferred_creature_on_level();
    update_dungeons_scores();
    update_dungeon_generation_speeds();
    init_traps();
    init_all_creature_states();
    init_keepers_map_exploration();
    SYNCDBG(9,"Finished");
}

short init_animating_texture_maps(void)
{
    SYNCDBG(8,"Starting");
    //_DK_init_animating_texture_maps(); return;
    anim_counter = 7;
    return update_animating_texture_maps();
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
    if ( game.packet_checksum )
      SYNCMSG("Packet Checksum Active");
    SYNCMSG("Fast Forward through %d game turns", game.turns_fastforward);
    if (game.turns_packetoff != -1)
      SYNCMSG("Packet Quit at %d", game.turns_packetoff);
    if (game.packet_load_enable)
    {
      if (game.log_things_end_turn != game.log_things_start_turn)
        SYNCMSG("Logging things, game turns %d -> %d", game.log_things_start_turn, game.log_things_end_turn);
    }
#endif
    game.game_kind = GKind_NetworkGame;
    if (!(game.packet_save_head.field_C & (1 << game.numfield_149F46))
      || (game.packet_save_head.field_D & (1 << game.numfield_149F46)))
      my_player_number = 0;
    else
      my_player_number = game.numfield_149F46;
    init_level();
    init_players();
    if (game.field_14E495 == 1)
      game.game_kind = GKind_NetworkGame;
    if (game.turns_stored < game.turns_fastforward)
      game.turns_fastforward = game.turns_stored;
    post_init_level();
    post_init_players();
    set_selected_level_number(0);
}

void faststartup_saved_packet_game(void)
{
    struct PlayerInfo *player;
    reenter_video_mode();
    startup_saved_packet_game();
    player = get_my_player();
    player->field_6 &= 0xFDu;
    set_gui_visible(false);
    set_flag_byte(&game.numfield_C,0x40,false);
}

void startup_network_game(TbBool local)
{
    SYNCDBG(0,"Starting up network game");
    //_DK_startup_network_game(); return;
    unsigned int flgmem;
    struct PlayerInfo *player;
    setup_count_players();
    player = get_my_player();
    flgmem = player->field_2C;
    init_level();
    player = get_my_player();
    player->field_2C = flgmem;
    //if (game.flagfield_14EA4A == 2) //was wrong because init_level sets this to 2. global variables are evil (though perhaps that's why they were chosen for DK? ;-))
    if (local)
    {
        game.game_kind = GKind_NetworkGame;
        init_players_local_game();
    } else
    {
        game.game_kind = GKind_KeeperGame;
        init_players_network_game();
    }
    if (fe_computer_players)
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
    reenter_video_mode();
    my_player_number = default_loc_player;
    game.game_kind = GKind_NetworkGame;
    if (!is_campaign_loaded())
    {
      if (!change_campaign(""))
        ERRORLOG("Unable to load campaign");
    }
    player = get_my_player();
    player->field_2C = 1;
    startup_network_game(true);
    player = get_my_player();
    player->field_6 &= ~0x02;
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
  // Make sure campaign is loaded
  if (!load_campaigns_list())
  {
    ERRORLOG("No valid campaign files found");
    exit_keeper = 1;
    return;
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
  if ((game.numfield_C & 0x02) != 0)
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
  if ( !frontend_load_data() )
  {
    ERRORLOG("Unable to load frontend data");
    exit_keeper = 1;
    return;
  }
  memset(scratch, 0, PALETTE_SIZE);
  LbPaletteSet(scratch);
  frontend_set_state(get_startup_menu_state());

  short finish_menu = 0;
  set_flag_byte(&game.flags_cd,0x40,false);
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
  short prev_state;
  prev_state = frontend_menu_state;
  frontend_set_state(0);
  if (exit_keeper)
  {
    player = get_my_player();
    player->field_6 &= ~0x02;
    return;
  }
  reenter_video_mode();

  display_loading_screen();
  short flgmem;
  switch (prev_state)
  {
  case FeSt_START_KPRLEVEL:
        my_player_number = default_loc_player;
        game.game_kind = GKind_NetworkGame;
        set_flag_byte(&game.system_flags,GSF_NetworkActive,false);
        player = get_my_player();
        player->field_2C = 1;
        startup_network_game(true);
        break;
  case FeSt_START_MPLEVEL:
        set_flag_byte(&game.system_flags,GSF_NetworkActive,true);
        game.game_kind = GKind_KeeperGame;
        player = get_my_player();
        player->field_2C = 1;
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
        set_flag_byte(&game.numfield_C,0x40,false);
        break;
  }
  player = get_my_player();
  player->field_6 &= ~0x02;
}

void game_loop(void)
{
  //_DK_game_loop(); return;
  unsigned long random_seed;
  unsigned long playtime;
  playtime = 0;
  random_seed = 0;
  SYNCDBG(0,"Entering gameplay loop.");
  while ( !exit_keeper )
  {
    update_mouse();
    wait_at_frontend();
    if ( exit_keeper )
      break;
    struct PlayerInfo *player;
    player = get_my_player();
    if (game.game_kind == GKind_NetworkGame)
    {
      if (game.numfield_15 == -1)
      {
        set_player_instance(player, PI_HeartZoom, 0);
      } else
      {
        game.numfield_15 = -1;
        set_flag_byte(&game.numfield_C,0x01,false);
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
    StopRedbookTrack();
    StopMusic();
    turn_off_all_menus();
    delete_all_structures();
    clear_mapwho();
    endtime = LbTimerClock();
    quit_game = 0;
    if ((game.numfield_C & 0x02) != 0)
        exit_keeper=true;
    playtime += endtime-starttime;
    SYNCDBG(0,"Play time is %d seconds",playtime>>10);
    random_seed += game.play_gameturn;
    reset_eye_lenses();
    close_packet_file();
    game.packet_load_enable = false;
    game.packet_save_enable = false;
  } // end while
  // Stop the movie recording if it's on
  if ((game.system_flags & GSF_CaptureMovie) != 0)
    movie_record_stop();
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

  SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain
  _DK_LbErrorLogSetup(0, 0, 1);

  set_default_startup_parameters();

  short bad_param;
  LevelNumber level_num;
  bad_param = 0;
  unsigned short narg;
  level_num = LEVELNUMBER_ERROR;
  narg = 1;
  while ( narg < argc )
  {
      char *par;
      par = argv[narg];
      if ( (par == NULL) || ((par[0] != '-') && (par[0] != '/')) )
          return -1;
      char parstr[CMDLN_MAXLEN+1];
      char pr2str[CMDLN_MAXLEN+1];
      strncpy(parstr, par+1, CMDLN_MAXLEN);
      if (narg+1 < argc)
        strncpy(pr2str,  argv[narg+1], CMDLN_MAXLEN);
      else
        pr2str[0]='\0';

      if (strcasecmp(parstr, "nointro") == 0)
      {
        start_params.no_intro = 1;
      } else
      if (strcasecmp(parstr, "nocd") == 0)
      {
          set_flag_byte(&start_params.flags_cd,MFlg_NoMusic,true);
      } else
      if (strcasecmp(parstr, "1player") == 0)
      {
          start_params.one_player = 1;
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
        set_flag_byte(&start_params.numfield_C,0x02,true);
        level_num = atoi(pr2str);
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
          lbMouseAutoReset = false;
      } else
      if ( strcasecmp(parstr,"vidriver") == 0 )
      {
          LbScreenHardwareConfig(pr2str,8);
          narg++;
      } else
      if (strcasecmp(parstr,"packetload") == 0)
      {
         if (start_params.packet_save_enable)
            WARNMSG("PacketSave disabled to enable PacketLoad.");
         start_params.packet_load_enable = true;
         start_params.packet_save_enable = false;
         strncpy(start_params.packet_fname,pr2str,149);
         narg++;
      } else
      if (strcasecmp(parstr,"packetsave") == 0)
      {
         if (start_params.packet_load_enable)
            WARNMSG("PacketLoad disabled to enable PacketSave.");
         start_params.packet_load_enable = false;
         start_params.packet_save_enable = true;
         strncpy(start_params.packet_fname,pr2str,149);
         narg++;
      } else
      if (strcasecmp(parstr,"q") == 0)
      {
         set_flag_byte(&start_params.numfield_C,0x02,true);
      } else
      if (strcasecmp(parstr,"columnconvert") == 0)
      {
         set_flag_byte(&start_params.numfield_C,0x08,true);
      } else
      if (strcasecmp(parstr,"lightconvert") == 0)
      {
         set_flag_byte(&start_params.numfield_C,0x10,true);
      } else
      if (strcasecmp(parstr, "dbgshots") == 0)
      {
          set_flag_byte(&start_params.debug_flags,DFlg_ShotsDamage,true);
      } else
      if (strcasecmp(parstr, "sessions") == 0) {
          narg++;
          LbNetwork_InitSessionsFromCmdLine(pr2str);
      } else
      if (strcasecmp(parstr,"alex") == 0)
      {
         set_flag_byte(&start_params.flags_font,FFlg_AlexCheat,true);
      } else
      {
        WARNMSG("Unrecognized command line parameter '%s'.",parstr);
        bad_param=narg;
      }
      narg++;
  }

  if (level_num == LEVELNUMBER_ERROR)
    level_num = first_singleplayer_level();
  start_params.selected_level_number = level_num;
  my_player_number = default_loc_player;
  return (bad_param==0);
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
  short retval;
  retval=0;
  LbErrorLogSetup("/", log_file_name, 5);
  LbScreenHardwareConfig("directx",8);

  retval=process_command_line(argc,argv);
  if ( retval < 1 )
  {
      static const char *msg_text="Command line parameters analysis failed.\n";
      error_dialog_fatal(__func__, 1, msg_text);
      LbErrorLogClose();
      return 0;
  }

  LbTimerInit();
  LbScreenInitialize();
  LbSetTitle(PROGRAM_NAME);
  LbSetIcon(1);
  LbScreenSetDoubleBuffering(true);
  srand(LbTimerClock());

  retval = setup_game();
  if (retval)
  {
    if ((install_info.lang_id == Lang_Japanese) ||
        (install_info.lang_id == Lang_ChineseInt) ||
        (install_info.lang_id == Lang_ChineseTra))
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
/*  {
      struct PlayerInfo *player;
      player = get_player(0);
      text = buf_sprintf("Position of the first Player is %06x, first Camera is %06x bytes.\n",((int)player) - ((int)&_DK_game),((int)&(player->acamera)) - ((int)player));
      error_dialog(__func__, 1, text);
      return 0;
  }
  {
      struct Dungeon *dungeon;
      dungeon = get_dungeon(0);
      text = buf_sprintf("Position of the first Dungeon is %06x, field_ACF is at %06x bytes.\n",
                  ((int)dungeon) - ((int)&game),((int)(&dungeon->field_ACF)) - ((int)dungeon));
      error_dialog(__func__, 1, text);
      return 0;
  }*/
  if (sizeof(struct Game) != SIZEOF_Game)
  {
      long delta1,delta2,delta3;
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
