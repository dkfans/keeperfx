
#include <windows.h>
#include <winbase.h>
#include <math.h>
#include "keeperfx.h"

#include "bflib_dernc.h"
#include "bflib_pom.h"
#include "bflib_math.h"
#include "bflib_memory.h"
#include "bflib_keybrd.h"
#include "bflib_datetm.h"
#include "bflib_bufrw.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_fileio.h"
#include "bflib_sndlib.h"
#include "bflib_fmvids.h"
#include "bflib_video.h"
#include "bflib_vidraw.h"
#include "bflib_guibtns.h"
#include "bflib_sound.h"
#include "bflib_mouse.h"
#include "bflib_filelst.h"

#include "frontend.h"
#include "front_input.h"
#include "scrcapt.h"
#include "vidmode.h"
#include "kjm_input.h"
#include "packets.h"
#include "config.h"
#include "lvl_script.h"
#include "thing_list.h"

int test_variable;

// Max length of the command line
#define CMDLN_MAXLEN 259
char cmndline[CMDLN_MAXLEN+1];
unsigned short bf_argc;
char *bf_argv[CMDLN_MAXLEN+1];
unsigned char palette_buf[768];

short is_new_moon = 0;

// Map size variables
int map_subtiles_x = 255;
int map_subtiles_y = 255;
int map_tiles_x = 85;
int map_tiles_y = 85;

const long VersionMajor = 1;
const long VersionMinor = 12;

unsigned int eye_lens_width = 0;
unsigned int eye_lens_height = 0;

unsigned short player_colors_map[] = {0, 1, 2, 3, 4, 5, 0, 0, 0, };

TbPixel const player_path_colours[] = {131, 90, 163, 181, 20, 4, };

unsigned char i_can_see_levels[] = {15, 20, 25, 30,};

char onscreen_msg_text[255]="";
int onscreen_msg_turns = 0;

char sound_dir[64] = "SOUND";
char window_class_name[128]="Bullfrog Shell";
short default_loc_player = 0;
short hero_player = 4;
unsigned long gold_per_hoarde = 2000;

#define TRACE LbNetLog

//DLLIMPORT extern struct TbLoadFiles _DK_legal_load_files[];
struct TbLoadFiles legal_load_files[] = {
    {"*PALETTE", &_DK_palette, NULL, PALETTE_SIZE, 0, 0},
    {"*SCRATCH", &scratch, NULL, 0x10000, 1, 0},
    {"", NULL, NULL, 0, 0, 0}, };

unsigned char *nocd_raw;
unsigned char *nocd_pal;

struct TbLoadFiles nocd_load_files[] = {
    {"data/nocd.raw", &nocd_raw, NULL, 0, 0, 0},
    {"data/nocd.pal", &nocd_pal, NULL, 0, 0, 0},
    {"", NULL, NULL, 0, 0, 0}, };

Perspect_Func perspective_routines[] = {
  perspective_standard,
  perspective_standard,
  perspective_standard,
  perspective_fisheye,
};

RotPers_Func rotpers_routines[] = {
  rotpers_parallel,
  rotpers_standard,
  rotpers_circular,
  rotpers_fisheye,
};

Thing_Class_Func class_functions[] = {
  NULL,
  update_object,
  update_shot,
  update_effect_element,
  update_dead_creature,
  update_creature,
  update_effect,
  process_effect_generator,
  update_trap,
  process_door,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
};

//long const imp_spangle_effects[] = {

unsigned short creature_graphics[][22] = {
  {   0,   0,   0,   0,   0,   0,   0,   0,   0,  0,   0,
      0,   0,   0,   0,   0,   0,   0,  0,   0,   0,   0,},
  { 426, 424, 424, 428,   0,   0,   0,  0, 430, 436, 442,
    438, 440, 444,  52, 432, 434, 946, 20, 164, 178,  20,},
  { 404, 402, 402, 406,   0,   0,   0,  0, 408, 414, 420,
    416, 418, 422,  62, 410, 412, 946, 21, 165, 180,  21,},
  { 382, 380, 380, 384,   0,   0,   0,  0, 386, 392, 398,
    394, 396, 400,  54, 388, 390, 946, 15, 168, 182,  24,},
  { 206, 204, 204, 208,   0,   0,   0,  0, 210, 216, 222,
    218, 220, 224,  48, 212, 214, 946, 15, 172, 184,  28,},
  { 360, 358, 358, 362,   0,   0,   0,  0, 364, 370, 376,
    372, 374, 378,  48, 366, 368, 946, 22, 166, 186,  22,},
  { 228, 226, 226, 230,   0,   0,   0,  0, 232, 238, 244,
    240, 242, 246,  60, 234, 236, 946, 18, 162, 188,  18,},
  { 162, 160, 160, 164,   0,   0,   0,  0, 166, 172, 178,
    174, 176, 180,  48, 168, 170, 946, 18, 214, 190,  18,},
  { 338, 336, 336, 340,   0,   0,   0,350, 342, 348, 354,
    350, 352, 356,  48, 344, 346, 946, 22, 167, 192,  23,},
  { 316, 314, 314, 318,   0,   0,   0,  0, 320, 326, 332,
    328, 330, 334,  48, 322, 324, 946, 26, 170, 194,  26,},
  { 294, 292, 292, 296,   0,   0,   0,  0, 298, 304, 310,
    306, 308, 312,  48, 300, 302, 946, 15, 169, 196,  25,},
  { 272, 270, 270, 274,   0,   0,   0,  0, 276, 282, 288,
    284, 286, 290,  58, 278, 280, 946, 15, 161, 198,  17,},
  { 250, 248, 248, 252,   0,   0,   0,  0, 254, 260, 266,
    262, 264, 268,  64, 256, 258, 946, 19, 163, 200,  19,},
  {  26,  24,  24,  28,   0,   0,   0,  0,  30,  36,  42,
     38,  40,  44,  48,  32,  34, 946, 15, 171, 202,  27,},
  { 754, 752, 752, 756, 766,   0,   0,  0, 758, 764, 770,
    766, 768, 772,  48, 760, 762, 946,  1, 145, 204,   1,},
  { 732, 730, 730, 734,   0,   0,   0,  0, 736, 742, 748,
    744, 746, 750,  92, 738, 740, 946,  2, 146, 206,   2,},
  { 710, 708, 708, 712,   0,   0,   0,  0, 714, 720, 726,
    722, 724, 728,  90, 716, 718, 946,  3, 147, 208,   3,},
  { 688, 686, 686, 690,   0,   0,   0,  0, 692, 698, 704,
    700, 702, 706,  88, 694, 696, 946,  4, 148, 210,   4,},
  { 666, 664, 664, 668,   0,   0,   0,  0, 670, 676, 682,
    678, 680, 684,  86, 672, 674, 946,  5, 149, 212,   5,},
  { 644, 644, 644, 646,   0,   0,   0,  0, 648, 654, 660,
    656, 658, 662,  84, 650, 652, 946,  6, 150, 214,   6,},
  { 624, 622, 622, 626,   0,   0,   0,  0, 628, 634, 640,
    636, 638, 642,  82, 630, 632, 946,  7, 151, 216,   7,},
  { 602, 600, 600, 604,   0,   0,   0,  0, 606, 612, 618,
    614, 616, 620,  80, 608, 610, 946,  8, 152, 218,   8,},
  { 580, 578, 578, 582,   0,   0,   0,  0, 584, 590, 596,
    592, 594, 598,  78, 586, 588, 946,  9, 153, 220,   9,},
  { 556, 554, 566, 558, 558, 568, 562,564, 560, 574, 576,
    564, 556, 556,  56, 570, 572, 946, 10, 154, 222,  10,},
  { 534, 532, 532, 536,   0,   0,   0,  0, 538, 544, 550,
    546, 548, 552,  76, 540, 542, 946, 11, 155, 224,  11,},
  { 512, 510, 510, 514,   0,   0,   0,  0, 516, 522, 528,
    524, 526, 530,  50, 518, 520, 946, 12, 156, 226,  12,},
  { 490, 488, 488, 492,   0,   0,   0,  0, 494, 500, 506,
    502, 504, 508,  74, 496, 498, 946, 13, 157, 228,  13,},
  {   2,   0,   0,   4,  22,   0,   0,  0,   6,  12,  18,
     14,  16,  20,  48,   8,  10, 946, 15, 159, 230,  15,},
  { 470, 468, 468, 472,   0,   0,   0,  0, 474, 480, 486,
    482, 484, 470,  48, 476, 478, 946, 16, 160, 232,  16,},
  { 448, 446, 446, 450,   0,   0,   0,  0, 452, 458, 464,
    460, 462, 466,  68, 454, 456, 946, 14, 158, 234,  14,},
  { 184, 182, 182, 186,   0,   0,   0,  0, 188, 194, 200,
    196, 198, 202,  66, 190, 192, 946, 19, 173, 496,  29,},
  { 980, 980, 980, 980, 980,   0,   0,  0, 980, 980, 980,
    980, 980, 980, 980, 980, 980, 980,  0,   0,   0,   0,},
};

long pinstfs_hand_grab(struct PlayerInfo *player, long *n);
long pinstfm_hand_grab(struct PlayerInfo *player, long *n);
long pinstfe_hand_grab(struct PlayerInfo *player, long *n);
long pinstfs_hand_drop(struct PlayerInfo *player, long *n);
long pinstfe_hand_drop(struct PlayerInfo *player, long *n);
long pinstfs_hand_whip(struct PlayerInfo *player, long *n);
long pinstfe_hand_whip(struct PlayerInfo *player, long *n);
long pinstfm_hand_drop(struct PlayerInfo *player, long *n);
long pinstfs_hand_whip_end(struct PlayerInfo *player, long *n);
long pinstfe_hand_whip_end(struct PlayerInfo *player, long *n);
long pinstfs_control_creature(struct PlayerInfo *player, long *n);
long pinstfm_control_creature(struct PlayerInfo *player, long *n);
long pinstfe_direct_control_creature(struct PlayerInfo *player, long *n);
long pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n);
long pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n);
long pinstfm_leave_creature(struct PlayerInfo *player, long *n);
long pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n);
long pinstfe_leave_creature(struct PlayerInfo *player, long *n);
long pinstfs_query_creature(struct PlayerInfo *player, long *n);
long pinstfs_unquery_creature(struct PlayerInfo *player, long *n);
long pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n);
long pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n);
long pinstfm_control_creature_fade(struct PlayerInfo *player, long *n);
long pinstfe_control_creature_fade(struct PlayerInfo *player, long *n);
long pinstfs_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfm_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfe_fade_to_map(struct PlayerInfo *player, long *n);
long pinstfs_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfm_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfe_fade_from_map(struct PlayerInfo *player, long *n);
long pinstfs_zoom_to_position(struct PlayerInfo *player, long *n);
long pinstfm_zoom_to_position(struct PlayerInfo *player, long *n);
long pinstfe_zoom_to_position(struct PlayerInfo *player, long *n);

struct PlayerInstanceInfo player_instance_info[] = {
  { 0, 0, NULL,                      NULL,                      NULL,                               0, 0, 0},
  { 3, 1, pinstfs_hand_grab,         pinstfm_hand_grab,         pinstfe_hand_grab,                  0, 0, 0},
  { 3, 1, pinstfs_hand_drop,         pinstfm_hand_drop,         pinstfe_hand_drop,                  0, 0, 0},
  { 4, 0, pinstfs_hand_whip,         NULL,                      pinstfe_hand_whip,                  0, 0, 0},
  { 5, 0, pinstfs_hand_whip_end,     NULL,                      pinstfe_hand_whip_end,              0, 0, 0},
  {12, 1, pinstfs_control_creature,  pinstfm_control_creature,  pinstfe_direct_control_creature,    0, 0, 0},
  {12, 1, pinstfs_control_creature,  pinstfm_control_creature,  pinstfe_passenger_control_creature, 0, 0, 0},
  {12, 1, pinstfs_direct_leave_creature,pinstfm_leave_creature, pinstfe_leave_creature,             0, 0, 0},
  {12, 1, pinstfs_passenger_leave_creature,pinstfm_leave_creature,pinstfe_leave_creature,           0, 0, 0},
  { 0, 1, pinstfs_query_creature,    NULL,                      NULL,                               0, 0, 0},
  { 0, 1, pinstfs_unquery_creature,  NULL,                      NULL,                               0, 0, 0},
  {16, 1, pinstfs_zoom_to_heart,     pinstfm_zoom_to_heart,     pinstfe_zoom_to_heart,              0, 0, 0},
  {16, 1, pinstfs_zoom_out_of_heart, pinstfm_zoom_out_of_heart, pinstfe_zoom_out_of_heart,          0, 0, 0},
  {12, 1, NULL,                      pinstfm_control_creature_fade,pinstfe_control_creature_fade,   0, 0, 0},
  { 8, 1, pinstfs_fade_to_map,       pinstfm_fade_to_map,       pinstfe_fade_to_map,                0, 0, 0},
  { 8, 1, pinstfs_fade_from_map,     pinstfm_fade_from_map,     pinstfe_fade_from_map,              0, 0, 0},
  {-1, 1, pinstfs_zoom_to_position,  pinstfm_zoom_to_position,  pinstfe_zoom_to_position,           0, 0, 0},
  { 0, 0, NULL,                      NULL,                      NULL,                               0, 0, 0},
  { 0, 0, NULL,                      NULL,                      NULL,                               0, 0, 0},
};

//static
TbClockMSec last_loop_time=0;

#ifdef __cplusplus
extern "C" {
#endif
DLLIMPORT void _DK_do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5);
DLLIMPORT char _DK_mouse_is_over_small_map(int, int);
DLLIMPORT unsigned short _DK_find_next_annoyed_creature(unsigned char a1, unsigned short a2);
DLLIMPORT unsigned char _DK_active_battle_exists(unsigned char a1);
DLLIMPORT unsigned char _DK_step_battles_forward(unsigned char a1);
DLLIMPORT void _DK_go_to_my_next_room_of_type(unsigned long rkind);
DLLIMPORT struct ActionPoint *_DK_allocate_free_action_point_structure_with_number(long apt_num);
DLLIMPORT struct Room *_DK_create_room(unsigned char a1, unsigned char a2, unsigned short a3, unsigned short a4);
DLLIMPORT void _DK_set_room_efficiency(struct Room *room);
DLLIMPORT long _DK_pinstfs_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_grab(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_whip(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_whip(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_hand_drop(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_hand_whip_end(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_hand_whip_end(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_direct_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_leave_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_query_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_unquery_creature(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_control_creature_fade(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_control_creature_fade(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_fade_to_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_fade_from_map(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfs_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfm_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT long _DK_pinstfe_zoom_to_position(struct PlayerInfo *player, long *n);
DLLIMPORT void _DK_leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing);
DLLIMPORT long _DK_creature_instance_has_reset(struct Thing *thing, long a2);
DLLIMPORT long _DK_get_human_controlled_creature_target(struct Thing *thing, long a2);
DLLIMPORT void _DK_set_creature_instance(struct Thing *thing, long a1, long a2, long a3, struct Coord3d *pos);
DLLIMPORT void _DK_instant_instance_selected(long a1);
DLLIMPORT void _DK_initialise_map_collides(void);
DLLIMPORT void _DK_initialise_map_health(void);
DLLIMPORT void _DK_initialise_extra_slab_info(unsigned long lv_num);
DLLIMPORT long _DK_add_gold_to_hoarde(struct Thing *thing, struct Room *room, long amount);
DLLIMPORT struct Thing *_DK_create_door(struct Coord3d *pos, unsigned short a1, unsigned char a2, unsigned short a3, unsigned char a4);
DLLIMPORT struct Thing *_DK_create_effect_generator(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4);
DLLIMPORT void _DK_clear_mapwho(void);
DLLIMPORT void _DK_clear_map(void);
DLLIMPORT void _DK_set_room_capacity(struct Room *room, long capac);
DLLIMPORT long _DK_ceiling_init(unsigned long a1, unsigned long a2);
DLLIMPORT unsigned char _DK_load_map_slab_file(unsigned long lv_num);
DLLIMPORT void _DK_init_top_texture_to_cube_table(void);
DLLIMPORT void _DK_init_columns(void);
DLLIMPORT void _DK_init_whole_blocks(void);
DLLIMPORT long _DK_load_column_file(unsigned long lv_num);
DLLIMPORT void _DK_load_slab_file(void);
DLLIMPORT long _DK_load_map_data_file(unsigned long lv_num);
DLLIMPORT void _DK_load_thing_file(unsigned long lv_num);
DLLIMPORT long _DK_load_action_point_file(unsigned long lv_num);
DLLIMPORT long _DK_load_texture_map_file(unsigned long lv_num, unsigned char n);
DLLIMPORT void _DK_draw_jonty_mapwho(struct JontySpr *jspr);
DLLIMPORT void _DK_draw_keepsprite_unscaled_in_buffer(unsigned short a1, short a2, unsigned char a3, unsigned char *a4);
DLLIMPORT void _DK_draw_engine_number(struct Number *num);
DLLIMPORT void _DK_draw_engine_room_flag_top(struct RoomFlag *rflg);
DLLIMPORT struct Thing *_DK_get_nearest_thing_for_hand_or_slap(unsigned char a1, long a2, long a3);
DLLIMPORT long _DK_screen_to_map(struct Camera *camera, long scrpos_x, long scrpos_y, struct Coord3d *mappos);
DLLIMPORT void _DK_draw_lens(unsigned char *dstbuf, unsigned char *srcbuf, unsigned long *lens_mem, int width, int height, int scanln);
DLLIMPORT void _DK_flyeye_blitsec(unsigned char *srcbuf, unsigned char *dstbuf, long srcwidth, long dstwidth, long n, long height);
DLLIMPORT void _DK_draw_swipe(void);
DLLIMPORT void _DK_draw_creature_view(struct Thing *thing);
DLLIMPORT void _DK_perspective_standard(struct XYZ *cor, struct PolyPoint *ppt);
DLLIMPORT void _DK_perspective_fisheye(struct XYZ *cor, struct PolyPoint *ppt);
DLLIMPORT void _DK_rotpers_parallel(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_rotpers_standard(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_rotpers_circular(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_rotpers_fisheye(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_init_lens(unsigned long *lens_mem, int width, int height, int scanln, int nlens);
DLLIMPORT void _DK_flyeye_setup(long width, long height);
DLLIMPORT void _DK_rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx);
DLLIMPORT void _DK_rotate_base_axis(struct M33 *matx, short a2, unsigned char a3);
DLLIMPORT void _DK_fill_in_points_perspective(long a1, long a2, struct MinMax *mm);
DLLIMPORT void _DK_fill_in_points_cluedo(long a1, long a2, struct MinMax *mm);
DLLIMPORT void _DK_fill_in_points_isometric(long a1, long a2, struct MinMax *mm);
DLLIMPORT void _DK_display_drawlist(void);
DLLIMPORT void _DK_frame_wibble_generate(void);
DLLIMPORT void _DK_find_gamut(void);
DLLIMPORT void _DK_fiddle_gamut(long a1, long a2);
DLLIMPORT void _DK_create_map_volume_box(long a1, long a2, long a3);
DLLIMPORT void _DK_setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8);
DLLIMPORT void _DK_do_a_plane_of_engine_columns_perspective(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_do_a_plane_of_engine_columns_cluedo(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_do_a_plane_of_engine_columns_isometric(long a1, long a2, long a3, long a4);
DLLIMPORT void _DK_draw_view(struct Camera *cam, unsigned char a2);
DLLIMPORT void _DK_draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7);
DLLIMPORT void _DK_draw_status_sprites(long a1, long a2, struct Thing *thing, long a4);
DLLIMPORT long _DK_element_top_face_texture(struct Map *map);
DLLIMPORT long _DK_thing_is_spellbook(struct Thing *thing);
DLLIMPORT int _DK_LbSpriteDrawOneColour(long x, long y, struct TbSprite *spr, TbPixel colour);
DLLIMPORT long _DK_object_is_gold(struct Thing *thing);
DLLIMPORT void _DK_check_players_won(void);
DLLIMPORT void _DK_check_players_lost(void);
DLLIMPORT void _DK_process_dungeon_power_magic(void);
DLLIMPORT void _DK_process_dungeon_devastation_effects(void);
DLLIMPORT void _DK_process_entrance_generation(void);
DLLIMPORT void _DK_process_things_in_dungeon_hand(void);
DLLIMPORT void _DK_process_payday(void);
DLLIMPORT void _DK_remove_thing_from_mapwho(struct Thing *thing);
DLLIMPORT void _DK_place_thing_in_mapwho(struct Thing *thing);
DLLIMPORT long _DK_get_thing_height_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT struct Room *_DK_player_has_room_of_type(long plr_idx, long roomkind);
DLLIMPORT struct Room *_DK_find_room_with_spare_room_item_capacity(unsigned char a1, signed char a2);
DLLIMPORT long _DK_create_workshop_object_in_workshop_room(long a1, long a2, long a3);
DLLIMPORT long _DK_get_next_manufacture(struct Dungeon *dungeon);
DLLIMPORT void _DK_process_creature_standing_on_corpses_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_cleanup_current_thing_state(struct Thing *thing);
DLLIMPORT unsigned long _DK_setup_move_off_lava(struct Thing *thing);
DLLIMPORT struct Thing *_DK_create_thing(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4);
DLLIMPORT short _DK_kill_creature(struct Thing *thing, struct Thing *tngrp, char a1, unsigned char a2, unsigned char a3, unsigned char a4);
DLLIMPORT void _DK_move_thing_in_map(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT void _DK_process_creature_instance(struct Thing *thing);
DLLIMPORT void _DK_update_creature_count(struct Thing *thing);
DLLIMPORT long _DK_process_creature_state(struct Thing *thing);
DLLIMPORT long _DK_get_floor_height_under_thing_at(struct Thing *thing, struct Coord3d *pos);
DLLIMPORT long _DK_move_creature(struct Thing *thing);
DLLIMPORT void _DK_process_spells_affected_by_effect_elements(struct Thing *thing);
DLLIMPORT long _DK_get_top_cube_at_pos(long mpos);
DLLIMPORT void _DK_apply_damage_to_thing_and_display_health(struct Thing *thing, long a1, char a2);
DLLIMPORT long _DK_get_foot_creature_has_down(struct Thing *thing);
DLLIMPORT void _DK_process_disease(struct Thing *thing);
DLLIMPORT void _DK_set_creature_graphic(struct Thing *thing);
DLLIMPORT void _DK_process_keeper_spell_effect(struct Thing *thing);
DLLIMPORT long _DK_creature_is_group_leader(struct Thing *thing);
DLLIMPORT void _DK_leader_find_positions_for_followers(struct Thing *thing);
DLLIMPORT long _DK_update_creature_levels(struct Thing *thing);
DLLIMPORT long _DK_process_creature_self_spell_casting(struct Thing *thing);
DLLIMPORT void _DK_process_thing_spell_effects(struct Thing *thing);
DLLIMPORT long _DK_update_object(struct Thing *thing);
DLLIMPORT long _DK_update_shot(struct Thing *thing);
DLLIMPORT long _DK_update_effect_element(struct Thing *thing);
DLLIMPORT long _DK_update_dead_creature(struct Thing *thing);
DLLIMPORT long _DK_update_creature(struct Thing *thing);
DLLIMPORT long _DK_update_effect(struct Thing *thing);
DLLIMPORT long _DK_process_effect_generator(struct Thing *thing);
DLLIMPORT long _DK_update_trap(struct Thing *thing);
DLLIMPORT long _DK_process_door(struct Thing *thing);
DLLIMPORT long _DK_light_is_light_allocated(long lgt_id);
DLLIMPORT void _DK_light_set_light_position(long lgt_id, struct Coord3d *pos);
DLLIMPORT struct Thing *_DK_get_trap_for_position(long x, long y);
DLLIMPORT void _DK_gui_set_button_flashing(long a1, long a2);
DLLIMPORT short _DK_send_creature_to_room(struct Thing *thing, struct Room *room);
DLLIMPORT struct Room *_DK_get_room_thing_is_on(struct Thing *thing);
DLLIMPORT short _DK_set_start_state(struct Thing *thing);
DLLIMPORT long _DK_load_stats_files(void);
DLLIMPORT void _DK_check_and_auto_fix_stats(void);
DLLIMPORT long _DK_update_dungeon_scores(void);
DLLIMPORT long _DK_update_dungeon_generation_speeds(void);
DLLIMPORT void _DK_calculate_dungeon_area_scores(void);
DLLIMPORT void _DK_setup_computer_players2(void);
DLLIMPORT long _DK_get_next_research_item(struct Dungeon *dungeon);
DLLIMPORT void _DK_init_creature_level(struct Thing *thing, long nlev);
DLLIMPORT long _DK_convert_old_column_file(unsigned long lv_num);
DLLIMPORT void _DK_clear_columns(void);
DLLIMPORT void _DK_load_level_file(long lvnum);
DLLIMPORT void _DK_delete_all_structures(void);
DLLIMPORT void _DK_clear_mapwho(void);
DLLIMPORT void _DK_light_initialise(void);
DLLIMPORT void _DK_clear_game(void);
DLLIMPORT void _DK_clear_game_for_save(void);
DLLIMPORT long _DK_update_cave_in(struct Thing *thing);
DLLIMPORT void _DK_update_thing_animation(struct Thing *thing);
DLLIMPORT long _DK_update_thing(struct Thing *thing);
DLLIMPORT long _DK_get_thing_checksum(struct Thing *thing);
DLLIMPORT long _DK_update_thing_sound(struct Thing *thing);
DLLIMPORT void _DK_update_power_sight_explored(struct PlayerInfo *player);
DLLIMPORT void _DK_init_messages(void);
DLLIMPORT void _DK_battle_initialise(void);
DLLIMPORT void _DK_event_initialise_all(void);
DLLIMPORT void _DK_add_thing_to_list(struct Thing *thing, struct StructureList *list);
DLLIMPORT struct Thing *_DK_allocate_free_thing_structure(unsigned char a1);
DLLIMPORT unsigned char _DK_i_can_allocate_free_thing_structure(unsigned char a1);
DLLIMPORT void _DK_message_add(char c);
DLLIMPORT void _DK_toggle_creature_tendencies(struct PlayerInfo *player, char val);
DLLIMPORT long _DK_event_move_player_towards_event(struct PlayerInfo *player, long var);
DLLIMPORT void _DK_turn_off_call_to_arms(long a);
DLLIMPORT long _DK_place_thing_in_power_hand(struct Thing *thing, long var);
DLLIMPORT short _DK_magic_use_power_obey(unsigned short plridx);
DLLIMPORT void _DK_set_player_state(struct PlayerInfo *player, unsigned char a1, long a2);
DLLIMPORT void _DK_event_delete_event(long plridx, long num);
DLLIMPORT long _DK_set_autopilot_type(long plridx, long aptype);
DLLIMPORT void _DK_set_player_mode(struct PlayerInfo *player, long val);
DLLIMPORT short _DK_dump_held_things_on_map(unsigned char a1, long a2, long a3, short a4);
DLLIMPORT void _DK_turn_off_sight_of_evil(long plridx);
DLLIMPORT void _DK_go_on_then_activate_the_event_box(long plridx, long val);
DLLIMPORT void _DK_output_message(int, int, int);
DLLIMPORT void _DK_directly_cast_spell_on_thing(unsigned char plridx, unsigned char a2, unsigned short a3, long a4);
DLLIMPORT void _DK_lose_level(struct PlayerInfo *player);
DLLIMPORT long _DK_magic_use_power_armageddon(unsigned char val);
DLLIMPORT long _DK_battle_move_player_towards_battle(struct PlayerInfo *player, long var);
DLLIMPORT void _DK_level_lost_go_first_person(long plridx);
DLLIMPORT void __cdecl _DK_set_player_instance(struct PlayerInfo *playerinf, long, int);
DLLIMPORT TbError _DK_LbNetwork_Exchange(struct Packet *pckt);
DLLIMPORT void _DK_resync_game(void);
DLLIMPORT void __cdecl _DK_set_gamma(char, int);
DLLIMPORT void _DK_complete_level(struct PlayerInfo *player);
DLLIMPORT void _DK_free_swipe_graphic(void);
DLLIMPORT void _DK_draw_sound_stuff(void);
DLLIMPORT void _DK_draw_bonus_timer(void);
DLLIMPORT void _DK_draw_power_hand(void);
DLLIMPORT void _DK_update_explored_flags_for_power_sight(struct PlayerInfo *player);
DLLIMPORT void _DK_engine(struct Camera *cam);
DLLIMPORT void _DK_smooth_screen_area(unsigned char *a1, long a2, long a3, long a4, long a5, long a6);
DLLIMPORT void _DK_remove_explored_flags_for_power_sight(struct PlayerInfo *player);
DLLIMPORT void _DK_DrawBigSprite(long x, long y, struct BigSprite *bigspr, struct TbSprite *sprite);
DLLIMPORT void _DK_draw_gold_total(unsigned char a1, long a2, long a3, long a4);
DLLIMPORT void _DK_pannel_map_draw(long x, long y, long zoom);
DLLIMPORT void _DK_draw_overlay_things(long zoom);
DLLIMPORT void _DK_draw_overlay_compass(long a1, long a2);
DLLIMPORT unsigned char _DK_find_first_battle_of_mine(unsigned char idx);
DLLIMPORT void _DK_set_engine_view(struct PlayerInfo *player, long a2);
DLLIMPORT void _DK_startup_network_game(void);
DLLIMPORT void _DK_reinit_level_after_load(void);
DLLIMPORT void _DK_reinit_tagged_blocks_for_player(unsigned char idx);
DLLIMPORT void _DK_reset_gui_based_on_player_mode(void);
DLLIMPORT void _DK_init_animating_texture_maps(void);
DLLIMPORT void _DK_init_lookups(void);
DLLIMPORT long _DK_init_navigation(void);
DLLIMPORT long _DK_load_game(long);
DLLIMPORT void _DK_sound_reinit_after_load(void);
DLLIMPORT void _DK_restore_computer_player_after_load(void);
DLLIMPORT struct Thing *_DK_create_effect(struct Coord3d *pos, unsigned short a2, unsigned char a3);
DLLIMPORT void _DK_delete_thing_structure(struct Thing *thing, long a2);
DLLIMPORT void _DK_make_safe(struct PlayerInfo *player);
DLLIMPORT struct Thing *_DK_create_creature(struct Coord3d *pos, unsigned short a1, unsigned short a2);
DLLIMPORT void _DK_set_creature_level(struct Thing *thing, long nlvl);
DLLIMPORT void _DK_remove_events_thing_is_attached_to(struct Thing *thing);
DLLIMPORT unsigned long _DK_steal_hero(struct PlayerInfo *player, struct Coord3d *pos);
DLLIMPORT void _DK_creature_increase_level(struct Thing *thing);
DLLIMPORT void _DK_clear_slab_dig(long a1, long a2, char a3);
DLLIMPORT void _DK_magic_use_power_hold_audience(unsigned char idx);
DLLIMPORT void _DK_activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player);
DLLIMPORT void _DK_resurrect_creature(struct Thing *thing, unsigned char a2, unsigned char a3, unsigned char a4);
DLLIMPORT void _DK_transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char a3);
DLLIMPORT long _DK_thing_is_special(Thing *thing);
DLLIMPORT int __cdecl _DK_play_smacker_file(char *fname, int);
DLLIMPORT void __cdecl _DK_wait_for_cd_to_be_available(void);
DLLIMPORT void __cdecl _DK_reset_eye_lenses(void);
DLLIMPORT void __cdecl _DK_reset_heap_manager(void);
DLLIMPORT void __cdecl _DK_reset_heap_memory(void);
DLLIMPORT int _DK_LoadMcgaData(void);
DLLIMPORT void _DK_initialise_eye_lenses(void);
DLLIMPORT void _DK_setup_eye_lens(long nlens);
DLLIMPORT void _DK_setup_heap_manager(void);
DLLIMPORT int _DK_setup_heap_memory(void);
DLLIMPORT long _DK_light_create_light(struct InitLight *ilght);
DLLIMPORT void _DK_light_set_light_never_cache(long idx);
DLLIMPORT void _DK_reset_player_mode(struct PlayerInfo *player, unsigned char a2);
DLLIMPORT void _DK_init_keeper_map_exploration(struct PlayerInfo *player);
DLLIMPORT void _DK_init_player_cameras(struct PlayerInfo *player);
DLLIMPORT void _DK_pannel_map_update(long x, long y, long w, long h);
DLLIMPORT void _DK_view_set_camera_y_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_view_set_camera_x_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_view_zoom_camera_in(struct Camera *cam, long a2, long a3);
DLLIMPORT void _DK_set_camera_zoom(struct Camera *cam, long val);
DLLIMPORT void _DK_view_zoom_camera_out(struct Camera *cam, long a2, long a3);
DLLIMPORT long _DK_get_camera_zoom(struct Camera *camera);
DLLIMPORT int __stdcall _DK_setup_game(void);
DLLIMPORT int __stdcall _DK_init_sound(void);
DLLIMPORT int __cdecl _DK_initial_setup(void);
DLLIMPORT long _DK_ceiling_set_info(long a1, long a2, long a3);
DLLIMPORT int __cdecl _DK_process_sound_heap(void);
DLLIMPORT void __cdecl _DK_startup_saved_packet_game(void);
#ifdef __cplusplus
}
#endif

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 1 byte long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_byte(unsigned char *flags,short mask,short value)
{
  if (value) value=mask;
  *flags = (*flags ^ (unsigned char)value) & mask ^ *flags;
}

/*
 * Sets a masked bit in the flags field to the value.
 * This version assumes the flag field is 4 bytes long.
 * @param flags Pointer to the flags byte.
 * @param mask Bitmask for the flag.
 * @param value The new logic value.
 */
void inline set_flag_dword(unsigned long *flags,unsigned long mask,unsigned long value)
{
  if (value) value=mask;
  *flags = (*flags ^ (unsigned long)value) & mask ^ *flags;
}

short show_onscreen_msg_va(int nturns, const char *fmt_str, va_list arg)
{
  vsprintf(onscreen_msg_text, fmt_str, arg);
  LbSyncLog("Onscreen message: %s\n",onscreen_msg_text);
  onscreen_msg_turns = nturns;
  return 1;
}

short show_onscreen_msg(int nturns, const char *fmt_str, ...)
{
    short result;
    va_list val;
    va_start(val, fmt_str);
    result=show_onscreen_msg_va(nturns, fmt_str, val);
    va_end(val);
    return result;
}

void perspective_standard(struct XYZ *cor, struct PolyPoint *ppt)
{
  //_DK_perspective_standard(cor, ppt);
  long i;
  if (cor->z >= 32)
  {
    i = (lens<<16)/(cor->z);
    ppt->field_0 = view_width_over_2 + (i * cor->x >> 16);
    ppt->field_4 = view_height_over_2 - (i * cor->y >> 16);
  } else
  {
    ppt->field_0 = view_width_over_2 + cor->x;
    ppt->field_4 = view_height_over_2 - cor->y;
  }
}

void perspective_fisheye(struct XYZ *cor, struct PolyPoint *ppt)
{ }

void rotpers_parallel(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_parallel(epos, matx);
}

void rotpers_standard(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_standard(epos, matx);
}

void rotpers_circular(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_circular(epos, matx);
}

void rotpers_fisheye(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_fisheye(epos, matx);
}

long pinstfs_hand_grab(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_hand_grab(player, n);
}

long pinstfm_hand_grab(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_hand_grab(player, n);
}

long pinstfe_hand_grab(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_hand_grab(player, n);
}

long pinstfs_hand_drop(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_hand_drop(player, n);
}

long pinstfe_hand_drop(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_hand_drop(player, n);
}

long pinstfs_hand_whip(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_hand_whip(player, n);
}

long pinstfe_hand_whip(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_hand_whip(player, n);
}

long pinstfm_hand_drop(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_hand_drop(player, n);
}

long pinstfs_hand_whip_end(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_hand_whip_end(player, n);
}

long pinstfe_hand_whip_end(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_hand_whip_end(player, n);
}

long pinstfs_control_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_control_creature(player, n);
}

long pinstfm_control_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_control_creature(player, n);
}

long pinstfe_direct_control_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_direct_control_creature(player, n);
}

long pinstfe_passenger_control_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_passenger_control_creature(player, n);
}

long pinstfs_direct_leave_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_direct_leave_creature(player, n);
}

long pinstfm_leave_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_leave_creature(player, n);
}

long pinstfs_passenger_leave_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_passenger_leave_creature(player, n);
}

long pinstfe_leave_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_leave_creature(player, n);
}

long pinstfs_query_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_query_creature(player, n);
}

long pinstfs_unquery_creature(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_unquery_creature(player, n);
}

long pinstfs_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_zoom_to_heart(player, n);
}

