/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file config_magic.h
 *     Header file for config_magic.c.
 * @par Purpose:
 *     Support of configuration files for magic spells.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     25 May 2009 - 26 Jul 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#ifndef DK_CFGMAGIC_H
#define DK_CFGMAGIC_H

#include "globals.h"
#include "bflib_basics.h"
#include "creature_instances.h"

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#define MAGIC_ITEMS_MAX        255
#define SPELL_MAX_LEVEL         8
#define MAGIC_OVERCHARGE_LEVELS (SPELL_MAX_LEVEL+1)
#define POWER_TYPES_MAX      64

enum SpellKinds {
    SplK_None = 0,
    SplK_Fireball,
    SplK_FireBomb,
    SplK_Freeze,
    SplK_Armour,
    SplK_Lightning,
    SplK_Rebound,
    SplK_Heal,
    SplK_PoisonCloud,
    SplK_Invisibility,
    SplK_Teleport,//[10]
    SplK_Speed,
    SplK_Slow,
    SplK_Drain,
    SplK_Fear,
    SplK_Missile,//[15]
    SplK_NavigMissile,
    SplK_FlameBreath,
    SplK_Wind,
    SplK_Light,
    SplK_Fly,//[20]
    SplK_Sight,
    SplK_Grenade,
    SplK_Hailstorm,
    SplK_WordOfPower,//[24]
    SplK_CrazyGas,
    SplK_Disease,
    SplK_Chicken,
    SplK_TimeBomb,//[28]
    SplK_Lizard,
};

enum CreatureSpellAffectedFlags {
    CSAfF_Slow         = 0x0001,
    CSAfF_Speed        = 0x0002,
    CSAfF_Armour       = 0x0004,
    CSAfF_Rebound      = 0x0008,
    CSAfF_Flying       = 0x0010,
    CSAfF_Invisibility = 0x0020,
    CSAfF_Sight        = 0x0040,
    CSAfF_Light        = 0x0080, // this was originally Freeze, but that is now done via stateblock_flags
    CSAfF_Disease      = 0x0100,
    CSAfF_Chicken      = 0x0200,
    CSAfF_PoisonCloud  = 0x0400,
    CSAfF_CalledToArms = 0x0800,
    CSAfF_MadKilling   = 0x1000,
    /** The creature does a free fall with magical effect, ie. it was just created with some initial velocity. */
    CSAfF_MagicFall    = 0x2000,
    CSAfF_ExpLevelUp   = 0x4000,
    /** For creature which are normally flying, this informs that its grounded due to spells or its condition. */
    CSAfF_Grounded     = 0x8000,
    CSAfF_Timebomb     = 0x10000,
};

enum PowerKinds {
    PwrK_None = 0,
    PwrK_HAND,
    PwrK_MKDIGGER,
    PwrK_OBEY,
    PwrK_SLAP,
    PwrK_SIGHT, // 5
    PwrK_CALL2ARMS,
    PwrK_CAVEIN,
    PwrK_HEALCRTR,
    PwrK_HOLDAUDNC,
    PwrK_LIGHTNING, // 10
    PwrK_SPEEDCRTR,
    PwrK_PROTECT,
    PwrK_CONCEAL,
    PwrK_DISEASE,
    PwrK_CHICKEN, // 15
    PwrK_DESTRWALLS,
    PwrK_TIMEBOMB,
    PwrK_POSSESS,
    PwrK_ARMAGEDDON,
    PwrK_PICKUPCRTR, // 20
    PwrK_PICKUPGOLD,
    PwrK_PICKUPFOOD,
    PwrK_REBOUND,
    PwrK_FREEZE,
    PwrK_SLOW, // 25
    PwrK_FLIGHT,
    PwrK_VISION,
};

/** Contains properties of a shot model, to be stored in ShotConfigStats.
 */
enum ShotModelFlags {
    /** Set if the shot can be slapped with hand of evil of owning player. */
    ShMF_Slappable      = 0x0001,
    ShMF_Navigable      = 0x0002,
    ShMF_Boulder        = 0x0004,
    ShMF_ReboundImmune  = 0x0008,
    ShMF_Digging        = 0x0010,
    ShMF_LifeDrain      = 0x0020,
    ShMF_GroupUp        = 0x0040,
    ShMF_NoStun         = 0x0080,
    ShMF_NoHit          = 0x0100,
    ShMF_StrengthBased  = 0x0200,
    ShMF_AlarmsUnits    = 0x0400,
    ShMF_CanCollide     = 0x0800,
    ShMF_Disarming      = 0x1000,
    ShMF_Exploding      = 0x2000,
    ShMF_BlocksRebirth  = 0x4000,
    ShMF_Penetrating    = 0x8000,
};

