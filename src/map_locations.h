/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_locations.h
 *     Header file for map_locations.c.
 * @par Purpose:
 *     Map location functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   KeeperFx Team
 * @date     7 Feb 2022
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_MAP_LOCATIONS_H
#define DK_MAP_LOCATIONS_H

#include "globals.h"
#include "bflib_basics.h"
#include "config.h"


#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

enum MapLocationTypes {
    MLoc_NONE = 0,
    MLoc_ACTIONPOINT,
    MLoc_HEROGATE,
    MLoc_PLAYERSHEART,
    MLoc_CREATUREKIND, // 4
    MLoc_OBJECTKIND,
    MLoc_ROOMKIND,
    MLoc_THING,
    MLoc_PLAYERSDUNGEON, // 8
    MLoc_APPROPRTDUNGEON,
    MLoc_DOORKIND,
    MLoc_TRAPKIND,
    MLoc_METALOCATION, // 12 // Triggered box, Combat, Last entered creature etc
    MLoc_COORDS, // 13 // contains stl coords encoded in the number actual location will be center of said subtile
};

enum MetaLocation {
  MML_LAST_EVENT = 1,
  MML_RECENT_COMBAT,
  MML_ACTIVE_CTA,
  MML_LAST_DEATH_EVENT,
  MML_LAST_TRAP_EVENT,
};

extern const struct NamedCommand head_for_desc[];

/******************************************************************************/
unsigned short get_map_location_type(TbMapLocation location);
unsigned long get_map_location_longval(TbMapLocation location);
unsigned long get_map_location_plyrval(TbMapLocation location);
TbBool get_map_location_code_name(TbMapLocation location, char *name);

TbBool get_coords_at_location(struct Coord3d *pos, TbMapLocation location, TbBool random_factor);
TbBool get_coords_at_meta_action(struct Coord3d *pos, PlayerNumber target_plyr_idx, long i);
TbBool get_coords_at_action_point(struct Coord3d *pos, long apt_idx, unsigned char random_factor);
TbBool get_coords_at_hero_door(struct Coord3d *pos, long gate_num, unsigned char random_factor);
TbBool get_coords_at_dungeon_heart(struct Coord3d *pos, PlayerNumber plyr_idx);

TbMapLocation get_coord_encoded_location(MapSubtlCoord stl_x,MapSubtlCoord stl_y);

void find_map_location_coords(TbMapLocation location, MapSubtlCoord *x, MapSubtlCoord *y, int plyr_idx, const char *func_name);

void find_location_pos(TbMapLocation location, PlayerNumber plyr_idx, struct Coord3d *pos, const char *func_name);

#define get_map_location_id(locname, location) get_map_location_id_f(locname, location, __func__, text_line_number)
TbBool get_map_location_id_f(const char *locname, TbMapLocation *location, const char *func_name, long ln_num);

#define get_map_heading_id(headname, target, location) get_map_heading_id_f(headname, target, location, __func__, text_line_number)
TbBool get_map_heading_id_f(const char *headname, long target, TbMapLocation *location, const char *func_name, long ln_num);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