long pinstfm_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_zoom_to_heart(player, n);
}

long pinstfe_zoom_to_heart(struct PlayerInfo *player, long *n)
{
  struct PlayerInstanceInfo *inst_info;
  InstncInfo_Func callback;
  long inum;
  //return _DK_pinstfe_zoom_to_heart(player, n);
  inum = player->instance_num%PLAYER_INSTANCES_COUNT;
  if ((inum <= 0) || (player_instance_info[inum].field_4 != 1))
  {
    player->instance_num = 12;
    inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
    player->field_4B1 = inst_info->field_0;
    callback = inst_info->start_cb;
    if (callback != NULL)
      callback(player, &inst_info->field_24);
  }
  LbPaletteStopOpenFade();
  return 0;
}

void leave_creature_as_controller(struct PlayerInfo *player, struct Thing *thing)
{
  _DK_leave_creature_as_controller(player, thing);
}

long pinstfs_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Camera *cam;
  //return _DK_pinstfs_zoom_out_of_heart(player, n);
  thing = game.things_lookup[player->field_2F%THINGS_COUNT];
  if ((thing != NULL) && (thing != game.things_lookup[0]))
    leave_creature_as_controller(player, thing);
  set_player_mode(player, 1);
  cam = player->camera;
  if (cam == NULL) return 0;
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  thing = game.things_lookup[dungeon->field_0%THINGS_COUNT];
  if ((thing == NULL) || (thing == game.things_lookup[0]))
  {
    cam->mappos.x.val = (map_subtiles_x << 8)/2;
    cam->mappos.y.val = (map_subtiles_y << 8)/2;
    cam->field_17 = 24000;
    cam->orient_a = 0;
    return 0;
  }
  cam->mappos.x.val = thing->mappos.x.val;
  if (player->field_37 == 5)
  {
    cam->mappos.y.val = thing->mappos.y.val;
    cam->field_17 = 65536;
  } else
  {
    cam->mappos.y.val = thing->mappos.y.val - (thing->field_58 >> 1) -  thing->mappos.z.val;
    cam->field_17 = 24000;
  }
  cam->orient_a = 0;
  return 0;
}

long pinstfm_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_zoom_out_of_heart(player, n);
}

long pinstfe_zoom_out_of_heart(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_zoom_out_of_heart(player, n);
}

long pinstfm_control_creature_fade(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_control_creature_fade(player, n);
}

long pinstfe_control_creature_fade(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_control_creature_fade(player, n);
}

long pinstfs_fade_to_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_fade_to_map(player, n);
}

long pinstfm_fade_to_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_fade_to_map(player, n);
}

long pinstfe_fade_to_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_fade_to_map(player, n);
}

long pinstfs_fade_from_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_fade_from_map(player, n);
}

long pinstfm_fade_from_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_fade_from_map(player, n);
}

long pinstfe_fade_from_map(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_fade_from_map(player, n);
}

long pinstfs_zoom_to_position(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfs_zoom_to_position(player, n);
}

long pinstfm_zoom_to_position(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfm_zoom_to_position(player, n);
}

long pinstfe_zoom_to_position(struct PlayerInfo *player, long *n)
{
  return _DK_pinstfe_zoom_to_position(player, n);
}

void reset_eye_lenses(void)
{
  //_DK_reset_eye_lenses(); return;
  if (eye_lens_memory != NULL)
  {
    LbMemoryFree(eye_lens_memory);
    eye_lens_memory = NULL;
  }
  if (eye_lens_spare_screen_memory != NULL)
  {
    LbMemoryFree(eye_lens_spare_screen_memory);
    eye_lens_spare_screen_memory = NULL;
  }
  game.flags_cd &= 0xFDu;
  game.numfield_1A = 0;
  game.numfield_1B = 0;
}

void view_set_camera_y_inertia(struct Camera *cam, long a2, long a3)
{
  _DK_view_set_camera_y_inertia(cam, a2, a3);
}

void view_set_camera_x_inertia(struct Camera *cam, long a2, long a3)
{
  _DK_view_set_camera_x_inertia(cam, a2, a3);
}

void view_set_camera_rotation_inertia(struct Camera *cam, long a2, long a3)
{
  _DK_view_set_camera_rotation_inertia(cam, a2, a3);
}

void view_zoom_camera_in(struct Camera *cam, long a2, long a3)
{
  _DK_view_zoom_camera_in(cam, a2, a3);
}

void set_camera_zoom(struct Camera *cam, long val)
{
  _DK_set_camera_zoom(cam, val);
}

void view_zoom_camera_out(struct Camera *cam, long a2, long a3)
{
  _DK_view_zoom_camera_out(cam, a2, a3);
}

long get_camera_zoom(struct Camera *camera)
{
  return _DK_get_camera_zoom(camera);
}

void initialise_eye_lenses(void)
{
  static const char *func_name="initialise_eye_lenses";
  unsigned long screen_size;
  //_DK_initialise_eye_lenses(); return;
  if ((eye_lens_memory != NULL) || (eye_lens_spare_screen_memory != NULL))
  {
    //error(func_name, 17, "EyeLens Memory already allocated");
    reset_eye_lenses();
  }
  if ((features_enabled & Ft_EyeLens) == 0)
  {
    game.flags_cd &= 0xFDu;
    return;
  }

  //TODO: Hack for compatibility - eye lens supported only in 3 modes
  if ((lbDisplay.ScreenMode != Lb_SCREEN_MODE_320_200_8) &&
      (lbDisplay.ScreenMode != Lb_SCREEN_MODE_640_400_8) &&
      (lbDisplay.ScreenMode != Lb_SCREEN_MODE_640_480_8))
  {
    LbWarnLog("EyeLens not supported in current screen mode\n");
    game.flags_cd &= 0xFDu;
    return;
  }

  eye_lens_width = lbDisplay.PhysicalScreenWidth;
  eye_lens_height = lbDisplay.PhysicalScreenHeight;
  screen_size = eye_lens_width * eye_lens_height + 2;
  eye_lens_memory = (unsigned long *)LbMemoryAlloc(screen_size*sizeof(unsigned long));
  eye_lens_spare_screen_memory = (unsigned char *)LbMemoryAlloc(screen_size*sizeof(TbPixel));
  if ((eye_lens_memory == NULL) || (eye_lens_spare_screen_memory == NULL))
  {
    reset_eye_lenses();
    error(func_name, 141, "Cannot allocate EyeLens Memory");
    return;
  }
  game.flags_cd |= MFlg_EyeLensReady;
}

void init_lens(unsigned long *lens_mem, int width, int height, int scanln, int nlens)
{
  _DK_init_lens(lens_mem, width, height, scanln, nlens);
}

void flyeye_setup(long width, long height)
{
  _DK_flyeye_setup(width, height);
}

void setup_eye_lens(long nlens)
{
  static const char *func_name="setup_eye_lens";
  //_DK_setup_eye_lens(nlens);return;
  struct PlayerInfo *player;
  char *fname;

  if ((game.flags_cd & MFlg_EyeLensReady) == 0)
    return;
  //TODO: Temporary hack, until CMistFade is not rewritten
  if ((nlens >= 4) && (nlens <= 12))
  {
    _DK_setup_eye_lens(nlens);
    return;
  }
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if ((game.numfield_1B >= 13) && (game.numfield_1B <= 14))
  {
      player->field_7 = 0;
      game.numfield_1A = 0;
  }
  if (nlens < 1)
  {
    game.numfield_1A = 0;
    game.numfield_1B = 0;
    return;
  }
  if (game.numfield_1A == nlens)
  {
    game.numfield_1B = nlens;
    return;
  }
  switch (nlens)
  {
  case 1:
  case 2:
      init_lens(eye_lens_memory, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size,
              lbDisplay.GraphicsScreenWidth, nlens);
      break;
  case 3:
      flyeye_setup(MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
      break;
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
      fname = prepare_file_fmtpath(FGrp_StdData,"frac%02d.dat",nlens-4);
      LbFileLoadAt(fname, eye_lens_memory);
/*
      Mist->setup(eye_lens_memory, fade_tables, ghost);
      Mist->animset(0, 1024);
*/
      break;
  case 12:
      fname = prepare_file_fmtpath(FGrp_StdData,"frac%02d.dat",nlens-4);
      LbFileLoadAt(fname, eye_lens_memory);
/*
      Mist->setup(eye_lens_memory, &fade_tables[1024], ghost);
      Mist->animset(0, 1024);
*/
      break;
  case 13:
      player->field_4C9 = dog_palette;
      player->field_7 = dog_palette;
      break;
  case 14:
      player->field_4C9 = vampire_palette;
      player->field_7 = vampire_palette;
      break;
  default:
      error(func_name, 159, "Invalid lens effect");
      nlens = 0;
      break;
  }
  game.numfield_1B = nlens;
  game.numfield_1A = nlens;
}

void reinitialise_eye_lens(long nlens)
{
  initialise_eye_lenses();
  if ((game.flags_cd & MFlg_EyeLensReady) && (nlens>0))
  {
      game.numfield_1B = 0;
      setup_eye_lens(nlens);
  }
}

void draw_jonty_mapwho(struct JontySpr *jspr)
{
  _DK_draw_jonty_mapwho(jspr);
}

void draw_keepsprite_unscaled_in_buffer(unsigned short a1, short a2, unsigned char a3, unsigned char *a4)
{
  _DK_draw_keepsprite_unscaled_in_buffer(a1, a2, a3, a4);
}

void draw_engine_number(struct Number *num)
{
  _DK_draw_engine_number(num);
}

void draw_engine_room_flag_top(struct RoomFlag *rflg)
{
  _DK_draw_engine_room_flag_top(rflg);
}

void setup_heap_manager(void)
{
  _DK_setup_heap_manager();
}

int setup_heap_memory(void)
{
  return _DK_setup_heap_memory();
}

void reset_heap_manager(void)
{
  _DK_reset_heap_manager();
}

void reset_heap_memory(void)
{
  _DK_reset_heap_memory();
}

long light_create_light(struct InitLight *ilght)
{
  return _DK_light_create_light(ilght);
}

void light_set_light_never_cache(long idx)
{
  _DK_light_set_light_never_cache(idx);
}

long light_is_light_allocated(long lgt_id)
{
  return _DK_light_is_light_allocated(lgt_id);
}

void light_set_light_position(long lgt_id, struct Coord3d *pos)
{
  _DK_light_set_light_position(lgt_id, pos);
}

unsigned long scale_camera_zoom_to_screen(unsigned long zoom_lvl)
{
  // Note: I don't know if the zoom may be scaled for current resolution,
  // as there may be different resolution on another computer if playing MP game.
  return ((zoom_lvl*units_per_pixel) >> 4)*pixel_size;
}

void init_player_as_single_keeper(struct PlayerInfo *player)
{
  unsigned short idx;
  struct InitLight ilght;
  memset(&ilght, 0, sizeof(struct InitLight));
  player->field_4CD = 0;
  ilght.field_0 = 0x0A00;
  ilght.field_2 = 48;
  ilght.field_3 = 5;
  ilght.field_11 = 1;
  idx = light_create_light(&ilght);
  player->field_460 = idx;
  light_set_light_never_cache(idx);
}

TbPixel get_player_path_colour(unsigned short owner)
{
  return player_path_colours[player_colors_map[owner % PLAYERS_EXT_COUNT]];
}

short send_creature_to_room(struct Thing *thing, struct Room *room)
{
  return _DK_send_creature_to_room(thing, room);
}

struct Room *get_room_thing_is_on(struct Thing *thing)
{
  return _DK_get_room_thing_is_on(thing);
}

short set_start_state(struct Thing *thing)
{
  return _DK_set_start_state(thing);
}

struct Thing *get_trap_for_position(long x, long y)
{
  return _DK_get_trap_for_position(x, y);
}

void reset_player_mode(struct PlayerInfo *player, unsigned char a2)
{
  _DK_reset_player_mode(player, a2);
}

void init_keeper_map_exploration(struct PlayerInfo *player)
{
  _DK_init_keeper_map_exploration(player);
}

void init_player_cameras(struct PlayerInfo *player)
{
  _DK_init_player_cameras(player);
}

void init_dungeons_research(void)
{
  struct Dungeon *dungeon;
  int i;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = (&game.dungeon[i]);
    dungeon->field_F78 = get_next_research_item(dungeon);
  }
}

void init_creature_state(struct Thing *thing)
{
  struct Room *room;
  if (thing->owner != game.field_14E497)
  {
    room = get_room_thing_is_on(thing);
    if (room != NULL)
    {
        switch (room->field_A)
        {
        case 4:
        case 5:
        case 16:
            if ( send_creature_to_room(thing, room) )
              return;
        default:
            break;
        }
    }
  }
  set_start_state(thing);
}

void clear_creature_pool(void)
{
  int i;
  for (i=0; i < CREATURE_TYPES_COUNT; i++)
  {
    game.creature_pool[i] = 0;
  }
}

void clear_slab_dig(long a1, long a2, char a3)
{
  _DK_clear_slab_dig(a1, a2, a3);
}

TbError LbNetwork_Init(unsigned long srvcp,struct _GUID guid, unsigned long maxplayrs, void *exchng_buf, unsigned long exchng_size, struct TbNetworkPlayerInfo *locplayr, struct SerialInitData *init_data)
{
  return _DK_LbNetwork_Init(srvcp,guid,maxplayrs,exchng_buf,exchng_size,locplayr,init_data);
}

TbError LbNetwork_ChangeExchangeBuffer(void *buf, unsigned long a2)
{
  return _DK_LbNetwork_ChangeExchangeBuffer(buf, a2);
}

void LbNetwork_ChangeExchangeTimeout(unsigned long tmout)
{
//  exchangeTimeout = 1000 * tmout;
}

short inline calculate_moon_phase(short do_calculate,short add_to_log)
{
  //Moon phase calculation
  if (do_calculate)
  {
    phase_of_moon=PhaseOfMoon::Calculate();
  }
  if ((phase_of_moon > -0.05) && (phase_of_moon < 0.05))
  {
    if (add_to_log)
      LbSyncLog("Full moon %.4f\n", phase_of_moon);
    game.is_full_moon = 1;
    is_new_moon = 0;
  } else
  if ((phase_of_moon < -0.95) || (phase_of_moon > 0.95))
  {
    if (add_to_log)
      LbSyncLog("New moon %.4f\n", phase_of_moon);
    game.is_full_moon = 0;
    is_new_moon = 1;
  } else
  {
    if (add_to_log)
      LbSyncLog("Moon phase %.4f\n", phase_of_moon);
    game.is_full_moon = 0;
    is_new_moon = 0;
  }

  return game.is_full_moon;
}

/*
 * Copies the given RAW image at center of screen buffer and swaps video
 * buffers to make the image visible.
 * @return Returns true on success.
 */
short copy_raw8_image_buffer(unsigned char *dst_buf,const int scanline,const int nlines,const int spx,const int spy,const unsigned char *src_buf,const int src_width,const int src_height,const int m)
{
  int w,h,i,k;
  unsigned char *dst;
  const unsigned char *src;
  w=0;
  h=0;
  // Clearing top of the canvas
  if (spy>0)
  {
    for (h=0; h<spy; h++)
    {
      dst = dst_buf + (h)*scanline;
      memset(dst,0,scanline);
    }
    // Clearing bottom of the canvas
    // (Note: it must be done before drawing, to make sure we won't overwrite last line)
    for (h=nlines-spy; h<nlines; h++)
    {
      dst = dst_buf + (h)*scanline;
      memset(dst,0,scanline);
    }
  }
  // Now drawing
  for (h=0; h<src_height; h++)
  {
    src = src_buf + h*src_width;
    for (k=0; k<m; k++)
    {
      if (spy+m*h+k<0) continue;
      if (spy+m*h+k>=nlines) break;
      dst = dst_buf + (spy+m*h+k)*scanline + spx;
      for (w=0; w<src_width; w++)
      {
        for (i=0;i<m;i++)
        {
            dst[m*w+i] = src[w];
        }
      }
    }
  }
  return true;
}

/*
 * Copies the given RAW image at center of screen buffer and swaps video
 * buffers to make the image visible.
 * @return Returns true on success.
 */
short copy_raw8_image_to_screen_center(const unsigned char *buf,const int img_width,const int img_height)
{
  struct TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(lbDisplay.ScreenMode);
  int w,h,m,i,k;
  int spx,spy;
  unsigned char *dst;
  const unsigned char *src;
  // Only 8bpp supported for now
  if (mdinfo->BitsPerPixel != 8)
    return false;
  w=0;
  h=0;
  for (m=0;m<5;m++)
  {
    w+=img_width;
    h+=img_height;
    if (w > mdinfo->Width) break;
    if (h > mdinfo->Height) break;
  }
  // The image width can't be larger than video resolution
  if (m<1)
  {
    if (w > mdinfo->Width)
    {
      LbSyncLog("The %dx%d image does not fit on %dx%d screen, skipped.\n", img_width, img_height,mdinfo->Width,mdinfo->Height);
      return false;
    }
    m=1;
  }
  // Locking screen
  if (LbScreenLock()!=1)
    return false;
  // Starting point coords
  spx = (mdinfo->Width-m*img_width)>>1;
  spy = (mdinfo->Height-m*img_height)>>1;
  copy_raw8_image_buffer(lbDisplay.WScreen,mdinfo->Width,mdinfo->Height,
      spx,spy,buf,img_width,img_height,m);
  perform_any_screen_capturing();
  LbScreenUnlock();
  LbScreenSwap();
  return true;
}

short show_rawimage_screen(unsigned char *raw,unsigned char *pal,int width,int height,TbClockMSec tmdelay)
{
  static const char *func_name="show_rawimage_screen";
      if (height>lbDisplay.PhysicalScreenHeight)
           height=lbDisplay.PhysicalScreenHeight;
      LbPaletteSet(pal);
      TbClockMSec end_time;
      end_time = LbTimerClock() + tmdelay;
      TbClockMSec tmdelta;
      tmdelta = tmdelay/100;
      if (tmdelta>100) tmdelta=100;
      if (tmdelta<10) tmdelta=10;
      while (LbTimerClock() < end_time)
      {
          LbWindowsControl();
          copy_raw8_image_to_screen_center(raw,width,height);
          if (is_key_pressed(KC_SPACE,KM_DONTCARE) ||
              is_key_pressed(KC_ESCAPE,KM_DONTCARE) ||
              is_key_pressed(KC_RETURN,KM_DONTCARE) ||
              is_mouse_pressed_lrbutton())
          {
              clear_key_pressed(KC_SPACE);
              clear_key_pressed(KC_ESCAPE);
              clear_key_pressed(KC_RETURN);
              clear_mouse_pressed_lrbutton();
              break;
          }
          LbSleepFor(tmdelta);
      }
}

short LbPaletteStopOpenFade(void)
{
    lbfade_open = 0;
    return 1;
}

void ProperFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg)
{
    if ( _DK_lbUseSdk )
    {
        TbClockMSec last_loop_time;
        last_loop_time = LbTimerClock();
        while ( LbPaletteFade(pal, n, flg) < n )
        {
          if (!is_key_pressed(KC_SPACE,KM_DONTCARE) &&
              !is_key_pressed(KC_ESCAPE,KM_DONTCARE) &&
              !is_key_pressed(KC_RETURN,KM_DONTCARE) &&
              !is_mouse_pressed_lrbutton())
          {
            LbSleepUntil(last_loop_time+50);
          }
          last_loop_time = LbTimerClock();
        }
    } else
    if ( pal != NULL )
    {
        LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        LbPaletteSet(palette_buf);
    }
}

void ProperForcedFadePalette(unsigned char *pal, long n, TbPaletteFadeFlag flg)
{
    if (flg==Lb_PALETTE_FADE_OPEN)
    {
        LbPaletteFade(pal, n, flg);
        return;
    }
    if ( _DK_lbUseSdk )
    {
        TbClockMSec last_loop_time;
        last_loop_time = LbTimerClock();
        while ( LbPaletteFade(pal, n, flg) < n )
        {
          LbSleepUntil(last_loop_time+50);
          last_loop_time = LbTimerClock();
        }
    } else
    if ( pal != NULL )
    {
        LbPaletteSet(pal);
    } else
    {
        memset(palette_buf, 0, sizeof(palette_buf));
        LbPaletteSet(palette_buf);
    }
}

void move_thing_in_map(struct Thing *thing, struct Coord3d *pos)
{
  static const char *func_name="move_thing_in_map";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_move_thing_in_map(thing, pos);
  if ((thing->mappos.x.stl.num == pos->x.stl.num) && (thing->mappos.y.stl.num == pos->y.stl.num))
  {
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
  } else
  {
    remove_thing_from_mapwho(thing);
    thing->mappos.x.val = pos->x.val;
    thing->mappos.y.val = pos->y.val;
    thing->mappos.z.val = pos->z.val;
    place_thing_in_mapwho(thing);
  }
  thing->field_60 = get_thing_height_at(thing, &thing->mappos);
}

void process_creature_instance(struct Thing *thing)
{
  _DK_process_creature_instance(thing);
}

void update_creature_count(struct Thing *thing)
{
  _DK_update_creature_count(thing);
}

long process_creature_state(struct Thing *thing)
{
  return _DK_process_creature_state(thing);
}

long get_floor_height_under_thing_at(struct Thing *thing, struct Coord3d *pos)
{
  return _DK_get_floor_height_under_thing_at(thing, pos);
}

long move_creature(struct Thing *thing)
{
  return _DK_move_creature(thing);
}

void process_spells_affected_by_effect_elements(struct Thing *thing)
{
  _DK_process_spells_affected_by_effect_elements(thing);
}

long get_top_cube_at_pos(long mpos)
{
  return _DK_get_top_cube_at_pos(mpos);
}

void apply_damage_to_thing_and_display_health(struct Thing *thing, long a1, char a2)
{
  _DK_apply_damage_to_thing_and_display_health(thing, a1, a2);
}

long get_foot_creature_has_down(struct Thing *thing)
{
  return _DK_get_foot_creature_has_down(thing);
}

void process_disease(struct Thing *thing)
{
  static const char *func_name="process_disease";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_disease(thing);
}

void set_creature_graphic(struct Thing *thing)
{
  _DK_set_creature_graphic(thing);
}

void process_keeper_spell_effect(struct Thing *thing)
{
  _DK_process_keeper_spell_effect(thing);
}

long creature_is_group_leader(struct Thing *thing)
{
  return _DK_creature_is_group_leader(thing);
}

void leader_find_positions_for_followers(struct Thing *thing)
{
  _DK_leader_find_positions_for_followers(thing);
}

long update_creature_levels(struct Thing *thing)
{
  return _DK_update_creature_levels(thing);
}

long process_creature_self_spell_casting(struct Thing *thing)
{
  return _DK_process_creature_self_spell_casting(thing);
}

void process_thing_spell_effects(struct Thing *thing)
{
  _DK_process_thing_spell_effects(thing);
}

short thing_is_special(struct Thing *thing)
{
  if ((thing->class_id != 1) || (thing->model >= OBJECT_TYPES_COUNT))
    return false;
  return (object_to_special[thing->model] > 0);
}

short kill_creature(struct Thing *thing, struct Thing *tngrp, char a1, unsigned char a2, unsigned char a3, unsigned char a4)
{
  return _DK_kill_creature(thing, tngrp, a1, a2, a3, a4);
}

long update_object(struct Thing *thing)
{
  static const char *func_name="update_object";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_update_object(thing);
}

long update_shot(struct Thing *thing)
{
  static const char *func_name="update_shot";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_update_shot(thing);
}

long update_effect_element(struct Thing *thing)
{
  static const char *func_name="update_effect_element";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_update_effect_element(thing);
}

long update_dead_creature(struct Thing *thing)
{
  static const char *func_name="update_dead_creature";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_update_dead_creature(thing);
}

short update_creature_movements(struct Thing *thing)
{
  static const char *func_name="update_creature_movements";
  struct CreatureControl *cctrl;
  short upd_done;
  int i;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
  upd_done = 0;
  if (cctrl->field_AB != 0)
  {
    upd_done = 1;
    cctrl->pos_BB.x.val = 0;
    cctrl->pos_BB.y.val = 0;
    cctrl->pos_BB.z.val = 0;
    cctrl->field_C8 = 0;
    cctrl->field_2 &= 0xFEu;
  } else
  {
    if ( thing->field_0 & 0x20 )
    {
      if ( thing->field_25 & 0x20 )
      {
        if (cctrl->field_C8 != 0)
        {
          cctrl->pos_BB.x.val = (LbSinL(thing->field_52)>> 8)
                * (cctrl->field_C8 * LbCosL(thing->field_54) >> 8) >> 16;
          cctrl->pos_BB.y.val = -((LbCosL(thing->field_52) >> 8)
                * (cctrl->field_C8 * LbCosL(thing->field_54) >> 8) >> 8) >> 8;
          cctrl->pos_BB.z.val = cctrl->field_C8 * LbSinL(thing->field_54) >> 16;
        }
        if (cctrl->field_CA != 0)
        {
          cctrl->pos_BB.x.val +=   cctrl->field_CA * LbSinL(thing->field_52 - 512) >> 16;
          cctrl->pos_BB.y.val += -(cctrl->field_CA * LbCosL(thing->field_52 - 512) >> 8) >> 8;
        }
      } else
      {
        if (cctrl->field_C8 != 0)
        {
          upd_done = 1;
          cctrl->pos_BB.x.val =   cctrl->field_C8 * lbSinTable[thing->field_52 & 0x7FF] >> 16;
          cctrl->pos_BB.y.val = -(cctrl->field_C8 * lbCosTable[thing->field_52 & 0x7FF] >> 8) >> 8;
        }
        if (cctrl->field_CA != 0)
        {
          upd_done = 1;
          cctrl->pos_BB.x.val +=   cctrl->field_CA * lbSinTable[(thing->field_52 - 512) & 0x7FF] >> 16;
          cctrl->pos_BB.y.val += -(cctrl->field_CA * lbCosTable[(thing->field_52 - 512) & 0x7FF] >> 8) >> 8;
        }
      }
    } else
    if (cctrl->field_2 & 0x01)
    {
      upd_done = 1;
      cctrl->field_2 &= 0xFE;
    } else
    if (cctrl->field_C8 != 0)
    {
      upd_done = 1;
      cctrl->pos_BB.x.val =   cctrl->field_C8 * lbSinTable[thing->field_52 & 0x7FF] >> 16;
      cctrl->pos_BB.y.val = -(cctrl->field_C8 * lbCosTable[thing->field_52 & 0x7FF] >> 8) >> 8;
      cctrl->pos_BB.z.val = 0;
    }
    if (((thing->field_25 & 0x20) != 0) && ((thing->field_0 & 0x20) == 0))
    {
      i = get_floor_height_under_thing_at(thing, &thing->mappos) - thing->mappos.z.val + 256;
      if (i > 0)
      {
        upd_done = 1;
        if (i >= 32)
          i = 32;
        cctrl->pos_BB.z.val += i;
      } else
      if (i < 0)
      {
        upd_done = 1;
        i = -i;
        if (i >= 32)
          i = 32;
        cctrl->pos_BB.z.val -= i;
      }
    }
  }
#if (BFDEBUG_LEVEL > 19)
    LbSyncLog("%s: Finished\n",func_name);
#endif
  if (upd_done)
    return true;
  else
    return ((cctrl->pos_BB.x.val != 0) || (cctrl->pos_BB.y.val != 0) || (cctrl->pos_BB.z.val != 0));
}

struct Thing *create_thing(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4)
{
  return _DK_create_thing(pos, a1, a2, a3, a4);
}

struct Thing *create_door(struct Coord3d *pos, unsigned short a1, unsigned char a2, unsigned short a3, unsigned char a4)
{
  return _DK_create_door(pos, a1, a2, a3, a4);
}

struct Thing *create_effect_generator(struct Coord3d *pos, unsigned short a1, unsigned short a2, unsigned short a3, long a4)
{
  return _DK_create_effect_generator(pos, a1, a2, a3, a4);
}

void process_creature_standing_on_corpses_at(struct Thing *thing, struct Coord3d *pos)
{
  _DK_process_creature_standing_on_corpses_at(thing, pos);
}

long cleanup_current_thing_state(struct Thing *thing)
{
  return _DK_cleanup_current_thing_state(thing);
}

unsigned long setup_move_off_lava(struct Thing *thing)
{
  return _DK_setup_move_off_lava(thing);
}

struct Thing *create_footprint_sine(struct Coord3d *crtr_pos, unsigned short phase, short nfoot, unsigned short model, unsigned short owner)
{
  struct Coord3d pos;
  unsigned int i;
  pos.x.val = crtr_pos->x.val;
  pos.y.val = crtr_pos->y.val;
  pos.z.val = crtr_pos->z.val;
  switch (nfoot)
  {
  case 1:
      i = ((phase - 512) & 0x7FF);
      pos.x.val +=   (lbSinTable[i] << 6) >> 16;
      pos.y.val += -((lbCosTable[i] << 6) >> 8) >> 8;
      return create_thing(&pos, 3, model, owner, -1);
  case 2:
      i = ((phase - 512) & 0x7FF);
      pos.x.val -=   (lbSinTable[i] << 6) >> 16;
      pos.y.val -= -((lbCosTable[i] << 6) >> 8) >> 8;
      return create_thing(&pos, 3, model, owner, -1);
  }
  return NULL;
}

void place_bloody_footprint(struct Thing *thing)
{
  struct CreatureControl *cctrl;
  short nfoot;
  cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
  nfoot = get_foot_creature_has_down(thing);
  switch (creatures[thing->model%CREATURE_TYPES_COUNT].field_6)
  {
  case 3:
  case 4:
      break;
  case 5:
      if (nfoot)
      {
        if (create_thing(&thing->mappos, 3, 23, thing->owner, -1) != NULL)
          cctrl->bloody_footsteps_turns--;
      }
      break;
  default:
      if (create_footprint_sine(&thing->mappos, thing->field_52, nfoot, 23, thing->owner) != NULL)
        cctrl->bloody_footsteps_turns--;
      break;
  }
}

