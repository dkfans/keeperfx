/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_trapdoor.c
 *     Slabs, rooms, traps and doors configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for trap and door elements.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "config_trapdoor.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_fileio.h"
#include "bflib_dernc.h"
#include "bflib_sound.h"

#include "config.h"
#include "config_players.h"
#include "config_strings.h"
#include "console_cmd.h"
#include "custom_sprites.h"
#include "frontmenu_ingame_tabs.h"
#include "game_legacy.h"
#include "player_instances.h"
#include "thing_doors.h"
#include "thing_effects.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct NamedCommand trap_desc[TRAPDOOR_TYPES_MAX];
struct NamedCommand door_desc[TRAPDOOR_TYPES_MAX];

static void refresh_trap_anim(long trap_id);

/******************************************************************************/
static TbBool load_trapdoor_config_file(const char *fname, unsigned short flags);
TbBool create_manufacture_array_from_trapdoor_data(void);

const struct ConfigFileData keeper_trapdoor_file_data = {
    .filename = "trapdoor.cfg",
    .load_func = load_trapdoor_config_file,
    .post_load_func = create_manufacture_array_from_trapdoor_data,
};

static const struct NamedCommand door_properties_commands[] = {
    {"RESIST_NON_MAGIC",     DoMF_ResistNonMagic},
    {"SECRET",               DoMF_Secret},
    {"THICK",                DoMF_Thick},
    {"MIDAS",                DoMF_Midas},
    {NULL,                   0},
  };

static const struct NamedCommand trap_trigger_type_commands[] = {
    {"NONE",             TrpTrg_None},
    {"LINE_OF_SIGHT_90", TrpTrg_LineOfSight90},
    {"PRESSURE_SLAB",    TrpTrg_Pressure_Slab},
    {"LINE_OF_SIGHT",    TrpTrg_LineOfSight},
    {"PRESSURE_SUBTILE", TrpTrg_Pressure_Subtile},
    {"ALWAYS",           TrpTrg_Always},
    {NULL,                   0},
};

static const struct NamedCommand trap_activation_type_commands[] = {
    {"NONE",                TrpAcT_None},
    {"HEAD_FOR_TARGET_90",  TrpAcT_HeadforTarget90},
    {"EFFECT_ON_TRAP",      TrpAcT_EffectonTrap},
    {"SHOT_ON_TRAP",        TrpAcT_ShotonTrap},
    {"SLAB_CHANGE",         TrpAcT_SlabChange},
    {"CREATURE_SHOT",       TrpAcT_CreatureShot},
    {"CREATURE_SPAWN",      TrpAcT_CreatureSpawn},
    {"POWER",               TrpAcT_Power},
    {NULL,                   0},
};

static void assign_panel_tab_idx_trap(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    struct ManufactureData* manufctr = get_manufacture_data(get_manufacture_data_index_for_thing(TCls_Trap, idx));

    manufctr->panel_tab_idx = value;
    assign_default(named_field, value, named_fields_set, idx, src_str, flags);
    if (flag_is_set(flags, ccf_DuringLevel))
    {
        update_trap_tab_to_config();
    }
}

static void assign_panel_tab_idx_door(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    struct ManufactureData* manufctr = get_manufacture_data(get_manufacture_data_index_for_thing(TCls_Door, idx));

    manufctr->panel_tab_idx = value;
    assign_default(named_field, value, named_fields_set, idx, src_str, flags);
    if (flag_is_set(flags, ccf_DuringLevel))
    {
        update_trap_tab_to_config();
    }
}

static void assign_tooltip_idx_trap(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    struct ManufactureData* manufctr = get_manufacture_data(get_manufacture_data_index_for_thing(TCls_Trap, idx));

    manufctr->tooltip_stridx = value;
    assign_default(named_field, value, named_fields_set, idx, src_str, flags);
    if (flag_is_set(flags, ccf_DuringLevel))
    {
        update_trap_tab_to_config();
    }
}

static void assign_tooltip_idx_door(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    struct ManufactureData* manufctr = get_manufacture_data(get_manufacture_data_index_for_thing(TCls_Door, idx));

    manufctr->tooltip_stridx = value;
    assign_default(named_field, value, named_fields_set, idx, src_str, flags);
    if (flag_is_set(flags, ccf_DuringLevel))
    {
        update_trap_tab_to_config();
    }
}