enum PowerCanCastFlags {
    PwCast_None          = 0x0000000000,
    /** Allow casting the spell on enemy creatures kept in custody. */
    PwCast_CustodyCrtrs  = 0x0000000001,
    /** Allow casting the spell on owned creatures not captured by enemy. */
    PwCast_OwnedCrtrs    = 0x0000000002,
    /** Allow casting the spell on creatures of allied players. */
    PwCast_AlliedCrtrs   = 0x0000000004,
    /** Allow casting the spell on creatures of enemy players. */
    PwCast_EnemyCrtrs    = 0x0000000008,
    /** Allow casting the spell on creatures which are unconscious or dying. */
    PwCast_NConscCrtrs   = 0x0000000010,
    /** Allow casting the spell on creatures which are bound by state (dragged, being sacrificed, teleported etc.). */
    PwCast_BoundCrtrs    = 0x0000000020,

    /** Allow casting the spell on neutral walkable tiles - path, water, lava. */
    PwCast_UnclmdGround  = 0x0000000080,
    /** Allow casting the spell on neutral ground - rooms floor and neutral claimed ground. */
    PwCast_NeutrlGround  = 0x0000000100,
    /** Allow casting the spell on owned ground - rooms floor and claimed ground. */
    PwCast_OwnedGround   = 0x0000000200,
    /** Allow casting the spell on allied players ground - rooms floor and claimed ground. */
    PwCast_AlliedGround  = 0x0000000400,
    /** Allow casting the spell on enemy players ground - rooms floor and claimed ground. */
    PwCast_EnemyGround   = 0x0000000800,

    /** Allow casting the spell on neutral tall slabs - earth, wall, gold. */
    PwCast_NeutrlTall    = 0x0000001000,
    /** Allow casting the spell on owned tall slabs - own fortified wall. */
    PwCast_OwnedTall     = 0x0000002000,
    /** Allow casting the spell on tall slabs owned by allies - their fortified walls. */
    PwCast_AlliedTall    = 0x0000004000,
    /** Allow casting the spell on tall slabs owned by enemies - their fortified walls. */
    PwCast_EnemyTall     = 0x0000008000,

    /** Allow casting the spell on owned food things (chickens). */
    PwCast_OwnedFood     = 0x0000020000,
    /** Allow casting the spell on neutral food things. */
    PwCast_NeutrlFood    = 0x0000040000,
    /** Allow casting the spell on enemy food things. */
    PwCast_EnemyFood     = 0x0000080000,
    /** Allow casting the spell on owned gold things (piles,pots etc.). */
    PwCast_OwnedGold     = 0x0000100000,
    /** Allow casting the spell on neutral gold things. */
    PwCast_NeutrlGold    = 0x0000200000,
    /** Allow casting the spell on enemy gold things. */
    PwCast_EnemyGold     = 0x0000400000,
    /** Allow casting the spell on owned spell books. */
    PwCast_OwnedSpell    = 0x0000800000,
    /** Allow casting the spell on owned deployed trap things. */
    PwCast_OwnedBoulders = 0x0001000000,
    /** Allow casting the spell only after a small delay from previous cast. */
    PwCast_NeedsDelay    = 0x0004000000,
    /** Allow casting the spell only on claimable/fortificable slabs (for ground - path or claimed, for tall - earth or fortified). */
    PwCast_Claimable     = 0x0008000000,
    /** Allow casting the spell on un-revealed tiles. */
    PwCast_Unrevealed    = 0x0010000000,
    /** Allow casting the spell on temporarily revealed tiles (with SOE spell). */
    PwCast_RevealedTemp  = 0x0020000000,
    /** Allow casting if only one of map-related and thing-related conditions is met. */
    PwCast_ThingOrMap    = 0x0040000000,
    /** There are no map-related conditions - allow casting the spell anywhere on revealed map. */
    PwCast_Anywhere      = 0x0080000000,
    PwCast_DiggersOnly   = 0x0100000000,
    PwCast_DiggersNot    = 0x0200000000,
};
#define PwCast_AllCrtrs (PwCast_CustodyCrtrs|PwCast_OwnedCrtrs|PwCast_AlliedCrtrs|PwCast_EnemyCrtrs|PwCast_NConscCrtrs|PwCast_BoundCrtrs)
#define PwCast_AllFood (PwCast_OwnedFood|PwCast_NeutrlFood|PwCast_EnemyFood)
#define PwCast_AllGold (PwCast_OwnedGold|PwCast_NeutrlGold|PwCast_EnemyGold)
#define PwCast_AllThings (PwCast_CustodyCrtrs|PwCast_OwnedCrtrs|PwCast_AlliedCrtrs|PwCast_EnemyCrtrs|PwCast_AllFood|PwCast_AllGold|PwCast_OwnedSpell|PwCast_OwnedBoulders)
#define PwCast_AllGround (PwCast_UnclmdGround|PwCast_NeutrlGround|PwCast_OwnedGround|PwCast_AlliedGround|PwCast_EnemyGround)
#define PwCast_NotEnemyGround (PwCast_UnclmdGround|PwCast_NeutrlGround|PwCast_OwnedGround|PwCast_AlliedGround)
#define PwCast_AllTall (PwCast_NeutrlTall|PwCast_OwnedTall|PwCast_AlliedTall|PwCast_EnemyTall)