void process_landscape_affecting_creature(struct Thing *thing)
{
  static const char *func_name="process_landscape_affecting_creature";
  struct CreatureControl *cctrl;
  int stl_idx;
  short nfoot;
  int i;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
  thing->field_25 &= 0xFEu;
  thing->field_25 &= 0xFDu;
  thing->field_25 &= 0x7Fu;
  cctrl->field_B9 = 0;

  stl_idx = (thing->mappos.y.stl.num << 8) + thing->mappos.x.stl.num;
  if (((game.field_CC157[stl_idx] & 0xF) << 8) == thing->mappos.z.val)
  {
    i = get_top_cube_at_pos(stl_idx);
    if ((i & 0xFFFFFFFE) == 40)
    {
      apply_damage_to_thing_and_display_health(thing, game.creature_stats[thing->model].field_18, -1);
      thing->field_25 |= 0x02;
    } else
    if (i == 39)
    {
      thing->field_25 |= 0x01;
    }

    if (thing->field_25 & 0x01)
    {
      nfoot = get_foot_creature_has_down(thing);
      if (nfoot)
      {
        create_effect(&thing->mappos, 19, thing->owner);
      }
      cctrl->bloody_footsteps_turns = 0;
    } else
    // Bloody footprints
    if (cctrl->bloody_footsteps_turns != 0)
    {
      place_bloody_footprint(thing);
      nfoot = get_foot_creature_has_down(thing);
      if (create_footprint_sine(&thing->mappos, thing->field_52, nfoot, 23, thing->owner) != NULL)
        cctrl->bloody_footsteps_turns--;
    } else
    // Snow footprints
    if (game.texture_id == 2)
    {
      stl_idx = map_to_slab[thing->mappos.x.stl.num] + map_tiles_x * map_to_slab[thing->mappos.y.stl.num];
      if (game.slabmap[stl_idx].slab == 10)
      {
        thing->field_25 |= 0x80u;
        nfoot = get_foot_creature_has_down(thing);
        create_footprint_sine(&thing->mappos, thing->field_52, nfoot, 94, thing->owner);
      }
    }
    process_creature_standing_on_corpses_at(thing, &thing->mappos);
  }
  if (((thing->field_0 & 0x20) == 0) && ((thing->field_25 & 0x02) != 0))
  {
    if (game.creature_stats[thing->model%CREATURE_TYPES_COUNT].field_18)
    {
        if (thing->field_7 == 14)
          i = thing->field_8;
        else
          i = thing->field_7;
        if ((i != -113) && (cctrl->field_2FE + 64 < game.seedchk_random_used))
        {
            cctrl->field_2FE = game.seedchk_random_used;
            if ( cleanup_current_thing_state(thing) )
            {
              if ( setup_move_off_lava(thing) )
                thing->field_8 = -113;
              else
                set_start_state(thing);
            }
        }
    }
  }
#if (BFDEBUG_LEVEL > 19)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

long update_creature(struct Thing *thing)
{
  static const char *func_name="update_creature";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_update_creature(thing);
  struct PlayerInfo *player;
  struct CreatureControl *cctrl;
  struct Thing *tngp;
  int stl_idx;
  int i;

  cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
  stl_idx = (thing->mappos.y.stl.num << 8) + thing->mappos.x.stl.num;
  if ((thing->field_7 == 67) && (game.map[stl_idx].field_0 & 0x40))
  {
    kill_creature(thing, game.things_lookup[0], -1, 1, 0, 1);
    return 0;
  }
  if (thing->field_5E < 0)
  {
    kill_creature(thing, game.things_lookup[0], -1, 0, 0, 0);
    return 0;
  }
  if (game.field_150356)
  {
    if ((cctrl->field_2EF != 0) && (cctrl->field_2EF <= game.seedchk_random_used))
    {
        cctrl->field_2EF = 0;
        create_effect(&thing->mappos, imp_spangle_effects[thing->owner], thing->owner);
        move_thing_in_map(thing, &game.pos_1517F0);
    }
  }
  if (cctrl->field_B1 > 0)
    cctrl->field_B1--;
  if (cctrl->field_8B == 0)
    cctrl->field_8B = game.field_14EA4B;
  if (cctrl->field_302 == 0)
    process_creature_instance(thing);
  update_creature_count(thing);
  if (thing->field_0 & 0x20)
  {
    if (cctrl->field_AB == 0)
    {
      if (cctrl->field_302 != 0)
      {
        cctrl->field_302--;
      } else
      if (process_creature_state(thing))
      {
        error(func_name, 970, "A state return type for a human controlled creature?");
      }
    }
    cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
    player = &(game.players[thing->owner%PLAYERS_COUNT]);
    if (cctrl->field_AB & 0x02)
    {
      if ((player->field_3 & 0x04) == 0)
        PaletteSetPlayerPalette(player, blue_palette);
    } else
    {
      if ((player->field_3 & 0x04) != 0)
        PaletteSetPlayerPalette(player, _DK_palette);
    }
  } else
  {
    if (cctrl->field_AB == 0)
    {
      if (cctrl->field_302 > 0)
      {
        cctrl->field_302--;
      } else
      if (process_creature_state(thing))
      {
        return 0;
      }
    }
  }

  if (update_creature_movements(thing))
  {
    thing->pos_38.x.val += cctrl->pos_BB.x.val;
    thing->pos_38.y.val += cctrl->pos_BB.y.val;
    thing->pos_38.z.val += cctrl->pos_BB.z.val;
  }
  move_creature(thing);
  if ((thing->field_0 & 0x20) != 0)
  {
    if ((cctrl->field_0 & 0x4000) == 0)
      cctrl->field_C8 /= 2;
    if ((cctrl->field_0 & 0x8000) == 0)
      cctrl->field_CA /= 2;
  } else
  {
    cctrl->field_C8 = 0;
  }
  process_spells_affected_by_effect_elements(thing);
  process_landscape_affecting_creature(thing);
  process_disease(thing);
  move_thing_in_map(thing, &thing->mappos);
  set_creature_graphic(thing);
  if (cctrl->field_2B0)
    process_keeper_spell_effect(thing);

  if (thing->word_17 > 0)
    thing->word_17--;

  if (cctrl->field_7A & 0x0FFF)
  {
    if ( creature_is_group_leader(thing) )
      leader_find_positions_for_followers(thing);
  }

  if (cctrl->field_6E > 0)
  {
    tngp = game.things_lookup[cctrl->field_6E%THINGS_COUNT];
    if (tngp->field_1 & 0x01)
      move_thing_in_map(tngp, &thing->mappos);
  }
  if (update_creature_levels(thing) == -1)
  {
    return 0;
  }
  process_creature_self_spell_casting(thing);
  cctrl->pos_BB.x.val = 0;
  cctrl->pos_BB.y.val = 0;
  cctrl->pos_BB.z.val = 0;
  cctrl->field_0 &= 0xBFFFu;
  cctrl->field_0 &= 0x7FFF;
  cctrl->field_AD &= 0xFBu;
  process_thing_spell_effects(thing);
  return 1;
}

long update_effect(struct Thing *thing)
{
  static const char *func_name="update_effect";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_update_effect(thing);
}

long process_effect_generator(struct Thing *thing)
{
  static const char *func_name="process_effect_generator";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_process_effect_generator(thing);
}

long update_trap(struct Thing *thing)
{
  static const char *func_name="update_trap";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_update_trap(thing);
}

long process_door(struct Thing *thing)
{
  static const char *func_name="process_door";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_process_door(thing);
}

void update_thing_animation(struct Thing *thing)
{
  static const char *func_name="update_thing_animation";
  int i;
  struct CreatureControl *cctrl;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_update_thing_animation(thing); return;
  if (thing->class_id == 5)
  {
    cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
    if (cctrl >= game.creature_control_data)
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
      if (thing->field_50 & 0x02)
        thing->field_4A = -thing->field_4A;
      else
        thing->field_4A = 0;
    }
  }
}

long update_thing(struct Thing *thing)
{
  static const char *func_name="update_thing";
  Thing_Class_Func classfunc;
  struct Coord3d pos;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //return _DK_update_thing(thing);

  if ((thing->field_25 & 0x40) == 0)
  {
    if ((thing->field_1 & 0x04) != 0)
    {
      thing->pos_2C.x.val += thing->pos_32.x.val;
      thing->pos_2C.y.val += thing->pos_32.y.val;
      thing->pos_2C.z.val += thing->pos_32.z.val;
      thing->pos_32.x.val = 0;
      thing->pos_32.y.val = 0;
      thing->pos_32.z.val = 0;
      thing->field_1 &= 0xFBu;
    }
    thing->pos_38.x.val = thing->pos_2C.x.val;
    thing->pos_38.y.val = thing->pos_2C.y.val;
    thing->pos_38.z.val = thing->pos_2C.z.val;
    if (thing->field_1 & 0x08)
    {
      thing->pos_38.x.val += thing->pos_26.x.val;
      thing->pos_38.y.val += thing->pos_26.y.val;
      thing->pos_38.z.val += thing->pos_26.z.val;
      thing->pos_26.x.val = 0;
      thing->pos_26.y.val = 0;
      thing->pos_26.z.val = 0;
      thing->field_1 &= 0xF7u;
    }
  }
  classfunc = class_functions[thing->class_id%THING_CLASSES_COUNT];
  if (classfunc == NULL)
      return 0;
  if (classfunc(thing) < 0)
      return 0;
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Class function end ok\n",func_name);
#endif
  if ((thing->field_25 & 0x40) == 0)
  {
      if (thing->mappos.z.val > thing->field_60)
      {
        if (thing->pos_2C.x.val != 0)
          thing->pos_2C.x.val = thing->pos_2C.x.val * (256 - thing->field_24) / 256;
        if (thing->pos_2C.y.val != 0)
          thing->pos_2C.y.val = thing->pos_2C.y.val * (256 - thing->field_24) / 256;
        if ((thing->field_25 & 0x20) == 0)
        {
          thing->pos_32.z.val -= thing->field_20;
          thing->field_1 |= 0x04;
        }
      } else
      {
        if (thing->pos_2C.x.val != 0)
          thing->pos_2C.x.val = thing->pos_2C.x.val * (256 - thing->field_23) / 256;
        if (thing->pos_2C.y.val != 0)
          thing->pos_2C.y.val = thing->pos_2C.y.val * (256 - thing->field_23) / 256;
        thing->mappos.z.val = thing->field_60;
        if ((thing->field_25 & 0x08) != 0)
        {
          thing->pos_2C.z.val = 0;
        }
      }
  }
  update_thing_animation(thing);
  update_thing_sound(thing);
  if ((do_lights) && (thing->field_62 != 0))
  {
      if (light_is_light_allocated(thing->field_62))
      {
        pos.x.val = thing->mappos.x.val;
        pos.y.val = thing->mappos.y.val;
        pos.z.val = thing->mappos.z.val + thing->field_58;
        light_set_light_position(thing->field_62, &pos);
      } else
      {
        thing->field_62 = 0;
      }
  }
  return 1;
}

TbBigChecksum get_thing_checksum(struct Thing *thing)
{
  static const char *func_name="get_thing_checksum";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_get_thing_checksum(thing);
}

short update_thing_sound(struct Thing *thing)
{
  //return _DK_update_thing_sound(thing);
  if (thing->field_66)
  {
    if ( S3DEmitterHasFinishedPlaying(thing->field_66) )
    {
      S3DDestroySoundEmitter(thing->field_66);
      thing->field_66 = 0;
    } else
    {
      S3DMoveSoundEmitterTo(thing->field_66,
          thing->mappos.x.val, thing->mappos.y.val, thing->mappos.z.val);
    }
  }
  return true;
}

/*
 * Displays the 320x200 loading screen.
 * Will work properly even on 640x4?0 resolutions.
 * @return Returns true on success.
 */