static void assign_icon_update_trap_tab(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    assign_icon(named_field,value,named_fields_set,idx,src_str,flags);
    if (flag_is_set(flags,ccf_DuringLevel))
    {
        update_trap_tab_to_config();
    }
}

static void assign_crate_door(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    game.conf.object_conf.object_to_door_or_trap[value] = idx;
    game.conf.object_conf.workshop_object_class[value] = TCls_Door;
    game.conf.trapdoor_conf.door_to_object[idx] = value;
}

static void assign_update_door_stats(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    if (flag_is_set(flags,ccf_DuringLevel))
    {
        update_all_door_stats();
    }
}

static void assign_crate_trap(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    game.conf.object_conf.object_to_door_or_trap[value] = idx;
    game.conf.object_conf.workshop_object_class[value] = TCls_Trap;
    game.conf.trapdoor_conf.trap_to_object[idx] = value;
}

int64_t value_activationeffect(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if (parameter_is_number(value_text))
    {
        return atoi(value_text);
    }
    else
    {
        long k;
        struct TrapConfigStats* trapst = get_trap_model_stats(idx);
        switch (trapst->activation_type)
        {
            case TrpAcT_EffectonTrap:
                k = get_id(effect_desc, value_text);
                break;
            case TrpAcT_SlabChange:
                k =  get_id(slab_desc, value_text);
                break;
            case TrpAcT_CreatureSpawn:
                k =  get_id(creature_desc, value_text);
                break;
            case TrpAcT_Power:
                k = get_id(power_desc, value_text);
                break;
            case TrpAcT_HeadforTarget90:
            case TrpAcT_ShotonTrap:
            case TrpAcT_CreatureShot:
            default:
                k = get_id(shot_desc, value_text);
                break;
        }
        if (k == -1)
        {
            NAMFIELDWRNLOG("unexpected value '%s' for %s [%s%d].",value_text, named_field->name, named_fields_set->block_basename, idx);
            return 0;
        }
        return k;
    }
}

int64_t value_min1(const struct NamedField* named_field, const char* value_text, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    if (parameter_is_number(value_text))
    {
        return max(0,atoi(value_text) - 1);
    }
    else
    {
        NAMFIELDWRNLOG("unexpected value '%s' for %s [%s%d].",value_text, named_field->name, named_fields_set->block_basename, idx);
        return 0;
    }
}

static void assign_multiple_refresh_trap_anim(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    assign_default(named_field, value, named_fields_set, idx, src_str, flags);
    named_field++;
    assign_default(named_field, value, named_fields_set, idx, src_str, flags);
    named_field++;
    assign_default(named_field, value, named_fields_set, idx, src_str, flags);
    if (flag_is_set(flags, ccf_DuringLevel))
    {
        refresh_trap_anim(idx);
    }
}

static void assign_refresh_trap_anim(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    if (flag_is_set(flags,ccf_DuringLevel))
    {
        refresh_trap_anim(idx);
    }
}

static void assign_refresh_trap_anim_anim_id(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    assign_animid(named_field,value,named_fields_set,idx,src_str,flags);
    if (flag_is_set(flags,ccf_DuringLevel))
    {
        refresh_trap_anim(idx);
    }
}

