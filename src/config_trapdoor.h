/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_trapdoor.h
 *     Header file for config_trapdoor.c.
 * @par Purpose:
 *     Traps and doors configuration loading functions.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 21 Dec 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGTRAPDOOR_H
#define DK_CFGTRAPDOOR_H

#include "bflib_basics.h"
#include "globals.h"
#include "engine_camera.h"
#include "config_objects.h"
#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/

#define TRAPDOOR_TYPES_MAX 2000

/******************************************************************************/
struct DoorConfigStats {
    char code_name[COMMAND_WORD_LEN];
    TextStringId name_stridx;
    TextStringId tooltip_stridx;
    long bigsym_sprite_idx;
    long medsym_sprite_idx;
    long pointer_sprite_idx;
    long panel_tab_idx;
    unsigned char manufct_level;
    unsigned long manufct_required;
    HitPoints health;
    unsigned short slbkind[2];
    unsigned short open_speed;
    unsigned short model_flags;
    GoldAmount selling_value;
    TbBool unsellable;
    short place_sound_idx;
    FuncIdx updatefn_idx;
};

/* Contains properties of a door model, to be stored in DoorConfigStats. */
enum DoorModelFlags {
    DoMF_ResistNonMagic = 0x0001,
    DoMF_Secret         = 0x0002,
    DoMF_Thick          = 0x0004,
    DoMF_Midas          = 0x0008,
};

struct TrapConfigStats {
    char code_name[COMMAND_WORD_LEN];
    TextStringId name_stridx;
    TextStringId tooltip_stridx;
    long bigsym_sprite_idx;
    long medsym_sprite_idx;
    long pointer_sprite_idx;
    long panel_tab_idx;
    unsigned char manufct_level;
    unsigned long manufct_required;
    int shots;
    GameTurnDelta shots_delay;
    unsigned short initial_delay; // Trap is placed on reload phase, value in game turns.
    unsigned char trigger_type;
    unsigned char activation_type;
    unsigned short created_itm_model; // Shot model, effect model, slab kind.
    unsigned char activation_level;
    unsigned char hit_type;
    TbBool hidden;
    unsigned char slappable;
    TbBool detect_invisible;
    TbBool notify;
    TbBool place_on_bridge;
    TbBool place_on_subtile;
    TbBool instant_placement;
    TbBool remove_once_depleted;
    HitPoints health;
    char destructible;
    char unstable;
    EffectOrEffElModel destroyed_effect;
    short size_xy;
    short size_z;
    unsigned long sprite_anim_idx;
    unsigned long attack_sprite_anim_idx;
    unsigned long recharge_sprite_anim_idx;
    unsigned long sprite_size_max;
    unsigned long anim_speed;
    unsigned long attack_anim_speed;
    unsigned long recharge_anim_speed;
    unsigned char unanimated;
    unsigned char unshaded;
    unsigned char random_start_frame;
    unsigned char flag_number;
    short light_radius; // Creates light if not null.
    unsigned char light_intensity;
    unsigned char light_flag;
    unsigned char transparency_flag; // Transparency in lower 2 bits.
    unsigned short shot_shift_x;
    unsigned short shot_shift_y;
    unsigned short shot_shift_z;
    struct ComponentVector shotvector;
    struct FlameProperties flame;
    GoldAmount selling_value;
    TbBool unsellable;
    short place_sound_idx;
    short trigger_sound_idx;
    FuncIdx updatefn_idx;
};

/* Manufacture types data. Originally was named TrapData, but stores both traps and doors, now no longer matches original. */
struct ManufactureData {
    ThingClass tngclass; // Thing class created when manufactured design is placed.
    ThingModel tngmodel; // Thing model created when manufactured design is placed.
    int32_t work_state; // Work state used to place the manufactured item on map.
    TextStringId tooltip_stridx;
    int32_t bigsym_sprite_idx;
    int32_t medsym_sprite_idx;
    int32_t panel_tab_idx;
};

struct TrapDoorConfig {
    int32_t trap_types_count;
    struct TrapConfigStats trap_cfgstats[TRAPDOOR_TYPES_MAX];
    int32_t door_types_count;
    struct DoorConfigStats door_cfgstats[TRAPDOOR_TYPES_MAX];
    ThingModel trap_to_object[TRAPDOOR_TYPES_MAX];
    ThingModel door_to_object[TRAPDOOR_TYPES_MAX];
    int32_t manufacture_types_count;
    /* Stores manufacturable items. Was originally named trap_data. */
    struct ManufactureData manufacture_data[2*TRAPDOOR_TYPES_MAX];
};
/******************************************************************************/
extern const struct ConfigFileData keeper_trapdoor_file_data;
extern struct NamedCommand trap_desc[TRAPDOOR_TYPES_MAX];
extern struct NamedCommand door_desc[TRAPDOOR_TYPES_MAX];
extern const struct NamedFieldSet trapdoor_door_named_fields_set;
extern const struct NamedFieldSet trapdoor_trap_named_fields_set;
/******************************************************************************/
struct TrapConfigStats* get_trap_model_stats(int tngmodel);
struct DoorConfigStats *get_door_model_stats(int tngmodel);
struct ManufactureData *get_manufacture_data(int manufctr_idx);
int get_manufacture_data_index_for_thing(ThingClass tngclass, ThingModel tngmodel);

ThingModel door_crate_object_model(ThingModel tngmodel);
ThingModel trap_crate_object_model(ThingModel tngmodel);
const char *door_code_name(int tngmodel);
const char *trap_code_name(int tngmodel);
int door_model_id(const char * code_name);
int trap_model_id(const char * code_name);

TbBool is_trap_placeable(PlayerNumber plyr_idx, long trap_idx);
TbBool is_trap_buildable(PlayerNumber plyr_idx, long trap_idx);
TbBool is_trap_built(PlayerNumber plyr_idx, long tngmodel);
TbBool is_door_placeable(PlayerNumber plyr_idx, long door_idx);
TbBool is_door_buildable(PlayerNumber plyr_idx, long door_idx);
TbBool is_door_built(PlayerNumber plyr_idx, long door_idx);
TbBool create_manufacture_array_from_trapdoor_data(void);
TbBool make_available_all_doors(PlayerNumber plyr_idx);
TbBool make_available_all_traps(PlayerNumber plyr_idx);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