short display_lores_loading_screen(void)
{
  static const char *func_name="display_lores_loading_screen";
  const int img_width = 320;
  const int img_height = 200;
  short dproceed;
  char *fname;
  unsigned char *buf;

  buf=NULL;
  memset(palette_buf, 0, 768);
  LbPaletteSet(palette_buf);
  wait_for_cd_to_be_available();
  dproceed = true;
  if (dproceed)
  {
      buf=(unsigned char *)malloc(img_width*img_height);
      if (buf==NULL)
      {
          error(func_name, 1056, "Cannot allocate bitmap buffer");
          dproceed = false;
      }
  }
  if (dproceed)
  {
      fname=prepare_file_path(FGrp_StdData,"loading.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      wait_for_cd_to_be_available();
      fname=prepare_file_path(FGrp_StdData,"loading.pal");
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      LbScreenClear(0);
      dproceed=copy_raw8_image_to_screen_center(buf,img_width,img_height);
  }
  free(buf);
  return dproceed;
}

/*
 * Displays the 640x480 loading screen.
 * Will work properly only on high resolutions.
 * @return Returns true on success.
 */
short display_hires_loading_screen(void)
{
  static const char *func_name="display_hires_loading_screen";
  const int img_width = 640;
  const int img_height = 480;
  char *fname;
  unsigned char *buf=NULL;

  memset(palette_buf, 0, 768);
  LbPaletteSet(palette_buf);

  wait_for_cd_to_be_available();
  short dproceed=true;
  if (dproceed)
  {
      buf=(unsigned char *)malloc(img_width*img_height);
      if (buf==NULL)
      {
          error(func_name, 1056, "Cannot allocate bitmap buffer");
          dproceed = false;
      }
  }
  if (dproceed)
  {
      fname=prepare_file_path(FGrp_FxData,"loading_640.raw");
      dproceed = (LbFileLoadAt(fname,buf) != -1);
  }
  if (dproceed)
  {
      wait_for_cd_to_be_available();
      fname=prepare_file_path(FGrp_FxData,"loading_640.pal");
      if ( LbFileLoadAt(fname, palette_buf) != 768 )
      {
        error(func_name, 1056, "Unable to load LOADING palette");
        memcpy(palette_buf, _DK_palette, 768);
      }
      LbScreenClear(0);
      dproceed=copy_raw8_image_to_screen_center(buf,img_width,img_height);
  }
  free(buf);
  return dproceed;
}

short display_loading_screen(void)
{
  struct TbScreenModeInfo *mdinfo = LbScreenGetModeInfo(lbDisplay.ScreenMode);
  short done=false;
  if (mdinfo->Width >= 640)
  {
      done=display_hires_loading_screen();
  }
  if (!done)
  {
      done=display_lores_loading_screen();
  }
  LbPaletteStopOpenFade();
  ProperForcedFadePalette(palette_buf, 64, Lb_PALETTE_FADE_CLOSED);
  return done;
}

/*
 * Plays a smacker animation file and sets frontend state to nstate.
 * @param nstate Frontend state; -1 means no change, -2 means don't even
 *    change screen mode.
 * @return Returns false if fatal error occured and probram execution should end.
 */
short play_smacker_file(char *filename, int nstate)
{
  static const char *func_name="play_smacker_file";
  unsigned int movie_flags = 0;
  if ( SoundDisabled )
    movie_flags |= 0x01;
  short result;

  result = 1;
  if ((result)&&(nstate>-2))
  {
    if ( setup_screen_mode_minimal(get_movies_vidmode()) )
    {
      LbMouseChangeSprite(0);
      LbScreenClear(0);
      LbScreenSwap();
    } else
    {
      error(func_name, 2356, "Can't enter movies video mode to play a Smacker file");
      result=0;
    }
  }
  if (result)
  {
    // Fail in playing shouldn't set result=0, because result=0 means fatal error.
    if (play_smk_(filename, 0, movie_flags | 0x100) == 0)
    {
      error(func_name, 2357, "Smacker play error");
      result=0;
    }
  }
  if (nstate>-2)
  {
    if ( !setup_screen_mode_minimal(get_frontend_vidmode()) )
    {
      error(func_name, 2358, "Can't re-enter frontend video mode after playing Smacker file");
      _DK_FatalError = 1;
      exit_keeper = 1;
      return 0;
    }
  } else
  {
    memset(_DK_frontend_palette, 0, PALETTE_SIZE);
  }
  LbScreenClear(0);
  LbScreenSwap();
  LbPaletteSet(_DK_frontend_palette);
  if ( nstate >= 0 )
    frontend_set_state(nstate);
  lbDisplay.LeftButton = 0;
  lbDisplay.RightButton = 0;
  lbDisplay.MiddleButton = 0;
  if (nstate > -2)
    LbMouseSetPosition(lbDisplay.PhysicalScreenWidth/2, lbDisplay.PhysicalScreenHeight/2);
  return result;
}

short setup_network_service(int srvidx)
{
//  return _DK_setup_network_service(srvidx);
  struct SerialInitData *init_data;
  long maxplayrs;

  switch (srvidx)
  {
  case 0:
      maxplayrs = 2;
      init_data = &_DK_net_serial_data;
      game.flags_font |= 0x10;
      LbSyncLog("Initializing %d-players serial network.\n",maxplayrs);
      break;
  case 1:
      maxplayrs = 2;
      init_data = &_DK_net_modem_data;
      game.flags_font |= 0x10;
      LbSyncLog("Initializing %d-players modem network.\n",maxplayrs);
      break;
  case 2:
      maxplayrs = 4;
      init_data = NULL;
      game.flags_font &= 0xEFu;
      LbSyncLog("Initializing %d-players IPX network.\n",maxplayrs);
      break;
  default:
      maxplayrs = 4;
      init_data = NULL;
      game.flags_font &= 0xEFu;
      LbSyncLog("Initializing %d-players type %d network.\n",maxplayrs,srvidx);
      break;
  }
  memset(&net_player_info[0], 0, sizeof(struct TbNetworkPlayerInfo));
  if ( LbNetwork_Init(srvidx, _DK_net_guid, maxplayrs, &_DK_net_screen_packet, 0xCu, &net_player_info[0], init_data) )
  {
    if (srvidx != 0)
      _DK_process_network_error(-800);
    return 0;
  }
  net_service_index_selected = srvidx;
  if (game.flags_font & 0x10)
    ;//rndseed_nullsub();
  frontend_set_state(5);
  return 1;
}

short init_sound(void)
{
  //_DK_init_sound(); return 1;
  unsigned long i;
  if (SoundDisabled)
    return false;
  SetupAudioOptionDefaults(&game.field_150374);
  game.field_150382 = 3;
  game.field_150380 = 1622;
  game.field_150374 = sound_dir;
  game.field_15037C = sound_dir;
  game.field_150386 = 1;
  game.field_150385 = 1;
  i = get_best_sound_heap_size(mem_size);
  if (i < 1048576)
    game.max_sounds = 10;
  else
    game.max_sounds = 16;
  game.field_150387 = 0;
  game.field_150389 = 0;
  game.field_150388 = 1;
  game.field_15038A = 1;
  if ((game.flags_font & 0x40) == 0)
    game.field_15038A = 0;
  game.field_15038C = 1;
  game.field_15038D = 1;
  game.field_15038B = 0;
  InitAudio(&game.field_150374);
  LoadMusic(0);
  if (!GetSoundInstalled())
  {
    SoundDisabled = 1;
    return false;
  }
  S3DInit();
  S3DSetNumberOfSounds(game.max_sounds);
  S3DSetMaximumSoundDistance(5120);
  return true;
}

short setup_game_sound_heap(void)
{
  return _DK_setup_game_sound_heap();
}

long ceiling_set_info(long a1, long a2, long a3)
{
  return _DK_ceiling_set_info(a1, a2, a3);
}

short initial_setup(void)
{
  static const char *func_name="initial_setup";
  //return _DK_initial_setup();
  MinimalResolutionSetup = false;
  if (LbDataLoadAll(game_load_files))
  {
    error(func_name, 499, "Unable to load game_load_files");
    return false;
  }
  if ( !LoadMcgaData() )
  {
    error(func_name, 689, "Load Mcga failed");
    return false;
  }
  load_pointer_file(0);
  update_screen_mode_data(320, 200);
  light_initialise();
  ceiling_set_info(12, 4, 1);
  lbDisplay.DrawFlags |= 0x4000u;
  return true;
}

/*
 * Displays 'legal' screens, intro and initializes basic game data.
 * If true is returned, then all files needed for startup were loaded,
 * and there should be the loading screen visible.
 * @return Returns true on success, false on error which makes the
 *   gameplay impossible (usually files loading failure).
 * @note The current screen resolution at end of this function may vary.
 */
short setup_game(void)
{
  static const char *func_name="setup_game";
  // CPU status variable
  struct CPU_INFO cpu_info;
  char *fname;
  //return _DK_setup_game();
  // Do only a very basic setup
  _DK_get_cpu_info(&cpu_info);
  update_memory_constraits();

  // Configuration file
  if ( !load_configuration() )
  {
      error(func_name, 1912, "Configuration load error.");
      return 0;
  }

  LbIKeyboardOpen();

  if (LbDataLoadAll(legal_load_files) != 0)
  {
      error(func_name, 1911, "Error on allocation/loading of legal_load_files.");
      return 0;
  }

  short result=1;
  short need_clean;

  // This should be closed in int initial_setup(void)

  // View the legal screen

  memset(_DK_palette, 0, PALETTE_SIZE);
  if ( LbScreenSetup(get_frontend_vidmode(), LEGAL_WIDTH, LEGAL_HEIGHT, _DK_palette, 1, 0) != 1 )
  {
      error(func_name, 1912, "Frontend screen mode setup error.");
      return 0;
  }

  unsigned char *legal_raw;

  if ( result )
  {
      LbScreenClear(0);
      LbScreenSwap();
      legal_raw = LbMemoryAlloc(LEGAL_WIDTH*LEGAL_HEIGHT);
      if (legal_raw == NULL)
      {
          error(func_name, 1456, "Unable to open allocate screen buffer");
          result=0;
      }
  }

  if ( result )
  {
      int loaded_size;
      fname = prepare_file_path(FGrp_StdData,"legal.raw");
      loaded_size = LbFileLoadAt(fname, legal_raw);
      if ( loaded_size != LEGAL_WIDTH*LEGAL_HEIGHT )
      {
          error(func_name, 1457, "Unable to load legal image data");
          result=0;
      }
  }
  if ( result )
  {
      int loaded_size;
      fname = prepare_file_path(FGrp_StdData,"legal.pal");
      loaded_size = LbFileLoadAt(fname, _DK_palette);
      if (loaded_size != PALETTE_SIZE)
      {
          error(func_name, 1458, "Unable to load legal image palette");
          result=0;
      }
  }

  if ( result )
      show_rawimage_screen(legal_raw,_DK_palette,LEGAL_WIDTH,LEGAL_HEIGHT,3000);
  else
      LbSyncLog("%s - Legal image skipped\n", func_name);

  // Now do more setup

  // Enable features thar require more resources
  update_features(mem_size);
  // Moon phase calculation
  calculate_moon_phase(true,true);
  // Start the sound system
  if (!init_sound())
    LbWarnLog("Sound system disabled.\n");

  // View second legal screen
  result = (legal_raw!=NULL);

  if ( result )
  {
      int loaded_size;
      fname = prepare_file_path(FGrp_FxData,"startup_fx.raw");
      loaded_size = LbFileLoadAt(fname, legal_raw);
      if ( loaded_size != LEGAL_WIDTH*LEGAL_HEIGHT )
      {
          error(func_name, 1477, "Unable to open startup_fx Screen");
          result=0;
      }
  }
  if ( result )
  {
      int loaded_size;
      fname = prepare_file_path(FGrp_FxData,"startup_fx.pal");
      loaded_size=LbFileLoadAt(fname, _DK_palette);
      if ( loaded_size != PALETTE_SIZE )
      {
          error(func_name, 1484, "Unable to open startup_fx Palette");
          result=0;
      }
  }

  if ( result )
      show_rawimage_screen(legal_raw,_DK_palette,LEGAL_WIDTH,LEGAL_HEIGHT,4000);
  else
      LbSyncLog("%s - startup_fx image skipped\n", func_name);

  {
      LbMemoryFree(legal_raw);
      memset(_DK_palette, 0, PALETTE_SIZE);
      LbScreenClear(0);
      LbScreenSwap();
  }

  // View Bullfrog company logo animation when new moon
  if ( is_new_moon )
  if ( !game.no_intro )
  {
      fname = prepare_file_path(FGrp_FxData,"bullfrog.smk");
      result = play_smacker_file(fname, -2);
      if ( !result )
        error(func_name, 1483, "Unable to play new moon movie");
  }

  result = 1;
  // The 320x200 mode is required only for the intro;
  // loading and no CD screens can run in both 320x2?0 and 640x4?0.
  if ( result && (!game.no_intro) )
  {
    int mode_ok = LbScreenSetup(get_movies_vidmode(), 320, 200, _DK_palette, 2, 0);
    if (mode_ok != 1)
    {
      error(func_name, 1500, "Can't enter movies screen mode to play intro");
      result=0;
    }
  }

  if ( result )
  {
      LbScreenClear(0);
      LbScreenSwap();
      result=wait_for_cd_to_be_available();
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

  char *text_end;
  if ( result )
  {
      fname = prepare_file_path(FGrp_StdData,"text.dat");
      long filelen;
      filelen = LbFileLengthRnc(fname);
      strings_data = (char *)LbMemoryAlloc(filelen + 256);
      if ( strings_data == NULL )
      {
        error(func_name, 1509, "Can't allocate memory for Strings data");
        result = 0;
      }
      text_end = strings_data+filelen+255;
  }
  if ( result )
  {
      int loaded_size = LbFileLoadAt(fname, strings_data);
      if (loaded_size < 16)
      {
          error(func_name, 1501, "Strings file couldn't be loaded or is too small");
          result = 0;
      }
  }
  if ( result )
  {
      char **text_arr = strings;
      int text_idx;
      char *text_ptr;
      text_idx = STRINGS_MAX;
      text_ptr = strings_data;
      while (text_idx>=0)
      {
        if (text_ptr>=text_end)
        {
            result = 0;
            break;
        }
        *text_arr = text_ptr;
        text_arr++;
        char chr_prev;
        do {
            chr_prev = *text_ptr;
            text_ptr++;
        } while ((chr_prev!='\0')&&(text_ptr<text_end));
        text_idx--;
      }
  }

  if ( result )
  {
      _DK_IsRunningMark();
      if ( !initial_setup() )
      {
        //error(func_name, 1502, "Initial setup failed");
        result = 0;
      }
  }
  if ( result )
  {
      _DK_load_settings();
      if ( !setup_game_sound_heap() )
      {
        error(func_name, 1503, "Sound heap setup failed");
        result = 0;
      }
  }

  if ( result )
  {
        _DK_init_creature_scores();
        if ( cpu_info.field_0 )
        {
            if ( ((cpu_info.field_0>>8) & 0x0Fu) >= 0x06 )
              _DK_set_cpu_mode(1);
        }
        set_gamma(settings.gamma_correction, 0);
        SetRedbookVolume(settings.redbook_volume);
        SetSoundMasterVolume(settings.sound_volume);
        SetMusicMasterVolume(settings.sound_volume);
        _DK_setup_3d();
        _DK_setup_stuff();
        init_lookups();
        result = 1;
  }
  return result;
}

void output_message(long idx, long a, long b)
{
  _DK_output_message(idx, a, b);
}

void input_eastegg(void)
{
  static const char *func_name="input_eastegg";
  //_DK_input_eastegg();return;
  // Maintain the FECKOFF cheat
  if ( lbKeyOn[KC_LSHIFT] )
  {
    switch (game.eastegg01_cntr)
    {
    case 0:
      if ( lbKeyOn[KC_F] )
        game.eastegg01_cntr++;
      break;
    case 1:
      if ( lbKeyOn[KC_E] )
        game.eastegg01_cntr++;
      break;
    case 2:
      if ( lbKeyOn[KC_C] )
        game.eastegg01_cntr++;
      break;
    case 3:
      if ( lbKeyOn[KC_K] )
        game.eastegg01_cntr++;
      break;
    case 4:
      if ( lbKeyOn[KC_O] )
        game.eastegg01_cntr++;
      break;
    case 5:
    case 6:
      if ( lbKeyOn[KC_F] )
        game.eastegg01_cntr++;
      break;
    }
  } else
  {
    game.eastegg01_cntr = 0;
  }
  // Maintain the JLW cheat
  if (game.flags_font & 0x20)
  {
    if ( lbKeyOn[KC_LSHIFT] && lbKeyOn[KC_RSHIFT] )
    {
      switch (game.eastegg02_cntr)
      {
      case 0:
        if ( lbKeyOn[KC_J] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      case 1:
        if ( lbKeyOn[KC_L] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      case 2:
        if ( lbKeyOn[KC_W] )
        {
          play_non_3d_sample(159);
          game.eastegg02_cntr++;
        }
        break;
      }
    } else
    {
      game.eastegg02_cntr = 0;
    }
  }
  // Maintain the SKEKSIS cheat
  if ( lbKeyOn[KC_LSHIFT] )
  {
    switch (eastegg03_cntr)
    {
    case 0:
    case 4:
      if ( lbInkey==KC_S )
      {
       eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 1:
    case 3:
      if ( lbInkey==KC_K )
      {
       eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 2:
      if ( lbInkey==KC_E )
      {
       eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 5:
      if ( lbInkey==KC_I )
      {
       eastegg03_cntr++;
       lbInkey=0;
      }
      break;
    case 6:
      if ( lbInkey==KC_S )
      {
        eastegg03_cntr++;
        lbInkey=0;
        //'Your pants are definitely too tight'
        output_message(94, 0, 1);
      }
      break;
    }
  } else
  {
    eastegg03_cntr = 0;
  }
  if (lbInkey!=0)
  {
    eastegg03_cntr = 0;
  }
}

void init_messages(void)
{
  _DK_init_messages();
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

void battle_initialise(void)
{
  _DK_battle_initialise();
}

void event_initialise_all(void)
{
  _DK_event_initialise_all();
}

void add_thing_to_list(struct Thing *thing, struct StructureList *list)
{
  _DK_add_thing_to_list(thing, list);
}

struct Thing *allocate_free_thing_structure(unsigned char a1)
{
  return _DK_allocate_free_thing_structure(a1);
}

unsigned char i_can_allocate_free_thing_structure(unsigned char a1)
{
  return _DK_i_can_allocate_free_thing_structure(a1);
}

struct Thing *create_ambient_sound(struct Coord3d *pos, unsigned short model, unsigned short owner)
{
  static const char *func_name="create_ambient_sound";
  struct Thing *thing;
  if ( !i_can_allocate_free_thing_structure(1) )
  {
    error(func_name, 3698, "'Told You So' error! Cannot create ambient sound because there are too many fucking things allocated.");
    return NULL;
  }
  thing = allocate_free_thing_structure(1);
  thing->class_id = 12;
  thing->model = model;
  thing->field_1D = thing->field_1B;
  memcpy(&thing->mappos,pos,sizeof(struct Coord3d));
  thing->owner = owner;
  thing->field_4F |= 0x01;
  add_thing_to_list(thing, &game.thing_lists[9]);
  return thing;
}

/*
 * Checks if the given screen point is over a gui menu.
 * @param x,y Screen coordinates to check.
 * @return Returns index of the menu, or -1 if there's no menu on this point.
 */
int point_is_over_gui_menu(long x, long y)
{
  int idx;
  int gidx = -1;
  for(idx=0;idx<ACTIVE_MENUS_COUNT;idx++)
  {
    struct GuiMenu *gmnu;
    gmnu=&active_menus[idx];
    if (gmnu->field_1 != 2)
        continue;
    if (gmnu->flgfield_1D == 0)
        continue;
    short gx = gmnu->pos_x;
    if ((x >= gx) && (x < gx+gmnu->width))
    {
        short gy = gmnu->pos_y;
        if ((y >= gy) && (y < gy+gmnu->height))
            gidx=idx;
    }
  }
  return gidx;
}

int first_monopoly_menu(void)
{
  int idx;
  const short gmax=8;//sizeof(active_menus)/sizeof(struct GuiMenu);
  struct GuiMenu *gmnu;
  for (idx=0;idx<gmax;idx++)
  {
    gmnu=&active_menus[idx];
    if ((gmnu->field_1!=0) && (gmnu->flgfield_1E!=0))
        return idx;
  }
  return -1;
}

inline void reset_scrolling_tooltip(void)
{
    tooltip_scroll_offset = 0;
    tooltip_scroll_timer = 25;
}

short setup_trap_tooltips(struct Coord3d *pos)
{
    struct Thing *thing;
    struct PlayerInfo *player;
    int stridx;
    thing = get_trap_for_position(map_to_slab[pos->x.stl.num],map_to_slab[pos->y.stl.num]);;
    if (thing == NULL) return false;
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    if ((thing->byte_17.h == 0) && (player->field_2B != thing->owner))
      return false;
    if (thing != tool_tip_box.target)
    {
      help_tip_time = 0;
      tool_tip_box.target = thing;
    }
    if ((help_tip_time > 20) || (player->field_453 == 12))
    {
      tool_tip_box.field_0 = 1;
      stridx = trap_data[thing->model].field_C;
      sprintf(tool_tip_box.text, "%s", strings[stridx%STRINGS_MAX]);
      tool_tip_box.pos_x = GetMouseX();
      tool_tip_box.pos_y = GetMouseY()+86;
      tool_tip_box.field_809 = 4;
    } else
    {
      help_tip_time++;
    }
    return true;
}

short setup_object_tooltips(struct Coord3d *pos)
{
  char *text;
  struct Thing *thing = NULL;
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  thing = game.things_lookup[player->field_35%THINGS_COUNT];
  if (thing == game.things_lookup[0])
      thing = NULL;
  if ((thing!=NULL)&&(!thing_is_special(thing)))
      thing = NULL;
  if (thing==NULL)
    thing = _DK_get_special_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    if ((void *)thing != tool_tip_box.target)
    {
      help_tip_time = 0;
      tool_tip_box.target = thing;
    }
    int stridx = specials_text[object_to_special[thing->model]];
    sprintf(tool_tip_box.text, "%s", strings[stridx%STRINGS_MAX]);
    tool_tip_box.pos_x = GetMouseX();
    tool_tip_box.field_0 = 1;
    tool_tip_box.field_809 = 5;
    tool_tip_box.pos_y = GetMouseY() + 86;
    return true;
  }
  thing = _DK_get_spellbook_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    if ( (void *)thing != tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      tool_tip_box.target = (void *)thing;
    }
    int stridx = _DK_spell_data[object_to_magic[thing->model]].field_D;
    if (stridx>0)
    {
      sprintf(tool_tip_box.text,"%s",strings[stridx%STRINGS_MAX]);
      tool_tip_box.field_0 = 1;
      tool_tip_box.pos_x = GetMouseX();
      tool_tip_box.field_809 = 5;
      tool_tip_box.pos_y = GetMouseY() + 86;
    }
    return 1;
  }
  thing = _DK_get_crate_at_position(pos->x.stl.num, pos->y.stl.num);
  if ( thing )
  {
    if ( (void *)thing != tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      tool_tip_box.target = (void *)thing;
    }
    tool_tip_box.field_0 = 1;
    int objidx = thing->model;
    int stridx;
    if ( _DK_workshop_object_class[objidx] == 8 )
      stridx = _DK_trap_data[_DK_object_to_door_or_trap[objidx]].field_C;
    else
      stridx = _DK_door_names[_DK_object_to_door_or_trap[objidx]];
    sprintf(tool_tip_box.text, "%s", strings[stridx%STRINGS_MAX]);
    tool_tip_box.pos_x = GetMouseX();
    tool_tip_box.pos_y = GetMouseY() + 86;
    tool_tip_box.field_809 = 5;
    return true;
  }
  if (!settings.tooltips_on)
    return false;
  thing = _DK_get_nearest_object_at_position(pos->x.stl.num, pos->y.stl.num);
  if (thing!=NULL)
  {
    int objidx = thing->model;
    int crtridx;
    if ( objidx == 49 )
    {
      text=buf_sprintf("%s", strings[545]);
    } else
    if (crtridx = _DK_objects[objidx].field_13)
    {
      int stridx=creature_data[crtridx].field_3;
      text=buf_sprintf("%s %s", strings[stridx%STRINGS_MAX], strings[609]);
    } else
    {
      return 0;
    }

    if ( (void *)thing != tool_tip_box.target )
    {
      _DK_help_tip_time = 0;
      tool_tip_box.target = (void *)thing;
    }
    if ( (_DK_help_tip_time>20) || (player->field_453==12))
    {
      tool_tip_box.field_0 = 1;
      strcpy(tool_tip_box.text, text);
      tool_tip_box.pos_x = GetMouseX();
      tool_tip_box.field_809 = 5;
      tool_tip_box.pos_y = GetMouseY() + 86;
    } else
    {
      _DK_help_tip_time++;
    }
    return true;
  }
  return false;
}

short setup_land_tooltips(struct Coord3d *pos)
{
  if (!settings.tooltips_on)
    return false;
  int slab_idx = map_to_slab[pos->x.stl.num] + map_tiles_x*map_to_slab[pos->y.stl.num];
  int attridx = game.slabmap[slab_idx].slab;
  int stridx = slab_attrs[attridx].field_0;
  if (stridx==201)
    return false;
  if ((void *)attridx != tool_tip_box.target)
  {
    _DK_help_tip_time = 0;
    tool_tip_box.target = (void *)attridx;
  }
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if ((help_tip_time > 20) || (player->field_453==12))
  {
    tool_tip_box.field_0 = 1;
    sprintf(tool_tip_box.text, "%s", strings[stridx%STRINGS_MAX]);
    tool_tip_box.pos_x = GetMouseX();
    tool_tip_box.field_809 = 2;
    tool_tip_box.pos_y = GetMouseY() + 86;
  } else
  {
    _DK_help_tip_time++;
  }
  return true;
}

short setup_room_tooltips(struct Coord3d *pos)
{
  if (!settings.tooltips_on)
    return false;
  int slab_idx = map_to_slab[pos->x.stl.num] + map_tiles_x*map_to_slab[pos->y.stl.num];
  struct Room *room;
  room = &game.rooms[game.slabmap[slab_idx].field_3];
  if (room==NULL)
    return false;
  int stridx=_DK_room_data[room->field_A].field_13;
  if ( stridx == 201 )
    return false;
  if ( room != tool_tip_box.target )
  {
    _DK_help_tip_time = 0;
    tool_tip_box.target = room;
  }
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  int widener=0;
  if ( (_DK_help_tip_time>20) || (player->field_453==12) )
  {
      if ( room->field_A >= 2 )
      {
        if ( (room->field_A<=14) || (room->field_A==16) )
          widener = 0;
      }
      sprintf(tool_tip_box.text, "%s", strings[stridx%STRINGS_MAX]);
      tool_tip_box.field_0 = 1;
      tool_tip_box.pos_x = GetMouseX();
      tool_tip_box.pos_y = GetMouseY() + 86 + 20*widener;
      tool_tip_box.field_809 = 1;
  } else
  {
    _DK_help_tip_time++;
  }
  return true;
}

void reveal_whole_map(struct PlayerInfo *player)
{
  unsigned short nflag;
  unsigned int x,y,i;
  unsigned long imax;

  nflag = (1 << player->field_2B);
  for (y=0; y<map_tiles_y; y++)
    for (x=0; x<map_tiles_x; x++)
    {
      clear_slab_dig(x, y, player->field_2B);
    }
  imax = (map_subtiles_x+1)*(map_subtiles_y+1);
  for (i=0; i<imax; i++)
  {
    x = (game.map[i].field_1 >> 28) | nflag;
    game.map[i].field_1 |= (x & 0x0F) << 28;
  }
  pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
}

void increase_level(struct PlayerInfo *player)
{
  static const char *func_name="increase_level";
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct CreatureControl *cctrl;
  int i;
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);

  i = dungeon->field_2D;
  while (i > 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4576,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    creature_increase_level(thing);
    if (thing->field_64 >= CREATURES_COUNT)
      break;
    cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
    if (cctrl < game.creature_control_data)
      break;
    i = cctrl->thing_idx;
  }

  i = dungeon->field_2F;
  while (i > 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4575,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    creature_increase_level(thing);
    if (thing->field_64 >= CREATURES_COUNT)
      break;
    cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
    if (cctrl < game.creature_control_data)
      break;
    i = cctrl->thing_idx;
  }
}

void start_resurrect_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct Dungeon *dungeon;
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  if (dungeon->field_139F != 0)
  {
    if (player == &game.players[my_player_number])
    {
      dungeon_special_selected = thing->field_1B;
      resurrect_creature_scroll_offset = 0;
      turn_off_menu(27);
      turn_on_menu(28);
    }
  }
}

void start_transfer_creature(struct PlayerInfo *player, struct Thing *thing)
{
  struct Dungeon *dungeon;
  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  if (dungeon->field_919 != 0)
  {
    if (player == &game.players[my_player_number])
    {
      dungeon_special_selected = thing->field_1B;
      transfer_creature_scroll_offset = 0;
      turn_off_menu(27);
      turn_on_menu(29);
    }
  }
}

short engine_point_to_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  *map_x = 0;
  *map_y = 0;
  if ( (pointer_x >= 0) && (pointer_y >= 0)
    && (pointer_x < (player->engine_window_width/pixel_size))
    && (pointer_y < (player->engine_window_height/pixel_size)) )
  {
    int i;
    i = player->field_453;
    if ( ((i == 1) && (player->field_454)) || (i == 2) ||
      (i == 18) || (i == 16) || (i == 8) || (i == 23) )
    {
          *map_x = ( top_pointed_at_x << 8) + top_pointed_at_frac_x;
          *map_y = ( top_pointed_at_y << 8) + top_pointed_at_frac_y;
    } else
    {
          *map_x = (block_pointed_at_x << 8) + pointed_at_frac_x;
          *map_y = (block_pointed_at_y << 8) + pointed_at_frac_y;
    }
    // Clipping coordinates
    if ( *map_y < 0 )
      *map_y = 0;
    else
    if ( *map_y > 0xFEFFu )
      *map_y = 0xFEFFu;
    if ( *map_x < 0 )
      *map_x = 0;
    else
    if ( *map_x > 0xFEFFu )
      *map_x = 0xFEFFu;
    return true;
  }
  return false;
}

void restore_computer_player_after_load(void)
{
  _DK_restore_computer_player_after_load();
}

short point_to_overhead_map(struct Camera *camera, long screen_x, long screen_y, long *map_x, long *map_y)
{
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
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

short screen_to_map(struct Camera *camera, long screen_x, long screen_y, struct Coord3d *mappos)
{
//return _DK_screen_to_map(camera,screen_x,screen_y,mappos);
  short result;
  long x,y;
  switch ( camera->field_6 )
  {
    case 1:
    case 2:
    case 5:
      // 3D view mode
      result=engine_point_to_map(camera,screen_x,screen_y,&x,&y);
      break;
    case 3: //map mode
      result=point_to_overhead_map(camera,screen_x,screen_y,&x,&y);
      break;
    default:
      result = false;
      break;
  }
  if ( result )
  {
    mappos->x.val = x;
    mappos->y.val = y;
  }
  if ( mappos->x.val > 0xFEFFu )
    mappos->x.val = 0xFEFFu;
  if ( mappos->y.val > 0xFEFFu )
    mappos->y.val = 0xFEFFu;
  return result;
}

short wait_for_cd_to_be_available(void)
{
    static const char *func_name="wait_for_cd_to_be_available";
//  _DK_wait_for_cd_to_be_available(); return;
  char *fname;
  const int img_width = 320;
  const int img_height = 200;
  short was_locked = LbScreenIsLocked();
  fname=prepare_file_path(FGrp_LoData,"dkwind00.dat");
  if ( LbFileExists(fname) )
    return true;
  if ( was_locked )
    LbScreenUnlock();
  if (LbDataLoadAll(nocd_load_files))
  {
      error(func_name, 78, "Unable to test for CD in drive");
      return false;
  }
  LbSyncLog("CD not found in drive, waiting\n");
  LbScreenClear(0);
  LbPaletteSet(nocd_pal);
  unsigned long counter;
  unsigned int i;
  counter=0;
  while ( !exit_keeper )
  {
      if ( LbFileExists(fname) )
        break;
      for (i=0;i<10;i++)
      {
        copy_raw8_image_to_screen_center(nocd_raw,img_width,img_height);
        while ( (!LbIsActive()) && (!exit_keeper) && (!quit_game) )
        {
          if (!LbWindowsControl())
          {
            exit_keeper = 1;
            break;
          }
        }
        if (is_key_pressed(KC_Q,KM_DONTCARE) || is_key_pressed(KC_X,KM_ALT))
        {
          error(func_name, 77, "User requested quit, giving up");
          clear_key_pressed(KC_Q);
          clear_key_pressed(KC_X);
          exit_keeper = 1;
          break;
        }
        LbSleepFor(100);
      }
      // One 'counter' cycle lasts approx. 1 second.
      counter++;
      if (counter>300)
      {
          error(func_name, 79, "Wait time too long, giving up");
          exit_keeper = 1;
      }
  }
  LbSyncLog("Finished waiting for CD after %lu seconds\n",counter);
  LbDataFreeAll(nocd_load_files);
  if ( was_locked )
    LbScreenLock();
  return (!exit_keeper);
}

short input_gameplay_tooltips(short gameplay_on)
{
  struct Coord3d mappos;
  unsigned short bitval;
  struct PlayerInfo *player;
  short shown;
  shown = false;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if ((gameplay_on) && (tool_tip_time==0) && (!busy_doing_gui))
  {
        if ( screen_to_map(player->camera,GetMouseX(),GetMouseY(),&mappos) )
        {
          int bblock_x=mappos.x.stl.num;
          int bblock_y=mappos.y.stl.num;
          // Get the top four bits - player flags
          bitval = (game.map[bblock_x+bblock_y*(map_subtiles_x+1)].field_1) >> 28;
          if ((1 << player->field_2B) & (bitval))
          {
            if (player->field_37 != 1)
            {
              if (!shown)
                shown=setup_trap_tooltips(&mappos);
              if (!shown)
                shown=setup_object_tooltips(&mappos);
              if (!shown)
                shown=setup_land_tooltips(&mappos);
              if (!shown)
                shown=setup_room_tooltips(&mappos);
              if (!shown)
              {
                help_tip_time = 0;
                tool_tip_box.target = NULL;
              }
            }
          }
        }
  }
  if (tool_tip_box.field_0 == 0)
    reset_scrolling_tooltip();
  return shown;
}
short get_gui_inputs(short gameplay_on)
{
  static const char *func_name="get_gui_inputs";
  static short doing_tooltip;
  static char over_slider_button=-1;
#if (BFDEBUG_LEVEL > 7)
  LbSyncLog("%s: Starting\n", func_name);
#endif
  //return _DK_get_gui_inputs(gameplay_on);
  doing_tooltip = 0;
  _DK_update_breed_activities();
  battle_creature_over = 0;
  gui_room_type_highlighted = -1;
  gui_door_type_highlighted = -1;
  gui_trap_type_highlighted = -1;
  gui_creature_type_highlighted = -1;
  if ( gameplay_on )
    _DK_maintain_my_battle_list();

  if ( !lbDisplay.MLeftButton )
  {
    drag_menu_x = -999;
    drag_menu_y = -999;
    int idx;
    for (idx=0;idx<ACTIVE_BUTTONS_COUNT;idx++)
    {
      struct GuiButton *gbtn = &active_buttons[idx];
      if ((gbtn->field_0 & 0x01) && (gbtn->gbtype == 6))
          gbtn->field_1 = 0;
    }
  }

  int gidx;
  gidx = point_is_over_gui_menu(GetMouseX(), GetMouseY());
  if ( gidx == -1 )
    _DK_busy_doing_gui = 0;
  else
    _DK_busy_doing_gui = 1;
  int fmmenu_idx = first_monopoly_menu();

  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  int gmbtn_idx = -1;
  struct GuiButton *gbtn;
  // Sweep through buttons
  for (gidx=0; gidx<ACTIVE_BUTTONS_COUNT; gidx++)
  {
    gbtn = &active_buttons[gidx];
    if ((gbtn->field_0 & 1)==0)
      continue;
    if (!active_menus[gbtn->gmenu_idx].flgfield_1D)
      continue;
    Gf_Btn_Callback callback;
    callback = gbtn->field_17;
    if ( callback != NULL )
      callback(gbtn);
    if ( ((gbtn->field_1B & 0x4000u)!=0) || mouse_is_over_small_map(player->mouse_x,player->mouse_y) )
      continue;

    if ( check_if_mouse_is_over_button(gbtn) && (!game_is_busy_doing_gui())
      || (gbtn->gbtype==6) && (gbtn->field_1!=0) )
    {
      if ( (fmmenu_idx==-1) || (gbtn->gmenu_idx==fmmenu_idx) )
      {
        gmbtn_idx = gidx;
        gbtn->field_0 |= 0x10;
        busy_doing_gui = 1;
        callback = gbtn->field_F;
        if ( callback != NULL )
          callback(gbtn);
        if (gbtn->gbtype == 6)
          break;
        if (gbtn->gbtype != Lb_SLIDER)
          over_slider_button = -1;
      } else
      {
        gbtn->field_0 &= 0xEFu;
      }
    } else
    {
      gbtn->field_0 &= 0xEFu;
    }
    if ( gbtn->gbtype == Lb_SLIDER )
    {
      int mouse_x;
      int mouse_y;
      int btnsize;
      mouse_x = GetMouseX();
      btnsize = gbtn->scr_pos_x + ((gbtn->slide_val)*(gbtn->width-64) >> 8);
      if ((mouse_x>(btnsize+22)) && (mouse_x<=(btnsize+44)))
      {
        mouse_y = GetMouseY();
        if ((mouse_y>gbtn->pos_y) && (mouse_y<=(gbtn->pos_y+gbtn->height)))
        {
          if ( left_button_clicked )
          {
            left_button_clicked = 0;
            over_slider_button = gidx;
            do_sound_menu_click();
          }
        }
      }
    }
  }  // end for

  short result = 0;
  if (game_is_busy_doing_gui())
  {
    busy_doing_gui = 1;
    if (_DK_get_button_area_input(input_button,input_button->id_num) != 0)
        result = 1;
  }
  if ((over_slider_button!=-1) && (left_button_released))
  {
      left_button_released = 0;
      if (gmbtn_idx!=-1)
        active_buttons[gmbtn_idx].field_1 = 0;
      over_slider_button = -1;
      do_sound_menu_click();
  }

  if (gmbtn_idx!=-1)
  {
    gbtn = &active_buttons[gmbtn_idx];
    if ((active_menus[gbtn->gmenu_idx].field_1 == 2) && ((gbtn->field_1B & 0x8000u)==0))
    {
      if (tool_tip_box.gbutton == gbtn)
      {
        if ((tool_tip_time > 10) || (player->field_453 == 12))
        {
          busy_doing_gui = 1;
          if ( gbtn->field_13 != gui_area_text )
            _DK_setup_gui_tooltip(gbtn);
        } else
        {
          tool_tip_time++;
          doing_tooltip = 1;
          busy_doing_gui = 1;
        }
      } else
      {
        tool_tip_time = 0;
        tool_tip_box.gbutton = gbtn;
        tool_tip_box.pos_x = GetMouseX();
        tool_tip_box.pos_y = GetMouseY()+86;
        tool_tip_box.field_809 = 0;
      }
    } else
    {
      tool_tip_time = 0;
      tool_tip_box.gbutton = NULL;
    }
  } else
  {
    tool_tip_time = 0;
    tool_tip_box.gbutton = NULL;
  }

  if ( over_slider_button != -1 )
  {
    gbtn = &active_buttons[over_slider_button];
    int mouse_x = GetMouseX();
    gbtn->field_1 = 1;
    int slide_start,slide_end;
    slide_start = gbtn->pos_x+32;
    slide_end = gbtn->pos_x+gbtn->width-32;
    if (mouse_x > slide_start)
    {
      if (mouse_x < slide_end)
      {
        if ( gbtn->width>64 )
            gbtn->slide_val = ((mouse_x-slide_start) << 8) / (gbtn->width-64);
        else
            gbtn->slide_val = ((mouse_x-gbtn->pos_x) << 8) / (gbtn->width+1);
      } else
      {
        gbtn->slide_val = 255;
      }
    } else
    {
      gbtn->slide_val = 0;
    }
    *gbtn->field_33 = (gbtn->slide_val) * (gbtn->field_2D+1) >> 8;
    Gf_Btn_Callback callback;
    callback = gbtn->click_event;
    if ( callback != NULL )
      callback(gbtn);
    return 1;
  }

  if ( gmbtn_idx != -1 )
  {
    gbtn = &active_buttons[gmbtn_idx];
    Gf_Btn_Callback callback;
    if ( lbDisplay.MLeftButton )
    {
      result = 1;
      callback = gbtn->click_event;
      if ((callback!=NULL) || (((gbtn->field_0 & 2)!=0) || (gbtn->field_2F!=0) || (gbtn->gbtype==Lb_RADIOBTN)))
        if ((gbtn->field_0 & 8)!=0)
        {
          switch (gbtn->gbtype)
          {
          case 1:
            if ( (gbtn->field_1>5) && (callback!=NULL) )
              callback(gbtn);
            else
              gbtn->field_1++;
            break;
          case 6:
            if (callback!=NULL)
              callback(gbtn);
            break;
          }
        }
    } else
    if ( lbDisplay.MRightButton )
    {
      result = 1;
      callback = gbtn->rclick_event;
      if ((callback != NULL) && ((gbtn->field_0 & 8)!=0))
      {
        switch (gbtn->gbtype)
        {
        case 1:
          if ( (gbtn->field_2>5) && (callback!=NULL) )
            callback(gbtn);
          else
            gbtn->field_2++;
          break;
        case 6:
          if (callback!=NULL)
            callback(gbtn);
          break;
        }
      }
    }
    if ( left_button_clicked )
    {
      result = 1;
      if (game.field_1516F3 != 0)
      {
        if (gbtn->id_num == game.field_1516F3)
          game.field_1516F3 = 0;
      }
      callback = gbtn->click_event;
      if ((callback!=NULL) || (gbtn->field_0 & 2) || (gbtn->field_2F) || (gbtn->gbtype==Lb_RADIOBTN) )
      {
        left_button_clicked = 0;
        gui_last_left_button_pressed_id = gbtn->id_num;
        do_button_click_actions(gbtn, &gbtn->field_1, callback);
      }
    } else
    if ( right_button_clicked )
    {
      result = 1;
      if (game.field_1516F3 != 0)
      {
        if (gbtn->id_num == game.field_1516F3)
          game.field_1516F3 = 0;
      }
      callback = gbtn->rclick_event;
      if ((callback!=NULL))
      {
        right_button_clicked = 0;
        gui_last_right_button_pressed_id = gbtn->id_num;
        do_button_click_actions(gbtn, &gbtn->field_2, callback);
      }
    }
  }

  for (gidx=0;gidx<ACTIVE_BUTTONS_COUNT;gidx++)
  {
    gbtn = &active_buttons[gidx];
    if (gbtn->field_0 & 1)
      if ( ((gmbtn_idx==-1) || (gmbtn_idx!=gidx)) && (gbtn->gbtype!=Lb_RADIOBTN) && (gbtn!=input_button) )
      {
        gbtn->field_0 &= 0xEFu;
        gbtn->field_1 = 0;
        gbtn->field_2 = 0;
      }
  }
  if ( gmbtn_idx != -1 )
  {
    Gf_Btn_Callback callback;
    gbtn = &active_buttons[gmbtn_idx%ACTIVE_BUTTONS_COUNT];
    if ((gbtn->field_1) && (left_button_released))
    {
      callback = gbtn->click_event;
      result = 1;
      if ((callback!=NULL) || (gbtn->field_0 & 0x02) || (gbtn->field_2F!=0) || (gbtn->gbtype==Lb_RADIOBTN))
      {
        left_button_released = 0;
        do_button_release_actions(gbtn, &gbtn->field_1, callback);
      }
    } else
    if ((gbtn->field_2) && (right_button_released))
    {
      callback = gbtn->rclick_event;
      result = 1;
      if ( callback!=NULL )
      {
        right_button_released = 0;
        do_button_release_actions(gbtn, &gbtn->field_2, callback);
      }
    }
  }
  input_gameplay_tooltips(gameplay_on);
  return result;
}

short save_settings(void)
{
  static const char *func_name="save_settings";
  char *fname;
  //_DK_save_settings();
  fname=prepare_file_path(FGrp_Save,"settings.dat");
  LbFileSaveAt(fname, &settings, sizeof(struct GameSettings));
  return true;
}

void toggle_tooltips(void)
{
  const char *statstr;
  settings.tooltips_on = !settings.tooltips_on;
  if (settings.tooltips_on)
  {
    do_sound_menu_click();
    statstr = "on";
  } else
  {
    statstr = "off";
  }
  show_onscreen_msg(2*game.num_fps, "Tooltips %s", statstr);
  save_settings();
}

void zoom_to_map(void)
{
  struct PlayerInfo *player;
  struct Packet *pckt;
  turn_off_all_window_menus();
  if ((game.numfield_C & 0x20) == 0)
    game.numfield_C ^= game.numfield_C & 0x40;
  else
    game.numfield_C |= 0x40;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  pckt = &game.packets[player->packet_num%PACKETS_COUNT];
  if ((game.numfield_A & 0x01) || (lbDisplay.PhysicalScreenWidth > 320))
  {
    if (!toggle_status_menu(false))
      game.numfield_C ^= game.numfield_C & 0x40;
    set_packet_action(pckt, 119, 4, 0, 0, 0);
    turn_off_roaming_menus();
  } else
  {
    set_packet_action(pckt, 80, 5, 0, 0, 0);
    turn_off_roaming_menus();
  }
}

void zoom_from_map(void)
{
  struct PlayerInfo *player;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (((game.numfield_A & 0x01) != 0) || (lbDisplay.PhysicalScreenWidth > 320))
  {
      if ((game.numfield_C & 0x40) != 0)
        toggle_status_menu(true);
      struct Packet *pckt = &game.packets[player->packet_num%PACKETS_COUNT];
      set_packet_action(pckt, 120,1,0,0,0);
  } else
  {
      struct Packet *pckt = &game.packets[player->packet_num%PACKETS_COUNT];
      set_packet_action(pckt, 80,6,0,0,0);
  }
}

void setup_engine_window(long x, long y, long width, long height)
{
  static const char *func_name="setup_engine_window";
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Setting coords %d,%d,%d,%d\n",func_name,x,y,width,height);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (game.numfield_C & 0x20)
  {
    if (x > MyScreenWidth)
      x = MyScreenWidth;
    if (x < status_panel_width)
      x = status_panel_width;
  } else
  {
    if (x > MyScreenWidth)
      x = MyScreenWidth;
    if (x < 0)
      x = 0;
  }
  if (y > MyScreenHeight)
    y = MyScreenHeight;
  if (y < 0)
    y = 0;
  if (x+width > MyScreenWidth)
    width = MyScreenWidth-x;
  if (width < 0)
    width = 0;
  if (y+height > MyScreenHeight)
    height = MyScreenHeight-y;
  if (height < 0)
    height = 0;
  player->engine_window_x = x;
  player->engine_window_y = y;
  player->engine_window_width = width;
  player->engine_window_height = height;
}

void store_engine_window(struct TbGraphicsWindow *ewnd,int divider)
{
  struct PlayerInfo *player;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (divider <= 1)
  {
    ewnd->x = player->engine_window_x;
    ewnd->y = player->engine_window_y;
    ewnd->width = player->engine_window_width;
    ewnd->height = player->engine_window_height;
  } else
  {
    ewnd->x = player->engine_window_x/divider;
    ewnd->y = player->engine_window_y/divider;
    ewnd->width = player->engine_window_width/divider;
    ewnd->height = player->engine_window_height/divider;
  }

}

void load_engine_window(struct TbGraphicsWindow *ewnd)
{
  struct PlayerInfo *player;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->engine_window_x = ewnd->x;
  player->engine_window_y = ewnd->y;
  player->engine_window_width = ewnd->width;
  player->engine_window_height = ewnd->height;
}

struct Thing *get_nearest_thing_for_hand_or_slap(unsigned char a1, long a2, long a3)
{
  return _DK_get_nearest_thing_for_hand_or_slap(a1, a2, a3);
}

long creature_instance_is_available(struct Thing *thing, long inum)
{
  struct CreatureControl *crctrl;
  crctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
  return crctrl->instances[inum];
}

struct Thing *create_creature(struct Coord3d *pos, unsigned short a1, unsigned short a2)
{
  return _DK_create_creature(pos, a1, a2);
}

void creature_increase_level(struct Thing *thing)
{
  _DK_creature_increase_level(thing);
}

void set_creature_level(struct Thing *thing, long nlvl)
{
  _DK_set_creature_level(thing, nlvl);
}

void remove_events_thing_is_attached_to(struct Thing *thing)
{
  _DK_remove_events_thing_is_attached_to(thing);
}

unsigned long steal_hero(struct PlayerInfo *player, struct Coord3d *pos)
{
  return _DK_steal_hero(player, pos);
}

void make_safe(struct PlayerInfo *player)
{
  _DK_make_safe(player);
}

void reset_gui_based_on_player_mode(void)
{
  _DK_reset_gui_based_on_player_mode();
}

void sound_reinit_after_load(void)
{
  _DK_sound_reinit_after_load();
}

void reinit_tagged_blocks_for_player(unsigned char idx)
{
  _DK_reinit_tagged_blocks_for_player(idx);
}

long load_stats_files(void)
{
  return _DK_load_stats_files();
}

void check_and_auto_fix_stats(void)
{
  _DK_check_and_auto_fix_stats();
}

long update_dungeon_scores(void)
{
  return _DK_update_dungeon_scores();
}

long update_dungeon_generation_speeds(void)
{
  return _DK_update_dungeon_generation_speeds();
}

void calculate_dungeon_area_scores(void)
{
  _DK_calculate_dungeon_area_scores();
}

void setup_computer_players2(void)
{
  _DK_setup_computer_players2();
}

void gui_set_button_flashing(long a1, long a2)
{
  _DK_gui_set_button_flashing(a1, a2);
}

long get_next_research_item(struct Dungeon *dungeon)
{
  return _DK_get_next_research_item(dungeon);
}

void init_creature_level(struct Thing *thing, long nlev)
{
  _DK_init_creature_level(thing,nlev);
}

long creature_instance_has_reset(struct Thing *thing, long a2)
{
  return _DK_creature_instance_has_reset(thing, a2);
}

long get_human_controlled_creature_target(struct Thing *thing, long a2)
{
  return _DK_get_human_controlled_creature_target(thing, a2);
}

void set_creature_instance(struct Thing *thing, long a1, long a2, long a3, struct Coord3d *pos)
{
  _DK_set_creature_instance(thing, a1, a2, a3, pos);
}

void instant_instance_selected(long a1)
{
  _DK_instant_instance_selected(a1);
}

unsigned short find_next_annoyed_creature(unsigned char a1, unsigned short a2)
{
  return _DK_find_next_annoyed_creature(a1, a2);
}

unsigned char active_battle_exists(unsigned char a1)
{
  return _DK_active_battle_exists(a1);
}

unsigned char step_battles_forward(unsigned char a1)
{
  return _DK_step_battles_forward(a1);
}

short zoom_to_fight(unsigned char a1)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Packet *pckt;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if (active_battle_exists(a1))
  {
    dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
    pckt = &game.packets[player->packet_num%PACKETS_COUNT];
    set_packet_action(pckt, 104, dungeon->field_1174, 0, 0, 0);
    step_battles_forward(a1);
    return true;
  }
  return false;
}

short zoom_to_next_annoyed_creature(void)
{
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct Packet *pckt;
  struct Thing *thing;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
  dungeon->field_1177 = find_next_annoyed_creature(player->field_2B,dungeon->field_1177);
  if (dungeon->field_1177 > 0)
    thing = game.things_lookup[dungeon->field_1177%THINGS_COUNT];
  else
    thing = NULL;
  if ((thing != NULL) && (thing != game.things_lookup[0]))
  {
    pckt = &game.packets[player->packet_num%PACKETS_COUNT];
    set_packet_action(pckt, 87, thing->mappos.x.val, thing->mappos.y.val, 0, 0);
    return true;
  }
  return false;
}

void go_to_my_next_room_of_type(unsigned long rkind)
{
  _DK_go_to_my_next_room_of_type(rkind);
}

short activate_bonus_level(struct PlayerInfo *player)
{
  static const char *func_name="activate_bonus_level";
  int i;
  short result;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  game.flags_font |= 0x02;
  i = array_index_for_levels_bonus(game.level_file_number);
  if ((i>=0) && (i<5))
  {
    LbSyncLog("Used index %d to store bonus award for level %d\n",i,game.level_file_number);
    game.bonus_levels[i] = 1;
    result = true;
  } else
  {
    LbWarnLog("Couldn't find index to store bonus award for level %d\n",game.level_file_number);
    error(func_name, 517, "No Bonus level assigned to this level");
    result = false;
  }
  game.numfield_C &= 0xFFFDu;
  return result;
}

void multiply_creatures(struct PlayerInfo *player)
{
  static const char *func_name="multiply_creatures";
  struct Dungeon *dungeon;
  struct Thing *thing;
  struct Thing *tncopy;
  struct CreatureControl *cctrl;
  int i;

  dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
  i = dungeon->field_2D;
  while (i > 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4573,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    tncopy = create_creature(&thing->mappos, thing->model, player->field_2B);
    if (tncopy == NULL)
      break;
    set_creature_level(tncopy, thing->field_23);
    tncopy->field_5E = thing->field_5E;
    if (thing->field_64 >= CREATURES_COUNT)
      break;
    cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
    if (cctrl < game.creature_control_data)
      break;
    i = cctrl->thing_idx;
  }

  i = dungeon->field_2F;
  while (i >= 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4572,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    tncopy = create_creature(&thing->mappos, thing->model, player->field_2B);
    if (tncopy == NULL)
      break;
    set_creature_level(tncopy, thing->field_23);
    tncopy->field_5E = thing->field_5E;
    if (thing->field_64 >= CREATURES_COUNT)
      break;
    cctrl = game.creature_control_lookup[thing->field_64%CREATURES_COUNT];
    if (cctrl < game.creature_control_data)
      break;
    i = cctrl->thing_idx;
  }
}

short toggle_computer_player(int idx)
{
  if (game.dungeon[idx].computer_enabled & 0x01)
    game.dungeon[idx].computer_enabled &= 0xFE;
  else
    game.dungeon[idx].computer_enabled |= 0x01;
  return 1;
}

void set_player_instance(struct PlayerInfo *player, long a2, int a3)
{
  _DK_set_player_instance(player, a2, a3);
}

short save_version_compatible(long filesize,struct Game *header)
{
  // Native version
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor == VersionMinor))
    return true;
  // Compatibility list
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor <= 8) &&
    (VersionMinor == 10))
    return true;
  if ((filesize == sizeof(struct Game)) &&
    (header->version_major == VersionMajor) &&
    (header->version_minor <= 10) &&
    (VersionMinor == 12))
    return true;
  return false;
}

short load_texture_map_file(unsigned long tmapidx, unsigned char n)
{
  static const char *func_name="load_texture_map_file";
  //return _DK_load_texture_map_file(tmapidx, n);
  char *fname;
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  fname = prepare_file_fmtpath(FGrp_StdData,"tmapa%03d.dat",tmapidx);
  wait_for_cd_to_be_available();
  if (!LbFileExists(fname))
  {
    LbWarnLog("Texture file \"%s\" doesn't exits.\n",fname);
    return 0;
  }
  // The texture file has always over 500kb
  if (LbFileLoadAt(fname, block_mem) < 65536)
  {
    LbWarnLog("Texture file \"%s\" can't be loaded or is too small.\n",fname);
    return 0;
  }
  return 1;
}

void reinit_level_after_load(void)
{
  static const char *func_name="reinit_level_after_load";
  //_DK_reinit_level_after_load(); return;
  struct PlayerInfo *player;
  int i;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_7 = 0;
  init_lookups();
  init_navigation();
  parchment_loaded = 0;
  for (i=0; i<5; i++)
  {
    player = &(game.players[i%PLAYERS_COUNT]);
    if (player->field_0 & 0x01)
      set_engine_view(player, player->field_37);
  }
  start_rooms = &game.rooms[1];
  end_rooms = &game.rooms[ROOMS_COUNT];
  load_texture_map_file(game.texture_id, 2);
  init_animating_texture_maps();
  init_gui();
  reset_gui_based_on_player_mode();
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  reinit_tagged_blocks_for_player(player->field_2B);
  restore_computer_player_after_load();
  sound_reinit_after_load();
}

short save_catalogue_slot_disable(unsigned int slot_idx)
{
  char *fname;
  if (slot_idx >= SAVE_SLOTS_COUNT)
    return false;
  game.save_catalogue[slot_idx].used = false;
  fname = prepare_file_path(FGrp_Save,"save.cat");
  LbFileSaveAt(fname, &game.save_catalogue, sizeof(struct CatalogueEntry)*SAVE_SLOTS_COUNT);
  return true;
}

short load_game(long num)
{
  static const char *func_name="load_game";
  //return _DK_load_game(num);
  char *fname;
  TbFileHandle handle;
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  unsigned char buffer[10];
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  reset_eye_lenses();
  fname = prepare_file_fmtpath(FGrp_Save,"game%04d.sav",num);
  wait_for_cd_to_be_available();
  handle = LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
  if (handle == -1)
  {
    LbWarnLog("Cannot open saved game file \"%s\".\n",fname);
    save_catalogue_slot_disable(num);
    return 0;
  }
  if (LbFileRead(handle, &buffer, 10) != 10)
  {
    LbFileClose(handle);
    save_catalogue_slot_disable(num);
    return 0;
  }
  LbFileClose(handle);
  if (!save_version_compatible(LbFileLength(fname),(struct Game *)buffer))
  {
    LbWarnLog("Saved game file \"%s\" has incompatible version; restarting level.\n",fname);
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    player->field_7 = 0;
    my_player_number = default_loc_player;
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    game.flagfield_14EA4A = 2;
    game.numfield_A &= 0xFEu;
    player->field_2C = 1;
    game.level_file_number = buffer[0];
    game.level_number = buffer[0];
    startup_network_game();
    return 1;
  }
  if (LbFileLoadAt(fname, &game) != sizeof(struct Game))
  {
    LbWarnLog("Couldn't correctly load saved game \"%s\".\n",fname);
    return 0;
  }
  init_lookups();
  reinit_level_after_load();
  output_message(102, 0, 1);
  pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
  calculate_moon_phase(false,false);
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_3 &= 0xF7;
  player->field_3 &= 0xFB;
  player->field_4C1 = 0;
  player->field_4C5 = 0;
  player->field_7 = 0;
  PaletteSetPlayerPalette(player, _DK_palette);
  reinitialise_eye_lens(game.numfield_1B);
  if (player->field_29 != 0)
  {
    frontstats_initialise();
    dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
    dungeon->player_score = 0;
    dungeon->allow_save_score = 1;
  }
  game.field_1516FF = 0;
  return 1;
}

int LbNetwork_Exchange(struct Packet *pckt)
{
  return _DK_LbNetwork_Exchange(pckt);
}

void light_initialise(void)
{
  _DK_light_initialise();
}

void delete_all_structures(void)
{
  _DK_delete_all_structures();
}

void clear_mapwho(void)
{
  //_DK_clear_mapwho();
  struct Map *map;
  unsigned long x,y;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      map->field_1 &= 0xFFC007FFu;
    }
}

void clear_slabs(void)
{
  struct SlabMap *slb;
  unsigned long x,y;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slb = &game.slabmap[y*map_tiles_x + x];
      slb->slab = SlbT_ROCK;
      slb->field_3 = 0;
      slb->field_4 = 0;
    }
}