const struct NamedField trapdoor_door_named_fields[] = {
    //name           //pos    //field                                                                //default //min     //max    //NamedCommand
    {"NAME",                 0, field(game.conf.trapdoor_conf.door_cfgstats[0].code_name),                0,   LONG_MIN,         UINT32_MAX, door_desc,                value_name,       assign_null},
    {"NAMETEXTID",           0, field(game.conf.trapdoor_conf.door_cfgstats[0].name_stridx),   GUIStr_Empty,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_default},
    {"TOOLTIPTEXTID",        0, field(game.conf.trapdoor_conf.door_cfgstats[0].tooltip_stridx),GUIStr_Empty,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_tooltip_idx_door},
    {"SYMBOLSPRITES",        0, field(game.conf.trapdoor_conf.door_cfgstats[0].bigsym_sprite_idx),        0,   LONG_MIN,         UINT32_MAX, NULL,                     value_icon,       assign_icon},
    {"SYMBOLSPRITES",        1, field(game.conf.trapdoor_conf.door_cfgstats[0].medsym_sprite_idx),        0,   LONG_MIN,         UINT32_MAX, NULL,                     value_icon,       assign_icon_update_trap_tab},
    {"POINTERSPRITES",       0, field(game.conf.trapdoor_conf.door_cfgstats[0].pointer_sprite_idx),       0,   LONG_MIN,         UINT32_MAX, NULL,                     value_icon,       assign_icon_update_trap_tab},
    {"PANELTABINDEX",        0, field(game.conf.trapdoor_conf.door_cfgstats[0].panel_tab_idx),            0,          0,                32, NULL,                     value_default,    assign_panel_tab_idx_door},
    {"CRATE",                0, NULL,0,                                                                   0,   LONG_MIN,         UINT32_MAX, object_desc,              value_default,    assign_crate_door},
    {"MANUFACTURELEVEL",     0, field(game.conf.trapdoor_conf.door_cfgstats[0].manufct_level),            0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_default},
    {"MANUFACTUREREQUIRED",  0, field(game.conf.trapdoor_conf.door_cfgstats[0].manufct_required),         0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_default},
    {"HEALTH",               0, field(game.conf.trapdoor_conf.door_cfgstats[0].health),                   1,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_update_door_stats},
    {"SLABKIND",             0, field(game.conf.trapdoor_conf.door_cfgstats[0].slbkind[1]),               0,          0, TERRAIN_ITEMS_MAX, slab_desc,                value_default,    assign_default},
    {"SLABKIND",             0, field(game.conf.trapdoor_conf.door_cfgstats[0].slbkind[0]),               0,          0, TERRAIN_ITEMS_MAX, slab_desc,                value_default,    assign_update_door_stats},
    {"OPENSPEED",            0, field(game.conf.trapdoor_conf.door_cfgstats[0].open_speed),             256,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_default},
    {"PROPERTIES",          -1, field(game.conf.trapdoor_conf.door_cfgstats[0].model_flags),              0,   LONG_MIN,         UINT32_MAX, door_properties_commands, value_flagsfield, assign_default},
    {"SELLINGVALUE",         0, field(game.conf.trapdoor_conf.door_cfgstats[0].selling_value),            0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_default},
    {"UNSELLABLE",           0, field(game.conf.trapdoor_conf.door_cfgstats[0].unsellable),               0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_default},
    {"PLACESOUND",           0, field(game.conf.trapdoor_conf.door_cfgstats[0].place_sound_idx),        117,   LONG_MIN,         UINT32_MAX, NULL,                     value_default,    assign_default},
    {"UPDATEFUNCTION",       0, field(game.conf.trapdoor_conf.door_cfgstats[0].updatefn_idx),             0,   LONG_MIN,         UINT32_MAX, NULL,                     value_function,   assign_default},
    {NULL},
};

const struct NamedFieldSet trapdoor_door_named_fields_set = {
    &game.conf.trapdoor_conf.door_types_count,
    "door",
    trapdoor_door_named_fields,
    door_desc,
    TRAPDOOR_TYPES_MAX,
    sizeof(game.conf.trapdoor_conf.door_cfgstats[0]),
    game.conf.trapdoor_conf.door_cfgstats,
};

