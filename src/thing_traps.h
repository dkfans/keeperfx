/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file thing_traps.h
 *     Header file for thing_traps.c.
 * @par Purpose:
 *     Traps support functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     17 Jun 2010 - 07 Jul 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_THING_TRAPS_H
#define DK_THING_TRAPS_H

#include "bflib_basics.h"
#include "globals.h"
#include "engine_camera.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
/******************************************************************************/
#pragma pack(1)

enum ThingTrapModels {
    TngTrp_None = 0,
    TngTrp_Boulder,
    TngTrp_Unknown02,
    TngTrp_Unknown03,
    TngTrp_Unknown04,
    TngTrp_Unknown05,
    TngTrp_Unknown06,
    TngTrp_Unknown07,
    TngTrp_Unknown08,
    TngTrp_Unknown09,
    TngTrp_Unknown10,
};

enum TrapTriggerTypes {
    TrpTrg_None = 0,
    TrpTrg_LineOfSight90,
    TrpTrg_Pressure_Slab,
    TrpTrg_LineOfSight,
    TrpTrg_Pressure_Subtile,
};
enum TrapActivationTypes {
    TrpAcT_None = 0,
    TrpAcT_HeadforTarget90,
    TrpAcT_EffectonTrap,
    TrpAcT_ShotonTrap,
    TrpAcT_SlabChange,
    TrpAcT_CreatureShot,
    TrpAcT_CreatureSpawn,
    TrpAcT_Power
};

struct Thing;

struct TrapStats {
  unsigned long health;
  unsigned long sprite_anim_idx;
  unsigned long sprite_size_max;
  unsigned char unanimated;
  unsigned long anim_speed;
  unsigned char unshaded;
  unsigned char transparency_flag; // transparency in lower 2 bits
  unsigned char random_start_frame;
  short size_xy;
  short size_yz;
  unsigned char trigger_type;
  unsigned char activation_type;
  unsigned char created_itm_model; // Shot model, effect model, slab kind
  unsigned char hit_type;
  short light_radius; // creates light if not null
  unsigned char light_intensity;
  unsigned char light_flag;
  struct ComponentVector shotvector;
};

/******************************************************************************/

#pragma pack()
/******************************************************************************/
TbBool slab_has_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool slab_has_sellable_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool subtile_has_sellable_trap_on(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool subtile_has_trap_on(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
TbBool slab_middle_row_has_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool slab_middle_column_has_trap_on(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool can_place_trap_on(PlayerNumber plyr_idx, MapSubtlCoord stl_x, MapSubtlCoord stl_y, ThingModel trpkind);

TbBool destroy_trap(struct Thing *thing);
struct Thing *create_trap(struct Coord3d *pos, ThingModel trpkind, PlayerNumber plyr_idx);
struct Thing* activate_trap_spawn_creature(struct Thing* traptng, unsigned char model);
struct Thing *get_trap_for_position(MapSubtlCoord stl_x, MapSubtlCoord stl_y);
struct Thing *get_trap_for_slab_position(MapSlabCoord slb_x, MapSlabCoord slb_y);
TbBool trap_is_active(const struct Thing *thing);
TbBool trap_is_slappable(const struct Thing *thing, PlayerNumber plyr_idx);
TbBool thing_is_deployed_trap(const struct Thing *thing);
short thing_is_destructible_trap(const struct Thing* thing);
TbBool thing_is_sellable_trap(const struct Thing* thing);
TbBool trap_on_bridge(ThingModel trpkind);
TbBool rearm_trap(struct Thing *traptng);
TngUpdateRet update_trap(struct Thing *thing);
void init_traps(void);
void activate_trap(struct Thing *traptng, struct Thing *creatng);

unsigned long remove_trap(struct Thing *traptng, long *sell_value);
unsigned long remove_trap_on_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *sell_value);
unsigned long remove_traps_around_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *sell_value);

void external_activate_trap_shot_at_angle(struct Thing *thing, long a2, struct Thing *hand);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