void clear_mapmap(void)
{
  struct Map *map;
  unsigned long x,y;
  unsigned short *wptr;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      wptr = &game.field_46157[y*(map_subtiles_x+1) + x];
      map->field_1 &= 0xFF3FFFFFu;
      map->field_1 &= 0xFFFFF800u;
      map->field_1 &= 0xFFC007FFu;
      map->field_1 &= 0x0FFFFFFFu;
      map->field_0 = 0;
      *wptr = 8192;
    }
}

void clear_map(void)
{
  //_DK_clear_map();
  clear_mapmap();
  clear_slabs();
  clear_columns();
}

void make_solidmask(struct Column *col)
{
  int i;
  col->solidmask = 0;
  for (i=0; i<COLUMN_STACK_HEIGHT; i++)
  {
    if (col->cubes[i] != 0)
      col->solidmask |= (1 << i);
  }
}

unsigned short find_column_height(struct Column *col)
{
  unsigned short h;
  h = 0;
  if (col->solidmask == 0)
    return h;
  while (col->cubes[h] > 0)
  {
    h++;
    if (h >= COLUMN_STACK_HEIGHT)
      return COLUMN_STACK_HEIGHT;
  }
  return h;
}

void clear_columns(void)
{
  //  _DK_clear_columns();
  struct Column *col;
  int i;
  for (i=0; i < COLUMNS_COUNT; i++)
  {
    col = &game.columns[i];
    memset(col, 0, sizeof(struct Column));
    col->baseblock = 1;
    make_solidmask(col);
  }
}

void clear_players(void)
{
  int i;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    memset(&game.players[i], 0, sizeof(struct PlayerInfo));
  }
}

void init_keepers_map_exploration(void)
{
  struct PlayerInfo *player;
  int i;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player = &(game.players[i%PLAYERS_COUNT]);
    if ((player->field_0 & 0x01) && (player->field_2C == 1))
    {
        if (player->field_0 & 0x40)
          init_keeper_map_exploration(player);
    }
  }
}

void clear_players_for_save(void)
{
  struct PlayerInfo *player;
  unsigned short mem1,mem2,memflg;
  int i;
  for (i=0; i < PLAYERS_COUNT; i++)
  {
    player=&(game.players[i%PLAYERS_COUNT]);
    mem1 = player->field_2B;
    mem2 = player->field_2C;
    memflg = player->field_0;
    memset(player, 0, sizeof(struct PlayerInfo));
    player->field_2B = mem1;
    player->field_2C = mem2;
    set_flag_byte(&player->field_0,0x01,((memflg & 0x01) != 0));
    set_flag_byte(&player->field_0,0x40,((memflg & 0x40) != 0));
  }
}

void clear_dungeons(void)
{
  int i;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    memset(&game.dungeon[i], 0, sizeof(struct Dungeon));
  }
}

void clear_game(void)
{
  //_DK_clear_game(); return;
  delete_all_structures();
  light_initialise();
  clear_mapwho();
  game.field_14E938 = 0;
  game.field_14BB4A = 0;
  game.numfield_C &= 0xFFFBu;
  clear_columns();
  clear_players();
  clear_dungeons();
}

void clear_game_for_save(void)
{
  //_DK_clear_game_for_save(); return;
  delete_all_structures();
  light_initialise();
  clear_mapwho();
  game.field_14E938 = 0;
  game.field_14BB4A = 0;
  game.numfield_C &= 0xFFFBu;
  clear_columns();
  clear_players_for_save();
  clear_dungeons();
}

void init_good_player_as(long plr_idx)
{
  struct PlayerInfo *player;
  game.field_14E496 = plr_idx;
  player = &(game.players[plr_idx%PLAYERS_COUNT]);
  player->field_0 |= 0x01;
  player->field_0 |= 0x40;
  player->field_2B = game.field_14E496;
}

void resync_game(void)
{
  return _DK_resync_game();
}

void free_swipe_graphic(void)
{
  static const char *func_name="free_swipe_graphic";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_free_swipe_graphic();
}

void message_add(char c)
{
  _DK_message_add(c);
}

void light_stat_light_map_clear_area(long x1, long y1, long x2, long y2)
{
  _DK_light_stat_light_map_clear_area(x1, y1, x2, y2);
}

void light_signal_stat_light_update_in_area(long x1, long y1, long x2, long y2)
{
  _DK_light_signal_stat_light_update_in_area(x1, y1, x2, y2);
}

void light_set_lights_on(char state)
{
  if (state)
  {
    game.field_46149 = 10;
    game.field_4614D = 1;
  } else
  {
    game.field_46149 = 32;
    game.field_4614D = 0;
  }
  light_stat_light_map_clear_area(0, 0, 255, 255);
  light_signal_stat_light_update_in_area(0, 0, 255, 255);
}

void change_engine_window_relative_size(long w_delta, long h_delta)
{
  struct PlayerInfo *myplyr;
  myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
  setup_engine_window(myplyr->engine_window_x, myplyr->engine_window_y,
    myplyr->engine_window_width+w_delta, myplyr->engine_window_height+h_delta);
}

void PaletteSetPlayerPalette(struct PlayerInfo *player, unsigned char *pal)
{
  _DK_PaletteSetPlayerPalette(player, pal);
}

short set_gamma(char corrlvl, short do_set)
{
  static const char *func_name="set_gamma";
  char *fname;
  short result=1;
  if (corrlvl < 0)
    corrlvl = 0;
  else
  if (corrlvl > 4)
    corrlvl = 4;
  settings.gamma_correction = corrlvl;
  fname=prepare_file_fmtpath(FGrp_StdData,"pal%05d.dat",settings.gamma_correction);
  if (!LbFileExists(fname))
  {
    LbWarnLog("Palette file \"%s\" doesn't exist.\n", fname);
    result = 0;
  }
  if (result)
  {
    result = (LbFileLoadAt(fname, _DK_palette) != -1);
  }
  if ((result)&&(do_set))
  {
    struct PlayerInfo *myplyr;
    myplyr=&(game.players[my_player_number%PLAYERS_COUNT]);
    PaletteSetPlayerPalette(myplyr, _DK_palette);
  }
  if (!result)
    error(func_name, 3720, "Can't load palette file.");
  return result;
}

void centre_engine_window(void)
{
  long x1,y1;
  struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (game.numfield_C & 0x20)
    x1 = (MyScreenWidth-player->engine_window_width-status_panel_width) / 2 + status_panel_width;
  else
    x1 = (MyScreenWidth-player->engine_window_width) / 2;
  y1 = (MyScreenHeight-player->engine_window_height) / 2;
  setup_engine_window(x1, y1, player->engine_window_width, player->engine_window_height);
}

void set_engine_view(struct PlayerInfo *player, long val)
{
  _DK_set_engine_view(player, val);
}

void toggle_creature_tendencies(struct PlayerInfo *player, short val)
{
  _DK_toggle_creature_tendencies(player, val);
}

void set_player_state(struct PlayerInfo *player, short a1, long a2)
{
  _DK_set_player_state(player, a1, a2);
}

void set_player_mode(struct PlayerInfo *player, long val)
{
  _DK_set_player_mode(player, val);
}

void turn_off_query(short a)
{
  _DK_turn_off_query(a);
}

void turn_off_call_to_arms(long a)
{
  _DK_turn_off_call_to_arms(a);
}

long event_move_player_towards_event(struct PlayerInfo *player, long var)
{
  return _DK_event_move_player_towards_event(player,var);
}

long battle_move_player_towards_battle(struct PlayerInfo *player, long var)
{
  return _DK_battle_move_player_towards_battle(player, var);
}

long place_thing_in_power_hand(struct Thing *thing, long var)
{
  return _DK_place_thing_in_power_hand(thing, var);
}

short dump_held_things_on_map(unsigned int plyridx, long a2, long a3, short a4)
{
  return _DK_dump_held_things_on_map(plyridx, a2, a3, a4);
}

void magic_use_power_armageddon(unsigned int plridx)
{
  _DK_magic_use_power_armageddon(plridx);
}

long set_autopilot_type(unsigned int plridx, long aptype)
{
  return _DK_set_autopilot_type(plridx, aptype);
}

short magic_use_power_obey(unsigned short plridx)
{
  return _DK_magic_use_power_obey(plridx);
}

void turn_off_sight_of_evil(long plridx)
{
  _DK_turn_off_sight_of_evil(plridx);
}

void reset_scroll_window(void)
{
  game.field_1512D9 = 0;
  game.field_1512DD = 0;
  game.field_1512DE = 0;
  game.field_1512E2 = 0;
}

unsigned char find_first_battle_of_mine(unsigned char idx)
{
  return _DK_find_first_battle_of_mine(idx);
}