const struct NamedField trapdoor_trap_named_fields[] = {
    //name           //pos    //field                                                                //default //min     //max    //NamedCommand
    {"NAME",                   0, field(game.conf.trapdoor_conf.trap_cfgstats[0].code_name),                        0,   LONG_MIN,         UINT32_MAX, door_desc,                value_name,    assign_null},
    {"MANUFACTURELEVEL",       0, field(game.conf.trapdoor_conf.trap_cfgstats[0].manufct_level),                    0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"MANUFACTUREREQUIRED",    0, field(game.conf.trapdoor_conf.trap_cfgstats[0].manufct_required),                 0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"SHOTS",                  0, field(game.conf.trapdoor_conf.trap_cfgstats[0].shots),                            0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"TIMEBETWEENSHOTS",       0, field(game.conf.trapdoor_conf.trap_cfgstats[0].shots_delay),                      0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"SELLINGVALUE",           0, field(game.conf.trapdoor_conf.trap_cfgstats[0].selling_value),                    0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"NAMETEXTID",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].name_stridx),                      0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"TOOLTIPTEXTID",          0, field(game.conf.trapdoor_conf.trap_cfgstats[0].tooltip_stridx),                   0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_tooltip_idx_trap},
    {"CRATE",                  0, NULL,0,                                                                           0,   LONG_MIN,         UINT32_MAX, object_desc,              value_default, assign_crate_trap},
    {"SYMBOLSPRITES",          0, field(game.conf.trapdoor_conf.trap_cfgstats[0].bigsym_sprite_idx),                0,   LONG_MIN,         UINT32_MAX, NULL,                        value_icon, assign_icon},
    {"SYMBOLSPRITES",          1, field(game.conf.trapdoor_conf.trap_cfgstats[0].medsym_sprite_idx),                0,   LONG_MIN,         UINT32_MAX, NULL,                        value_icon, assign_icon_update_trap_tab},
    {"POINTERSPRITES",         0, field(game.conf.trapdoor_conf.trap_cfgstats[0].pointer_sprite_idx),               0,   LONG_MIN,         UINT32_MAX, NULL,                        value_icon, assign_icon_update_trap_tab},
    {"PANELTABINDEX",          0, field(game.conf.trapdoor_conf.trap_cfgstats[0].panel_tab_idx),                    0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_panel_tab_idx_trap},
    {"TRIGGERTYPE",            0, field(game.conf.trapdoor_conf.trap_cfgstats[0].trigger_type),                     0,   LONG_MIN,         UINT32_MAX, trap_trigger_type_commands,value_default, assign_default},
    {"ACTIVATIONTYPE",         0, field(game.conf.trapdoor_conf.trap_cfgstats[0].activation_type),                  0,   LONG_MIN,         UINT32_MAX, trap_activation_type_commands,value_default, assign_default},
    {"EFFECTTYPE",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].created_itm_model),                0,   LONG_MIN,         UINT32_MAX, NULL,            value_activationeffect, assign_default},
    {"ACTIVATIONLEVEL",        0, field(game.conf.trapdoor_conf.trap_cfgstats[0].activation_level),                 0,          0,                 9, NULL,                        value_min1, assign_default},
    {"ANIMATIONID",            0, field(game.conf.trapdoor_conf.trap_cfgstats[0].sprite_anim_idx),                  0,   LONG_MIN,         UINT32_MAX, NULL,                      value_animid, assign_refresh_trap_anim_anim_id},
    {"MODEL",                  0, field(game.conf.trapdoor_conf.trap_cfgstats[0].sprite_anim_idx),                  0,   LONG_MIN,         UINT32_MAX, NULL,                      value_animid, assign_refresh_trap_anim_anim_id}, // Backward compatibility.
    {"RECHARGEANIMATIONID",    0, field(game.conf.trapdoor_conf.trap_cfgstats[0].recharge_sprite_anim_idx),         0,   LONG_MIN,         UINT32_MAX, NULL,                      value_animid, assign_refresh_trap_anim_anim_id},
    {"ATTACKANIMATIONID",      0, field(game.conf.trapdoor_conf.trap_cfgstats[0].attack_sprite_anim_idx),           0,   LONG_MIN,         UINT32_MAX, NULL,                      value_animid, assign_animid},
    {"MODELSIZE",              0, field(game.conf.trapdoor_conf.trap_cfgstats[0].sprite_size_max),                  0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_refresh_trap_anim},
    {"ANIMATIONSPEED",         0, field(game.conf.trapdoor_conf.trap_cfgstats[0].anim_speed),                       0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_multiple_refresh_trap_anim},
    {"ATTACKANIMATIONSPEED",   0, field(game.conf.trapdoor_conf.trap_cfgstats[0].attack_anim_speed),                0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_refresh_trap_anim},
    {"RECHARGEANIMATIONSPEED", 0, field(game.conf.trapdoor_conf.trap_cfgstats[0].recharge_anim_speed),              0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_refresh_trap_anim},
    {"UNANIMATED",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].unanimated),                       0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_refresh_trap_anim},
    {"HIDDEN",                 0, field(game.conf.trapdoor_conf.trap_cfgstats[0].hidden),                        true,          0,                 1, NULL,                     value_default, assign_default},
    {"SLAPPABLE",              0, field(game.conf.trapdoor_conf.trap_cfgstats[0].slappable),                        0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"TRIGGERALARM",           0, field(game.conf.trapdoor_conf.trap_cfgstats[0].notify),                           0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"HEALTH",                 0, field(game.conf.trapdoor_conf.trap_cfgstats[0].health),                           1,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"UNSHADED",               0, field(game.conf.trapdoor_conf.trap_cfgstats[0].unshaded),                         0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"RANDOMSTARTFRAME",       0, field(game.conf.trapdoor_conf.trap_cfgstats[0].random_start_frame),               0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"THINGSIZE",              0, field(game.conf.trapdoor_conf.trap_cfgstats[0].size_xy),                          0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"THINGSIZE",              1, field(game.conf.trapdoor_conf.trap_cfgstats[0].size_z),                           0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"HITTYPE",                0, field(game.conf.trapdoor_conf.trap_cfgstats[0].hit_type),                         0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"LIGHTRADIUS",            0, field(game.conf.trapdoor_conf.trap_cfgstats[0].light_radius),                     0,   LONG_MIN,         UINT32_MAX, NULL,                  value_stltocoord, assign_default},
    {"LIGHTINTENSITY",         0, field(game.conf.trapdoor_conf.trap_cfgstats[0].light_intensity),                  0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"LIGHTFLAGS",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].light_flag),                       0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"TRANSPARENCYFLAGS",      0, field(game.conf.trapdoor_conf.trap_cfgstats[0].transparency_flag),                0,   LONG_MIN,         UINT32_MAX, NULL,                   value_transpflg, assign_default},
    {"SHOTVECTOR",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].shotvector.x),                     0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"SHOTVECTOR",             1, field(game.conf.trapdoor_conf.trap_cfgstats[0].shotvector.y),                     0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"SHOTVECTOR",             2, field(game.conf.trapdoor_conf.trap_cfgstats[0].shotvector.z),                     0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"DESTRUCTIBLE",           0, field(game.conf.trapdoor_conf.trap_cfgstats[0].destructible),                     0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"UNSTABLE",               0, field(game.conf.trapdoor_conf.trap_cfgstats[0].unstable),                         0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"UNSELLABLE",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].unsellable),                       0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"PLACEONBRIDGE",          0, field(game.conf.trapdoor_conf.trap_cfgstats[0].place_on_bridge),                  0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"SHOTORIGIN",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].shot_shift_x),                     0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"SHOTORIGIN",             1, field(game.conf.trapdoor_conf.trap_cfgstats[0].shot_shift_y),                     0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"SHOTORIGIN",             2, field(game.conf.trapdoor_conf.trap_cfgstats[0].shot_shift_z),                     0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"PLACESOUND",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].place_sound_idx),                117,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"TRIGGERSOUND",           0, field(game.conf.trapdoor_conf.trap_cfgstats[0].trigger_sound_idx),              176,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"DESTROYEDEFFECT",        0, field(game.conf.trapdoor_conf.trap_cfgstats[0].destroyed_effect), -TngEffElm_Blast2,   LONG_MIN,         UINT32_MAX, NULL,                  value_effOrEffEl, assign_default},
    {"INITIALDELAY",           0, field(game.conf.trapdoor_conf.trap_cfgstats[0].initial_delay),                    0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"PLACEONSUBTILE",         0, field(game.conf.trapdoor_conf.trap_cfgstats[0].place_on_subtile),                 0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"FLAMEANIMATIONID",       0, field(game.conf.trapdoor_conf.trap_cfgstats[0].flame.animation_id),               0,   LONG_MIN,         UINT32_MAX, NULL,                      value_animid, assign_refresh_trap_anim_anim_id},
    {"FLAMEANIMATIONSPEED",    0, field(game.conf.trapdoor_conf.trap_cfgstats[0].flame.anim_speed),                 0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"FLAMEANIMATIONSIZE",     0, field(game.conf.trapdoor_conf.trap_cfgstats[0].flame.sprite_size),                0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"FLAMEANIMATIONOFFSET",   0, field(game.conf.trapdoor_conf.trap_cfgstats[0].flame.fp_add_x),                   0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"FLAMEANIMATIONOFFSET",   1, field(game.conf.trapdoor_conf.trap_cfgstats[0].flame.fp_add_y),                   0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"FLAMEANIMATIONOFFSET",   2, field(game.conf.trapdoor_conf.trap_cfgstats[0].flame.td_add_x),                   0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"FLAMEANIMATIONOFFSET",   3, field(game.conf.trapdoor_conf.trap_cfgstats[0].flame.td_add_y),                   0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"FLAMETRANSPARENCYFLAGS", 0, field(game.conf.trapdoor_conf.trap_cfgstats[0].flame.transparency_flags),         0,   LONG_MIN,         UINT32_MAX, NULL,                   value_transpflg, assign_default},
    {"DETECTINVISIBLE",        0, field(game.conf.trapdoor_conf.trap_cfgstats[0].detect_invisible),              true,          0,                 1, NULL,                     value_default, assign_default},
    {"INSTANTPLACEMENT",       0, field(game.conf.trapdoor_conf.trap_cfgstats[0].instant_placement),                0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"REMOVEONCEDEPLETED",     0, field(game.conf.trapdoor_conf.trap_cfgstats[0].remove_once_depleted),             0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"FLAGNUMBER",             0, field(game.conf.trapdoor_conf.trap_cfgstats[0].flag_number),                      0,   LONG_MIN,         UINT32_MAX, NULL,                     value_default, assign_default},
    {"UPDATEFUNCTION",         0, field(game.conf.trapdoor_conf.trap_cfgstats[0].updatefn_idx),                     0,   LONG_MIN,         UINT32_MAX, NULL,                     value_function,assign_default},
    {NULL},
};

