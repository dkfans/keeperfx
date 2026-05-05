/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_magic.c
 *     Keeper and creature spells configuration loading functions.
 * @par Purpose:
 *     Support of configuration files for magic spells.
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
#include "config_magic.h"
#include "globals.h"

#include "bflib_basics.h"
#include "bflib_dernc.h"
#include "config.h"
#include "config_creature.h"
#include "config_crtrmodel.h"
#include "config_cubes.h"
#include "config_effects.h"
#include "config_effects.h"
#include "config_objects.h"
#include "config_objects.h"
#include "config_players.h"
#include "config_rules.h"
#include "console_cmd.h"
#include "custom_sprites.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "power_process.h"
#include "thing_creature.h"
#include "thing_effects.h"
#include "thing_physics.h"
#include "thing_shots.h"

#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
static TbBool load_magic_config_file(const char *fname, unsigned short flags);

const struct ConfigFileData keeper_magic_file_data = {
    .filename = "magic.cfg",
    .load_func = load_magic_config_file,
    .pre_load_func = NULL,
    .post_load_func = NULL,
};

const struct NamedCommand magic_spell_commands[] = {
    {"NAME",             1},
    {"DURATION",         2},
    {"SELFCASTED",       3},
    {"CASTATTHING",      4},
    {"SHOTMODEL",        5},
    {"EFFECTMODEL",      6},
    {"SYMBOLSPRITES",    7},
    {"SPELLPOWER",       8},
    {"AURAEFFECT",       9},
    {"SPELLFLAGS",      10},
    {"SUMMONCREATURE",  11},
    {"COUNTDOWN",       12},
    {"HEALINGRECOVERY", 13},
    {"DAMAGE",          14},
    {"DAMAGEFREQUENCY", 15},
    {"AURADURATION",    16},
    {"AURAFREQUENCY",   17},
    {"CLEANSEFLAGS",    18},
    {"PROPERTIES",      19},
    {NULL,               0},
};

const struct NamedCommand spell_effect_flags[] = {
    {"SLOW",          CSAfF_Slow},
    {"SPEED",         CSAfF_Speed},
    {"ARMOUR",        CSAfF_Armour},
    {"REBOUND",       CSAfF_Rebound},
    {"FLYING",        CSAfF_Flying},
    {"INVISIBILITY",  CSAfF_Invisibility},
    {"SIGHT",         CSAfF_Sight},
    {"LIGHT",         CSAfF_Light},
    {"DISEASE",       CSAfF_Disease},
    {"CHICKEN",       CSAfF_Chicken},
    {"POISON_CLOUD",  CSAfF_PoisonCloud},
    {"FREEZE",        CSAfF_Freeze},
    {"MAD_KILLING",   CSAfF_MadKilling},
    {"FEAR",          CSAfF_Fear},
    {"HEAL",          CSAfF_Heal},
    {"TELEPORT",      CSAfF_Teleport},
    {"TIMEBOMB",      CSAfF_Timebomb},
    {"WIND",          CSAfF_Wind},
    {"SPELL_BLOCKS",  CSAfF_SpellBlocks},
    {NULL,            0},
};

const struct NamedCommand magic_spell_properties[] = {
    {"FIXED_DAMAGE",    1},
    {"PERCENT_BASED",   2},
    {"MAX_HEALTH",      3},
    {NULL,              0},
};

static const struct NamedCommand shotmodel_properties_commands[] = {
  {"SLAPPABLE",           ShMF_Slappable       },
  {"NAVIGABLE",           ShMF_Navigable       },
  {"BOULDER",             ShMF_Boulder         },
  {"REBOUND_IMMUNE",      ShMF_ReboundImmune   },
  {"DIGGING",             ShMF_Digging         },
  {"LIFE_DRAIN",          ShMF_LifeDrain       },
  {"GROUP_UP",            ShMF_GroupUp         },
  {"NO_STUN",             ShMF_NoStun          },
  {"NO_HIT",              ShMF_NoHit           },
  {"STRENGTH_BASED",      ShMF_StrengthBased   },
  {"ALARMS_UNITS",        ShMF_AlarmsUnits     },
  {"CAN_COLLIDE",         ShMF_CanCollide      },
  {"EXPLODE_FLESH",       ShMF_Exploding       },
  {"NO_AIR_DAMAGE",       ShMF_NoAirDamage     },
  {"WIND_IMMUNE",         ShMF_WindImmune      },
  {"FIXED_DAMAGE",        ShMF_FixedDamage     },
  {"HIDDEN_PROJECTILE",   ShMF_HiddenProjectile},
  {"DISARMING",           ShMF_Disarming       },
  {"BLOCKS_REBIRTH",      ShMF_BlocksRebirth   },
  {"PENETRATING",         ShMF_Penetrating     },
  {"NEVER_BLOCK",         ShMF_NeverBlock      },
  {"WALL_PIERCE",         ShMF_WallPierce      },
  {NULL,                  0},
};

enum ShotHitWithstandFlags {
    ShHW_Creature = 0x1,
    ShHW_Wall = 0x2,
    ShHW_Door = 0x4,
    ShHW_Water = 0x8,
    ShHW_Lava = 0x10,
    ShHW_Dig = 0x20,
};

static const struct NamedCommand shotmodel_withstand_types[] = {
  {"CREATURE",      ShHW_Creature},
  {"WALL",          ShHW_Wall},
  {"DOOR",          ShHW_Door},
  {"WATER",         ShHW_Water},
  {"LAVA",          ShHW_Lava},
  {"DIG",           ShHW_Dig},
  {NULL,            0},
};

static void assign_withstand(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    struct ShotConfigStats* shotst = get_shot_model_stats(idx);

    shotst->hit_creature.withstand = (value & ShHW_Creature) ? 1 : 0;
    shotst->hit_generic.withstand  = (value & ShHW_Wall) ? 1 : 0;
    shotst->hit_door.withstand     = (value & ShHW_Door) ? 1 : 0;
    shotst->hit_water.withstand    = (value & ShHW_Water) ? 1 : 0;
    shotst->hit_lava.withstand     = (value & ShHW_Lava) ? 1 : 0;
    shotst->dig.withstand          = (value & ShHW_Dig) ? 1 : 0;
}


static const struct NamedCommand shotfirelogic_commands[] = {
    {"DEFAULT",      ShFL_Default},
    {"0",            ShFL_Default},
    {"BEAM",         ShFL_Beam},
    {"1",            ShFL_Beam},
    {"BREATHE",      ShFL_Breathe},
    {"2",            ShFL_Breathe},
    {"HAIL",         ShFL_Hail},
    {"3",            ShFL_Hail},
    {"LIZARD",       ShFL_Lizard},
    {"4",            ShFL_Lizard},
    {"VOLLEY",       ShFL_Volley},
    {"5",            ShFL_Volley},
    {NULL,            0},
};

static const struct NamedCommand shotupdatelogic_commands[] = {
    {"DEFAULT",      ShUL_Default},
    {"0",            ShUL_Default},
    {"LIGHTNING",    ShUL_Lightning},
    {"1",            ShUL_Lightning},
    {"WIND",         ShUL_Wind},
    {"2",            ShUL_Wind},
    {"GRENADE",      ShUL_Grenade},
    {"3",            ShUL_Grenade},
    {"GODLIGHTNING", ShUL_GodLightning},
    {"4",            ShUL_GodLightning},
    {"LIZARD",       ShUL_Lizard},
    {"5",            ShUL_Lizard},
    {"GODLIGHTBALL", ShUL_GodLightBall},
    {"6",            ShUL_GodLightBall},
    {"TRAPTNT",      ShUL_TrapTNT},
    {"7",            ShUL_TrapTNT},
    {"TRAPLIGHTNING",ShUL_TrapLightning},
    {"8",            ShUL_TrapLightning},
    {NULL,            0},
};