enum PowerConfigFlags {
    PwCF_Instinctive  = 0x0001, /**< Set if the power doesn't need to be selected from menu to be activated. */
    PwCF_HasProgress  = 0x0002, /**< Set if the power has a progress bar when active. */
    PwCF_IsParent     = 0x0004, /**< Set if the power has children and is just an aggregate. */
};

/**
 * Configuration parameters for spells.
 */
struct SpellConfigStats {
    char code_name[COMMAND_WORD_LEN];
};


struct ShotHitConfig {
    short effect_model; /**< Effect kind to be created when the shot hits. */
    short sndsample_idx; /**< Base sound sample to be played on hit. */
    unsigned char sndsample_range; /**< Range for random sound sample selection. */
    unsigned char withstand; /**< Whether the shot can withstand a hit without getting destroyed; could be converted to flags. */
};

struct ShotDetonateConfig {
    short effect1_model;
    short effect2_model; 
    short around_effect1_model;
    short around_effect2_model;
};

struct ShotVisualConfig {
    short effect_model;
    unsigned char amount;
    short random_range;
    short shot_health;
};

/**
 * Configuration parameters for shots.
 */
struct ShotConfigStats {
    char code_name[COMMAND_WORD_LEN];
    unsigned long model_flags;
    /** Health of a shot decreases by 1 on every turn, so it works also as lifespan. */
    HitPoints health;
    /** Range of area damage, if the spell causes area damage. */
    MapCoordDelta area_range;
    /** Amount of area damage points inflicted, if the spell causes area damage. */
    HitPoints area_damage;
    /** Hit type used for area damage, controls which things are affected. */
    ThingHitType area_hit_type;
    /** Strength of the blow which pushes creatures on explosion. */
    MapCoordDelta area_blow;
    /** Type of the damage inflicted by this shot. */
    short damage;
    short speed;
    DamageType damage_type;
    struct ShotHitConfig hit_generic;
    struct ShotHitConfig hit_door;
    struct ShotHitConfig hit_water;
    struct ShotHitConfig hit_lava;
    struct ShotHitConfig hit_creature;
    struct ShotHitConfig dig;
    struct ShotDetonateConfig explode;
    struct ShotVisualConfig visual;
    short firing_sound;
    short shot_sound;
    short sound_priority;
    unsigned char firing_sound_variants;
    short max_range;
    unsigned short sprite_anim_idx;
    unsigned short sprite_size_max;
    short size_xy;
    short size_z;
    unsigned char fall_acceleration;
    unsigned char cast_spell_kind;
    unsigned char push_on_hit;
    unsigned char hidden_projectile;
    unsigned char destroy_on_first_hit;
    short experience_given_to_shooter;
    short inertia_floor;
    short inertia_air;
    short bounce_angle;
    short wind_immune;
    short no_air_damage;
    unsigned char target_hitstop_turns;
    short animation_transparency;
    short fixed_damage;
    short light_radius;
    unsigned char light_intensity;
    unsigned char lightf_53;
    unsigned char unshaded;
    unsigned char soft_landing;
    EffectOrEffElModel effect_id;
    unsigned char fire_logic; // see enum ShotFireLogics
    unsigned char update_logic; // see enum ShotUpdateLogics
    unsigned short effect_spacing;
    unsigned char effect_amount;
};

typedef unsigned char (*Expand_Check_Func)(void);

/**
 * Configuration parameters for powers.
 */
struct PowerConfigStats {
    char code_name[COMMAND_WORD_LEN];
    ThingModel artifact_model;
    unsigned long long can_cast_flags;
    unsigned long config_flags;
    unsigned char overcharge_check_idx;
    long work_state;
    PowerKind parent_power;
    /** Sprite index of big symbol icon representing the power. */
    short bigsym_sprite_idx;
    /** Sprite index of medium symbol icon representing the power. */
    short medsym_sprite_idx;
    unsigned short name_stridx;
    unsigned short tooltip_stridx;
    short select_sample_idx;
    short pointer_sprite_idx;
    long panel_tab_idx;
    unsigned short select_sound_idx;
    short cast_cooldown;
};

/**
 * Configuration parameters for specials.
 */