const struct NamedFieldSet trapdoor_trap_named_fields_set = {
    &game.conf.trapdoor_conf.trap_types_count,
    "trap",
    trapdoor_trap_named_fields,
    trap_desc,
    TRAPDOOR_TYPES_MAX,
    sizeof(game.conf.trapdoor_conf.trap_cfgstats[0]),
    game.conf.trapdoor_conf.trap_cfgstats,
};

/******************************************************************************/
struct TrapConfigStats *get_trap_model_stats(int tngmodel)
{
    if (tngmodel >= game.conf.trapdoor_conf.trap_types_count)
        return &game.conf.trapdoor_conf.trap_cfgstats[0];
    return &game.conf.trapdoor_conf.trap_cfgstats[tngmodel];
}

struct DoorConfigStats *get_door_model_stats(int tngmodel)
{
    if (tngmodel >= game.conf.trapdoor_conf.door_types_count)
        return &game.conf.trapdoor_conf.door_cfgstats[0];
    return &game.conf.trapdoor_conf.door_cfgstats[tngmodel];
}

/**
 * Returns manufacture data for a given manufacture index.
 * @param manufctr_idx Manufacture array index.
 * @return Dummy entry pinter if not found, manufacture data pointer otherwise.
 */
struct ManufactureData *get_manufacture_data(int manufctr_idx)
{
    if ((manufctr_idx < 0) || (manufctr_idx >= game.conf.trapdoor_conf.manufacture_types_count)) {
        return &game.conf.trapdoor_conf.manufacture_data[0];
    }
    return &game.conf.trapdoor_conf.manufacture_data[manufctr_idx];
}