void level_lost_go_first_person(long plridx)
{
  static const char *func_name="level_lost_go_first_person";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_level_lost_go_first_person(plridx);
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void go_on_then_activate_the_event_box(long plridx, long evidx)
{
  static const char *func_name="go_on_then_activate_the_event_box";
  //_DK_go_on_then_activate_the_event_box(plridx, val); return;
  struct Dungeon *dungeon;
  struct Event *event;
  struct Thing *thing;
  char *text;
  int i;
  short other_off;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  dungeon = &(game.dungeon[plridx%DUNGEONS_COUNT]);
  dungeon->field_1173 = evidx;
  event = &game.event[evidx];
  if (plridx == my_player_number)
  {
    i = event_button_info[event->field_B].field_6;
    if (i != 201)
      strcpy(game.text_info, strings[i%STRINGS_MAX]);
  }
  if (event->field_B == 2)
    dungeon->field_1174 = find_first_battle_of_mine(plridx);
  if (plridx == my_player_number)
  {
    other_off = 0;
    switch ( event->field_B )
    {
    case 1:
    case 4:
        other_off = 1;
        turn_on_menu(16);
        break;
    case 2:
        turn_off_menu(16);
        turn_on_menu(34);
        break;
    case 3:
        strcpy(game.text_info, game.field_1506D9);
        for (i=13; i>=0; i--)
        {
          // Not sure about the bound being 121
          if (game.event[dungeon->field_13A7[i] % 121].field_B == 3)
          {
            other_off = 1;
            turn_on_menu(16);
            new_objective = 0;
            break;
          }
        }
        break;
    case 5:
        other_off = 1;
        i = room_data[event->field_C].field_13;
        text=buf_sprintf("%s:\n %s",game.text_info,strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 6:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = creature_data[thing->model % CREATURE_TYPES_COUNT].field_3;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 7:
        other_off = 1;
        i = spell_data[event->field_C % (SPELL_TYPES_COUNT+1)].field_D;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 8:
        other_off = 1;
        i = trap_data[event->field_C % TRAP_TYPES_COUNT].field_C;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 9:
        other_off = 1;
        i = door_names[event->field_C % DOOR_TYPES_COUNT];
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 10:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = creature_data[thing->model % CREATURE_TYPES_COUNT].field_3;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 11:
    case 13:
        other_off = 1;
        turn_on_menu(16);
        break;
    case 12:
        other_off = 1;
        text = buf_sprintf("%s:\n %d", game.text_info, event->field_C);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 14:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = spell_data[object_to_magic[thing->model % OBJECT_TYPES_COUNT]].field_D;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 15:
        other_off = 1;
        i = room_data[event->field_C].field_13;
        text = buf_sprintf("%s:\n %s",game.text_info,strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 16:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = creature_data[thing->model % CREATURE_TYPES_COUNT].field_3;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    case 17:
    case 18:
    case 19:
    case 20:
    case 22:
    case 23:
        other_off = 1;
        turn_on_menu(16);
        break;
    case 21:
        i = (long)event->field_C;
        if (i < 0)
        {
          i = -i;
          event->field_C = i;
        }
        strncpy(game.field_150AD9, strings[i%STRINGS_MAX], sizeof(game.field_150AD9)-1);
        strncpy(game.text_info, game.field_150AD9, sizeof(game.text_info)-1);
        other_off = 1;
        turn_on_menu(16);
        break;
    case 24:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = trap_data[object_to_door_or_trap[thing->model % OBJECT_TYPES_COUNT]].field_C;
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
      case 25:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = door_names[object_to_door_or_trap[thing->model % OBJECT_TYPES_COUNT]];
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
      case 26:
        other_off = 1;
        thing = game.things_lookup[event->field_C%THINGS_COUNT];
        if ((thing == game.things_lookup[0]) || (thing == NULL))
          break;
        i = specials_text[object_to_special[thing->model % OBJECT_TYPES_COUNT]];
        text = buf_sprintf("%s:\n %s", game.text_info, strings[i%STRINGS_MAX]);
        strncpy(game.text_info,text,sizeof(game.text_info)-1);
        turn_on_menu(16);
        break;
    default:
        text = buf_sprintf("Undefined event type: %d", (int)event->field_B);
        error(func_name, 9100, text);
        break;
    }
    reset_scroll_window();
    if (other_off)
    {
      turn_off_menu(34);
      turn_off_menu(27);
      turn_off_menu(28);
      turn_off_menu(29);
    }
  }
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void directly_cast_spell_on_thing(long plridx, unsigned char a2, unsigned short a3, long a4)
{
  _DK_directly_cast_spell_on_thing(plridx, a2, a3, a4);
}

void event_delete_event(long plridx, long num)
{
  _DK_event_delete_event(plridx, num);
}

void lose_level(struct PlayerInfo *player)
{
  _DK_lose_level(player);
}

void complete_level(struct PlayerInfo *player)
{
  static const char *func_name="complete_level";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_complete_level(player);
}

long init_navigation(void)
{
  return _DK_init_navigation();
}

void init_lookups(void)
{
  _DK_init_lookups();
}

void  toggle_ally_with_player(long plyridx, unsigned int allyidx)
{
  struct PlayerInfo *player=&(game.players[plyridx%PLAYERS_COUNT]);
  player->field_2A ^= (1 << allyidx);
}

void set_mouse_light(struct PlayerInfo *player)
{
  static const char *func_name="set_mouse_light";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_set_mouse_light(player);
}

/*
 * Scales camera zoom level on resolution change. If prev_units_per_pixel_size
 * is zero, then the zoom level will be only clipped, without any scaling.
 */
void keep_camera_zoom_level(struct Camera *cam,unsigned long prev_units_per_pixel_size)
{
  long zoom_val;
  unsigned long zoom_min,zoom_max;
  zoom_min = scale_camera_zoom_to_screen(CAMERA_ZOOM_MIN);
  zoom_max = scale_camera_zoom_to_screen(CAMERA_ZOOM_MAX);
  zoom_val = get_camera_zoom(cam);
  // Note: I don't know if the zoom may be scaled for current resolution,
  // as there may be different resolution on another computer if playing MP game.
  if (prev_units_per_pixel_size > 0)
    zoom_val = zoom_val*units_per_pixel*pixel_size/prev_units_per_pixel_size;
  if (zoom_val < zoom_min)
  {
    zoom_val = zoom_min;
  } else
  if (zoom_val > zoom_max)
  {
    zoom_val = zoom_max;
  }
  set_camera_zoom(cam, zoom_val);
}

/*
 * Scales local player camera zoom level on resolution change. If prev_units_per_pixel_size
 * is zero, then the zoom level will be only clipped, without any scaling.
 */
void keep_local_camera_zoom_level(unsigned long prev_units_per_pixel_size)
{
  struct PlayerInfo *player;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  keep_camera_zoom_level(player->camera,prev_units_per_pixel_size);
}

/*
 * Conducts clipping to zoom level of given camera, based on current screen mode.
 */
void update_camera_zoom_bounds(struct Camera *cam,unsigned long zoom_max,unsigned long zoom_min)
{
  static const char *func_name="update_camera_zoom_bounds";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  long zoom_val;
  zoom_val = get_camera_zoom(cam);
  if (zoom_val < zoom_min)
  {
    zoom_val = zoom_min;
  } else
  if (zoom_val > zoom_max)
  {
    zoom_val = zoom_max;
  }
  set_camera_zoom(cam, zoom_val);
}

void magic_use_power_hold_audience(unsigned char idx)
{
  _DK_magic_use_power_hold_audience(idx);
}

void delete_thing_structure(struct Thing *thing, long a2)
{
  _DK_delete_thing_structure(thing, a2);
}

struct Thing *create_effect(struct Coord3d *pos, unsigned short a2, unsigned char a3)
{
  return _DK_create_effect(pos, a2, a3);
}

void create_special_used_effect(struct Coord3d *pos, long a2)
{
  create_effect(pos, 67, a2);
}

void activate_dungeon_special(struct Thing *thing, struct PlayerInfo *player)
{
  static const char *func_name="activate_dungeon_special";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_activate_dungeon_special(thing, player); return;
  short used;
  struct Coord3d pos;
  int spkindidx;

  // Gathering data which we'll need if the special is used and disposed.
  memcpy(&pos,&thing->mappos,sizeof(struct Coord3d));
  spkindidx = thing->model - 86;
  used = 0;
  if ((thing->field_0 & 0x01) && is_dungeon_special(thing))
  {
    switch (thing->model)
    {
        case 86:
          reveal_whole_map(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 87:
          start_resurrect_creature(player, thing);
          break;
        case 88:
          start_transfer_creature(player, thing);
          break;
        case 89:
          if (steal_hero(player, &thing->mappos))
          {
            remove_events_thing_is_attached_to(thing);
            used = 1;
            delete_thing_structure(thing, 0);
          }
          break;
        case 90:
          multiply_creatures(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 91:
          increase_level(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 92:
          make_safe(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        case 93:
          activate_bonus_level(player);
          remove_events_thing_is_attached_to(thing);
          used = 1;
          delete_thing_structure(thing, 0);
          break;
        default:
          error(func_name, 360, buf_sprintf("Invalid dungeon special (Model %d)", (int)thing->model));
          break;
      }
      if ( used )
      {
        if (player == &game.players[my_player_number])
          output_message(special_desc[spkindidx].field_8, 0, 1);
        create_special_used_effect(&pos, player->field_2B);
      }
  }
}

void resurrect_creature(struct Thing *thing, unsigned char a2, unsigned char a3, unsigned char a4)
{
  _DK_resurrect_creature(thing, a2, a3, a4);
}

void transfer_creature(struct Thing *tng1, struct Thing *tng2, unsigned char a3)
{
  _DK_transfer_creature(tng1, tng2, a3);
}

void process_rooms(void)
{
  static const char *func_name="process_rooms";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_rooms();
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void check_players_won(void)
{
  static const char *func_name="check_players_won";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_check_players_won();
}

void check_players_lost(void)
{
  static const char *func_name="check_players_lost";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_check_players_lost();
}

void process_dungeon_power_magic(void)
{
  static const char *func_name="process_dungeon_power_magic";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_dungeon_power_magic();
}

void process_dungeon_devastation_effects(void)
{
  static const char *func_name="process_dungeon_devastation_effects";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_dungeon_devastation_effects();
}

void process_entrance_generation(void)
{
  static const char *func_name="process_entrance_generation";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_entrance_generation();
}

void process_things_in_dungeon_hand(void)
{
  _DK_process_things_in_dungeon_hand();
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
    dungeon = (&game.dungeon[i]);
    player = &(game.players[i%PLAYERS_COUNT]);
    if (player->field_0 & 0x01)
    {
      game.field_14E4A0 += dungeon->field_AF9;
      game.field_14E4A4 += dungeon->field_918;
      game.field_14E49E += dungeon->field_919;
    }
  }
}

void process_dungeons(void)
{
  static const char *func_name="process_dungeons";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_process_dungeons();
  check_players_won();
  check_players_lost();
  process_dungeon_power_magic();
  count_dungeon_stuff();
  process_dungeon_devastation_effects();
  process_entrance_generation();
  process_payday();
  process_things_in_dungeon_hand();
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void process_messages(void)
{
  static const char *func_name="process_messages";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_messages();
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

/*
 * Returns if there is a bonus timer visible on the level.
 */
short bonus_timer_enabled(void)
{
  return is_bonus_level(game.level_file_number);
}

void find_nearest_rooms_for_ambient_sound(void)
{
  _DK_find_nearest_rooms_for_ambient_sound();
}

void light_render_area(int startx, int starty, int endx, int endy)
{
  _DK_light_render_area(startx, starty, endx, endy);
}

void process_3d_sounds(void)
{
  _DK_process_3d_sounds();
}

void process_player_research(int plr_idx)
{
  _DK_process_player_research(plr_idx);
}

struct Room *player_has_room_of_type(long plr_idx, long roomkind)
{
  return _DK_player_has_room_of_type(plr_idx, roomkind);
}

struct Room *find_room_with_spare_room_item_capacity(unsigned char a1, signed char a2)
{
  return _DK_find_room_with_spare_room_item_capacity(a1, a2);
}

long create_workshop_object_in_workshop_room(long a1, long a2, long a3)
{
  return _DK_create_workshop_object_in_workshop_room(a1, a2, a3);
}

long get_next_manufacture(struct Dungeon *dungeon)
{
  return _DK_get_next_manufacture(dungeon);
}

void remove_thing_from_mapwho(struct Thing *thing)
{
  static const char *func_name="remove_thing_from_mapwho";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_remove_thing_from_mapwho(thing);
}

void place_thing_in_mapwho(struct Thing *thing)
{
  static const char *func_name="place_thing_in_mapwho";
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_place_thing_in_mapwho(thing);
}

long get_thing_height_at(struct Thing *thing, struct Coord3d *pos)
{
  static const char *func_name="get_thing_height_at";
#if (BFDEBUG_LEVEL > 18)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  return _DK_get_thing_height_at(thing, pos);
}

long manufacture_required(long mfcr_type, unsigned long mfcr_kind, const char *func_name)
{
  switch (mfcr_type)
  {
  case 8:
      return game.traps_config[mfcr_kind%TRAP_TYPES_COUNT].field_4;
  case 9:
      return game.doors_config[mfcr_kind%DOOR_TYPES_COUNT].field_4;
  default:
      error(func_name, 310, "Invalid type of manufacture");
      return 0;
  }
}

short process_player_manufacturing(long plr_idx)
{
  struct Dungeon *dungeon;
  struct PlayerInfo *player;
  int i,k;
  static const char *func_name="process_player_manufacturing";
#if (BFDEBUG_LEVEL > 17)
    LbSyncLog("%s: Starting\n",func_name);
#endif
//  return _DK_process_player_manufacturing(plr_idx);

  dungeon = &(game.dungeon[plr_idx%DUNGEONS_COUNT]);
  if (player_has_room_of_type(plr_idx, 8) == NULL)
    return true;
  if (dungeon->field_1189 == 0)
  {
    get_next_manufacture(dungeon);
    return true;
  }
  k = manufacture_required(dungeon->field_1189, dungeon->field_118A, func_name);
  if (dungeon->field_1185 < (k << 8))
    return true;

  if (find_room_with_spare_room_item_capacity(plr_idx, 8) == NULL)
  {
    dungeon->field_1189 = 0;
    return false;
  }
  if (create_workshop_object_in_workshop_room(plr_idx, dungeon->field_1189, dungeon->field_118A) == 0)
  {
    error(func_name, 173, "Could not create manufactured item");
    return false;
  }

  switch (dungeon->field_1189)
  {
  case 8:
      i = (dungeon->field_118A + 1);
      if (dungeon->field_1007[i] >= 199)
      {
        error(func_name, 159, "Bad trap choice");
        return false;
      }
      dungeon->field_1007[i]++;
      dungeon->field_122F++;
      i = (dungeon->field_118A + 15);
      dungeon->field_1007[i] = 1;
      // If that's local player - make a message
      player=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (player->field_2B == plr_idx)
        output_message(45, 0, 1);
      break;
  case 9:
      i = (dungeon->field_118A + 22);
      if (dungeon->field_1007[i] >= 199)
      {
        error(func_name, 139, "Bad door choice");
        return 0;
      }
      dungeon->field_1007[i]++;
      dungeon->field_122B++;
      i = (dungeon->field_118A + 32);
      dungeon->field_1007[i] = 1;
      // If that's local player - make a message
      player=&(game.players[my_player_number%PLAYERS_COUNT]);
      if (player->field_2B == plr_idx)
        output_message(44, 0, 1);
      break;
  default:
      error(func_name, 166, "Invalid type of new manufacture");
      return false;
  }

  dungeon->field_1185 -= (k << 8);
  dungeon->field_118B = game.seedchk_random_used;
  dungeon->field_1233++;
  get_next_manufacture(dungeon);
  return true;
}

void event_process_events(void)
{
  _DK_event_process_events();
}

void update_all_events(void)
{
  _DK_update_all_events();
}

void process_computer_players2(void)
{
  _DK_process_computer_players2();
}

void process_level_script(void)
{
  _DK_process_level_script(); return;

}

long process_action_points(void)
{
  return _DK_process_action_points();
}

long PaletteFadePlayer(struct PlayerInfo *player)
{
  return _DK_PaletteFadePlayer(player);
}

void message_update(void)
{
  _DK_message_update();
}

void process_player_instance(struct PlayerInfo *player)
{
  static const char *func_name="process_player_instance";
  struct PlayerInstanceInfo *inst_info;
  InstncInfo_Func callback;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting for instance %d\n",func_name,player->instance_num);
#endif
  //_DK_process_player_instance(player); return;
  if (player->instance_num > 0)
  {
    if (player->field_4B1 > 0)
    {
      player->field_4B1--;
      inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
      callback = inst_info->maintain_cb;
      if (callback != NULL)
        callback(player, &inst_info->field_24);
    }
    if (player->field_4B1 == 0)
    {
      inst_info = &player_instance_info[player->instance_num%PLAYER_INSTANCES_COUNT];
      player->instance_num = 0;
      callback = inst_info->end_cb;
      if (callback != NULL)
        callback(player, &inst_info->field_24);
    }
  }
}

void process_player_instances(void)
{
  static const char *func_name="process_player_instances";
  //_DK_process_player_instances();return;
  int i;
  struct PlayerInfo *player;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i]);
    if (player->field_0 & 0x01)
      process_player_instance(player);
  }
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

long wander_point_update(struct Wander *wandr)
{
  return _DK_wander_point_update(wandr);
}

void update_player_camera(struct PlayerInfo *player)
{
  _DK_update_player_camera(player);
}

void update_research(void)
{
  static const char *func_name="update_research";
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) && (player->field_2C == 1))
      {
          process_player_research(i);
      }
  }
}

void update_manufacturing(void)
{
  static const char *func_name="update_manufacturing";
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) && (player->field_2C == 1))
      {
          process_player_manufacturing(i);
      }
  }
}

void update_all_players_cameras(void)
{
  static const char *func_name="update_all_players_cameras";
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  for (i=0; i<PLAYERS_COUNT; i++)
  {
    player=&(game.players[i]);
    if ((player->field_0 & 0x01) && ((player->field_0 & 0x40) == 0))
    {
          update_player_camera(player);
    }
  }
}

#define LIGHT_MAX_RANGE 30
void update_light_render_area(void)
{
  static const char *func_name="update_light_render_area";
  int subtile_x,subtile_y;
  int delta_x,delta_y;
  int startx,endx,starty,endy;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  if (player->field_37 >= 1)
    if ((player->field_37 <= 2) || (player->field_37 == 5))
    {
        game.field_14BB5D = LIGHT_MAX_RANGE;
        game.field_14BB59 = LIGHT_MAX_RANGE;
    }
  delta_x=abs(game.field_14BB59);
  delta_y=abs(game.field_14BB5D);
  // Prepare the area constraits
  subtile_y = player->camera->mappos.y.stl.num;
  subtile_x = player->camera->mappos.x.stl.num;
//LbSyncLog("LghtRng %d,%d CamTil %d,%d\n",game.field_14BB59,game.field_14BB5D,tile_x,tile_y);
  if (subtile_y > delta_y)
  {
    starty = subtile_y - delta_y;
    if (starty > map_subtiles_y) starty = map_subtiles_y;
  } else
    starty = 0;
  if (subtile_x > delta_x)
  {
    startx = subtile_x - delta_x;
    if (startx > map_subtiles_x) startx = map_subtiles_x;
  } else
    startx = 0;
  endy = subtile_y + delta_y;
  if (endy < starty) endy = starty;
  if (endy > map_subtiles_y) endy = map_subtiles_y;
  endx = subtile_x + delta_x;
  if (endx < startx) endx = startx;
  if (endx > map_subtiles_x) endx = map_subtiles_x;
  // Set the area
  light_render_area(startx, starty, endx, endy);
}

void update_flames_nearest_camera(struct Camera *camera)
{
  _DK_update_flames_nearest_camera(camera);
}

void update_footsteps_nearest_camera(struct Camera *camera)
{
  _DK_update_footsteps_nearest_camera(camera);
}

void process_player_states(void)
{
  static const char *func_name="process_player_states";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_process_player_states();
}

void update_power_sight_explored(struct PlayerInfo *player)
{
  static const char *func_name="update_power_sight_explored";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_update_power_sight_explored(player);
}

void update_player_objectives(int plridx)
{
  static const char *func_name="update_player_objectives";
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting for player %d\n",func_name,plridx);
#endif
  player=&(game.players[plridx]);
  if (game.numfield_A & 0x01)
  {
    if ((!player->field_4EB) && (player->field_29))
      player->field_4EB = game.seedchk_random_used+1;
  }
  if (player->field_4EB == game.seedchk_random_used)
  {
    switch (player->field_29)
    {
    case 1:
        if (plridx == my_player_number)
          _DK_set_level_objective(strings[0]);
        display_objectives(player->field_2B, 0, 0);
        break;
    case 2:
        if (plridx == my_player_number)
          _DK_set_level_objective(strings[335]);
        display_objectives(player->field_2B, 0, 0);
        break;
    }
  }
}

void process_players(void)
{
  static const char *func_name="process_players";
  int i;
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  process_player_instances();
  process_player_states();
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player=&(game.players[i]);
      if ((player->field_0 & 0x01) && (player->field_2C == 1))
      {
  #if (BFDEBUG_LEVEL > 6)
      LbSyncLog("%s: Doing updates for player %d\n",func_name,i);
  #endif
          wander_point_update(&player->wandr1);
          wander_point_update(&player->wandr2);
          update_power_sight_explored(player);
          update_player_objectives(i);
      }
  }
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

short update_animating_texture_maps(void)
{
  int i;
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

void draw_slab64k(long pos_x, long pos_y, long width, long height)
{
  _DK_draw_slab64k(pos_x, pos_y, width, height);
}

long update_cave_in(struct Thing *thing)
{
  return _DK_update_cave_in(thing);
}

void update_player_sounds(void)
{
  int k;
  struct Camera *camera;
  if ((game.numfield_C & 0x01) == 0)
  {
    camera=game.players[my_player_number%PLAYERS_COUNT].camera;
    process_messages();
    if (!SoundDisabled)
    {
      if ((game.flags_cd & MFlg_NoMusic) == 0)
      {
        if (game.audiotrack > 0)
          PlayRedbookTrack(game.audiotrack);
      }
      S3DSetSoundReceiverPosition(camera->mappos.x.val,camera->mappos.y.val,camera->mappos.z.val);
      S3DSetSoundReceiverOrientation(camera->orient_a,camera->orient_b,camera->orient_c);
    }
    game.seedchk_random_used++;
  }
  find_nearest_rooms_for_ambient_sound();
  process_3d_sounds();
  k = (game.field_1517E2-game.seedchk_random_used) / 2;
  if (bonus_timer_enabled())
  {
    if ((game.field_1517E2 == game.seedchk_random_used) ||
        (game.field_1517E2 > game.seedchk_random_used) && ((k<=100) && ((k % 10) == 0) ||
        (k<=300) && ((k % 50)==0) || ((k % 250)==0)) )
      play_non_3d_sample(89);
  }
}

void update(void)
{
  static const char *func_name="update";
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  int i,k;
#if (BFDEBUG_LEVEL > 4)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_update();return;

  if ((game.numfield_C & 0x01) == 0)
    update_light_render_area();
  process_packets();
  if (quit_game)
  {
    return;
  }
  if (game.flagfield_14EA4A == 1)
  {
    game.field_14EA4B = 0;
    return;
  }

  if ((game.numfield_C & 0x01) == 0)
  {
    player=&(game.players[my_player_number%PLAYERS_COUNT]);
    if (player->field_3 & 0x08)
    {
      PaletteSetPlayerPalette(player, _DK_palette);
      player->field_3 &= 0xF7u;
    }

    for (i=0; i<=game.field_14E496; i++)
    {
      dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
      memset((char *)dungeon->field_64, 0, 480 * sizeof(short));
      memset((char *)dungeon->field_424, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
      memset((char *)dungeon->field_4E4, 0, CREATURE_TYPES_COUNT*3*sizeof(unsigned short));
    }

    game.creature_pool_empty = 1;
    for (i=1; i < CREATURE_TYPES_COUNT; i++)
    {
      if (game.creature_pool[i] > 0)
      { game.creature_pool_empty = 0; break; }
    }

    if ((game.seedchk_random_used & 0x01) != 0)
    {
      update_animating_texture_maps();
    }
    update_things();
    process_rooms();
    process_dungeons();
    update_research();
    update_manufacturing();
    event_process_events();
    update_all_events();
    process_level_script();
    if (game.numfield_D & 0x04)
      process_computer_players2();
    process_players();
    process_action_points();
    player=&(game.players[my_player_number%PLAYERS_COUNT]);
    if (player->field_37 == 1)
      update_flames_nearest_camera(player->camera);
    update_footsteps_nearest_camera(player->camera);
    PaletteFadePlayer(player);
    _DK_process_armageddon();
  }

  message_update();
  update_all_players_cameras();
  update_player_sounds();

  // Rare message easter egg
  if ((game.seedchk_random_used != 0) && ((game.seedchk_random_used % 0x4E20) == 0))
  {
      if (seed_check_random(0x7D0u, &game.field_14BB4A, func_name, 4345) == 0)
      {
        if (seed_check_random(10, &game.field_14BB4E, func_name, 4346) == 7)
        {
          output_message(94, 0, 1);// 'Your pants are definitely too tight'
        } else
        {
          output_message((game.field_14BB4E % 10) + 91, 0, 1);
        }
      }
  }
  game.field_14EA4B = 0;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void draw_tooltip(void)
{
  static const char *func_name="draw_tooltip";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_draw_tooltip();
}

long map_fade_in(long a)
{
  return _DK_map_fade_in(a);
}

long map_fade_out(long a)
{
  return _DK_map_fade_out(a);
}

void draw_2d_map(void)
{
  _DK_draw_2d_map();
}

/*
 * Strange name to hide easter eggs ;). Displays easter egg messages on screen.
 */
void draw_sound_stuff(void)
{
  static const char *func_name="draw_sound_stuff";
  char *text;
  int i;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  if (eastegg03_cntr > 6)
  {
      unsigned char pos;
      eastegg03_cntr++;
      lbFontPtr = winfont;
      text=buf_sprintf("Dene says a big 'Hello' to Goth Buns, Tarts and Barbies");
      lbDisplay.DrawFlags = 0x40;
      for (i=0; i<30; i+=2)
      {
        pos = game.seedchk_random_used - i;
        lbDisplay.DrawColour = pos;
        LbTextDraw((lbCosTable[16*(pos&0x7F)] / 512 + 120) / pixel_size,
          (lbSinTable[32*(pos&0x3F)] / 512 + 200) / pixel_size, text);
      }
      lbDisplay.DrawFlags &= 0xFFBFu;
      pos=game.seedchk_random_used;
      LbTextDraw((lbCosTable[16*(pos&0x7F)] / 512 + 120) / pixel_size,
        (lbSinTable[32*(pos&0x3F)] / 512 + 200) / pixel_size, text);
      if (eastegg03_cntr > 1000)
        eastegg03_cntr = 0;
  }
  _DK_draw_sound_stuff();
}

void process_pointer_graphic(void)
{
  _DK_process_pointer_graphic();
}

void draw_bonus_timer(void)
{
  _DK_draw_bonus_timer(); return;
}

void message_draw(void)
{
  static const char *func_name="message_draw";
  int i,h;
  long x,y;
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  lbFontPtr = winfont;
  if (lbFontPtr != NULL)
    h = lbFontPtr[1].SHeight;
  else
    h = 0;
  x = 148;
  y = 28;
  for (i=0; i < game.active_messages_count; i++)
  {
      LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
      lbDisplay.DrawFlags &= 0xFFBFu;
      LbTextDraw((x+32)/pixel_size, y/pixel_size, game.messages[i].text);
      LbSpriteDraw(x/pixel_size, y/pixel_size, &gui_panel_sprites[488+game.messages[i].field_40]);
      y += pixel_size * h;
  }
}

void draw_lens(unsigned char *dstbuf, unsigned char *srcbuf, unsigned long *lens_mem, int width, int height, int scanln)
{
  _DK_draw_lens(dstbuf, srcbuf, lens_mem, width, height, scanln);
}

void flyeye_blitsec(unsigned char *srcbuf, unsigned char *dstbuf, long srcwidth, long dstwidth, long n, long height)
{
  _DK_flyeye_blitsec(srcbuf, dstbuf, srcwidth, dstwidth, n, height);
}

void draw_power_hand(void)
{
  static const char *func_name="draw_power_hand";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_draw_power_hand();
}

void draw_creature_view(struct Thing *thing)
{
  static const char *func_name="draw_creature_view";
  struct TbGraphicsWindow grwnd;
  struct PlayerInfo *player;
  long grscr_w,grscr_h;
  unsigned char *wscr_cp;
  unsigned char *scrmem;
  //_DK_draw_creature_view(thing); return;

  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if (((game.flags_cd & MFlg_EyeLensReady) == 0) || (eye_lens_memory == NULL))
  {
    engine((struct Camera *)&player->field_66);
    return;
  }
  if ((game.numfield_1B == 0) || (game.numfield_1B == 13) || (game.numfield_1B >= 14))
  {
    engine((struct Camera *)&player->field_66);
    return;
  }
  //TODO: Temporary hack, until CMistFade is not rewritten
  if ((game.numfield_1B >= 4) && (game.numfield_1B <= 12))
  {
    _DK_draw_creature_view(thing);
    return;
  }
  scrmem = eye_lens_spare_screen_memory;
  // Store previous graphics settings
  wscr_cp = lbDisplay.WScreen;
  grscr_w = lbDisplay.GraphicsScreenWidth;
  grscr_h = lbDisplay.GraphicsScreenHeight;
  LbScreenStoreGraphicsWindow(&grwnd);
  // Prepare new settings
  LbMemorySet(scrmem, 0, eye_lens_width*eye_lens_height*sizeof(TbPixel));
  lbDisplay.WScreen = scrmem;
  lbDisplay.GraphicsScreenHeight = eye_lens_height;
  lbDisplay.GraphicsScreenWidth = eye_lens_width;
  LbScreenSetGraphicsWindow(0, 0, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  // Draw on our buffer
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  engine((struct Camera *)&player->field_66);
  // Restore original graphics settings
  lbDisplay.WScreen = wscr_cp;
  lbDisplay.GraphicsScreenWidth = grscr_w;
  lbDisplay.GraphicsScreenHeight = grscr_h;
  LbScreenLoadGraphicsWindow(&grwnd);
  // Draw the buffer on real screen
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  switch (game.numfield_1B)
  {
  case 1:
  case 2:
      draw_lens(lbDisplay.WScreen, scrmem, eye_lens_memory,
            MyScreenWidth/pixel_size, MyScreenHeight/pixel_size, lbDisplay.GraphicsScreenWidth);
      break;
  case 3:
      flyeye_blitsec(scrmem, lbDisplay.WScreen, MyScreenWidth/pixel_size,
            lbDisplay.GraphicsScreenWidth, 1, MyScreenHeight/pixel_size);
      break;
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
/*
      Mist->mist(lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, scrmem,
          MyScreenWidth/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
      Mist->animate();
*/
      break;
  case 12:
/*
      Mist->mist(lbDisplay.WScreen, lbDisplay.GraphicsScreenWidth, scrmem,
          MyScreenWidth/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
      Mist->animate();
*/
      break;
  default:
      error(func_name, 768, "Invalid lens mode");
      break;
  }
}

void draw_swipe(void)
{
  _DK_draw_swipe();
}

void do_map_rotate_stuff(long a1, long a2, long *a3, long *a4, long a5)
{
  _DK_do_map_rotate_stuff(a1, a2, a3, a4, a5);
}

short mouse_is_over_small_map(long x, long y)
{
  long cmx,cmy;
  long px,py;
  long n;
  cmx = GetMouseX();
  cmy = GetMouseY();
  px = (cmx-(x+SMALL_MAP_RADIUS));
  py = (cmy-(y+SMALL_MAP_RADIUS));
  return (LbSqrL(px*px + py*py) < SMALL_MAP_RADIUS);
}

void draw_whole_status_panel(void)
{
  struct Dungeon *dungeon;
  struct PlayerInfo *player;
  long i;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
  lbDisplay.DrawColour = colours[15][15][15];
  lbDisplay.DrawFlags = 0;
  DrawBigSprite(0, 0, &status_panel, gui_panel_sprites);
  draw_gold_total(player->field_2B, 60, 134, dungeon->field_AF9);
  if (pixel_size < 3)
      i = (player->minimap_zoom) / (3-pixel_size);
  else
      i = player->minimap_zoom;
  pannel_map_draw(player->mouse_x, player->mouse_y, i);
  draw_overlay_things(i);
}

void redraw_creature_view(void)
{
  static const char *func_name="redraw_creature_view";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  struct TbGraphicsWindow ewnd;
  struct PlayerInfo *player;
  struct Thing *thing;
  long i;
  //_DK_redraw_creature_view(); return;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  if (player->field_45F != 2)
    player->field_45F = 2;
  update_explored_flags_for_power_sight(player);
  thing = NULL;
  i = player->field_2F;
  if ((i>0) && (i<THINGS_COUNT))
    thing = game.things_lookup[i];
  draw_creature_view(thing);
  if (smooth_on)
  {
    store_engine_window(&ewnd,pixel_size);
    smooth_screen_area(lbDisplay.WScreen, ewnd.x, ewnd.y,
        ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
  }
  remove_explored_flags_for_power_sight(player);
  draw_swipe();
  if (game.numfield_C & 0x20)
    draw_whole_status_panel();
  draw_gui();
  if (game.numfield_C & 0x20)
    draw_overlay_compass(player->mouse_x, player->mouse_y);
  message_draw();
  gui_draw_all_boxes();
  draw_tooltip();
}

void smooth_screen_area(unsigned char *scrbuf, long x, long y, long w, long h, long scanln)
{
  static const char *func_name="smooth_screen_area";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  long i,k;
  unsigned char *buf;
  unsigned char *lnbuf;
  unsigned int ghpos;
  lnbuf = scrbuf + scanln*y + x;
  for (i = h-y-1; i>0; i--)
  {
    buf = lnbuf;
    for (k = w-x-1; k>0; k--)
    {
        ghpos = (buf[0] << 8) + buf[1];
        ghpos = (buf[scanln] << 8) + ghost[ghpos];
        buf[0] = ghpos;
        buf++;
    }
    lnbuf += scanln;
  }
}

void make_camera_deviations(struct PlayerInfo *player,struct Dungeon *dungeon)
{
  static const char *func_name="make_camera_deviations";
  long x,y;
  x = player->camera->mappos.x.val;
  y = player->camera->mappos.y.val;
  if (dungeon->field_EA0)
  {
    x += seed_check_random(80, &game.field_14BB4E, func_name, 8653) - 40;
    y += seed_check_random(80, &game.field_14BB4E, func_name, 8654) - 40;
  }
  if (dungeon->field_EA4)
  {
    x += ( (dungeon->field_EA4 * lbSinTable[player->camera->orient_a] >> 8) >> 8);
    y += (-(dungeon->field_EA4 * lbCosTable[player->camera->orient_a] >> 8) >> 8);
  }
  if ((dungeon->field_EA0) || (dungeon->field_EA4))
  {
    // bounding position
    if (x < 0)
    {
      x = 0;
    } else
    if (x > 65535)
    {
      x = 65535;
    }
    if (y < 0)
    {
      y = 0;
    } else
    if (y > 65535)
    {
      y = 65535;
    }
    // setting deviated position
    player->camera->mappos.x.val = x;
    player->camera->mappos.y.val = y;
  }
}

void redraw_isometric_view(void)
{
  static const char *func_name="redraw_isometric_view";
  struct PlayerInfo *player;
  struct Dungeon *dungeon;
  struct TbGraphicsWindow ewnd;
  struct Coord3d pos;
  int i;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_redraw_isometric_view(); return;

  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  memcpy(&pos,&player->camera->mappos,sizeof(struct Coord3d));
  if (player->field_45F != 1)
    player->field_45F = 1;
  dungeon = &(game.dungeon[my_player_number%DUNGEONS_COUNT]);
  // Camera position modifications
  make_camera_deviations(player,dungeon);
  update_explored_flags_for_power_sight(player);
  if (game.flags_font & 0x08)
  {
    store_engine_window(&ewnd,1);
    setup_engine_window(ewnd.x, ewnd.y, ewnd.width >> 1, ewnd.height >> 1);
  }
  engine((struct Camera *)&player->cam_mappos);
  if (game.flags_font & 0x08)
  {
    load_engine_window(&ewnd);
  }
  if (smooth_on)
  {
    store_engine_window(&ewnd,pixel_size);
    smooth_screen_area(lbDisplay.WScreen, ewnd.x, ewnd.y,
        ewnd.width, ewnd.height, lbDisplay.GraphicsScreenWidth);
  }
  remove_explored_flags_for_power_sight(player);
  if (game.numfield_C & 0x20)
    draw_whole_status_panel();
  draw_gui();
  if ( game.numfield_C & 0x20)
    draw_overlay_compass(player->mouse_x, player->mouse_y);
  message_draw();
  draw_power_hand();
  draw_tooltip();
  memcpy(&player->camera->mappos,&pos,sizeof(struct Coord3d));
#if (BFDEBUG_LEVEL > 8)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void redraw_frontview(void)
{
  static const char *func_name="redraw_frontview";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_redraw_frontview();
}

void draw_texture(long a1, long a2, long a3, long a4, long a5, long a6, long a7)
{
  _DK_draw_texture(a1, a2, a3, a4, a5, a6, a7);
}

void draw_status_sprites(long a1, long a2, struct Thing *thing, long a4)
{
  _DK_draw_status_sprites(a1, a2, thing, a4);
}

long element_top_face_texture(struct Map *map)
{
  return _DK_element_top_face_texture(map);
}

long thing_is_spellbook(struct Thing *thing)
{
  return _DK_thing_is_spellbook(thing);
}

int LbSpriteDrawOneColour(long x, long y, struct TbSprite *spr, TbPixel colour)
{
  return _DK_LbSpriteDrawOneColour(x, y, spr, colour);
}

long object_is_gold(struct Thing *thing)
{
  return _DK_object_is_gold(thing);
}

void draw_zoom_box_things_on_mapblk(struct Map *mapblk,unsigned short subtile_size,int scr_x,int scr_y)
{
  static const char *func_name="draw_zoom_box_things_on_mapblk";
  struct PlayerInfo *player;
  struct TbSprite *spr;
  struct Thing *thing;
  int spos_x,spos_y;
  int tpos_x,tpos_y;
  TbPixel color;
  long spridx;
  unsigned long k;
  long i;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  i = ((mapblk->field_1 & 0x3FF800u) >> 11);
  while (i > 0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->field_2;
    if (((thing->field_0 & 0x10) == 0) && ((thing->field_1 & 0x02) == 0))
    {
      spos_x = ((subtile_size * thing->mappos.x.stl.pos) >> 8);
      spos_y = ((subtile_size * thing->mappos.y.stl.pos) >> 8);
      switch (thing->class_id)
      {
      case 5:
        spridx = creature_graphics[thing->model][20];
        if ((spridx <= 0) || (spridx > GUI_PANEL_SPRITES_COUNT))
          break;
        spr = &gui_panel_sprites[spridx];
        tpos_y = scr_y - pixel_size * spr->SHeight / 2;
        tpos_x = scr_x - pixel_size * spr->SWidth / 2;
        if (game.seedchk_random_used & 0x04)
        {
          color = get_player_path_colour(thing->owner);
          LbSpriteDrawOneColour((tpos_x+spos_x)/pixel_size, (tpos_y+spos_y)/pixel_size, spr, color);
        } else
        {
          LbSpriteDraw((tpos_x+spos_x)/pixel_size, (tpos_y+spos_y)/pixel_size, spr);
        }
        draw_status_sprites((spos_x+scr_x)/pixel_size - 10, (spos_y+scr_y-20)/pixel_size, thing, 4096);
        break;
      case 8:
        if ((!thing->byte_17.h) && (player->field_2B != thing->owner))
          break;
        spridx = trap_data[thing->model].field_A;
        if ((spridx <= 0) || (spridx > GUI_PANEL_SPRITES_COUNT))
          break;
        spr = &gui_panel_sprites[spridx];
        tpos_y = scr_y - pixel_size * spr->SHeight / 2;
        tpos_x = scr_x - pixel_size * spr->SWidth / 2;
        LbSpriteDraw((tpos_x+spos_x)/pixel_size, (tpos_y+spos_y)/pixel_size, spr);
        break;
      case 1:
        if (thing->model == 5)
        {
          spridx = 512;
          spr = &gui_panel_sprites[spridx];
          tpos_y = scr_y - pixel_size * spr->SHeight / 2;
          tpos_x = scr_x - pixel_size * spr->SWidth / 2;
          LbSpriteDraw((tpos_x+spos_x)/pixel_size, (tpos_y+spos_y)/pixel_size, spr);
        } else
        if (object_is_gold(thing))
        {
          spridx = 511;
          spr = &gui_panel_sprites[spridx];
          tpos_y = scr_y - pixel_size * spr->SHeight / 2;
          tpos_x = scr_x - pixel_size * spr->SWidth / 2;
          LbSpriteDraw((tpos_x+spos_x)/pixel_size, (tpos_y+spos_y)/pixel_size, spr);
        }
        break;
      default:
        if ( thing_is_special(thing) )
        {
          spridx = 164;
          spr = &gui_panel_sprites[spridx];
          tpos_y = scr_y - pixel_size * spr->SHeight / 2;
          tpos_x = scr_x - pixel_size * spr->SWidth / 2;
          LbSpriteDraw((tpos_x+spos_x)/pixel_size, (tpos_y+spos_y)/pixel_size, spr);
        } else
        if ( thing_is_spellbook(thing) )
        {
          spridx = spell_data[object_to_magic[thing->model]].field_B;
          if ((spridx <= 0) || (spridx > GUI_PANEL_SPRITES_COUNT))
            break;
          spr = &gui_panel_sprites[spridx];
          tpos_y = scr_y - pixel_size * spr->SHeight / 2;
          tpos_x = scr_x - pixel_size * spr->SWidth / 2;
          LbSpriteDraw((tpos_x+spos_x)/pixel_size, (tpos_y+spos_y)/pixel_size, spr);
        }
        break;
      }
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}


/*
 * Draws a box near mouse with more detailed top view of map.
 * Requires screen to be locked before.
 */
void draw_zoom_box(void)
{
  //_DK_draw_zoom_box(); return;
  struct PlayerInfo *player;
  struct Map *mapblk;
  const int subtile_size = 8;
  long map_tiles_x,map_tiles_y;
  long scrtop_x,scrtop_y;
  int map_dx,map_dy;
  int scr_x,scr_y;
  int map_x,map_y;
  int i,k;

  map_tiles_x = 13;
  map_tiles_y = 13;

  lbDisplay.DrawFlags = 0;
  scrtop_x = GetMouseX() + 24;
  scrtop_y = GetMouseY() + 24;
  map_x = (3*GetMouseX()-450) / 4 - 6;
  map_y = (3*GetMouseY()-168) / 4 - 6;

  // Draw only on map area
  if ((map_x < -map_tiles_x+4) || (map_x >= map_subtiles_x+1-map_tiles_x+6)
   || (map_y < -map_tiles_y+4) || (map_y >= map_subtiles_x+1-map_tiles_y+6))
    return;

  scrtop_x += 4;
  scrtop_y -= 4;
  setup_vecs(lbDisplay.WScreen, 0, lbDisplay.GraphicsScreenWidth, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  if (scrtop_y > MyScreenHeight-map_tiles_y*subtile_size)
    scrtop_y = MyScreenHeight-map_tiles_y*subtile_size;
  if (scrtop_y < 0)
      scrtop_y = 0;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);

  i = (map_y << 8) + map_x;
  scr_y = scrtop_y;
  for (map_dy=0; map_dy < map_tiles_y; map_dy++)
  {
    mapblk = &game.map[i];
    scr_x = scrtop_x;
    for (map_dx=0; map_dx < map_tiles_x; map_dx++)
    {
      if ((map_x+map_dx >= 0) && (map_x+map_dx < map_subtiles_x)
       && (map_y+map_dy >= 0) && (map_y+map_dy < map_subtiles_y))
      {
        if ((mapblk->field_1 >> 28) & (1 << player->field_2B))
        {
          k = element_top_face_texture(mapblk);
          draw_texture(scr_x, scr_y, subtile_size, subtile_size, k, 0, -1);
        } else
        {
          LbDrawBox(scr_x/pixel_size, scr_y/pixel_size, 8/pixel_size, 8/pixel_size, 1);
        }
      } else
      {
        LbDrawBox(scr_x/pixel_size, scr_y/pixel_size, 8/pixel_size, 8/pixel_size, 1);
      }
      scr_x += subtile_size;
      mapblk++;
    }
    scr_y += subtile_size;
    i += (1 << 8);
  }
  lbDisplay.DrawFlags |= 0x0010;
  LbDrawBox(scrtop_x/pixel_size, scrtop_y/pixel_size,
      (map_tiles_x*subtile_size)/pixel_size, (map_tiles_y*subtile_size)/pixel_size, 0);
  lbDisplay.DrawFlags &= 0xFFEFu;
  LbScreenSetGraphicsWindow( (scrtop_x+2)/pixel_size, (scrtop_y+2)/pixel_size,
      (map_tiles_y*subtile_size-4)/pixel_size, (map_tiles_y*subtile_size-4)/pixel_size);
  i = (map_y << 8) + map_x;
  scr_y = 0;
  for (map_dy=0; map_dy < map_tiles_y; map_dy++)
  {
    mapblk = &game.map[i];
    scr_x = 0;
    for (map_dx=0; map_dx < map_tiles_x; map_dx++)
    {
      if ((map_x+map_dx >= 0) && (map_x+map_dx < map_subtiles_x)
       && (map_y+map_dy >= 0) && (map_y+map_dy < map_subtiles_y))
      {
        if ((mapblk->field_1 >> 28) & (1 << player->field_2B))
        {
          draw_zoom_box_things_on_mapblk(mapblk,subtile_size,scr_x,scr_y);
        }
      }
      scr_x += subtile_size;
      mapblk++;
    }
    scr_y += subtile_size;
    i += (1 << 8);
  }
  LbScreenSetGraphicsWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  LbSpriteDraw((scrtop_x-24)/pixel_size, (scrtop_y-20)/pixel_size, &button_sprite[194]);
  LbSpriteDraw((scrtop_x+54)/pixel_size, (scrtop_y-20)/pixel_size, &button_sprite[195]);
  LbSpriteDraw((scrtop_x-24)/pixel_size, (scrtop_y+50)/pixel_size, &button_sprite[196]);
  LbSpriteDraw((scrtop_x+54)/pixel_size, (scrtop_y+50)/pixel_size, &button_sprite[197]);
  LbScreenSetGraphicsWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
}

void update_explored_flags_for_power_sight(struct PlayerInfo *player)
{
  _DK_update_explored_flags_for_power_sight(player);
}

void rotpers_parallel_3(struct EngineCoord *epos, struct M33 *matx)
{
  _DK_rotpers_parallel_3(epos, matx);
}

void rotate_base_axis(struct M33 *matx, short a2, unsigned char a3)
{
  _DK_rotate_base_axis(matx, a2, a3);
}

void fill_in_points_perspective(long a1, long a2, struct MinMax *mm)
{
  _DK_fill_in_points_perspective(a1, a2, mm);
}

void fill_in_points_cluedo(long a1, long a2, struct MinMax *mm)
{
  _DK_fill_in_points_cluedo(a1, a2, mm);
}

void fill_in_points_isometric(long a1, long a2, struct MinMax *mm)
{
  _DK_fill_in_points_isometric(a1, a2, mm);
}

void display_drawlist(void)
{
  static const char *func_name="display_drawlist";
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_display_drawlist();
}

void frame_wibble_generate(void)
{
  _DK_frame_wibble_generate();
}

void find_gamut(void)
{
  _DK_find_gamut();
}

void fiddle_gamut(long a1, long a2)
{
  _DK_fiddle_gamut(a1, a2);
}

void create_map_volume_box(long a1, long a2, long a3)
{
  _DK_create_map_volume_box(a1, a2, a3);
}

void setup_rotate_stuff(long a1, long a2, long a3, long a4, long a5, long a6, long a7, long a8)
{
  _DK_setup_rotate_stuff(a1, a2, a3, a4, a5, a6, a7, a8);
}

void do_a_plane_of_engine_columns_perspective(long a1, long a2, long a3, long a4)
{
  _DK_do_a_plane_of_engine_columns_perspective(a1, a2, a3, a4);
}

void do_a_plane_of_engine_columns_cluedo(long a1, long a2, long a3, long a4)
{
  _DK_do_a_plane_of_engine_columns_cluedo(a1, a2, a3, a4);
}

void do_a_plane_of_engine_columns_isometric(long a1, long a2, long a3, long a4)
{
  _DK_do_a_plane_of_engine_columns_isometric(a1, a2, a3, a4);
}

long compute_cells_away(void)
{
  long xmin,ymin,xmax,ymax;
  long xcell,ycell;
  struct PlayerInfo *player;
  long ncells_a;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
    if ((vert_offset[1]) || (hori_offset[1]))
    {
      xcell = 660/pixel_size - player->engine_window_x/pixel_size - x_init_off;
      ycell = (8 * high_offset[1] >> 8) - 20/pixel_size - player->engine_window_y/pixel_size - y_init_off;
      ymax = (((vert_offset[1] * xcell) >> 1) - ((vert_offset[0] * ycell) >> 1))
         / ((hori_offset[0] * vert_offset[1] - vert_offset[0] * hori_offset[1]) >> 11) >> 2;
      xmax = (((hori_offset[1] * xcell) >> 1) - ((hori_offset[0] * ycell) >> 1))
         / ((vert_offset[0] * hori_offset[1] - hori_offset[0] * vert_offset[1]) >> 11) >> 2;
    } else
    {
      ymax = 0;
      xmax = 0;
    }
    if ((vert_offset[1]) || (hori_offset[1]))
    {
      xcell = 320 / pixel_size - player->engine_window_x/pixel_size - x_init_off;
      ycell = 200 / pixel_size - ymin - y_init_off;
      ymin = (((vert_offset[1] * xcell) >> 1) - ((vert_offset[0] * ycell) >> 1))
          / ((hori_offset[0] * vert_offset[1] - vert_offset[0] * hori_offset[1]) >> 11) >> 2;
      xmin = (((hori_offset[1] * xcell) >> 1) - ((hori_offset[0] * ycell) >> 1))
         / ((vert_offset[0] * hori_offset[1] - hori_offset[0] * vert_offset[1]) >> 11) >> 2;
    } else
    {
      ymin = 0;
      xmin = 0;
    }
    xcell = abs(ymax - ymin);
    ycell = abs(xmax - xmin);
    if (ycell >= xcell)
      ncells_a = ycell + (xcell >> 1);
    else
      ncells_a = xcell + (ycell >> 1);
    ncells_a += 2;
    if (ncells_a > max_i_can_see)
      ncells_a = max_i_can_see;
    return ncells_a;
}

void init_coords_and_rotation(struct EngineCoord *origin,struct M33 *matx)
{
  origin->x = 0;
  origin->y = 0;
  origin->z = 0;
  matx->r0[0] = 0x4000u;
  matx->r0[1] = 0;
  matx->r0[2] = 0;
  matx->r1[0] = 0;
  matx->r1[1] = 0x4000u;
  matx->r1[2] = 0;
  matx->r2[0] = 0;
  matx->r2[1] = 0;
  matx->r2[2] = 0x4000u;
}

void update_fade_limits(long ncells_a)
{
  fade_max = (ncells_a << 8);
  fade_scaler = (ncells_a << 8);
  fade_way_out = (ncells_a + 1) << 8;
  fade_min = 768 * ncells_a / 4;
  split_1 = (split1at << 8);
  split_2 = (split2at << 8);
}

void update_normal_shade(struct M33 *matx)
{
  normal_shade_left = matx->r2[0];
  normal_shade_right = -matx->r2[0];
  normal_shade_back = -matx->r2[2];
  normal_shade_front = matx->r2[2];
  if (normal_shade_front < 0)
    normal_shade_front = 0;
  if (normal_shade_back < 0)
    normal_shade_back = 0;
  if (normal_shade_left < 0)
    normal_shade_left = 0;
  if (normal_shade_right < 0)
    normal_shade_right = 0;
}

void update_engine_settings(struct PlayerInfo *player)
{
  engine_player_number = player->field_2B;
  player_bit = (1 << engine_player_number);
  switch (settings.field_0)
  {
  case 0:
      split1at = 4;
      split2at = 3;
      break;
  case 1:
      split1at = 3;
      split2at = 2;
      break;
  case 2:
      split1at = 2;
      split2at = 1;
      break;
  case 3:
      split1at = 0;
      split2at = 0;
      break;
  }
  me_pointed_at = NULL;
  me_distance = 100000000;
  max_i_can_see = i_can_see_levels[settings.view_distance % 4];
  if (lens_mode != 0)
    temp_cluedo_mode = 0;
  else
    temp_cluedo_mode = settings.video_cluedo_mode;
  thing_pointed_at = NULL;
}

void do_perspective_rotation(long x, long y, long z)
{
  struct PlayerInfo *player;
  struct EngineCoord epos;

  player = &(game.players[my_player_number%PLAYERS_COUNT]);
    epos.x = -x;
    epos.y = 0;
    epos.z = y;
    rotpers_parallel_3(&epos, &camera_matrix);
    x_init_off = epos.field_0;
    y_init_off = epos.field_4;
    depth_init_off = epos.z;
    epos.x = 65536;
    epos.y = 0;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix);
    hori_offset[0] = epos.field_0 - ((player->engine_window_width/pixel_size) >> 1);
    hori_offset[1] = epos.field_4 - ((player->engine_window_height/pixel_size) >> 1);
    hori_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 0;
    epos.z = -65536;
    rotpers_parallel_3(&epos, &camera_matrix);
    vert_offset[0] = epos.field_0 - ((player->engine_window_width/pixel_size) >> 1);
    vert_offset[1] = epos.field_4 - ((player->engine_window_height/pixel_size) >> 1);
    vert_offset[2] = epos.z;
    epos.x = 0;
    epos.y = 65536;
    epos.z = 0;
    rotpers_parallel_3(&epos, &camera_matrix);
    high_offset[0] = epos.field_0 - ((player->engine_window_width/pixel_size) >> 1);
    high_offset[1] = epos.field_4 - ((player->engine_window_height/pixel_size) >> 1);
    high_offset[2] = epos.z;
}

void update_block_pointed(int i,long x, long x_frac, long y, long y_frac)
{
  struct Map *map;
  short visible;
  unsigned int mask;
  long k;

  if (i > 0)
  {
    map = &game.map[((map_subtiles_x+1)*y)+x];
    visible = (player_bit & (map->field_1 >> 28) != 0);
    if ((!visible) || ((map->field_1 & 0x7FF) > 0))
    {
      if (visible)
        k = map->field_1 & 0x7FF;
      else
        k = game.field_149E77;
      mask = game.columns[k].solidmask;
      if ((temp_cluedo_mode) && (mask != 0))
      {
        if (visible)
          k = map->field_1 & 0x7FF;
        else
          k = game.field_149E77;
        if (game.columns[k].solidmask >= 8)
        {
          if ((!visible) || !(game.field_CC157[(((map_subtiles_x+1)*y)+x)] & 0x80) && !(map->field_0 & 0x02))
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
      map = &game.map[((map_subtiles_x+1)*y)+x];
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
  long k;
  int i;
    if ((!vert_offset[1]) && (!hori_offset[1]))
    {
      block_pointed_at_x = 0;
      block_pointed_at_y = 0;
      me_pointed_at = &game.map[0];
    } else
    {
      hori_ptr_y = hori_offset[0] * (pointer_y - y_init_off);
      vert_ptr_y = vert_offset[0] * (pointer_y - y_init_off);
      hori_hdelta_y = hori_offset[0] * (high_offset[1] >> 8);
      vert_hdelta_y = vert_offset[0] * (high_offset[1] >> 8);
      vert_ptr_x = (signed long)(vert_offset[1] * (pointer_x - x_init_off)) >> 1;
      hori_ptr_x = (signed long)(hori_offset[1] * (pointer_x - x_init_off)) >> 1;
      hvdiv_x = (signed long)(hori_offset[0] * vert_offset[1] - vert_offset[0] * hori_offset[1]) >> 11;
      hvdiv_y = (signed long)(vert_offset[0] * hori_offset[1] - hori_offset[0] * vert_offset[1]) >> 11;
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
}

void draw_view(struct Camera *cam, unsigned char a2)
{
  static const char *func_name="draw_view";
  long nlens;
  long x,y,z;
  long xcell,ycell;
  long i;
  long aposc,bposc;
  struct EngineCol *ec;
  struct MinMax *mm;
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_draw_view(cam, a2); retutn;
  nlens = cam->field_17 / pixel_size;
  getpoly = poly_pool;
  memset(buckets, 0, 0xB00u);
  perspective = perspective_routines[lens_mode%PERS_ROUTINES_COUNT];
  rotpers = rotpers_routines[lens_mode%PERS_ROUTINES_COUNT];
  update_fade_limits(cells_away);
  init_coords_and_rotation(&object_origin,&camera_matrix);
  rotate_base_axis(&camera_matrix, cam->orient_a, 2);
  update_normal_shade(&camera_matrix);
  rotate_base_axis(&camera_matrix, -cam->orient_b, 1);
  rotate_base_axis(&camera_matrix, -cam->orient_c, 3);
  map_angle = cam->orient_a;
  map_roll = cam->orient_c;
  map_tilt = -cam->orient_b;
  x = cam->mappos.x.val;
  y = cam->mappos.y.val;
  z = cam->mappos.z.val;
  frame_wibble_generate();
  view_alt = z;
  if (lens_mode)
  {
    cells_away = max_i_can_see;
    update_fade_limits(cells_away);
    fade_range = (fade_max - fade_min) >> 8;
    setup_rotate_stuff(x, y, z, fade_max, fade_min, lens, map_angle, map_roll);
  } else
  {
    fade_min = 1000000;
    setup_rotate_stuff(x, y, z, fade_max, fade_min, nlens, map_angle, map_roll);
    do_perspective_rotation(x, y, z);
    cells_away = compute_cells_away();
  }
  xcell = (x >> 8);
  aposc = -(unsigned char)x;
  bposc = (cells_away << 8) + (y & 0xFF);
  ycell = (y >> 8) - (cells_away+1);
  find_gamut();
  fiddle_gamut(xcell, ycell + (cells_away+1));
  apos = aposc;
  bpos = bposc;
  back_ec = &ecs1[0];
  front_ec = &ecs2[0];
  mm = &minmaxs[31-cells_away];
  if (lens_mode)
  {
    fill_in_points_perspective(xcell, ycell, mm);
  } else
  {
    if (settings.video_cluedo_mode)
      fill_in_points_cluedo(xcell, ycell, mm);
    else
      fill_in_points_isometric(xcell, ycell, mm);
  }
  for (i = 2*cells_away-1; i > 0; i--)
  {
      ycell++;
      bposc -= 256;
      mm++;
      ec = front_ec;
      front_ec = back_ec;
      back_ec = ec;
      apos = aposc;
      bpos = bposc;
      if (lens_mode)
      {
        fill_in_points_perspective(xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_perspective(xcell, ycell, mm->min, mm->max);
        }
      } else
      if ( settings.video_cluedo_mode )
      {
        fill_in_points_cluedo(xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_cluedo(xcell, ycell, mm->min, mm->max);
        }
      } else
      {
        fill_in_points_isometric(xcell, ycell, mm);
        if (mm->min < mm->max)
        {
          apos = aposc;
          bpos = bposc;
          do_a_plane_of_engine_columns_isometric(xcell, ycell, mm->min, mm->max);
        }
      }
  }
  if (map_volume_box.field_0)
    create_map_volume_box(x, y, z);
  display_drawlist();
  map_volume_box.field_0 = 0;
#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void engine(struct Camera *cam)
{
  static const char *func_name="engine";
  struct TbGraphicsWindow grwnd;
  struct TbGraphicsWindow ewnd;
  unsigned short flg_mem;
  struct PlayerInfo *player;

#if (BFDEBUG_LEVEL > 9)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_engine(cam); return;
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  flg_mem = lbDisplay.DrawFlags;
  update_engine_settings(player);
  mx = cam->mappos.x.val;
  my = cam->mappos.y.val;
  mz = cam->mappos.z.val;
  pointer_x = (GetMouseX() - player->engine_window_x) / pixel_size;
  pointer_y = (GetMouseY() - player->engine_window_y) / pixel_size;
  lens = cam->field_13 * MyScreenWidth/pixel_size / 320;
  if (lens_mode == 0)
    update_blocks_pointed();
  LbScreenStoreGraphicsWindow(&grwnd);
  store_engine_window(&ewnd,pixel_size);
  view_height_over_2 = ewnd.height/2;
  view_width_over_2 = ewnd.width/2;
  LbScreenSetGraphicsWindow(ewnd.x, ewnd.y, ewnd.width, ewnd.height);
  setup_vecs(lbDisplay.GraphicsWindowPtr, 0, lbDisplay.GraphicsScreenWidth,
      ewnd.width, ewnd.height);
  draw_view(cam, 0);
  lbDisplay.DrawFlags = flg_mem;
  thing_being_displayed = 0;
  LbScreenLoadGraphicsWindow(&grwnd);
}

void remove_explored_flags_for_power_sight(struct PlayerInfo *player)
{
  _DK_remove_explored_flags_for_power_sight(player);
}

void DrawBigSprite(long x, long y, struct BigSprite *bigspr, struct TbSprite *sprite)
{
  _DK_DrawBigSprite(x, y, bigspr, sprite);
}

void draw_gold_total(unsigned char a1, long a2, long a3, long a4)
{
  _DK_draw_gold_total(a1, a2, a3, a4);
}

void pannel_map_draw(long x, long y, long zoom)
{
  _DK_pannel_map_draw(x, y, zoom);
}

void draw_overlay_things(long zoom)
{
  static const char *func_name="draw_overlay_things";
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  _DK_draw_overlay_things(zoom);
}

void draw_overlay_compass(long a1, long a2)
{
  _DK_draw_overlay_compass(a1, a2);
}

void draw_map_level_name(void)
{
  char *lv_name;
  int x,y,w,h;
  // Retrieving name
  lv_name = NULL;
  if (is_original_singleplayer_level(game.level_file_number))
  {
    lv_name = strings[(game.level_file_number+201)%STRINGS_MAX];
  } else
  if ( is_bonus_level(game.level_file_number) )
  {
    lv_name = strings[430];
  } else
  if (is_multiplayer_level(game.level_file_number))
  {
    lv_name = level_name;
  }
  // Retrieving position
  x = 0;
  y = 0;
  w = 640;//MyScreenWidth;
  h = MyScreenHeight;
  // Drawing
  if (lv_name != NULL)
  {
    lbFontPtr = winfont;
    lbDisplay.DrawFlags = 0;
    LbTextSetWindow(x/pixel_size, y/pixel_size, w/pixel_size, h/pixel_size);
    LbTextDraw((w-pixel_size*LbTextStringWidth(lv_name))/2 / pixel_size, 32 / pixel_size, lv_name);
  }
}

void load_parchment_file(void)
{
  if ( !parchment_loaded )
  {
    reload_parchment_file(lbDisplay.PhysicalScreenWidth >= 640);
  }
}

void reload_parchment_file(short hires)
{
  char *fname;
  if (hires)
  {
    fname=prepare_file_path(FGrp_StdData,"gmaphi.raw");
    LbFileLoadAt(fname, hires_parchment);
  } else
  {
    fname=prepare_file_path(FGrp_StdData,"gmap.raw");
    LbFileLoadAt(fname, poly_pool);
  }
  parchment_loaded = 1;
}

void redraw_parchment_view(void)
{
  static const char *func_name="redraw_parchment_view";
  char *fname;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  load_parchment_file();
  draw_map_parchment();
  draw_2d_map();
  draw_gui();
  gui_draw_all_boxes();
  draw_zoom_box();
  draw_map_level_name();
  draw_tooltip();
}

void redraw_display(void)
{
  static const char *func_name="redraw_display";
  //_DK_redraw_display();return;
  char *text;
  struct PlayerInfo *player;
  int i;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);

  player->field_6 &= 0xFEu;
  if (game.flagfield_14EA4A == 1)
    return;

  if (game.small_map_state == 2)
    LbMouseChangeSpriteAndHotspot(NULL, 0, 0);
  else
    process_pointer_graphic();
  switch (player->field_37)
  {
  case 1:
      redraw_creature_view();
      parchment_loaded = 0;
      break;
  case 2:
      redraw_isometric_view();
      parchment_loaded = 0;
      break;
  case 3:
      redraw_parchment_view();
      break;
  case 5:
      redraw_frontview();
      parchment_loaded = 0;
      break;
  case 6:
      parchment_loaded = 0;
      player->field_4BD = map_fade_in(player->field_4BD);
      break;
  case 7:
      parchment_loaded = 0;
      player->field_4BD = map_fade_out(player->field_4BD);
      break;
  default:
      break;
  }
  LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  lbFontPtr = winfont;
  lbDisplay.DrawFlags &= 0xFFBFu;
  LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
  if (player->field_0 & 0x04)
  {
      text=buf_sprintf( ">%s_", player->strfield_463);
      LbTextDraw(148/pixel_size, 8/pixel_size, text);
  }
  if ( draw_spell_cost )
  {
      long pos_x,pos_y;
      unsigned short drwflags_mem;
      drwflags_mem = lbDisplay.DrawFlags;
      LbTextSetWindow(0, 0, MyScreenWidth, MyScreenHeight);
      lbDisplay.DrawFlags = 0;
      lbFontPtr = winfont;
      text = buf_sprintf("%d", draw_spell_cost);
      pos_y = GetMouseY() - pixel_size*LbTextStringHeight(text)/2 - 2;
      pos_x = GetMouseX() - pixel_size*LbTextStringWidth(text)/2;
      LbTextDraw(pos_x/pixel_size, pos_y/pixel_size, text);
      lbDisplay.DrawFlags = drwflags_mem;
      draw_spell_cost = 0;
  }
  if (bonus_timer_enabled())
    draw_bonus_timer();
  if (((game.numfield_C & 0x01) != 0) && ((game.numfield_C & 0x80) == 0))
  {
        lbFontPtr = winfont;
        long pos_x,pos_y;
        long w,h;
        int i;
        i = 0;
        if (lbFontPtr != NULL)
          i = lbFontPtr[1].SWidth;
        w = pixel_size * (LbTextStringWidth(strings[320]) + 2*i);
        i = player->field_37;
        if ((i == 2) || (i == 5) || (i == 1))
          pos_x = player->engine_window_x + (MyScreenWidth-w-player->engine_window_x)/2;
        else
          pos_x = (MyScreenWidth-w)/2;
        pos_y=16;
        i = 0;
        if (lbFontPtr != NULL)
          i = lbFontPtr[1].SHeight;
        lbDisplay.DrawFlags = 0x0100;
        h = pixel_size*i + pixel_size*i/2;
        LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
        draw_slab64k(pos_x, pos_y, w, h);
        LbTextDraw(0/pixel_size, 0/pixel_size, strings[320]);
        LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  }
  if (game.field_150356 != 0)
  {
    long pos_x,pos_y;
    long w,h;
    int i;
    if (game.armageddon+game.field_150356 <= game.seedchk_random_used)
    {
      i = 0;
      if ( game.field_15035A - game.field_1517EC <= game.seedchk_random_used )
        i = game.field_15035A - game.seedchk_random_used;
    } else
    {
      i = game.seedchk_random_used - game.field_150356 - game.armageddon;
    }
    lbFontPtr = winfont;
    text=buf_sprintf(" %s %03d", strings[646], i/2);
    i = 0;
    if (lbFontPtr != NULL)
      i = lbFontPtr[1].SWidth;
    w = pixel_size*LbTextStringWidth(text) + 6*i;
    pos_x = MyScreenWidth - w - 16;
    pos_y = 16;
    i = 0;
    if (lbFontPtr != NULL)
      i = lbFontPtr[1].SHeight;
    lbDisplay.DrawFlags = 0x0100;
    h = pixel_size*i + pixel_size*i/2;
    LbTextSetWindow(pos_x/pixel_size, pos_y/pixel_size, w/pixel_size, h/pixel_size);
    draw_slab64k(pos_x, pos_y, w, h);
    LbTextDraw(0/pixel_size, 0/pixel_size, text);
    LbTextSetWindow(0/pixel_size, 0/pixel_size, MyScreenWidth/pixel_size, MyScreenHeight/pixel_size);
  }
  draw_sound_stuff();
#if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Finished\n",func_name);
#endif
}

void find_frame_rate(void)
{
  static TbClockMSec prev_time2=0;
  static TbClockMSec cntr_time2=0;
  unsigned long curr_time;
  curr_time = LbTimerClock();
  cntr_time2++;
  if ( curr_time-prev_time2 >= 1000 )
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

/*
 * Checks if the game screen needs redrawing.
 */
short display_should_be_updated_this_turn(void)
{
  if ((game.numfield_C & 0x01) != 0)
    return true;
  if ( (game.turns_fastforward==0) && (!game.numfield_149F38) )
  {
    find_frame_rate();
    if ( (game.timingvar1==0) || ((game.seedchk_random_used % game.timingvar1)==0))
      return true;
  } else
  if ( ((game.seedchk_random_used & 0x3F)==0) ||
       ((game.numfield_149F38) && ((game.seedchk_random_used & 7)==0)) )
  {
    packet_load_find_frame_rate(64);
    return true;
  }
  return false;
}

/*
 * Makes last updates to the video buffer, and swaps buffers to show
 * the new image.
 */
short keeper_screen_swap(void)
{
/*  // For resolution 640x480, move the graphics data 40 lines lower
  if ( lbDisplay.ScreenMode == Lb_SCREEN_MODE_640_480_8 )
    if ( LbScreenLock() == 1 )
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
  return 1;
}

/*
 * Waits until the next game turn. Delay is usually controlled by
 * num_fps variable.
 */
short keeper_wait_for_next_turn(void)
{
  if (game.numfield_D & 0x10)
  {
      // No idea when such situation occurs
      TbClockMSec sleep_end = last_loop_time + 1000;
      LbSleepUntil(sleep_end);
      last_loop_time = LbTimerClock();
      return 1;
  }
  if (game.timingvar1 == 0)
  {
      // Standard delaying system
      TbClockMSec sleep_end = last_loop_time + 1000/game.num_fps;
      LbSleepUntil(sleep_end);
      last_loop_time = LbTimerClock();
      return 1;
  }
  return 0;
}

/*
 * Redraws the game display buffer.
 */
short keeper_screen_redraw(void)
{
  static const char *func_name="keeper_screen_redraw";
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  player = &(game.players[my_player_number%PLAYERS_COUNT]);
  LbScreenClear(0);
  if ( LbScreenLock() == 1 )
  {
    setup_engine_window(player->engine_window_x, player->engine_window_y,
        player->engine_window_width, player->engine_window_height);
    redraw_display();
    LbScreenUnlock();
    return 1;
  }
  return 0;
}

/*
 * Draws the crucial warning messages on screen.
 * Requires the screen to be locked before.
 */
short draw_onscreen_direct_messages(void)
{
  static const char *func_name="draw_onscreen_direct_messages";
  char *text;
  unsigned int msg_pos;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  // Display in-game message for debug purposes
  if (onscreen_msg_turns > 0)
  {
      if ( LbScreenIsLocked() )
        LbTextDraw(260/pixel_size, 0/pixel_size, onscreen_msg_text);
      onscreen_msg_turns--;
  }
  msg_pos = 200;
  if (game.numfield_A & 0x02)
  {
      text=buf_sprintf("OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
      error(func_name, 413, text);
      if ( LbScreenIsLocked() )
        LbTextDraw(300/pixel_size, msg_pos/pixel_size, "OUT OF SYNC");
      msg_pos += 20;
  }
  if (game.numfield_A & 0x04)
  {
      text=buf_sprintf("SEED OUT OF SYNC (GameTurn %7d)", game.seedchk_random_used);
      error(func_name, 427, text);
      if ( LbScreenIsLocked() )
        LbTextDraw(300/pixel_size, msg_pos/pixel_size, "SEED OUT OF SYNC");
      msg_pos += 20;
  }
  return 1;
}

void process_sound_heap(void)
{
    static const char *func_name="process_sound_heap";
    _DK_process_sound_heap();
}

void keeper_gameplay_loop(void)
{
    static const char *func_name="keeper_gameplay_loop";
    short do_draw;
    struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    PaletteSetPlayerPalette(player, _DK_palette);
    if (game.numfield_C & 0x02)
      initialise_eye_lenses();
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Entering the gameplay loop for level %d\n",(int)game.level_number);
#endif

    //the main gameplay loop starts
    while ((!quit_game) && (!exit_keeper))
    {
      if ((game.flags_font & 0x10) != 0)
      {
        if (game.seedchk_random_used == 4)
          LbNetwork_ChangeExchangeTimeout(0);
      }

      // Check if we should redraw screen in this turn
      do_draw = display_should_be_updated_this_turn() || (!LbIsActive());

      update_mouse();
      input_eastegg();
      input();
      update();

      if ( do_draw )
        keeper_screen_redraw();

      do {
        if ( !LbWindowsControl() )
        {
          if ((game.numfield_A & 0x01) == 0)
          {
            exit_keeper = 1;
            break;
          }
          LbSyncLog("%s - Alex's point reached\n",func_name);
        }
        if ((game.numfield_A & 0x01) || (LbIsActive()))
          break;
      } while ((!exit_keeper) && (!quit_game));

      // Direct information/error messages
      if ( LbScreenLock() == 1 )
      {
        if ( do_draw )
          perform_any_screen_capturing();
        draw_onscreen_direct_messages();
        LbScreenUnlock();
      }

      // Music and sound control
      if ( !SoundDisabled )
        if ((game.turns_fastforward==0) && (!game.numfield_149F38))
        {
            MonitorStreamedSoundTrack();
            process_sound_heap();
        }

      // Move the graphics window to center of screen buffer and swap screen
      if ( do_draw )
        keeper_screen_swap();

      // Make delay if the machine is too fast
      if ((!game.packet_load_enable) || (game.turns_fastforward == 0))
        keeper_wait_for_next_turn();

      if ( game.numfield_149F42 == game.seedchk_random_used )
        exit_keeper = 1;
    } // end while
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Gameplay loop finished after %u turns\n",game.seedchk_random_used);
#endif
}

void initialise_load_game_slots(void)
{
  _DK_load_game_save_catalogue(save_game_catalogue);
  number_of_saved_games = 0;
  int entry_idx;
  for (entry_idx=0;entry_idx<8;entry_idx++)
  {
    if ( save_game_catalogue[entry_idx].used )
      number_of_saved_games++;
  }
}

short continue_game_available()
{
  static const char *func_name="continue_game_available";
  unsigned char buf[12];
  char *fname;
  static short continue_needs_checking_file = 1;
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  wait_for_cd_to_be_available();
  fname=prepare_file_path(FGrp_Save,"continue.sav");
  if (LbFileLength(fname) != sizeof(struct Game))
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: No correct .SAV file; there's no continue\n",func_name);
  #endif
    return false;
  }
  if ( continue_needs_checking_file )
  {
    TbFileHandle fh=LbFileOpen(fname,Lb_FILE_MODE_READ_ONLY);
    if ( fh != -1 )
    {
      LbFileRead(fh, buf, 10);
      LbFileClose(fh);
      if (((struct Game *)buf)->continue_level > 0)
        game.level_number = ((struct Game *)buf)->continue_level;
    }
    continue_needs_checking_file = 0;
  }
  if (is_singleplayer_level(game.level_number))
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: Returning that continue is available\n",func_name);
  #endif
    return true;
  } else
  {
  #if (BFDEBUG_LEVEL > 7)
    LbSyncLog("%s: The level in .SAV isn't correct continue level\n",func_name);
  #endif
    return false;
  }
}

void intro(void)
{
    char *fname;
    fname=prepare_file_path(FGrp_LoData,"intromix.smk");
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Playing intro movie \"%s\"\n",fname);
#endif
    play_smacker_file(fname, 1);
}

void outro(void)
{
    char *fname;
    fname=prepare_file_path(FGrp_LoData,"outromix.smk");
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Playing outro movie \"%s\"\n",fname);
#endif
    play_smacker_file(fname, 17);
}

unsigned long seed_check_random(unsigned long range, unsigned long *seed, const char *func_name, unsigned long place)
{
  if (range == 0)
    return 0;
  unsigned long i;
  i = 9377 * (*seed) + 9439;
  *seed = _lrotr(i, 13);
  i = (*seed) % range;
/*
  if (byte_5642DD & 0x01)
  {
      if (a2 == &dword_5E6742)
        LbSyncLog("%s: place %d, val %d\n", func_name, place, i);
  }
*/
  return i;
}

struct Room *create_room(unsigned char a1, unsigned char a2, unsigned short a3, unsigned short a4)
{
  return _DK_create_room(a1, a2, a3, a4);
}

void set_room_efficiency(struct Room *room)
{
  _DK_set_room_efficiency(room);
}

void init_dungeons(void)
{
  int i,k;
  struct Dungeon *dungeon;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    dungeon = &(game.dungeon[game.field_14E496%DUNGEONS_COUNT]);
    dungeon->field_B01[i%DUNGEONS_COUNT] = game.field_14E946;
    dungeon = &(game.dungeon[i%DUNGEONS_COUNT]);
    dungeon->field_B01[game.field_14E496%DUNGEONS_COUNT] = game.field_14E946;
    dungeon->field_918 = 0;
    dungeon->field_919 = 0;
    dungeon->field_2D = 0;
    dungeon->field_2F = 0;
    dungeon->field_E9F = i;
    dungeon->field_105C = game.field_14EB8C;
    dungeon->field_139F = 0;
    dungeon->field_13A3 = 0;
    for (k=0; k < DUNGEONS_COUNT; k++)
    {
      if (k == i)
        dungeon->field_B01[k] = game.field_14E94A;
      else
        dungeon->field_B01[k] = game.field_14E946;
    }
    memset(dungeon->field_1489, 0, 32);
  }
}

short thing_model_is_gold_hoarde(unsigned short model)
{
  return ((model >= 52) && (model <= 56));
}

long add_gold_to_hoarde(struct Thing *thing, struct Room *room, long amount)
{
  return _DK_add_gold_to_hoarde(thing, room, amount);
}

/*
 * Updates thing interaction with rooms. Sometimes deletes the given thing.
 * @return Returns true if everything is ok, false if the thing was incorrect.
 */
short check_and_asimilate_thing_by_room(struct Thing *thing)
{
  struct Room *room;
  unsigned long n;
  if (thing_model_is_gold_hoarde(thing->model))
  {
    room = get_room_thing_is_on(thing);
    if (room == NULL)
    {
      delete_thing_structure(thing, 0);
      return false;
    }
    n = (gold_per_hoarde/5)*(thing->model-51);
    thing->owner = room->owner;
    add_gold_to_hoarde(thing, room, n);
  }
  return true;
}

short thing_create_thing(struct InitThing *itng)
{
  static const char *func_name="thing_create_thing";
  struct Thing *thing;
  char *text;
  if (itng->owner == 7)
  {
    text = buf_sprintf("Invalid owning player %d, fixing to %d", (int)itng->owner, (int)game.field_14E496);
    error(func_name, 1225, text);
    itng->owner = game.field_14E496;
  } else
  if (itng->owner == 8)
  {
    text = buf_sprintf("Invalid owning player %d, fixing to %d", (int)itng->owner, (int)game.field_14E497);
    error(func_name, 1231, text);
    itng->owner = game.field_14E497;
  }
  if (itng->owner > 5)
  {
    text = buf_sprintf("Invalid owning player %d, thing discarded", (int)itng->owner);
    error(func_name, 1237, text);
    return false;
  }
  switch (itng->oclass)
  {
  case TCls_Object:
      thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
      if (thing != NULL)
      {
        if (itng->model == 49)
          thing->field_13 = itng->params[1];
        check_and_asimilate_thing_by_room(thing);
        // make sure we don't have invalid pointer
        thing = game.things_lookup[0];
      }
      break;
  case TCls_Creature:
      thing = create_creature(&itng->mappos, itng->model, itng->owner);
      if (thing != NULL)
      {
        init_creature_level(thing, itng->params[1]);
      }
      break;
  case TCls_EffectGen:
      thing = create_effect_generator(&itng->mappos, itng->model, itng->range, itng->owner, itng->index);
      break;
  case TCls_Trap:
      thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
      break;
  case TCls_Door:
      thing = create_door(&itng->mappos, itng->model, itng->params[0], itng->owner, itng->params[1]);
      break;
  case 10:
  case 11:
      thing = create_thing(&itng->mappos, itng->oclass, itng->model, itng->owner, itng->index);
      break;
  default:
      text = buf_sprintf("Invalid class %d, thing discarded", (int)itng->oclass);
      error(func_name, 1306, text);
      return false;
  }
  if (thing == NULL)
  {
    text = buf_sprintf("Couldn't create thing of class %d, model %d", (int)itng->oclass, (int)itng->model);
    error(func_name, 1306, text);
    return false;
  }
  return true;
}


void init_top_texture_to_cube_table(void)
{
  _DK_init_top_texture_to_cube_table();
}

void init_columns(void)
{
  _DK_init_columns();
}

void init_whole_blocks(void)
{
  _DK_init_whole_blocks();
}

/*
 * Loads map file with given level number and file extension.
 * @return Returns NULL if the file doesn't exist or is smaller than ldsize;
 * on success, returns a buffer which should be freed after use,
 * and sets ldsize into its size.
 */
unsigned char *load_single_map_file_to_buffer(unsigned long lv_num,const char *fext,long *ldsize)
{
  unsigned char *buf;
  char *fname;
  long fsize;
  fname = prepare_file_fmtpath(FGrp_Levels,"map%05lu.%s",lv_num,fext);
  wait_for_cd_to_be_available();
  fsize = LbFileLengthRnc(fname);
  if (fsize < *ldsize)
  {
    LbWarnLog("Map file \"map%05lu.%s\" doesn't exist or is too small.\n",lv_num,fext);
    return NULL;
  }
  buf = LbMemoryAlloc(fsize+16);
  if (buf == NULL)
  {
    LbWarnLog("Can't allocate %ld bytes to load \"map%05lu.%s\".\n",fsize,lv_num,fext);
    return NULL;
  }
  fsize = LbFileLoadAt(fname,buf);
  if (fsize < *ldsize)
  {
    LbWarnLog("Reading map file \"map%05lu.%s\" failed.\n",lv_num,fext);
    LbMemoryFree(buf);
    return false;
  }
  *ldsize = fsize;
  return buf;
}

long convert_old_column_file(unsigned long lv_num)
{
  _DK_convert_old_column_file(lv_num);
}

long load_column_file(unsigned long lv_num)
{
  //return _DK_load_column_file(lv_num);
  struct Column *col;
  unsigned long i;
  long k;
  unsigned short n;
  long total;
  unsigned char *buf;
  long fsize;
  if (game.numfield_C & 0x08)
  {
    convert_old_column_file(lv_num);
    game.numfield_C &= 0xFFF7u;
  }
  fsize = 8;
  buf = load_single_map_file_to_buffer(lv_num,"clm",&fsize);
  if (buf == NULL)
    return false;
  clear_columns();
  i = 0;
  total = llong(&buf[i]);
  i += 4;
  // Validate total amount of columns
  if ((total < 0) || (total > (fsize-8)/sizeof(struct Column)))
  {
    total = (fsize-8)/sizeof(struct Column);
    LbWarnLog("Bad amount of columns in CLM file; corrected to %ld.\n",total);
  }
  if (total > COLUMNS_COUNT)
  {
    LbWarnLog("Only %d columns supported, CLM file has %ld.\n",COLUMNS_COUNT,total);
    total = COLUMNS_COUNT;
  }
  // Read and validate second amount
  game.field_14AB3F = llong(&buf[i]);
  if (game.field_14AB3F >= COLUMNS_COUNT)
  {
    game.field_14AB3F = COLUMNS_COUNT-1;
  }
  i += 4;
  // Fill the columns
  for (k=0; k < total; k++)
  {
    col = &game.columns[k];
    LbMemoryCopy(col, &buf[i], sizeof(struct Column));
    n = find_column_height(col);
    col->bitfileds = (n<<4) ^ ((n<<4) ^ (col->bitfileds)) & 0xF;
    i += sizeof(struct Column);
  }
  LbMemoryFree(buf);
  return true;
}

long load_map_data_file(unsigned long lv_num)
{
  //return _DK_load_map_data_file(lv_num);
  struct Map *map;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  unsigned long n;
  unsigned short *wptr;
  long fsize;
  clear_map();
  fsize = 2*(map_subtiles_y+1)*(map_subtiles_x+1);
  buf = load_single_map_file_to_buffer(lv_num,"dat",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      n = -lword(&buf[i]);
      map->field_1 = map->field_1 ^ (map->field_1 ^ n) & 0x7FF;
      i += 2;
    }
  LbMemoryFree(buf);
  // Clear some bits and do some other setup
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      wptr = &game.field_46157[y*(map_subtiles_x+1) + x];
      *wptr = 32;
      map->field_1 &= 0xFFC007FFu;
      map->field_1 &= 0xF0FFFFFFu;
      map->field_1 &= 0x0FFFFFFFu;
    }
  return true;
}

short load_thing_file(unsigned long lv_num)
{
  //_DK_load_thing_file(lv_num); return true;
  struct InitThing itng;
  unsigned long i;
  long k;
  long total;
  unsigned char *buf;
  long fsize;
  fsize = 2;
  buf = load_single_map_file_to_buffer(lv_num,"tng",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  total = lword(&buf[i]);
  i += 2;
  // Validate total amount of things
  if ((total < 0) || (total > (fsize-2)/sizeof(struct InitThing)))
  {
    total = (fsize-2)/sizeof(struct InitThing);
    LbWarnLog("Bad amount of things in TNG file; corrected to %ld.\n",total);
  }
  if (total > THINGS_COUNT-2)
  {
    LbWarnLog("Only %d things supported, TNG file has %ld.\n",THINGS_COUNT-2,total);
    total = THINGS_COUNT-2;
  }
  // Create things
  for (k=0; k < total; k++)
  {
    LbMemoryCopy(&itng, &buf[i], sizeof(struct InitThing));
    thing_create_thing(&itng);
    i += sizeof(struct InitThing);
  }
  LbMemoryFree(buf);
  return true;
}

struct ActionPoint *allocate_free_action_point_structure_with_number(long apt_num)
{
  return _DK_allocate_free_action_point_structure_with_number(apt_num);
}

struct ActionPoint *actnpoint_create_actnpoint(struct InitActionPoint *iapt)
{
  struct ActionPoint *apt;
  apt = allocate_free_action_point_structure_with_number(iapt->num);
  if ((apt == NULL) || (apt == &game.action_points[0]))
    return &game.action_points[0];
  apt->mappos.x.val = iapt->mappos.x.val;
  apt->mappos.y.val = iapt->mappos.y.val;
  apt->range = iapt->range;
  return apt;
}

long load_action_point_file(unsigned long lv_num)
{
  static const char *func_name="load_action_point_file";
  //return _DK_load_action_point_file(lv_num);
  struct InitActionPoint iapt;
  unsigned long i;
  long k;
  long total;
  unsigned char *buf;
  char *text;
  long fsize;
  fsize = 4;
  buf = load_single_map_file_to_buffer(lv_num,"apt",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  total = llong(&buf[i]);
  i += 4;
  // Validate total amount of action points
  if ((total < 0) || (total > (fsize-4)/sizeof(struct InitActionPoint)))
  {
    total = (fsize-4)/sizeof(struct InitActionPoint);
    LbWarnLog("Bad amount of action points in APT file; corrected to %ld.\n",total);
  }
  if (total > ACTN_POINTS_COUNT-1)
  {
    LbWarnLog("Only %d action points supported, APT file has %ld.\n",ACTN_POINTS_COUNT-1,total);
    total = ACTN_POINTS_COUNT-1;
  }
  // Create action points
  for (k=0; k < total; k++)
  {
    LbMemoryCopy(&iapt, &buf[i], sizeof(struct InitActionPoint));
    if (actnpoint_create_actnpoint(&iapt) == &game.action_points[0])
    {
      text = buf_sprintf("Cannot allocate action point %d during APT load",k);
      error(func_name, 472, text);
    }
    i += sizeof(struct InitActionPoint);
  }
  LbMemoryFree(buf);
  return true;
}

void load_slab_file(void)
{
  _DK_load_slab_file();
}

void initialise_map_collides(void)
{
  _DK_initialise_map_collides();
}

void initialise_map_health(void)
{
  _DK_initialise_map_health();
}

unsigned short slab_to_room_type(unsigned short slab_type)
{
  switch (slab_type)
  {
  case 14:
      return 1;
  case 16:
      return 2;
  case 18:
      return 3;
  case 20:
      return 4;
  case 22:
      return 5;
  case 24:
      return 6;
  case 26:
      return 7;
  case 28:
      return 8;
  case 30:
      return 9;
  case 32:
      return 10;
  case 34:
      return 11;
  case 36:
      return 13;
  case 38:
      return 14;
  case 40:
      return 12;
  case 51:
      return 15;
  case 53:
      return 16;
  default:
      return 0;
  }
}

short initialise_map_rooms(void)
{
  static const char *func_name="initialise_map_rooms";
  struct SlabMap *slb;
  struct Room *room;
  unsigned long x,y;
  unsigned long n;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slb = &game.slabmap[y*map_tiles_x + x];
      n = slab_to_room_type(slb->slab);
      if (n > 0)
        room = create_room(slb->field_5 & 7, n, 3*x+1, 3*y+1);
      else
        room = NULL;
      if (room != NULL)
      {
        set_room_efficiency(room);
        set_room_capacity(room, 0);
      }
    }
  return true;
}

short initialise_map_wlb_auto(void)
{
  static const char *func_name="initialise_map_wlb_auto";
  struct SlabMap *slb;
  unsigned long x,y;
  unsigned long n;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slb = &game.slabmap[y*map_tiles_x + x];
      if (slb->slab == 51)
        n = (slab_attrs[13].field_15 << 3);
      else
        n = (slab_attrs[slb->slab%SLAB_TYPES_COUNT].field_15 << 3);
      slb->field_5 ^= (slb->field_5 ^ n) & 0x18;
    }
  return true;
}

short load_map_wlb_file(unsigned long lv_num)
{
  static const char *func_name="load_map_wlb_file";
  struct SlabMap *slb;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  unsigned long n;
  unsigned long nfixes;
  char *text;
  long fsize;
  nfixes = 0;
  fsize = map_tiles_y*map_tiles_x;
  buf = load_single_map_file_to_buffer(lv_num,"wlb",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slb = &game.slabmap[y*map_tiles_x + x];
      n = (buf[i] << 3);
      n = slb->field_5 ^ (slb->field_5 ^ n) & 0x18;
      slb->field_5 = n;
      n &= 0x18;
      if ((n != 16) || (slb->slab != 13))
        if ((n != 8) || (slb->slab != 12))
          if (((n == 16) || (n == 8)) && (slb->slab != 51))
          {
              nfixes++;
              slb->field_5 &= 0xE7u;
          }
      i++;
    }
  LbMemoryFree(buf);
  if (nfixes>0)
  {
    text = buf_sprintf("WLB file is muddled - Fixed values for %lu slabs");
    error(func_name, 4696, text);
  }
  return true;
}

short initialise_extra_slab_info(unsigned long lv_num)
{
  short result;
  //_DK_initialise_extra_slab_info(lv_num);
  result = true;
  result = result && initialise_map_rooms();
  result = result && load_map_wlb_file(lv_num);
  result = result && initialise_map_wlb_auto();
  return result;
}

short load_map_slab_file(unsigned long lv_num)
{
  static const char *func_name="load_map_slab_file";
  //return _DK_load_map_slab_file(lv_num);
  struct SlabMap *slbmap;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  unsigned long n;
  long fsize;
  fsize = 2*map_tiles_y*map_tiles_x;
  buf = load_single_map_file_to_buffer(lv_num,"slb",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < map_tiles_y; y++)
    for (x=0; x < map_tiles_x; x++)
    {
      slbmap = &game.slabmap[y*map_tiles_x + x];
      n = lword(&buf[i]);
      if (n > SLAB_TYPES_COUNT)
      {
        LbWarnLog("Slab Type %d exceeds limit of %d\n",n,SLAB_TYPES_COUNT);
        n = SlbT_ROCK;
      }
      slbmap->slab = n;
      i += 2;
    }
  LbMemoryFree(buf);
  initialise_map_collides();
  initialise_map_health();
  initialise_extra_slab_info(lv_num);
  return true;
}

short load_map_flag_file(unsigned long lv_num)
{
  struct Map *map;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  long fsize;
  fsize = 2*(map_subtiles_y+1)*(map_subtiles_x+1);
  buf = load_single_map_file_to_buffer(lv_num,"flg",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      map->field_0 = buf[i];
      i += 2;
    }
  LbMemoryFree(buf);
  return true;
}

long load_static_light_file(unsigned long lv_num)
{
  unsigned long i;
  long k;
  long total;
  unsigned char *buf;
  struct InitLight ilght;
  long fsize;
  fsize = 4;
  buf = load_single_map_file_to_buffer(lv_num,"lgt",&fsize);
  if (buf == NULL)
    return false;
  light_initialise();
  i = 0;
  total = llong(&buf[i]);
  i += 4;
  // Validate total amount of lights
  if ((total < 0) || (total > (fsize-4)/sizeof(struct InitLight)))
  {
    total = (fsize-4)/sizeof(struct InitLight);
    LbWarnLog("Bad amount of static lights in LGT file; corrected to %ld.\n",total);
  }
  if (total >= LIGHTS_COUNT)
  {
    LbWarnLog("Only %d static lights supported, LGT file has %ld.\n",LIGHTS_COUNT,total);
    total = LIGHTS_COUNT-1;
  } else
  if (total >= LIGHTS_COUNT/2)
  {
    LbWarnLog("More than %d%% of light slots used by static lights.\n",100*total/LIGHTS_COUNT);
  }
  // Create the lights
  for (k=0; k < total; k++)
  {
    LbMemoryCopy(&ilght, &buf[i], sizeof(struct InitLight));
    light_create_light(&ilght);
    i += sizeof(struct InitLight);
  }
  LbMemoryFree(buf);
  return true;
}

short load_and_setup_map_info(unsigned long lv_num)
{
  char *fname;
  unsigned char *buf;
  long fsize;
  fsize = 1;
  buf = load_single_map_file_to_buffer(lv_num,"inf",&fsize);
  if (buf == NULL)
  {
    game.texture_id = 0;
    return false;
  }
  game.texture_id = buf[0];
  LbMemoryFree(buf);
  return true;
}

long load_map_wibble_file(unsigned long lv_num)
{
  struct Map *map;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i,k;
  char *fname;
  long fsize;
  fsize = (map_subtiles_y+1)*(map_subtiles_x+1);
  buf = load_single_map_file_to_buffer(lv_num,"wib",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      map = &game.map[y*(map_subtiles_x+1) + x];
      k = buf[i];
      k = map->field_1 ^ (k << 22);
      map->field_1 = map->field_1 ^ k & 0xC00000;
      i++;
    }
  LbMemoryFree(buf);
  return true;
}

short load_map_ownership_file(unsigned long lv_num)
{
  struct SlabMap *slbmap;
  unsigned long x,y;
  unsigned char *buf;
  unsigned long i;
  char *fname;
  long fsize;
  fsize = (map_subtiles_y+1)*(map_subtiles_x+1);
  buf = load_single_map_file_to_buffer(lv_num,"own",&fsize);
  if (buf == NULL)
    return false;
  i = 0;
  for (y=0; y < (map_subtiles_y+1); y++)
    for (x=0; x < (map_subtiles_x+1); x++)
    {
      slbmap = &game.slabmap[map_to_slab[y]*map_tiles_x + map_to_slab[x]];
      slbmap->field_5 ^= (slbmap->field_5 ^ buf[i]) & 7;
      i++;
    }
  LbMemoryFree(buf);
  return true;
}

long ceiling_init(unsigned long a1, unsigned long a2)
{
  return _DK_ceiling_init(a1, a2);
}

void set_room_capacity(struct Room *room, long capac)
{
  _DK_set_room_capacity(room, capac);
}

void reinitialise_treaure_rooms(void)
{
  static const char *func_name="reinitialise_treaure_rooms";
  struct Dungeon *dungeon;
  struct Room *room;
  unsigned int i,k,n;
  for (n=0; n < DUNGEONS_COUNT; n++)
  {
    dungeon = &(game.dungeon[n]);
    i = dungeon->field_F;
    k = 0;
    while (i > 0)
    {
      if (i > ROOMS_COUNT)
      {
        error(func_name,478,"Jump out of rooms array detected");
        break;
      }
      room = &game.rooms[i];
      if (room == &game.rooms[0])
        break;
      i = room->field_6;
      set_room_capacity(room, 1);
      k++;
      if (k > ROOMS_COUNT)
      {
        error(func_name,479,"Infinite loop detected when sweeping rooms list");
        break;
      }
    }
  }
}

short load_level_file(unsigned long lvnum)
{
  static const char *func_name="load_level_file";
  char *fname;
  short result;
  //_DK_load_level_file(lvnum); return true;
  fname = prepare_file_fmtpath(FGrp_Levels,"map%05lu.slb",lvnum);
  wait_for_cd_to_be_available();
  if (LbFileExists(fname))
  {
    result = true;
    load_map_data_file(lvnum);
    load_map_flag_file(lvnum);
    load_column_file(lvnum);
    init_whole_blocks();
    load_slab_file();
    init_columns();
    load_static_light_file(lvnum);
    if (!load_map_ownership_file(lvnum))
      result = false;
    load_map_wibble_file(lvnum);
    load_and_setup_map_info(lvnum);
    load_texture_map_file(game.texture_id, 2);
    load_action_point_file(lvnum);
    if (!load_map_slab_file(lvnum))
      result = false;
    if (!load_thing_file(lvnum))
      result = false;
    reinitialise_treaure_rooms();
    ceiling_init(0, 1);
  } else
  {
    error(func_name,79,"The given map doesn't exist; creating empty map.");
    init_whole_blocks();
    load_slab_file();
    init_columns();
    game.texture_id = 0;
    load_texture_map_file(game.texture_id, 2);
    init_top_texture_to_cube_table();
    result = false;
  }
  return result;
}

short load_map_file(long lvidx)
{
  return load_level_file(lvidx);
}

void clear_messages(void)
{
  int i;
  for (i=0; i<MESSAGE_QUEUE_COUNT; i++)
  {
    memset(&message_queue[i], 0, sizeof(struct MessageQueueEntry));
  }
}

void init_dungeon_owner(unsigned short owner)
{
  static const char *func_name="init_dungeon_owner";
  struct Dungeon *dungeon;
  struct Thing *thing;
  int i,k;
  k = 0;
  i = game.thing_lists[2].field_4;
  while (i>0)
  {
    if (i >= THINGS_COUNT)
    {
      error(func_name,4578,"Jump out of things array bounds detected");
      break;
    }
    thing = game.things_lookup[i];
    if ((thing == game.things_lookup[0]) || (thing == NULL))
      break;
    i = thing->next_of_class;
    if ((game.objects_config[thing->model].field_6) && (thing->owner == owner))
    {
      dungeon = &(game.dungeon[owner%DUNGEONS_COUNT]);
      dungeon->field_0 = thing->field_1B;
      break;
    }
    k++;
    if (k > THINGS_COUNT)
    {
      error(func_name,4579,"Infinite loop detected when sweeping things list");
      break;
    }
  }
}

void init_level(void)
{
  static const char *func_name="init_level";
#if (BFDEBUG_LEVEL > 6)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  struct Thing *thing;
  struct Coord3d pos;
  unsigned short mem1,mem2;
  int i,k;
  //_DK_init_level(); return;

  mem2 = game.field_1517E6;
  mem1 = game.field_1517E7;
  memset(&pos,0,sizeof(struct Coord3d));
  game.field_14BB4A = 1;
  free_swipe_graphic();
  game.field_1516FF = -1;
  game.seedchk_random_used = 0;
  clear_game();
  reset_heap_manager();
  setup_heap_manager();
  init_good_player_as(hero_player);
  light_set_lights_on(1);
  start_rooms = &game.rooms[1];
  end_rooms = &game.rooms[ROOMS_COUNT];
  init_dungeons();
  load_map_file(game.level_file_number);
  init_navigation();
  clear_messages();
  game.field_14BB4E = (unsigned long)LbTimeSec();
  if (!SoundDisabled)
  {
    game.field_14BB54 = (seed_check_random(67, &game.field_14BB4E, func_name, 5712) % 3 + 1);
    game.field_14BB55 = 0;
  }
  light_set_lights_on(1);
  init_dungeon_owner(game.field_14E496);
  game.numfield_D |= 0x04;
  game.field_1517E6 = mem2;
  game.field_1517E7 = mem1;
  event_initialise_all();
  battle_initialise();

  thing = create_ambient_sound(&pos, 1, game.field_14E497);
  if (thing != NULL)
    game.field_14E906 = thing->field_1B;
  else
    error(func_name, 481, "Could not create ambient sound object");
  zero_messages();
  game.field_150356 = 0;
  game.field_15035A = 0;
  init_messages();
  game.field_1517FB = 0;
  game.field_1517FC = 0;
  game.field_15033A = 0;
  game.field_151801 = 0;
  game.field_151805 = 0;
  game.field_151809 = 0;
  game.chosen_spell_type = 0;
  game.chosen_spell_look = 0;
  game.chosen_spell_tooltip = 0;
  game.numfield_151819 = 0;
  game.numfield_15181D = 0;
  game.numfield_151821 = 0;
}

void pannel_map_update(long x, long y, long w, long h)
{
  _DK_pannel_map_update(x, y, w, h);
}

void init_player_music(struct PlayerInfo *player)
{
  static const char *func_name="init_player_music";
  game.audiotrack = ((game.level_file_number - 1) % -4) + 3;
  StopMusic();
  switch (seed_check_random(3, &game.field_14BB4E, func_name, 363))
  {
  case 0:
      if (LoadAwe32Soundfont("bullfrog"))
        StartMusic(1, 127);
      break;
  case 1:
      if (LoadAwe32Soundfont("atmos1"))
        StartMusic(1, 127);
      break;
  case 2:
      if (LoadAwe32Soundfont("atmos2"))
        StartMusic(1, 127);
      break;
  }
}

void init_player(struct PlayerInfo *player, short no_explore)
{
  static const char *func_name="init_player";
#if (BFDEBUG_LEVEL > 5)
    LbSyncLog("%s: Starting\n",func_name);
#endif
  //_DK_init_player(player, no_explore); return;
  player->mouse_x = 10;
  player->mouse_y = 12;
  player->minimap_zoom = 256;
  player->field_4D1 = player->field_2B;
  setup_engine_window(0, 0, MyScreenWidth, MyScreenHeight);
  player->field_456 = 1;
  player->field_453 = 1;
  player->field_14 = 2;
  player->field_4C9 = _DK_palette;
  if (player == &game.players[my_player_number])
  {
    game.numfield_C |= 0x40;
    set_gui_visible(true);
    init_gui();
    turn_on_menu(1);
    turn_on_menu(2);
  }
  switch (game.flagfield_14EA4A)
  {
  case 2:
    init_player_as_single_keeper(player);
    init_player_start(player);
    reset_player_mode(player, 1);
    if ( !no_explore )
      init_keeper_map_exploration(player);
    break;
  case 5:
    if (player->field_2C != 1)
    {
      error(func_name, 290, "Non Keeper in Keeper game");
      break;
    }
    init_player_as_single_keeper(player);
    init_player_start(player);
    reset_player_mode(player, 1);
    init_keeper_map_exploration(player);
    break;
  default:
    error(func_name, 309, "How do I set up this player?");
    break;
  }
  init_player_cameras(player);
  pannel_map_update(0, 0, map_subtiles_x+1, map_subtiles_y+1);
  player->strfield_463[0] = '\0';
  if (player == &game.players[my_player_number])
  {
    init_player_music(player);
  }
  player->field_2A = (1 << player->field_2B);
  player->field_10 = 0;
}

void init_players(void)
{
  struct PlayerInfo *player;
  int i;
  for (i=0;i<PLAYERS_COUNT;i++)
  {
    player=&(game.players[i]);
    player->field_0 ^= (player->field_0 ^ ((game.packet_save_head.field_C & (1 << i)) >> i)) & 1;
    if (player->field_0 & 0x01)
    {
      player->field_2B = i;
      player->field_0 ^= (player->field_0 ^ (((game.packet_save_head.field_D & (1 << i)) >> i) << 6)) & 0x40;
      if ((player->field_0 & 0x40) == 0)
      {
        game.field_14E495++;
        player->field_2C = 1;
        game.flagfield_14EA4A = 5;
        init_player(player, 0);
      }
    }
  }
}

void post_init_level(void)
{
  //_DK_post_init_level(); return;
  if (game.packet_save_enable)
    open_new_packet_file_for_save();
  calculate_dungeon_area_scores();
  init_animating_texture_maps();

  int i,k;
  for (i=0; i < DUNGEONS_COUNT; i++)
  {
    for (k=0; k < CREATURE_TYPES_COUNT; k++)
    {
      game.dungeon[i].field_A4F[k] = 10;
    }
  }
  clear_creature_pool();
  setup_computer_players2();
  load_stats_files();
  check_and_auto_fix_stats();
  load_script(game.level_file_number);
  init_dungeons_research();

  struct PlayerInfo *player;
  struct Thing *thing;
  struct Dungeon *dungeon;
  struct Coord3d *pos;
  if (game.field_1517E6)
  {
    player = &(game.players[my_player_number%PLAYERS_COUNT]);
    dungeon = &(game.dungeon[player->field_2B%DUNGEONS_COUNT]);
    i = dungeon->field_0;
    thing = game.things_lookup[i%THINGS_COUNT];
    pos = &(thing->mappos);
    thing = create_creature(pos, game.field_1517E6, 5);
    if (thing != NULL)
      init_creature_level(thing, game.field_1517E7);
    game.field_1517E6 = 0;
    game.field_1517E7 = 0;
  }

  update_dungeon_scores();
  update_dungeon_generation_speeds();
  init_traps();
  init_all_creature_states();
  init_keepers_map_exploration();
}

void post_init_players(void)
{
  _DK_post_init_players(); return;
}

short init_animating_texture_maps(void)
{
  //_DK_init_animating_texture_maps(); return;
  anim_counter = 7;
  return update_animating_texture_maps();
}

short frontend_is_player_allied(long plyr1, long plyr2)
{
  return _DK_frontend_is_player_allied(plyr1, plyr2);
}

void setup_alliances(void)
{
  int i;
  struct PlayerInfo *player;
  for (i=0; i<PLAYERS_COUNT; i++)
  {
      player=&(game.players[i]);
      if ((i != my_player_number) && (player->field_0 & 0x01))
      {
          if (frontend_is_player_allied(my_player_number, i))
          {
            toggle_ally_with_player(my_player_number, i);
            toggle_ally_with_player(i, my_player_number);
          }
      }
  }
}

/*
 * Exchanges verification packets between all players.
 * @return Returns true if all players return same checksum.
 */
short perform_checksum_verification()
{
  static const char *func_name="perform_checksum_verification";
  struct PlayerInfo *player;
  struct Packet *pckt;
  struct Thing *thing;
  unsigned long checksum_mem;
  short result;
  int i;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  result = true;
  checksum_mem = 0;
  for (i=1; i<THINGS_COUNT; i++)
  {
      thing = game.things_lookup[i];
      if ((thing == game.things_lookup[0]) || (thing == NULL))
        continue;
      if (thing->field_0 & 0x01)
      {
        checksum_mem += thing->mappos.z.val + thing->mappos.y.val + thing->mappos.x.val;
      }
  }
  clear_packets();
  pckt = &game.packets[player->packet_num];
  set_packet_action(pckt, 12, 0, 0, 0, 0);
  pckt->chksum = checksum_mem + game.field_14BB4A;
  if (LbNetwork_Exchange(pckt))
  {
    error(func_name, 210, "Network exchange failed on level checksum verification");
    result = false;
  }
  if ( checksums_different() )
  {
    error(func_name, 219, "Level checksums different for network players");
    result = false;
  }
  return result;
}

short setup_select_player_number(void)
{
  static const char *func_name="setup_select_player_number";
  struct PlayerInfo *player;
  short is_set;
  int i,k;
  is_set = 0;
  k = 0;
#if (BFDEBUG_LEVEL > 6)
  LbSyncLog("%s: Starting\n",func_name);
#endif
  for (i=0; i<NET_PLAYERS_COUNT; i++)
  {
      player=&(game.players[i%PLAYERS_COUNT]);
      if ( net_player_info[i].field_20 )
      {
        player->packet_num = i;
        if ((!is_set) && (my_player_number == i))
        {
          is_set = 1;
          my_player_number = k;
        }
        k++;
      }
  }
  return is_set;
}

void setup_exchange_player_number(void)
{
  static const char *func_name="setup_exchange_player_number";
  struct PlayerInfo *player;
  struct Packet *pckt;
  int i,k;
#if (BFDEBUG_LEVEL > 6)
  LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  clear_packets();
  pckt = &game.packets[my_player_number];
  set_packet_action(pckt, 10, player->field_2C, settings.field_3, 0, 0);
  pckt = &game.packets[my_player_number];
  if ( LbNetwork_Exchange(pckt) )
      error(func_name, 156, "LbNetwork_Exchange failed");
  k = 0;
  for (i=0; i<NET_PLAYERS_COUNT; i++)
  {
      pckt = &game.packets[i];
      if ((net_player_info[i].field_20) && (pckt->field_5 == 10))
      {
          player = &(game.players[k%PLAYERS_COUNT]);
          player->field_2B = k;
          player->field_0 |= 0x01;
          if (pckt->field_8 < 1u)
            player->field_4B5 = 2;
          else
            player->field_4B5 = 5;
          player->field_2C = pckt->field_6;
          init_player(player, 0);
          strncpy(player->field_15,enum_players_callback[i].field_0,sizeof(struct TbNetworkCallbackData));
          k++;
      }
  }
}
void init_players_local_game(void)
{
  static const char *func_name="init_players_local_game";
  struct PlayerInfo *player;
#if (BFDEBUG_LEVEL > 4)
  LbSyncLog("%s: Starting\n",func_name);
#endif
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_2B = my_player_number;
  player->field_0 |= 0x01;
  if (settings.field_3 < 1u)
    player->field_4B5 = 2;
  else
    player->field_4B5 = 5;
  init_player(player, 0);
}

void init_players_network_game(void)
{
  static const char *func_name="init_players_network_game";
  int i,k;
  struct PlayerInfo *player;
  struct Packet *pckt;
#if (BFDEBUG_LEVEL > 4)
  LbSyncLog("%s: Starting\n",func_name);
#endif
  if (LbNetwork_ChangeExchangeBuffer(game.packets, sizeof(struct Packet)))
      error(func_name, 119, "Unable to reinitialise ExchangeBuffer");
  setup_select_player_number();
  setup_exchange_player_number();
  perform_checksum_verification();
  setup_alliances();
}

void setup_count_players(void)
{
  int i;
  if (game.flagfield_14EA4A == 2)
  {
    game.field_14E495 = 1;
  } else
  {
    game.field_14E495 = 0;
    for (i=0; i<NET_PLAYERS_COUNT; i++)
    {
      if (net_player_info[i].field_20)
        game.field_14E495++;
    }
  }
}

void startup_saved_packet_game(void)
{
  //_DK_startup_saved_packet_game(); return;
  struct PlayerInfo *player;
  int i;
  clear_packets();
  open_packet_file_for_load(game.packet_fname);
  game.level_file_number = game.packet_save_head.level_num;
  lbDisplay.DrawColour = colours[15][15][15];
  game.pckt_gameturn = 0;
#if (BFDEBUG_LEVEL > 0)
  LbSyncLog("Initialising level %d\n", game.level_file_number);
  LbSyncLog("Packet Loading Active (File contains %d turns)\n", game.field_149F30);
  if ( game.packet_checksum )
    LbSyncLog("Packet Checksum Active\n");
  LbSyncLog("Fast Forward through %d game turns\n", game.turns_fastforward);
  if (game.numfield_149F42 != -1)
    LbSyncLog("Packet Quit at %d\n", game.numfield_149F42);
  if (game.packet_load_enable)
  {
    if (game.numfield_149F3E != game.numfield_149F3A)
      LbSyncLog("Logging things, game turns %d -> %d\n", game.numfield_149F3A, game.numfield_149F3E);
  }
#endif
  game.flagfield_14EA4A = 2;
  if (!(game.packet_save_head.field_C & (1 << game.numfield_149F46))
    || (game.packet_save_head.field_D & (1 << game.numfield_149F46)) )
    my_player_number = 0;
  else
    my_player_number = game.numfield_149F46;
  init_level();
  init_players();
  if (game.field_14E495 == 1)
    game.flagfield_14EA4A = 2;
  if (game.field_149F30 < game.turns_fastforward)
    game.turns_fastforward = game.field_149F30;
  post_init_level();
  post_init_players();
}

void faststartup_saved_packet_game(void)
{
  struct PlayerInfo *player;
  reenter_video_mode();
  startup_saved_packet_game();
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_6 &= 0xFDu;
  set_gui_visible(false);
  game.numfield_C ^= (game.numfield_C & 0x40);
}

void startup_network_game(void)
{
  static const char *func_name="startup_network_game";
#if (BFDEBUG_LEVEL > 0)
  LbSyncLog("Starting up network game.\n");
#endif
  //_DK_startup_network_game(); return;
  unsigned int flgmem;
  struct PlayerInfo *player;
  setup_count_players();
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  flgmem = player->field_2C;
  init_level();
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_2C = flgmem;
  if (game.flagfield_14EA4A == 2)
  {
    init_players_local_game();
  } else
  {
    init_players_network_game();
  }
  if (fe_computer_players)
    setup_computer_players();
  post_init_level();
  post_init_players();
  post_init_packets();
}

void faststartup_network_game(void)
{
  struct PlayerInfo *player;
  reenter_video_mode();
  my_player_number = default_loc_player;
  game.flagfield_14EA4A = 2;
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_2C = 1;
  game.level_file_number = game.numfield_16;
  startup_network_game();
  player=&(game.players[my_player_number%PLAYERS_COUNT]);
  player->field_6 &= 0xFDu;
}

int setup_old_network_service(void)
{
    return setup_network_service(_DK_net_service_index_selected);
}

void wait_at_frontend(void)
{
  static const char *func_name="wait_at_frontend";
  //_DK_wait_at_frontend(); return;
#if (BFDEBUG_LEVEL > 0)
  LbSyncLog("Falling into frontend menu.\n");
#endif
  // Moon phase calculation
  calculate_moon_phase(true,false);
  // Returning from Demo Mode
  if (game.flags_cd & MFlg_IsDemoMode)
  {
    close_packet_file();
    game.packet_load_enable = 0;
  }
  game.numfield_15 = -1;
  // Make sure campaign is loaded
  if (!load_default_campaign())
  {
    error(func_name, 731, "Unable to load default campaign");
    exit_keeper = 1;
    return;
  }
  // Init load/save catalogue
  initialise_load_game_slots();
  // Prepare to enter PacketLoad game
  if ( (game.packet_load_enable) && (!game.numfield_149F47) )
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
    _DK_FatalError = 1;
    exit_keeper = 1;
    return;
  }
  LbScreenClear(0);
  LbScreenSwap();
  if ( !frontend_load_data() )
  {
    error(func_name, 738, "Unable to load frontend data");
    exit_keeper = 1;
    return;
  }
  memset(scratch, 0, 0x300u);
  LbPaletteSet(scratch);
  frontend_set_state(get_startup_menu_state());

  short finish_menu = 0;
  game.flags_cd &= 0xBFu;

  // Begin the frontend loop
  long last_loop_time = LbTimerClock();
  do
  {
    if ((!LbWindowsControl()) && ((game.numfield_A & 0x01) == 0))
    {
      exit_keeper = 1;
#if (BFDEBUG_LEVEL > 0)
      LbSyncLog("%s: Windows Control exit condition invoked\n",func_name);
#endif
      break;
    }
//LbSyncLog("update_mouse\n");
    update_mouse();
    old_mouse_over_button = frontend_mouse_over_button;
    frontend_mouse_over_button = 0;

//LbSyncLog("frontend_input\n");
    frontend_input();
    if ( exit_keeper )
    {
#if (BFDEBUG_LEVEL > 0)
      LbSyncLog("%s: Frontend Input exit condition invoked\n",func_name);
#endif
      break; // end while
    }

//LbSyncLog("frontend_update\n");
    frontend_update(&finish_menu);
    if ( exit_keeper )
    {
#if (BFDEBUG_LEVEL > 0)
      LbSyncLog("%s: Frontend Update exit condition invoked\n",func_name);
#endif
      break; // end while
    }

    if ((!finish_menu) && (LbIsActive()))
    {
//LbSyncLog("frontend_draw\n");
      frontend_draw();
      LbScreenSwap();
    }

    if ( !SoundDisabled )
    {
      process_3d_sounds();
      process_sound_heap();
      MonitorStreamedSoundTrack();
    }

    if ( fade_palette_in )
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
  short prev_state = frontend_menu_state;
  frontend_set_state(0);
  if ( exit_keeper )
  {
    game.players[my_player_number].field_6 &= 0xFDu;
    return;
  }
  reenter_video_mode();

  display_loading_screen();
  short flgmem;
  switch ( prev_state )
  {
  case 7:
        my_player_number = default_loc_player;
        game.flagfield_14EA4A = 2;
        game.numfield_A &= 0xFFFEu;
        game.players[my_player_number].field_2C = 1;
        game.level_file_number = game.numfield_16;
        startup_network_game();
        break;
  case 8:
        game.level_file_number = game.numfield_16;
        game.numfield_A |= 0x01;
        game.flagfield_14EA4A = 5;
        game.players[my_player_number].field_2C = 1;
        startup_network_game();
        break;
  case 10:
        flgmem = game.numfield_15;
        game.numfield_A &= 0xFFFEu;
        if ( game.numfield_15 == -2 )
        {
          error(func_name, 1012, "Why are we here");
          game.numfield_15 = flgmem;
        } else
        {
          LbScreenClear(0);
          LbScreenSwap();
          load_game(game.numfield_15);
          game.numfield_15 = flgmem;
        }
        break;
  case 25:
        game.flags_cd |= MFlg_IsDemoMode;
        startup_saved_packet_game();
        set_gui_visible(false);
        game.numfield_C ^= (game.numfield_C & 0x40);
        break;
  }
  game.players[my_player_number].field_6 &= 0xFDu;
}

void game_loop(void)
{
  //_DK_game_loop(); return;
  unsigned long random_seed;
  unsigned long playtime;
  playtime = 0;
  random_seed = 0;
#if (BFDEBUG_LEVEL > 0)
  LbSyncLog("Entering gameplay loop.\n");
#endif
  while ( !exit_keeper )
  {
    update_mouse();
    wait_at_frontend();
    if ( exit_keeper )
      break;
    struct PlayerInfo *player=&(game.players[my_player_number%PLAYERS_COUNT]);
    if ( game.flagfield_14EA4A == 2 )
    {
      if ( game.numfield_15 == -1 )
      {
        set_player_instance(player, 11, 0);
      } else
      {
        game.numfield_15 = -1;
        game.numfield_C &= 0xFFFEu;
      }
    }
    unsigned long starttime;
    unsigned long endtime;
    starttime = LbTimerClock();
    game.dungeon[my_player_number%DUNGEONS_COUNT].time1 = timeGetTime();//starttime;
    game.dungeon[my_player_number%DUNGEONS_COUNT].time2 = timeGetTime();//starttime;
    LbScreenClear(0);
    LbScreenSwap();
    keeper_gameplay_loop();
    LbMouseChangeSpriteAndHotspot(0, 0, 0);
    LbScreenClear(0);
    LbScreenSwap();
    StopRedbookTrack();
    StopMusic();
    frontstats_initialise();
    delete_all_structures();
    clear_mapwho();
    endtime = LbTimerClock();
    quit_game = 0;
    if ((game.numfield_C & 0x02) != 0)
        exit_keeper=true;
    playtime += endtime-starttime;
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("Play time is %d seconds\n",playtime>>10);
#endif
    random_seed += game.seedchk_random_used;
    reset_eye_lenses();
    close_packet_file();
  } // end while
  // Stop the movie recording if it's on
  if (game.numfield_A & 0x08)
    movie_record_stop();
}

short reset_game(void)
{
  _DK_IsRunningUnmark();
  _DK_LbMouseSuspend();
  LbIKeyboardClose();
  LbScreenReset();
  LbDataFreeAll(game_load_files);
  LbMemoryFree(strings_data);
  strings_data = NULL;
  FreeAudio();
  return _DK_LbMemoryReset();
}

short process_command_line(unsigned short argc, char *argv[])
{
  static const char *func_name="process_command_line";
  char fullpath[CMDLN_MAXLEN+1];
  strncpy(fullpath, argv[0], CMDLN_MAXLEN);

  sprintf( keeper_runtime_directory, fullpath);
  char *endpos=strrchr( keeper_runtime_directory, '\\');
  if (endpos==NULL)
      endpos=strrchr( keeper_runtime_directory, '/');
  if (endpos!=NULL)
      *endpos='\0';

  SoundDisabled = 0;
  // Note: the working log file is set up in LbBullfrogMain
  _DK_LbErrorLogSetup(0, 0, 1);

  game.numfield_149F42 = -1;
  game.numfield_149F46 = 0;
  game.packet_checksum = 1;
  game.numfield_1503A2 = -1;
  game.flags_font &= 0xFEu;
  game.numfield_149F47 = 0;
  game.numfield_16 = 0;
  game.num_fps = 20;
  game.flags_cd &= 0xFEu;
  game.flags_cd |= 0x40;

  short bad_param = 0;
  long level_num = -1;

  unsigned short narg;
  narg = 1;
  while ( narg < argc )
  {
      char *par;
      par = argv[narg];
      if ( (par==NULL) || ((par[0] != '-') && (par[0] != '/')) )
          return -1;
      char parstr[CMDLN_MAXLEN+1];
      char pr2str[CMDLN_MAXLEN+1];
      strncpy(parstr, par+1, CMDLN_MAXLEN);
      if ( narg+1 < argc )
        strncpy(pr2str,  argv[narg+1], CMDLN_MAXLEN);
      else
        pr2str[0]='\0';

      if ( stricmp(parstr, "nointro") == 0 )
      {
        game.no_intro = 1;
      } else
      if ( stricmp(parstr, "nocd") == 0 )
      {
          game.flags_cd |= MFlg_NoMusic;
      } else
      if ( stricmp(parstr, "1player") == 0 )
      {
          game.one_player = 1;
      } else
      if ( (stricmp(parstr, "s") == 0) || (stricmp(parstr, "nosound") == 0) )
      {
          SoundDisabled = 1;
      } else
      if ( stricmp(parstr, "fps") == 0 )
      {
          narg++;
          game.num_fps = atoi(pr2str);
      } else
      if ( stricmp(parstr, "human") == 0 )
      {
          narg++;
          default_loc_player = atoi(pr2str);
      } else
      if ( stricmp(parstr, "usersfont") == 0 )
      {
          game.flags_font |= 0x40;
      } else
      if ( stricmp(parstr, "vidsmooth") == 0 )
      {
          smooth_on = 1;
      } else
      if ( stricmp(parstr,"level") == 0 )
      {
        game.numfield_C |= 0x02;
        level_num = atoi(pr2str);
        narg++;
      } else
      if ( stricmp(parstr,"packetload") == 0 )
      {
         if (game.packet_save_enable)
            LbWarnLog("PacketSave disabled to enable PacketLoad.\n");
         game.packet_load_enable = true;
         game.packet_save_enable = false;
         strncpy(game.packet_fname,pr2str,149);
         narg++;
      } else
      if ( stricmp(parstr,"packetsave") == 0 )
      {
         if (game.packet_load_enable)
            LbWarnLog("PacketLoad disabled to enable PacketSave.\n");
         game.packet_load_enable = false;
         game.packet_save_enable = true;
         strncpy(game.packet_fname,pr2str,149);
         narg++;
      } else
      if ( stricmp(parstr,"q") == 0 )
      {
         game.numfield_C |= 0x02;
      } else
      if ( stricmp(parstr,"columnconvert") == 0 )
      {
         game.numfield_C |= 0x08;
      } else
      if ( stricmp(parstr,"lightconvert") == 0 )
      {
         game.numfield_C |= 0x10;
      } else
      if ( stricmp(parstr, "alex") == 0 )
      {
          game.flags_font |= 0x20;
      } else
      {
#if (BFDEBUG_LEVEL > 0)
        LbWarnLog("Unrecognized command line parameter '%s'.\n",parstr);
#endif
        bad_param=narg;
      }
      narg++;
  }

  if (level_num == -1)
    level_num = first_singleplayer_level();
  game.numfield_16 = level_num;
  game.level_number = level_num;
  if ((game.numfield_C & 0x02) == 0)
    game.level_number = first_singleplayer_level();
  my_player_number = default_loc_player;
  return (bad_param==0);
}

void close_video_context(void)
{
  if ( _DK_lpDDC != NULL )
  {
    TDDrawBaseVTable *vtable=_DK_lpDDC->vtable;
    //Note: __thiscall is for functions with no arguments same as __fastcall,
    // so we're using __fastcall and passing the "this" pointer manually
    vtable->reset_screen(_DK_lpDDC);
    if ( _DK_lpDDC != NULL )
    {
      vtable=_DK_lpDDC->vtable;
      // Destructor requires a flag passed in AX
      asm volatile ("  movl $1,%eax\n");
      vtable->dt(_DK_lpDDC);
    }
  }
}

int LbBullfrogMain(unsigned short argc, char *argv[])
{
  static const char *func_name="LbBullfrogMain";
  //return _DK_LbBullfrogMain(argc, argv);
  short retval;
  retval=0;
  LbErrorLogSetup("/", log_file_name, 5);
  strcpy(window_class_name, PROGRAM_NAME);
  LbTimerInit();
  LbSetIcon(110);
  srand(LbTimerClock());

  retval=process_command_line(argc,argv);
  if ( retval < 1 )
  {
      static const char *msg_text="Command line parameters analysis failed.\n";
      close_video_context();
      error_dialog_fatal(func_name, 1, msg_text);
      LbErrorLogClose();
      return 0;
  }

  retval=setup_game();
  if ( retval )
    game_loop();
  reset_game();
/*
          MessageBox(NULL, "Finished OK.",
             "Dungeon Keeper Fan eXpansion", MB_OK | MB_ICONINFORMATION);
*/
  close_video_context();
  if ( !retval )
  {
      static const char *msg_text="Setting up game failed.\n";
      error_dialog_fatal(func_name, 2, msg_text);
  } else
  {
#if (BFDEBUG_LEVEL > 0)
    LbSyncLog("%s finished properly.\n",func_name);
#endif
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

int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
//  return _DK_WinMain(hThisInstance,hPrevInstance,lpszArgument,nFunsterStil);
  static const char *func_name="WinMain";
  char *text;
  _DK_hInstance = hThisInstance;
  _DK_lpDDC = NULL;

  get_cmdln_args(bf_argc, bf_argv);

//TODO: delete when won't be needed anymore
  memcpy(_DK_menu_list,menu_list,sizeof(menu_list));
#if (BFDEBUG_LEVEL > 1)
/*  {
      struct PlayerInfo *player=&(game.players[0]);
      text = buf_sprintf("Position of the first Player is %06x, first Camera is %06x bytes.\n",((int)player) - ((int)&_DK_game),((int)&(player->camera)) - ((int)player));
      error_dialog(func_name, 1, text);
      return 0;
  }
  {
      struct Dungeon *dungeon=&(game.dungeon[0]);
      text = buf_sprintf("Position of the first Dungeon is %06x, field_ACF is at %06x bytes.\n",
                  ((int)dungeon) - ((int)&game),((int)(&dungeon->field_ACF)) - ((int)dungeon));
      error_dialog(func_name, 1, text);
      return 0;
  }*/
  if (sizeof(struct PlayerInfo)!=SIZEOF_PlayerInfo)
  {
      text = buf_sprintf("Bad compilation - struct PlayerInfo has wrong size!\nThe difference is %d bytes.\n",sizeof(struct PlayerInfo)-SIZEOF_PlayerInfo);
      error_dialog(func_name, 1, text);
      return 0;
  }
  if (sizeof(struct Dungeon)!=SIZEOF_Dungeon)
  {
      text = buf_sprintf("Bad compilation - struct Dungeon has wrong size!\nThe difference is %d bytes.\n",sizeof(struct Dungeon)-SIZEOF_Dungeon);
      error_dialog(func_name, 1, text);
      return 0;
  }
  if (sizeof(struct Game)!=SIZEOF_Game)
  {
      text = buf_sprintf("Bad compilation - struct Game has wrong size!\nThe difference is %d bytes.\n",sizeof(struct Game)-SIZEOF_Game);
      error_dialog(func_name, 1, text);
      return 0;
  }
#endif

  LbBullfrogMain(bf_argc, bf_argv);

//  LbFileSaveAt("!tmp_file", &_DK_game, sizeof(struct Game));

  return 0;
}