struct SpecialConfigStats {
    char code_name[COMMAND_WORD_LEN];
    ThingModel artifact_model;
    TextStringId tooltip_stridx;
    short speech;
    short effect_id;
    short value;
};

 /**
 * Spell information structure.
 * Stores configuration of spells; to be replaced with SpellConfigStats when all fields are in CFG.
 * It no longer matches the similar struct from DK - fields were added at end.
 */
struct SpellConfig {
    /** Informs if the spell can be targeted on a thing. */
    unsigned char cast_at_thing;
    /** Shot model to be fired while casting. */
    unsigned char shot_model;
    /** Informs if caster is affected by the spell. */
    unsigned char caster_affected;
    /** Effect model created while casting. */
    short cast_effect_model;
    /** If caster is affected by the spell, indicates sound sample to be played. */
    unsigned short caster_affect_sound;
    /** Sprite index of big symbol icon representing the spell. */
    short bigsym_sprite_idx;
    /** Sprite index of medium symbol icon representing the spell. */
    short medsym_sprite_idx;
    short cast_sound;
    short crtr_summon_model;
    short crtr_summon_level;
    short crtr_summon_amount;
    short linked_power;
    short duration;
    short aura_effect;
    unsigned short spell_flags;
};

struct MagicStats {
  long cost[MAGIC_OVERCHARGE_LEVELS];
  long duration;
  long strength[MAGIC_OVERCHARGE_LEVELS];
};

struct MagicConfig {
    long spell_types_count;
    struct SpellConfig spell_config[MAGIC_ITEMS_MAX];// should get merged into SpellConfigStats
    struct SpellConfigStats spell_cfgstats[MAGIC_ITEMS_MAX];
    long shot_types_count;
    struct ShotConfigStats shot_cfgstats[MAGIC_ITEMS_MAX];
    long power_types_count;
    struct PowerConfigStats power_cfgstats[MAGIC_ITEMS_MAX];
    long special_types_count;
    struct SpecialConfigStats special_cfgstats[MAGIC_ITEMS_MAX];
    struct InstanceInfo instance_info[MAGIC_ITEMS_MAX]; //count in crtr_conf
    struct MagicStats keeper_power_stats[POWER_TYPES_MAX]; // should get merged into PowerConfigStats
};

/******************************************************************************/
extern const char keeper_magic_file[];
extern struct NamedCommand spell_desc[];
extern struct NamedCommand shot_desc[];
extern struct NamedCommand power_desc[];
extern struct SpellConfig spell_config[];
extern const struct NamedCommand powermodel_properties_commands[];
extern const struct LongNamedCommand powermodel_castability_commands[];
extern const struct NamedCommand powermodel_expand_check_func_type[];
extern const struct NamedCommand magic_power_commands[];
extern const Expand_Check_Func powermodel_expand_check_func_list[];
/******************************************************************************/
struct SpellConfig *get_spell_config(int mgc_idx);
TbBool spell_config_is_invalid(const struct SpellConfig *mgcinfo);
TextStringId get_power_description_strindex(PowerKind pwkind);
TextStringId get_power_name_strindex(PowerKind pwkind);
TbBool power_is_instinctive(int pwkind);
long get_power_index_for_work_state(long work_state);
long get_special_description_strindex(int spckind);
struct SpellConfigStats *get_spell_model_stats(SpellKind spmodel);
struct ShotConfigStats *get_shot_model_stats(ThingModel tngmodel);
struct PowerConfigStats *get_power_model_stats(PowerKind pwmodel);
TbBool power_model_stats_invalid(const struct PowerConfigStats *powerst);
struct MagicStats *get_power_dynamic_stats(PowerKind pwkind);
struct SpecialConfigStats *get_special_model_stats(SpecialKind spckind);
const char *spell_code_name(SpellKind spmodel);
const char *shot_code_name(ThingModel tngmodel);
const char *power_code_name(PowerKind pwkind);
int power_model_id(const char * code_name);
/******************************************************************************/
TbBool load_magic_config(const char *conf_fname,unsigned short flags);
TbBool make_all_powers_cost_free(void);
TbBool make_all_powers_researchable(PlayerNumber plyr_idx);
TbBool set_power_available(PlayerNumber plyr_idx, PowerKind spl_idx, long resrch, long avail);
TbBool is_power_available(PlayerNumber plyr_idx, PowerKind spl_idx);
TbBool is_power_obtainable(PlayerNumber plyr_idx, PowerKind pwkind);
TbBool add_power_to_player(PowerKind spl_idx, PlayerNumber plyr_idx);
void remove_power_from_player(PowerKind spl_idx, PlayerNumber plyr_idx);
TbBool make_available_all_researchable_powers(PlayerNumber plyr_idx);
/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