/**
 * Finds index into manufactures data array for a given trap/door class and model.
 * @param tngclass Manufacturable thing class.
 * @param tngmodel Manufacturable thing model.
 * @return 0 if not found, otherwise index where 1 <= index < manufacture_types_count
 */
int get_manufacture_data_index_for_thing(ThingClass tngclass, ThingModel tngmodel)
{
    for (int i = 1; i < game.conf.trapdoor_conf.manufacture_types_count; i++)
    {
        struct ManufactureData* manufctr = &game.conf.trapdoor_conf.manufacture_data[i];
        if ((manufctr->tngclass == tngclass) && (manufctr->tngmodel == tngmodel)) {
            return i;
        }
    }
    return 0;
}

static TbBool load_trapdoor_config_file(const char *fname, unsigned short flags)
{
    SYNCDBG(0,"%s file \"%s\".",((flags & CnfLd_ListOnly) == 0)?"Reading":"Parsing",fname);
    long len = LbFileLengthRnc(fname);
    if (len < MIN_CONFIG_FILE_SIZE)
    {
        if ((flags & CnfLd_IgnoreErrors) == 0)
            WARNMSG("file \"%s\" doesn't exist or is too small.",fname);
        return false;
    }
    char* buf = (char*)calloc(len + 256, 1);
    if (buf == NULL)
        return false;

    if ((flags & CnfLd_AcceptPartial) == 0)
    {
        for (int i = 0; i < TRAPDOOR_TYPES_MAX; i++)
        {
            game.conf.object_conf.object_to_door_or_trap[i] = 0;
        }
    }

    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file
    if (result)
    {
        parse_named_field_blocks(buf, len, fname, flags, &trapdoor_trap_named_fields_set);
        parse_named_field_blocks(buf, len, fname, flags, &trapdoor_door_named_fields_set);
    }
    //Freeing and exiting
    free(buf);
    SYNCDBG(19,"Done");
    return result;
}

