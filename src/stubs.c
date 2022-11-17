/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file stubs.c
 *     Stubs for unimplemented DK routines.
 * @par Purpose:
 *     Stubs for unimplemented DK routines.
 * @par Comment:
 *     None.
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "globals.h"
#include "bflib_basics.h"

TbBool _DK_light_render_light_sub1_sub2(int a1, SubtlCodedCoords stl_num, int a3) { return 0; }
char _DK_light_render_light_sub2(struct Light *lgt, int radius, int a3, unsigned int a4) { return 0; }
int _DK_light_render_light_sub3(struct Light *lgt, int radius, int a3, unsigned int a4) { return 0; }
int _DK_light_render_light_sub1_sub1(unsigned int a1,unsigned int a2,int a3,unsigned int a4,unsigned int a5,long *a6,long *a7) { return 0; }
long _DK_ceiling_init(unsigned long a1, unsigned long a2) { return 0; }
long _DK_ceiling_block_is_solid_including_corners_return_height(long a1, long a2, long a3) { return 0; }
long _DK_check_out_unreinforced_place(struct Thing *creatng) { return 0; }
long _DK_check_out_unreinforced_area(struct Thing *creatng) { return 0; }
long _DK_imp_will_soon_be_converting_at_excluding(struct Thing *creatng, long slb_x, long slb_y) { return 0; }
void _DK_set_slab_explored_flags(unsigned char flag, long tgslb_x, long tgslb_y) { }
long _DK_ceiling_partially_recompute_heights(long sx, long sy, long ex, long ey) { return 0; }
void _DK_shuffle_unattached_things_on_slab(long a1, long stl_x) { }
char *_DK_mdlf_for_cd(struct TbLoadFiles *) { return NULL; }
char *_DK_mdlf_default(struct TbLoadFiles *) { return NULL; }
short _DK_hug_round(struct Thing *creatng, struct Coord3d *pos1, struct Coord3d *pos2, unsigned short a4, long *a5) { return 0; }
signed char _DK_get_starting_angle_and_side_of_hug(struct Thing *creatng, struct Coord3d *pos, long *a3, unsigned char *a4, long a5, unsigned char direction) { return 0; }
long _DK_get_map_index_of_first_block_thing_colliding_with_travelling_to(struct Thing *creatng, struct Coord3d *startpos, struct Coord3d *endpos, long a4, unsigned char a5) { return 0; }
long _DK_get_map_index_of_first_block_thing_colliding_with_at(struct Thing *creatng, struct Coord3d *pos, long a3, unsigned char a4) { return 0; }
long _DK_ariadne_check_forward_for_wallhug_gap(struct Thing *thing, struct Ariadne *arid, struct Coord3d *pos, long outfri_y2) { return 0; }
long _DK_heapmgr_free_handle(struct HeapMgrHeader *hmhead, struct HeapMgrHandle *hmhandle) { return 0; }
long _DK_computer_look_for_opponent(struct Computer2 *comp, long stl_x, long stl_y, long a4) { return 0; }
void _DK_fill_in_explored_area(unsigned char plyr_idx, short stl_x, short stl_y) { }
long _DK_get_next_gap_creature_can_fit_in_below_point(struct Thing *creatng, struct Coord3d *pos) { return 0; }