const struct NamedField magic_shot_named_fields[] = {
  {"NAME",                  0, field(game.conf.magic_conf.shot_cfgstats[0].code_name),                   0,  INT32_MIN,  UINT32_MAX, shot_desc,                    value_name,      assign_null},
  {"HEALTH",                0, field(game.conf.magic_conf.shot_cfgstats[0].health),                      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"DAMAGE",                0, field(game.conf.magic_conf.shot_cfgstats[0].damage),                      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"ISMAGICAL",             0, field(game.conf.magic_conf.shot_cfgstats[0].is_magical),                  0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITTYPE",               0, field(game.conf.magic_conf.shot_cfgstats[0].area_hit_type),  THit_CrtrsOnly,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"AREADAMAGE",            0, field(game.conf.magic_conf.shot_cfgstats[0].area_range),                  0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"AREADAMAGE",            1, field(game.conf.magic_conf.shot_cfgstats[0].area_damage),                 0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"AREADAMAGE",            2, field(game.conf.magic_conf.shot_cfgstats[0].area_blow),                   0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SPEED",                 0, field(game.conf.magic_conf.shot_cfgstats[0].speed),                       0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"PROPERTIES",           -1, field(game.conf.magic_conf.shot_cfgstats[0].model_flags),                 0,  INT32_MIN,  UINT32_MAX, shotmodel_properties_commands,value_flagsfield,assign_default},
  {"PUSHONHIT",             0, field(game.conf.magic_conf.shot_cfgstats[0].push_on_hit),                 0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"FIRINGSOUND",           0, field(game.conf.magic_conf.shot_cfgstats[0].firing_sound),                0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SHOTSOUND",             0, field(game.conf.magic_conf.shot_cfgstats[0].shot_sound),                  0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"FIRINGSOUNDVARIANTS",   0, field(game.conf.magic_conf.shot_cfgstats[0].firing_sound_variants),       0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"MAXRANGE",              0, field(game.conf.magic_conf.shot_cfgstats[0].max_range),                   0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"ANIMATION",             0, field(game.conf.magic_conf.shot_cfgstats[0].sprite_anim_idx),             0,  INT32_MIN,  UINT32_MAX, NULL,                         value_animid,    assign_animid},
  {"ANIMATIONSIZE",         0, field(game.conf.magic_conf.shot_cfgstats[0].sprite_size_max),             0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SPELLEFFECT",           0, field(game.conf.magic_conf.shot_cfgstats[0].cast_spell_kind ),            0,  INT32_MIN,  UINT32_MAX, spell_desc,                   value_default,   assign_default},
  {"BOUNCEANGLE",           0, field(game.conf.magic_conf.shot_cfgstats[0].bounce_angle),                0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SIZE_XY",               0, field(game.conf.magic_conf.shot_cfgstats[0].size_xy),                     0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SIZE_YZ",               0, field(game.conf.magic_conf.shot_cfgstats[0].size_z),                      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SIZE_Z",                0, field(game.conf.magic_conf.shot_cfgstats[0].size_z),                      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"FALLACCELERATION",      0, field(game.conf.magic_conf.shot_cfgstats[0].fall_acceleration),           0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"VISUALEFFECT",          0, field(game.conf.magic_conf.shot_cfgstats[0].visual.effect_model),         0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"VISUALEFFECTAMOUNT",    0, field(game.conf.magic_conf.shot_cfgstats[0].visual.amount),               0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"VISUALEFFECTSPREAD",    0, field(game.conf.magic_conf.shot_cfgstats[0].visual.random_range),         0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"VISUALEFFECTHEALTH",    0, field(game.conf.magic_conf.shot_cfgstats[0].visual.shot_health),          0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITWALLEFFECT",         0, field(game.conf.magic_conf.shot_cfgstats[0].hit_generic.effect_model),    0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"HITWALLSOUND",          0, field(game.conf.magic_conf.shot_cfgstats[0].hit_generic.sndsample_idx),   0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITWALLSOUND",          1, field(game.conf.magic_conf.shot_cfgstats[0].hit_generic.sndsample_range),   0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITCREATUREEFFECT",     0, field(game.conf.magic_conf.shot_cfgstats[0].hit_creature.effect_model),   0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"HITCREATURESOUND",      0, field(game.conf.magic_conf.shot_cfgstats[0].hit_creature.sndsample_idx),  0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITCREATURESOUND",      1, field(game.conf.magic_conf.shot_cfgstats[0].hit_creature.sndsample_range),  0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITDOOREFFECT",         0, field(game.conf.magic_conf.shot_cfgstats[0].hit_door.effect_model),       0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"HITDOORSOUND",          0, field(game.conf.magic_conf.shot_cfgstats[0].hit_door.sndsample_idx),      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITDOORSOUND",          1, field(game.conf.magic_conf.shot_cfgstats[0].hit_door.sndsample_range),      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITWATEREFFECT",        0, field(game.conf.magic_conf.shot_cfgstats[0].hit_water.effect_model),      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"HITWATERSOUND",         0, field(game.conf.magic_conf.shot_cfgstats[0].hit_water.sndsample_idx),     0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITWATERSOUND",         1, field(game.conf.magic_conf.shot_cfgstats[0].hit_water.sndsample_range),     0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITLAVAEFFECT",         0, field(game.conf.magic_conf.shot_cfgstats[0].hit_lava.effect_model),       0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"HITLAVASOUND",          0, field(game.conf.magic_conf.shot_cfgstats[0].hit_lava.sndsample_idx),      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITLAVASOUND",          1, field(game.conf.magic_conf.shot_cfgstats[0].hit_lava.sndsample_range),      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"DIGHITEFFECT",          0, field(game.conf.magic_conf.shot_cfgstats[0].dig.effect_model),            0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"DIGHITSOUND",           0, field(game.conf.magic_conf.shot_cfgstats[0].dig.sndsample_idx),           0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"DIGHITSOUND",           1, field(game.conf.magic_conf.shot_cfgstats[0].dig.sndsample_range),           0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"EXPLOSIONEFFECTS",      0, field(game.conf.magic_conf.shot_cfgstats[0].explode.effect1_model),       0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"EXPLOSIONEFFECTS",      1, field(game.conf.magic_conf.shot_cfgstats[0].explode.effect2_model),       0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"EXPLOSIONEFFECTS",      2, field(game.conf.magic_conf.shot_cfgstats[0].explode.around_effect1_model),0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"EXPLOSIONEFFECTS",      3, field(game.conf.magic_conf.shot_cfgstats[0].explode.around_effect2_model),0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"WITHSTANDHITAGAINST",  -1, NULL,0,                                                                   0,  INT32_MIN,  INT32_MAX,  shotmodel_withstand_types,    value_flagsfield,   assign_withstand},
  {"ANIMATIONTRANSPARENCY", 0, field(game.conf.magic_conf.shot_cfgstats[0].animation_transparency),      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"DESTROYONHIT",          0, field(game.conf.magic_conf.shot_cfgstats[0].destroy_on_first_hit),        0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"BASEEXPERIENCEGAIN",    0, field(game.conf.magic_conf.shot_cfgstats[0].experience_given_to_shooter), 0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"TARGETHITSTOPTURNS",    0, field(game.conf.magic_conf.shot_cfgstats[0].target_hitstop_turns),        0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SHOTSOUNDPRIORITY",     0, field(game.conf.magic_conf.shot_cfgstats[0].sound_priority),              0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"LIGHTING",              0, field(game.conf.magic_conf.shot_cfgstats[0].light_radius),                0,  INT32_MIN,  UINT32_MAX, NULL,                         value_stltocoord,   assign_default},
  {"LIGHTING",              1, field(game.conf.magic_conf.shot_cfgstats[0].light_intensity),             0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"LIGHTING",              2, field(game.conf.magic_conf.shot_cfgstats[0].light_flags),                 0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"INERTIA",               0, field(game.conf.magic_conf.shot_cfgstats[0].inertia_floor),               0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"INERTIA",               1, field(game.conf.magic_conf.shot_cfgstats[0].inertia_air),                 0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"UNSHADED",              0, field(game.conf.magic_conf.shot_cfgstats[0].unshaded),                    0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SOFTLANDING",           0, field(game.conf.magic_conf.shot_cfgstats[0].soft_landing),                0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"EFFECTMODEL",           0, field(game.conf.magic_conf.shot_cfgstats[0].effect_id),                   0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"FIRELOGIC",             0, field(game.conf.magic_conf.shot_cfgstats[0].fire_logic),                  0,  INT32_MIN,  UINT32_MAX, shotfirelogic_commands,       value_default,   assign_default},
  {"UPDATELOGIC",           0, field(game.conf.magic_conf.shot_cfgstats[0].update_logic),                0,  INT32_MIN,  UINT32_MAX, shotupdatelogic_commands,     value_function,   assign_default},
  {"EFFECTSPACING",         0, field(game.conf.magic_conf.shot_cfgstats[0].effect_spacing),              0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"EFFECTAMOUNT",          0, field(game.conf.magic_conf.shot_cfgstats[0].effect_amount),               0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITHEARTEFFECT",        0, field(game.conf.magic_conf.shot_cfgstats[0].hit_heart.effect_model),      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"HITHEARTSOUND",         0, field(game.conf.magic_conf.shot_cfgstats[0].hit_heart.sndsample_idx),     0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITHEARTSOUND",         1, field(game.conf.magic_conf.shot_cfgstats[0].hit_heart.sndsample_range),   0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"BLEEDINGEFFECT",        0, field(game.conf.magic_conf.shot_cfgstats[0].effect_bleeding),             0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"FROZENEFFECT",          0, field(game.conf.magic_conf.shot_cfgstats[0].effect_frozen),               0,  INT32_MIN,  UINT32_MAX, NULL,                         value_effOrEffEl,   assign_default},
  {"PERIODICAL",            0, field(game.conf.magic_conf.shot_cfgstats[0].periodical),                  0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SPEEDDEVIATION",        0, field(game.conf.magic_conf.shot_cfgstats[0].speed_deviation),             0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SPREAD_XY",             0, field(game.conf.magic_conf.shot_cfgstats[0].spread_xy),                   0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"SPREAD_Z",              0, field(game.conf.magic_conf.shot_cfgstats[0].spread_z),                    0,  INT32_MIN,  UINT32_MAX, NULL,                         value_default,   assign_default},
  {"HITTHINGFUNC",          0, field(game.conf.magic_conf.shot_cfgstats[0].hit_thing_lua_func_idx),      0,  INT32_MIN,  UINT32_MAX, NULL,                         value_function,   assign_default},
  {NULL},
  };

const struct NamedFieldSet magic_shot_named_fields_set = {
    &game.conf.magic_conf.shot_types_count,
    "shot",
    magic_shot_named_fields,
    shot_desc,
    MAGIC_ITEMS_MAX,
    sizeof(game.conf.magic_conf.shot_cfgstats[0]),
    game.conf.magic_conf.shot_cfgstats,
};

const struct NamedCommand magic_power_commands[] = {
  {"NAME",            1},
  {"POWER",           2},
  {"COST",            3},
  {"DURATION",        4},
  {"CASTABILITY",     5},
  {"ARTIFACT",        6},
  {"NAMETEXTID",      7},
  {"TOOLTIPTEXTID",   8},
  {"SYMBOLSPRITES",  10},
  {"POINTERSPRITES", 11},
  {"PANELTABINDEX",  12},
  {"SOUNDSAMPLES",   13},
  {"PROPERTIES",     14},
  {"CASTEXPANDFUNC", 15},
  {"PLAYERSTATE",    16},
  {"PARENTPOWER",    17},
  {"SOUNDPLAYED",    18},
  {"COOLDOWN",       19},
  {"SPELL",          20},
  {"EFFECT",         21},
  {"USEFUNCTION",    22},
  {"CREATURETYPE",   23},
  {"COSTFORMULA",    24},
  {NULL,              0},
  };

const struct NamedCommand magic_special_commands[] = {
  {"NAME",             1},
  {"ARTIFACT",         2},
  {"TOOLTIPTEXTID",    3},
  {"SPEECHPLAYED",     4},
  {"ACTIVATIONEFFECT", 5},
  {"VALUE",            6},
  {NULL,               0},
  };

const struct LongNamedCommand powermodel_castability_commands[] = {
  {"CUSTODY_CRTRS",    PwCast_CustodyCrtrs},
  {"OWNED_CRTRS",      PwCast_OwnedCrtrs},
  {"ALLIED_CRTRS",     PwCast_AlliedCrtrs},
  {"ENEMY_CRTRS",      PwCast_EnemyCrtrs},
  {"UNCONSC_CRTRS",    PwCast_NConscCrtrs},
  {"BOUND_CRTRS",      PwCast_BoundCrtrs},
  {"UNCLMD_GROUND",    PwCast_UnclmdGround},
  {"NEUTRL_GROUND",    PwCast_NeutrlGround},
  {"OWNED_GROUND",     PwCast_OwnedGround},
  {"ALLIED_GROUND",    PwCast_AlliedGround},
  {"ENEMY_GROUND",     PwCast_EnemyGround},
  {"NEUTRL_TALL",      PwCast_NeutrlTall},
  {"OWNED_TALL",       PwCast_OwnedTall},
  {"ALLIED_TALL",      PwCast_AlliedTall},
  {"ENEMY_TALL",       PwCast_EnemyTall},
  {"OWNED_FOOD",       PwCast_OwnedFood},
  {"NEUTRL_FOOD",      PwCast_NeutrlFood},
  {"ENEMY_FOOD",       PwCast_EnemyFood},
  {"OWNED_GOLD",       PwCast_OwnedGold},
  {"NEUTRL_GOLD",      PwCast_NeutrlGold},
  {"ENEMY_GOLD",       PwCast_EnemyGold},
  {"OWNED_SPELL",      PwCast_OwnedSpell},
  {"OWNED_BOULDERS",   PwCast_OwnedBoulders},
  {"NEEDS_DELAY",      PwCast_NeedsDelay},
  {"CLAIMABLE",        PwCast_Claimable},
  {"UNREVEALED",       PwCast_Unrevealed},
  {"REVEALED_TEMP",    PwCast_RevealedTemp},
  {"THING_OR_MAP",     PwCast_ThingOrMap},
  {"ONLY_DIGGERS",     PwCast_DiggersOnly},
  {"NO_DIGGERS",       PwCast_DiggersNot},
  {"ANYWHERE",         PwCast_Anywhere},
  {"ALL_CRTRS",        PwCast_AllCrtrs},
  {"ALL_FOOD",         PwCast_AllFood},
  {"ALL_GOLD",         PwCast_AllGold},
  {"ALL_THINGS",       PwCast_AllThings},
  {"ALL_GROUND",       PwCast_AllGround},
  {"NOT_ENEMY_GROUND", PwCast_NotEnemyGround},
  {"ALL_TALL",         PwCast_AllTall},
  {"ALL_OBJECTS",      PwCast_AllObjects},
  {"OWNED_OBJECTS",    PwCast_OwnedObjects},
  {"NEUTRL_OBJECTS",   PwCast_NeutrlObjects},
  {"ENEMY_OBJECTS",    PwCast_EnemyObjects},
  {NULL,                0},
  };

const struct NamedCommand powermodel_properties_commands[] = {
    {"INSTINCTIVE",       PwCF_Instinctive},
    {"HAS_PROGRESS",      PwCF_HasProgress},
    {NULL,                0},
};

const struct NamedCommand powermodel_expand_check_func_type[] = {
  {"general_expand",           OcC_General_expand},
  {"sight_of_evil_expand",     OcC_SightOfEvil_expand},
  {"call_to_arms_expand",      OcC_CallToArms_expand},
  {"do_not_expand",            OcC_do_not_expand},
  {NULL,                       OcC_Null},
};

const struct NamedCommand magic_cost_formula_commands[] = {
  {"none",       Cost_Default},
  {"digger",     Cost_Digger},
  {"dwarf",      Cost_Dwarf},
};

const struct NamedCommand magic_use_func_commands[] = {
    {"none",                           0},
    {"magic_use_power_hand",           1},
    {"magic_use_power_apply_spell",    2},
    {"magic_use_power_slap_thing",     3},
    {"magic_use_power_possess_thing",  4},
    {"magic_use_power_call_to_arms",   5},
    {"magic_use_power_lightning",      6},
    {"magic_use_power_imp",            7},
    {"magic_use_power_sight",          8},
    {"magic_use_power_cave_in",        9},
    {"magic_use_power_destroy_walls", 10},
    {"magic_use_power_obey",          11},
    {"magic_use_power_hold_audience", 12},
    {"magic_use_power_armageddon",    13},
    {"magic_use_power_tunneller",     14},
    {NULL,                             0},
};


const Expand_Check_Func powermodel_expand_check_func_list[] = {
  NULL,
  general_expand_check,
  sight_of_evil_expand_check,
  call_to_arms_expand_check,
  NULL,
  NULL,
};

/******************************************************************************/
struct NamedCommand spell_desc[MAGIC_ITEMS_MAX];
struct NamedCommand shot_desc[MAGIC_ITEMS_MAX];
struct NamedCommand power_desc[MAGIC_ITEMS_MAX];
struct NamedCommand special_desc[MAGIC_ITEMS_MAX];
/******************************************************************************/

static void assign_artifact(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    game.conf.object_conf.object_to_power_artifact[value] = idx;
}

static void assign_strength_before_last(const struct NamedField* named_field, int64_t value, const struct NamedFieldSet* named_fields_set, int idx, const char* src_str, unsigned char flags)
{
    // Old power max is one short for spell max, so duplicate final power value to use for lvl10 creatures.
    assign_default(named_field,value,named_fields_set,idx,src_str,flags);
    named_field++;
    assign_default(named_field,value,named_fields_set,idx,src_str,flags);
}

static const struct NamedField magic_powers_named_fields[] = {
    //name                     //pos    //field                                                                 //default //min     //max    //NamedCommand
    {"NAME",           0, field(game.conf.magic_conf.power_cfgstats[0].code_name),              0, INT32_MIN,UINT32_MAX, power_desc,                          value_name,      assign_null},
    {"POWER",          0, field(game.conf.magic_conf.power_cfgstats[0].strength[0]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"POWER",          1, field(game.conf.magic_conf.power_cfgstats[0].strength[1]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"POWER",          2, field(game.conf.magic_conf.power_cfgstats[0].strength[2]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"POWER",          3, field(game.conf.magic_conf.power_cfgstats[0].strength[3]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"POWER",          4, field(game.conf.magic_conf.power_cfgstats[0].strength[4]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"POWER",          5, field(game.conf.magic_conf.power_cfgstats[0].strength[5]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"POWER",          6, field(game.conf.magic_conf.power_cfgstats[0].strength[6]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"POWER",          7, field(game.conf.magic_conf.power_cfgstats[0].strength[7]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"POWER",          8, field(game.conf.magic_conf.power_cfgstats[0].strength[8]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_strength_before_last},
    {"POWER",          8, field(game.conf.magic_conf.power_cfgstats[0].strength[9]),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           0, field(game.conf.magic_conf.power_cfgstats[0].cost[0]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           1, field(game.conf.magic_conf.power_cfgstats[0].cost[1]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           2, field(game.conf.magic_conf.power_cfgstats[0].cost[2]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           3, field(game.conf.magic_conf.power_cfgstats[0].cost[3]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           4, field(game.conf.magic_conf.power_cfgstats[0].cost[4]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           5, field(game.conf.magic_conf.power_cfgstats[0].cost[5]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           6, field(game.conf.magic_conf.power_cfgstats[0].cost[6]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           7, field(game.conf.magic_conf.power_cfgstats[0].cost[7]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COST",           7, field(game.conf.magic_conf.power_cfgstats[0].cost[8]),                0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"DURATION",       0, field(game.conf.magic_conf.power_cfgstats[0].duration),               0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"CASTABILITY",   -1, field(game.conf.magic_conf.power_cfgstats[0].can_cast_flags),         0,         0,UINT64_MAX, (struct NamedCommand*)powermodel_castability_commands, value_longflagsfield,   assign_default},
    {"ARTIFACT",       0, field(game.conf.magic_conf.power_cfgstats[0].artifact_model),         0, INT32_MIN,UINT32_MAX, object_desc,                         value_default,   assign_artifact},
    {"NAMETEXTID",     0, field(game.conf.magic_conf.power_cfgstats[0].name_stridx),            0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"TOOLTIPTEXTID",  0, field(game.conf.magic_conf.power_cfgstats[0].tooltip_stridx),         0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"SYMBOLSPRITES",  0, field(game.conf.magic_conf.power_cfgstats[0].bigsym_sprite_idx),      0, INT32_MIN,UINT32_MAX, NULL,                                value_icon,      assign_icon},
    {"SYMBOLSPRITES",  1, field(game.conf.magic_conf.power_cfgstats[0].medsym_sprite_idx),      0, INT32_MIN,UINT32_MAX, NULL,                                value_icon,      assign_icon},
    {"POINTERSPRITES", 0, field(game.conf.magic_conf.power_cfgstats[0].pointer_sprite_idx),     0, INT32_MIN,UINT32_MAX, NULL,                                value_icon,      assign_icon},
    {"PANELTABINDEX",  0, field(game.conf.magic_conf.power_cfgstats[0].panel_tab_idx),          0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"SOUNDSAMPLES",   0, field(game.conf.magic_conf.power_cfgstats[0].select_sample_idx),      0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"PROPERTIES",     0, field(game.conf.magic_conf.power_cfgstats[0].config_flags),           0, INT32_MIN,UINT32_MAX, powermodel_properties_commands,      value_flagsfield,assign_default},
    {"CASTEXPANDFUNC", 0, field(game.conf.magic_conf.power_cfgstats[0].overcharge_check_idx),   0, INT32_MIN,UINT32_MAX, powermodel_expand_check_func_type,   value_default,   assign_default},
    {"PLAYERSTATE",    0, field(game.conf.magic_conf.power_cfgstats[0].work_state),             0, INT32_MIN,UINT32_MAX, player_state_commands,               value_default,   assign_default},
    {"PARENTPOWER",    0, field(game.conf.magic_conf.power_cfgstats[0].parent_power),           0, INT32_MIN,UINT32_MAX, power_desc,                          value_default,   assign_default},
    {"SOUNDPLAYED",    0, field(game.conf.magic_conf.power_cfgstats[0].select_sound_idx),       0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"COOLDOWN",       0, field(game.conf.magic_conf.power_cfgstats[0].cast_cooldown),          0, INT32_MIN,UINT32_MAX, NULL,                                value_default,   assign_default},
    {"SPELL",          0, field(game.conf.magic_conf.power_cfgstats[0].spell_idx),              0, INT32_MIN,UINT32_MAX, spell_desc,                          value_default,   assign_default},
    {"EFFECT",         0, field(game.conf.magic_conf.power_cfgstats[0].effect_id),              0, INT32_MIN,UINT32_MAX, NULL,                                value_effOrEffEl,assign_default},
    {"USEFUNCTION",    0, field(game.conf.magic_conf.power_cfgstats[0].magic_use_func_idx),     0, INT32_MIN,UINT32_MAX, magic_use_func_commands,             value_function,  assign_default},
    {"CREATURETYPE",   0, field(game.conf.magic_conf.power_cfgstats[0].creature_model),         0, INT32_MIN,UINT32_MAX, creature_desc,                       value_default,   assign_default},
    {"COSTFORMULA",    0, field(game.conf.magic_conf.power_cfgstats[0].cost_formula),           0, INT32_MIN,UINT32_MAX, magic_cost_formula_commands,         value_default,   assign_default},
    {NULL},
};

const struct NamedFieldSet magic_powers_named_fields_set = {
    &game.conf.magic_conf.power_types_count,
    "power",
    magic_powers_named_fields,
    power_desc,
    MAGIC_ITEMS_MAX,
    sizeof(game.conf.magic_conf.power_cfgstats[0]),
    game.conf.magic_conf.power_cfgstats,
};

#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct SpellConfig *get_spell_config(SpellKind spell_idx)
{
    if ((spell_idx == 0) || (spell_idx >= game.conf.magic_conf.spell_types_count))
    {
        return &game.conf.magic_conf.spell_config[0];
    }
    return &game.conf.magic_conf.spell_config[spell_idx];
}

TbBool spell_config_is_invalid(struct SpellConfig *mgcinfo)
{
    if (mgcinfo <= &game.conf.magic_conf.spell_config[0])
    {
        return true;
    }
    return false;
}

TextStringId get_power_name_strindex(PowerKind pwkind)
{
    if (pwkind >= game.conf.magic_conf.power_types_count)
        return game.conf.magic_conf.power_cfgstats[0].name_stridx;
    return game.conf.magic_conf.power_cfgstats[pwkind].name_stridx;
}

TextStringId get_power_description_strindex(PowerKind pwkind)
{
  if (pwkind >= game.conf.magic_conf.power_types_count)
    return game.conf.magic_conf.power_cfgstats[0].tooltip_stridx;
  return game.conf.magic_conf.power_cfgstats[pwkind].tooltip_stridx;
}

int32_t get_special_description_strindex(int spckind)
{
  if ((spckind < 0) || (spckind >= game.conf.magic_conf.power_types_count))
    return game.conf.magic_conf.special_cfgstats[0].tooltip_stridx;
  return game.conf.magic_conf.special_cfgstats[spckind].tooltip_stridx;
}

int32_t get_power_index_for_work_state(int32_t work_state)
{
    for (int32_t i = 0; i < game.conf.magic_conf.power_types_count; i++)
    {
        if (game.conf.magic_conf.power_cfgstats[i].work_state == work_state) {
            return i;
        }
    }
    return 0;
}

struct SpellConfigStats *get_spell_model_stats(SpellKind spmodel)
{
    if (spmodel >= game.conf.magic_conf.spell_types_count)
        return &game.conf.magic_conf.spell_cfgstats[0];
    return &game.conf.magic_conf.spell_cfgstats[spmodel];
}

struct ShotConfigStats *get_shot_model_stats(ThingModel tngmodel)
{
    if (tngmodel >= game.conf.magic_conf.shot_types_count)
        return &game.conf.magic_conf.shot_cfgstats[0];
    return &game.conf.magic_conf.shot_cfgstats[tngmodel];
}

struct PowerConfigStats *get_power_model_stats(PowerKind pwkind)
{
    if (pwkind >= game.conf.magic_conf.power_types_count)
        return &game.conf.magic_conf.power_cfgstats[0];
    return &game.conf.magic_conf.power_cfgstats[pwkind];
}

TbBool power_model_stats_invalid(const struct PowerConfigStats *powerst)
{
  if (powerst <= &game.conf.magic_conf.power_cfgstats[0])
    return true;
  return false;
}

TbBool power_is_instinctive(int pwkind)
{
    const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
    // Invalid powers are instinctive (as this usually means skipping an action)
    if (power_model_stats_invalid(powerst))
        return true;
    return ((powerst->config_flags & PwCF_Instinctive) != 0);
}

struct SpecialConfigStats *get_special_model_stats(SpecialKind spckind)
{
    if (spckind >= game.conf.magic_conf.special_types_count)
        return &game.conf.magic_conf.special_cfgstats[0];
    return &game.conf.magic_conf.special_cfgstats[spckind];
}

short write_magic_shot_to_log(const struct ShotConfigStats *shotst, int num)
{
  JUSTMSG("[shot%d]",(int)num);
  JUSTMSG("Name = %s",shotst->code_name);
  JUSTMSG("Values = %d %d",(int)shotst->is_magical,(int)shotst->experience_given_to_shooter);
  return true;
}

TbBool parse_magic_spell_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct SpellConfigStats *spellst;
  struct SpellConfig *spconf;
  // Initialize the array
  for (int i = 0; i < MAGIC_ITEMS_MAX; i++) {
    spellst = &game.conf.magic_conf.spell_cfgstats[i];
    if ((!flag_is_set(flags,CnfLd_AcceptPartial)) || (strlen(spellst->code_name) <= 0))
    {
        if (flag_is_set(flags, CnfLd_ListOnly))
        {
            memset(&spellst->code_name, 0, COMMAND_WORD_LEN);
            spell_desc[i].name = spellst->code_name;
            spell_desc[i].num = i;
        }
      spconf = &game.conf.magic_conf.spell_config[i];
      spconf->linked_power = 0;
      spconf->duration = 0;
      spconf->caster_affected = 0;
      spconf->caster_affect_sound = 0;
      spconf->cast_at_thing = 0;
      spconf->shot_model = 0;
      spconf->cast_effect_model = 0;
      spconf->bigsym_sprite_idx = 0;
      spconf->medsym_sprite_idx = 0;
      spconf->crtr_summon_model = 0;
      spconf->crtr_summon_level = 0;
      spconf->crtr_summon_amount = 0;
      spconf->aura_effect = 0;
      spconf->aura_duration = 0;
      spconf->aura_frequency = 0;
      spconf->spell_flags = 0;
      spconf->properties_flags = 0;
      spconf->countdown = 0;
      spconf->healing_recovery = 0;
      spconf->damage = 0;
      spconf->damage_frequency = 0;
    }
  }
  spell_desc[MAGIC_ITEMS_MAX - 1].name = NULL; // must be null for get_id
  // Load the file
  const char * blockname = NULL;
  int blocknamelen = 0;
  int32_t pos = 0;
  while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
  {
    // look for blocks starting with "spell", followed by one or more digits
    if (blocknamelen < 6) {
        continue;
    } else if (memcmp(blockname, "spell", 5) != 0) {
        continue;
    }
    const int i = natoi(&blockname[5], blocknamelen - 5);
    if (i < 0 || i >= MAGIC_ITEMS_MAX) {
        continue;
    } else if (i >= game.conf.magic_conf.spell_types_count) {
      game.conf.magic_conf.spell_types_count = i + 1;
    }
    spconf = &game.conf.magic_conf.spell_config[i];
    spellst = &game.conf.magic_conf.spell_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_spell_commands,cmd_num)
    while (pos < len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, magic_spell_commands);
      // Now store the config item in correct place
      if (cmd_num == ccr_endOfBlock) break; // if next block starts
      //Do the name when listing, the rest when not listing.
      if ((flag_is_set(flags, CnfLd_ListOnly) && cmd_num > 1) || (!flag_is_set(flags, CnfLd_ListOnly) && cmd_num <= 1))
      {
          cmd_num = ccr_comment;
      }
      int n = 0, k = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,spellst->code_name,COMMAND_WORD_LEN) <= 0)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
              break;
          }
          else
          {
              spell_desc[i].name = spellst->code_name;
              spell_desc[i].num = i;
          }
          n++;
          break;
      case 2: // DURATION
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->duration = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 3: // SELFCASTED
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->caster_affected = k;
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->caster_affect_sound = k;
              n++;
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->caster_sounds_count = k;
              n++;
          }
          if (n < 3)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 4: // CASTATTHING
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->cast_at_thing = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 5: // SHOTMODEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(shot_desc, word_buf);
              if (k >= 0) {
                  spconf->shot_model = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect shot model \"%s\" in [%.*s] block of %s file.",
                  word_buf, blocknamelen, blockname, config_textname);
              break;
          }
          break;
      case 6: // EFFECTMODEL
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = effect_or_effect_element_id(word_buf);
              spconf->cast_effect_model = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect effect model \"%s\" in [%.*s] block of %s file.",
                  word_buf, blocknamelen, blockname, config_textname);
              break;
          }
          break;
      case 7: // SYMBOLSPRITES
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              spconf->bigsym_sprite_idx = get_icon_id(word_buf);
              if (spconf->bigsym_sprite_idx != bad_icon_id)
              {
                  n++;
              }
          }
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              spconf->medsym_sprite_idx = get_icon_id(word_buf);
              if (spconf->medsym_sprite_idx != bad_icon_id)
              {
                  n++;
              }
          }
          if (n < 2)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 8: // SPELLPOWER
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              if (parameter_is_number(word_buf))
              {
                  k = atoi(word_buf);
              }
              else
              {
                  k = get_id(power_desc, word_buf);
              }
              if (k >= 0)
              {
                  spconf->linked_power = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 9: // AURAEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = effect_or_effect_element_id(word_buf);
              spconf->aura_effect = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
        case 10: // SPELLFLAGS
            spconf->spell_flags = 0;
            while (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                if (parameter_is_number(word_buf))
                {
                    k = atoi(word_buf);
                    spconf->spell_flags = k;
                    n++;
                }
                else
                {
                    k = get_id(spell_effect_flags, word_buf);
                    if (k > 0)
                    {
                        set_flag(spconf->spell_flags, k);
                        n++;
                    }
                }
                if (flag_is_set(spconf->spell_flags, CSAfF_PoisonCloud))
                {
                    clear_flag(spconf->spell_flags, CSAfF_PoisonCloud);
                    WARNLOG("'POISON_CLOUD' has no effect on spells, spell flag is not set on %s", spell_code_name(i));
                }
                if (flag_is_set(spconf->spell_flags, CSAfF_Wind))
                {
                    clear_flag(spconf->spell_flags, CSAfF_Wind);
                    WARNLOG("'WIND' has no effect on spells, spell flag is not set on %s", spell_code_name(i));
                }
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 11: // SUMMONCREATURE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = get_id(creature_desc, word_buf);
              if (k < 0)
              {
                  if (parameter_is_number(word_buf))
                  {
                      k = atoi(word_buf);
                      spconf->crtr_summon_model = k;
                      n++;
                  }
              }
              else
              {
                  spconf->crtr_summon_model = k;
                  n++;
              }
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->crtr_summon_level = k;
              n++;
          }
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              spconf->crtr_summon_amount = k;
              n++;
          }
          if (n < 3)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
        case 12: // COUNTDOWN
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                spconf->countdown = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 13: // HEALINGRECOVERY
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                spconf->healing_recovery = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 14: // DAMAGE
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                spconf->damage = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 15: // DAMAGEFREQUENCY
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                spconf->damage_frequency = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 16: // AURADURATION
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                spconf->aura_duration = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 17: // AURAFREQUENCY
            if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                k = atoi(word_buf);
                spconf->aura_frequency = k;
                n++;
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 18: // CLEANSEFLAGS
            spconf->cleanse_flags = 0;
            while (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                if (parameter_is_number(word_buf))
                {
                    k = atoi(word_buf);
                    spconf->cleanse_flags = k;
                    n++;
                }
                else
                {
                    k = get_id(spell_effect_flags, word_buf);
                    if (k > 0)
                    {
                        set_flag(spconf->cleanse_flags, k);
                        n++;
                    }
                }
                if (flag_is_set(spconf->cleanse_flags, CSAfF_PoisonCloud))
                {
                    clear_flag(spconf->cleanse_flags, CSAfF_PoisonCloud);
                    WARNLOG("'POISON_CLOUD' has no effect on spells, cleanse flag is not set on %s", spell_code_name(i));
                }
                if (flag_is_set(spconf->cleanse_flags, CSAfF_Wind))
                {
                    clear_flag(spconf->cleanse_flags, CSAfF_Wind);
                    WARNLOG("'WIND' has no effect on spells, cleanse flag is not set on %s", spell_code_name(i));
                }
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
        case 19: // PROPERTIES
            spconf->properties_flags = 0;
            while (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
            {
                if (parameter_is_number(word_buf))
                {
                    k = atoi(word_buf);
                    spconf->properties_flags = k;
                    n++;
                }
                else
                {
                    k = get_id(magic_spell_properties, word_buf);
                    switch (k)
                    {
                        case 1: // FIXED_DAMAGE
                            set_flag(spconf->properties_flags, SPF_FixedDamage);
                            n++;
                            break;
                        case 2: // PERCENT_BASED
                            set_flag(spconf->properties_flags, SPF_PercentBased);
                            n++;
                            break;
                        case 3: // MAX_HEALTH
                            set_flag(spconf->properties_flags, SPF_MaxHealth);
                            n++;
                            break;
                        default:
                            CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
                            break;
                    }
                }
            }
            if (n < 1)
            {
                CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                    COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
            }
            break;
      case ccr_comment:
          break;
      case ccr_endOfFile:
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%.*s] block of %s file.",
              cmd_num, blocknamelen, blockname, config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
  }
  return true;
}

TbBool parse_magic_special_blocks(char *buf, long len, const char *config_textname, unsigned short flags)
{
  struct SpecialConfigStats *specst;
  int k = 0;
  // Initialize the array
  if ((flags & CnfLd_AcceptPartial) == 0) {
      for (int i = 0; i < MAGIC_ITEMS_MAX; i++) {
          specst = &game.conf.magic_conf.special_cfgstats[i];
          memset(specst->code_name, 0, COMMAND_WORD_LEN);
          specst->artifact_model = 0;
          specst->tooltip_stridx = 0;
          special_desc[i].name = specst->code_name;
          special_desc[i].num = i;
          game.conf.object_conf.object_to_special_artifact[i] = 0;
      }
  }
  special_desc[MAGIC_ITEMS_MAX - 1].name = NULL; // must be null for get_id
  // Load the file
  const char * blockname = NULL;
  int blocknamelen = 0;
  int32_t pos = 0;
  while (iterate_conf_blocks(buf, &pos, len, &blockname, &blocknamelen))
  {
    // look for blocks starting with "special", followed by one or more digits
    if (blocknamelen < 8) {
        continue;
    } else if (memcmp(blockname, "special", 7) != 0) {
        continue;
    }
    const int i = natoi(&blockname[7], blocknamelen - 7);
    if (i < 0 || i >= MAGIC_ITEMS_MAX) {
        continue;
    } else if (i >= game.conf.magic_conf.special_types_count) {
        game.conf.magic_conf.special_types_count = i + 1;
    }
    specst = &game.conf.magic_conf.special_cfgstats[i];
#define COMMAND_TEXT(cmd_num) get_conf_parameter_text(magic_special_commands,cmd_num)
    while (pos < len)
    {
      // Finding command number in this line
      int cmd_num = recognize_conf_command(buf, &pos, len, magic_special_commands);
      // Now store the config item in correct place
      if (cmd_num == ccr_endOfBlock) break; // if next block starts
      if ((flags & CnfLd_ListOnly) != 0) {
          // In "List only" mode, accept only name command
          if (cmd_num > 1) {
              cmd_num = 0;
          }
      }
      int n = 0;
      char word_buf[COMMAND_WORD_LEN];
      switch (cmd_num)
      {
      case 1: // NAME
          if (get_conf_parameter_single(buf,&pos,len,specst->code_name,COMMAND_WORD_LEN) <= 0)
          {
              CONFWRNLOG("Couldn't read \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
              break;
          }
          break;
      case 2: // ARTIFACT
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
              k = get_id(object_desc, word_buf);
              if (k >= 0) {
                  specst->artifact_model = k;
                  game.conf.object_conf.object_to_special_artifact[k] = i;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect object model \"%s\" in [%.*s] block of %s file.",
                  word_buf, blocknamelen, blockname, config_textname);
              break;
          }
          break;
      case 3: // TOOLTIPTEXTID
          if (get_conf_parameter_single(buf,&pos,len,word_buf,sizeof(word_buf)) > 0)
          {
            k = atoi(word_buf);
            if (k > 0)
            {
                specst->tooltip_stridx = k;
                n++;
            }
          }
          if (n < 1)
          {
            CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 4: // SPEECHPLAYED
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              if (k >= 0)
              {
                  specst->speech = k;
                  n++;
              }
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 5: // ACTIVATIONEFFECT
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = effect_or_effect_element_id(word_buf);
              specst->effect_id = k;
              n++;
          }
          if (n < 1)
          {
              CONFWRNLOG("Incorrect value of \"%s\" parameter in [%.*s] block of %s file.",
                  COMMAND_TEXT(cmd_num), blocknamelen, blockname, config_textname);
          }
          break;
      case 6: // VALUE
          if (get_conf_parameter_single(buf, &pos, len, word_buf, sizeof(word_buf)) > 0)
          {
              k = atoi(word_buf);
              specst->value = k;
          }
          break;
      case ccr_comment:
          break;
      case ccr_endOfFile:
          break;
      default:
          CONFWRNLOG("Unrecognized command (%d) in [%.*s] block of %s file.",
              cmd_num, blocknamelen, blockname, config_textname);
          break;
      }
      skip_conf_to_next_line(buf,&pos,len);
    }
#undef COMMAND_TEXT
  }
  return true;
}

static TbBool load_magic_config_file(const char *fname, unsigned short flags)
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

   if (!flag_is_set(flags, CnfLd_AcceptPartial)) {
       for (int i = 0; i < MAGIC_ITEMS_MAX; i++) {
           game.conf.object_conf.object_to_power_artifact[i] = 0;
       }
    }

    game.conf.magic_conf.spell_types_count = MAGIC_ITEMS_MAX;
    game.conf.magic_conf.special_types_count = MAGIC_ITEMS_MAX;

    // Loading file data
    len = LbFileLoadAt(fname, buf);
    TbBool result = (len > 0);
    // Parse blocks of the config file
    if (result)
    {
        parse_magic_spell_blocks(buf, len, fname, flags);
        parse_named_field_blocks(buf, len, fname, flags, &magic_shot_named_fields_set);
        parse_named_field_blocks(buf, len, fname, flags, &magic_powers_named_fields_set);
        parse_magic_special_blocks(buf, len, fname, flags);
    }


    if ((flags & CnfLd_ListOnly) == 0)
    {
      // Mark powers which have children
      for (int i = 0; i < game.conf.magic_conf.power_types_count; i++)
      {
        struct PowerConfigStats *powerst = get_power_model_stats(i);
          struct PowerConfigStats* parent_powerst = get_power_model_stats(powerst->parent_power);
          if (!power_model_stats_invalid(parent_powerst)) {
              parent_powerst->config_flags |= PwCF_IsParent;
          }
      }
    }


    //Freeing and exiting
    free(buf);
    return result;
}

/**
 * Returns Code Name (name to use in script file) of given spell model.
 */
const char *spell_code_name(SpellKind spmodel)
{
    const char* name = get_conf_parameter_text(spell_desc, spmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given shot model.
 */
const char *shot_code_name(ThingModel tngmodel)
{
    const char* name = get_conf_parameter_text(shot_desc, tngmodel);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns Code Name (name to use in script file) of given keepers power kind.
 */
const char *power_code_name(PowerKind pwkind)
{
    const char* name = get_conf_parameter_text(power_desc, pwkind);
    if (name[0] != '\0')
        return name;
    return "INVALID";
}

/**
 * Returns the power model identifier for a given code name (found in script file).
 * Linear running time.
 * @param code_name
 * @return A positive integer for the power model if found, otherwise -1
 */
int power_model_id(const char * code_name)
{
    for (int i = 0; i < game.conf.magic_conf.power_types_count; ++i)
    {
        if (strncmp(game.conf.magic_conf.power_cfgstats[i].code_name, code_name,
                COMMAND_WORD_LEN) == 0) {
            return i;
        }
    }

    return -1;
}

/**
 * Adds given power to players available powers.
 *
 * @param pwkind
 * @param plyr_idx
 * @return
 * @note originally add_spell_to_player()
 */
TbBool add_power_to_player(PowerKind pwkind, PlayerNumber plyr_idx)
{
    if (pwkind >= game.conf.magic_conf.power_types_count)
    {
        ERRORLOG("Can't add incorrect power %d to player %d",(int)pwkind, (int)plyr_idx);
        return false;
    }
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Can't add %s to player %d which has no dungeon",power_code_name(pwkind), (int)plyr_idx);
        return false;
    }
    long i = dungeon->magic_level[pwkind];
    if (i >= 255)
    {
        ERRORLOG("Power %s has bad magic_level=%d for player %d, reset", power_code_name(pwkind), (int)i, (int)plyr_idx);
        i = 0;
    }
    dungeon->magic_level[pwkind] = i+1;
    return true;
}

void remove_power_from_player(PowerKind pwkind, PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    struct PlayerInfo* player;
    struct Thing* thing;
    if (dungeon_invalid(dungeon))
    {
        ERRORLOG("Cannot remove spell %s from invalid dungeon %d!",power_code_name(pwkind),(int)plyr_idx);
        return;
    }
    long i = dungeon->magic_level[pwkind];
    if (i < 1)
    {
        ERRORLOG("Cannot remove spell %s (%d) from player %d as he doesn't have it!",power_code_name(pwkind),(int)pwkind,(int)plyr_idx);
        return;
    }
    SYNCDBG(4,"Decreasing spell %s of player %d to level %d",power_code_name(pwkind),(int)plyr_idx,(int)i-1);
    dungeon->magic_level[pwkind] = i-1;
    switch (pwkind)
    {
    case PwrK_OBEY:
        if (player_uses_power_obey(plyr_idx))
            turn_off_power_obey(plyr_idx);
        break;
    case PwrK_SIGHT:
        if (player_uses_power_sight(plyr_idx))
            turn_off_power_sight_of_evil(plyr_idx);
        break;
    case PwrK_CALL2ARMS:
        if (player_uses_power_call_to_arms(plyr_idx))
            turn_off_power_call_to_arms(plyr_idx);
        break;
    case PwrK_POSSESS:
        player = get_player(plyr_idx);
        if (player->view_type == PVT_CreatureContrl)
        {
            thing = thing_get(player->controlled_thing_idx);
            prepare_to_controlled_creature_death(thing);
        }
        break;
    }
    if (game.chosen_spell_type == pwkind)
    {
        set_chosen_power_none();
    }
}

/**
 * Zeroes all the costs for all spells.
 */
TbBool make_all_powers_cost_free(void)
{
    for (long i = 0; i < game.conf.magic_conf.power_types_count; i++)
    {
        struct PowerConfigStats * powerst = get_power_model_stats(i);
        for (long n = 0; n < MAGIC_OVERCHARGE_LEVELS; n++)
            powerst->cost[n] = 0;
  }
  return true;
}

/**
 * Makes all keeper spells to be available to research.
 */
TbBool make_all_powers_researchable(PlayerNumber plyr_idx)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    for (long i = 0; i < game.conf.magic_conf.power_types_count; i++)
    {
        dungeon->magic_resrchable[i] = 1;
    }
    return true;
}

/**
 * Sets power availability state.
 */
TbBool set_power_available(PlayerNumber plyr_idx, PowerKind pwkind, long resrch, long avail)
{
    SYNCDBG(8,"Starting for power %d, player %d, state %ld,%ld",(int)pwkind,(int)plyr_idx,resrch,avail);
    // note that we can't get_players_num_dungeon() because players
    // may be uninitialized yet when this is called.
    struct Dungeon* dungeon = get_dungeon(plyr_idx);
    if (dungeon_invalid(dungeon)) {
        ERRORDBG(11,"Cannot set power availability; player %d has no dungeon",(int)plyr_idx);
        return false;
    }
    dungeon->magic_resrchable[pwkind] = resrch;
    if (avail <= 0)
    {
        if (is_power_available(plyr_idx, pwkind))
        {
            remove_power_from_player(pwkind, plyr_idx);
        }
        return true;
    }
    if (is_power_available(plyr_idx, pwkind))
    {
        return true;
    }
    return add_power_to_player(pwkind, plyr_idx);
}

/**
 * Returns if the power can be used by a player.
 * Checks only if it's available and if the player is 'alive'.
 * Doesn't check if the player has enough money or map position is on correct spot.
 */
TbBool is_power_available(PlayerNumber plyr_idx, PowerKind pwkind)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    //TODO POWERS Mapping child powers to their parent - remove that when magic_level array is enlarged
    {
        const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
        if (powerst->parent_power != 0)
            pwkind = powerst->parent_power;
    }
    // Player must have dungeon heart to cast spells, with no heart only floating spirit spell works
    if (!player_has_heart(plyr_idx) && (pwkind != PwrK_POSSESS)) {
        return false;
    }
    if (pwkind >= game.conf.magic_conf.power_types_count)
    {
        ERRORLOG("Incorrect power %u (player %d)", pwkind, plyr_idx);
        return false;
    }
    if (dungeon->magic_level[pwkind] > 0) {
        return true;
    }
    return false;
}

/**
 * Returns if the power can be or already is obtained by a player.
 */
TbBool is_power_obtainable(PlayerNumber plyr_idx, PowerKind pwkind)
{
    struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
    // Check if the player even have a dungeon
    if (dungeon_invalid(dungeon)) {
        return false;
    }
    //TODO POWERS Mapping child powers to their parent - remove that when magic_level array is enlarged
    {
        const struct PowerConfigStats* powerst = get_power_model_stats(pwkind);
        if (powerst->parent_power != 0)
            pwkind = powerst->parent_power;
    }
    // Player must have dungeon heart to cast spells, with no heart only floating spirit spell works
    if (!player_has_heart(plyr_idx) && (pwkind != PwrK_POSSESS)) {
        return false;
    }
    if (pwkind >= game.conf.magic_conf.power_types_count) {
        ERRORLOG("Incorrect power %u (player %d)",pwkind, plyr_idx);
        return false;
    }
    return (dungeon->magic_level[pwkind] > 0) || (dungeon->magic_resrchable[pwkind]);
}

/**
 * Makes all the powers, which are researchable, to be instantly available.
 */
TbBool make_available_all_researchable_powers(PlayerNumber plyr_idx)
{
  SYNCDBG(0,"Starting");
  TbBool ret = true;
  struct Dungeon* dungeon = get_players_num_dungeon(plyr_idx);
  if (dungeon_invalid(dungeon)) {
      ERRORDBG(11,"Cannot make research available; player %d has no dungeon",(int)plyr_idx);
      return false;
  }
  for (long i = 0; i < game.conf.magic_conf.power_types_count; i++)
  {
    if (dungeon->magic_resrchable[i])
    {
      ret &= add_power_to_player(i, plyr_idx);
    }
  }
  return ret;
}

/******************************************************************************/