TbBool create_manufacture_array_from_trapdoor_data(void)
{
    int i;
    struct ManufactureData *manufctr;
    // Initialize the manufacture array
    game.conf.trapdoor_conf.manufacture_types_count = 0;
    int arr_size = sizeof(game.conf.trapdoor_conf.manufacture_data) / sizeof(game.conf.trapdoor_conf.manufacture_data[0]);
    for (i=0; i < arr_size; i++)
    {
        manufctr = &game.conf.trapdoor_conf.manufacture_data[i];
        manufctr->tngclass = TCls_Empty;
        manufctr->tngmodel = 0;
        manufctr->work_state = PSt_None;
        manufctr->tooltip_stridx = GUIStr_Empty;
        manufctr->bigsym_sprite_idx = 0;
        manufctr->medsym_sprite_idx = 0;
        manufctr->panel_tab_idx = 0;
    }
    // Let manufacture 0 be empty
    game.conf.trapdoor_conf.manufacture_types_count++;
    // Fill manufacture entries
    for (i=1; i < game.conf.trapdoor_conf.trap_types_count; i++)
    {
        struct TrapConfigStats* trapst = get_trap_model_stats(i);
        manufctr = &game.conf.trapdoor_conf.manufacture_data[game.conf.trapdoor_conf.manufacture_types_count];
        manufctr->tngclass = TCls_Trap;
        manufctr->tngmodel = i;
        manufctr->work_state = PSt_PlaceTrap;
        manufctr->tooltip_stridx = trapst->tooltip_stridx;
        manufctr->bigsym_sprite_idx = trapst->bigsym_sprite_idx;
        manufctr->medsym_sprite_idx = trapst->medsym_sprite_idx;
        manufctr->panel_tab_idx = trapst->panel_tab_idx;
        game.conf.trapdoor_conf.manufacture_types_count++;
    }
    for (i=1; i < game.conf.trapdoor_conf.door_types_count; i++)
    {
        struct DoorConfigStats* doorst = get_door_model_stats(i);
        manufctr = &game.conf.trapdoor_conf.manufacture_data[game.conf.trapdoor_conf.manufacture_types_count];
        manufctr->tngclass = TCls_Door;
        manufctr->tngmodel = i;
        manufctr->work_state = PSt_PlaceDoor;
        manufctr->tooltip_stridx = doorst->tooltip_stridx;
        manufctr->bigsym_sprite_idx = doorst->bigsym_sprite_idx;
        manufctr->medsym_sprite_idx = doorst->medsym_sprite_idx;
        manufctr->panel_tab_idx = doorst->panel_tab_idx;
        game.conf.trapdoor_conf.manufacture_types_count++;
    }
    return true;
}

ThingModel door_crate_object_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= TRAPDOOR_TYPES_MAX))
        return game.conf.trapdoor_conf.door_to_object[0];
    return game.conf.trapdoor_conf.door_to_object[tngmodel];

}

ThingModel trap_crate_object_model(ThingModel tngmodel)
{
    if ((tngmodel <= 0) || (tngmodel >= TRAPDOOR_TYPES_MAX))
        return game.conf.trapdoor_conf.trap_to_object[0];
    return game.conf.trapdoor_conf.trap_to_object[tngmodel];
}

/**
 * Returns Code Name (name to use in script file) of given door model.
 */
const char *door_code_name(int tngmodel)
{
    const char* name = get_conf_parameter_text(door_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given trap model.
 */
const char *trap_code_name(int tngmodel)
{
    const char* name = get_conf_parameter_text(trap_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the door model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the door model if found, otherwise -1
 */
int door_model_id(const char * code_name)
{
    for (int i = 0; i < game.conf.trapdoor_conf.door_types_count; ++i)
    {
        if (strncasecmp(game.conf.trapdoor_conf.door_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Returns the trap model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the trap model if found, otherwise -1
 */
int trap_model_id(const char * code_name)
{
    for (int i = 0; i < game.conf.trapdoor_conf.trap_types_count; ++i)
    {
        if (strncasecmp(game.conf.trapdoor_conf.trap_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Returns if the trap can be placed by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if map position is on correct spot.
 */
TbBool is_trap_placeable(PlayerNumber plyr_idx, long tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to place traps
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= game.conf.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if (dungeon->mnfct_info.trap_amount_placeable[tngmodel] > 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the trap can be manufactured by a player.
 * Checks only if it's set as buildable in level script.
 * Doesn't check if player has workshop or workforce for the task.
 */
TbBool is_trap_buildable(PlayerNumber plyr_idx, long tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= game.conf.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if ((dungeon->mnfct_info.trap_build_flags[tngmodel] & MnfBldF_Manufacturable) != 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the trap was at least once built by a player.
 */
TbBool is_trap_built(PlayerNumber plyr_idx, long tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= game.conf.trapdoor_conf.trap_types_count)) {
        ERRORLOG("Incorrect trap %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if ((dungeon->mnfct_info.trap_build_flags[tngmodel] & MnfBldF_Built) != 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the door can be placed by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if map position is on correct spot.
 */
TbBool is_door_placeable(PlayerNumber plyr_idx, long tngmodel)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to place doors
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((tngmodel <= 0) || (tngmodel >= game.conf.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)tngmodel, (int)plyr_idx);
        return false;
    }
    if (dungeon->mnfct_info.door_amount_placeable[tngmodel] > 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the door can be manufactured by a player.
 * Checks only if it's set as buildable in level script.
 * Doesn't check if player has workshop or workforce for the task.
 */
TbBool is_door_buildable(PlayerNumber plyr_idx, long door_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((door_idx <= 0) || (door_idx >= game.conf.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)door_idx, (int)plyr_idx);
        return false;
    }
    if ((dungeon->mnfct_info.door_build_flags[door_idx] & MnfBldF_Manufacturable) != 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the door was at least one built by a player.
 */
TbBool is_door_built(PlayerNumber plyr_idx, long door_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    // Player must have dungeon heart to build anything
    if (!player_has_heart(plyr_idx)) {
        return false;
    }
    if ((door_idx <= 0) || (door_idx >= game.conf.trapdoor_conf.door_types_count)) {
        ERRORLOG("Incorrect door %d (player %d)",(int)door_idx, (int)plyr_idx);
        return false;
    }
    if ((dungeon->mnfct_info.door_build_flags[door_idx] & MnfBldF_Built) != 0) {
        return true;
    }
    return false;
}

/**
 * Makes all door types manufacturable.
 */
TbBool make_available_all_doors(PlayerNumber plyr_idx)
{
  SYNCDBG(0,"Starting");
  struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon)) {
      ERRORDBG(11,"Cannot make doors available; player %d has no dungeon",(int)plyr_idx);
      return false;
  }
  for (long i = 1; i < game.conf.trapdoor_conf.door_types_count; i++)
  {
    if (!set_door_buildable_and_add_to_amount(plyr_idx, i, 1, 0))
    {
        ERRORLOG("Could not make door %s available for player %d", door_code_name(i), plyr_idx);
        return false;
    }
  }
  return true;
}

/**
 * Makes all trap types manufacturable.
 */
TbBool make_available_all_traps(PlayerNumber plyr_idx)
{
  SYNCDBG(0,"Starting");
  struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon)) {
      ERRORDBG(11,"Cannot make traps available; player %d has no dungeon",(int)plyr_idx);
      return false;
  }
  for (long i = 1; i < game.conf.trapdoor_conf.trap_types_count; i++)
  {
    if (!set_trap_buildable_and_add_to_amount(plyr_idx, i, 1, 0))
    {
        ERRORLOG("Could not make trap %s available for player %d", trap_code_name(i), plyr_idx);
        return false;
    }
  }
  return true;
}

static void refresh_trap_anim(long trap_id)
{
    int k = 0;
    const struct StructureList* slist = get_list_for_thing_class(TCls_Trap);
    struct TrapConfigStats *trapst_old = get_trap_model_stats(trap_id);
    struct TrapConfigStats *trapst_new;
    long correct_anim_speed;
    int i = slist->index;
    while (i != 0)
    {
        struct Thing* traptng = thing_get(i);
        if (thing_is_invalid(traptng))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = traptng->next_of_class;
        // Per thing code.
        if (traptng->model == trap_id)
        {
            if ((traptng->trap.wait_for_rearm == true) || (trapst_old->recharge_sprite_anim_idx == 0))
            {
                traptng->anim_sprite = trapst_old->sprite_anim_idx;
                correct_anim_speed = trapst_old->anim_speed;
            }
            else
            {
                traptng->anim_sprite = trapst_old->recharge_sprite_anim_idx;
                correct_anim_speed = trapst_old->recharge_anim_speed;
            }
            trapst_new = get_trap_model_stats(traptng->model);
            char start_frame;
            if (trapst_new->random_start_frame) {
                start_frame = -1;
            }
            else {
                start_frame = 0;
            }
            set_thing_draw(traptng, trapst_new->sprite_anim_idx, correct_anim_speed, trapst_new->sprite_size_max, trapst_new->unanimated, start_frame, ODC_Default);
        }
        // Per thing code ends.
        k++;
        if (k > slist->index)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break;
        }
    }
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
